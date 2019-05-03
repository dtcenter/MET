// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   pb2nc.cc
//
//   Description:
//      Based on user specified options, this tool filters point
//      observations from a PrepBufr input file, derives requested
//      observation types, and writes the output to a NetCDF file.
//      PrepBufr observations must be reformatted in this way prior to
//      using them in the Point-Stat tool.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    03-27-07  Halley Gotway  New
//   001    12-19-07  Halley Gotway  Add support for ANYAIR, ANYSFC,
//                    and ONLYSF message types.
//   002    12-20-07  Halley Gotway  Derive PRMSL.
//   003    01-31-08  Halley Gotway  Remove unused array elements from
//                    the hdr_arr and obs_arr and allow it to run on
//                    multiple PrepBufr files at once.
//   004    08-22-08  Halley Gotway  Change the arguments to dumppb
//                    function for it to work with Fortran90/95
//                    compilers.
//   005    02-12-09  Halley Gotway  Fix npbmsg bug when reading
//                    multiple PrepBufr files and use of valid_beg and
//                    valid_end options.
//   006    06-01-10  Halley Gotway  Pass flags to dumppb to only dump
//                    observation for reqeusted message types.
//   007    02-15-11  Halley Gotway  Fix event_stack bug for which the
//                    logic was reversed.
//   008    02-28-11  Halley Gotway  Modify relative humidity derivation
//                    to match the Unified-PostProcessor.
//   009    06-01-11  Halley Gotway  Call closebp for input file
//                    descriptor.
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
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "netcdf.hh"

#include "pb2nc_Conf.h"

#include "vx_wrfdata/vx_wrfdata.h"
#include "vx_data_grids/grid.h"
#include "vx_util/vx_util.h"
#include "vx_pb_util/vx_pb_util.h"
#include "vx_met_util/vx_met_util.h"
#include "vx_analysis_util/mask_poly.h"
#include "vx_cal/vx_cal.h"
#include "vx_math/vx_math.h"

////////////////////////////////////////////////////////////////////////

//
// Constants
//

static const char *program_name = "pb2nc";

static const float fill_value   = -9999.f;

// Constants used to interface to Fortran subroutines

// Missing value for BUFR data
static const double r8bfms      = 1.0E10;
// Maximum number of BUFR parameters
static const int mxr8pm         = 10;
// Maximum number of BUFR levels
static const int mxr8lv         = 255;
// Maximum number of BUFR event sequences
static const int mxr8vn         = 10;
// Maximum number of BUFR variable types
static const int mxr8vt         = 6;

// Length of the "YYYYMMDD_HHMMSS" string
static const int strl_len       = 16;
// Observation header length
static const int hdr_arr_len    = 3;
// Observation values length
static const int obs_arr_len    = 5;

// File unit number for opening the PrepBufr file
static int file_unit            = 11;
// 2nd file unit number for opening the PrepBufr file
static int dump_unit            = 22;
static int msg_typ_ret[n_vld_msg_typ];

// Grib codes corresponding to the variable types
static const int var_gc[mxr8vt] = {
   pres_grib_code, spfh_grib_code, tmp_grib_code,
   hgt_grib_code,  ugrd_grib_code, vgrd_grib_code
};

// Number of variable types which may be derived
static const int n_derive_gc = 6;

// Listing of grib codes for variable types which may be derived
static const int derive_gc[n_derive_gc] = {
   dpt_grib_code,  wdir_grib_code,
   wind_grib_code, rh_grib_code,
   mixr_grib_code, prmsl_grib_code
};

////////////////////////////////////////////////////////////////////////

//
// Variables for command line arguments
//

// StringArray to store PrepBufr file name
static StringArray pbfile;

// Output NetCDF file name
static ConcatString ncfile;

// Input configuration file
static ConcatString config_file;

// PB2NCConfig settings
static pb2nc_Conf  conf;

// Beginning and ending retention times
static unixtime valid_beg_ut, valid_end_ut;

// Logging level
static int verbosity = 1;

// Number of PrepBufr messages to process from the command line
static int nmsg      = -1;

// Dump contents of PrepBufr file to ASCII files
static int dump_flag = 0;
static ConcatString dump_dir = ".";

////////////////////////////////////////////////////////////////////////

//
// Variables for configuration file selections
//

// Masking grid and polyline
static Grid        grid_mask;
static MaskPoly    poly_mask;

// Number of grib codes corresponding to observation types
// to be retained and/or derived
static int         n_obs_gc;
static int        *obs_gc = (int *) 0;

// Flags to indicate whether ANYAIR, ANYSFC, or ONLYSF have been
// provided in the config file
static int         anyair_flag = 0;
static int         anysfc_flag = 0;
static int         onlysf_flag = 0;

// Flags to indicate what type of masking to apply
static int         apply_mask_grid = 0;
static int         apply_mask_poly = 0;

// Shared memory defined in the PREPBC common block in readpb.prm
static double      hdr[mxr8pm];
static double      evns[mxr8vt][mxr8vn][mxr8lv][mxr8pm];
static int         nlev;

////////////////////////////////////////////////////////////////////////

//
// Variables to store header data elements
//
static StringArray hdr_typ_sa;     // Message type
static StringArray hdr_sid_sa;     // Station ID
static StringArray hdr_vld_sa;     // Valid time
static NumArray    hdr_arr_lat_na; // Latitude
static NumArray    hdr_arr_lon_na; // Longitude
static NumArray    hdr_arr_elv_na; // Elevation
static int         n_total_obs;    // Running total of observations

//
// Output NetCDF file, dimensions, and variables
//
static NcFile *f_out      = (NcFile *) 0;

static NcDim *strl_dim    = (NcDim *)  0; // Maximum string length
static NcDim *hdr_arr_dim = (NcDim *)  0; // Header array width
static NcDim *obs_arr_dim = (NcDim *)  0; // Observation array width
static NcDim *hdr_dim     = (NcDim *)  0; // Header array length
static NcDim *obs_dim     = (NcDim *)  0; // Observation array length

static NcVar *hdr_typ_var = (NcVar *)  0; // Message type
static NcVar *hdr_sid_var = (NcVar *)  0; // Station ID
static NcVar *hdr_vld_var = (NcVar *)  0; // Valid time
static NcVar *hdr_arr_var = (NcVar *)  0; // Header array
static NcVar *obs_arr_var = (NcVar *)  0; // Observation array

////////////////////////////////////////////////////////////////////////

