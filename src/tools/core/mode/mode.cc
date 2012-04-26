
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////
//
//   Filename:   mode.cc
//
//   Description:
//      Based on user specified parameters, this tool derives objects
//      within two gridded datasets using a convolution-thresholding
//      approach.  It then compares objects within the same field and
//      across fields, and calculates a total interest value for each
//      pair of objects using a fuzzy logic approach.  The interest
//      values are thresholded, and object pairs with a high enough
//      interest value are associated with one another.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    10-11-06  Halley Gotway  New
//   001    12-20-06  Halley Gotway  Write check_xy_ll to reduce
//          duplicated code.
//   002    01/11/08  Halley Gotway  Modify sprintf statements which
//          use the GRIB code abbreviation string
//   003    02/06/08  Halley Gotway  Modify to read the updated NetCDF
//          output of PCP-Combine
//   004    09/23/08  Halley Gotway  Add two output fields to the
//                    NetCDF object file for the raw fcst/obs values.
//   005    09/23/08  Halley Gotway  Change argument sequence for the
//                    get_grib_record routine.
//   006    05/03/10  Halley Gotway  Remove the variable/level info
//                    from the output file naming convention.
//   007    05/11/10  Halley Gotway  Plot polyline lines thicker.
//   008    06/30/10  Halley Gotway  Enhance grid equality checks.
//   009    07/27/10  Halley Gotway  Add lat/lon variables to NetCDF.
//   010    08/09/10  Halley Gotway  Add valid time variable attributes
//                    to NetCDF output.
//   011    10/28/11  Holmes         Added use of command line class to
//                                   parse the command line arguments.
//   012    11/15/11  Holmes         Added code to enable reading of
//                                   multiple config files.
//
//   013    01/11/11  Bullock        Ported to new repository
//
///////////////////////////////////////////////////////////////////////

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
#include "grib_classes.h"

#include "vx_data2d.h"
#include "vx_data2d_factory.h"
#include "data2d_factory_utils.h"
#include "vx_shapedata.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_nc_util.h"
#include "vx_cal.h"
#include "vx_grid.h"
#include "vx_math.h"
#include "vx_statistics.h"

#include "vx_color.h"
#include "vx_ps.h"
#include "vx_pxm.h"
#include "vx_render.h"
#include "vx_plot_util.h"

///////////////////////////////////////////////////////////////////////
//
// Create output files using the following naming convention:
//
//    mode_YYYYMMDDI_FHH_HHA.ps/.txt
//
//    Where I indicates initilization time, L indicates lead time,
//    and A indicates accumulation time.
//
///////////////////////////////////////////////////////////////////////

static const char * program_name = "mode";

///////////////////////////////////////////////////////////////////////

enum EngineType {
   NoEng = 0,
   FOEng = 1,
   FFEng = 2,
   OOEng = 3
};

///////////////////////////////////////////////////////////////////////

static const char * default_config_filename = "MET_BASE/data/config/MODEConfig_default";

// static const char * default_out_dir = "MET_BASE/out/mode";
static const char * default_out_dir = ".";

static const int unmatched_id = -1;

// Input configuration files
static ConcatString default_config_file;
static ConcatString match_config_file;
static ConcatString merge_config_file;

// Input files
static ConcatString fcst_file;
static ConcatString obs_file;

static unixtime     fcst_valid_ut = (unixtime) 0;
static int          fcst_lead_sec = bad_data_int;
static unixtime     obs_valid_ut  = (unixtime) 0;
static int          obs_lead_sec  = bad_data_int;

static const int n_cts = 3;
static const char *cts_str[n_cts] = {"RAW", "FILTER", "OBJECT"};
static TTContingencyTable cts[n_cts];

static Engine engine;
static Grid grid;
static Box xy_bb;
static ConcatString out_dir;
static ConcatString met_data_dir;

// Grib Codes to be verified for the forecast and observation fields
static VarInfo * fcst_info = (VarInfo *) 0;
static VarInfo *  obs_info = (VarInfo *) 0;
static double data_min, data_max;

///////////////////////////////////////////////////////////////////////
//
// Plotting Info
//
///////////////////////////////////////////////////////////////////////

static ColorTable fcst_ct, obs_ct;
static int stride = 1;
static int plot_flag = 1;
static int obj_stat_flag = 1;
static int obj_plot_flag = 1;
static int ct_stat_flag = 1;
static const Color c_map(25, 25, 25);
static const Color c_hull(0, 0, 0);
static const Color c_bndy(0, 0, 255);
static const double l_thin = 0.50;
static const double l_thick = 1.00;

static int n_page;

static const double page_width = 8.5*72.0;
static const double page_height = 11.0*72.0;

static const double h_margin = 20.0;
static const double v_margin = 20.0;

static const double plot_text_sep = 15.0;

static const Box sm_pane_bb(h_margin, 450.0, 270.0, 720.0);

static const Box lg_pane_bb( h_margin, page_width-3.0*h_margin, v_margin, page_height-4.0*v_margin);

static const double h_tab_cen = page_width/2.0;

static double sm_plot_height, lg_plot_height;

static double h_tab_1, h_tab_2, h_tab_3;
static double v_tab_1, v_tab_2, v_tab_3;

static Color c_fcst_fill(150, 150, 150);
static Color c_obs_fill(150, 150, 150);

///////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_fcst_obs_files();
static void process_masks(ShapeData &, ShapeData &);
static void process_output();

static void check_engine_config();
static void compute_ct_stats();

static void set_plot_dims(int, int);
static void plot_engine();
static void plot_engine(PSfile &, Engine &, EngineType, const char *);
static void plot_threshold_merging(PSfile &, Engine &, const char *, int);

static void draw_border(PSfile &, Box &);
static void draw_map(PSfile &, Box &);
static void render_image(PSfile &, Engine &, EngineType, const ShapeData &, Box &,
                         int, int);
static void plot_simple_ids(PSfile &, Engine &, Box &, int);
static void draw_convex_hulls(PSfile &, Engine &, const Box &, int, int);
static void draw_boundaries(PSfile &, Engine &, const Box &, int);
static void draw_boundaries(PSfile &, ShapeData &, int, const Box &);
static void draw_polyline(PSfile &, Polyline &, const Color &, Box, bool);
static void plot_colorbar(PSfile &, Box &, int);
static void set_dim(Box &, double, double, double);
static void set_xy_bb();
static void valid_xy_bb(const ShapeData *, Box &);
static void build_outfile_name(const char *, ConcatString &);
static void write_obj_stats();
static void write_obj_netcdf();
static void write_bdy_netcdf(NcFile *);
static void write_fcst_bdy_netcdf(NcFile *);
static void write_obs_bdy_netcdf(NcFile *);
static void write_ct_stats();
static void usage();
static void set_config_merge_file(const StringArray &);
static void set_fcst_valid_time(const StringArray &);
static void set_fcst_lead_time(const StringArray &);
static void set_obs_valid_time(const StringArray &);
static void set_obs_lead_time(const StringArray &);
static void set_outdir(const StringArray &);
static void set_plot(const StringArray &);
static void set_obj_plot(const StringArray &);
static void set_obj_stat(const StringArray &);
static void set_ct_stat(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);


///////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])

{

   //
   // Set handler to be called for memory allocation error
   //
   set_new_handler(oom);

   //
   // Process the command line arguments
   //
   process_command_line(argc, argv);

   //
   // Process the forecast and observation files
   //
   process_fcst_obs_files();

   //
   // Write output files
   //
   process_output();

   if ( fcst_info )  { delete fcst_info;  fcst_info = (VarInfo *) 0; }
   if (  obs_info )  { delete  obs_info;   obs_info = (VarInfo *) 0; }

   return(0);
}

///////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv)

{

   CommandLine cline;
   ConcatString grib_ptv_str;
   ConcatString s;
   GrdFileType ftype = FileType_None;
   GrdFileType otype = FileType_None;
   VarInfoFactory vfactory;

   // Set the default output directory
   out_dir = replace_path(default_out_dir);

   //
   // check for zero arguments
   //
   if(argc == 1)
      usage();

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
   cline.add(set_config_merge_file, "-config_merge", 1);
   cline.add(set_fcst_valid_time, "-fcst_valid", 1);
   cline.add(set_fcst_lead_time, "-fcst_lead", 1);
   cline.add(set_obs_valid_time, "-obs_valid", 1);
   cline.add(set_obs_lead_time, "-obs_lead", 1);
   cline.add(set_outdir, "-outdir", 1);
   cline.add(set_plot, "-plot", 0);
   cline.add(set_obj_plot, "-obj_plot", 0);
   cline.add(set_obj_stat, "-obj_stat", 0);
   cline.add(set_ct_stat, "-ct_stat", 0);
   cline.add(set_logfile, "-log", 1);
   cline.add(set_verbosity, "-v", 1);

   //
   // parse the command line
   //
   cline.parse();

   //
   // Check for error. There should be three arguments left; the
   // forecast filename, the observation filename, and the config
   // filename.
   //
   if(cline.n() != 3)
      usage();

   //
   // Store the input forecast and observation file names
   //
   fcst_file         = cline[0];
   obs_file          = cline[1];
   match_config_file = cline[2];

   // Determine the input file types
   ftype = grd_file_type(fcst_file);
   otype = grd_file_type(obs_file);

   fcst_info = vfactory.new_var_info(ftype);
    obs_info = vfactory.new_var_info(otype);

   //
   // If the merge config file was not set using the optional
   // arguments, then set it to the match config file
   //
   if(merge_config_file.length() == 0)
      merge_config_file = match_config_file;

   //
   // Read the default config file first and then read the user's match config file
   //
   default_config_file = replace_path(default_config_filename);

   // List the config files
   mlog << Debug(1)
        << "Default Config File: " << default_config_file << "\n"
        << "Match Config File: " << match_config_file << "\n"
        << "Merge Config File: " << merge_config_file << "\n";

   //
   // Read the default and match config files
   //
   engine.wconf.read(default_config_file);
   engine.wconf.read(match_config_file);

   //
   // Process the engine configuration
   //
   engine.process_engine_config();

   //
   // Check the configuration of the engine
   //
   check_engine_config();

   //
   // Parse out the GRIB code and level information for the forecast
   // and observation GRIB codes.
   //

   grib_ptv_str << cs_erase << (engine.wconf.grib_ptv().ival());

   fcst_info = vfactory.new_var_info(ftype);
    obs_info = vfactory.new_var_info(otype);

   fcst_info->set_pair(CONFIG_GRIB_PTV, grib_ptv_str);
   fcst_info->set_magic(engine.wconf.fcst_field().sval());
   if ( fcst_valid_ut != 0 )  fcst_info->set_valid(fcst_valid_ut);
   if ( !is_bad_data(fcst_lead_sec) ) fcst_info->set_lead(fcst_lead_sec);

   obs_info->set_pair(CONFIG_GRIB_PTV, grib_ptv_str);
   obs_info->set_magic(engine.wconf.obs_field().sval());
   if ( obs_valid_ut != 0 )  obs_info->set_valid(obs_valid_ut);
   if ( !is_bad_data(obs_lead_sec) ) obs_info->set_lead(obs_lead_sec);

   //
   // Check to make sure that MODE can be run on the GRIB codes specified
   //
   if(fcst_info->is_wind_direction() || obs_info->is_wind_direction()) {
      mlog << Error << "\nmain() -> "
           << "mode may not be run on a wind direction field.\n\n";
      exit(1);
   }

   //
   // List the input files
   //
   mlog << Debug(1)
        << "Forecast File: " << fcst_file << "\n"
        << "Observation File: " << obs_file << "\n";

   return;
}

///////////////////////////////////////////////////////////////////////

void process_fcst_obs_files()

