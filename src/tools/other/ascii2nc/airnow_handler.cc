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

#include "airnow_handler.h"

static const char *AIRNOW_NA_STR = "N/A";
static const char *airnow_stations_env = "MET_AIRNOW_STATIONS";

const int AirnowHandler::AIRNOW_FORMAT_VERSION_HOURLY = 1;
const int AirnowHandler::AIRNOW_FORMAT_VERSION_HOURLYAQOBS = 2;
const int AirnowHandler::AIRNOW_FORMAT_VERSION_DAILYV2 = 3;
const int AirnowHandler::AIRNOW_FORMAT_VERSION_UNKNOWN = -1;

const int AirnowHandler::NUM_COLS_HOURLY = 9;
const int AirnowHandler::NUM_COLS_HOURLYAQOBS = 34;
const int AirnowHandler::NUM_COLS_DAILYV2 = 13;

// header values shared by more than one format
const string hdr_aqsid = "AQSID";
const string hdr_site = "SiteName";
const string hdr_lat = "Latitude";
const string hdr_lon = "Longitude";
const string hdr_valid_date = "ValidDate";
const string hdr_source = "DataSource";
const string hdr_valid_time = "ValidTime";
const string hdr_param = "ParameterName";
const string hdr_units = "ReportingUnits";
const string hdr_value = "Value";

// header values specific to dailyv2 - defined by this software as there is no header in the data
const string hdr_dailyv2_ave_period = "AveragingPeriod";
const string hdr_dailyv2_aqi_value = "AQIValue";
const string hdr_dailyv2_aqi_category = "AQICategory";
const string hdr_dailyv2_full_aqsid = "FullAQSID";

// header values specific to hourlyaqobs
const string hdr_hourlyaqobs_status = "Status";
const string hdr_hourlyaqobs_epa_region = "EPARegion";
const string hdr_hourlyaqobs_elevation = "Elevation";
const string hdr_gmtoffset = "GMTOffset";
const string hdr_hourlyaqobs_countrycode = "CountryCode";
const string hdr_hourlyaqobs_statename = "StateName";
const string hdr_hourlyaqobs_reportingarea = "ReportingArea_PipeDelimited";
const string hdr_hourlyaqobs_ozone_aqi = "OZONE_AQI";
const string hdr_hourlyaqobs_ozone_measured = "OZONE_Measured";
const string hdr_hourlyaqobs_ozone = "OZONE";
const string hdr_hourlyaqobs_ozone_units = "OZONE_Unit";
const string hdr_hourlyaqobs_pm10_aqi = "PM10_AQI";
const string hdr_hourlyaqobs_pm10_measured = "PM10_Measured";
const string hdr_hourlyaqobs_pm10 = "PM10";
const string hdr_hourlyaqobs_pm10_units = "PM10_Unit";
const string hdr_hourlyaqobs_pm25_aqi = "PM25_AQI";
const string hdr_hourlyaqobs_pm25_measured = "PM25_Measured";
const string hdr_hourlyaqobs_pm25 = "PM25";
const string hdr_hourlyaqobs_pm25_units = "PM25_Unit";
const string hdr_hourlyaqobs_no2_aqi = "NO2_AQI";
const string hdr_hourlyaqobs_no2_measured = "NO2_Measured";
const string hdr_hourlyaqobs_no2 = "NO2";
const string hdr_hourlyaqobs_no2_units = "NO2_Unit";
const string hdr_hourlyaqobs_co = "CO";
const string hdr_hourlyaqobs_co_units = "CO_Unit";
const string hdr_hourlyaqobs_so2 = "SO2";
const string hdr_hourlyaqobs_so2_units = "SO2_Unit";

string remove_quotes(string &s);
bool doubleOrMissing(const string &s, double &value);
vector<string> parseHourlyAqobsLine(const string &asciiLine, bool &ok);

////////////////////////////////////////////////////////////////////////


//
//  Code for class AirnowHandler
//


////////////////////////////////////////////////////////////////////////


AirnowHandler::AirnowHandler(const string &program_name) :
  FileHandler(program_name)
{
  use_var_id = true;
  format_version = AIRNOW_FORMAT_VERSION_UNKNOWN;

  ConcatString fname;

  //
  // search for MET_AIRNOW_STATIONS environment variable
  // if not defined, use a hardwired value
  //
  if(get_env(airnow_stations_env, fname)) {
    monitoringSiteFileName = fname;
  } else {
    monitoringSiteFileName = "MET_BASE/table_files/airnow_monitoring_site_locations_v2.txt";
  }
}

