// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <string>
#include <sstream>
#include <assert.h> 
#include <algorithm>

#include "vx_log.h"
#include "vx_math.h"
#include "vx_util.h"

#include "ndbc_handler.h"

static const char *stations_env = "MET_NDBC_STATIONS";

const int NdbcHandler::NDBC_FORMAT_VERSION_STANDARD = 1;
const int NdbcHandler::NDBC_FORMAT_VERSION_UNKNOWN= -1;

const int NdbcHandler::NUM_COLS_STANDARD = 19;
const int NdbcHandler::NUM_DATA_COLS_STANDARD = 14;

// Time related header values
const string hdr_year = "#YY";
const string hdr_month = "MM";
const string hdr_day = "DD";
const string hdr_hour = "hh";
const string hdr_minute = "mm";

// Data related header values
const string hdr_wind_dir = "WDIR";
const string hdr_wind_speed = "WSPD";
const string hdr_gust_speed = "GST";
const string hdr_wave_height = "WVHT";
const string hdr_dominant_wave_period = "DPD";
const string hdr_average_wave_period = "APD";
const string hdr_dominant_wave_direction = "MWD";
const string hdr_sea_level_pressure = "PRES";
const string hdr_air_temp = "ATMP";
const string hdr_sea_surface_temp = "WTMP";
const string hdr_dewpoint = "DEWP";
const string hdr_visibility = "VIS";
const string hdr_pressure_tendency = "PTDY";
const string hdr_tide = "TIDE";

////////////////////////////////////////////////////////////////////////


//
//  Code for class NdbcHandler
//


////////////////////////////////////////////////////////////////////////


NdbcHandler::NdbcHandler(const string &program_name) :
  FileHandler(program_name)
{
  use_var_id = true;
  format_version = NDBC_FORMAT_VERSION_UNKNOWN;

  ConcatString fname;

  //
  // search for MET_NDBC_STATIONS environment variable
  // if not defined, use a hardwired value
  //
  if(get_env(stations_env, fname)) {
    locationsFileName = fname;
  } else {
    locationsFileName = "MET_BASE/table_files/ndbc_stations.xml";
  }

  // read in and parse the locations file
  if (!locations.initialize(locationsFileName)) {
    mlog << Error << "\nCannot initialize NDBC station loations file: "
         << locationsFileName << "\n\n";
    exit(1);
  }

  //
  // store column info for all the data columns (column names)
  // NOTE these will be used as index values in the observations
  // 0 = wind dir, 1 = wind speed, etc.
  //
  column.push_back(Column(hdr_wind_dir));
  column.push_back(Column(hdr_wind_speed));
  column.push_back(Column(hdr_gust_speed));
  column.push_back(Column(hdr_wave_height));
  column.push_back(Column(hdr_dominant_wave_period));
  column.push_back(Column(hdr_average_wave_period));
  column.push_back(Column(hdr_dominant_wave_direction));
  column.push_back(Column(hdr_sea_level_pressure));
  column.push_back(Column(hdr_air_temp));
  column.push_back(Column(hdr_sea_surface_temp));
  column.push_back(Column(hdr_dewpoint));
  column.push_back(Column(hdr_visibility));
  column.push_back(Column(hdr_pressure_tendency));
  column.push_back(Column(hdr_tide));

  numMissingStations = 0;
}

////////////////////////////////////////////////////////////////////////


NdbcHandler::~NdbcHandler()
{
  mlog << Debug(1) << "Number of NDBC skipped files due to no lookup " <<  numMissingStations
       << "\n";
}


////////////////////////////////////////////////////////////////////////


bool NdbcHandler::isFileType(LineDataFile &ascii_file) const
{
  //
  // Initialize the return value.
  //
  bool is_file_type = false;

  //
  // Read the first line from the file.
  // which should be the header
  //
  DataLine dl;
  dl.set_delimiter(" ");
  ascii_file >> dl;
  string line = dl.get_line();
  ConcatString cstring(line);

  // see if it fits the NDBC standard, the header line has the same
  // number of columns as everything else
  StringArray tokens = cstring.split(" ");
  if (NUM_COLS_STANDARD == tokens.n_elements()) {
    // look at the first token it should be #YY
    if (tokens[0] == "#YY") {
      is_file_type = true;
      return is_file_type;
    }
  }
  return is_file_type;
}


////////////////////////////////////////////////////////////////////////


void NdbcHandler::setFormatVersion(int version) {

  format_version = version;
}

////////////////////////////////////////////////////////////////////////
// Private/Protected methods
////////////////////////////////////////////////////////////////////////

bool NdbcHandler::_readObservations(LineDataFile &ascii_file)
{

  DataLine data_line;
  string method_name = "NdbcHandler::_readObservations() ";

  if (format_version == NDBC_FORMAT_VERSION_UNKNOWN) {
    // try to figure out which one it is and set it
    if (!_determineFileType(ascii_file)) {
      return false;
    }
  }
  
  if (!_readHeaderInfo(ascii_file))
    return false;

  string fname = ascii_file.filename();
  // parse this name to set the station id, and use that to set up the lat/lon/elev
  if (!_setStationInfo(fname)) {
    numMissingStations++;
    return true;
  }

  //
  // Process the observation lines
  //
  int bad_line_count = 0;
  data_line.set_delimiter(" ");

  // skip the next line which is also a header
  // (can pull out units if we want)
  ascii_file >> data_line;

  while (ascii_file >> data_line) {
    if (!_parseObservationLineStandard(data_line, ascii_file.filename())) {
      bad_line_count++;
    }
  }
  return true;
}
  
