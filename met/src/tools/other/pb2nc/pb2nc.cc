// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
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
//   010    10/28/11  Holmes         Added use of command line class to
//                    parse the command line arguments.
//   011    11/14/11  Holmes         Added code to enable reading of
//                    multiple config files.
//   012    05/11/12  Halley Gotway  Switch to using vx_config library.
//   013    07/12/13  Halley Gotway  Use observations of sensible
//                    temperature instead of virtual temperature.
//   014    07/23/13  Halley Gotway  Update sensible temperature fix
//                    to handle older GDAS PREPBUFR files.
//          06/07/17  Howard Soh     Added more options: -vars, -all, and -index.
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
#include <limits>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "pb2nc_conf_info.h"
#include "data_class.h"
#include "data2d_factory.h"
#include "vx_log.h"
#include "vx_nc_util.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_pb_util.h"
#include "vx_analysis_util.h"
#include "vx_cal.h"
#include "vx_math.h"

#define OBS_BUFFER_SIZE  (128 * 1024)

////////////////////////////////////////////////////////////////////////

//
// Constants
//


static const char * default_config_filename = "MET_BASE/config/PB2NCConfig_default";

static const char *program_name = "pb2nc";

static const float fill_value   = -9999.f;

// Constants used to interface to Fortran subroutines

// Missing value for BUFR data
static const double r8bfms      = 1.0E10;
// Maximum number of BUFR parameters
static const int mxr8pm         = 10;
// Maximum number of BUFR levels
static const int mxr8lv         = 1023;   // was 255;
// Maximum number of BUFR event sequences
static const int mxr8vn         = 10;
// Maximum number of BUFR variable types
static const int mxr8vt         = 6;
// Maximum length of BUFR variable name
static const int mxr8nm         = 8;
// Maximum number of BUFR variable types

// Length of the "YYYYMMDD_HHMMSS" string
static const int strl_len       = 16;
// Observation header length
static const int hdr_arr_len    = 3;
// Observation values length
static const int obs_arr_len    = 5;

// File unit number for opening the PrepBufr file
static const int file_unit      = 11;
// 2nd file unit number for opening the PrepBufr file
static const int dump_unit      = 22;
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
static int bufr_var_code[mxr8vt];
static int bufr_derive_code[n_derive_gc];

// PREPBUFR VIRTMP program code
static const double virtmp_prog_code = 8.0;
static const int MIN_FORTRAN_FILE_ID =  1;
static const int MAX_FORTRAN_FILE_ID = 99;

////////////////////////////////////////////////////////////////////////

//
// Variables for command line arguments
//

// StringArray to store PrepBufr file name
static StringArray pbfile;

// Output NetCDF file name
static ConcatString ncfile;

// Input configuration file
static ConcatString  config_file;
static PB2NCConfInfo conf_info;

// Beginning and ending retention times
static unixtime valid_beg_ut, valid_end_ut;

// Number of PrepBufr messages to process from the command line
static int nmsg = -1;

// Dump contents of PrepBufr file to ASCII files
static bool dump_flag = false;
static ConcatString dump_dir = ".";

static bool do_all_vars      = false;
static bool use_var_id       = false;
static bool use_met_vars     = false;
static bool collect_metadata = false;
static ConcatString bufr_target_variables;

static ConcatString data_plane_filename;

static int compress_level = -1;

////////////////////////////////////////////////////////////////////////

// Shared memory defined in the PREPBC common block in readpb.prm
static double hdr[mxr8pm];
static double evns[mxr8vt][mxr8vn][mxr8lv][mxr8pm];
static int    nlev;

static const int BUFR_NUMBER_START = 13;
static const int BUFR_NAME_START = 2;
static const int BUFR_NAME_LEN = 8;
static const int BUFR_DESCRIPTION_START = 22;
static const int BUFR_DESCRIPTION_LEN = 56;
static const int BUFR_UNIT_START = 40;
static const int BUFR_UNIT_LEN = 25;
static const int BUFR_SEQUENCE_START = 13;
static const int BUFR_SEQUENCE_LEN = 66;

static const char *prepbufr_p_event = "POB PQM PPC PRC PFC PAN CAT";
static const char *prepbufr_q_event = "QOB QQM QPC QRC QFC QAN CAT";
static const char *prepbufr_t_event = "TOB TQM TPC TRC TFC TAN CAT";
static const char *prepbufr_z_event = "ZOB ZQM ZPC ZRC ZFC ZAN CAT";
static const char *prepbufr_u_event = "UOB WQM WPC WRC UFC UAN CAT";
static const char *prepbufr_v_event = "VOB WQM WPC WRC VFC VAN CAT";

static const char *prepbufr_hdrs_str = "SID XOB YOB DHR ELV TYP T29 ITP";

static const char *default_sid_name = "SID";
static const char *default_lat_name = "YOB";
static const char *default_lon_name = "XOB";
static const char *default_ymd_name = "YEAR MNTH DAYS";
static const char *default_hms_name = "HOUR MINU SECO";

// Pick the latter one if exists multiuple variables
static const char *bufr_avail_sid_names = "SID SAID RPID";
static const char *bufr_avail_latlon_names = "XOB CLON CLONH YOB CLAT CLATH";

static double bufr_obs[mxr8lv][mxr8pm];
static double bufr_obs_evt[mxr8vn][mxr8lv][mxr8pm];
static double bufr_pres_lv[mxr8lv]; // Retain the pressure in hPa
static double bufr_msl_lv[mxr8lv];  // Convert from geopotential height to MSL

static StringArray prepbufr_hdrs;
static StringArray prepbufr_vars;
static StringArray prepbufr_event_members;

static StringArray prepbufr_derive_vars;
static StringArray event_names;
static StringArray event_members;
static StringArray ascii_vars;
static StringArray var_names;
static StringArray var_units;
static StringArray tableB_vars;
static StringArray tableB_descs;
static ConcatString bufr_hdrs;      // header name list to read header
static StringArray bufr_hdr_arr;    // available header name list
static StringArray bufr_obs_arr;    // available obs. name list
static IntArray filtered_times;

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

//int   obs_buf_size;
//int   processed_count;
int   obs_data_idx;
int   obs_data_offset;
int   hdr_data_offset;

char   hdr_typ_buf[OBS_BUFFER_SIZE][strl_len];
char   hdr_sid_buf[OBS_BUFFER_SIZE][strl_len];
char   hdr_vld_buf[OBS_BUFFER_SIZE][strl_len];
float  hdr_arr_buf[OBS_BUFFER_SIZE][hdr_arr_len];
float obs_data_buf[OBS_BUFFER_SIZE][obs_arr_len];
char  qty_data_buf[OBS_BUFFER_SIZE][strl_len];


//
// Output NetCDF file, dimensions, and variables
//
static NcFile *f_out      = (NcFile *) 0;

static NcDim strl_dim    ; // Maximum string length
static NcDim hdr_arr_dim ; // Header array width
static NcDim obs_arr_dim ; // Observation array width
static NcDim hdr_dim     ; // Header array length
static NcDim obs_dim     ; // Observation array length

static NcVar hdr_typ_var ; // Message type
static NcVar hdr_sid_var ; // Station ID
static NcVar hdr_vld_var ; // Valid time
static NcVar hdr_arr_var ; // Header array
static NcVar obs_qty_var ; // Quality flag
static NcVar obs_arr_var ; // Observation array

static NcDim bufr_unit_dim ;    // BUFR variable unit length
static NcDim bufr_desc_dim ;    // BUFR variable description length
static NcDim bufr_var_dim ;     // BUFR variable names array length
static NcVar bufr_obs_var ;     // BUFR variable names
static NcVar bufr_unit_var ;    // BUFR variable units
static NcVar bufr_desc_var ;    // BUFR variable descriptions

////////////////////////////////////////////////////////////////////////

extern "C" {
   void numpbmsg_(int *, int *);
   void openpb_(const char *, int *);
   void closepb_(int *);
   void readpb_(int *, int *, int *, double[mxr8pm],
                double[mxr8vt][mxr8vn][mxr8lv][mxr8pm]);
   void ireadns_  (int *, char *, int *);
   void readpbevt_(int *, int *, int *,
                double[mxr8vn][mxr8lv][mxr8pm], char[mxr8lv*mxr8pm], int *);
   void readpbint_(int *, int *, int *, double[mxr8lv][mxr8pm], char[mxr8lv*mxr8pm], int *);
   void dumppb_(const char *, int *, const char *, int *,
                const char *, int *, int *);
   void dump_tbl_(const char *, int *, const char *, int *);
}

////////////////////////////////////////////////////////////////////////

static void   initialize();
static void   process_command_line(int, char **);
static void   open_netcdf();
static void   process_pbfile(int);
static void   process_pbfile_metadata(int);
static void   write_netcdf_hdr_data();
static void   clean_up();

static int    get_event_index(int, int, int);
static int    get_event_index_temp(int, int, int);
static void   dbl2str(double *, char *);

static bool   keep_message_type(const char *);
static bool   keep_station_id(const char *);
static bool   keep_valid_time(const unixtime, const unixtime,
                              const unixtime);
static bool   keep_pb_report_type(int);
static bool   keep_in_report_type(int);
static bool   keep_instrument_type(int);
static bool   keep_obs_grib_code(int);
static bool   keep_level_category(int);

static float  derive_grib_code(int, float *, float *, double,
                               float&);

static void   usage();
static void   set_pbfile(const StringArray &);
static void   set_valid_beg_time(const StringArray &);
static void   set_valid_end_time(const StringArray &);
static void   set_nmsg(const StringArray &);
static void   set_dump_path(const StringArray &);
static void   set_do_all_variables(const StringArray & a);
static void   set_collect_metadata(const StringArray &);
static void   set_target_variables(const StringArray & a);
static void   set_use_var_id(const StringArray &);
static void   set_logfile(const StringArray &);
static void   set_verbosity(const StringArray &);
static void   set_compress(const StringArray &);

