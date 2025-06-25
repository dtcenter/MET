// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_log.h"
#include "vx_math.h"
#include "vx_util.h"

#include "uscrn_handler.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

//
// U.S. Climate Reference Network (USCRN) Quality Controlled Datasets
//    URL: https://www.ncei.noaa.gov/access/crn/qcdatasets.html
//   Data: ftp://ftp.ncei.noaa.gov/pub/data/uscrn/products/{type}
//

//
// "monthly01" with files named "CRNM0102-{Location}.txt
//   - Format number "02"
//   - Contains 15 columns defined by:
//     https://www.ncei.noaa.gov/pub/data/uscrn/products/monthly01/readme.txt
//
static const USCRNFormatInfo uscrn_monthly_info = {
  "USCRN-Monthly", "CRNM0102", ".txt", 15, 0, 1, -1, 3, 4,
  {
    {  5, "T_MONTHLY_MAX", "Celsius",
          "Maximum air temperature", -1 },
    {  6, "T_MONTHLY_MIN", "Celsius",
          "Minimum air temperature", -1 },
    {  7, "T_MONTHLY_MEAN", "Celsius",
          "Mean air temperature, calculated as (T_MONTHLY_MAX + T_MONTHLY_MIN) / 2", -1 },
    {  8, "T_MONTHLY_AVG", "Celsius",
          "Average air temperature", -1 },
    {  9, "P_MONTHLY_CALC", "mm",
          "Total amount of precipitation", -1 },
    { 10, "SOLRAD_MONTHLY_AVG", "MJ/m^2",
          "Average daily total solar energy received", -1 },
    { 12, "SUR_TEMP_MONTHLY_MAX", "Celcius",
          "Maximum infrared surface temperature", 11 },
    { 13, "SUR_TEMP_MONTHLY_MIN", "Celcius",
          "Minimum infrared surface temperature", 11 },
    { 14, "SUR_TEMP_MONTHLY_AVG", "Celcius",
          "Average infrared surface temperature", 11 }
  }
};

//
// "daily01" with files named "{YYYY}/CRND0103-{YYYY}-{Location}.txt
//   - Format number "03"
//   - Contains 28 columns defined by:
//     https://www.ncei.noaa.gov/pub/data/uscrn/products/daily01/readme.txt
//
static const USCRNFormatInfo uscrn_daily_info = {
  "USCRN-Daily", "CRND0103", ".txt", 28, 0, 1, -1, 3, 4,
  {
    {  5, "T_DAILY_MAX", "Celsius",
          "Maximum air temperature", -1 },
    {  6, "T_DAILY_MIN", "Celsius",
          "Minimum air temperature", -1 },
    {  7, "T_DAILY_MEAN", "Celsius",
          "Mean air temperature, calculated as (T_MONTHLY_MAX + T_MONTHLY_MIN) / 2", -1 },
    {  8, "T_DAILY_AVG", "Celsius",
          "Average air temperature", -1 },
    {  9, "P_DAILY_CALC", "mm",
          "Total amount of precipitation", -1 },
    { 10, "SOLARAD_DAILY", "MJ/m^2",
          "Total solar energy, calculated from the hourly average rates", -1 },
    { 12, "SUR_TEMP_DAILY_MAX", "Celsius",
          "Maximum infrared surface temperature", -1 },
    { 13, "SUR_TEMP_DAILY_MIN", "Celsius",
          "Minimum infrared surface temperature", -1 },
    { 14, "SUR_TEMP_DAILY_AVG", "Celsius",
          "Average infrared surface temperature", -1 },
    { 15, "RH_DAILY_MAX", "%",
          "Maximum relative humidity", -1 },
    { 16, "RH_DAILY_MIN", "%",
          "Minimum relative humidity", -1 },
    { 17, "RH_DAILY_AVG", "%",
          "Average relative humidity", -1 },
    { 18, "SOIL_MOISTURE_5_DAILY", "m^3/m^3",
          "Average soil moisture at 5 cm below the surface", -1 },
    { 19, "SOIL_MOISTURE_10_DAILY", "m^3/m^3",
          "Average soil moisture at 10 cm below the surface", -1 },
    { 20, "SOIL_MOISTURE_20_DAILY", "m^3/m^3",
          "Average soil moisture at 20 cm below the surface", -1 },
    { 21, "SOIL_MOISTURE_50_DAILY", "m^3/m^3",
          "Average soil moisture at 50 cm below the surface", -1 },
    { 22, "SOIL_MOISTURE_100_DAILY", "m^3/m^3",
          "Average soil moisture at 100 cm below the surface", -1 },
    { 23, "SOIL_TEMP_5_DAILY", "Celsius",
          "Average soil temperature at 5 cm below the surface", -1 },
    { 24, "SOIL_TEMP_10_DAILY", "Celsius",
          "Average soil temperature at 10 cm below the surface", -1 },
    { 25, "SOIL_TEMP_20_DAILY", "Celsius",
          "Average soil temperature at 20 cm below the surface", -1 },
    { 26, "SOIL_TEMP_50_DAILY", "Celsius",
          "Average soil temperature at 50 cm below the surface", -1 },
    { 27, "SOIL_TEMP_100_DAILY", "Celsius",
          "Average soil temperature at 100 cm below the surface", -1 }
  }
};

