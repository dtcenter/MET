// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   madis2nc.cc
//
//   Description:
//      Parse MADIS NetCDF files containing surface point observations
//      and reformat them for use by MET.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    07-14-09  Contributed    New
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <regex.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "netcdf.hh"

#include "vx_analysis_util.h"
#include "vx_met_util.h"
#include "vx_cal.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

// Constants
static const char *program_name = "madis2nc";
static const float fill_value   = -9999.f;
static const int   strl_len     = 16; // Length of "YYYYMMDD_HHMMSS"
static const int   hdr_arr_len  = 3;  // Observation header length
static const int   obs_arr_len  = 5;  // Observation values length

// Strings for input file: these are for metar files:
const char *in_recNum_str;
const char *in_fillValue_str;
const char *in_reportType_str;
const char *in_stationName_str;
const char *in_timeObs_str;
const char *in_latitude_str;
const char *in_longitude_str;
const char *in_elevation_str;
const char *in_visibility_str;
const char *in_seaLevelPress_str;
const char *in_temperature_str;
const char *in_dewpoint_str;
const char *in_windDir_str;
const char *in_windSpeed_str;
const char *in_windGust_str;
const char *in_minTemp24Hour_str;
const char *in_maxTemp24Hour_str;
const char *in_precip1Hour_str;
const char *in_precip3Hour_str;
const char *in_precip6Hour_str;
const char *in_precip24Hour_str;
const char *in_snowCover_str;

// Corresponding GRIB code values
const int   in_visibility_gc     = 20;
const int   in_seaLevelPress_gc  = 2;
const int   in_temperature_gc    = 11;
const int   in_dewpoint_gc       = 17;
const int   in_windDir_gc        = 31;
const int   in_windSpeed_gc      = 32;
const int   in_windGust_gc       = 180;
const int   in_minTemp24Hour_gc  = 16;
const int   in_maxTemp24Hour_gc  = 15;
const int   in_precip_gc         = 61; // Convert from m to mm
const int   in_snowCover_gc      = 66;

// Strings for out file
const char* out_hdr_typ_str = "hdr_typ";
const char* out_hdr_sid_str = "hdr_sid";
const char* out_hdr_vld_str = "hdr_vld";
const char* out_hdr_arr_str = "hdr_arr";
const char* out_obs_arr_str = "obs_arr";

// Variables for command line arguments
static ConcatString awfile;
static ConcatString ncfile;
static int verbosity = 2;

////////////////////////////////////////////////////////////////////////

static void write_data(NcFile *&, NcFile *&);
static void write_obs_arr(NcFile *&f_in, const int, const char *,
                          const int, const float, NcVar *&, int &, const float);
static void usage(int, char **);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   int i;

   //
   // Set handler to be called for memory allocation error
   //
   set_new_handler(oom);

   //
   // Input and output files
   //
   NcFile *f_in  = (NcFile *) 0; 
   NcFile *f_out = (NcFile *) 0;

   //
   // Check for the correct number of arguments
   //
   if(argc < 3) {
      usage(argc, argv);
      exit(1);
   }

   //
   // Store the input MADIS file name and the output NetCDF file name
   //
   awfile = argv[1];
   ncfile = argv[2];

   //
   // Parse command line arguments
   //
   for(i=0; i<argc; i++) {

      if(strcmp(argv[i], "-v") == 0) {
         verbosity = atoi(argv[i+1]);
         i++;
      }
      else if(argv[i][0] == '-') {
         cerr << "\n\nERROR: main() -> "
              << "unrecognized command line switch: "
              << argv[i] << "\n\n" << flush;
         exit(1);
      }
   } // end for i

   //
   // Open the input MADIS NetCDF file for reading
   //
   if(verbosity > 0) cout << "Reading: " << awfile << "\n" << flush;

   f_in = new NcFile(awfile);

   if(!f_in->is_valid()) {
      cerr << "\n\nERROR: main() -> "
           << "can't open input NetCDF file \"" << awfile
           << "\" for reading.\n\n" << flush;
      f_in->close();
      delete f_in;
      f_in = (NcFile *) 0;

      exit(1);
   }

   //
   // Open the output NetCDF file for writing
   //
   if(verbosity > 0) cout << "Writing: " << ncfile << "\n" << flush;

   f_out = new NcFile(ncfile, NcFile::Replace);

   if(!f_out->is_valid()) {
      cerr << "\n\nERROR: main() -> "
           << "can't open output NetCDF file \"" << ncfile
           << "\" for writing.\n\n" << flush;
      f_out->close();
      delete f_out;
      f_out = (NcFile *) 0;

      exit(1);
   }

   //
   // Process the MADIS observations
   //
   write_data(f_in, f_out);

   return(0);
}

