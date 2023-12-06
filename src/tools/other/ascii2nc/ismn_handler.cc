// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>

#include "vx_log.h"
#include "vx_math.h"
#include "vx_util.h"

#include "ismn_handler.h"

const int IsmnHandler::MIN_NUM_HDR_COLS = 8;
const int IsmnHandler::NUM_OBS_COLS = 5;

// Relevant GRIB codes
static const int PRATE_GRIB_CODE = 59;
static const int SNOD_GRIB_CODE = 66;
static const int SOILW_GRIB_CODE = 144;
static const int SMS_GRIB_CODE = -1;
static const int WEASD_GRIB_CODE = 65;
static const int TMP_GRIB_CODE = 11;
static const int TSOIL_GRIB_CODE = 85;
static const int AVSFT_GRIB_CODE = 148;

// Mapping of ISMN strings to output variable names
map<string,obsVarInfo> IsmnObsVarMap = {
   { "p",    { PRATE_GRIB_CODE, "PRATE" } },
   { "sd",   { SNOD_GRIB_CODE,  "SNOD"  } },
   { "sm",   { SOILW_GRIB_CODE, "SOILW" } },
   { "su",   { SMS_GRIB_CODE,   "SMS"   } },
   { "sweq", { WEASD_GRIB_CODE, "WEASD" } },
   { "ta",   { TMP_GRIB_CODE,   "TMP"   } },
   { "ts",   { TSOIL_GRIB_CODE, "TSOIL" } },
   { "tsf",  { AVSFT_GRIB_CODE, "AVSFT" } }
};

////////////////////////////////////////////////////////////////////////
//
// Code for class IsmnHandler
//
////////////////////////////////////////////////////////////////////////

IsmnHandler::IsmnHandler(const string &program_name) :
  FileHandler(program_name) {
   use_var_id = true;
}

////////////////////////////////////////////////////////////////////////

IsmnHandler::~IsmnHandler() { }

////////////////////////////////////////////////////////////////////////

bool IsmnHandler::isFileType(LineDataFile &ascii_file) const {

   // ISMN files are identified by having a .stm suffix and
   // checking the number of header and data columns.
   // The header and data lines look like this:
   //   MAQU MAQU NST_24 33.99908 102.13661 3449.0 0.4000 0.4000 ECH20 EC-TM
   //   2014/10/21 07:00 -27.7 G M

   // Initialize using the filename suffix
   bool is_file_type = check_prefix_suffix(ascii_file.short_filename(),
                           nullptr, ".stm");

   // Read the header line
   DataLine dl;
   while(dl.n_items() == 0) ascii_file >> dl;

   // Check the minimum number of header columns
   if(dl.n_items() < MIN_NUM_HDR_COLS) is_file_type = false;

   // Check the number of data line columns
   ascii_file >> dl;
   if(dl.n_items() != NUM_OBS_COLS) is_file_type = false;

   return(is_file_type);
}

////////////////////////////////////////////////////////////////////////
// Private/Protected methods
////////////////////////////////////////////////////////////////////////

bool IsmnHandler::_readObservations(LineDataFile &ascii_file) {

   // Read and save the header information
   if(!_readHeaderInfo(ascii_file)) return(false);

   // Process the observation lines
   DataLine dl;
   while(ascii_file >> dl) {

      // Make sure that the line contains the correct number of tokens
      if(dl.n_items() != NUM_OBS_COLS) {
         mlog << Error << "\nIsmnHandler::_readObservations() -> "
              << "unexpected number of columns (" << dl.n_items()
              << " != " << NUM_OBS_COLS << ") on line number "
              << dl.line_number() << " of ISMN file \""
              << ascii_file.filename() << "\"!\n\n";
         return(false);
      }

      // Extract the valid time from the data line
      time_t valid_time = _getValidTime(dl);
      if(valid_time == 0) return(false);

      // Store the observation value
      double obs_value = atof(dl[2]);

      // Handle unit conversion
      switch(_obsVarInfo._gribCode) {

         // Convert precip rate in second to hours
         case PRATE_GRIB_CODE:
            obs_value /= 3600;
            break;

         // Convert mm to m
         case SNOD_GRIB_CODE:
            obs_value /= 1000.0;
            break;

         // Convert C to K
         case TMP_GRIB_CODE:
         case TSOIL_GRIB_CODE:
         case AVSFT_GRIB_CODE:
            obs_value += 273.15;
            break;

         default:
            break;
      }

      // Store the observation
      _addObservations(Observation(
         _networkName, _stationId, valid_time,
         _stationLat, _stationLon, _stationElv,
         dl[3], _obsVarInfo._gribCode, bad_data_double,
         _depth, obs_value, _obsVarInfo._varName));

   } // end while

   return(true);
}

