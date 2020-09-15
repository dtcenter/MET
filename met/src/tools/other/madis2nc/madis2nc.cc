// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//
////////////////////////////////////////////////////////////////////////
//
//   Filename:   madis2nc.cc
//
//   Description:
//      Parse MADIS NetCDF files containing surface point observations
//      and reformat them for use by MET.  Current release provides
//      support for METAR, RAOB, PROFILER, MARITIME, MESONET, and
//      ACARSPROFILES MADIS types.
//      Support for additional MADIS types may be added.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    07-21-11  Halley Gotway  Adapted from contributed code.
//   001    01-06-12  Holmes         Added use of command line class to
//                                   parse the command line arguments.
//   002    07-13-12  Oldenburg      Added support for profiler
//                                   observation type.
//   003    02-26-14  Halley Gotway  Added support for mesonet
//                                   observation type.
//   004    07-07-14  Halley Gotway  Added the mask_grid and mask_poly
//                                   options to filter spatially.
//   005    10-01-15  Chaudhuri      Added support for acarsProfiles
//                                   observation type.
//   006    07-23-18  Halley Gotway  Support masks from gen_vx_mask.
//   007    01-11-19  Howard Soh     Added config file option.
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

#include <netcdf>
using namespace netCDF;

#include "madis2nc.h"

#include "data2d_factory.h"
#include "apply_mask.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

static const int FIELD_COUNT =  50;
static const int BUFFER_SIZE = OBS_BUFFER_SIZE / FIELD_COUNT;

static int nc_buf_size;
static NetcdfObsVars obs_vars;
static vector< Observation > obs_vector;
static vector< ConcatString > md_files;

////////////////////////////////////////////////////////////////////////

static void initialize();
static void process_command_line(int, char **);
static void process_madis_file(const char *);
static void clean_up();

static void setup_netcdf_out(int nhdr);

static bool get_filtered_nc_data(NcVar var, float *data, const long dim,
                                 const long cur, const char *var_name);
static bool get_filtered_nc_data_2d(NcVar var, int *data, const long *dim,
                                    const long *cur, const char *var_name, bool count_bad=false);
static bool get_filtered_nc_data_2d(NcVar var, float *data, const long *dim,
                                    const long *cur, const char *var_name, bool count_bad=false);

static void check_quality_control_flag(int &value, const char qty, const char *var_name);
static void check_quality_control_flag(float &value, const char qty, const char *var_name);

static int process_obs(const int gc, const float conversion,
                       float *obs_arr, char qty, const NcVar &,
                       const ConcatString &header_type,
                       const ConcatString &station_id,
                       const time_t valid_time, const double latitude,
                       const double longitude, const double elevation);
//static void write_qty(char &qty);

static MadisType get_madis_type(NcFile *&f_in);
static void      convert_wind_wdir_to_u_v(float wind, float wdir,
                                          float &u, float &v);
static bool      check_masks(double lat, double lon, const char *sid);

static void process_madis_metar(NcFile *&f_in);
static void process_madis_raob(NcFile *&f_in);
static void process_madis_profiler(NcFile *&f_in);
static void process_madis_maritime(NcFile *&f_in);
static void process_madis_mesonet(NcFile *&f_in);
static void process_madis_acarsProfiles(NcFile *&f_in);

