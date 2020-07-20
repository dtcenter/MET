// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
//                    observation for requested message types.
//   007    02-15-11  Halley Gotway  Fix event_stack bug for which the
//                    logic was reversed.
//   008    02-28-11  Halley Gotway  Modify relative humidity derivation
//                    to match the Unified-PostProcessor.
//   009    06-01-11  Halley Gotway  Call closepb for input file
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
//          09/15/17  Howard Soh     Removed options: -all, and -use_var_id.
//   015    02/10/18  Halley Gotway  Add message_type_group_map.
//   016    07/23/18  Halley Gotway  Support masks defined by gen_vx_mask.
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
#include <assert.h>

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
#include "write_netcdf.h"

#include "vx_summary.h"
#include "nc_obs_util.h"
#include "nc_summary.h"

////////////////////////////////////////////////////////////////////////

struct derive_var_cfg {
   int var_index;
   ConcatString var_name;
   derive_var_cfg(ConcatString _var_name);
};

////////////////////////////////////////////////////////////////////////

//
// Constants
//

static const char * default_config_filename = "MET_BASE/config/PB2NCConfig_default";

static const char *program_name = "pb2nc";
static const char *not_assigned = "not_assigned";

static const float fill_value   = -9999.f;
static const int missing_cycle_minute = -1;

// Constants used to interface to Fortran subroutines

// Missing value for BUFR data
static const double r8bfms      = 1.0E10;
// Maximum number of BUFR parameters
static const int mxr8pm         = 10;
// Maximum number of BUFR levels
static const int mxr8lv_small   = 255;
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
static const int COUNT_THRESHOLD = 5;

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
static int bufr_var_code[mxr8vt];

// Number of variable types which may be derived
static const int n_derive_gc = 6;

// Listing of grib codes for variable types which may be derived
static const int derive_gc[n_derive_gc] = {
   dpt_grib_code,  wdir_grib_code,
   wind_grib_code, rh_grib_code,
   mixr_grib_code, prmsl_grib_code
};

// The fortran code is hard-coded with 200 levels
#define MAX_CAPE_VALUE 10000
#define MAX_CAPE_LEVEL 200
#define CAPE_INPUT_VARS 3
static float cape_data_pres[MAX_CAPE_LEVEL];
static float cape_data_temp[MAX_CAPE_LEVEL];
static float cape_data_spfh[MAX_CAPE_LEVEL];
static float static_dummy_200[MAX_CAPE_LEVEL];
static float static_dummy_201[MAX_CAPE_LEVEL+1];

#define ROG             287.04
#define MAX_PBL         5000
#define MAX_PBL_LEVEL   256
#define PBL_DEBUG_LEVEL 8
static bool IGNORE_Q_PBL = true;
static bool IGNORE_Z_PBL = true;
static bool USE_LOG_INTERPOLATION = true;
static float pbl_data_pres[MAX_PBL_LEVEL];  // mb
static float pbl_data_temp[MAX_PBL_LEVEL];  // Kelvin
static float pbl_data_spfh[MAX_PBL_LEVEL];  // ? / 1,000,000
static float pbl_data_hgt[MAX_PBL_LEVEL];
static float pbl_data_ugrd[MAX_PBL_LEVEL];
static float pbl_data_vgrd[MAX_PBL_LEVEL];

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
static NcObsOutputData nc_out_data;

// Beginning and ending retention times
static unixtime valid_beg_ut, valid_end_ut;

// Number of PrepBufr messages to process from the command line
static int nmsg = -1;
static int nmsg_percent = -1;

// Dump contents of PrepBufr file to ASCII files
static bool dump_flag = false;
static ConcatString dump_dir = (string)".";

static bool obs_to_vector    = true;
static bool do_all_vars      = false;
static bool override_vars    = false;
static bool collect_metadata = false;
static StringArray bufr_target_variables;

static ConcatString data_plane_filename;

static int compress_level = -1;
static bool save_summary_only = false;


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

static const char *airnow_aux_vars = "TPHR QCIND";

// Pick the latter one if exists multiuple variables
static const char *bufr_avail_sid_names = "SID SAID RPID";
static const char *bufr_avail_latlon_names = "XOB CLON CLONH YOB CLAT CLATH";
static const char *derived_cape = "D_CAPE";
static const char *derived_pbl  = "D_PBL";

static double bufr_obs[mxr8lv][mxr8pm];
static double bufr_obs_extra[mxr8lv][mxr8pm];
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
static ConcatString bufr_hdrs;          // header name list to read header
static StringArray bufr_hdr_name_arr;   // available header name list
static StringArray bufr_obs_name_arr;   // available obs. name list
static IntArray filtered_times;
static vector<derive_var_cfg> bufr_derive_cfgs;
static map<ConcatString, StringArray> variableTypeMap;

static bool do_summary;
static SummaryObs *summary_obs;
static NetcdfObsVars obs_vars;


////////////////////////////////////////////////////////////////////////

static int         n_total_obs;    // Running total of observations
static vector< Observation > observations;

//
// Output NetCDF file, dimensions, and variables
//
static NcFile *f_out      = (NcFile *) 0;

////////////////////////////////////////////////////////////////////////

extern "C" {
   void numpbmsg_(int *, int *);
   void openpb_(const char *, int *);
   void closepb_(int *);
   void readpb_(int *, int *, int *, double[mxr8pm],
                double[mxr8vt][mxr8vn][mxr8lv][mxr8pm], int *);
   void readpb_hdr_ (int *,int *,double[mxr8pm]);
   void ireadns_  (int *, char *, int *);
   void readpbevt_(int *, int *, int *, double[mxr8vn][mxr8lv][mxr8pm],
                   char[mxr8lv*mxr8pm], int *, int *);
   void readpbint_(int *, int *, int *, double[mxr8lv][mxr8pm],
                   char[mxr8lv*mxr8pm], int *, int *);
   void dumppb_(const char *, int *, const char *, int *,
                const char *, int *, int *);
   void dump_tbl_(const char *, int *, const char *, int *);
   int  get_tmin_(int *, int *);
   void calcape_(const int *, const int *, float [MAX_CAPE_LEVEL], float [MAX_CAPE_LEVEL],
                 float [MAX_CAPE_LEVEL],float *, float *, float *, float [MAX_CAPE_LEVEL+1],
                 const int *, const int *, const int *, const int *,
                 float *, float *, float *, float *, float [MAX_CAPE_LEVEL]);
   void calpbl_(float[MAX_PBL_LEVEL], float[MAX_PBL_LEVEL], float[MAX_PBL_LEVEL],
                float[MAX_PBL_LEVEL], float[MAX_PBL_LEVEL], float[MAX_PBL_LEVEL],
                const int *, float *, int *);
}

// Defined at write_netcdf.cc
extern int   obs_data_idx;

////////////////////////////////////////////////////////////////////////

static void   initialize();
static void   process_command_line(int, char **);
static void   open_netcdf();
static void   process_pbfile(int);
static void   process_pbfile_metadata(int);
static void   write_netcdf_hdr_data();
static void   clean_up();

static void addObservation(const float *obs_arr, const ConcatString &hdr_typ,
      const ConcatString &hdr_sid, const time_t hdr_vld,
      const float hdr_lat, const float hdr_lon, const float hdr_elv,
      const float quality_mark, const int buf_size);

static int    get_event_index(int, int, int);
static int    get_event_index_temp(int, int, int);
static void   dbl2str(double *, ConcatString&);

static bool   keep_message_type(const char *);
static bool   keep_station_id(const char *);
static bool   keep_valid_time(const unixtime, const unixtime,
                              const unixtime);
static bool   keep_pb_report_type(int);
static bool   keep_in_report_type(int);
static bool   keep_instrument_type(int);
static bool   keep_bufr_obs_index(int);
static bool   keep_level_category(int);

static float  derive_grib_code(int, float *, float *, double, float&);

static int    combine_tqz_and_uv(map<float, float*>,
                                 map<float, float*>, map<float, float*> &);
static float  compute_pbl(map<float, float*> pqtzuv_map_tq,
                          map<float, float*> pqtzuv_map_uv);
static void   copy_pqtzuv(float *to_pqtzuv, float *from_pqtzuv, bool copy_all=true);
static void   insert_pbl(float *obs_arr, const float pbl_value, const int pbl_code,
                         const float pbl_p, const float pbl_h, const float pbl_qm,
                         const float hdr_lat, const float hdr_lon,
                         const float hdr_elv, const time_t hdr_vld_ut,
                         const ConcatString &hdr_typ, const ConcatString &hdr_sid);
static int    interpolate_by_pressure(int length, float *pres_data, float *var_data);
static void   interpolate_pqtzuv(float*, float*, float*);
static void   log_merged_tqz_uv(map<float, float*> pqtzuv_map_tq,
                                map<float, float*> pqtzuv_map_uv,
                                map<float, float*> &pqtzuv_map_merged,
                                const char *method_name);
static void   log_pbl_input(int pbl_level, const char *method_name);
static void   log_tqz_and_uv(map<float, float*> pqtzuv_map_tq,
                             map<float, float*> pqtzuv_map_uv,
                             const char *method_name);
static void   merge_records(float *first_pqtzuv, map<float, float*> pqtzuv_map_pivot,
                            map<float, float*> pqtzuv_map_aux,
                            map<float, float*> &pqtzuv_map_merged);

static void   usage();
static void   set_pbfile(const StringArray &);
static void   set_valid_beg_time(const StringArray &);
static void   set_valid_end_time(const StringArray &);
static void   set_nmsg(const StringArray &);
static void   set_dump_path(const StringArray &);
static void   set_collect_metadata(const StringArray &);
static void   set_target_variables(const StringArray & a);
static void   set_logfile(const StringArray &);
static void   set_verbosity(const StringArray &);
static void   set_compress(const StringArray &);

static void   display_bufr_variables(const StringArray &, const StringArray &,
                                     const StringArray &, const StringArray &);
static void   cleanup_hdr_typ(char *hdr_typ, bool is_prepbufr=false);

////////////////////////////////////////////////////////////////////////

derive_var_cfg::derive_var_cfg(ConcatString _var_name) {
   var_index = bad_data_int;
   var_name = _var_name;
}

////////////////////////////////////////////////////////////////////////