//
// "hourly02" with files named "{YYYY}/CRNH0203-{YYYY}-{Location}.txt
//   - Format number "03"
//   - Contains 38 columns defined by:
//     https://www.ncei.noaa.gov/pub/data/uscrn/products/hourly02/readme.txt
//
static const USCRNFormatInfo uscrn_hourly_info = {
  "USCRN-Hourly", "CRNH0203", ".txt", 38, 0, 1, 2, 6, 7,
  {
    {  8, "T_CALC", "Celsius",
          "Average air temperature during the last 5 minutes of the hour", -1 },
    {  9, "T_HR_AVG", "Celsius",
          "Average air temperature for the entire hour", -1 },
    { 10, "T_MAX", "Celsius",
          "Maximum air temperature during the hour", -1 },
    { 11, "T_MIN", "Celsius",
          "Minimum air temperature during the hour", -1 },
    { 12, "P_CALC", "mm",
          "Total amount of precipitation during the hour", -1 },
    { 13, "SOLARAD", "W/m^2",
          "Average global solar radiation", 14 },
    { 15, "SOLARAD_MAX", "W/m^2",
          "Maximum global solar radiation", 16 },
    { 17, "SOLARAD_MIN", "W/m^2",
          "Minimum global solar radiation", 18 },
    { 20, "SUR_TEMP", "Celsius",
          "Average infrared surface temperature", 21 },
    { 22, "SUR_TEMP_MAX", "Celsius",
          "Maximum infrared surface temperature", 23 },
    { 24, "SUR_TEMP_MIN", "Celsius",
          "Minimum infrared surface temperature", 25 },
    { 26, "RH_HR_AVG", "%",
          "Average relative humidity", 27 },
    { 28, "SOIL_MOISTURE_5", "m^3/m^3",
          "Average soil moisture at 5 cm below the surface", -1 },
    { 29, "SOIL_MOISTURE_10", "m^3/m^3",
          "Average soil moisture at 10 cm below the surface", -1 },
    { 30, "SOIL_MOISTURE_20", "m^3/m^3",
          "Average soil moisture at 20 cm below the surface", -1 },
    { 31, "SOIL_MOISTURE_50", "m^3/m^3",
          "Average soil moisture at 50 cm below the surface", -1 },
    { 32, "SOIL_MOISTURE_100", "m^3/m^3",
          "Average soil moisture at 100 cm below the surface", -1 },
    { 33, "SOIL_TEMP_5", "Celsius",
          "Average soil temperature at 5 cm below the surface", -1 },
    { 34, "SOIL_TEMP_10", "Celsius",
          "Average soil temperature at 10 cm below the surface", -1 },
    { 35, "SOIL_TEMP_20", "Celsius",
          "Average soil temperature at 20 cm below the surface", -1 },
    { 36, "SOIL_TEMP_50", "Celsius",
          "Average soil temperature at 50 cm below the surface", -1 },
    { 37, "SOIL_TEMP_100", "Celsius",
          "Average soil temperature at 100 cm below the surface", -1 }
  }
};

