// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

// Commenting out functionality for Angstrom - We are pulling back on
// this until we have a use case.

////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <string>
#include <sstream>
#include <assert.h> 

#include "vx_log.h"
#include "vx_math.h"
#include "vx_util.h"

#include "aeronet_handler.h"

static const char *AERONET_NA_STR = "N/A";

const int AeronetHandler::NUM_HDR_COLS = 7;
const int AeronetHandler::NUM_OBS_COLS = 45;
//const int NUM_OBS_COLS_V3 = 113;
//const int NUM_OBS_COLS_V3_tot  = 57;
//const int NUM_OBS_COLS_V3_oneill = 267;

const int version_3_columns[7] = { 113, 81, 53, 33, 41, 259 };

const string site_name_col = "AERONET_Site_Name";
const string lat_col1      = "Site_Latitude";       // "Site_Latitude(Degrees)";
const string lon_col1      = "Site_Longitude";      // "Site_Longitude(Degrees)";
const string elv_col1      = "Site_Elevation";      // "Site_Elevation(m)";
const string lat_col2      = "Latitude";            // "Latitude(degrees)"
const string lon_col2      = "Longitude";           // "Longitude(degrees)"
const string elv_col2      = "Elevation";           // "Elevation(meters)"

const string AeronetHandler::HEADER_TYPE = "";  /////

const int AeronetHandler::AOT_GRIB_CODE = 129;
// Version 2
const string AOT_NAME = "AOT";
// Version 3
const string AOD_NAME = "AOD";
const string INPUT_AOD_NAME = "Input_AOD";  // 870nm_Input_AOD
const string NO2_NAME       = "NO2";        // NO2(Dobson)
const string OPTICAL_AIR_MASS_NAME   = "Optical_Air_Mass";   // Optical_Air_Mass
const string OZONE_NAME              = "Ozone";              // Ozone(Dobson)
const string PRECIP_WATER_NAME       = "Precipitable_Water";
const string PRESSURE_NAME           = "Pressure";           //Pressure(hPa)
const string SENSOR_TEMP_NAME        = "Sensor_Temperature"; // Sensor_Temperature(Degrees_C)
const string SOLOR_ZENITH_ANGLE_NAME = "Solar_Zenith_Angle"; // Solar_Zenith_Angle(Degrees)
const string WAVELENGTHS_AOD_NAME = "Exact_Wavelengths_of_AOD"; // Exact_Wavelengths_of_AOD(um)_865nm
const string WAVELENGTHS_PW_NAME  = "Exact_Wavelengths_of_PW";  // Exact_Wavelengths_of_PW(um)_935nm
const string WAVELENGTHS_INPUT_AOD_NAME = "Exact_Wavelengths_for_Input_AOD";    // Exact_Wavelengths_for_Input_AOD(um)

static int format_version;

const float AERONET_MISSING_VALUE = -999.;

double angstrom_power_interplation(double value_1, double value_2, double level_1, double level_2, double target_level);

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AeronetHandler
   //


////////////////////////////////////////////////////////////////////////


AeronetHandler::AeronetHandler(const string &program_name) :
  FileHandler(program_name)
{
}


////////////////////////////////////////////////////////////////////////


AeronetHandler::~AeronetHandler()
{
}


////////////////////////////////////////////////////////////////////////



bool AeronetHandler::isFileType(LineDataFile &ascii_file) const
{
  //
  // Initialize the return value.
  //
  bool is_file_type = false;

  //
  // Read the first, second, and third lines from the file.  We will
  // skip these line since we don't know how many tokens they will have.
  //
  DataLine dl;
  dl.set_delimiter(",");
  ascii_file >> dl;

  // Check if version 3
  string line = dl.get_line();
  if (line.length() > 17) {
    line = line.substr(0, 17);
    if (strcmp(line.c_str(), "AERONET Version 3") == 0) {
      is_file_type = true;
      format_version = 3;
      return is_file_type;
    }
  }

  ascii_file >> dl;
  ascii_file >> dl;

  //
  // Read the fourth line in the file.  It should start with AOD Level.
  //
  ascii_file >> dl;
  line = dl.get_line();
  line = line.substr(0, 9);

  if (strcmp(line.c_str(), "AOD Level") == 0)
    is_file_type = true;

  return is_file_type;
}