int main(int argc, char *argv[]) {
   int i;

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Initialize static variables
   initialize();

   // Process the command line arguments
   process_command_line(argc, argv);

   if (collect_metadata) {
      // Process each PrepBufr file
      for(i=0; i<pbfile.n_elements(); i++) {
         process_pbfile_metadata(i);
      }
      display_bufr_variables(tableB_vars, tableB_descs,
                             bufr_hdr_name_arr, bufr_obs_name_arr);

   }
   else {
      // Open the NetCDF file
      open_netcdf();

      // Process each PrepBufr file
      for(i=0; i<pbfile.n_elements(); i++) {
         process_pbfile_metadata(i);
         process_pbfile(i);
      }

      if (do_summary) {
         TimeSummaryInfo summaryInfo = conf_info.getSummaryInfo();
         summary_obs->summarizeObs(summaryInfo);
         summary_obs->setSummaryInfo(summaryInfo);
      }

      // Write the NetCDF file
      write_netcdf_hdr_data();
   }

   // Deallocate memory and clean up
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void initialize() {

   n_total_obs = 0;

   nc_obs_initialize();

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

   prepbufr_derive_vars.add("D_DPT");
   prepbufr_derive_vars.add("D_WDIR");
   prepbufr_derive_vars.add("D_WIND");
   prepbufr_derive_vars.add("D_RH");
   prepbufr_derive_vars.add("D_MIXR");
   prepbufr_derive_vars.add("D_PRMSL");
   prepbufr_derive_vars.add("D_CAPE");
   prepbufr_derive_vars.add("D_PBL");

   for (int idx=0; idx<(sizeof(hdr) / sizeof(hdr[0])); idx++) {
      hdr[idx] = r8bfms * 10;
   }

   summary_obs = new SummaryObs();
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
   cline.add(set_collect_metadata,  "-index",  0);
   cline.add(set_target_variables, "-vars", 1);
   cline.add(set_logfile, "-log", 1);
   cline.add(set_verbosity, "-v", 1);
   cline.add(set_compress,  "-compress",  1);

   // Parse the command line
   cline.parse();

   // Check for error. There should be three arguments left:
   // PrepBufr, output NetCDF, and config filenames
   if(cline.n() != 3 && !collect_metadata) usage();

   // Store the input file names
   pbfile.add(cline[0]);
   if(cline.n() > 1) ncfile      = cline[1];
   if(cline.n() > 2) config_file = cline[2];

   // Create the default config file name
   default_config_file = replace_path(default_config_filename);
   if(cline.n() < 2)      config_file = default_config_file;
   else if(cline.n() < 3) config_file = cline[1];

   // List the config files
   mlog << Debug(1)
        << "Default Config File: " << default_config_file << "\n"
        << "User Config File: "    << config_file << "\n";

   // Read the config files
   conf_info.read_config(default_config_file.c_str(), config_file.c_str());

   // Process the configuration
   conf_info.process_config();

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

   if (!override_vars) {
      bufr_target_variables = conf_info.obs_bufr_var;
      do_all_vars = (0 == conf_info.obs_bufr_var.n_elements());
   }

   do_summary = conf_info.getSummaryInfo().flag;
   if (!do_summary) save_summary_only = false;
   else save_summary_only = !conf_info.getSummaryInfo().raw_data;

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString save_bufr_table_to_file(const char *blk_file, int file_id) {
   int len;
   ConcatString tbl_filename, tbl_prefix;

   tbl_prefix << conf_info.tmp_dir << "/" << "tmp_pb2nc_bufr";
   tbl_filename = make_temp_file_name(tbl_prefix.c_str(), "tbl");
   len = tbl_filename.length();
   if (file_id > MAX_FORTRAN_FILE_ID || file_id < MIN_FORTRAN_FILE_ID) {
      mlog << Error << "\nsave_bufr_table_to_file() -> "
           << "Invalid file ID [" << file_id << "] between 1 and 99.\n\n";
   }
   openpb_(blk_file, &file_id);
   dump_tbl_(blk_file, &file_id, tbl_filename.c_str(), &len);
   closepb_(&file_id);
   //close(file_id);
   // Delete the temporary blocked file
   remove_temp_file((string)blk_file);
   return tbl_filename;
}


bool is_prepbufr_file(StringArray *events) {
   bool is_prepbufr = events->has("P__EVENT") && events->has("Q__EVENT")
         && events->has("T__EVENT") && events->has("Z__EVENT");
   return is_prepbufr;
}

////////////////////////////////////////////////////////////////////////

void get_variable_info(const char* tbl_filename) {
   static const char *method_name = "  get_variable_info()";

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
      char var_desc[max(BUFR_DESCRIPTION_LEN,BUFR_SEQUENCE_LEN)+1];
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
         mlog << Debug(10) << method_name << " sec. 1 var: ["
              << var_name << "] \tdesc: [" << var_desc << "]\n";
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
         mlog << Debug(10) << method_name << " event: ["
              << var_name << "] \tdesc: [" << var_desc << "]\n";
         event_names.add(var_name);
         event_members.add(var_desc);
      }
      getline(&line, &len, fp);

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
            strncpy(var_unit_str, "CCITT IA5", sizeof(var_unit_str));
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
            mlog << Debug(10) << method_name << " sec. 3 var: ["
                 << var_name << "] \tdesc: [" << var_desc << "]\n";
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
   f_out = open_ncfile(ncfile.c_str(), true);

   // Check for a valid file
   if(IS_INVALID_NC_P(f_out)) {
      mlog << Error << "\nopen_netcdf() -> "
           << "trouble opening output file: " << ncfile << "\n\n";

      delete f_out;
      f_out = (NcFile *) 0;

      exit(1);
   }

   // Define netCDF variables
   init_nc_dims_vars_config(obs_vars);
   obs_vars.attr_pb2nc = true;

   if (!obs_to_vector) {

      create_nc_obs_vars(obs_vars, f_out,
                         (compress_level >= 0 ? compress_level :
                          conf_info.conf.nc_compression()));

      // Add global attributes
      write_netcdf_global(f_out, ncfile.text(), program_name);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_pbfile(int i_pb) {
   int npbmsg, npbmsg_total, unit, yr, mon, day, hr, min, sec;
   int i, i_msg, i_read, n_file_obs, i_ret, i_date, n_hdr_obs;
   int rej_typ, rej_sid, rej_vld, rej_grid, rej_poly;
   int rej_elv, rej_pb_rpt, rej_in_rpt, rej_itp, rej_nobs;
   int lv, ev, ev_temp, kk, len1, len2;

   double   x, y;

   int cycle_minute;
   unixtime file_ut = (unixtime) 0;
   unixtime adjusted_file_ut;
   unixtime msg_ut, beg_ut, end_ut;
   unixtime min_msg_ut, max_msg_ut;

   beg_ut = end_ut = (unixtime) 0;

   ConcatString file_name, blk_prefix, blk_file, log_message;
   ConcatString prefix;
   char     time_str[max_str_len];
   ConcatString  start_time_str, end_time_str;
   char     min_time_str[max_str_len], max_time_str[max_str_len];

   char     hdr_typ[max_str_len];
   ConcatString hdr_sid;
   char     modified_hdr_typ[max_str_len];
   double   hdr_lat = bad_data_double;
   double   hdr_lon = bad_data_double;
   double   hdr_elv = bad_data_double;
   float    pb_report_type, in_report_type, instrument_type;
   unixtime hdr_vld_ut = (unixtime) 0;

   float    quality_mark, dl_category;
   float    obs_arr[obs_arr_len];
   float    pqtzuv[mxr8vt], pqtzuv_qty[mxr8vt];

   const int debug_level_for_performance = 3;
   int start_t, end_t, method_start, method_end;
   start_t = end_t = method_start = method_end = clock();

   IntArray diff_file_times;
   int diff_file_time_count;
   StringArray variables_big_nlevels;
   static const char *method_name = "process_pbfile()";

   bool apply_grid_mask = (conf_info.grid_mask.nx() > 0 &&
                           conf_info.grid_mask.ny() > 0);
   bool apply_area_mask = (conf_info.area_mask.nx() > 0 &&
                           conf_info.area_mask.ny() > 0);
   bool apply_poly_mask = (conf_info.poly_mask.n_points() > 0);

   // List the PrepBufr file being processed
   mlog << Debug(1) << "Processing Bufr File:\t" << pbfile[i_pb]<< "\n";

   // Initialize
   filtered_times.clear();
   min_msg_ut = max_msg_ut = (unixtime) 0;
   min_time_str[0] = bad_data_char;
   max_time_str[0] = bad_data_char;

   // Set the file name for the PrepBufr file
   file_name << pbfile[i_pb];

   // Build the temporary block file name
   blk_prefix << conf_info.tmp_dir << "/" << "tmp_pb2nc_blk";
   blk_file = make_temp_file_name(blk_prefix.c_str(), NULL);

   mlog << Debug(1) << "Blocking Bufr file to:\t" << blk_file << "\n";

   // Assume that the input PrepBufr file is unblocked.
   // Block the PrepBufr file and open it for reading.
   pblock(file_name.c_str(), blk_file.c_str(), block);

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
      prefix = get_short_name(pbfile[i_pb].c_str());
      len1 = dump_dir.length();
      len2 = prefix.length();
      dumppb_(blk_file.c_str(), &unit, dump_dir.c_str(), &len1,
              prefix.c_str(), &len2, msg_typ_ret);
   }

   // Open the blocked temp PrepBufr file for reading
   unit = file_unit + i_pb;
   if (unit > MAX_FORTRAN_FILE_ID || unit < MIN_FORTRAN_FILE_ID) {
      mlog << Error << "\n" << method_name << " -> "
           << "Invalid file ID [" << unit << "] between 1 and 99.\n\n";
   }
   openpb_(blk_file.c_str(), &unit);

   // Compute the number of PrepBufr records in the current file.
   numpbmsg_(&unit, &npbmsg);
   npbmsg_total = npbmsg;

   // Use the number of records requested by the user if there
   // are enough present.
   if(nmsg > 0 && nmsg < npbmsg) {
      npbmsg = (nmsg_percent > 0 && nmsg_percent <= 100)
            ? (npbmsg * nmsg_percent / 100) : nmsg;
   }

   // Check for zero messages to process
   if(npbmsg <= 0 || npbmsg_total <= 0) {
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

   //processed_count = 0;
   int buf_nlev;
   bool showed_progress = false;
   bool is_prepbufr = is_prepbufr_file(&event_names);
   if(mlog.verbosity_level() >= debug_level_for_performance) {
      end_t = clock();
      mlog << Debug(debug_level_for_performance) << " PERF: " << method_name << " "
           << (end_t-start_t)/double(CLOCKS_PER_SEC)
           << " seconds for preparing\n";
      start_t = clock();
   }

   log_message = (is_prepbufr ? " PrepBufr" : " Bufr");
   log_message.add(" messages");
   if (npbmsg != npbmsg_total) {
      log_message << " (out of " << unixtime_to_string(npbmsg_total) << ")";
   }
   mlog << Debug(2) << "Processing " << npbmsg << log_message << "...\n";

   int nlev_max_req = mxr8lv;
   if (0 < conf_info.end_level && conf_info.end_level < mxr8lv) {
      nlev_max_req = conf_info.end_level;
      mlog << Debug(4) << "Request up to " << nlev_max_req << " vertical levels\n";
   }

   int grib_code, bufr_var_index;
   map<ConcatString, ConcatString> message_type_map = conf_info.getMessageTypeMap();

   int bin_count = nint(npbmsg/20.0);
   int bufr_hdr_length = bufr_hdrs.length();
   ConcatString bufr_hdr_names;
   bufr_hdr_names = bufr_hdrs.text();

   // To compute CAPE
   int ivirt = 1;   // 0: regular thetap and thetaa
                    // 1: virtual thetap and thetaa
   int itype = 1;   // itype 1: where a parcel is lifted from the ground
                    // itype 2: Where the "best cape" in a number of parcels
   int cape_code = -1;
   float p1d,t1d,q1d;
   int IMM, JMM;
   int cape_level=0, cape_count=0, cape_cnt_too_big=0, cape_cnt_surface_msgs=0;
   int cape_cnt_no_levels=0, cape_cnt_missing_values=0, cape_cnt_zero_values=0;
   float cape_p, cape_h;
   float cape_qm = bad_data_float;

   // To compute PBL
   int pbl_level = 0;
   int pbl_code = -1;
   float pbl_p, pbl_h;
   float pbl_qm = bad_data_float;

   bool has_pbl_data;
   bool do_pbl = false;
   bool cal_cape = bufr_obs_name_arr.has(derived_cape, cape_code, false);
   bool cal_pbl = bufr_obs_name_arr.has(derived_pbl, pbl_code, false);

   bool     is_same_header;
   unixtime prev_hdr_vld_ut = (unixtime) 0;
   float    prev_quality_mark;
   char     prev_hdr_typ[max_str_len], prev_hdr_sid[max_str_len];
   double   prev_hdr_lat, prev_hdr_lon, prev_hdr_elv;
   map<float, float*> pqtzuv_map_tq;
   map<float, float*> pqtzuv_map_uv;

   // Initialize
   prev_hdr_lat = prev_hdr_lon = prev_hdr_elv = bad_data_double;

   if (cal_pbl) {
      is_same_header = false;
      prev_hdr_vld_ut = -1;
      strncpy(prev_hdr_typ, not_assigned, strlen(not_assigned));
      strncpy(prev_hdr_sid, not_assigned, strlen(not_assigned));
   }

   IMM = JMM =1;
   p1d = t1d = q1d = r8bfms * 10;
   cape_h = pbl_h = 0;
   cape_p = pbl_p = bad_data_float;

   diff_file_time_count = 0;
   cycle_minute = missing_cycle_minute;     // initialize

   for (int idx=0; idx<obs_arr_len; idx++) obs_arr[idx] = 0;

   // Loop through the PrepBufr messages from the input file
   for(i_read=0; i_read<npbmsg && i_ret == 0; i_read++) {

      if(mlog.verbosity_level() > 0) {
         if(bin_count > 0 && (i_read+1)%bin_count == 0) {
            cout << nint((double) (i_read+1)/npbmsg*100.0) << "% " << flush;
            showed_progress = true;
            if(mlog.verbosity_level() >= debug_level_for_performance) {
               end_t = clock();
               cout << (end_t-start_t)/double(CLOCKS_PER_SEC)
                    << " seconds\n";
               start_t = clock();
            }
         }
      }

      // Get the next PrepBufr message (header only)
      ireadns_(&unit, hdr_typ, &i_date);
      readpb_hdr_(&unit, &i_ret, hdr);

      snprintf(time_str, sizeof(time_str), "%.10i", i_date);
      msg_ut = yyyymmddhh_to_unix(time_str);

      // Check to make sure that the message time hasn't changed
      // from one PrepBufr message to the next
      if(file_ut == (unixtime) 0) {
         adjusted_file_ut = file_ut = msg_ut;

         // Get minutes offset by calling IUPVS01(unit, "MINU")
         // It's not changed per message
         get_tmin_(&unit, &cycle_minute);
         if (cycle_minute != missing_cycle_minute) adjusted_file_ut += (cycle_minute * 60);

         mlog << Debug(2) << (is_prepbufr ? "PrepBufr" : "Bufr")
              << " Time Center:\t\t" << unix_to_yyyymmdd_hhmmss(adjusted_file_ut)
              << "\n";

         // Check if valid_beg_ut and valid_end_ut were set on the
         // command line.  If so, use them.  If not, use beg_ds and
         // end_ds.
         if(valid_beg_ut != (unixtime) 0 ||
            valid_end_ut != (unixtime) 0) {
            beg_ut = valid_beg_ut;
            end_ut = valid_end_ut;
         }
         else {
            beg_ut = adjusted_file_ut + conf_info.beg_ds;
            end_ut = adjusted_file_ut + conf_info.end_ds;
         }

         if(beg_ut != (unixtime) 0) {
            unix_to_yyyymmdd_hhmmss(beg_ut, start_time_str);
         }
         else {
            start_time_str = "NO_BEG_TIME";
         }

         if(end_ut != (unixtime) 0) {
            unix_to_yyyymmdd_hhmmss(end_ut, end_time_str);
         }
         else {
            end_time_str = "NO_END_TIME";
         }

         mlog << Debug(2) << "Searching Time Window:\t\t" << start_time_str
              << " to " << end_time_str << "\n";

      }
      else if(file_ut != msg_ut) {
         diff_file_time_count++;
         if (!diff_file_times.has(msg_ut)) diff_file_times.add(msg_ut);
      }

      // Add minutes by calling IUPVS01(unit, "MINU")
      if (cycle_minute != missing_cycle_minute) {
         msg_ut += cycle_minute * 60;
      }

      if (!is_prepbufr) {
         int index, req_hdr_level = 1;
         ConcatString tmp_str;

         //Read header (station id, lat, lon, ele, time)
         tmp_str = bufr_hdrs;
         readpbint_(&unit, &i_ret, &nlev, bufr_obs, (char*)bufr_hdr_names.c_str(),
                    &bufr_hdr_length, &req_hdr_level );

         // Copy sid, lat, lon, and dhr
         for (index=0; index<4; index++) {
            hdr[index] = bufr_obs[0][index];
         }

         // Update hdr[3] and file_ut for obs_time
         if (bufr_obs[0][3] > r8bfms) {
            hdr[3] = 0;
            yr  = nint(bufr_obs[0][4]);
            mon = nint(bufr_obs[0][5]);
            day = nint(bufr_obs[0][6]);
            hr  = nint(bufr_obs[0][7]);
            min = nint(bufr_obs[0][8]);
            sec = nint(bufr_obs[0][9]);
            if (hr  > r8bfms) hr = 0;
            if (min > r8bfms) min = 0;
            if (sec > r8bfms || sec < 0) sec = 0;
            msg_ut = mdyhms_to_unix(mon, day, yr, hr, min, sec);
            mlog << Debug(7) << "Bufr obs_time:\t"
                 << unix_to_yyyymmdd_hhmmss(msg_ut) << "\n\n";
            for (index=0; index<mxr8lv; index++) {
               bufr_pres_lv[index] = fill_value;
               bufr_msl_lv[index]  = fill_value;
            }
         }
      }

      // Null terminate the message type string
      cleanup_hdr_typ(hdr_typ, is_prepbufr);

      // If the message type is not listed in the configuration
      // file and it is not the case that all message types should be
      // retained, continue to the next PrepBufr message
      if(!keep_message_type(hdr_typ)) {
         rej_typ++;
         continue;
      }

      // Convert the SID to a string, then
      // crop to 8 characters and remove whitespace
      dbl2str(&hdr[0], hdr_sid);
      hdr_sid = hdr_sid.string().substr(0, mxr8nm);
      hdr_sid.ws_strip();

      // If the station id is not listed in the configuration
      // file and it is not the case that all station ids should be
      // retained, continue to the next PrepBufr message
      if(!keep_station_id(hdr_sid.c_str())) {
         rej_sid++;
         continue;
      }

      // Read the header array elements which consists of:
      //    LON LAT DHR ELV TYP T29 ITP

      // Longitude
      if(hdr[1] > r8bfms) hdr_lon     = fill_value;
      else                hdr_lon     = hdr[1];

      // Latitude
      if(hdr[2] > r8bfms) hdr_lat     = fill_value;
      else                hdr_lat     = hdr[2];

      // Elevation
      if(hdr[4] > r8bfms) hdr_elv     = fill_value;
      else                hdr_elv     = hdr[4];

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
      hdr_vld_ut = msg_ut + (unixtime)nint(hdr[3]*sec_per_hour);
      if (0 == min_msg_ut || min_msg_ut > hdr_vld_ut) {
         min_msg_ut = hdr_vld_ut;
      }
      if (max_msg_ut < hdr_vld_ut) {
         max_msg_ut = hdr_vld_ut;
      }
      if(!keep_valid_time(hdr_vld_ut, beg_ut, end_ut)) {
         if (!filtered_times.has(hdr_vld_ut, false)) {
            filtered_times.add(hdr_vld_ut);
         }
         rej_vld++;
         continue;
      }

      // Rescale the longitude value from 0 to 360 -> -180 to 180
      hdr_lon = rescale_lon(hdr_lon);

      // If the lat/lon for the PrepBufr message is not on the
      // grid_mask, continue to the next PrepBufr message
      if(apply_grid_mask) {
         conf_info.grid_mask.latlon_to_xy(hdr_lat, (-1.0*hdr_lon), x, y);
         if(x < 0 || x >= conf_info.grid_mask.nx() ||
            y < 0 || y >= conf_info.grid_mask.ny()) {
            rej_grid++;
            continue;
         }

         // Include the area mask rejection counts with the polyline since
         // it is specified using the mask.poly config option.
         if(apply_area_mask) {
            if(!conf_info.area_mask.s_is_on(nint(x), nint(y))) {
               rej_poly++;
               continue;
            }
         }
      }

      // If the lat/lon for the PrepBufr message is not inside the mask
      // polyline continue to the next PrepBufr message.  Multiply by
      // -1 to convert from degrees_east to degrees_west
      if(apply_poly_mask &&
         !conf_info.poly_mask.latlon_is_inside_dege(hdr_lat, hdr_lon)) {
         rej_poly++;
         continue;
      }

      // Check if the message elevation is within the specified range.
      // Missing data values for elevation are retained.
      if (!is_eq(hdr_elv, fill_value) &&
         (hdr_elv < conf_info.beg_elev ||
          hdr_elv > conf_info.end_elev) ) {
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

      // Get the next PrepBufr message
      readpb_(&unit, &i_ret, &nlev, hdr, evns, &nlev_max_req);

      // Special handling for "AIRNOW" and "ANOWPM"
      bool is_airnow = (0 == strcmp("AIRNOW", hdr_typ) ||
                        0 == strcmp("ANOWPM", hdr_typ));

      if (0 < message_type_map.count((string)hdr_typ)) {
         ConcatString mappedMessageType = message_type_map[(string)hdr_typ];
         mlog << Debug(6) << "\n" << method_name << " -> "
              << "Switching report type \"" << hdr_typ
              << "\" to message type \"" << mappedMessageType << "\".\n";
         if (mappedMessageType.length() < HEADER_STR_LEN) {
            strncpy(modified_hdr_typ, mappedMessageType.c_str(), sizeof(modified_hdr_typ));
         }
         else {
            strncpy(modified_hdr_typ, mappedMessageType.c_str(), HEADER_STR_LEN);
         }
      }
      else {
         strncpy(modified_hdr_typ, hdr_typ, sizeof(modified_hdr_typ));
      }

      // Search through the observation values and store them as:
      //    HDR_ID GC LVL HGT OB

      // Store the index to the header data
      obs_arr[0] = (float) get_nc_hdr_cur_index();

      buf_nlev = nlev;
      if (nlev > mxr8lv) {
         buf_nlev = mxr8lv;
         for(kk=0; kk<mxr8vt; kk++) {
            if (!variables_big_nlevels.has(bufr_obs_name_arr[kk], false)) {
               mlog << Warning << "\n" << method_name << " -> "
                    << "Too many vertical levels (" << nlev
                    << ") for " << bufr_obs_name_arr[kk]
                    << "). Ignored the vertical levels above " << mxr8lv << ".\n\n";
               variables_big_nlevels.add(bufr_obs_name_arr[kk]);
            }
         }
      }

      if (cal_cape) {
         cape_level = 0;
      }

      do_pbl = cal_pbl && 0 == strcmp("ADPUPA", hdr_typ);
      if (do_pbl) {
         pbl_level = 0;
      }

      // Search through the vertical levels
      for(lv=0, n_hdr_obs = 0; lv<buf_nlev; lv++) {

         // If the observation vertical level is not within the
         // specified valid range, continue to the next vertical
         // level
         if(lv+1 < conf_info.beg_level) continue;
         if(lv+1 > conf_info.end_level) break;

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
                                               hdr_lat);
            }
            bufr_pres_lv[lv] = obs_arr[2];
            bufr_msl_lv[lv]  = obs_arr[3];
         }

         // Initialize the P, Q, T, Z, U, V variables
         for(i=0; i<mxr8vt; i++){
            pqtzuv[i]     = fill_value;
            pqtzuv_qty[i] = fill_value;
         }

         int cape_member_cnt = 0;
         // Index through the variable types 'P, Q, T, Z, U, V'
         for(kk=0; kk<mxr8vt; kk++) {

            // Convert the observation variable index to the
            // corresponding grib code and store as the second element
            // of obs_arr
            grib_code = var_gc[kk];
            bufr_var_index = bufr_var_code[kk];
            obs_arr[1] = (float)bufr_var_index;

            // Get the event index to be used based on the contents of
            // the event stack flag
            ev = get_event_index(conf_info.event_stack_flag, kk, lv);

            // Apply special logic to avoid virtual temperature observations
            if(kk == 2) {

               ev_temp = get_event_index_temp(conf_info.event_stack_flag, kk, lv);

               // Check for bad data
               if(is_bad_data(ev_temp)) {

                  mlog << Debug(8)
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
                  mlog << Debug(8) << "For " << hdr_typ << ", station id "
                       << hdr_sid << ", pressure " << obs_arr[2] << " mb, and height "
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

            // If the quality mark is greater than than the quality
            // mark threshold in the configuration file
            // continue to the next observation event
            if(conf_info.quality_mark_thresh < quality_mark) continue;

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
               bool is_cape_input = false;
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

               if (cal_cape) {
                  if(grib_code == pres_grib_code) {
                     is_cape_input = true;
                     if (cape_level < MAX_CAPE_LEVEL) cape_data_pres[cape_level] = obs_arr[4];
                     cape_member_cnt++;
                  }
                  else if(grib_code == spfh_grib_code) {
                     is_cape_input = true;
                     if (cape_level < MAX_CAPE_LEVEL) cape_data_spfh[cape_level] = obs_arr[4];
                     cape_member_cnt++;
                  }
                  else if(grib_code == tmp_grib_code) {
                     is_cape_input = true;
                     if (cape_level < MAX_CAPE_LEVEL) cape_data_temp[cape_level] = obs_arr[4];
                     cape_member_cnt++;
                  }
                  if (is_cape_input && (cape_level == 0)) {
                     cape_qm = quality_mark;
                  }
               }

               if (do_pbl && (pbl_level == 0)) {
                  pbl_qm = quality_mark;
               }
            }

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
            if(!keep_bufr_obs_index(bufr_var_index)) continue;

            addObservation(obs_arr, (string)hdr_typ, (string)hdr_sid, hdr_vld_ut,
                  hdr_lat, hdr_lon, hdr_elv, quality_mark,
                  OBS_BUFFER_SIZE);

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
         if (n_derive_gc > bufr_derive_cfgs.size()) {
            mlog << Debug(3) << "\n" << method_name << " -> "
                 << "Skip the derived variables because of not requested ("
                 << bufr_derive_cfgs.size() << ").\n\n";
         }
         else {
            for(i=0; i<n_derive_gc; i++) {
      
               bufr_var_index = bufr_derive_cfgs[i].var_index;
               if(keep_bufr_obs_index(bufr_var_index)) {
      
                  // Only derive PRMSL for surface message
                  if(derive_gc[i] == prmsl_grib_code &&
                     !conf_info.surface_message_types.has(hdr_typ, false))
                     continue;
      
                  // Store the grib code to be derived
                  obs_arr[1] = (float)bufr_var_index;
      
                  // Derive the value for the grib code
                  obs_arr[4] = derive_grib_code(derive_gc[i], pqtzuv,
                                                pqtzuv_qty, hdr_lat,
                                                quality_mark);
      
                  if(is_eq(obs_arr[4], fill_value)) continue;
      
                  addObservation(obs_arr, (string)hdr_typ, (string)hdr_sid, hdr_vld_ut,
                        hdr_lat, hdr_lon, hdr_elv, quality_mark,
                        OBS_BUFFER_SIZE);
      
                  // Increment the current and total observations counts
                  n_file_obs++;
                  n_total_obs++;
      
                  // Increment the number of obs counter for this header
                  n_hdr_obs++;
               }
            } // end for i
         }

         if (cal_cape) {
            if (cape_member_cnt >= 3) cape_level++;
         }
         if (do_pbl && !is_eq(pqtzuv[0], bad_data_float)) {

            // Allocated memory is deleted after all observations are processed
            float *tmp_pqtzuv = new float [mxr8vt];

            for(kk=0; kk<mxr8vt; kk++) tmp_pqtzuv[kk] = pqtzuv[kk];

            bool has_tq = false;
            bool has_uv = false;
            if (!is_eq(tmp_pqtzuv[4],bad_data_float) &&
                !is_eq(tmp_pqtzuv[5],bad_data_float)) {
               has_uv = true;
               pqtzuv_map_uv[pqtzuv[0]] = tmp_pqtzuv;
            }
            if (!is_eq(tmp_pqtzuv[2],bad_data_float) &&
                     (!is_eq(tmp_pqtzuv[1],bad_data_float) || IGNORE_Q_PBL) &&
                     (!is_eq(tmp_pqtzuv[3],bad_data_float) || IGNORE_Z_PBL)) {
               has_tq = true;
               pqtzuv_map_tq[pqtzuv[0]] = tmp_pqtzuv;
            }
            if (!(has_tq || has_uv)) {
               mlog << Debug(5) << method_name << " PBL: excluded " << lv
                    << "-th level record:" << " T=" << tmp_pqtzuv[2]
                    << ", U=" << tmp_pqtzuv[4] << ", V=" << tmp_pqtzuv[5]
                    << ", Q=" << tmp_pqtzuv[0] << ", Z=" << tmp_pqtzuv[3]
                    << " valid_time: " << unix_to_yyyymmdd_hhmmss(hdr_vld_ut)
                    << " " << hdr_typ << " " << hdr_sid << "\n";
            }
         }
      } // end for lv

      if (cal_cape) {
         if (1 < cape_level) {
            bool reverse_levels;
            float cape_val, cin_val, PLCL,PEQL;
         
            cape_val = bad_data_double;
            cin_val  = bad_data_double;
         
            if (cape_level > MAX_CAPE_LEVEL) cape_level = MAX_CAPE_LEVEL;
            reverse_levels = (cape_data_pres[0] > cape_data_pres[cape_level-1]);
            if (reverse_levels) {
               int buf_idx;
               float swap_value;
               mlog << Debug(5) << method_name << " Reverse levels\n";
               mlog << Debug(7) << method_name << "   pres[0]: " << cape_data_pres[0]
                    << ", pres[" << (cape_level-1) << "]: "
                    << cape_data_pres[cape_level-1] << "\n";
               for (int idx=0; idx<(cape_level+1)/2; idx++) {
                  buf_idx = cape_level - idx - 1;
                  swap_value = cape_data_pres[idx];
                  cape_data_pres[idx] = cape_data_pres[buf_idx];
                  cape_data_pres[buf_idx] = swap_value;
         
                  swap_value = cape_data_temp[idx];
                  cape_data_temp[idx] = cape_data_temp[buf_idx];
                  cape_data_temp[buf_idx] = swap_value;
         
                  swap_value = cape_data_spfh[idx];
                  cape_data_spfh[idx] = cape_data_spfh[buf_idx];
                  cape_data_spfh[buf_idx] = swap_value;
               }
            }
            if (cape_level < MAX_CAPE_LEVEL) {
               cape_data_pres[cape_level] = cape_data_pres[cape_level-1];
               cape_data_temp[cape_level] = cape_data_temp[cape_level-1];
               cape_data_spfh[cape_level] = cape_data_spfh[cape_level-1];
               for (int idx=cape_level+1; idx<MAX_CAPE_LEVEL; idx++) {
                  cape_data_pres[idx] = r8bfms * 10;
                  cape_data_temp[idx] = r8bfms * 10;
                  cape_data_spfh[idx] = r8bfms * 10;
               }
            }
         
            //p1d = cape_p;
            //t1d = cape_data_temp[cape_level-1];
            //q1d = cape_data_spfh[cape_level-1];
            calcape_(&ivirt,&itype, cape_data_temp, cape_data_spfh, cape_data_pres,
                     &p1d,&t1d,&q1d, static_dummy_201,
                     &cape_level, &IMM,&JMM, &cape_level,
                     &cape_val, &cin_val, &PLCL, &PEQL, static_dummy_200);
         
            if(mlog.verbosity_level() >= 7) {
               mlog << Debug(7) << method_name << " index,P,T,Q to compute CAPE from "
                    << i_read << "-th message\n" ;
               for (int idx=0; idx<cape_level; idx++) {
                  mlog << Debug(7) << idx << ", " << cape_data_pres[idx] << ", "
                       << cape_data_temp[idx] << ", " << cape_data_spfh[idx] << "\n";
               }
               mlog << Debug(7) << method_name 
                    << " calcape_(" << ivirt << "," << itype << ") cape_val: "
                    << cape_val << " cape_level: " << cape_level
                    << ", cin_val: " << cin_val
                    << ", PLCL: " << PLCL << ", PEQL: " << PEQL
                    << "   lat: " << hdr_lat << ", lon: " << hdr_lon
                    << " valid_time: " << unix_to_yyyymmdd_hhmmss(hdr_vld_ut)
                    << " " << hdr_typ << " " << hdr_sid
                    << "\n\n" ;
            }
         
            if (cape_val > MAX_CAPE_VALUE) {
               cape_cnt_too_big++;
               mlog << Debug(5) << method_name 
                    << " Ignored cape_value: " << cape_val << " cape_level: " << cape_level
                    << ", cin_val: " << cin_val
                    << ", PLCL: " << PLCL << ", PEQL: " << PEQL << "\n";
            }
            else if (cape_val >= 0) {
               obs_arr[1] = cape_code;
               obs_arr[2] = cape_p;
               obs_arr[3] = cape_h;
               obs_arr[4] = cape_val; // observation value
               addObservation(obs_arr, (string)hdr_typ, (string)hdr_sid, hdr_vld_ut,
                              hdr_lat, hdr_lon, hdr_elv, cape_qm,
                              OBS_BUFFER_SIZE);
               cape_count++;
               if (is_eq(cape_val, 0.)) cape_cnt_zero_values++;
            }
            else cape_cnt_missing_values++;
         }
         else if (1 < buf_nlev) cape_cnt_no_levels++;
         else cape_cnt_surface_msgs++;
      }

      //if (do_all_vars) {
      {
         quality_mark = bad_data_int;
         ConcatString quality_mark_str;

         if (is_airnow) {
            int tmp_ret, tmp_nlev, tmp_len;
            for (int index1=0; index1<mxr8lv; index1++) {
              for (int index2=0; index2<mxr8pm; index2++) {
                 bufr_obs_extra[index1][index2] = bad_data_double;
              }
            }
            tmp_len = strlen(airnow_aux_vars);
            readpbint_(&unit, &tmp_ret, &tmp_nlev, bufr_obs_extra,
                       (char *)airnow_aux_vars, &tmp_len, &nlev_max_req);
         }
         bool isDegC;
         bool isMgKg;
         bool isMilliBar;
         int var_index;
         int n_other_file_obs  = 0;
         int n_other_total_obs = 0;
         int n_other_hdr_obs   = 0;
         int var_count = bufr_obs_name_arr.n_elements();
         for (int vIdx=0; vIdx<var_count; vIdx++) {
            int nlev2;
            ConcatString var_name;
            int var_name_len;
            var_name = bufr_obs_name_arr[vIdx];
            var_name_len = var_name.length();
            if (is_prepbufr &&
                  (prepbufr_vars.has(var_name, false)
                   || prepbufr_event_members.has(var_name)
                   || prepbufr_derive_vars.has(var_name))) continue;

            isDegC = false;
            isMgKg = false;
            isMilliBar = false;
            if (var_names.has(var_name, var_index, false)) {
               if ("DEG C" == var_units[var_index]) {
                  isDegC = true;
               }
               else if ("MB" == var_units[var_index]) {
                  isMilliBar = true;
               }
               else if ("MG/KG" == var_units[var_index]) {
                  isMgKg = true;
               }
            }

            readpbint_(&unit, &i_ret, &nlev2, bufr_obs,
                       (char*)var_name.c_str(), &var_name_len, &nlev_max_req);
            if (0 >= nlev2) continue;

            buf_nlev = nlev2;
            if (nlev2 > mxr8lv) {
               buf_nlev = mxr8lv;
               if (!variables_big_nlevels.has(var_name, false)) {
                  mlog << Warning << "\n" << method_name << " -> "
                       << "Too many vertical levels (" << nlev2
                       << ") for " << var_name
                       << ". Ignored the vertical levels above " << mxr8lv << ".\n\n";
                  variables_big_nlevels.add(var_name);
               }
            }
            mlog << Debug(10) << "var: " << var_name << " nlev2: " << nlev2
                 << ", vIdx: " << vIdx << ", obs_data_idx: "
                 << get_nc_obs_buf_index() << ", nlev: " << nlev << "\n";
            // Search through the vertical levels
            for(lv=0; lv<buf_nlev; lv++) {

               // If the observation vertical level is not within the
               // specified valid range, continue to the next vertical level
               if((lv+1) < conf_info.beg_level) continue;
               if((lv+1) > conf_info.end_level) break;

               // If the pressure level is not valid, continue to the next level
               if (is_prepbufr) {
                  if ((lv >= nlev) || is_eq(bufr_pres_lv[lv], fill_value)) continue;
               }

               if (bufr_obs[lv][0] > r8bfms) continue;
               mlog << Debug(10) << " value:   bufr_obs[" << lv << "][0]: " << bufr_obs[lv][0] << "\n";

               obs_arr[1] = (float)vIdx;        // BUFR variable index
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

               if (is_airnow) {
                  obs_arr[2]   = abs(bufr_obs_extra[lv][0] * 3600);  // hour to second
                  obs_arr[3]   = 0;                                  // AIRNOW obs at surface
                  quality_mark = bufr_obs_extra[lv][1];
                  // Convert a special number (1e+11) to NA at addObservation
                  if(conf_info.quality_mark_thresh < quality_mark) continue;
               }
               else {
                  // Retain the pressure in hPa for each observation record
                  obs_arr[2] = bufr_pres_lv[lv];
                  obs_arr[3] = bufr_msl_lv[lv];
                  if (is_eq(obs_arr[2], fill_value) && is_eq(obs_arr[3], fill_value) && 0 < nlev2) {
                     obs_arr[2] = lv;
                     obs_arr[3] = lv;
                  }
               }

               addObservation(obs_arr, (string)hdr_typ, (string)hdr_sid, hdr_vld_ut,
                     hdr_lat, hdr_lon, hdr_elv, quality_mark, OBS_BUFFER_SIZE);

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

      if (do_pbl) {
         pbl_level = 0;
         is_same_header = (prev_hdr_vld_ut == hdr_vld_ut)
               && is_eq(prev_hdr_lat, hdr_lat)
               && is_eq(prev_hdr_lon, hdr_lon)
               && is_eq(prev_hdr_elv, hdr_elv)
               && 0 == strcmp(prev_hdr_typ, hdr_typ)
               && 0 == strcmp(prev_hdr_sid, hdr_sid.c_str());
         has_pbl_data = (pqtzuv_map_tq.size() > 0 && pqtzuv_map_uv.size() > 0);
         if (is_same_header && has_pbl_data) {
            float pbl_value = compute_pbl(pqtzuv_map_tq, pqtzuv_map_uv);
            
            insert_pbl(obs_arr, pbl_value, pbl_code, pbl_p, pbl_h, pbl_qm,
                       hdr_lat, hdr_lon, hdr_elv, hdr_vld_ut, hdr_typ, hdr_sid);
         
            std::map<float,float*>::iterator it;
            for (it=pqtzuv_map_tq.begin(); it!=pqtzuv_map_tq.end(); ++it) {
               delete it->second;
            }
            for (it=pqtzuv_map_uv.begin(); it!=pqtzuv_map_uv.end(); ++it) {
               delete it->second;
            }
            pqtzuv_map_tq.clear();
            pqtzuv_map_uv.clear();
         }
         //is_same_header = false;
         prev_hdr_vld_ut = hdr_vld_ut;
         prev_hdr_lat = hdr_lat;
         prev_hdr_lon = hdr_lon;
         prev_hdr_elv = hdr_elv;
         prev_quality_mark = quality_mark;
         strncpy(prev_hdr_typ, hdr_typ, strlen(not_assigned));
         strncpy(prev_hdr_sid, hdr_sid.c_str(), strlen(not_assigned));
      }

      // If the number of observations for this header is non-zero,
      // store the header data and increment the PrepBufr record
      // counter
      if(n_hdr_obs > 0) {
         add_nc_header_to_array(modified_hdr_typ, hdr_sid.c_str(), hdr_vld_ut,
                                hdr_lat, hdr_lon, hdr_elv);
         if (is_prepbufr) {
            add_nc_header_prepbufr(pb_report_type, in_report_type, instrument_type);
         }

         i_msg++;
      }
      else {
         rej_nobs++;
      }
   } // end for i_read

   has_pbl_data = (pqtzuv_map_tq.size() > 0 || pqtzuv_map_uv.size() > 0);
   if (do_pbl && has_pbl_data) {
      float pbl_value = compute_pbl(pqtzuv_map_tq, pqtzuv_map_uv);
      insert_pbl(obs_arr, pbl_value, pbl_code, pbl_p, pbl_h, pbl_qm,
                 hdr_lat, hdr_lon, hdr_elv, hdr_vld_ut, hdr_typ, hdr_sid);
      
      //insert_pbl();
      std::map<float,float*>::iterator it;
      for (it=pqtzuv_map_tq.begin(); it!=pqtzuv_map_tq.end(); ++it) {
         delete it->second;
      }
      for (it=pqtzuv_map_uv.begin(); it!=pqtzuv_map_uv.end(); ++it) {
         delete it->second;
      }
      pqtzuv_map_tq.clear();
      pqtzuv_map_uv.clear();
   }

   if(showed_progress) {
      log_message = "100% ";
      if(mlog.verbosity_level() >= debug_level_for_performance) {
         end_t = clock();
         log_message << (end_t-start_t)/double(CLOCKS_PER_SEC) << " seconds";
         start_t = clock();
      }
      cout << log_message << "\n";
   }

   if(0 < diff_file_time_count && 0 < diff_file_times.n_elements()) {
      mlog << Warning << "\n" << method_name
           << " -> The observation time should remain the same for "
           << "all " << (is_prepbufr ? "PrepBufr" : "Bufr") << " messages\n";
      mlog << Warning << method_name << "   "
           << diff_file_time_count << " messages with different reference time ("
           << unix_to_yyyymmdd_hhmmss(file_ut) << "):\n";
      for (int idx=0; idx<diff_file_times.n_elements(); idx++) {
         mlog << Warning << method_name << "\t"
              << unix_to_yyyymmdd_hhmmss(diff_file_times[idx]) << "\n";
      }
      mlog << Warning << "\n";
   }
   int obs_buf_index = get_nc_obs_buf_index();
   if (obs_buf_index > 0) write_nc_obs_buffer(obs_buf_index);

   if(mlog.verbosity_level() > 0) cout << "\n" << flush;

   mlog << Debug(2)
        << "Total Messages processed\t\t= " << npbmsg << "\n"
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
        << "Total Messages retained\t\t= "
        << i_msg << "\n"
        << "Total observations retained or derived\t= "
        << n_file_obs << "\n";

   if (cal_cape) {
      mlog << Debug(3) << "\nDerived CAPE = " << cape_count
           << "\tZero = " << cape_cnt_zero_values
           << "\n\tnot derived: No cape inputs = " << (cape_cnt_no_levels)
           << "\tNo vertical levels = " << (cape_cnt_surface_msgs)
           << "\n\tfiltered: " << cape_cnt_missing_values << ", "
           << cape_cnt_too_big
           << "\n";
   }


   if (npbmsg == rej_vld && 0 < rej_vld) {
      mlog << Warning << "\n" << method_name << " -> "
           << "All messages were filtered out by valid time.\n"
           << "\tPlease adjust time range with \"-valid_beg\" and \"-valid_end\".\n"
           << "\tmin/max obs time from BUFR file: " << min_time_str
           << " and " << max_time_str << ".\n"
           << "\ttime range: " << start_time_str << " and " << end_time_str << ".\n";
   }
   else {
      mlog << Debug(1) << "Obs time between " << unix_to_yyyymmdd_hhmmss(min_msg_ut)
           << " and " << unix_to_yyyymmdd_hhmmss(max_msg_ut) << "\n";

      int debug_level = 5;
      if(mlog.verbosity_level() >= debug_level) {
         log_message = "Filtered time:";
         for (kk=0; kk<filtered_times.n_elements();kk++) {
            log_message.add((0 == (kk % 3)) ? "\n\t" : "  ");
            log_message.add(unix_to_yyyymmdd_hhmmss(filtered_times[kk]));
         }
         mlog << Debug(debug_level) << log_message << "\n";
      }
   }

   // Close the PREPBUFR file
   closepb_(&unit);

   // Delete the temporary blocked file
   remove_temp_file(blk_file);
   if(mlog.verbosity_level() >= debug_level_for_performance) {
      method_end = clock();
      cout << " PERF: " << method_name << " "
           << (method_end-method_start)/double(CLOCKS_PER_SEC)
           << " seconds\n";
   }

   if(i_msg <= 0) {
      mlog << Warning << "\n" << method_name << " -> "
           << "No " << (is_prepbufr ? "PrepBufr" : "Bufr")
           << " messages retained from file: "
           << pbfile[i_pb] << "\n\n";
      return;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_pbfile_metadata(int i_pb) {
   int npbmsg, unit;
   int i, i_msg, i_read, i_ret, i_date;
   int lv, var_index;
   int debug_threshold = 10;

   int tmp_nlev_max_req = (mxr8lv_small / 64);
   bool check_all = do_all_vars || collect_metadata;
   char hdr_typ[max_str_len];
   StringArray tmp_bufr_obs_name_arr;
   ConcatString file_name, blk_prefix, blk_file, blk_prefix2;
   static const char *method_name = "process_pbfile_metadata()";

   // Collects the BUFR variables including header variables
   bufr_hdr_name_arr.clear();

   mlog << Debug(1) << "Pre-processing BUFR File for variable names from "
                    << pbfile[i_pb] << "\n";

   // Set the file name for the PrepBufr file
   file_name << pbfile[i_pb];

   // Build the temporary block file name
   blk_prefix  << conf_info.tmp_dir << "/" << "tmp_pb2nc_meta_blk";
   blk_prefix2 << conf_info.tmp_dir << "/" << "tmp_pb2nc_tbl_blk";

   blk_file = make_temp_file_name(blk_prefix2.c_str(), NULL);

   mlog << Debug(3) << "   Blocking Bufr file (metadata) to:\t" << blk_file << "\n";

   // Assume that the input PrepBufr file is unblocked.
   // Block the PrepBufr file and open it for reading.
   pblock(file_name.c_str(), blk_file.c_str(), block);

   unit = dump_unit + i_pb + file_unit;
   ConcatString tbl_filename = save_bufr_table_to_file(blk_file.c_str(), unit);
   get_variable_info(tbl_filename.c_str());
   if(mlog.verbosity_level() < debug_threshold) remove_temp_file(tbl_filename);

   // Assume that the input PrepBufr file is unblocked.
   // Block the PrepBufr file and open it for reading.
   unit = dump_unit + i_pb;
   blk_file = make_temp_file_name(blk_prefix.c_str(), NULL);
   pblock(file_name.c_str(), blk_file.c_str(), block);
   if (unit > MAX_FORTRAN_FILE_ID || unit < MIN_FORTRAN_FILE_ID) {
      mlog << Error << "\n" << method_name << " -> "
           << "Invalid file ID [" << unit << "] between 1 and 99.\n\n";
   }

   // Open the blocked temp PrepBufr file for reading
   openpb_(blk_file.c_str(), &unit);

   // Compute the number of PrepBufr records in the current file.
   numpbmsg_(&unit, &npbmsg);
   mlog << Debug(1) << method_name << " -> "
        << "the number of records: " << npbmsg << "\n";

   // Use the number of records requested by the user if there
   // are enough present.
   if(nmsg >= 0 && nmsg <= npbmsg) {
      if (nmsg_percent < 0 || nmsg_percent >= 100)
         npbmsg = nmsg;
      else
         npbmsg = npbmsg * nmsg_percent / 100;
      mlog << Debug(2) << method_name << "  processing "
           << npbmsg << " records...\n";
   }

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
   i_ret = i_msg = 0;
   StringArray headers;
   StringArray tmp_hdr_array;
   headers.add(prepbufr_hdrs);
   tmp_hdr_array.clear();
   tmp_hdr_array.parse_wsss(bufr_avail_sid_names);
   headers.add(tmp_hdr_array);
   tmp_hdr_array.clear();
   tmp_hdr_array.parse_wsss(bufr_avail_latlon_names);
   headers.add(tmp_hdr_array);

   StringArray unchecked_var_list;
   if (check_all) {
      for(i=0; i<tableB_vars.n_elements(); i++) {
         if (!headers.has(tableB_vars[i]) && check_all) {
            unchecked_var_list.add(tableB_vars[i]);
         }
      }
   }

   //Initialize index for prepbufr common variables
   for (int index=0; index<mxr8vt; index++) {
      bufr_var_code[index] = bad_data_int;
   }

   int index;
   int length;
   bool is_prepbufr_hdr = false;
   bool showed_progress = false;
   // Loop through the PrepBufr messages from the input file
   for(i_read=0; i_read<npbmsg && i_ret == 0; i_read++) {

      if(mlog.verbosity_level() > 0) {
         if(nint(npbmsg/20.0) > 0 && (i_read+1)%nint(npbmsg/20.0) == 0) {
            showed_progress = true;
            cout << nint((double) (i_read+1)/npbmsg*100.0) << "% " << flush;
         }
      }

      // Get the next PrepBufr message
      ireadns_(&unit, hdr_typ, &i_date);
      cleanup_hdr_typ(hdr_typ);

      if (0 == i_read) {
         // Checks the variables for header

         int hdr_level = 1;
         char tmp_str[mxr8lv*mxr8pm];
         ConcatString var_name;
         ConcatString hdr_name_str;

         strncpy(tmp_str, prepbufr_hdrs_str, sizeof(tmp_str));
         length = strlen(tmp_str);
         readpbint_(&unit, &i_ret, &nlev, bufr_obs, tmp_str, &length, &hdr_level );
         is_prepbufr_hdr = (0 < nlev);
         if (is_prepbufr_hdr) {
            for (index=0; index<4; index++) {
               if (bufr_obs[0][index] > r8bfms) {
                  is_prepbufr_hdr = false;
                  break;
               }
            }
         }

         //bufr_hdr_name_arr.clear();
         if (is_prepbufr_hdr) {
            tmp_hdr_array.clear();
            tmp_hdr_array.parse_wsss(prepbufr_hdrs_str);
            for (index=0; index<tmp_hdr_array.n_elements(); index++) {
               if (!bufr_hdr_name_arr.has(tmp_hdr_array[index], false)) bufr_hdr_name_arr.add(tmp_hdr_array[index]);
            }
         }
         else {
            var_name = default_sid_name;
            strncpy(tmp_str, bufr_avail_sid_names, sizeof(tmp_str));
            length = strlen(tmp_str);
            readpbint_(&unit, &i_ret, &nlev, bufr_obs, tmp_str, &length, &hdr_level );
            if (0 < nlev) {
               tmp_hdr_array.clear();
               tmp_hdr_array.parse_wsss(bufr_avail_sid_names);
               for (index=0; index<tmp_hdr_array.n_elements(); index++) {
                  if (bufr_obs[0][index] < r8bfms) {
                     var_name = tmp_hdr_array[index];
                     if (!bufr_hdr_name_arr.has(var_name, false)) bufr_hdr_name_arr.add(var_name);
                     mlog << Debug(10) << "found station id: " << var_name << "=" << bufr_obs[0][index] << "\n";
                  }
               }
            }
            if (0 < hdr_name_str.length()) hdr_name_str.add(" ");
            hdr_name_str.add(var_name);

            strncpy(tmp_str, bufr_avail_latlon_names, sizeof(tmp_str));
            length = strlen(tmp_str);
            readpbint_(&unit, &i_ret, &nlev, bufr_obs, tmp_str, &length, &hdr_level );
            if (0 < nlev) {
               tmp_hdr_array.clear();
               tmp_hdr_array.parse_wsss(bufr_avail_latlon_names);
               var_name = default_lon_name;
               for (index=0; index<(tmp_hdr_array.n_elements()/2); index++) {
                  if (bufr_obs[0][index] < r8bfms) {
                     var_name = tmp_hdr_array[index];
                     if (!bufr_hdr_name_arr.has(var_name, false)) bufr_hdr_name_arr.add(var_name);
                     mlog << Debug(10) << "found  longitude: " << var_name << "=" << bufr_obs[0][index] << "\n";
                  }
               }
               if (0 < hdr_name_str.length()) hdr_name_str.add(" ");
               hdr_name_str.add(var_name);

               var_name = default_lat_name;
               for (index=(tmp_hdr_array.n_elements()/2); index<tmp_hdr_array.n_elements(); index++) {
                  if (bufr_obs[0][index] < r8bfms) {
                     var_name = tmp_hdr_array[index];
                     if (!bufr_hdr_name_arr.has(var_name, false)) bufr_hdr_name_arr.add(var_name);
                     mlog << Debug(10) << "found   latitude: " << var_name << "=" << bufr_obs[0][index] << "\n";
                  }
               }
               if (0 < hdr_name_str.length()) hdr_name_str.add(" ");
               hdr_name_str.add(var_name);
            }
            else {
               if (0 < hdr_name_str.length()) hdr_name_str.add(" ");
               if (!bufr_hdr_name_arr.has(default_lon_name, false)) hdr_name_str.add(default_lon_name);
               if (0 < hdr_name_str.length()) hdr_name_str.add(" ");
               if (!bufr_hdr_name_arr.has(default_lat_name, false)) hdr_name_str.add(default_lat_name);
            }

            ConcatString time_hdr_names;
            time_hdr_names.add("DHR");

            time_hdr_names.add(" ");
            if (event_names.has("YYMMDD", index)) {
               time_hdr_names.add(event_members[index]);
               tmp_hdr_array.clear();
               tmp_hdr_array.parse_wsss(event_members[index]);
               for (index=0; index<tmp_hdr_array.n_elements(); index++) {
                  if (!bufr_hdr_name_arr.has(tmp_hdr_array[index], false)) bufr_hdr_name_arr.add(tmp_hdr_array[index]);
               }
            }
            else {
               time_hdr_names.add(default_ymd_name);
               tmp_hdr_array.clear();
               tmp_hdr_array.parse_wsss(default_ymd_name);
               for (index=0; index<tmp_hdr_array.n_elements(); index++) {
                  if (!bufr_hdr_name_arr.has(tmp_hdr_array[index], false)) bufr_hdr_name_arr.add(tmp_hdr_array[index]);
               }
            }

            time_hdr_names.add(" ");
            if (event_names.has("HHMMSS", index)) {
               time_hdr_names.add(event_members[index]);
               tmp_hdr_array.clear();
               tmp_hdr_array.parse_wsss(event_members[index]);
               for (index=0; index<tmp_hdr_array.n_elements(); index++) {
                  if (!bufr_hdr_name_arr.has(tmp_hdr_array[index], false)) bufr_hdr_name_arr.add(tmp_hdr_array[index]);
               }
            }
            else if (event_names.has("HHMM", index)) {
               time_hdr_names.add(event_members[index]);
               time_hdr_names.add(" SECO");
               tmp_hdr_array.clear();
               tmp_hdr_array.parse_wsss(event_members[index]);
               for (index=0; index<tmp_hdr_array.n_elements(); index++) {
                  if (!bufr_hdr_name_arr.has(tmp_hdr_array[index], false)) bufr_hdr_name_arr.add(tmp_hdr_array[index]);
               }
               if (!bufr_hdr_name_arr.has("SECO")) bufr_hdr_name_arr.add("SECO");
            }
            else {
               time_hdr_names.add(default_hms_name);
               tmp_hdr_array.clear();
               tmp_hdr_array.parse_wsss(default_hms_name);
               for (index=0; index<tmp_hdr_array.n_elements(); index++) {
                  if (!bufr_hdr_name_arr.has(tmp_hdr_array[index], false)) bufr_hdr_name_arr.add(tmp_hdr_array[index]);
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
            // Remove variables for headers
            for (index=0; index<bufr_hdr_name_arr.n_elements(); index++) {
               if (unchecked_var_list.has(bufr_hdr_name_arr[index], var_index, false)) {
                  unchecked_var_list.shift_down(var_index, 1);
               }
            }
         }
         else {
            for (index=0; index<bufr_target_variables.n_elements(); index++) {
               const char *bufr_var_name = bufr_target_variables[index].c_str();
               if (!tableB_vars.has(bufr_var_name) && !prepbufr_derive_vars.has(bufr_var_name)) {
                  mlog << Error << "\n" << method_name << " -> variable \""
                       << bufr_var_name <<"\" does not exist at BUFR file\n\n";
                  exit(1);
               }
               if (!tmp_bufr_obs_name_arr.has(bufr_var_name, false)) {
                  tmp_bufr_obs_name_arr.add(bufr_var_name);
               }
            }
            break;
         }
      } // if (0 == i_read)

      if (0 == unchecked_var_list.n_elements()) break;

      int var_count = unchecked_var_list.n_elements();
      for (int vIdx=var_count-1; vIdx>=0; vIdx--) {
         int nlev2, buf_nlev;
         bool has_valid_data;
         ConcatString var_name = unchecked_var_list[vIdx];
         int var_name_len = var_name.length();

         readpbint_(&unit, &i_ret, &nlev2, bufr_obs, (char*)var_name.c_str(), &var_name_len, &tmp_nlev_max_req);
         if (0 >= nlev2) continue;
         
         // Search through the vertical levels
         buf_nlev = nlev2;
         if (nlev2 > mxr8lv) buf_nlev = mxr8lv;
         has_valid_data = false;
         int count = 0;
         for(lv=0; lv<buf_nlev; lv++) {
            if (bufr_obs[lv][0] < r8bfms) {
               has_valid_data = true;
               if (do_all_vars) {
                  if (0 == count) {
                     mlog << Debug(5) << " found valid data: " << var_name
                          << " (" << bufr_obs[lv][0] << ") at level index "
                          << lv << "\n";
                  }
                  count++;
                  if (COUNT_THRESHOLD < count) break;
               }
            }
            break;
         }  //end for lv
         
         if (has_valid_data) {
            if (!tmp_bufr_obs_name_arr.has(var_name, false)
                && !bufr_hdr_name_arr.has(var_name, false)) {
               tmp_bufr_obs_name_arr.add(var_name);
            }
            if (do_all_vars) {
               if (var_name == unchecked_var_list[vIdx])
                  unchecked_var_list.shift_down(vIdx, 1);
            }
            else {
               StringArray typeArray;
               if (0 < variableTypeMap.count(var_name)) typeArray = variableTypeMap[var_name];
               if (!typeArray.has(hdr_typ, false)) {
                  typeArray.add(hdr_typ);
                  variableTypeMap[var_name] = typeArray;
               }
            }
         }
      }  // end for vIdx
   } // end for
   if(showed_progress && mlog.verbosity_level() > 0) cout << "\n";

   int bufr_var_index = 0;
   bool has_prepbufr_vars = false;
   const char * tmp_var_name;
   bufr_obs_name_arr.clear();
   for (index=0; index<prepbufr_vars.n_elements(); index++) {
      tmp_var_name = prepbufr_vars[index].c_str();
      if (do_all_vars || bufr_target_variables.has(tmp_var_name, false)) {
         if (tableB_vars.has(tmp_var_name)) {
            bufr_obs_name_arr.add(tmp_var_name);
            bufr_var_code[index] = bufr_var_index;
            bufr_var_index++;
            has_prepbufr_vars = true;
         }
      }
   }
   bufr_derive_cfgs.clear();
   if (has_prepbufr_vars) {
      for (int vIdx=0; vIdx< prepbufr_derive_vars.n_elements(); vIdx++) {
         tmp_var_name = prepbufr_derive_vars[vIdx].c_str();
         bufr_derive_cfgs.push_back(derive_var_cfg(tmp_var_name));
         if (do_all_vars || bufr_target_variables.has(tmp_var_name)) {
            // Set the variable index if requested
            bufr_obs_name_arr.add(tmp_var_name);
            bufr_derive_cfgs[vIdx].var_index = bufr_var_index;
            bufr_var_index++;
         }
      }
   }
   for (i=0; i<tmp_bufr_obs_name_arr.n_elements(); i++) {
      if (!bufr_obs_name_arr.has(tmp_bufr_obs_name_arr[i], false)) {
         bufr_obs_name_arr.add(tmp_bufr_obs_name_arr[i]);
      }
   }

   // Close the PREPBUFR file
   closepb_(&unit);

   // Delete the temporary blocked file
   remove_temp_file(blk_file);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_netcdf_hdr_data() {
   long dim_count, pb_hdr_count;
   bool is_prepbufr = is_prepbufr_file(&event_names);
   static const string method_name = "\nwrite_netcdf_hdr_data()";

   pb_hdr_count = (long) get_nc_hdr_cur_index();
   if (obs_to_vector) {
      int deflate_level = compress_level;
      if (deflate_level < 0) deflate_level = conf_info.conf.nc_compression();

      nc_out_data.processed_hdr_cnt = pb_hdr_count;
      nc_out_data.deflate_level = deflate_level;
      nc_out_data.observations = observations;
      nc_out_data.summary_obs = summary_obs;
      nc_out_data.summary_info = conf_info.getSummaryInfo();

      init_netcdf_output(f_out, obs_vars, nc_out_data, program_name);
      dim_count = obs_vars.hdr_cnt;
   }
   else {
      dim_count = pb_hdr_count;
      if (do_summary) {
         int summary_hdr_cnt = summary_obs->countSummaryHeaders();
         if (save_summary_only)
            dim_count = summary_hdr_cnt;
         else
            dim_count += summary_hdr_cnt;
      }
   }

   // Check for no messages retained
   if(dim_count <= 0) {
      mlog << Error << method_name << " -> "
           << "No PrepBufr messages retained.  Nothing to write.\n\n";
      // Delete the NetCDF file
      remove_temp_file(ncfile);
      exit(1);
   }

   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = conf_info.conf.nc_compression();
   if (is_prepbufr) {
      if (!nc_out_data.summary_info.flag || (nc_out_data.summary_info.flag && nc_out_data.summary_info.raw_data))
         create_nc_pb_hdrs(obs_vars, f_out, pb_hdr_count, deflate_level);
   }

   // Make sure all obs data is processed before handling header
   if (obs_to_vector) {
      write_observations(f_out, obs_vars, nc_out_data);
   }
   else {
      if (do_summary) {
         // Write out the summary data
         if (save_summary_only) reset_header_buffer(pb_hdr_count, true);
         write_nc_observations(obs_vars, summary_obs->getSummaries());
         mlog << Debug(4) << "write_netcdf_hdr_data obs count: "
              << (int)summary_obs->getObservations().size()
              << "  summary count: " << (int)summary_obs->getSummaries().size()
              << " header count: " << dim_count
              << " summary header count: " << (dim_count-pb_hdr_count) << "\n";

         TimeSummaryInfo summaryInfo = conf_info.getSummaryInfo();
         if (summaryInfo.flag) {
            write_summary_attributes(f_out, summaryInfo);
         }
      }

      create_nc_hdr_vars(obs_vars, f_out, dim_count, deflate_level);
      create_nc_table_vars(obs_vars, f_out, deflate_level);

      // Write out the header data
      write_nc_arr_headers(obs_vars);
      if (get_nc_hdr_cur_index() > 0) {
         // Write out the remaining header data
         write_nc_buf_headers(obs_vars);
      }
   }

   StringArray nc_var_name_arr;
   StringArray nc_var_unit_arr;
   StringArray nc_var_desc_arr;
   map<ConcatString, ConcatString> obs_var_map = conf_info.getObsVarMap();
   for(int i=0; i<bufr_obs_name_arr.n_elements(); i++) {
      int var_index;
      std::string var_name;
      ConcatString unit_str, desc_str;

      unit_str = "";
      var_name = bufr_obs_name_arr[i];
      if (var_names.has(var_name, var_index, false)) {
         unit_str = var_units[var_index];
         if (0 == strcmp("DEG C", unit_str.c_str())) {
            unit_str = "KELVIN";
         }
         else if (0 == strcmp("MB", unit_str.c_str())) {
            unit_str = "PASCALS";
         }
         else if (0 == strcmp("MG/KG", unit_str.c_str())) {
            unit_str = "KG/KG";
         }
      }
      nc_var_unit_arr.add(unit_str);

      desc_str = (tableB_vars.has(var_name, var_index))
            ? tableB_descs[var_index]
            : "";
      nc_var_desc_arr.add(desc_str);

      ConcatString grib_name = obs_var_map[var_name];
      if (0 < grib_name.length()) {
         var_name = grib_name;
      }
      nc_var_name_arr.add(var_name);

   } // end for i

   dim_count = bufr_obs_name_arr.n_elements();
   create_nc_obs_name_vars (obs_vars, f_out, dim_count, dim_count, deflate_level);
   write_obs_var_names (obs_vars,  nc_var_name_arr);
   write_obs_var_units (obs_vars, nc_var_unit_arr);
   write_obs_var_descriptions (obs_vars, nc_var_desc_arr);

   return;
}

////////////////////////////////////////////////////////////////////////

void addObservation(const float *obs_arr, const ConcatString &hdr_typ,
      const ConcatString &hdr_sid, const time_t hdr_vld,
      const float hdr_lat, const float hdr_lon, const float hdr_elv,
      const float quality_mark, const int buf_size)
{
   // Write the quality flag to the netCDF file
   ConcatString obs_qty;
   int quality_code = nint(quality_mark);
   if (quality_code == bad_data_int || quality_mark > r8bfms) {
      obs_qty.add("NA");
   }
   else {
      obs_qty.format("%d", quality_code);
   }

   if (obs_to_vector) {
      int var_index = obs_arr[1];
      //assert(var_index >= 0 && strlen("Variable index can't be negative"));
      string var_name = bufr_obs_name_arr[var_index];
      Observation obs = Observation(hdr_typ.text(),
                                   hdr_sid.text(),
                                   hdr_vld,
                                   hdr_lat, hdr_lon, hdr_elv,
                                   obs_qty.text(),
                                   var_index,
                                   obs_arr[2], obs_arr[3], obs_arr[4],
                                   var_name);
      obs.setHeaderIndex(obs_arr[0]);
      observations.push_back(obs);
      if (do_summary) summary_obs->addObservationObj(obs);
   }
   else {
      if (!save_summary_only)
          write_nc_observation(obs_vars, obs_arr, obs_qty.text());
      if (do_summary) {
         string var_name = bufr_obs_name_arr[(obs_arr[1])];
         conf_info.getSummaryInfo();
         summary_obs->addObservation(
               hdr_typ.text(),
               hdr_sid.text(),
               hdr_vld,
               hdr_lat, hdr_lon, hdr_elv,
               obs_qty.text(),
               obs_arr[1], obs_arr[2], obs_arr[3], obs_arr[4],
               var_name);
      }
   }
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

   ev = 0;
   // Check the event_stack_flag to determine if the top or bottom
   // of the event stack is to be used
   //   Top of the stack:    ev = 0
   //   Bottom of the stack: ev > 0
   if(!flag) {
      // If the bottom of the event stack is to be used, find the
      // correct event index

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

static void cleanup_hdr_typ(char *hdr_typ, bool is_prepbufr) {
   if (is_prepbufr) hdr_typ[6] = '\0';
   else             hdr_typ[mxr8nm] = '\0';
   // Change the trailing blank space to a null
   for (int i=0; i<mxr8nm; i++) {
      if (' ' == hdr_typ[i]) {
         hdr_typ[i] = '\0';
         break;
      }
   }
}

////////////////////////////////////////////////////////////////////////

void dbl2str(double *d, ConcatString & str) {
   const char *fmt_str = "%s";

   str.format(fmt_str, d);
   if (0 == str.length()) {
     str = "NA";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool keep_message_type(const char *mt_str) {
   bool keep = false;

   keep = conf_info.message_type.n_elements() == 0 ||
          conf_info.message_type.has(mt_str, false);

   return(keep);
}

////////////////////////////////////////////////////////////////////////

bool keep_station_id(const char *sid_str) {

   return(conf_info.station_id.n_elements() == 0 ||
          conf_info.station_id.has(sid_str, false));
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
          conf_info.pb_report_type.has(type, false));
}

////////////////////////////////////////////////////////////////////////

bool keep_in_report_type(int type) {
   return(conf_info.in_report_type.n_elements() == 0 ||
          conf_info.in_report_type.has(type, false));
}

////////////////////////////////////////////////////////////////////////

bool keep_instrument_type(int type) {
   return(conf_info.instrument_type.n_elements() == 0 ||
          conf_info.instrument_type.has(type, false));
}

////////////////////////////////////////////////////////////////////////

bool keep_bufr_obs_index(int code) {
   return(code >= 0 && (do_all_vars || code < bufr_target_variables.n_elements()));
}


////////////////////////////////////////////////////////////////////////

bool keep_level_category(int category) {
   return(conf_info.level_category.n_elements() == 0 ||
          conf_info.level_category.has(category, false));
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
         result = fill_value;
         break;
   } // end switch

   return(result);
}

////////////////////////////////////////////////////////////////////////

void display_bufr_variables(const StringArray &all_vars, const StringArray &all_descs,
                            const StringArray &hdr_arr, const StringArray &obs_arr) {
   int i, index;
   ConcatString description;
   ConcatString line_buf;

   mlog << Debug(1) << "\n   Header variables (" << hdr_arr.n_elements() << ") :\n";
   for(i=0; i<hdr_arr.n_elements(); i++) {
      if (all_vars.has(hdr_arr[i], index, false)) {
         description = all_descs[index];
      }
      else {
         description = "";
      }
      line_buf.format("   %8s: %s\n", hdr_arr[i].c_str(), description.c_str());
      mlog << Debug(1) << line_buf;
   }

   mlog << Debug(1) << "\n   Observation variables (" << obs_arr.n_elements()
        << ")  Name: Description                       Types:\n";

   for(i=0; i<obs_arr.n_elements(); i++) {
      if (all_vars.has(obs_arr[i], index, false)) {
         description = all_descs[index];
      }
      else {
         description = "";
      }
      if (0 < variableTypeMap.count(obs_arr[i])) {
         ConcatString message_types;
         StringArray typeArray = variableTypeMap[obs_arr[i]];
         
         int type_cnt = typeArray.n_elements();
         for (int ii=0; ii<type_cnt; ii++) {
            if (message_types.nonempty()) message_types.add(" ");
            message_types.add(typeArray[ii]);
         }
         line_buf.format("  %6s: %-55s\t%s\n", obs_arr[i].c_str(),
               description.c_str(), message_types.c_str());
      }
      else {
         line_buf.format("   %8s: %s\n", obs_arr[i].c_str(), description.c_str());
      }
      mlog << Debug(1) << line_buf;
   }
}

////////////////////////////////////////////////////////////////////////

void copy_pqtzuv(float *to_pqtzuv, float *from_pqtzuv, bool copy_all) {
   int start_idx = (copy_all ? 0 : 1);
   for (int index = start_idx; index < mxr8vt; index++) {
      if (copy_all || !is_eq(from_pqtzuv[index], bad_data_float))
         to_pqtzuv[index] = from_pqtzuv[index];
   }
}

////////////////////////////////////////////////////////////////////////

int combine_tqz_and_uv(map<float, float*> pqtzuv_map_tq,
      map<float, float*> pqtzuv_map_uv, map<float, float*> &pqtzuv_map_merged) {
   int common_count = 0;
   static const char *method_name = "combine_tqz_and_uv() ";
   int tq_count = pqtzuv_map_tq.size();
   int uv_count = pqtzuv_map_uv.size();
   if (tq_count > 0 && uv_count > 0) {
      IntArray common_levels, tq_levels;
      float *pqtzuv_tq, *pqtzuv_uv;
      float *pqtzuv_merged = (float *) 0;
      float *next_pqtzuv, *prev_pqtzuv;
      std::map<float,float*>::iterator it, it_tq, it_uv;

      // Gets pressure levels for TQZ records
      for (it=pqtzuv_map_tq.begin(); it!=pqtzuv_map_tq.end(); ++it) {
         tq_levels.add(int(it->first));
      }
      // Gets pressure levels for common records
      for (it=pqtzuv_map_uv.begin(); it!=pqtzuv_map_uv.end(); ++it) {
         if (tq_levels.has(int(it->first))) {
            common_levels.add(int(it->first));
         }
      }

      if(mlog.verbosity_level() >= PBL_DEBUG_LEVEL) {
         log_tqz_and_uv(pqtzuv_map_tq, pqtzuv_map_uv, method_name);
      }

      // Select first record by 1) merging two records with the same pressure
      // level or 2) interpolate
      next_pqtzuv = (float *)0;
      it_tq = pqtzuv_map_tq.begin();
      it_uv = pqtzuv_map_uv.begin();
      pqtzuv_tq = (float *)it_tq->second;
      pqtzuv_uv = (float *)it_uv->second;;
      pqtzuv_merged = new float[mxr8vt];
      if (common_levels.has(int(it_tq->first)) 
            || common_levels.has(int(it_uv->first))) {
         // Found the records with the same precsure level
         if (it_tq->first != it_uv->first) {
            if (common_levels.has(int(it_uv->first))) {
               pqtzuv_uv = pqtzuv_map_uv[it_uv->first];
            }
            else if (common_levels.has(int(it_tq->first))) {
               pqtzuv_tq = pqtzuv_map_tq[it_tq->first];
            }
         }
         copy_pqtzuv(pqtzuv_merged, pqtzuv_tq);
         copy_pqtzuv(pqtzuv_merged, pqtzuv_uv, false);
      }
      else {
         if (it_tq->first > it_uv->first) {
            //Interpolate UV into TQZ
            prev_pqtzuv = pqtzuv_uv;
            copy_pqtzuv(pqtzuv_merged, pqtzuv_tq);
            while (it_tq->first > it_uv->first && it_uv != pqtzuv_map_uv.end()) {
               prev_pqtzuv = (float *)it_uv->second;
               ++it_uv;
            }
            next_pqtzuv = it_uv->second;
         }
         else {
            //Interpolate TQZ into UV
            prev_pqtzuv = pqtzuv_tq;
            copy_pqtzuv(pqtzuv_merged, pqtzuv_uv);
            while (it_tq->first < it_uv->first && it_tq != pqtzuv_map_tq.end()) {
               prev_pqtzuv = (float *)it_tq->second;
               ++it_tq;
            }
            next_pqtzuv = it_tq->second;
         }
         interpolate_pqtzuv(prev_pqtzuv, pqtzuv_merged, next_pqtzuv);
      }
      float first_pres = (pqtzuv_merged == 0 ? bad_data_float : pqtzuv_merged[0]);
      pqtzuv_map_merged[first_pres] = pqtzuv_merged;
      mlog << Debug(9) << method_name << "Added " << first_pres << " to merged records\n";

      if (pqtzuv_merged != 0) {
         //Merge UV into TQZ records
         merge_records(pqtzuv_merged, pqtzuv_map_tq, pqtzuv_map_uv, pqtzuv_map_merged);
         //Merge TQZ into UV records
         merge_records(pqtzuv_merged, pqtzuv_map_uv, pqtzuv_map_tq, pqtzuv_map_merged);
      }

      if(mlog.verbosity_level() >= PBL_DEBUG_LEVEL) {
         log_merged_tqz_uv(pqtzuv_map_tq, pqtzuv_map_uv, pqtzuv_map_merged, method_name);
      }
   }

   return pqtzuv_map_merged.size();
}

////////////////////////////////////////////////////////////////////////

float compute_pbl(map<float, float*> pqtzuv_map_tq,
                  map<float, float*> pqtzuv_map_uv) {
   int jpbl;        //       -      BOUNDARY-LAYER TOP (LEVEL NUMBER - INTEGER)
   int mzbl;        //       -      TOP OF MODEL DOMAIN (LEVEL NUMBER - INTEGER)
   float hpbl;      //       M      ATMOSPHERIC BOUNDARY-LAYER DEPTH
   int index;
   int pbl_level;
   int tq_count = pqtzuv_map_tq.size();
   int uv_count = pqtzuv_map_uv.size();
   std::map<float,float*>::iterator it;
   static const char *method_name = "compute_pbl() ";

   hpbl = bad_data_float;
   mlog << Debug(7) << method_name << "is called: TQZ: "
        << tq_count << "  UV: " << uv_count << "\n";
   if (tq_count > 0 || uv_count > 0) {
      int hgt_cnt, spfh_cnt;
      IntArray selected_levels;

      map<float, float*> pqtzuv_map_merged;
      pbl_level = combine_tqz_and_uv(pqtzuv_map_tq, pqtzuv_map_uv,
                                     pqtzuv_map_merged);
      mlog << Debug(7) << method_name << "pbl_level= " << pbl_level
           << " from TQ (" << tq_count << ") and UV (" << uv_count
           << ")\n";

      // Order all observations by pressure from bottom to top
      index = pbl_level - 1;
      hgt_cnt = spfh_cnt = 0;
      for (it=pqtzuv_map_merged.begin(); it!=pqtzuv_map_merged.end(); ++it) {
         if (index < 0) {
            mlog << Error << method_name  << "negative index: " << index << "\n";
            break;
         }

         if (index < MAX_PBL_LEVEL) {
            float *pqtzuv = it->second;
            pbl_data_pres[index] = pqtzuv[0];
            pbl_data_spfh[index] = pqtzuv[1];
            pbl_data_temp[index] = pqtzuv[2];
            pbl_data_hgt[index]  = pqtzuv[3];
            pbl_data_ugrd[index] = pqtzuv[4];
            pbl_data_vgrd[index] = pqtzuv[5];
            if (!is_eq(pbl_data_spfh[index], bad_data_float)) spfh_cnt++;
            if (!is_eq(pbl_data_hgt[index], bad_data_float)) hgt_cnt++;
            selected_levels.add(int(it->first));
         }

         index--;
      }
      if (index != -1) {
         mlog << Error << method_name  << "Missing some levels (" << index << ")\n";
      }

      if (pbl_level > MAX_PBL_LEVEL) {
         it = pqtzuv_map_tq.begin();
         // Find vertical levels with both data
         float highest_pressure = bad_data_float;
         for (; it!=pqtzuv_map_tq.end(); ++it) {
            if (pqtzuv_map_uv.count(it->first) > 0) {
               highest_pressure = it->first;
               break;
            }
         }
         if (!is_eq(highest_pressure, bad_data_float)) {
            index = MAX_PBL_LEVEL - 1;
            for (; it!=pqtzuv_map_tq.end(); ++it) {
               int pres_level = int(it->first);
               if (selected_levels.has(pres_level)) break;
               
               float *pqtzuv = pqtzuv_map_merged[it->first];
               pbl_data_pres[index] = pqtzuv[0];
               pbl_data_spfh[index] = pqtzuv[1];
               pbl_data_temp[index] = pqtzuv[2];
               pbl_data_hgt[index]  = pqtzuv[3];
               pbl_data_ugrd[index] = pqtzuv[4];
               pbl_data_vgrd[index] = pqtzuv[5];
               mlog << Debug(5) << method_name << "Force to add "
                    << pres_level << " into " << index << "\n";
               index--;
            }
         }
      }
      else {
         // Clear buffer
         for (int idx=pbl_level; idx<MAX_PBL_LEVEL; idx++) {
            pbl_data_pres[idx] = bad_data_float;
            pbl_data_temp[idx] = bad_data_float;
            pbl_data_spfh[idx] = bad_data_float;
             pbl_data_hgt[idx] = bad_data_float;
            pbl_data_ugrd[idx] = bad_data_float;
            pbl_data_vgrd[idx] = bad_data_float;
         }
      }

      if (hgt_cnt < pbl_level) {
         hgt_cnt += interpolate_by_pressure(pbl_level, pbl_data_pres, pbl_data_hgt);
         mlog << Debug(4) << method_name << "interpolate Z (HGT)\n";
      }
      if (spfh_cnt < pbl_level) {
         spfh_cnt += interpolate_by_pressure(pbl_level, pbl_data_pres, pbl_data_spfh);
         mlog << Debug(4) << method_name << "interpolate Q (SPFH)\n";
      }

      if ((spfh_cnt>0) && (pbl_level>0)) {
         mzbl = pbl_level;
         mlog << Debug(PBL_DEBUG_LEVEL) << method_name << "mzbl: " << mzbl
              << "  missing count: Q: " << (pbl_level - spfh_cnt)
              << ", Z: " << (pbl_level - hgt_cnt) << "\n\n";
         if(mlog.verbosity_level() >= PBL_DEBUG_LEVEL) {
            log_pbl_input(pbl_level, method_name);
         }

         //SUBROUTINE CALPBL(T,Q,P,Z,U,V,MZBL,HPBL,jpbl)
         calpbl_(pbl_data_temp, pbl_data_spfh, pbl_data_pres, pbl_data_hgt,
                 pbl_data_ugrd, pbl_data_vgrd, &mzbl, &hpbl, &jpbl);
         if (is_eq(hpbl, bad_data_float))
            mlog << Debug(5) << method_name << " fail to compute PBL. TQ records: "
                 << tq_count << " UV records: " << uv_count << " merged records: "
                 << pqtzuv_map_merged.size() << "\n";
      }
   }
   return hpbl;
}

////////////////////////////////////////////////////////////////////////

void insert_pbl(float *obs_arr, const float pbl_value, const int pbl_code,
                const float pbl_p, const float pbl_h, const float pbl_qm,
                const float hdr_lat, const float hdr_lon,
                const float hdr_elv, const time_t hdr_vld_ut, 
                const ConcatString &hdr_typ, const ConcatString &hdr_sid) {
   ConcatString hdr_info;
   hdr_info << unix_to_yyyymmdd_hhmmss(hdr_vld_ut)
            << " " << hdr_typ << " " << hdr_sid;
   if (is_eq(pbl_value, bad_data_float)) {
      mlog << Warning << "Failed to compute PBL " << hdr_info << "\n\n";
   }
   else if (pbl_value < hdr_elv) {
      mlog << Warning << "Not saved because the computed PBL (" << pbl_value
           << ") is less than the station elevation (" << hdr_elv
           << "). " << hdr_info << "\n\n";
      obs_arr[4] = 0;
   }
   else {
      obs_arr[1] = pbl_code;
      obs_arr[2] = pbl_p;
      obs_arr[3] = pbl_h;
      obs_arr[4] = pbl_value - hdr_elv; // observation value
      mlog << Debug(7) << " hpbl: " << pbl_value << ", obs_arr[4]: " << obs_arr[4]
           << "   lat: " << hdr_lat << ", lon: " << hdr_lon
           << ", elv: " << hdr_elv << " " << hdr_info << "\n\n";
      if (obs_arr[4] > MAX_PBL) {
         mlog << Warning << " Computed PBL (" << obs_arr[4] << " from "
              << pbl_value << ") is too high, Reset to " << MAX_PBL
              << "  " << hdr_info<< "\n\n";
         obs_arr[4] = MAX_PBL;
      }

      addObservation(obs_arr, (string)hdr_typ, (string)hdr_sid, hdr_vld_ut,
                     hdr_lat, hdr_lon, hdr_elv, pbl_qm, OBS_BUFFER_SIZE);
   }
}

////////////////////////////////////////////////////////////////////////

int interpolate_by_pressure(int length, float *pres_data, float *var_data) {
   int idx, idx2, idx_start, idx_end;
   int count_interpolated;
   bool skip_missing;
   static const char *method_name = "interpolate_by_pressure() ";

   idx_start = -1;
   //idx_end = length;
   skip_missing = false;
   count_interpolated = 0;
   for (idx=0; idx<length; idx++) {
      if (is_eq(var_data[idx], bad_data_float)) {
         skip_missing = true;
      }
      else {
         if (skip_missing) {
            idx_end = idx;
            if (idx_start >= 0) {
               mlog << Debug(7) << method_name << " index between "
                    << idx_start <<" and " << idx_end <<", data between "
                    << var_data[idx_start] << " and " << var_data[idx_end] << "\n";
               float data_diff = var_data[idx_end] - var_data[idx_start];
               for (idx2 = idx_start+1; idx2<idx_end; idx2++) {
                  float pres_ratio = (pres_data[idx2] - pres_data[idx_start])
                        / (pres_data[idx_end] - pres_data[idx_start]);
                  var_data[idx2] = var_data[idx_start] + (data_diff * pres_ratio);
                  mlog << Debug(7) << method_name << " interpolated value["
                       << idx2 << "] = " << var_data[idx2] << "\n";
                  count_interpolated++;
               }
               idx_start = idx;
               skip_missing = false;
            }
         }
         else {
            idx_start = idx;
         }
      }
   }
   mlog << Debug(5) << method_name << "interpolated count: "
        << count_interpolated << "\n";

   return count_interpolated;
}

////////////////////////////////////////////////////////////////////////

void interpolate_pqtzuv(float *prev_pqtzuv, float *cur_pqtzuv, float *next_pqtzuv) {
   static const char *method_name = "interpolate_pqtzuv() ";

   if ((prev_pqtzuv[0] == cur_pqtzuv[0]) || (next_pqtzuv[0] == cur_pqtzuv[0])) {
      mlog << Error << method_name 
              << "  Can't interpolate because of same pressure levels. prev: "
              << prev_pqtzuv[0] << ", cur: " << cur_pqtzuv[0]
              << ", next: " <<  prev_pqtzuv[0] << "\n";
   }
   else {
      float p_ratio = (cur_pqtzuv[0] - prev_pqtzuv[0]) / (next_pqtzuv[0] - prev_pqtzuv[0]);
      float p_ratio_log = (log(cur_pqtzuv[0]) - log(prev_pqtzuv[0]))
            / (log(next_pqtzuv[0]) - log(prev_pqtzuv[0]));
      for (int index=1; index < mxr8vt; index++) {
         if (is_eq(cur_pqtzuv[index], bad_data_float)) {
            float prev_value, next_value, ratio_pres;
            prev_value = prev_pqtzuv[index];
            next_value = next_pqtzuv[index];
            if (is_eq(prev_value, bad_data_float) || is_eq(next_value, bad_data_float)) {
               if ((!IGNORE_Q_PBL && index==1) || (!IGNORE_Z_PBL && index==3)) {
                  mlog << Warning << method_name << "   Missing value for "
                       << (is_eq(prev_value, bad_data_float) ? "previous" : "next")
                       << " data for index " << index << " at pressure "
                       << (is_eq(prev_value, bad_data_float) ? prev_pqtzuv[0] : next_pqtzuv[0])
                       << "\n";
               }
            }
            else {
               ratio_pres = USE_LOG_INTERPOLATION ? p_ratio_log : p_ratio;
               cur_pqtzuv[index] = prev_value + (next_value - prev_value) * ratio_pres;
            }
         }
      }
      mlog << Debug(9) << method_name << " pressure level [" << cur_pqtzuv[0]
          << "] between " << prev_pqtzuv[0] << " and " << next_pqtzuv[0] << "\n";
   }
}

////////////////////////////////////////////////////////////////////////

void merge_records(float *first_pqtzuv, map<float, float*> pqtzuv_map_pivot,
      map<float, float*> pqtzuv_map_aux, map<float, float*> &pqtzuv_map_merged) {

   float cur_pres, next_pres;
   float *cur_pqtzuv, *next_pqtzuv, *prev_pqtzuv;
   float *pqtzuv_merged;
   std::map<float,float*>::iterator it_pivot, it_aux;
   static const char *method_name = "merge_records() ";

   float first_pres = first_pqtzuv[0];
   prev_pqtzuv = first_pqtzuv;
   it_aux = pqtzuv_map_aux.begin();
   for (it_pivot=pqtzuv_map_pivot.begin(); it_pivot!=pqtzuv_map_pivot.end(); ++it_pivot) {
      // Skip the records below the first mathcing/interpolated level
      if (first_pres < it_pivot->first) break;
   }
   mlog << Debug(8) << method_name << "pivot->first: " << it_pivot->first
        << "  aux->first " << it_aux->first << "  first_pres: " << first_pres
        << "  prev_pqtzuv[0]" << prev_pqtzuv[0] << "\n";
   // Find next UV level
   for (; it_aux!=pqtzuv_map_aux.end(); ++it_aux) {
      // Skip the records below the first mathcing/interpolated level
      if (it_pivot->first < it_aux->first) {
         next_pres = it_aux->first;
         break;
      }
      prev_pqtzuv = (float *)it_aux->second;
   }
   for (; it_pivot!=pqtzuv_map_pivot.end(); ++it_pivot) {
      cur_pres = (float)it_pivot->first;
      // Skip if already merged
      if (0 < pqtzuv_map_merged.count(cur_pres)) {
         mlog << Debug(9) << method_name << cur_pres << ": merged already\n";
         continue;
      }

      cur_pqtzuv = (float *)it_pivot->second;
      pqtzuv_merged = new float[mxr8vt];
      copy_pqtzuv(pqtzuv_merged, cur_pqtzuv);
      // Advance aux record if necessary
      if (prev_pqtzuv[0] < cur_pres) {
         for (; it_aux!=pqtzuv_map_aux.end(); ++it_aux) {
            if (it_aux->first > cur_pres) break;
            prev_pqtzuv = (float *)it_aux->second;
         }
         //Stop if no more aux records
         if (it_aux == pqtzuv_map_aux.end()) {
            if (prev_pqtzuv[0] == pqtzuv_merged[0]) {
               copy_pqtzuv(pqtzuv_merged, prev_pqtzuv, false);
            }
            mlog << Debug(9) << method_name << "Added " << cur_pres << "\n";
            pqtzuv_map_merged[cur_pres] = pqtzuv_merged;
            break;
         }
      }
      
      if (it_aux->first == cur_pres) {
         copy_pqtzuv(pqtzuv_merged, it_aux->second, false);
         prev_pqtzuv = pqtzuv_merged;
      }
      else if (prev_pqtzuv[0] == pqtzuv_merged[0] ) {
         copy_pqtzuv(pqtzuv_merged, prev_pqtzuv, false);
      }
      else {
         next_pqtzuv = (float *)it_aux->second;
         interpolate_pqtzuv(prev_pqtzuv, pqtzuv_merged, next_pqtzuv);
      }
      mlog << Debug(9) << method_name << "Added " << cur_pres << "\n";
      pqtzuv_map_merged[cur_pres] = pqtzuv_merged;
   }
}

////////////////////////////////////////////////////////////////////////

void log_tqz_and_uv(map<float, float*> pqtzuv_map_tq,
                    map<float, float*> pqtzuv_map_uv, const char *method_name) {
   int offset;
   ConcatString buf;
   StringArray log_array;
   std::map<float,float*>::iterator it;

   for (it=pqtzuv_map_tq.begin(); it!=pqtzuv_map_tq.end(); ++it) {
      float *pqtzuv = it->second;
      buf.clear();
      for (int idx=0; idx<mxr8vt; idx++) {
         buf << " " << pqtzuv[idx];
      }
      log_array.add(buf.c_str());
   }
   offset = 0;
   for (int idx=log_array.n_elements()-1; idx>=0; idx--) {
      mlog << Debug(PBL_DEBUG_LEVEL) << method_name << "TQZ record: "
           << offset++ << "\t" << log_array[idx] << "\n";
   }
   log_array.clear();
   for (it=pqtzuv_map_uv.begin(); it!=pqtzuv_map_uv.end(); ++it) {
      float *pqtzuv = it->second;
      buf.clear();
      for (int idx=0; idx<mxr8vt; idx++) {
         buf << " " << pqtzuv[idx];
      }
      log_array.add(buf.c_str());
   }
   offset = 0;
   for (int idx=log_array.n_elements()-1; idx>=0; idx--) {
      mlog << Debug(PBL_DEBUG_LEVEL) << method_name << " UV record: "
           << offset++ << "\t" << log_array[idx] << "\n";
   }
   log_array.clear();
}

////////////////////////////////////////////////////////////////////////

void log_merged_tqz_uv(map<float, float*> pqtzuv_map_tq,
                       map<float, float*> pqtzuv_map_uv,
                       map<float, float*> &pqtzuv_map_merged,
                       const char *method_name) {
   ConcatString buf;
   StringArray log_array;
   for (std::map<float,float*>::iterator it=pqtzuv_map_merged.begin();
         it!=pqtzuv_map_merged.end(); ++it) {
      float *pqtzuv = it->second;
      buf.clear();
      for (int idx=0; idx<mxr8vt; idx++) {
        buf << " " << pqtzuv[idx];
      }
      bool has_tq = (0 < (pqtzuv_map_tq.count(it->first)));
      bool has_uv = (0 < (pqtzuv_map_uv.count(it->first)));
      if (has_tq || has_uv) {
         buf << "\t(";
         if (has_tq) buf << " tqz";
         if (has_uv) buf << " uv";
         buf << " )";
      }
      log_array.add(buf.c_str());
   }
   int offset = 0;
   for (int idx=log_array.n_elements()-1; idx>=0; idx--) {
      mlog << Debug(PBL_DEBUG_LEVEL) << method_name << "    merged: "
           << offset++ << "\t" << log_array[idx] << "\n";
   }
   log_array.clear();
}

////////////////////////////////////////////////////////////////////////

void log_pbl_input(int pbl_level, const char *method_name) {
   ConcatString buf;
   StringArray log_array;
   int vlevels = pbl_level;
   if (vlevels >= MAX_PBL_LEVEL) vlevels = MAX_PBL_LEVEL;
   for (int idx=0; idx<vlevels; idx++) {
      buf.clear();
      buf << " " << pbl_data_pres[idx];
      buf << " " << pbl_data_temp[idx];
      buf << " " << pbl_data_spfh[idx];
      buf << " " <<  pbl_data_hgt[idx];
      buf << " " << pbl_data_ugrd[idx];
      buf << " " << pbl_data_vgrd[idx];
      log_array.add(buf.c_str());
   }
   int offset = 0;
   for (int idx=log_array.n_elements()-1; idx>=0; idx--) {
      mlog << Debug(PBL_DEBUG_LEVEL) << method_name << "    input to calpbl_: "
           << offset++ << "\t" << log_array[idx] << "\n";
   }
   log_array.clear();
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
        << "\t[-index]\n"
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

        << "\t\t\"-index\" indicates that the meta data (available variables and headers)"
        << " is extracted from \"prepbufr_file\" (optional). "
        << "No NetCDF outputs. \"-all\" or \"-vars\" is ignored.\n"

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
   valid_beg_ut = timestring_to_unix(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_valid_end_time(const StringArray & a)
{
   valid_end_ut = timestring_to_unix(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_nmsg(const StringArray & a)
{
   nmsg = atoi(a[0].c_str());
   int tmp_len = a[0].length();
   if (1 < tmp_len) {
      if (a[0][tmp_len-1] == '%') {
         nmsg_percent = nmsg;
      }
   }
}

////////////////////////////////////////////////////////////////////////

void set_dump_path(const StringArray & a)
{
   dump_flag = true;
   dump_dir = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_collect_metadata(const StringArray & a)
{
   collect_metadata = true;
}

////////////////////////////////////////////////////////////////////////

void set_target_variables(const StringArray & a)
{
   if ("_all_" == a[0]) {
      do_all_vars = true;
   }
   else {
     override_vars = true;
     ConcatString arg = a[0];
     bufr_target_variables = arg.split(",+ ");
   }
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a)
{
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a)
{
   mlog.set_verbosity_level(atoi(a[0].c_str()));
}

////////////////////////////////////////////////////////////////////////

void set_compress(const StringArray & a) {
   compress_level = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////