////////////////////////////////////////////////////////////////////////


AirnowHandler::~AirnowHandler()
{
}


////////////////////////////////////////////////////////////////////////



bool AirnowHandler::isFileType(LineDataFile &ascii_file) const
{
  //
  // Initialize the return value.
  //
  bool is_file_type = false;

  //
  // Read the first line from the file.
  //
  DataLine dl;
  dl.set_delimiter(",");
  ascii_file >> dl;
  string line = dl.get_line();
  ConcatString cstring(line);

  // try delimeter "|" and check for DAILYV2 or HOURLY
  StringArray tokens = cstring.split("|");
  if (NUM_COLS_DAILYV2 == tokens.n()) {
    is_file_type = true;
    return is_file_type;
  } else if (NUM_COLS_HOURLY == tokens.n()) {
    is_file_type = true;
    return is_file_type;
  }

  // try with "," delimiter and look for HOURLYABQ
  StringArray tokens2 = cstring.split(",");
  if (NUM_COLS_HOURLYAQOBS == tokens2.n()) {
    is_file_type = true;
    return is_file_type;
  }
  return is_file_type;
}

void AirnowHandler::setFormatVersion(int version) {

  format_version = version;
}

////////////////////////////////////////////////////////////////////////
// Private/Protected methods
////////////////////////////////////////////////////////////////////////

bool AirnowHandler::_readObservations(LineDataFile &ascii_file)
{
  DataLine data_line;
  string method_name = "AirnowHandler::_readObservations() -> ";
  string header_type;
  string delimiter;
  int column_cnt;

  _initializeColumnPointers();
  
  if (format_version == AIRNOW_FORMAT_VERSION_UNKNOWN) {
    // try to figure out which one it is and set it
    if (!_determineFileType(ascii_file)) {
      return false;
    }
  }
  
  if (format_version == AIRNOW_FORMAT_VERSION_DAILYV2) {
    doStripQuotes = false;
    _setDailyv2HeaderInfo();
    header_type = "AIRNOW_DAILY_V2";
    delimiter = "|";
    column_cnt = NUM_COLS_DAILYV2;
  }
  else if (format_version == AIRNOW_FORMAT_VERSION_HOURLYAQOBS) {
    doStripQuotes = true;
    if (!_readHeaderInfo(ascii_file)) return false;

    header_type = "AIRNOW_HOURLY_AQOBS";
    delimiter = ",";
    column_cnt = NUM_COLS_HOURLYAQOBS;
  }
  else if (format_version == AIRNOW_FORMAT_VERSION_HOURLY) {
    doStripQuotes = false;
    _setHourlyHeaderInfo();
    if (!locations.initialize(monitoringSiteFileName)) return false;
    header_type = "AIRNOW_HOURLY";
    delimiter = "|";
    column_cnt = NUM_COLS_HOURLY;
  }
  else {
    mlog << Error << method_name
         << "format value=" << format_version << " expect "
         << AIRNOW_FORMAT_VERSION_DAILYV2 << " or " << AIRNOW_FORMAT_VERSION_HOURLYAQOBS
         << " or " << AIRNOW_FORMAT_VERSION_HOURLY << "\n\n";
    header_type = "AIRNOW_HOURLY";
    return false;
  }

  if (format_version == AIRNOW_FORMAT_VERSION_HOURLYAQOBS) {
    // need to process this a special way because of comma separated strings with
    // commas within some strings
    return _readObservationsHourlyAqobs(ascii_file, column_cnt, delimiter, header_type);
  } else {
    return _readObservationsStandard(ascii_file, column_cnt, delimiter, header_type);
  }
}

////////////////////////////////////////////////////////////////////////

bool AirnowHandler::_readObservationsStandard(LineDataFile &ascii_file,
                                              int column_cnt, const string &delimiter,
                                              const string &header_type)
{
  DataLine data_line;
  //
  // Process the observation lines
  //
  int bad_line_count = 0;
  data_line.set_delimiter(delimiter.c_str());
  while (ascii_file >> data_line) {
    if (!_parseObservationLineStandard(data_line, ascii_file.filename(),
                                       column_cnt, header_type)) {
      bad_line_count++;
    }
  }
  return true;
}
  