////////////////////////////////////////////////////////////////////////

void write_data(NcFile *&f_in, NcFile *&f_out) {
   int mon, day, yr, hr, min, sec;
   int nhdr, i_obs, i_hdr;
   char tmp_str[max_str_len];
   char hostname_str[max_str_len];
   char attribute_str[PATH_MAX];
   DataLine dl;
   double ut;
   ConcatString hdr_typ, hdr_sid, hdr_vld;
   float hdr_arr[hdr_arr_len];

   /////////////////////////////////////////////////////////////////////

   //
   // Define input NetCDF dimensions and variables
   //
   NcDim *in_recNum_dim        = (NcDim *) 0; // Record number dimension

   // Header variables
   NcVar *in_reportType_var    = (NcVar *) 0;
   NcVar *in_stationName_var   = (NcVar *) 0;
   NcVar *in_timeObs_var       = (NcVar *) 0;
   NcVar *in_latitude_var      = (NcVar *) 0;
   NcVar *in_longitude_var     = (NcVar *) 0;
   NcVar *in_elevation_var     = (NcVar *) 0;

   // Strings for input file: these are common for all MADIS files:
   in_recNum_str        = "recNum";
   in_fillValue_str     = "_FillValue";
   in_stationName_str   = "stationName";
   in_latitude_str      = "latitude";
   in_longitude_str     = "longitude";

   // get "title" attribute:
   NcAtt *id_att = f_in->get_att("title");
   if(!id_att || !id_att->is_valid()) {
      cerr << "\n\nERROR: write_data() -> "
           << "can't get the \"title\" attribute \n" << flush;
      exit(1);
   }
   NcValues *id_val=id_att->values();
   strcpy(tmp_str,(char *)id_val->base());
   if(verbosity > 1) {
      cout << "Retrieved global attribute \"title\"=" << tmp_str << "\n" << flush;
   }
   enum madis_type {UNSUPPORTED,ACARS,ACARSPROFILES,COOP,HYDRO,MARITIME,MESONET,METAR,SAO,SNOW};
   enum madis_type found_id=UNSUPPORTED;
   if (strncmp(tmp_str, "MADIS ACARS data", 16) == 0) {
     //TBD: add support for this type
     found_id = ACARS;
// Strings for input file: these are for acars files:
     in_stationName_str   = "origAirport";
     in_reportType_str    = "destAirport";
     hdr_typ = "ACARS"; //ignore stationType, encode all as "ACARS"
     in_timeObs_str       = "timeObs";
     in_elevation_str     = "indAltitude";
     in_visibility_str    = "NA";
     in_seaLevelPress_str = "NA";
     in_temperature_str   = "temperature";
     in_dewpoint_str      = "dewpoint";
     in_windDir_str       = "windDir";
     in_windSpeed_str     = "windSpeed";
     in_windGust_str      = "NA";
     in_minTemp24Hour_str = "NA";
     in_maxTemp24Hour_str = "NA";
     in_precip1Hour_str   = "NA";
     in_precip3Hour_str   = "NA";
     in_precip6Hour_str   = "NA";
     in_precip24Hour_str  = "NA";
     in_snowCover_str     = "NA";
   }
   if (strncmp(tmp_str, "MADIS ACARS profile data", 24) == 0) {
     //TBD: add support for this type
     found_id = ACARSPROFILES;
// Strings for input file: these are for acars files:
     in_reportType_str    = "profileAirport";
     in_stationName_str   = "locationName";
     hdr_typ = "PROACARS"; //ignore stationType, encode all as "PROACARS"
     in_timeObs_str       = "profileTime";
     in_elevation_str     = "altitude";
     in_visibility_str    = "NA";
     in_seaLevelPress_str = "NA";
     in_temperature_str   = "temperature";
     in_dewpoint_str      = "dewpoint";
     in_windDir_str       = "windDir";
     in_windSpeed_str     = "windSpeed";
     in_windGust_str      = "NA";
     in_minTemp24Hour_str = "NA";
     in_maxTemp24Hour_str = "NA";
     in_precip1Hour_str   = "NA";
     in_precip3Hour_str   = "NA";
     in_precip6Hour_str   = "NA";
     in_precip24Hour_str  = "NA";
     in_snowCover_str     = "NA";
   }
   if (strncmp(tmp_str, "MADIS - Meteorological Surface - Modernized NWS Cooperative Observer", 68) == 0) {
     //TBD: add support for this type
     found_id = COOP;
// Strings for input file: these are for coop files:
     in_reportType_str    = "stationType";
     hdr_typ = "MSONET"; //ignore stationType, encode all as "MSONET"
     in_timeObs_str       = "observationTime";
     in_elevation_str     = "elevation";
     in_visibility_str    = "NA";
     in_seaLevelPress_str = "NA";
     in_temperature_str   = "temperature";
     in_dewpoint_str      = "NA";
     in_windDir_str       = "windDir";
     in_windSpeed_str     = "windSpeed";
     in_windGust_str      = "windGust";
     in_minTemp24Hour_str = "minDailyTemp";
     in_maxTemp24Hour_str = "maxDailyTemp";
     in_precip1Hour_str   = "NA";
     in_precip3Hour_str   = "NA";
     in_precip6Hour_str   = "NA";
     in_precip24Hour_str  = "precipAccum";
     in_snowCover_str     = "NA";
   }
   if (strncmp(tmp_str, "MADIS - Hydrological Surface", 28) == 0) {
     //TBD: add support for this type
     found_id = HYDRO;
// Strings for input file: these are for hydro files:
     in_reportType_str    = "stationType";
     hdr_typ = "MSONET"; //ignore stationType, encode all as "MSONET"
     in_timeObs_str       = "observationTime";
     in_elevation_str     = "elevation";
     in_visibility_str    = "NA";
     in_seaLevelPress_str = "NA";
     in_temperature_str   = "NA";
     in_dewpoint_str      = "NA";
     in_windDir_str       = "NA";
     in_windSpeed_str     = "NA";
     in_windGust_str      = "NA";
     in_minTemp24Hour_str = "NA";
     in_maxTemp24Hour_str = "NA";
     in_precip1Hour_str   = "precip1hr";
     in_precip3Hour_str   = "precip3hr";
     in_precip6Hour_str   = "precip6hr";
     in_precip24Hour_str  = "precip24hr";
     in_snowCover_str     = "NA";
   }
   if (strncmp(tmp_str, "MADIS - Meteorological Surface - Maritime", 41) == 0) {
     //TBD: add support for this type
     found_id = MARITIME;
// Strings for input file: these are for mesonet files:
     in_reportType_str    = "stationName";
     hdr_typ = "MSONET"; //ignore stationType, encode all as "MSONET"
     in_timeObs_str       = "timeObs";
     in_elevation_str     = "elevation";
     in_visibility_str    = "visibility";
     in_seaLevelPress_str = "seaLevelPress";
     in_temperature_str   = "temperature";
     in_dewpoint_str      = "dewpoint";
     in_windDir_str       = "windDir";
     in_windSpeed_str     = "windSpeed";
     in_windGust_str      = "windGust";
     in_minTemp24Hour_str = "NA";
     in_maxTemp24Hour_str = "NA";
     in_precip1Hour_str   = "precip1Hour";
     in_precip3Hour_str   = "NA";
     in_precip6Hour_str   = "precip1Hour";
     in_precip24Hour_str  = "precip24Hour";
     in_snowCover_str     = "NA";
   }
   if (strncmp(tmp_str, "MADIS - Meteorological Surface - Integrated Mesonet", 51) == 0) {
     found_id = MESONET;
// Strings for input file: these are for mesonet files:
     in_reportType_str    = "stationType";
     hdr_typ = "MSONET"; //ignore stationType, encode all as "MSONET"
     in_timeObs_str       = "observationTime";
     in_elevation_str     = "elevation";
     in_visibility_str    = "visibility";
     in_seaLevelPress_str = "seaLevelPressure";
     in_temperature_str   = "temperature";
     in_dewpoint_str      = "dewpoint";
     in_windDir_str       = "windDir";
     in_windSpeed_str     = "windSpeed";
     in_windGust_str      = "windGust";
     in_minTemp24Hour_str = "NA";
     in_maxTemp24Hour_str = "NA";
     in_precip1Hour_str   = "NA";
     in_precip3Hour_str   = "NA";
     in_precip6Hour_str   = "NA";
     in_precip24Hour_str  = "NA";
     in_snowCover_str     = "NA";
   }
   if (strncmp(tmp_str, "MADIS - Meteorological Surface - METAR", 38) == 0) {
     found_id = METAR;
     in_reportType_str    = "reportType";
     in_timeObs_str       = "timeObs";
     in_elevation_str     = "elevation";
     in_visibility_str    = "visibility";
     in_seaLevelPress_str = "seaLevelPress";
     in_temperature_str   = "temperature";
     in_dewpoint_str      = "dewpoint";
     in_windDir_str       = "windDir";
     in_windSpeed_str     = "windSpeed";
     in_windGust_str      = "windGust";
     in_minTemp24Hour_str = "minTemp24Hour";
     in_maxTemp24Hour_str = "maxTemp24Hour";
     in_precip1Hour_str   = "precip1Hour";
     in_precip3Hour_str   = "precip3Hour";
     in_precip6Hour_str   = "precip6Hour";
     in_precip24Hour_str  = "precip24Hour";
     in_snowCover_str     = "snowCover"; // surface_snow_amount, in m
   }
   if (strncmp(tmp_str, "MADIS - Meteorological Surface - SAO", 36) == 0) {
     //TBD: add support for this type
     found_id = SAO;
     in_reportType_str    = "reportType";
     in_timeObs_str       = "timeObs";
     in_elevation_str     = "elevation";
     in_visibility_str    = "visibility";
     in_seaLevelPress_str = "seaLevelPress";
     in_temperature_str   = "temperature";
     in_dewpoint_str      = "dewpoint";
     in_windDir_str       = "windDir";
     in_windSpeed_str     = "windSpeed";
     in_windGust_str      = "windGust";
     in_minTemp24Hour_str = "minTemp24Hour";
     in_maxTemp24Hour_str = "maxTemp24Hour";
     in_precip1Hour_str   = "precip1Hour";
     in_precip3Hour_str   = "precip3Hour";
     in_precip6Hour_str   = "precip6Hour";
     in_precip24Hour_str  = "precip24Hour";
     in_snowCover_str     = "NA";
   }
   if (strncmp(tmp_str, "MADIS - Snow", 12) == 0) {
     //TBD: add support for this type
     found_id = SNOW;
     in_reportType_str    = "stationType";
     hdr_typ = "MSONET"; //ignore stationType, encode all as "MSONET"
     in_timeObs_str       = "observationTime";
     in_elevation_str     = "elevation";
     in_visibility_str    = "NA";
     in_seaLevelPress_str = "NA";
     in_temperature_str   = "NA";
     in_dewpoint_str      = "NA";
     in_windDir_str       = "NA";
     in_windSpeed_str     = "NA";
     in_windGust_str      = "NA";
     in_minTemp24Hour_str = "NA";
     in_maxTemp24Hour_str = "NA";
     in_precip1Hour_str   = "NA";
     in_precip3Hour_str   = "NA";
     in_precip6Hour_str   = "NA";
     in_precip24Hour_str  = "NA";
     in_snowCover_str     = "snowDepth"; // surface_snow_amount, in mm
   }
   if(found_id == UNSUPPORTED) {
      cerr << "\n\nERROR: write_data() -> "
           << "Unsupported MADIS \"title\" attribute \"" << tmp_str << "\" from "
           << "NetCDF file: " << awfile << "\n\n" << flush;
      exit(1);
   }
   //
   // Retrieve the record number from the input file.
   //
   if(!(in_recNum_dim = f_in->get_dim(in_recNum_str))) {
      cerr << "\n\nERROR: write_data() -> "
           << "can't read \"" << in_recNum_str << "\" dimension from "
           << "NetCDF file: " << awfile << "\n\n" << flush;
      exit(1);
   }
   nhdr = in_recNum_dim->size();

   if(verbosity > 1) {
      cout << "Processing " << nhdr << " records.\n" << flush;
   }

   /////////////////////////////////////////////////////////////////////

   //
   // Define output NetCDF dimensions
   //
   NcDim *strl_dim    = f_out->add_dim("mxstr", (long) strl_len);
   NcDim *hdr_arr_dim = f_out->add_dim("hdr_arr_len", (long) hdr_arr_len);
   NcDim *obs_arr_dim = f_out->add_dim("obs_arr_len", (long) obs_arr_len);
   NcDim *hdr_dim     = f_out->add_dim("nhdr", (long) nhdr);
   NcDim *obs_dim     = f_out->add_dim("nobs"); // unlimited dimension

   //
   // Define output NetCDF variables
   //
   NcVar *hdr_typ_var = f_out->add_var(out_hdr_typ_str, ncChar, hdr_dim, strl_dim);
   NcVar *hdr_sid_var = f_out->add_var(out_hdr_sid_str, ncChar, hdr_dim, strl_dim);
   NcVar *hdr_vld_var = f_out->add_var(out_hdr_vld_str, ncChar, hdr_dim, strl_dim);
   NcVar *hdr_arr_var = f_out->add_var(out_hdr_arr_str, ncFloat, hdr_dim, hdr_arr_dim);
   NcVar *obs_arr_var = f_out->add_var(out_obs_arr_str, ncFloat, obs_dim, obs_arr_dim);

   //
   // Add attributes to the NetCDF variables
   //
   hdr_typ_var->add_att("long_name", "message type");
   hdr_sid_var->add_att("long_name", "station identification");
   hdr_vld_var->add_att("long_name", "valid time");
   hdr_vld_var->add_att("units", "YYYYMMDD_HHMMSS");

   hdr_arr_var->add_att("long_name", "array of observation station header values");
   hdr_arr_var->add_att("_fill_value", fill_value);
   hdr_arr_var->add_att("columns", "lat lon elv");
   hdr_arr_var->add_att("lat_long_name", "latitude");
   hdr_arr_var->add_att("lat_units", "degrees_north");
   hdr_arr_var->add_att("lon_long_name", "longitude");
   hdr_arr_var->add_att("lon_units", "degrees_east");
   hdr_arr_var->add_att("elv_long_name", "elevation");
   hdr_arr_var->add_att("elv_units", "meters above sea level (msl)");

   obs_arr_var->add_att("long_name", "array of observation values");
   obs_arr_var->add_att("_fill_value", fill_value);
   obs_arr_var->add_att("columns", "hdr_id gc lvl hgt ob");
   obs_arr_var->add_att("hdr_id_long_name", "index of matching header data");
   obs_arr_var->add_att("gc_long_name", "grib code corresponding to the observation type");
   obs_arr_var->add_att("lvl_long_name", "pressure level (hPa) or accumulation interval (sec)");
   obs_arr_var->add_att("hgt_long_name", "height in meters above sea level (msl)");
   obs_arr_var->add_att("ob_long_name", "observation value");

   //
   // Add global attributes
   //
   unix_to_mdyhms(time(NULL), mon, day, yr, hr, min, sec);
   sprintf(tmp_str, "%.4i%.2i%.2i_%.2i%.2i%.2i", yr, mon, day, hr, min, sec);
   gethostname(hostname_str, max_str_len);
   sprintf(attribute_str, "File %s generated %s UTC on host %s by the madis2nc tool",
           ncfile.text(), tmp_str, hostname_str);
   f_out->add_att("FileOrigins", attribute_str);
   f_out->add_att("MET_version", met_version);
   f_out->add_att("MET_tool", program_name);

   /////////////////////////////////////////////////////////////////////

   //
   // Retrieve the input variables
   //
   if(!(in_reportType_var = f_in->get_var(in_reportType_str))) {
      cerr << "\n\nERROR: write_data() -> "
           << "can't read \"" << in_reportType_str << "\" variable from "
           << "NetCDF file: " << awfile << "\n\n" << flush;
      exit(1);
   }

   if(!(in_stationName_var = f_in->get_var(in_stationName_str))) {
      cerr << "\n\nERROR: write_data() -> "
           << "can't read \"" << in_stationName_str << "\" variable from "
           << "NetCDF file: " << awfile << "\n\n" << flush;
      exit(1);
   }

   if(!(in_timeObs_var = f_in->get_var(in_timeObs_str))) {
      cerr << "\n\nERROR: write_data() -> "
           << "can't read \"" << in_timeObs_str << "\" variable from "
           << "NetCDF file: " << awfile << "\n\n" << flush;
      exit(1);
   }

   if(!(in_latitude_var = f_in->get_var(in_latitude_str))) {
      cerr << "\n\nERROR: write_data() -> "
           << "can't read \"" << in_latitude_str << "\" variable from "
           << "NetCDF file: " << awfile << "\n\n" << flush;
      exit(1);
   }

   if(!(in_longitude_var = f_in->get_var(in_longitude_str))) {
      cerr << "\n\nERROR: write_data() -> "
           << "can't read \"" << in_longitude_str << "\" variable from "
           << "NetCDF file: " << awfile << "\n\n" << flush;
      exit(1);
   }

   if(!(in_elevation_var = f_in->get_var(in_elevation_str))) {
      cerr << "\n\nERROR: write_data() -> "
           << "can't read \"" << in_elevation_str << "\" variable from "
           << "NetCDF file: " << awfile << "\n\n" << flush;
      exit(1);
   }

   /////////////////////////////////////////////////////////////////////

   //
   // Process the header data
   //
   for(i_hdr=0; i_hdr<nhdr; i_hdr++) {

      //
      // Mapping of NetCDF variable names from input to output:
      // Output                    = Input
      // hdr_typ                   = reportType(maxRepLen = 6)
      // hdr_sid                   = stationName(maxStaNamLen = 5)
      // hdr_vld (YYYYMMDD_HHMMSS) = timeObs (unixtime)
      // hdr_arr[0](Lat)           = latitude
      // hdr_arr[1](Lon)           = longitude
      // hdr_arr[2](Elv)           = elevation
      //

      //
      // Retrieve Header Type
      //
     int rep_len=6;
     if (found_id == SAO) rep_len=5;
      if(!in_reportType_var->set_cur((long) i_hdr)
      || !in_reportType_var->get(&tmp_str[0], 1, rep_len)) {
         cerr << "\n\nERROR: write_data() -> "
              << "can't read \"" << in_reportType_str
              << "\" data for record number "
              << i_hdr << "\n\n" << flush;
         exit(1);
      }

      if (found_id == METAR) {
        // Use ADPDFC if it matches METAR string, otherwise use value from file
        if(strncmp(tmp_str, "METAR", 6) == 0 ||
           strncmp(tmp_str, "SPECI", 6) == 0) hdr_typ = "ADPSFC";
        else                                  hdr_typ = tmp_str;
      }
      // TBD: add handling for other types (right now use constant value set above)

      //
      // Store Header Type
      //
      if(!hdr_typ_var->set_cur(i_hdr, (long) 0) ||
         !hdr_typ_var->put(hdr_typ, (long) 1, (long) hdr_typ.length())) {
         cerr << "\n\nERROR: write_data() -> "
              << "can't write \"" << out_hdr_typ_str
              << "\" data for record number "
              << i_hdr << "\n\n" << flush;
         exit(1);
      }

      //
      // Retrieve Station ID
      //
      if(!in_stationName_var->set_cur((long) i_hdr)
      || !in_stationName_var->get(&tmp_str[0], 1, 5)) {
         cerr << "\n\nERROR: write_data() -> "
              << "can't read \"" << in_stationName_str
              << "\" data for record number "
              << i_hdr << "\n\n" << flush;
         exit(1);
      }
      hdr_sid = tmp_str;

      //
      // Store Station ID
      //
      if(!hdr_sid_var->set_cur(i_hdr, (long) 0) ||
         !hdr_sid_var->put(hdr_sid, (long) 1, (long) hdr_sid.length())) {
         cerr << "\n\nERROR: write_data() -> "
              << "can't write \"" << out_hdr_sid_str
              << "\" data for record number "
              << i_hdr << "\n\n" << flush;
         exit(1);
      }

      //
      // Retrieve Valid Time
      //
      if(!in_timeObs_var->set_cur((long) i_hdr) ||
         !in_timeObs_var->get(&ut, 1)) {
         cerr << "\n\nERROR: write_data() -> "
              << "can't read \"" << in_timeObs_str
              << "\" data for record number "
              << i_hdr << "\n\n" << flush;
         exit(1);
      }
      unix_to_yyyymmdd_hhmmss((unixtime) ut, tmp_str);
      hdr_vld = tmp_str;

      //
      // Store Valid Time
      //
      if(!hdr_vld_var->set_cur(i_hdr, (long) 0) ||
         !hdr_vld_var->put(hdr_vld, (long) 1, (long) hdr_vld.length())) {
         cerr << "\n\nERROR: write_data() -> "
              << "can't write \"" << out_hdr_vld_str
              << "\" data for record number "
              << i_hdr << "\n\n" << flush;
         exit(1);
      }

      //
      // Retrieve Latitude
      //
      if(!in_latitude_var->set_cur((long) i_hdr) ||
         !in_latitude_var->get(&hdr_arr[0], 1)) {
         cerr << "\n\nERROR: write_data() -> "
              << "can't read \"" << in_latitude_str
              << "\" data for record number "
              << i_hdr << "\n\n" << flush;
         exit(1);
      }

      //
      // Retrieve Longitude
      //
      if(!in_longitude_var->set_cur((long) i_hdr) ||
         !in_longitude_var->get(&hdr_arr[1], 1)) {
         cerr << "\n\nERROR: write_data() -> "
              << "can't read \"" << in_longitude_str
              << "\" data for record number "
              << i_hdr << "\n\n" << flush;
         exit(1);
      }

      //
      // Retrieve Elevation
      //
      if(!in_elevation_var->set_cur((long) i_hdr) ||
         !in_elevation_var->get(&hdr_arr[2], 1)) {
         cerr << "\n\nERROR: write_data() -> "
              << "can't read \"" << in_elevation_str
              << "\" data for record number "
              << i_hdr << "\n\n" << flush;
         exit(1);
      }

      //
      // Store Header Array
      //
      if(!hdr_arr_var->set_cur(i_hdr, (long) 0) ||
         !hdr_arr_var->put(hdr_arr, (long) 1, (long) hdr_arr_len)) {
         cerr << "\n\nERROR: write_data() -> "
              << "can't write \"" << out_hdr_arr_str
              << "\" data for record number "
              << i_hdr << "\n\n" << flush;
         exit(1);
      }
   } // end for

   //
   // Retrieve and store each of the data values
   //
   i_obs = 0;
   float conversion=1.0;

   // Sea Level Pressure
   write_obs_arr(f_in, nhdr, in_seaLevelPress_str, in_seaLevelPress_gc,
                 bad_data_float, obs_arr_var, i_obs, conversion);

   // Visibility
   write_obs_arr(f_in, nhdr, in_visibility_str, in_visibility_gc,
                 bad_data_float, obs_arr_var, i_obs, conversion);

   // Temperature
   write_obs_arr(f_in, nhdr, in_temperature_str, in_temperature_gc,
                 bad_data_float, obs_arr_var, i_obs, conversion);

   // Dewpoint
   write_obs_arr(f_in, nhdr, in_dewpoint_str, in_dewpoint_gc,
                 bad_data_float, obs_arr_var, i_obs, conversion);

   // Wind Direction
   write_obs_arr(f_in, nhdr, in_windDir_str, in_windDir_gc,
                 bad_data_float, obs_arr_var, i_obs, conversion);

   // Wind Speed
   write_obs_arr(f_in, nhdr, in_windSpeed_str, in_windSpeed_gc,
                 bad_data_float, obs_arr_var, i_obs, conversion);

   // Wind Gust
   write_obs_arr(f_in, nhdr, in_windGust_str, in_windGust_gc,
                 bad_data_float, obs_arr_var, i_obs, conversion);

   // Min Temperature - 24 Hour
   write_obs_arr(f_in, nhdr, in_minTemp24Hour_str, in_minTemp24Hour_gc,
                 bad_data_float, obs_arr_var, i_obs, conversion);

   // Max Temperature - 24 Hour
   write_obs_arr(f_in, nhdr, in_maxTemp24Hour_str, in_maxTemp24Hour_gc,
                 bad_data_float, obs_arr_var, i_obs, conversion);

   conversion = 1000.0;
   // Precipitation - 1 Hour
   write_obs_arr(f_in, nhdr, in_precip1Hour_str, in_precip_gc,
                 1.0*sec_per_hour, obs_arr_var, i_obs, conversion);

   // Precipitation - 3 Hour
   write_obs_arr(f_in, nhdr, in_precip3Hour_str, in_precip_gc,
                 3.0*sec_per_hour, obs_arr_var, i_obs, conversion);

   // Precipitation - 6 Hour
   write_obs_arr(f_in, nhdr, in_precip6Hour_str, in_precip_gc,
                 6.0*sec_per_hour, obs_arr_var, i_obs, conversion);

   // Precipitation - 24 Hour
   write_obs_arr(f_in, nhdr, in_precip24Hour_str, in_precip_gc,
                 24.0*sec_per_hour, obs_arr_var, i_obs, conversion);

   // Snow Cover
   conversion = 1.0;
   if (found_id == SNOW) conversion=1./1000.;
   write_obs_arr(f_in, nhdr, in_snowCover_str, in_snowCover_gc,
                 bad_data_float, obs_arr_var, i_obs, conversion);

   if(verbosity > 1) {
      cout << "Finished processing " << i_obs << " observations from "
           << nhdr << " records.\n" << flush;
   }

   //
   // Close the output NetCDF file
   //
   f_out->close();
   delete f_out;
   f_out = (NcFile *) 0;

   //
   // Close the input NetCDF file
   //
   f_in->close();
   delete f_in;
   f_in = (NcFile *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_obs_arr(NcFile *&f_in, const int nhdr,
                   const char *in_str, const int in_gc, const float in_lvl,
                   NcVar *&obs_arr_var, int &i_obs, const float conversion) {
   int i_hdr;
   NcVar *in_var   = (NcVar *) 0;
   NcVar *in_varqca   = (NcVar *) 0;
   NcVar *in_varqcr   = (NcVar *) 0;
   NcAtt *fill_att = (NcAtt *) 0;
   float obs_arr[obs_arr_len], in_fill_value;
   int qca, qcr, qcfail=0,i_fill=0,i_write=0;

   if (strncmp(in_str,"NA",2) == 0)
     return;

   //
   // Retrieve the input variable and qca and qcr variables
   //
   if(!(in_var = f_in->get_var(in_str))) {
      cerr << "\n\nERROR: write_obs_arr() -> "
           << "can't read \"" << in_str << "\" variable.\n\n" << flush;
      exit(1);
   }
   char *qca_str=new char[strlen((const char*)in_str)+3];
   strcpy(qca_str,(const char*)in_str);
   strcat(qca_str,"QCA");
   if(!(in_varqca = f_in->get_var(qca_str))) {
      cerr << "\n\nERROR: write_obs_arr() -> "
           << "can't read \"" << qca_str << "\" variable.\n\n" << flush;
      exit(1);
   }
   char *qcr_str=new char[strlen((const char*)in_str)+3];
   strcpy(qcr_str,(const char*)in_str);
   strcat(qcr_str,"QCR");
   if(!(in_varqcr = f_in->get_var(qcr_str))) {
      cerr << "\n\nERROR: write_obs_arr() -> "
           << "can't read \"" << qcr_str << "\" variable.\n\n" << flush;
      exit(1);
   }

   //
   // Retrieve the fill value
   //
   fill_att = in_var->get_att(in_fillValue_str);
   if(!fill_att || !fill_att->is_valid()) {
      cerr << "\n\nERROR: write_obs_arr() -> "
           << "can't get the \"" << in_fillValue_str
           << "\" attribute for variable \"" << in_str
           << "\"\n\n" << flush;
      exit(1);
   }
   in_fill_value = fill_att->as_float(0);

   // Store the GRIB code and height
   obs_arr[1] = in_gc;
   obs_arr[2] = in_lvl;
   obs_arr[3] = bad_data_float;

   //
   // Process each observation value
   //
   for(i_hdr=0; i_hdr<nhdr; i_hdr++) {

      // Store the header id
      obs_arr[0] = i_hdr;

      //
      // Retrieve the values
      //
      if(!in_var->set_cur((long) i_hdr) ||
         !in_var->get(&obs_arr[4], 1)) {
         cerr << "\n\nERROR: write_obs_arr() -> "
              << "can't read \"" << in_str
              << "\" data for record number "
              << i_hdr << "\n\n" << flush;
         exit(1);
      }
      if(!in_varqca->set_cur((long) i_hdr) ||
         !in_varqca->get(&qca, 1)) {
         cerr << "\n\nERROR: write_obs_arr() -> "
              << "can't read \"" << qca_str
              << "\" data for record number "
              << i_hdr << "\n\n" << flush;
         exit(1);
      }
      if(!in_varqcr->set_cur((long) i_hdr) ||
         !in_varqcr->get(&qcr, 1)) {
         cerr << "\n\nERROR: write_obs_arr() -> "
              << "can't read \"" << qcr_str
              << "\" data for record number "
              << i_hdr << "\n\n" << flush;
         exit(1);
      }

      //
      // Continue if data is missing
      //
      if(obs_arr[4] == in_fill_value) continue;
      if (qca == 0 || qcr != 0) {
        qcfail++;
        obs_arr[4]=fill_value; //missing-fill obs with failed qc
      }
      if(obs_arr[4] == fill_value) continue; // also skip bad data

      //
      // Apply conversion factor, if any:
      //
      obs_arr[4] *= conversion;

      //
      // Store values
      //
      if(!obs_arr_var->set_cur(i_obs, (long) 0) ||
         !obs_arr_var->put(obs_arr, (long) 1, (long) obs_arr_len)) {
         cerr << "\n\nERROR: write_obs_arr() -> "
              << "can't write \"" << out_obs_arr_str
              << "\" data for variable \"" << in_str
              << "\" and record number " << i_hdr
              << ".\n\n" << flush;
         exit(1);
      }

      // Increment the observation count
      i_obs++;
      i_write++;
   }
   i_fill=nhdr-qcfail-i_write;
   if (verbosity > 1) {
     cout << "For " << in_str << " processed " << nhdr << " records, "
          << qcfail << " failed qc, " << i_fill << " were missing-filled, "
          << i_write << " were written out\n" << flush;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void usage(int argc, char *argv[]) {

   cout << "\nUsage: "
        << program_name << "\n"
        << "\tmadis_file\n"
        << "\tout_file\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"madis_file\" is the MADIS NetCDF point "
        << "observation file (required).\n"

        << "\t\t\"out_file\" is the output NetCDF file (required).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << verbosity << ") (optional: 0 - none; 1 - minimal; 2 - summary).\n\n"

        << flush;

   return;
}

////////////////////////////////////////////////////////////////////////