void AeronetHandler::setFormatVersion(int version) {

  format_version = version;
}

////////////////////////////////////////////////////////////////////////
// Private/Protected methods
////////////////////////////////////////////////////////////////////////

bool AeronetHandler::_readObservations(LineDataFile &ascii_file)
{
  DataLine data_line;
  string method_name = "AeronetHandler::_readObservations() ";

  //
  // Read and save the station name, latitude, longitude, and elevation.
  //

  if (!_readHeaderInfo(ascii_file))
    return false;

  //
  // Get the Level number
  //

  ascii_file >> data_line;
  string header_type = "AERONET_AOD";
  
  //
  // Get the field information from the fifth header line
  //

  if (format_version == 3) {
    use_var_id = true;
    // Get the field information from the 7-th header line
    ascii_file >> data_line;
    ascii_file >> data_line;
    ascii_file >> data_line;
  }
  ascii_file >> data_line;
  StringArray hdr_tokens, extra_hdr_tokens;
  IntArray process_flag;

  hdr_tokens.parse_css(data_line.get_line());
  if (data_line.n_items() > 1) {
    for (int idx=1; idx<data_line.n_items(); idx++) {
      extra_hdr_tokens.parse_css(data_line[idx]);
      hdr_tokens.add(extra_hdr_tokens);
    }
  }

  int flag;
  int aod_var_id = bad_data_int;
  int var_idx, sid_idx, elv_idx, lat_idx, lon_idx;
  double height_from_header;
  string aot = "AOT";
  //string angstrom = "Angstrom";
  string var_name;
  StringArray hdr_names;
  NumArray header_heights;
  IntArray header_var_index;
  StringArray header_var_names;
  bool has_aod_column_at_550 = false;
  
  sid_idx = elv_idx = lat_idx = lon_idx = -1;

  for (int j = 0; j < hdr_tokens.n_elements(); j++)
  {
    string hdr_field = hdr_tokens[j];

    // Set the process_flag to 1 if AOT (or Angstrom - future code) is in the field name
    // otherwise set the process flag to 0
    flag = 0;

    if (hdr_field.find(aot) != string::npos)
    {
      flag = 1;
    }
    //else if (hdr_field.find(angstrom) != string::npos)
    //{
    //  process_flag.add(1);
    //}
    // version 3
    else if ((0 == hdr_field.find(AOD_NAME))
        || (string::npos != hdr_field.find(INPUT_AOD_NAME))
        || (string::npos != hdr_field.find(WAVELENGTHS_AOD_NAME))
        || (string::npos != hdr_field.find(WAVELENGTHS_PW_NAME))
        || (0 == hdr_field.find(NO2_NAME))
        || (0 == hdr_field.find(OPTICAL_AIR_MASS_NAME))
        || (0 == hdr_field.find(OZONE_NAME))
        || (0 == hdr_field.find(PRECIP_WATER_NAME))
        || (0 == hdr_field.find(PRESSURE_NAME))
        || (0 == hdr_field.find(SENSOR_TEMP_NAME))
        || (0 == hdr_field.find(SOLOR_ZENITH_ANGLE_NAME))
        )
    {
      if (hdr_field.find("Empty") == string::npos) flag = 1;
    }
    
    process_flag.add(flag);
    
    if (format_version == 3) {
      if (0 == hdr_field.find(site_name_col)) sid_idx = j;
      else if (0 == hdr_field.find(lat_col1) || 0 == hdr_field.find(lat_col2))  lat_idx = j;
      else if (0 == hdr_field.find(lon_col1) || 0 == hdr_field.find(lon_col2))  lon_idx = j;
      else if (0 == hdr_field.find(elv_col1) || 0 == hdr_field.find(elv_col2))  elv_idx = j;
      
      // Collect variable names and index
      var_name = make_var_name_from_header(hdr_field);
      if (!var_names.has(var_name.c_str(), var_idx)) {
        if (flag) {
          var_idx = var_names.n_elements();
          var_names.add(var_name.c_str());
          if (strcmp(var_name.c_str(), AOD_NAME.c_str()) == 0) aod_var_id = var_idx;
        }
      }
      height_from_header = extract_height(hdr_field);
      header_var_index.add(var_idx);
      header_var_names.add(var_name.c_str());
      header_heights.add(height_from_header);
      if (0 == strcmp(var_name.c_str(), AOD_NAME.c_str())
          && is_eq(height_from_header, 550)) has_aod_column_at_550 = true;
      mlog << Debug(5) << method_name << "header_idx: " << j
           << ", var_idx: " << var_idx << ", var: " << var_name << " from " << hdr_field
           << ", flag: " << flag << ", height: " << height_from_header << "\n";
    }
  }
  
  int column_cnt = NUM_OBS_COLS;
  if (format_version == 3) {
    column_cnt = get_header_count_v3(hdr_tokens);
  }
  
  //
  // Process the observation lines
  //
  int bad_line_count = 0;
  bool first_line = true;
  data_line.set_delimiter(",");
  while (ascii_file >> data_line)
  {
    //
    // Make sure that the line contains the correct number of tokens
    //

    if (data_line.n_items() != column_cnt)
    {
      bad_line_count++;
      if (format_version != 3) {
        mlog << Error << "\nAeronetHandler" << method_name << "-> "
             << "line number " << data_line.line_number()
             << " does not have the correct number of columns " << data_line.n_items()
             << " (" << column_cnt << "). Stop processing \""
             << ascii_file.filename() << "\".\n\n";
        return false;
      }
      else if (data_line.n_items() < column_cnt) {
        mlog << Error << "\nAeronetHandler::_readObservations() -> "
             << "line number " << data_line.line_number()
             << " does not have the correct number of columns " << data_line.n_items()
             << " (" << column_cnt << "). Stop processing \""
             << ascii_file.filename() << "\".\n\n";
        break;
      }
      else {
        if (bad_line_count < 5) {
          mlog << Warning << "\nAeronetHandler::_readObservations() -> "
               << "line number " << data_line.line_number()
               << " has more number of columns " << data_line.n_items()
               << " (" << column_cnt << ").\n\n";
        }
      }
    }

    if (first_line) {
      if (format_version == 3) {
        // Get the stationId
        if (elv_idx < 0) {
          mlog << Warning << "AeronetHandler::_readObservations() Can not find header column \""
               << elv_col2 << "\". from " << ascii_file.filename() << "\".\n\n";
          break;
        }
        else if ((lat_idx < 0) || (lon_idx < 0)) {
          string field_name = (lat_idx < 0) ? lat_col2 : lon_col2;
          mlog << Error << "AeronetHandler::_readObservations() Can not find header column \""
               << field_name << "\". Skip the input \"" << ascii_file.filename()
               << "\"\n\n";
          break;
        }
        else {
          if (sid_idx < 0) {
            mlog << Warning << "AeronetHandler::_readObservations() Can not find header column \""
                 << site_name_col << "\" from the input \"" << ascii_file.filename()
                 << "\"\n\n";
          }
          else if (_stationId != data_line[sid_idx]) {
            mlog << Error << "\nAeronetHandler::_readObservations() The header and data columns don't match."
                 << " The station ID from data column (" << data_line[sid_idx] << ") at " << sid_idx
                 << " is different from " << _stationId
                 << ". Skip this input \"" << ascii_file.filename()
                 << "\"\n\n";
            break;
          }
        }
        
        // Get the stationLat
        _stationLat = atof(data_line[lat_idx]);
        // Get the stationLon
        _stationLon = atof(data_line[lon_idx]);
        // Get the stationAlt
        if (elv_idx >= 0) _stationAlt = atof(data_line[elv_idx]);
        else _stationAlt = bad_data_float;
        
        mlog << Debug(5) << "AeronetHandler::_readObservations() stationID: "
             << ((sid_idx < 0) ? _stationId : data_line[sid_idx]) << " from index " << sid_idx
             << "  lat: " << _stationLat
             << "  lon: " << _stationLon
             << "  elv: " << _stationAlt << " from index " << elv_idx << "\n";
      }
      first_line = false;
    }
    //
    // Pull the valid time from the data line
    //

    time_t valid_time = _getValidTime(data_line);

    if (valid_time == 0)
      return false;

    bool has_aod_at_550;
    double aod_at_440, aod_at_675;
    int var_id = AOT_GRIB_CODE;
    
    has_aod_at_550 = false;
    aod_at_440 = aod_at_675 = bad_data_float;
    
    var_name = AOT_NAME;
    //
    // Save the desired observations from the line
    //
    for (int k = 0; k < process_flag.n_elements(); k++)
    {
      if (process_flag[k] != 1) continue;

      string hdr_field = hdr_tokens[k];
      size_t found_aot = hdr_field.find(aot);
      //int found_angstrom = hdr_field.find(angstrom);
      string height = "";

      if (found_aot != string::npos)
      {
        height = hdr_field.substr((found_aot + 4), hdr_field.size() - 1);
      }

      //if (found_angstrom != string::npos)
      //{
      //  size_t found_dash = hdr_field.find("-");
      //  if (found_dash != string::npos)
      //  {
      //    height = hdr_field.substr(0, found_dash);
      //  }
      //}

      if(strcmp(data_line[k], AERONET_NA_STR) == 0) continue;

      var_name = AOT_NAME;
      
      double dlevel = bad_data_double;
      double dheight = atoi(height.c_str());

      if (format_version == 3) {
        var_id   = header_var_index[k];
        var_name = header_var_names[k];
        dheight  = header_heights[k];
        //if (is_eq(atof(data_line[k]), AERONET_MISSING_VALUE)) continue;
        if (strcmp(var_name.c_str(), AOD_NAME.c_str()) == 0) {
          if (is_eq(dheight, 550)) has_aod_at_550 = true;
          else if (is_eq(dheight, 440)) aod_at_440 = atof(data_line[k]);
          else if (is_eq(dheight, 675)) aod_at_675 = atof(data_line[k]);
        }
      }
      
      _addObservations(Observation(header_type, _stationId,
                                   valid_time,
                                   _stationLat, _stationLon,
                                   _stationAlt,
                                   na_str,
                                   var_id,
                                   dlevel, dheight,
                                   atof(data_line[k]),
                                   var_name));
    }
    if (format_version == 3) {
      if (!has_aod_at_550 && !is_eq(aod_at_440, bad_data_float) && !is_eq(aod_at_675, bad_data_float)) {
        var_id   = aod_var_id;
        var_name = AOD_NAME;
        double dheight  = 550;
        double aod_at_550 = angstrom_power_interplation(aod_at_675,aod_at_440,675.,440.,dheight);
        _addObservations(Observation(header_type, _stationId, valid_time,
                                     _stationLat, _stationLon, _stationAlt,
                                     na_str, var_id, bad_data_double, dheight,
                                     aod_at_550,
                                     var_name));
        mlog << Debug(7) << "AeronetHandler::_readObservations() AOD at 550: "
             << aod_at_550 << "\t440: " << aod_at_440
             << "\t675: " << aod_at_675 << "\n";
      }
    }
  } // end while
  if (bad_line_count > 0) {
    mlog << Warning << "\nAeronetHandler::_readObservations() -> "
         << "Found " << bad_line_count 
         << " lines with more data columns from " 
         << ascii_file.filename() << "\".\n\n";
  }
  
  if (format_version == 3) {
    double aod_at_675, aod_at_440;
    double aod_at_550_expected, angstrom_675_440_expected;
    double angstrom_675_440, aod_at_550;
    
    aod_at_675 = 0.645283;
    aod_at_440 = 0.794593;
    aod_at_550_expected = 0.71286864;
    //angstrom_675_440_expected = 0.486381371;
    aod_at_550 = angstrom_power_interplation(aod_at_675,aod_at_440,675.,440.,550);
    if (! is_eq(aod_at_550, aod_at_550_expected))
      mlog << Warning << "AeronetHandler::_readObservations() Check AOD at 550: "
           << aod_at_550 << " (" << aod_at_550_expected << ")"
           << "\t440: " << aod_at_440
           << "\t675: " << aod_at_675 
           << "\n";
    else
      mlog << Debug(3) << "AeronetHandler::_readObservations() Confirmed AOD interpolation at 550: "
           << aod_at_550 << " (" << aod_at_550_expected << ")"
           << "\t440: " << aod_at_440
           << "\t675: " << aod_at_675 
           << "\n";

    aod_at_675 = 0.669274;
    aod_at_440 = 0.83858;
    aod_at_550_expected = 0.745546058;
    //angstrom_675_440_expected = 0.526983959;
    aod_at_550 = angstrom_power_interplation(aod_at_675,aod_at_440,675.,440.,550);
    if (! is_eq(aod_at_550, aod_at_550_expected))
      mlog << Warning << "AeronetHandler::_readObservations() Check AOD at 550: "
           << aod_at_550 << " (" << aod_at_550_expected << ")"
           << "\t440: " << aod_at_440
           << "\t675: " << aod_at_675 
           << "\n";
    else
      mlog << Debug(3) << "AeronetHandler::_readObservations() Confirmed AOD interpolation at 550: "
           << aod_at_550 << " (" << aod_at_550_expected << ")"
           << "\t440: " << aod_at_440
           << "\t675: " << aod_at_675 
           << "\n";
  }
  return true;
}