////////////////////////////////////////////////////////////////////////

bool AirnowHandler::_parseObservationLineStandard(DataLine &data_line,
                                                  const string &filename,
                                                  int column_cnt,
                                                  const string &header_type)
{
  string method_name = "AirnowHandler::_parseObservationLineStandard() -> ";

  //
  // Make sure that the line contains the correct number of tokens
  //
  if (data_line.n_items() != column_cnt) {
    mlog << Error << "\n" << method_name
         << "line number " << data_line.line_number()
         << " does not have the correct number of columns " << data_line.n_items()
         << " (" << column_cnt << "). Skipping this line in \""
         << filename << "\".\n\n";
    return false;
  }

  time_t valid_time = _getValidTime(data_line);
  if (valid_time == 0) {
    mlog << Error << "\n" << method_name
         << "line number " << data_line.line_number()
         << " time could not be parsed, skipping this line in \""
         << filename << "\".\n\n";
    return false;
  }
    
  // fill in expected things
  double lat, lon, elev;
  string stationId = _extractColumn(data_line, stationIdPtr);
  string col;
    
  if (format_version == AIRNOW_FORMAT_VERSION_HOURLY) {

    // skip lines for which no location is found
    if (!locations.lookupLatLonElev(stationId, lat, lon, elev)) {
      mlog << Warning << "\n" << method_name
           << "Skipping line number " << data_line.line_number()
           << " since StationId " << stationId << " not found in locations file ("
           << monitoringSiteFileName << ")! Set the " << airnow_stations_env
           << " environment variable to define an updated version.\n\n";
      return false;
    }
  } else {
    col = _extractColumn(data_line, latPtr);
    lat = atof(col.c_str());
    col = _extractColumn(data_line, lonPtr);
    lon = atof(col.c_str());
    if (elevPtr >= 0) {
      col = _extractColumn(data_line, elevPtr);
      elev = atof(col.c_str());
    } else {
      elev = bad_data_double;
    }
  }
  string varName;
  string units;
  double value;
  int avgPeriodSec;
  int aqiValue;
  int aqiCategory;
  
  if (format_version == AIRNOW_FORMAT_VERSION_DAILYV2) {

    varName      = _extractColumn(data_line, varnamePtr);
    units        = _extractColumn(data_line, unitsPtr);
    col          = _extractColumn(data_line, valuePtr);
    value        = atof(col.c_str());
    col          = _extractColumn(data_line, avgperiodPtr);
    avgPeriodSec = atoi(col.c_str()) * 3600;
    col          = _extractColumn(data_line, aqiPtr);
    aqiValue     = atoi(col.c_str());
    col          = _extractColumn(data_line, aqiCategoryPtr);
    aqiCategory  = atoi(col.c_str());

    // add the observation
    _addObservations(Observation(header_type, stationId, valid_time,
                                 lat, lon, elev, na_str, _getVarIndex(varName, units),
                                 avgPeriodSec, bad_data_double,
                                 value, varName));

  } else if (format_version == AIRNOW_FORMAT_VERSION_HOURLY) {

    varName      = _extractColumn(data_line, varnamePtr);
    units        = _extractColumn(data_line, unitsPtr);
    col          = _extractColumn(data_line, valuePtr);
    value        = atof(col.c_str());

   // averaging period is 1-hour
    avgPeriodSec = 3600;

    // add the observation
    _addObservations(Observation(header_type, stationId, valid_time,
                                 lat, lon, elev, na_str, _getVarIndex(varName, units),
                                 avgPeriodSec, bad_data_double,
                                 value, varName));
  }
  return true;
}

////////////////////////////////////////////////////////////////////////