{

   ShapeData fcst_wd, obs_wd;
   char tmp_str[max_str_len], tmp2_str[max_str_len];
   char merge_str[max_str_len];
   double fmin, omin, fmax, omax;
   Met2dDataFileFactory factory;
   Met2dDataFile * fcst_mtddf = (Met2dDataFile *) 0;
   Met2dDataFile * obs_mtddf  = (Met2dDataFile *) 0;
   

   //
   // Read the gridded data from the input forecast file
   //

   fcst_mtddf = factory.new_met_2d_data_file(fcst_file);

   if ( !fcst_mtddf )  {

      mlog << Error << "\nprocess_fcst_obs_files() -> "
           << "can't open data file \"" << fcst_file << "\"\n\n";

      exit ( 1 );

   }

   if ( ! (fcst_mtddf->data_plane(*fcst_info, fcst_wd.data)) )  {

      mlog << Error << "\nprocess_fcst_obs_files() -> "
           << "can't get data from file \"" << fcst_file << "\"\n\n";

      exit ( 1 );

   }

   //
   // Store the forecast lead and valid times
   //
   if(fcst_valid_ut == (unixtime) 0) fcst_valid_ut = fcst_wd.data.valid();
   if(is_bad_data(fcst_lead_sec))    fcst_lead_sec = fcst_wd.data.lead();

   //
   // For probability fields, rescale from [0, 100] to [0, 1]
   //
   if(fcst_info->p_flag()) rescale_probability(fcst_wd.data);
   
   //
   // Read the gridded data from the input observation file
   //

   obs_mtddf = factory.new_met_2d_data_file(obs_file);

   if ( !obs_mtddf )  {

      mlog << Error << "\nprocess_fcst_obs_files() -> "
           << "can't open data file \"" << obs_file << "\"\n\n";

      exit ( 1 );

   }

   if ( ! (obs_mtddf->data_plane(*obs_info, obs_wd.data)) )  {

      mlog << Error << "\nprocess_fcst_obs_files() -> "
           << "can't get data from file \"" << obs_file << "\"\n\n";

      exit ( 1 );

   }

   //
   // Store the observation lead and valid times
   //
   if(obs_valid_ut == (unixtime) 0) obs_valid_ut = obs_wd.data.valid();
   if(is_bad_data(obs_lead_sec))    obs_lead_sec = obs_wd.data.lead();

   //
   // For probability fields, rescale from [0, 100] to [0, 1]
   //
   if(obs_info->p_flag()) rescale_probability(obs_wd.data);
   
   //
   // Check that the grids match
   //
   if(!(fcst_mtddf->grid() == obs_mtddf->grid())) {
      mlog << Error << "\nprocess_fcst_obs_files() -> "
           << "The forecast and observation grids do not match.\n\n";
      exit(1);
   }
   grid = fcst_mtddf->grid();

   //
   // Deallocate gridded data files
   //
   if ( fcst_mtddf )  { delete fcst_mtddf;  fcst_mtddf = (Met2dDataFile *) 0; }
   if ( obs_mtddf  )  { delete obs_mtddf;   obs_mtddf  = (Met2dDataFile *) 0; }

   //
   // Print a warning if the valid times do not match
   //
   if(fcst_valid_ut != obs_valid_ut) {

      unix_to_yyyymmdd_hhmmss(fcst_valid_ut, tmp_str);
      unix_to_yyyymmdd_hhmmss(obs_valid_ut, tmp2_str);

      mlog << Warning << "\nprocess_fcst_obs_files() -> "
           << "Forecast and observation valid times do not match "
           << tmp_str << " != " << tmp2_str << ".\n\n";
   }

   //
   // Print a warning if the accumulation intervals do not match
   //
   if(fcst_info->level().type() == LevelType_Accum &&
      obs_info->level().type()  == LevelType_Accum &&
      fcst_wd.data.accum()     != obs_wd.data.accum()) {

      sec_to_hhmmss(fcst_wd.data.accum(), tmp_str);
      sec_to_hhmmss(obs_wd.data.accum(), tmp2_str);

      mlog << Warning << "\nprocess_fcst_obs_files() -> "
           << "Forecast and observation accumulation times do not match "
           << tmp_str << " != " << tmp2_str << ".\n\n";
   }

   //
   // Store the forecast and observation variable, level, and units.
   //
   engine.fcst_var_str  = (const char *) fcst_info->name();
   engine.fcst_lvl_str  = (const char *) fcst_info->level_name();
   engine.fcst_unit_str = (const char *) fcst_info->units();

   engine.obs_var_str   = (const char *) obs_info->name();
   engine.obs_lvl_str   = (const char *) obs_info->level_name();
   engine.obs_unit_str  = (const char *) obs_info->units();

   mlog << Debug(1)
        << "Forecast Field: "
        << engine.fcst_var_str << " at " << engine.fcst_lvl_str
        << "\n"
        << "Observation Field: "
        << engine.obs_var_str << " at " << engine.obs_lvl_str
        << "\n";

   //
   // Mask out the missing data in one field with the other if requested
   //
   if(engine.wconf.mask_missing_flag().ival() == 1 ||
      engine.wconf.mask_missing_flag().ival() == 3)   mask_bad_data(fcst_wd.data, obs_wd.data);
   if(engine.wconf.mask_missing_flag().ival() == 2 ||
      engine.wconf.mask_missing_flag().ival() == 3)   mask_bad_data(obs_wd.data, fcst_wd.data);

   //
   // Parse the grid and/or polyline masks from the configuration
   //
   process_masks(fcst_wd, obs_wd);

   //
   // Compute the min and max data values across both raw fields for use
   // in setting up the color table
   //
   fcst_wd.data.data_range(fmin, fmax);
    obs_wd.data.data_range(omin, omax);

   data_min = min(fmin, omin);
   data_max = max(fmax, omax);

   //
   // Set up the engine with these raw fields
   //
   mlog << Debug(2)
        << "Identifying objects in the forecast and observation fields...\n";

   engine.set(fcst_wd, obs_wd);

   //
   // Compute the contingency table statistics for the fields
   //
   if(ct_stat_flag) {
      compute_ct_stats();
   }

   if(plot_flag || obj_stat_flag || obj_plot_flag) {
      mlog << Debug(2)
           << "Identified: " << engine.n_fcst << " forecast objects "
           << "and " << engine.n_obs << " observation objects.\n";

      if(engine.wconf.fcst_merge_flag().ival() == 1)
         strcpy(merge_str, "threshold merging");
      else if(engine.wconf.fcst_merge_flag().ival() == 2)
         strcpy(merge_str, "engine merging");
      else if(engine.wconf.fcst_merge_flag().ival() == 3)
         strcpy(merge_str, "threshold merging and engine merging");
      else
         strcpy(merge_str, "no merging");

      mlog << Debug(2)
           << "Performing merging (" << merge_str << ") in the forecast field.\n";

      engine.do_fcst_merging(default_config_file, merge_config_file);

      if(engine.wconf.obs_merge_flag().ival() == 1)
         strcpy(merge_str, "threshold merging");
      else if(engine.wconf.obs_merge_flag().ival() == 2)
         strcpy(merge_str, "engine merging");
      else if(engine.wconf.obs_merge_flag().ival() == 3)
         strcpy(merge_str, "threshold merging and engine merging");
      else
         strcpy(merge_str, "no merging");

      mlog << Debug(2)
           << "Performing merging (" << merge_str << ") in the observation field.\n";

      engine.do_obs_merging(default_config_file, merge_config_file);

      mlog << Debug(2)
           << "Remaining: " << engine.n_fcst << " forecast objects "
           << "and " << engine.n_obs << " observation objects.\n";

      mlog << Debug(2)
           << "Performing matching between the forecast and observation fields.\n";

      //
      // Do the matching of objects between fields
      //
      engine.do_matching();
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void process_masks(ShapeData &fcst_wd, ShapeData &obs_wd) {
   ShapeData grid_mask, poly_mask;
   ConcatString tmp_str;

   //
   // Parse the grid mask into a ShapeData object
   //
   if(engine.wconf.mask_grid_flag().ival() != 0) {
      parse_grid_mask(engine.wconf.mask_grid().sval(), grid,
                      grid_mask.data, tmp_str);
   }

   //
   // Parse the poly mask into a ShapeData object
   //
   if(engine.wconf.mask_poly_flag().ival() != 0) {
      parse_poly_mask(engine.wconf.mask_poly().sval(), grid,
                      poly_mask.data, tmp_str);
   }

   //
   // Apply the grid mask to the forecast field if requested
   //
   if(engine.wconf.mask_grid_flag().ival() == 1 ||
      engine.wconf.mask_grid_flag().ival() == 3) {
      apply_mask(fcst_wd, grid_mask);
   }

   //
   // Apply the grid mask to the observation field if requested
   //
   if(engine.wconf.mask_grid_flag().ival() == 2 ||
      engine.wconf.mask_grid_flag().ival() == 3) {
      apply_mask(obs_wd, grid_mask);
   }

   //
   // Apply the Lat/Lon polyline mask to the forecast field if requested
   //
   if(engine.wconf.mask_poly_flag().ival() == 1 ||
      engine.wconf.mask_poly_flag().ival() == 3) {
      apply_mask(fcst_wd, poly_mask);
   }

   //
   // Apply the Lat/Lon polyline mask to the observation field if requested
   //
   if(engine.wconf.mask_poly_flag().ival() == 2 ||
      engine.wconf.mask_poly_flag().ival() == 3) {
      apply_mask(obs_wd, poly_mask);
   }


   return;
}

///////////////////////////////////////////////////////////////////////

void process_output() {

   //
   // Create output stats files and plots
   //
   if(ct_stat_flag)  write_ct_stats();
   if(obj_stat_flag) write_obj_stats();
   if(obj_plot_flag) write_obj_netcdf();
   if(plot_flag)     plot_engine();

   return;
}

///////////////////////////////////////////////////////////////////////

void check_engine_config()

{

   ColorTable ct_test;
   ConcatString s;

   // Check that the model name is not empty
   if(strlen(engine.wconf.model().sval()) == 0 ||
      check_reg_exp(ws_reg_exp, engine.wconf.model().sval()) == true) {

      mlog << Error << "\ncheck_engine_config() -> "
           << "The model name (\"" << engine.wconf.model().sval()
           << "\") must be non-empty and contain no embedded "
           << "whitespace.\n\n";
      exit(1);
   }

   // Check that grid_res is > 0
   if(engine.wconf.grid_res().dval() <= 0) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "grid_res (" << engine.wconf.grid_res().dval()
           << ") must be set > 0 in the configuration file\n\n";
      exit(1);
   }

   // Check that mask_missing_flag is set between 0 and 3
   if(engine.wconf.mask_missing_flag().ival() < 0 ||
      engine.wconf.mask_missing_flag().ival() > 3) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "The mask_missing_flag (" << engine.wconf.mask_missing_flag().ival()
           << ") must be set between 0 and 3\n\n";
      exit(1);
   }

   // Check that mask_grid_flag is set between 0 and 3
   if(engine.wconf.mask_grid_flag().ival() < 0 ||
      engine.wconf.mask_grid_flag().ival() > 3) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "The mask_grid_flag (" << engine.wconf.mask_grid_flag().ival()
           << ") must be set between 0 and 3\n\n";
      exit(1);
   }

   // Check that mask_poly_flag is set between 0 and 3
   if(engine.wconf.mask_poly_flag().ival() < 0 ||
      engine.wconf.mask_poly_flag().ival() > 3) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "The mask_poly_flag (" << engine.wconf.mask_poly_flag().ival()
           << ") must be set between 0 and 3\n\n";
      exit(1);
   }

   // Check that fcst_conv_radius and obs_conv_radius are non-negative
   if(engine.wconf.fcst_conv_radius().ival() < 0 ||
      engine.wconf.obs_conv_radius().ival() < 0) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "fcst_conv_radius (" << engine.wconf.fcst_conv_radius().ival()
           << ") and obs_conv_radius (" << engine.wconf.obs_conv_radius().ival()
           << ") must be non-negative\n\n";
      exit(1);
   }

   // Check that fcst_area_thresh and obs_area_thresh are non-negative
   if(engine.fcst_area_thresh.thresh < 0 ||
      engine.obs_area_thresh.thresh  < 0) {
      mlog << Warning << "\ncheck_engine_config() -> "
           << "fcst_area_thresh (" << engine.wconf.fcst_area_thresh().ival()
           << ") and obs_area_thresh (" << engine.wconf.obs_area_thresh().ival()
           << ") should be non-negative\n\n";
   }

   // Check that fcst_merge_flag is set between 0 and 3
   if(engine.wconf.fcst_merge_flag().ival() < 0 ||
      engine.wconf.fcst_merge_flag().ival() > 3) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "The fcst_merge_flag (" << engine.wconf.fcst_merge_flag().ival()
           << ") must be set between 0 and 3\n\n";
      exit(1);
   }

   // Check that obs_merge_flag is set between 0 and 3
   if(engine.wconf.obs_merge_flag().ival() < 0 ||
      engine.wconf.obs_merge_flag().ival() > 3) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "The obs_merge_flag (" << engine.wconf.obs_merge_flag().ival()
           << ") must be set between 0 and 3\n\n";
      exit(1);
   }

   // Check that match_flag is set between 0 and 3
   if(engine.wconf.match_flag().ival() < 0 ||
      engine.wconf.match_flag().ival() > 3) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "The match_flag (" << engine.wconf.match_flag().ival()
           << ") must be set between 0 and 3\n\n";
      exit(1);
   }

   // Check that match_flag is set between 0 and 3
   if(engine.wconf.match_flag().ival() == 0 &&
      (engine.wconf.fcst_merge_flag().ival() != 0 ||
       engine.wconf.obs_merge_flag().ival() != 0) ) {
      mlog << Warning << "\ncheck_engine_config() -> "
           << "when matching is disabled (match_flag = " << engine.wconf.match_flag().ival()
           << ") but merging is requested (fcst_merge_flag = " << engine.wconf.fcst_merge_flag().ival()
           << ", obs_merge_flag = " << engine.wconf.obs_merge_flag().ival()
           << ") any merging information will be discarded.\n\n";
   }

   // Check that max_centroid_dist is > 0
   if(engine.wconf.max_centroid_dist().dval() <= 0) {
      mlog << Warning << "\ncheck_engine_config() -> "
           << "max_centroid_dist (" << engine.wconf.max_centroid_dist().dval()
           << ") should be set > 0\n\n";
   }

   // Check that the fuzzy engine weights are non-negative
   if(engine.wconf.centroid_dist_weight().dval() < 0 ||
      engine.wconf.boundary_dist_weight().dval() < 0 ||
      engine.wconf.convex_hull_dist_weight().dval() < 0 ||
      engine.wconf.angle_diff_weight().dval() < 0 ||
      engine.wconf.area_ratio_weight().dval() < 0 ||
      engine.wconf.int_area_ratio_weight().dval() < 0 ||
      engine.wconf.complexity_ratio_weight().dval() < 0 ||
      engine.wconf.intensity_ratio_weight().dval() < 0) {

      mlog << Error << "\ncheck_engine_config() -> "
           << "All of the fuzzy engine weights must be >= 0\n\n";
      exit(1);
   }

   // Check that the sum of the weights is non-zero if matching is
   // requested.
   if(engine.wconf.match_flag().ival() != 0 &&
      is_eq((engine.wconf.centroid_dist_weight().dval()
       + engine.wconf.boundary_dist_weight().dval()
       + engine.wconf.convex_hull_dist_weight().dval()
       + engine.wconf.angle_diff_weight().dval()
       + engine.wconf.area_ratio_weight().dval()
       + engine.wconf.int_area_ratio_weight().dval()
       + engine.wconf.complexity_ratio_weight().dval()
       + engine.wconf.intensity_ratio_weight().dval()), 0.0)) {

      mlog << Error << "\ncheck_engine_config() -> "
           << "When matching is requested, the sum of the fuzzy engine "
           << "weights cannot be 0\n\n";
      exit(1);
   }

   // Check that intensity_percentile >= 0 and <= 100
   if(engine.wconf.intensity_percentile().ival() < 0 ||
      engine.wconf.intensity_percentile().ival() > 100) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "intensity_percentile (" << engine.wconf.intensity_percentile().ival()
           << ") must be >= 0 and <= 100\n\n";
      exit(1);
   }

   // Check that total_interest_thresh >= 0 and <= 1
   if(engine.wconf.total_interest_thresh().dval() < 0 ||
      engine.wconf.total_interest_thresh().dval() > 1) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "total_interest_thresh (" << engine.wconf.total_interest_thresh().dval()
           << ") must be >= 0 and <= 1\n\n";
      exit(1);
   }

   // Check that print_interest_thresh >= 0 and <= 1
   if(engine.wconf.print_interest_thresh().dval() < 0 ||
      engine.wconf.print_interest_thresh().dval() > 1) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "print_interest_thresh (" << engine.wconf.print_interest_thresh().dval()
           << ") must be >= 0 and <= 1\n\n";
      exit(1);
   }

   // Check that zero_border_size >= 1
   if(engine.wconf.zero_border_size().ival() < 1) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "zero_border_size (" << engine.wconf.zero_border_size().ival()
           << ") must be >= 1\n\n";
      exit(1);
   }

   // Get the MET Data directory from which to read the plotting data files
   met_data_dir = replace_path(engine.wconf.met_data_dir().sval());

   // Check that fcst_raw_color_table can be read
   ct_test.clear();
   s = replace_path(engine.wconf.fcst_raw_color_table().sval());
   if(!ct_test.read(s)) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "cannot read fcst_raw_color_table file ("
           << s
           << ")\n\n";
      exit(1);
   }

   // Check that obs_raw_color_table can be read
   ct_test.clear();
   s = replace_path(engine.wconf.obs_raw_color_table().sval());
   if(!ct_test.read(s)) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "cannot read obs_raw_color_table file ("
           << s
           << ")\n\n";
      exit(1);
   }

   // Check that the stride length is set >= 1
   if((stride = engine.wconf.stride_length().ival()) < 1) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "stride_length must be set >= 1: " << stride
           << "\n\n";
      exit(1);
   }

   // Check that mode_color_table can be read
   ct_test.clear();
   s = replace_path(engine.wconf.mode_color_table().sval());
   if(!ct_test.read(s)) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "cannot read mode_color_table file ("
           << s
           << ")\n\n";
      exit(1);
   }

   // Check that plot_valid_flag is set between 0 and 1
   if(engine.wconf.plot_valid_flag().ival() < 0 ||
      engine.wconf.plot_valid_flag().ival() > 1) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "The plot_valid_flag (" << engine.wconf.plot_valid_flag().ival()
           << ") must be set to 0 or 1\n\n";
      exit(1);
   }

   // Check that plot_gcarc_flag is set between 0 and 1
   if(engine.wconf.plot_gcarc_flag().ival() < 0 ||
      engine.wconf.plot_gcarc_flag().ival() > 1) {
      mlog << Error << "\ncheck_engine_config() -> "
           << "The plot_gcarc_flag (" << engine.wconf.plot_gcarc_flag().ival()
           << ") must be set to 0 or 1\n\n";
      exit(1);
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void compute_ct_stats() {
   int i, x, y;
   ShapeData fcst_mask, obs_mask;

   mlog << Debug(2)
        << "Computing contingency table statistics...\n";

   for(i=0; i<n_cts; i++) {

      cts[i].set_name(cts_str[i]);

      // Raw fields
      if(i == 0) {
         fcst_mask = *engine.fcst_raw;
         obs_mask  = *engine.obs_raw;

         // Apply the thresholds specified in the config file
         fcst_mask.threshold(engine.fcst_conv_thresh);
         obs_mask.threshold(engine.obs_conv_thresh);
      }
      // Filtered fields
      else if(i == 1) {
         fcst_mask = *engine.fcst_filter;
         obs_mask  = *engine.obs_filter;

         // Apply the thresholds specified in the config file
         fcst_mask.threshold(engine.fcst_conv_thresh);
         obs_mask.threshold(engine.obs_conv_thresh);
      }
      // Object fields
      else if(i == 2) {
         fcst_mask = *engine.fcst_mask;
         obs_mask  = *engine.obs_mask;
      }

      // Compute contingency table counts
      for(x=0; x<fcst_mask.data.nx(); x++) {
         for(y=0; y<fcst_mask.data.ny(); y++) {

            // Key off of the bad data values in the raw field
            if(engine.fcst_raw->is_bad_data(x, y) ||
               engine.obs_raw->is_bad_data(x, y)) continue;

            else if( fcst_mask.s_is_on(x, y) &&  obs_mask.s_is_on(x, y)) cts[i].inc_fy_oy();
            else if( fcst_mask.s_is_on(x, y) && !obs_mask.s_is_on(x, y)) cts[i].inc_fy_on();
            else if(!fcst_mask.s_is_on(x, y) &&  obs_mask.s_is_on(x, y)) cts[i].inc_fn_oy();
            else if(!fcst_mask.s_is_on(x, y) && !obs_mask.s_is_on(x, y)) cts[i].inc_fn_on();
         }
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void set_plot_dims(int nx, int ny) {
   double grid_ar, sm_plot_ar, lg_plot_ar;

   grid_ar    = ((double) nx)/ny;
   sm_plot_ar = (sm_pane_bb.width()/2.0)/(sm_pane_bb.height()/3.0);
   lg_plot_ar = (lg_pane_bb.width()/1.0)/(lg_pane_bb.height()/2.0);

   if(grid_ar > sm_plot_ar) sm_plot_height = sm_pane_bb.height()/3.0*sm_plot_ar/grid_ar;
   else                     sm_plot_height = sm_pane_bb.height()/3.0;

   if(grid_ar > lg_plot_ar) lg_plot_height = lg_pane_bb.height()/2.0*lg_plot_ar/grid_ar;
   else                     lg_plot_height = lg_pane_bb.height()/2.0;

   h_tab_1 = sm_pane_bb.left()  + sm_pane_bb.width()/4.0;
   h_tab_2 = sm_pane_bb.left()  + sm_pane_bb.width()/4.0 * 3.0;
   h_tab_3 = sm_pane_bb.right() + plot_text_sep;

   v_tab_1 = sm_pane_bb.top() - sm_plot_height;
   v_tab_2 = v_tab_1 - sm_plot_height;
   v_tab_3 = v_tab_2 - sm_plot_height;

   return;
}

///////////////////////////////////////////////////////////////////////

void plot_engine() {
   PSfile plot;
   double dmin, dmax;
   ConcatString s;
   ConcatString ps_file;

   //
   // Setup the x/y bounding box for the data to be plotted.
   // If requested in the config file, only plot the regions of valid data.
   // Otherwise, plot the entire grid.
   //
   set_xy_bb();

   //
   // Setup the plotting dimensions based on the x/y bounding box
   //
   set_plot_dims(nint(xy_bb.width()), nint(xy_bb.height()));

   //
   // Create output PostScript file name
   //
   build_outfile_name(".ps", ps_file);

   //
   // Load the raw forecast color table
   //
   s = replace_path(engine.wconf.fcst_raw_color_table().sval());
   mlog << Debug(1) << "Loading forecast raw color table: " << s << "\n";
   fcst_ct.read(s);

   //
   // Load the raw observation color table
   //
   s = replace_path(engine.wconf.obs_raw_color_table().sval());
   mlog << Debug(1) << "Loading observation raw color table: " << s << "\n";
   obs_ct.read(s);

   //
   // If the forecast and observation fields are the same and if the range
   // of both colortables is [0, 1], rescale both colortables to the
   // data_min and data_max values
   //
   if(fcst_info->name() == obs_info->name() &&
      is_eq(fcst_ct.data_min(bad_data_double), 0.0) &&
      is_eq(fcst_ct.data_max(bad_data_double), 1.0) &&
      is_eq(obs_ct.data_min(bad_data_double),  0.0) &&
      is_eq(obs_ct.data_max(bad_data_double),  1.0)) {

      fcst_ct.rescale(data_min, data_max, bad_data_double);
      obs_ct.rescale(data_min, data_max, bad_data_double);
   }
   //
   // Otherwise, if the range of either colortable is [0, 1], rescale
   // the field using the min/max values in the field
   //
   else {
      if(is_eq(fcst_ct.data_min(bad_data_double), 0.0) &&
         is_eq(fcst_ct.data_max(bad_data_double), 1.0)) {

         engine.fcst_raw->data.data_range(dmin, dmax);

         fcst_ct.rescale(dmin, dmax, bad_data_double);
      }
      if(is_eq(obs_ct.data_min(bad_data_double), 0.0) &&
         is_eq(obs_ct.data_max(bad_data_double), 1.0)) {

         engine.obs_raw->data.data_range(dmin, dmax);

         obs_ct.rescale(dmin, dmax, bad_data_double);
      }
   }

   //
   // If the fcst_raw_plot_min or fcst_raw_plot_max value is set in the
   // config file, rescale the forecast colortable to the requested range
   //
   if(!is_eq(engine.wconf.fcst_raw_plot_min().dval(), 0.0) ||
      !is_eq(engine.wconf.fcst_raw_plot_max().dval(), 0.0)) {
      fcst_ct.rescale(engine.wconf.fcst_raw_plot_min().dval(),
                      engine.wconf.fcst_raw_plot_max().dval(),
                      bad_data_double);
   }

   //
   // If the obs_raw_plot_min or obs_raw_plot_max value is set in the
   // config file, rescale the observation colortable to the requested range
   //
   if(!is_eq(engine.wconf.obs_raw_plot_min().dval(), 0.0) ||
      !is_eq(engine.wconf.obs_raw_plot_max().dval(), 0.0)) {
      obs_ct.rescale(engine.wconf.obs_raw_plot_min().dval(),
                     engine.wconf.obs_raw_plot_max().dval(),
                     bad_data_double);
   }

   //
   // Set the fill colors.  If a fill value is not specified in the range
   // of the color table, use the default color.  Otherwise, use the
   // color specified in the color table.
   //
   if(bad_data_double >= fcst_ct.data_min() &&
      bad_data_double <= fcst_ct.data_max()) c_fcst_fill = fcst_ct.nearest(bad_data_double);
   if(bad_data_double >= obs_ct.data_min() &&
      bad_data_double <= obs_ct.data_max())  c_obs_fill = obs_ct.nearest(bad_data_double);

   mlog << Debug(1) << "Creating postscript file: " << ps_file << "\n";

   plot.open(ps_file);
   n_page = 1;

   s << cs_erase
     << "MODE: " << engine.fcst_var_str << " at " << engine.fcst_lvl_str
     << " vs "   << engine.obs_var_str  << " at " << engine.obs_lvl_str;
   plot_engine(plot, engine, FOEng, s);

   if(engine.wconf.fcst_merge_flag().ival() == 1 ||
      engine.wconf.fcst_merge_flag().ival() == 3 ) {
      plot_threshold_merging(plot, engine, "Forecast: Threshold Merging", 1);
   }

   if(engine.wconf.fcst_merge_flag().ival() == 2 ||
      engine.wconf.fcst_merge_flag().ival() == 3) {
      plot_engine(plot, *engine.fcst_engine, FFEng, "Forecast: Engine Merging");
   }

   if(engine.wconf.obs_merge_flag().ival() == 1 ||
      engine.wconf.obs_merge_flag().ival() == 3) {
      plot_threshold_merging(plot, engine, "Observation: Threshold Merging", 0);
   }

   if(engine.wconf.obs_merge_flag().ival() == 2 ||
      engine.wconf.obs_merge_flag().ival() == 3) {
      plot_engine(plot, *engine.obs_engine, OOEng, "Observation: Engine Merging");
   }

   return;
}

///////////////////////////////////////////////////////////////////////
//
// Flag value of 2 indicates that both fcst and obs should be enlarged
// Flag value of 1 indicates that only the fcst should be enlarged
// Flag value of 0 indicates that only the obs should be enlarged
//
///////////////////////////////////////////////////////////////////////

void plot_engine(PSfile &p, Engine &eng, EngineType eng_type, const char *title)

{

   Box dim;
   ConcatString label;
   char junk[1024];
   char fcst_str[max_str_len], fcst_short_str[max_str_len];
   char obs_str[max_str_len], obs_short_str[max_str_len];
   char thresh_str[max_str_len];
   ConcatString tmp_str, tmp2_str, tmp3_str;
   double v_tab, h_tab, h_tab_a, h_tab_b, h_tab_c, v;
   bool draw_line;
   int i, mon, day, yr, hr, minute, sec;

   ////////////////////////////////////////////////////////////////////
   //
   // First Page: create a 6 plot
   //
   ////////////////////////////////////////////////////////////////////

   p.pagenumber(n_page);

   p.choose_font(31, 24.0, met_data_dir);
   p.write_centered_text(1, 1, 306.0, 752.0, 0.5, 0.5, title);

   // Plot forecast versus observation
   if(eng_type == FOEng) {
      strcpy(fcst_str, "Forecast");
      strcpy(fcst_short_str, "Fcst");
      strcpy(obs_str, "Observation");
      strcpy(obs_short_str, "Obs");
   }
   // Plot forecast versus forecast
   else if(eng_type == FFEng) {
      strcpy(fcst_str, "Forecast");
      strcpy(fcst_short_str, "Fcst");
      strcpy(obs_str, "Forecast");
      strcpy(obs_short_str, "Fcst");
   }
   // Plot observation versus observation
   else if(eng_type == OOEng) {
      strcpy(fcst_str, "Observation");
      strcpy(fcst_short_str, "Obs");
      strcpy(obs_str, "Observation");
      strcpy(obs_short_str, "Obs");
   }

   p.choose_font(31, 18.0, met_data_dir);
   p.write_centered_text(1, 1, h_tab_1, 727.0, 0.5, 0.5, fcst_str);
   p.write_centered_text(1, 1, h_tab_2, 727.0, 0.5, 0.5, obs_str);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw raw forecast field
   //
   ////////////////////////////////////////////////////////////////////

   set_dim(dim, v_tab_1, v_tab_1 + sm_plot_height, h_tab_1);
   render_image(p, eng, eng_type, *eng.fcst_raw, dim, 1, 0);
   draw_border(p, dim);
   draw_map(p, dim);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw raw observation field
   //
   ////////////////////////////////////////////////////////////////////

   set_dim(dim, v_tab_1, v_tab_1 + sm_plot_height, h_tab_2);
   render_image(p, eng, eng_type, *eng.obs_raw, dim, 0, 0);
   draw_border(p, dim);
   draw_map(p, dim);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw fcst split field
   //
   ////////////////////////////////////////////////////////////////////

   set_dim(dim, v_tab_2, v_tab_2 + sm_plot_height, h_tab_1);
   render_image(p, eng, eng_type, *eng.fcst_split, dim, 1, 1);
   draw_border(p, dim);
   draw_map(p, dim);
   draw_convex_hulls(p, eng, dim, 1, 0);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw obs split field
   //
   ////////////////////////////////////////////////////////////////////

   set_dim(dim, v_tab_2, v_tab_2 + sm_plot_height, h_tab_2);
   render_image(p, eng, eng_type, *eng.obs_split, dim, 0, 1);
   draw_border(p, dim);
   draw_map(p, dim);
   draw_convex_hulls(p, eng, dim, 0, 0);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw fcst simple ids
   //
   ////////////////////////////////////////////////////////////////////

   set_dim(dim, v_tab_3, v_tab_3 + sm_plot_height, h_tab_1);
   draw_border(p, dim);
   draw_map(p, dim);
   plot_simple_ids(p, eng, dim, 1);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw obs simple ids
   //
   ////////////////////////////////////////////////////////////////////

   set_dim(dim, v_tab_3, v_tab_3 + sm_plot_height, h_tab_2);
   draw_border(p, dim);
   draw_map(p, dim);
   plot_simple_ids(p, eng, dim, 0);

   ////////////////////////////////////////////////////////////////////
   //
   // Plot interest values
   //
   ////////////////////////////////////////////////////////////////////

   p.choose_font(31, 11.0, met_data_dir);

   p.write_centered_text(1, 1, h_tab_3 + 1.5*plot_text_sep, 727.0, 0.5, 0.5, fcst_short_str);
   p.write_centered_text(1, 1, h_tab_3 + 4.5*plot_text_sep, 727.0, 0.5, 0.5, obs_short_str);
   p.write_centered_text(1, 1, h_tab_3 + 7.5*plot_text_sep, 727.0, 0.5, 0.5, "Interest");

   v_tab = 727.0 - 1.5*plot_text_sep;
   draw_line = false;

   for(i=0; i<(eng.n_fcst*eng.n_obs) && v_tab >= v_margin; i++) {

      if(eng.info[i].interest_value < eng.wconf.total_interest_thresh().dval()
      && draw_line == false) {

         p.write_centered_text(1, 1, h_tab_3 + 4.5*plot_text_sep,
                               v_tab, 0.5, 0.5, "----------------------------------");
         draw_line = true;
         v_tab -= plot_text_sep;
      }

      label << cs_erase << eng.info[i].fcst_number;
      p.write_centered_text(1, 1, h_tab_3 + 1.5*plot_text_sep, v_tab, 0.5, 0.5, label);

      label << cs_erase << eng.info[i].obs_number;
      p.write_centered_text(1, 1, h_tab_3 + 4.5*plot_text_sep, v_tab, 0.5, 0.5, label);

      if ( eng.info[i].interest_value < 0 ) label = na_str;
      else {
         label.set_precision(4);
         label << cs_erase << eng.info[i].interest_value;
      }
      p.write_centered_text(1, 1, h_tab_3 + 7.5*plot_text_sep,
                            v_tab, 0.5, 0.5, label);

      v_tab -= plot_text_sep;
   }

   ////////////////////////////////////////////////////////////////////
   //
   // Model Name, Initialization, Valid, Lead, and Accumulation Times
   //
   ////////////////////////////////////////////////////////////////////

   v_tab   = v_tab_3 - 1.0*plot_text_sep;
   h_tab_a = h_tab_1 - 0.5*dim.width();
   h_tab_b = h_tab_a + 4.0*plot_text_sep;
   h_tab_c = h_tab_a + 9.0*plot_text_sep;

   //
   // Field name
   //
   p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, fcst_str);
   p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, obs_str);
   v_tab -= plot_text_sep;

   //
   // Model Name
   //
   p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Model:");
   p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5,
                         eng.wconf.model().sval());
   v_tab -= plot_text_sep;

   //
   // Variable Name
   //
   p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Field:");
   p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5,
                         eng.fcst_var_str);
   p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5,
                         eng.obs_var_str);
   v_tab -= plot_text_sep;

   //
   // Level Name
   //
   p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Level:");
   p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5,
                         eng.fcst_lvl_str);
   p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5,
                         eng.obs_lvl_str);
   v_tab -= plot_text_sep;

   //
   // Units
   //
   p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Units:");
   p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5,
                         eng.fcst_unit_str);
   p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5,
                         eng.obs_unit_str);
   v_tab -= plot_text_sep;

   //
   // Initialization Time
   //
   p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Initial:");
   unix_to_mdyhms(eng.fcst_raw->data.valid() -
                  eng.fcst_raw->data.lead(),
                  mon, day, yr, hr, minute, sec);
   snprintf(junk, sizeof(junk), "%.4i%.2i%.2i", yr, mon, day);
   p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
   snprintf(junk, sizeof(junk), "%.2i:%.2i:%.2i", hr, minute, sec);
   p.write_centered_text(1, 1, h_tab_b, v_tab-plot_text_sep,
                         0.0, 0.5, junk);
   unix_to_mdyhms(eng.obs_raw->data.valid() -
                  eng.obs_raw->data.lead(),
                  mon, day, yr, hr, minute, sec);
   snprintf(junk, sizeof(junk), "%.4i%.2i%.2i", yr, mon, day);
   p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
   snprintf(junk, sizeof(junk), "%.2i:%.2i:%.2i", hr, minute, sec);
   p.write_centered_text(1, 1, h_tab_c, v_tab-plot_text_sep,
                         0.0, 0.5, junk);
   v_tab -= 2.0*plot_text_sep;

   //
   // Valid time
   //
   p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Valid:");
   unix_to_mdyhms(eng.fcst_raw->data.valid(),
                  mon, day, yr, hr, minute, sec);
   snprintf(junk, sizeof(junk), "%.4i%.2i%.2i", yr, mon, day);
   p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
   snprintf(junk, sizeof(junk), "%.2i:%.2i:%.2i", hr, minute, sec);
   p.write_centered_text(1, 1, h_tab_b, v_tab-plot_text_sep,
                         0.0, 0.5, junk);
   unix_to_mdyhms(eng.obs_raw->data.valid(),
                  mon, day, yr, hr, minute, sec);
   snprintf(junk, sizeof(junk), "%.4i%.2i%.2i", yr, mon, day);
   p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
   snprintf(junk, sizeof(junk), "%.2i:%.2i:%.2i", hr, minute, sec);
   p.write_centered_text(1, 1, h_tab_c, v_tab-plot_text_sep,
                         0.0, 0.5, junk);
   v_tab -= 2.0*plot_text_sep;

   //
   // Accumulation time
   //
   p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Accum:");
   sec_to_hms(eng.fcst_raw->data.accum(), hr, minute, sec);
   snprintf(junk, sizeof(junk), "%.2i:%.2i:%.2i", hr, minute, sec);
   p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
   sec_to_hms(eng.obs_raw->data.accum(), hr, minute, sec);
   snprintf(junk, sizeof(junk), "%.2i:%.2i:%.2i", hr, minute, sec);
   p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
   v_tab -= plot_text_sep;

   ////////////////////////////////////////////////////////////////////
   //
   // Engine Weights
   //
   ////////////////////////////////////////////////////////////////////

   v_tab -= plot_text_sep;

   h_tab_a = h_tab_1 - 0.5*dim.width();
   h_tab_b = h_tab_a + 8.0*plot_text_sep;
   h_tab_c = h_tab_a + 11.0*plot_text_sep;

   //
   // Centroid and Boundary Distance Weights
   //
   p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                         "Centroid/Boundary:");
   snprintf(junk, sizeof(junk), "%.2f", eng.wconf.centroid_dist_weight().dval());
   p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
   snprintf(junk, sizeof(junk), "%.2f", eng.wconf.boundary_dist_weight().dval());
   p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
   v_tab -= plot_text_sep;

   //
   // Convex Hull Distance and Angle Difference Weights
   //
   p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                         "Convex Hull/Angle:");
   snprintf(junk, sizeof(junk), "%.2f", eng.wconf.convex_hull_dist_weight().dval());
   p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
   snprintf(junk, sizeof(junk), "%.2f", eng.wconf.angle_diff_weight().dval());
   p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
   v_tab -= plot_text_sep;

   //
   // Area Ratio and Intesection Over Minimum Area Weights
   //
   p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                         "Area/Intersection Area:");
   snprintf(junk, sizeof(junk), "%.2f", eng.wconf.area_ratio_weight().dval());
   p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
   snprintf(junk, sizeof(junk), "%.2f", eng.wconf.int_area_ratio_weight().dval());
   p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
   v_tab -= plot_text_sep;

   //
   // Complexity Ratio and Intensity Ratio Weights
   //
   p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                         "Complexity/Intensity:");
   snprintf(junk, sizeof(junk), "%.2f", eng.wconf.complexity_ratio_weight().dval());
   p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
   snprintf(junk, sizeof(junk), "%.2f", eng.wconf.intensity_ratio_weight().dval());
   p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
   v_tab -= plot_text_sep;

   //
   // Total Interest Threshold
   //
   p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                         "Total Interest Thresh:");
   snprintf(junk, sizeof(junk), "%.2f", eng.wconf.total_interest_thresh().dval());
   p.write_centered_text(1, 1, (h_tab_b+h_tab_c)/2.0, v_tab, 0.0, 0.5,
                         junk);
   v_tab -= plot_text_sep;

   ////////////////////////////////////////////////////////////////////
   //
   // Engine configuration
   //
   ////////////////////////////////////////////////////////////////////

   v_tab   = v_tab_3 - 1.0*plot_text_sep;
   h_tab_a = h_tab_2 - 0.5*dim.width();
   h_tab_b = h_tab_a + 5.0*plot_text_sep;
   h_tab_c = h_tab_a + 10.0*plot_text_sep;

   if(eng_type == FOEng) {

      //
      // Field name
      //
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, fcst_str);
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, obs_str);
      v_tab -= plot_text_sep;

      //
      // Mask missing, grid, and polyline Flags
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Mask M/G/P:");
      if(eng.wconf.mask_missing_flag().ival() == 1 ||
         eng.wconf.mask_missing_flag().ival() == 3) tmp_str = "on";
      else                                          tmp_str = "off";
      if(eng.wconf.mask_grid_flag().ival() == 1 ||
         eng.wconf.mask_grid_flag().ival() == 3)    tmp2_str = "on";
      else                                          tmp2_str = "off";
      if(eng.wconf.mask_grid_flag().ival() == 1 ||
         eng.wconf.mask_grid_flag().ival() == 3)    tmp3_str = "on";
      else                                          tmp3_str = "off";
      label << cs_erase << tmp_str << '/' << tmp2_str << '/' << tmp3_str;
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label);

      if(eng.wconf.mask_missing_flag().ival() == 2 ||
         eng.wconf.mask_missing_flag().ival() == 3) tmp_str = "on";
      else                                          tmp_str = "off";
      if(eng.wconf.mask_grid_flag().ival() == 2 ||
         eng.wconf.mask_grid_flag().ival() == 3)    tmp2_str = "on";
      else                                          tmp2_str = "off";
      if(eng.wconf.mask_grid_flag().ival() == 2 ||
         eng.wconf.mask_grid_flag().ival() == 3)    tmp3_str = "on";
      else                                          tmp3_str = "off";
      label << cs_erase << tmp_str << '/' << tmp2_str << '/' << tmp3_str;
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, label);

      v_tab -= plot_text_sep;

      //
      // Raw threshold
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Raw Thresh:");
      eng.fcst_raw_thresh.get_str(thresh_str, 2);
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, thresh_str);
      eng.obs_raw_thresh.get_str(thresh_str, 2);
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, thresh_str);
      v_tab -= plot_text_sep;

      //
      // Convolution Radius
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Conv Radius:");
      snprintf(junk, sizeof(junk), "%.0i gs", eng.wconf.fcst_conv_radius().ival());
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
      snprintf(junk, sizeof(junk), "%.0i gs", eng.wconf.obs_conv_radius().ival());
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
      v_tab -= plot_text_sep;

      //
      // Convolution Threshold
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Conv Thresh:");
      eng.fcst_conv_thresh.get_str(thresh_str, 2);
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, thresh_str);
      eng.obs_conv_thresh.get_str(thresh_str, 2);
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, thresh_str);
      v_tab -= plot_text_sep;

      //
      // Area Threshold
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Area Thresh:");
      eng.fcst_area_thresh.get_str(thresh_str, 0);
      snprintf(junk, sizeof(junk), "%s gs", thresh_str);
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
      eng.obs_area_thresh.get_str(thresh_str, 0);
      snprintf(junk, sizeof(junk), "%s gs", thresh_str);
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
      v_tab -= plot_text_sep;

      //
      // Intensity Percentile and Threshold
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab,
                           0.0, 0.5, "Inten Thresh:");

      if(     nint(eng.wconf.fcst_inten_perc().ival()) == 101) tmp_str = "mean";
      else if(nint(eng.wconf.fcst_inten_perc().ival()) == 102) tmp_str = "sum";
      else {
         snprintf(junk, sizeof(junk), "p%.0i", eng.wconf.fcst_inten_perc().ival());
         tmp_str = junk;
      }
      eng.fcst_inten_perc_thresh.get_str(thresh_str, 2);
      label << cs_erase << tmp_str << thresh_str;
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label);

      if(     nint(eng.wconf.obs_inten_perc().ival()) == 101) tmp_str = "mean";
      else if(nint(eng.wconf.obs_inten_perc().ival()) == 102) tmp_str = "sum";
      else {
         snprintf(junk, sizeof(junk), "p%.0i", eng.wconf.obs_inten_perc().ival());
         tmp_str = junk;
      }
      eng.obs_inten_perc_thresh.get_str(thresh_str, 2);
      label << cs_erase << tmp_str << thresh_str;
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, label);
      v_tab -= plot_text_sep;

      //
      // Merge Threshold
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Merge Thresh:");
      eng.fcst_merge_thresh.get_str(thresh_str, 2);
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, thresh_str);
      eng.obs_merge_thresh.get_str(thresh_str, 2);
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, thresh_str);
      v_tab -= plot_text_sep;

      /////////////////////////////////////////////////////////////////
      //
      // Matching/Merging Criteria
      //
      /////////////////////////////////////////////////////////////////

      //
      // Merging flag
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Merging:");
      if(eng.wconf.fcst_merge_flag().ival() == 0)       label = "none";
      else if(eng.wconf.fcst_merge_flag().ival() == 1)  label = "thresh";
      else if(eng.wconf.fcst_merge_flag().ival() == 2)  label = "engine";
      else if(eng.wconf.fcst_merge_flag().ival() == 3)  label = "thresh/engine";
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label);

      if(eng.wconf.obs_merge_flag().ival() == 0)        label = "none";
      else if(eng.wconf.obs_merge_flag().ival() == 1)   label = "thresh";
      else if(eng.wconf.obs_merge_flag().ival() == 2)   label = "engine";
      else if(eng.wconf.obs_merge_flag().ival() == 3)   label = "thresh/engine";
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, label);
      v_tab -= plot_text_sep;

      //
      // Matching scheme
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Matching:");
      if(eng.wconf.match_flag().ival() == 0)      label = "none";
      else if(eng.wconf.match_flag().ival() == 1) label = "match/merge";
      else if(eng.wconf.match_flag().ival() == 2) label = "match/fcst merge";
      else if(eng.wconf.match_flag().ival() == 3) label = "match/no merge";
      p.write_centered_text(1, 1, (h_tab_b+h_tab_c)/2.0, v_tab, 0.0, 0.5, label);
      v_tab -= plot_text_sep;

      /////////////////////////////////////////////////////////////////
      //
      // Object Counts and Areas
      //
      /////////////////////////////////////////////////////////////////

      //
      // Simple objects counts (Matched Simples/Unmatched Simples)
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Simple/M/U:");
      snprintf(junk, sizeof(junk), "%i/%i/%i", eng.n_fcst,
              eng.get_matched_fcst(0), eng.get_unmatched_fcst(0));
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
      snprintf(junk, sizeof(junk), "%i/%i/%i", eng.n_obs,
              eng.get_matched_obs(0), eng.get_unmatched_obs(0));
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
      v_tab -= plot_text_sep;

      //
      // Area counts
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Area:");
      snprintf(junk, sizeof(junk), "%i gs",
              eng.get_matched_fcst(1) + eng.get_unmatched_fcst(1));
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
      snprintf(junk, sizeof(junk), "%i gs",
              eng.get_matched_obs(1) + eng.get_unmatched_obs(1));
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
      v_tab -= plot_text_sep;

      //
      // Area counts (Matched Simple/Unmatched Simples)
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Area M/U:");
      snprintf(junk, sizeof(junk), "%i/%i",
              eng.get_matched_fcst(1),  eng.get_unmatched_fcst(1));
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
      snprintf(junk, sizeof(junk), "%i/%i",
              eng.get_matched_obs(1),  eng.get_unmatched_obs(1));
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
      v_tab -= plot_text_sep;

      //
      // Cluster object counts
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Cluster:");
      snprintf(junk, sizeof(junk), "%i", eng.collection.n_sets);
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
      snprintf(junk, sizeof(junk), "%i", eng.collection.n_sets);
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
      v_tab -= plot_text_sep;

      /////////////////////////////////////////////////////////////////
      //
      // Interest Values
      //
      /////////////////////////////////////////////////////////////////

      //
      // Median of Maximum Interest Values
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "MMI:");
      v = interest_percentile(eng, 50.0, 1);
      snprintf(junk, sizeof(junk), "%.4f", v);
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
      v = interest_percentile(eng, 50.0, 2);
      snprintf(junk, sizeof(junk), "%.4f", v);
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
      v_tab -= plot_text_sep;

      //
      // Median of Maximum Interest Values
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "MMI (F+O):");
      v = interest_percentile(eng, 50.0, 3);
      snprintf(junk, sizeof(junk), "%.4f", v);
      p.write_centered_text(1, 1, (h_tab_b+h_tab_c)/2.0, v_tab, 0.0, 0.5, junk);
      v_tab -= plot_text_sep;

   }
   else {

      v_tab -= 12.0*plot_text_sep;

      //
      // Field name
      //
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, fcst_str);
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, obs_str);
      v_tab -= plot_text_sep;

      /////////////////////////////////////////////////////////////////
      //
      // Object Counts and Areas
      //
      /////////////////////////////////////////////////////////////////

      //
      // Simple objects counts
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Simple:");
      snprintf(junk, sizeof(junk), "%i", eng.n_fcst);
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
      snprintf(junk, sizeof(junk), "%i", eng.n_obs);
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
      v_tab -= plot_text_sep;

      //
      // Area counts
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Area:");
      snprintf(junk, sizeof(junk), "%i gs",
              eng.get_matched_fcst(1) + eng.get_unmatched_fcst(1));
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
      snprintf(junk, sizeof(junk), "%i gs",
              eng.get_matched_obs(1) + eng.get_unmatched_obs(1));
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
      v_tab -= plot_text_sep;

      //
      // Cluster object counts
      //
      p.write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5, "Cluster:");
      snprintf(junk, sizeof(junk), "%i", eng.collection.n_sets);
      p.write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, junk);
      snprintf(junk, sizeof(junk), "%i", eng.collection.n_sets);
      p.write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5, junk);
      v_tab -= plot_text_sep;
   }

   ////////////////////////////////////////////////////////////////////
   //
   // Finished with this page
   //
   ////////////////////////////////////////////////////////////////////

   p.showpage();
   n_page++;

   if(eng_type == FOEng || eng_type == FFEng) {

      /////////////////////////////////////////////////////////////////
      //
      // New Page: englargement of forecast fields
      //
      /////////////////////////////////////////////////////////////////

      p.pagenumber(n_page);

      p.choose_font(31, 24.0, met_data_dir);
      p.write_centered_text(1, 1, h_tab_cen, 752.0, 0.5, 0.5, title);
      p.write_centered_text(1, 1, h_tab_cen, 722.0, 0.5, 0.5, fcst_str);

      /////////////////////////////////////////////////////////////////
      //
      // Draw raw forecast field
      //
      /////////////////////////////////////////////////////////////////

      v_tab = page_height - 4.0*v_margin;
      set_dim(dim, v_tab - lg_plot_height, v_tab, h_tab_cen);
      render_image(p, eng, eng_type, *eng.fcst_raw, dim, 1, 0);
      draw_border(p, dim);
      draw_map(p, dim);
      plot_colorbar(p, dim, 1);

      /////////////////////////////////////////////////////////////////
      //
      // Draw fcst split field
      //
      /////////////////////////////////////////////////////////////////

      v_tab -= lg_plot_height;
      set_dim(dim, v_tab - lg_plot_height, v_tab, h_tab_cen);
      render_image(p, eng, eng_type, *eng.fcst_split, dim, 1, 1);
      draw_border(p, dim);
      draw_map(p, dim);
      draw_convex_hulls(p, eng, dim, 1, 0);

      /////////////////////////////////////////////////////////////////
      //
      // Finished with this page
      //
      /////////////////////////////////////////////////////////////////

      p.showpage();
      n_page++;
   }

   if(eng_type == FOEng || eng_type == OOEng) {

      /////////////////////////////////////////////////////////////////
      //
      // New Page: englargement of observation fields
      //
      /////////////////////////////////////////////////////////////////

      p.pagenumber(n_page);

      p.choose_font(31, 24.0, met_data_dir);
      p.write_centered_text(1, 1, h_tab_cen, 752.0, 0.5, 0.5, title);
      p.write_centered_text(1, 1, h_tab_cen, 722.0, 0.5, 0.5, obs_str);


      /////////////////////////////////////////////////////////////////
      //
      // Draw raw observation field
      //
      /////////////////////////////////////////////////////////////////

      v_tab = page_height - 4.0*v_margin;
      set_dim(dim, v_tab - lg_plot_height, v_tab, h_tab_cen);
      render_image(p, eng, eng_type, *eng.obs_raw, dim, 0, 0);
      draw_border(p, dim);
      draw_map(p, dim);
      plot_colorbar(p, dim, 0);

      /////////////////////////////////////////////////////////////////
      //
      // Draw observation split field
      //
      /////////////////////////////////////////////////////////////////

      v_tab -= lg_plot_height;
      set_dim(dim, v_tab - lg_plot_height, v_tab, h_tab_cen);
      render_image(p, eng, eng_type, *eng.obs_split, dim, 0, 1);
      draw_border(p, dim);
      draw_map(p, dim);
      draw_convex_hulls(p, eng, dim, 0, 0);

      /////////////////////////////////////////////////////////////////
      //
      // Finished with this page
      //
      /////////////////////////////////////////////////////////////////

      p.showpage();
      n_page++;
   }

   if(eng_type == FOEng) {

      /////////////////////////////////////////////////////////////////
      //
      // New Page: overlap of object fields
      //
      /////////////////////////////////////////////////////////////////

      p.pagenumber(n_page);

      /////////////////////////////////////////////////////////////////
      //
      // Draw the forecast object field and observation boundaries
      //
      /////////////////////////////////////////////////////////////////

      v_tab = page_height - 2.0*v_margin;
      set_dim(dim, v_tab - lg_plot_height, v_tab, h_tab_cen);

      tmp_str << cs_erase << fcst_str << " Objects with " << obs_str << " Outlines";
      p.choose_font(31, 24.0, met_data_dir);
      p.write_centered_text(1, 1, h_tab_cen, dim.top() + plot_text_sep/2.0,
                            0.5, 0.5, tmp_str);

      render_image(p, eng, eng_type, *eng.fcst_split, dim, 1, 1);
      draw_border(p, dim);
      draw_map(p, dim);
      draw_boundaries(p, eng, dim, 0);

      /////////////////////////////////////////////////////////////////
      //
      // Draw the observation object field and forecast boundaries
      //
      /////////////////////////////////////////////////////////////////

      v_tab = v_tab - lg_plot_height - 2.0*v_margin;
      set_dim(dim, v_tab - lg_plot_height, v_tab, h_tab_cen);

      tmp_str << cs_erase << obs_str << " Objects with " << fcst_str << " Outlines";
      p.choose_font(31, 24.0, met_data_dir);
      p.write_centered_text(1, 1, h_tab_cen, dim.top() + plot_text_sep/2.0,
                            0.5, 0.5, tmp_str);

      render_image(p, eng, eng_type, *eng.obs_split, dim, 0, 1);
      draw_border(p, dim);
      draw_map(p, dim);
      draw_boundaries(p, eng, dim, 1);

      /////////////////////////////////////////////////////////////////
      //
      // Finished with this page
      //
      /////////////////////////////////////////////////////////////////

      p.showpage();
      n_page++;

      /////////////////////////////////////////////////////////////////
      //
      // New Page: cluster object pairs
      //
      /////////////////////////////////////////////////////////////////

      p.pagenumber(n_page);

      h_tab_a = h_tab_cen - sm_pane_bb.width()/4.0;
      h_tab_b = h_tab_cen + sm_pane_bb.width()/4.0;

      v_tab = page_height - 2.0*v_margin;
      set_dim(dim, v_tab - lg_plot_height, v_tab, h_tab_cen);

      p.choose_font(31, 24.0, met_data_dir);
      p.write_centered_text(1, 1, h_tab_cen, dim.top() + plot_text_sep/2.0,
                            0.5, 0.5, "Cluster Object Information");

      p.choose_font(31, 18.0, met_data_dir);
      p.write_centered_text(1, 1, h_tab_a, 727.0, 0.5, 0.5, fcst_str);
      p.write_centered_text(1, 1, h_tab_b, 727.0, 0.5, 0.5, obs_str);

      /////////////////////////////////////////////////////////////////
      //
      // Draw fcst split field
      //
      /////////////////////////////////////////////////////////////////

      set_dim(dim, v_tab_1, v_tab_1 + sm_plot_height, h_tab_a);
      render_image(p, eng, eng_type, *eng.fcst_split, dim, 1, 1);
      draw_border(p, dim);
      draw_map(p, dim);
      draw_convex_hulls(p, eng, dim, 1, 0);

      /////////////////////////////////////////////////////////////////
      //
      // Draw obs split field
      //
      /////////////////////////////////////////////////////////////////

      set_dim(dim, v_tab_1, v_tab_1 + sm_plot_height, h_tab_b);
      render_image(p, eng, eng_type, *eng.obs_split, dim, 0, 1);
      draw_border(p, dim);
      draw_map(p, dim);
      draw_convex_hulls(p, eng, dim, 0, 0);

      /////////////////////////////////////////////////////////////////
      //
      // Draw fcst cluster ids
      //
      /////////////////////////////////////////////////////////////////

      set_dim(dim, v_tab_2, v_tab_2 + sm_plot_height, h_tab_a);
      draw_border(p, dim);
      draw_map(p, dim);
      draw_convex_hulls(p, eng, dim, 1, 1);

      /////////////////////////////////////////////////////////////////
      //
      // Draw obs cluster ids
      //
      /////////////////////////////////////////////////////////////////

      set_dim(dim, v_tab_2, v_tab_2 + sm_plot_height, h_tab_b);
      draw_border(p, dim);
      draw_map(p, dim);
      draw_convex_hulls(p, eng, dim, 0, 1);

      /////////////////////////////////////////////////////////////////
      //
      // Plot cluster pair attributes
      //
      /////////////////////////////////////////////////////////////////

      p.choose_font(31, 11.0, met_data_dir);

      v_tab = v_tab_2 - 1.0*plot_text_sep;
      h_tab = h_margin;

      //
      // Plot the column headers
      //
      p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, "CLUS");
      p.write_centered_text(1, 1, h_tab, v_tab-10.0, 0.0, 0.5, "PAIR");
      h_tab += 3.0*plot_text_sep;
      p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, "CEN");
      p.write_centered_text(1, 1, h_tab, v_tab-10.0, 0.0, 0.5, "DIST");
      h_tab += 3.0*plot_text_sep;
      p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, "ANG");
      p.write_centered_text(1, 1, h_tab, v_tab-10.0, 0.0, 0.5, "DIFF");
      h_tab += 3.0*plot_text_sep;
      p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, "FCST");
      p.write_centered_text(1, 1, h_tab, v_tab-10.0, 0.0, 0.5, "AREA");
      h_tab += 3.0*plot_text_sep;
      p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, "OBS");
      p.write_centered_text(1, 1, h_tab, v_tab-10.0, 0.0, 0.5, "AREA");
      h_tab += 3.0*plot_text_sep;
      p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, "INTER");
      p.write_centered_text(1, 1, h_tab, v_tab-10.0, 0.0, 0.5, "AREA");
      h_tab += 3.0*plot_text_sep;
      p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, "UNION");
      p.write_centered_text(1, 1, h_tab, v_tab-10.0, 0.0, 0.5, "AREA");
      h_tab += 3.0*plot_text_sep;
      p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, "SYM");
      p.write_centered_text(1, 1, h_tab, v_tab-10.0, 0.0, 0.5, "DIFF");
      h_tab += 3.0*plot_text_sep;
      p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, "FCST");
      p.write_centered_text(1, 1, h_tab, v_tab-10.0, 0.0, 0.5, "INT50");
      h_tab += 3.0*plot_text_sep;
      p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, "OBS");
      p.write_centered_text(1, 1, h_tab, v_tab-10.0, 0.0, 0.5, "INT50");
      h_tab += 3.0*plot_text_sep;
      p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, "FCST");
      p.write_centered_text(1, 1, h_tab, v_tab-10.0, 0.0, 0.5, "INT90");
      h_tab += 3.0*plot_text_sep;
      p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, "OBS");
      p.write_centered_text(1, 1, h_tab, v_tab-10.0, 0.0, 0.5, "INT90");
      h_tab += 3.0*plot_text_sep;
      p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, "TOT");
      p.write_centered_text(1, 1, h_tab, v_tab-10.0, 0.0, 0.5, "INTR");
      v_tab -= (plot_text_sep + 10.0);

      for(i=0; i<eng.n_clus && v_tab >= v_margin; i++) {

         h_tab = h_margin;

         // Cluster ID
         snprintf(junk, sizeof(junk), "%i", i+1);
         p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, junk);
         h_tab += 3.0*plot_text_sep;

         // Centroid Distance
         snprintf(junk, sizeof(junk), "%.2f", eng.pair_clus[i].centroid_dist);
         p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, junk);
         h_tab += 3.0*plot_text_sep;

         // Angle Difference
         snprintf(junk, sizeof(junk), "%.2f", eng.pair_clus[i].angle_diff);
         p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, junk);
         h_tab += 3.0*plot_text_sep;

         // Forecast Area
         snprintf(junk, sizeof(junk), "%i", nint(eng.pair_clus[i].Fcst[0].area));
         p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, junk);
         h_tab += 3.0*plot_text_sep;

         // Observation Area
         snprintf(junk, sizeof(junk), "%i", nint(eng.pair_clus[i].Obs[0].area));
         p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, junk);
         h_tab += 3.0*plot_text_sep;

         // Intersection Area
         snprintf(junk, sizeof(junk), "%i", nint(eng.pair_clus[i].intersection_area));
         p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, junk);
         h_tab += 3.0*plot_text_sep;

         // Union Area
         snprintf(junk, sizeof(junk), "%i", nint(eng.pair_clus[i].union_area));
         p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, junk);
         h_tab += 3.0*plot_text_sep;

         // Symmetric Difference
         snprintf(junk, sizeof(junk), "%i", nint(eng.pair_clus[i].symmetric_diff));
         p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, junk);
         h_tab += 3.0*plot_text_sep;

         // Forecast median intensity
         snprintf(junk, sizeof(junk), "%.2f", eng.pair_clus[i].Fcst[0].intensity_ptile.p50);
         p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, junk);
         h_tab += 3.0*plot_text_sep;

         // Observation median intensity
         snprintf(junk, sizeof(junk), "%.2f", eng.pair_clus[i].Obs[0].intensity_ptile.p50);
         p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, junk);
         h_tab += 3.0*plot_text_sep;

         // Forecast 90th percentile of intensity
         snprintf(junk, sizeof(junk), "%.2f", eng.pair_clus[i].Fcst[0].intensity_ptile.p90);
         p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, junk);
         h_tab += 3.0*plot_text_sep;

         // Observation median intensity
         snprintf(junk, sizeof(junk), "%.2f", eng.pair_clus[i].Obs[0].intensity_ptile.p90);
         p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, junk);
         h_tab += 3.0*plot_text_sep;

         // Total Interest
         if(eng.info_clus[i].interest_value < 0) label = na_str;
         else {
            snprintf(junk, sizeof(junk), "%.4f", eng.info_clus[i].interest_value);
            label = junk;
         }
         p.write_centered_text(1, 1, h_tab, v_tab, 0.0, 0.5, label);
         h_tab += 3.0*plot_text_sep;

         v_tab -= plot_text_sep;
      }

      /////////////////////////////////////////////////////////////////
      //
      // Finished with this page
      //
      /////////////////////////////////////////////////////////////////

      p.showpage();
      n_page++;
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void plot_threshold_merging(PSfile &p, Engine &eng, const char *title, int fcst) {
   Box dim;
   ShapeData merge_mask, merge_split, merge_shape;
   Polyline poly;
   int n_merge;
   double v_tab;

   if(fcst == 1) {
      merge_mask = *(eng.fcst_conv);
      merge_mask.threshold(eng.fcst_merge_thresh);
   }
   else if(fcst == 0) {
      merge_mask = *(eng.obs_conv);
      merge_mask.threshold(eng.obs_merge_thresh);
   }
   merge_split = split(merge_mask, n_merge);

   p.pagenumber(n_page);

   p.choose_font(31, 24.0, met_data_dir);
   p.write_centered_text(1, 1, h_tab_cen, 752.0, 0.5, 0.5, title);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw raw field
   //
   ////////////////////////////////////////////////////////////////////

   v_tab = page_height - 3.0*v_margin;
   set_dim(dim, v_tab - lg_plot_height, v_tab, h_tab_cen);
   if(fcst == 1) render_image(p, eng, FOEng, *eng.fcst_raw, dim, fcst, 0);
   if(fcst == 0) render_image(p, eng, FOEng, *eng.obs_raw, dim, fcst, 0);
   draw_border(p, dim);
   draw_map(p, dim);
   plot_colorbar(p, dim, fcst);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw split field
   //
   ////////////////////////////////////////////////////////////////////

   v_tab -= lg_plot_height;
   set_dim(dim, v_tab - lg_plot_height, v_tab, h_tab_cen);
   if(fcst == 1) render_image(p, eng, FOEng, *eng.fcst_split, dim, fcst, 2);
   if(fcst == 0) render_image(p, eng, FOEng, *eng.obs_split, dim, fcst, 2);
   draw_border(p, dim);
   draw_map(p, dim);
   draw_boundaries(p, merge_split, n_merge, dim);

   ////////////////////////////////////////////////////////////////////
   //
   // Finished with this page
   //
   ////////////////////////////////////////////////////////////////////

   p.showpage();
   n_page++;

   return;
}