////////////////////////////////////////////////////////////////////////

time_t AeronetHandler::_getValidTime(const DataLine &data_line) const
{
  //
  // Pull out the date information
  //

  ConcatString date_string(data_line[0]);
  StringArray dateTokens = date_string.split(":");
  if (1 == dateTokens.n_elements()) {
    mlog << Error << "\nAeronetHandler::_getValidTime -> "
         << "Not supported date: \"" << date_string << "\".\n\n";
    return 0;
  }
  
  string mday = dateTokens[0];
  string mon  = dateTokens[1];
  string year = dateTokens[2];

  //
  // Pull out the time information
  //

  ConcatString time_string(data_line[1]);
  StringArray timeTokens = time_string.split(":");

  //
  // Set up the time structure
  //

  struct tm time_struct;
  memset(&time_struct, 0, sizeof(time_struct));

  time_struct.tm_year = atoi(year.c_str()) - 1900;
  time_struct.tm_mon = atoi(mon.c_str()) - 1;
  time_struct.tm_mday = atoi(mday.c_str());
  if (3 <= timeTokens.n_elements()) {
    string hour = timeTokens[0];
    string min = timeTokens[1];
    string sec = timeTokens[2];
    
    time_struct.tm_hour = atoi(hour.c_str());
    time_struct.tm_min = atoi(min.c_str());
    time_struct.tm_sec = atoi(sec.c_str());
  }

  return timegm(&time_struct);

}