static void usage();
static void set_type(const StringArray &);
static void set_qc_dd(const StringArray &);
static void set_lvl_dim(const StringArray &);
static void set_rec_beg(const StringArray &);
static void set_rec_end(const StringArray &);
static void set_logfile(const StringArray &);
static void set_mask_grid(const StringArray &);
static void set_mask_poly(const StringArray &);
static void set_mask_sid(const StringArray &);
static void set_verbosity(const StringArray &);
static void set_compress(const StringArray &);
static void set_config(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

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

   nc_obs_initialize();

   //
   // Process the MADIS file
   //
   for (vector< ConcatString >::const_iterator it_mdfile = md_files.begin();
       it_mdfile != md_files.end(); ++it_mdfile)
   {
     process_madis_file((*it_mdfile).c_str());
   }

   bool use_var_id = true;
   bool do_header = false;
   int nhdr = get_nc_hdr_cur_index();

   if (conf_info.getSummaryInfo().flag) {
      int summmary_hdr_cnt = 0;
      TimeSummaryInfo summaryInfo = conf_info.getSummaryInfo();
      summary_obs->summarizeObs(summaryInfo);
      summary_obs->setSummaryInfo(summaryInfo);
      summmary_hdr_cnt = summary_obs->countSummaryHeaders();
      if (save_summary_only)
         nhdr = summmary_hdr_cnt;
      else
         nhdr += summmary_hdr_cnt;
   }
   setup_netcdf_out(nhdr);

   write_observations(f_out, obs_vars, nc_out_data);

   //
   // Deallocate memory and clean up
   //
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void initialize() {

   mdfile.clear();
   ncfile.clear();
   mask_sid.clear();
   qc_dd_sa.clear();
   lvl_dim_sa.clear();
   i_obs    = 0;
   rej_fill = 0;
   rej_qc   = 0;
   rej_grid = 0;
   rej_poly = 0;
   rej_sid  = 0;

   summary_obs = new SummaryObs();
   return;
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;
   int i;

   //
   // check for zero arguments
   //
   if (argc == 1)
      usage();

   //
   // Store command line arguments to be written to output file.
   //
   argv_str = argv[0];
   for(i=1; i<argc; i++) argv_str << " " << argv[i];

   //
   // parse the command line into tokens
   //
   cline.set(argc, argv);

   //
   // set the usage function
   //
   cline.set_usage(usage);

   //
   // add the options function calls
   //
   cline.add(set_type,      "-type",      1);
   cline.add(set_qc_dd,     "-qc_dd",     1);
   cline.add(set_lvl_dim,   "-lvl_dim",   1);
   cline.add(set_rec_beg,   "-rec_beg",   1);
   cline.add(set_rec_end,   "-rec_end",   1);
   cline.add(set_logfile,   "-log",       1);
   cline.add(set_verbosity, "-v",         1);
   cline.add(set_mask_grid, "-mask_grid", 1);
   cline.add(set_mask_poly, "-mask_poly", 1);
   cline.add(set_mask_sid,  "-mask_sid",  1);
   cline.add(set_compress,  "-compress",  1);
   cline.add(set_config,    "-config",    1);

   //
   // parse the command line
   //
   cline.parse();

   //
   // Check for error. There should be two arguments left; the
   // madis input filename and the netCDF output filename.
   //
   if(cline.n() < 2) usage();

   //
   // Store the input MADIS file name and the output NetCDF file name
   //
   for (i = 0; i < cline.n() - 1; ++i)
     md_files.push_back((string)cline[i]);
   ncfile = cline[cline.n() - 1];

   conf_info.read_config(DEFAULT_CONFIG_FILENAME, config_filename.text());

   do_summary = conf_info.getSummaryInfo().flag;
   save_summary_only = false;
   if (do_summary) {
      save_summary_only = !conf_info.getSummaryInfo().raw_data;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_madis_file(const char *madis_file) {
   MadisType my_mtype = mtype;

   // Print out current file name
   mlog << Debug(1) << "Reading MADIS File:\t" << madis_file << "\n";

   // Open the input NetCDF file
   NcFile *f_in = open_ncfile(madis_file);

   // Check for a valid file
   if(IS_INVALID_NC_P(f_in)) {
      mlog << Error << "\nprocess_madis_file() -> "
           << "can't open input NetCDF file \"" << madis_file
           << "\" for reading.\n\n";
      delete f_in;
      f_in = (NcFile *) 0;

      exit(1);
   }
   // If the MADIS type is not already set, try to guess.
   if(my_mtype == madis_none) my_mtype = get_madis_type(f_in);

   // Switch on the MADIS type and process accordingly.
   switch(my_mtype) {
      case(madis_metar):
         process_madis_metar(f_in);
         break;
      case(madis_raob):
         process_madis_raob(f_in);
         break;
      case (madis_profiler):
         process_madis_profiler(f_in);
         break;
      case(madis_maritime):
         process_madis_maritime(f_in);
         break;

      case(madis_mesonet):
         process_madis_mesonet(f_in);
         break;

      case(madis_acarsProfiles):
         process_madis_acarsProfiles(f_in);
         break;

      case(madis_coop):
      case(madis_HDW):
      case(madis_HDW1h):
      case(madis_hydro):
      case(madis_POES):
      case(madis_acars):
      case(madis_radiometer):
      case(madis_sao):
      case(madis_satrad):
      case(madis_snow):
      case(madis_none):
      default:
         mlog << Error << "\nprocess_madis_file() -> "
              << "MADIS type (" << my_mtype
              << ") not currently supported.\n\n";
         exit(1);
         break;
   }

   // Close the input NetCDF file
   if(f_in) {
      delete f_in;
      f_in = (NcFile *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {

   if (summary_obs) delete summary_obs;
   
   //
   // Close the output NetCDF file
   //
   if(f_out) {
      delete f_out;
      f_out = (NcFile *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_netcdf_out(int nhdr) {

   //
   // Create the output netCDF file for writing
   //
   mlog << Debug(1) << "Writing MET File:\t" << ncfile << "\n";
   f_out = open_ncfile(ncfile.c_str(), true);

   //
   // Check for a valid file
   //
   if(IS_INVALID_NC_P(f_out)) {
      mlog << Error << "\nsetup_netcdf_out() -> "
           << "trouble opening output file: " << ncfile << "\n\n";
      delete f_out;
      f_out = (NcFile *) 0;
      exit(1);
   }

   bool use_var_id = false;
   init_nc_dims_vars_config(obs_vars, use_var_id);
   obs_vars.obs_cnt = obs_vector.size();
   mlog << Debug(5) << "setup_netcdf_out() nhdr:\t" << nhdr
        << "\tobs_cnt:\t" << obs_vars.obs_cnt << "\n";

   nc_out_data.processed_hdr_cnt = 0;
   nc_out_data.deflate_level = compress_level;
   nc_out_data.observations = obs_vector;
   nc_out_data.summary_obs = summary_obs;
   nc_out_data.summary_info = conf_info.getSummaryInfo();

   init_netcdf_output(f_out, obs_vars, nc_out_data, program_name);

   //
   // Add the command line arguments that were applied.
   //
   add_att(f_out, "RunCommand", (string)argv_str);

   return;
}

////////////////////////////////////////////////////////////////////////

static bool get_filtered_nc_data(NcVar var, float *data,
                                 const long dim, const long cur,
                                 const char *var_name) {

   bool status;
   float in_fill_value;
   const char *method_name = "get_filtered_nc_data(float) ";

   if (IS_VALID_NC(var)) {
      if(!(status = get_nc_data(&var, data, dim, cur))) return status;
      
      get_nc_att(&var, (string)in_fillValue_str, in_fill_value);
      for (int idx=0; idx<dim; idx++) {
         if(is_eq(data[idx], in_fill_value)) {
            data[idx] = bad_data_float;
            rej_fill++;
         }
      }
   }
   else {
      status = false;
      mlog << Error << "\n" << method_name
           << "Can not read a NetCDF data because the variable [" << var_name << "] is missing.\n\n";
   }
   return status;

}

////////////////////////////////////////////////////////////////////////

static bool get_filtered_nc_data_2d(NcVar var, int *data, const long *dim,
                                    const long *cur, const char *var_name,
                                    bool count_bad) {

   bool status;
   int in_fill_value;
   const char *method_name = "get_filtered_nc_data_2d(int)";

   if (IS_VALID_NC(var)) {
      if(!(status = get_nc_data(&var, data, dim, cur))) return status;
      
      get_nc_att(&var, (string)in_fillValue_str, in_fill_value);
      mlog << Debug(5)  << "    " << method_name << ": in_fill_value="
           << in_fill_value << "\n";
      
      int offset, offsetStart = 0;
      for (int idx=0; idx<dim[0]; idx++) {
         offsetStart = idx * dim[1];
         for (int vIdx=0; vIdx<dim[1]; vIdx++) {
            offset = offsetStart + vIdx;
      
            if(is_eq(data[offset], in_fill_value)) {
               data[offset] = bad_data_int;
               if(count_bad) {
                  rej_fill++;
               }
            }
         }
      }
   }
   else {
      status = false;
      mlog << Error << "\n" << method_name
           << "Can not read a NetCDF data because the variable [" << var_name << "] is missing.\n\n";
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

static bool get_filtered_nc_data_2d(NcVar var, float *data, const long *dim,
                                    const long *cur, const char *var_name,
                                    bool count_bad) {

   bool status;
   float in_fill_value;
   const char *method_name = "get_filtered_nc_data_2d(float) ";

   if (IS_VALID_NC(var)) {

      if(!(status = get_nc_data(&var, data, dim, cur))) return status;
      
      get_nc_att(&var, (string)in_fillValue_str, in_fill_value);
      mlog << Debug(5)  << method_name << "in_fill_value="
           << in_fill_value << "\n";
      
      int offset, offsetStart = 0;
      for (int idx=0; idx<dim[0]; idx++) {
         offsetStart = idx * dim[1];
         for (int vIdx=0; vIdx<dim[1]; vIdx++) {
            offset = offsetStart + vIdx;
      
            if(is_eq(data[offset], in_fill_value)) {
               data[offset] = bad_data_float;
               if(count_bad) {
                  rej_fill++;
               }
            }
         }
      }
   }
   else {
      status = false;
      mlog << Error << "\n" << method_name
           << "Can not read a NetCDF data because the variable [" << var_name << "] is missing.\n\n";
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

void check_quality_control_flag(int &value, const char qty, const char *var_name) {
   ConcatString dd_str;

   if (qty) {
      dd_str << cs_erase << qty;
   }
   else {
      dd_str << cs_erase << na_str;
   }

   //
   // Check quality control flag
   //
   if(!is_bad_data(value)  &&
      qc_dd_sa.n_elements() > 0 &&
      !qc_dd_sa.has(dd_str)) {
      value = bad_data_int;
      rej_qc++;
   }

   mlog << Debug(3)  << "    [" << (is_bad_data(value) ? "REJECT" : "ACCEPT") << "] " << var_name
        << ": value = " << value
        << ", qc = " << dd_str
        << "\n";
}

////////////////////////////////////////////////////////////////////////

void check_quality_control_flag(float &value, const char qty, const char *var_name) {
   ConcatString dd_str;

   if (qty) {
      dd_str << cs_erase << qty;
   }
   else {
      dd_str << cs_erase << na_str;
   }

   //
   // Check quality control flag
   //
   if(!is_bad_data(value)  &&
      qc_dd_sa.n_elements() > 0 &&
      !qc_dd_sa.has(dd_str)) {
      value = bad_data_float;
      rej_qc++;
   }

   mlog << Debug(3)  << "    [" << (is_bad_data(value) ? "REJECT" : "ACCEPT") << "] " << var_name
        << ": value = " << value
        << ", qc = " << dd_str
        << "\n";
}

////////////////////////////////////////////////////////////////////////

int process_obs(const int in_gc, const float conversion,
                float *obs_arr, char qty, const NcVar &nc_var,
                const ConcatString &header_type,
                const ConcatString &station_id,
                const time_t valid_time, const double latitude,
                const double longitude, const double elevation) {
   int cur_processed_count = 0;
   //
   // Check that the input variable contains valid data.
   //
   if(IS_INVALID_NC(nc_var)) return cur_processed_count;

   check_quality_control_flag(obs_arr[4], qty, GET_NC_NAME(nc_var).c_str());

   //
   // Store the GRIB code
   //
   obs_arr[1] = in_gc;

   //
   // Check for bad data and apply conversion factor
   //
   if(!is_bad_data(obs_arr[4])) {
      char  var_name[max_str_len];
      snprintf(var_name, sizeof(var_name), "GRIB_%d", (int) obs_arr[1]);

      obs_arr[4] *= conversion;

      ConcatString qty_str;
      if(qty == '\0') qty_str = na_str;
      else            qty_str << qty;
      qty_str.replace(" ", "_", false);

      Observation obs = Observation(
            header_type.text(),
            station_id.text(),
            valid_time,
            latitude, longitude, elevation,
            qty_str.text(),
            in_gc,
            obs_arr[2], obs_arr[3], obs_arr[4],
            var_name);

      obs_vector.push_back(obs);
      if (do_summary) summary_obs->addObservationObj(obs);

      i_obs++;
      cur_processed_count++;
   }

   return cur_processed_count;
}


////////////////////////////////////////////////////////////////////////

MadisType get_madis_type(NcFile *&f_in) {
   MadisType madis_type = madis_none;
   ConcatString attr_value;
   //
   // FUTURE WORK: Interrogate the MADIS file and determine it's type.
   //
   if (get_global_att(f_in, (string)"id", attr_value)) {
      if (attr_value == "MADIS_MARITIME")     madis_type = madis_maritime;
      else if (attr_value == "MADIS_MESONET") madis_type = madis_mesonet;
      else if (attr_value == "MADIS_METAR")   madis_type = madis_metar;
   }
   else if (get_global_att(f_in, (string)"title", attr_value)) {
      if (attr_value.contents("MADIS ACARS") != "") madis_type = madis_acarsProfiles;
   }
   return(madis_type);
}

////////////////////////////////////////////////////////////////////////

void convert_wind_wdir_to_u_v(float wind, float wdir,
                              float &u, float &v) {
   //
   // Convert wind direction and speed to U and V
   //
   if(is_bad_data(wind) || is_bad_data(wdir)) {
      u = v = bad_data_float;
   }
   else {
      u = (float) -1.0*wind*sind(wdir);
      v = (float) -1.0*wind*cosd(wdir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool check_masks(double lat, double lon, const char *sid) {
   double grid_x, grid_y;

   //
   // Check grid masking.
   //
   if(mask_grid.nx() > 0 || mask_grid.ny() > 0) {
      mask_grid.latlon_to_xy(lat, -1.0*lon, grid_x, grid_y);
      if(grid_x < 0 || grid_x >= mask_grid.nx() ||
         grid_y < 0 || grid_y >= mask_grid.ny()) {
         rej_grid++;
         return false;
      }

      //
      // Check area mask.
      //
      if(mask_area.nx() > 0 || mask_area.ny() > 0) {
         if(!mask_area.s_is_on(nint(grid_x), nint(grid_y))) {
            rej_poly++;
            return false;
         }
      }
   }

   //
   // Check polyline masking.
   //
   if(mask_poly.n_points() > 0) {
      if(!mask_poly.latlon_is_inside_dege(lat, lon)) {
         rej_poly++;
         return false;
      }
   }

   //
   // Check station ID masking.
   //
   if(mask_sid.n_elements() > 0) {
      if(!mask_sid.has(sid)) {
         rej_sid++;
         return false;
      }
   }

   return true;
}

////////////////////////////////////////////////////////////////////////

void print_rej_counts() {

   mlog << Debug(2)
        << "Rejected recs based on masking grid\t\t= " << rej_grid << "\n"
        << "Rejected recs based on masking poly\t\t= " << rej_poly << "\n"
        << "Rejected recs based on masking station ID's\t= " << rej_sid << "\n"
        << "Rejected obs  based on fill value\t\t= " << rej_fill << "\n"
        << "Rejected obs  based on quality control\t\t= " << rej_qc << "\n"
        << "Total observations retained or derived\t\t= " << i_obs << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void process_madis_metar(NcFile *&f_in) {
   int nhdr;
   long i_hdr, i_hdr_s;
   int hdr_typ_len, hdr_sid_len, i_idx;
   double tmp_dbl;
   time_t hdr_vld;
   ConcatString hdr_typ, hdr_sid;
   float hdr_arr[hdr_arr_len], obs_arr[obs_arr_len], conversion;
   float wdir, wind, ugrd, vgrd;
   int count;
   StringArray missing_vars, missing_qty_vars;
   const char *method_name = "process_madis_metar() ";

   //
   // Input header variables
   //
   NcVar in_hdr_typ_var = get_var(f_in, "reportType");
   NcVar in_hdr_sid_var = get_var(f_in, "stationName");
   NcVar in_hdr_vld_var = get_var(f_in, "timeObs");
   NcVar in_hdr_lat_var = get_var(f_in, "latitude");
   NcVar in_hdr_lon_var = get_var(f_in, "longitude");
   NcVar in_hdr_elv_var = get_var(f_in, "elevation");

   NcVar seaLevelPress_var = get_var(f_in, "seaLevelPress");
   NcVar visibility_var    = get_var(f_in, "visibility");
   NcVar temperature_var   = get_var(f_in, "temperature");
   NcVar dewpoint_var      = get_var(f_in, "dewpoint");
   NcVar windDir_var       = get_var(f_in, "windDir");
   NcVar windSpeed_var     = get_var(f_in, "windSpeed");
   NcVar windGust_var      = get_var(f_in, "windGust");
   NcVar minTemp24Hour_var = get_var(f_in, "minTemp24Hour");
   NcVar maxTemp24Hour_var = get_var(f_in, "maxTemp24Hour");
   NcVar precip1Hour_var   = get_var(f_in, "precip1Hour");
   NcVar precip3Hour_var   = get_var(f_in, "precip3Hour");
   NcVar precip6Hour_var   = get_var(f_in, "precip6Hour");
   NcVar precip24Hour_var  = get_var(f_in, "precip24Hour");
   NcVar snowCover_var     = get_var(f_in, "snowCover");

   NcVar seaLevelPressQty_var = get_var(f_in, "seaLevelPressDD");
   NcVar visibilityQty_var  = get_var(f_in, "visibilityDD");
   NcVar temperatureQty_var = get_var(f_in, "temperatureDD");
   NcVar dewpointQty_var    = get_var(f_in, "dewpointDD");
   NcVar windDirQty_var     = get_var(f_in, "windDirDD");
   NcVar windSpeedQty_var   = get_var(f_in, "windSpeedDD");
   NcVar windGustQty_var    = get_var(f_in, "windGustDD");
   NcVar minTemp24HourQty_var = get_var(f_in, "minTemp24HourDD");
   NcVar maxTemp24HourQty_var = get_var(f_in, "maxTemp24HourDD");
   NcVar precip1HourQty_var  = get_var(f_in, "precip1HourDD");
   NcVar precip3HourQty_var  = get_var(f_in, "precip3HourDD");
   NcVar precip6HourQty_var  = get_var(f_in, "precip6HourDD");
   NcVar precip24HourQty_var = get_var(f_in, "precip24HourDD");
   NcVar snowCoverQty_var    = get_var(f_in, "snowCoverDD");

   if (IS_INVALID_NC(in_hdr_typ_var)) missing_vars.add("reportType");
   if (IS_INVALID_NC(in_hdr_sid_var)) missing_vars.add("stationName");
   if (IS_INVALID_NC(in_hdr_vld_var)) missing_vars.add("timeObs");
   if (IS_INVALID_NC(in_hdr_lat_var)) missing_vars.add("latitude");
   if (IS_INVALID_NC(in_hdr_lon_var)) missing_vars.add("longitude");
   if (IS_INVALID_NC(in_hdr_elv_var)) missing_vars.add("elevation");

   if (IS_INVALID_NC(seaLevelPress_var)) missing_vars.add("seaLevelPress");
   if (IS_INVALID_NC(visibility_var   )) missing_vars.add("visibility");
   if (IS_INVALID_NC(temperature_var  )) missing_vars.add("temperature");
   if (IS_INVALID_NC(dewpoint_var     )) missing_vars.add("dewpoint");
   if (IS_INVALID_NC(windDir_var      )) missing_vars.add("windDir");
   if (IS_INVALID_NC(windSpeed_var    )) missing_vars.add("windSpeed");
   if (IS_INVALID_NC(windGust_var     )) missing_vars.add("windGust");
   if (IS_INVALID_NC(minTemp24Hour_var)) missing_vars.add("minTemp24Hour");
   if (IS_INVALID_NC(maxTemp24Hour_var)) missing_vars.add("maxTemp24Hour");
   if (IS_INVALID_NC(precip1Hour_var  )) missing_vars.add("precip1Hour");
   if (IS_INVALID_NC(precip3Hour_var  )) missing_vars.add("precip3Hour");
   if (IS_INVALID_NC(precip6Hour_var  )) missing_vars.add("precip6Hour");
   if (IS_INVALID_NC(precip24Hour_var )) missing_vars.add("precip24Hour");
   if (IS_INVALID_NC(snowCover_var    )) missing_vars.add("snowCover");

   if (IS_INVALID_NC(seaLevelPressQty_var)) missing_qty_vars.add("seaLevelPressDD");
   if (IS_INVALID_NC(visibilityQty_var   )) missing_qty_vars.add("visibilityDD");
   if (IS_INVALID_NC(temperatureQty_var  )) missing_qty_vars.add("temperatureDD");
   if (IS_INVALID_NC(dewpointQty_var     )) missing_qty_vars.add("dewpointDD");
   if (IS_INVALID_NC(windDirQty_var      )) missing_qty_vars.add("windDirDD");
   if (IS_INVALID_NC(windSpeedQty_var    )) missing_qty_vars.add("windSpeedDD");
   if (IS_INVALID_NC(windGustQty_var     )) missing_qty_vars.add("windGustDD");
   if (IS_INVALID_NC(minTemp24HourQty_var)) missing_qty_vars.add("minTemp24HourDD");
   if (IS_INVALID_NC(maxTemp24HourQty_var)) missing_qty_vars.add("maxTemp24HourDD");
   if (IS_INVALID_NC(precip1HourQty_var  )) missing_qty_vars.add("precip1HourDD");
   if (IS_INVALID_NC(precip3HourQty_var  )) missing_qty_vars.add("precip3HourDD");
   if (IS_INVALID_NC(precip6HourQty_var  )) missing_qty_vars.add("precip6HourDD");
   if (IS_INVALID_NC(precip24HourQty_var )) missing_qty_vars.add("precip24HourDD");
   if (IS_INVALID_NC(snowCoverQty_var    )) missing_qty_vars.add("snowCoverDD");

   if (missing_vars.n() > 0) {
      mlog << Error << "\n" << method_name << "Please check if the input is a METAR.\n\n";
      for (int idx=0; idx<missing_vars.n(); idx++)
         mlog << Warning << "    missing variable: " << missing_vars[idx] << "\n";
      exit(1);
   }
   if (missing_qty_vars.n() > 0) {
      for (int idx=0; idx<missing_qty_vars.n(); idx++)
         mlog << Debug(1) << "    missing Qty variable: " << missing_qty_vars[idx] << "\n";
   }

   //
   // Retrieve applicable dimensions
   //
   hdr_typ_len = get_dim_value(f_in, "maxRepLen");
   hdr_sid_len = get_dim_value(f_in, "maxStaNamLen");
   nhdr        = get_dim_value(f_in, in_recNum_str);
   int my_rec_end = (rec_end == 0) ? nhdr : rec_end;

   //
   // Setup the output NetCDF file
   //
   //setup_netcdf_out(nhdr);

   mlog << Debug(2) << "Processing METAR recs\t\t\t\t= " << my_rec_end - rec_beg << "\n";

   //
   // Initialize variables for processing observations
   //
   conversion = 1.0;

   //
   // Arrays of longs for indexing into NetCDF variables
   //
   long *cur = new long [2];
   cur[0] = cur[1] = 0;
   long *dim = new long [1];
   dim[0] = 1;

   int hdr_idx = 0;
   processed_count = 0;

   //
   // Loop through each record and get the header data.
   //
   for(i_hdr_s=rec_beg; i_hdr_s<my_rec_end; i_hdr_s+=BUFFER_SIZE) {
      long dim2D [2];
      int buf_size = ((my_rec_end - i_hdr_s) > BUFFER_SIZE) ? BUFFER_SIZE: (my_rec_end - i_hdr_s);
      dim[0] = buf_size;
      cur[0] = i_hdr_s;

      float hdr_lat_arr[buf_size];
      float hdr_lon_arr[buf_size];
      float hdr_elv_arr[buf_size];
      double tmp_dbl_arr[buf_size];

      float seaLevelPress[buf_size];
      float visibility[buf_size];
      float temperature[buf_size];
      float dewpoint[buf_size];
      float windDir[buf_size];
      float windSpeed[buf_size];
      float windGust[buf_size];
      float minTemp24Hour[buf_size];
      float maxTemp24Hour[buf_size];
      float precip1Hour[buf_size];
      float precip3Hour[buf_size];
      float precip6Hour[buf_size];
      float precip24Hour[buf_size];
      float snowCover[buf_size];

      char seaLevelPressQty[buf_size];
      char visibilityQty[buf_size];
      char temperatureQty[buf_size];
      char dewpointQty[buf_size];
      char windDirQty[buf_size];
      char windSpeedQty[buf_size];
      char windGustQty[buf_size];
      char minTemp24HourQty[buf_size];
      char maxTemp24HourQty[buf_size];
      char precip1HourQty[buf_size];
      char precip3HourQty[buf_size];
      char precip6HourQty[buf_size];
      char precip24HourQty[buf_size];
      char snowCoverQty[buf_size];

      char hdr_typ_arr[buf_size][hdr_typ_len];
      char hdr_sid_arr[buf_size][hdr_sid_len];

      nc_buf_size = buf_size * FIELD_COUNT;
      if (nc_buf_size > BUFFER_SIZE) nc_buf_size = BUFFER_SIZE;

      get_nc_data(&in_hdr_vld_var, &tmp_dbl_arr[0], buf_size, i_hdr_s);
      get_nc_data(&in_hdr_lat_var, &hdr_lat_arr[0], buf_size, i_hdr_s);
      get_nc_data(&in_hdr_lon_var, &hdr_lon_arr[0], buf_size, i_hdr_s);
      get_filtered_nc_data(in_hdr_elv_var, &hdr_elv_arr[0], buf_size, i_hdr_s, "elevation");

      if (!IS_INVALID_NC(seaLevelPressQty_var)) get_nc_data(&seaLevelPressQty_var, seaLevelPressQty, buf_size, i_hdr_s);
      else memset(seaLevelPressQty, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(visibilityQty_var))    get_nc_data(&visibilityQty_var, visibilityQty, buf_size, i_hdr_s);
      else memset(visibilityQty, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(temperatureQty_var))   get_nc_data(&temperatureQty_var, temperatureQty, buf_size, i_hdr_s);
      else memset(temperatureQty, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(dewpointQty_var))      get_nc_data(&dewpointQty_var, dewpointQty, buf_size, i_hdr_s);
      else memset(dewpointQty, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(windDirQty_var))       get_nc_data(&windDirQty_var, windDirQty, buf_size, i_hdr_s);
      else memset(windDirQty, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(windSpeedQty_var))     get_nc_data(&windSpeedQty_var, windSpeedQty, buf_size, i_hdr_s);
      else memset(windSpeedQty, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(windGustQty_var))      get_nc_data(&windGustQty_var, windGustQty, buf_size, i_hdr_s);
      else memset(windGustQty, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(minTemp24HourQty_var)) get_nc_data(&minTemp24HourQty_var, minTemp24HourQty, buf_size, i_hdr_s);
      else memset(minTemp24HourQty, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(maxTemp24HourQty_var)) get_nc_data(&maxTemp24HourQty_var, maxTemp24HourQty, buf_size, i_hdr_s);
      else memset(maxTemp24HourQty, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(precip1HourQty_var))   get_nc_data(&precip1HourQty_var, precip1HourQty, buf_size, i_hdr_s);
      else memset(precip1HourQty, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(precip3HourQty_var))   get_nc_data(&precip3HourQty_var, precip3HourQty, buf_size, i_hdr_s);
      else memset(precip3HourQty, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(precip6HourQty_var))   get_nc_data(&precip6HourQty_var, precip6HourQty, buf_size, i_hdr_s);
      else memset(precip6HourQty, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(precip24HourQty_var))  get_nc_data(&precip24HourQty_var, precip24HourQty, buf_size, i_hdr_s);
      else memset(precip24HourQty, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(snowCoverQty_var))     get_nc_data(&snowCoverQty_var, snowCoverQty, buf_size, i_hdr_s);
      else memset(snowCoverQty, 0, buf_size*sizeof(char));

      get_filtered_nc_data(seaLevelPress_var,  seaLevelPress, buf_size, i_hdr_s, "seaLevelPress" );
      get_filtered_nc_data(visibility_var,     visibility,    buf_size, i_hdr_s, "visibility"    );
      get_filtered_nc_data(temperature_var,    temperature,   buf_size, i_hdr_s, "temperature"   );
      get_filtered_nc_data(dewpoint_var,       dewpoint,      buf_size, i_hdr_s, "dewpoint"      );
      get_filtered_nc_data(windDir_var,        windDir,       buf_size, i_hdr_s, "windDir"       );
      get_filtered_nc_data(windSpeed_var,      windSpeed,     buf_size, i_hdr_s, "windSpeed"     );
      get_filtered_nc_data(windGust_var,       windGust,      buf_size, i_hdr_s, "windGust"      );
      get_filtered_nc_data(minTemp24Hour_var,  minTemp24Hour, buf_size, i_hdr_s, "minTemp24Hour" );
      get_filtered_nc_data(maxTemp24Hour_var,  maxTemp24Hour, buf_size, i_hdr_s, "maxTemp24Hour" );
      get_filtered_nc_data(precip1Hour_var,    precip1Hour,   buf_size, i_hdr_s, "precip1Hour"   );
      get_filtered_nc_data(precip3Hour_var,    precip3Hour,   buf_size, i_hdr_s, "precip3Hour"   );
      get_filtered_nc_data(precip6Hour_var,    precip6Hour,   buf_size, i_hdr_s, "precip6Hour"   );
      get_filtered_nc_data(precip24Hour_var,   precip24Hour,  buf_size, i_hdr_s, "precip24Hour"  );
      get_filtered_nc_data(snowCover_var,      snowCover,     buf_size, i_hdr_s, "snowCover"     );

      dim2D[0] = buf_size;
      dim2D[1] = hdr_typ_len;
      get_nc_data(&in_hdr_typ_var, (char *)&hdr_typ_arr[0], dim2D, cur);
      dim2D[1] = hdr_sid_len;
      get_nc_data(&in_hdr_sid_var, (char *)&hdr_sid_arr[0], dim2D, cur);

      for (i_idx=0; i_idx<buf_size; i_idx++) {

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

         count = 0;
         i_hdr = i_hdr_s + i_idx;

         mlog << Debug(3) << "Record Number: " << i_hdr << "\n";

         //
         // Use cur to index into the NetCDF variables.
         //

         //
         // Process the latitude, longitude, and elevation.
         //
         hdr_arr[0] = hdr_lat_arr[i_idx];
         hdr_arr[1] = hdr_lon_arr[i_idx];
         hdr_arr[2] = hdr_elv_arr[i_idx];

         //
         // Check masking regions.
         //
         if(!check_masks(hdr_arr[0], hdr_arr[1], hdr_sid_arr[i_idx])) continue;

         //
         // Process the header type.
         // For METAR or SPECI, encode as ADPSFC.
         // Otherwise, use value from file.
         //
         hdr_typ = hdr_typ_arr[i_idx];
         if(hdr_typ == metar_str || hdr_typ == "SPECI") hdr_typ = "ADPSFC";

         //
         // Process the station name.
         //
         hdr_sid = hdr_sid_arr[i_idx];

         //
         // Process the observation time.
         //
         tmp_dbl = tmp_dbl_arr[i_idx];
         if(is_bad_data(tmp_dbl)) continue;

         hdr_vld = (time_t)tmp_dbl;

         //
         // Initialize the observation array: hdr_id, gc, lvl, hgt, ob
         //
         obs_arr[0] = (float)hdr_idx;  // Index into header array
         obs_arr[2] = bad_data_float;  // Level: accum(sec) or pressure
         obs_arr[3] = bad_data_float;  // Height

         // Sea Level Pressure
         obs_arr[4] = seaLevelPress[i_idx];
         count += process_obs(2, conversion, obs_arr, seaLevelPressQty[i_idx],
                     seaLevelPress_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Visibility
         obs_arr[4] = visibility[i_idx];
         count += process_obs(20, conversion, obs_arr, visibilityQty[i_idx],
                     visibility_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Temperature
         obs_arr[4] = temperature[i_idx];
         count += process_obs(11, conversion, obs_arr, temperatureQty[i_idx],
                     temperature_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Dewpoint
         obs_arr[4] = dewpoint[i_idx];
         count += process_obs(17, conversion, obs_arr, dewpointQty[i_idx],
                     dewpoint_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Wind Direction
         obs_arr[4] = windDir[i_idx];
         count += process_obs(31, conversion, obs_arr, windDirQty[i_idx],
                     windDir_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);
         wdir = obs_arr[4];

         // Wind Speed
         obs_arr[4] = windSpeed[i_idx];
         count += process_obs(32, conversion, obs_arr, windSpeedQty[i_idx],
                     windSpeed_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);
         wind = obs_arr[4];

         // Convert the wind direction and speed into U and V components
         convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);

         // Write U-component of wind
         obs_arr[4] = ugrd;
         count += process_obs(33, conversion, obs_arr, windSpeedQty[i_idx],
                     windSpeed_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Write V-component of wind
         obs_arr[4] = vgrd;
         count += process_obs(34, conversion, obs_arr, windSpeedQty[i_idx],
                     windSpeed_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Wind Gust
         obs_arr[4] = windGust[i_idx];
         count += process_obs(180, conversion, obs_arr, windGustQty[i_idx],
                     windGust_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Min Temperature - 24 Hour
         obs_arr[4] = minTemp24Hour[i_idx];
         count += process_obs(16, conversion, obs_arr, minTemp24HourQty[i_idx],
                     minTemp24Hour_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Max Temperature - 24 Hour
         obs_arr[4] = maxTemp24Hour[i_idx];
         count += process_obs(15, conversion, obs_arr, maxTemp24HourQty[i_idx],
                     maxTemp24Hour_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         conversion = 1000.0;
         // Precipitation - 1 Hour
         obs_arr[2] = 1.0*sec_per_hour;
         obs_arr[4] = precip1Hour[i_idx];
         count += process_obs(61, conversion, obs_arr, precip1HourQty[i_idx],
                     precip1Hour_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Precipitation - 3 Hour
         obs_arr[2] = 3.0*sec_per_hour;
         obs_arr[4] = precip3Hour[i_idx];
         count += process_obs(61, conversion, obs_arr, precip3HourQty[i_idx],
                     precip3Hour_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Precipitation - 6 Hour
         obs_arr[2] = 6.0*sec_per_hour;
         obs_arr[4] = precip6Hour[i_idx];
         count += process_obs(61, conversion, obs_arr, precip6HourQty[i_idx],
                     precip6Hour_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Precipitation - 24 Hour
         obs_arr[2] = 24.0*sec_per_hour;
         obs_arr[4] = precip24Hour[i_idx];
         count += process_obs(61, conversion, obs_arr, precip24HourQty[i_idx],
                     precip24Hour_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         conversion = 1.0;
         // Snow Cover
         obs_arr[2] = bad_data_float;
         obs_arr[4] = snowCover[i_idx];
         //count += process_obs(66, conversion, obs_arr, snowCoverQty[i_idx],
         //            snowCover_var, hdr_typ, hdr_sid, hdr_vld,
         //            hdr_arr[0], hdr_arr[1], hdr_arr[2]);
         process_obs(66, conversion, obs_arr, snowCoverQty[i_idx],
            snowCover_var, hdr_typ, hdr_sid, hdr_vld,
            hdr_arr[0], hdr_arr[1], hdr_arr[2]);

      }

   } // end for i_hdr

   print_rej_counts();

   //
   // Cleanup
   //
   if(cur) { delete [] cur; cur = (long *) 0; }
   if(dim) { delete [] dim; dim = (long *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_madis_raob(NcFile *&f_in) {
   int nhdr, nlvl, i_lvl;
   long i_hdr, i_hdr_s;
   int hdr_sid_len;
   double tmp_dbl;
   char qty;
   time_t hdr_vld;
   ConcatString hdr_typ, hdr_sid;
   float hdr_arr[hdr_arr_len], obs_arr[obs_arr_len], conversion;
   float wdir, wind, ugrd, vgrd;
   int count;
   StringArray missing_vars, missing_qty_vars;
   const char *method_name = "process_madis_raob() ";

   int maxlvl_manLevel;
   int maxlvl_sigTLevel;
   int maxlvl_sigWLevel;
   int maxlvl_sigPresWLevel;
   int maxlvl_mTropNum;
   int maxlvl_mWndNum ;

   //
   // Input header variables:
   // Note: hdr_typ is always set to ADPUPA
   //
   NcVar in_hdr_sid_var = get_var(f_in, "staName");
   NcVar in_hdr_vld_var = get_var(f_in, "synTime");
   NcVar in_hdr_lat_var = get_var(f_in, "staLat");
   NcVar in_hdr_lon_var = get_var(f_in, "staLon");
   NcVar in_hdr_elv_var = get_var(f_in, "staElev");

   //
   // Variables for vertical level sizes
   //
   NcVar in_man_var    = get_var(f_in, "numMand");
   NcVar in_sigt_var   = get_var(f_in, "numSigT");
   NcVar in_sigw_var   = get_var(f_in, "numSigW");
   NcVar in_sigprw_var = get_var(f_in, "numSigPresW");
   NcVar in_trop_var   = get_var(f_in, "numTrop");
   NcVar in_maxw_var   = get_var(f_in, "numMwnd");

   NcVar prMan_var    = get_var(f_in, "prMan");
   NcVar htMan_var    = get_var(f_in, "htMan");
   NcVar tpMan_var    = get_var(f_in, "tpMan");
   NcVar tdMan_var    = get_var(f_in, "tdMan");
   NcVar wdMan_var    = get_var(f_in, "wdMan");
   NcVar wsMan_var    = get_var(f_in, "wsMan");
   NcVar prSigT_var   = get_var(f_in, "prSigT");
   NcVar tpSigT_var   = get_var(f_in, "tpSigT");
   NcVar tdSigT_var   = get_var(f_in, "tdSigT");
   NcVar htSigW_var   = get_var(f_in, "htSigW");
   NcVar wdSigW_var   = get_var(f_in, "wdSigW");
   NcVar wsSigW_var   = get_var(f_in, "wsSigW");
   NcVar prSigW_var   = get_var(f_in, "prSigW");
   NcVar wdSigPrW_var = get_var(f_in, "wdSigPrW");
   NcVar wsSigPrW_var = get_var(f_in, "wsSigPrW");
   NcVar prTrop_var   = get_var(f_in, "prTrop");
   NcVar tpTrop_var   = get_var(f_in, "tpTrop");
   NcVar tdTrop_var   = get_var(f_in, "tdTrop");
   NcVar wdTrop_var   = get_var(f_in, "wdTrop");
   NcVar wsTrop_var   = get_var(f_in, "wsTrop");
   NcVar prMaxW_var   = get_var(f_in, "prMaxW");
   NcVar wdMaxW_var   = get_var(f_in, "wdMaxW");
   NcVar wsMaxW_var   = get_var(f_in, "wsMaxW");

   NcVar prManQty_var    = get_var(f_in, "prManDD");
   NcVar htManQty_var    = get_var(f_in, "htManDD");
   NcVar tpManQty_var    = get_var(f_in, "tpManDD");
   NcVar tdManQty_var    = get_var(f_in, "tdManDD");
   NcVar wdManQty_var    = get_var(f_in, "wdManDD");
   NcVar wsManQty_var    = get_var(f_in, "wsManDD");
   NcVar prSigTQty_var   = get_var(f_in, "prSigTDD");
   NcVar tpSigTQty_var   = get_var(f_in, "tpSigTDD");
   NcVar tdSigTQty_var   = get_var(f_in, "tdSigTDD");
   NcVar htSigWQty_var   = get_var(f_in, "htSigWDD");
   NcVar wdSigWQty_var   = get_var(f_in, "wdSigWDD");
   NcVar wsSigWQty_var   = get_var(f_in, "wsSigWDD");
   NcVar prSigWQty_var   = get_var(f_in, "prSigWDD");
   NcVar wdSigPrWQty_var = get_var(f_in, "wdSigPrWDD");
   NcVar wsSigPrWQty_var = get_var(f_in, "wsSigPrWDD");
   NcVar prTropQty_var   = get_var(f_in, "prTropDD");
   NcVar tpTropQty_var   = get_var(f_in, "tpTropDD");
   NcVar tdTropQty_var   = get_var(f_in, "tdTropDD");
   NcVar wdTropQty_var   = get_var(f_in, "wdTropDD");
   NcVar wsTropQty_var   = get_var(f_in, "wsTropDD");
   NcVar prMaxWQty_var   = get_var(f_in, "prMaxWDD");
   NcVar wdMaxWQty_var   = get_var(f_in, "wdMaxWDD");
   NcVar wsMaxWQty_var   = get_var(f_in, "wsMaxWDD");

   if (IS_INVALID_NC(in_hdr_sid_var)) missing_vars.add("staName");
   if (IS_INVALID_NC(in_hdr_vld_var)) missing_vars.add("synTime");
   if (IS_INVALID_NC(in_hdr_lat_var)) missing_vars.add("staLat");
   if (IS_INVALID_NC(in_hdr_lon_var)) missing_vars.add("staLon");
   if (IS_INVALID_NC(in_hdr_elv_var)) missing_vars.add("staElev");

   if (IS_INVALID_NC(in_man_var   )) missing_vars.add("numMand");
   if (IS_INVALID_NC(in_sigt_var  )) missing_vars.add("numSigT");
   if (IS_INVALID_NC(in_sigw_var  )) missing_vars.add("numSigW");
   if (IS_INVALID_NC(in_sigprw_var)) missing_vars.add("numSigPresW");
   if (IS_INVALID_NC(in_trop_var  )) missing_vars.add("numTrop");
   if (IS_INVALID_NC(in_maxw_var  )) missing_vars.add("numMwnd");

   if (IS_INVALID_NC(prMan_var   )) missing_vars.add("prMan");
   if (IS_INVALID_NC(htMan_var   )) missing_vars.add("htMan");
   if (IS_INVALID_NC(tpMan_var   )) missing_vars.add("tpMan");
   if (IS_INVALID_NC(tdMan_var   )) missing_vars.add("tdMan");
   if (IS_INVALID_NC(wdMan_var   )) missing_vars.add("wdMan");
   if (IS_INVALID_NC(wsMan_var   )) missing_vars.add("wsMan");
   if (IS_INVALID_NC(prSigT_var  )) missing_vars.add("prSigT");
   if (IS_INVALID_NC(tpSigT_var  )) missing_vars.add("tpSigT");
   if (IS_INVALID_NC(tdSigT_var  )) missing_vars.add("tdSigT");
   if (IS_INVALID_NC(htSigW_var  )) missing_vars.add("htSigW");
   if (IS_INVALID_NC(wdSigW_var  )) missing_vars.add("wdSigW");
   if (IS_INVALID_NC(wsSigW_var  )) missing_vars.add("wsSigW");
   if (IS_INVALID_NC(prSigW_var  )) missing_vars.add("prSigW");
   if (IS_INVALID_NC(wdSigPrW_var)) missing_vars.add("wdSigPrW");
   if (IS_INVALID_NC(wsSigPrW_var)) missing_vars.add("wsSigPrW");
   if (IS_INVALID_NC(prTrop_var  )) missing_vars.add("prTrop");
   if (IS_INVALID_NC(tpTrop_var  )) missing_vars.add("tpTrop");
   if (IS_INVALID_NC(tdTrop_var  )) missing_vars.add("tdTrop");
   if (IS_INVALID_NC(wdTrop_var  )) missing_vars.add("wdTrop");
   if (IS_INVALID_NC(wsTrop_var  )) missing_vars.add("wsTrop");
   if (IS_INVALID_NC(prMaxW_var  )) missing_vars.add("prMaxW");
   if (IS_INVALID_NC(wdMaxW_var  )) missing_vars.add("wdMaxW");
   if (IS_INVALID_NC(wsMaxW_var  )) missing_vars.add("wsMaxW");

   if (IS_INVALID_NC(prManQty_var   )) missing_qty_vars.add("prManDD");
   if (IS_INVALID_NC(htManQty_var   )) missing_qty_vars.add("htManDD");
   if (IS_INVALID_NC(tpManQty_var   )) missing_qty_vars.add("tpManDD");
   if (IS_INVALID_NC(tdManQty_var   )) missing_qty_vars.add("tdManDD");
   if (IS_INVALID_NC(wdManQty_var   )) missing_qty_vars.add("wdManDD");
   if (IS_INVALID_NC(wsManQty_var   )) missing_qty_vars.add("wsManDD");
   if (IS_INVALID_NC(prSigTQty_var  )) missing_qty_vars.add("prSigTDD");
   if (IS_INVALID_NC(tpSigTQty_var  )) missing_qty_vars.add("tpSigTDD");
   if (IS_INVALID_NC(tdSigTQty_var  )) missing_qty_vars.add("tdSigTDD");
   if (IS_INVALID_NC(htSigWQty_var  )) missing_qty_vars.add("htSigWDD");
   if (IS_INVALID_NC(wdSigWQty_var  )) missing_qty_vars.add("wdSigWDD");
   if (IS_INVALID_NC(wsSigWQty_var  )) missing_qty_vars.add("wsSigWDD");
   if (IS_INVALID_NC(prSigWQty_var  )) missing_qty_vars.add("prSigWDD");
   if (IS_INVALID_NC(wdSigPrWQty_var)) missing_qty_vars.add("wdSigPrWDD");
   if (IS_INVALID_NC(wsSigPrWQty_var)) missing_qty_vars.add("wsSigPrWDD");
   if (IS_INVALID_NC(prTropQty_var  )) missing_qty_vars.add("prTropDD");
   if (IS_INVALID_NC(tpTropQty_var  )) missing_qty_vars.add("tpTropDD");
   if (IS_INVALID_NC(tdTropQty_var  )) missing_qty_vars.add("tdTropDD");
   if (IS_INVALID_NC(wdTropQty_var  )) missing_qty_vars.add("wdTropDD");
   if (IS_INVALID_NC(wsTropQty_var  )) missing_qty_vars.add("wsTropDD");
   if (IS_INVALID_NC(prMaxWQty_var  )) missing_qty_vars.add("prMaxWDD");
   if (IS_INVALID_NC(wdMaxWQty_var  )) missing_qty_vars.add("wdMaxWDD");
   if (IS_INVALID_NC(wsMaxWQty_var  )) missing_qty_vars.add("wsMaxWDD");

   if (missing_vars.n() > 0) {
      mlog << Error << "\n" << method_name << "Please check if the input is a RAOB.\n\n";
      for (int idx=0; idx<missing_vars.n(); idx++)
         mlog << Warning << "    missing variable: " << missing_vars[idx] << "\n";
      exit(1);
   }
   if (missing_qty_vars.n() > 0) {
      for (int idx=0; idx<missing_qty_vars.n(); idx++)
         mlog << Debug(1) << "    missing Qty variable: " << missing_qty_vars[idx] << "\n";
   }

   //
   // Retrieve applicable dimensions
   //
   hdr_sid_len  = get_dim_value(f_in, "staNameLen");
   nhdr         = get_dim_value(f_in, in_recNum_str);
   int my_rec_end = (rec_end == 0) ? nhdr : rec_end;

   get_dim(f_in, (string)"manLevel", maxlvl_manLevel);
   get_dim(f_in, (string)"sigTLevel", maxlvl_sigTLevel);
   get_dim(f_in, (string)"sigWLevel", maxlvl_sigWLevel);
   get_dim(f_in, (string)"sigPresWLevel", maxlvl_sigPresWLevel);
   get_dim(f_in, (string)"mTropNum", maxlvl_mTropNum);
   get_dim(f_in, (string)"mWndNum", maxlvl_mWndNum);

   //
   // Setup the output NetCDF file
   //
   //setup_netcdf_out(nhdr);

   mlog << Debug(2) << "Processing RAOB recs\t\t\t= " << my_rec_end - rec_beg << "\n";

   //
   // Initialize variables for processing observations
   //
   conversion = 1.0;

   //
   // Arrays of longs for indexing into NetCDF variables
   //
   long *cur = new long [3];
   cur[0] = cur[1] = cur[2] = 0;
   long *dim = new long [3];
   dim[0] = dim[1] = dim[2] = 1;

   int hdr_idx = 0;
   processed_count = 0;

   //
   // Process the header type.: For RAOB, store as ADPUPA.
   //
   hdr_typ = "ADPUPA";

   //
   // Loop through each record and get the header data.
   //
   for(i_hdr_s=rec_beg; i_hdr_s<my_rec_end; i_hdr_s+=BUFFER_SIZE) {
      long dim2D [2];
      long dim3D [3];
      int buf_size = ((my_rec_end - i_hdr_s) > BUFFER_SIZE) ? BUFFER_SIZE: (my_rec_end - i_hdr_s);

      int nlvl_manLevel[buf_size];
      int nlvl_sigTLevel[buf_size];
      int nlvl_sigWLevel[buf_size];
      int nlvl_sigPresWLevel[buf_size];
      int nlvl_mTropNum[buf_size];
      int nlvl_mWndNum[buf_size];

      float hdr_lat_arr[buf_size];
      float hdr_lon_arr[buf_size];
      float hdr_elv_arr[buf_size];
      double tmp_dbl_arr[buf_size];

      float prMan[buf_size][maxlvl_manLevel];
      float htMan[buf_size][maxlvl_manLevel];
      float tpMan[buf_size][maxlvl_manLevel];
      float tdMan[buf_size][maxlvl_manLevel];
      float wdMan[buf_size][maxlvl_manLevel];
      float wsMan[buf_size][maxlvl_manLevel];
      float prSigT[buf_size][maxlvl_sigTLevel];
      float tpSigT[buf_size][maxlvl_sigTLevel];
      float tdSigT[buf_size][maxlvl_sigTLevel];
      float htSigW[buf_size][maxlvl_sigWLevel];
      float wdSigW[buf_size][maxlvl_sigWLevel];
      float wsSigW[buf_size][maxlvl_sigWLevel];
      float prSigW[buf_size][maxlvl_sigPresWLevel];
      float wdSigPrW[buf_size][maxlvl_sigPresWLevel];
      float wsSigPrW[buf_size][maxlvl_sigPresWLevel];
      float prTrop[buf_size][maxlvl_mTropNum];
      float tpTrop[buf_size][maxlvl_mTropNum];
      float tdTrop[buf_size][maxlvl_mTropNum];
      float wdTrop[buf_size][maxlvl_mTropNum];
      float wsTrop[buf_size][maxlvl_mTropNum];
      float prMaxW[buf_size][maxlvl_mWndNum];
      float wdMaxW[buf_size][maxlvl_mWndNum];
      float wsMaxW[buf_size][maxlvl_mWndNum];

      char prManQty[buf_size][maxlvl_manLevel];
      char htManQty[buf_size][maxlvl_manLevel];
      char tpManQty[buf_size][maxlvl_manLevel];
      char tdManQty[buf_size][maxlvl_manLevel];
      char wdManQty[buf_size][maxlvl_manLevel];
      char wsManQty[buf_size][maxlvl_manLevel];
      char prSigTQty[buf_size][maxlvl_sigTLevel];
      char tpSigTQty[buf_size][maxlvl_sigTLevel];
      char tdSigTQty[buf_size][maxlvl_sigTLevel];
      char htSigWQty[buf_size][maxlvl_sigWLevel];
      char wdSigWQty[buf_size][maxlvl_sigWLevel];
      char wsSigWQty[buf_size][maxlvl_sigWLevel];
      char prSigWQty[buf_size][maxlvl_sigPresWLevel];
      char wdSigPrWQty[buf_size][maxlvl_sigPresWLevel];
      char wsSigPrWQty[buf_size][maxlvl_sigPresWLevel];
      char prTropQty[buf_size][maxlvl_mTropNum];
      char tpTropQty[buf_size][maxlvl_mTropNum];
      char tdTropQty[buf_size][maxlvl_mTropNum];
      char wdTropQty[buf_size][maxlvl_mTropNum];
      char wsTropQty[buf_size][maxlvl_mTropNum];
      char prMaxWQty[buf_size][maxlvl_mWndNum];
      char wdMaxWQty[buf_size][maxlvl_mWndNum];
      char wsMaxWQty[buf_size][maxlvl_mWndNum];

      //char hdr_typ_arr[buf_size][hdr_typ_len];
      char hdr_sid_arr[buf_size][hdr_sid_len];
      //char *hdr_typ_arr_ptr = &hdr_typ_arr[0];
      //char *hdr_sid_arr_ptr = &hdr_sid_arr[0];

      nc_buf_size = buf_size * FIELD_COUNT;
      if (nc_buf_size > BUFFER_SIZE) nc_buf_size = BUFFER_SIZE;

      cur[0] = i_hdr_s;

      dim[0] = buf_size;

      get_nc_data(&in_man_var,    nlvl_manLevel,      buf_size, i_hdr_s);
      get_nc_data(&in_sigt_var,   nlvl_sigTLevel,     buf_size, i_hdr_s);
      get_nc_data(&in_sigw_var,   nlvl_sigWLevel,     buf_size, i_hdr_s);
      get_nc_data(&in_sigprw_var, nlvl_sigPresWLevel, buf_size, i_hdr_s);
      get_nc_data(&in_trop_var,   nlvl_mTropNum,      buf_size, i_hdr_s);
      get_nc_data(&in_maxw_var,   nlvl_mWndNum,       buf_size, i_hdr_s);

      get_nc_data(&in_hdr_vld_var, tmp_dbl_arr, buf_size, i_hdr_s);
      get_nc_data(&in_hdr_lat_var, hdr_lat_arr, buf_size, i_hdr_s);
      get_nc_data(&in_hdr_lon_var, hdr_lon_arr, buf_size, i_hdr_s);
      get_filtered_nc_data(in_hdr_elv_var, hdr_elv_arr, buf_size, i_hdr_s, "eleveation");

      dim2D[0] = buf_size;
      //dim2D[1] = hdr_typ_len;
      //get_nc_data(in_hdr_typ_var, (char *)&hdr_typ_arr[0], dim2D, cur);
      dim2D[1] = hdr_sid_len;
      get_nc_data(&in_hdr_sid_var, (char *)&hdr_sid_arr, dim2D, cur);

      dim3D[0] = buf_size;
      dim3D[1] = maxlvl_manLevel;
      if (!IS_INVALID_NC(prManQty_var)) get_nc_data(&prManQty_var, (char *)&prManQty, dim3D, cur);
      else memset(prManQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(htManQty_var)) get_nc_data(&htManQty_var, (char *)&htManQty, dim3D, cur);
      else memset(htManQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(tpManQty_var)) get_nc_data(&tpManQty_var, (char *)&tpManQty, dim3D, cur);
      else memset(tpManQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(tdManQty_var)) get_nc_data(&tdManQty_var, (char *)&tdManQty, dim3D, cur);
      else memset(tdManQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(wdManQty_var)) get_nc_data(&wdManQty_var, (char *)&wdManQty, dim3D, cur);
      else memset(wdManQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(wsManQty_var)) get_nc_data(&wsManQty_var, (char *)&wsManQty, dim3D, cur);
      else memset(wsManQty, 0, buf_size*dim3D[1]*sizeof(char));
      dim3D[1] = maxlvl_sigTLevel;
      if (!IS_INVALID_NC(prSigTQty_var)) get_nc_data(&prSigTQty_var, (char *)&prSigTQty, dim3D, cur);
      else memset(prSigTQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(tpSigTQty_var)) get_nc_data(&tpSigTQty_var, (char *)&tpSigTQty, dim3D, cur);
      else memset(tpSigTQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(tdSigTQty_var)) get_nc_data(&tdSigTQty_var, (char *)&tdSigTQty, dim3D, cur);
      else memset(tdSigTQty, 0, buf_size*dim3D[1]*sizeof(char));
      dim3D[1] = maxlvl_sigWLevel;
      if (!IS_INVALID_NC(htSigWQty_var)) get_nc_data(&htSigWQty_var, (char *)&htSigWQty, dim3D, cur);
      else memset(htSigWQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(wdSigWQty_var)) get_nc_data(&wdSigWQty_var, (char *)&wdSigWQty, dim3D, cur);
      else memset(wdSigWQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(wsSigWQty_var)) get_nc_data(&wsSigWQty_var, (char *)&wsSigWQty, dim3D, cur);
      else memset(wsSigWQty, 0, buf_size*dim3D[1]*sizeof(char));
      dim3D[1] = maxlvl_sigPresWLevel;
      if (!IS_INVALID_NC(prSigWQty_var  )) get_nc_data(&prSigWQty_var  ,   (char *)&prSigWQty, dim3D, cur);
      else memset(prSigWQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(wdSigPrWQty_var)) get_nc_data(&wdSigPrWQty_var, (char *)&wdSigPrWQty, dim3D, cur);
      else memset(wdSigPrWQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(wsSigPrWQty_var)) get_nc_data(&wsSigPrWQty_var, (char *)&wsSigPrWQty, dim3D, cur);
      else memset(wsSigPrWQty, 0, buf_size*dim3D[1]*sizeof(char));
      dim3D[1] = maxlvl_mTropNum;
      if (!IS_INVALID_NC(prTropQty_var)) get_nc_data(&prTropQty_var, (char *)&prTropQty, dim3D, cur);
      else memset(prTropQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(tpTropQty_var)) get_nc_data(&tpTropQty_var, (char *)&tpTropQty, dim3D, cur);
      else memset(tpTropQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(tdTropQty_var)) get_nc_data(&tdTropQty_var, (char *)&tdTropQty, dim3D, cur);
      else memset(tdTropQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(wdTropQty_var)) get_nc_data(&wdTropQty_var, (char *)&wdTropQty, dim3D, cur);
      else memset(wdTropQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(wsTropQty_var)) get_nc_data(&wsTropQty_var, (char *)&wsTropQty, dim3D, cur);
      else memset(wsTropQty, 0, buf_size*dim3D[1]*sizeof(char));
      dim3D[1] = maxlvl_mWndNum;
      if (!IS_INVALID_NC(prMaxWQty_var)) get_nc_data(&prMaxWQty_var, (char *)&prMaxWQty, dim3D, cur);
      else memset(prMaxWQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(wdMaxWQty_var)) get_nc_data(&wdMaxWQty_var, (char *)&wdMaxWQty, dim3D, cur);
      else memset(wdMaxWQty, 0, buf_size*dim3D[1]*sizeof(char));
      if (!IS_INVALID_NC(wsMaxWQty_var)) get_nc_data(&wsMaxWQty_var, (char *)&wsMaxWQty, dim3D, cur);
      else memset(wsMaxWQty, 0, buf_size*dim3D[1]*sizeof(char));
      
      dim3D[1] = maxlvl_manLevel;
      get_filtered_nc_data_2d(prMan_var, (float *)&prMan[0], dim3D, cur, "prMan");
      get_filtered_nc_data_2d(htMan_var, (float *)&htMan[0], dim3D, cur, "htMan");
      get_filtered_nc_data_2d(tpMan_var, (float *)&tpMan[0], dim3D, cur, "tpMan");
      get_filtered_nc_data_2d(tdMan_var, (float *)&tdMan[0], dim3D, cur, "tdMan");
      get_filtered_nc_data_2d(wdMan_var, (float *)&wdMan[0], dim3D, cur, "wdMan");
      get_filtered_nc_data_2d(wsMan_var, (float *)&wsMan[0], dim3D, cur, "wsMan");
      dim3D[1] = maxlvl_sigTLevel;
      get_filtered_nc_data_2d(prSigT_var, (float *)&prSigT, dim3D, cur, "prSigT");
      get_filtered_nc_data_2d(tpSigT_var, (float *)&tpSigT, dim3D, cur, "tpSigT");
      get_filtered_nc_data_2d(tdSigT_var, (float *)&tdSigT, dim3D, cur, "tdSigT");
      dim3D[1] = maxlvl_sigWLevel;
      get_filtered_nc_data_2d(htSigW_var, (float *)&htSigW, dim3D, cur, "htSigW");
      get_filtered_nc_data_2d(wdSigW_var, (float *)&wdSigW, dim3D, cur, "wdSigW");
      get_filtered_nc_data_2d(wsSigW_var, (float *)&wsSigW, dim3D, cur, "wsSigW");
      dim3D[1] = maxlvl_sigPresWLevel;
      get_filtered_nc_data_2d(prSigW_var  ,   (float *)&prSigW, dim3D, cur,   "prSigW");
      get_filtered_nc_data_2d(wdSigPrW_var, (float *)&wdSigPrW, dim3D, cur, "wdSigPrW");
      get_filtered_nc_data_2d(wsSigPrW_var, (float *)&wsSigPrW, dim3D, cur, "wsSigPrW");
      dim3D[1] = maxlvl_mTropNum;
      get_filtered_nc_data_2d(prTrop_var, (float *)&prTrop, dim3D, cur, "prTrop");
      get_filtered_nc_data_2d(tpTrop_var, (float *)&tpTrop, dim3D, cur, "tpTrop");
      get_filtered_nc_data_2d(tdTrop_var, (float *)&tdTrop, dim3D, cur, "tdTrop");
      get_filtered_nc_data_2d(wdTrop_var, (float *)&wdTrop, dim3D, cur, "wdTrop");
      get_filtered_nc_data_2d(wsTrop_var, (float *)&wsTrop, dim3D, cur, "wsTrop");
      dim3D[1] = maxlvl_mWndNum;
      get_filtered_nc_data_2d(prMaxW_var, (float *)&prMaxW, dim3D, cur, "prMaxW");
      get_filtered_nc_data_2d(wdMaxW_var, (float *)&wdMaxW, dim3D, cur, "wdMaxW");
      get_filtered_nc_data_2d(wsMaxW_var, (float *)&wsMaxW, dim3D, cur, "wsMaxW");

      dim[0] = 1;

      for (int i_idx=0; i_idx<buf_size; i_idx++) {

         //
         // Mapping of NetCDF variable names from input to output:
         // Output                    = Input
         // hdr_typ                   = NA - always set to ADPUPA
         // hdr_sid                   = staName (staNameLen = 50)
         // hdr_vld (YYYYMMDD_HHMMSS) = synTime (unixtime) - synoptic time
         // hdr_arr[0](Lat)           = staLat
         // hdr_arr[1](Lon)           = staLon
         // hdr_arr[2](Elv)           = staElev
         //

         count = 0;
         i_hdr = i_hdr_s + i_idx;
         mlog << Debug(3) << "Record Number: " << i_hdr << "\n";

         //
         // Use cur to index into the NetCDF variables.
         //
         cur[0] = i_hdr;
         cur[1] = 0;

         //
         // Process the latitude, longitude, and elevation.
         //
         hdr_arr[0] = hdr_lat_arr[i_idx];
         hdr_arr[1] = hdr_lon_arr[i_idx];
         hdr_arr[2] = hdr_elv_arr[i_idx];

         //
         // Check masking regions.
         //
         if(!check_masks(hdr_arr[0], hdr_arr[1], hdr_sid_arr[i_idx])) continue;

         //
         // Process the observation time.
         //
         tmp_dbl = tmp_dbl_arr[i_idx];
         if(is_bad_data(tmp_dbl)) continue;

         hdr_vld = (time_t)tmp_dbl;

         hdr_idx = get_nc_hdr_cur_index();

         //
         // Process the station name.
         //
         hdr_sid = hdr_sid_arr[i_idx];

         //
         // Initialize the observation array: hdr_id
         //
         obs_arr[0] = (float) (hdr_idx);


         //
         // Loop through the mandatory levels
         //
         nlvl = nlvl_manLevel[i_idx];
         for(i_lvl=0; i_lvl<nlvl; i_lvl++) {

            mlog << Debug(3) << "  Mandatory Level: " << i_lvl << "\n";

            //
            // Use cur to index into the NetCDF variables.
            //
            cur[1] = i_lvl;

            //
            // Get the pressure and height for this level
            //
            obs_arr[2] = prMan[i_idx][i_lvl];
            obs_arr[3] = htMan[i_idx][i_lvl];

            //
            // Check for bad data
            //
            if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;

            // Pressure
            obs_arr[4] = prMan[i_idx][i_lvl];
            count += process_obs(1, conversion, obs_arr, prManQty[i_idx][i_lvl],
                        prMan_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

            // Height
            obs_arr[4] = htMan[i_idx][i_lvl];
            count += process_obs(7, conversion, obs_arr, htManQty[i_idx][i_lvl],
                        htMan_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

            // Temperature
            obs_arr[4] = tpMan[i_idx][i_lvl];
            count += process_obs(11, conversion, obs_arr, tpManQty[i_idx][i_lvl],
                        tpMan_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

            // Dewpoint
            obs_arr[4] = tdMan[i_idx][i_lvl];
            count += process_obs(17, conversion, obs_arr, tdManQty[i_idx][i_lvl],
                        tdMan_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

            // Wind Direction
            obs_arr[4] = wdMan[i_idx][i_lvl];
            count += process_obs(31, conversion, obs_arr, wdManQty[i_idx][i_lvl],
                        wdMan_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);
            wdir = obs_arr[4];

            // Wind Speed
            qty = wsManQty[i_idx][i_lvl];
            obs_arr[4] = wsMan[i_idx][i_lvl];
            count += process_obs(32, conversion, obs_arr, qty, wsMan_var,
                        hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);
            wind = obs_arr[4];

            // Convert the wind direction and speed into U and V components
            convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);

            // Write U-component of wind
            obs_arr[4] = ugrd;
            count += process_obs(33, conversion, obs_arr, qty, wsMan_var,
                        hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

            // Write V-component of wind
            obs_arr[4] = vgrd;
            count += process_obs(34, conversion, obs_arr, qty, wsMan_var,
                        hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2] );

         } // end for i_lvl

         //
         // Loop through the significant levels wrt T
         //
         nlvl = nlvl_sigTLevel[i_idx];
         for(i_lvl=0; i_lvl<nlvl; i_lvl++) {

            mlog << Debug(3) << "  Significant T Level: " << i_lvl << "\n";

            //
            // Use cur to index into the NetCDF variables.
            //
            cur[1] = i_lvl;

            //
            // Get the pressure and height for this level
            //
            obs_arr[2] = prSigT[i_idx][i_lvl];
            obs_arr[3] = bad_data_float;

            //
            // Check for bad data
            //
            if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;

            // Temperature
            obs_arr[4] = tpSigT[i_idx][i_lvl];
            count += process_obs(11, conversion, obs_arr, tpSigTQty[i_idx][i_lvl],
                        tpSigT_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

            // Dewpoint
            obs_arr[4] = tdSigT[i_idx][i_lvl];
            count += process_obs(17, conversion, obs_arr, tdSigTQty[i_idx][i_lvl],
                        tdSigT_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         } // end for i_lvl

         //
         // Loop through the significant levels wrt W
         //
         nlvl = nlvl_sigWLevel[i_idx];
         for(i_lvl=0; i_lvl<nlvl; i_lvl++) {

            mlog << Debug(3) << "  Significant W Level: " << i_lvl << "\n";

            //
            // Use cur to index into the NetCDF variables.
            //
            cur[1] = i_lvl;

            //
            // Get the pressure and height for this level
            //
            obs_arr[2] = bad_data_float;
            obs_arr[3] = htSigW[i_idx][i_lvl];

            //
            // Check for bad data
            //
            if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;

            // Wind Direction
            obs_arr[4] = wdSigW[i_idx][i_lvl];
            count += process_obs(31, conversion, obs_arr, wdSigWQty[i_idx][i_lvl],
                        wdSigW_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);
            wdir = obs_arr[4];

            // Wind Speed
            qty = wsSigWQty[i_idx][i_lvl];
            obs_arr[4] = wsSigW[i_idx][i_lvl];
            count += process_obs(32, conversion, obs_arr, qty,
                        wsSigW_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);
            wind = obs_arr[4];

            // Convert the wind direction and speed into U and V components
            convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);

            // Write U-component of wind
            obs_arr[4] = ugrd;
            count += process_obs(33, conversion, obs_arr, qty, wsSigW_var,
                        hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

            // Write V-component of wind
            obs_arr[4] = vgrd;
            count += process_obs(34, conversion, obs_arr, qty, wsSigW_var,
                        hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         } // end for i_lvl

         //
         // Loop through the significant levels wrt W-by-P
         //
         nlvl = nlvl_sigPresWLevel[i_idx];
         for(i_lvl=0; i_lvl<nlvl; i_lvl++) {

            mlog << Debug(3) << "  Significant W-by-P Level: " << i_lvl << "\n";

            //
            // Use cur to index into the NetCDF variables.
            //
            cur[1] = i_lvl;

            //
            // Get the pressure and height for this level
            //
            obs_arr[2] = prSigW[i_idx][i_lvl];
            obs_arr[3] = bad_data_float;

            //
            // Check for bad data
            //
            if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;

            // Wind Direction
            obs_arr[4] = wdSigPrW[i_idx][i_lvl];
            count += process_obs(31, conversion, obs_arr, wdSigPrWQty[i_idx][i_lvl],
                        wdSigPrW_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);
            wdir = obs_arr[4];

            // Wind Speed
            qty = wsSigPrWQty[i_idx][i_lvl];
            obs_arr[4] = wsSigPrW[i_idx][i_lvl];
            count += process_obs(32, conversion, obs_arr, qty,
                        wsSigPrW_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);
            wind = obs_arr[4];

            // Convert the wind direction and speed into U and V components
            convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);

            // Write U-component of wind
            obs_arr[4] = ugrd;
            count += process_obs(33, conversion, obs_arr, qty, wsSigPrW_var,
                        hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

            // Write V-component of wind
            obs_arr[4] = vgrd;
            count += process_obs(34, conversion, obs_arr, qty, wsSigPrW_var,
                        hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         } // end for i_lvl

         //
         // Loop through the tropopause levels
         //
         nlvl = nlvl_mTropNum[i_idx];
         for(i_lvl=0; i_lvl<nlvl; i_lvl++) {

            mlog << Debug(3) << "  Tropopause Level: " << i_lvl << "\n";

            //
            // Use cur to index into the NetCDF variables.
            //
            cur[1] = i_lvl;

            //
            // Get the pressure and height for this level
            //
            obs_arr[2] = prTrop[i_idx][i_lvl];
            obs_arr[3] = bad_data_float;

            //
            // Check for bad data
            //
            if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;

            // Temperature
            obs_arr[4] = tpTrop[i_idx][i_lvl];
            count += process_obs(11, conversion, obs_arr, tpTropQty[i_idx][i_lvl],
                        tpTrop_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

            // Dewpoint
            obs_arr[4] = tdTrop[i_idx][i_lvl];
            count += process_obs(17, conversion, obs_arr, tdTropQty[i_idx][i_lvl],
                        tdTrop_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

            // Wind Direction
            obs_arr[4] = wdTrop[i_idx][i_lvl];
            count += process_obs(31, conversion, obs_arr, wdTropQty[i_idx][i_lvl],
                        wdTrop_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);
            wdir = obs_arr[4];

            // Wind Speed
            qty = wsTropQty[i_idx][i_lvl];
            obs_arr[4] = wsTrop[i_idx][i_lvl];
            count += process_obs(32, conversion, obs_arr, qty,
                        wsTrop_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);
            wind = obs_arr[4];

            // Convert the wind direction and speed into U and V components
            convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);

            // Write U-component of wind
            obs_arr[4] = ugrd;
            count += process_obs(33, conversion, obs_arr, qty, wsTrop_var,
                        hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

            // Write V-component of wind
            obs_arr[4] = vgrd;
            count += process_obs(34, conversion, obs_arr, qty, wsTrop_var,
                        hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         } // end for i_lvl

         //
         // Loop through the maximum wind levels
         //
         nlvl = nlvl_mWndNum[i_idx];
         for(i_lvl=0; i_lvl<nlvl; i_lvl++) {

            mlog << Debug(3) << "  Maximum Wind Level: " << i_lvl << "\n";

            //
            // Use cur to index into the NetCDF variables.
            //
            cur[1] = i_lvl;

            //
            // Get the pressure and height for this level
            //
            obs_arr[2] = prMaxW[i_idx][i_lvl];
            obs_arr[3] = bad_data_float;

            //
            // Check for bad data
            //
            if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;

            // Wind Direction
            obs_arr[4] = wdMaxW[i_idx][i_lvl];
            count += process_obs(31, conversion, obs_arr, wdMaxWQty[i_idx][i_lvl],
                        wdMaxW_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);
            wdir = obs_arr[4];

            // Wind Speed
            qty = wsMaxWQty[i_idx][i_lvl];
            obs_arr[4] = wsMaxW[i_idx][i_lvl];
            count += process_obs(32, conversion, obs_arr, qty,
                        wsMaxW_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);
            wind = obs_arr[4];

            // Convert the wind direction and speed into U and V components
            convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);

            // Write U-component of wind
            obs_arr[4] = ugrd;
            count += process_obs(33, conversion, obs_arr, qty, wsMaxW_var,
                        hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

            // Write V-component of wind
            obs_arr[4] = vgrd;
            count += process_obs(34, conversion, obs_arr, qty, wsMaxW_var,
                        hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         } // end for i_lvl

      } // end for i_hdr

   } // end for i_hdr

   print_rej_counts();

   //
   // Cleanup
   //
   if(cur) { delete [] cur; cur = (long *) 0; }
   if(dim) { delete [] dim; dim = (long *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_madis_profiler(NcFile *&f_in) {
   int nhdr, nlvl, i_lvl;
   long i_hdr, i_hdr_s;
   int hdr_sid_len;
   double tmp_dbl;
   time_t hdr_vld;
   ConcatString hdr_typ, hdr_sid;
   float hdr_arr[hdr_arr_len], obs_arr[obs_arr_len], conversion;
   float pressure;
   int count;
   StringArray missing_vars, missing_qty_vars;
   const char *method_name = "process_madis_profiler() ";

   //
   // Input header variables:
   // Note: hdr_typ is always set to ADPUPA
   //
   NcVar in_hdr_sid_var = get_var(f_in, "staName");
   NcVar in_hdr_vld_var = get_var(f_in, "timeObs");
   NcVar in_hdr_lat_var = get_var(f_in, "staLat");
   NcVar in_hdr_lon_var = get_var(f_in, "staLon");
   NcVar in_hdr_elv_var = get_var(f_in, "staElev");
   NcVar in_uComponent_var = get_var(f_in, "uComponent");
   NcVar in_vComponent_var = get_var(f_in, "vComponent");
   NcVar in_uComponentQty_var = get_var(f_in, "uComponentDD");
   NcVar in_vComponentQty_var = get_var(f_in, "vComponentDD");

   //
   // Variables for vertical level information
   //
   NcVar in_pressure_var = get_var(f_in, "pressure");
   NcVar var_levels = get_var(f_in, "levels");

   if (IS_INVALID_NC(in_hdr_sid_var)) missing_vars.add("staName");
   if (IS_INVALID_NC(in_hdr_vld_var)) missing_vars.add("timeObs");
   if (IS_INVALID_NC(in_hdr_lat_var)) missing_vars.add("staLat");
   if (IS_INVALID_NC(in_hdr_lon_var)) missing_vars.add("staLon");
   if (IS_INVALID_NC(in_hdr_elv_var)) missing_vars.add("staElev");
   if (IS_INVALID_NC(in_uComponent_var)) missing_vars.add("uComponent");
   if (IS_INVALID_NC(in_vComponent_var)) missing_vars.add("vComponent");
   if (IS_INVALID_NC(in_uComponentQty_var)) missing_qty_vars.add("uComponentDD");
   if (IS_INVALID_NC(in_vComponentQty_var)) missing_qty_vars.add("vComponentDD");

   if (IS_INVALID_NC(in_pressure_var)) missing_vars.add("pressure");
   if (IS_INVALID_NC(var_levels)) missing_vars.add("levels");
   
   if (missing_vars.n() > 0) {
      mlog << Error << "\n" << method_name << "Please check if the input is a profiler.\n\n";
      for (int idx=0; idx<missing_vars.n(); idx++)
         mlog << Warning << "    missing variable: " << missing_vars[idx] << "\n";
      exit(1);
   }
   if (missing_qty_vars.n() > 0) {
      for (int idx=0; idx<missing_qty_vars.n(); idx++)
         mlog << Debug(1) << "    missing Qty variable: " << missing_qty_vars[idx] << "\n";
   }

   //
   // Retrieve applicable dimensions
   //
   nlvl         = get_dim_value(f_in, "level");
   hdr_sid_len  = get_dim_value(f_in, "staNamLen");
   nhdr         = get_dim_value(f_in, in_recNum_str);
   int my_rec_end = (rec_end == 0) ? nhdr : rec_end;

   //
   // Setup the output NetCDF file
   //
   //setup_netcdf_out(nhdr);

   mlog << Debug(2) << "Processing PROFILER recs\t\t= " << my_rec_end - rec_beg << "\n";

   //
   // Initialize variables for processing observations
   //
   conversion = 1.0;

   //
   // Arrays of longs for indexing into NetCDF variables
   //
   long *cur = new long [3];
   cur[0] = cur[1] = cur[2] = 0;
   long *dim = new long [3];
   dim[0] = dim[1] = dim[2] = 1;

   //int[] hdr_lat_arr = new int[BUFFER_SIZE];
   int hdr_idx = 0;
   processed_count = 0;

   //
   // Process the header type.
   // For PROFILER, store as ADPUPA.
   //
   hdr_typ = "ADPUPA";

   //
   // Loop through each record and get the header data.
   //
   for(i_hdr_s=rec_beg; i_hdr_s<my_rec_end; i_hdr_s+=BUFFER_SIZE) {
      int buf_size = ((my_rec_end - i_hdr_s) > BUFFER_SIZE) ? BUFFER_SIZE: (my_rec_end - i_hdr_s);
      float hdr_lat_arr[buf_size];
      float hdr_lon_arr[buf_size];
      float hdr_elv_arr[buf_size];
      double tmp_dbl_arr[buf_size];
      char hdr_sid_arr[buf_size][hdr_sid_len];

      float pressure_arr[buf_size];
      float levels_arr[buf_size][nlvl];
      float uComponent_arr[buf_size][nlvl];
      float vComponent_arr[buf_size][nlvl];
      char uComponentQty_arr[buf_size][nlvl];
      char vComponentQty_arr[buf_size][nlvl];

      nc_buf_size = buf_size * FIELD_COUNT;
      if (nc_buf_size > BUFFER_SIZE) nc_buf_size = BUFFER_SIZE;

      cur[0] = i_hdr_s;
      dim[0] = buf_size;
      get_nc_data(&in_hdr_vld_var, tmp_dbl_arr, buf_size, i_hdr_s);
      get_nc_data(&in_hdr_lat_var, hdr_lat_arr, buf_size, i_hdr_s);
      get_nc_data(&in_hdr_lon_var, hdr_lon_arr, buf_size, i_hdr_s);
      get_filtered_nc_data(in_hdr_elv_var,  hdr_elv_arr,  buf_size, i_hdr_s, "eleveation");
      get_filtered_nc_data(in_pressure_var, pressure_arr, buf_size, i_hdr_s, "pressure");

      dim[1] = hdr_sid_len;
      get_nc_data(&in_hdr_sid_var, (char *)hdr_sid_arr, dim, cur);

      dim[1] = nlvl;
      get_nc_data(&var_levels, (float *)levels_arr, dim, cur);

      if (!IS_INVALID_NC(in_uComponentQty_var)) get_nc_data(&in_uComponentQty_var, (char *)uComponentQty_arr, buf_size, i_hdr_s);
      else memset(uComponentQty_arr, 0, buf_size*dim[1]*sizeof(char));

      if (!IS_INVALID_NC(in_vComponentQty_var)) get_nc_data(&in_vComponentQty_var, (char *)vComponentQty_arr, buf_size, i_hdr_s);
      else memset(vComponentQty_arr, 0, buf_size*dim[1]*sizeof(char));

      get_filtered_nc_data_2d(in_uComponent_var, (float *)uComponent_arr, dim, cur, "uComponent");
      get_filtered_nc_data_2d(in_vComponent_var, (float *)vComponent_arr, dim, cur, "vComponent");

      dim[0] = 1;
      dim[1] = 1;

      for (int i_idx=0; i_idx<buf_size; i_idx++) {

         //
         // Mapping of NetCDF variable names from input to output:
         // Output                    = Input
         // hdr_typ                   = NA - always set to ADPUPA
         // hdr_sid                   = staName (staNameLen = 50)
         // hdr_vld (YYYYMMDD_HHMMSS) = synTime (unixtime) - synoptic time
         // hdr_arr[0](Lat)           = staLat
         // hdr_arr[1](Lon)           = staLon
         // hdr_arr[2](Elv)           = staElev
         //

         count = 0;
         i_hdr = i_hdr_s + i_idx;
         mlog << Debug(3) << "Record Number: " << i_hdr << "\n";

         //
         // Use cur to index into the NetCDF variables.
         //
         cur[0] = i_hdr;
         cur[1] = 0;

         //
         // Process the latitude, longitude, and elevation.
         //
         hdr_arr[0] = hdr_lat_arr[i_idx];
         hdr_arr[1] = hdr_lon_arr[i_idx];
         hdr_arr[2] = hdr_elv_arr[i_idx];

         //
         // Check masking regions.
         //
         if(!check_masks(hdr_arr[0], hdr_arr[1], hdr_sid_arr[i_idx])) continue;



         //
         // Process the station name.
         //
         hdr_sid = hdr_sid_arr[i_idx];

         //
         // Process the observation time.
         //
         tmp_dbl = tmp_dbl_arr[i_idx];
         if(is_bad_data(tmp_dbl)) continue;

         hdr_vld = (time_t)tmp_dbl;

         hdr_idx = get_nc_hdr_cur_index();

         //
         // Initialize the observation array: hdr_id
         //
         obs_arr[0] = (float)(hdr_idx);

         //
         // Get the pressure for the current level
         //
         pressure = pressure_arr[i_idx];

         //
         // Loop through the mandatory levels
         //
         for(i_lvl=0; i_lvl<nlvl; i_lvl++) {

            mlog << Debug(3) << "  Level: " << i_lvl << "\n";

            //
            // Use cur to index into the NetCDF variables.
            //
            cur[1] = i_lvl;

            //
            // Set the pressure and height for this level
            //
            obs_arr[2] = pressure;
            obs_arr[3] = levels_arr[i_idx][i_lvl];

            //
            // Check for bad data
            //
            if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;

            // Wind U
            obs_arr[4] = uComponent_arr[i_idx][i_lvl];
            count += process_obs(33, conversion, obs_arr, uComponentQty_arr[i_idx][i_lvl],
                        in_uComponent_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

            // Wind V
            obs_arr[4] = vComponent_arr[i_idx][i_lvl];
            count += process_obs(34, conversion, obs_arr, vComponentQty_arr[i_idx][i_lvl],
                        in_vComponent_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         } // end for i_lvl

      } // end for i_hdr

   } // end for i_hdr

   print_rej_counts();

   //
   // Cleanup
   //
   if(cur) { delete [] cur; cur = (long *) 0; }
   if(dim) { delete [] dim; dim = (long *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_madis_maritime(NcFile *&f_in) {
   int nhdr;
   long i_hdr, i_hdr_s;
   int hdr_sid_len;
   double tmp_dbl;
   time_t hdr_vld;
   ConcatString hdr_typ, hdr_sid;
   float hdr_arr[hdr_arr_len], obs_arr[obs_arr_len], conversion;
   float pressure;
   int count;
   StringArray missing_vars, missing_qty_vars;
   const char *method_name = "process_madis_maritime() ";

   //
   // Input header variables:
   // Note: hdr_typ is always set to ADPUPA
   //
   NcVar in_hdr_sid_var = get_var(f_in, "stationName");
   NcVar in_hdr_vld_var = get_var(f_in, "timeObs");
   NcVar in_hdr_lat_var = get_var(f_in, "latitude");
   NcVar in_hdr_lon_var = get_var(f_in, "longitude");
   NcVar in_hdr_elv_var = get_var(f_in, "elevation");

   //
   // Variables for vertical level information
   //
   NcVar in_pressure_var = get_var(f_in, "stationPress");

   NcVar in_windDir_var = get_var(f_in, "windDir");
   NcVar in_windSpeed_var = get_var(f_in, "windSpeed");
   NcVar in_temperature_var = get_var(f_in, "temperature");
   NcVar in_dewpoint_var = get_var(f_in, "dewpoint");
   NcVar in_seaLevelPress_var = get_var(f_in, "seaLevelPress");
   NcVar in_windGust_var = get_var(f_in, "windGust");
   NcVar in_precip1Hour_var = get_var(f_in, "precip1Hour");
   NcVar in_precip6Hour_var = get_var(f_in, "precip6Hour");
   NcVar in_precip12Hour_var = get_var(f_in, "precip12Hour");
   NcVar in_precip18Hour_var = get_var(f_in, "precip18Hour");
   NcVar in_precip24Hour_var = get_var(f_in, "precip24Hour");

   NcVar in_windDirQty_var = get_var(f_in, "windDirDD");
   NcVar in_windSpeedQty_var = get_var(f_in, "windSpeedDD");
   NcVar in_temperatureQty_var = get_var(f_in, "temperatureDD");
   NcVar in_dewpointQty_var = get_var(f_in, "dewpointDD");
   NcVar in_seaLevelPressQty_var = get_var(f_in, "seaLevelPressDD");
   NcVar in_windGustQty_var = get_var(f_in, "windGustDD");
   NcVar in_precip1HourQty_var = get_var(f_in, "precip1HourDD");
   NcVar in_precip6HourQty_var = get_var(f_in, "precip6HourDD");
   NcVar in_precip12HourQty_var = get_var(f_in, "precip12HourDD");
   NcVar in_precip18HourQty_var = get_var(f_in, "precip18HourDD");
   NcVar in_precip24HourQty_var = get_var(f_in, "precip24HourDD");


   if (IS_INVALID_NC(in_hdr_sid_var)) missing_vars.add("stationName");
   if (IS_INVALID_NC(in_hdr_vld_var)) missing_vars.add("timeObs");
   if (IS_INVALID_NC(in_hdr_lat_var)) missing_vars.add("latitude");
   if (IS_INVALID_NC(in_hdr_lon_var)) missing_vars.add("longitude");
   if (IS_INVALID_NC(in_hdr_elv_var)) missing_vars.add("elevation");

   if (IS_INVALID_NC(in_pressure_var)) missing_vars.add("stationPress");

   if (IS_INVALID_NC(in_windDir_var)) missing_vars.add("windDir");
   if (IS_INVALID_NC(in_windSpeed_var)) missing_vars.add("windSpeed");
   if (IS_INVALID_NC(in_temperature_var)) missing_vars.add("temperature");
   if (IS_INVALID_NC(in_dewpoint_var)) missing_vars.add("dewpoint");
   if (IS_INVALID_NC(in_seaLevelPress_var)) missing_vars.add("seaLevelPress");
   if (IS_INVALID_NC(in_windGust_var)) missing_vars.add("windGust");
   if (IS_INVALID_NC(in_precip1Hour_var)) missing_vars.add("precip1Hour");
   if (IS_INVALID_NC(in_precip6Hour_var)) missing_vars.add("precip6Hour");
   if (IS_INVALID_NC(in_precip12Hour_var)) missing_vars.add("precip12Hour");
   if (IS_INVALID_NC(in_precip18Hour_var)) missing_vars.add("precip18Hour");
   if (IS_INVALID_NC(in_precip24Hour_var)) missing_vars.add("precip24Hour");

   if (IS_INVALID_NC(in_windDirQty_var)) missing_qty_vars.add("windDirDD");
   if (IS_INVALID_NC(in_windSpeedQty_var)) missing_qty_vars.add("windSpeedDD");
   if (IS_INVALID_NC(in_temperatureQty_var)) missing_qty_vars.add("temperatureDD");
   if (IS_INVALID_NC(in_dewpointQty_var)) missing_qty_vars.add("dewpointDD");
   if (IS_INVALID_NC(in_seaLevelPressQty_var)) missing_qty_vars.add("seaLevelPressDD");
   if (IS_INVALID_NC(in_windGustQty_var)) missing_qty_vars.add("windGustDD");
   if (IS_INVALID_NC(in_precip1HourQty_var)) missing_qty_vars.add("precip1HourDD");
   if (IS_INVALID_NC(in_precip6HourQty_var)) missing_qty_vars.add("precip6HourDD");
   if (IS_INVALID_NC(in_precip12HourQty_var)) missing_qty_vars.add("precip12HourDD");
   if (IS_INVALID_NC(in_precip18HourQty_var)) missing_qty_vars.add("precip18HourDD");
   if (IS_INVALID_NC(in_precip24HourQty_var)) missing_qty_vars.add("precip24HourDD");

   if (missing_vars.n() > 0) {
      mlog << Error << "\n" << method_name << "Please check if the input is a maritime.\n\n";
      for (int idx=0; idx<missing_vars.n(); idx++)
         mlog << Warning << "    missing variable: " << missing_vars[idx] << "\n";
      exit(1);
   }
   if (missing_qty_vars.n() > 0) {
      for (int idx=0; idx<missing_qty_vars.n(); idx++)
         mlog << Warning << "    missing Qty variable: " << missing_qty_vars[idx] << "\n";
   }
   
   //
   // Retrieve applicable dimensions
   //
   hdr_sid_len  = get_dim_value(f_in, "maxStaNamLen");
   nhdr         = get_dim_value(f_in, in_recNum_str);
   int my_rec_end = (rec_end == 0) ? nhdr : rec_end;

   //
   // Setup the output NetCDF file
   //
   //setup_netcdf_out(nhdr);

   mlog << Debug(2) << "Processing MARITIME recs\t\t= " << my_rec_end - rec_beg << "\n";

   //
   // Initialize variables for processing observations
   //
   conversion = 1.0;

   //
   // Arrays of longs for indexing into NetCDF variables
   //
   long *cur = new long [3];
   cur[0] = cur[1] = cur[2] = 0;
   long *dim = new long [3];
   dim[0] = dim[1] = dim[2] = 1;

   int hdr_idx = 0;

   //
   // Process the header type.
   // For maritime, store as SFCSHP.
   //
   hdr_typ = "SFCSHP";

   //
   // Loop through each record and get the header data.
   //
   for(i_hdr_s=rec_beg; i_hdr_s<my_rec_end; i_hdr_s+=BUFFER_SIZE) {
      int buf_size = ((my_rec_end - i_hdr_s) > BUFFER_SIZE) ? BUFFER_SIZE: (my_rec_end - i_hdr_s);
      float hdr_lat_arr[buf_size];
      float hdr_lon_arr[buf_size];
      float hdr_elv_arr[buf_size];
      double tmp_dbl_arr[buf_size];
      char hdr_sid_arr[buf_size][hdr_sid_len];

      float pressure_arr[buf_size];

      float windDir_arr[buf_size];
      float windSpeed_arr[buf_size];
      float temperature_arr[buf_size];
      float dewpoint_arr[buf_size];
      float seaLevelPress_arr[buf_size];
      float windGust_arr[buf_size];
      float precip1Hour_arr[buf_size];
      float precip6Hour_arr[buf_size];
      float precip12Hour_arr[buf_size];
      float precip18Hour_arr[buf_size];
      float precip24Hour_arr[buf_size];
      char windDirQty_arr[buf_size];
      char windSpeedQty_arr[buf_size];
      char temperatureQty_arr[buf_size];
      char dewpointQty_arr[buf_size];
      char seaLevelPressQty_arr[buf_size];
      char windGustQty_arr[buf_size];
      char precip1HourQty_arr[buf_size];
      char precip6HourQty_arr[buf_size];
      char precip12HourQty_arr[buf_size];
      char precip18HourQty_arr[buf_size];
      char precip24HourQty_arr[buf_size];

      nc_buf_size = buf_size * FIELD_COUNT;
      if (nc_buf_size > BUFFER_SIZE) nc_buf_size = BUFFER_SIZE;

      cur[0] = i_hdr_s;
      dim[0] = buf_size;
      get_nc_data(&in_hdr_vld_var, tmp_dbl_arr, buf_size, i_hdr_s);
      get_nc_data(&in_hdr_lat_var, hdr_lat_arr, buf_size, i_hdr_s);
      get_nc_data(&in_hdr_lon_var, hdr_lon_arr, buf_size, i_hdr_s);
      get_filtered_nc_data(in_hdr_elv_var, hdr_elv_arr, buf_size, i_hdr_s, "eleveation");

      if (!IS_INVALID_NC(in_windDirQty_var))       get_nc_data(&in_windDirQty_var, windDirQty_arr, buf_size, i_hdr_s);
      else memset(windDirQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_windSpeedQty_var))     get_nc_data(&in_windSpeedQty_var, windSpeedQty_arr, buf_size, i_hdr_s);
      else memset(windSpeedQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_temperatureQty_var))   get_nc_data(&in_temperatureQty_var, temperatureQty_arr, buf_size, i_hdr_s);
      else memset(temperatureQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_dewpointQty_var))      get_nc_data(&in_dewpointQty_var, dewpointQty_arr, buf_size, i_hdr_s);
      else memset(dewpointQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_seaLevelPressQty_var)) get_nc_data(&in_seaLevelPressQty_var, seaLevelPressQty_arr, buf_size, i_hdr_s);
      else memset(seaLevelPressQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_windGustQty_var))      get_nc_data(&in_windGustQty_var, windGustQty_arr, buf_size, i_hdr_s);
      else memset(windGustQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_precip1HourQty_var))   get_nc_data(&in_precip1HourQty_var, precip1HourQty_arr, buf_size, i_hdr_s);
      else memset(precip1HourQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_precip6HourQty_var))   get_nc_data(&in_precip6HourQty_var, precip6HourQty_arr, buf_size, i_hdr_s);
      else memset(precip6HourQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_precip12HourQty_var))  get_nc_data(&in_precip12HourQty_var, precip12HourQty_arr, buf_size, i_hdr_s);
      else memset(precip12HourQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_precip18HourQty_var))  get_nc_data(&in_precip18HourQty_var, precip18HourQty_arr, buf_size, i_hdr_s);
      else memset(precip18HourQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_precip24HourQty_var))  get_nc_data(&in_precip24HourQty_var, precip24HourQty_arr, buf_size, i_hdr_s);
      else memset(precip24HourQty_arr, 0, buf_size*sizeof(char));

      get_filtered_nc_data(in_pressure_var,      pressure_arr,      buf_size, i_hdr_s, "pressure"     );
      get_filtered_nc_data(in_windDir_var,       windDir_arr,       buf_size, i_hdr_s, "windDir"      );
      get_filtered_nc_data(in_windSpeed_var,     windSpeed_arr,     buf_size, i_hdr_s, "windSpeed"    );
      get_filtered_nc_data(in_temperature_var,   temperature_arr,   buf_size, i_hdr_s, "temperature"  );
      get_filtered_nc_data(in_dewpoint_var,      dewpoint_arr,      buf_size, i_hdr_s, "dewpoint"     );
      get_filtered_nc_data(in_seaLevelPress_var, seaLevelPress_arr, buf_size, i_hdr_s, "seaLevelPress");
      get_filtered_nc_data(in_windGust_var,      windGust_arr,      buf_size, i_hdr_s, "windGust"     );
      get_filtered_nc_data(in_precip1Hour_var,   precip1Hour_arr,   buf_size, i_hdr_s, "precip1Hour"  );
      get_filtered_nc_data(in_precip6Hour_var,   precip6Hour_arr,   buf_size, i_hdr_s, "precip6Hour"  );
      get_filtered_nc_data(in_precip12Hour_var,  precip12Hour_arr,  buf_size, i_hdr_s, "precip12Hour" );
      get_filtered_nc_data(in_precip18Hour_var,  precip18Hour_arr,  buf_size, i_hdr_s, "precip18Hour" );
      get_filtered_nc_data(in_precip24Hour_var,  precip24Hour_arr,  buf_size, i_hdr_s, "precip24Hour" );

      dim[1] = hdr_sid_len;
      get_nc_data(&in_hdr_sid_var, (char *)hdr_sid_arr, dim, cur);

      dim[0] = 1;
      dim[1] = 1;

      for (int i_idx=0; i_idx<buf_size; i_idx++) {

         //
         // Mapping of NetCDF variable names from input to output:
         // Output                    = Input
         // hdr_typ                   = NA - always set to ADPUPA
         // hdr_sid                   = staName (staNameLen = 50)
         // hdr_vld (YYYYMMDD_HHMMSS) = synTime (unixtime) - synoptic time
         // hdr_arr[0](Lat)           = staLat
         // hdr_arr[1](Lon)           = staLon
         // hdr_arr[2](Elv)           = staElev
         //

         count = 0;
         i_hdr = i_hdr_s + i_idx;
         mlog << Debug(3) << "Record Number: " << i_hdr << "\n";

         //
         // Use cur to index into the NetCDF variables.
         //
         cur[0] = i_hdr;
         cur[1] = 0;

         //
         // Process the latitude, longitude, and elevation.
         //
         hdr_arr[0] = hdr_lat_arr[i_idx];
         hdr_arr[1] = hdr_lon_arr[i_idx];
         hdr_arr[2] = hdr_elv_arr[i_idx];

         //
         // Check masking regions.
         //
         if(!check_masks(hdr_arr[0], hdr_arr[1], hdr_sid_arr[i_idx])) continue;


         //
         // Process the station name.
         //
         hdr_sid = hdr_sid_arr[i_idx];

         //
         // Process the observation time.
         //
         tmp_dbl = tmp_dbl_arr[i_idx];
         if(is_bad_data(tmp_dbl)) continue;

         hdr_vld = (time_t)tmp_dbl;

         hdr_idx = get_nc_hdr_cur_index();


         //
         // Initialize the observation array: hdr_id
         //
         obs_arr[0] = (float) (hdr_idx);

         //
         // Get the pressure for the current level
         //
         pressure = pressure_arr[i_idx];

         //
         // Set the pressure and height for this level
         //
         obs_arr[2] = pressure;
         obs_arr[3] = hdr_arr[2];

         //
         // Check for bad data
         //
         if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;

         // Wind Direction
         obs_arr[4] = windDir_arr[i_idx];
         count += process_obs(31, conversion, obs_arr, windDirQty_arr[i_idx],
                     in_windDir_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Wind Speed
         obs_arr[4] = windSpeed_arr[i_idx];
         count += process_obs(32, conversion, obs_arr, windSpeedQty_arr[i_idx],
                     in_windSpeed_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Temperature
         obs_arr[4] = temperature_arr[i_idx];
         count += process_obs(11, conversion, obs_arr, temperatureQty_arr[i_idx],
                     in_temperature_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Dew Point temperature
         obs_arr[4] = dewpoint_arr[i_idx];
         count += process_obs(17, conversion, obs_arr, dewpointQty_arr[i_idx],
                     in_dewpoint_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Pressure reduced to MSL
         obs_arr[4] = seaLevelPress_arr[i_idx];
         count += process_obs(2, conversion, obs_arr, seaLevelPressQty_arr[i_idx],
                     in_seaLevelPress_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Surface wind gust
         obs_arr[4] = windGust_arr[i_idx];
         count += process_obs(180, conversion, obs_arr, windGustQty_arr[i_idx],
                     in_windGust_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // APCP_01
         obs_arr[2] = 3600;
         obs_arr[4] = precip1Hour_arr[i_idx];
         count += process_obs(61, conversion, obs_arr, precip1HourQty_arr[i_idx],
                     in_precip1Hour_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // APCP_06
         obs_arr[2] = 21600;
         obs_arr[4] = precip6Hour_arr[i_idx];
         count += process_obs(61, conversion, obs_arr, precip6HourQty_arr[i_idx],
                     in_precip6Hour_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // APCP_12
         obs_arr[2] = 43200;
         obs_arr[4] = precip12Hour_arr[i_idx];
         count += process_obs(61, conversion, obs_arr, precip12HourQty_arr[i_idx],
                     in_precip12Hour_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // APCP_18
         obs_arr[2] = 64800;
         obs_arr[4] = precip18Hour_arr[i_idx];
         count += process_obs(61, conversion, obs_arr, precip18HourQty_arr[i_idx],
                     in_precip18Hour_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // APCP_24
         obs_arr[2] = 86400;
         obs_arr[4] = precip24Hour_arr[i_idx];
         //count += process_obs(61, conversion, obs_arr, precip24HourQty_arr[i_idx],
         //            in_precip24Hour_var, hdr_typ, hdr_sid, hdr_vld,
         //               hdr_arr[0], hdr_arr[1], hdr_arr[2]);
         process_obs(61, conversion, obs_arr, precip24HourQty_arr[i_idx],
            in_precip24Hour_var, hdr_typ, hdr_sid, hdr_vld,
            hdr_arr[0], hdr_arr[1], hdr_arr[2]);

      }

   } // end for i_hdr

   print_rej_counts();

   //
   // Cleanup
   //
   if(cur) { delete [] cur; cur = (long *) 0; }
   if(dim) { delete [] dim; dim = (long *) 0; }

   return;
}
////////////////////////////////////////////////////////////////////////

void process_madis_mesonet(NcFile *&f_in) {
   int nhdr;
   long i_hdr, i_hdr_s;
   int hdr_sid_len;
   double tmp_dbl;
   time_t hdr_vld;
   ConcatString hdr_typ, hdr_sid;
   float hdr_arr[hdr_arr_len], obs_arr[obs_arr_len], conversion;
   float wdir, wind, ugrd, vgrd;
   int count;
   StringArray missing_vars, missing_qty_vars;
   const char *method_name = "process_madis_mesonet() ";

   //
   // Input header variables
   //
   NcVar in_hdr_sid_var = get_var(f_in, "stationId");
   NcVar in_hdr_vld_var = get_var(f_in, "observationTime");
   NcVar in_hdr_lat_var = get_var(f_in, "latitude");
   NcVar in_hdr_lon_var = get_var(f_in, "longitude");
   NcVar in_hdr_elv_var = get_var(f_in, "elevation");

   NcVar in_temperature_var = get_var(f_in, "temperature");
   NcVar in_dewpoint_var = get_var(f_in, "dewpoint");
   NcVar in_relHumidity_var = get_var(f_in, "relHumidity");
   NcVar in_stationPressure_var = get_var(f_in, "stationPressure");
   NcVar in_seaLevelPressure_var = get_var(f_in, "seaLevelPressure");
   NcVar in_windDir_var = get_var(f_in, "windDir");
   NcVar in_windSpeed_var = get_var(f_in, "windSpeed");
   NcVar in_windGust_var = get_var(f_in, "windGust");
   NcVar in_visibility_var = get_var(f_in, "visibility");
   NcVar in_precipRate_var = get_var(f_in, "precipRate");
   NcVar in_solarRadiation_var = get_var(f_in, "solarRadiation");
   NcVar in_seaSurfaceTemp_var = get_var(f_in, "seaSurfaceTemp");
   NcVar in_totalColumnPWV_var = get_var(f_in, "totalColumnPWV");
   NcVar in_soilTemperature_var = get_var(f_in, "soilTemperature");
   NcVar in_minTemp24Hour_var = get_var(f_in, "minTemp24Hour");
   NcVar in_maxTemp24Hour_var = get_var(f_in, "maxTemp24Hour");
   NcVar in_precip3hr_var = get_var(f_in, "precip3hr");
   NcVar in_precip6hr_var = get_var(f_in, "precip6hr");
   NcVar in_precip12hr_var = get_var(f_in, "precip12hr");
   NcVar in_precip10min_var = get_var(f_in, "precip10min");
   NcVar in_precip1min_var = get_var(f_in, "precip1min");
   NcVar in_windDir10_var = get_var(f_in, "windDir10");
   NcVar in_windSpeed10_var = get_var(f_in, "windSpeed10");

   NcVar in_temperatureQty_var = get_var(f_in, "temperatureDD");
   NcVar in_dewpointQty_var = get_var(f_in, "dewpointDD");
   NcVar in_relHumidityQty_var = get_var(f_in, "relHumidityDD");
   NcVar in_stationPressureQty_var = get_var(f_in, "stationPressureDD");
   NcVar in_seaLevelPressureQty_var = get_var(f_in, "seaLevelPressureDD");
   NcVar in_windDirQty_var = get_var(f_in, "windDirDD");
   NcVar in_windSpeedQty_var = get_var(f_in, "windSpeedDD");
   NcVar in_windGustQty_var = get_var(f_in, "windGustDD");
   NcVar in_visibilityQty_var = get_var(f_in, "visibilityDD");
   NcVar in_precipRateQty_var = get_var(f_in, "precipRateDD");
   NcVar in_solarRadiationQty_var = get_var(f_in, "solarRadiationDD");
   NcVar in_seaSurfaceTempQty_var = get_var(f_in, "seaSurfaceTempDD");
   NcVar in_totalColumnPWVQty_var = get_var(f_in, "totalColumnPWVDD");
   NcVar in_soilTemperatureQty_var = get_var(f_in, "soilTemperatureDD");
   NcVar in_minTemp24HourQty_var = get_var(f_in, "minTemp24HourDD");
   NcVar in_maxTemp24HourQty_var = get_var(f_in, "maxTemp24HourDD");
   NcVar in_precip3hrQty_var = get_var(f_in, "precip3hrDD");
   NcVar in_precip6hrQty_var = get_var(f_in, "precip6hrDD");
   NcVar in_precip12hrQty_var = get_var(f_in, "precip12hrDD");
   NcVar in_precip10minQty_var = get_var(f_in, "precip10minDD");
   NcVar in_precip1minQty_var = get_var(f_in, "precip1minDD");
   NcVar in_windDir10Qty_var = get_var(f_in, "windDir10DD");
   NcVar in_windSpeed10Qty_var = get_var(f_in, "windSpeed10DD");

   if (IS_INVALID_NC(in_hdr_sid_var)) missing_vars.add("stationId");
   if (IS_INVALID_NC(in_hdr_vld_var)) missing_vars.add("observationTime");
   if (IS_INVALID_NC(in_hdr_lat_var)) missing_vars.add("latitude");
   if (IS_INVALID_NC(in_hdr_lon_var)) missing_vars.add("longitude");
   if (IS_INVALID_NC(in_hdr_elv_var)) missing_vars.add("elevation");

   if (IS_INVALID_NC(in_temperature_var)) missing_vars.add("temperature");
   if (IS_INVALID_NC(in_dewpoint_var)) missing_vars.add("dewpoint");
   if (IS_INVALID_NC(in_relHumidity_var)) missing_vars.add("relHumidity");
   if (IS_INVALID_NC(in_stationPressure_var)) missing_vars.add("stationPressure");
   if (IS_INVALID_NC(in_seaLevelPressure_var)) missing_vars.add("seaLevelPressure");
   if (IS_INVALID_NC(in_windDir_var)) missing_vars.add("windDir");
   if (IS_INVALID_NC(in_windSpeed_var)) missing_vars.add("windSpeed");
   if (IS_INVALID_NC(in_windGust_var)) missing_vars.add("windGust");
   if (IS_INVALID_NC(in_visibility_var)) missing_vars.add("visibility");
   if (IS_INVALID_NC(in_precipRate_var)) missing_vars.add("precipRate");
   if (IS_INVALID_NC(in_solarRadiation_var)) missing_vars.add("solarRadiation");
   if (IS_INVALID_NC(in_seaSurfaceTemp_var)) missing_vars.add("seaSurfaceTemp");
   if (IS_INVALID_NC(in_totalColumnPWV_var)) missing_vars.add("totalColumnPWV");
   if (IS_INVALID_NC(in_soilTemperature_var)) missing_vars.add("soilTemperature");
   if (IS_INVALID_NC(in_minTemp24Hour_var)) missing_vars.add("minTemp24Hour");
   if (IS_INVALID_NC(in_maxTemp24Hour_var)) missing_vars.add("maxTemp24Hour");
   if (IS_INVALID_NC(in_precip3hr_var)) missing_vars.add("precip3hr");
   if (IS_INVALID_NC(in_precip6hr_var)) missing_vars.add("precip6hr");
   if (IS_INVALID_NC(in_precip12hr_var)) missing_vars.add("precip12hr");
   if (IS_INVALID_NC(in_precip10min_var)) missing_vars.add("precip10min");
   if (IS_INVALID_NC(in_precip1min_var)) missing_vars.add("precip1min");
   if (IS_INVALID_NC(in_windDir10_var)) missing_vars.add("windDir10");
   if (IS_INVALID_NC(in_windSpeed10_var)) missing_vars.add("windSpeed10");

   if (IS_INVALID_NC(in_temperatureQty_var)) missing_qty_vars.add("temperatureDD");
   if (IS_INVALID_NC(in_dewpointQty_var)) missing_qty_vars.add("dewpointDD");
   if (IS_INVALID_NC(in_relHumidityQty_var)) missing_qty_vars.add("relHumidityDD");
   if (IS_INVALID_NC(in_stationPressureQty_var)) missing_qty_vars.add("stationPressureDD");
   if (IS_INVALID_NC(in_seaLevelPressureQty_var)) missing_qty_vars.add("seaLevelPressureDD");
   if (IS_INVALID_NC(in_windDirQty_var)) missing_qty_vars.add("windDirDD");
   if (IS_INVALID_NC(in_windSpeedQty_var)) missing_qty_vars.add("windSpeedDD");
   if (IS_INVALID_NC(in_windGustQty_var)) missing_qty_vars.add("windGustDD");
   if (IS_INVALID_NC(in_visibilityQty_var)) missing_qty_vars.add("visibilityDD");
   if (IS_INVALID_NC(in_precipRateQty_var)) missing_qty_vars.add("precipRateDD");
   if (IS_INVALID_NC(in_solarRadiationQty_var)) missing_qty_vars.add("solarRadiationDD");
   if (IS_INVALID_NC(in_seaSurfaceTempQty_var)) missing_qty_vars.add("seaSurfaceTempDD");
   if (IS_INVALID_NC(in_totalColumnPWVQty_var)) missing_qty_vars.add("totalColumnPWVDD");
   if (IS_INVALID_NC(in_soilTemperatureQty_var)) missing_qty_vars.add("soilTemperatureDD");
   if (IS_INVALID_NC(in_minTemp24HourQty_var)) missing_qty_vars.add("minTemp24HourDD");
   if (IS_INVALID_NC(in_maxTemp24HourQty_var)) missing_qty_vars.add("maxTemp24HourDD");
   if (IS_INVALID_NC(in_precip3hrQty_var)) missing_qty_vars.add("precip3hrDD");
   if (IS_INVALID_NC(in_precip6hrQty_var)) missing_qty_vars.add("precip6hrDD");
   if (IS_INVALID_NC(in_precip12hrQty_var)) missing_qty_vars.add("precip12hrDD");
   if (IS_INVALID_NC(in_precip10minQty_var)) missing_qty_vars.add("precip10minDD");
   if (IS_INVALID_NC(in_precip1minQty_var)) missing_qty_vars.add("precip1minDD");
   if (IS_INVALID_NC(in_windDir10Qty_var)) missing_qty_vars.add("windDir10DD");
   if (IS_INVALID_NC(in_windSpeed10Qty_var)) missing_qty_vars.add("windSpeed10DD");

   if (missing_vars.n() > 0) {
      mlog << Error << "\n" << method_name << "Please check if the input is a MESONET.\n\n";
      for (int idx=0; idx<missing_vars.n(); idx++)
         mlog << Warning << "    missing variable: " << missing_vars[idx] << "\n";
      exit(1);
   }
   if (missing_qty_vars.n() > 0) {
      for (int idx=0; idx<missing_qty_vars.n(); idx++)
         mlog << Debug(1) << "    missing Qty variable: " << missing_qty_vars[idx] << "\n";
   }
   
   //
   // Retrieve applicable dimensions
   //
   hdr_sid_len = get_dim_value(f_in, "maxStaIdLen");
   nhdr        = get_dim_value(f_in, in_recNum_str);
   int my_rec_end = (rec_end == 0) ? nhdr : rec_end;

   //
   // Setup the output NetCDF file
   //
   //setup_netcdf_out(nhdr);

   mlog << Debug(2) << "Processing Integrated Mesonet recs\t= " << my_rec_end - rec_beg << "\n";

   //
   // Initialize variables for processing observations
   //
   conversion = 1.0;

   //
   // Arrays of longs for indexing into NetCDF variables
   //
   long *cur = new long [2];
   cur[0] = cur[1] = 0;
   long *dim = new long [2];
   dim[0] = 1;

   int hdr_idx = 0;

   //
   // Encode the header type as ADPSFC for MESONET observations.
   //
   hdr_typ = "ADPSFC";

   //
   // Loop through each record and get the header data.
   //
   for(i_hdr_s=rec_beg; i_hdr_s<my_rec_end; i_hdr_s+=BUFFER_SIZE) {
      int buf_size = ((my_rec_end - i_hdr_s) > BUFFER_SIZE) ? BUFFER_SIZE: (my_rec_end - i_hdr_s);
      float hdr_lat_arr[buf_size];
      float hdr_lon_arr[buf_size];
      float hdr_elv_arr[buf_size];
      double tmp_dbl_arr[buf_size];
      char hdr_sid_arr[buf_size][hdr_sid_len];

      float temperature_arr[buf_size];
      float dewpoint_arr[buf_size];
      float relHumidity_arr[buf_size];
      float stationPressure_arr[buf_size];
      float seaLevelPressure_arr[buf_size];
      float windDir_arr[buf_size];
      float windSpeed_arr[buf_size];
      float windGust_arr[buf_size];
      float visibility_arr[buf_size];
      float precipRate_arr[buf_size];
      float solarRadiation_arr[buf_size];
      float seaSurfaceTemp_arr[buf_size];
      float totalColumnPWV_arr[buf_size];
      float soilTemperature_arr[buf_size];
      float minTemp24Hour_arr[buf_size];
      float maxTemp24Hour_arr[buf_size];
      float precip3hr_arr[buf_size];
      float precip6hr_arr[buf_size];
      float precip12hr_arr[buf_size];
      float precip10min_arr[buf_size];
      float precip1min_arr[buf_size];
      float windDir10_arr[buf_size];
      float windSpeed10_arr[buf_size];

      char temperatureQty_arr[buf_size];
      char dewpointQty_arr[buf_size];
      char relHumidityQty_arr[buf_size];
      char stationPressureQty_arr[buf_size];
      char seaLevelPressureQty_arr[buf_size];
      char windDirQty_arr[buf_size];
      char windSpeedQty_arr[buf_size];
      char windGustQty_arr[buf_size];
      char visibilityQty_arr[buf_size];
      char precipRateQty_arr[buf_size];
      char solarRadiationQty_arr[buf_size];
      char seaSurfaceTempQty_arr[buf_size];
      char totalColumnPWVQty_arr[buf_size];
      char soilTemperatureQty_arr[buf_size];
      char minTemp24HourQty_arr[buf_size];
      char maxTemp24HourQty_arr[buf_size];
      char precip3hrQty_arr[buf_size];
      char precip6hrQty_arr[buf_size];
      char precip12hrQty_arr[buf_size];
      char precip10minQty_arr[buf_size];
      char precip1minQty_arr[buf_size];
      char windDir10Qty_arr[buf_size];
      char windSpeed10Qty_arr[buf_size];

      nc_buf_size = buf_size * FIELD_COUNT;
      if (nc_buf_size > BUFFER_SIZE) nc_buf_size = BUFFER_SIZE;

      cur[0] = i_hdr_s;
      dim[0] = buf_size;

      get_nc_data(&in_hdr_vld_var, tmp_dbl_arr, buf_size, i_hdr_s);
      get_nc_data(&in_hdr_lat_var, hdr_lat_arr, buf_size, i_hdr_s);
      get_nc_data(&in_hdr_lon_var, hdr_lon_arr, buf_size, i_hdr_s);
      get_filtered_nc_data(in_hdr_elv_var, hdr_elv_arr, buf_size, i_hdr_s, "eleveation");

      if (!IS_INVALID_NC(in_temperatureQty_var))      get_nc_data(&in_temperatureQty_var, temperatureQty_arr, buf_size, i_hdr_s);
      else memset(temperatureQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_dewpointQty_var))         get_nc_data(&in_dewpointQty_var, dewpointQty_arr, buf_size, i_hdr_s);
      else memset(dewpointQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_relHumidityQty_var))      get_nc_data(&in_relHumidityQty_var, relHumidityQty_arr, buf_size, i_hdr_s);
      else memset(relHumidityQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_stationPressureQty_var))  get_nc_data(&in_stationPressureQty_var, stationPressureQty_arr, buf_size, i_hdr_s);
      else memset(stationPressureQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_seaLevelPressureQty_var)) get_nc_data(&in_seaLevelPressureQty_var, seaLevelPressureQty_arr, buf_size, i_hdr_s);
      else memset(seaLevelPressureQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_windDirQty_var))          get_nc_data(&in_windDirQty_var, windDirQty_arr, buf_size, i_hdr_s);
      else memset(windDirQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_windSpeedQty_var))        get_nc_data(&in_windSpeedQty_var, windSpeedQty_arr, buf_size, i_hdr_s);
      else memset(windSpeedQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_windGustQty_var))         get_nc_data(&in_windGustQty_var, windGustQty_arr, buf_size, i_hdr_s);
      else memset(windGustQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_visibilityQty_var))       get_nc_data(&in_visibilityQty_var, visibilityQty_arr, buf_size, i_hdr_s);
      else memset(visibilityQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_precipRateQty_var))       get_nc_data(&in_precipRateQty_var, precipRateQty_arr, buf_size, i_hdr_s);
      else memset(precipRateQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_solarRadiationQty_var))   get_nc_data(&in_solarRadiationQty_var, solarRadiationQty_arr, buf_size, i_hdr_s);
      else memset(solarRadiationQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_seaSurfaceTempQty_var))   get_nc_data(&in_seaSurfaceTempQty_var, seaSurfaceTempQty_arr, buf_size, i_hdr_s);
      else memset(seaSurfaceTempQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_totalColumnPWVQty_var))   get_nc_data(&in_totalColumnPWVQty_var, totalColumnPWVQty_arr, buf_size, i_hdr_s);
      else memset(totalColumnPWVQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_soilTemperatureQty_var))  get_nc_data(&in_soilTemperatureQty_var, soilTemperatureQty_arr, buf_size, i_hdr_s);
      else memset(soilTemperatureQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_minTemp24HourQty_var))    get_nc_data(&in_minTemp24HourQty_var, minTemp24HourQty_arr, buf_size, i_hdr_s);
      else memset(minTemp24HourQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_maxTemp24HourQty_var))    get_nc_data(&in_maxTemp24HourQty_var, maxTemp24HourQty_arr, buf_size, i_hdr_s);
      else memset(maxTemp24HourQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_precip3hrQty_var))        get_nc_data(&in_precip3hrQty_var, precip3hrQty_arr, buf_size, i_hdr_s);
      else memset(precip3hrQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_precip6hrQty_var))        get_nc_data(&in_precip6hrQty_var, precip6hrQty_arr, buf_size, i_hdr_s);
      else memset(precip6hrQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_precip12hrQty_var))       get_nc_data(&in_precip12hrQty_var, precip12hrQty_arr, buf_size, i_hdr_s);
      else memset(precip12hrQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_precip10minQty_var))      get_nc_data(&in_precip10minQty_var, precip10minQty_arr, buf_size, i_hdr_s);
      else memset(precip10minQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_precip1minQty_var))       get_nc_data(&in_precip1minQty_var, precip1minQty_arr, buf_size, i_hdr_s);
      else memset(precip1minQty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_windDir10Qty_var))        get_nc_data(&in_windDir10Qty_var, windDir10Qty_arr, buf_size, i_hdr_s);
      else memset(windDir10Qty_arr, 0, buf_size*sizeof(char));
      if (!IS_INVALID_NC(in_windSpeed10Qty_var))      get_nc_data(&in_windSpeed10Qty_var, windSpeed10Qty_arr, buf_size, i_hdr_s);
      else memset(windSpeed10Qty_arr, 0, buf_size*sizeof(char));

      get_filtered_nc_data(in_temperature_var,      temperature_arr,      buf_size, i_hdr_s, "temperature"     );
      get_filtered_nc_data(in_dewpoint_var,         dewpoint_arr,         buf_size, i_hdr_s, "dewpoint"        );
      get_filtered_nc_data(in_relHumidity_var,      relHumidity_arr,      buf_size, i_hdr_s, "relHumidity"     );
      get_filtered_nc_data(in_stationPressure_var,  stationPressure_arr,  buf_size, i_hdr_s, "stationPressure" );
      get_filtered_nc_data(in_seaLevelPressure_var, seaLevelPressure_arr, buf_size, i_hdr_s, "seaLevelPressure");
      get_filtered_nc_data(in_windDir_var,          windDir_arr,          buf_size, i_hdr_s, "windDir"         );
      get_filtered_nc_data(in_windSpeed_var,        windSpeed_arr,        buf_size, i_hdr_s, "windSpeed"       );
      get_filtered_nc_data(in_windGust_var,         windGust_arr,         buf_size, i_hdr_s, "windGust"        );
      get_filtered_nc_data(in_visibility_var,       visibility_arr,       buf_size, i_hdr_s, "visibility"      );
      get_filtered_nc_data(in_precipRate_var,       precipRate_arr,       buf_size, i_hdr_s, "precipRate"      );
      get_filtered_nc_data(in_solarRadiation_var,   solarRadiation_arr,   buf_size, i_hdr_s, "solarRadiation"  );
      get_filtered_nc_data(in_seaSurfaceTemp_var,   seaSurfaceTemp_arr,   buf_size, i_hdr_s, "seaSurfaceTemp"  );
      get_filtered_nc_data(in_totalColumnPWV_var,   totalColumnPWV_arr,   buf_size, i_hdr_s, "totalColumnPWV"  );
      get_filtered_nc_data(in_soilTemperature_var,  soilTemperature_arr,  buf_size, i_hdr_s, "soilTemperature" );
      get_filtered_nc_data(in_minTemp24Hour_var,    minTemp24Hour_arr,    buf_size, i_hdr_s, "minTemp24Hour"   );
      get_filtered_nc_data(in_maxTemp24Hour_var,    maxTemp24Hour_arr,    buf_size, i_hdr_s, "maxTemp24Hour"   );
      get_filtered_nc_data(in_precip3hr_var,        precip3hr_arr,        buf_size, i_hdr_s, "precip3hr"       );
      get_filtered_nc_data(in_precip6hr_var,        precip6hr_arr,        buf_size, i_hdr_s, "precip6hr"       );
      get_filtered_nc_data(in_precip12hr_var,       precip12hr_arr,       buf_size, i_hdr_s, "precip12hr"      );
      get_filtered_nc_data(in_precip10min_var,      precip10min_arr,      buf_size, i_hdr_s, "precip10min"     );
      get_filtered_nc_data(in_precip1min_var,       precip1min_arr,       buf_size, i_hdr_s, "precip1min"      );
      get_filtered_nc_data(in_windDir10_var,        windDir10_arr,        buf_size, i_hdr_s, "windDir10"       );
      get_filtered_nc_data(in_windSpeed10_var,      windSpeed10_arr,      buf_size, i_hdr_s, "windSpeed10"     );

      dim[1] = hdr_sid_len;
      get_nc_data(&in_hdr_sid_var, (char *)hdr_sid_arr, dim, cur);

      dim[0] = 1;
      dim[1] = 1;

      for (int i_idx=0; i_idx<buf_size; i_idx++) {

         //
         // Mapping of NetCDF variable names from input to output:
         // Output                    = Input
         // hdr_typ                   = ADPSFC (MESONET observations are at the surface)
         // hdr_sid                   = stationId (maxStaIdLen = 6)
         // hdr_vld (YYYYMMDD_HHMMSS) = observationTime (unixtime)
         // hdr_arr[0](Lat)           = latitude
         // hdr_arr[1](Lon)           = longitude
         // hdr_arr[2](Elv)           = elevation
         //

         count = 0;
         i_hdr = i_hdr_s + i_idx;
         mlog << Debug(3) << "Record Number: " << i_hdr << "\n";

         //
         // Use cur to index into the NetCDF variables.
         //
         cur[0] = i_hdr;

         //
         // Process the latitude, longitude, and elevation.
         //
         hdr_arr[0] = hdr_lat_arr[i_idx];
         hdr_arr[1] = hdr_lon_arr[i_idx];
         hdr_arr[2] = hdr_elv_arr[i_idx];

         //
         // Check masking regions.
         //
         if(!check_masks(hdr_arr[0], hdr_arr[1], hdr_sid_arr[i_idx])) continue;


         //
         // Process the station name.
         //
         hdr_sid = hdr_sid_arr[i_idx];

         //
         // Process the observation time.
         //
         tmp_dbl = tmp_dbl_arr[i_idx];
         if(is_bad_data(tmp_dbl)) continue;
         hdr_vld = (time_t)tmp_dbl;

         hdr_idx = get_nc_hdr_cur_index();

         //
         // Initialize the observation array: hdr_id, gc, lvl, hgt, ob
         //
         obs_arr[0] = (float)hdr_idx;   // Index into header array
         obs_arr[2] = bad_data_float;   // Level: accum(sec) or pressure
         obs_arr[3] = 0;                // Height for surface is 0 meters

         // Temperature
         obs_arr[4] = temperature_arr[i_idx];
         count += process_obs(11, conversion, obs_arr, temperatureQty_arr[i_idx],
                     in_temperature_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Dewpoint
         obs_arr[4] = dewpoint_arr[i_idx];
         count += process_obs(17, conversion, obs_arr, dewpointQty_arr[i_idx],
                     in_dewpoint_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Relative Humidity
         obs_arr[4] = relHumidity_arr[i_idx];
         count += process_obs(52, conversion, obs_arr, relHumidityQty_arr[i_idx],
                     in_relHumidity_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Station Pressure
         obs_arr[4] = stationPressure_arr[i_idx];
         count += process_obs(1, conversion, obs_arr, stationPressureQty_arr[i_idx],
                     in_stationPressure_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Sea Level Pressure
         obs_arr[4] = seaLevelPressure_arr[i_idx];
         count += process_obs(2, conversion, obs_arr, seaLevelPressureQty_arr[i_idx],
                     in_seaLevelPressure_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Wind Direction
         obs_arr[4] = windDir_arr[i_idx];
         count += process_obs(31, conversion, obs_arr, windDirQty_arr[i_idx],
                     in_windDir_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);
         wdir = obs_arr[4];

         // Wind Speed
         obs_arr[4] = windSpeed_arr[i_idx];
         char qty = windSpeedQty_arr[i_idx];
         count += process_obs(32, conversion, obs_arr, qty,
                     in_windSpeed_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);
         wind = obs_arr[4];

         // Convert the wind direction and speed into U and V components
         convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);

         // Write U-component of wind
         obs_arr[4] = ugrd;
         count += process_obs(33, conversion, obs_arr, qty, in_windSpeed_var,
                     hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Write V-component of wind
         obs_arr[4] = vgrd;
         count += process_obs(34, conversion, obs_arr, qty, in_windSpeed_var,
                     hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Wind Gust
         obs_arr[4] = windGust_arr[i_idx];
         count += process_obs(180, conversion, obs_arr, windGustQty_arr[i_idx],
                     in_windGust_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Visibility
         obs_arr[4] = visibility_arr[i_idx];
         count += process_obs(20, conversion, obs_arr, visibilityQty_arr[i_idx],
                     in_visibility_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Precipitation Rate
         // Convert input meters/second to output millimeters/second
         obs_arr[4] = precipRate_arr[i_idx];
         count += process_obs(59, 1000.0, obs_arr, precipRateQty_arr[i_idx],
                     in_precipRate_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Solar Radiation
         obs_arr[4] = solarRadiation_arr[i_idx];
         count += process_obs(250, conversion, obs_arr, solarRadiationQty_arr[i_idx],
                     in_solarRadiation_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Sea Surface Temperature
         obs_arr[4] = seaSurfaceTemp_arr[i_idx];
         count += process_obs(80, conversion, obs_arr, seaSurfaceTempQty_arr[i_idx],
                     in_seaSurfaceTemp_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Precipitable Water
         // Convert input cm to output mm
         obs_arr[4] = totalColumnPWV_arr[i_idx];
         count += process_obs(54, 10.0, obs_arr, totalColumnPWVQty_arr[i_idx],
                     in_totalColumnPWV_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Soil Temperature
         obs_arr[4] = soilTemperature_arr[i_idx];
         count += process_obs(85, conversion, obs_arr, soilTemperatureQty_arr[i_idx],
                     in_soilTemperature_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Minimum Temperature
         obs_arr[4] = minTemp24Hour_arr[i_idx];
         count += process_obs(16, conversion, obs_arr, minTemp24HourQty_arr[i_idx],
                     in_minTemp24Hour_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Maximum Temperature
         obs_arr[4] = maxTemp24Hour_arr[i_idx];
         count += process_obs(15, conversion, obs_arr, maxTemp24HourQty_arr[i_idx],
                     in_maxTemp24Hour_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Precipitation - 3 Hour
         obs_arr[2] = 3.0*sec_per_hour;
         obs_arr[4] = precip3hr_arr[i_idx];
         count += process_obs(61, conversion, obs_arr, precip3hrQty_arr[i_idx],
                     in_precip3hr_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Precipitation - 6 Hour
         obs_arr[2] = 6.0*sec_per_hour;
         obs_arr[4] = precip6hr_arr[i_idx];
         count += process_obs(61, conversion, obs_arr, precip6hrQty_arr[i_idx],
                     in_precip6hr_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Precipitation - 12 Hour
         obs_arr[2] = 12.0*sec_per_hour;
         obs_arr[4] = precip12hr_arr[i_idx];
         count += process_obs(61, conversion, obs_arr, precip12hrQty_arr[i_idx],
                     in_precip12hr_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Precipitation - 10 minutes
         obs_arr[2] = 600;
         obs_arr[4] = precip10min_arr[i_idx];
         count += process_obs(61, conversion, obs_arr, precip10minQty_arr[i_idx],
                     in_precip10min_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Precipitation - 1 minutes
         obs_arr[2] = 60;
         obs_arr[4] = precip1min_arr[i_idx];
         count += process_obs(61, conversion, obs_arr, precip1minQty_arr[i_idx],
                     in_precip1min_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Set the level to bad data and the height to 10 meters
         obs_arr[2] = bad_data_float;
         obs_arr[3] = 10;

         // 10m Wind Direction
         obs_arr[4] = windDir10_arr[i_idx];
         count += process_obs(31, conversion, obs_arr, windDir10Qty_arr[i_idx],
                     in_windDir10_var, hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);
         wdir = obs_arr[4];

         // 10m Wind Speed
         qty = windSpeed10Qty_arr[i_idx];
         obs_arr[4] = windSpeed10_arr[i_idx];
         count += process_obs(32, conversion, obs_arr, qty, in_windSpeed10_var,
                     hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);
         wind = obs_arr[4];

         // Convert the wind direction and speed into U and V components
         convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);

         // Write U-component of 10m wind
         obs_arr[4] = ugrd;
         count += process_obs(33, conversion, obs_arr, qty, in_windSpeed10_var,
                     hdr_typ, hdr_sid, hdr_vld,
                     hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         // Write V-component of 10m wind
         obs_arr[4] = vgrd;
         //count += process_obs(34, conversion, obs_arr, qty, in_windSpeed10_var,
         //            hdr_typ, hdr_sid, hdr_vld,
         //            hdr_arr[0], hdr_arr[1], hdr_arr[2]);
         process_obs(34, conversion, obs_arr, qty, in_windSpeed10_var,
            hdr_typ, hdr_sid, hdr_vld,
            hdr_arr[0], hdr_arr[1], hdr_arr[2]);

      }

   } // end for i

   print_rej_counts();

   //
   // Cleanup
   //
   if(cur) { delete [] cur; cur = (long *) 0; }
   if(dim) { delete [] dim; dim = (long *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_madis_acarsProfiles(NcFile *&f_in) {
   int nhdr, nlvl,nlvl1, i_lvl, maxLevels;
   long i_hdr, i_cnt, i_hdr_s;
   int hdr_sid_len;
   int buf_size;
   double tmp_dbl, tmp_dbl2, tmp_dbl1;
   char qty;
   time_t hdr_vld;
   ConcatString hdr_typ, hdr_sid;
   float hdr_arr[hdr_arr_len], obs_arr[obs_arr_len], conversion;
   float pressure, wdir, wind, ugrd, vgrd;
   int count;
   StringArray missing_vars, missing_qty_vars;
   const char * method_name = "process_madis_acarsProfiles() -> ";

   //
   // Input header variables:
   // Note: hdr_typ is always set to AIRCFT
   //
   NcVar in_hdr_sid_var = get_var(f_in, "profileAirport");
   NcVar in_hdr_vld_var = get_var(f_in, "profileTime");
   NcVar in_hdr_lat_var = get_var(f_in, "trackLat");
   NcVar in_hdr_lon_var = get_var(f_in, "trackLon");
   NcVar in_hdr_elv_var = get_var(f_in, "altitude");
   NcVar in_hdr_tob_var = get_var(f_in, "obsTimeOfDay");

   NcVar in_temperature_var = get_var(f_in, "temperature");
   NcVar in_dewpoint_var = get_var(f_in, "dewpoint");
   NcVar in_windDir_var = get_var(f_in, "windDir");
   NcVar in_windSpeed_var = get_var(f_in, "windSpeed");

   NcVar in_temperatureQty_var = get_var(f_in, "temperatureDD");
   NcVar in_dewpointQty_var = get_var(f_in, "dewpointDD");
   NcVar in_windDirQty_var = get_var(f_in, "windDirDD");
   NcVar in_windSpeedQty_var = get_var(f_in, "windSpeedDD");
   NcVar in_nLevelsQty_var = get_var(f_in, "nLevelsDD");
   NcVar in_altitudeQty_var = get_var(f_in, "altitudeDD");


   if (IS_INVALID_NC(in_hdr_sid_var)) missing_vars.add("profileAirport");
   if (IS_INVALID_NC(in_hdr_vld_var)) missing_vars.add("profileTime");
   if (IS_INVALID_NC(in_hdr_lat_var)) missing_vars.add("trackLat");
   if (IS_INVALID_NC(in_hdr_lon_var)) missing_vars.add("trackLon");
   if (IS_INVALID_NC(in_hdr_elv_var)) missing_vars.add("altitude");
   if (IS_INVALID_NC(in_hdr_tob_var)) missing_vars.add("obsTimeOfDay");

   if (IS_INVALID_NC(in_temperature_var)) missing_vars.add("temperature");
   if (IS_INVALID_NC(in_dewpoint_var)) missing_vars.add("dewpoint");
   if (IS_INVALID_NC(in_windDir_var)) missing_vars.add("windDir");
   if (IS_INVALID_NC(in_windSpeed_var)) missing_vars.add("windSpeed");

   if (IS_INVALID_NC(in_temperatureQty_var)) missing_qty_vars.add("temperatureDD");
   if (IS_INVALID_NC(in_dewpointQty_var)) missing_qty_vars.add("dewpointDD");
   if (IS_INVALID_NC(in_windDirQty_var)) missing_qty_vars.add("windDirDD");
   if (IS_INVALID_NC(in_windSpeedQty_var)) missing_qty_vars.add("windSpeedDD");
   if (IS_INVALID_NC(in_nLevelsQty_var)) missing_qty_vars.add("nLevelsDD");
   if (IS_INVALID_NC(in_altitudeQty_var)) missing_qty_vars.add("altitudeDD");

   //
   // Retrieve applicable dimensions
   //
   hdr_sid_len  = get_dim_value(f_in, "AirportIdLen");
   nhdr         = get_dim_value(f_in, in_recNum_str);
   maxLevels    = get_dim_value(f_in, "maxLevels");

   int my_rec_end = (rec_end == 0) ? nhdr : rec_end;

   //
   // Initialize variables for processing observations
   //
   conversion = 1.0;
   nlvl1 = 0;

   //
   // Arrays of longs for indexing into NetCDF variables
   //
   long *cur = new long [3];
   cur[0] = cur[1] = cur[2] = 0;
   long *dim = new long [3];
   dim[0] = dim[1] = dim[2] = 1;

   //
   // Get the number of levels
   //
   NcVar in_var = get_var(f_in, "nLevels");
   if (IS_INVALID_NC(in_var)) missing_vars.add("nLevels");
   if (missing_vars.n() > 0) {
      mlog << Error << "\n" << method_name << "Please check if the input is a ACARS Profiles.\n\n";
      for (int idx=0; idx<missing_vars.n(); idx++)
         mlog << Warning << "    missing variable: " << missing_vars[idx] << "\n";
      exit(1);
   }
   if (missing_qty_vars.n() > 0) {
      for (int idx=0; idx<missing_qty_vars.n(); idx++)
         mlog << Warning << "    missing Qty variable: " << missing_qty_vars[idx] << "\n";
   }

   //
   // Obtain the total number of levels
   //
   //buf_size = ((my_rec_end - rec_beg) > BUFFER_SIZE) ? BUFFER_SIZE: (my_rec_end - rec_beg);
   buf_size = (my_rec_end - rec_beg);   // read all

   int  levels[buf_size];
   char levelsQty[buf_size];
   cur[0] = rec_beg;
   dim[0] = buf_size;
   get_nc_data(&in_var, levels, buf_size, cur[0]);
   if (!IS_INVALID_NC(in_nLevelsQty_var)) get_nc_data(&in_nLevelsQty_var, (char *)&levelsQty, buf_size, cur[0]);
   for(i_hdr=0; i_hdr<buf_size; i_hdr++) {
      nlvl1 += levels[i_hdr];
   }
   cur[0] = 0;
   dim[0] = 1;

   //
   // Setup the output NetCDF file
   //
   //setup_netcdf_out(nlvl1);

   mlog << Debug(2) << "Processing ACARS Profiles recs\t\t= " << nlvl1 << "\n";

   //
   // Initialize
   //
   i_cnt = -1;
   int hdr_idx = 0;

   //
   // Loop through each record and get the header data.
   //
   for(i_hdr_s=rec_beg; i_hdr_s<my_rec_end; i_hdr_s+=BUFFER_SIZE) {
      buf_size = ((my_rec_end - i_hdr_s) > BUFFER_SIZE) ? BUFFER_SIZE: (my_rec_end - i_hdr_s);

      double tmp_dbl_arr[buf_size];
      float hdr_lat_arr[buf_size][maxLevels];
      float hdr_lon_arr[buf_size][maxLevels];
      float hdr_elv_arr[buf_size][maxLevels];
      char hdr_sid_arr[buf_size][hdr_sid_len];

      int obsTimeOfDay_arr[buf_size][maxLevels];
      float temperature_arr[buf_size][maxLevels];
      float dewpoint_arr[buf_size][maxLevels];
      float windDir_arr[buf_size][maxLevels];
      float windSpeed_arr[buf_size][maxLevels];

      char temperatureQty_arr[buf_size][maxLevels];
      char dewpointQty_arr[buf_size][maxLevels];
      char windDirQty_arr[buf_size][maxLevels];
      char windSpeedQty_arr[buf_size][maxLevels];
      char altitudeQty_arr[buf_size][maxLevels];

      nc_buf_size = buf_size * FIELD_COUNT;
      if (nc_buf_size > BUFFER_SIZE) nc_buf_size = BUFFER_SIZE;

      cur[0] = i_hdr_s;
      dim[0] = buf_size;
      dim[1] = maxLevels;
      get_nc_data(&in_hdr_vld_var, tmp_dbl_arr, dim, cur);
      get_nc_data(&in_hdr_lat_var, (float *)hdr_lat_arr, dim, cur);
      get_nc_data(&in_hdr_lon_var, (float *)hdr_lon_arr, dim, cur);
      get_filtered_nc_data_2d(in_hdr_elv_var, (float *)hdr_elv_arr, dim, cur, "elevation");

      if (!IS_INVALID_NC(in_temperatureQty_var)) get_nc_data(&in_temperatureQty_var, (char *)&temperatureQty_arr, dim, cur);
      else memset(temperatureQty_arr, 0, buf_size*dim[1]*sizeof(char));
      if (!IS_INVALID_NC(in_dewpointQty_var))    get_nc_data(&in_dewpointQty_var, (char *)&dewpointQty_arr, dim, cur);
      else memset(dewpointQty_arr, 0, buf_size*dim[1]*sizeof(char));
      if (!IS_INVALID_NC(in_windDirQty_var))     get_nc_data(&in_windDirQty_var, (char *)&windDirQty_arr, dim, cur);
      else memset(windDirQty_arr, 0, buf_size*dim[1]*sizeof(char));
      if (!IS_INVALID_NC(in_windSpeedQty_var))   get_nc_data(&in_windSpeedQty_var, (char *)&windSpeedQty_arr, dim, cur);
      else memset(windSpeedQty_arr, 0, buf_size*dim[1]*sizeof(char));
      if (!IS_INVALID_NC(in_altitudeQty_var))    get_nc_data(&in_altitudeQty_var, (char *)&altitudeQty_arr, dim, cur);
      else memset(altitudeQty_arr, 0, buf_size*dim[1]*sizeof(char));

      get_filtered_nc_data_2d(in_hdr_tob_var,     (int *)&obsTimeOfDay_arr,  dim, cur, "obsTimeOfDay");
      get_filtered_nc_data_2d(in_temperature_var, (float *)&temperature_arr, dim, cur, "temperature");
      get_filtered_nc_data_2d(in_dewpoint_var,    (float *)&dewpoint_arr,    dim, cur, "dewpoint");
      get_filtered_nc_data_2d(in_windDir_var,     (float *)&windDir_arr,     dim, cur, "windDir");
      get_filtered_nc_data_2d(in_windSpeed_var,   (float *)&windSpeed_arr,   dim, cur, "windSpeed");

      dim[1] = hdr_sid_len;
      get_nc_data(&in_hdr_sid_var, (char *)hdr_sid_arr, dim, cur);

      dim[0] = 1;
      dim[1] = 1;

      //
      // Process the header type.
      // For ACARS, store as AIRCFT.
      //
      hdr_typ = "AIRCFT";


      for (int i_idx=0; i_idx<buf_size; i_idx++) {
         i_hdr = i_hdr_s + i_idx;
         mlog << Debug(3) << "Record Number: " << i_hdr << "\n";

         //
         // Use cur to index into the NetCDF variables.
         //
         cur[0] = i_hdr;
         cur[1] = 0;

         //
         // Process the station i.e. airport name.
         //
         hdr_sid = hdr_sid_arr[i_idx];

         //
         // Process the observation time.
         //
         tmp_dbl1 = tmp_dbl_arr[i_idx];
         if(is_bad_data(tmp_dbl1)) continue;


         //
         // Process the number of levels
         //
         check_quality_control_flag(levels[i_idx],
                                    levelsQty[i_idx],
                                    GET_NC_NAME(in_var).c_str());
         nlvl = levels[i_idx];
         obs_arr[2] = levels[i_idx];

         //
         // Loop through each level of each track
         //
         for(i_lvl=0; i_lvl<nlvl; i_lvl++) {

            count = 0;
            i_cnt++;
            mlog << Debug(3) << "  Mandatory Level: " << i_lvl << "\n";

            hdr_idx = get_nc_hdr_cur_index();

            //
            // Use cur to index into the NetCDF variables.
            //
            cur[1] = i_lvl;
            obs_arr[0] = (float) (hdr_idx);

            //
            // Process the latitude, longitude, and elevation.
            //
            check_quality_control_flag(hdr_elv_arr[i_idx][i_lvl],
                                       altitudeQty_arr[i_idx][i_lvl],
                                       GET_NC_NAME(in_hdr_elv_var).c_str());
            hdr_arr[0] = hdr_lat_arr[i_idx][i_lvl];
            hdr_arr[1] = hdr_lon_arr[i_idx][i_lvl];
            hdr_arr[2] = hdr_elv_arr[i_idx][i_lvl];

            //
            // Check masked regions
            //
            if(!check_masks(hdr_arr[0], hdr_arr[1], hdr_sid.c_str())) continue;

            //
            // Get the number of levels  and height for this level
            //
            obs_arr[3] = hdr_elv_arr[i_idx][i_lvl];


            //
            // Check for bad data
            //
            if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;

            //
            // Process the observation time.
            //
            tmp_dbl2 = obsTimeOfDay_arr[i_idx][i_lvl];

            //
            // Write header type
            // Write Airport ID
            // Add to Profile Time
            // Observation Time is relative to time of day
            //
            tmp_dbl = tmp_dbl1+tmp_dbl2-fmod(tmp_dbl1, 86400);
            hdr_vld = (time_t)tmp_dbl;

            //
            // Compute the pressure (hPa) from altitude data
            // Equation obtained from http://www.srh.noaa.gov/
            //
            pressure = 1013.25*pow((1-2.25577e-5*obs_arr[3]),5.25588);

            //
            // Replace number of Levels to Pressure values in Observation Array
            //
            obs_arr[2] = pressure;

            // Temperature
            obs_arr[4] = temperature_arr[i_idx][i_lvl];
            count += process_obs(11, conversion, obs_arr, temperatureQty_arr[i_idx][i_lvl],
                        in_temperature_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

            // Dewpoint
            obs_arr[4] = dewpoint_arr[i_idx][i_lvl];
            count += process_obs(17, conversion, obs_arr, dewpointQty_arr[i_idx][i_lvl],
                        in_dewpoint_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

            // Wind Direction
            obs_arr[4] = windDir_arr[i_idx][i_lvl];
            count += process_obs(31, conversion, obs_arr, windDirQty_arr[i_idx][i_lvl],
                        in_windDir_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);
            wdir = obs_arr[4];

            // Wind Speed
            obs_arr[4] = windSpeed_arr[i_idx][i_lvl];
            qty = windSpeedQty_arr[i_idx][i_lvl];
            count += process_obs(32, conversion, obs_arr, qty,
                        in_windSpeed_var, hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);
            wind = obs_arr[4];

            // Convert the wind direction and speed into U and V components
            convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);

            // Write U-component of wind
            obs_arr[4] = ugrd;
            count += process_obs(33, conversion, obs_arr, qty, in_windSpeed_var,
                        hdr_typ, hdr_sid, hdr_vld,
                        hdr_arr[0], hdr_arr[1], hdr_arr[2]);

            // Write V-component of wind
            obs_arr[4] = vgrd;
            //count += process_obs(34, conversion, obs_arr, qty, in_windSpeed_var,
            //            hdr_typ, hdr_sid, hdr_vld,
            //            hdr_arr[0], hdr_arr[1], hdr_arr[2]);
            process_obs(34, conversion, obs_arr, qty, in_windSpeed_var,
               hdr_typ, hdr_sid, hdr_vld,
               hdr_arr[0], hdr_arr[1], hdr_arr[2]);

         } // end for i_lvl
      }
   } // end for i_hdr

   print_rej_counts();

   //
   // Cleanup
   //
   if(cur) { delete [] cur; cur = (long *) 0; }
   if(dim) { delete [] dim; dim = (long *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\nUsage: "
        << program_name << "\n"
        << "\tmadis_file [madis_file2 ... madis_filen]\n"
        << "\tout_file\n"
        << "\t-type str\n"
        << "\t[-config file]\n"
        << "\t[-qc_dd list]\n"
        << "\t[-lvl_dim list]\n"
        << "\t[-rec_beg n]\n"
        << "\t[-rec_end n]\n"
        << "\t[-mask_grid string]\n"
        << "\t[-mask_poly file]\n"
        << "\t[-mask_sid file|list]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"madis_file\" is the MADIS NetCDF point "
        << "observation file (required).\n"

        << "\t\t\"out_file\" is the output NetCDF file (required).\n"

        << "\t\t\"-type str\" specifies the type of MADIS observations "
        << "(metar, raob, profiler, maritime, mesonet, or acarsProfiles) "
        << "(required).\n"

        << "\t\t\"-config file\" uses the specified configuration file "
        << "to generate time summaries of the MADIS data (optional).\n"

        << "\t\t\"-qc_dd list\" specifies a comma-separated list of "
        << "QC flag values to be accepted (Z,C,S,V,X,Q,K,G,B) "
        << "(optional).\n"

        << "\t\t\"-lvl_dim list\" specifies a comma-separated list of "
        << "vertical level dimensions to be processed. (optional).\n"

        << "\t\t\"-rec_beg n\" specifies the index of the first "
        << "MADIS record to process, zero-based (optional).\n"

        << "\t\t\"-rec_end n\" specifies the index of the last "
        << "MADIS record to process, zero-based (optional).\n"

        << "\t\t\"-mask_grid string\" is a named grid or a data file "
        << "defining grid for filtering the point observations spatially (optional).\n"

        << "\t\t\"-mask_poly file\" is a polyline masking file for filtering "
        << "the point observations spatially (optional).\n"

        << "\t\t\"-mask_sid file|list\" is a station ID masking file or a "
        << "comma-separated list of station ID's for filtering the point "
        << "observations spatially (optional).\n"
        << "\t\t   For a list of length greater than one, the first element is "
        << "stored as the mask name (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t\"-compress level\" specifies the compression level of "
        << "output NetCDF variable (optional).\n\n"

        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_type(const StringArray & a)
{
   //
   // Parse the MADIS type
   //
   if(strcasecmp(a[0].c_str(), metar_str) == 0) {
      mtype = madis_metar;
   }
   else if(strcasecmp(a[0].c_str(), raob_str) == 0) {
      mtype = madis_raob;
   }
   else if(strcasecmp(a[0].c_str(), profiler_str) == 0) {
      mtype = madis_profiler;
   }
   else if(strcasecmp(a[0].c_str(), maritime_str) == 0) {
      mtype = madis_maritime;
   }
   else if(strcasecmp(a[0].c_str(), mesonet_str) == 0) {
      mtype = madis_mesonet;
   }
   else if(strcasecmp(a[0].c_str(), acarsProfiles_str) == 0) {
      mtype = madis_acarsProfiles;
   }
   else {
      mlog << Error << "\nprocess_command_line() -> "
            << "MADIS type \"" << a[0]
            << "\" not currently supported.\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

void set_qc_dd(const StringArray & a)
{
   //
   // Parse the list of QC flags to be used
   //
   qc_dd_sa.parse_css(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_lvl_dim(const StringArray & a)
{
   //
   // Parse the list vertical level dimensions to be processed
   //
   lvl_dim_sa.parse_css(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_rec_beg(const StringArray & a)
{
   rec_beg = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_rec_end(const StringArray & a)
{
   rec_end = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a)
{
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
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
   ConcatString mask_name;

   // List the station ID mask
   mlog << Debug(1)
        << "Station ID Mask: " << a[0] << "\n";

   parse_sid_mask(a[0], mask_sid, mask_name);

   // List the length of the station ID mask
   mlog << Debug(2)
        << "Parsed Station ID Mask: " << mask_name
        << " containing " << mask_sid.n_elements() << " points\n";
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

void set_config(const StringArray & a) {
   config_filename = a[0];
}

////////////////////////////////////////////////////////////////////////