///////////////////////////////////////////////////////////////////////

void draw_border(PSfile &p, Box &dim) {

   p.gsave();
   p.setlinewidth(l_thin);
   p.newpath();
   p.moveto(dim.left(), dim.bottom());
   p.lineto(dim.right(), dim.bottom());
   p.lineto(dim.right(), dim.top());
   p.lineto(dim.left(), dim.top());
   p.closepath();
   p.stroke();
   p.grestore();

   return;
}

///////////////////////////////////////////////////////////////////////

void draw_map(PSfile &p, Box &dim) {

   p.gsave();
   p.setlinewidth(l_thin);
   draw_map(grid, xy_bb, p, dim, c_map, met_data_dir);
   p.grestore();

   return;
}

///////////////////////////////////////////////////////////////////////

void render_image(PSfile &p, Engine &eng, EngineType eng_type,
                  const ShapeData &wd, Box &dim, int fcst, int split) {
   RenderInfo render_info;
   Ppm ppm_image;
   int x, y, v_int;
   double mag, v;
   Color c;
   Color *c_fill_ptr = (Color *) 0;
   ColorTable *ct_ptr = (ColorTable *) 0;

   //
   // Set up pointers to the appropriate colortable and fill color values
   //
   if(eng_type == FFEng) {
      ct_ptr = &fcst_ct;
      c_fill_ptr = &c_fcst_fill;
   }
   else if(eng_type == OOEng) {
      ct_ptr = &obs_ct;
      c_fill_ptr = &c_obs_fill;
   }
   else { // eng_type == FOEng

      if(fcst == 1) {
         ct_ptr = &fcst_ct;
         c_fill_ptr = &c_fcst_fill;
      }
      else {
         ct_ptr = &obs_ct;
         c_fill_ptr = &c_obs_fill;
      }
   }

   //
   // Convert the ShapeData object to PPM
   //
   ppm_image.set_size_xy((int) xy_bb.width(), (int) xy_bb.height());

   for(x=nint(xy_bb.left()); x<xy_bb.right(); x++) {
      for(y=nint(xy_bb.bottom()); y<xy_bb.top(); y++) {

         v = wd.data(x, y);

         v_int = nint(v);

         //
         // Single object field
         //
         if(split == 1) {
            if(is_bad_data(v)) { // Should be no bad data left at this point
               c = *c_fill_ptr;
            }
            else if(v_int == 0) {        // Set to white
               c.set_rgb(255, 255, 255);
            }
            // Single fcst object field
            else if(v_int > 0 && fcst) {
               c = eng.fcst_color[v_int-1];
            }
            // Single obs object field
            else if(v_int > 0 && !fcst) {
               c = eng.obs_color[v_int-1];
            }
         }
         //
         // Cluster object field
         //
         else if(split == 2) {
            if(is_bad_data(v)) { // Should be no bad data left at this point
               c = *c_fill_ptr;
            }
            else if(v_int == 0) {        // Set to white
               c.set_rgb(255, 255, 255);
            }
            else {
               c = c_bndy;
            }
         }
         //
         // Raw data field
         //
         else {
            if(is_bad_data(v)) {
               c = *c_fill_ptr;
            }
            else {
               c = ct_ptr->nearest(wd.data(x, y));
            }
         }

         ppm_image.putxy(c, x-nint(xy_bb.left()), y-nint(xy_bb.bottom()));
      } // end for y
   } // end for x

   mag = (dim.right() - dim.left())/xy_bb.width();

   render_info.set_ll(dim.left(), dim.bottom());
   render_info.set_mag(mag);
   render_info.set_color();
   render_info.add_filter(RunLengthEncode);
   render_info.add_filter(ASCII85Encode);
   render(p, ppm_image, render_info);

   return;
}