////////////////////////////////////////////////////////////////////////

bool AeronetHandler::_readHeaderInfo(LineDataFile &ascii_file)
{
  DataLine data_line;
  data_line.set_delimiter(",");

  //
  // Skip the first two lines
  //
  ascii_file >> data_line;
  ascii_file >> data_line;

  //
  // The third line of the file contains the station name, latitude,
  // longitude, and elevation.
  //

  if (format_version == 3) {
    _stationId = data_line[0];
    if (' ' == _stationId[0]) _stationId = _stationId.substr(1);
    mlog << Debug(5) << " _stationId: [" <<  _stationId << "]\n";
    // read lat/lon from https://aeronet.gsfc.nasa.gov/aeronet_locations_v3.txt
    return true;
  }

  if (!(ascii_file >> data_line))
  {
    mlog << Error << "\nAeronetHandler::_readHeaderInfo() -> "
         << "error reading station id line from input ASCII file \""
         << ascii_file.filename() << "\"\n\n";

    return false;
  }

  //
  // Check for the correct number of columns in the header line
  //

  if (data_line.n_items() != NUM_HDR_COLS)
  {
    mlog << Error << "\nAeronetHandler::_readHeaderInfo() -> "
         << "AERONET file has incorrect number of columns ("
         << data_line.n_items() << ") in header line\n\n";
    return false;
  }

  //
  // Get the stationId
  //

  ConcatString stationId_string(data_line[0]);
  StringArray stationIdTokens = stationId_string.split("=");
  _stationId = stationIdTokens[stationIdTokens.n_elements()-1];

  //
  // Get the stationLon
  //

  ConcatString stationLon_string(data_line[1]);
  StringArray stationLonTokens = stationLon_string.split("=");
  _stationLon = atof(stationLonTokens[stationLonTokens.n_elements()-1].c_str());

  //
  // Get the stationLat
  //

  ConcatString stationLat_string(data_line[2]);
  StringArray stationLatTokens = stationLat_string.split("=");
  _stationLat = atof(stationLatTokens[stationLatTokens.n_elements()-1].c_str());

  //
  // Get the stationAlt
  //

  ConcatString stationAlt_string(data_line[3]);
  StringArray stationAltTokens = stationAlt_string.split("=");
  _stationAlt = atof(stationAltTokens[stationAltTokens.n_elements()-1].c_str());

  return true;

}