//
// "subhourly01" with files named "{YYYY}/CRNS0101-{MM}-{YYYY}-{Location}.txt
//   - Format number "01"
//   - Contains 23 columns defined by:
//     https://www.ncei.noaa.gov/pub/data/uscrn/products/subhourly01/readme.txt
//
static const USCRNFormatInfo uscrn_subhourly_info = {
  "USCRN-SubHourly", "CRNS0101", ".txt", 23, 0, 1, 2, 6, 7,
  {
    {  8, "AIR_TEMPERATURE", "Celsius",
          "Average air temperature", -1 },
    {  9, "PRECIPITATION", "mm",
          "Total amount of precipitation", -1 },
    { 10, "SOLAR_RADIATION", "W/m^2",
          "Average global solar radiation received", 11 },
    { 12, "SURFACE_TEMPERATURE", "Celsius",
          "Average infrared surface temperature", 14 },
    { 15, "RELATIVE_HUMIDITY", "%",
          "Average relative humidity", 16 },
    { 17, "SOIL_MOISTURE_5", "m^3/m^3",
          "Average soil moisture at 5 cm below the surface", -1 },
    { 18, "SOIL_TEMPERATURE_5", "Celsius",
          "Average soil temperature at 5 cm below the surface", -1 },
    { 19, "WETNESS", "Ohms",
          "presence or absence of moisture due to precipitation", 20 },
    { 21, "WIND_1_5", "m/s",
          "Average wind speed at a height of 1.5 meters", 22 }
  }
};

//
// "soil/soilanom01" with files named "CRNSSM0101-{Location}.csv"
//   - Format number "01"
//   - Contains 30 NAMED columns but with no README file provided
//
// Note that "soil/soilclim01" files named "CRNSMC0101-{Location}.csv"
// are not supported directly here.
//
static const USCRNFormatInfo uscrn_soilanom_info = {
  "USCRN-SoilAnom", "CRNSSM0101", ".csv", 30, 0, 1, -1, 2, 3,
  {
    {  4, "SMVWC_5_CM", "m^3/m^3",
           "Hourly average of 5 cm soil moisture conditions", -1 },
    {  5, "SMANOM_5_CM", "Standardized",
          "Hourly average of 5 cm standardized soil moisture anomalies", -1 },
    {  7, "ST_5_CM", "Celsius",
          "Hourly average of 5 cm soil temperatures", -1 },
    {  8, "SMVWC_10_CM", "m^3/m^3",
          "Hourly average of 10 cm soil moisture conditions", -1 },
    {  9, "SMANOM_10_CM", "Standardized",
          "Hourly average of 10 cm standardized soil moisture anomalies", -1 },
    { 11, "ST_10_CM", "Celsius",
          "Hourly average of 10 cm soil temperatures", -1 },
    { 12, "SMVWC_20_CM", "m^3/m^3",
          "Hourly average of 20 cm soil moisture conditions", -1 },
    { 13, "SMANOM_20_CM", "Standardized",
          "Hourly average of 20 cm standardized soil moisture anomalies", -1 },
    { 15, "ST_20_CM", "Celsius",
          "Hourly average of 20 cm soil temperatures", -1 },
    { 16, "SMVWC_50_CM", "m^3/m^3",
          "Hourly average of 50 cm soil moisture conditions", -1 },
    { 17, "SMANOM_50_CM", "Standardized",
          "Hourly average of 50 cm standardized soil moisture anomalies", -1 },
    { 19, "ST_50_CM", "Celsius",
          "Hourly average of 50 cm soil temperatures", -1 },
    { 20, "SMVWC_100_CM", "m^3/m^3",
          "Hourly average of 100 cm soil moisture conditions", -1 },
    { 21, "SMANOM_100_CM", "Standardized",
          "Hourly average of 100 cm standardized soil moisture anomalies", -1 },
    { 23, "ST_100_CM", "Celsius",
          "Hourly average of 100 cm soil temperatures", -1 },
    { 24, "SMVWC_TOP", "m^3/m^3",
          "Hourly average of top (i.e. 5 & 10 cm) soil moisture conditions", -1 },
    { 25, "SMANOM_TOP", "Standardized",
          "Hourly average of top (i.e. 5 & 10 cm) soil moisture anomalies", -1 },
    { 27, "SMVWC_COLUMN", "m^3/m^3",
          "Hourly average of column (i.e. all avaliable depths) soil moisture conditions", -1 },
    { 28, "SMANOM_COLUMN", "Standardized",
          "Hourly average of column (i.e. all avaliable depths) soil moisture anomalies", -1 }
  }
};

