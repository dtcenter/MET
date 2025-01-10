// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
// Mapping of USCRN {type} variants to metadata. 
//

std::map<USCRNFormat,USCRNFormatInfo> USCRNFormatMap = {

  //
  // "monthly01" with files named "CRNM0102-{Location}.txt
  //   - Format number "02"
  //   - Contains 15 columns defined by:
  //     https://www.ncei.noaa.gov/pub/data/uscrn/products/monthly01/readme.txt
  //
  { USCRNFormat::Monthly,
    { "USCRN-Monthly", "CRNM0102", ".txt", 15, 0, 1, -1, 3, 4, {
      {  5, "T_MONTHLY_MAX",        "C"      },
      {  6, "T_MONTHLY_MIN",        "C"      },
      {  7, "T_MONTHLY_MEAN",       "C"      },
      {  8, "T_MONTHLY_AVG",        "C"      },
      {  9, "P_MONTHLY_CALC",       "mm"     },
      { 10, "SOLRAD_MONTHLY_AVG",   "MJ/m^2" },
      { 12, "SUR_TEMP_MONTHLY_MAX", "C"      },
      { 13, "SUR_TEMP_MONTHLY_MIN", "C"      },
      { 14, "SUR_TEMP_MONTHLY_AVG", "C"      }}
    }
  },

  //
  // "daily01" with files named "{YYYY}/CRND0103-{YYYY}-{Location}.txt
  //   - Format number "03"
  //   - Contains 28 columns defined by:
  //     https://www.ncei.noaa.gov/pub/data/uscrn/products/daily01/readme.txt
  //
  { USCRNFormat::Daily,
    { "USCRN-Daily", "CRND0103", ".txt", 28, 0, 1, -1, 3, 4, {
      {  5, "T_DAILY_MAX",             "C"       },
      {  6, "T_DAILY_MIN",             "C"       },
      {  7, "T_DAILY_MEAN",            "C"       },
      {  8, "T_DAILY_AVG",             "C"       },
      {  9, "P_DAILY_CALC",            "mm"      },
      { 10, "SOLARAD_DAILY",           "MJ/m^2"  },
      { 12, "SUR_TEMP_DAILY_MAX",      "C"       },
      { 13, "SUR_TEMP_DAILY_MIN",      "C"       },
      { 14, "SUR_TEMP_DAILY_AVG",      "C"       },
      { 15, "RH_DAILY_MAX",            "%"       },
      { 16, "RH_DAILY_MIN",            "%"       },
      { 17, "RH_DAILY_AVG",            "%"       },
      { 18, "SOIL_MOISTURE_5_DAILY",   "m^3/m^3" },
      { 19, "SOIL_MOISTURE_10_DAILY",  "m^3/m^3" },
      { 20, "SOIL_MOISTURE_20_DAILY",  "m^3/m^3" },
      { 21, "SOIL_MOISTURE_50_DAILY",  "m^3/m^3" },
      { 22, "SOIL_MOISTURE_100_DAILY", "m^3/m^3" },
      { 23, "SOIL_TEMP_5_DAILY",       "C"       },
      { 24, "SOIL_TEMP_10_DAILY",      "C"       },
      { 25, "SOIL_TEMP_20_DAILY",      "C"       },
      { 26, "SOIL_TEMP_50_DAILY",      "C"       },
      { 27, "SOIL_TEMP_100_DAILY",     "C"       }}
    }
  },

  //
  // "hourly02" with files named "{YYYY}/CRNH0203-{YYYY}-{Location}.txt
  //   - Format number "03"
  //   - Contains 38 columns defined by:
  //     https://www.ncei.noaa.gov/pub/data/uscrn/products/hourly02/readme.txt
  //
  { USCRNFormat::Hourly,
    { "USCRN-Hourly", "CRNH0203", ".txt", 38, 0, 1, 2, 6, 7, {
      {  8, "T_CALC",            "C"         },
      {  9, "T_HR_AVG",          "C"         },
      { 10, "T_MAX",             "C"         },
      { 11, "T_MIN",             "C"         },
      { 12, "P_CALC",            "mm"        },
      { 13, "SOLARAD",           "W/m^2", 14 },
      { 15, "SOLARAD_MAX",       "W/m^2", 16 },
      { 17, "SOLARAD_MIN",       "W/m^2", 18 },
      { 20, "SUR_TEMP",          "C",     21 },
      { 22, "SUR_TEMP_MAX",      "C",     23 },
      { 24, "SUR_TEMP_MIN",      "C",     25 },
      { 26, "RH_HR_AVG",         "%",     27 },
      { 28, "SOIL_MOISTURE_5",   "m^3/m^3"   },
      { 29, "SOIL_MOISTURE_10",  "m^3/m^3"   },
      { 30, "SOIL_MOISTURE_20",  "m^3/m^3"   },
      { 31, "SOIL_MOISTURE_50",  "m^3/m^3"   },
      { 32, "SOIL_MOISTURE_100", "m^3/m^3"   },
      { 33, "SOIL_TEMP_5",       "C"         },
      { 34, "SOIL_TEMP_10",      "C"         },
      { 35, "SOIL_TEMP_20",      "C"         },
      { 36, "SOIL_TEMP_50",      "C"         },
      { 37, "SOIL_TEMP_100",     "C"         }}
    }
  },

  //
  // "subhourly01" with files named "{YYYY}/CRNS0101-{MM}-{YYYY}-{Location}.txt
  //   - Format number "01"
  //   - Contains 23 columns defined by:
  //     https://www.ncei.noaa.gov/pub/data/uscrn/products/subhourly01/readme.txt
  //
  { USCRNFormat::SubHourly,
    { "USCRN-SubHourly", "CRNS0101", ".txt", 23, 0, 1, 2, 6, 7, {
      {  8, "AIR_TEMPERATURE",     "C"         },
      {  9, "PRECIPITATION",       "mm"        },
      { 10, "SOLAR_RADIATION",     "W/m^2", 11 },
      { 12, "SURFACE_TEMPERATURE", "C",     14 },
      { 15, "RELATIVE_HUMIDITY",   "%",     16 },
      { 17, "SOIL_MOISTURE_5",     "m^3/m^3"   },
      { 18, "SOIL_TEMPERATURE_5",  "C"         },
      { 19, "WETNESS",             "Ohms", 20  },
      { 21, "WIND_1_5",            "m/s",  22  }}
    }
  },

  //
  // "soil/soilanom01" with files named "CRNSSM0101-{Location}.csv"
  //   - Format number "01"
  //   - Contains 30 NAMED columns but with no README file provided.
  //
  // Note that "soil/soilclim01" files named "CRNSMC0101-{Location}.csv"
  // are not supported directly here.
  //
  { USCRNFormat::SoilAnom,
    { "USCRN-SoilAnom", "CRNSSM0101", ".csv", 30, 0, 1, -1, 2, 3, {
      {  4, "SMVWC_5_CM",    "m^3/m^3"      },
      {  5, "SMANOM_5_CM",   "Standardized" },
      {  6, "SMPERC_5_CM",   "fraction"     },
      {  7, "ST_5_CM",       "C"            },
      {  8, "SMVWC_10_CM",   "m^3/m^3"      },
      {  9, "SMANOM_10_CM",  "Standardized" },
      { 10, "SMPERC_10_CM",  "fraction"     },
      { 11, "ST_10_CM",      "C"            },
      { 12, "SMVWC_20_CM",   "m^3/m^3"      },
      { 13, "SMANOM_20_CM",  "Standardized" },
      { 14, "SMPERC_20_CM",  "fraction"     },
      { 15, "ST_20_CM",      "C"            },
      { 16, "SMVWC_50_CM",   "m^3/m^3"      },
      { 17, "SMANOM_50_CM",  "Standardized" },
      { 18, "SMPERC_50_CM",  "fraction"     },
      { 19, "ST_50_CM",      "C"            },
      { 20, "SMVWC_100_CM",  "m^3/m^3"      },
      { 21, "SMANOM_100_CM", "Standardized" },
      { 22, "SMPERC_100_CM", "fraction"     },
      { 23, "ST_100_CM",     "C"            },
      { 24, "SMVWC_TOP",     "m^3/m^3"      },
      { 25, "SMANOM_TOP",    "Standardized" },
      { 26, "SMPERC_TOP",    "fraction"     },
      { 27, "SMVWC_COLUMN",  "m^3/m^3"      },
      { 28, "SMANOM_COLUMN", "Standardized" },
      { 29, "SMPERC_COLUMN", "fraction"     }}
    }
  },

  //
  // "heat01" with files named "SCRNHE0101-{Location}.csv"
  //   - Format number "01"
  //   - Contains 16 NAMED columns described by:
  //     https://www.ncei.noaa.gov/pub/data/uscrn/products/heat01/readme.txt
  //
  { USCRNFormat::Heat,
    { "USCRN-Heat", "CRNHE0101", ".csv", 16, 0, 1, -1, 2, 3, {
      {  4, "RELATIVE_HUMIDITY",             "%"     },
      {  5, "SURFACE_PRESSURE",              "hPa"   },
      {  6, "SOLAR_RADIATION",               "W/m^2" },
      {  7, "ESTIMATED_10_METER_WIND_SPEED", "m/s"   },
      {  8, "DRY_BULB_TEMPERATURE_C",        "C"     },
      {  9, "HEAT_INDEX_C",                  "C"     },
      { 10, "APPARENT_TEMPERATURE_C",        "C"     },
      { 11, "WET_BULB_GLOBE_TEMPERATURE_C",  "C"     },
      { 12, "DRY_BULB_TEMPERATURE_F",        "F"     },
      { 13, "HEAT_INDEX_F",                  "F"     },
      { 14, "APPARENT_TEMPERATURE_F",        "F"     },
      { 15, "WET_BULB_GLOBE_TEMPERATURE_F",  "F"     }}
    }
  },

  //
  // "drought01" with files named "CRNDI0101-{Location}.csv
  //   - Format number "01"
  //   - Contains 32 NAMED columns described by:
  //     https://www.ncei.noaa.gov/pub/data/uscrn/products/drought01/readme.txt
  //
  // TODO: Consider excluding the 30COUNTS and 70COUNTS columns since they
  //       are likely not useful for verification.
  //
  { USCRNFormat::Drought,
    { "USCRN-Drought", "CRNDI0101", ".csv", 32, 0, 1, -1, 2, 3, {
      {  4, "SMVWC_5_CM_MEAN",          "m^3/m^3"       },
      {  5, "SMANOM_5_CM_MEAN",         "Standardized"  },
      {  6, "SMPERC_5_CM_30COUNTS",     "fraction"      },
      {  7, "SMPERC_5_CM_70COUNTS",     "fraction"      },
      {  8, "SMVWC_10_CM_MEAN",         "m^3/m^3"       },
      {  9, "SMANOM_10_CM_MEAN",        "Standardized"  },
      { 10, "SMPERC_10_CM_30COUNTS",    "fraction"      },
      { 11, "SMPERC_10_CM_70COUNTS",    "fraction"      },
      { 12, "SMVWC_20_CM_MEAN",         "m^3/m^3"       },
      { 13, "SMANOM_20_CM_MEAN",        "Standardized"  },
      { 14, "SMPERC_20_CM_30COUNTS",    "fraction"      },
      { 15, "SMPERC_20_CM_70COUNTS",    "fraction"      },
      { 16, "SMVWC_50_CM_MEAN",         "m^3/m^3"       },
      { 17, "SMANOM_50_CM_MEAN",        "Standardized"  },
      { 18, "SMPERC_50_CM_30COUNTS",    "fraction"      },
      { 19, "SMPERC_50_CM_70COUNTS",    "fraction"      },
      { 20, "SMVWC_100_CM_MEAN",        "m^3/m^3"       },
      { 21, "SMANOM_100_CM_MEAN",       "Standardized"  },
      { 22, "SMPERC_100_CM_30COUNTS",   "fraction"      },
      { 23, "SMPERC_100_CM_70COUNTS",   "fraction"      },
      { 24, "SMVWC_TOP_CM_MEAN",        "m^3/m^3"       },
      { 25, "SMANOM_TOP_CM_MEAN",        "Standardized" },
      { 26, "SMPERC_TOP_CM_30COUNTS",    "fraction"     },
      { 27, "SMPERC_TOP_CM_70COUNTS",    "fraction"     },
      { 28, "SMVWC_COLUMN_CM_MEAN",      "m^3/m^3"      },
      { 29, "SMANOM_COLUMN_CM_MEAN",     "Standardized" },
      { 30, "SMPERC_COLUMN_CM_30COUNTS", "fraction"     },
      { 31, "SMPERC_COLUMN_CM_70COUNTS", "fraction"     }}
    }
  }
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

UscrnHandler::~UscrnHandler() { }

////////////////////////////////////////////////////////////////////////

bool UscrnHandler::isFileType(LineDataFile &ascii_file) const {
   return _getFileFormat(ascii_file) != USCRNFormat::None;
}

////////////////////////////////////////////////////////////////////////
// Private/Protected methods
////////////////////////////////////////////////////////////////////////

USCRNFormat UscrnHandler::_getFileFormat(const LineDataFile &ascii_file) const {
   USCRNFormat fmt = USCRNFormat::None;

   // USCRN files are identified by their prefix and suffix.

   // Loop over supported USCRN formats
   for(auto &x : USCRNFormatMap) {

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

   // Read and save the header information
   if(!_readHeaderInfo(ascii_file)) return false;

   // Check for .csv format:
   // - .csv files have a header line to be skipped.
   // - .txt files have no header line to be skipped.
   DataLine dl;
   ConcatString file_name(ascii_file.filename());
   if(file_name.endswith(".csv")) {

      // Set the delimiter
      dl.set_delimiter(",");

      // Allow empty columns
      dl.set_allow_empty_columns();

      // Read the and skip the header line with column names
      while(dl.n_items() == 0) ascii_file >> dl;
   }

   // Process the data lines
   while(ascii_file >> dl) {

      // Check the expected number of columns
      if(dl.n_items() != USCRNFormatMap[_format]._nCols) {
         mlog << Error << "\nUscrnHandler::_readObservations() -> "
              << "unexpected number of columns ("
              << dl.n_items() << " != " << USCRNFormatMap[_format]._nCols
              << ") on line number " << dl.line_number()
              << " of USCRN file \"" << ascii_file.filename()
              << "\"!\n\n";
         return false;
      }

      // Extract the valid time from the data line
      time_t valid_time = _getValidTime(dl);
      if(valid_time == 0) return false;

      // Process all observations from the line
      for(auto &col : USCRNFormatMap[_format]._obsInfo) {

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
            _formatName, _stationId, valid_time,
            _stationLat, _stationLon, bad_data_double,
            qc_str, -1, bad_data_double,
            bad_data_double, obs_val,
	    col._name));
	 // TODO: Add logic to convert/store units
      }
   } // end while

   return true;
}

