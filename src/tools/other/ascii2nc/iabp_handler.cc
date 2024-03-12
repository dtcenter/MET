// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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

#include "iabp_handler.h"

const int IabpHandler::MIN_NUM_HDR_COLS = 8;

// days in the month
static int daysOfMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};    

static int _lookfor(const DataLine &dl, const string &name);
static int _lookfor(const DataLine &dl, const string &name, const string &ascii_file, bool &ok);
static time_t _time(const string &syear, const string &shour, const string &smin, const string &sdoy);

#ifdef NOT
// const int IabpHandler::NUM_OBS_COLS = 5;

// Mapping of IABP strings to output variable names
map<string,obsVarInfo> IabpObsVarMap = {
   { "p",    { PRATE_GRIB_CODE, "PRATE" } },
   { "sd",   { SNOD_GRIB_CODE,  "SNOD"  } },
   { "sm",   { SOILW_GRIB_CODE, "SOILW" } },
   { "su",   { SMS_GRIB_CODE,   "SMS"   } },
   { "sweq", { WEASD_GRIB_CODE, "WEASD" } },
   { "ta",   { TMP_GRIB_CODE,   "TMP"   } },
   { "ts",   { TSOIL_GRIB_CODE, "TSOIL" } },
   { "tsf",  { AVSFT_GRIB_CODE, "AVSFT" } }
};
#endif

////////////////////////////////////////////////////////////////////////
//
// Code for class IabpHandler
//
////////////////////////////////////////////////////////////////////////

IabpHandler::IabpHandler(const string &program_name) :
  FileHandler(program_name) {
   use_var_id = false;
}

////////////////////////////////////////////////////////////////////////

IabpHandler::~IabpHandler() { }

////////////////////////////////////////////////////////////////////////

bool IabpHandler::isFileType(LineDataFile &ascii_file) const {

   // IABP files are identified by having a .dat suffix and
   // checking the always present data columns.
   // The header look like this:
   //     BuoyID     Year     Hour     Min     DOY     POS_DOY     Lat     Lon   [  BP     Ts     Ta]

   // Initialize using the filename suffix
   bool is_file_type = check_prefix_suffix(ascii_file.short_filename(),
                           nullptr, ".dat");

   // Read the header line
   DataLine dl;
   while(dl.n_items() == 0) ascii_file >> dl;

   // Check the minimum number of header columns
   if(dl.n_items() < MIN_NUM_HDR_COLS) is_file_type = false;

   string line = dl.get_line();
   ConcatString cstring(line);

   StringArray tokens = cstring.split(" ");
   //string s = tokens[0];
   if (tokens[0] != "BuoyID") is_file_type = false;
   if (tokens[1] != "Year") is_file_type = false;
   if (tokens[2] != "Hour") is_file_type = false;
   if (tokens[3] != "Min") is_file_type = false;
   if (tokens[4] != "DOY") is_file_type = false;
   if (tokens[5] != "POS_DOY") is_file_type = false;
   if (tokens[6] != "Lat") is_file_type = false;
   if (tokens[7] != "Lon") is_file_type = false;

   return(is_file_type);
}

////////////////////////////////////////////////////////////////////////
// Private/Protected methods
////////////////////////////////////////////////////////////////////////