bool AirnowHandler::_readObservationsHourlyAqobs(LineDataFile &ascii_file,
                                                 int column_cnt, const string &delimiter,
                                                 const string &header_type)
{
  string method_name = "AirnowHandler::_readObservationsHourlyAqobs() -> ";

  //
  // Process the observation lines
  //
  int bad_line_count = 0;

  //
  // hack set the delimiter to something invalid so you get one big string
  //
  DataLine data_line;
  data_line.set_delimiter("%$@");

  int lineNumber = -1;
  while (ascii_file >> data_line) {
    lineNumber++;
    if (!_parseObservationLineAqobs(data_line[0], column_cnt, header_type,
                                    lineNumber, ascii_file.filename())) {
      ++bad_line_count;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////////

bool AirnowHandler::_parseObservationLineAqobs(const string &data_line,
                                               int column_cnt,
                                               const string &header_type,
                                               int lineNumber,
                                               const string &filename)
{
  string method_name = "AirnowHandler::_parseObservationLineAqobs() -> ";

  bool ok = true;
  vector<string> tokens = parseHourlyAqobsLine(data_line, ok);
  if (!ok) {
    return false;
  }

  if ((int)tokens.size()  != column_cnt) {
    mlog << Error << "\nAirnowHandler" << method_name
         << "line number " << lineNumber
         << " does not have the correct number of columns " << tokens.size()
         << " (" << column_cnt << "). Skipping this line in \""
         << filename << "\".\n\n";
    // for now just skip this line
    return false;
  }
  time_t valid_time = _getValidTime(tokens);
  if (valid_time == 0) {
    mlog << Error << "\n" << method_name
         << "line number " << lineNumber
         << " time could not be parsed, skipping this line in \""
         << filename << "\".\n\n";
    return false;
  }
    
  // fill in expected things
  double lat, lon, elev;
  string stationId = tokens[stationIdPtr];
  string col;
    
  lat = atof(tokens[latPtr].c_str());
  lon = atof(tokens[lonPtr].c_str());
  if (elevPtr >= 0) {
    elev = atof(tokens[elevPtr].c_str());
  } else {
    elev = 0.0;
  }
    
  _addHourlyAqobsObs(tokens, header_type, stationId, valid_time, lat, lon, elev,
                     ozoneMeasuredPtr, ozoneAqiPtr, ozonePtr, ozoneUnitPtr,
                     hdr_hourlyaqobs_ozone);
  _addHourlyAqobsObs(tokens, header_type, stationId, valid_time, lat, lon, elev,
                     pm10MeasuredPtr, pm10AqiPtr, pm10Ptr, pm10UnitPtr,
                     hdr_hourlyaqobs_pm10);
  _addHourlyAqobsObs(tokens, header_type, stationId, valid_time, lat, lon, elev,
                     pm25MeasuredPtr, pm25AqiPtr, pm25Ptr, pm25UnitPtr,
                     hdr_hourlyaqobs_pm25);
  _addHourlyAqobsObs(tokens, header_type, stationId, valid_time, lat, lon, elev,
                     no2MeasuredPtr, no2AqiPtr, no2Ptr, no2UnitPtr,
                     hdr_hourlyaqobs_no2);
  _addHourlyAqobsObs(tokens, header_type, stationId, valid_time, lat, lon, elev,
                     coPtr, coUnitPtr,  hdr_hourlyaqobs_co);
  _addHourlyAqobsObs(tokens, header_type, stationId, valid_time, lat, lon, elev,
                     so2Ptr, so2UnitPtr,  hdr_hourlyaqobs_so2);
  return true;
}

////////////////////////////////////////////////////////////////////////

bool AirnowHandler::_determineFileType(LineDataFile &ascii_file)
{
  //
  // Read the first line from the file.
  //
  DataLine dl;
  dl.set_delimiter(",");
  ascii_file >> dl;
  ascii_file.rewind();
  string line = dl.get_line();
  ConcatString cstring(line);

  // try delimeter "|" and check for DAILYV2 or HOURLY
  StringArray tokens = cstring.split("|");
  if (NUM_COLS_DAILYV2 == tokens.n()) {
    format_version = AIRNOW_FORMAT_VERSION_DAILYV2;
    return true;
  } else if (NUM_COLS_HOURLY == tokens.n()) {
    format_version = AIRNOW_FORMAT_VERSION_HOURLY;
    return true;
  }

  // try with "," delimiter and look for HOURLYABQ
  StringArray tokens2 = cstring.split(",");
  if (NUM_COLS_HOURLYAQOBS == tokens2.n()) {
    format_version = AIRNOW_FORMAT_VERSION_HOURLYAQOBS;
    return true;
  }
  format_version = AIRNOW_FORMAT_VERSION_UNKNOWN;
  mlog << Error << "\nAirnowHandler::_determineFileType -> "
       << "Unknown file type\n\n";
  return false;
}

////////////////////////////////////////////////////////////////////////

void AirnowHandler::_addHourlyAqobsObs(const vector<string> &data_line, const string &header_type,
                                       const string &stationId, const time_t &valid_time,
                                       double lat, double lon, double elev,
                                       int measuredPtr, int aqiPtr, int valuePtr,
                                       int unitPtr, const string &varname)
{
  string col;
  int status;
  int aqi;
  double value;
  string units;

  // averging period is 1-hour
  int avgPeriodSec = 3600;
  
  status = atoi(data_line[measuredPtr].c_str());
  if (status == 1) {
    aqi = atoi(data_line[aqiPtr].c_str());
    if (doubleOrMissing(data_line[valuePtr], value)) {
      units = data_line[unitPtr];

      // add the observation
      _addObservations(Observation(header_type, stationId, valid_time,
                                   lat, lon, elev, na_str, _getVarIndex(varname, units),
                                   avgPeriodSec, bad_data_double,
                                   value, varname));
    }        
  }
}

////////////////////////////////////////////////////////////////////////


void AirnowHandler::_addHourlyAqobsObs(const vector<string> &data_line, const string &header_type,
                                       const string &stationId, const time_t &valid_time,
                                       double lat, double lon, double elev,
                                       int valuePtr, int unitPtr, const string &varname)
{
  string col;
  double value;
  string units;

  // averging period is 1-hour
  int avgPeriodSec = 3600;

  if (doubleOrMissing(data_line[valuePtr], value)) {
    units = data_line[unitPtr];

    // add the observation
    _addObservations(Observation(header_type, stationId, valid_time,
                                 lat, lon, elev, na_str, _getVarIndex(varname, units),
                                 avgPeriodSec, bad_data_double,
                                 value, varname));
  }
}

////////////////////////////////////////////////////////////////////////

time_t AirnowHandler::_getValidTime(const DataLine &data_line) const
   
{
  //
  // Pull out the date information
  //
  if (datePtr  < 0) {
    mlog << Error << "\nAirnowHandler::_getValidTime -> "
         << "Date column pointer is not set\n\n";
    return 0;
  }
  string dateStr = _extractColumn(data_line, datePtr);
  string timeStr;
  if (timePtr >= 0) {
    timeStr = _extractColumn(data_line, timePtr);
  } else {
    timeStr = "";
  }
  return _getValidTime(dateStr, timeStr);
}

////////////////////////////////////////////////////////////////////////

time_t AirnowHandler::_getValidTime(const vector<string> &data_line) const
{
  //
  // Pull out the date information
  //
  if (datePtr  < 0) {
    mlog << Error << "\nAirnowHandler::_getValidTime -> "
         << "Date column pointer is not set\n\n";
    return 0;
  }
  string dateStr = data_line[datePtr];
  string timeStr;
  if (timePtr >= 0) {
    timeStr = data_line[timePtr];
  } else {
    timeStr = "";
  }
  return _getValidTime(dateStr, timeStr);
}

////////////////////////////////////////////////////////////////////////

time_t AirnowHandler::_getValidTime(const string &dateStr, const string &timeStr) const
{
  string mon, mday, year;
  string hour, min, sec;
  
  ConcatString date_string(dateStr);
  StringArray dateTokens = date_string.split("/");
  if (1 == dateTokens.n()) {
    mlog << Error << "\nAirnowHandler::_getValidTime -> "
         << "Not supported date: \"" << date_string << "\".\n\n";
    return 0;
  }
  mon = dateTokens[0];
  mday = dateTokens[1];
  year = dateTokens[2];
  if (year.size() == 2) {
    // assume it's at least the year 2000 and this is the last 2 digits
    // (really should look at file name).  This will work till 2099 if
    // assumptions are true
    year = "20" + year;
  }
  
  hour = "00";
  min = "00";
  sec = "00";
  if (!timeStr.empty()) {
    //
    // Pull out the time information, which is optional
    //
    ConcatString time_string(timeStr);
    StringArray timeTokens = time_string.split(":");
    if (1 == timeTokens.n()) {
      // assume its hour
      hour = timeTokens[0];
    } else if (2 == timeTokens.n()) {
      hour = timeTokens[0];
      min = timeTokens[1];
    } else if (3 == timeTokens.n()) {
      hour = timeTokens[0];
      min = timeTokens[1];
      sec = timeTokens[2];
    } else {
      mlog << Error << "\nAirnowHandler::_getValidTime -> "
           << "Not supported time: \"" << time_string << "\".\n\n";
      return 0;
    }
  }
  
  //
  // Set up the time structure
  //

  struct tm time_struct;
  memset(&time_struct, 0, sizeof(time_struct));

  time_struct.tm_year = atoi(year.c_str()) -1900;
  time_struct.tm_mon = atoi(mon.c_str()) - 1;
  time_struct.tm_mday = atoi(mday.c_str());
  time_struct.tm_hour = atoi(hour.c_str());
  time_struct.tm_min = atoi(min.c_str());
  time_struct.tm_sec = atoi(sec.c_str());
  return timegm(&time_struct);
}

////////////////////////////////////////////////////////////////////////

void AirnowHandler::_setDailyv2HeaderInfo()
{
  header_names.clear();
  header_names.add(hdr_valid_date);
  datePtr = 0;
  header_names.add(hdr_aqsid);
  header_names.add(hdr_site);
  header_names.add(hdr_param);
  varnamePtr = 3;
  header_names.add(hdr_units);
  unitsPtr = 4;
  header_names.add(hdr_value);
  valuePtr = 5;
  header_names.add(hdr_dailyv2_ave_period);
  avgperiodPtr = 6;
  header_names.add(hdr_source);
  header_names.add(hdr_dailyv2_aqi_value);
  aqiPtr = 8;
  header_names.add(hdr_dailyv2_aqi_category);
  aqiCategoryPtr = 9;
  header_names.add(hdr_lat);
  latPtr = 10;
  header_names.add(hdr_lon);
  lonPtr = 11;
  header_names.add(hdr_dailyv2_full_aqsid);
  stationIdPtr = 12;
}


////////////////////////////////////////////////////////////////////////

void AirnowHandler::_setHourlyHeaderInfo()
{
  header_names.clear();
  header_names.add(hdr_valid_date);
  datePtr = 0;
  header_names.add(hdr_valid_time);
  timePtr = 1;
  header_names.add(hdr_aqsid);
  stationIdPtr = 2;
  header_names.add(hdr_site);
  header_names.add(hdr_gmtoffset);
  header_names.add(hdr_param);
  varnamePtr = 5;
  header_names.add(hdr_units);
  unitsPtr = 6;
  header_names.add(hdr_value);
  valuePtr = 7;
  header_names.add(hdr_source);
}

////////////////////////////////////////////////////////////////////////

bool AirnowHandler::_readHeaderInfo(LineDataFile &ascii_file)
{
  StringArray reference_headers;

  // set the expected stuff, noting that the order seen in a sample
  // file is different than in the documentation, meaning need to do
  // a 'one to one and onto' check refererence versus actual to be thorough
  reference_headers.clear();
  reference_headers.add(hdr_aqsid);
  reference_headers.add(hdr_site);
  reference_headers.add(hdr_hourlyaqobs_status);
  reference_headers.add(hdr_hourlyaqobs_epa_region);
  reference_headers.add(hdr_lat);
  reference_headers.add(hdr_lon);
  reference_headers.add(hdr_hourlyaqobs_elevation);
  reference_headers.add(hdr_gmtoffset);
  reference_headers.add(hdr_hourlyaqobs_countrycode);
  reference_headers.add(hdr_hourlyaqobs_statename);
  reference_headers.add(hdr_valid_date);
  reference_headers.add(hdr_valid_time);
  reference_headers.add(hdr_source);
  reference_headers.add(hdr_hourlyaqobs_reportingarea);
  reference_headers.add(hdr_hourlyaqobs_ozone_aqi);
  reference_headers.add(hdr_hourlyaqobs_pm10_aqi);
  reference_headers.add(hdr_hourlyaqobs_pm25_aqi);
  reference_headers.add(hdr_hourlyaqobs_no2_aqi);
  reference_headers.add(hdr_hourlyaqobs_ozone_measured);
  reference_headers.add(hdr_hourlyaqobs_pm10_measured);
  reference_headers.add(hdr_hourlyaqobs_pm25_measured);
  reference_headers.add(hdr_hourlyaqobs_no2_measured);
  reference_headers.add(hdr_hourlyaqobs_pm25);
  reference_headers.add(hdr_hourlyaqobs_pm25_units);
  reference_headers.add(hdr_hourlyaqobs_ozone);
  reference_headers.add(hdr_hourlyaqobs_ozone_units);
  reference_headers.add(hdr_hourlyaqobs_no2);
  reference_headers.add(hdr_hourlyaqobs_no2_units);
  reference_headers.add(hdr_hourlyaqobs_co);
  reference_headers.add(hdr_hourlyaqobs_co_units);
  reference_headers.add(hdr_hourlyaqobs_so2);
  reference_headers.add(hdr_hourlyaqobs_so2_units);
  reference_headers.add(hdr_hourlyaqobs_pm10);
  reference_headers.add(hdr_hourlyaqobs_pm10_units);

  DataLine data_line;
  data_line.set_delimiter(",");

  //
  // The first line of the file contains the headers
  //

  if (!(ascii_file >> data_line))
    {
      mlog << Error << "\nAirnowHandler::_readHeaderInfo() -> "
           << "error reading header line from input ASCII file \""
           << ascii_file.filename() << "\"\n\n";
      return false;
    }

  //
  // Check for the correct number of columns in the header line
  //

  if (data_line.n_items() != NUM_COLS_HOURLYAQOBS) {
    mlog << Error << "\nAirnowHandler::_readHeaderInfo() -> "
         << "AIRNOW file has incorrect number of columns ("
         << data_line.n_items() << ") in header line\n\n";
    return false;
  }

  //
  // look at each column and check if it is one of the
  // reference strings, while also setting pointers to
  // the columns that are of interest
  //
  
  header_names.clear();
  bool status = true;
  for (int i=0; i<NUM_COLS_HOURLYAQOBS; ++i) {
    string s = _extractColumn(data_line, i);
    if (reference_headers.has(s)) {
      header_names.add(s);
      if (s == hdr_aqsid) {
        stationIdPtr = i;
      } else if (s == hdr_valid_date) {
        datePtr = i;
      } else if (s == hdr_valid_time) {
        timePtr = i;
      } else if (s == hdr_lat) {
        latPtr = i;
      } else if (s == hdr_lon) {
        lonPtr = i;
      } else if (s == hdr_hourlyaqobs_elevation) {
        elevPtr = i;
      } else if (s == hdr_hourlyaqobs_ozone_aqi) {
        ozoneAqiPtr = i;
      } else if (s == hdr_hourlyaqobs_ozone_measured) {
        ozoneMeasuredPtr = i;
      } else if (s == hdr_hourlyaqobs_ozone) {
        ozonePtr = i;
      } else if (s == hdr_hourlyaqobs_ozone_units) {
        ozoneUnitPtr = i;
      } else if (s == hdr_hourlyaqobs_pm10_aqi) {
        pm10AqiPtr = i;
      } else if (s == hdr_hourlyaqobs_pm10_measured) {
        pm10MeasuredPtr = i;
      } else if (s == hdr_hourlyaqobs_pm10) {
        pm10Ptr = i;
      } else if (s == hdr_hourlyaqobs_pm10_units) {
        pm10UnitPtr = i;
      } else if (s == hdr_hourlyaqobs_pm25_aqi) {
        pm25AqiPtr = i;
      } else if (s == hdr_hourlyaqobs_pm25_measured) {
        pm25MeasuredPtr = i;
      } else if (s == hdr_hourlyaqobs_pm25) {
        pm25Ptr = i;
      } else if (s == hdr_hourlyaqobs_pm25_units) {
        pm25UnitPtr = i;
      } else if (s == hdr_hourlyaqobs_no2_aqi) {
        no2AqiPtr = i;
      } else if (s == hdr_hourlyaqobs_no2_measured) {
        no2MeasuredPtr = i;
      } else if (s == hdr_hourlyaqobs_no2) {
        no2Ptr = i;
      } else if (s == hdr_hourlyaqobs_no2_units) {
        no2UnitPtr = i;
      } else if (s == hdr_hourlyaqobs_co) {
        coPtr = i;
      } else if (s == hdr_hourlyaqobs_co_units) {
        coUnitPtr = i;
      } else if (s == hdr_hourlyaqobs_so2) {
        so2Ptr = i;
      } else if (s == hdr_hourlyaqobs_so2_units) {
        so2UnitPtr = i;
      }
    } else {
      mlog << Error << "\nAirnowHandler::_readHeaderInfo() -> "
           << "AIRNOW file has unknown header item " << s << "\n\n";
      status = false;
    }
  }
  return status;
}

////////////////////////////////////////////////////////////////////////


void AirnowHandler::_initializeColumnPointers()
{
  // column pointers set for both daily v2 and hourly aqobs
  stationIdPtr = -1;
  datePtr = -1;
  latPtr = -1;
  lonPtr = -1;
  varnamePtr = -1;
  unitsPtr = -1;
  valuePtr = -1;
  avgperiodPtr = -1;
  aqiPtr = -1;
  aqiCategoryPtr = -1;
  timePtr = -1;
  elevPtr = -1;
  ozoneAqiPtr = -1;
  ozoneMeasuredPtr = -1;
  ozonePtr = -1;
  ozoneUnitPtr = -1;
  pm10AqiPtr = -1;
  pm10MeasuredPtr = -1;
  pm10Ptr = -1;
  pm10UnitPtr = -1;
  pm25AqiPtr = -1;
  pm25MeasuredPtr = -1;
  pm25Ptr = -1;
  pm25UnitPtr = -1;
  no2AqiPtr = -1;
  no2MeasuredPtr = -1;
  no2Ptr = -1;
  no2UnitPtr = -1;
  coPtr = -1;
  coUnitPtr = -1;
  so2Ptr = -1;
  so2UnitPtr = -1;
}

////////////////////////////////////////////////////////////////////////

string AirnowHandler::_extractColumn(const DataLine &data_line, int ptr) const
{
  string c = data_line[ptr];
  if (doStripQuotes) {
    c = remove_quotes(c);
  }

  // if you see a '\r' at the end remove that 
  std::size_t i1 = c.find_last_of("\r");
  if (i1 == string::npos) {
    return c;
  }
  return c.substr(0, i1-1);
}

////////////////////////////////////////////////////////////////////////

int AirnowHandler::_getVarIndex(const string &var_name, const string &units)
{
   int var_index = bad_data_int;

   // variable name already exists
   if (obs_names.has(var_name, var_index)) {

      // print warning if the units change
      if (units != obs_units[var_index]) {
         mlog << Warning << "\nAirnowHandler::_getVarIndex() -> "
              << "the units for observation variable \"" << var_name
              << "\" changed from \"" << obs_units[var_index]
              << "\" to \"" << units << "\"!\n\n";
      }
   }
   // add new variable name and units
   else {
      obs_names.add(var_name);
      obs_units.add(units);
      var_index = obs_names.n() - 1;
   }

   return var_index;
}

////////////////////////////////////////////////////////////////////////
//
// Begin utility functions
//
////////////////////////////////////////////////////////////////////////

string remove_quotes(string &s)
{
  std::size_t i0, i1;
  i0 = s.find_first_of("\"");
  i1 = s.find_last_of("\"");
  if (i0 == string::npos || i1 == string::npos) {
    return s;
  }
  if (i1-i0-1 > 0) {
    return s.substr(i0+1, i1-i0-1);
  } else {
    // it must be the empty string, so return that
    return "";
  }
}

////////////////////////////////////////////////////////////////////////

bool doubleOrMissing(const string &s, double &value)
{
  if (s.empty()) {
    value = bad_data_double;
    return false;
  } else {
    value = atof(s.c_str());
    return true;
  }
}

////////////////////////////////////////////////////////////////////////

vector<string> parseHourlyAqobsLine(const string &asciiLine, bool &ok)
{
  string fullLine = asciiLine;
  vector<string> tokens;

  // break this line into all the things between double quotes, ignoring the commas
  string remainder = fullLine;
  std::size_t i0, i1;
  ok = true;
  for (;;) {
    i0 = remainder.find_first_of("\"");
    if (i0 == string::npos) {
      break;
    }
    remainder = remainder.substr(i0+1);
    i1 = remainder.find_first_of("\"");
    if (i1 == string::npos) {
      mlog << Warning << "\nparseHourlyAqobsLine -> "
           << "line doesn't have matching double quotes\n"
           << fullLine << "\n\n";
      // skip this line
      ok = false;
      break;
    } else {
      string token = remainder.substr(0, i1);
      tokens.push_back(token);
      remainder = remainder.substr(i1+1);
    }        
  }
  return tokens;
}

////////////////////////////////////////////////////////////////////////