///////////////////////////////////////////////////////////////////////

void plot_simple_ids(PSfile &p, Engine &eng, Box &dim, int fcst) {
   int i, n_shapes;
   double grid_x, grid_y, page_x, page_y;
   Color c;
   char label[max_str_len];

   p.gsave();

   if(fcst) n_shapes = eng.n_fcst;
   else     n_shapes = eng.n_obs;

   p.setlinewidth(0.0);

   for(i=0; i<n_shapes; i++) {

      if(fcst) {
         grid_x = eng.fcst_single[i].centroid_x;
         grid_y = eng.fcst_single[i].centroid_y;
      }
      else {
         grid_x = eng.obs_single[i].centroid_x;
         grid_y = eng.obs_single[i].centroid_y;
      }

      gridxy_to_pagexy(grid, xy_bb, grid_x, grid_y, page_x, page_y, dim);

      if(fcst) c = eng.fcst_color[i];
      else     c = eng.obs_color[i];

      p.setrgbcolor(c.red()/255.0, c.green()/255.0, c.blue()/255.0);
      p.choose_font(28, 16.0, met_data_dir);
      snprintf(label, sizeof(label), "%i", i+1);
      p.write_centered_text(2, 1, page_x, page_y, 0.5, 0.5, label);

      // Draw outline in black
      p.setrgbcolor(0.0, 0.0, 0.0);
      p.write_centered_text(2, 0, page_x, page_y, 0.5, 0.5, label);
   }

   p.grestore();

   return;
}

