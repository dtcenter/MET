// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   ioda2nc.cc
//
//   Description:
//      Based on user specified options, this tool filters point
//      observations from a IODA input file, derives requested
//      observation types, and writes the output to a NetCDF file.
//      IODA observations must be reformatted in this way prior to
//      using them in the Point-Stat tool.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    07-21-20  Howard Soh     New
//   001    07-06-22  Howard Soh     METplus-Internal #19 Rename main to met_main
//   002    09-29-22  Prestopnik     MET #2227 Remove namespace std and netCDF from header files
//
////////////////////////////////////////////////////////////////////////


#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <fstream>
#include <limits>
#include <assert.h>

#include <netcdf>

#include "main.h"
#include "apply_mask.h"
#include "ioda2nc_conf_info.h"
#include "vx_log.h"
#include "vx_nc_util.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "write_netcdf.h"

#include "vx_summary.h"
#include "nc_obs_util.h"
#include "nc_point_obs_out.h"
#include "nc_summary.h"

using namespace std;
using namespace netCDF;


////////////////////////////////////////////////////////////////////////

//
// Constants
//

static const char *DEF_CONFIG_NAME = "MET_BASE/config/IODA2NCConfig_default";

static const char *program_name = "ioda2nc";

static const int REJECT_DEBUG_LEVEL = 9;
static const int string_data_len = 512;

static const char *metadata_group_name = "MetaData";
static const char *qc_group_name = "QCFlags";
static const char *qc_postfix = "PreQC";
static const char *obs_group_name = "ObsValue";
static const char *derived_obs_group_name = "DerivedObsValue";

enum class e_ioda_format { v1, v2 };

////////////////////////////////////////////////////////////////////////

//
// Variables for command line arguments
//

// StringArray to store IODA file name
static StringArray ioda_files;

static StringArray core_dims;
static StringArray core_dims_v1;
static StringArray core_meta_vars;

// Output NetCDF file name
static ConcatString ncfile;

// Input configuration file
static ConcatString  config_file;
static IODA2NCConfInfo conf_info;

static Grid        mask_grid;
static MaskPlane   mask_area;
static MaskPoly    mask_poly;
static StringArray mask_sid;

// Beginning and ending retention times
static unixtime valid_beg_ut, valid_end_ut;

// Number of IODA messages to process from the command line
static int nmsg = -1;
static int nmsg_percent = -1;

static bool do_all_vars      = false;
static StringArray obs_var_names;
static StringArray obs_var_units;
static StringArray obs_var_descs;

static int compress_level = -1;

////////////////////////////////////////////////////////////////////////

static IntArray filtered_times;

static bool do_summary;
static bool save_summary_only = false;
static SummaryObs *summary_obs;
static MetNcPointObsOut nc_point_obs;


////////////////////////////////////////////////////////////////////////

static int n_total_obs;    // Running total of observations
static vector<Observation> observations;

//
// Output NetCDF file, dimensions, and variables
//
static NcFile *f_out = (NcFile *) nullptr;

////////////////////////////////////////////////////////////////////////

static void initialize();
static void process_command_line(int, char **);
static void open_netcdf();
static void process_ioda_file(int);
static void write_netcdf_hdr_data();
static void clean_up();

static void addObservation(const float *obs_arr, const ConcatString &hdr_typ,
      const ConcatString &hdr_sid, const time_t hdr_vld,
      const float hdr_lat, const float hdr_lon, const float hdr_elv,
      const float quality_mark, const int buf_size);

static bool keep_message_type(const char *);
static bool keep_station_id(const char *);
static bool keep_valid_time(const unixtime, const unixtime, const unixtime);

static void usage();
static void set_compress(const StringArray &);
static void set_config(const StringArray &);
static void set_ioda_files(const StringArray &);
static void set_mask_grid(const StringArray &);
static void set_mask_poly(const StringArray &);
static void set_mask_sid(const StringArray &);
static void set_logfile(const StringArray &);
static void set_nmsg(const StringArray &);
static void set_obs_var(const StringArray & a);
static void set_valid_beg_time(const StringArray &);
static void set_valid_end_time(const StringArray &);
static void set_verbosity(const StringArray &);

static bool check_core_data(const bool, const bool,
                            StringArray &, StringArray &, e_ioda_format);
static bool check_missing_thresh(float value);
static ConcatString find_meta_name(string meta_key, StringArray available_names);
static bool get_meta_data_float(NcFile *, StringArray &, const char *, float *,
                                const int);
static bool get_meta_data_strings(NcVar &, char *);
static bool get_meta_data_strings(NcVar &, char **);
static bool get_obs_data_float(NcFile *, const ConcatString, NcVar *,
                               float *, int *, const int, const e_ioda_format);
static bool has_postfix(const std::string &, std::string const &);
static bool is_in_metadata_map(string metadata_key, StringArray &available_list);

////////////////////////////////////////////////////////////////////////


int met_main(int argc, char *argv[]) {

   // Initialize static variables
   initialize();

   // Process the command line arguments
   process_command_line(argc, argv);

   // Open the NetCDF file
   open_netcdf();

   // Process each IODA file
   for(int i=0; i<ioda_files.n_elements(); i++) {
      //process_ioda_file_metadata(i);
      process_ioda_file(i);
   }

   if(do_summary) {
      TimeSummaryInfo summaryInfo = conf_info.getSummaryInfo();
      summary_obs->summarizeObs(summaryInfo);
      summary_obs->setSummaryInfo(summaryInfo);
   }

   // Write the NetCDF file
   write_netcdf_hdr_data();

   // Deallocate memory and clean up
   clean_up();

   return 0;
}

////////////////////////////////////////////////////////////////////////

const string get_tool_name() {
   return "ioda2nc";
}

////////////////////////////////////////////////////////////////////////

