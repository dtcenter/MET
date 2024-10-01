// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
//   001    01-25-21  Halley Gotway  MET #1630 Handle zero obs.
//   002    07-06-22  Howard Soh     METplus-Internal #19 Rename main to met_main
//   003    10-03-23  Prestopnik     MET #2227 Remove namespace std and netCDF from header files
//   004    05-20-24  Howard Soh     MET #2867 Fix -qc ADP bug.
//   004    06-24-24  Halley Gotway  MET #2880 Filter obs_quality.
//
////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <dirent.h>

#include <netcdf>

#include "main.h"
#include "vx_log.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_nc_util.h"
#include "vx_grid.h"
#include "vx_regrid.h"
#include "vx_util.h"
#include "vx_statistics.h"
#include "var_info_nc_cf.h"
#include "nc_obs_util.h"
#include "nc_point_obs_in.h"

#include "point2grid_conf_info.h"

#ifdef WITH_PYTHON
#include "data2d_nc_met.h"
#include "pointdata_python.h"
#endif

using namespace std;
using namespace netCDF;

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

// Constants
constexpr int TYPE_UNKNOWN  = 0;     // Can not process the input file
constexpr int TYPE_OBS      = 1;     // MET Point Obs NetCDF (from xxx2nc)
constexpr int TYPE_NCCF     = 2;     // CF NetCDF with time and lat/lon variables
constexpr int TYPE_GOES     = 5;
constexpr int TYPE_GOES_ADP = 6;
constexpr int TYPE_PYTHON   = 7;     // MET Point Obs NetCDF from PYTHON

constexpr InterpMthd DefaultInterpMthd = InterpMthd::UW_Mean;
constexpr int        DefaultInterpWdth = 2;
constexpr double     DefaultVldThresh  = 0.5;

constexpr float      MISSING_LATLON = -999.0;
constexpr int        QC_NA_INDEX = -1;
constexpr int        LEVEL_FOR_PERFORMANCE = 6;

constexpr char default_config_filename[] = "MET_BASE/config/Point2GridConfig_default";