//
// "heat01" with files named "SCRNHE0101-{Location}.csv"
//   - Format number "01"
//   - Contains 16 NAMED columns described by:
//     https://www.ncei.noaa.gov/pub/data/uscrn/products/heat01/readme.txt
//
static const USCRNFormatInfo uscrn_heat_info = {
  "USCRN-Heat", "CRNHE0101", ".csv", 16, 0, 1, -1, 2, 3,
  {
    {  4, "RELATIVE_HUMIDITY", "%",
          "Hourly average of relative humidity", -1 },
    {  5, "SURFACE_PRESSURE", "hPa",
          "Hourly average of surface pressure", -1 },
    {  6, "SOLAR_RADIATION", "W/m^2",
          "Hourly average of solar radiation", -1 },
    {  7, "ESTIMATED_10_METER_WIND_SPEED", "m/s",
          "Hourly average of 10 meter wind speed", -1 },
    {  8, "DRY_BULB_TEMPERATURE_C", "Celsius",
          "Hourly average of air temperature", -1 },
    {  9, "HEAT_INDEX_C", "Celsius",
          "Hourly average of heat index", -1 },
    { 10, "APPARENT_TEMPERATURE_C", "Celsius",
          "Hourly average of apparent temperature", -1 },
    { 11, "WET_BULB_GLOBE_TEMPERATURE_C", "Celsius",
          "Hourly average of wet bulb temperature", -1 },
    { 12, "DRY_BULB_TEMPERATURE_F", "Fahrenheit",
          "Hourly average of air temperature", -1 },
    { 13, "HEAT_INDEX_F", "Fahrenheit",
          "Hourly average of heat index", -1 },
    { 14, "APPARENT_TEMPERATURE_F", "Fahrenheit",
          "Hourly average of apparent temperature", -1 },
    { 15, "WET_BULB_GLOBE_TEMPERATURE_F", "Fahrenheit",
          "Hourly average of wet bulb temperature", -1 }
  }
};