extern "C" {
   void numpbmsg_(int *, int *);
   void openpb_(const char *, int *);
   void closepb_(int *);
   void readpb_(int *, char *, int *, int *, int *, double[mxr8pm],
                double[mxr8vt][mxr8vn][mxr8lv][mxr8pm]);
   void dumppb_(const char *, int *, const char *, int *,
                const char *, int *, int *);
}

////////////////////////////////////////////////////////////////////////

static void   initialize();
static void   process_command_line(int, char **);
static void   process_config();
static void   open_netcdf();
static void   process_pbfile(int);
static void   write_netcdf_hdr_data();
static void   clean_up();

static int    get_event_index(int, int, int);
static void   dbl2str(double *, char *);

static int    keep_message_type(const char *);
static int    keep_station_id(const char *);
static int    keep_valid_time(const unixtime, const unixtime,
                              const unixtime);
static int    keep_pb_report_type(int);
static int    keep_in_report_type(int);
static int    keep_instrument_type(int);
static int    keep_obs_grib_code(int);
static int    keep_level_category(int);

static float  derive_grib_code(int, float *, double);

static void   usage(int, char **);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   int i;

   //
   // Set handler to be called for memory allocation error
   //
   set_new_handler(oom);

   //
   // Initialize static variables
   //
   initialize();

   //
   // Process the command line arguments
   //
   process_command_line(argc, argv);

   //
   // Open the NetCDF file
   //
   open_netcdf();

   //
   // Process each PrepBufr file
   //
   for(i=0; i<pbfile.n_elements(); i++) {
      process_pbfile(i);
   }

   //
   // Write the NetCDF file
   //
   write_netcdf_hdr_data();

   //
   // Deallocate memory and clean up
   //
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void initialize() {

   n_total_obs = 0;
   hdr_typ_sa.clear();
   hdr_sid_sa.clear();
   hdr_vld_sa.clear();
   hdr_arr_lat_na.clear();
   hdr_arr_lon_na.clear();
   hdr_arr_elv_na.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   int i;
   char tmp_str[max_str_len], tmp2_str[max_str_len];

   //
   // Check for the minimum number of arguments
   //
   if(argc < 4) {
      usage(argc, argv);
      exit(1);
   }

   //
   // Initialize retention times
   //
   valid_beg_ut = valid_end_ut = (unixtime) 0;

   //
   // Store the input PrepBufr filename, the output
   // netCDF file name, and the config file name
   //
   pbfile.add(argv[1]);
   ncfile      = argv[2];
   config_file = argv[3];

   //
   // Parse command line arguments
   //
   for(i=0; i<argc; i++) {

      if(strcmp(argv[i], "-pbfile") == 0) {
         pbfile.add(argv[i+1]);
         i++;
      }
      else if(strcmp(argv[i], "-valid_beg") == 0) {
         valid_beg_ut = timestring_to_unix(argv[i+1]);
         i++;
      }
      else if(strcmp(argv[i], "-valid_end") == 0) {
         valid_end_ut = timestring_to_unix(argv[i+1]);
         i++;
      }
      else if(strcmp(argv[i], "-v") == 0) {
         verbosity = atoi(argv[i+1]);
         i++;
      }
      else if(strcmp(argv[i], "-nmsg") == 0) {
         nmsg = atoi(argv[i+1]);
         i++;
      }
      else if(strcmp(argv[i], "-dump") == 0) {
         dump_flag = 1;
         dump_dir = argv[i+1];
         i++;
      }
      else if(argv[i][0] == '-') {
         cerr << "\n\nERROR: process_command_line() -> "
              << "unrecognized command line switch: "
              << argv[i] << "\n\n" << flush;
         exit(1);
      }
   }

   //
   // Parse the PB2NC config file
   //
   if(verbosity > 0) {
      cout << "Reading Config File:\t" << config_file
           << "\n" << flush;
   }
   conf.read(config_file);

   //
   // Check that the end_ut >= beg_ut
   //
   if(valid_beg_ut != (unixtime) 0 &&
      valid_end_ut != (unixtime) 0 &&
      valid_beg_ut > valid_end_ut) {

      unix_to_yyyymmdd_hhmmss(valid_beg_ut, tmp_str);
      unix_to_yyyymmdd_hhmmss(valid_end_ut, tmp2_str);

      cerr << "\n\nERROR: process_command_line() -> "
           << "the ending time (" << tmp2_str
           << ") must be greater than the beginning time ("
           << tmp_str << ").\n\n" << flush;
      exit(1);
   }

   //
   // Do error checking on the configuration values and parse
   // any masking grids or polylines specified.
   //
   process_config();

   return;
}

////////////////////////////////////////////////////////////////////////