////////////////////////////////////////////////////////////////////////

time_t UscrnHandler::_getValidTime(const DataLine &dl) const {
   struct tm time_struct;
   memset(&time_struct, 0, sizeof(time_struct));

   // Store date string (required)
   string date_str(dl[USCRNFormatMap[_format]._ymdOffset]);

   // Store time string (optional)
   string time_str;
   int time_offset = USCRNFormatMap[_format]._hmOffset;
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
      mlog << Error << "\nUscrnHandler::_getValidTime() -> "
           << "unexpected date format (" << date_str
           << ") on line number " << dl.line_number() << "!\n\n";
      exit(1);
   }

   // Parse time components
   time_struct.tm_year = stoi(date_str.substr(0, 4));
   time_struct.tm_mon  = stoi(date_str.substr(4, 2));
   time_struct.tm_mday = stoi(date_str.substr(6, 2));
   time_struct.tm_hour = stoi(time_str.substr(0, 2));
   time_struct.tm_min  = stoi(time_str.substr(2, 2));

   return timegm(&time_struct);
}

////////////////////////////////////////////////////////////////////////

bool UscrnHandler::_readHeaderInfo(LineDataFile &ascii_file) {

   // Check for .csv format
   DataLine dl;
   ConcatString file_name(ascii_file.filename());
   if(file_name.endswith(".csv")) dl.set_delimiter(",");

   // Read the header line
   while(dl.n_items() == 0) ascii_file >> dl;

   // Check the expected number of columns
   if(dl.n_items() != USCRNFormatMap[_format]._nCols) {
      mlog << Error << "\nUscrnHandler::_readHeaderInfo() -> "
           << "unexpected number of columns ("
           << dl.n_items() << " != " << USCRNFormatMap[_format]._nCols
           << ") on line number " << dl.line_number()
           << " of USCRN file \"" << ascii_file.filename()
           << "\"!\n\n";
      return false;
   }

   // Store the header information
   _formatName = USCRNFormatMap[_format]._formatName;
   _stationId  = dl[USCRNFormatMap[_format]._sidOffset];
   _stationLon = atof(dl[USCRNFormatMap[_format]._lonOffset]);
   _stationLat = atof(dl[USCRNFormatMap[_format]._latOffset]);

   // Rewind to the beginning
   ascii_file.rewind();

   return true;
}

////////////////////////////////////////////////////////////////////////