///////////////////////////////////////////////////////////////////////

void draw_convex_hulls(PSfile &p, Engine &eng, const Box &dim,
                       int fcst, int id_flag) {
   int i, j;
   double grid_x, grid_y, page_x, page_y;
   Polyline poly;
   Color c;
   char label[max_str_len];

   p.gsave();
   p.setlinewidth(0.0);

   //
   // Draw convex hulls around collection of shapes
   //
   for(i=0; i<eng.collection.n_sets; i++) {

      if(fcst) poly = eng.pair_clus[i].Fcst[0].convex_hull;
      else     poly = eng.pair_clus[i].Obs[0].convex_hull;
      draw_polyline(p, poly, c_hull, dim, false);

      //
      // Plot the cluster id
      //
      if(id_flag) {

         //
         // Get the centroid location and color for this set
         //
         if(fcst) {
            grid_x = eng.pair_clus[i].Fcst[0].centroid_x;
            grid_y = eng.pair_clus[i].Fcst[0].centroid_y;
            j = eng.collection.set[i].fcst_number[0]-1;
            c = eng.fcst_color[j];
         }
         else {
            grid_x = eng.pair_clus[i].Obs[0].centroid_x;
            grid_y = eng.pair_clus[i].Obs[0].centroid_y;
            j = eng.collection.set[i].obs_number[0]-1;
            c = eng.obs_color[j];
         }
         gridxy_to_pagexy(grid, xy_bb, grid_x, grid_y, page_x, page_y, dim);

         p.setrgbcolor(c.red()/255.0, c.green()/255.0, c.blue()/255.0);
         p.choose_font(28, 16.0, met_data_dir);
         snprintf(label, sizeof(label), "%i", i+1);
         p.write_centered_text(2, 1, page_x, page_y, 0.5, 0.5, label);

         // Draw outline in black
         p.setrgbcolor(0.0, 0.0, 0.0);
         p.write_centered_text(2, 0, page_x, page_y, 0.5, 0.5, label);
      }
   }

   p.grestore();

   return;
}