//
// "drought01" with files named "CRNDI0101-{Location}.csv
//   - Format number "01"
//   - Contains 32 NAMED columns described by:
//     https://www.ncei.noaa.gov/pub/data/uscrn/products/drought01/readme.txt
//
static const USCRNFormatInfo uscrn_drought_info = {
  "USCRN-Drought", "CRNDI0101", ".csv", 32, 0, 1, -1, 2, 3,
  {
    {  4, "SMVWC_5_CM_MEAN", "m^3/m^3",
          "Weekly average of hourly 5 cm soil moisture conditions", -1 },
    {  5, "SMANOM_5_CM_MEAN", "Standardized",
          "Weekly average of hourly 5 cm standardized soil moisture anomalies", -1 },
    {  8, "SMVWC_10_CM_MEAN", "m^3/m^3",
          "Weekly average of hourly 10 cm soil moisture conditions", -1 },
    {  9, "SMANOM_10_CM_MEAN", "Standardized",
          "Weekly average of hourly 10 cm standardized soil moisture anomalies", -1 },
    { 12, "SMVWC_20_CM_MEAN", "m^3/m^3",
          "Weekly average of hourly 20 cm soil moisture conditions", -1 },
    { 13, "SMANOM_20_CM_MEAN", "Standardized",
          "Weekly average of hourly 20 cm standardized soil moisture anomalies", -1 },
    { 16, "SMVWC_50_CM_MEAN", "m^3/m^3",
          "Weekly average of hourly 50 cm soil moisture conditions", -1 },
    { 17, "SMANOM_50_CM_MEAN", "Standardized",
          "Weekly average of hourly 50 cm standardized soil moisture anomalies", -1 },
    { 20, "SMVWC_100_CM_MEAN", "m^3/m^3",
          "Weekly average of hourly 100 cm soil moisture conditions", -1 },
    { 21, "SMANOM_100_CM_MEAN", "Standardized",
          "Weekly average of hourly 100 cm standardized soil moisture anomalies", -1 },
    { 24, "SMVWC_TOP_MEAN", "m^3/m^3",
          "Weekly average of hourly top (i.e. 5 & 10 cm) soil moisture conditions", -1 },
    { 25, "SMANOM_TOP_MEAN", "Standardized",
          "Weekly average of hourly top (i.e. 5 & 10 cm) soil moisture anomalies", -1 },
    { 28, "SMVWC_COLUMN_MEAN", "m^3/m^3",
          "Weekly average of hourly column (i.e. all depths) soil moisture conditions", -1 },
    { 29, "SMANOM_COLUMN_MEAN", "Standardized",
          "Weekly average of hourly column (i.e. all depths) soil moisture anomalies", -1 }
  }
};

// Mapping of USCRN format variants to metadata
static const std::map<USCRNFormat,USCRNFormatInfo> USCRNFormatMap = {
  { USCRNFormat::Monthly,   uscrn_monthly_info   },
  { USCRNFormat::Daily,     uscrn_daily_info     },
  { USCRNFormat::Hourly,    uscrn_hourly_info    },
  { USCRNFormat::SubHourly, uscrn_subhourly_info },
  { USCRNFormat::SoilAnom,  uscrn_soilanom_info  },
  { USCRNFormat::Heat,      uscrn_heat_info      },
  { USCRNFormat::Drought,   uscrn_drought_info   }
};

// List of bad data input values found in USCRN data
static const NumArray USCRNBadDataInput({ -99.0, -9999.0});

////////////////////////////////////////////////////////////////////////
//
// Code for class UscrnHandler
//
////////////////////////////////////////////////////////////////////////

UscrnHandler::UscrnHandler(const string &program_name) :
  FileHandler(program_name) {
   use_var_id = true;
   _format = USCRNFormat::None;
}

////////////////////////////////////////////////////////////////////////

bool UscrnHandler::isFileType(LineDataFile &ascii_file) const {
   return _getFileFormat(ascii_file) != USCRNFormat::None;
}

////////////////////////////////////////////////////////////////////////

USCRNFormat UscrnHandler::_getFileFormat(const LineDataFile &ascii_file) const {
   USCRNFormat fmt = USCRNFormat::None;

   // USCRN files are identified by their prefix and suffix.

   // Loop over supported USCRN formats
   for(const auto &x : USCRNFormatMap) {

      // Check for expected prefix and suffix
      if(check_prefix_suffix(ascii_file.short_filename(),
                             x.second._filePrefix.c_str(),
                             x.second._fileSuffix.c_str())) {
         fmt = x.first;
         break;
      }
   }

   return fmt;
}

////////////////////////////////////////////////////////////////////////