static const vector<string> GOES_global_attr_names = {
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
static ConcatString adp_filename;
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
static NcFile *nc_out  = (NcFile *) nullptr;
static NcDim  lat_dim ;
static NcDim  lon_dim ;

static int adp_qc_high;     /* 3 as baseline algorithm, 0 for enterprise algorithm */
static int adp_qc_medium;   /* 1 as baseline algorithm, 1 for enterprise algorithm */
static int adp_qc_low;      /* 0 as baseline algorithm, 2 for enterprise algorithm */

enum class GOES_QC {
   HIGH = 0,
   MEDIUM,
   LOW,
   NA
};

static const ConcatString att_name_values = "flag_values";
static const ConcatString att_name_meanings = "flag_meanings";

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_data_file();
static void process_point_file(NcFile *nc_in, MetConfig &config,
                               VarInfo *, const Grid to_grid);
#ifdef WITH_PYTHON
static void process_point_python(const string python_command, MetConfig &config,
                                 VarInfo *vinfo, const Grid to_grid, bool use_xarray);
#endif
static void process_point_nccf_file(NcFile *nc_in, MetConfig &config,
                                    VarInfo *, Met2dDataFile *fr_mtddf,
                                    const Grid to_grid);
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
static void set_config(const StringArray &);
static void set_compress(const StringArray &);
static void set_adp(const StringArray &);
static void set_gaussian_dx(const StringArray &);
static void set_gaussian_radius(const StringArray &);

static unixtime compute_unixtime(NcVar *time_var, unixtime var_value);
void clear_cell_mapping(IntArray *cell_mapping);
static bool get_grid_mapping(const Grid &fr_grid, const Grid &to_grid, IntArray *cellMapping,
                             NcVar var_lat, NcVar var_lon, bool *skip_times);
static bool get_grid_mapping(const Grid &to_grid, IntArray *cellMapping,
                             const IntArray obs_index_array, const int *obs_hids,
                             const float *hdr_lats, const float *hdr_lons);
static int  get_obs_type(NcFile *nc_in);
static void regrid_nc_variable(NcFile *nc_in, Met2dDataFile *fr_mtddf,
                               VarInfo *vinfo, DataPlane &fr_dp, DataPlane &to_dp,
                               Grid to_grid, IntArray *cellMapping);

static bool keep_message_type(const int mt_index);

static bool has_lat_lon_vars(const NcFile *nc_in);
static void set_adp_gc_values(NcVar var_adp_qc);

////////////////////////////////////////////////////////////////////////
// for GOES 16
//
static const int factor_float_to_int = 1000000;
constexpr char key_geostationary_data[] = "MET_GEOSTATIONARY_DATA";
constexpr char dim_name_lat[] = "lat";
constexpr char dim_name_lon[] = "lon";
constexpr char var_name_lat[] = "latitude";
constexpr char var_name_lon[] = "longitude";
static const ConcatString vname_dust("Dust");
static const ConcatString vname_smoke("Smoke");

static IntArray qc_flags;

static void process_goes_file(NcFile *nc_in, MetConfig &config,
            VarInfo *, const Grid fr_grid, const Grid to_grid);
static unixtime find_valid_time(NcVar time_var);
static ConcatString get_goes_grid_input(MetConfig config, const Grid fr_grid);
static void get_grid_mapping(const Grid &fr_grid, const Grid &to_grid,
                             IntArray *cellMapping, const ConcatString &geostationary_file);
static int  get_lat_count(const NcFile *);
static int  get_lon_count(const NcFile *);
static NcVar get_goes_nc_var(NcFile *nc, const ConcatString &var_name,
                             bool exit_if_error=true);
static bool is_time_mismatch(NcFile *nc_in, NcFile *nc_adp);
static ConcatString make_geostationary_filename(Grid fr_grid);
static void regrid_goes_variable(NcFile *nc_in, const VarInfo *vinfo,
            DataPlane &fr_dp, DataPlane &to_dp,
            Grid fr_grid, Grid to_grid, IntArray *cellMapping, NcFile *nc_adp);
static void save_geostationary_data(const ConcatString geostationary_file,
            const float *latitudes, const float *longitudes,
            const GoesImagerData &grid_data);
static void set_qc_flags(const StringArray &);

////////////////////////////////////////////////////////////////////////

int met_main(int argc, char *argv[]) {

   // Store the program name
   program_name = get_short_name(argv[0]);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the input data file
   process_data_file();

   return 0;
}

////////////////////////////////////////////////////////////////////////

string get_tool_name() {
   return "point2grid";
}

////////////////////////////////////////////////////////////////////////

static void process_command_line(int argc, char **argv) {
   CommandLine cline;
   static const char *method_name = "process_command_line() -> ";

   // Set default regridding options
   RGInfo.enable     = true;
   RGInfo.field      = FieldType::None;
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
   cline.add(set_field,           "-field",           1);
   cline.add(set_method,          "-method",          1);
   cline.add(set_vld_thresh,      "-vld_thresh",      1);
   cline.add(set_name,            "-name",            1);
   cline.add(set_compress,        "-compress",        1);
   cline.add(set_qc_flags,        "-qc",              1);
   cline.add(set_adp,             "-adp",             1);
   cline.add(set_config,          "-config",          1);
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

   // Check for python format
   string python_command = InputFilename;
   bool use_xarray = (0 == python_command.find(conf_val_python_xarray));
   bool use_python = use_xarray || (0 == python_command.find(conf_val_python_numpy));

   // Check if the input file
#ifdef WITH_PYTHON
   if (use_python) {
      auto offset = python_command.find("=");
      if (offset == std::string::npos) {
         mlog << Error << "\n" << method_name
              << "trouble parsing the python command " << python_command << ".\n\n";
         exit(1);
      }
   }
   else
#else
   if (use_python) python_compile_error(method_name);
#endif
      if ( !file_exists(InputFilename.c_str()) ) {
         mlog << Error << "\n" << method_name
              << "file does not exist \"" << InputFilename << "\"\n\n";
         exit(1);
      }

   // Check for at least one configuration string
   if(FieldSA.n() < 1) {
      mlog << Error << "\nprocess_command_line() -> "
           << "The -field option must be used at least once!\n\n";
      usage();
   }

   // Check that same variable is required multiple times without -name argument
   if(VarNameSA.n() == 0) {
      MetConfig config;
      ConcatString vname;
      StringArray var_names;
      VarInfo *vinfo = VarInfoFactory::new_var_info(FileType_NcMet);
      for(int i=0; i<FieldSA.n(); i++) {
         vinfo->clear();
         // Populate the VarInfo object using the config string
         config.read_string(FieldSA[i].c_str());
         vinfo->set_dict(config);
         vname = vinfo->name();
         if (var_names.has(vname)) {
            mlog << Error << "\n" << method_name
                 << "Please add -name argument to avoid the output name "
                 << "conflicts on handling the variable \""
                 << vname << "\" multiple times.\n\n";
            usage();
         }
         else var_names.add(vname);
      }
      // Clean up
      if(vinfo) { delete vinfo; vinfo = (VarInfo *) nullptr; }
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

}

////////////////////////////////////////////////////////////////////////

static void process_data_file() {
   Grid fr_grid;
   GrdFileType ftype;
   ConcatString run_cs;
   auto nc_in = (NcFile *) nullptr;
   static const char *method_name = "process_data_file() -> ";

   // Initialize configuration object
   MetConfig config;
   config.read(replace_path(config_const_filename).c_str());
   config.read_string(FieldSA[0].c_str());

   // Note: The command line argument MUST processed before this
   if (compress_level < 0) compress_level = conf_info.conf.nc_compression();

   // Get the gridded file type from config string, if present
   ftype = parse_conf_file_type(&conf_info.conf);

   // Open the input file
   mlog << Debug(1)  << "Reading data file: " << InputFilename << "\n";
   bool goes_data = false;
   bool use_python = false;
   int obs_type;
   auto fr_mtddf = (Met2dDataFile *) nullptr;
#ifdef WITH_PYTHON
   string python_command = InputFilename;
   bool use_xarray = (0 == python_command.find(conf_val_python_xarray));
   use_python = use_xarray || (0 == python_command.find(conf_val_python_numpy));
   if (use_python) {
      auto offset = python_command.find("=");
      if (offset == std::string::npos) {
         mlog << Error << "\n" << method_name
              << "trouble parsing the python command " << python_command << ".\n\n";
         exit(1);
      }

      python_command = python_command.substr(offset+1);
      obs_type = TYPE_PYTHON;
      fr_mtddf = new MetNcMetDataFile();
   }
   else
#endif
   {
      nc_in = open_ncfile(InputFilename.c_str());

      // Get the obs type before opening NetCDF
      obs_type = get_obs_type(nc_in);
      goes_data = (obs_type == TYPE_GOES || obs_type == TYPE_GOES_ADP);
      if (obs_type == TYPE_UNKNOWN && ftype == FileType_NcCF) obs_type = TYPE_NCCF;
      if (obs_type == TYPE_NCCF) setenv(nc_att_met_point_nccf, "yes", 1);

      // Read the input data file
      fr_mtddf = Met2dDataFileFactory::new_met_2d_data_file(InputFilename.c_str(), ftype);
   }

   if(!fr_mtddf) {
      mlog << Error << "\n" << method_name
           << "\"" << InputFilename << "\" not a valid data file\n\n";
      exit(1);
   }

   // Store the input data file types
   ftype = fr_mtddf->file_type();

   // Setup the VarInfo request object
   VarInfo *vinfo = VarInfoFactory::new_var_info(ftype);

   if(!vinfo) {
      mlog << Error << "\n" << method_name
           << "unable to determine file type of \"" << InputFilename
           << "\"\n\n";
      exit(1);
   }

   // For python types read the first field to set the grid

   // Determine the "from" grid
#ifdef WITH_PYTHON
   if (!use_python) fr_grid = fr_mtddf->grid();
#else
   fr_grid = fr_mtddf->grid();
#endif

   // Determine the "to" grid
   Grid to_grid = parse_vx_grid(RGInfo, &fr_grid, &fr_grid);

   mlog << Debug(2) << "Interpolation options: "
        << "method = " << interpmthd_to_string(RGInfo.method)
        << ", vld_thresh = " << RGInfo.vld_thresh << "\n";

   // Build the run command string
   run_cs << "Point obs (" << fr_grid.serialize() << ") to " << to_grid.serialize();

   if (goes_data) {
      mlog << Debug(2) << "Input grid: " << fr_grid.serialize() << "\n";
      ConcatString grid_string = get_goes_grid_input(config, fr_grid);
      if (!grid_string.empty()) run_cs << " with " << grid_string;
   }
   mlog << Debug(2) << "Output grid: " << to_grid.serialize() << "\n";

   // Open the output file
   open_nc(to_grid, run_cs);

   if (goes_data) {
      process_goes_file(nc_in, config, vinfo, fr_grid, to_grid);
   }
   else if (TYPE_OBS == obs_type) {
      process_point_file(nc_in, config, vinfo, to_grid);
   }
   else if (TYPE_NCCF == obs_type) {
      process_point_nccf_file(nc_in, config, vinfo, fr_mtddf, to_grid);
      unsetenv(nc_att_met_point_nccf);
   }
#ifdef WITH_PYTHON
   else if (TYPE_PYTHON == obs_type) {
      process_point_python(python_command, config, vinfo, to_grid, use_xarray);
   }
#endif
   else {
      mlog << Error << "\n" << method_name
           << "Please check the input file. Only supports GOES, MET point obs, "
           << "and CF complaint NetCDF with time/lat/lon variables.\n\n";
      exit(1);
   }

   // Close the output file
   close_nc();

   // Clean up
   if(nc_in)    { delete nc_in;    nc_in  = nullptr; }
   if(fr_mtddf) { delete fr_mtddf; fr_mtddf = (Met2dDataFile *) nullptr; }
   if(vinfo)    { delete vinfo;    vinfo    = (VarInfo *)       nullptr; }

   return;
}

////////////////////////////////////////////////////////////////////////
// returns true if no error

bool get_nc_data_int_array(NcFile *nc, char *var_name, int *data_array, bool stop=true) {
   bool status = false;
   NcVar nc_var = get_nc_var(nc, var_name, stop);
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
   NcVar nc_var = get_nc_var(nc, var_name);
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

static int get_obs_type(NcFile *nc) {
   int obs_type = TYPE_UNKNOWN;
   MetConfig config;
   ConcatString att_val_scene_id;
   ConcatString att_val_project;
   ConcatString input_type;
   static const char *method_name = "get_obs_type() -> ";

   bool has_attr_grid = false;
   auto vinfo = VarInfoFactory::new_var_info(FileType_NcCF);
   for(int i=0; i<FieldSA.n(); i++) {
      vinfo->clear();
      // Populate the VarInfo object using the config string
      config.read_string(FieldSA[i].c_str());
      vinfo->set_dict(config);
      if (vinfo->grid_attr().is_set()) {
         has_attr_grid = true;
         break;
      }
   }
   if (vinfo) { delete vinfo; vinfo = (VarInfo *) nullptr; }

   if (has_attr_grid) {
      obs_type = TYPE_NCCF;
      input_type = "OBS_NCCF";
   }
   else if (get_global_att(nc, (string)"scene_id", att_val_scene_id)
            && get_global_att(nc, (string)"project", att_val_project)
            && att_val_project == "GOES" ) {
      obs_type = TYPE_GOES;
      input_type = "GOES";
      if (!adp_filename.empty()) {
         obs_type = TYPE_GOES_ADP;
         input_type = "GOES_ADP";
         if (!file_exists(adp_filename.c_str())) {
            mlog << Error << "\n" << method_name
                 << "ADP input \"" << adp_filename << "\" does not exist!\n\n";
            exit(1);
         }
      }
   }
   else if (has_lat_lon_vars(nc)) {
      obs_type = TYPE_NCCF;
      input_type = "OBS_NCCF";
   }
   else if (has_dim(nc, nc_dim_nhdr) && has_dim(nc, nc_dim_nobs)) {
      obs_type = TYPE_OBS;
      input_type = "OBS_MET";
   }

   mlog << Debug(5) << method_name << "input type: \"" << input_type << "\"\n";

   return obs_type;
}

////////////////////////////////////////////////////////////////////////
// Check the message types

void prepare_message_types(const StringArray &hdr_types) {
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
         mlog << Error << "\n" << method_name
              << "No matching message types ["
              << msg_list_str << "].\n\n";
         exit(1);
      }
   }
}

////////////////////////////////////////////////////////////////////////

IntArray prepare_qc_array(const StringArray &qc_tables) {
   IntArray qc_idx_array;
   if (qc_flags.n() > 0) {
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

std::set<GOES_QC> prepare_qoes_qc_array() {
   std::set<GOES_QC> qc_flags_set;
   for(int idx=0; idx<qc_flags.n(); idx++) {
      switch (qc_flags[idx]) {
         case 0: qc_flags_set.insert(GOES_QC::HIGH);    break;
         case 1: qc_flags_set.insert(GOES_QC::MEDIUM);  break;
         case 2: qc_flags_set.insert(GOES_QC::LOW);     break;
         default:                                       break;
      }
   }
   return qc_flags_set;
}

////////////////////////////////////////////////////////////////////////

void process_point_met_data(MetPointData *met_point_obs, MetConfig &config, VarInfo *vinfo,
                            const Grid &to_grid) {
   int hdr_idx;
   int var_idx_or_gc;
   ConcatString vname;
   DataPlane fr_dp;
   DataPlane to_dp;
   DataPlane cnt_dp;
   DataPlane mask_dp;
   DataPlane prob_dp;
   DataPlane prob_mask_dp;

   bool has_prob_thresh = !prob_cat_thresh.check(bad_data_double);

   unixtime requested_valid_time, valid_time;
   static const char *method_name = "process_point_met_data() -> ";

   // Check for at least one configuration string
   if(FieldSA.n() < 1) {
      mlog << Error << "\n" << method_name
           << "The -field option must be used at least once!\n\n";
      usage();
   }

   const MetPointHeader *header_data = met_point_obs->get_header_data();
   MetPointObsData *obs_data = met_point_obs->get_point_obs_data();

   int nhdr = met_point_obs->get_hdr_cnt();
   int nobs = met_point_obs->get_obs_cnt();
   bool empty_input = (nhdr == 0 && nobs == 0);
   bool use_var_id = met_point_obs->is_using_var_id();

   auto hdr_lats = new float[nhdr];
   auto hdr_lons = new float[nhdr];
   IntArray var_index_array;
   IntArray valid_time_array;
   StringArray qc_tables = met_point_obs->get_qty_data();
   StringArray var_names = met_point_obs->get_var_names();
   StringArray hdr_valid_times = header_data->vld_array;
   hdr_valid_times.sort();

   met_point_obs->get_lats(hdr_lats);
   met_point_obs->get_lons(hdr_lons);

   // Check the message types
   prepare_message_types(header_data->typ_array);

   // Check and read obs_vid and obs_var if exists

   bool has_qc_flags = (qc_flags.n() > 0);
   IntArray qc_idx_array = prepare_qc_array(qc_tables);

   // Initialize size and values of output fields
   int nx = to_grid.nx();
   int ny = to_grid.ny();
   to_dp.set_size(nx, ny);
   to_dp.set_constant(bad_data_double);
   cnt_dp.set_size(nx, ny);
   cnt_dp.set_constant(0);
   mask_dp.set_size(nx, ny);
   mask_dp.set_constant(0);
   if (has_prob_thresh || do_gaussian_filter) {
      prob_dp.set_size(nx, ny);
      prob_dp.set_constant(0);
      prob_mask_dp.set_size(nx, ny);
      prob_mask_dp.set_constant(0);
   }

   // Loop through the requested fields
   int obs_count_zero_to = 0;
   int obs_count_zero_from = 0;
   int obs_count_non_zero_to = 0;
   int obs_count_non_zero_from = 0;
   auto cellMapping = (IntArray *) nullptr;
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
               exit_by_field_name_error = true;
               error_msg << "The variable \"" << vname << "\" is not available.\n";
            }
         }
         else {
            const int TMP_BUF_LEN = 128;
            char grib_code[TMP_BUF_LEN + 1];
            var_idx_or_gc = atoi(vname.c_str());
            snprintf(grib_code, TMP_BUF_LEN, "%d", var_idx_or_gc);
            if (vname != grib_code) {
               ConcatString var_id = conf_info.get_var_id(vname);
               if( var_id.nonempty() ) {
                  var_idx_or_gc = atoi(var_id.c_str());
                  snprintf(grib_code, TMP_BUF_LEN, "%d", var_idx_or_gc);
               }
               else {
                  exit_by_field_name_error = true;
                  error_msg << "Invalid GRIB code [" << vname << "]\n";
               }
            }
            else {
               bool not_found_grib_code = true;
               for (int idx=0; idx<nobs; idx++) {
                  if (var_idx_or_gc == obs_data->obs_ids[idx]) {
                     not_found_grib_code = false;
                     break;
                  }
               }
               if (not_found_grib_code) {
                  exit_by_field_name_error = true;
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
               if (!grib_codes.has(obs_data->obs_ids[idx])) {
                  grib_codes.add(obs_data->obs_ids[idx]);
                  if (0 < idx) log_msg << ", ";
                  log_msg << obs_data->obs_ids[idx];
               }
            }
         }
         if (empty_input) {
            mlog << Warning << "\n" << method_name
                 << error_msg << "\tBut ignored because of empty input\n\n";
         }
         else {
            mlog << Error << "\n" << method_name
                 << error_msg
                 << "Try setting the \"name\" in the \"-field\" command line option to one of the available names:\n"
                 << "\t" << log_msg << "\n\n";
            exit(1);
         }
      }

      // Check the time range. Apply the time window
      bool valid_time_from_config = true;
      unixtime valid_beg_ut;
      unixtime valid_end_ut;
      unixtime obs_time;

      valid_time_array.clear();
      valid_time = vinfo->valid();
      if (valid_time == 0) valid_time = conf_info.valid_time;
      requested_valid_time = valid_time;
      if (0 < valid_time) {
         valid_beg_ut = valid_end_ut = valid_time;
         if (!is_bad_data(conf_info.beg_ds)) valid_beg_ut += conf_info.beg_ds;
         if (!is_bad_data(conf_info.end_ds)) valid_end_ut += conf_info.end_ds;
         for(int idx=0; idx<hdr_valid_times.n(); idx++) {
            obs_time = timestring_to_unix(hdr_valid_times[idx].c_str());
            if (valid_beg_ut <= obs_time && obs_time <= valid_end_ut) {
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
         for(int idx=0; idx<hdr_valid_times.n(); idx++) {
            obs_time = timestring_to_unix(hdr_valid_times[idx].c_str());
            if (obs_time > valid_time) valid_time = obs_time;
         }
      }
      mlog << Debug(3) << method_name << "valid_time from "
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
           << "var: " << vname << ", index: " << var_idx_or_gc << ".\n";

      int var_count = 0;
      int var_count2 = 0;
      int to_count = 0;
      int filtered_by_time = 0;
      int filtered_by_msg_type = 0;
      int filtered_by_qc = 0;
      for (int idx=0; idx < nobs; idx++) {
         if (var_idx_or_gc == obs_data->obs_ids[idx]) {
            var_count2++;
            hdr_idx = obs_data->obs_hids[idx];
            if (0 < valid_time_array.n() &&
                  !valid_time_array.has(header_data->vld_idx_array[hdr_idx])) {
               filtered_by_time++;
               continue;
            }

            if(!keep_message_type(header_data->typ_idx_array[hdr_idx])) {
               filtered_by_msg_type++;
               continue;
            }

            // Filter by QC flag (-qc command line option)
            if (has_qc_flags && !qc_idx_array.has(obs_data->obs_qids[idx])) {
               filtered_by_qc++;
               continue;
            }

            // Filter by QC inclusion/exclusion lists (obs_quality_inc/exc config option)
            if ((conf_info.obs_qty_inc.n() > 0 &&
                !conf_info.obs_qty_inc.has(obs_data->get_obs_qty(idx))) ||
                (conf_info.obs_qty_exc.n() > 0 &&
                 conf_info.obs_qty_exc.has(obs_data->get_obs_qty(idx)))) {
               filtered_by_qc++;
               continue;
            }

            var_index_array.add(idx);
            var_count++;
            if (is_eq(obs_data->obs_vals[idx], (float)0.)) obs_count_zero_from++;
            else obs_count_non_zero_from++;
         }
      }

      if (cellMapping) {
         clear_cell_mapping(cellMapping);
         delete [] cellMapping;
         cellMapping = (IntArray *) nullptr;
      }
      cellMapping = new IntArray[nx * ny];
      if( get_grid_mapping(to_grid, cellMapping, var_index_array,
                           obs_data->obs_hids, hdr_lats, hdr_lons) ) {
         int from_index;
         IntArray cellArray;
         NumArray dataArray;
         int offset = 0;
         int valid_count = 0;
         int censored_count = 0;
         double data_value;
         double from_min_value =  10e10;
         double from_max_value = -10e10;

         // Initialize counter and output fields
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
               if (0 < cellArray.n()) {
                  valid_count = 0;
                  dataArray.clear();
                  dataArray.extend(cellArray.n());
                  for (int dIdx=0; dIdx<cellArray.n(); dIdx++) {
                     from_index = cellArray[dIdx];
                     data_value = obs_data->get_obs_val(from_index);
                     if (is_bad_data(data_value)) continue;

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
                     double to_value;
                     if      (RGInfo.method == InterpMthd::Min) to_value = dataArray.min();
                     else if (RGInfo.method == InterpMthd::Max) to_value = dataArray.max();
                     else if (RGInfo.method == InterpMthd::Median) {
                        dataArray.sort_array();
                        to_value = dataArray[data_count/2];
                        if (0 == data_count % 2)
                           to_value = (float)(to_value + dataArray[(data_count/2)+1])/2;
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
                     mlog << Debug(8) << method_name << "data at " << x_idx << "," << y_idx
                          << ", value: " << to_value << "\n";
                  }
               }
            }
         }
      }

      // Write the regridded data
      write_nc(to_dp, to_grid, vinfo, vname.c_str());

      ConcatString vname_cnt = vname;
      vname_cnt << "_cnt";
      ConcatString vname_mask = vname;
      vname_mask << "_mask";

      ConcatString var_long_name = vinfo->long_name();
      ConcatString dim_string = "(*,*)";

      ConcatString tmp_long_name = vname_cnt;
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

      mlog << Debug(7) << method_name << "obs_count_zero_to: " << obs_count_zero_to
           << ", obs_count_non_zero_to: " << obs_count_non_zero_to << "\n";

      ConcatString log_msg;
      log_msg << "Filtered by time: " << filtered_by_time;
      if (0 < requested_valid_time) {
         log_msg << " [" << unix_to_yyyymmdd_hhmmss(requested_valid_time) << "]";
      }
      log_msg << ", by msg_type: " << filtered_by_msg_type;
      if (0 < filtered_by_msg_type) {
         log_msg << " [" << write_css(conf_info.message_type) << "]";
      }
      log_msg << ", by QC: " << filtered_by_qc;
      if (0 < filtered_by_qc) {
         if (0 < qc_flags.n()) {
            log_msg << " [-qc "
                    << write_css(qc_flags)
                    << "]";
         }
         if (0 < conf_info.obs_qty_inc.n()) {
            log_msg << " [obs_quality_inc = "
                    << write_css(conf_info.obs_qty_inc)
                    << "]";
         }
         if (0 < conf_info.obs_qty_exc.n()) {
            log_msg << " [obs_quality_exc = "
                    << write_css(conf_info.obs_qty_exc)
                    << "]";
         }
      }
      log_msg << ", out of " << var_count2;
      int filtered_count = filtered_by_msg_type + filtered_by_qc + (int)requested_valid_time;
      if (0 == var_count) {
         if (0 == filtered_count) {
            mlog << Warning << "\n" << method_name
                 << "No valid data for the variable ["
                 << vinfo->name() << "]\n\n";
         }
         else {
            mlog << Warning << "\n" << method_name
                 << "No valid data after filtering.\n\t"
                 << log_msg << ".\n\n";
         }
      }
      else {
         mlog << Debug(2) << method_name << "var_count=" << var_count
              << ", grid: " << to_count << " out of " << (nx * ny) << "  "
              << (0 < filtered_count ? log_msg.c_str() : " ") << "\n";
      }
   } // end for i

   if (cellMapping) {
      clear_cell_mapping(cellMapping);
      delete [] cellMapping;   cellMapping = (IntArray *) nullptr;
   }

   delete [] hdr_lats;
   delete [] hdr_lons;

   return;
}

////////////////////////////////////////////////////////////////////////

static void process_point_file(NcFile *nc_in, MetConfig &config, VarInfo *vinfo,
                               const Grid to_grid) {
   ConcatString vname;
   clock_t start_clock =  clock();
   static const char *method_name = "process_point_file() -> ";
   static const char *method_name_s = "process_point_file()";

   // Check for at least one configuration string
   if(FieldSA.n() < 1) {
      mlog << Error << "\n" << method_name
           << "The -field option must be used at least once!\n\n";
      usage();
   }

   MetNcPointObsIn nc_point_obs;
   nc_point_obs.set_netcdf(nc_in, true);
   // Read the dimensions and variables
   nc_point_obs.read_dim_headers();
   nc_point_obs.check_nc(GET_NC_NAME_P(nc_in).c_str(), method_name_s);   // exit if missing dims/vars
   // Read all obs data to compute the cell mapping
   nc_point_obs.read_obs_data();
   process_point_met_data(&nc_point_obs, config, vinfo, to_grid);

   nc_point_obs.close();

   mlog << Debug(LEVEL_FOR_PERFORMANCE) << method_name << "took "
        << get_exe_duration(start_clock) << " seconds\n";

}

////////////////////////////////////////////////////////////////////////

#ifdef WITH_PYTHON

static void process_point_python(const string python_command, MetConfig &config,
                                 VarInfo *vinfo, const Grid to_grid, bool use_xarray) {
   clock_t start_clock =  clock();
   static const char *method_name = "process_point_python() -> ";

   // Check for at least one configuration string
   if(FieldSA.n() < 1) {
      mlog << Error << "\n" << method_name
           << "The -field option must be used at least once!\n\n";
      usage();
   }

   MetPythonPointDataFile met_point_file;
   if(!met_point_file.open(python_command.c_str(), use_xarray)) {
      met_point_file.close();

      mlog << Error << "\n" << method_name
           << "trouble getting point observation file from python command "
           << python_command << "\n\n";

      exit(1);
   }

   MetPointData *met_point_obs = met_point_file.get_met_point_data();
   process_point_met_data(met_point_obs, config, vinfo, to_grid);

   met_point_file.close();

   mlog << Debug(LEVEL_FOR_PERFORMANCE) << method_name << "took "
        << get_exe_duration(start_clock) << " seconds\n";

   return;
}
#endif

////////////////////////////////////////////////////////////////////////

static void process_point_nccf_file(NcFile *nc_in, MetConfig &config,
                                    VarInfo *vinfo, Met2dDataFile *fr_mtddf,
                                    const Grid to_grid) {
   ConcatString vname;
   DataPlane fr_dp, to_dp;
   DataPlane cnt_dp, mask_dp;
   unixtime valid_beg_ut, valid_end_ut;
   bool *skip_times = nullptr;
   double *valid_times = nullptr;
   int filtered_by_time = 0;
   int time_from_size = 1;
   clock_t start_clock =  clock();
   bool opt_all_attrs = false;
   Grid fr_grid = fr_mtddf->grid();
   static const char *method_name = "process_point_nccf_file() -> ";

   NcVar var_lat;
   NcVar var_lon;
   bool user_defined_latlon = false;
   ConcatString lat_vname = conf_info.get_var_name(conf_key_lat_vname);
   ConcatString lon_vname = conf_info.get_var_name(conf_key_lon_vname);

   if (lat_vname != conf_key_lat_vname && lon_vname != conf_key_lon_vname) {
      if (has_var(nc_in, lat_vname.c_str())) {
         var_lat = get_nc_var(nc_in, lat_vname.c_str());
      }
      if (has_var(nc_in, lon_vname.c_str())) {
         var_lon = get_nc_var(nc_in, lon_vname.c_str());
      }
      if (IS_INVALID_NC(var_lat)) {
         mlog << Error << "\n" << method_name
              << "can not find the latitude variable (" << lat_vname
              << ") from the config file (" << config_filename << ").\n\n";
         exit(1);
      }
      else if (IS_INVALID_NC(var_lon)) {
         mlog << Error << "\n" << method_name
              << "can not find the longitude variable (" << lon_vname
              << ") from the config file (" << config_filename << ").\n\n";
         exit(1);
      }
      else user_defined_latlon = true;
   }
   // Find lat/lon variables from the coordinates attribue
   if (0 < FieldSA.n() && !user_defined_latlon) {
      ConcatString coordinates_value;
      auto var_info = VarInfoNcCF(*(VarInfoNcCF *)vinfo);
      // Initialize
      var_info.clear();
      // Populate the VarInfo object using the config string
      config.read_string(FieldSA[0].c_str());
      var_info.set_dict(config);
      NcVar var_data = get_nc_var(nc_in, var_info.name().c_str());
      if (get_nc_att_value(&var_data, coordinates_att_name, coordinates_value)) {
         StringArray sa = coordinates_value.split(" ");
         ConcatString units;
         for (int idx=0; idx<sa.n_elements(); idx++) {
            NcVar nc_var = get_nc_var(nc_in, sa[idx].c_str());
            if (get_var_units(&nc_var, units)) {
               if (IS_INVALID_NC(var_lat) && is_nc_unit_latitude(units.c_str())) var_lat = nc_var;
               else if (IS_INVALID_NC(var_lon) && is_nc_unit_longitude(units.c_str())) var_lon = nc_var;
            }
         }
      }
   }
   if (IS_INVALID_NC(var_lat)) var_lat = get_nc_var_lat(nc_in);
   if (IS_INVALID_NC(var_lon)) var_lon = get_nc_var_lon(nc_in);

   if (IS_INVALID_NC(var_lat)) {
      mlog << Error << "\n" << method_name
           << "can not find the latitude variable.\n\n";
      exit(1);
   }
   if (IS_INVALID_NC(var_lon)) {
      mlog << Error << "\n" << method_name
           << "can not find the longitude variable.\n\n";
      exit(1);
   }

   // Check for at least one configuration string
   if(FieldSA.n() < 1) {
      mlog << Error << "\n" << method_name
           << "The -field option must be used at least once!\n\n";
      usage();
   }

   lat_vname = GET_NC_NAME(var_lat);
   lon_vname = GET_NC_NAME(var_lon);
   mlog << Debug(5) << method_name << "Cell mapping from "
        << lat_vname << " and " << lon_vname << " variables\n";

   unixtime valid_time = bad_data_int;
   valid_beg_ut = valid_end_ut = conf_info.valid_time;

   NcVar time_var = get_nc_var_time(nc_in);
   if( IS_VALID_NC(time_var) ) {
      if( 1 < get_dim_count(&time_var) ) {
         double max_time = bad_data_double;
         time_from_size = get_data_size(&time_var);
         skip_times = new bool[time_from_size];
         valid_times = new double[time_from_size];
         if (get_nc_data(&time_var, valid_times)) {
            int sec_per_unit = 0;
            bool no_leap_year = false;
            auto ref_ut = (unixtime) 0;
            unixtime tmp_time;
            if( conf_info.valid_time > 0 ) {
               if (!is_bad_data(conf_info.beg_ds)) valid_beg_ut += conf_info.beg_ds;
               if (!is_bad_data(conf_info.end_ds)) valid_end_ut += conf_info.end_ds;
               ref_ut = get_reference_unixtime(&time_var, sec_per_unit, no_leap_year);
            }
            for (int i=0; i<time_from_size; i++) {
               if( conf_info.valid_time > 0 ) {
                  tmp_time = add_to_unixtime(ref_ut, sec_per_unit,
                                             valid_times[i], no_leap_year);
                  skip_times[i] = (valid_beg_ut > tmp_time || tmp_time > valid_end_ut);
                  if( skip_times[i]) filtered_by_time++;
               }
               else skip_times[i] = false;
               if (max_time < valid_times[i]) max_time = valid_times[i];
            }
            valid_time = compute_unixtime(&time_var, max_time);
         }
      }
      else valid_time = find_valid_time(time_var);
   }
   to_dp.set_size(to_grid.nx(), to_grid.ny());
   IntArray *var_cell_mapping = nullptr;
   auto cellMapping = new IntArray[to_grid.nx() * to_grid.ny()];
   get_grid_mapping(fr_grid, to_grid, cellMapping, var_lat, var_lon, skip_times);
   if( skip_times ) delete [] skip_times;
   if( valid_times ) delete [] valid_times;

   // Loop through the requested fields
   for(int i=0; i<FieldSA.n(); i++) {

      // Initialize
      vinfo->clear();

      // Populate the VarInfo object using the config string
      config.read_string(FieldSA[i].c_str());
      vinfo->set_dict(config);

      NcVar var_data = get_nc_var(nc_in, vinfo->name().c_str());
      ConcatString coordinates_value;
      if (!user_defined_latlon && get_nc_att_value(&var_data, coordinates_att_name, coordinates_value)) {
         StringArray sa = coordinates_value.split(" ");
         int count = sa.n_elements();
         if (count >= 2) {
            bool match_lat = false;
            bool match_lon = false;
            for (int idx=0; idx<count; idx++) {
               if (lat_vname == sa[idx]) match_lat = true;
               if (lon_vname == sa[idx]) match_lon = true;
            }
            if (!match_lat && !match_lon) {
               NcVar v_lat;
               NcVar v_lon;
               ConcatString units;
               for (int idx=0; idx<count; idx++) {
                  NcVar nc_var = get_nc_var(nc_in, sa[idx].c_str());
                  if (get_var_units(&nc_var, units)) {
                     if (is_nc_unit_latitude(units.c_str())) v_lat = nc_var;
                     else if (is_nc_unit_longitude(units.c_str())) v_lon = nc_var;
                  }
               }
               var_cell_mapping = new IntArray[to_grid.nx() * to_grid.ny()];
               get_grid_mapping(fr_grid, to_grid, var_cell_mapping, v_lat, v_lon, skip_times);
               mlog << Debug(4) << method_name << "Override cell mapping from "
                    << GET_NC_NAME(v_lat) << " and " << GET_NC_NAME(v_lon) << "\n";
            }
         }
      }

      to_dp.erase();
      to_dp.set_init(valid_time);
      to_dp.set_valid(valid_time);
      regrid_nc_variable(nc_in, fr_mtddf, vinfo, fr_dp, to_dp, to_grid,
                         (nullptr != var_cell_mapping ? var_cell_mapping: cellMapping));

      // List range of data values
      if(mlog.verbosity_level() >= 2) {
         double fr_dmin;
         double fr_dmax;
         double to_dmin;
         double to_dmax;
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

      bool has_prob_thresh = !prob_cat_thresh.check(bad_data_double);
      if (has_prob_thresh || do_gaussian_filter) {
         DataPlane prob_dp;
         DataPlane prob_mask_dp;
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
               auto value = to_dp.get(x, y);
               if (!is_bad_data(value) &&
                     ((has_prob_thresh && prob_cat_thresh.check(value))
                       || (do_gaussian_filter && !has_prob_thresh))) {
                  prob_dp.set(1, x, y);
               }
            }
         }

         if (do_gaussian_filter) interp_gaussian_dp(prob_dp, RGInfo.gaussian, RGInfo.vld_thresh);
         write_nc(prob_dp, to_grid, vinfo, vname_prob.c_str());
         if(IS_VALID_NC(var_data)) {
            NcVar out_var = get_nc_var(nc_out, vname.c_str());
            copy_nc_atts(&var_data, &out_var, opt_all_attrs);
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

      if (nullptr != var_cell_mapping) {
         clear_cell_mapping(var_cell_mapping);
         delete [] var_cell_mapping;
         var_cell_mapping = nullptr;
      }

   } // end for i

   clear_cell_mapping(cellMapping);
   delete [] cellMapping;
   cellMapping = (IntArray *) nullptr;
   if( 0 < filtered_by_time ) {
      mlog << Debug(2) << method_name << "Filtered by time: " << filtered_by_time
           << " out of " << time_from_size
           << " [" << unix_to_yyyymmdd_hhmmss(valid_beg_ut) << " to "
           << unix_to_yyyymmdd_hhmmss(valid_end_ut) << "]\n";
   }
   mlog << Debug(LEVEL_FOR_PERFORMANCE) << method_name << "took "
        << get_exe_duration(start_clock) << " seconds\n";

   return;
}

////////////////////////////////////////////////////////////////////////

static void regrid_nc_variable(NcFile *nc_in, Met2dDataFile *fr_mtddf,
                               VarInfo *vinfo, DataPlane &fr_dp, DataPlane &to_dp,
                               Grid to_grid, IntArray *cellMapping) {

   int to_cell_cnt = 0;
   clock_t start_clock = clock();
   Grid fr_grid = fr_mtddf->grid();
   static const char *method_name = "regrid_nc_variable() -> ";

   NcVar var_data = get_nc_var(nc_in, vinfo->name().c_str());
   if (IS_INVALID_NC(var_data)) {
      mlog << Error << "\n" << method_name
           << "the variable \"" << vinfo->name() << "\" does not exist at \""
           << InputFilename << "\"\n\n";
      exit(1);
   }

   if(!fr_mtddf->data_plane(*vinfo, fr_dp)) {
      mlog << Error << "\n" << method_name
           << "Trouble reading data \""
           << vinfo->name() << "\" from file \""
           << InputFilename << "\"\n\n";
      exit(1);
   }

   int from_lat_count = fr_grid.ny();
   int from_lon_count = fr_grid.nx();
   if (0 == from_lat_count) from_lat_count = fr_dp.ny();
   if (0 == from_lon_count) from_lon_count = fr_dp.nx();

   int from_data_size = from_lat_count * from_lon_count;
   bool is_to_north = !fr_grid.get_swap_to_north();
   auto from_data = new float[from_data_size];
   for (int xIdx=0; xIdx<from_lon_count; xIdx++) {
      for (int yIdx=0; yIdx<from_lat_count; yIdx++) {
         int offset = fr_dp.two_to_one(xIdx,yIdx,is_to_north);
         from_data[offset] = fr_dp.get(xIdx,yIdx);
      }
   }
   mlog << Debug(LEVEL_FOR_PERFORMANCE) << method_name << "took "
        << get_exe_duration(start_clock) << " seconds for read variable\n";

   int from_index;
   int no_map_cnt = 0;
   int missing_cnt = 0;
   int non_missing_cnt = 0;
   double data_value;
   IntArray cellArray;
   NumArray dataArray;
   double from_min_value =  10e10;
   double from_max_value = -10e10;
   int to_lat_cnt = to_grid.ny();
   int to_lon_cnt = to_grid.nx();

   missing_cnt = non_missing_cnt = 0;
   to_dp.set_constant(bad_data_double);

   for (int xIdx=0; xIdx<to_lon_cnt; xIdx++) {
      for (int yIdx=0; yIdx<to_lat_cnt; yIdx++) {
         int offset = to_dp.two_to_one(xIdx,yIdx);
         cellArray = cellMapping[offset];
         if (0 < cellArray.n()) {
            dataArray.clear();
            dataArray.extend(cellArray.n());
            for (int dIdx=0; dIdx<cellArray.n(); dIdx++) {
               from_index = cellArray[dIdx];
               data_value = from_data[from_index];
               if (is_bad_data(data_value)) {
                  missing_cnt++;
                  continue;
               }

               dataArray.add(data_value);
               non_missing_cnt++;
               if(mlog.verbosity_level() >= 4) {
                  if (from_min_value > data_value) from_min_value = data_value;
                  if (from_max_value < data_value) from_max_value = data_value;
               }
            }

            if (0 < dataArray.n()) {
               double to_value;
               int data_cnt = dataArray.n();
               if (1 == data_cnt) to_value = dataArray[0];
               else if (RGInfo.method == InterpMthd::Min) to_value = dataArray.min();
               else if (RGInfo.method == InterpMthd::Max) to_value = dataArray.max();
               else if (RGInfo.method == InterpMthd::Median) {
                  dataArray.sort_array();
                  to_value = dataArray[data_cnt/2];
                  if (0 == data_cnt % 2)
                     to_value = (to_value + dataArray[(data_cnt/2)+1])/2;
               }
               else to_value = dataArray.sum() / data_cnt;    // UW_Mean

               to_dp.set(to_value, xIdx, yIdx);
               to_cell_cnt++;
               if(mlog.verbosity_level() >= 9) {
                  double to_lat;
                  double to_lon;
                  to_grid.xy_to_latlon(xIdx,yIdx, to_lat, to_lon);
                  to_lon *= -1;
                  if (1 == data_cnt)
                     mlog << Debug(9) << method_name
                          << "value: " << to_value << " to (" << to_lon << ", " << to_lat
                          << ") from offset " << from_index << ".\n";
                  else
                     mlog << Debug(9) << method_name
                          <<   "value: " << to_value
                          << ", max: " << dataArray.max()
                          << ", min: " << dataArray.min()
                          << ", mean: " << dataArray.sum()/data_cnt
                          << " from " << data_cnt << " (out of " << cellArray.n()
                          << ") data values to (" << to_lon << ", " << to_lat << ").\n";
               }
            }
         }
         else {
            no_map_cnt++;
         }
      }
   }

   delete [] from_data;

   mlog << Debug(4) << method_name << "[Count] data cells: " << to_cell_cnt
        << ", missing: " << missing_cnt << ", non_missing: " << non_missing_cnt
        << ", non mapped cells: " << no_map_cnt
        << " out of " << (to_lat_cnt*to_lon_cnt)
        << "\n\tRange:  data: [" << from_min_value << " - " << from_max_value
        << "]\n";

   if (to_cell_cnt == 0) {
      mlog << Warning << "\n" << method_name
           << " There are no matching cells between input and the target grid.\n\n";
   }

   mlog << Debug(LEVEL_FOR_PERFORMANCE) << method_name << "took "
        << get_exe_duration(start_clock) << " seconds\n";
}

////////////////////////////////////////////////////////////////////////

static void open_nc(const Grid &grid, ConcatString run_cs) {

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
   write_netcdf_proj(nc_out, grid, lat_dim, lon_dim);

   // Add the lat/lon variables
   write_netcdf_latlon(nc_out, &lat_dim, &lon_dim, grid);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nc_data(const DataPlane &dp, const Grid &grid, NcVar *data_var) {

   // Allocate memory to store data values for each grid point
   auto data = new float [grid.nx()*grid.ny()];

   // Store the data
   int grid_nx = grid.nx();
   int grid_ny = grid.ny();
   for(int x=0; x<grid_nx; x++) {
      for(int y=0; y<grid_ny; y++) {
         int n = DefaultTO.two_to_one(grid_nx, grid_ny, x, y);
         data[n] = (float)dp(x, y);
      } // end for y
   } // end for x

   // Write out the data
   if(!put_nc_data_with_dims(data_var, &data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_nc_data() -> "
           << "error writing data to the output file.\n\n";
      exit(1);
   }

   // Clean up
   if(data) { delete [] data;  data = (float *) nullptr; }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nc_data_int(const DataPlane &dp, const Grid &grid, NcVar *data_var) {

   // Allocate memory to store data values for each grid point
   auto data = new int [grid.nx()*grid.ny()];

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
   if(data) { delete [] data;  data = (int *) nullptr; }

   return;
}

////////////////////////////////////////////////////////////////////////

static void write_nc(const DataPlane &dp, const Grid &grid,
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

static void write_nc_int(const DataPlane &dp, const Grid &grid,
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

static void process_goes_file(NcFile *nc_in, MetConfig &config, VarInfo *vinfo,
                              const Grid fr_grid, const Grid to_grid) {
   DataPlane fr_dp, to_dp;
   ConcatString vname;
   const size_t global_attr_count = GOES_global_attr_names.size();
   bool opt_all_attrs = false;
   clock_t start_clock =  clock();
   auto nc_adp = (NcFile *) nullptr;
   static const char *method_name = "process_goes_file() -> ";

   ConcatString tmp_dir = config.get_tmp_dir();
   ConcatString geostationary_file(tmp_dir);
   geostationary_file.add("/");
   geostationary_file.add(make_geostationary_filename(fr_grid));

   // Open ADP file if exists
   if (!adp_filename.empty() && file_exists(adp_filename.c_str())) {
      nc_adp = open_ncfile(adp_filename.c_str());
      if (IS_INVALID_NC_P(nc_adp)) {
         mlog << Error << "\n" << method_name
              << "Can't open the ADP input \"" << adp_filename << "\"\n\n";
         exit(1);
      }
      else if (is_time_mismatch(nc_in, nc_adp)) {
         exit(1);
      }
   }

   NcVar time_var = get_nc_var_time(nc_in);
   unixtime valid_time = find_valid_time(time_var);
   to_dp.set_size(to_grid.nx(), to_grid.ny());
   auto cellMapping = new IntArray[to_grid.nx() * to_grid.ny()];
   get_grid_mapping(fr_grid, to_grid, cellMapping, geostationary_file);

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
         double fr_dmin;
         double fr_dmax;
         double to_dmin;
         double to_dmax;
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
         for (size_t idx=0; idx<global_attr_count; idx++) {
            copy_nc_att(nc_in, &to_var, (string)GOES_global_attr_names[idx]);
         }
         copy_nc_atts(&var_data, &to_var, opt_all_attrs);
      }

      bool has_prob_thresh = !prob_cat_thresh.check(bad_data_double);
      if (has_prob_thresh || do_gaussian_filter) {
         DataPlane prob_dp;
         DataPlane prob_mask_dp;
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
               double value = to_dp.get(x, y);
               if (!is_bad_data(value) &&
                     ((has_prob_thresh && prob_cat_thresh.check(value))
                       || (do_gaussian_filter && !has_prob_thresh))) {
                  prob_dp.set(1, x, y);
               }
            }
         }

         if (do_gaussian_filter) interp_gaussian_dp(prob_dp, RGInfo.gaussian, RGInfo.vld_thresh);
         write_nc(prob_dp, to_grid, vinfo, vname_prob.c_str());
         if(IS_VALID_NC(var_data)) {
            NcVar out_var = get_nc_var(nc_out, vname.c_str());
            for (size_t idx=0; idx<global_attr_count; idx++) {
               copy_nc_att(nc_in, &out_var, (string)GOES_global_attr_names[idx]);
            }
            copy_nc_atts(&var_data, &out_var, opt_all_attrs);
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

   multimap<string,NcVar> mapVar = GET_NC_VARS_P(nc_in);
   for (const auto &kv : mapVar) {
      if (kv.first == "t" || string::npos != kv.first.find("time")) {
         NcVar from_var = kv.second;
         copy_nc_var(nc_out, &from_var);
      }
   }

   delete nc_adp; nc_adp = nullptr;
   clear_cell_mapping(cellMapping);
   delete [] cellMapping;   cellMapping = (IntArray *) nullptr;
   mlog << Debug(LEVEL_FOR_PERFORMANCE) << method_name << "took "
        << get_exe_duration(start_clock) << " seconds\n";

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
   static const char *method_name = "check_lat_lon() -> ";
   for (int idx=0; idx<data_size; idx++) {
      if (cnt_printed < 10 && latitudes[idx] > MISSING_LATLON
          && longitudes[idx] > MISSING_LATLON) {
         mlog << Debug(11) << method_name << "index: " << idx <<  " lat: "
              << latitudes[idx] << ", lon: " << longitudes[idx] << "\n";
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
   mlog << Debug(7) << method_name << "Count: missing -> lat: " << cnt_missing_lat
        << ", lon: " << cnt_missing_lon << "   invalid -> lat: "
        << cnt_bad_lat << ", lon: " << cnt_bad_lon << "\n"
        << method_name << "LAT: " << min_lat << " to " << max_lat << "\n"
        << method_name << "LONG: " << min_lon << " to " << max_lon << "\n";
}

////////////////////////////////////////////////////////////////////////
//              QC flags: 0=high, 1=medium, 2=low
//   Enterpise algorithm: 0=high, 1=medium, 2=low
//    Baseline algorithm: 3=high, 1=medium, 0=low (high=12/48, medium=4/16)
// returns bad_data_int if it does not belong to high, mediuam, or low.

GOES_QC compute_adp_qc_flag(int adp_qc, int shift_bits) {
   GOES_QC adp_qc_flag;
   int particle_qc = ((adp_qc >> shift_bits) & 0x03);
   if (particle_qc == adp_qc_high)        adp_qc_flag = GOES_QC::HIGH;
   else if (particle_qc == adp_qc_medium) adp_qc_flag = GOES_QC::MEDIUM;
   else if (particle_qc == adp_qc_low)    adp_qc_flag = GOES_QC::LOW;
   else adp_qc_flag = GOES_QC::NA;

   return adp_qc_flag;
}


////////////////////////////////////////////////////////////////////////

void clear_cell_mapping(IntArray *cell_mapping) {
   if (nullptr != cell_mapping) {
      for (int idx=0; idx<cell_mapping->n(); idx++) cell_mapping[idx].clear();
   }
}

////////////////////////////////////////////////////////////////////////

static unixtime compute_unixtime(NcVar *time_var, unixtime var_value) {
   unixtime obs_time = bad_data_int;
   static const char *method_name = "compute_unixtime() -> ";

   if (IS_VALID_NC_P(time_var)) {
      int sec_per_unit;
      bool no_leap_year;
      unixtime ref_ut = get_reference_unixtime(time_var, sec_per_unit, no_leap_year);
      obs_time= add_to_unixtime(ref_ut, sec_per_unit, var_value, no_leap_year);
      mlog << Debug(3) << method_name << "valid time: " << var_value
           << " ==> " << unix_to_yyyymmdd_hhmmss(obs_time) << "\n";
   }

   return obs_time;
}

////////////////////////////////////////////////////////////////////////

GOES_QC convert_aod_qc_flag(int aod_qc) {
   GOES_QC aod_qc_flag;
   if (0 == aod_qc)      aod_qc_flag = GOES_QC::HIGH;
   else if (1 == aod_qc) aod_qc_flag = GOES_QC::MEDIUM;
   else if (2 == aod_qc) aod_qc_flag = GOES_QC::LOW;
   else aod_qc_flag = GOES_QC::NA;

   return aod_qc_flag;
}

////////////////////////////////////////////////////////////////////////

static bool get_grid_mapping(const Grid &to_grid, IntArray *cellMapping,
                             const IntArray obs_index_array, const int *obs_hids,
                             const float *hdr_lats, const float *hdr_lons) {
   bool status = false;
   clock_t start_clock =  clock();
   static const char *method_name = "get_grid_mapping(MET_obs) -> ";

   int obs_count = obs_index_array.n();
   if (0 == obs_count) {
      mlog << Warning << "\n" << method_name
           << "no valid point observation data!\n\n";
      return status;
   }

   double x;
   double y;
   DataPlane to_dp;
   int to_lat_count = to_grid.ny();
   int to_lon_count = to_grid.nx();
   int count_in_grid = 0;
   to_dp.set_size(to_lon_count, to_lat_count);
   for (int idx=0; idx<obs_count; idx++) {
      int obs_idx = obs_index_array[idx];
      int hdr_idx = obs_hids[obs_idx];
      float lat = hdr_lats[hdr_idx];
      float lon = hdr_lons[hdr_idx];
      if( lat < MISSING_LATLON || lon < MISSING_LATLON ) continue;
      to_grid.latlon_to_xy(lat, -1.0*lon, x, y);
      int idx_x = nint(x);
      int idx_y = nint(y);
      if (0 <= idx_x && idx_x < to_lon_count && 0 <= idx_y && idx_y < to_lat_count) {
         int to_offset = to_dp.two_to_one(idx_x, idx_y);
         cellMapping[to_offset].add(obs_idx);
         count_in_grid++;
      }
   }

   if (0 == count_in_grid)
      mlog << Warning << "\n" << method_name
           << "no valid point observation data within to grid\n\n";
   else {
      status = true;
      mlog << Debug(3) << method_name << "count in grid: " << count_in_grid
           << " out of " << obs_count << " ("
           << ((obs_count > 0) ? 1.0*count_in_grid/obs_count*100 : 0) << "%)\n";
   }
   mlog << Debug(LEVEL_FOR_PERFORMANCE) << method_name << "took "
        << get_exe_duration(start_clock) << " seconds\n";
   return status;
}


////////////////////////////////////////////////////////////////////////

static void get_grid_mapping_latlon(
      DataPlane from_dp, DataPlane to_dp, Grid to_grid,
      IntArray *cellMapping, float *latitudes, float *longitudes,
      int from_lat_count, int from_lon_count, bool *skip_times, bool to_north, bool is_2d) {
   double x;
   double y;
   double to_ll_lat;
   double to_ll_lon;
   int count_in_grid = 0;
   clock_t start_clock =  clock();
   int to_lat_count = to_grid.ny();
   int to_lon_count = to_grid.nx();
   int to_size  = to_lat_count * to_lon_count;
   int data_size  = from_lat_count * from_lon_count;
   static const char *method_name = "get_grid_mapping_latlon(lats, lons) -> ";

   auto to_cell_counts = new int[to_size];
   auto mapping_indices = new int[data_size];
   for (int xIdx=0; xIdx<to_size; xIdx++) to_cell_counts[xIdx] = 0;
   for (int xIdx=0; xIdx<data_size; xIdx++) mapping_indices[xIdx] = bad_data_int;

   to_grid.xy_to_latlon(0, 0, to_ll_lat, to_ll_lon);
   mlog << Debug(5) << method_name << " to_grid ll corner: (" << to_ll_lon << ", " << to_ll_lat << ")\n";

   //Count the number of cells to be mapped to TO_GRID
   //Following the logic at DataPlane::two_to_one(int x, int y) n = y*Nx + x;
   for (int yIdx=0; yIdx<from_lat_count; yIdx++) {
      int lat_offset = yIdx;
      for (int xIdx=0; xIdx<from_lon_count; xIdx++) {
         int lon_offset = xIdx;
         int coord_offset = from_dp.two_to_one(xIdx, yIdx, to_north);
         if (is_2d) {
            lon_offset = lat_offset = coord_offset;
            if( skip_times != 0 && skip_times[coord_offset] ) continue;
         }
         float lat = latitudes[lat_offset];
         float lon = longitudes[lon_offset];
         if( lat < MISSING_LATLON || lon < MISSING_LATLON ) continue;
         to_grid.latlon_to_xy(lat, -1.0*rescale_lon(lon), x, y);
         int idx_x = nint(x);
         int idx_y = nint(y);
         if (0 <= idx_x && idx_x < to_lon_count && 0 <= idx_y && idx_y < to_lat_count) {
            int to_offset = to_dp.two_to_one(idx_x, idx_y);
            mapping_indices[coord_offset] = to_offset;
            to_cell_counts[to_offset] += 1;
            count_in_grid++;
            if(mlog.verbosity_level() >= 15) {
               double to_lat;
               double to_lon;
               to_grid.xy_to_latlon(idx_x, idx_y, to_lat, to_lon);
               mlog << Debug(15) << method_name << " index: [" << xIdx << "," << yIdx << "] to " << coord_offset
                    << " (" << lon << ", " << lat << ") to (" << rescale_lon(-1*to_lon) << ", " << to_lat << ")\n";
            }
         }
      }
   }
   mlog << Debug(LEVEL_FOR_PERFORMANCE+2) << method_name << "took "
        << get_exe_duration(start_clock) << " seconds for mapping cells\n";

   // Count the mapping cells for each to_cell and prepare IntArray
   int max_count = 0;
   clock_t tmp_clock = clock();
   for (int xIdx=0; xIdx<to_size; xIdx++) {
      int cell_count = to_cell_counts[xIdx];
      if( cell_count > 0 ) {
         cellMapping[xIdx].extend(cell_count+1);
         if( cell_count > 32768 ) {
            mlog << Debug(LEVEL_FOR_PERFORMANCE+1) << method_name
                 << "extent IntArray to " << cell_count << " at " << xIdx << "\n";
         }
         if( max_count < cell_count ) max_count = cell_count;
      }
      else if( cell_count < 0 ) {
         mlog << Error << "\n" << method_name
              << "the mapping cell count can not be negative "
              << cell_count << " at " << xIdx << "\n\n";
         exit(1);
      }
   }
   mlog << Debug(LEVEL_FOR_PERFORMANCE+1) << method_name << "took "
        << get_exe_duration(tmp_clock)
        << " seconds for extending IntArray (max_cells=" << max_count << ")\n";

   // Build cell mapping
   for (int xIdx=0; xIdx<data_size; xIdx++) {
      int to_offset = mapping_indices[xIdx];
      if( is_bad_data(to_offset) ) continue;
      if( to_offset < 0 || to_offset >= to_size ) {
         mlog << Error << "\n" << method_name
              << "the mapped cell is out of range: "
              << to_offset << " at " << xIdx << "\n\n";
         exit(1);
      }
      else cellMapping[to_offset].add(xIdx);
   }
   delete [] to_cell_counts;
   delete [] mapping_indices;

   mlog << Debug(3) << method_name << "within grid: " << count_in_grid
        << " out of " << data_size << " (" << 1.0*count_in_grid/data_size*100 << "%)\n";
   mlog << Debug(LEVEL_FOR_PERFORMANCE) << method_name << "took "
        << get_exe_duration(start_clock) << " seconds\n";
}

////////////////////////////////////////////////////////////////////////

static bool get_grid_mapping(const Grid &fr_grid, const Grid &to_grid, IntArray *cellMapping,
                             NcVar var_lat, NcVar var_lon, bool *skip_times) {
   bool status = false;
   DataPlane from_dp;
   DataPlane to_dp;
   ConcatString cur_coord_name;
   clock_t start_clock =  clock();
   static const char *method_name = "get_grid_mapping(var_lat, var_lon) -> ";

   int to_lat_count = to_grid.ny();
   int to_lon_count = to_grid.nx();
   int from_lat_count = fr_grid.ny();
   int from_lon_count = fr_grid.nx();
   if (0 == from_lat_count) {
      int dim_offset = get_dim_count(&var_lat) - 1;
      if (dim_offset < 0) dim_offset = 0;
      from_lat_count = get_dim_size(&var_lat, dim_offset);
      dim_offset = get_dim_count(&var_lon) - 2;
      if (dim_offset < 0) dim_offset = 0;
      from_lon_count = get_dim_size(&var_lon, dim_offset);
   }
   // Override the from nx & ny from NetCDF if exists
   int data_size  = from_lat_count * from_lon_count;
   mlog << Debug(4) << method_name << "data_size (ny*nx): " << data_size
        << " = " << from_lat_count << " * " << from_lon_count << "\n"
        << "                    target grid (nx,ny): " << to_lon_count * to_lat_count
        << " = " << to_lon_count << " * " << to_lat_count << "\n";

   from_dp.set_size(from_lon_count, from_lat_count);
   to_dp.set_size(to_lon_count, to_lat_count);

   if (IS_INVALID_NC(var_lat)) {
      mlog << Error << "\n" << method_name
           << "Fail to get latitudes!\n\n";
      exit(1);
   }
   else if (IS_INVALID_NC(var_lon)) {
      mlog << Error << "\n" << method_name
           << "Fail to get longitudes!\n\n";
      exit(1);
   }
   else if (data_size > 0) {
      int last_idx = data_size - 1;
      int lat_count = get_data_size(&var_lat);
      int lon_count = get_data_size(&var_lon);
      auto latitudes  = new float[lat_count];
      auto longitudes = new float[lon_count];
      status = get_nc_data(&var_lat, latitudes);
      if( status ) status = get_nc_data(&var_lon, longitudes);
      if( status ) {
         get_grid_mapping_latlon(from_dp, to_dp, to_grid, cellMapping,
                                 latitudes, longitudes, from_lat_count,
                                 from_lon_count, skip_times,
                                 !fr_grid.get_swap_to_north(), (lon_count==data_size));

         if (is_eq(latitudes[0], latitudes[last_idx]) ||
             is_eq(longitudes[0], longitudes[last_idx])) {
            mlog << Warning << "\n" << method_name << "same latitude or longitude. lat[0]="
                 << latitudes[0] << " lat[" << last_idx << "]=" << latitudes[last_idx]
                 << " lon[0]=" << longitudes[0] << " lon[" << last_idx << "]="
                 << longitudes[last_idx] << "\n\n";
         }
      }
      if( latitudes )  delete [] latitudes;
      if( longitudes ) delete [] longitudes;
   }   //  if data_size > 0
   mlog << Debug(LEVEL_FOR_PERFORMANCE) << method_name << "took "
        << get_exe_duration(start_clock) << " seconds\n";
   return status;
}

////////////////////////////////////////////////////////////////////////

static unixtime find_valid_time(NcVar time_var) {
   unixtime valid_time = bad_data_int;
   static const char *method_name = "find_valid_time() -> ";

   if( IS_VALID_NC(time_var) || get_dim_count(&time_var) < 2) {
      int time_count = get_dim_size(&time_var, 0);

      double time_values [time_count + 1];
      if (get_nc_data(&time_var, time_values)) {
         valid_time = compute_unixtime(&time_var, time_values[0]);
      }
      else {
         mlog << Error << "\n" << method_name
              << "Can not read \"" << GET_NC_NAME(time_var)
              << "\" variable from \"" << InputFilename << "\"\n\n";
      }
   }

   if (is_bad_data(valid_time)) {
      mlog << Error << "\n" << method_name
           << "trouble finding time variable from \""
           << InputFilename << "\"\n\n";
      exit(1);
   }

   return valid_time;
}

////////////////////////////////////////////////////////////////////////

static ConcatString get_goes_grid_input(MetConfig config, const Grid fr_grid) {
   ConcatString run_string;
   ConcatString env_coord_name;
   ConcatString tmp_dir = config.get_tmp_dir();
   ConcatString geostationary_file(tmp_dir);
   geostationary_file.add("/");
   geostationary_file.add(make_geostationary_filename(fr_grid));
   if (get_env(key_geostationary_data, env_coord_name)
       && env_coord_name.nonempty()
       && file_exists(env_coord_name.c_str())) {
      run_string << env_coord_name;
   }
   else if (file_exists(geostationary_file.c_str())) {
      run_string << geostationary_file;
   }

   return run_string;
}

////////////////////////////////////////////////////////////////////////

static void get_grid_mapping(const Grid &fr_grid, const Grid &to_grid, IntArray *cellMapping,
                             const ConcatString &geostationary_file) {
   static const char *method_name = "get_grid_mapping() -> ";
   DataPlane from_dp;
   DataPlane to_dp;
   ConcatString cur_coord_name;

   clock_t start_clock =  clock();
   int to_lat_count = to_grid.ny();
   int to_lon_count = to_grid.nx();
   int from_lat_count = fr_grid.ny();
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
   auto coord_nc_in = (NcFile *) nullptr;
   if (has_coord_input) {
      mlog << Debug(2)  << method_name << "Reading coord file: " << cur_coord_name << "\n";
      coord_nc_in = open_ncfile(cur_coord_name.c_str());
      if (IS_VALID_NC_P(coord_nc_in)) {
         from_lat_count = get_lat_count(coord_nc_in);
         from_lon_count = get_lon_count(coord_nc_in);
      }
   }
   int data_size  = from_lat_count * from_lon_count;
   mlog << Debug(4) << method_name << "data_size (ny*nx): " << data_size
        << " = " << from_lat_count << " * " << from_lon_count << "\n"
        << "                    target grid (nx,ny): " << to_lon_count * to_lat_count
        << " = " << to_lon_count << " * " << to_lat_count << "\n";

   from_dp.set_size(from_lon_count, from_lat_count);
   to_dp.set_size(to_lon_count, to_lat_count);

   if (data_size > 0) {
      int lat_count = data_size;
      int lon_count = data_size;
      auto latitudes  = (float *)nullptr;
      auto longitudes = (float *)nullptr;
      auto latitudes_buf  = (float *)nullptr;
      auto longitudes_buf = (float *)nullptr;
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
               lat_count = get_data_size(&var_lat);
               lon_count = get_data_size(&var_lon);
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
               const float *tmp_lats = grid_data.lat_values;
               const float *tmp_lons = grid_data.lon_values;

               for (int idx=0; idx<data_size; idx++) {
                   if ((latitudes[idx] > MISSING_LATLON) && (tmp_lats[idx] > MISSING_LATLON)) {
                      if (!is_eq(latitudes[idx], tmp_lats[idx], loose_tol)) {
                         lat_mis_matching_count++;
                         mlog << Warning << "\n" << method_name
                              << "diff lat at " << idx << "  binary-computing: "
                              << latitudes[idx] << " - " << tmp_lats[idx] << " = "
                              << (latitudes[idx]-tmp_lats[idx]) << "\n\n";
                      }
                      else lat_matching_count++;
                   }
                   else lat_matching_count++;
                   if ((longitudes[idx] > MISSING_LATLON) && (tmp_lons[idx] > MISSING_LATLON)) {
                      if (!is_eq(longitudes[idx], tmp_lons[idx], loose_tol)) {
                         lon_mis_matching_count++;
                         mlog << Warning << "\n" << method_name
                              << "diff lon at " << idx << "  binary-computing: "
                              << longitudes[idx] << " - " << tmp_lons[idx] << " = "
                              << (longitudes[idx]-tmp_lons[idx]) << "\n\n";
                      }
                      else lon_matching_count++;
                   }
                   else lon_matching_count++;
               }
               if ((lon_mis_matching_count > 0) || (lat_mis_matching_count > 0)) {
                  mlog << Warning << "\n" << method_name
                       << "mis-matching: lon = " << lon_mis_matching_count
                       << "  lat =  " << lat_mis_matching_count << "   matched:  lon = "
                       << lon_matching_count << "  lat =  " << lat_matching_count << "\n\n";
               }
            }
         }
      }
      else if (fr_grid.info().gi) {
         grid_data.copy(fr_grid.info().gi);
         grid_data.compute_lat_lon();
         latitudes = grid_data.lat_values;
         longitudes = grid_data.lon_values;
         if (!file_exists(geostationary_file.c_str())) {
            save_geostationary_data(geostationary_file,
                  latitudes, longitudes, grid_data);
         }
      }
      if (nullptr == latitudes) {
         mlog << Error << "\n" << method_name
              << "Fail to get latitudes!\n\n";
      }
      else if (nullptr == longitudes) {
         mlog << Error << "\n" << method_name
              << "Fail to get longitudes!\n\n";
      }
      else {
         check_lat_lon(data_size, latitudes, longitudes);
         get_grid_mapping_latlon(from_dp, to_dp, to_grid, cellMapping, latitudes,
                                 longitudes, from_lat_count, from_lon_count, nullptr,
                                 !fr_grid.get_swap_to_north(), (lon_count==data_size));
      }

      if (latitudes_buf)  delete [] latitudes_buf;
      if (longitudes_buf) delete [] longitudes_buf;

      grid_data.release();

   }   //  if data_size > 0

   if(coord_nc_in) delete coord_nc_in;

   mlog << Debug(LEVEL_FOR_PERFORMANCE) << method_name << "took "
        << get_exe_duration(start_clock) << " seconds\n";
}

////////////////////////////////////////////////////////////////////////

static int get_lat_count(const NcFile *_nc) {
   int lat_count = 0;
   NcDim dim_lat = get_nc_dim(_nc, dim_name_lat);
   if(IS_INVALID_NC(dim_lat)) dim_lat = get_nc_dim(_nc, "y");
   if(IS_VALID_NC(dim_lat)) lat_count= get_dim_size(&dim_lat);
   return lat_count;
}

////////////////////////////////////////////////////////////////////////

static int get_lon_count(const NcFile *_nc) {
   int lon_count = 0;
   NcDim dim_lon = get_nc_dim(_nc, dim_name_lon);
   if(IS_INVALID_NC(dim_lon)) dim_lon = get_nc_dim(_nc, "x");
   if(IS_VALID_NC(dim_lon)) lon_count= get_dim_size(&dim_lon);
   return lon_count;
}

////////////////////////////////////////////////////////////////////////

static NcVar get_goes_nc_var(NcFile *nc, const ConcatString &var_name,
                             bool exit_if_error) {
   NcVar var_data;
   static const char *method_name = "get_goes_nc_var() -> ";
   if (has_var(nc, var_name.c_str())) var_data = get_nc_var(nc, var_name.c_str(), false);
   if (IS_INVALID_NC(var_data)) {
      mlog << Debug(4) << method_name
           << "The variable \"" << var_name << "\" does not exist. Find \""
           << var_name.split("_")[0] << "\" variable\n";
      var_data = get_nc_var(nc, var_name.split("_")[0].c_str());
   }
   if (IS_INVALID_NC(var_data)) {
      mlog << Error << "\n" << method_name
           << "The variable \"" << var_name << "\" does not exist\n\n";
      if (exit_if_error) exit(1);
   }
   return var_data;
}

////////////////////////////////////////////////////////////////////////


static ConcatString make_geostationary_filename(Grid fr_grid) {
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
   geo_data_filename << ".nc";
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
      mlog << Error << "\nis_time_mismatch() -> "
           << "The ADP input does not match with AOD input: "
           << log_message << "\n\n";
      exit(1);
   }
   return time_mismatch;
}

////////////////////////////////////////////////////////////////////////

static void regrid_goes_variable(NcFile *nc_in, const VarInfo *vinfo,
      DataPlane &fr_dp, DataPlane &to_dp,
      Grid fr_grid, Grid to_grid, IntArray *cellMapping, NcFile *nc_adp) {

   bool has_qc_var = false;
   bool has_adp_qc_var = false;
   const int log_debug_level = 4;
   clock_t start_clock =  clock();
   int to_lat_count = to_grid.ny();
   int to_lon_count = to_grid.nx();
   int from_lat_count = fr_grid.ny();
   int from_lon_count = fr_grid.nx();
   int from_data_size = from_lat_count * from_lon_count;
   ConcatString goes_var_name;
   ConcatString goes_var_sub_name;
   ConcatString qc_var_name;
   uchar qc_value;
   auto qc_data = new uchar[from_data_size];
   auto adp_data = new uchar[from_data_size];
   auto from_data = new float[from_data_size];
   auto adp_qc_data = new unsigned short[from_data_size];
   static const char *method_name = "regrid_goes_variable() -> ";

   // -99 is arbitrary number as invalid QC value
   memset(qc_data, -99, from_data_size*sizeof(uchar));

   adp_qc_high = 3;     /* 3 as baseline algorithm, 0 for enterprise algorithm */
   adp_qc_medium = 1;   /* 1 as baseline algorithm, 1 for enterprise algorithm */
   adp_qc_low = 0;      /* 0 as baseline algorithm, 2 for enterprise algorithm */

   NcVar var_qc;
   NcVar var_adp;
   NcVar var_adp_qc;
   NcVar var_data = get_goes_nc_var(nc_in, vinfo->name());
   bool is_dust_only = false;
   bool is_smoke_only = false;
   string actual_var_name = GET_NC_NAME(var_data);
   auto actual_var_len = actual_var_name.length();
   bool is_adp_variable = (0 != actual_var_name.compare(vinfo->name().c_str()));

   memset(adp_data, 1, from_data_size*sizeof(uchar));   // Default: 1 = data present
   memset(adp_qc_data, 255, from_data_size*sizeof(unsigned short));
   if (is_adp_variable && IS_VALID_NC_P(nc_adp)) {
      is_dust_only  = (0 == vinfo->name().comparecase((actual_var_len + 1),
            vname_dust.length(), vname_dust.c_str()));
      is_smoke_only = (0 == vinfo->name().comparecase((actual_var_len + 1),
            vname_smoke.length(), vname_smoke.c_str()));

      if      (is_dust_only)    var_adp = get_goes_nc_var(nc_adp, vname_dust);
      else if (is_smoke_only)   var_adp = get_goes_nc_var(nc_adp, vname_smoke);

      if (IS_VALID_NC(var_adp)) {
         get_nc_data(&var_adp, adp_data, true);

         //ADP Smoke:ancillary_variables: ubyte DQF(y, x)
         if (get_att_value_string(&var_adp, (string)"ancillary_variables", qc_var_name)) {
            var_adp_qc = get_nc_var(nc_adp, qc_var_name.c_str());
            if (IS_VALID_NC(var_adp_qc)) {
               get_nc_data(&var_adp_qc, adp_qc_data);
               set_adp_gc_values(var_adp_qc);
               has_adp_qc_var = true;
               mlog << Debug(5) << method_name << "found QC var: " << qc_var_name
                    << " for " << GET_NC_NAME(var_adp) << ".\n";
            }
         }
         else {
            mlog << Warning << "\n" << method_name
                 << "QC var name (" << qc_var_name
                 << " for " << GET_NC_NAME(var_adp)
                 << ") does not exist.\n\n";
         }
      }
   }
   mlog << Debug(5) << method_name << "is_dust: " << is_dust_only
        << ", is_smoke: " << is_smoke_only << "\n";

   //AOD ancillary_variables: byte DQF(y, x)
   if (get_att_value_string(&var_data, (string)"ancillary_variables", qc_var_name)) {
      var_qc = get_nc_var(nc_in, qc_var_name.c_str());
      if (IS_VALID_NC(var_qc)) {
         get_nc_data(&var_qc, qc_data);
         has_qc_var = true;
         mlog << Debug(3) << method_name << "found QC var: " << qc_var_name << ".\n";
      }
      else {
         mlog << Warning << "\n" << method_name
              << "QC var name (" << qc_var_name
              << ") does not exist.\n";
      }
   }

   get_nc_data(&var_data, from_data);

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
   float from_min_value =  10e10;
   float from_max_value = -10e10;
   float qc_min_value =  10e10;
   float qc_max_value = -10e10;
   IntArray cellArray;
   NumArray dataArray;
   bool has_qc_flags = (qc_flags.n() > 0);
   std::set<GOES_QC> aod_qc_flags = prepare_qoes_qc_array();

   missing_count = non_missing_count = 0;
   to_dp.set_constant(bad_data_double);

   int shift_bits = 2;
   if (is_dust_only) shift_bits += 2;

   int cnt_aod_qc_low = 0;
   int cnt_aod_qc_high = 0;
   int cnt_aod_qc_medium = 0;
   int cnt_aod_qc_nr = 0;   // no_retrieval_qf
   int cnt_adp_qc_low = 0;
   int cnt_adp_qc_high = 0;
   int cnt_adp_qc_medium = 0;
   int cnt_adp_qc_nr = 0;   // no_retrieval_qf
   int cnt_adp_qc_high_to_low = 0;
   int cnt_adp_qc_high_to_medium = 0;
   int cnt_adp_qc_medium_to_low = 0;
   for (int xIdx=0; xIdx<to_lon_count; xIdx++) {
      for (int yIdx=0; yIdx<to_lat_count; yIdx++) {
         int offset = to_dp.two_to_one(xIdx,yIdx);
         cellArray = cellMapping[offset];
         if (0 < cellArray.n()) {
            int valid_count = 0;
            dataArray.clear();
            dataArray.extend(cellArray.n());
            for (int dIdx=0; dIdx<cellArray.n(); dIdx++) {
               from_index = cellArray[dIdx];
               auto data_value = from_data[from_index];
               if (is_bad_data(data_value)) {
                  missing_count++;
                  continue;
               }
               non_missing_count++;
               if(mlog.verbosity_level() >= 4) {
                  if (from_min_value > data_value) from_min_value = data_value;
                  if (from_max_value < data_value) from_max_value = data_value;
               }

               // Apply censor threshold
               for(int i=0; i<vinfo->censor_thresh().n(); i++) {
                  // Break out after the first match.
                  if(vinfo->censor_thresh()[i].check(data_value)) {
                     data_value = vinfo->censor_val()[i];
                     censored_count++;
                     break;
                  }
               }

               // Check the data existance (always 1 if ADP variable does not exist)
               if (0 == adp_data[from_index]) {
                  absent_count++;
                  continue;
               }

               // Filter by QC flag
               if (has_qc_flags && (has_qc_var || has_adp_qc_var)) {
                  qc_value = qc_data[from_index];
                  GOES_QC aod_qc_flag = convert_aod_qc_flag(qc_value);
                  if (mlog.verbosity_level() >= log_debug_level) {
                     if (qc_min_value > qc_value) qc_min_value = qc_value;
                     if (qc_max_value < qc_value) qc_max_value = qc_value;
                     switch (aod_qc_flag) {
                        case GOES_QC::HIGH:   cnt_aod_qc_high++;    break;
                        case GOES_QC::MEDIUM: cnt_aod_qc_medium++;  break;
                        case GOES_QC::LOW:    cnt_aod_qc_low++;     break;
                        default: cnt_aod_qc_nr++;                   break;
                     }
                  }
                  if (has_adp_qc_var) {
                     GOES_QC adp_qc_flag = compute_adp_qc_flag(adp_qc_data[from_index], shift_bits);
                     if (mlog.verbosity_level() >= log_debug_level) {
                        switch (adp_qc_flag) {
                           case GOES_QC::HIGH:   cnt_adp_qc_high++;     break;
                           case GOES_QC::MEDIUM: cnt_adp_qc_medium++;   break;
                           case GOES_QC::LOW:    cnt_adp_qc_low++;      break;
                           default: cnt_adp_qc_nr++;                    break;
                        }
                     }

                     bool filter_out = GOES_QC::NA == adp_qc_flag;
                     if (!filter_out) {
                        /* Adjust the quality by AOD data QC */
                        if (GOES_QC::LOW == aod_qc_flag) {
                           if (GOES_QC::LOW != adp_qc_flag) {
                              if (GOES_QC::HIGH == adp_qc_flag) cnt_adp_qc_high_to_low++;
                              else if (GOES_QC::MEDIUM == adp_qc_flag) cnt_adp_qc_medium_to_low++;
                              adp_qc_flag = GOES_QC::LOW; /* high/medium to low quality */
                           }
                        }
                        else if (GOES_QC::MEDIUM == aod_qc_flag && GOES_QC::HIGH == adp_qc_flag) {
                           adp_qc_flag = GOES_QC::MEDIUM; /* high to medium quality */
                           cnt_adp_qc_high_to_medium++;
                        }
                        if (0 == aod_qc_flags.count(adp_qc_flag)) filter_out = true;
                     }
                     if (filter_out) {
                        adp_qc_filtered_count++;
                        continue;
                     }
                  }
                  else if (has_qc_var && 0 == aod_qc_flags.count(aod_qc_flag)) {
                     qc_filtered_count++;
                     continue;
                  }
               }
               dataArray.add(data_value);
               valid_count++;
            }

            if (0 < dataArray.n()) {
               int data_count = dataArray.n();
               double to_value;
               if      (RGInfo.method == InterpMthd::Min) to_value = dataArray.min();
               else if (RGInfo.method == InterpMthd::Max) to_value = dataArray.max();
               else if (RGInfo.method == InterpMthd::Median) {
                  dataArray.sort_array();
                  to_value = dataArray[data_count/2];
                  if (0 == data_count % 2)
                     to_value = (to_value + dataArray[(data_count/2)+1])/2;
               }
               else to_value = dataArray.sum() / data_count; // UW_Mean

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
      }
   }

   delete [] qc_data;
   delete [] adp_data;
   delete [] from_data;
   delete [] adp_qc_data;

   int cnt_adjused_low = cnt_adp_qc_low + cnt_adp_qc_high_to_low + cnt_adp_qc_medium_to_low;
   int cnt_adjused_high = cnt_adp_qc_high - cnt_adp_qc_high_to_medium - cnt_adp_qc_high_to_low;
   int cnt_adjused_medium = cnt_adp_qc_medium + cnt_adp_qc_high_to_medium - cnt_adp_qc_medium_to_low;
   int cnt_adjused_total = cnt_adp_qc_high_to_medium + cnt_adp_qc_high_to_low + cnt_adp_qc_medium_to_low;

   mlog << Debug(log_debug_level) << method_name << "Count: actual: " << to_cell_count
        << ", missing: " << missing_count << ", non_missing: " << non_missing_count
        << "\n   Filtered: by QC: " << qc_filtered_count
        << ", by adp QC: " << adp_qc_filtered_count
        << ", by absent: " << absent_count
        << ", total: " << (qc_filtered_count + adp_qc_filtered_count + absent_count)
        << "\n   Range:  data: [" << from_min_value << " - " << from_max_value
        << "]  QC: [" << qc_min_value << " - " << qc_max_value << "]\n";
   if (has_qc_flags) {
      mlog << Debug(log_debug_level)
           << "   AOD QC: high=" << cnt_aod_qc_high
           << " medium=" << cnt_aod_qc_medium << ", low=" << cnt_aod_qc_low
           << ", no_retrieval=" << cnt_aod_qc_nr
           << "\n   ADP QC: high=" << cnt_adjused_high << " (" << cnt_adp_qc_high
           << "), medium=" << cnt_adjused_medium  << " (" << cnt_adp_qc_medium
           << "), low=" << cnt_adjused_low << " (" << cnt_adp_qc_low
           << "), no_retrieval=" << cnt_adp_qc_nr
           << "\n   adjusted: high to medium=" << cnt_adp_qc_high_to_medium
           << ", high to low=" << cnt_adp_qc_high_to_low
           << ", medium to low=" << cnt_adp_qc_medium_to_low
           << ", total=" << cnt_adjused_total
           << "\n";
   }

   if (to_cell_count == 0) {
      mlog << Warning << "\n" << method_name
           << "No valid data!\n\n";
   }

   mlog << Debug(LEVEL_FOR_PERFORMANCE) << method_name << "took "
        << get_exe_duration(start_clock) << " seconds\n";
}

////////////////////////////////////////////////////////////////////////

static void save_geostationary_data(const ConcatString geostationary_file,
      const float *latitudes, const float *longitudes,
      const GoesImagerData &grid_data) {
   bool has_error = false;
   int deflate_level = 0;
   clock_t start_clock =  clock();
   static const char *method_name = "save_geostationary_data() -> ";

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
      if(!put_nc_data(&lat_var, latitudes)) {
         has_error = true;
         mlog << Warning << "\nsave_geostationary_data() -> "
              << "Cannot save latitudes!\n\n";
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
      if(!put_nc_data(&lon_var, longitudes)) {
         has_error = true;
         mlog << Warning << "\nsave_geostationary_data() -> "
              << "Cannot save longitudes!\n\n";
      }
   }

   add_att(nc_file, "Conventions", "CF-1.6");

   if (has_error) {
      remove(geostationary_file.c_str());
      mlog << Warning << "\nsave_geostationary_data() -> "
           << "The geostationary data file ("
           << geostationary_file << ") was not saved!\n";
   }
   else {
     mlog << Debug(3) << method_name << "The geostationary data file ("
          << geostationary_file << ") was saved\n";
   }

   delete nc_file;  nc_file = nullptr;

   mlog << Debug(LEVEL_FOR_PERFORMANCE) << method_name << "took "
        << get_exe_duration(start_clock) << " seconds\n";
}

////////////////////////////////////////////////////////////////////////

static void close_nc() {

   // Clean up
   if(nc_out) {
      delete nc_out; nc_out = (NcFile *) nullptr;
   }

   // List the output file
   mlog << Debug(1)
        << "Writing output file: " << OutputFilename << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

static bool keep_message_type(const int mt_index) {

   bool keep = false;

   keep = message_type_list.n() == 0 ||
          message_type_list.has(mt_index);

   return keep;
}

////////////////////////////////////////////////////////////////////////

static bool has_lat_lon_vars(const NcFile *nc) {

   bool has_lat_var = IS_VALID_NC(get_nc_var_lat(nc));
   bool has_lon_var = IS_VALID_NC(get_nc_var_lon(nc));
   bool has_time_var = IS_VALID_NC(get_nc_var_time(nc));

   //TODO: check if this is a gridded data or a point data here!!!
   mlog << Debug(7) << "has_lat_lon_vars() "
        << " has_lat_var: "  << has_lat_var
        << ", has_lon_var: " << has_lon_var
        << ", has_time_var: " << has_time_var << "\n";
   return (has_lat_var && has_lon_var);
}

////////////////////////////////////////////////////////////////////////

__attribute__((noreturn)) static void usage() {

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

        << "\twhere\t\"input_filename\" is the input NetCDF point observation "
        << "file to be read (required).\n"

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

static void set_adp_gc_values(NcVar var_adp_qc) {
   ConcatString att_flag_meanings;

   if (get_nc_att_value(&var_adp_qc, (ConcatString)"flag_meanings", att_flag_meanings)) {
      StringArray flag_meanings = to_lower(att_flag_meanings).split(" ");
      unsigned short flag_values[flag_meanings.n()+128];    /* reserve enough storage */
      for (int i=0; i<flag_meanings.n(); i++) flag_values[i] = (unsigned short)-1;

      if (get_nc_att_values(&var_adp_qc, att_name_values, flag_values)) {
         int idx;
         if (flag_meanings.has("low_confidence_smoke_detection_qf", idx)) {
            adp_qc_low = (flag_values[idx] >> 2) & 0x03;
         }
         if (flag_meanings.has("medium_confidence_smoke_detection_qf", idx)) {
            adp_qc_medium = (flag_values[idx] >> 2) & 0x03;
         }
         if (flag_meanings.has("high_confidence_smoke_detection_qf", idx)) {
            adp_qc_high = (flag_values[idx] >> 2) & 0x03;
         }
      }
      mlog << Debug(4) << "set_adp_gc_values() "
           << " high_confidence = "  << adp_qc_high
           << ", medium_confidence = " << adp_qc_medium
           << ", low_confidence = " << adp_qc_low << "\n";
   }
}

////////////////////////////////////////////////////////////////////////

static void set_field(const StringArray &a) {
   FieldSA.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

static void set_method(const StringArray &a) {
   InterpMthd method_id = string_to_interpmthd(a[0].c_str());
   if (method_id == InterpMthd::Gaussian || method_id == InterpMthd::MaxGauss ) {
      do_gaussian_filter = true;
      if (method_id == InterpMthd::MaxGauss) RGInfo.method = InterpMthd::Max;
   }
   else RGInfo.method = method_id;
   opt_override_method = true;
}

////////////////////////////////////////////////////////////////////////

static void set_prob_cat_thresh(const StringArray &a) {
   prob_cat_thresh.set(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

static void set_vld_thresh(const StringArray &a) {
   RGInfo.vld_thresh = atof(a[0].c_str());
   if(RGInfo.vld_thresh > 1 || RGInfo.vld_thresh < 0) {
      mlog << Error << "\nset_vld_thresh() -> "
           << "-vld_thresh may only be set between 0 and 1: "
           << RGInfo.vld_thresh << "\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

static void set_name(const StringArray & a) {
   VarNameSA.add_css(a[0]);
}

////////////////////////////////////////////////////////////////////////

static void set_config(const StringArray & a) {
   config_filename = a[0];
}

////////////////////////////////////////////////////////////////////////

static void set_compress(const StringArray & a) {
   compress_level = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

static void set_qc_flags(const StringArray & a) {
   int qc_flag;
   StringArray sa;

   sa.parse_css(a[0]);
   for (int idx=0; idx<sa.n(); idx++) {
      qc_flag = atoi(sa[idx].c_str());
      if ( !qc_flags.has(qc_flag) ) qc_flags.add(qc_flag);
   }
}

////////////////////////////////////////////////////////////////////////

static void set_adp(const StringArray & a) {
   adp_filename = a[0];
   if (!file_exists(adp_filename.c_str())) {
      mlog << Error << "\nset_adp() -> "
           << "\"" << adp_filename << "\" does not exist\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

static void set_gaussian_dx(const StringArray &a) {
   RGInfo.gaussian.dx = atof(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

static void set_gaussian_radius(const StringArray &a) {
   RGInfo.gaussian.radius = atof(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////