static void   display_bufr_variables(const StringArray &, const StringArray &,
                                     const StringArray &, const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   int i;

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Initialize static variables
   initialize();

   // Process the command line arguments
   process_command_line(argc, argv);

   // Open the NetCDF file
   if (!collect_metadata) open_netcdf();

   // Process each PrepBufr file
   for(i=0; i<pbfile.n_elements(); i++) {
      process_pbfile_metadata(i);
      if (!collect_metadata) process_pbfile(i);
   }
   
   // Write the NetCDF file
   if (!collect_metadata) {
      write_netcdf_hdr_data();
   }
   else {
      display_bufr_variables(tableB_vars, tableB_descs,
                             bufr_hdr_arr, bufr_obs_arr);
   }

   // Deallocate memory and clean up
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

   prepbufr_vars.clear();
   prepbufr_hdrs.clear();
   prepbufr_event_members.clear();

   prepbufr_hdrs.parse_wsss(prepbufr_hdrs_str);
   
   StringArray tmp_hdr_array;
   tmp_hdr_array.clear();
   tmp_hdr_array.parse_wsss(prepbufr_p_event);
   prepbufr_event_members.add(tmp_hdr_array);
   if (0 < tmp_hdr_array.n_elements()) prepbufr_vars.add(tmp_hdr_array[0]);
   
   tmp_hdr_array.clear();
   tmp_hdr_array.parse_wsss(prepbufr_q_event);
   prepbufr_event_members.add(tmp_hdr_array);
   if (0 < tmp_hdr_array.n_elements()) prepbufr_vars.add(tmp_hdr_array[0]);
   
   
   tmp_hdr_array.clear();
   tmp_hdr_array.parse_wsss(prepbufr_t_event);
   prepbufr_event_members.add(tmp_hdr_array);
   if (0 < tmp_hdr_array.n_elements()) prepbufr_vars.add(tmp_hdr_array[0]);
   
   tmp_hdr_array.clear();
   tmp_hdr_array.parse_wsss(prepbufr_z_event);
   prepbufr_event_members.add(tmp_hdr_array);
   if (0 < tmp_hdr_array.n_elements()) prepbufr_vars.add(tmp_hdr_array[0]);
   
   tmp_hdr_array.clear();
   tmp_hdr_array.parse_wsss(prepbufr_u_event);
   prepbufr_event_members.add(tmp_hdr_array);
   if (0 < tmp_hdr_array.n_elements()) prepbufr_vars.add(tmp_hdr_array[0]);
   
   tmp_hdr_array.clear();
   tmp_hdr_array.parse_wsss(prepbufr_v_event);
   prepbufr_event_members.add(tmp_hdr_array);
   if (0 < tmp_hdr_array.n_elements()) prepbufr_vars.add(tmp_hdr_array[0]);
   
   prepbufr_derive_vars.add("D_DWP");
   prepbufr_derive_vars.add("D_WDIR");
   prepbufr_derive_vars.add("D_WIND");
   prepbufr_derive_vars.add("D_RH");
   prepbufr_derive_vars.add("D_MIXR");
   prepbufr_derive_vars.add("D_PRMSL");
   
   return;
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;
   ConcatString default_config_file;

   // Check for zero arguments
   if(argc == 1) usage();

   // Initialize retention times
   valid_beg_ut = valid_end_ut = (unixtime) 0;

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_pbfile, "-pbfile", 1);
   cline.add(set_valid_beg_time, "-valid_beg", 1);
   cline.add(set_valid_end_time, "-valid_end", 1);
   cline.add(set_nmsg, "-nmsg", 1);
   cline.add(set_dump_path, "-dump", 1);
   cline.add(set_do_all_variables,  "-all",  0);
   cline.add(set_collect_metadata,  "-index",  0);
   cline.add(set_target_variables, "-vars", 1);
   cline.add(set_use_var_id, "-use_var_id", 0);
   cline.add(set_logfile, "-log", 1);
   cline.add(set_verbosity, "-v", 1);
   cline.add(set_compress,  "-compress",  1);

   // Parse the command line
   cline.parse();

   // Check for error. There should be three arguments left:
   // PrepBufr, output NetCDF, and config filenames
   if(cline.n() != 3) usage();

   // Store the input file names
   pbfile.add(cline[0]);
   ncfile      = cline[1];
   config_file = cline[2];

   // Create the default config file name
   default_config_file = replace_path(default_config_filename);

   // List the config files
   mlog << Debug(1)
        << "Default Config File: " << default_config_file << "\n"
        << "User Config File: "    << config_file << "\n";

   // Read the config files
   conf_info.read_config(default_config_file, config_file);
   
   // Process the configuration
   conf_info.process_config();
   if (conf_info.use_var_id) use_var_id = true;

   if (!use_met_vars && 0 < conf_info.bufr_var_name.n_elements()) {
      use_met_vars = true;
      for (int idx=0; idx<conf_info.bufr_var_name.n_elements(); idx++) {
         if (0 < bufr_target_variables.length()) bufr_target_variables.add(" ");
         bufr_target_variables.add(conf_info.bufr_var_name[idx]);
      }
   }

   // Check that valid_end_ut >= valid_beg_ut
   if(valid_beg_ut != (unixtime) 0 &&
      valid_end_ut != (unixtime) 0 &&
      valid_beg_ut > valid_end_ut) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the ending time (" << unix_to_yyyymmdd_hhmmss(valid_end_ut)
           << ") must be greater than the beginning time ("
           << unix_to_yyyymmdd_hhmmss(valid_beg_ut) << ").\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString save_bufr_table_to_file(const char *blk_file, int file_id) {
   int len;
   char pid_str[128];
   ConcatString tbl_filename = conf_info.tmp_dir;
   
   tbl_filename.add("/tmp_pb2nc_bufr_");
   sprintf(pid_str, "%d", getpid());
   tbl_filename.add(pid_str);
   tbl_filename.add(".tbl");
   len = tbl_filename.length();
   if (file_id > MAX_FORTRAN_FILE_ID || file_id < MIN_FORTRAN_FILE_ID) {
      mlog << Error << "\nsave_bufr_table_to_file() -> "
           << "Invalid file ID [" << file_id << "] between 1 and 99.\n\n";
   }
   openpb_(blk_file, &file_id);
   dump_tbl_(blk_file, &file_id, tbl_filename, &len);
   closepb_(&file_id);
   return tbl_filename;
}


bool is_prepbufr_file(StringArray *events) {
   bool is_prepbufr = events->has("P__EVENT") && events->has("Q__EVENT")
         && events->has("T__EVENT") && events->has("Z__EVENT");
   return is_prepbufr;
}

////////////////////////////////////////////////////////////////////////

void get_variable_info(const char* tbl_filename) {
   static const int maximumLineLength = 128;
   char *lineBuffer = (char *)malloc(sizeof(char) * maximumLineLength);
   static const char *method_name = "get_variable_info()";

   FILE * fp;
   char * line = NULL;
   size_t len = 0;
   ssize_t read;

   event_names.clear();
   event_members.clear();
   ascii_vars.clear();
   var_names.clear();
   var_units.clear();
   tableB_vars.clear();
   tableB_descs.clear();
   
   fp = fopen(tbl_filename, "r");
   ConcatString input_data;
   if (fp != NULL) {
      char var_name[BUFR_NAME_LEN+1];
      char var_desc[BUFR_DESCRIPTION_LEN+1];
      char var_unit_str[BUFR_UNIT_LEN+1];
      bool find_mnemonic = false;
      
      // Processing section 1
      int var_count1 = 0;
      while ((read = getline(&line, &len, fp)) != -1) {
         if (NULL != strstr(line,"--------")) continue;
         if (NULL != strstr(line,"MNEMONIC")) {
            if (find_mnemonic) break;
            find_mnemonic = true;
            continue;
         }
         if ('0' != line[BUFR_NUMBER_START]) continue;
         
         strncpy(var_name, (line+BUFR_NAME_START), BUFR_NAME_LEN);
         var_name[BUFR_NAME_LEN] = bad_data_char;
         for (int idx=(BUFR_NAME_LEN-1); idx >=0; idx--) {
            if (' ' != var_name[idx] ) break;
            var_name[idx] = '\0';
         }
         if (0 == strlen(var_name)) continue;
         
         var_count1++;
         strncpy(var_desc, (line+BUFR_DESCRIPTION_START), BUFR_DESCRIPTION_LEN);
         var_desc[BUFR_DESCRIPTION_LEN] = bad_data_char;
         for (int idx=(BUFR_DESCRIPTION_LEN-1); idx>=0; idx--) {
            if (' ' != var_desc[idx] && '|' != var_desc[idx]) {
               break;
            }
            var_desc[idx] = '\0';
         }
         mlog << Debug(10) << "   " << method_name << " sec. 1 var name: ["
              << var_name << "]\tdesc: [" << var_desc << "]\n";
         tableB_vars.add(var_name);
         tableB_descs.add(var_desc);
      }

      // Skip  section 2
      while ((read = getline(&line, &len, fp)) != -1) {
         if (NULL != strstr(line,"MNEMONIC")) break;
         if (NULL == strstr(line,"EVENT")) continue;
         
         strncpy(var_name, (line+BUFR_NAME_START), BUFR_NAME_LEN);
         var_name[BUFR_NAME_LEN] = bad_data_char;
         for (int idx=(BUFR_NAME_LEN-1); idx >=0; idx--) {
            if (' ' != var_name[idx] ) break;
            var_name[idx] = '\0';
         }
         //if (NULL == strstr(var_name,"EVENT")) continue;
         
         strncpy(var_desc, (line+BUFR_SEQUENCE_START), BUFR_SEQUENCE_LEN);
         var_desc[BUFR_SEQUENCE_LEN] = bad_data_char;
         for (int idx=(BUFR_SEQUENCE_LEN-1); idx>=0; idx--) {
            if (' ' != var_desc[idx] && '|' != var_desc[idx]) {
               break;
            }
            var_desc[idx] = '\0';
         }
         mlog << Debug(10) << "   " << method_name << " event name: ["
              << var_name << "]\tdesc: [" << var_desc << "]\n";
         event_names.add(var_name);
         event_members.add(var_desc);
      }
      read = getline(&line, &len, fp);

      // Processing section 3
      while ((read = getline(&line, &len, fp)) != -1) {
         if (' ' == line[BUFR_NAME_START]) continue;
         if ('-' == line[BUFR_NAME_START]) break;
         
         strncpy(var_name, (line+BUFR_NAME_START), BUFR_NAME_LEN);
         var_name[BUFR_NAME_LEN] = bad_data_char;
         for (int idx=(BUFR_NAME_LEN-1); idx >=0; idx--) {
            if (' ' != var_name[idx] ) break;
            var_name[idx] = '\0';
         }
         
         if (NULL != strstr(line,"CCITT IA5")) {
            ascii_vars.add(var_name);
            strcpy(var_unit_str,"CCITT IA5");
         }
         else {
            strncpy(var_unit_str, (line+BUFR_UNIT_START), BUFR_UNIT_LEN);
            var_unit_str[BUFR_UNIT_LEN] = bad_data_char;
            for (int idx=(BUFR_UNIT_LEN-1); idx>=0; idx--) {
               if (' ' != var_unit_str[idx] && '|' != var_unit_str[idx]) {
                  break;
               }
               var_unit_str[idx] = '\0';
            }
            var_names.add(var_name);
            var_units.add(var_unit_str);
            mlog << Debug(10) << "   " << method_name << " sec. 3 var name: ["
                 << var_name << "]\tdesc: [" << var_desc << "]\n";
         }
      }

      fclose(fp);
      if (line)
         free(line);
     
   }
   return;
}

////////////////////////////////////////////////////////////////////////

void open_netcdf() {

   // Create the output netCDF file for writing
   mlog << Debug(1) << "Creating NetCDF File:\t\t" << ncfile << "\n";
   f_out = open_ncfile(ncfile, true);

   // Check for a valid file
   if(IS_INVALID_NC_P(f_out)) {
      mlog << Error << "\nopen_netcdf() -> "
           << "trouble opening output file: " << ncfile << "\n\n";

      delete f_out;
      f_out = (NcFile *) 0;

      exit(1);
   }

   // Define netCDF dimensions
   strl_dim    = add_dim(f_out, nc_dim_mxstr,   (long) strl_len);
   hdr_arr_dim = add_dim(f_out, nc_dim_hdr_arr, (long) hdr_arr_len);
   obs_arr_dim = add_dim(f_out, nc_dim_obs_arr, (long) obs_arr_len);
   obs_dim     = add_dim(f_out, nc_dim_nobs); // unlimited dimension

   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = conf_info.conf.nc_compression();

   // Define netCDF variables
   obs_qty_var = add_var(f_out, "obs_qty", ncChar,  obs_dim, strl_dim,    deflate_level);
   obs_arr_var = add_var(f_out, "obs_arr", ncFloat, obs_dim, obs_arr_dim, deflate_level);

   // Add global attributes
   write_netcdf_global(f_out, ncfile.text(), program_name);
   add_att(f_out, nc_att_use_var_id, (use_var_id ? "true" : "false"));

   // Add variable attributes
   add_att(&obs_qty_var, "long_name", "quality flag");
   add_att(&obs_arr_var, "long_name", "array of observation values");
   add_att(&obs_arr_var, "missing_value", fill_value);
   add_att(&obs_arr_var, "_FillValue", fill_value);
   add_att(&obs_arr_var, "hdr_id_long_name", "index of matching header data");
   if (use_var_id) {
      add_att(&obs_arr_var, "columns", "hdr_id var_id lvl hgt ob");
      add_att(&obs_arr_var, "var_id_long_name", "index of BUFR variable corresponding to the observation type");
   }
   else {
      add_att(&obs_arr_var, "columns", "hdr_id gc lvl hgt ob");
      add_att(&obs_arr_var, "gc_long_name", "grib code corresponding to the observation type");
   }
   add_att(&obs_arr_var, "lvl_long_name", "pressure level (hPa) or accumulation interval (sec)");
   add_att(&obs_arr_var, "hgt_long_name", "height in meters above sea level (msl)");
   add_att(&obs_arr_var, "ob_long_name", "observation value");

   return;
}

////////////////////////////////////////////////////////////////////////