void process_config() {
   int i, n;
   char tmp_str[PATH_MAX];

   //
   // Conf: version
   //

   if(strncasecmp(conf.version().sval(), met_version,
      strlen(conf.version().sval())) != 0) {

      cerr << "\n\nERROR: process_config() -> "
           << "The version number listed in the config file ("
           << conf.version().sval() << ") does not match the version "
           << "of the code (" << met_version << ").\n\n" << flush;
      exit(1);
   }

   //
   // Check whether ANYAIR, ANYSFC, or ONLYSF have been specified
   // in the config file
   //
   anyair_flag = anysfc_flag = onlysf_flag = 0;
   if(keep_message_type(anyair_str)) anyair_flag = 1;
   else                              anyair_flag = 0;
   if(keep_message_type(anysfc_str)) anysfc_flag = 1;
   else                              anysfc_flag = 0;
   if(keep_message_type(onlysf_str)) onlysf_flag = 1;
   else                              onlysf_flag = 0;

   //
   // Check that the end_ds >= beg_ds
   //
   if(conf.beg_ds().ival() > conf.end_ds().ival()) {
      cerr << "\n\nERROR: process_config_file() -> "
           << "the ending time offset (end_ds) must be greater "
           << "than the starting time offset (beg_ds).\n\n"
           << flush;
      exit(1);
   }

   //
   // Check that the masking grid requested is a valid grid
   //
   if(strlen(conf.mask_grid().sval()) != 0) {

      apply_mask_grid = 1;

      if(!find_grid_by_name(conf.mask_grid().sval(), grid_mask)) {
         cerr << "\n\nERROR: process_config_file() -> "
              << "the mask_grid requested \""
              << conf.mask_grid().sval()
              << "\" is not defined.\n\n" << flush;
         exit(1);
      }
   }
   else {
      apply_mask_grid = 0;
   }

   //
   // Replace any instances of MET_BASE in mask_poly with it's
   // expanded value
   //
   replace_string(met_base_str, MET_BASE,
                  conf.mask_poly().sval(), tmp_str);

   //
   // Process the mask_poly string as a file.
   //
   if(strlen(conf.mask_poly().sval()) != 0) {

      apply_mask_poly = 1;

      poly_mask.load(tmp_str);
   }
   else {
      apply_mask_poly = 0;
   }

   //
   // Check that the end_elev >= beg_elev
   //
   if(conf.end_elev().dval() < conf.beg_elev().dval()) {
      cerr << "\n\nERROR: process_config_file() -> "
           << "the ending elevation (end_elev) cannot be set "
           << "less than the starting elevation (beg_elev).\n\n"
           << flush;
      exit(1);
   }

   //
   // Check that the end_level >= beg_level
   //
   if(conf.end_level().dval() < conf.beg_level().dval()) {
      cerr << "\n\nERROR: process_config_file() -> "
           << "the ending vertical level (end_level) cannot be set "
           << "less than the starting vertical level(beg_level).\n\n"
           << flush;
      exit(1);
   }

   //
   // Check that the pb_report_type values between 100 and 300
   //
   n = conf.n_pb_report_type_elements();
   for(i=0; i<n; i++) {

      if(conf.pb_report_type(i).ival() < 100 ||
         conf.pb_report_type(i).ival() > 300) {
            cerr << "\n\nERROR: process_config_file() -> "
                 << "the pb_report_type values must be in the valid "
                 << "range of 100 through 300.\n\n" << flush;
            exit(1);
      }
   }

   //
   // Parse the observation grib codes and abbreviations into an
   // array of requested grib codes
   //
   n_obs_gc = conf.n_obs_grib_code_elements();
   if(n_obs_gc == 0) {
      cerr << "\n\nERROR: process_config() -> "
           << "At least grib code or grib code abbreviation must be "
           << "provided for obs_grib_code.\n\n" << flush;
      exit(1);
   }
   else {
      obs_gc = new int [n_obs_gc];

      for(i=0; i<n_obs_gc; i++) {
         obs_gc[i] = str_to_grib_code(conf.obs_grib_code(i).sval());
      }
   }

   //
   // Check that the quality_mark_thresh is in the valid range
   //
   if(conf.quality_mark_thresh().ival() < 0 ||
      conf.quality_mark_thresh().ival() > 15) {
      cerr << "\n\nERROR: process_config_file() -> "
           << "the quality_mark_thresh must be in the valid range "
           << "of 0 through 15.\n\n" << flush;
      exit(1);
   }

   //
   // Check to make sure that the level categories are in the valid
   // range
   //
   n = conf.n_level_category_elements();
   for(i=0; i<n; i++) {

      if(conf.level_category(i).ival() < 0 ||
         conf.level_category(i).ival() > 7) {
            cerr << "\n\nERROR: process_config_file() -> "
                 << "the level_category values must be in the valid "
                 << "range of 0 through 7.\n\n" << flush;
            exit(1);
      }
   }

   //
   // Check to make sure that the temporary directory exists
   //
   if(opendir(conf.tmp_dir().sval()) == NULL ) {
      cerr << "\n\nERROR: process_config_file() -> "
           << "Cannot access the tmp_dir temporary directory: "
           << conf.tmp_dir().sval() << "\n\n" << flush;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void open_netcdf() {

   //
   // Create the output netCDF file for writing
   //
   if(verbosity > 0) {
      cout << "Creating NetCDF File:\t" << ncfile << "\n" << flush;
   }
   f_out = new NcFile(ncfile, NcFile::Replace);

   //
   // Check for a valid file
   //
   if(!f_out->is_valid()) {
      cerr << "\n\nERROR: open_netcdf() -> "
           << "trouble opening output file: " << ncfile << "\n\n";

      f_out->close();
      delete f_out;
      f_out = (NcFile *) 0;

      exit(1);
   }

   //
   // Define netCDF dimensions
   //
   strl_dim    = f_out->add_dim("mxstr", (long) strl_len);
   hdr_arr_dim = f_out->add_dim("hdr_arr_len", (long) hdr_arr_len);
   obs_arr_dim = f_out->add_dim("obs_arr_len", (long) obs_arr_len);
   obs_dim     = f_out->add_dim("nobs"); // unlimited dimension

   //
   // Define netCDF variables
   //
   obs_arr_var = f_out->add_var("obs_arr", ncFloat, obs_dim,
                                obs_arr_dim);

   //
   // Add global attributes
   //
   write_netcdf_global(f_out, ncfile.text(), program_name);

   //
   // Add variable attributes
   //
   obs_arr_var->add_att("long_name", "array of observation values");
   obs_arr_var->add_att("_fill_value", fill_value);
   obs_arr_var->add_att("columns", "hdr_id gc lvl hgt ob");
   obs_arr_var->add_att("hdr_id_long_name",
                        "index of matching header data");
   obs_arr_var->add_att("gc_long_name",
      "grib code corresponding to the observation type");
   obs_arr_var->add_att("lvl_long_name",
      "pressure level (hPa) or accumulation interval (sec)");
   obs_arr_var->add_att("hgt_long_name",
                        "height in meters above sea level (msl)");
   obs_arr_var->add_att("ob_long_name", "observation value");

   return;
}

////////////////////////////////////////////////////////////////////////

void process_pbfile(int i_pb) {
   int npbmsg, unit, yr, mon, day, hr, min, sec;
   int i, i_msg, i_read, n_file_obs, i_ret, i_date, n_hdr_obs;
   int rej_typ, rej_sid, rej_vld, rej_grid, rej_poly;
   int rej_elv, rej_pb_rpt, rej_in_rpt, rej_itp, rej_nobs;
   int lv, ev, kk, len1, len2;

   double   x, y;

   unixtime file_ut = (unixtime) 0;
   unixtime msg_ut, beg_ut, end_ut;

   ConcatString file_name, blk_prefix, blk_file;
   char     prefix[max_str_len];
   char     time_str[max_str_len], time_str2[max_str_len];

   char     hdr_typ[max_str_len], hdr_sid[max_str_len];
   double   hdr_arr_lat, hdr_arr_lon, hdr_arr_elv;
   float    pb_report_type, in_report_type, instrument_type;
   unixtime hdr_vld_ut;

   float    quality_mark, dl_category;
   float    obs_arr[obs_arr_len];
   float    pqtzuv[mxr8vt];

   //
   // List the PrepBufr file being processed
   //
   if(verbosity > 0) {
      cout << "Processing PrepBufr File:\t" << pbfile[i_pb]
           << "\n" << flush;
   }

   //
   // Set the file name for the PrepBufr file
   //
   file_name << pbfile[i_pb];

   //
   // Build the temporary block file name
   //
   blk_prefix << conf.tmp_dir().sval() << "/" << "tmp_pb2nc_blk";
   blk_file = make_temp_file_name(blk_prefix, '\0');

   if(verbosity > 1) {
      cout << "Blocking PrepBufr file to:\t" << blk_file
           << "\n" << flush;
   }

   //
   // Assume that the input PrepBufr file is unblocked.
   // Block the PrepBufr file and open it for reading.
   //
   pblock(file_name, blk_file, block);

   //
   // If dump_flag is specified, dump the contents of the PrepBufr
   // file ASCII files
   //
   if(dump_flag) {

      if(verbosity > 0) {
         cout << "Dumping to ASCII output directory:\t"
              << dump_dir << "\n" << flush;
      }

      //
      // Check whether we're trying to process more than one PrepBufr
      //  file
      //
      if(pbfile.n_elements() > 1) {
         cerr << "\n\nERROR: process_pbfile() -> "
              << "the \"-dump\" and \"-pbfile\" options may not be "
              << "used together.  Only one PrepBufr file may be dump "
              << "to ASCII at a time.\n\n" << flush;
         exit(1);
      }

      for(i=0; i<n_vld_msg_typ; i++) {
         msg_typ_ret[i] = keep_message_type(vld_msg_typ_list[i]);
      }

      unit = dump_unit+i_pb;
      strcpy(prefix, get_short_name(pbfile[i_pb]));
      len1 = strlen(dump_dir);
      len2 = strlen(prefix);
      dumppb_(blk_file, &unit, dump_dir.text(), &len1,
              prefix, &len2, msg_typ_ret);
   }

   //
   // Open the blocked temp PrepBufr file for reading
   //
   unit = file_unit + i_pb;
   openpb_(blk_file, &unit);

   //
   // Compute the number of PrepBufr records in the current file.
   //
   numpbmsg_(&unit, &npbmsg);

   //
   // Use the number of records requested by the user if there
   // are enough present.
   //
   if(nmsg >= 0 && nmsg <= npbmsg) npbmsg = nmsg;

   //
   // Check for zero messages to process
   //
   if(npbmsg <= 0) {
      cout << "***WARNING*** process_pbfile() -> "
           << "No PrepBufr messages to process in file: "
           << pbfile[i_pb] << "\n" << flush;

      //
      // Delete the temporary blocked file
      //
      remove_temp_file(blk_file);

      return;
   }

   //
   // Initialize counts
   //
   i_ret   = n_file_obs = i_msg      = 0;
   rej_typ = rej_sid    = rej_vld    = rej_grid = rej_poly = 0;
   rej_elv = rej_pb_rpt = rej_in_rpt = rej_itp  = rej_nobs = 0;

   //
   // Loop through the PrepBufr messages from the input file
   //
   for(i_read=0; i_read<npbmsg && i_ret == 0; i_read++) {

      if(verbosity > 1
      && nint(npbmsg/20.0) > 0
      && (i_read+1)%nint(npbmsg/20.0) == 0) {
         cout << nint((double) (i_read+1)/npbmsg*100.0) << "% "
              << flush;
      }

      //
      // Get the next PrepBufr message
      //
      readpb_(&unit, hdr_typ, &i_date, &i_ret, &nlev, hdr, evns);

      sprintf(time_str, "%.10i", i_date);
      msg_ut = yyyymmddhh_to_unix(time_str);

      //
      // Check to make sure that the message time hasn't changed
      // from one PrepBufr message to the next
      //
      if(file_ut == (unixtime) 0) {
         file_ut = msg_ut;

         if(verbosity > 1) {
            unix_to_mdyhms(file_ut, mon, day, yr, hr, min, sec);
            sprintf(time_str, "%.4i%.2i%.2i_%.2i%.2i%.2i",
                    yr, mon, day, hr, min, sec);
            cout << "PrepBufr Time Center:\t" << time_str
                 << "\n" << flush;
         }

         //
         // Check if valid_beg_ut and valid_end_ut were set on the
         // command line.  If so, use them.  If not, use beg_ds and
         // end_ds.
         //
         if(valid_beg_ut != (unixtime) 0 ||
            valid_end_ut != (unixtime) 0) {
            beg_ut = valid_beg_ut;
            end_ut = valid_end_ut;
         }
         else {
            beg_ut = file_ut + conf.beg_ds().ival();
            end_ut = file_ut + conf.end_ds().ival();
         }

         if(verbosity > 1) {

            if(beg_ut != (unixtime) 0) {
               unix_to_mdyhms(beg_ut, mon, day, yr, hr, min, sec);
               sprintf(time_str, "%.4i%.2i%.2i_%.2i%.2i%.2i",
                       yr, mon, day, hr, min, sec);
            }
            else {
               strcpy(time_str, "NO_BEG_TIME");
            }

            if(end_ut != (unixtime) 0) {
               unix_to_mdyhms(end_ut, mon, day, yr, hr, min, sec);
               sprintf(time_str2, "%.4i%.2i%.2i_%.2i%.2i%.2i",
                       yr, mon, day, hr, min, sec);
            }
            else {
               strcpy(time_str2, "NO_END_TIME");
            }

            cout << "Searching Time Window:\t" << time_str << " to "
                 << time_str2 << "\n" << flush;

            cout << "Processing " << npbmsg
                 << " PrepBufr messages...\n" << flush;
         }
      }
      else if(file_ut != msg_ut) {
         cerr << "\n\nERROR: process_pbfile() -> "
              << "the observation time should remain the same for "
              << "all PrepBufr messages: " << msg_ut << " != "
              << file_ut << "\n\n" << flush;
         exit(1);
      }

      //
      // Null terminate the message type string
      //
      hdr_typ[6] = '\0';

      //
      // If the message type is not listed in the configuration
      // file and it is not the case that all message types should be
      // retained, continue to the next PrepBufr message
      //
      if(!keep_message_type(hdr_typ)) {
         rej_typ++;
         continue;
      }

      //
      // Convert the SID to a string and null terminate
      //
      dbl2str(&hdr[0], hdr_sid);
      hdr_sid[8] = '\0';

      //
      // Change the first blank space to a null
      //
      for(i=0; i<9; i++) {
         if(hdr_sid[i] == ' ') {
            hdr_sid[i] = '\0';
            break;
         }
      }

      //
      // If the station id is not listed in the configuration
      // file and it is not the case that all station ids should be
      // retained, continue to the next PrepBufr message
      //
      if(!keep_station_id(hdr_sid)) {
         rej_sid++;
         continue;
      }

      //
      // Read the header array elements which consists of:
      //    LON LAT DHR ELV TYP T29 ITP
      //

      // Longitude
      if(hdr[1] > r8bfms) hdr_arr_lon     = fill_value;
      else                hdr_arr_lon     = hdr[1];

      // Latitude
      if(hdr[2] > r8bfms) hdr_arr_lat     = fill_value;
      else                hdr_arr_lat     = hdr[2];

      // Elevation
      if(hdr[4] > r8bfms) hdr_arr_elv     = fill_value;
      else                hdr_arr_elv     = hdr[4];

      // PrepBufr Report Type
      if(hdr[5] > r8bfms) pb_report_type  = fill_value;
      else                pb_report_type  = hdr[5];

      // Input Report Type
      if(hdr[6] > r8bfms) in_report_type  = fill_value;
      else                in_report_type  = hdr[6];

      // Instrument Type
      if(hdr[7] > r8bfms) instrument_type = fill_value;
      else                instrument_type = hdr[7];

      //
      // Compute the valid time and check if it is within the
      // specified valid range
      //
      hdr_vld_ut = file_ut + (unixtime) (hdr[3]*sec_per_hour);
      if(!keep_valid_time(hdr_vld_ut, beg_ut, end_ut)) {
         rej_vld++;
         continue;
      }

      //
      // Rescale the longitude value from 0 to 360 -> -180 to 180
      //
      hdr_arr_lon = rescale_lon(hdr_arr_lon);

      //
      // If the lat/lon for the PrepBufr message is not on the
      // grid_mask, continue to the next PrepBufr message
      //
      if(apply_mask_grid) {
         grid_mask.latlon_to_xy(hdr_arr_lat, (-1.0*hdr_arr_lon), x, y);
         if(x < 0 || x >= grid_mask.nx() ||
            y < 0 || y >= grid_mask.ny()) {
            rej_grid++;
            continue;
         }
      }

      //
      // If the lat/lon for the PrepBufr message is not inside the mask
      // polyline continue to the next PrepBufr message.  Multiply by
      // -1 to convert from degrees_east to degrees_west
      //
      if(apply_mask_poly &&
         !poly_mask.latlon_is_inside(hdr_arr_lat, hdr_arr_lon)) {
         rej_poly++;
         continue;
      }

      //
      // Check if the message elevation is within the specified range.
      // Missing data values for elevation are retained.
      //
      if (!is_eq(hdr_arr_elv, fill_value) &&
         (hdr_arr_elv < conf.beg_elev().dval() ||
          hdr_arr_elv > conf.end_elev().dval()) ) {
         rej_elv++;
         continue;
      }

      //
      // If the PrepBufr report type is not listed in the configuration
      // file and it is not the case that all PrepBufr report types
      // should be retained, continue to the next PrepBufr message.
      //
      if(!keep_pb_report_type(nint(pb_report_type))) {
         rej_pb_rpt++;
         continue;
      }

      //
      // If the input report type is not listed in the configuration
      // file and it is not the case that all input report types
      // should be retained, continue to the next PrepBufr message.
      //
      if(!keep_in_report_type(nint(in_report_type))) {
         rej_in_rpt++;
         continue;
      }

      //
      // If the instrument type is not listed in the configuration
      // file, and it is not the case that all instrument types
      // should be retained, continue to the next PrepBufr message.
      //
      if(!keep_instrument_type(nint(instrument_type))) {
         rej_itp++;
         continue;
      }

      //
      // Search through the observation values and store them as:
      //    HDR_ID GC LVL HGT OB
      //

      //
      // Store the index to the header data
      //
      obs_arr[0] = (float) hdr_typ_sa.n_elements();

      //
      // Search through the vertical levels
      //
      for(lv=0, n_hdr_obs=0; lv<nlev; lv++) {

         //
         // If the observation vertical level is not within the
         // specified valid range, continue to the next vertical
         // level
         //
         if(lv+1 < conf.beg_level().dval() ||
            lv+1 > conf.end_level().dval()) continue;

         //
         // Get the pressure level for this set of observations.
         // If not valid, continue to the next level
         //
         if(evns[0][0][lv][0] > r8bfms) continue;
         else {

            //
            // Get the event index to be used for the pressure
            // observation
            //
            ev = get_event_index(conf.event_stack_flag().ival(), 0, lv);

            //
            // Retain the pressure in hPa for each observation record
            //
            obs_arr[2] = evns[0][ev][lv][0];

            //
            // Get the event index to be used for the height
            // observation
            //
            ev = get_event_index(conf.event_stack_flag().ival(), 3, lv);

            //
            // Retain the vertical height for each observation record
            //
            if(evns[3][ev][lv][0] > r8bfms) {
               obs_arr[3] = fill_value;
            }
            else {

               //
               // Convert from geopotential height to MSL
               //
               obs_arr[3] = convert_gpm_to_msl(evns[3][ev][lv][0],
                                               hdr_arr_lat);
            }
         }

         //
         // Initialize the P, Q, T, Z, U, V variables
         //
         for(i=0; i<mxr8vt; i++) pqtzuv[i] = fill_value;

         //
         // Index through the variable types 'P, Q, T, Z, U, V'
         //
         for(kk=0; kk<mxr8vt; kk++) {

            //
            // Convert the observation variable index to the
            // corresponding grib code and store as the second element
            // of obs_arr
            //
            obs_arr[1] = (float) var_gc[kk];

            //
            // Get the event index to be used based on the contents of
            // the event stack flag
            //
            ev = get_event_index(conf.event_stack_flag().ival(),
                                 kk, lv);

            //
            // If the observation value or the quality mark is not
            // valid, continue to the next variable type
            //
            if(evns[kk][ev][lv][0] > r8bfms ||
               evns[kk][ev][lv][1] > r8bfms) {
               continue;
            }
            //
            // Get the actual observation value and quality mark
            //
            else {
               obs_arr[4]   = (float) evns[kk][ev][lv][0];
               quality_mark = (float) evns[kk][ev][lv][1];
            }

            //
            // Retrieve the data level category from the top of the
            // event stack: ev = 0
            //
            if(evns[kk][0][lv][6] > r8bfms) {
               dl_category = fill_value;
            }
            else {
               dl_category = (float) evns[kk][0][lv][6];
            }

            //
            // Convert pressure from millibars to pascals
            //
            if(!is_eq(obs_arr[4], fill_value) &&
               nint(obs_arr[1]) == pres_grib_code) {
               obs_arr[4] *= pa_per_mb;
            }
            //
            // Convert specific humidity from mg/kg to kg/kg
            //
            else if(!is_eq(obs_arr[4], fill_value) &&
                    nint(obs_arr[1]) == spfh_grib_code) {
               obs_arr[4] *= kg_per_mg;
            }
            //
            // Convert temperature from celcius to kelvin
            //
            else if(!is_eq(obs_arr[4], fill_value) &&
                    nint(obs_arr[1]) == tmp_grib_code) {
                obs_arr[4] += c_to_k;
            }

            //
            // If the quality mark is greater than than the quality
            // mark threshold in the configuration file
            // continue to the next observation event
            //
            if(conf.quality_mark_thresh().ival() < quality_mark)
               continue;

            //
            // If the data level category is not listed in the
            // configuration file and it is not the case that
            // all data level categories should be retained,
            // continue to the next event
            //
            if(!keep_level_category(nint(dl_category))) continue;

            //
            // Store the observation values from which other
            // variables may be derived
            //
            pqtzuv[kk] = obs_arr[4];

            //
            // If the grib code corrsponding to the observation
            // variable is not listed in the configuration file
            // continue to the next observation variable
            //
            if(!keep_obs_grib_code(var_gc[kk])) continue;

            //
            // Write the observation array to the netCDF file
            //
            if(!obs_arr_var->set_cur(n_total_obs, (long) 0) ||
               !obs_arr_var->put(obs_arr, (long) 1,
                                 (long) obs_arr_len)) {
               cerr << "\n\nERROR: process_pbfile() -> "
                    << "error writing the observation array to the "
                    << "netCDF file\n\n" << flush;
               exit(1);
            }

            //
            // Increment the current and total observations counts
            //
            n_file_obs++;
            n_total_obs++;

            //
            // Increment the counter for the number of obs for this
            // header
            //
            n_hdr_obs++;
         } // end for kk

         //
         // Reset obs_arr[1] and obs_arr[4] to fill_value
         //
         obs_arr[1] = fill_value; // grib code
         obs_arr[4] = fill_value; // observation value

         //
         // Derive quantities which can be derived from
         // P, Q, T, Z, U, V
         //
         for(i=0; i<n_derive_gc; i++) {

            if(keep_obs_grib_code(derive_gc[i])) {

               //
               // Only derive PRMSL for message types ADPSFC and SFCSHP
               // which are stored in the onlysf_msg_typ_str
               //
               if(derive_gc[i] == prmsl_grib_code &&
                  strstr(onlysf_msg_typ_str, hdr_typ) == NULL)
                  continue;

               //
               // Store the grib code to be derived
               //
               obs_arr[1] = derive_gc[i];

               //
               // Derive the value for the grib code
               //
               obs_arr[4] = derive_grib_code(derive_gc[i], pqtzuv,
                                             hdr_arr_lat);

               if(is_eq(obs_arr[4], fill_value)) continue;

               //
               // Write the observation array to the netCDF file
               //
               if(!obs_arr_var->set_cur(n_total_obs, (long) 0) ||
                  !obs_arr_var->put(obs_arr, (long) 1,
                                   (long) obs_arr_len) ) {
                  cerr << "\n\nERROR: main() -> "
                       << "error writing the derive observation array "
                       << "to the netCDF file\n\n" << flush;
                  exit(1);
               }

               //
               // Increment the current and total observations counts
               //
               n_file_obs++;
               n_total_obs++;

               //
               // Increment the counter for the number of obs for this
               // header
               //
               n_hdr_obs++;
            }
         } // end for i
      } // end for lv

      //
      // If the number of observations for this header is non-zero,
      // store the header data and increment the PrepBufr record
      // counter
      //
      if(n_hdr_obs > 0) {

         hdr_typ_sa.add(hdr_typ);
         hdr_sid_sa.add(hdr_sid);
         unix_to_mdyhms(hdr_vld_ut, mon, day, yr, hr, min, sec);
         sprintf(time_str, "%.4i%.2i%.2i_%.2i%.2i%.2i",
                 yr, mon, day, hr, min, sec);
         hdr_vld_sa.add(time_str);
         hdr_arr_lat_na.add(hdr_arr_lat);
         hdr_arr_lon_na.add(hdr_arr_lon);
         hdr_arr_elv_na.add(hdr_arr_elv);

         i_msg++;
      }
      else {
         rej_nobs++;
      }

   } // end for

   if(verbosity > 1) {
      cout << "\nTotal PrepBufr Messages processed\t= "
           << npbmsg << "\n"
           << "Rejected based on message type\t\t= "
           << rej_typ << "\n"
           << "Rejected based on station id\t\t= "
           << rej_sid << "\n"
           << "Rejected based on valid time\t\t= "
           << rej_vld << "\n"
           << "Rejected based on masking grid\t\t= "
           << rej_grid << "\n"
           << "Rejected based on masking polygon\t= "
           << rej_poly << "\n"
           << "Rejected based on elevation\t\t= "
           << rej_elv << "\n"
           << "Rejected based on pb report type\t= "
           << rej_pb_rpt << "\n"
           << "Rejected based on input report type\t= "
           << rej_in_rpt << "\n"
           << "Rejected based on instrument type\t= "
           << rej_itp << "\n"
           << "Rejected based on zero observations\t= "
           << rej_nobs << "\n"
           << "Total PrepBufr Messages retained\t= "
           << i_msg << "\n"
           << "Total observations retained or derived\t= "
           << n_file_obs << "\n"
           << flush;
   }

   //
   // Close the PREPBUFR file
   //
   closepb_(&unit);

   //
   // Delete the temporary blocked file
   //
   remove_temp_file(blk_file);

   if(i_msg <= 0) {
      cout << "***WARNING*** process_pbfile() -> "
           << "No PrepBufr messages retained from file: "
           << pbfile[i_pb] << "\n" << flush;
      return;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_netcdf_hdr_data() {
   int i;
   float hdr_arr[hdr_arr_len];

   //
   // Check for no messages retained
   //
   if(hdr_typ_sa.n_elements() <= 0) {
      cerr << "\n\nERROR: write_netcdf_hdr_data() -> "
           << "No PrepBufr messages retained.  Nothing to write.\n\n"
           << flush;

      //
      // Delete the NetCDF file
      //
      if(remove(ncfile) != 0) {
         cerr << "\n\nERROR: write_netcdf_hdr_data() -> "
              << "can't remove output NetCDF file \"" << ncfile
              << "\"\n\n" << flush;
         exit(1);
      }
      exit(1);
   }

   //
   // Define netCDF dimensions
   //
   hdr_dim = f_out->add_dim("nhdr", (long) hdr_typ_sa.n_elements());

   //
   // Define netCDF variables
   //
   hdr_typ_var = f_out->add_var("hdr_typ", ncChar,  hdr_dim, strl_dim);
   hdr_sid_var = f_out->add_var("hdr_sid", ncChar,  hdr_dim, strl_dim);
   hdr_vld_var = f_out->add_var("hdr_vld", ncChar,  hdr_dim, strl_dim);
   hdr_arr_var = f_out->add_var("hdr_arr", ncFloat, hdr_dim,
                                hdr_arr_dim);

   //
   // Add variable attributes
   //
   hdr_typ_var->add_att("long_name", "message type");
   hdr_sid_var->add_att("long_name", "station identification");
   hdr_vld_var->add_att("long_name", "valid time");
   hdr_vld_var->add_att("units", "YYYYMMDD_HHMMSS");

   hdr_arr_var->add_att("long_name",
                        "array of observation station header values");
   hdr_arr_var->add_att("_fill_value", fill_value);
   hdr_arr_var->add_att("columns", "lat lon elv");
   hdr_arr_var->add_att("lat_long_name", "latitude");
   hdr_arr_var->add_att("lat_units", "degrees_north");
   hdr_arr_var->add_att("lon_long_name", "longitude");
   hdr_arr_var->add_att("lon_units", "degrees_east");
   hdr_arr_var->add_att("elv_long_name", "elevation");
   hdr_arr_var->add_att("elv_units", "meters above sea level (msl)");

   // Loop through and write out the header data
   for(i=0; i<hdr_typ_sa.n_elements(); i++) {

      //
      // PrepBufr Message type
      //
      if(!hdr_typ_var->set_cur(i, (long) 0) ||
         !hdr_typ_var->put(hdr_typ_sa[i], (long) 1,
                           (long) strlen(hdr_typ_sa[i]))) {
         cerr << "\n\nERROR: write_netcdf_hdr_data() -> "
              << "error writing the prepbufr message type string to "
              << "the netCDF file\n\n" << flush;
         exit(1);
      }

      //
      // Station ID
      //
      if(!hdr_sid_var->set_cur(i, (long) 0) ||
         !hdr_sid_var->put(hdr_sid_sa[i], (long) 1,
                           (long) strlen(hdr_sid_sa[i]))) {
         cerr << "\n\nERROR: write_netcdf_hdr_data() -> "
              << "error writing the station id string to the "
              << "netCDF file\n\n" << flush;
         exit(1);
      }

      //
      // Valid Time
      //
      if(!hdr_vld_var->set_cur(i, (long) 0) ||
         !hdr_vld_var->put(hdr_vld_sa[i], (long) 1,
                           (long) strlen(hdr_vld_sa[i]))) {
         cerr << "\n\nERROR: write_netcdf_hdr_data() -> "
              << "error writing the valid time to the "
              << "netCDF file\n\n" << flush;
         exit(1);
      }

      //
      // Write the header array which consists of the following:
      //    LAT LON ELV
      //
      hdr_arr[0] = (float) hdr_arr_lat_na[i];
      hdr_arr[1] = (float) hdr_arr_lon_na[i];
      hdr_arr[2] = (float) hdr_arr_elv_na[i];

      if(!hdr_arr_var->set_cur(i, (long) 0) ||
         !hdr_arr_var->put(hdr_arr, (long) 1,
                           (long) hdr_arr_len)) {
         cerr << "\n\nERROR: write_netcdf_hdr_data() -> "
              << "error writing the header array to the "
              << "netCDF file\n\n" << flush;
         exit(1);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {

   //
   // Deallocate memory and clean up
   //
   if(obs_gc) { delete [] obs_gc; obs_gc = (int *) 0; }

   f_out->close();
   delete f_out;
   f_out = (NcFile *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////

int get_event_index(int flag, int i_var, int i_lvl) {
   int ev, i;

   //
   // Check the event_stack_flag to determine if the top or bottom
   // of the event stack is to be used
   //   Top of the stack:    ev = 0
   //   Bottom of the stack: ev > 0
   //
   if(conf.event_stack_flag().ival() == 1) {
      ev = 0;
   }
   //
   // If the bottom of the event stack is to be used, find the
   // correct event index
   //
   else {

      // Index through the events
      for(i=0; i<mxr8vn; i++) {

         // Check if the observation variable is invalid
         if(evns[i_var][i][i_lvl][0] > r8bfms) {
            ev = i-1;
            break;
         }
      } // end for i
      if(ev < 0) ev = 0;
   }

   return(ev);
}

////////////////////////////////////////////////////////////////////////

void dbl2str(double *d, char *str) {
   const char *fmt_str = "%s";

   sprintf(str, fmt_str, d);

   return;
}

////////////////////////////////////////////////////////////////////////

int keep_message_type(const char *mt_str) {
   int i, n, keep;

   n = conf.n_message_type_elements();
   if(n == 0) return(1);

   keep = 0;
   for(i=0; i<n; i++) {
      if(strcmp(conf.message_type(i).sval(), mt_str) == 0) {
         keep = 1;
         break;
      }
   }

   // If ANYAIR, ANYSFC, or ONLYSF have been specified, check to see
   // what matches
   if(!keep) {
      if((anyair_flag && strstr(anyair_msg_typ_str, mt_str) != NULL) ||
         (anysfc_flag && strstr(anysfc_msg_typ_str, mt_str) != NULL) ||
         (onlysf_flag && strstr(onlysf_msg_typ_str, mt_str) != NULL)) {
         keep = 1;
      }
   }

   return(keep);
}

////////////////////////////////////////////////////////////////////////

int keep_station_id(const char *sid_str) {
   int i, n, keep;

   n = conf.n_station_id_elements();
   if(n == 0) return(1);

   keep = 0;
   for(i=0; i<n; i++) {
      if(strcmp(conf.station_id(i).sval(), sid_str) == 0) {
         keep = 1;
         break;
      }
   }

   return(keep);
}

////////////////////////////////////////////////////////////////////////

int keep_valid_time(const unixtime ut, const unixtime min_ut,
                    const unixtime max_ut) {
   int keep;

   //
   // Initialize to keep
   //
   keep = 1;

   //
   // If min_ut and max_ut both set, check the range
   //
   if(min_ut != (unixtime) 0 && max_ut != (unixtime) 0) {
      if(ut < min_ut || ut > max_ut) keep = 0;
   }

   //
   // If only min_ut set, check the lower bound
   //
   else if(min_ut != (unixtime) 0 && max_ut == (unixtime) 0) {
      if(ut < min_ut) keep = 0;
   }

   //
   // If only max_ut set, check the upper bound
   //
   else if(min_ut == (unixtime) 0 && max_ut != (unixtime) 0) {
      if(ut > max_ut) keep = 0;
   }

   return(keep);
}

////////////////////////////////////////////////////////////////////////

int keep_pb_report_type(int type) {
   int i, n, keep;

   n = conf.n_pb_report_type_elements();
   if(n == 0) return(1);

   keep = 0;
   for(i=0; i<n; i++) {
      if(conf.pb_report_type(i).ival() == type) {
         keep = 1;
         break;
      }
   }

   return(keep);
}

////////////////////////////////////////////////////////////////////////

int keep_in_report_type(int type) {
   int i, n, keep;

   n = conf.n_in_report_type_elements();
   if(n == 0) return(1);

   keep = 0;
   for(i=0; i<n; i++) {
      if(conf.in_report_type(i).ival() == type) {
         keep = 1;
         break;
      }
   }

   return(keep);
}

////////////////////////////////////////////////////////////////////////

int keep_instrument_type(int type) {
   int i, n, keep;

   n = conf.n_instrument_type_elements();
   if(n == 0) return(1);

   keep = 0;
   for(i=0; i<n; i++) {
      if(conf.instrument_type(i).ival() == type) {
         keep = 1;
         break;
      }
   }

   return(keep);
}


////////////////////////////////////////////////////////////////////////

int keep_obs_grib_code(int code) {
   int i, found;

   found = 0;

   for(i=0; i<n_obs_gc; i++) {
      if(obs_gc[i] == code) found = 1;
   }

   return(found);
}

////////////////////////////////////////////////////////////////////////

int keep_level_category(int category) {
   int i, n, keep;

   n = conf.n_level_category_elements();
   if(n == 0) return(1);

   keep = 0;
   for(i=0; i<n; i++) {
      if(conf.level_category(i).ival() == category) {
         keep = 1;
         break;
      }
   }

   return(keep);
}

////////////////////////////////////////////////////////////////////////

float derive_grib_code(int gc, float *pqtzuv, double lat) {
   float result;
   double p, q, t, z, u, v, w, vp;

   switch(gc) {

      //
      // Pressure Reduced to Mean Sea Level
      //
      case(prmsl_grib_code):
         p      = (double) pqtzuv[0];
         t      = (double) pqtzuv[2];
         z      = (double) pqtzuv[3];
         result = (float) convert_p_t_z_to_prmsl(p, t, z, lat);
         break;

      //
      // Humidity mixing ratio
      //
      case(mixr_grib_code):
         q      = (double) pqtzuv[1];
         result = (float) convert_q_to_w(q);
         break;

      //
      // Dewpoint temperature
      // Derived from p and q
      //
      case(dpt_grib_code):
         p      = (double) pqtzuv[0];
         q      = (double) pqtzuv[1];
         w      = convert_q_to_w(q);
         vp     = convert_p_w_to_vp(p, w);
         result = (float) convert_vp_to_dpt(vp);
         break;

      //
      // Relative humidity
      //
      case(rh_grib_code):
         p      = (double) pqtzuv[0];
         q      = (double) pqtzuv[1];
         t      = (double) pqtzuv[2];
         result = (float) convert_p_q_t_to_rh(p, q, t);
         break;

      //
      // Wind direction (direction wind is coming from)
      // Derived from u and v
      //
      case(wdir_grib_code):
         u      = (double) pqtzuv[4];
         v      = (double) pqtzuv[5];
         result = (float) convert_u_v_to_wdir(u, v);
         break;

      //
      // Wind speed
      // Derived from u and v
      //
      case(wind_grib_code):
         u      = (double) pqtzuv[4];
         v      = (double) pqtzuv[5];
         result = (float) convert_u_v_to_wind(u, v);
         break;

      default:
         v = fill_value;
         break;
   } // end switch

   return(result);
}

////////////////////////////////////////////////////////////////////////

void usage(int argc, char *argv[]) {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tprepbufr_file\n"
        << "\tnetcdf_file\n"
        << "\tconfig_file\n"
        << "\t[-pbfile prepbufr_file]\n"
        << "\t[-valid_beg time]\n"
        << "\t[-valid_end time]\n"
        << "\t[-nmsg n]\n"
        << "\t[-dump path]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"prepbufr_file\" is the input PrepBufr "
        << "observation file to be converted to netCDF format "
        << "(required).\n"

        << "\t\t\"netcdf_file\" indicates the name of the output "
        << "netCDF file to be written (required).\n"

        << "\t\t\"config_file\" is a PB2NCConfig file containing the "
        << "desired configuration settings (required).\n"

        << "\t\t\"-pbfile prepbufr_file\" may be used to specify "
        << "additional input PrepBufr observation files to be used "
        << "(optional).\n"

        << "\t\t\"-valid_beg time\" in YYYYMMDD[_HH[MMSS]] sets the "
        << "beginning of the retention time window (optional).\n"

        << "\t\t\"-valid_end time\" in YYYYMMDD[_HH[MMSS]] sets the "
        << "end of the retention time window (optional).\n"

        << "\t\t\"-nmsg n\" indicates the number of PrepBufr message "
        << "to process (optional).\n"

        << "\t\t\"-dump path\" indicates that the entire contents of "
        << "\"prepbufr_file\" should also be dumped to text files "
        << "in the directory specified (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << verbosity << ") (optional).\n\n"

        << flush;

   return;
}

////////////////////////////////////////////////////////////////////////