///////////////////////////////////////////////////////////////////////

void draw_boundaries(PSfile &p, Engine &eng, const Box &dim, int fcst) {
   int i, j;

   //
   // Draw boundaries around each of the objects
   //

   if(fcst) {
      for(i=0; i<eng.n_fcst; i++) {
         for(j=0; j<eng.fcst_single[i].n_bdy; j++) {
            draw_polyline(p, eng.fcst_single[i].boundary[j], c_bndy, dim, false);
         }
      }
   }
   else {
      for(i=0; i<eng.n_obs; i++) {
         for(j=0; j<eng.obs_single[i].n_bdy; j++) {
            draw_polyline(p, eng.obs_single[i].boundary[j], c_bndy, dim, false);
         }
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void draw_boundaries(PSfile &p, ShapeData &split_wd, int n_shapes,
                     const Box &dim) {
   int i;
   Polyline poly;
   ShapeData wd_shape;

   //
   // Draw boundary for each shape in the split field
   //
   for(i=0; i<n_shapes; i++) {
      wd_shape = select(split_wd, i+1);
      poly = wd_shape.single_boundary();
      draw_polyline(p, poly, c_bndy, dim, false);
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void draw_polyline(PSfile &p, Polyline &poly, const Color &c,
                   Box dim, bool latlon) {
   int i;
   double lat, lon;
   double grid_x_prev, grid_y_prev, grid_x_cur, grid_y_cur;
   double page_x, page_y;

   // Nothing to draw for empty polylines
   if(poly.n_points <= 0) {

      return;
   }

   // Set to specified values
   p.gsave();
   p.setlinewidth(l_thick);
   p.setrgbcolor(c.red()/255.0, c.green()/255.0, c.blue()/255.0);
   p.newpath();

   //
   // Set the starting Grid x,y to the first vertex of
   // the polyline
   //
   if(latlon) {
      lat = poly.u[0];
      lon = poly.v[0];
      grid.latlon_to_xy(lat, lon, grid_x_prev, grid_y_prev);
   }
   // Grid coordinates
   else {
      grid_x_prev = poly.u[0];
      grid_y_prev = poly.v[0];
   }

   // Convert the starting Grid x/y to page x,y and move to it
   gridxy_to_pagexy(grid, xy_bb, grid_x_prev, grid_y_prev, page_x, page_y, dim);
   p.moveto(page_x, page_y);

   // Loop through the vertices and back to the starting vertex
   for(i=1; i<poly.n_points; i++) {

      //
      // Draw an arc between the previous Grid x,y point and the
      // current point.
      //
      if(latlon) {
         lat = poly.u[i];
         lon = poly.v[i];
         grid.latlon_to_xy(lat, lon, grid_x_cur, grid_y_cur);
      }
      // Grid coordinates
      else {
         grid_x_cur = poly.u[i];
         grid_y_cur = poly.v[i];
      }

      gridxy_to_pagexy(grid, xy_bb, grid_x_cur, grid_y_cur, page_x, page_y, dim);

      // Plot polylines as straight lines in the grid
      if(engine.wconf.plot_gcarc_flag().ival() == 0) {
         p.lineto(page_x, page_y);
      }
      // Plot polylines using great circle arcs
      else {
         gc_arcto(grid, xy_bb, p, grid_x_prev, grid_y_prev,
                  grid_x_cur, grid_y_cur, 10, dim);
      }

      // Reset the previous Grid x,y to the current Grid x,y
      grid_x_prev = grid_x_cur;
      grid_y_prev = grid_y_cur;
   }

   p.closepath();
   // Stroke the path
   p.stroke();

   p.grestore();

   return;
}

///////////////////////////////////////////////////////////////////////

void plot_colorbar(PSfile &p, Box &dim, int fcst) {
   int i, n_colors;
   char label[max_str_len];
   double bar_width, bar_height, x_ll, y_ll;
   ColorTable *ct_ptr = (ColorTable *) 0;
   Color c;

   //
   // Set up the pointer to the appropriate colortable
   //
   if(fcst == 1) ct_ptr = &fcst_ct;
   else          ct_ptr = &obs_ct;

   //
   // Get the number of colortable entries
   //
   n_colors = ct_ptr->n_entries();

   //
   // Draw colorbar in the bottom right corner of the Bounding Box
   //
   p.gsave();
   p.setlinewidth(l_thin);
   p.choose_font(28, 8.0, met_data_dir);

   bar_width = h_margin/2.0;
   bar_height = (dim.top() - dim.bottom())/n_colors;

   x_ll = dim.right();
   y_ll = dim.bottom();

   for(i=0; i<n_colors; i++) {

     c = (*ct_ptr)[i].color();

     //
     // Color box
     //
     p.setrgbcolor(c.red()/255.0, c.green()/255.0, c.blue()/255.0);
     p.newpath();
     p.moveto(x_ll,             y_ll);
     p.lineto(x_ll,             y_ll + bar_height);
     p.lineto(x_ll + bar_width, y_ll + bar_height);
     p.lineto(x_ll + bar_width, y_ll);
     p.closepath();
     p.fill();

     //
     // Outline color box
     //
     p.setrgbcolor(0.0, 0.0, 0.0);
     p.newpath();
     p.moveto(x_ll,             y_ll);
     p.lineto(x_ll,             y_ll + bar_height);
     p.lineto(x_ll + bar_width, y_ll + bar_height);
     p.lineto(x_ll + bar_width, y_ll);
     p.closepath();
     p.stroke();

     //
     // Add text
     //
     if(i > 0 && i%stride == 0) {

        //
        // Choose the label format based on whether the colortable
        // has been rescaled
        //
        if(ct_ptr->rescale_flag) {
           snprintf(label, sizeof(label), "%.2f", (*ct_ptr)[i].value_low());
        }
        else {
           snprintf(label, sizeof(label), "%g", (*ct_ptr)[i].value_low());
        }

        p.write_centered_text(2, 1,  x_ll + bar_width + 2.0,
                              y_ll, 0.0, 0.5, label);
     }

     y_ll += bar_height;
   }

   p.grestore();

  return;
}

//////////////////////////////////////////////////////////////////////

void build_outfile_name(const char *suffix, ConcatString &str) {
   int mon, day, yr, hr, min, sec;
   int l_hr, l_min, l_sec;
   int a_hr, a_min, a_sec;
   char tmp_str[max_str_len];

   //
   // Create output file name
   //

   // Append the output directory and program name
   str << cs_erase << out_dir << "/" << program_name;

   // Append the output prefix, if defined
   if(strlen(engine.wconf.output_prefix().sval()) > 0)
      str << "_" << engine.wconf.output_prefix().sval();

   // Append the timing information
   sec_to_hms(engine.fcst_raw->data.lead(), l_hr, l_min, l_sec);
   unix_to_mdyhms(engine.fcst_raw->data.valid(),
                  mon, day, yr, hr, min, sec);
   sec_to_hms(engine.fcst_raw->data.accum(), a_hr, a_min, a_sec);
   snprintf(tmp_str, sizeof(tmp_str), 
           "%.2i%.2i%.2iL_%.4i%.2i%.2i_%.2i%.2i%.2iV_%.2i%.2i%.2iA",
           l_hr, l_min, l_sec,
           yr, mon, day, hr, min, sec,
           a_hr, a_min, a_sec);
   str << "_" << tmp_str;

   // Append the suffix
   str << suffix;

   return;
}

//////////////////////////////////////////////////////////////////////

void write_obj_stats() {
   AsciiTable obj_at, fcst_merge_at, obs_merge_at;
   ofstream out;
   ConcatString stat_file;

   //
   // Create output MODE object stats files
   //
   build_outfile_name("_obj.txt", stat_file);

   //
   // Open output stat file
   //
   out.open(stat_file);

   if(!out) {
      mlog << Error << "\nwrite_obj_stats() -> "
           << "unable to open stats output file \""
           << stat_file << "\"\n\n";
      exit(1);
   }
   out.setf(ios::fixed);

   //
   // List stat file as it is being created
   //
   mlog << Debug(1) << "Creating Fcst-Obs Object Statistics file: " << stat_file << "\n";

   //
   // Write the output statistics to an AsciiTable object
   //
   write_engine_stats(engine, grid, obj_at);

   //
   // Write the AsciiTable object to the output file
   //
   out << obj_at;

   out.close();

   if(engine.wconf.fcst_merge_flag().ival() == 2 ||
      engine.wconf.fcst_merge_flag().ival() == 3) {

      //
      // Create output stats file for forecast merging
      //
      build_outfile_name("_fcst_merge.txt", stat_file);
      out.open(stat_file);

      if(!out) {
         mlog << Error << "\nwrite_obj_stats() -> "
              << "unable to open stats output file \""
              << stat_file << "\"\n\n";
         exit(1);
      }
      out.setf(ios::fixed);

      mlog << Debug(1) << "Creating Fcst-Fcst Object Statistics file: " << stat_file << "\n";

      //
      // Write the output statistics to an AsciiTable object
      //
      write_engine_stats(*engine.fcst_engine, grid, fcst_merge_at);

      //
      // Write the AsciiTable object to the output file
      //
      out << fcst_merge_at;

      out.close();
   }

   if(engine.wconf.obs_merge_flag().ival() == 2 ||
      engine.wconf.obs_merge_flag().ival() == 3) {

      //
      // Create output stats file for obseravation merging
      //
      build_outfile_name("_obs_merge.txt", stat_file);
      out.open(stat_file);

      if(!out) {
         mlog << Error << "\nwrite_obj_stats() -> "
              << "unable to open stats output file \""
              << stat_file << "\"\n\n";
         exit(1);
      }
      out.setf(ios::fixed);

      mlog << Debug(1) << "Creating Obs-Obs Object Statistics file: " << stat_file << "\n";

      //
      // Write the output statistics to an AsciiTable object
      //
      write_engine_stats(*engine.obs_engine, grid, obs_merge_at);

      //
      // Write the AsciiTable object to the output file
      //
      out << obs_merge_at;

      out.close();
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void write_obj_netcdf() {
   int n, x, y;
   ConcatString out_file;

   float *fcst_raw_data  = (float *) 0;
   int   *fcst_obj_data  = (int *)   0;
   int   *fcst_clus_data = (int *)   0;
   float *obs_raw_data   = (float *) 0;
   int   *obs_obj_data   = (int *)   0;
   int   *obs_clus_data  = (int *)   0;

   NcFile *f_out         = (NcFile *) 0;

   NcDim  *lat_dim       = (NcDim *)  0;
   NcDim  *lon_dim       = (NcDim *)  0;

   NcVar  *fcst_raw_var  = (NcVar *)  0;
   NcVar  *fcst_obj_var  = (NcVar *)  0;
   NcVar  *fcst_clus_var = (NcVar *)  0;

   NcVar  *obs_raw_var   = (NcVar *)  0;
   NcVar  *obs_obj_var   = (NcVar *)  0;
   NcVar  *obs_clus_var  = (NcVar *)  0;

   //
   // Create output NetCDF file name
   //
   build_outfile_name("_obj.nc", out_file);

   mlog << Debug(1) << "Creating Object NetCDF file: " << out_file << "\n";

   //
   // Create a new NetCDF file and open it
   // NOTE: must multiply longitudes throughout by -1 to convert from
   // degrees_west to degree_east
   //
   f_out = new NcFile(out_file, NcFile::Replace);

   if(!f_out->is_valid()) {
      mlog << Error << "\nwrite_obj_netcdf() -> trouble opening output file "
           << out_file << "\n\n";
      f_out->close();
      delete f_out;
      f_out = (NcFile *) 0;

      exit(1);
   }

   // Add global attributes
   write_netcdf_global(f_out, out_file.text(), program_name);

   // Add the projection information
   write_netcdf_proj(f_out, grid);

   // Define Dimensions
   lat_dim = f_out->add_dim("lat", (long) grid.ny());
   lon_dim = f_out->add_dim("lon", (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(f_out, lat_dim, lon_dim, grid);

   // Define Variables
   fcst_raw_var = f_out->add_var("fcst_obj_raw", ncFloat, lat_dim, lon_dim);
   fcst_obj_var = f_out->add_var("fcst_obj_id", ncInt, lat_dim, lon_dim);
   fcst_clus_var = f_out->add_var("fcst_clus_id", ncInt, lat_dim, lon_dim);

   obs_raw_var = f_out->add_var("obs_obj_raw", ncFloat, lat_dim, lon_dim);
   obs_obj_var = f_out->add_var("obs_obj_id", ncInt, lat_dim, lon_dim);
   obs_clus_var = f_out->add_var("obs_clus_id", ncInt, lat_dim, lon_dim);
   
   // Add forecast variable attributes
   fcst_raw_var->add_att("long_name", "Forecast Object Raw Values");
   write_netcdf_var_times(fcst_raw_var, engine.fcst_raw->data);
   fcst_raw_var->add_att("_FillValue", bad_data_float);

   fcst_obj_var->add_att("long_name", "Forecast Object ID");
   write_netcdf_var_times(fcst_obj_var, engine.fcst_raw->data);
   fcst_obj_var->add_att("_FillValue", bad_data_int);

   fcst_clus_var->add_att("long_name", "Forecast Cluster Object ID");
   write_netcdf_var_times(fcst_clus_var, engine.fcst_raw->data);
   fcst_clus_var->add_att("_FillValue", bad_data_int);

   // Add observation variable attributes
   obs_raw_var->add_att("long_name", "Observation Object Raw Values");
   write_netcdf_var_times(obs_raw_var, engine.obs_raw->data);
   obs_raw_var->add_att("_FillValue", bad_data_float);

   obs_obj_var->add_att("long_name", "Observation Object ID");
   write_netcdf_var_times(obs_obj_var, engine.obs_raw->data);
   obs_obj_var->add_att("_FillValue", bad_data_int);

   obs_clus_var->add_att("long_name", "Observation Cluster Object ID");
   write_netcdf_var_times(obs_clus_var, engine.obs_raw->data);
   obs_clus_var->add_att("_FillValue", bad_data_int);

   //
   // Allocate memory for the raw values and object ID's for each grid box
   //
   fcst_raw_data  = new float [grid.nx()*grid.ny()];
   fcst_obj_data  = new int   [grid.nx()*grid.ny()];
   fcst_clus_data = new int   [grid.nx()*grid.ny()];
   obs_raw_data   = new float [grid.nx()*grid.ny()];
   obs_obj_data   = new int   [grid.nx()*grid.ny()];
   obs_clus_data  = new int   [grid.nx()*grid.ny()];

   for(x=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         n = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);

         //
         // Get raw values and object ID's for each grid box
         //
         if(engine.fcst_split->is_nonzero(x, y) ) {
            fcst_raw_data[n] = engine.fcst_raw->data(x, y);
            fcst_obj_data[n] = nint(engine.fcst_split->data(x, y));
         }
         else {
            fcst_raw_data[n] = bad_data_float;
            fcst_obj_data[n] = bad_data_int;
         }

         if(engine.obs_split->is_nonzero(x, y) ) {
            obs_raw_data[n] = engine.obs_raw->data(x, y);
            obs_obj_data[n] = nint(engine.obs_split->data(x, y));
         }
         else {
            obs_raw_data[n] = bad_data_float;
            obs_obj_data[n] = bad_data_int;
         }

         //
         // Get cluster object ID's for each grid box
         //

         // Write the index of the cluster object
         if(engine.fcst_clus_split->data(x, y) > 0) {
            fcst_clus_data[n] = nint(engine.fcst_clus_split->data(x, y));
         }
         // Write a value of 0 for unmatched simple objects
         else if(engine.fcst_split->data(x, y) > 0) {
            fcst_clus_data[n] = unmatched_id;
         }
         // Otherwise, write bad data
         else {
            fcst_clus_data[n] = bad_data_int;
         }

         // Write the index of the cluster object
         if(engine.obs_clus_split->data(x, y) > 0) {
            obs_clus_data[n] = nint(engine.obs_clus_split->data(x, y));
         }
         // Write a value of 0 for unmatched simple objects
         else if(engine.obs_split->data(x, y) > 0) {
            obs_clus_data[n] = unmatched_id;
         }
         // Otherwise, write bad data
         else {
            obs_clus_data[n] = bad_data_int;
         }
      }
   }

   //
   // Write the forecast and observation raw value variables
   //
   if( !fcst_raw_var->put(&fcst_raw_data[0], grid.ny(), grid.nx()) ||
       !obs_raw_var->put(&obs_raw_data[0], grid.ny(), grid.nx()) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with the fcst_raw_var->put or obs_raw_var->put\n\n";
      exit(1);
   }

   //
   // Write the forecast and observation object ID variables
   //
   if( !fcst_obj_var->put(&fcst_obj_data[0], grid.ny(), grid.nx()) ||
       !obs_obj_var->put(&obs_obj_data[0], grid.ny(), grid.nx()) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with the fcst_obj_var->put or obs_obj_var->put\n\n";
      exit(1);
   }

   //
   // Write the forecast and observation cluster object ID variables
   //
   if( !fcst_clus_var->put(&fcst_clus_data[0], grid.ny(), grid.nx()) ||
       !obs_clus_var->put(&obs_clus_data[0], grid.ny(), grid.nx()) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with the fcst_clus_var->put or obs_clus_var->put\n\n";
      exit(1);
   }

   //
   // Delete allocated memory
   //
   if(fcst_raw_data)  { delete fcst_raw_data;  fcst_raw_data = (float *) 0; }
   if(fcst_obj_data)  { delete fcst_obj_data;  fcst_obj_data = (int *) 0; }
   if(fcst_clus_data) { delete fcst_clus_data; fcst_clus_data = (int *) 0; }
   if(obs_raw_data)   { delete obs_raw_data;   obs_raw_data = (float *) 0; }
   if(obs_obj_data)   { delete obs_obj_data;   obs_obj_data = (int *) 0; }
   if(obs_clus_data)  { delete obs_clus_data;  obs_clus_data = (int *) 0; }

   //
   // Write out the values of the vertices of the boundary polylines for
   // the simple objects.
   //
   write_bdy_netcdf(f_out);

   //
   // Close the NetCDF file
   //
   f_out->close();
   delete f_out;
   f_out = (NcFile *) 0;

   return;
}

///////////////////////////////////////////////////////////////////////

void write_bdy_netcdf(NcFile *f_out) {

   //
   // Write out the number of forecast and observation objects
   //
   NcVar *n_fcst_obj_var = (NcVar *)  0;
   NcVar *n_obs_obj_var  = (NcVar *)  0;

   // Define scalar variables
   n_fcst_obj_var = f_out->add_var("n_fcst_obj", ncInt);
   n_obs_obj_var  = f_out->add_var("n_obs_obj",  ncInt);

   //
   // Write the number of forecast and observation objects
   //
   if( !n_fcst_obj_var->put(&engine.n_fcst) ||
       !n_obs_obj_var->put(&engine.n_obs) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with the n_fcst_obj_var-> or "
           << "n_obs_obj_var->put\n\n";
      exit(1);
   }

   //
   // Only write out the boundary polyline points if there are objects
   // present.
   //
   if(engine.n_fcst > 0) write_fcst_bdy_netcdf(f_out);
   if(engine.n_obs  > 0) write_obs_bdy_netcdf(f_out);

   return;
}

///////////////////////////////////////////////////////////////////////

void write_fcst_bdy_netcdf(NcFile *f_out) {
   int i, j, x, y, n_pts;
   double lat, lon;

   int   *bdy_start          = (int   *) 0;
   int   *bdy_npts           = (int   *) 0;
   float *bdy_lat            = (float *) 0;
   float *bdy_lon            = (float *) 0;
   int   *bdy_x              = (int   *) 0;
   int   *bdy_y              = (int   *) 0;

   // Dimensions and variables for each object
   NcDim  *obj_dim           = (NcDim *)  0;
   NcVar  *obj_bdy_start_var = (NcVar *)  0;
   NcVar  *obj_bdy_npts_var  = (NcVar *)  0;

   // Dimensions and variables for each boundary point
   NcDim  *bdy_dim           = (NcDim *)  0;
   NcVar  *bdy_lat_var       = (NcVar *)  0;
   NcVar  *bdy_lon_var       = (NcVar *)  0;
   NcVar  *bdy_x_var         = (NcVar *)  0;
   NcVar  *bdy_y_var         = (NcVar *)  0;

   // Get the number of forecast boundary polyline points
   for(i=0, n_pts=0; i<engine.n_fcst; i++)
      n_pts += engine.fcst_single[i].boundary[0].n_points;

   // Define dimensions
   obj_dim = f_out->add_dim("fcst_obj", (long) engine.n_fcst);
   bdy_dim = f_out->add_dim("fcst_bdy", (long) n_pts);

   // Define variables
   obj_bdy_start_var = f_out->add_var("fcst_obj_bdy_start", ncInt, obj_dim);
   obj_bdy_npts_var  = f_out->add_var("fcst_obj_bdy_npts", ncInt, obj_dim);
   bdy_lat_var       = f_out->add_var("fcst_bdy_lat", ncFloat, bdy_dim);
   bdy_lon_var       = f_out->add_var("fcst_bdy_lon", ncFloat, bdy_dim);
   bdy_x_var         = f_out->add_var("fcst_bdy_x", ncInt, bdy_dim);
   bdy_y_var         = f_out->add_var("fcst_bdy_y", ncInt, bdy_dim);

   // Add variable attributes
   obj_bdy_start_var->add_att("long_name", "Forecast Boundary Starting Index");
   obj_bdy_npts_var->add_att("long_name", "Number of Forecast Boundary Points");
   bdy_lat_var->add_att("long_name", "Forecast Boundary Point Latitude");
   bdy_lat_var->add_att("units", "degrees_north");
   bdy_lon_var->add_att("long_name", "Forecast Boundary Point Longitude");
   bdy_lon_var->add_att("units", "degrees_east");
   bdy_x_var->add_att("long_name", "Forecast Boundary Point X-Coordinate");
   bdy_y_var->add_att("long_name", "Forecast Boundary Point Y-Coordinate");

   //
   // Allocate memory for the forecast boundary points
   //
   bdy_start = new int   [engine.n_fcst];
   bdy_npts  = new int   [engine.n_fcst];
   bdy_lat   = new float [n_pts];
   bdy_lon   = new float [n_pts];
   bdy_x     = new int   [n_pts];
   bdy_y     = new int   [n_pts];

   //
   // Process each forecast object and store the boundary
   // polyline points.
   //
   for(i=0, n_pts=0; i<engine.n_fcst; i++) {

      // Store the starting point for this object.
      bdy_start[i] = n_pts;

      // Store the number of points in this polyline.
      bdy_npts[i] = engine.fcst_single[i].boundary[0].n_points;

      for(j=0; j<bdy_npts[i]; j++, n_pts++) {

         // Get the boundary point (x,y) coordinates and store them
         x = nint(engine.fcst_single[i].boundary[0].u[j]);
         y = nint(engine.fcst_single[i].boundary[0].v[j]);
         bdy_x[n_pts] = x;
         bdy_y[n_pts] = y;

         // Convert to lat/lon and store them
         grid.xy_to_latlon(x, y, lat, lon);
         bdy_lat[n_pts] = lat;
         bdy_lon[n_pts] = -1.0*lon;
      }
   }

   //
   // Write the forecast boundary polyline information
   //
   if( !obj_bdy_start_var->put(&bdy_start[0], engine.n_fcst) ||
       !obj_bdy_npts_var->put(&bdy_npts[0], engine.n_fcst) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with the obj_bdy_start_var->put or "
           << "obj_bdy_npts_var->put\n\n";
      exit(1);
   }

   //
   // Write the forecast boundary lat/lon points
   //
   if( !bdy_lat_var->put(&bdy_lat[0], n_pts) ||
       !bdy_lon_var->put(&bdy_lon[0], n_pts) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with bdy_lat_var->put "
           << "or bdy_lon_var->put\n\n";
      exit(1);
   }

   //
   // Write the forecast boundary (x,y) points
   //
   if( !bdy_x_var->put(&bdy_x[0], n_pts) ||
       !bdy_y_var->put(&bdy_y[0], n_pts) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with bdy_x_var->put "
           << "or bdy_y_var->put\n\n";
      exit(1);
   }

   //
   // Delete allocated memory
   //
   if(bdy_start) { delete bdy_start; bdy_start = (int   *) 0; }
   if(bdy_npts)  { delete bdy_npts;  bdy_npts  = (int   *) 0; }
   if(bdy_lat)   { delete bdy_lat;   bdy_lat   = (float *) 0; }
   if(bdy_lon)   { delete bdy_lon;   bdy_lon   = (float *) 0; }
   if(bdy_x)     { delete bdy_x;     bdy_x     = (int   *) 0; }
   if(bdy_y)     { delete bdy_y;     bdy_y     = (int   *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////

void write_obs_bdy_netcdf(NcFile *f_out) {
   int i, j, x, y, n_pts;
   double lat, lon;

   int   *bdy_start          = (int   *) 0;
   int   *bdy_npts           = (int   *) 0;
   float *bdy_lat            = (float *) 0;
   float *bdy_lon            = (float *) 0;
   int   *bdy_x              = (int   *) 0;
   int   *bdy_y              = (int   *) 0;

   // Dimensions and variables for each object
   NcDim  *obj_dim           = (NcDim *)  0;
   NcVar  *obj_bdy_start_var = (NcVar *)  0;
   NcVar  *obj_bdy_npts_var  = (NcVar *)  0;

   // Dimensions and variables for each boundary point
   NcDim  *bdy_dim           = (NcDim *)  0;
   NcVar  *bdy_lat_var       = (NcVar *)  0;
   NcVar  *bdy_lon_var       = (NcVar *)  0;
   NcVar  *bdy_x_var         = (NcVar *)  0;
   NcVar  *bdy_y_var         = (NcVar *)  0;

   // Get the number of observation boundary polyline points
   for(i=0, n_pts=0; i<engine.n_obs; i++)
      n_pts += engine.obs_single[i].boundary[0].n_points;

   // Define dimensions
   obj_dim = f_out->add_dim("obs_obj", (long) engine.n_obs);
   bdy_dim = f_out->add_dim("obs_bdy", (long) n_pts);

   // Define variables
   obj_bdy_start_var = f_out->add_var("obs_obj_bdy_start", ncInt, obj_dim);
   obj_bdy_npts_var  = f_out->add_var("obs_obj_bdy_npts", ncInt, obj_dim);
   bdy_lat_var       = f_out->add_var("obs_bdy_lat", ncFloat, bdy_dim);
   bdy_lon_var       = f_out->add_var("obs_bdy_lon", ncFloat, bdy_dim);
   bdy_x_var         = f_out->add_var("obs_bdy_x", ncInt, bdy_dim);
   bdy_y_var         = f_out->add_var("obs_bdy_y", ncInt, bdy_dim);

   // Add variable attributes
   obj_bdy_start_var->add_att("long_name", "Observation Boundary Starting Index");
   obj_bdy_npts_var->add_att("long_name", "Number of Observation Boundary Points");
   bdy_lat_var->add_att("long_name", "Observation Boundary Point Latitude");
   bdy_lat_var->add_att("units", "degrees_north");
   bdy_lon_var->add_att("long_name", "Observation Boundary Point Longitude");
   bdy_lon_var->add_att("units", "degrees_east");
   bdy_x_var->add_att("long_name", "Observation Boundary Point X-Coordinate");
   bdy_y_var->add_att("long_name", "Observation Boundary Point Y-Coordinate");

   //
   // Allocate memory for the observation boundary points
   //
   bdy_start = new int   [engine.n_obs];
   bdy_npts  = new int   [engine.n_obs];
   bdy_lat   = new float [n_pts];
   bdy_lon   = new float [n_pts];
   bdy_x     = new int   [n_pts];
   bdy_y     = new int   [n_pts];

   //
   // Process each observation object and store the boundary
   // polyline points.
   //
   for(i=0, n_pts=0; i<engine.n_obs; i++) {

      // Store the starting point for this object.
      bdy_start[i] = n_pts;

      // Store the number of points in this polyline.
      bdy_npts[i] = engine.obs_single[i].boundary[0].n_points;

      for(j=0; j<bdy_npts[i]; j++, n_pts++) {

         // Get the boundary point (x,y) coordinates and store them
         x = nint(engine.obs_single[i].boundary[0].u[j]);
         y = nint(engine.obs_single[i].boundary[0].v[j]);
         bdy_x[n_pts] = x;
         bdy_y[n_pts] = y;

         // Convert to lat/lon and store them
         grid.xy_to_latlon(x, y, lat, lon);
         bdy_lat[n_pts] = lat;
         bdy_lon[n_pts] = -1.0*lon;
      }
   }

   //
   // Write the observation boundary polyline information
   //
   if( !obj_bdy_start_var->put(&bdy_start[0], engine.n_obs) ||
       !obj_bdy_npts_var->put(&bdy_npts[0], engine.n_obs) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with the obj_bdy_start_var->put or "
           << "obj_bdy_npts_var->put\n\n";
      exit(1);
   }

   //
   // Write the observation boundary lat/lon points
   //
   if( !bdy_lat_var->put(&bdy_lat[0], n_pts) ||
       !bdy_lon_var->put(&bdy_lon[0], n_pts) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with bdy_lat_var->put "
           << "or bdy_lon_var->put\n\n";
      exit(1);
   }

   //
   // Write the observation boundary (x,y) points
   //
   if( !bdy_x_var->put(&bdy_x[0], n_pts) ||
       !bdy_y_var->put(&bdy_y[0], n_pts) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with bdy_x_var->put "
           << "or bdy_y_var->put\n\n";
      exit(1);
   }

   //
   // Delete allocated memory
   //
   if(bdy_start) { delete bdy_start; bdy_start = (int   *) 0; }
   if(bdy_npts)  { delete bdy_npts;  bdy_npts  = (int   *) 0; }
   if(bdy_lat)   { delete bdy_lat;   bdy_lat   = (float *) 0; }
   if(bdy_lon)   { delete bdy_lon;   bdy_lon   = (float *) 0; }
   if(bdy_x)     { delete bdy_x;     bdy_x     = (int   *) 0; }
   if(bdy_y)     { delete bdy_y;     bdy_y     = (int   *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////

void write_ct_stats() {
   AsciiTable cts_at;
   ofstream out;
   int i;
   double v;
   ConcatString stat_file;

   //
   // Create output contingency table stat file name
   //
   build_outfile_name("_cts.txt", stat_file);

   //
   // Open output stat file
   //
   out.open(stat_file);

   if(!out) {
      mlog << Error << "\nwrite_ct_stats() -> "
           << "unable to open stats output file \""
           << stat_file << "\"\n\n";
      exit(1);
   }
   out.setf(ios::fixed);

   //
   // List stat file as it is being created
   //
   mlog << Debug(1) << "Creating Contingency Table Statistics file: " << stat_file << "\n";

   //
   // Setup the AsciiTable to be used
   //
   cts_at.clear();
   i = n_mode_hdr_columns + n_mode_cts_columns;
   cts_at.set_size(4, i);                      // Set table size
   cts_at.set_table_just(LeftJust);            // Left-justify columns
   cts_at.set_precision(default_precision);    // Set the precision
   cts_at.set_bad_data_value(bad_data_double); // Set the bad data value
   cts_at.set_bad_data_str(na_str);            // Set the bad data string
   cts_at.set_delete_trailing_blank_rows(1);   // No trailing blank rows

   //
   // Write out the MODE header columns
   //
   for(i=0; i<n_mode_hdr_columns; i++) {
      cts_at.set_entry(0, i, mode_hdr_columns[i]);
   }

   //
   // Write out the MODE contingecy table header columns
   //
   for(i=0; i<n_mode_cts_columns; i++) {
      cts_at.set_entry(0, i + n_mode_hdr_columns, mode_cts_columns[i]);
   }

   //
   // Store the contingency table counts and statistics in the AsciiTable
   // object.
   //
   for(i=0; i<n_cts; i++) {

      // Write out the header columns
      write_header_columns(engine, cts_at, i+1);

      // Field
      cts_at.set_entry(i+1, mode_field_offset, cts[i].name());

      // Total
      cts_at.set_entry(i+1, mode_total_offset, cts[i].n());

      // FY_OY
      cts_at.set_entry(i+1, mode_fy_oy_offset, cts[i].fy_oy());

      // FY_ON
      cts_at.set_entry(i+1, mode_fy_on_offset, cts[i].fy_on());

      // FN_OY
      cts_at.set_entry(i+1, mode_fn_oy_offset, cts[i].fn_oy());

      // FN_ON
      cts_at.set_entry(i+1, mode_fn_on_offset, cts[i].fn_on());

      // Base Rate
      v = cts[i].oy_tp();
      cts_at.set_entry(i+1, mode_baser_offset, v);

      // Forecast Mean
      v = cts[i].fy_tp();
      cts_at.set_entry(i+1, mode_fmean_offset, v);

      // Accuracy
      v = cts[i].accuracy();
      cts_at.set_entry(i+1, mode_acc_offset, v);

      // Forecast Bias
      v = cts[i].fbias();
      cts_at.set_entry(i+1, mode_fbias_offset, v);

      // PODY
      v = cts[i].pod_yes();
      cts_at.set_entry(i+1, mode_pody_offset, v);

      // PODN
      v = cts[i].pod_no();
      cts_at.set_entry(i+1, mode_podn_offset, v);

      // POFD
      v = cts[i].pofd();
      cts_at.set_entry(i+1, mode_pofd_offset, v);

      // FAR
      v = cts[i].far();
      cts_at.set_entry(i+1, mode_far_offset, v);

      // CSI
      v = cts[i].csi();
      cts_at.set_entry(i+1, mode_csi_offset, v);

      // GSS
      v = cts[i].gss();
      cts_at.set_entry(i+1, mode_gss_offset, v);

      // HK
      v = cts[i].hk();
      cts_at.set_entry(i+1, mode_hk_offset, v);

      // HSS
      v = cts[i].hss();
      cts_at.set_entry(i+1, mode_hss_offset, v);

      // ODDS
      v = cts[i].odds();
      cts_at.set_entry(i+1, mode_odds_offset, v);
   }

   //
   // Write the AsciiTable object to the output file
   //
   out << cts_at;

   out.close();

   return;
}

///////////////////////////////////////////////////////////////////////

void set_dim(Box &dim, double y_ll, double y_ur, double x_cen)

{

double L, R, B, T;
double mag, W;

B = y_ll;
T = y_ur;

mag = (T - B)/(xy_bb.height());

W = mag*xy_bb.width();

L = x_cen - 0.5*W;
R = x_cen + 0.5*W;

dim.set_lrbt(L, R, B, T);

return;

}

///////////////////////////////////////////////////////////////////////

void set_xy_bb() {
   Box fcst_xy_bb, obs_xy_bb;
   double L, R, B, T;

   //
   // Check the value of the plot_valid_flag in the config file.
   // If not set, reset the xy_bb to the entire grid.
   //
   if(engine.wconf.plot_valid_flag().ival() == 0) {

     xy_bb.set_llwh(0.0, 0.0, grid.nx(), grid.ny());
   }
   //
   // Search the raw forecast and raw observation fields to define a
   // bounding box around the regions of valid data
   //
   else {

      //
      // Compute the x/y bounding box for valid data in each field
      //
      valid_xy_bb(engine.fcst_raw, fcst_xy_bb);
      valid_xy_bb(engine.obs_raw, obs_xy_bb);

      //
      // Compute the x/y bounding box as the union of the
      // fcst and obs x/y bounding boxes
      //
      L = min(fcst_xy_bb.left(), obs_xy_bb.left());
      B = min(fcst_xy_bb.bottom(), obs_xy_bb.bottom());
      R = max(fcst_xy_bb.right(), obs_xy_bb.right());
      T = max(fcst_xy_bb.top(), obs_xy_bb.top());

      xy_bb.set_lrbt(L, R, B, T);

   }

   return;
}

///////////////////////////////////////////////////////////////////////
//
// Compute the smallest rectangle around the region of valid data in
// the field.
//
///////////////////////////////////////////////////////////////////////

void valid_xy_bb(const ShapeData *wd_ptr, Box &bb) {
   int x, y;
   int L, R, B, T;

   //
   // Initialize the x/y bounding box
   //

   L = grid.nx();
   B = grid.ny();
   R = 0;
   T = 0;

   for(x=0; x<wd_ptr->data.nx(); x++) {
      for(y=0; y<wd_ptr->data.ny(); y++) {

         if(wd_ptr->is_valid_xy(x, y)) {

            if(x < L) L = x;
            if(x > R) R = x;
            if(y < B) B = y;
            if(y > T) T = y;
         }
      } // end for y
   } // end for x

   bb.set_lrbt(L, R, B, T);

   return;
}

///////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tfcst_file\n"
        << "\tobs_file\n"
        << "\tconfig_file\n"
        << "\t[-config_merge merge_config_file]\n"
        << "\t[-fcst_valid time]\n"
        << "\t[-fcst_lead time]\n"
        << "\t[-obs_valid time]\n"
        << "\t[-obs_lead time]\n"
        << "\t[-outdir path]\n"
        << "\t[-plot]\n"
        << "\t[-obj_plot]\n"
        << "\t[-obj_stat]\n"
        << "\t[-ct_stat]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"fcst_file\" is a forecast file in either GRIB "
        << "or netCDF format (output of pcp_combine) containing the "
        << "field to be verified (required).\n"

        << "\t\t\"obs_file\" is an observation file in either GRIB "
        << "or netCDF format (output of pcp_combine) containing the "
        << "verifying field (required).\n"

        << "\t\t\"config_file\" is a WrfModeConfig file containing "
        << "the desired configuration settings (required).\n"

        << "\t\t\"-config_merge merge_config_file\" overrides the default "
        << "fuzzy engine settings for merging within the fcst/obs fields "
        << "(optional).\n"

        << "\t\t\"-fcst_valid time\" in YYYYMMDD[_HH[MMSS]] format "
        << "sets the forecast valid time to be verified (optional).\n"

        << "\t\t\"-fcst_lead time\" in HH[MMSS] format sets "
        << "the forecast lead time to be verified (optional).\n"

        << "\t\t\"-obs_valid time\" in YYYYMMDD[_HH[MMSS]] format "
        << "sets the observation valid time to be used (optional).\n"

        << "\t\t\"-obs_lead time\" in HH[MMSS] format sets "
        << "the observation lead time to be used (optional).\n"

        << "\t\t\"-outdir path\" overrides the default output directory ("
        << out_dir << ") (optional).\n"

        << "\t\t\"-plot\" disables plotting (optional).\n"

        << "\t\t\"-obj_plot\" disables the output of the object split "
        << "and cluster fields to a NetCDF file (optional).\n"

        << "\t\t\"-obj_stat\" disables the output of the object "
        << "statistics file (optional).\n"

        << "\t\t\"-ct_stat\" disables the output of the contingency "
        << "table standard statistics file (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\n\tNOTE: The forecast and observation fields must be "
        << "on the same grid.\n\n";

   exit (1);
}

///////////////////////////////////////////////////////////////////////

void set_config_merge_file(const StringArray & a)
{
   merge_config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_fcst_valid_time(const StringArray & a)
{
   fcst_valid_ut = timestring_to_unix(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_fcst_lead_time(const StringArray & a)
{
   fcst_lead_sec = timestring_to_sec(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_obs_valid_time(const StringArray & a)
{
   obs_valid_ut = timestring_to_unix(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_obs_lead_time(const StringArray & a)
{
   obs_lead_sec = timestring_to_sec(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_outdir(const StringArray & a)
{
   out_dir = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_plot(const StringArray &)
{
   plot_flag = 0;
}

////////////////////////////////////////////////////////////////////////

void set_obj_plot(const StringArray &)
{
   obj_plot_flag = 0;
}

////////////////////////////////////////////////////////////////////////

void set_obj_stat(const StringArray &)
{
   obj_stat_flag = 0;
}

////////////////////////////////////////////////////////////////////////

void set_ct_stat(const StringArray &)
{
   ct_stat_flag = 0;
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
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////