void process_pbfile(int i_pb) {
   int npbmsg, unit, yr, mon, day, hr, min, sec;
   int i, i_msg, i_read, n_file_obs, i_ret, i_date, n_hdr_obs;
   int rej_typ, rej_sid, rej_vld, rej_grid, rej_poly;
   int rej_elv, rej_pb_rpt, rej_in_rpt, rej_itp, rej_nobs;
   int lv, ev, ev_temp, kk, len1, len2;

   double   x, y;

   unixtime file_ut = (unixtime) 0;
   unixtime msg_ut, beg_ut, end_ut;
   unixtime min_msg_ut, max_msg_ut;

   ConcatString file_name, blk_prefix, blk_file;
   char     prefix[max_str_len];
   char     time_str[max_str_len];
   char     start_time_str[max_str_len], end_time_str[max_str_len];
   char     min_time_str[max_str_len], max_time_str[max_str_len];

   char     hdr_typ[max_str_len], hdr_sid[max_str_len];
   double   hdr_arr_lat, hdr_arr_lon, hdr_arr_elv;
   float    pb_report_type, in_report_type, instrument_type;
   unixtime hdr_vld_ut;

   float    quality_mark, dl_category;
   float    obs_arr[obs_arr_len];
   float    pqtzuv[mxr8vt], pqtzuv_qty[mxr8vt];
   
   const int debug_level_for_performance = 3;
   int start_t, end_t = clock();
   int method_end, method_start = clock();

   StringArray variables_big_nlevels;
   static const char *method_name = "process_pbfile()";

   bool apply_grid_mask = (conf_info.grid_mask.nx() > 0 &&
                           conf_info.grid_mask.ny() > 0);
   bool apply_poly_mask = (conf_info.poly_mask.n_points() > 0);

   // List the PrepBufr file being processed
   mlog << Debug(1) << "Processing Bufr File:\t" << pbfile[i_pb]
        << "\n";

   // Initialize
   filtered_times.clear();
   min_msg_ut = max_msg_ut = (unixtime) 0;
   min_time_str[0] = bad_data_char;
   max_time_str[0] = bad_data_char;
   
   // Set the file name for the PrepBufr file
   file_name << pbfile[i_pb];

   // Build the temporary block file name
   blk_prefix << conf_info.tmp_dir << "/" << "tmp_pb2nc_blk";
   blk_file = make_temp_file_name(blk_prefix, '\0');

   mlog << Debug(1) << "Blocking Bufr file to:\t" << blk_file
        << "\n";

   // Assume that the input PrepBufr file is unblocked.
   // Block the PrepBufr file and open it for reading.
   pblock(file_name, blk_file, block);

   // Dump the contents of the PrepBufr file to ASCII files
   if(dump_flag) {
      mlog << Debug(1) << "Dumping to ASCII output directory:\t"
           << dump_dir << "\n";

      // Check for multiple PrepBufr files
      if(pbfile.n_elements() > 1) {
         mlog << Error << "\n" << method_name << " -> "
              << "the \"-dump\" and \"-pbfile\" options may not be "
              << "used together.  Only one Bufr file may be dump "
              << "to ASCII at a time.\n\n";
         exit(1);
      }

      for(i=0; i<n_vld_msg_typ; i++) {
         msg_typ_ret[i] = keep_message_type(vld_msg_typ_list[i]);
      }

      unit = dump_unit+i_pb;
      if (unit > MAX_FORTRAN_FILE_ID || unit < MIN_FORTRAN_FILE_ID) {
         mlog << Error << "\n" << method_name << " -> "
              << "Invalid file ID [" << unit << "] between 1 and 99.\n\n";
      }
      strcpy(prefix, get_short_name(pbfile[i_pb]));
      len1 = strlen(dump_dir);
      len2 = strlen(prefix);
      dumppb_(blk_file, &unit, dump_dir.text(), &len1,
              prefix, &len2, msg_typ_ret);
   }

   // Open the blocked temp PrepBufr file for reading
   unit = file_unit + i_pb;
   if (unit > MAX_FORTRAN_FILE_ID || unit < MIN_FORTRAN_FILE_ID) {
      mlog << Error << "\n" << method_name << " -> "
           << "Invalid file ID [" << unit << "] between 1 and 99.\n\n";
   }
   openpb_(blk_file, &unit);

   // Compute the number of PrepBufr records in the current file.
   numpbmsg_(&unit, &npbmsg);

   // Use the number of records requested by the user if there
   // are enough present.
   if(nmsg >= 0 && nmsg <= npbmsg) npbmsg = nmsg;

   // Check for zero messages to process
   if(npbmsg <= 0) {
      mlog << Warning << "\n" << method_name << " -> "
           << "No Bufr messages to process in file: "
           << pbfile[i_pb] << "\n\n";

      // Delete the temporary blocked file
      remove_temp_file(blk_file);

      return;
   }

   // Initialize counts
   i_ret   = n_file_obs = i_msg      = 0;
   rej_typ = rej_sid    = rej_vld    = rej_grid = rej_poly = 0;
   rej_elv = rej_pb_rpt = rej_in_rpt = rej_itp  = rej_nobs = 0;

   long offsets[2] = { 0, 0 };
   long lengths[2] = { OBS_BUFFER_SIZE, 1} ;

   //processed_count = 0;
   obs_data_idx = 0;
   obs_data_offset = 0;
   hdr_data_offset = 0;

   int buf_nlev;
   int valid_data_count = 0;
   bool is_prepbufr = is_prepbufr_file(&event_names);
   if(mlog.verbosity_level() >= debug_level_for_performance) {
      end_t = clock();
      cout << method_name << " " << (end_t-start_t)/double(CLOCKS_PER_SEC) << " seconds for preparing" << endl;
      start_t = clock();
   }
   
   int grib_code;
   map<ConcatString, ConcatString> _messageTypeMap = conf_info.getMessageTypeMap();
   // Loop through the PrepBufr messages from the input file
   for(i_read=0; i_read<npbmsg && i_ret == 0; i_read++) {

      if(mlog.verbosity_level() > 0) {
         if(nint(npbmsg/20.0) > 0 && (i_read+1)%nint(npbmsg/20.0) == 0) {
            cout << nint((double) (i_read+1)/npbmsg*100.0) << "% " << flush;
            if(mlog.verbosity_level() >= debug_level_for_performance) {
               end_t = clock();
               cout << (end_t-start_t)/double(CLOCKS_PER_SEC) << " seconds" << endl;
               start_t = clock();
            }
         }
      }

      // Get the next PrepBufr message
      ireadns_(&unit, hdr_typ, &i_date);
      readpb_(&unit, &i_ret, &nlev, hdr, evns);

      sprintf(time_str, "%.10i", i_date);
      msg_ut = yyyymmddhh_to_unix(time_str);
      
      // Check to make sure that the message time hasn't changed
      // from one PrepBufr message to the next
      if(file_ut == (unixtime) 0) {
         file_ut = msg_ut;
      
         unix_to_mdyhms(file_ut, mon, day, yr, hr, min, sec);
         sprintf(time_str, "%.4i%.2i%.2i_%.2i%.2i%.2i",
                 yr, mon, day, hr, min, sec);
         mlog << Debug(2) << (is_prepbufr ? "PrepBufr" : "Bufr")
              << " Time Center:\t\t" << time_str << "\n";
      
         // Check if valid_beg_ut and valid_end_ut were set on the
         // command line.  If so, use them.  If not, use beg_ds and
         // end_ds.
         if(valid_beg_ut != (unixtime) 0 ||
            valid_end_ut != (unixtime) 0) {
            beg_ut = valid_beg_ut;
            end_ut = valid_end_ut;
         }
         else {
            beg_ut = file_ut + conf_info.beg_ds;
            end_ut = file_ut + conf_info.end_ds;
         }
      
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
            sprintf(end_time_str, "%.4i%.2i%.2i_%.2i%.2i%.2i",
                     yr, mon, day, hr, min, sec);
         }
         else {
            strcpy(end_time_str, "NO_END_TIME");
         }
         strcpy(start_time_str, time_str);
      
         mlog << Debug(2) << "Searching Time Window:\t\t" << start_time_str
              << " to " << end_time_str << "\n";
      
         mlog << Debug(2) << "Processing " << npbmsg
              << (is_prepbufr ? " PrepBufr" : " Bufr") << " messages...\n";
      }
      else if((file_ut != msg_ut) && is_prepbufr) {
         mlog << Error << "\n" << method_name << " -> "
              << "the observation time should remain the same for "
              << "all " << (is_prepbufr ? "PrepBufr" : "Bufr") << " messages: " << msg_ut << " != "
              << file_ut << "\n\n";
         exit(1);
      }
      
      if (!is_prepbufr) {
         int index;
         int length;
         char tmp_str[mxr8lv*mxr8pm];
         
         //Read header (station id, lat, lon, ele, time)
         length = bufr_hdrs.length();
         strcpy(tmp_str, bufr_hdrs.text());
         readpbint_(&unit, &i_ret, &nlev, bufr_obs, tmp_str, &length );
         
         // Copy sid, lat, and lon
         for (index=0; index<3; index++) {
            hdr[index] = bufr_obs[0][index];
         }
         // Update hdr[3] and file_ut for obs_time
         hdr[3] = 0;
         if (bufr_obs[0][3] > r8bfms) {
            yr  = nint(bufr_obs[0][4]);
            mon = nint(bufr_obs[0][5]);
            day = nint(bufr_obs[0][6]);
            hr  = nint(bufr_obs[0][7]);
            min = nint(bufr_obs[0][8]);
            sec = nint(bufr_obs[0][9]);
            if (hr  > r8bfms) hr = 0;
            if (min > r8bfms) min = 0;
            if (sec > r8bfms || sec < 0) sec = 0;
            file_ut = mdyhms_to_unix(mon, day, yr, hr, min, sec);
            sprintf(time_str, "%.4i%.2i%.2i_%.2i%.2i%.2i",
                    yr, mon, day, hr, min, sec);
            mlog << Debug(7) << "Bufr obs_time:\t" << time_str << "\n\n";
            for (index=0; index<mxr8lv; index++) {
               bufr_pres_lv[index] = fill_value;
               bufr_msl_lv[index]  = fill_value;
            }
         }
      }
         
      // Null terminate the message type string
      if (is_prepbufr) hdr_typ[6] = '\0';
      else             hdr_typ[mxr8nm] = '\0';
      // Change the trailing blank space to a null
      for (i=mxr8nm-1; i>0; i--) {
         if (' ' != hdr_typ[i]) break;
         hdr_typ[i] = '\0';
      }
      
      // If the message type is not listed in the configuration
      // file and it is not the case that all message types should be
      // retained, continue to the next PrepBufr message
      if(!keep_message_type(hdr_typ)) {
         rej_typ++;
         continue;
      }
      
      // Convert the SID to a string and null terminate
      dbl2str(&hdr[0], hdr_sid);
      hdr_sid[mxr8nm] = '\0';
      // Change the trailing blank space to a null
      for (i=mxr8nm-1; i>0; i--) {
         if (' ' != hdr_sid[i]) break;
         hdr_sid[i] = '\0';
      }
      
      // If the station id is not listed in the configuration
      // file and it is not the case that all station ids should be
      // retained, continue to the next PrepBufr message
      if(!keep_station_id(hdr_sid)) {
         rej_sid++;
         continue;
      }
      
      // Read the header array elements which consists of:
      //    LON LAT DHR ELV TYP T29 ITP
      
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
      
      // Compute the valid time and check if it is within the
      // specified valid range
      hdr_vld_ut = file_ut + (unixtime) (hdr[3]*sec_per_hour);
      if (0 == min_msg_ut || min_msg_ut > hdr_vld_ut) {
         min_msg_ut = hdr_vld_ut;
         strcpy(min_time_str, time_str);
      }
      if (max_msg_ut < hdr_vld_ut) {
         max_msg_ut = hdr_vld_ut;
         strcpy(max_time_str, time_str);
      }
      if(!keep_valid_time(hdr_vld_ut, beg_ut, end_ut)) {
         if (!filtered_times.has(hdr_vld_ut)) {
            filtered_times.add(hdr_vld_ut);
         }
         rej_vld++;
         continue;
      }
      
      // Rescale the longitude value from 0 to 360 -> -180 to 180
      hdr_arr_lon = rescale_lon(hdr_arr_lon);
      
      // If the lat/lon for the PrepBufr message is not on the
      // grid_mask, continue to the next PrepBufr message
      if(apply_grid_mask) {
         conf_info.grid_mask.latlon_to_xy(hdr_arr_lat, (-1.0*hdr_arr_lon), x, y);
         if(x < 0 || x >= conf_info.grid_mask.nx() ||
            y < 0 || y >= conf_info.grid_mask.ny()) {
            rej_grid++;
            continue;
         }
      }
      
      // If the lat/lon for the PrepBufr message is not inside the mask
      // polyline continue to the next PrepBufr message.  Multiply by
      // -1 to convert from degrees_east to degrees_west
      if(apply_poly_mask &&
         !conf_info.poly_mask.latlon_is_inside_dege(hdr_arr_lat, hdr_arr_lon)) {
         rej_poly++;
         continue;
      }
      
      // Check if the message elevation is within the specified range.
      // Missing data values for elevation are retained.
      if (!is_eq(hdr_arr_elv, fill_value) &&
         (hdr_arr_elv < conf_info.beg_elev ||
          hdr_arr_elv > conf_info.end_elev) ) {
         rej_elv++;
         continue;
      }
      
      // If the PrepBufr report type is not listed in the configuration
      // file and it is not the case that all PrepBufr report types
      // should be retained, continue to the next PrepBufr message.
      if(!keep_pb_report_type(nint(pb_report_type))) {
         rej_pb_rpt++;
         continue;
      }
      
      // If the input report type is not listed in the configuration
      // file and it is not the case that all input report types
      // should be retained, continue to the next PrepBufr message.
      if(!keep_in_report_type(nint(in_report_type))) {
         rej_in_rpt++;
         continue;
      }
      
      // If the instrument type is not listed in the configuration
      // file, and it is not the case that all instrument types
      // should be retained, continue to the next PrepBufr message.
      if(!keep_instrument_type(nint(instrument_type))) {
         rej_itp++;
         continue;
      }
      
      // Search through the observation values and store them as:
      //    HDR_ID GC LVL HGT OB
      
      // Store the index to the header data
      obs_arr[0] = (float) hdr_typ_sa.n_elements();
      
      buf_nlev = nlev;
      if (nlev > mxr8lv) {
         buf_nlev = mxr8lv;
         for(kk=0; kk<mxr8vt; kk++) {
            if (!variables_big_nlevels.has(bufr_obs_arr[kk])) {
               mlog << Warning << "\n" << method_name << " -> "
                    << "Too many vertical levels (" << nlev
                    << ") for " << bufr_obs_arr[kk] 
                    << "). Ignored the vertical levels above " << mxr8lv << ".\n\n";
               variables_big_nlevels.add(bufr_obs_arr[kk]);
            }
         }
      }
      // Search through the vertical levels
      for(lv=0, n_hdr_obs = 0; lv<buf_nlev; lv++) {
      
         // If the observation vertical level is not within the
         // specified valid range, continue to the next vertical
         // level
         if(lv+1 < conf_info.beg_level ||
            lv+1 > conf_info.end_level) continue;
      
         // Get the pressure level for this set of observations.
         // If not valid, continue to the next level
         if(evns[0][0][lv][0] > r8bfms) {
            bufr_pres_lv[lv] = fill_value;
            bufr_msl_lv[lv]  = fill_value;
            continue;
         }
         else {
      
            // Get the event index to be used for the pressure
            // observation
            ev = get_event_index(conf_info.event_stack_flag, 0, lv);
      
            // Retain the pressure in hPa for each observation record
            obs_arr[2] = evns[0][ev][lv][0];
      
            // Get the event index to be used for the height
            // observation
            ev = get_event_index(conf_info.event_stack_flag, 3, lv);
      
            // Retain the vertical height for each observation record
            if(evns[3][ev][lv][0] > r8bfms) {
               obs_arr[3] = fill_value;
            }
            else {
               // Convert from geopotential height to MSL
               obs_arr[3] = convert_gpm_to_msl(evns[3][ev][lv][0],
                                               hdr_arr_lat);
            }
            bufr_pres_lv[lv] = obs_arr[2];
            bufr_msl_lv[lv]  = obs_arr[3];
         }
      
         // Initialize the P, Q, T, Z, U, V variables
         for(i=0; i<mxr8vt; i++){
            pqtzuv[i]     = fill_value;
            pqtzuv_qty[i] = fill_value;
         }
      
         // Index through the variable types 'P, Q, T, Z, U, V'
         for(kk=0; kk<mxr8vt; kk++) {
      
            // Convert the observation variable index to the
            // corresponding grib code and store as the second element
            // of obs_arr
            grib_code = var_gc[kk];
            obs_arr[1] = (float)(use_var_id ? bufr_var_code[kk] : grib_code);
      
            // Get the event index to be used based on the contents of
            // the event stack flag
            ev = get_event_index(conf_info.event_stack_flag, kk, lv);
      
            // Apply special logic to avoid virtual temperature observations
            if(kk == 2) {
      
               ev_temp = get_event_index_temp(conf_info.event_stack_flag, kk, lv);
      
               // Check for bad data
               if(is_bad_data(ev_temp)) {
      
                  mlog << Debug(4)
                       << "For " << hdr_typ << ", station id " << hdr_sid
                       << ", pressure " << obs_arr[2] << " mb, and height "
                       << obs_arr[3] << " msl, skipping virtual temperature observation ("
                       << evns[kk][ev][lv][0] << " C) with program code "
                       << evns[kk][ev][lv][2] << " and reason code "
                       << evns[kk][ev][lv][3] << ".\n";
                  continue;
               }
      
               // Check for the event index changing
               else if(ev != ev_temp) {
                  mlog << Debug(4)
                       << "For " << hdr_typ << ", station id " << hdr_sid
                       << ", pressure " << obs_arr[2] << " mb, and height "
                       << obs_arr[3] << " msl, selected sensible temperature ("
                       << evns[kk][ev_temp][lv][0]
                       << " C) from event index " << ev_temp
                       << " instead of virtual temperature ("
                       << evns[kk][ev][lv][0]
                       << " C) from event index " << ev << ".\n";
                  ev = ev_temp;
               }
            }
      
            // If the observation value or the quality mark is not
            // valid, continue to the next variable type
            if(evns[kk][ev][lv][0] > r8bfms ||
               evns[kk][ev][lv][1] > r8bfms) {
               continue;
            }
            // Get the actual observation value and quality mark
            else {
               obs_arr[4]   = (float) evns[kk][ev][lv][0];
               quality_mark = (float) evns[kk][ev][lv][1];
            }
      
            // Retrieve the data level category from the top of the
            // event stack: ev = 0
            if(evns[kk][0][lv][6] > r8bfms) {
               dl_category = fill_value;
            }
            else {
               dl_category = (float) evns[kk][0][lv][6];
            }
      
            if(!is_eq(obs_arr[4], fill_value)) {
               // Convert pressure from millibars to pascals
               if(grib_code == pres_grib_code) {
                  obs_arr[4] *= pa_per_mb;
               }
               // Convert specific humidity from mg/kg to kg/kg
               else if(grib_code == spfh_grib_code) {
                  obs_arr[4] *= kg_per_mg;
               }
               // Convert temperature from celcius to kelvin
               else if(grib_code == tmp_grib_code) {
                  obs_arr[4] += c_to_k;
               }
            }
      
            // If the quality mark is greater than than the quality
            // mark threshold in the configuration file
            // continue to the next observation event
            if(conf_info.quality_mark_thresh < quality_mark)
               continue;
      
            // If the data level category is not listed in the
            // configuration file and it is not the case that
            // all data level categories should be retained,
            // continue to the next event
            if(!keep_level_category(nint(dl_category))) continue;
      
            // Store the observation values from which other
            // variables may be derived
            pqtzuv[kk]     = obs_arr[4];
            pqtzuv_qty[kk] = quality_mark;
      
            // If the grib code corrsponding to the observation
            // variable is not listed in the configuration file
            // continue to the next observation variable
            if(!keep_obs_grib_code(grib_code)) continue;
      
            // Write the quality flag to the netCDF file
            ConcatString quality_mark_str;
            quality_mark_str.format("%d", nint(quality_mark));
      
            strncpy(qty_data_buf[obs_data_idx], quality_mark_str, quality_mark_str.length());
            qty_data_buf[obs_data_idx][quality_mark_str.length()] = bad_data_char;
            //qty_data_idx++;
      
            // Write the observation array to the netCDF file
            for (int idx=0; idx<obs_arr_len; idx++) {
               obs_data_buf[obs_data_idx][idx] = obs_arr[idx];
            }
      
            obs_data_idx++;
            if (obs_data_idx >= OBS_BUFFER_SIZE) {
               lengths[1] = strl_len;
               if(!put_nc_data(&obs_qty_var, (char*)qty_data_buf[0], lengths, offsets)) {
                  mlog << Error << "\n" << method_name << " -> "
                       << "error writing the quality flag to the "
                       << "netCDF file\n\n";
                  exit(1);
               }
               lengths[1] = obs_arr_len;
               if(!put_nc_data(&obs_arr_var, (float*)obs_data_buf[0], lengths, offsets)) {
                  mlog << Error << "\n" << method_name << " -> "
                       << "error writing the observation array to the "
                       << "netCDF file\n\n";
                  exit(1);
               }
               offsets[0] += OBS_BUFFER_SIZE;
               obs_data_idx = 0;
            }
      
            // Increment the current and total observations counts
            n_file_obs++;
            n_total_obs++;
      
            // Increment the number of obs counter for this header
            n_hdr_obs++;
         } // end for kk
      
         // Reset obs_arr[1] and obs_arr[4] to fill_value
         obs_arr[1] = fill_value; // grib code
         obs_arr[4] = fill_value; // observation value
      
         // Derive quantities which can be derived from
         // P, Q, T, Z, U, V
         for(i=0; i<n_derive_gc; i++) {
      
            if(keep_obs_grib_code(derive_gc[i])) {
      
               // Only derive PRMSL for message types ADPSFC and SFCSHP
               // which are stored in the onlysf_msg_typ_str
               if(derive_gc[i] == prmsl_grib_code &&
                  strstr(onlysf_msg_typ_str, hdr_typ) == NULL)
                  continue;
      
               // Store the grib code to be derived
               obs_arr[1] = (float)(use_var_id ? bufr_derive_code[i] :derive_gc[i]);

               // Derive the value for the grib code
               obs_arr[4] = derive_grib_code(derive_gc[i], pqtzuv,
                                             pqtzuv_qty, hdr_arr_lat,
                                             quality_mark);
      
               if(is_eq(obs_arr[4], fill_value)) continue;
      
               // Write the quality flag to the netCDF file
               lengths[1] = strl_len;
               ConcatString quality_mark_str;
               quality_mark_str.format("%d", nint(quality_mark));
      
               strncpy(qty_data_buf[obs_data_idx], quality_mark_str, quality_mark_str.length());
               qty_data_buf[obs_data_idx][quality_mark_str.length()] = bad_data_char;
      
               // Write the observation array to the netCDF file
               for (int idx=0; idx<obs_arr_len; idx++) {
                  obs_data_buf[obs_data_idx][idx] = obs_arr[idx];
               }
      
               obs_data_idx++;
               if (obs_data_idx >= OBS_BUFFER_SIZE) {
                  lengths[1] = strl_len;
                  if(!put_nc_data(&obs_qty_var, (char*)qty_data_buf[0], lengths, offsets)) {
                     mlog << Error << "\n" << method_name << " -> "
                          << "error writing the quality flag to the "
                          << "netCDF file\n\n";
                     exit(1);
                  }
                  lengths[1] = obs_arr_len;
                  if(!put_nc_data(&obs_arr_var, (float*)obs_data_buf[0], lengths, offsets)) {
                     mlog << Error << "\n" << method_name << " -> "
                          << "error writing the observation array to the "
                          << "netCDF file\n\n";
                     exit(1);
                  }
                  offsets[0] += OBS_BUFFER_SIZE;
                  obs_data_idx = 0;
               }
      
               // Increment the current and total observations counts
               n_file_obs++;
               n_total_obs++;
      
               // Increment the number of obs counter for this header
               n_hdr_obs++;
            }
         } // end for i
      } // end for lv
      
      if (do_all_vars || use_met_vars) {
         //quality_mark = 0;
         ConcatString quality_mark_str;
         //quality_mark_str.format("%d", nint(quality_mark));
         quality_mark_str.add("NA");
         
         bool isDegC;
         bool isMgKg;
         bool isMilliBar;
         int var_index;
         int n_other_file_obs  = 0;
         int n_other_total_obs = 0;
         int n_other_hdr_obs   = 0;
         int var_count = bufr_obs_arr.n_elements();
         for (int vIdx=0; vIdx<var_count; vIdx++) {
            int nlev2;
            char *var_name;
            int var_name_len;
            var_name = (char *)bufr_obs_arr[vIdx];
            var_name_len = strlen(var_name);
            if (is_prepbufr &&
                  (prepbufr_vars.has(var_name)
                   || prepbufr_event_members.has(var_name)
                   || prepbufr_derive_vars.has(var_name))) continue;
            
            isDegC = false;
            isMgKg = false;
            isMilliBar = false;
            char unit_str[BUFR_UNIT_LEN];
            if (var_names.has(var_name, var_index)) {
               //mlog << Debug(7) <<  "  var name  " << var_name << ", unit str=" << var_units[var_index] << "\n";
               if (0 == strcmp("DEG C", var_units[var_index])) {
                  isDegC = true;
               }
               else if (0 == strcmp("MB", var_units[var_index])) {
                  isMilliBar = true;
               }
               else if (0 == strcmp("MG/KG", var_units[var_index])) {
                  isMgKg = true;
               }
            }
             
            readpbint_(&unit, &i_ret, &nlev2, bufr_obs, var_name, &var_name_len);
            if (0 >= nlev2) continue;
         
            buf_nlev = nlev2;
            if (nlev2 > mxr8lv) {
               buf_nlev = mxr8lv;
               if (!variables_big_nlevels.has(bufr_obs_arr[vIdx])) {
                  mlog << Warning << "\n" << method_name << " -> "
                       << "Too many vertical levels (" << nlev2
                       << ") for " << bufr_obs_arr[vIdx] 
                       << ". Ignored the vertical levels above " << mxr8lv << ".\n\n";
                  variables_big_nlevels.add(bufr_obs_arr[vIdx]);
               }
            }
            mlog << Debug(10) << "var: " << var_name << " nlev2: " << nlev2 << ", vIdx: " << vIdx << ", obs_data_idx: " << obs_data_idx << ", nlev: " << nlev << "\n";
            // Search through the vertical levels
            for(lv=0; lv<buf_nlev; lv++) {
         
               // If the observation vertical level is not within the
               // specified valid range, continue to the next vertical level
               if((lv+1) < conf_info.beg_level || (lv+1) > conf_info.end_level) continue;
         
               // If the pressure level is not valid, continue to the next level
               if (is_prepbufr) {
                  if ((lv >= nlev) || is_eq(bufr_pres_lv[lv], fill_value)) continue;
               }
         
               if (bufr_obs[lv][0] > r8bfms) continue;
               mlog << Debug(10) << " value:   bufr_obs[" << lv << "][0]: " << bufr_obs[lv][0] << "\n";
                  
               obs_arr[1] = (float)vIdx;        // Burf variable index
               obs_arr[4] = bufr_obs[lv][0];    // observation value
               if(!is_eq(obs_arr[4], fill_value)) {
                  // Convert pressure from millibars to pascals
                  if (isMilliBar) {
                     obs_arr[4] *= pa_per_mb;
                  }
                  // Convert specific humidity from mg/kg to kg/kg
                  else if(isMgKg) {
                     obs_arr[4] *= kg_per_mg;
                  }
                  // Convert temperature from celcius to kelvin
                  else if(isDegC) {
                     obs_arr[4] += c_to_k;
                  }
               }
               
               // Retain the pressure in hPa for each observation record
               obs_arr[2] = bufr_pres_lv[lv];
               obs_arr[3] = bufr_msl_lv[lv];
               if (is_eq(obs_arr[2], fill_value) && is_eq(obs_arr[3], fill_value) && 0 < nlev2) {
                  obs_arr[2] = lv;
                  obs_arr[3] = lv;
               }
               
               // Write the quality flag to the netCDF file
               //ConcatString quality_mark_str;
               //quality_mark_str.format("%d", nint(quality_mark));
               
               strncpy(qty_data_buf[obs_data_idx], quality_mark_str, quality_mark_str.length());
               qty_data_buf[obs_data_idx][quality_mark_str.length()] = bad_data_char;
               //qty_data_idx++;
         
               for (int idx=0; idx<obs_arr_len; idx++) {
                  obs_data_buf[obs_data_idx][idx] = obs_arr[idx];
               }
               
               obs_data_idx++;
               if (obs_data_idx >= OBS_BUFFER_SIZE) {
                  lengths[1] = strl_len;
                  if(!put_nc_data(&obs_qty_var, (char*)qty_data_buf[0], lengths, offsets)) {
                     mlog << Error << "\n" << method_name << " -> "
                          << "error writing the quality flag to the "
                          << "netCDF file\n\n";
                     exit(1);
                  }
                  lengths[1] = obs_arr_len;
                  if(!put_nc_data(&obs_arr_var, (float*)obs_data_buf[0], lengths, offsets)) {
                     mlog << Error << "\n" << method_name << " -> "
                          << "error writing the observation array to the "
                          << "netCDF file\n\n";
                     exit(1);
                  }
                  offsets[0] += OBS_BUFFER_SIZE;
                  obs_data_idx = 0;
               }                  
               
               // Increment the current and total observations counts
               n_other_file_obs++;
               n_other_total_obs++;
               
               // Increment the number of obs counter for this header
               n_other_hdr_obs++;
            }
         }
         n_hdr_obs += n_other_hdr_obs;
         n_file_obs += n_other_file_obs;
         n_total_obs += n_other_total_obs;
      }
      
      // If the number of observations for this header is non-zero,
      // store the header data and increment the PrepBufr record
      // counter
      if(n_hdr_obs > 0) {
         if (_messageTypeMap[hdr_typ]) {
            ConcatString mappedMessageType = _messageTypeMap[hdr_typ];
            mlog << Debug(5) << "\n" << method_name << " -> "
                 << "Switching report type \"" << hdr_typ
                 << "\" to message type \"" << mappedMessageType << "\".\n";
            if (mappedMessageType.length() < strl_len) {
               hdr_typ_sa.add(mappedMessageType);
            }
            else {
               char tmp_message_type[strl_len];
               strncpy(tmp_message_type, mappedMessageType.text(), strl_len);
               hdr_typ_sa.add(tmp_message_type);
            }
         }
         else {
            hdr_typ_sa.add(hdr_typ);
         }
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

   if (obs_data_idx > 0) {
      lengths[0] = obs_data_idx;
      lengths[1] = strl_len;
      if(!put_nc_data(&obs_qty_var, (char*)qty_data_buf[0], lengths, offsets)) {
         mlog << Error << "\n" << method_name << " -> "
              << "error writing the quality flag to the "
              << "netCDF file\n\n";
         exit(1);
      }
      lengths[1] = obs_arr_len;
      if(!put_nc_data(&obs_arr_var, (float*)obs_data_buf[0], lengths, offsets)) {
         mlog << Error << "\n" << method_name << " -> "
              << "error writing the observation array to the "
              << "netCDF file\n\n";
         exit(1);
      }
   }

   if(mlog.verbosity_level() > 0) cout << "\n" << flush;
   
   mlog << Debug(2) << "Total " << (is_prepbufr ? "PrepBufr" : "Bufr")
        << " Messages processed\t= " << npbmsg << "\n"
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
        << "Total " << (is_prepbufr ? "PrepBufr" : "Bufr") << " Messages retained\t= "
        << i_msg << "\n"
        << "Total observations retained or derived\t= "
        << n_file_obs << "\n";

   if (npbmsg == rej_vld && 0 < rej_vld) {
      mlog << Warning << "\n" << method_name << " -> "
           << "All messages were filtered out by valid time.\n"
           << "\tPlease adjust time range with \"-valid_beg\" and \"-valid_end\".\n"
           << "\tmin/max obs time from BUFR file: " << min_time_str 
           << " and " << max_time_str << ".\n"
           << "\ttime range: " << start_time_str << " and " << end_time_str << ".\n";
   }
   else {
      mlog << Debug(3) << "\tMin obs_time = " << min_time_str 
           << "\tMax obs_time = " << max_time_str << "\n";
       
      int debug_level = 5;
      if(mlog.verbosity_level() >= debug_level) {
         for (kk=0; kk<filtered_times.n_elements();kk++) {
            unix_to_mdyhms(filtered_times[kk], mon, day, yr, hr, min, sec);
            sprintf(time_str, "%.4i%.2i%.2i_%.2i%.2i%.2i",
                        yr, mon, day, hr, min, sec);
            mlog << Debug(debug_level) << "\t Filtered time: " << time_str
                 << " (" << filtered_times[kk] << ")\n";
         }
         cout << endl;
      }
   }

   // Close the PREPBUFR file
   closepb_(&unit);

   // Delete the temporary blocked file
   remove_temp_file(blk_file);
   if(mlog.verbosity_level() >= debug_level_for_performance) {
      method_end = clock();
      cout << method_name << " " << (method_end-method_start)/double(CLOCKS_PER_SEC) << " seconds" << endl;
   }

   if(i_msg <= 0) {
      mlog << Warning << "\n" << method_name << " -> "
           << "No " << (is_prepbufr ? "PrepBufr" : "Bufr") << " messages retained from file: "
           << pbfile[i_pb] << "\n\n";
      return;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_pbfile_metadata(int i_pb) {
   int npbmsg, unit, yr, mon, day, hr, min, sec;
   int i, i_msg, i_read, i_ret, i_date, n_hdr_obs;
   int lv, ev, ev_temp, kk, len1, len2, var_index;
   int debug_threshold = 10;

   double   x, y;

   bool check_all = do_all_vars || collect_metadata;
   unixtime file_ut = (unixtime) 0;
   char     time_str[max_str_len];
   char     hdr_typ[max_str_len];
   StringArray local_bufr_obs_arr;
   ConcatString file_name, blk_prefix, blk_file;
   static const char *method_name = "process_pbfile_metadata()";

   // Collects the prepbufr variables including headers
   bufr_hdr_arr.clear();
   
   // List the PrepBufr file being processed
   mlog << Debug(1) << "\nPre-processing Bufr File for metadata "
                    << "(BUFR variable names) from " << pbfile[i_pb] << "\n\n";

   // Set the file name for the PrepBufr file
   file_name << pbfile[i_pb];

   // Build the temporary block file name
   blk_prefix << conf_info.tmp_dir << "/" << "tmp_pb2nc_blk";
   blk_file = make_temp_file_name(blk_prefix, '\0');

   mlog << Debug(3) << "   Blocking Bufr file (metadata) to:\t" << blk_file << "\n";

   // Assume that the input PrepBufr file is unblocked.
   // Block the PrepBufr file and open it for reading.
   pblock(file_name, blk_file, block);

   unit = dump_unit + i_pb;
   ConcatString tbl_filename = save_bufr_table_to_file(blk_file, unit);
   get_variable_info(tbl_filename);
   if(mlog.verbosity_level() < debug_threshold) remove_temp_file(tbl_filename);

   if (unit > MAX_FORTRAN_FILE_ID || unit < MIN_FORTRAN_FILE_ID) {
      mlog << Error << "\n" << method_name << " -> "
           << "Invalid file ID [" << unit << "] between 1 and 99.\n\n";
   }
   // Open the blocked temp PrepBufr file for reading
   openpb_(blk_file, &unit);

   // Compute the number of PrepBufr records in the current file.
   numpbmsg_(&unit, &npbmsg);

   // Use the number of records requested by the user if there
   // are enough present.
   if(nmsg >= 0 && nmsg <= npbmsg) npbmsg = nmsg;

   // Check for zero messages to process
   if(npbmsg <= 0) {
      mlog << Warning << "\n" << method_name << " -> "
           << "No Bufr messages to process in file: "
           << pbfile[i_pb] << "\n\n";

      closepb_(&unit);
      
      // Delete the temporary blocked file
      remove_temp_file(blk_file);

      return;
   }

   // Initialize counts
   i_ret   =  i_msg     = 0;

   int valid_data_count = 0;
   bool is_prepbufr = is_prepbufr_file(&event_names);

   //
   StringArray headers;
   StringArray tmp_hdr_array;
   headers.add(prepbufr_hdrs);
   tmp_hdr_array.clear();
   tmp_hdr_array.parse_wsss(bufr_avail_sid_names);
   headers.add(tmp_hdr_array);
   tmp_hdr_array.clear();
   tmp_hdr_array.parse_wsss(bufr_avail_latlon_names);
   headers.add(tmp_hdr_array);
   //headers.dump(cout);
   
   StringArray unchecked_var_list;
   ConcatString bufr_hdr_strings;
   for(i=0; i<tableB_vars.n_elements(); i++) {
      if (!headers.has(tableB_vars[i]) && check_all) {
         unchecked_var_list.add(tableB_vars[i]);
      }
   }
   //Initialize index for prepbufr common variables
   for (int index=0; index<mxr8vt; index++) {
      bufr_var_code[index] = -1;
   }
   for (int index=0; index<n_derive_gc; index++) {
      bufr_derive_code[index] = -1;
   }

   int index;
   int length;
   int var_idx;
   bool is_prepbufr_hdr = false;
   // Loop through the PrepBufr messages from the input file
   for(i_read=0; i_read<npbmsg && i_ret == 0; i_read++) {

      if(mlog.verbosity_level() > 0) {
         if(nint(npbmsg/20.0) > 0 && (i_read+1)%nint(npbmsg/20.0) == 0) {
            cout << nint((double) (i_read+1)/npbmsg*100.0) << "% " << flush;
         }
      }

      // Get the next PrepBufr message
      ireadns_(&unit, hdr_typ, &i_date);
      
      if (0 == i_read) {
         // Checks the varaibles for header
         
         char tmp_str[mxr8lv*mxr8pm];
         char var_name[BUFR_NAME_LEN+1];
         ConcatString hdr_name_str;

         strcpy(tmp_str, prepbufr_hdrs_str);
         length = strlen(tmp_str);
         readpbint_(&unit, &i_ret, &nlev, bufr_obs, tmp_str, &length );
         is_prepbufr_hdr = (0 < nlev);
         if (is_prepbufr_hdr) {
            for (index=0; index<4; index++) {
               if (bufr_obs[0][index] > r8bfms) {
                  is_prepbufr_hdr = false;
                  break;
               }
            }
         }
         
         //bufr_hdr_arr.clear();
         if (is_prepbufr_hdr) {
            tmp_hdr_array.clear();
            tmp_hdr_array.parse_wsss(prepbufr_hdrs_str);
            for (index=0; index<tmp_hdr_array.n_elements(); index++) {
               if (!bufr_hdr_arr.has(tmp_hdr_array[index])) bufr_hdr_arr.add(tmp_hdr_array[index]);
            }
         }
         else {
            strcpy(var_name, default_sid_name);
            strcpy(tmp_str, bufr_avail_sid_names);
            length = strlen(tmp_str);
            readpbint_(&unit, &i_ret, &nlev, bufr_obs, tmp_str, &length );
            if (0 < nlev) {
               tmp_hdr_array.clear();
               tmp_hdr_array.parse_wsss(bufr_avail_sid_names);
               for (index=0; index<tmp_hdr_array.n_elements(); index++) {
                  if (bufr_obs[0][index] < r8bfms) {
                     strcpy(var_name, tmp_hdr_array[index]);
                     if (!bufr_hdr_arr.has(var_name)) bufr_hdr_arr.add(var_name);
                     mlog << Debug(10) << "found station id: " << var_name << "=" << bufr_obs[0][index] << "\n";
                  }
               }
            }
            if (0 < hdr_name_str.length()) hdr_name_str.add(" ");
            hdr_name_str.add(var_name);
            
            strcpy(tmp_str, bufr_avail_latlon_names);
            length = strlen(tmp_str);
            readpbint_(&unit, &i_ret, &nlev, bufr_obs, tmp_str, &length );
            if (0 < nlev) {
               tmp_hdr_array.clear();
               tmp_hdr_array.parse_wsss(bufr_avail_latlon_names);
               strcpy(var_name, default_lon_name);
               for (index=0; index<(tmp_hdr_array.n_elements()/2); index++) {
                  if (bufr_obs[0][index] < r8bfms) {
                     strcpy(var_name, tmp_hdr_array[index]);
                     if (!bufr_hdr_arr.has(var_name)) bufr_hdr_arr.add(var_name);
                     mlog << Debug(10) << "found  longitude: " << var_name << "=" << bufr_obs[0][index] << "\n";
                  }
               }
               if (0 < hdr_name_str.length()) hdr_name_str.add(" ");
               hdr_name_str.add(var_name);
            
               strcpy(var_name, default_lat_name);
               for (index=(tmp_hdr_array.n_elements()/2); index<tmp_hdr_array.n_elements(); index++) {
                  if (bufr_obs[0][index] < r8bfms) {
                     strcpy(var_name, tmp_hdr_array[index]);
                     if (!bufr_hdr_arr.has(var_name)) bufr_hdr_arr.add(var_name);
                     mlog << Debug(10) << "found   latitude: " << var_name << "=" << bufr_obs[0][index] << "\n";
                  }
               }
               if (0 < hdr_name_str.length()) hdr_name_str.add(" ");
               hdr_name_str.add(var_name);
            }
            else {
               if (0 < hdr_name_str.length()) hdr_name_str.add(" ");
               if (!bufr_hdr_arr.has(default_lon_name)) hdr_name_str.add(default_lon_name);
               if (0 < hdr_name_str.length()) hdr_name_str.add(" ");
               if (!bufr_hdr_arr.has(default_lat_name)) hdr_name_str.add(default_lat_name);
            }
            
            ConcatString time_hdr_names;
            time_hdr_names.add("DHR");
            
            time_hdr_names.add(" ");
            if (event_names.has("YYMMDD", index)) {
               time_hdr_names.add(event_members[index]);
               tmp_hdr_array.clear();
               tmp_hdr_array.parse_wsss(event_members[index]);
               for (index=0; index<tmp_hdr_array.n_elements(); index++) {
                  if (!bufr_hdr_arr.has(tmp_hdr_array[index])) bufr_hdr_arr.add(tmp_hdr_array[index]);
               }
            }
            else {
               time_hdr_names.add(default_ymd_name);
               tmp_hdr_array.clear();
               tmp_hdr_array.parse_wsss(default_ymd_name);
               for (index=0; index<tmp_hdr_array.n_elements(); index++) {
                  if (!bufr_hdr_arr.has(tmp_hdr_array[index])) bufr_hdr_arr.add(tmp_hdr_array[index]);
               }
            }
            
            time_hdr_names.add(" ");
            if (event_names.has("HHMMSS", index)) {
               time_hdr_names.add(event_members[index]);
               tmp_hdr_array.clear();
               tmp_hdr_array.parse_wsss(event_members[index]);
               for (index=0; index<tmp_hdr_array.n_elements(); index++) {
                  if (!bufr_hdr_arr.has(tmp_hdr_array[index])) bufr_hdr_arr.add(tmp_hdr_array[index]);
               }
            }
            else if (event_names.has("HHMM", index)) {
               time_hdr_names.add(event_members[index]);
               time_hdr_names.add(" SECO");
               tmp_hdr_array.clear();
               tmp_hdr_array.parse_wsss(event_members[index]);
               for (index=0; index<tmp_hdr_array.n_elements(); index++) {
                  if (!bufr_hdr_arr.has(tmp_hdr_array[index])) bufr_hdr_arr.add(tmp_hdr_array[index]);
               }
               if (!bufr_hdr_arr.has("SECO")) bufr_hdr_arr.add("SECO");
            }
            else {
               time_hdr_names.add(default_hms_name);
               tmp_hdr_array.clear();
               tmp_hdr_array.parse_wsss(default_hms_name);
               for (index=0; index<tmp_hdr_array.n_elements(); index++) {
                  if (!bufr_hdr_arr.has(tmp_hdr_array[index])) bufr_hdr_arr.add(tmp_hdr_array[index]);
               }
            }
            
            if (0 < hdr_name_str.length()) hdr_name_str.add(" ");
            hdr_name_str.add(time_hdr_names);
         }
         
         bufr_hdrs.clear();
         if (is_prepbufr_hdr) {
            bufr_hdrs.add(prepbufr_hdrs_str);
         }
         else {
            bufr_hdrs.add(hdr_name_str);
         }
         
         if (check_all) {
            for (index=0; index<bufr_hdr_arr.n_elements(); index++) {
               if (unchecked_var_list.has(bufr_hdr_arr[index], var_index)) {
                  unchecked_var_list.shift_down(var_index, 1);
               }
            }
         }
         
         if (use_met_vars) {
            StringArray taregt_vars = bufr_target_variables.split(",+");
            for (index=0; index<taregt_vars.n_elements(); index++) {
               if (!tableB_vars.has(taregt_vars[index])) {
                  mlog << Error << "\n" << method_name << " -> variable \""
                       << taregt_vars[index] <<"\" does not exist at BUFR file\n\n";
                  exit(1);
               }
               if (!local_bufr_obs_arr.has(taregt_vars[index])) {
                  local_bufr_obs_arr.add(taregt_vars[index]);
               }
            }
         }
         if (!check_all) break;
      } // if (0 == i_read)

      if (0 == unchecked_var_list.n_elements()) break;

      int var_count = unchecked_var_list.n_elements();
      for (int vIdx=var_count-1; vIdx>=0; vIdx--) {
         int nlev2, buf_nlev;
         int var_name_len;
         char var_name[BUFR_NAME_LEN];
         strcpy(var_name, (char *)unchecked_var_list[vIdx]);
         var_name_len = strlen(var_name);
            
         readpbint_(&unit, &i_ret, &nlev2, bufr_obs, var_name, &var_name_len);
         if (0 >= nlev2) continue;
      
         // Search through the vertical levels
         buf_nlev = nlev2;
         if (nlev2 > mxr8lv) buf_nlev = mxr8lv;
         for(lv=0; lv<buf_nlev; lv++) {
            if (bufr_obs[lv][0] < r8bfms) {
               if (!local_bufr_obs_arr.has(var_name) && !bufr_hdr_arr.has(var_name)) {
                  local_bufr_obs_arr.add(var_name);
                  if (prepbufr_vars.has(var_name, var_idx)) {
                     bufr_var_code[var_idx] = vIdx;
                  }
               }
               unchecked_var_list.shift_down(vIdx, 1);
               mlog << Debug(5) << " found valid data: " << var_name
                    << " (" << bufr_obs[lv][0] << ")\n";
               break;
            }
         }  //end for lv
      }  // end for vIdx
   } // end for

   // Close the PREPBUFR file
   closepb_(&unit);
   
   bufr_obs_arr.clear();
   if (is_prepbufr) {
      for (index=0; index<prepbufr_vars.n_elements(); index++) {
         if (tableB_vars.has(prepbufr_vars[index])) {
            bufr_obs_arr.add(prepbufr_vars[index]);
            bufr_var_code[index] = index;
         }
      }
      length = bufr_obs_arr.n_elements();
      for (int vIdx=0; vIdx< prepbufr_derive_vars.n_elements(); vIdx++) {
         bufr_derive_code[vIdx] = length + vIdx;
         bufr_obs_arr.add(prepbufr_derive_vars[vIdx]);
      }
   }
   for (i=0; i<local_bufr_obs_arr.n_elements(); i++) {
      if (!bufr_obs_arr.has(local_bufr_obs_arr[i])) {
         bufr_obs_arr.add(local_bufr_obs_arr[i]);
      }
   }

   // Delete the temporary blocked file
   remove_temp_file(blk_file);
   return;
}

////////////////////////////////////////////////////////////////////////

void write_netcdf_hdr_data() {
   int i, dim_count, buf_len;
   float hdr_arr[hdr_arr_len];
   bool is_prepbufr = is_prepbufr_file(&event_names);
   static const char *method_name = "write_netcdf_hdr_data()";

   // Check for no messages retained
   if(hdr_typ_sa.n_elements() <= 0) {
      mlog << Error << "\nwrite_netcdf_hdr_data() -> "
           << "No PrepBufr messages retained.  Nothing to write.\n\n";

      // Delete the NetCDF file
      if(remove(ncfile) != 0) {
         mlog << Error << "\nwrite_netcdf_hdr_data() -> "
              << "can't remove output NetCDF file \"" << ncfile
              << "\"\n\n";
         exit(1);
      }
      exit(1);
   }

   // Define netCDF dimensions
   hdr_dim = add_dim(f_out, nc_dim_nhdr, (long) hdr_typ_sa.n_elements());

   dim_count = bufr_obs_arr.n_elements();
   bufr_var_dim  = add_dim(f_out, nc_dim_nvar, (long)dim_count);
   bufr_unit_dim = add_dim(f_out, nc_dim_unit, (long)BUFR_UNIT_LEN);
   bufr_desc_dim = add_dim(f_out, nc_dim_desc, (long)BUFR_DESCRIPTION_LEN);

   
   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = conf_info.conf.nc_compression();

   // Define netCDF variables
   hdr_typ_var = add_var(f_out, "hdr_typ", ncChar,  hdr_dim, strl_dim,    deflate_level);
   hdr_sid_var = add_var(f_out, "hdr_sid", ncChar,  hdr_dim, strl_dim,    deflate_level);
   hdr_vld_var = add_var(f_out, "hdr_vld", ncChar,  hdr_dim, strl_dim,    deflate_level);
   hdr_arr_var = add_var(f_out, "hdr_arr", ncFloat, hdr_dim, hdr_arr_dim, deflate_level);

   bufr_obs_var  = add_var(f_out, nc_var_obs_var, ncChar, bufr_var_dim,      strl_dim, deflate_level);
   bufr_unit_var = add_var(f_out,    nc_var_unit, ncChar, bufr_var_dim, bufr_unit_dim, deflate_level);
   bufr_desc_var = add_var(f_out,    nc_var_desc, ncChar, bufr_var_dim, bufr_desc_dim, deflate_level);
   
   add_att(&hdr_typ_var, "long_name", "message type");
   add_att(&hdr_sid_var, "long_name", "station identification");
   add_att(&hdr_vld_var, "long_name", "valid time");
   add_att(&hdr_vld_var, "units", "YYYYMMDD_HHMMSS");

   add_att(&hdr_arr_var, "long_name",
                        "array of observation station header values");
   add_att(&hdr_arr_var, "_fill_value", fill_value);
   add_att(&hdr_arr_var, "columns", "lat lon elv");
   add_att(&hdr_arr_var, "lat_long_name", "latitude");
   add_att(&hdr_arr_var, "lat_units", "degrees_north");
   add_att(&hdr_arr_var, "lon_long_name", "longitude");
   add_att(&hdr_arr_var, "lon_units", "degrees_east");
   add_att(&hdr_arr_var, "elv_long_name", "elevation");
   add_att(&hdr_arr_var, "elv_units", "meters above sea level (msl)");

   add_att(&bufr_obs_var,  "long_name", "BUFR variable names from Table B");
   add_att(&bufr_unit_var, "long_name", "BUFR variable units");
   add_att(&bufr_desc_var, "long_name", "BUFR variable descriptions");
   
   long offsets[2] = { 0, 0 };
   long lengths[2] = { OBS_BUFFER_SIZE, strl_len } ;

   // Loop through and write out the header data

   int hdr_idx = 0;
   int hdr_str_len;
   for(i=0; i<hdr_typ_sa.n_elements(); i++) {

      // PrepBufr Message type
      hdr_str_len = strlen(hdr_typ_sa[i]);
      if (hdr_str_len > strl_len) hdr_str_len = strl_len;
      strncpy(hdr_typ_buf[hdr_idx], hdr_typ_sa[i], hdr_str_len);
      hdr_typ_buf[hdr_idx][hdr_str_len] = bad_data_char;

      // Station ID
      hdr_str_len = strlen(hdr_sid_sa[i]);
      if (hdr_str_len > strl_len) hdr_str_len = strl_len;
      strncpy(hdr_sid_buf[hdr_idx], hdr_sid_sa[i], hdr_str_len);
      hdr_sid_buf[hdr_idx][hdr_str_len] = bad_data_char;

      // Valid Time
      hdr_str_len = strlen(hdr_vld_sa[i]);
      if (hdr_str_len > strl_len) hdr_str_len = strl_len;
      strncpy(hdr_vld_buf[hdr_idx], hdr_vld_sa[i], hdr_str_len);
      hdr_vld_buf[hdr_idx][hdr_str_len] = bad_data_char;

      // Write the header array which consists of the following:
      //    LAT LON ELV
      hdr_arr[0] = (float) hdr_arr_lat_na[i];
      hdr_arr[1] = (float) hdr_arr_lon_na[i];
      hdr_arr[2] = (float) hdr_arr_elv_na[i];

      for (int idx=0; idx<hdr_arr_len; idx++) {
         hdr_arr_buf[hdr_idx][idx] = hdr_arr[idx];
      }

      hdr_idx++;
      if (hdr_idx >= OBS_BUFFER_SIZE) {
         lengths[1] = strl_len;
         if(!put_nc_data(&hdr_typ_var, (char *)hdr_typ_buf[0], lengths, offsets)) {
            mlog << Error << "\nwrite_netcdf_hdr_data() -> "
                 << "error writing the prepbufr message type string to "
                 << "the netCDF file\n\n";
            exit(1);
         }
         
         // Station ID
         if(!put_nc_data(&hdr_sid_var, (char *)hdr_sid_buf[0], lengths, offsets)) {
            mlog << Error << "\nwrite_netcdf_hdr_data() -> "
                 << "error writing the station id string to the "
                 << "netCDF file\n\n";
            exit(1);
         }
         
         // Valid Time
         if(!put_nc_data(&hdr_vld_var, (char *)hdr_vld_buf[0], lengths, offsets)) {
            mlog << Error << "\nwrite_netcdf_hdr_data() -> "
                 << "error writing the valid time to the "
                 << "netCDF file\n\n";
            exit(1);
         }
         
         // Write the header array which consists of the following:
         //    LAT LON ELV
         
         lengths[1] = hdr_arr_len;
         if(!put_nc_data(&hdr_arr_var, (float *)hdr_arr_buf[0], lengths, offsets)) {
            mlog << Error << "\nwrite_netcdf_hdr_data() -> "
                 << "error writing the header array to the "
                 << "netCDF file\n\n";
            exit(1);
         }

         for(int hi=0; hi<OBS_BUFFER_SIZE; hi++) {
            for(int hj=0; hj<strl_len; hj++) {
               hdr_typ_buf[hi][hj] = bad_data_char;
               hdr_sid_buf[hi][hj] = bad_data_char;
            }
         }

         offsets[0] += OBS_BUFFER_SIZE;
         hdr_idx = 0;
      }
   } // end for i


   if (hdr_idx > 0) {
      lengths[0] = hdr_idx;
      lengths[1] = strl_len;
      if(!put_nc_data(&hdr_typ_var, (char *)hdr_typ_buf[0], lengths, offsets)) {
         mlog << Error << "\nwrite_netcdf_hdr_data() -> "
              << "error writing the prepbufr message type string to "
              << "the netCDF file\n\n";
         exit(1);
      }
      
      // Station ID
      if(!put_nc_data(&hdr_sid_var, (char *)hdr_sid_buf[0], lengths, offsets)) {
         mlog << Error << "\nwrite_netcdf_hdr_data() -> "
              << "error writing the station id string to the "
              << "netCDF file\n\n";
         exit(1);
      }
      
      // Valid Time
      if(!put_nc_data(&hdr_vld_var, (char *)hdr_vld_buf[0], lengths, offsets)) {
         mlog << Error << "\nwrite_netcdf_hdr_data() -> "
              << "error writing the valid time to the "
              << "netCDF file\n\n";
         exit(1);
      }
      
      // Write the header array which consists of the following:
      //    LAT LON ELV
      
      lengths[1] = hdr_arr_len;
      if(!put_nc_data(&hdr_arr_var, (float *)hdr_arr_buf[0], lengths, offsets)) {
         mlog << Error << "\nwrite_netcdf_hdr_data() -> "
              << "error writing the header array to the "
              << "netCDF file\n\n";
         exit(1);
      }
   }

   offsets[0] = 0;
   offsets[1] = 0;
   lengths[0] = 1 ;
   StringArray nc_obs_arr;
   nc_obs_arr.add(bufr_obs_arr);
   map<ConcatString, ConcatString> obs_var_map = conf_info.getObsVarMap();
   for(i=0; i<nc_obs_arr.n_elements(); i++) {
      int var_index;
      char var_name[BUFR_NAME_LEN+1];
      char unit_str[BUFR_UNIT_LEN+1];
      char desc_str[BUFR_DESCRIPTION_LEN+1];

      for(int tIdx=0; tIdx<BUFR_NAME_LEN; tIdx++) {
         var_name[tIdx] = bad_data_char;
      }
      for(int tIdx=0; tIdx<BUFR_UNIT_LEN; tIdx++) {
         unit_str[tIdx] = bad_data_char;
      }
      
      hdr_str_len = strlen(nc_obs_arr[i]);
      if (hdr_str_len > BUFR_NAME_LEN) hdr_str_len = BUFR_NAME_LEN;
      strncpy(var_name, nc_obs_arr[i], hdr_str_len);
      var_name[hdr_str_len] = bad_data_char;

      if (var_names.has(var_name, var_index)) {
         if (0 == strcmp("DEG C", var_units[var_index])) {
            strcpy(unit_str, "KELVIN");
         }
         else if (0 == strcmp("MB", var_units[var_index])) {
            strcpy(unit_str, "PASCALS");
         }
         else if (0 == strcmp("MG/KG", var_units[var_index])) {
            strcpy(unit_str, "KG/KG");
         }
         else {
            hdr_str_len = strlen(var_units[var_index]);
            if (hdr_str_len > BUFR_UNIT_LEN) hdr_str_len = BUFR_UNIT_LEN;
            strncpy(unit_str, var_units[var_index], hdr_str_len);
            unit_str[hdr_str_len] = bad_data_char;
         }
      }

      buf_len = 0;
      if (tableB_vars.has(var_name, var_index)) {
         strcpy(desc_str, tableB_descs[var_index]);
         buf_len = strlen(desc_str);
      }
      for(int tIdx=buf_len; tIdx<BUFR_DESCRIPTION_LEN; tIdx++) {
         desc_str[tIdx] = bad_data_char;
      }

      // Variable name
      lengths[1] = BUFR_NAME_LEN;
      if (obs_var_map[var_name]) {
         ConcatString grib_name = obs_var_map[var_name];
         if (0 < grib_name.length()) {
            int name_len = strlen(var_name);
            strcpy(var_name, grib_name);
            for (int idx=grib_name.length(); idx<name_len; idx++)
               var_name[idx] = bad_data_char;
         }
      }
      if(!put_nc_data(&bufr_obs_var, (char *)var_name, lengths, offsets)) {
         mlog << Error << "\nwrite_netcdf_hdr_data() -> "
              << "error writing the BUFR variable name to "
              << "the netCDF file\n\n";
         exit(1);
      }
      
      // Variable unit
      lengths[1] = BUFR_UNIT_LEN;
      if(!put_nc_data(&bufr_unit_var, (char *)unit_str, lengths, offsets)) {
         mlog << Error << "\nwrite_netcdf_hdr_data() -> "
              << "error writing the variable unit to the "
              << "netCDF file\n\n";
         exit(1);
      }
         
      // Variable description
      int buf_len = 0;
      lengths[1] = BUFR_DESCRIPTION_LEN;
      if(!put_nc_data(&bufr_desc_var, (char *)desc_str, lengths, offsets)) {
         mlog << Error << "\nwrite_netcdf_hdr_data() -> "
              << "error writing the variable description to the "
              << "netCDF file\n\n";
         exit(1);
      }
         
      offsets[0] += 1;
   } // end for i
   
   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {

   if(f_out) {
      delete f_out;
      f_out = (NcFile *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

int get_event_index(int flag, int i_var, int i_lvl) {
   int ev, i;

   // Check the event_stack_flag to determine if the top or bottom
   // of the event stack is to be used
   //   Top of the stack:    ev = 0
   //   Bottom of the stack: ev > 0
   if(flag) {
      ev = 0;
   }
   // If the bottom of the event stack is to be used, find the
   // correct event index
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
//
// Special processing for observations of temperature.
// Do not use virtual temperatures observations for verification.
// PREPBUFR Table 14 describes the VIRTMP processing step:
//    http://www.emc.ncep.noaa.gov/mmb/data_processing/prepbufr.doc/table_14.htm
//
// For the bottom of the event stack, skip the observation if it has
// the VIRTMP program code of 8.
//
// For the top of the event stack, search the entire stack looking
// for the VIRTMP program code.  For VIRTMP program code with reason
// code 3, do not use this this observation since all versions are
// virtual temperature.  For VIRTMP program code with any other
// reason code, step down the event stack to find sensible temperature.
//
////////////////////////////////////////////////////////////////////////

int get_event_index_temp(int flag, int i_var, int i_lvl) {
   int ev, i;

   // For the top of the event stack, search the entire stack looking
   // for the VIRTMP program code.  If found, use the next entry.
   if(flag) {

      // Initialize to the top of the event stack
      ev = 0;

      // Loop through the event stack
      for(i=0; i<mxr8vn && evns[i_var][i][i_lvl][0]<r8bfms; i++) {

         // Check for the VIRTMP program code
         if(is_eq(evns[i_var][i][i_lvl][2], virtmp_prog_code)) {

            // Skip this observation if the reason code is 3
            if(is_eq(evns[i_var][i][i_lvl][3], 3.0)) return(bad_data_int);

            // Use the next entry in the event stack but keep searching
            ev = i+1;
         }
      }
   }

   // For the bottom of the event stack, skip the VIRTMP program code
   else {

      // Retrieve the bottom of the event stack like normal
      ev = get_event_index(flag, i_var, i_lvl);

      // Check for the VIRTMP program code
      if(is_eq(evns[i_var][ev][i_lvl][2], virtmp_prog_code)) return(bad_data_int);
   }

   return(ev);
}

////////////////////////////////////////////////////////////////////////

void dbl2str(double *d, char *str) {
   const char *fmt_str = "%s";

   sprintf(str, fmt_str, d);
   if (0 == strlen(str)) {
      //const char *fmt_str_d = "%d";
      //sprintf(str, fmt_str_d, nint(*d));
      strcpy(str, "NA");
   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool keep_message_type(const char *mt_str) {
   bool keep = false;

   keep = conf_info.message_type.n_elements() == 0 ||
          conf_info.message_type.has(mt_str);

   // Handle ANYAIR, ANYSFC, or ONLYSF message types
   if(!keep) {
      if((conf_info.anyair_flag && strstr(anyair_msg_typ_str, mt_str) != NULL) ||
         (conf_info.anysfc_flag && strstr(anysfc_msg_typ_str, mt_str) != NULL) ||
         (conf_info.onlysf_flag && strstr(onlysf_msg_typ_str, mt_str) != NULL)) {
         keep = true;
      }
   }

   return(keep);
}

////////////////////////////////////////////////////////////////////////

bool keep_station_id(const char *sid_str) {

   return(conf_info.station_id.n_elements() == 0 ||
          conf_info.station_id.has(sid_str));
}

////////////////////////////////////////////////////////////////////////

bool keep_valid_time(const unixtime ut,
                     const unixtime min_ut, const unixtime max_ut) {
   bool keep = true;

   // If min_ut and max_ut both set, check the range
   if(min_ut != (unixtime) 0 && max_ut != (unixtime) 0) {
      if(ut < min_ut || ut > max_ut) keep = false;
   }
   // If only min_ut set, check the lower bound
   else if(min_ut != (unixtime) 0 && max_ut == (unixtime) 0) {
      if(ut < min_ut) keep = false;
   }
   // If only max_ut set, check the upper bound
   else if(min_ut == (unixtime) 0 && max_ut != (unixtime) 0) {
      if(ut > max_ut) keep = false;
   }

   return(keep);
}

////////////////////////////////////////////////////////////////////////

bool keep_pb_report_type(int type) {

   return(conf_info.pb_report_type.n_elements() == 0 ||
          conf_info.pb_report_type.has(type));
}

////////////////////////////////////////////////////////////////////////

bool keep_in_report_type(int type) {
   return(conf_info.in_report_type.n_elements() == 0 ||
          conf_info.in_report_type.has(type));
}

////////////////////////////////////////////////////////////////////////

bool keep_instrument_type(int type) {
   return(conf_info.instrument_type.n_elements() == 0 ||
          conf_info.instrument_type.has(type));
}

////////////////////////////////////////////////////////////////////////

bool keep_obs_grib_code(int code) {
   return(conf_info.obs_grib_code.n_elements() == 0 ||
          conf_info.obs_grib_code.has(code));
}

////////////////////////////////////////////////////////////////////////

bool keep_level_category(int category) {
   return(conf_info.level_category.n_elements() == 0 ||
          conf_info.level_category.has(category));
}

////////////////////////////////////////////////////////////////////////

float derive_grib_code(int gc, float *pqtzuv, float *pqtzuv_qty,
                       double lat, float &qty) {
   float result;
   double p, q, t, z, u, v, w, vp;

   switch(gc) {

      // Pressure Reduced to Mean Sea Level
      case(prmsl_grib_code):
         p      = (double) pqtzuv[0];
         t      = (double) pqtzuv[2];
         z      = (double) pqtzuv[3];
         qty    = pqtzuv_qty[0];
         qty    = pqtzuv_qty[2] > qty ? pqtzuv_qty[2] : qty;
         qty    = pqtzuv_qty[3] > qty ? pqtzuv_qty[3] : qty;
         result = (float) convert_p_t_z_to_prmsl(p, t, z, lat);
         break;

      // Humidity mixing ratio
      case(mixr_grib_code):
         q      = (double) pqtzuv[1];
         qty    = pqtzuv_qty[1];
         result = (float) convert_q_to_w(q);
         break;

      // Dewpoint temperature: derived from p and q
      case(dpt_grib_code):
         p      = (double) pqtzuv[0];
         q      = (double) pqtzuv[1];
         qty    = pqtzuv_qty[0];
         qty    = pqtzuv_qty[1] > qty ? pqtzuv_qty[1] : qty;
         w      = convert_q_to_w(q);
         vp     = convert_p_w_to_vp(p, w);
         result = (float) convert_vp_to_dpt(vp);
         break;

      // Relative humidity
      case(rh_grib_code):
         p      = (double) pqtzuv[0];
         q      = (double) pqtzuv[1];
         t      = (double) pqtzuv[2];
         qty    = pqtzuv_qty[0];
         qty    = pqtzuv_qty[1] > qty ? pqtzuv_qty[1] : qty;
         qty    = pqtzuv_qty[2] > qty ? pqtzuv_qty[2] : qty;
         result = (float) convert_p_q_t_to_rh(p, q, t);
         break;

      // Wind direction (direction wind is coming from): derived from u and v
      case(wdir_grib_code):
         u      = (double) pqtzuv[4];
         v      = (double) pqtzuv[5];
         qty    = pqtzuv_qty[4];
         qty    = pqtzuv_qty[5] > qty ? pqtzuv_qty[5] : qty;
         result = (float) convert_u_v_to_wdir(u, v);
         break;

      // Wind speed: derived from u and v
      case(wind_grib_code):
         u      = (double) pqtzuv[4];
         v      = (double) pqtzuv[5];
         qty    = pqtzuv_qty[4];
         qty    = pqtzuv_qty[5] > qty ? pqtzuv_qty[5] : qty;
         result = (float) convert_u_v_to_wind(u, v);
         break;

      default:
         v = fill_value;
         break;
   } // end switch

   return(result);
}

////////////////////////////////////////////////////////////////////////

void display_bufr_variables(const StringArray &all_vars, const StringArray &all_descs,
                            const StringArray &hdr_arr, const StringArray &obs_arr) {
   int i, index;
   char description[BUFR_DESCRIPTION_LEN+1];
   char line_buf[(BUFR_DESCRIPTION_LEN+1)*2];
   
   mlog << Debug(1) << "\n   Header variables:\n";
   for(i=0; i<hdr_arr.n_elements(); i++) {
      if (all_vars.has(hdr_arr[i], index)) {
         strcpy(description, all_descs[index]);
      }
      else {
         strcpy(description, "");
      }
      sprintf(line_buf, "   %8s: %s\n", hdr_arr[i], description);
      mlog << Debug(1) << line_buf;
   }
   
   mlog << Debug(1) << "\n   Observation variables:\n";
   for(i=0; i<obs_arr.n_elements(); i++) {
      if (all_vars.has(obs_arr[i], index)) {
         strcpy(description, all_descs[index]);
      }
      else {
         strcpy(description, "");
      }
      sprintf(line_buf, "   %8s: %s\n", obs_arr[i], description);
      mlog << Debug(1) << line_buf;
   }
}

////////////////////////////////////////////////////////////////////////

void usage() {

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
        << "\t[-all]\n"
        << "\t[-index]\n"
        << "\t[-use_var_id]\n"
        << "\t[-vars var1,var2]\n"
        << "\t[-dump path]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

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

        << "\t\t\"-use_var_id\" indicates that the MET variable index "
        << "is saved instead of the GRIB code (optional). This option will be "
        << "automatically enabled if \"-all\" or \"-vars\" option is given.\n"

        << "\t\t\"-all\" indicates that all variables are extracted from"
        << " \"prepbufr_file\" (optional). Auotomatically enables -use_var_id.\n"

        << "\t\t\"-index\" indicates that the meta data (available variables and headers)"
        << " is extracted from \"prepbufr_file\" (optional). "
        << "No NetCDF outputs. \"-all\" or \"-vars\" is ignored.\n"

        << "\t\t\"-vars variables\" indicates that the comma seperated variables"
        << " to NetCDF outputs (optional). Auotomatically enables -use_var_id\n\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t\"-compress level\" overrides the compression level of NetCDF variable ("
        << conf_info.conf.nc_compression() << ") (optional).\n\n"

        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_pbfile(const StringArray & a)
{
   pbfile.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_valid_beg_time(const StringArray & a)
{
   valid_beg_ut = timestring_to_unix(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_valid_end_time(const StringArray & a)
{
   valid_end_ut = timestring_to_unix(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_nmsg(const StringArray & a)
{
   nmsg = atoi(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_dump_path(const StringArray & a)
{
   dump_flag = true;
   dump_dir = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_do_all_variables(const StringArray & a)
{
   do_all_vars = true;
   use_var_id = true;
}

////////////////////////////////////////////////////////////////////////

void set_collect_metadata(const StringArray & a)
{
   collect_metadata = true;
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a)
{
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_target_variables(const StringArray & a)
{
   use_var_id = true;
   use_met_vars = true;
   bufr_target_variables.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_use_var_id(const StringArray & a)
{
   use_var_id = true;
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a)
{
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////

void set_compress(const StringArray & a) {
   compress_level = atoi(a[0]);
}

////////////////////////////////////////////////////////////////////////

