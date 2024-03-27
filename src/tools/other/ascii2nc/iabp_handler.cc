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

const double IabpHandler::IABP_MISSING_VALUE = -999.0;


const int IabpHandler::MIN_NUM_HDR_COLS = 8;

// days in the month
static int daysOfMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};    

static int _lookfor(const DataLine &dl, const string &name);
static int _lookfor(const DataLine &dl, const string &name, const string &ascii_file, bool &ok);
static time_t _time(const string &syear, const string &shour, const string &smin, const string &sdoy);


////////////////////////////////////////////////////////////////////////
//
// Code for class IabpHandler
//
////////////////////////////////////////////////////////////////////////

IabpHandler::IabpHandler(const string &program_name) :
  FileHandler(program_name) {
   use_var_id = true;
}

////////////////////////////////////////////////////////////////////////

IabpHandler::~IabpHandler() {
}

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
   if(dl.n_items() < MIN_NUM_HDR_COLS) {
      return false;
   }

   string line = dl.get_line();
   ConcatString cstring(line);

   StringArray tokens = cstring.split(" ");
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

      // Extract the valid time from the data line, using POS_DOY (scientist is most
      // interested in location) Use POS_DOY to compute the month and day based on year
      // (to handle leap year)
      time_t valid_time = _time(dl[_yearPtr], dl[_hourPtr], dl[_minutePtr], dl[_posdoyPtr]);
      if(valid_time == 0) {
         mlog << Warning << "\nIabpHandler::_readObservations() -> "
              << "No valid time computed in file, line number "
              << dl.line_number() << " of IABP file \""
              << ascii_file.filename() << "\".  Ignore this line\n\n";
         return(false);
      }
      
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
         // is this the right placeholder for this? To always put it in to the
         // fixed slot of an observation?
         pres = stod(dl[_bpPtr]);
         if (pres == IABP_MISSING_VALUE) {
            pres = bad_data_double;
         }
      }
      if (_tsPtr >= 0) {
         ts = stod(dl[_tsPtr]);
         if (ts == IABP_MISSING_VALUE) {
            ts = bad_data_double;
         }
      }
      _addObservations(Observation(
                                   header_type, stationId, valid_time,
                                   lat, lon, elev, quality_flag, grib_code, 
                                   pres, height_m, ts, "Temp_surface"));
      grib_code++;
      
      if (_taPtr >= 0) {
         ta = stod(dl[_taPtr]);
         if (ta == IABP_MISSING_VALUE) {
            ta = bad_data_double;
         }
      }
      _addObservations(Observation(
                                   header_type, stationId, valid_time,
                                   lat, lon, elev, quality_flag, grib_code, 
                                   pres, height_m, ta, "Temp_air"));
   } // end while

   return(true);
}

// ////////////////////////////////////////////////////////////////////////

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

   return retval;
}
