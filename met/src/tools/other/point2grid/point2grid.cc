 // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////
//
//
//   Description:
//      This tool reads the point observation fields from a GOES16/17 or
//      MET generated point NetCDF (by ascii2nc, madis2nc, pb2nc, etc),
//      regrids them to the user-specified output grid, and writes the
//      regridded output data in NetCDF format.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    12-11-19  Howard Soh     Support GOES-16
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <iostream>

#include "vx_log.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_nc_util.h"
#include "vx_grid.h"
#include "vx_regrid.h"
#include "vx_util.h"
#include "vx_statistics.h"

//#include "GridTemplate.h"
#include "point2grid_conf_info.h"

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

// Constants
static const int        TYPE_OBS      = 1;
static const int        TYPE_GOES     = 5;
static const int        TYPE_GOES_ADP = 6;

static const InterpMthd DefaultInterpMthd = InterpMthd_UW_Mean;
static const int        DefaultInterpWdth = 2;
static const double     DefaultVldThresh  = 0.5;

static const float      MISSING_LATLON = -999.0;
static const int        QC_NA_INDEX = -1;

static const char * default_config_filename = "MET_BASE/config/Point2GridConfig_default";

static const char * GOES_global_attr_names[] = {
      "naming_authority",
      "project",
      "production_site",
      "production_environment",
      "spatial_resolution",
      "orbital_slot",
      "platform_ID",
      "instrument_type",
      "scene_id",
      "instrument_ID",
      "dataset_name",
      "iso_series_metadata_id",
      "title",
      "keywords",
      "keywords_vocabulary",
      "processing_level",
      "date_created",
      "cdm_data_type",
      "time_coverage_start",
      "time_coverage_end",
      "timeline_id",
      "id"
};

// Beginning and ending retention times
static IntArray message_type_list;

// Variables for command line arguments
static ConcatString InputFilename;
static ConcatString OutputFilename;
static ConcatString AdpFilename;
static ConcatString config_filename;
static PointToGridConfInfo conf_info;
static StringArray FieldSA;
static RegridInfo RGInfo;
static StringArray VarNameSA;
static int compress_level = -1;
static bool opt_override_method = false;
static bool do_gaussian_filter = false;
static SingleThresh prob_cat_thresh;

// Output NetCDF file
static NcFile *nc_out  = (NcFile *) 0;
static NcDim  lat_dim ;
static NcDim  lon_dim ;


////////////////////////////////////////////////////////////////////////

static int process_command_line(int, char **);
static void process_data_file(int obs_type);
static void process_point_file(NcFile *nc_in, MetConfig &config,
            VarInfo *, const Grid fr_grid, const Grid to_grid);
static void open_nc(const Grid &grid, const ConcatString run_cs);
static void write_nc(const DataPlane &dp, const Grid &grid,
                     const VarInfo *vinfo, const char *vname);
static void write_nc_int(const DataPlane &dp, const Grid &grid,
                     const VarInfo *vinfo, const char *vname);
static void close_nc();
static void usage();
static void set_field(const StringArray &);
static void set_method(const StringArray &);
static void set_prob_cat_thresh(const StringArray &);
static void set_vld_thresh(const StringArray &);
static void set_name(const StringArray &);
static void set_logfile(const StringArray &);
static void set_config(const StringArray &);
static void set_verbosity(const StringArray &);
static void set_compress(const StringArray &);
static void set_adp(const StringArray &);
static void set_gaussian_dx(const StringArray &);
static void set_gaussian_radius(const StringArray &);

static bool get_grid_mapping(Grid to_grid, IntArray *cellMapping,
                            const IntArray obs_index_array, const int *obs_hids,
                            const float *hdr_lats, const float *hdr_lons);

//static bool keep_message_type(const char *mt_str);
static bool keep_message_type(const int mt_index);


////////////////////////////////////////////////////////////////////////
// for GOES 16
//
static const int factor_float_to_int = 1000000;
static const char *key_geostationary_data = "MET_GEOSTATIONARY_DATA";
static const char *dim_name_lat = "lat";
static const char *dim_name_lon = "lon";
static const char *var_name_lat = "latitude";
static const char *var_name_lon = "longitude";
static const ConcatString vname_dust("Dust");
static const ConcatString vname_smoke("Smoke");

static IntArray qc_flags;

static void process_goes_file(NcFile *nc_in, MetConfig &config,
            VarInfo *, const Grid fr_grid, const Grid to_grid);
static unixtime find_valid_time(multimap<string,NcVar> mapVar);
static ConcatString get_goes_grid_input(MetConfig config, Grid fr_grid, Grid to_grid);
static void get_grid_mapping(Grid fr_grid, Grid to_grid,
            IntArray *cellMapping, ConcatString geostationary_file);
static int  get_lat_count(NcFile *);
static int  get_lon_count(NcFile *);
static NcVar get_goes_nc_var(NcFile *nc, const ConcatString var_name,
                             bool exit_if_error=true);
static bool is_time_mismatch(NcFile *nc_in, NcFile *nc_adp);
static ConcatString make_geostationary_filename(Grid fr_grid, Grid to_grid,
            ConcatString regrid_name, bool grid_map=true);
static IntArray *read_grid_mapping(const char *grid_map_file);
static void regrid_goes_variable(NcFile *nc_in, VarInfo *vinfo,
            DataPlane &fr_dp, DataPlane &to_dp,
            Grid fr_grid, Grid to_grid, IntArray *cellMapping, NcFile *nc_adp);
static void save_geostationary_data(const ConcatString geostationary_file,
            const float *latitudes, const float *longitudes,
            const GoesImagerData grid_data);
static void set_qc_flags(const StringArray &);
static void write_grid_mapping(const char *grid_map_file,
            IntArray *cellMapping, Grid from_grid, Grid to_grid);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Store the program name
   program_name = get_short_name(argv[0]);

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   int obs_type = process_command_line(argc, argv);

   // Process the input data file
   process_data_file(obs_type);

   return(0);
}

////////////////////////////////////////////////////////////////////////

int process_command_line(int argc, char **argv) {
   CommandLine cline;
   int obs_type = TYPE_OBS;
   static const char *method_name = "process_command_line() -> ";

   // Set default regridding options
   RGInfo.enable     = true;
   RGInfo.field      = FieldType_None;
   RGInfo.method     = DefaultInterpMthd;
   RGInfo.width      = DefaultInterpWdth;
   RGInfo.vld_thresh = DefaultVldThresh;
   RGInfo.gaussian.dx     = default_gaussian_dx;
   RGInfo.gaussian.radius = default_gaussian_radius;
   RGInfo.gaussian.trunc_factor = default_trunc_factor;

   // Check for zero arguments
   if(argc == 1) usage();

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_field,      "-field",      1);
   cline.add(set_method,     "-method",     1);
   cline.add(set_vld_thresh, "-vld_thresh", 1);
   cline.add(set_name,       "-name",       1);
   cline.add(set_logfile,    "-log",        1);
   cline.add(set_verbosity,  "-v",          1);
   cline.add(set_compress,   "-compress",   1);
   cline.add(set_qc_flags,   "-qc",         1);
   cline.add(set_adp,        "-adp",        1);
   cline.add(set_config,     "-config",     1);
   cline.add(set_prob_cat_thresh, "-prob_cat_thresh", 1);
   cline.add(set_gaussian_radius, "-gaussian_radius", 1);
   cline.add(set_gaussian_dx,     "-gaussian_dx",     1);

   cline.allow_numbers();

   // Parse the command line
   cline.parse();

   // Check for error. There should be three arguments left:
   // - input filename
   // - destination grid,
   // - output filename
   if(cline.n() != 3) usage();

   // Store the filenames and config string.
   InputFilename  = cline[0];
   RGInfo.name    = cline[1];
   OutputFilename = cline[2];

   // Check for at least one configuration string
   if(FieldSA.n() < 1) {
      mlog << Error << "\nprocess_command_line() -> "
           << "The -field option must be used at least once!\n\n";
      usage();
   }

   // Check that same variable is required multiple times without -name argument
   if(VarNameSA.n() == 0) {
      VarInfo *vinfo;
      MetConfig config;
      VarInfoFactory v_factory;
      ConcatString vname;
      StringArray var_names;
      vinfo = v_factory.new_var_info(FileType_NcMet);
      for(int i=0; i<FieldSA.n(); i++) {
         vinfo->clear();
         // Populate the VarInfo object using the config string
         config.read_string(FieldSA[i].c_str());
         vinfo->set_dict(config);
         vname = vinfo->name();
         if (var_names.has(vname)) {
            mlog << Error << "\n" << method_name
                 << "Please add -name argument to avoid the output name conflicts on handling the variable \""
                 << vname << "\" multiple times.\n\n";
            usage();
         }
         else var_names.add(vname);
      }
      // Clean up
      if(vinfo) { delete vinfo; vinfo = (VarInfo *) 0; }
   }
   // Check that the number of output names and fields match
   else if(VarNameSA.n() != FieldSA.n()) {
      mlog << Error << "\n" << method_name
           << "When the -name option is used, the number of entries ("
           << VarNameSA.n() << ") must match the number of "
           << "-field entries (" << FieldSA.n() << ")!\n\n";
      usage();
   }

   RGInfo.validate_point();
   if (do_gaussian_filter) RGInfo.gaussian.compute();

   // Read the config files
   conf_info.read_config(default_config_filename,
                         (config_filename.empty()
                          ? default_config_filename : config_filename.c_str()));
   // Process the configuration
   conf_info.process_config();

   ConcatString att_val;
   if (get_global_att(InputFilename.c_str(), (string)"scene_id", att_val)) {
      obs_type = TYPE_GOES;
      if (0 < AdpFilename.length()) {
         obs_type = TYPE_GOES_ADP;
         if (!file_exists(AdpFilename.c_str())) {
            mlog << Error << method_name << "ADP input \"" << AdpFilename << "\" does not exist!\n";
            exit(1);
         }
      }
   }

   return obs_type;
}

////////////////////////////////////////////////////////////////////////