////////////////////////////////////////////////////////////////////////

double AeronetHandler::extract_height(string hdr_field) {
  double height;
  string height_str = "";

  if (string::npos == hdr_field.find("Empty")) {
    size_t offset;
    int token_count = 0;
    bool with_unit = false;
    string tmp_height;
    StringArray hdr_names;
    
    hdr_names.parse_delim(hdr_field.c_str(), "_");
    // AOD_1640nm-Total,AOD_1640nm-AOD,AOD_1640nm-Rayleigh,AOD_1640nm-O3,
    //   AOD_1640nm-NO2,AOD_1640nm-CO2,AOD_1640nm-CH4,AOD_1640nm-WaterVapor
    if (0 == (offset = hdr_field.find(AOD_NAME))) {
      with_unit = true;
      tmp_height = hdr_names[hdr_names.n_elements()-1];
      hdr_names.clear();
      hdr_names.parse_delim(tmp_height.c_str(), "-");
      height_str = hdr_names[0];
    }
    // Exact_Wavelengths_for_Input_AOD(um)
    else if (0 == (offset = hdr_field.find(WAVELENGTHS_INPUT_AOD_NAME))) {
      StringArray tmp_hdr_names;
      tmp_hdr_names.parse_delim(WAVELENGTHS_INPUT_AOD_NAME.c_str(), "_");
      token_count = tmp_hdr_names.n_elements();
    }
    // 870nm_Input_AOD
    else if (string::npos != (offset = hdr_field.find(INPUT_AOD_NAME))) {
      with_unit = true;
      height_str = hdr_names[0];
    }
    // Exact_Wavelengths_of_AOD(um)_865nm
    // Exact_Wavelengths_of_PW(um)_935nm
    else if (string::npos != (offset = hdr_field.find(WAVELENGTHS_AOD_NAME))
        || string::npos != (offset = hdr_field.find(WAVELENGTHS_PW_NAME)) ) {
      with_unit = true;
      StringArray tmp_hdr_names;
      tmp_hdr_names.parse_delim(WAVELENGTHS_AOD_NAME.c_str(), "_");
      token_count = tmp_hdr_names.n_elements();
    }
    // Exact_Wavelengths_of_INPUT_AOD(um)
    else if (string::npos != (offset = hdr_field.find(WAVELENGTHS_INPUT_AOD_NAME)) ) {
      StringArray tmp_hdr_names;
      tmp_hdr_names.parse_delim(WAVELENGTHS_INPUT_AOD_NAME.c_str(), "_");
      token_count = tmp_hdr_names.n_elements();
    }
    
    if (0 < token_count && token_count < hdr_names.n_elements())
      height_str = hdr_names[hdr_names.n_elements()-1];
    
    if (with_unit && height_str.length() > 2)
      height_str = height_str.substr(0, (height_str.length()-2));
  }
  mlog << Debug(10) << "AeronetHandler::extract_height() height: "
       << height_str << " from " << hdr_field << "\n";

  if (height_str == "") height = bad_data_double;
  else {
    height = (double)atoi(height_str.c_str());
    if (is_eq(height, 0,0)) {
      mlog << Warning << "AeronetHandler::extract_height() converted to 0 from (" << height_str << ")\n";
    }
  }
 
  return height;
}