////////////////////////////////////////////////////////////////////////

time_t IsmnHandler::_getValidTime(const DataLine &dl) const {
   struct tm time_struct;
   memset(&time_struct, 0, sizeof(time_struct));

   // Formatted as YYYY/MM/DD HH:MM
   string ymd_str(dl[0]);
   string hm_str(dl[1]);

   // Validate the time strings
   if(!check_reg_exp("^[0-9]\\{4\\}/[0-9]\\{2\\}/[0-9]\\{2\\}$", dl[0]) ||
      !check_reg_exp("^[0-9]\\{2\\}:[0-9]\\{2\\}$", dl[1])) {
      mlog << Warning << "\nIsmnHandler::_getValidTime() -> "
           << "unexpected time stamp format on line number "
           << dl.line_number() << " of ISMN input file:\n"
           << "  " << dl << "\n\n";
   }
   else {

      // Parse time components
      time_struct.tm_year = stoi(ymd_str.substr(0, 4)) - 1900;
      time_struct.tm_mon  = stoi(ymd_str.substr(5, 2)) - 1;
      time_struct.tm_mday = stoi(ymd_str.substr(8, 2));
      time_struct.tm_hour = stoi( hm_str.substr(0, 2));
      time_struct.tm_min  = stoi( hm_str.substr(3, 2));
   }

   return(timegm(&time_struct));
}

////////////////////////////////////////////////////////////////////////

bool IsmnHandler::_readHeaderInfo(LineDataFile &ascii_file) {

   // The file name is delimited with underscores and the variable name
   // is the fourth item
   ConcatString cs(ascii_file.short_filename());
   StringArray sa = cs.split("_");

   // Validate the file name
   if(sa.n() < 4) {
      mlog << Error << "\nIsmnHandler::_readHeaderInfo() -> "
           << "unexpected ISMN file name \"" << ascii_file.filename()
           << "\"!\n\n";
      return(false);
   }

   // Validate the variable name
   if(IsmnObsVarMap.count(sa[3]) == 0) {
      mlog << Error << "\nIsmnHandler::_readHeaderInfo() -> "
           << "unexpected variable name (" << sa[3]
           << ") found in ISMN file name \"" << ascii_file.filename()
           << "\"!\n\n";
      return(false);
   }

   // Store the observation variable info
   _obsVarInfo = IsmnObsVarMap[sa[3]];

   // Read the header line
   DataLine dl;
   while(dl.n_items() == 0) ascii_file >> dl;

   // Check the minimum number of header columns
   if(dl.n_items() < MIN_NUM_HDR_COLS) {
      mlog << Error << "\nIsmnHandler::_readHeaderInfo() -> "
           << "unexpected number of header columns ("
           << dl.n_items() << " < " << MIN_NUM_HDR_COLS
           << ") in ISMN file \"" << ascii_file.filename()
           << "\"!\n\n";
      return(false);
   }

   // Store the header information
   _networkName = dl[1];
   _stationId   = dl[2];
   _stationLat  = atof(dl[3]);
   _stationLon  = atof(dl[4]);
   _stationElv  = atof(dl[5]);

   // Set the depth for precip as 0
   if(_obsVarInfo._gribCode == PRATE_GRIB_CODE) {
      _depth = 0.0;
   }
   // Otherwise, store the average of the two depths
   else {
     _depth = (atof(dl[6]) + atof(dl[7]))/2.0;
   }

   return(true);
}

////////////////////////////////////////////////////////////////////////