void process_data_file(int obs_type) {
   DataPlane fr_dp;
   Grid fr_grid, to_grid;
   GrdFileType ftype;
   ConcatString run_cs;
   bool goes_data = (obs_type == TYPE_GOES || obs_type == TYPE_GOES_ADP);
   NcFile *nc_in = (NcFile *)0;
   static const char *method_name = "process_data_file() ";

   // Initialize configuration object
   MetConfig config;
   config.read(replace_path(config_const_filename).c_str());
   config.read_string(FieldSA[0].c_str());

   // Note: The command line argument MUST processed before this
   if (compress_level < 0) compress_level = config.nc_compression();

   // Get the gridded file type from config string, if present
   ftype = parse_conf_file_type(&config);

   // Read the input data file
   Met2dDataFileFactory m_factory;
   Met2dDataFile *fr_mtddf = (Met2dDataFile *) 0;
   fr_mtddf = m_factory.new_met_2d_data_file(InputFilename.c_str(), ftype);

   if(!fr_mtddf) {
      mlog << Error << "\n" << method_name << " -> "
           << "\"" << InputFilename << "\" not a valid data file\n\n";
      exit(1);
   }

   // Store the input data file types
   ftype = fr_mtddf->file_type();

   // Setup the VarInfo request object
   VarInfoFactory v_factory;
   VarInfo *vinfo;
   vinfo = v_factory.new_var_info(ftype);

   if(!vinfo) {
      mlog << Error << "\n" << method_name << " -> "
           << "unable to determine file type of \"" << InputFilename
           << "\"\n\n";
      exit(1);
   }

   // For python types read the first field to set the grid

   // Determine the "from" grid
   fr_grid = fr_mtddf->grid();
   if (goes_data) {
      mlog << Debug(2) << "Input grid: " << fr_grid.serialize() << "\n";
   }

   // Determine the "to" grid
   to_grid = parse_vx_grid(RGInfo, &fr_grid, &fr_grid);
   mlog << Debug(2) << "Output grid: " << to_grid.serialize() << "\n";

   //GridTemplateFactory gtf;
   mlog << Debug(2) << "Interpolation options: "
        << "method = " << interpmthd_to_string(RGInfo.method)
        << ", vld_thresh = " << RGInfo.vld_thresh << "\n";

   // Build the run command string
   run_cs << "Point obs (" << fr_grid.serialize() << ") to " << to_grid.serialize();
   if (goes_data) {
      ConcatString grid_string = get_goes_grid_input(config, fr_grid, to_grid);
      if (grid_string.length() > 0) run_cs << " with " << grid_string;
   }

   // Open the output file
   open_nc(to_grid, run_cs);

   // Open the input file
   mlog << Debug(1)  << "Reading data file: " << InputFilename << "\n";
   nc_in = open_ncfile(InputFilename.c_str());

   if (goes_data) {
      process_goes_file(nc_in, config, vinfo, fr_grid, to_grid);
   }
   else {
      process_point_file(nc_in, config, vinfo, fr_grid, to_grid);
   }

   // Close the output file
   close_nc();

   // Clean up
   if(nc_in)    { delete nc_in;    nc_in  = 0; }
   if(fr_mtddf) { delete fr_mtddf; fr_mtddf = (Met2dDataFile *) 0; }
   if(vinfo)    { delete vinfo;    vinfo    = (VarInfo *)       0; }

   return;
}

////////////////////////////////////////////////////////////////////////
// returns true if no error

bool get_nc_data_int_array(NcFile *nc, char *var_name, int *data_array, bool stop=true) {
   bool status = false;
   NcVar nc_var = get_nc_var(nc, (char *)var_name, stop);
   if (IS_INVALID_NC(nc_var)) {
      if (stop) exit(1);
   }
   else {
      status = get_nc_data(&nc_var, data_array);
   }
   return status;
}

////////////////////////////////////////////////////////////////////////
// returns true if no error

bool get_nc_data_int_array(NcFile *nc, const char *var_name, int *data_array, bool stop=true) {
   bool status = false;
   char *_var_name = strdup(var_name);
   if (_var_name) {
      status = get_nc_data_int_array(nc, _var_name, data_array, stop);
      free(_var_name);
   }
   return status;
}

////////////////////////////////////////////////////////////////////////
// returns true if no error

bool get_nc_data_float_array(NcFile *nc, char *var_name, float *data_array) {
   NcVar nc_var = get_nc_var(nc, (char *)var_name);
   if (IS_INVALID_NC(nc_var)) exit(1);

   bool status = get_nc_data(&nc_var, data_array);
   return status;
}

////////////////////////////////////////////////////////////////////////
// returns true if no error

bool get_nc_data_float_array(NcFile *nc, const char *var_name, float *data_array) {
   bool status = false;
   char *_var_name = strdup(var_name);
   if (_var_name) {
      status = get_nc_data_float_array(nc, _var_name, data_array);
      free(_var_name);
   }
   return status;
}

////////////////////////////////////////////////////////////////////////
// returns true if no error

bool get_nc_data_string_array(NcFile *nc, char *var_name,
                              StringArray *stringArray) {
   NcVar nc_var = get_nc_var(nc, var_name, true);
   if (IS_INVALID_NC(nc_var)) exit(1);

   bool status = get_nc_data_to_array(&nc_var, stringArray);
   return status;
}


////////////////////////////////////////////////////////////////////////
// returns true if no error

bool get_nc_data_string_array(NcFile *nc, const char *var_name,
                              StringArray *stringArray) {
   bool status = false;
   char *_var_name = strdup(var_name);
   if (_var_name) {
      status = get_nc_data_string_array(nc, _var_name, stringArray);
      free(_var_name);
   }
   return status;
}

////////////////////////////////////////////////////////////////////////
// Check the message types

void prepare_message_types(const StringArray hdr_types) {
   static const char *method_name = "prepare_message_types() -> ";
   message_type_list.clear();
   if (0 < conf_info.message_type.n()) {
      int hdr_idx;
      ConcatString msg_list_str;
      for(int idx=0; idx<conf_info.message_type.n(); idx++) {
         string message_type_str = conf_info.message_type[idx];
         if (hdr_types.has(message_type_str, hdr_idx)) {
            message_type_list.add(hdr_idx);
         }
         else {
            mlog << Debug(3) << method_name << "The message type \""
                 << message_type_str << "\" does not exist\n";
         }
         if (idx > 0) msg_list_str << ",";
         msg_list_str << message_type_str;
      }
      if (0 == message_type_list.n()) {
         mlog << Error << method_name << " No matching message types ["
              << msg_list_str << "].\n";
         exit(1);
      }
   }
}

////////////////////////////////////////////////////////////////////////

IntArray prepare_qc_array(const IntArray qc_flags, StringArray qc_tables) {
   IntArray qc_idx_array;
   bool has_qc_flags = (qc_flags.n() > 0);
   if (has_qc_flags) {
      for(int idx=0; idx<qc_tables.n(); idx++) {
         int qc_value = (qc_tables[idx] == "NA")
                        ? QC_NA_INDEX : atoi(qc_tables[idx].c_str());
         if (qc_flags.has(qc_value) && !qc_idx_array.has(idx)) {
            qc_idx_array.add(idx);
         }
      }
   }
   return qc_idx_array;
}

////////////////////////////////////////////////////////////////////////