bool IabpHandler::_readObservations(LineDataFile &ascii_file)
{
   // Read and save the header information
   if(!_readHeaderInfo(ascii_file)) return(false);

   string header_type = "IABP_STANDARD";

   // // Get the var_id to use
   // int var_id = bad_data_int;
   // if(!_varNames.has(_obsVarInfo._varName, var_id)) return(false);
   
   // Process the observation lines
   DataLine dl;
   while(ascii_file >> dl) {

      // Make sure that the line contains the correct number of tokens
      if(dl.n_items() != _numColumns) {
         mlog << Error << "\nIabpHandler::_readObervations() -> "
              << "unexpected number of columns (" << dl.n_items()
              << " != " << _numColumns << ") on line number "
              << dl.line_number() << " of IABP file \""
              << ascii_file.filename() << "\"!\n\n";
         return(false);
      }

      // Extract the valid time from the data line
      time_t valid_time = _time(dl[_yearPtr], dl[_hourPtr], dl[_minutePtr], dl[_doyPtr]);
      if(valid_time == 0) return(false);

      double lat = stod(dl[_latPtr]);
      double lon = stod(dl[_lonPtr]);
      string stationId = dl[_idPtr];
      string quality_flag = na_str;
      int grib_code = 0;
      double height_m = bad_data_double;
      double pres = bad_data_double;
      double elev = bad_data_double;
      double ts = bad_data_double;
      double ta = bad_data_double;

      if (_bpPtr >= 0) {
         pres = stod(dl[_bpPtr]);
      }
      if (_tsPtr >= 0) {
         ts = stod(dl[_tsPtr]);
      }
      _addObservations(Observation(
                                      header_type, stationId, valid_time,
                                      lat, lon, elev, quality_flag, grib_code, 
                                      pres, height_m, ts, "Temp_surface"));
      // }
      if (_taPtr >= 0) {
         ta = stod(dl[_taPtr]);
      }
      _addObservations(Observation(
                                      header_type, stationId, valid_time,
                                      lat, lon, elev, quality_flag, grib_code+1, 
                                      pres, height_m, ta, "Temp_air"));

      // }

      // if (_tsPtr < 0 && _taPtr < 0) {
         // add a placeholder that will be all missing, (Need to have a variable).
      //    _addObservations(Observation(
      //                                 header_type, stationId, valid_time,
      //                                 lat, lon, elev, quality_flag, grib_code+1, 
      //                                 pres, height_m, ta, "Temp_air"));
      // }

   } // end while

   return(true);
}

// ////////////////////////////////////////////////////////////////////////

// time_t IabpHandler::_getValidTime(const DataLine &dl) const {


//    // we have a year, hour, minute, and dayofyear
   
//    struct tm time_struct;
//    memset(&time_struct, 0, sizeof(time_struct));

//    // Formatted as YYYY/MM/DD HH:MM
//    string ymd_str(dl[0]);
//    string hm_str(dl[1]);

//    // Validate the time strings
//    if(!check_reg_exp("^[0-9]\\{4\\}/[0-9]\\{2\\}/[0-9]\\{2\\}$", dl[0]) ||
//       !check_reg_exp("^[0-9]\\{2\\}:[0-9]\\{2\\}$", dl[1])) {
//       mlog << Warning << "\nIabpHandler::_getValidTime() -> "
//            << "unexpected time stamp format on line number "
//            << dl.line_number() << " of IABP input file:\n"
//            << "  " << dl << "\n\n";
//    }
//    else {

//       // Parse time components
//       time_struct.tm_year = stoi(ymd_str.substr(0, 4)) - 1900;
//       time_struct.tm_mon  = stoi(ymd_str.substr(5, 2)) - 1;
//       time_struct.tm_mday = stoi(ymd_str.substr(8, 2));
//       time_struct.tm_hour = stoi( hm_str.substr(0, 2));
//       time_struct.tm_min  = stoi( hm_str.substr(3, 2));
//    }

//    return(timegm(&time_struct));
// #endif
//    return 0;
// }

////////////////////////////////////////////////////////////////////////