////////////////////////////////////////////////////////////////////////

bool NdbcHandler::_parseObservationLineStandard(DataLine &data_line,
                                                const string &filename)
{
  string method_name = "NdbcHandler::_parseObservationLineStandard() ";

  if (format_version != NDBC_FORMAT_VERSION_STANDARD) {
    mlog << Warning << "\n" << method_name << "->"
         << "Standard NDBC format is the only supported format: " 
         << filename << "\n\n";
    return false;
  }

  //
  // Make sure that the line contains the correct number of tokens
  //
  if (data_line.n_items() != NUM_COLS_STANDARD) {
    mlog << Warning << "\n" << method_name << "-> "
         << "Skipping line number " << data_line.line_number()
         << " with an unexpected number of columns ("
         << data_line.n_items() << " != " << NUM_COLS_STANDARD << "): "
         << filename << "\n\n";
    return false;
  }

  //
  // extract the time information
  //
  time_t valid_time = _getValidTime(data_line);
  if (valid_time == 0) {
    mlog << Warning << "\n" << method_name << "-> "
         << "Skipping line number " << data_line.line_number()
         << " whose vaild time cannot not be parsed: "
         << filename << "\n\n";
    return false;
  }

  string quality_flag = na_str;
  int grib_code = 0;
  double pressure_level_hpa = bad_data_double;
  double height_m = stationAlt;
  string header_type = "NDBC_STANDARD";
  
  double value;
  string name;
  for (size_t i=0; i<column.size(); ++i) {
    value = _extractDouble(data_line, column[i].ptr);
    // here could check for missing and not output that variable?
    name = column[i].name;
    grib_code = i;  // it's not actually grib code, its obs_vid, according to howard
    _addObservations(Observation(header_type, stationId, valid_time,
                                 stationLat, stationLon, stationAlt,
                                 quality_flag, grib_code, pressure_level_hpa,
                                 height_m, value, name));
  }
  return true;
}

  
////////////////////////////////////////////////////////////////////////