void process_point_file(NcFile *nc_in, MetConfig &config, VarInfo *vinfo,
                        const Grid fr_grid, const Grid to_grid) {
   int nhdr, nobs;
   int nx, ny, var_count, to_count, var_count2;
   int idx, hdr_idx;
   int var_idx_or_gc;
   int filtered_by_time, filtered_by_msg_type, filtered_by_qc;
   ConcatString vname, vname_cnt, vname_mask;
   DataPlane fr_dp, to_dp;
   DataPlane cnt_dp, mask_dp;
   DataPlane prob_dp, prob_mask_dp;
   NcVar var_obs_gc, var_obs_var;

   bool use_var_id = true;
   bool has_prob_thresh = !prob_cat_thresh.check(bad_data_double);

   unixtime requested_valid_time, valid_time = 0;
   IntArray *cellMapping = (IntArray *)0;
   static const char *method_name = "process_point_file() -> ";

   if (!get_dim(nc_in, ConcatString(nc_dim_nhdr), nhdr, true)) {
      mlog << Error << "\n" << method_name
           << "can not find the header dimension\"" << nc_dim_nhdr << "\".\n\n";
      exit(1);
   }
   if (!get_dim(nc_in, ConcatString(nc_dim_nobs), nobs, true)) {
      mlog << Error << "\n" << method_name
           << "can not find the obs dimension\"" << nc_dim_nobs << "\".\n\n";
      exit(1);
   }

   // Check for at least one configuration string
   if(FieldSA.n() < 1) {
      mlog << Error << "\n" << method_name
           << "The -field option must be used at least once!\n\n";
      usage();
   }

   int *obs_ids = new int[nobs];       // grib_code or var_id
   int *obs_hids = new int[nobs];
   float *hdr_lats = new float[nhdr];
   float *hdr_lons = new float[nhdr];
   //float *hdr_elvs[nhdr] = new float[nhdr];
   float *obs_lvls = new float[nobs];
   float *obs_hgts = new float[nobs];
   float *obs_vals = new float[nobs];
   int *hdr_typ_ids = new int[nhdr];
   int *hdr_vld_ids = new int[nhdr];
   int *obs_qty_ids = new int[nobs];
   IntArray var_index_array;
   IntArray valid_time_array;
   StringArray qc_tables;
   StringArray var_names;
   StringArray hdr_types;
   StringArray hdr_valid_times;

   if (!get_nc_data_string_array(nc_in, nc_var_hdr_typ_tbl, &hdr_types)) exit(1);
   // Check the message types
   prepare_message_types(hdr_types);

   // Check and read obs_vid and obs_var if exists
   bool success_to_read = true;
   NcVar obs_vid_var = get_var(nc_in, nc_var_obs_vid);
   if (IS_VALID_NC(obs_vid_var)) {
      if (success_to_read) success_to_read = get_nc_data_int_array(nc_in, nc_var_obs_vid, obs_ids);
      if (success_to_read) success_to_read = get_nc_data_string_array(nc_in, nc_var_obs_var, &var_names);
   }
   else {
      use_var_id = false;
      if (success_to_read) success_to_read = get_nc_data_int_array(nc_in, nc_var_obs_gc, obs_ids, false);
      if (!success_to_read) {
         mlog << Error << "\n" << method_name
              << "\"" << InputFilename << "\" is very old format. Not supported\n\n";
      }
   }

   if (success_to_read)
      success_to_read = get_nc_data_int_array(nc_in, nc_var_obs_hid, obs_hids);
   if (success_to_read)
      success_to_read = get_nc_data_int_array(nc_in, nc_var_hdr_vld, hdr_vld_ids);
   if (success_to_read)
      success_to_read = get_nc_data_int_array(nc_in, nc_var_hdr_typ, hdr_typ_ids);
   if (success_to_read)
      success_to_read = get_nc_data_int_array(nc_in, nc_var_obs_qty, obs_qty_ids);
   if (success_to_read)
      success_to_read = get_nc_data_float_array(nc_in, nc_var_hdr_lat, hdr_lats);
   if (success_to_read)
      success_to_read = get_nc_data_float_array(nc_in, nc_var_hdr_lon, hdr_lons);
   //if (!get_nc_data_float_array(nc_in, nc_var_hdr_elv, hdr_elvs)) exit(1);
   if (success_to_read)
      success_to_read = get_nc_data_float_array(nc_in, nc_var_obs_lvl, obs_lvls);
   if (success_to_read)
      success_to_read = get_nc_data_float_array(nc_in, nc_var_obs_hgt, obs_hgts);
   if (success_to_read)
      success_to_read = get_nc_data_float_array(nc_in, nc_var_obs_val, obs_vals);
   if (success_to_read)
      success_to_read = get_nc_data_string_array(
            nc_in, nc_var_hdr_vld_tbl, &hdr_valid_times);
   if (success_to_read)
      success_to_read = get_nc_data_string_array(
            nc_in, nc_var_obs_qty_tbl, &qc_tables);
   if (success_to_read) {
      bool has_qc_flags = (qc_flags.n() > 0);
      IntArray qc_idx_array = prepare_qc_array(qc_flags, qc_tables);

      // Initialize configuration object

      nx = to_grid.nx();
      ny = to_grid.ny();
      to_dp.set_size(nx, ny);
      cnt_dp.set_size(nx, ny);
      mask_dp.set_size(nx, ny);
      if (has_prob_thresh || do_gaussian_filter) {
         prob_dp.set_size(nx, ny);
         prob_mask_dp.set_size(nx, ny);
      }

      // Loop through the requested fields
      int obs_count_zero_to, obs_count_non_zero_to;
      int obs_count_zero_from, obs_count_non_zero_from;

      obs_count_zero_to = obs_count_non_zero_to = 0;
      obs_count_zero_from = obs_count_non_zero_from = 0;
      for(int i=0; i<FieldSA.n(); i++) {
         var_idx_or_gc = -1;
         // Initialize
         vinfo->clear();

         // Populate the VarInfo object using the config string
         config.read_string(FieldSA[i].c_str());
         vinfo->set_dict(config);

         // Check the variable name
         ConcatString error_msg;
         vname = vinfo->name();
         bool exit_by_field_name_error = false;
         if (vname == "obs_val" || vname == "obs_lvl" || vname == "obs_hgt") {
            exit_by_field_name_error = true;
            error_msg << "The variable \"" << vname
                      << "\" exists but is not a valid field name.\n";
         }
         else {
            if (use_var_id) {
               if (!var_names.has(vname, var_idx_or_gc)) {
                  exit_by_field_name_error = true;;
                  error_msg << "The variable \"" << vname << "\" is not available.\n";
               }
            }
            else {
               char grib_code[128];
               var_idx_or_gc = atoi(vname.c_str());
               sprintf(grib_code, "%d", var_idx_or_gc);
               if (vname != grib_code) {
                  exit_by_field_name_error = true;;
                  error_msg << "Invalid GRIB code [" << vname << "]\n";
               }
               else {
                  bool not_found_grib_code = true;
                  for (int idx=0; idx<nobs; idx++) {
                     if (var_idx_or_gc == obs_ids[idx]) {
                        not_found_grib_code = false;
                        break;
                     }
                  }
                  if (not_found_grib_code) {
                     exit_by_field_name_error = true;;
                     error_msg << "No data for the GRIB code [" << vname << "]\n";
                  }
               }
            }
         }

         if (exit_by_field_name_error) {
            ConcatString log_msg;
            if (use_var_id) {
               for (int idx=0; idx<var_names.n(); idx++) {
                  if (0 < idx) log_msg << ", ";
                  log_msg << var_names[idx];
               }
            }
            else {
               log_msg << "GRIB codes: ";
               IntArray grib_codes;
               for (int idx=0; idx<nobs; idx++) {
                  if (!grib_codes.has(obs_ids[idx])) {
                     grib_codes.add(obs_ids[idx]);
                     if (0 < idx) log_msg << ", ";
                     log_msg << obs_ids[idx];
                  }
               }
            }
            mlog << Error << "\n" << method_name
                 << error_msg
                 << "Try setting the \"name\" in the \"-field\" command line option to one of the available names:\n"
                 << "\t" << log_msg <<"\n\n";
            exit(1);
         }

         // Check the time range. Apply the time window
         bool valid_time_from_config = true;
         unixtime valid_beg_ut, valid_end_ut, obs_time;

         valid_time_array.clear();
         valid_time = vinfo->valid();
         if (valid_time == 0) valid_time = conf_info.valid_time;
         requested_valid_time = valid_time;
         if (0 < valid_time) {
            valid_beg_ut = valid_end_ut = valid_time;
            if (!is_eq(bad_data_int, conf_info.beg_ds)) valid_beg_ut += conf_info.beg_ds;
            if (!is_eq(bad_data_int, conf_info.end_ds)) valid_end_ut += conf_info.end_ds;
            for(idx=0; idx<hdr_valid_times.n(); idx++) {
               obs_time = timestring_to_unix(hdr_valid_times[idx].c_str());
               if (valid_beg_ut <= obs_time and obs_time <= valid_end_ut) {
                  valid_time_array.add(idx);
                  mlog << Debug(4) << method_name << "The obs time "
                       << hdr_valid_times[idx] << " was included\n";
               }
            }
            valid_time_array.add(bad_data_int);    // added dummy entry
         }
         else {
            valid_time_from_config = false;
            // Set the latest available valid time
            valid_time = 0;
            for(idx=0; idx<hdr_valid_times.n(); idx++) {
               obs_time = timestring_to_unix(hdr_valid_times[idx].c_str());
               if (obs_time > valid_time) valid_time = obs_time;
            }
         }
         mlog << Debug(3) << method_name << " valid_time from "
              << (valid_time_from_config ? "config" : "input data") << ": "
              << unix_to_yyyymmdd_hhmmss(valid_time) << "\n";

         to_dp.set_init(valid_time);
         to_dp.set_valid(valid_time);
         cnt_dp.set_init(valid_time);
         cnt_dp.set_valid(valid_time);
         mask_dp.set_init(valid_time);
         mask_dp.set_valid(valid_time);
         if (has_prob_thresh || do_gaussian_filter) {
            prob_dp.set_init(valid_time);
            prob_dp.set_valid(valid_time);
            prob_mask_dp.set_init(valid_time);
            prob_mask_dp.set_valid(valid_time);
         }

         var_index_array.clear();
         // Select output variable name
         vname = (VarNameSA.n() == 0)
                 ? conf_info.get_var_name(vinfo->name())
                 : conf_info.get_var_name(VarNameSA[i]);
         mlog << Debug(4) << method_name
              << " var: " << vname << ", index: " << var_idx_or_gc << ".\n";

         var_count = var_count2 = to_count = 0;
         filtered_by_time = filtered_by_msg_type = filtered_by_qc = 0;
         for (idx=0; idx < nobs; idx++) {
            if (var_idx_or_gc == obs_ids[idx]) {
               var_count2++;
               hdr_idx = obs_hids[idx];
               if (0 < valid_time_array.n() &&
                     !valid_time_array.has(hdr_vld_ids[hdr_idx])) {
                  filtered_by_time++;
                  continue;
               }

               if(!keep_message_type(hdr_typ_ids[hdr_idx])) {
                  filtered_by_msg_type++;
                  continue;
               }

               //Filter by QC flag
               if (has_qc_flags && !qc_idx_array.has(obs_qty_ids[idx])) {
                  filtered_by_qc++;
                  continue;
               }

               var_index_array.add(idx);
               var_count++;
               if (is_eq(obs_vals[idx], 0.)) obs_count_zero_from++;
               else obs_count_non_zero_from++;
            }
         }
         if (cellMapping) {
            delete [] cellMapping;
            cellMapping = (IntArray *)0;
         }
         cellMapping = new IntArray[nx * ny];
         if (get_grid_mapping(to_grid, cellMapping, var_index_array,
               obs_hids, hdr_lats, hdr_lons)) {
            int from_index;
            IntArray cellArray;
            NumArray dataArray;
            int offset = 0;
            int valid_count = 0;
            int absent_count = 0;
            int censored_count = 0;
            int qc_filtered_count = 0;
            int adp_qc_filtered_count = 0;
            float data_value;
            float from_min_value =  10e10;
            float from_max_value = -10e10;

            to_count = 0;
            to_dp.set_constant(bad_data_double);
            cnt_dp.set_constant(0);
            mask_dp.set_constant(0);
            if (has_prob_thresh || do_gaussian_filter) {
               prob_dp.set_constant(0);
               prob_mask_dp.set_constant(0);
            }

            for (int x_idx = 0; x_idx<nx; x_idx++) {
               for (int y_idx = 0; y_idx<ny; y_idx++) {
                  offset = to_dp.two_to_one(x_idx,y_idx);
                  cellArray = cellMapping[offset];
                  int prob_cnt = 0;
                  int prob_value_sum = 0;
                  if (0 < cellArray.n()) {
                     valid_count = 0;
                     dataArray.clear();
                     for (int dIdx=0; dIdx<cellArray.n(); dIdx++) {
                        from_index = cellArray[dIdx];
                        data_value = obs_vals[from_index];
                        if (is_eq(data_value, bad_data_float)) continue;

                        if(mlog.verbosity_level() >= 4) {
                           if (from_min_value > data_value) from_min_value = data_value;
                           if (from_max_value < data_value) from_max_value = data_value;
                        }

                        for(int ic=0; ic<vinfo->censor_thresh().n(); ic++) {
                           // Break out after the first match.
                           if(vinfo->censor_thresh()[ic].check(data_value)) {
                              data_value = vinfo->censor_val()[ic];
                              censored_count++;
                              break;
                           }
                        }

                        dataArray.add(data_value);
                        valid_count++;
                     }
                     if (0 < valid_count) to_count++;

                     int data_count = dataArray.n();
                     if (0 < data_count) {
                        float to_value;
                        if      (RGInfo.method == InterpMthd_Min) to_value = dataArray.min();
                        else if (RGInfo.method == InterpMthd_Max) to_value = dataArray.max();
                        else if (RGInfo.method == InterpMthd_Median) {
                           cellArray.sort_increasing();
                           to_value = dataArray[data_count/2];
                           if (0 == data_count % 2)
                              to_value = (to_value + dataArray[(data_count/2)+1])/2;
                        }
                        else to_value = dataArray.sum() / data_count;

                        if (is_eq(to_value, 0.)) obs_count_zero_to++;
                        else obs_count_non_zero_to++;

                        cnt_dp.set(data_count, x_idx, y_idx);
                        mask_dp.set(1, x_idx, y_idx);
                        to_dp.set(to_value, x_idx, y_idx);
                        if ((has_prob_thresh && prob_cat_thresh.check(to_value))
                            || (do_gaussian_filter && !has_prob_thresh)) {
                           prob_dp.set(1, x_idx, y_idx);
                           prob_mask_dp.set(1, x_idx, y_idx);
                        }

                        if (1 < data_count) {
                           mlog << Debug(9) << method_name
                                << " to_value:" << to_value
                                << " at " << x_idx << "," << y_idx
                                << ", max: " << dataArray.max()
                                << ", min: " << dataArray.min()
                                << ", mean: " << dataArray.sum()/data_count
                                << " from " << data_count << " data values.\n";
                        }
                        mlog << Debug(8) << method_name << " data at " << x_idx << "," << y_idx
                             << ", value: " << to_value << "\n";
                     }
                  }
               }
            }
         }

         // Write the regridded data
         write_nc(to_dp, to_grid, vinfo, vname.c_str());

         vname_cnt = vname;
         vname_cnt << "_cnt";
         vname_mask = vname;
         vname_mask << "_mask";

         ConcatString tmp_long_name;
         ConcatString var_long_name = vinfo->long_name();
         ConcatString dim_string = "(*,*)";

         tmp_long_name = vname_cnt;
         tmp_long_name << dim_string;
         vinfo->set_long_name(tmp_long_name.c_str());
         write_nc_int(cnt_dp, to_grid, vinfo, vname_cnt.c_str());

         tmp_long_name = vname_mask;
         tmp_long_name << dim_string;
         vinfo->set_long_name(tmp_long_name.c_str());
         write_nc_int(mask_dp, to_grid, vinfo, vname_mask.c_str());

         if (has_prob_thresh || do_gaussian_filter) {
            ConcatString vname_prob = vname;
            vname_prob << "_prob_" << prob_cat_thresh.get_abbr_str();
            ConcatString vname_prob_mask = vname_prob;
            vname_prob_mask << "_mask";

            if (do_gaussian_filter) interp_gaussian_dp(prob_dp, RGInfo.gaussian, RGInfo.vld_thresh);

            tmp_long_name = vname_prob;
            tmp_long_name << dim_string;
            vinfo->set_long_name(tmp_long_name.c_str());
            write_nc(prob_dp, to_grid, vinfo, vname_prob.c_str());
            if (do_gaussian_filter) {
               NcVar prob_var = get_var(nc_out, vname_prob.c_str());
               if (IS_VALID_NC(prob_var)) {
                  add_att(&prob_var, "gaussian_radius", RGInfo.gaussian.radius);
                  add_att(&prob_var, "gaussian_dx", RGInfo.gaussian.dx);
                  add_att(&prob_var, "trunc_factor", RGInfo.gaussian.trunc_factor);
               }
            }

            tmp_long_name = vname_prob_mask;
            tmp_long_name << dim_string;
            vinfo->set_long_name(tmp_long_name.c_str());
            write_nc_int(prob_mask_dp, to_grid, vinfo, vname_prob_mask.c_str());
         }
         vinfo->set_long_name(var_long_name.c_str());

         mlog << Debug(7) << method_name << " obs_count_zero_to: " << obs_count_zero_to
              << ", obs_count_non_zero_to: " << obs_count_non_zero_to << "\n";

         ConcatString log_msg;
         log_msg << "Filtered by time: " << filtered_by_time;
         if (0 < requested_valid_time) {
            log_msg << " [" << unix_to_yyyymmdd_hhmmss(requested_valid_time) << "]";
         }
         log_msg << ", by msg_type: " << filtered_by_msg_type;
         if (0 < filtered_by_msg_type) {
            log_msg << " [";
            for(idx=0; idx<conf_info.message_type.n(); idx++) {
               if (idx > 0) log_msg << ",";
               log_msg << conf_info.message_type[idx];
            }
            log_msg << "]";
         }
         log_msg << ", by QC: " << filtered_by_qc;
         if (0 < filtered_by_qc) {
            log_msg << " [";
            for(idx=0; idx<qc_flags.n(); idx++) {
               if (idx > 0) log_msg << ",";
               log_msg << qc_flags[idx];
            }
            log_msg << "]";
         }
         log_msg << ", out of " << var_count2;
         int filtered_count = filtered_by_msg_type + filtered_by_qc + requested_valid_time;
         if (0 == var_count) {
            if (0 == filtered_count) {
               mlog << Error << method_name << " No valid data for the variable ["
                    << vinfo->name() << "]\n";
            }
            else {
               mlog << Error << method_name << " No valid data after filtering.\n\t"
                    << log_msg << ".\n";
            }
            exit(1);
         }
         else {
            mlog << Debug(2) << method_name << " var_count=" << var_count
                 << ", grid: " << to_count << " out of " << (nx * ny) << "  "
                 << (0 < filtered_count ? log_msg.c_str() : " ")
                 //<< ",    Obs_value] zero: " << obs_count_zero_from
                 //<< ", non_zero: " <<obs_count_non_zero_from
                 << "\n";
         }
      } // end for i

      if (cellMapping) {
         delete [] cellMapping;   cellMapping = (IntArray *)0;
      }
   }

   delete [] obs_ids;
   delete [] obs_hids;
   delete [] hdr_lats;
   delete [] hdr_lons;
   delete [] obs_lvls;
   delete [] obs_hgts;
   delete [] obs_vals;
   delete [] hdr_typ_ids;
   delete [] hdr_vld_ids;
   delete [] obs_qty_ids;

   return;
}