bool IabpHandler::_readHeaderInfo(LineDataFile &ascii_file) {

   DataLine dl;
   if (!(ascii_file >> dl))
   {
      mlog << Error << "\nIabpHandler::_readHeaderInfo() -> "
           << "error reading header line from input ASCII file \""
           << ascii_file.filename() << "\"\n\n";
      return false;
   }
      
   // Check the minimum number of header columns
   if(dl.n_items() < MIN_NUM_HDR_COLS) {
      mlog << Error << "\nIabpHandler::_readHeaderInfo() -> "
           << "unexpected number of header columns ("
           << dl.n_items() << " < " << MIN_NUM_HDR_COLS
           << ") in IABP file \"" << ascii_file.filename()
           << "\"!\n\n";
      return(false);
   }

   // Map the header information to column numbers
   bool ok = true;
   string filename = ascii_file.filename();
   _idPtr = _lookfor(dl, "BuoyID", filename, ok);
   _yearPtr = _lookfor(dl, "Year", filename, ok);
   _hourPtr = _lookfor(dl, "Hour", filename, ok);
   _minutePtr = _lookfor(dl, "Min", filename, ok);
   _doyPtr = _lookfor(dl, "DOY", filename, ok);
   _posdoyPtr = _lookfor(dl, "POS_DOY", filename, ok);
   _latPtr = _lookfor(dl, "Lat", filename, ok);
   _lonPtr = _lookfor(dl, "Lon", filename, ok);
   _numColumns = MIN_NUM_HDR_COLS;
   _bpPtr = _lookfor(dl, "BP");
   if (_bpPtr >= 0) ++_numColumns;
   _tsPtr = _lookfor(dl, "Ts");
   if (_tsPtr >= 0) ++_numColumns;
   _taPtr = _lookfor(dl, "Ta");
   if (_taPtr >= 0) ++_numColumns;
   return ok;

   // _networkName = dl[1];
   // _stationId   = dl[2];
   // _stationLat  = atof(dl[3]);
   // _stationLon  = atof(dl[4]);
   // _stationElv  = atof(dl[5]);

   // return(true);
}

////////////////////////////////////////////////////////////////////////

static int _lookfor(const DataLine &dl, const string &name, const string &ascii_file, bool &ok)
{
   for (int i=0; i<dl.n_items(); ++i) {
      if (dl[i] == name) {
         return i;
      }
   }
   mlog << Warning << "\nIabpHandler::_lookfor() -> "
        << "reading ASCII file \""
        << ascii_file << "\" did not find expected header item:\"" << name << "\" ignore file\n\n";
   ok = false;
   return -1;
}

////////////////////////////////////////////////////////////////////////

static int _lookfor(const DataLine &dl, const string &name)
{
   for (int i=0; i<dl.n_items(); ++i) {
      if (dl[i] == name) {
         return i;
      }
   }
   return -1;
}

////////////////////////////////////////////////////////////////////////

static time_t _time(const string &syear, const string &shour, const string &smin, const string &sdoy)
{
   int year = stoi(syear);
   int hour = stoi(shour);
   int min = stoi(smin);
   double doy = stod(sdoy);
   time_t retval = doyhms_to_unix((int)doy, year, hour, min, 0);

#ifdef TESTING_ONLY
   // an approach to compare figuring out hour and minute using doy, as apposed to using input hour/minute
   int mjd = date_to_mjd(1, 0, year) + (int)doy;
   int day, month;
   mjd_to_date(mjd, month, day, year);

   // have day and month, now just examine the fractional part of doy, this is in units of days I guess
   double hms = doy - (double)(int)doy;

   // 24 hours per day, convert to hours
   double dhours = hms*24;

   // truncate 
   int hour2 = (int)dhours;  

   double dms = (dhours - (double)hour2)*60.0;
   // round
   int min2 = nint(dms-0.5);
   while (min2 >= 60)
   {
      min2 -= 60;
      hour2 += 1;
   }

   if (min2 != min || hour2 != hour)
   {
      mlog << Warning << "doy=" << doy << " hour=" << hour << " hour2=" << hour2 << " min=" << min << " min2=" << min2 << "\n";
   }


   int seconds = 0;  // for now
   
   time_t answer2 = mdyhms_to_unix(month, day, year, hour2, minute, seconds);
   // check for consistency
   if (answer != retval) {
      mlog << Warning << "\nIabpHandler::_time() -> time mismatch by " << answer-retval << " seconds\n\n";
   }   
#endif
   
   return retval;
}