bool NdbcHandler::_setStationInfo(const string &filename)
{
  // look for last '/' in case its a path, not just a name
  std::size_t i0;
  string fname;
  i0 = filename.find_last_of("/");
  if (i0 == string::npos) {
    fname = filename;
  } else {
    fname = filename.substr(i0+1);
  }
  // expect <stationid>.txt as the name
  i0 = fname.find(".txt");
  if (i0 == string::npos) {
    mlog << Warning << "\n" << "NDBC file name does not follow the "
         << "expected '<stationid>.txt' format: " << fname << "\n\n";
    return false;
  }
  stationId = fname.substr(0, i0);
  if (!locations.lookupLatLonElev(stationId, stationLat, stationLon,
                                  stationAlt)) {
    mlog << Warning << "\n" << "NDBC station " << stationId
         << " location information not found: " << filename << "\n\n";
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////

bool NdbcHandler::_determineFileType(LineDataFile &ascii_file)
{
  //
  // Read the first line from the file.
  //
  DataLine dl;
  dl.set_delimiter(" ");
  ascii_file >> dl;
  ascii_file.rewind();
  string line = dl.get_line();
  ConcatString cstring(line);

  StringArray tokens = cstring.split(" ");
  if (NUM_COLS_STANDARD == tokens.n_elements()) {
    if (tokens[0] == "#YY") {
      format_version = NDBC_FORMAT_VERSION_STANDARD;
      return true;
    }
  }
  format_version = NDBC_FORMAT_VERSION_UNKNOWN;
  mlog << Warning << "\nNdbcHandler::_determineFileType -> "
       << "Unknown file type: " << ascii_file.filename() << "\n\n";
  return false;
}

////////////////////////////////////////////////////////////////////////

time_t NdbcHandler::_getValidTime(const DataLine &data_line) const
{
  //
  // Pull out the date information
  //
  if (column_pointer_year < 0 || column_pointer_month  < 0 || column_pointer_day < 0 ||
      column_pointer_hour < 0 || column_pointer_minute < 0) {
    mlog << Warning << "\nNdbcHandler::_getValidTime -> "
         << "Not all time related column pointers are set.\n\n";
    return 0;
  }
  string year = _extractColumn(data_line, column_pointer_year);
  string month = _extractColumn(data_line, column_pointer_month);
  string day = _extractColumn(data_line, column_pointer_day);
  string hour = _extractColumn(data_line, column_pointer_hour);
  string min = _extractColumn(data_line, column_pointer_minute);
  string sec = "00";
  
  if (year.size() == 2) {
    // assume it's at least the year 2000 and this is the last 2 digits
    // (really should look at file name).  This will work till 2099 if
    // assumptions are true
    year = "20" + year;
  }

  //
  // Set up the time structure
  //
  struct tm time_struct;
  memset(&time_struct, 0, sizeof(time_struct));

  time_struct.tm_year = atoi(year.c_str()) -1900;
  time_struct.tm_mon = atoi(month.c_str()) - 1;
  time_struct.tm_mday = atoi(day.c_str());
  time_struct.tm_hour = atoi(hour.c_str());
  time_struct.tm_min = atoi(min.c_str());
  time_struct.tm_sec = atoi(sec.c_str());
  return timegm(&time_struct);
}

////////////////////////////////////////////////////////////////////////

bool NdbcHandler::_readHeaderInfo(LineDataFile &ascii_file)
{
  // initialize all pointers to not set
  column_pointer_year = -1;
  column_pointer_month = -1;
  column_pointer_day = -1;
  column_pointer_hour = -1;
  column_pointer_minute = -1;
  for (size_t i=0; i<column.size(); ++i) {
    column[i].clear();
  }

  DataLine data_line;
  data_line.set_delimiter(" ");

  //
  // The first line of the file contains the headers
  //
  if (!(ascii_file >> data_line)) {
    mlog << Warning << "\nNdbcHandler::_readHeaderInfo() -> "
         << "Problem reading header line from input ASCII file: "
         << ascii_file.filename() << "\n\n";
    return false;
  }

  //
  // Check for the correct number of columns in the header line
  //
  if (data_line.n_items() != NUM_COLS_STANDARD) {
    mlog << Warning << "\nNdbcHandler::_readHeaderInfo() -> "
         << "Unexpected number of header columns (" << data_line.n_items()
         << " != " << NUM_COLS_STANDARD << "): "
         << ascii_file.filename() << "\n\n";
    return false;
  }

  //
  // look at each column, attempt to match to what we want and store column index
  //
  bool status = true;
  for (int i=0; i<NUM_COLS_STANDARD; ++i) {
    string s = _extractColumn(data_line, i);
    if (s == hdr_year) {
      column_pointer_year = i;
    } else if (s == hdr_month) {
      column_pointer_month = i;
    } else if (s == hdr_day) {
      column_pointer_day = i;
    } else if (s == hdr_hour) {
      column_pointer_hour = i;
    } else if (s == hdr_minute) {
      column_pointer_minute = i;
    } else {
      bool found = false;
      for (size_t j=0; j<column.size(); ++j) {
        if (column[j].nameEquals(s)) {
          column[j].setPtr(i);
          found = true;
          break;
        }
      }
      if (!found) {
        mlog << Warning << "\nNdbcHandler::_readHeaderInfo() -> "
             << "Unexpected header column (" << s << "): "
             << ascii_file.filename() << "\n\n";
        status = false;
      }
    }
  }
  if (column_pointer_year   == -1 || column_pointer_month == -1 ||
      column_pointer_day    == -1 || column_pointer_hour  == -1 ||
      column_pointer_minute == -1) {
    mlog << Warning << "\nNdbcHandler::_readHeaderInfo() -> "
         << "NDBC file did not have all time fields in header: "
         << ascii_file.filename() << "\n\n";
    status = false;
  } 
  for (size_t j=0; j<column.size(); ++j) {
    if (column[j].notSet()) {
      mlog << Warning << "\nNdbcHandler::_readHeaderInfo() -> "
           << "NDBC file did not have all expected fields in header: "
           << ascii_file.filename() << "\n\n";
      status = false;
      break;
    }
  }
  
  return status;
}

////////////////////////////////////////////////////////////////////////

string NdbcHandler::_extractColumn(const DataLine &data_line, int ptr) const
{
  string c = data_line[ptr];
  // if you see a '\r' at the end remove that 
  std::size_t i1 = c.find_last_of("\r");
  if (i1 == string::npos) {
    return c;
  }
  return c.substr(0, i1-1);
}

////////////////////////////////////////////////////////////////////////

double NdbcHandler::_extractDouble(const DataLine &data_line, int ptr) const
{
  string c = _extractColumn(data_line, ptr);
  // 'MM' is missing, but also need to guess which '99' containing values
  // should be missing, based on documentation here:
  //     https://www.ndbc.noaa.gov/measdes.shtml
  // which suggests 'anything with only 9s is missing data'
  if (c == "MM" ||
      c == "99" ||     c == "99.0" ||
      c == "999" ||    c == "999.0" ||
      c == "9999" ||   c == "9999.0" ||
      c == "99999" ||  c == "99999.0" ||
      c == "999999" || c == "999999.0" ||
      c == "9999999" ||c == "9999999.0" ||
      c == "-99" ||     c == "-99.0" ||
      c == "-999" ||    c == "-999.0" ||
      c == "-9999" ||   c == "-9999.0" ||
      c == "-99999" ||  c == "-99999.0" ||
      c == "-999999" || c == "-999999.0" ||
      c == "-9999999" ||c == "-9999999.0")  {
    return bad_data_double;
  }

  double value = atof(c.c_str());
  return value;
}