////////////////////////////////////////////////////////////////////////

void open_nc(const Grid &grid, ConcatString run_cs) {

   // Create output file
   nc_out = open_ncfile(OutputFilename.c_str(), true);

   if(IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nopen_nc() -> "
           << "trouble opening output NetCDF file \""
           << OutputFilename << "\"\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, OutputFilename.c_str(), program_name.c_str());

   // Add the run command
   add_att(nc_out, "RunCommand", run_cs);

   // Add the projection information
   write_netcdf_proj(nc_out, grid);

   // Define Dimensions
   lat_dim = add_dim(nc_out, "lat", (long) grid.ny());
   lon_dim = add_dim(nc_out, "lon", (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(nc_out, &lat_dim, &lon_dim, grid);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nc_data(const DataPlane &dp, const Grid &grid, NcVar *data_var) {

   // Allocate memory to store data values for each grid point
   float *data = new float [grid.nx()*grid.ny()];

   // Store the data
   int grid_nx = grid.nx();
   int grid_ny = grid.ny();
   for(int x=0; x<grid_nx; x++) {
      for(int y=0; y<grid_ny; y++) {
         int n = DefaultTO.two_to_one(grid_nx, grid_ny, x, y);
         data[n] = (float) dp(x, y);
      } // end for y
   } // end for x

   // Write out the data
   if(!put_nc_data_with_dims(data_var, &data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_nc_data() -> "
           << "error writing data to the output file.\n\n";
      exit(1);
   }

   // Clean up
   if(data) { delete [] data;  data = (float *)  0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nc_data_int(const DataPlane &dp, const Grid &grid, NcVar *data_var) {

   // Allocate memory to store data values for each grid point
   int *data = new int [grid.nx()*grid.ny()];

   // Store the data
   int grid_nx = grid.nx();
   int grid_ny = grid.ny();
   for(int x=0; x<grid_nx; x++) {
      for(int y=0; y<grid_ny; y++) {
         int n = DefaultTO.two_to_one(grid_nx, grid_ny, x, y);
         data[n] = (int) dp(x, y);
      } // end for y
   } // end for x

   // Write out the data
   if(!put_nc_data_with_dims(data_var, &data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_nc_data_int() -> "
           << "error writing data to the output file.\n\n";
      exit(1);
   }

   // Clean up
   if(data) { delete [] data;  data = (int *)  0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nc(const DataPlane &dp, const Grid &grid,
              const VarInfo *vinfo, const char *vname) {

   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = 0;

   NcVar data_var = add_var(nc_out, (string)vname, ncFloat,
                            lat_dim, lon_dim, deflate_level);
   add_att(&data_var, "name", (string)vname);
   add_att(&data_var, "long_name", (string)vinfo->long_name());
   add_att(&data_var, "level", (string)vinfo->level_name());
   add_att(&data_var, "units", (string)vinfo->units());
   add_att(&data_var, "_FillValue", bad_data_float);
   write_netcdf_var_times(&data_var, dp);

   write_nc_data(dp, grid, &data_var);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nc_int(const DataPlane &dp, const Grid &grid,
              const VarInfo *vinfo, const char *vname) {

   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = 0;

   NcVar data_var = add_var(nc_out, (string)vname, ncInt,
                            lat_dim, lon_dim, deflate_level);
   add_att(&data_var, "name", (string)vname);
   add_att(&data_var, "long_name", (string)vinfo->long_name());
   add_att(&data_var, "level", (string)vinfo->level_name());
   add_att(&data_var, "units", (string)vinfo->units());
   add_att(&data_var, "_FillValue", bad_data_int);
   write_netcdf_var_times(&data_var, dp);

   write_nc_data_int(dp, grid, &data_var);

   return;
}

//
////////////////////////////////////////////////////////////////////////
// GOES related modules
//

////////////////////////////////////////////////////////////////////////

void process_goes_file(NcFile *nc_in, MetConfig &config, VarInfo *vinfo,
      const Grid fr_grid, const Grid to_grid) {
   DataPlane fr_dp, to_dp;
   ConcatString vname;
   unixtime valid_time = 0;
   int global_attr_count;
   bool opt_all_attrs = false;
   NcFile *nc_adp = (NcFile *)0;
   IntArray *cellMapping = (IntArray *)0;
   static const char *method_name = "process_goes_file() ";

   // Initialize configuration object
   //MetConfig config;
   //config.read(replace_path(config_const_filename).c_str());
   //config.read_string(FieldSA[0].c_str());

   ConcatString tmp_dir = config.get_tmp_dir();
   ConcatString geostationary_file(tmp_dir);
   ConcatString grid_map_file(tmp_dir);
   grid_map_file.add("/");
   grid_map_file.add(make_geostationary_filename(fr_grid, to_grid, RGInfo.name));
   geostationary_file.add("/");
   geostationary_file.add(make_geostationary_filename(fr_grid, to_grid, RGInfo.name, false));

   // Open ADP file if exists
   if (0 < AdpFilename.length() && file_exists(AdpFilename.c_str())) {
      nc_adp = open_ncfile(AdpFilename.c_str());
      if (IS_INVALID_NC_P(nc_adp)) {
         mlog << Error << method_name << "Can't open the ADP input \"" << AdpFilename << "\"\n";
         exit(1);
      }
      else if (is_time_mismatch(nc_in, nc_adp)) {
         exit(1);
      }
   }

   multimap<string,NcVar> mapVar = GET_NC_VARS_P(nc_in);

   valid_time = find_valid_time(mapVar);
   to_dp.set_size(to_grid.nx(), to_grid.ny());
   global_attr_count =  sizeof(GOES_global_attr_names)/sizeof(*GOES_global_attr_names);
   if (file_exists(grid_map_file.text())) {
      cellMapping = read_grid_mapping(grid_map_file.text());
   }
   else {
      cellMapping = new IntArray[to_grid.nx() * to_grid.ny()];
      get_grid_mapping(fr_grid, to_grid, cellMapping, geostationary_file);
      write_grid_mapping(grid_map_file.text(), cellMapping,
            fr_grid, to_grid);
   }

   // Loop through the requested fields
   for(int i=0; i<FieldSA.n(); i++) {

      // Initialize
      vinfo->clear();

      // Populate the VarInfo object using the config string
      config.read_string(FieldSA[i].c_str());
      vinfo->set_dict(config);

      to_dp.erase();
      to_dp.set_init(valid_time);
      to_dp.set_valid(valid_time);
      regrid_goes_variable(nc_in, vinfo, fr_dp, to_dp,
            fr_grid, to_grid, cellMapping, nc_adp);

      // List range of data values
      if(mlog.verbosity_level() >= 2) {
         double fr_dmin, fr_dmax, to_dmin, to_dmax;
         fr_dp.data_range(fr_dmin, fr_dmax);
         to_dp.data_range(to_dmin, to_dmax);
         mlog << Debug(2) << "Range of data (" << FieldSA[i] << ")\n"
              << "\tinput: " << fr_dmin << " to " << fr_dmax
              << "\tregridded: " << to_dmin << " to " << to_dmax << ".\n";
      }

      // Select output variable name
      if(VarNameSA.n() == 0) {
         vname << cs_erase << vinfo->name();
      }
      else {
         vname = VarNameSA[i];
      }

      // Write the regridded data
      write_nc(to_dp, to_grid, vinfo, vname.c_str());

      NcVar to_var = get_nc_var(nc_out, vname.c_str());
      NcVar var_data = get_goes_nc_var(nc_in, vinfo->name());
      if(IS_VALID_NC(var_data)) {
         for (int idx=0; idx<global_attr_count; idx++) {
            copy_nc_att(nc_in, &to_var, (string)GOES_global_attr_names[idx]);
         }
         copy_nc_atts(&var_data, &to_var, opt_all_attrs);
      }

      bool has_prob_thresh = !prob_cat_thresh.check(bad_data_double);
      if (has_prob_thresh || do_gaussian_filter) {
         DataPlane prob_dp, prob_mask_dp;
         ConcatString vname_prob = vname;
         vname_prob << "_prob_" << prob_cat_thresh.get_abbr_str();
         int nx = to_dp.nx();
         int ny = to_dp.ny();
         prob_dp.set_size(nx, ny);
         prob_dp.set_init(to_dp.init());
         prob_dp.set_valid(to_dp.valid());
         prob_dp.set_constant(0);
         for (int x=0; x<nx; x++) {
            for (int y=0; y<ny; y++) {
               float value = to_dp.get(x, y);
               if (!is_eq(value, bad_data_float) &&
                     ((has_prob_thresh && prob_cat_thresh.check(value))
                       || (do_gaussian_filter && !has_prob_thresh))) {
                  prob_dp.set(1, x, y);
               }
            }
         }

         if (do_gaussian_filter) interp_gaussian_dp(prob_dp, RGInfo.gaussian, RGInfo.vld_thresh);
         write_nc(prob_dp, to_grid, vinfo, vname_prob.c_str());
         if(IS_VALID_NC(var_data)) {
            NcVar prob_var = get_nc_var(nc_out, vname.c_str());
            for (int idx=0; idx<global_attr_count; idx++) {
               copy_nc_att(nc_in, &prob_var, (string)GOES_global_attr_names[idx]);
            }
            copy_nc_atts(&var_data, &prob_var, opt_all_attrs);
            if (do_gaussian_filter) {
               NcVar prob_var = get_var(nc_out, vname_prob.c_str());
               if (IS_VALID_NC(prob_var)) {
                  add_att(&prob_var, "gaussian_radius", RGInfo.gaussian.radius);
                  add_att(&prob_var, "gaussian_dx", RGInfo.gaussian.dx);
                  add_att(&prob_var, "trunc_factor", RGInfo.gaussian.trunc_factor);
               }
            }
         }
      }

   } // end for i

   for (multimap<string,NcVar>::iterator itVar = mapVar.begin();
         itVar != mapVar.end(); ++itVar) {
      if ((*itVar).first == "t"
            || string::npos != (*itVar).first.find("time")) {
         NcVar from_var = (*itVar).second;
         copy_nc_var(nc_out, &from_var);
      }
   }
   //copy_nc_atts(_nc_in, nc_out, opt_all_attrs);

   delete nc_adp; nc_adp = 0;
   delete [] cellMapping;   cellMapping = (IntArray *)0;

   return;
}

////////////////////////////////////////////////////////////////////////

void check_lat_lon(int data_size, float  *latitudes, float  *longitudes) {
   int cnt_printed = 0;
   int cnt_missing_lat = 0;
   int cnt_missing_lon = 0;
   int cnt_bad_lat = 0;
   int cnt_bad_lon = 0;
   float min_lat=90.0;
   float max_lat=-90.0;
   float min_lon=360.0;
   float max_lon=-360.0;
   for (int idx=0; idx<data_size; idx++) {
      if (cnt_printed < 10 && latitudes[idx] > MISSING_LATLON
            && longitudes[idx] > MISSING_LATLON) {
         mlog << Debug(7) << "  index: " << idx <<  " lat: " << latitudes[idx]
                          << ", lon: " << longitudes[idx] << "\n";
         cnt_printed++;
      }
      if (latitudes[idx] <= MISSING_LATLON) cnt_missing_lat++;
      else if (latitudes[idx] < -90 || latitudes[idx] > 90)  cnt_bad_lat++;
      else {
         if (min_lat > latitudes[idx]) min_lat = latitudes[idx];
         if (max_lat < latitudes[idx]) max_lat = latitudes[idx];
      }
      if (longitudes[idx] <= MISSING_LATLON) cnt_missing_lon++;
      else if (longitudes[idx] < -180 || longitudes[idx] > 180) cnt_bad_lon++;
      else {
         if (min_lon > longitudes[idx]) min_lon = longitudes[idx];
         if (max_lon < longitudes[idx]) max_lon = longitudes[idx];
      }
   }
   mlog << Debug(7) << "\n Count: missing - lat: " << cnt_missing_lat
        << ", lon: " << cnt_missing_lon << "\n"
        << "        invalid - lat: " << cnt_bad_lat << ", lon: " << cnt_bad_lon << "\n"
        << "     LAT min: " << min_lat << ", max: " << max_lat << "\n"
        << "    LONG min: " << min_lon << ", max: " << max_lon << "\n";
}

////////////////////////////////////////////////////////////////////////

static bool get_grid_mapping(Grid to_grid, IntArray *cellMapping,
                             const IntArray obs_index_array, const int *obs_hids,
                             const float *hdr_lats, const float *hdr_lons) {
   bool status = false;
   static const char *method_name = "get_grid_mapping() ";

   int obs_count = obs_index_array.n();
   if (0 == obs_count) {
      mlog << Warning << method_name << " no valid point observation data\n";
      return status;
   }

   int hdr_idx, obs_idx;
   int count_in_grid;
   double x, y;
   float  lat, lon;
   DataPlane to_dp;
   int to_offset, idx_x, idx_y;
   int to_lat_count = to_grid.ny();
   int to_lon_count = to_grid.nx();

   count_in_grid = 0;
   to_dp.set_size(to_lon_count, to_lat_count);
   for (int idx=0; idx<obs_count; idx++) {
      obs_idx = obs_index_array[idx];
      hdr_idx = obs_hids[obs_idx];
      lat = hdr_lats[hdr_idx];
      lon = hdr_lons[hdr_idx];
      to_grid.latlon_to_xy(lat, -1.0*lon, x, y);
      idx_x = nint(x);
      idx_y = nint(y);
      if (0 <= idx_x && idx_x < to_lon_count && 0 <= idx_y && idx_y < to_lat_count) {
         to_offset = to_dp.two_to_one(idx_x, idx_y);
         cellMapping[to_offset].add(obs_idx);
         count_in_grid++;
      }
   }

   if (0 == count_in_grid)
      mlog << Warning << method_name << " no valid point observation data within to grid\n";
   else {
      status = true;
      mlog << Debug(3) << method_name << " count in grid: " << count_in_grid
           << " out of " << obs_count << " ("
           << ((obs_count > 0) ? count_in_grid*100/obs_count : 0) << "%)\n";
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

unixtime find_valid_time(multimap<string,NcVar> mapVar) {
   NcVar from_var;
   unixtime valid_time;
   ConcatString time_unit, tmp_time_unit;
   double time_values [100];
   static const char *method_name = "find_valid_time() ";

   valid_time = 0;
   time_values[0] = 0;
   for (multimap<string,NcVar>::iterator itVar = mapVar.begin();
         itVar != mapVar.end(); ++itVar) {
      if ((*itVar).first == "t" || (*itVar).first == "time") {
         from_var = (*itVar).second;
         get_nc_data(&from_var, time_values);
             get_nc_att(&from_var, (string)"units", time_unit);
         valid_time = get_reference_unixtime(time_unit);
         valid_time += (unixtime)nint(time_values[0]);
         mlog << Debug(2) << method_name << "valid time: " << time_values[0]
              << " (" << (int)time_values[0] << ") " << time_unit << " ==> "
              << unix_to_yyyymmdd_hhmmss(valid_time) << "\n";
         break;
      }
   }

   if (valid_time == 0) {
      mlog << Error << "\n" << method_name << "-> "
           << "trouble finding time variable from \""
           << InputFilename << "\"\n\n";
      exit(1);
   }

   return valid_time;
}

////////////////////////////////////////////////////////////////////////

ConcatString get_goes_grid_input(MetConfig config, Grid fr_grid, Grid to_grid) {
   ConcatString run_string;
   ConcatString env_coord_name;
   ConcatString tmp_dir = config.get_tmp_dir();
   ConcatString geostationary_file(tmp_dir);
   ConcatString grid_map_file(tmp_dir);
   grid_map_file.add("/");
   grid_map_file.add(make_geostationary_filename(fr_grid, to_grid, RGInfo.name));
   geostationary_file.add("/");
   geostationary_file.add(make_geostationary_filename(fr_grid, to_grid, RGInfo.name, false));
   if (file_exists(grid_map_file.c_str())) {
      run_string << grid_map_file;
   }
   else if (get_env(key_geostationary_data, env_coord_name) &&
            env_coord_name.nonempty() &&
            file_exists(env_coord_name.c_str())) {
      run_string << env_coord_name;
   }
   else if (file_exists(geostationary_file.c_str())) {
      run_string << geostationary_file;
   }

   return run_string;
}

////////////////////////////////////////////////////////////////////////

void get_grid_mapping(Grid fr_grid, Grid to_grid, IntArray *cellMapping,
                      ConcatString geostationary_file) {
   static const char *method_name = "get_grid_mapping() ";
   DataPlane from_dp, to_dp;
   ConcatString cur_coord_name;

   int to_lat_count = to_grid.ny();
   int to_lon_count = to_grid.nx();
   int from_lat_count = fr_grid.ny();;
   int from_lon_count = fr_grid.nx();

   bool has_coord_input = false;
   ConcatString tmp_coord_name;

   if (get_env(key_geostationary_data, tmp_coord_name) &&
       tmp_coord_name.nonempty() &&
       file_exists(tmp_coord_name.c_str())) {
      has_coord_input = true;
      cur_coord_name = tmp_coord_name;
   }
   else {
      if (file_exists(geostationary_file.c_str())) {
         has_coord_input = true;
         cur_coord_name = geostationary_file;
      }
   }

   // Override the from nx & ny from NetCDF if exists
   NcFile *coord_nc_in = (NcFile *)0;
   if (has_coord_input) {
      mlog << Debug(2)  << method_name << " Reading coord file: " << cur_coord_name << "\n";
      coord_nc_in = open_ncfile(cur_coord_name.c_str());
      if (IS_VALID_NC_P(coord_nc_in)) {
         from_lat_count = get_lat_count(coord_nc_in);
         from_lon_count = get_lon_count(coord_nc_in);
      }
   }
   int data_size  = from_lat_count * from_lon_count;
   mlog << Debug(4) << method_name << " data_size (ny*nx): " << data_size
        << " = " << from_lat_count << " * " << from_lon_count << "\n"
        << "                    target grid (nx,ny)="
        << to_lon_count << "," << to_lat_count << "\n";

   from_dp.set_size(from_lon_count, from_lat_count);
   to_dp.set_size(to_lon_count, to_lat_count);

   if (data_size > 0) {
      double x, y;
      float  lat, lon;
      int    idx_x, idx_y;
      int    coord_offset, to_offset;
      int    count_in_grid;
      float  *latitudes  = (float *)NULL;
      float  *longitudes = (float *)NULL;
      float  *latitudes_buf  = (float *)NULL;
      float  *longitudes_buf = (float *)NULL;
      int buff_size = data_size*sizeof(float);
      GoesImagerData grid_data;
      grid_data.reset();

      if (has_coord_input) {
         latitudes_buf  = new float[data_size];
         longitudes_buf = new float[data_size];

         latitudes = latitudes_buf;
         longitudes = longitudes_buf;
         memset(latitudes,  0, buff_size);
         memset(longitudes, 0, buff_size);

         if (IS_VALID_NC_P(coord_nc_in)) {
            NcVar var_lat = get_nc_var(coord_nc_in, var_name_lat);
            NcVar var_lon = get_nc_var(coord_nc_in, var_name_lon);
            if (IS_VALID_NC(var_lat) && IS_VALID_NC(var_lon)) {
               get_nc_data(&var_lat, latitudes);
               get_nc_data(&var_lon, longitudes);
            }
         }
         else {
            FILE *pFile = met_fopen ( cur_coord_name.c_str(), "rb" );
            (void) fread (latitudes,sizeof(latitudes[0]),data_size,pFile);
            (void) fread (longitudes,sizeof(longitudes[0]),data_size,pFile);
            fclose (pFile);

            bool compare_binary_and_computation = false;
            if (compare_binary_and_computation && fr_grid.info().gi) {
               grid_data.copy(fr_grid.info().gi);
               grid_data.compute_lat_lon();
               grid_data.test();

               int lat_matching_count = 0;
               int lat_mis_matching_count = 0;
               int lon_matching_count = 0;
               int lon_mis_matching_count = 0;
               float *tmp_lats  = grid_data.lat_values;
               float *tmp_lons  = grid_data.lon_values;

               for (int idx=0; idx<data_size; idx++) {
                   if ((latitudes[idx] > MISSING_LATLON) && (tmp_lats[idx] > MISSING_LATLON)) {
                      if (!is_eq(latitudes[idx], tmp_lats[idx], loose_tol)) {
                         lat_mis_matching_count++;
                         mlog << Warning << method_name << "diff lat at " << idx
                              << "  binary-computing: " << latitudes[idx] << " - "
                              << tmp_lats[idx] << " = " << (latitudes[idx]-tmp_lats[idx]) << "\n";
                      }
                      else lat_matching_count++;
                   }
                   else lat_matching_count++;
                   if ((longitudes[idx] > MISSING_LATLON) && (tmp_lons[idx] > MISSING_LATLON)) {
                      if (!is_eq(longitudes[idx], tmp_lons[idx], loose_tol)) {
                         lon_mis_matching_count++;
                         mlog << Warning << method_name << "diff lon at " << idx
                              << "  binary-computing: " << longitudes[idx] << " - "
                              << tmp_lons[idx] << " = " << (longitudes[idx]-tmp_lons[idx]) << "\n";
                      }
                      else lon_matching_count++;
                   }
                   else lon_matching_count++;
               }
               if ((lon_mis_matching_count > 0) || (lat_mis_matching_count > 0)) {
                  mlog << Warning << method_name << "mis-matching: lon = " << lon_mis_matching_count
                       << "  lat =  " << lat_mis_matching_count << "   matched:  lon = "
                       << lon_matching_count << "  lat =  " << lat_matching_count << "\n";
               }
            }
         }
      }
      else {
         if (fr_grid.info().gi) {
            grid_data.copy(fr_grid.info().gi);
            grid_data.compute_lat_lon();
            latitudes = grid_data.lat_values;
            longitudes = grid_data.lon_values;
            if (!file_exists(geostationary_file.c_str())) {
               save_geostationary_data(geostationary_file,
                     latitudes, longitudes, grid_data);
            }
         }
      }
      if (latitudes && longitudes) {
         check_lat_lon(data_size, latitudes, longitudes);

         count_in_grid = 0;

         //Following the logic at DataPlane::two_to_one(int x, int y) n = y*Nx + x;
         for (int xIdx=0; xIdx<from_lat_count; xIdx++) {
            for (int yIdx=0; yIdx<from_lon_count; yIdx++) {
               coord_offset = from_dp.two_to_one(yIdx, xIdx);
               lat = latitudes[coord_offset];
               lon = longitudes[coord_offset];
               to_grid.latlon_to_xy(lat, -1.0*lon, x, y);
               idx_x = nint(x);
               idx_y = nint(y);

               if (0 <= idx_x && idx_x < to_lon_count && 0 <= idx_y && idx_y < to_lat_count) {
                  to_offset = to_dp.two_to_one(idx_x, idx_y);
                  cellMapping[to_offset].add(coord_offset);
                  count_in_grid++;
               }
            }
         }
         mlog << Debug(3) << method_name << " within grid: " << count_in_grid
              << " out of " << data_size << " (" << count_in_grid*100/data_size << "%)\n";
      }
      else {
         if (0 == latitudes)
            mlog << Error << method_name << " Fail to get latitude\n";
         if (0 == longitudes)
            mlog << Error << method_name << " Fail to get longitudes\n";
      }

      if (latitudes_buf)  { delete [] latitudes_buf;   latitudes_buf  = NULL; }
      if (longitudes_buf) { delete [] longitudes_buf;  longitudes_buf = NULL; }

      grid_data.release();

   }   //  if data_size > 0


   if(coord_nc_in) {
      delete coord_nc_in;  coord_nc_in = 0;
   }
}

////////////////////////////////////////////////////////////////////////

int get_lat_count(NcFile *_nc) {
   int lat_count = 0;
   NcDim dim_lat = get_nc_dim(_nc, dim_name_lat);
   if(IS_INVALID_NC(dim_lat)) dim_lat = get_nc_dim(_nc, "y");
   if(IS_VALID_NC(dim_lat)) lat_count= get_dim_size(&dim_lat);
   return lat_count;
}

int get_lon_count(NcFile *_nc) {
   int lon_count = 0;
   NcDim dim_lon = get_nc_dim(_nc, dim_name_lon);
   if(IS_INVALID_NC(dim_lon)) dim_lon = get_nc_dim(_nc, "x");
   if(IS_VALID_NC(dim_lon)) lon_count= get_dim_size(&dim_lon);
   return lon_count;
}

////////////////////////////////////////////////////////////////////////

static NcVar get_goes_nc_var(NcFile *nc, const ConcatString var_name,
                             bool exit_if_error) {
   NcVar var_data = get_nc_var(nc, var_name.c_str(), false, false);
   if (IS_INVALID_NC(var_data)) {
       var_data = get_nc_var(nc, var_name.split("_")[0].c_str());
   }
   if (IS_INVALID_NC(var_data)) {
      mlog << Error << "The variable \"" << var_name << "\" does not exist\n";
      if (exit_if_error) exit(1);
   }
   return var_data;
}

////////////////////////////////////////////////////////////////////////


static ConcatString make_geostationary_filename(Grid fr_grid, Grid to_grid,
      ConcatString regrid_name, bool grid_map) {
   ConcatString geo_data_filename;
   GridInfo info = fr_grid.info();

   if (info.gi) {
      size_t offset;
      string scene_id = info.gi->scene_id;
      offset = scene_id.find(' ');
      if (offset != string::npos) {
         geo_data_filename << scene_id.substr(0, offset).c_str() << "_";
      }
      else {
         geo_data_filename << scene_id.c_str() << "_";
      }
   }
   geo_data_filename << fr_grid.nx() << "_" << fr_grid.ny();
   if (info.gi) {
      geo_data_filename << "_" << int(info.gi->dx_rad * factor_float_to_int)
            << "_" << int(info.gi->dy_rad * factor_float_to_int)
            << "_" << int(info.gi->x_image_bounds[0] * factor_float_to_int)
            << "_" << int(info.gi->y_image_bounds[0] * factor_float_to_int);
   }
   if (grid_map) {
      geo_data_filename << "_to_";
      if (file_exists(regrid_name.c_str())) {
          StringArray file_names = regrid_name.split("\\/");
          geo_data_filename << file_names[file_names.n()-1];
      }
      else geo_data_filename << to_grid.name();
      geo_data_filename << ".grid_map";
   }
   else {
      geo_data_filename << ".nc";
   }
   return geo_data_filename;
}

////////////////////////////////////////////////////////////////////////

static bool is_time_mismatch(NcFile *nc_in, NcFile *nc_adp) {
   // Verify the coverage start and end times
   bool time_mismatch = false;
   ConcatString aod_start_time, aod_end_time;
   ConcatString adp_start_time, adp_end_time;
   get_att_value_string(nc_in,  (string)"time_coverage_start", aod_start_time);
   get_att_value_string(nc_in,  (string)"time_coverage_end",   aod_end_time  );
   get_att_value_string(nc_adp, (string)"time_coverage_start", adp_start_time);
   get_att_value_string(nc_adp, (string)"time_coverage_end",   adp_end_time  );

   ConcatString log_message;
   if (aod_start_time != adp_start_time) {
      time_mismatch = true;
      log_message << "\tThe start time: "
                  << aod_start_time << " != " << adp_start_time << "\n";
   }
   if (aod_end_time != adp_end_time) {
      time_mismatch = true;
      log_message << "\tThe end time: "
                  << aod_end_time << " != " << adp_end_time << "\n";
   }
   if (time_mismatch) {
      mlog << Error << "The ADP input does not match with AOD input\n" << log_message;
   }
   return time_mismatch;
}

////////////////////////////////////////////////////////////////////////


IntArray *read_grid_mapping(const char *grid_map_file) {
   static const char *method_name = "read_grid_mapping() ";

   //if ( ! file_exists(filename) )  {
   int nx, ny, map_size;
   IntArray *cellMapping = (IntArray *)0;

   nx = ny = map_size = 0;
   string line;
   ifstream map_file (grid_map_file);
   if (map_file.is_open()) {
      bool map_data;
      int to_offset, coord_offset, map_count;
      StringArray str_arr;
      StringArray cell_index_arr;

      map_count = 0;
      map_data = false;
      while ( getline (map_file, line) ) {
         if (0 == line.compare("[mapping]")) {
            map_data = true;
            continue;
         }
         if (0 == line.compare("[metadata]")) {
            map_data = false;
            continue;
         }

         str_arr.clear();
         str_arr.parse_delim(line.c_str(), "=");
         if (str_arr.n() == 2) {
            if (map_data) {
               cell_index_arr.clear();
               to_offset = atoi(str_arr[0].c_str());
               if ((to_offset >= 0) && (to_offset < map_size)) {
                  cell_index_arr.parse_delim(str_arr[1], ",");
                  if (0 < cell_index_arr.n()) map_count++;
                  for (int idx=0; idx<cell_index_arr.n(); idx++) {
                     coord_offset = atoi(cell_index_arr[idx].c_str());
                     if (coord_offset >= 0) {
                        cellMapping[to_offset].add(coord_offset);
                     }
                  }
               }
               else {
                  mlog << Warning << method_name << "The offset "
                       << to_offset  << "is out of range\n";
               }
            }
            else if ( strcasecmp(str_arr[0].c_str(), "nx") == 0 )  {
               nx = atoi(str_arr[1].c_str());
               if ((nx > 0) && (ny > 0)) {
                  if (cellMapping == 0) {
                     map_size = nx * ny;
                     cellMapping = new IntArray[map_size];
                  }
                  else {
                     mlog << Warning << method_name << "Ignored " << line << "\n";
                  }
               }
            }
            else if ( strcasecmp(str_arr[0].c_str(), "ny") == 0 )  {
               ny = atoi(str_arr[1].c_str());
               if ((nx > 0) && (ny > 0)) {
                  if (cellMapping == 0) {
                     map_size = nx * ny;
                     cellMapping = new IntArray[map_size];
                  }
                  else {
                     mlog << Warning << method_name << "Ignored " << line << "\n";
                  }
               }
            }
            else {
               mlog << Debug(7) << method_name << "Not using " << line << "\n";
            }
         }
      }
      map_file.close();
      mlog << Debug(7) << method_name << "Read " << map_count
           << " mappings (out of " << map_size << ")\n";
   }
   else {
      mlog << Error << method_name << "Unable to open file" << grid_map_file << "\n";
   }
   return cellMapping;
}

////////////////////////////////////////////////////////////////////////

void regrid_goes_variable(NcFile *nc_in, VarInfo *vinfo,
      DataPlane &fr_dp, DataPlane &to_dp,
      Grid fr_grid, Grid to_grid, IntArray *cellMapping, NcFile *nc_adp) {

   bool has_qc_var = false;
   bool has_adp_qc_var = false;
   int to_lat_count = to_grid.ny();
   int to_lon_count = to_grid.nx();
   int from_lat_count = fr_grid.ny();
   int from_lon_count = fr_grid.nx();
   int from_data_size = from_lat_count * from_lon_count;
   ConcatString goes_var_name;
   ConcatString goes_var_sub_name;
   ConcatString qc_var_name;
   ncbyte qc_value;
   ncbyte *qc_data = new ncbyte[from_data_size];
   uchar *adp_data = new uchar[from_data_size];
   float *from_data = new float[from_data_size];
   unsigned short *adp_qc_data = new unsigned short[from_data_size];
   static const char *method_name = "regrid_goes_variable() ";

   // -99 is arbitrary number as invalid QC value
   memset(qc_data, -99, from_data_size*sizeof(ncbyte));

   NcVar var_qc;
   NcVar var_adp;
   NcVar var_adp_qc;
   NcVar var_data = get_goes_nc_var(nc_in, vinfo->name());
   //if (IS_INVALID_NC(var_data)) {
   //   mlog << Error << "The variable \"" << vinfo->name() << "\" does not exist\n";
   //   exit(1);
   //}
   bool is_dust_only = false;
   bool is_smoke_only = false;
   string actual_var_name = GET_NC_NAME(var_data);
   int  actual_var_len = actual_var_name.length();
   bool is_adp_variable = (0 != actual_var_name.compare(vinfo->name().c_str()));

   memset(adp_data, 1, from_data_size*sizeof(uchar));   // Default: 1 = data present
   memset(adp_qc_data, 65535, from_data_size*sizeof(unsigned short));
   if (is_adp_variable && IS_VALID_NC_P(nc_adp)) {
      is_dust_only  = (0 == vinfo->name().comparecase((actual_var_len + 1),
            vname_dust.length(), vname_dust.c_str()));
      is_smoke_only = (0 == vinfo->name().comparecase((actual_var_len + 1),
            vname_smoke.length(), vname_smoke.c_str()));

      if      (is_dust_only)    var_adp = get_goes_nc_var(nc_adp, vname_dust);
      else if (is_smoke_only)   var_adp = get_goes_nc_var(nc_adp, vname_smoke);

      if (IS_VALID_NC(var_adp)) {
         get_nc_data(&var_adp, adp_data);

         //Smoke:ancillary_variables = "DQF" ; ubyte DQF(y, x) ;
         if (get_att_value_string(&var_adp, (string)"ancillary_variables", qc_var_name)) {
            var_adp_qc = get_nc_var(nc_adp, qc_var_name.c_str());
            if (IS_VALID_NC(var_adp_qc)) {
               get_nc_data(&var_adp_qc, adp_qc_data);
               has_adp_qc_var = true;
               mlog << Debug(5) << method_name << "found QC var: " << qc_var_name
                    << " for " << GET_NC_NAME(var_adp) << ".\n";
            }
         }
         else mlog << Warning << method_name << "found QC var name (" << qc_var_name
                   << " for " << GET_NC_NAME(var_adp) << ") but does not exist.\n";
      }
   }
   mlog << Debug(5) << method_name << " is_dust: " << is_dust_only
        << ", is_smoke: " << is_smoke_only << "\n";

   //AOD:ancillary_variables = "DQF" ; byte DQF(y, x) ;
   if (get_att_value_string(&var_data, (string)"ancillary_variables", qc_var_name)) {
      var_qc = get_nc_var(nc_in, qc_var_name.c_str());
      if (IS_VALID_NC(var_qc)) {
         get_nc_data(&var_qc, qc_data);
         has_qc_var = true;
         mlog << Debug(3) << method_name << "found QC var: " << qc_var_name << ".\n";
      }
      else mlog << Warning << method_name << "found QC var name ("
                << qc_var_name << ") but does not exist.\n";
   }

   get_nc_data(&var_data, (float *)from_data);

   fr_dp.set_size(from_lon_count, from_lat_count);
   for (int xIdx=0; xIdx<from_lon_count; xIdx++) {
      for (int yIdx=0; yIdx<from_lat_count; yIdx++) {
         int offset = fr_dp.two_to_one(xIdx,yIdx);
         fr_dp.set(from_data[offset],xIdx,yIdx);
      }
   }

   int from_index;
   int absent_count = 0;
   int censored_count = 0;
   int missing_count = 0;
   int to_cell_count = 0;
   int non_missing_count = 0;
   int qc_filtered_count = 0;
   int adp_qc_filtered_count = 0;
   // int global_attr_count;
   float data_value;
   float from_min_value =  10e10;
   float from_max_value = -10e10;
   float qc_min_value =  10e10;
   float qc_max_value = -10e10;
   IntArray cellArray;
   NumArray dataArray;
   int particle_qc;
   bool has_qc_flags = (qc_flags.n() > 0);

   missing_count = non_missing_count = 0;
   to_dp.set_constant(bad_data_double);

   for (int xIdx=0; xIdx<to_lon_count; xIdx++) {
      for (int yIdx=0; yIdx<to_lat_count; yIdx++) {
         int offset = to_dp.two_to_one(xIdx,yIdx);
         cellArray = cellMapping[offset];
         if (0 < cellArray.n()) {
            int valid_count = 0;
            dataArray.clear();
            for (int dIdx=0; dIdx<cellArray.n(); dIdx++) {
               from_index = cellArray[dIdx];
               data_value = from_data[from_index];
               if (is_eq(data_value, bad_data_float)) {
                  missing_count++;
                  continue;
               }

               non_missing_count++;
               if(mlog.verbosity_level() >= 4) {
                  if (from_min_value > data_value) from_min_value = data_value;
                  if (from_max_value < data_value) from_max_value = data_value;
               }

               //Filter by QC flag
               qc_value = qc_data[from_index];
               if (!has_qc_var || !has_qc_flags || qc_flags.has(qc_value)) {
                  for(int i=0; i<vinfo->censor_thresh().n(); i++) {
                     // Break out after the first match.
                     if(vinfo->censor_thresh()[i].check(data_value)) {
                        data_value = vinfo->censor_val()[i];
                        censored_count++;
                        break;
                     }
                  }
                  if (0 == adp_data[from_index]) {
                     absent_count++;
                     continue;
                  }

                  if (has_adp_qc_var && has_qc_flags) {
                     int shift_bits = 2;
                     if (is_dust_only) shift_bits += 2;
                     particle_qc = ((adp_qc_data[from_index] >> shift_bits) & 0x03);
                     int qc_for_flag = 3 - particle_qc; // high = 3, qc_flag for high = 0
                     if (!qc_flags.has(qc_for_flag)) {
                        adp_qc_filtered_count++;
                        continue;
                     }
                  }

                  dataArray.add(data_value);
                  if (mlog.verbosity_level() >= 4) {
                     if (qc_min_value > qc_value) qc_min_value = qc_value;
                     if (qc_max_value < qc_value) qc_max_value = qc_value;
                  }
               }
               else {
                  qc_filtered_count++;
               }
               valid_count++;
            }

            if (0 < dataArray.n()) {
               int data_count = dataArray.n();
               float to_value;
               if      (RGInfo.method == InterpMthd_Min) to_value = dataArray.min();
               else if (RGInfo.method == InterpMthd_Max) to_value = dataArray.max();
               else if (RGInfo.method == InterpMthd_Median) {
                  cellArray.sort_increasing();
                  to_value = dataArray[data_count/2];
                  if (0 == data_count % 2)
                     to_value = (to_value + dataArray[(data_count/2)+1])/2;
               }
               else to_value = dataArray.sum() / data_count;    // UW_Mean

               to_dp.set(to_value, xIdx, yIdx);
               to_cell_count++;
               mlog << Debug(9) << method_name
                    <<   "max: " << dataArray.max()
                    << ", min: " << dataArray.min()
                    << ", mean: " << dataArray.sum()/data_count
                    << " from " << valid_count << " out of "
                    << data_count << " data values.\n";
            }
         }
         else {}
      }
   }

   delete [] qc_data;
   delete [] adp_data;
   delete [] from_data;
   delete [] adp_qc_data;

   mlog << Debug(4) << method_name << " Count: actual: " << to_cell_count
        << ", missing: " << missing_count << ", non_missing: " << non_missing_count
        << "\n\tFiltered: by QC: " << qc_filtered_count
        << ", by adp QC: " << adp_qc_filtered_count
        << ", by absent: " << absent_count
        << ", total: " << (qc_filtered_count + adp_qc_filtered_count + absent_count)
        << "\n\tRange:  data: [" << from_min_value << " - " << from_max_value
        << "]  QC: [" << qc_min_value << " - " << qc_max_value << "]\n";

   if (to_cell_count == 0)
      mlog << Warning << method_name << " No valid data\n";
}

////////////////////////////////////////////////////////////////////////

static void save_geostationary_data(const ConcatString geostationary_file,
      const float *latitudes, const float *longitudes,
      const GoesImagerData grid_data) {
   bool has_error = false;
   int deflate_level = 0;
   static const char *method_name = "save_geostationary_data() ";

   NcFile *nc_file = open_ncfile(geostationary_file.text(), true);
   NcDim xdim = add_dim(nc_file, dim_name_lon, grid_data.nx);
   NcDim ydim = add_dim(nc_file, dim_name_lat, grid_data.ny);

   NcVar lat_var = add_var(nc_file, var_name_lat, ncFloat, ydim, xdim, deflate_level);
   NcVar lon_var = add_var(nc_file, var_name_lon, ncFloat, ydim, xdim, deflate_level);

   if (IS_VALID_NC(lat_var)) {
      if (grid_data.dy_rad >= 0) {
         add_att(&lat_var, "standard_name", var_name_lat);
         add_att(&lat_var, "units","degrees_north");
      }
      else {
         add_att(&lat_var, "long_name", var_name_lat);
         add_att(&lat_var, "units","degrees_south");
      }
      add_att(&lat_var, "dy_rad", grid_data.dy_rad);
      if(!put_nc_data((NcVar *)&lat_var, latitudes)) {
         has_error = true;
         mlog << Error << "Can not save latitudes\n";
      }
   }
   if (IS_VALID_NC(lon_var)) {
      if (grid_data.dy_rad >= 0) {
         add_att(&lon_var, "standard_name", var_name_lon);
         add_att(&lon_var, "units","degrees_east");
      }
      else {
         add_att(&lon_var, "long_name", var_name_lon);
         add_att(&lon_var, "units","degrees_west");
      }
      add_att(&lon_var, "dx_rad", grid_data.dx_rad);
      if(!put_nc_data((NcVar *)&lon_var, longitudes)) {
         has_error = true;
         mlog << Error << "Can not save longitudes\n";
      }
   }

   add_att(nc_file, "Conventions", "CF-1.6");

   if (has_error) {
      remove(geostationary_file.c_str());
      mlog << Warning << "The geostationary data file ("
           << geostationary_file << ") was not saved!\n";
   }
   else {
     mlog << Debug(3) << method_name << "The geostationary data file ("
          << geostationary_file << ") was saved\n";
   }

   delete nc_file;  nc_file = 0;

}

////////////////////////////////////////////////////////////////////////

void write_grid_mapping(const char *grid_map_file,
      IntArray *cellMapping, Grid from_grid, Grid to_grid) {
   static const char *method_name = "write_grid_mapping() ";
   int nx = to_grid.nx();
   int ny = to_grid.ny();

   if ( file_exists(grid_map_file) )  {
      mlog << Warning << method_name << grid_map_file << "Exist already\n";
   }
   else {
      ofstream map_file (grid_map_file);
      if (map_file.is_open()) {
         int map_count = 0;
         int map_size = nx * ny;
         GridInfo info = from_grid.info();
         map_file << "[metadata]\n";
         map_file << "nx=" << nx << "\n";
         map_file << "ny=" << ny << "\n";
         map_file << "from_nx=" << from_grid.nx() << "\n";
         map_file << "from_ny=" << from_grid.ny() << "\n";
         if (info.gi) {
            map_file << "from_dx=" << info.gi->dx_rad << "\n";
            map_file << "from_dy=" << info.gi->dy_rad << "\n";
         }
         map_file << "[mapping]\n";
         for (int idx=0; idx<map_size; idx++) {
            int mem_count = cellMapping[idx].n();
            map_file << idx << "=";
            if (mem_count > 0) {
               for (int idx2=0; idx2<mem_count; idx2++) {
                  map_file << cellMapping[idx][idx2];
                  if (idx2 < (mem_count-1)) map_file << ",";
               }
               map_count++;
            }
            map_file << "\n";
         }
         mlog << Debug(5) << method_name << "Write "
              << map_count << " grid mappings (out of " << map_size << ")\n";
      }
      else {
         mlog << Error << method_name << "Unable to open file "
              << grid_map_file << "\n";
      }
      map_file.close();
   }
   return;
}


////////////////////////////////////////////////////////////////////////

void close_nc() {

   // Clean up
   if(nc_out) {
      delete nc_out; nc_out = (NcFile *) 0;
   }

   // List the output file
   mlog << Debug(1)
        << "Writing output file: " << OutputFilename << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

//bool keep_message_type(const char *mt_str) {
bool keep_message_type(const int mt_index) {

   bool keep = false;

   keep = message_type_list.n() == 0 ||
          message_type_list.has(mt_index);

   return(keep);
}

////////////////////////////////////////////////////////////////////////

void usage() {

   //GridTemplateFactory gtf;
   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tinput_filename\n"
        << "\tto_grid\n"
        << "\toutput_filename\n"
        << "\t-field string\n"
        << "\t[-config file]\n"
        << "\t[-qc flags]\n"
        << "\t[-method type]\n"
        << "\t[-gaussian_dx n]\n"
        << "\t[-gaussian_radius n]\n"
        << "\t[-prob_cat_thresh string]\n"
        << "\t[-vld_thresh n]\n"
        << "\t[-name list]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"input_filename\" is the gridded data file to be "
        << "read (required).\n"

        << "\t\t\"to_grid\" defines the output grid as a named grid, the "
        << "path to a gridded data file, or an explicit grid "
        << "specification string (required).\n"

        << "\t\t\"output_filename\" is the output NetCDF file to be "
        << "written (required).\n"

        << "\t\t\"-field string\" may be used multiple times to define "
        << "the data to be regridded (required).\n"

        << "\t\t\"-config file\" uses the specified configuration file "
        << "to generate gridded data (optional).\n"

        << "\t\t\"-qc flags\" specifies a comma-separated list of QC flags, for example \"0,1\" (optional).\n"
        << "\t\t\tOnly applied if grid_mapping is set to \"goes_imager_projection\" and the QC variable exists.\n"

        << "\t\t\"-adp adp_file_name\" specifies a ADP data input for AOD dataset (ignored if the input is not AOD from GOES16/17).\n"

        << "\t\t\"-method type\" overrides the default regridding "
        << "method (default: " << interpmthd_to_string(RGInfo.method)
        << ", optional) to -field variable. Additional gaussian smoothing only to the probabililty variable"
        << " with additional \"-method GAUSSIAN\" or \"-method MAXGAUSS\".\n"

        << "\t\t\"-gaussian_dx n\" specifies a delta distance for Gaussian smoothing."
        << " The default is " << RGInfo.gaussian.dx << ". Ignored if not Gaussian method (optional).\n"

        << "\t\t\"-gaussian_radius n\" specifies the radius of influence for Gaussian smoothing."
        << " The default is " << RGInfo.gaussian.radius << "). Ignored if not Gaussian method (optional).\n"

        << "\t\t\"-prob_cat_thresh string\" sets the threshold to compute the probability of occurrence (optional).\n"

        << "\t\t\"-vld_thresh n\" overrides the default required "
        << "ratio of valid data for regridding (" << RGInfo.vld_thresh
        << ") (optional).\n"

        << "\t\t\"-name list\" specifies a comma-separated list of "
        << "output variable names for each field specified (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t\"-compress level\" overrides the compression level of NetCDF variable (optional).\n\n" << flush;

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_field(const StringArray &a) {
   FieldSA.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_method(const StringArray &a) {
   InterpMthd method_id = string_to_interpmthd(a[0].c_str());
   if (method_id == InterpMthd_Gaussian || method_id == InterpMthd_MaxGauss ) {
      do_gaussian_filter = true;
      if (method_id == InterpMthd_MaxGauss) RGInfo.method = InterpMthd_Max;
   }
   else RGInfo.method = method_id;
   opt_override_method = true;
}

////////////////////////////////////////////////////////////////////////

void set_prob_cat_thresh(const StringArray &a) {
   prob_cat_thresh.set(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_vld_thresh(const StringArray &a) {
   RGInfo.vld_thresh = atof(a[0].c_str());
   if(RGInfo.vld_thresh > 1 || RGInfo.vld_thresh < 0) {
      mlog << Error << "\nset_vld_thresh() -> "
           << "-vld_thresh may only be set between 0 and 1: "
           << RGInfo.vld_thresh << "\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

void set_name(const StringArray & a) {
   VarNameSA.add_css(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray &a) {
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_config(const StringArray & a) {
   config_filename = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray &a) {
   mlog.set_verbosity_level(atoi(a[0].c_str()));
}

////////////////////////////////////////////////////////////////////////

void set_compress(const StringArray & a) {
   compress_level = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_qc_flags(const StringArray & a) {
   int qc_flag;
   StringArray sa;

   sa.parse_css(a[0]);
   for (int idx=0; idx<sa.n(); idx++) {
      qc_flag = atoi(sa[idx].c_str());
      if ( !qc_flags.has(qc_flag) ) qc_flags.add(qc_flag);
   }
}

////////////////////////////////////////////////////////////////////////

void set_adp(const StringArray & a) {
   AdpFilename = a[0];
   if (!file_exists(AdpFilename.c_str())) {
      mlog << Error << "\nset_adp() -> "
           << "\"" << AdpFilename << "\" does not exist\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

void set_gaussian_dx(const StringArray &a) {
   RGInfo.gaussian.dx = atof(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_gaussian_radius(const StringArray &a) {
   RGInfo.gaussian.radius = atof(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////