void initialize() {

   n_total_obs = 0;

   nc_point_obs.init_buffer();

   core_dims.clear();
   core_dims.add("nlocs");
   
   core_dims_v1.clear();
   core_dims_v1.add("nvars");
   core_dims_v1.add("nlocs");
   core_dims_v1.add("nstring");
   //core_dims_v1.add("ndatetime");

   core_meta_vars.clear();
   core_meta_vars.add("datetime");
   core_meta_vars.add("latitude");
   core_meta_vars.add("longitude");

   summary_obs = new SummaryObs();
   return;
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;
   static const char *method_name = "process_command_line() -> ";
   
   // Check for zero arguments
   if(argc == 1) usage();

   // Initialize retention times
   valid_beg_ut = valid_end_ut = (unixtime) 0;

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_ioda_files, "-iodafile", 1);
   cline.add(set_valid_beg_time, "-valid_beg", 1);
   cline.add(set_valid_end_time, "-valid_end", 1);
   cline.add(set_nmsg, "-nmsg", 1);
   cline.add(set_obs_var, "-obs_var", 1);
   cline.add(set_config,    "-config",    1);
   cline.add(set_mask_grid, "-mask_grid", 1);
   cline.add(set_mask_poly, "-mask_poly", 1);
   cline.add(set_mask_sid,  "-mask_sid",  1);
   cline.add(set_logfile, "-log", 1);
   cline.add(set_verbosity, "-v", 1);
   cline.add(set_compress,  "-compress",  1);

   // Parse the command line
   cline.parse();

   // Check for error. There should be three arguments left:
   // IODA, output NetCDF, and config filenames
   if(cline.n() < 2) usage();

   // Store the input file names
   ioda_files.add(cline[0]);
   if(cline.n() > 1) ncfile = cline[1];

   // Create the default config file name
   ConcatString default_config_file = replace_path(DEF_CONFIG_NAME);

   // List the config files and read the config files
   if(0 < config_file.length()) {
      if( !file_exists(config_file.c_str()) ) {
         mlog << Error << "\n" << method_name
              << "file does not exist \"" << config_file << "\"\n\n";
         exit(1);
      }
      mlog << Debug(1)
           << "Default Config File: " << default_config_file << "\n"
           << "User Config File: "    << config_file << "\n";
   }
   else mlog << Debug(1)
             << "Default Config File: " << default_config_file << "\n";
   conf_info.read_config(default_config_file.c_str(), config_file.c_str());

   // Process the configuration
   conf_info.process_config();

   // Check that valid_end_ut >= valid_beg_ut
   if(valid_beg_ut != (unixtime) 0 && valid_end_ut != (unixtime) 0
      && valid_beg_ut > valid_end_ut) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the ending time (" << unix_to_yyyymmdd_hhmmss(valid_end_ut)
           << ") must be greater than the beginning time ("
           << unix_to_yyyymmdd_hhmmss(valid_beg_ut) << ").\n\n";
      exit(1);
   }

   do_summary = conf_info.getSummaryInfo().flag;
   if(do_summary) save_summary_only = !conf_info.getSummaryInfo().raw_data;
   else save_summary_only = false;

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
      f_out = (NcFile *) nullptr;

      exit(1);
   }

   // Define netCDF variables
   int deflate_level = compress_level;
   if(deflate_level < 0) deflate_level = conf_info.conf.nc_compression();
   nc_point_obs.set_netcdf(f_out, true);
   nc_point_obs.init_obs_vars(true, deflate_level);

   // Add global attributes
   write_netcdf_global(f_out, ncfile.text(), program_name);

   return;
}

////////////////////////////////////////////////////////////////////////