bool UscrnHandler::_readObservations(LineDataFile &ascii_file) {

   // Determine the format for each input file so that the same
   // handle can process multiple USCRN formats in a single run
   _format = _getFileFormat(ascii_file);

   // Check for a match
   if(_format == USCRNFormat::None) {
      mlog << Error << "\nUscrnHandler::_readObservataions() -> "
           << "unknown USCRN format for file \""
           << ascii_file.filename() << "\"!\n\n";
      return false; 
   }

   // Check for .csv format:
   // - .csv files have one header line to be skipped
   // - .txt files have no header line to be skipped
   DataLine dl;
   ConcatString file_name(ascii_file.filename());
   if(file_name.endswith(".csv")) {

      // Set the delimiter
      dl.set_delimiter(",");

      // Allow empty columns
      dl.set_allow_empty_columns();
   }

   // Process the data lines
   while(ascii_file >> dl) {

      // Check the expected number of columns
      if(dl.n_items() != USCRNFormatMap.at(_format)._nCols) {
         mlog << Error << "\nUscrnHandler::_readObservations() -> "
              << "unexpected number of columns ("
              << dl.n_items() << " != " << USCRNFormatMap.at(_format)._nCols
              << ") on line number " << dl.line_number()
              << " of USCRN file \"" << ascii_file.filename()
              << "\"!\n\n";
         return false;
      }

      // Store the header information
      _stationId  = dl[USCRNFormatMap.at(_format)._sidOffset];
      _stationLon = atof(dl[USCRNFormatMap.at(_format)._lonOffset]);
      _stationLat = atof(dl[USCRNFormatMap.at(_format)._latOffset]);

      // Skip header lines where station ID begins with "WBAN"
      if(_stationId.substr(0,4) == "WBAN") continue;

      // Extract the valid time from the data line
      time_t valid_time = _getUscrnValidTime(dl);
      if(valid_time == 0) return false;

      // Process all observations from the line
      for(const auto &col : USCRNFormatMap.at(_format)._obsInfo) {

         // Get the observation
         string obs_str(dl[col._offset]);

         // Check for empty string
         if(obs_str.empty()) continue;

         // Check for bad data value
         double obs_val = stod(obs_str);
         if(USCRNBadDataInput.has(obs_val)) continue; 

         // Check for QC flag
         string qc_str(na_str);
         if(col._qcOffset > 0) qc_str = dl[col._qcOffset];     

         // Store the observation
         _addObservations(Observation(
            USCRNFormatMap.at(_format)._formatName,
            _stationId, valid_time,
            _stationLat, _stationLon, bad_data_double,
            qc_str, -1, bad_data_double,
            bad_data_double, obs_val,
            col._name, col._units, col._desc));
      }
   } // end while

   return true;
}

////////////////////////////////////////////////////////////////////////

time_t UscrnHandler::_getUscrnValidTime(const DataLine &dl) const {
   struct tm time_struct;
   memset(&time_struct, 0, sizeof(time_struct));

   // Store date string (required)
   string date_str(dl[USCRNFormatMap.at(_format)._ymdOffset]);

   // Store time string (optional)
   string time_str;
   int time_offset = USCRNFormatMap.at(_format)._hmOffset;
   if(time_offset > 0) time_str = dl[time_offset];
   else                time_str = "0000";

   // Process date string formats
   if(date_str.length() == 6) {
      // Append DD to YYYYMM
      date_str.append("01");
   }
   else if(date_str.length() == 8) {
      // No work to do for YYYYMMDD format
   }
   else if(date_str.length() == 10) {
      // Split YYYYMMDDHH into pieces
      time_str = date_str.substr(8, 2);
      time_str.append("00");
      date_str = date_str.substr(0, 8);
   }
   else {
      mlog << Error << "\nUscrnHandler::_getUscrnValidTime() -> "
           << "unexpected date format (" << date_str
           << ") on line number " << dl.line_number() << "!\n\n";
      exit(1);
   }

   // Parse time components
   time_struct.tm_year = stoi(date_str.substr(0, 4)) - 1900;
   time_struct.tm_mon  = stoi(date_str.substr(4, 2)) - 1;
   time_struct.tm_mday = stoi(date_str.substr(6, 2));
   time_struct.tm_hour = stoi(time_str.substr(0, 2));
   time_struct.tm_min  = stoi(time_str.substr(2, 2));

   return timegm(&time_struct);
}

////////////////////////////////////////////////////////////////////////