////////////////////////////////////////////////////////////////////////

int AeronetHandler::get_header_count_v3(StringArray hdr_tokens) {
  int header_cnt = hdr_tokens.n_elements();
  
  mlog << Debug(5) << "get_header_count_v3() " << header_cnt << "\n";
  return header_cnt;
}

////////////////////////////////////////////////////////////////////////

string AeronetHandler::make_var_name_from_header(string hdr_field) {
  string var_name = hdr_field;
  if (format_version == 3) {
    int offset;
    bool found = true;
    StringArray hdr_names;
    // AOD_1640nm-Total,AOD_1640nm-AOD,AOD_1640nm-Rayleigh,AOD_1640nm-O3,
    //   AOD_1640nm-NO2,AOD_1640nm-CO2,AOD_1640nm-CH4,AOD_1640nm-WaterVapor
    if (0 == (offset = hdr_field.find(AOD_NAME))) {
      hdr_names.parse_delim(hdr_field.c_str(), "_");
      var_name = hdr_names[0];
    }
    // Exact_Wavelengths_for_Input_AOD(um)
    else if (0 == (offset = hdr_field.find(WAVELENGTHS_INPUT_AOD_NAME))) {
      var_name = WAVELENGTHS_INPUT_AOD_NAME;
    }
    // const string Input_AOD_NAME = "Input_AOD";  // 870nm_Input_AOD
    else if ((int) string::npos != (offset = hdr_field.find(INPUT_AOD_NAME))) {
      var_name = INPUT_AOD_NAME;
    }
    // Exact_Wavelengths_of_AOD(um)_865nm
    else if ((int) string::npos != (offset = hdr_field.find(WAVELENGTHS_AOD_NAME))) {
      var_name = WAVELENGTHS_AOD_NAME;
    }
    // Exact_Wavelengths_of_PW(um)_935nm
    else if ((int) string::npos != (offset = hdr_field.find(WAVELENGTHS_PW_NAME))) {
      var_name = WAVELENGTHS_PW_NAME;
    }
    else if ((hdr_field == OPTICAL_AIR_MASS_NAME)) {
      var_name = hdr_field;
    }
    else {
      found = false;
    }
    if (found) {
      hdr_names.clear();
      hdr_names.parse_delim(hdr_field.c_str(), "-");
      if (hdr_names.n_elements() > 1) {
        var_name += "-";
        var_name += hdr_names[hdr_names.n_elements()-1];
      }
    }
    else {
      hdr_names.clear();
      hdr_names.parse_delim(hdr_field.c_str(), "(");
      if (hdr_names.n_elements() > 1) {
        var_name = hdr_names[0];
      }
    }
  }
  return var_name;
}

double angstrom_power_interplation(double value_1, double value_2,
    double level_1, double level_2, double target_level) {
  double angstrom_log = -log10(value_1/value_2)/log10(level_1/level_2);
  double angstrom_value = value_2 * pow((target_level/level_2),-angstrom_log);
  return angstrom_value;
}