void process_ioda_file(int i_pb) {
   int npbmsg, npbmsg_total;
   int idx, i_msg, i_read, n_file_obs, n_hdr_obs;
   int rej_typ, rej_sid, rej_vld, rej_grid, rej_poly;
   int rej_elv, rej_nobs;
   double   x, y;

   bool status;
   bool is_time_offset = false;
   bool is_time_string = false;
   unixtime file_ut;
   unixtime adjusted_file_ut;
   unixtime msg_ut, beg_ut, end_ut;
   unixtime min_msg_ut, max_msg_ut;

   ConcatString file_name, blk_prefix, blk_file, log_message;
   ConcatString prefix;
   ConcatString  start_time_str, end_time_str;
   char     min_time_str[max_str_len], max_time_str[max_str_len];

   char     hdr_typ[max_str_len];
   ConcatString hdr_sid;
   char     modified_hdr_typ[max_str_len];
   double   hdr_lat, hdr_lon, hdr_elv;
   unixtime hdr_vld_ut;
   float    obs_arr[OBS_ARRAY_LEN];

   const int debug_level_for_performance = 3;
   int start_t, end_t, method_start, method_end;
   start_t = end_t = method_start = method_end = clock();

   IntArray diff_file_times;
   int diff_file_time_count;
   StringArray variables_big_nlevels;
   static const char *method_name = "process_ioda_file() -> ";
   static const char *method_name_s = "process_ioda_file() ";

   bool apply_grid_mask = (conf_info.grid_mask.nx() > 0 &&
                           conf_info.grid_mask.ny() > 0);
   bool apply_area_mask = (conf_info.area_mask.nx() > 0 &&
                           conf_info.area_mask.ny() > 0);
   bool apply_poly_mask = (conf_info.poly_mask.n_points() > 0);

   // List the IODA file being processed
   mlog << Debug(1) << "Processing IODA File:\t" << ioda_files[i_pb]<< "\n";

   NcFile *f_in = open_ncfile(ioda_files[i_pb].c_str());

   // Check for a valid file
   if(IS_INVALID_NC_P(f_in)) {
      mlog << Error << "\n" << method_name
           << "can't open input NetCDF file \"" << ioda_files[i_pb]
           << "\" for reading.\n\n";
      delete f_in;
      f_in = (NcFile *) nullptr;

      exit(1);
   }

   // Initialize
   hdr_typ[0] = 0;
   file_ut = beg_ut = end_ut = hdr_vld_ut = (unixtime) 0;
   filtered_times.clear();
   min_msg_ut = max_msg_ut = (unixtime) 0;
   min_time_str[0] = 0;
   max_time_str[0] = 0;
   modified_hdr_typ[0] = 0;

   // Set the file name for the IODA file
   file_name << ioda_files[i_pb];

   int nrecs = 0;
   int nlocs = 0;
   int nstring = 0;
   int nvars = 0;
   StringArray dim_names;
   StringArray metadata_vars;
   StringArray obs_value_vars;
   bool error_out = true;
   e_ioda_format ioda_format = e_ioda_format::v2;

   get_dim_names(f_in, &dim_names);
   ConcatString nlocs_name = find_meta_name("nlocs", dim_names);
   if(0 < nlocs_name.length()) nlocs = get_dim_value(f_in, nlocs_name.c_str(), error_out); // number of locations

   nvars = bad_data_int ;
   nstring = string_data_len;
   if (! has_nc_group(f_in, obs_group_name)) ioda_format = e_ioda_format::v1;

   if ( ioda_format == e_ioda_format::v1 ) {
      StringArray var_names;
      get_var_names(f_in, &var_names);
      for(idx=0; idx<var_names.n(); idx++) {
         if(has_postfix(var_names[idx], "@MetaData")) {
            metadata_vars.add(var_names[idx].substr(0, var_names[idx].find('@')));
         }
         if(has_postfix(var_names[idx], "@ObsValue") || has_postfix(var_names[idx], "@DerivedObsValue")) {
            obs_value_vars.add(var_names[idx].substr(0, var_names[idx].find('@')));
         }
      }
      if(mlog.verbosity_level() >= 8) {
         for(idx=0; idx<var_names.n(); idx++)
            mlog << Debug(8) << method_name << "var_name: " << var_names[idx] << "\n";
      }

      ConcatString nvars_ = find_meta_name("nvars", dim_names);
      if(has_dim(f_in, nvars_.c_str())) {
         nvars = get_dim_value(f_in, nvars_.c_str(), error_out); // number of variables
         ConcatString nstring_ = find_meta_name("nstring", dim_names);
         if(dim_names.has(nstring_)) nstring = get_dim_value(f_in, nstring_.c_str(), error_out);

         ConcatString nrecs_ = find_meta_name("nrecs", dim_names);
         if(dim_names.has(nrecs_)) nrecs = get_dim_value(f_in, nrecs_.c_str(), false);
         else {
            nrecs = nvars * nlocs;
            mlog << Debug(3) << "\n" << method_name
                 << "nrecs dimension does not exist, so computed\n";
         }
      }
   }
   else {
      StringArray group_names;
      group_names.add(metadata_group_name);
      get_var_names(f_in, &metadata_vars, group_names);
      group_names.clear();
      group_names.add("ObsValue");
      group_names.add("DerivedObsValue");
      get_var_names(f_in, &obs_value_vars, group_names);
   }
   if(mlog.verbosity_level() >= 6) {
      for(idx=0; idx<dim_names.n(); idx++)
         mlog << Debug(8) << method_name << "dim_name: " << dim_names[idx] << "\n";
      for(idx=0; idx<metadata_vars.n(); idx++)
         mlog << Debug(8) << method_name << "metadata: " << metadata_vars[idx] << "\n";
      for(idx=0; idx<obs_value_vars.n(); idx++)
         mlog << Debug(8) << method_name << "ObsValue or Derived: " << obs_value_vars[idx] << "\n";
   }

   ConcatString msg_type_name = find_meta_name(conf_key_message_type, metadata_vars);
   ConcatString station_id_name = find_meta_name(conf_key_station_id, metadata_vars);
   ConcatString datetime_name = find_meta_name(conf_key_datetime, metadata_vars);
   ConcatString lon_name = find_meta_name("longitude", metadata_vars);
   ConcatString lat_name = find_meta_name("latitude", metadata_vars);

   bool has_msg_type = 0 < msg_type_name.length();
   bool has_station_id = 0 < station_id_name.length();
   bool is_netcdf_ready = check_core_data(has_msg_type, has_station_id,
                                          dim_names, metadata_vars, ioda_format);

   if(!is_netcdf_ready) {
      mlog << Error << "\n" << method_name
           << "Please check the IODA file (required dimensions or meta variables are missing).\n\n";
      delete f_in;
      f_in = (NcFile *) nullptr;
      exit(1);
   }
   
   // Compute the number of IODA records in the current file.

   unixtime base_ut;
   int sec_per_unit;
   int ndatetime = 1;
   bool no_leap_year = false;
   NcVar in_hdr_lat_var = get_var(f_in, lat_name.c_str(), metadata_group_name);
   NcVar in_hdr_lon_var = get_var(f_in, lon_name.c_str(), metadata_group_name);
   NcVar in_hdr_vld_var = get_var(f_in, datetime_name.c_str(), metadata_group_name);

   base_ut = sec_per_unit = 0;
   if (IS_INVALID_NC(in_hdr_vld_var)) {
      mlog << Error << "\n" << method_name << "Fail to get datetime variable\n\n";
      exit(-1);
   }
   else {

      if (NC_STRING == GET_NC_TYPE_ID(in_hdr_vld_var)) {
         is_time_string = true;
      }
      else if (NC_CHAR == GET_NC_TYPE_ID(in_hdr_vld_var)) {
         if(dim_names.has("ndatetime")) ndatetime = get_dim_value(f_in, "ndatetime", error_out);
         else {
            NcDim datetime_dim = get_nc_dim(&in_hdr_vld_var, 1);
            ndatetime = IS_VALID_NC(datetime_dim) ? get_dim_size(&datetime_dim) : nstring;
            mlog << Debug(3) << "\n" << method_name
                 << "ndatetime dimension does not exist!\n";
         }
         mlog << Debug(5) << method_name << "dimensions: nvars=" << nvars << ", nlocs=" << nlocs
              << ", nrecs=" << nrecs << ", nstring=" << nstring << ", ndatetime=" << ndatetime << "\n";
      }
      else {
         ConcatString units;
         is_time_offset = true;
         no_leap_year = get_att_no_leap_year(&in_hdr_vld_var);
         if (get_var_units(&in_hdr_vld_var, units) && (0 < units.length())) {
            parse_cf_time_string(units.c_str(), base_ut, sec_per_unit);
         }
         else {
            mlog << Error << "\n" << method_name << "Fail to get time units from "
                 << GET_NC_NAME(in_hdr_vld_var) << "\n\n";
            exit(-1);
         }
      }
   }

   npbmsg_total = npbmsg = nlocs;

   // Use the number of records requested by the user if there
   // are enough present.
   if(nmsg > 0 && nmsg < npbmsg) {
      npbmsg = (nmsg_percent > 0 && nmsg_percent <= 100)
               ? (npbmsg * nmsg_percent / 100) : nmsg;
   }

   vector<float> hdr_lat_arr  (nlocs);
   vector<float> hdr_lon_arr  (nlocs);
   vector<float> hdr_elv_arr  (nlocs);
   vector<float> obs_pres_arr (nlocs);
   vector<float> obs_hght_arr (nlocs);
   vector<float> hdr_time_arr (nlocs);
   char *hdr_vld_block = new char[nlocs*ndatetime];
   char *hdr_msg_types = nullptr;
   char *hdr_station_ids = nullptr;
   char **hdr_vld_block2 = nullptr;
   char **hdr_msg_types2 = nullptr;
   char **hdr_station_ids2 = nullptr;
   vector<int *> v_qc_data;
   vector<float *> v_obs_data;

   if (is_time_string) {
      hdr_vld_block2 = (char**) calloc(nlocs, sizeof(char*));
      for (int i=0; i<nlocs; i++ ) hdr_vld_block2[i] = (char*)calloc(nstring, sizeof(char));
   }
   
   get_meta_data_float(f_in, metadata_vars, "pressure", obs_pres_arr.data(), nlocs);
   get_meta_data_float(f_in, metadata_vars, "height",   obs_hght_arr.data(), nlocs);
   get_meta_data_float(f_in, metadata_vars, "elevation", hdr_elv_arr.data(), nlocs);

   if(has_msg_type) {
      NcVar msg_type_var = get_var(f_in, msg_type_name.c_str(), metadata_group_name);
      if (NC_STRING == GET_NC_TYPE_ID(msg_type_var)) {
         hdr_msg_types2 = (char**) calloc(nlocs, sizeof(char*));
         for (int i=0; i<nlocs; i++ ) hdr_msg_types2[i] = (char*)calloc(nstring, sizeof(char));
         get_nc_data(&msg_type_var, hdr_msg_types2);
      }
      else {
         hdr_msg_types = new char[nlocs*nstring];
         get_meta_data_strings(msg_type_var, hdr_msg_types);
      }
   }
   else {
      mlog << Debug(1) << method_name
           << "The metadata variable for message type does not exist!\n";
   }

   if(has_station_id) {
      NcVar sid_var = get_var(f_in, station_id_name.c_str(), metadata_group_name);
      if (NC_STRING == GET_NC_TYPE_ID(sid_var)) {
         hdr_station_ids2 = (char**) calloc(nlocs, sizeof(char*));
         for (int i=0; i<nlocs; i++ ) hdr_station_ids2[i] = (char*)calloc(nstring, sizeof(char));
         get_nc_data(&sid_var, hdr_station_ids2);
      }
      else {
         hdr_station_ids = new char[nlocs*nstring];
         get_meta_data_strings(sid_var, hdr_station_ids);
      }
   }
   else {
      mlog << Debug(1) << method_name
           << "The metadata variable for station ID does not exist!\n";
   }

   if(!get_nc_data(&in_hdr_lat_var, hdr_lat_arr.data(), nlocs)) {
      mlog << Error << "\n" << method_name
           << "trouble getting latitude\n\n";
      exit(1);
   }
   if(!get_nc_data(&in_hdr_lon_var, hdr_lon_arr.data(), nlocs)) {
      mlog << Error << "\n" << method_name
           << "trouble getting longitude\n\n";
      exit(1);
   }

   status = is_time_offset ? get_nc_data(&in_hdr_vld_var, hdr_time_arr.data())
                           : is_time_string
                             ? get_nc_data(&in_hdr_vld_var, hdr_vld_block2)
                             : get_nc_data(&in_hdr_vld_var, hdr_vld_block);
   if(!status) {
      mlog << Error << "\n" << method_name
           << "trouble getting datetime\n\n";
      exit(1);
   }

   StringArray raw_var_names;
   if(do_all_vars || obs_var_names.n() == 0) raw_var_names = obs_value_vars;
   else raw_var_names = obs_var_names;

   NcVar obs_var, qc_var;
   ConcatString unit_attr;
   ConcatString desc_attr;
   for(idx=0; idx<raw_var_names.n(); idx++ ) {
      int *qc_data = new int[nlocs];
      float *obs_data = new float[nlocs];

      for (int idx2=0; idx2<nlocs; idx2++) {
         qc_data[idx2] = bad_data_int;
         obs_data[idx2] = bad_data_float;
      }
      mlog << Debug(7) << method_name
           << "processing \"" << raw_var_names[idx] << "\" variable!\n";
      obs_var = get_var(f_in, raw_var_names[idx].c_str(), obs_group_name);
      if (IS_INVALID_NC(obs_var)) obs_var = get_var(f_in, raw_var_names[idx].c_str(), derived_obs_group_name);
      v_qc_data.push_back(qc_data);
      v_obs_data.push_back(obs_data);
      unit_attr.clear();
      desc_attr.clear();
      if(IS_VALID_NC(obs_var)) {
         get_obs_data_float(f_in, raw_var_names[idx], &obs_var, obs_data, qc_data, nlocs, ioda_format);
         get_var_units(&obs_var, unit_attr);
         get_att_value_string(&obs_var, "long_name", desc_attr);
      }

      // Replace the input variable name to the output variable name
      ConcatString raw_name = raw_var_names[idx];
      // Filter out the same variable names from multiple input files
      if (!obs_var_names.has(raw_name)) {
         obs_var_names.add(raw_name);
         obs_var_units.add(unit_attr);
         obs_var_descs.add(desc_attr);
      }
   }

   // Initialize counts
   n_file_obs = i_msg = 0;
   rej_typ = rej_sid  = rej_vld    = rej_grid = rej_poly = 0;
   rej_elv = rej_nobs = 0;

   bool showed_progress = false;
   if(mlog.verbosity_level() >= debug_level_for_performance) {
      end_t = clock();
      mlog << Debug(debug_level_for_performance) << " PERF: " << method_name_s << " "
           << (end_t-start_t)/double(CLOCKS_PER_SEC)
           << " seconds for preparing\n";
      start_t = clock();
   }

   log_message = " IODA messages";
   if(npbmsg != npbmsg_total) {
      log_message << " (out of " << npbmsg_total << ")";
   }
   mlog << Debug(2) << "Processing " << npbmsg << log_message << "...\n";

   int      bin_count = nint(npbmsg/20.0);
   unixtime prev_hdr_vld_ut = (unixtime) 0;
   map<ConcatString, ConcatString> message_type_map = conf_info.getMessageTypeMap();

   // Initialize
   diff_file_time_count = 0;

   for(idx=0; idx<OBS_ARRAY_LEN; idx++) obs_arr[idx] = 0;

   // Loop through the IODA messages from the input file
   bool do_set_time_config = true;
   for(i_read=0; i_read<npbmsg; i_read++) {

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

      if (is_time_offset) {
         msg_ut = add_to_unixtime(base_ut, sec_per_unit,
                                  hdr_time_arr[i_read], no_leap_year);
      }
      else if (is_time_string) {
         char valid_time[nstring+1];

         m_strncpy(valid_time, (const char *)hdr_vld_block2[i_read],
                   nstring, method_name_s, "valid_time", true);
         valid_time[nstring] = 0;
         msg_ut = yyyymmddThhmmss_to_unix(valid_time);
      }
      else {
         char valid_time[ndatetime+1];
         m_strncpy(valid_time, (const char *)(hdr_vld_block + (i_read * ndatetime)),
                   ndatetime, method_name_s, "valid_time", true);
         valid_time[ndatetime] = 0;
         msg_ut = yyyymmddThhmmss_to_unix(valid_time);
      }

      // Check to make sure that the message time hasn't changed
      // from one IODA message to the next
      if(file_ut == (unixtime) 0) {
         adjusted_file_ut = file_ut = msg_ut;

         mlog << Debug(2)
              << " IODA Time Center:\t\t" << unix_to_yyyymmdd_hhmmss(adjusted_file_ut)
              << "\n";

         // Check if valid_beg_ut and valid_end_ut were set on the
         // command line.  If so, use them.  If not, use beg_ds and
         // end_ds.
         if(valid_beg_ut != (unixtime) 0 || valid_end_ut != (unixtime) 0) {
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
         if(!diff_file_times.has(msg_ut)) diff_file_times.add(msg_ut);
      }

      if(has_msg_type) {
         if (nullptr != hdr_msg_types2) {
            m_strncpy(hdr_typ, hdr_msg_types2[i_read], nstring, method_name_s, "hdr_typ2");
         }
         else {
            m_strncpy(hdr_typ, hdr_msg_types+(i_read*nstring), nstring, method_name_s, "hdr_typ");

         }
         m_rstrip(hdr_typ, nstring);

         // If the message type is not listed in the configuration
         // file and it is not the case that all message types should be
         // retained, continue to the next IODA message
         if(!keep_message_type(hdr_typ)) {
            rej_typ++;
            continue;
         }
      }
      else m_strncpy(hdr_typ, "NA", HEADER_STR_LEN,
                     method_name_s, "missing_hdr_typ");
      if(0 < message_type_map.count((string)hdr_typ)) {
         int buf_len = sizeof(modified_hdr_typ);
         ConcatString mappedMessageType = message_type_map[(string)hdr_typ];
         mlog << Debug(6) << "\n" << method_name
              << "Switching report type \"" << hdr_typ
              << "\" to message type \"" << mappedMessageType << "\".\n";
         if(mappedMessageType.length() < HEADER_STR_LEN) buf_len = HEADER_STR_LEN;
         m_strncpy(modified_hdr_typ, mappedMessageType.c_str(), buf_len,
                   method_name_s, "modified_hdr_typ");
      }

      if(has_station_id) {
         char tmp_sid[nstring+1];
         if (nullptr != hdr_station_ids2) {
            m_strncpy(tmp_sid, hdr_station_ids2[i_read], nstring, method_name_s, "tmp_sid2");
         }
         else {
            m_strncpy(tmp_sid, hdr_station_ids+(i_read*nstring), nstring, method_name_s, "tmp_sid");
         }
         m_rstrip(tmp_sid, nstring, false);
         m_replace_char(tmp_sid, ' ', '_');
         hdr_sid = tmp_sid;
      }
      else hdr_sid.clear();

      // If the station id is not listed in the configuration
      // file and it is not the case that all station ids should be
      // retained, continue to the next IODA message
      if(!keep_station_id(hdr_sid.c_str())) {
         rej_sid++;
         continue;
      }

      // Read the header array elements which consists of:
      //    LON LAT DHR ELV TYP T29 ITP

      // Longitude
      hdr_lon = hdr_lon_arr[i_read];

      // Latitude
      hdr_lat = hdr_lat_arr[i_read];

      // Elevation
      hdr_elv = hdr_elv_arr[i_read];

      // Compute the valid time and check if it is within the
      // specified valid range
      //hdr_vld_ut = msg_ut + (unixtime)nint(hdr[3]*sec_per_hour);
      hdr_vld_ut = msg_ut;
      if(0 == min_msg_ut || min_msg_ut > hdr_vld_ut) min_msg_ut = hdr_vld_ut;
      if(max_msg_ut < hdr_vld_ut) max_msg_ut = hdr_vld_ut;
      if(!keep_valid_time(hdr_vld_ut, beg_ut, end_ut)) {
         if(!filtered_times.has(hdr_vld_ut, false)) {
            filtered_times.add(hdr_vld_ut);
         }
         rej_vld++;
         continue;
      }

      // Rescale the longitude value from 0 to 360 -> -180 to 180
      hdr_lon = rescale_lon(hdr_lon);

      // If the lat/lon for the IODA message is not on the
      // grid_mask, continue to the next IODA message
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

      // If the lat/lon for the IODA message is not inside the mask
      // polyline continue to the next IODA message.  Multiply by
      // -1 to convert from degrees_east to degrees_west
      if(apply_poly_mask &&
            !conf_info.poly_mask.latlon_is_inside_dege(hdr_lat, hdr_lon)) {
         rej_poly++;
         continue;
      }

      // Check if the message elevation is within the specified range.
      // Missing data values for elevation are retained.
      if(!check_missing_thresh(hdr_elv) &&
         (hdr_elv < conf_info.beg_elev || hdr_elv > conf_info.end_elev) ) {
         rej_elv++;
         continue;
      }

      // Store the index to the header data
      obs_arr[0] = (float)nc_point_obs.get_hdr_index();

      n_hdr_obs = 0;
      for(idx=0; idx<v_obs_data.size(); idx++ ) {
         int var_idx = 0;
         if (!obs_var_names.has(raw_var_names[idx], var_idx)) {
            mlog << Warning << "\n" << method_name
                 << "Skip the variable " << raw_var_names[idx]
                 << " at " << ioda_files[i_pb] << "\n\n";
            continue;
         }
         obs_arr[1] = var_idx;
         obs_arr[2] = obs_pres_arr[i_read];
         obs_arr[3] = obs_hght_arr[i_read];
         obs_arr[4] = v_obs_data[idx][i_read];
         addObservation(obs_arr, (string)hdr_typ, (string)hdr_sid, hdr_vld_ut,
               hdr_lat, hdr_lon, hdr_elv, (float)v_qc_data[idx][i_read],
               OBS_BUFFER_SIZE);

         // Increment the current and total observations counts
         n_file_obs++;
         n_total_obs++;

         // Increment the number of obs counter for this header
         n_hdr_obs++;
      } // end for idx

      // If the number of observations for this header is non-zero,
      // store the header data and increment the IODA record
      // counter
      if(n_hdr_obs > 0) {
         i_msg++;
      }
      else {
         rej_nobs++;
      }
   } // end for i_read

   if(showed_progress) {
      log_message = "100% ";
      if(mlog.verbosity_level() >= debug_level_for_performance) {
         end_t = clock();
         log_message << (end_t-start_t)/double(CLOCKS_PER_SEC) << " seconds";
      }
      cout << log_message << "\n";
   }

   nc_point_obs.write_observation();

   if(mlog.verbosity_level() > 0) cout << "\n" << flush;

   mlog << Debug(2)
        << "Total records processed\t\t= " << npbmsg << "\n"
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
        << "Rejected based on zero observations\t= "
        << rej_nobs << "\n"
        << "Total Records retained\t\t\t= "
        << i_msg << "\n"
        << "Total observations retained or derived\t= "
        << n_file_obs << "\n";

   if(npbmsg == rej_vld && 0 < rej_vld) {
      mlog << Warning << "\n" << method_name
           << "All records were filtered out by valid time.\n"
           << "\tPlease adjust time range with \"-valid_beg\" and \"-valid_end\".\n"
           << "\tmin/max obs time from IODA file: " << min_time_str
           << " and " << max_time_str << ".\n"
           << "\ttime range: " << start_time_str << " and " << end_time_str << ".\n\n";
   }
   else {
      mlog << Debug(1) << "Obs time between " << unix_to_yyyymmdd_hhmmss(min_msg_ut)
           << " and " << unix_to_yyyymmdd_hhmmss(max_msg_ut) << "\n";

      int debug_level = 5;
      if(mlog.verbosity_level() >= debug_level) {
         log_message = "Filtered time:";
         for(int kk=0; kk<filtered_times.n_elements();kk++) {
            log_message.add((0 == (kk % 3)) ? "\n\t" : "  ");
            log_message.add(unix_to_yyyymmdd_hhmmss(filtered_times[kk]));
         }
         mlog << Debug(debug_level) << log_message << "\n";
      }
   }
   
   delete [] hdr_vld_block;

   if (hdr_msg_types) delete [] hdr_msg_types;
   if (hdr_station_ids) delete [] hdr_station_ids;
   if (nullptr != hdr_msg_types2) {
      for (int i=0; i<nlocs; i++ ) delete hdr_msg_types2[i];
      delete [] hdr_msg_types2;
   }
   if (nullptr != hdr_station_ids2) {
      for (int i=0; i<nlocs; i++ ) delete hdr_station_ids2[i];
      delete [] hdr_station_ids2;
   }
   if (nullptr != hdr_vld_block2) {
      for (int i=0; i<nlocs; i++ ) delete hdr_vld_block2[i];
      delete [] hdr_vld_block2;
   }

   for(idx=0; idx<v_obs_data.size(); idx++ ) delete [] v_obs_data[idx];
   for(idx=0; idx<v_qc_data.size(); idx++ ) delete [] v_qc_data[idx];
   v_obs_data.clear();
   v_qc_data.clear();

   // Close the input NetCDF file
   if(f_in) {
      delete f_in;
      f_in = (NcFile *) nullptr;
   }

   if(mlog.verbosity_level() >= debug_level_for_performance) {
      method_end = clock();
      cout << " PERF: " << method_name_s
           << (method_end-method_start)/double(CLOCKS_PER_SEC)
           << " seconds\n";
   }

   if(i_msg <= 0) {
      mlog << Warning << "\n" << method_name
           << "No IODA records retained from file: "
           << ioda_files[i_pb] << "\n\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_netcdf_hdr_data() {
   int obs_cnt, hdr_cnt;
   static const string method_name = "\nwrite_netcdf_hdr_data()";

   nc_point_obs.get_hdr_index();
   nc_point_obs.set_nc_out_data(observations, summary_obs, conf_info.getSummaryInfo());
   nc_point_obs.get_dim_counts(&obs_cnt, &hdr_cnt);
   nc_point_obs.init_netcdf(obs_cnt, hdr_cnt, program_name);

   // Check for no messages retained
   if(hdr_cnt <= 0) {
      mlog << Error << method_name << " -> "
           << "No IODA records retained.  Nothing to write.\n\n";
      // Delete the NetCDF file
      remove_temp_file(ncfile);
      exit(1);
   }

   // Make sure all obs data is processed before handling header
   StringArray nc_var_name_arr;
   StringArray nc_var_unit_arr;
   StringArray nc_var_desc_arr;
   const long var_count = obs_var_names.n();
   const long units_count = obs_var_units.n();
   map<ConcatString,ConcatString> name_map = conf_info.getObsVarMap();

   for(int i=0; i<var_count; i++) {
      ConcatString new_name = name_map[obs_var_names[i]];
      if (0 >= new_name.length()) new_name = obs_var_names[i];
      nc_var_name_arr.add(new_name);
   }
   for(int i=0; i<units_count; i++) {
      nc_var_unit_arr.add(obs_var_units[i]);
   }
   for(int i=0; i<obs_var_descs.n(); i++) {
      nc_var_desc_arr.add(obs_var_descs[i]);
   }

   nc_point_obs.write_to_netcdf(nc_var_name_arr, nc_var_unit_arr, nc_var_desc_arr);

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
   if(check_missing_thresh(quality_mark))
      obs_qty.add("NA");
   else obs_qty.format("%d", nint(quality_mark));

   int var_index = obs_arr[1];
   map<ConcatString,ConcatString> name_map = conf_info.getObsVarMap();
   string var_name = obs_var_names[var_index];
   string out_name = name_map[var_name];
   Observation obs = Observation(hdr_typ.text(),
                                 hdr_sid.text(),
                                 hdr_vld,
                                 hdr_lat, hdr_lon, hdr_elv,
                                 obs_qty.text(),
                                 var_index,
                                 obs_arr[2], obs_arr[3], obs_arr[4],
                                 (0<out_name.length() ? out_name : var_name));
   obs.setHeaderIndex(obs_arr[0]);
   observations.push_back(obs);
   if(do_summary) summary_obs->addObservationObj(obs);
   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {

   nc_point_obs.close();

   if(f_out) {
      delete f_out;
      f_out = (NcFile *) nullptr;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool keep_message_type(const char *mt_str) {
   bool keep = conf_info.message_type.n_elements() == 0 ||
               conf_info.message_type.has(mt_str, false);

   if(!keep && mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
      mlog << Debug(REJECT_DEBUG_LEVEL) << "The message type [" << mt_str << "] is rejected\n";
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////

bool keep_station_id(const char *sid_str) {

   bool keep = (conf_info.station_id.n_elements() == 0 ||
                conf_info.station_id.has(sid_str, false));

   if(!keep && mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
      mlog << Debug(REJECT_DEBUG_LEVEL) << "The station ID [" << sid_str << "] is rejected\n";
   }

   return keep;
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

   if(!keep && mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
      mlog << Debug(REJECT_DEBUG_LEVEL) << "The valid_time [" << ut << ", "
           << unix_to_yyyymmdd_hhmmss(ut) << "] is rejected\n";
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////

bool check_core_data(const bool has_msg_type, const bool has_station_id,
                     StringArray &dim_names, StringArray &metadata_vars,
                     e_ioda_format ioda_format) {
   bool is_netcdf_ready = true;
   static const char *method_name = "check_core_data() -> ";

   StringArray &t_core_dims = (ioda_format == e_ioda_format::v2)
                              ? core_dims : core_dims_v1;
   for(int idx=0; idx<t_core_dims.n(); idx++) {
      if (!is_in_metadata_map(t_core_dims[idx], dim_names)) {
         mlog << Error << "\n" << method_name << "-> "
              << "core dimension \"" << t_core_dims[idx] << "\" is missing.\n\n";
         is_netcdf_ready = false;
      }
   }

   if (ioda_format == e_ioda_format::v1) {
      if(has_msg_type || has_station_id) {
         if (!is_in_metadata_map("nstring", dim_names)) {
            mlog << Error << "\n" << method_name << "-> "
                 << "core dimension \"nstring\" is missing.\n\n";
            is_netcdf_ready = false;
         }
      }
   }

   for(int idx=0; idx<core_meta_vars.n(); idx++) {
      if(!is_in_metadata_map(core_meta_vars[idx], metadata_vars)) {
         mlog << Error << "\n" << method_name << "-> "
              << "core variable  \"" << core_meta_vars[idx] << "\" is missing.\n\n";
         is_netcdf_ready = false;
      }
   }
   return is_netcdf_ready;
}

////////////////////////////////////////////////////////////////////////

bool check_missing_thresh(float value) {
   bool check = false;
   for(int idx=0; idx<conf_info.missing_thresh.n(); idx++) {
      if(conf_info.missing_thresh[idx].check(value)) {
          check = true;
          break;
      }
   }
   return check;
}

////////////////////////////////////////////////////////////////////////

ConcatString find_meta_name(string meta_key, StringArray available_names) {
   ConcatString metadata_name;
   if (available_names.has(meta_key)) metadata_name = meta_key;
   else {
      StringArray alt_names = conf_info.metadata_map[meta_key];
      for (int idx =0; idx<alt_names.n(); idx++) {
         if (available_names.has(alt_names[idx])) {
            metadata_name = alt_names[idx];
            break;
         }
      }
   }
   return metadata_name;
}

////////////////////////////////////////////////////////////////////////

bool get_meta_data_float(NcFile *f_in, StringArray &metadata_vars,
                         const char *metadata_key, float *metadata_buf,
                         const int nlocs) {
   bool status = false;
   static const char *method_name = "get_meta_data_float() -> ";
   
   ConcatString metadata_name = find_meta_name(metadata_key, metadata_vars);

   if(metadata_name.length() > 0) {
      NcVar meta_var = get_var(f_in, metadata_name.c_str(), metadata_group_name);
      if(IS_VALID_NC(meta_var)) {
         status = get_nc_data(&meta_var, metadata_buf, nlocs);
         if(!status) mlog << Debug(3) << method_name
                           << "trouble getting " << metadata_name << "\n";
      }
   }
   else mlog << Debug(4) << method_name
             << "Metadata for " << metadata_key << " does not exist!\n";
   if(status) {
      for(int idx=0; idx<nlocs; idx++)
         if(check_missing_thresh(metadata_buf[idx])) metadata_buf[idx] = bad_data_float;
   }
   else {
      for(int idx=0; idx<nlocs; idx++)
         metadata_buf[idx] = bad_data_float;
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

bool get_meta_data_strings(NcVar &var, char *metadata_buf) {
   bool status = false;
   static const char *method_name = "get_meta_data_strings() -> ";

   if(IS_VALID_NC(var)) {
      status = get_nc_data(&var, metadata_buf);
      if(!status) {
         mlog << Error << "\n" << method_name << " -> "
              << "trouble getting " << GET_NC_NAME(var) << "\n\n";
         exit(1);
      }
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

bool get_obs_data_float(NcFile *f_in, const ConcatString var_name,
                        NcVar *obs_var, float *obs_buf, int *qc_buf,
                        const int nlocs, const e_ioda_format ioda_format) {
   bool status = false;
   static const char *method_name = "get_obs_data_float() -> ";
   
   if(IS_VALID_NC_P(obs_var)) {
      status = get_nc_data(obs_var, obs_buf, nlocs);
      if(status) {
         for(int idx=0; idx<nlocs; idx++)
            if(check_missing_thresh(obs_buf[idx])) obs_buf[idx] = bad_data_float;
      }
      else mlog << Error << "\n" << method_name
                << "trouble getting " << var_name << "\n\n";
   }
   else mlog << Error << "\n" << method_name
             << var_name << " does not exist!\n\n";
   if(!status) exit(1);
   
   status = false;
   if(var_name.length() > 0) {
      ConcatString qc_name = var_name;
      ConcatString qc_group = qc_postfix;
      if (ioda_format == e_ioda_format::v2) {
         NcGroup nc_grp = get_nc_group(f_in, qc_postfix);
         if (IS_INVALID_NC(nc_grp)) qc_group = qc_group_name;
         StringArray qc_names = conf_info.obs_to_qc_map[var_name];
         if (0 < qc_names.n()) {
            for (int idx=0; idx<qc_names.n(); idx++) {
               if (has_var(f_in, qc_names[idx].c_str(), qc_group_name)) {
                  qc_name = qc_names[idx];
                  mlog << Debug(4) << method_name
                       << "QC variable name is changed to " << qc_name  << " for " << var_name << "\n";
                  break;
               }
            }
         }
      }

      NcVar qc_var = get_var(f_in, qc_name.c_str(), qc_group.c_str());
      if(IS_VALID_NC(qc_var)) {
         status = get_nc_data(&qc_var, qc_buf, nlocs);
         if(!status) mlog << Debug(4) << method_name
                          << "trouble getting " << var_name << "\n";
      }
      else mlog << Debug(4) << method_name
                << "\"" << var_name << "\" does not exist!\n";
   }
   if(status) {
      for(int idx=0; idx<nlocs; idx++)
         if(check_missing_thresh(qc_buf[idx])) qc_buf[idx] = bad_data_float;
   }
   else {
      for(int idx=0; idx<nlocs; idx++)
         qc_buf[idx] = bad_data_int;
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

bool has_postfix(const std::string &str_buf, std::string const &postfix) {
   auto buf_len = str_buf.length();
   auto postfix_len = postfix.length();
   if(buf_len >= postfix_len) {
      return (0 == str_buf.compare(buf_len - postfix_len, postfix_len, postfix));
   } else {
      return false;
   }
}

////////////////////////////////////////////////////////////////////////

bool is_in_metadata_map(std::string metadata_key, StringArray &available_list) {
   bool found = available_list.has(metadata_key);

   if (!found) {
      StringArray alt_names = conf_info.metadata_map[metadata_key];
      if (alt_names.n() > 0) {
         for (int idx=0; idx<alt_names.n(); idx++) {
            found = available_list.has(alt_names[idx]);
            if (found) break;
         }
      }
   }
   return found;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tioda_file\n"
        << "\tnetcdf_file\n"
        << "\t[-config config_file]\n"
        << "\t[-obs_var var]\n"
        << "\t[-iodafile ioda_file]\n"
        << "\t[-valid_beg time]\n"
        << "\t[-valid_end time]\n"
        << "\t[-nmsg n]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"ioda_file\" is the input IODA "
        << "observation file to be converted to netCDF format "
        << "(required).\n"

        << "\t\t\"netcdf_file\" indicates the name of the output "
        << "netCDF file to be written (required).\n"

        << "\t\t\"-config config_file\" is a IODA2NCConfig file containing the "
        << "desired configuration settings (optional).\n"

        << "\t\t\"-obs_var var1,...\" or multiple \"-obs_var var\" sets the variable list to be saved"
        << " from input IODA observation files (optional, default: all).\n"

        << "\t\t\"-iodafile ioda_file\" may be used to specify "
        << "additional input IODA observation files to be used "
        << "(optional).\n"

        << "\t\t\"-valid_beg time\" in YYYYMMDD[_HH[MMSS]] sets the "
        << "beginning of the retention time window (optional).\n"

        << "\t\t\"-valid_end time\" in YYYYMMDD[_HH[MMSS]] sets the "
        << "end of the retention time window (optional).\n"

        << "\t\t\"-nmsg n\" indicates the number of IODA records "
        << "to process (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t\"-compress level\" overrides the compression level of NetCDF variable ("
        << conf_info.conf.nc_compression() << ") (optional).\n\n"

        << flush;

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_ioda_files(const StringArray & a)
{
   ioda_files.add(a[0]);
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
   if(1 < tmp_len) {
      if(a[0][tmp_len-1] == '%') {
         nmsg_percent = nmsg;
      }
   }
}

////////////////////////////////////////////////////////////////////////

void set_config(const StringArray & a) {
   config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_mask_grid(const StringArray & a) {

   // List the grid masking file
   mlog << Debug(1)
        << "Grid Masking: " << a[0] << "\n";

   parse_grid_mask(a[0], mask_grid);

   // List the grid mask
   mlog << Debug(2)
        << "Parsed Masking Grid: " << mask_grid.name() << " ("
        << mask_grid.nx() << " x " << mask_grid.ny() << ")\n";
}

////////////////////////////////////////////////////////////////////////

void set_mask_poly(const StringArray & a) {
   ConcatString mask_name;

   // List the poly masking file
   mlog << Debug(1)
        << "Polyline Masking File: " << a[0] << "\n";

   parse_poly_mask(a[0], mask_poly, mask_grid, mask_area, mask_name);

   // List the mask information
   if(mask_poly.n_points() > 0) {
      mlog << Debug(2)
           << "Parsed Masking Polyline: " << mask_poly.name()
           << " containing " <<  mask_poly.n_points() << " points\n";
   }

   // List the area mask information
   if(mask_area.nx() > 0 || mask_area.ny() > 0) {
      mlog << Debug(2)
           << "Parsed Masking Area: " << mask_name
           << " for (" << mask_grid.nx() << " x " << mask_grid.ny()
           << ") grid\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void set_mask_sid(const StringArray & a) {

   // List the station ID mask
   mlog << Debug(1) << "Station ID Mask: " << a[0] << "\n";

   MaskSID ms = parse_sid_mask(a[0]);
   for(auto item : ms.sid_list) mask_sid.add(item.first);

   // List the length of the station ID mask
   mlog << Debug(2) << "Parsed Station ID Mask: " << ms.name
        << " containing " << mask_sid.n() << " stations\n";
}

////////////////////////////////////////////////////////////////////////

void set_obs_var(const StringArray & a)
{
   if("_all_" == a[0] || "all" == a[0]) do_all_vars = true;
   else {
      ConcatString arg = a[0];
      obs_var_names.add(arg.split(",+ "));
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
