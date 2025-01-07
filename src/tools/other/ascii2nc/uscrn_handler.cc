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

// Mapping of USCRN variant to metadata 
std::map<USCRNFormat,USCRNFormatInfo> USCRNFormatMap = {
  { USCRNFormat::Monthly,
    { "USCRN-Monthly", "CRNM0102", ".txt", 15, 0, 1, -1, 3, 4, {
      {  5, "TMP_MAX",   "C"  },
      {  6, "TMP_MIN",   "C"  },
      {  7, "TMP_MEAN",  "C"  },
      {  8, "TMP_AVG",   "C"  },
      {  9, "APCP",      "mm" },
      { 12, "SKINT_MAX", "C"  },
      { 13, "SKINT_MIN", "C"  },
      { 14, "SKINT_AVG", "C"  }}
    }
  },
  { USCRNFormat::Daily,
    { "USCRN-Daily", "CRND0103", ".txt", 28, 0, 1, -1, 3, 4, {
      {  5, "TMP_MAX",     "C"       },
      {  6, "TMP_MIN",     "C"       },
      {  7, "TMP_MEAN",    "C"       },
      {  8, "TMP_AVG",     "C"       },
      {  9, "APCP",        "mm"      },
      { 12, "SKINT_MAX",   "C"       },
      { 13, "SKINT_MIN",   "C"       },
      { 14, "SKINT_AVG",   "C"       },
      { 15, "RH_MAX",      "%"       },
      { 16, "RH_MIN",      "%"       },
      { 17, "RH_AVG",      "%"       },
      { 18, "SOILMOI_5",   "m^3/m^3" },
      { 19, "SOILMOI_10",  "m^3/m^3" },
      { 20, "SOILMOI_20",  "m^3/m^3" },
      { 21, "SOILMOI_50",  "m^3/m^3" },
      { 22, "SOILMOI_100", "m^3/m^3" },
      { 23, "SOILTMP_5",   "C"       },
      { 24, "SOILTMP_10",  "C"       },
      { 25, "SOILTMP_20",  "C"       },
      { 26, "SOILTMP_50",  "C"       },
      { 27, "SOILTMP_100", "C"       }}
    }
  },
  { USCRNFormat::Hourly,
    { "USCRN-Hourly", "CRNH0203", ".txt", 38, 0, 1, 2, 6, 7, {
      {  8, "TMP_CALC",    "C"         },
      {  9, "TMP_AVG",     "C"         },
      { 10, "TMP_MAX",     "C"         },
      { 11, "TMP_MIN",     "C"         },
      { 12, "APCP",        "mm"        },
      { 13, "SOLARAD_AVG", "W/m^2", 14 },
      { 15, "SOLARAD_MAX", "W/m^2", 16 },
      { 17, "SOLARAD_MIN", "W/m^2", 18 },
      { 20, "SKINT_AVG",   "C",     21 },
      { 22, "SKINT_MAX",   "C",     23 },
      { 24, "SKINT_MIN",   "C",     25 },
      { 26, "RH_AVG",      "%",     27 },
      { 28, "SOILMOI_5",   "m^3/m^3"   },
      { 29, "SOILMOI_10",  "m^3/m^3"   },
      { 30, "SOILMOI_20",  "m^3/m^3"   },
      { 31, "SOILMOI_50",  "m^3/m^3"   },
      { 32, "SOILMOI_100", "m^3/m^3"   },
      { 33, "SOILTMP_5",   "C"         },
      { 34, "SOILTMP_10",  "C"         },
      { 35, "SOILTMP_20",  "C"         },
      { 36, "SOILTMP_50",  "C"         },
      { 37, "SOILTMP_100", "C"         }}
    }
  }
};
// TODO: Add support for other format types
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

   // Process the data lines
   DataLine dl;
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
         double obs_val = stod(obs_str);

         // Check for missing or bad data
         if(obs_str.empty() || USCRNBadDataInput.has(obs_val)) continue; 

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

   // YMD formatted as YYYYMM or YYYYMMDD
   string ymd_str(dl[USCRNFormatMap[_format]._ymdOffset]);

   // Append 01 to YYYYMM string
   if(ymd_str.length() == 6) ymd_str.append("01"); 

   // HM is either not present (-1) or formatted as HHMM
   string hm_str;
   int offset = USCRNFormatMap[_format]._hmOffset;
   if(offset > 0) hm_str = dl[offset];
   else           hm_str = "0000";

   // Parse time components
   time_struct.tm_year = stoi(ymd_str.substr(0, 4));
   time_struct.tm_mon  = stoi(ymd_str.substr(4, 2));
   time_struct.tm_mday = stoi(ymd_str.substr(6, 2));
   time_struct.tm_hour = stoi( hm_str.substr(0, 2));
   time_struct.tm_min  = stoi( hm_str.substr(2, 2));

   return timegm(&time_struct);
}

////////////////////////////////////////////////////////////////////////

bool UscrnHandler::_readHeaderInfo(LineDataFile &ascii_file) {

   // Read the header line
   DataLine dl;
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
