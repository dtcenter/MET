// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////
//
//   Filename:   wavelet_stat.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    04/02/08  Halley Gotway   New
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
#include "vx_grib_classes/grib_classes.h"

#include "vx_wrfdata/vx_wrfdata.h"
#include "vx_met_util/vx_met_util.h"
#include "vx_data_grids/grid.h"
#include "vx_util/vx_util.h"
#include "vx_cal/vx_cal.h"
#include "vx_math/vx_math.h"
#include "vx_contable/vx_contable.h"
#include "vx_gsl_prob/vx_gsl_prob.h"
#include "vx_econfig/result.h"

#include "vx_color/vx_color.h"
#include "vx_ps/vx_ps.h"
#include "vx_pxm/vx_pxm.h"
#include "vx_render/vx_render.h"
#include "vx_plot_util/vx_plot_util.h"

#include "wavelet_stat.h"

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_fcst_obs_files();
static void process_scores();
static void clean_up();

static void setup_first_pass(const WrfData &, const Grid &);
static void setup_txt_files (unixtime, int);
static void setup_table     (AsciiTable &);
static void setup_nc_file   (unixtime, int);
static void setup_ps_file   (unixtime, int);

static void build_outfile_name(unixtime, int, const char *,
                               ConcatString &);
static double get_fill_value(const WrfData &, int);
static void fill_bad_data(WrfData &, double);
static void pad_field(WrfData &, double);
static void get_tile(const WrfData &, const WrfData &, int, int,
                     NumArray &, NumArray &);
static int  get_tile_tot_count();

static void do_intensity_scale(const NumArray &, const NumArray &,
                               ISCInfo *&, int, int);

static void aggregate_isc_info(ISCInfo **, int, int, ISCInfo &);

static void compute_cts(const double *, const double *, int, ISCInfo &);
static void compute_mse(const double *, const double *, int, double &);
static void compute_energy(const double *, int, double &);

static void write_nc_raw(const double *, const double *,
                         int, int, int);
static void write_nc_wav(const double *, const double *,
                         int, int, int, int, SingleThresh &);

static void close_in_files();
static void close_out_files();

static double sum_array(double *, int);
static double mean_array(double *, int);

static void plot_ps_raw(const WrfData &, const WrfData &,
                        const WrfData &, const WrfData &, int);
static void plot_ps_wvlt(const double *, int, int, int, ISCInfo &,
                         int, int);
static double compute_percentage(double, double);

static void set_plot_dims(int, int);
static void set_xy_bb();
static void set_ll_bb();
static void check_xy_ll(int, int);
static void set_dim(BoundingBox &, double, double, double);
static void draw_colorbar(PSfile *, BoundingBox &, int);
static void draw_border(PSfile *, BoundingBox &);
static void draw_map(PSfile *, BoundingBox &);
static void draw_tiles(PSfile *, BoundingBox &, int, int, int);
static void render_image(PSfile *, const WrfData &, BoundingBox &);
static void render_tile(PSfile *, const double *, int, int, BoundingBox &);

static void usage(int, char **);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the forecast and observation input files
   process_fcst_obs_files();

   // Compute the scores and write them out
   process_scores();

   // Close the text files and deallocate memory
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   int i;
   char tmp_str[PATH_MAX];

   // Set the default output directory
   replace_string(met_base_str, MET_BASE, default_out_dir, tmp_str);
   out_dir << tmp_str;

   if(argc < 4) {
      usage(argc, argv);
      exit(1);
   }

   // Store the input forecast and observation file names
   fcst_file   = argv[1];
   obs_file    = argv[2];
   config_file = argv[3];

   // Parse command line arguments
   for(i=0; i<argc; i++) {

      if(strcmp(argv[i], "-outdir") == 0) {
         out_dir = argv[i+1];
         i++;
      }
      else if(strcmp(argv[i], "-v") == 0) {
         verbosity = atoi(argv[i+1]);
         i++;
      }
      else if(strcmp(argv[i], "-ps") == 0) {
         ps_flag = 0;
         i++;
      }
      else if(strcmp(argv[i], "-nc") == 0) {
         nc_flag = 0;
         i++;
      }
      else if(argv[i][0] == '-') {
         cerr << "\n\nERROR: process_command_line() -> "
              << "unrecognized command line switch: "
              << argv[i] << "\n\n" << flush;
         exit(1);
      }
   }

   // Read the config file
   conf_info.read_config(config_file);

   // Set the MET data directory
   replace_string(met_base_str, MET_BASE, conf_info.conf.met_data_dir().sval(),
                  tmp_str);
   met_data_dir = tmp_str;

   // Set the model name
   shc.set_model(conf_info.conf.model().sval());

   // List the input files
   if(verbosity > 0) {
      cout << "Forecast File: " << fcst_file << "\n"
           << "Observation File: " << obs_file << "\n"
           << "Configuration File: " << config_file << "\n" << flush;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_fcst_obs_files() {

   // Switch based on the forecast file type
   fcst_ftype = get_file_type(fcst_file);

   switch(fcst_ftype) {

      // GRIB file type
      case(GbFileType):

         // Open the GRIB file
         if(!(fcst_gb_file.open(fcst_file))) {
            cerr << "\n\nERROR: process_fcst_obs_files() -> "
                 << "can't open GRIB forecast file: "
                 << fcst_file << "\n\n" << flush;
            exit(1);
         }
         break;

      // NetCDF file type
      case(NcFileType):

         // Open the NetCDF File
         fcst_nc_file = new NcFile(fcst_file);
         if(!fcst_nc_file->is_valid()) {
            cerr << "\n\nERROR: process_fcst_obs_files() -> "
                 << "can't open NetCDF forecast file: "
                 << fcst_file << "\n\n" << flush;
            exit(1);
         }
         break;

      default:
         cerr << "\n\nERROR: process_fcst_obs_files() -> "
              << "unsupport forecast file type: "
              << fcst_file << "\n\n" << flush;
         exit(1);
         break;
   }

   // Switch based on the observation file type
   obs_ftype = get_file_type(obs_file);

   switch(obs_ftype) {

      // GRIB file type
      case(GbFileType):

         // Open the GRIB file
         if(!(obs_gb_file.open(obs_file))) {
            cerr << "\n\nERROR: process_fcst_obs_files() -> "
                 << "can't open GRIB observation file: "
                 << obs_file << "\n\n" << flush;
            exit(1);
         }
         break;

      // NetCDF file type
      case(NcFileType):

         // Open the NetCDF File
         obs_nc_file = new NcFile(obs_file);
         if(!obs_nc_file->is_valid()) {
            cerr << "\n\nERROR: process_fcst_obs_files() -> "
                 << "can't open NetCDF observation file: "
                 << obs_file << "\n\n" << flush;
            exit(1);
         }
         break;

      default:
         cerr << "\n\nERROR: process_fcst_obs_files() -> "
              << "unsupported observation file type: "
              << obs_file << "\n\n" << flush;
         exit(1);
         break;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_scores() {
   int i, j, k, status;
   double fcst_fill, obs_fill;
   WrfData fcst_wd, obs_wd;
   WrfData fcst_wd_fill, obs_wd_fill;
   GribRecord fcst_r, obs_r;
   char tmp_str[max_str_len];
   NumArray f_na, o_na;
   ISCInfo **isc_info, isc_aggr;
   Grid fcst_grid, obs_grid;

   // Loop through each of the fields to be verified
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Continue based on the forecast file type
      if(fcst_ftype == GbFileType) {

         status = get_grib_record(fcst_gb_file, fcst_r,
                                  conf_info.gci[i],
                                  fcst_wd, fcst_grid, verbosity);

         if(status != 0) {
            cout << "***WARNING***: process_scores() -> "
                 << conf_info.gci[i].info_str
                 << " not found in GRIB file: " << fcst_file
                 << "\n" << flush;
            continue;
         }
      }
      // fcst_ftype == NcFileType
      else {

         read_netcdf(fcst_nc_file,
                     conf_info.gci[i].abbr_str.text(),
                     tmp_str,
                     fcst_wd, fcst_grid, verbosity);
      }

      // Set the forecast lead and valid times
      shc.set_fcst_lead_sec(fcst_wd.get_lead_time());
      shc.set_fcst_valid_beg(fcst_wd.get_valid_time());
      shc.set_fcst_valid_end(fcst_wd.get_valid_time());

      // Continue based on the forecast file type
      if(obs_ftype == GbFileType) {

         status = get_grib_record(obs_gb_file, obs_r,
                                  conf_info.gci[i],
                                  obs_wd, obs_grid, verbosity);

         if(status != 0) {
            cout << "***WARNING***: process_scores() -> "
                 << conf_info.gci[i].info_str
                 << " not found in GRIB file: " << obs_file
                 << "\n" << flush;
            continue;
         }
      }
      // obs_ftype == NcFileType
      else {

         read_netcdf(obs_nc_file,
                     conf_info.gci[i].abbr_str.text(),
                     tmp_str,
                     obs_wd, obs_grid, verbosity);
      }

      // Set the observation lead and valid times
      shc.set_obs_lead_sec(obs_wd.get_lead_time());
      shc.set_obs_valid_beg(obs_wd.get_valid_time());
      shc.set_obs_valid_end(obs_wd.get_valid_time());

      // Check that the grid dimensions match
      if(fcst_grid.nx() != obs_grid.nx() ||
         fcst_grid.ny() != obs_grid.ny()) {
         cerr << "\n\nERROR: process_scores() -> "
              << "Forecast and observation grid dimensions "
              << "do not match (" << fcst_grid.nx() << ", "
              << fcst_grid.ny() << ") != (" << obs_grid.nx()
              << ", " << obs_grid.ny() << ") for field " << i+1
              << ".\n\n" << flush;
         exit(1);
      }

      // Check that the valid times match
      if(fcst_wd.get_valid_time() != obs_wd.get_valid_time()) {
         cout << "***WARNING***: process_scores() -> "
              << "Forecast and observation valid times do not match "
              << fcst_wd.get_valid_time() << " != "
              << obs_wd.get_valid_time() << " for field " << i+1
              << ".\n\n" << flush;
      }

      // Check that the accumulation intervals match
      if(fcst_wd.get_accum_time() != obs_wd.get_accum_time()) {
         cout << "***WARNING***: process_scores() -> "
              << "Forecast and observation accumulation times "
              << "do not match " << fcst_wd.get_accum_time()
              << " != " << obs_wd.get_accum_time() << " for field "
              << i+1 << ".\n\n" << flush;
      }

      // This is the first pass through the loop and grid is unset
      if(grid.nx() == 0 && grid.ny() == 0) {

         // Setup the first pass through the data
         setup_first_pass(fcst_wd, fcst_grid);
      }
      else {

         // For multiple verification fields, check to make sure that
         // the grid dimensions don't change
         if(fcst_grid.nx() != grid.nx() ||
            fcst_grid.ny() != grid.ny() ||
            obs_grid.nx()  != grid.nx() ||
            obs_grid.ny()  != grid.ny()) {
            cerr << "\n\nERROR: process_scores() -> "
                 << "The grid dimensions must remain constant ("
                 << grid.nx() << ", " << grid.ny() << ") != ("
                 << fcst_grid.nx() << ", " << fcst_grid.ny()
                 << ") or ("
                 << obs_grid.nx() << ", " << obs_grid.ny() << ")\n\n"
                 << flush;
            exit(1);
         }
      }

      // Set the message type
      if(conf_info.gci[i].code == apcp_grib_code) {
         shc.set_msg_typ("MC_PCP");
      }
      else {
         shc.set_msg_typ("ANALYS");
      }

      // Store the forecast variable and level names
      shc.set_fcst_var(conf_info.gci[i].abbr_str.text());
      shc.set_fcst_lev(conf_info.gci[i].lvl_str.text());

      // Store the observation variable and level names
      shc.set_obs_var(conf_info.gci[i].abbr_str.text());
      shc.set_obs_lev(conf_info.gci[i].lvl_str.text());

      if(verbosity > 1) {
         cout << "\n----------------------------------------\n\n"
              << "Processing field " << conf_info.gci[i].info_str << ".\n"
              << flush;
      }

      // Mask out the missing data in one field with the other
      // if requested
      if(conf_info.conf.mask_missing_flag().ival() == 1 ||
         conf_info.conf.mask_missing_flag().ival() == 3)
         mask_bad_data(fcst_wd, obs_wd);

      if(conf_info.conf.mask_missing_flag().ival() == 2 ||
         conf_info.conf.mask_missing_flag().ival() == 3)
         mask_bad_data(obs_wd, fcst_wd);

      // Get the fill data value to be used for each field
      fcst_fill = get_fill_value(fcst_wd, i);
      obs_fill  = get_fill_value(obs_wd, i);

      // Initialize the fill fields
      fcst_wd_fill = fcst_wd;
      obs_wd_fill  = obs_wd;

      // Replace any bad data in the fields with a fill value
      if(verbosity > 1) cout << "Forecast field: ";
      fill_bad_data(fcst_wd_fill, fcst_fill);

      if(verbosity > 1) cout << "Observation field: ";
      fill_bad_data(obs_wd_fill,  obs_fill);

      // Pad the fields out to the nearest power of two if requsted
      if(conf_info.conf.grid_decomp_flag().ival() == 2) {
         if(verbosity > 1) {
            cout << "Padding the fields out to the nearest integer "
                 << "power of two.\n"
                 << flush;
         }

         pad_field(fcst_wd_fill, fcst_fill);
         pad_field(obs_wd_fill,  obs_fill);
      }

      // Write out the raw fields to PostScript
      if(ps_flag) plot_ps_raw(fcst_wd, obs_wd, fcst_wd_fill,
                              obs_wd_fill, i);

      // Allocate memory for ISCInfo objects sized as [n_tile][n_thresh]
      isc_info = new ISCInfo * [conf_info.get_n_tile()];
      for(j=0; j<conf_info.get_n_tile(); j++) {
         isc_info[j] = new ISCInfo [conf_info.ta[i].n_elements()];
      }

      // Loop through the tiles to be applied
      for(j=0; j<conf_info.get_n_tile(); j++) {

         // Set the mask name
         if(conf_info.get_n_tile() > 1) sprintf(tmp_str, "TILE%i", j+1);
         else                           strcpy(tmp_str, "TILE_TOT");
         shc.set_mask(tmp_str);

         // Apply the current tile to the fields
         get_tile(fcst_wd_fill, obs_wd_fill, i, j, f_na, o_na);

         // Compute Intensity-Scale scores
         if(conf_info.conf.output_flag(i_isc).ival()) {

            // Do the intensity-scale decomposition
            do_intensity_scale(f_na, o_na, isc_info[j], i, j);

            // Write out the ISC statistics
            if(conf_info.conf.output_flag(i_isc).ival()) {

               for(k=0; k<conf_info.ta[i].n_elements(); k++) {

                  // Store the tile definition parameters
                  isc_info[j][k].tile_dim = conf_info.get_tile_dim();
                  isc_info[j][k].tile_xll = nint(conf_info.tile_xll[j]);
                  isc_info[j][k].tile_yll = nint(conf_info.tile_xll[j]);

                  // Set the forecast and observation thresholds
                  shc.set_fcst_thresh(conf_info.ta[i][k]);
                  shc.set_obs_thresh(conf_info.ta[i][k]);

                  write_isc_row(shc, isc_info[j][k],
                     conf_info.conf.output_flag(i_isc).ival(),
                     stat_at, i_stat_row,
                     isc_at, i_isc_row);
               }
            } // end write ISC
         } // end if
      } // end for j

      // Aggregate the scores across tiles
      if(conf_info.get_n_tile() > 1) {

         // Set the mask name
         shc.set_mask("TILE_TOT");

         for(j=0; j<conf_info.ta[i].n_elements(); j++) {

            // Set the forecast and observation thresholds
            shc.set_fcst_thresh(conf_info.ta[i][j]);
            shc.set_obs_thresh(conf_info.ta[i][j]);

            // Aggregate the tiles for the current threshold
            aggregate_isc_info(isc_info, i, j, isc_aggr);

            write_isc_row(shc, isc_aggr,
               conf_info.conf.output_flag(i_isc).ival(),
               stat_at, i_stat_row,
               isc_at, i_isc_row);
         }
      }

      // Deallocate memory for ISCInfo objects
      for(j=0; j<conf_info.get_n_tile(); j++) {
         if(isc_info[j]) {
            delete [] isc_info[j];
            isc_info[j] = (ISCInfo *) 0;
         }
      }
      if(isc_info) {
         delete [] isc_info;
         isc_info = (ISCInfo **) 0;
      }

   } // end for i

   if(verbosity > 1) {
      cout << "\n----------------------------------------\n\n" << flush;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {

   // Close the input files
   close_in_files();

   // Close the output files
   close_out_files();

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_first_pass(const WrfData &wd, const Grid &gr) {

   // Store the grid to be used through the verification
   grid = gr;

   // Setup the tiles to be used
   conf_info.process_tiles(grid, verbosity);

   // Create output text files as requested in the configuration file
   setup_txt_files(wd.get_valid_time(), wd.get_lead_time());

   // If requested, create a NetCDF file
   if(nc_flag) {
      setup_nc_file(wd.get_valid_time(), wd.get_lead_time());
   }

   // If requested, create a PostScript file
   if(ps_flag) {
      setup_ps_file(wd.get_valid_time(), wd.get_lead_time());
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_txt_files(unixtime valid_ut, int lead_sec) {
   ConcatString tmp_str;

   // Create output file names for the stat file and optional text files
   build_outfile_name(valid_ut, lead_sec, "", tmp_str);

   /////////////////////////////////////////////////////////////////////
   //
   // Setup the output STAT file
   //
   /////////////////////////////////////////////////////////////////////

   // Initialize file stream
   stat_out = (ofstream *) 0;

   // Build the file name
   stat_file << tmp_str << stat_file_ext;

   // Create the output STAT file
   open_txt_file(stat_out, stat_file, verbosity);

   // Setup the STAT AsciiTable
   stat_at.set_size(conf_info.n_stat_row() + 1,
                    max_stat_col + n_header_columns + 1);
   setup_table(stat_at);

   // Write the text header row
   write_header_row((const char **) 0, 0, 1, stat_at, 0, 0);

   // Initialize the row index to 1 to account for the header
   i_stat_row = 1;

   /////////////////////////////////////////////////////////////////////
   //
   // Setup the output ISC file
   //
   /////////////////////////////////////////////////////////////////////

   if(conf_info.conf.output_flag(i_isc).ival() >= flag_txt_out) {


      // Initialize file stream
      isc_out   = (ofstream *) 0;

      // Build the file name
      isc_file << tmp_str << "_" << isc_file_abbr << txt_file_ext;

      // Create the output STAT file
      open_txt_file(isc_out, isc_file, verbosity);

      // Setup the ISC AsciiTable
      isc_at.set_size(conf_info.n_isc_row() + 1,
                      max_stat_col + n_header_columns + 1);
      setup_table(isc_at);

      // Write the text header row
      write_header_row(isc_columns, n_isc_columns, 1,
                       isc_at, 0, 0);

      // Initialize the row index to 1 to account for the header
      i_isc_row = 1;

   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_table(AsciiTable &at) {

   // Left-justify all columns
   at.set_table_just(LeftJust);

   // Set the precision
   at.set_precision(default_precision);

   // Set the bad data value
   at.set_bad_data_value(bad_data_double);

   // Set the bad data string
   at.set_bad_data_str(na_str);

   // Don't write out trailing blank rows
   at.set_delete_trailing_blank_rows(1);

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_nc_file(unixtime valid_ut, int lead_sec) {
   int i, x, y;
   int yr, mon, day, hr, min, sec;
   char attribute_str[max_str_len], time_str[max_str_len];
   char hostname_str[max_str_len];
   unixtime ut;

   // Create output NetCDF file name
   build_outfile_name(valid_ut, lead_sec, ".nc", out_nc_file);

   // Create a new NetCDF file and open it
   nc_out = new NcFile(out_nc_file, NcFile::Replace);

   if(!nc_out || !nc_out->is_valid()) {
      cerr << "\n\nERROR: setup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_nc_file << "\n\n" << flush;
      exit(1);
   }

   ut = time(NULL);
   unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
   sprintf(time_str, "%.4i-%.2i-%.2i %.2i:%.2i:%.2i", yr, mon, day, hr, min, sec);

   gethostname(hostname_str, max_str_len);

   sprintf(attribute_str, "File %s generated %s UTC on host %s",
           out_nc_file.text(), time_str, hostname_str);
   nc_out->add_att("FileOrigins", attribute_str);
   nc_out->add_att("Difference", "Forecast Value - Observation Value");

   // Set the NetCDF dimensions
   x_dim     = (NcDim *) 0;
   x_dim     = nc_out->add_dim("x", conf_info.get_tile_dim());

   y_dim     = (NcDim *) 0;
   y_dim     = nc_out->add_dim("y", conf_info.get_tile_dim());

   scale_dim = (NcDim *) 0;
   scale_dim = nc_out->add_dim("scale", conf_info.get_n_scale()+2);

   tile_dim  = (NcDim *) 0;
   tile_dim  = nc_out->add_dim("tile", conf_info.get_n_tile());

   // Add the x_ll and y_ll variables
   NcVar *x_ll_var = (NcVar *) 0;
   NcVar *y_ll_var = (NcVar *) 0;

   x_ll_var = nc_out->add_var("x_ll", ncInt, tile_dim);
   y_ll_var = nc_out->add_var("y_ll", ncInt, tile_dim);

   for(i=0; i<conf_info.get_n_tile(); i++) {

      x = nint(conf_info.tile_xll[i]);
      y = nint(conf_info.tile_yll[i]);

      // Write the x_ll value
      if(!x_ll_var->set_cur(i) ||
         !x_ll_var->put(&x, 1)) {

         cerr << "\n\nERROR: setup_nc_file() -> "
              << "error with the x_ll-var->put"
              << "\n\n" << flush;
         exit(1);
      }

      // Write the y_ll value
      if(!y_ll_var->set_cur(i) ||
         !y_ll_var->put(&y, 1)) {

         cerr << "\n\nERROR: setup_nc_file() -> "
              << "error with the y_ll-var->put"
              << "\n\n" << flush;
         exit(1);
      }
   }

   // Add the projection information
   write_netcdf_proj(nc_out, grid);

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_ps_file(unixtime valid_ut, int lead_sec) {

   // Create output PostScript file name
   build_outfile_name(valid_ut, lead_sec, ".ps", out_ps_file);

   // Create a new PostScript file and open it
   ps_out = new PSfile;
   ps_out->open(out_ps_file);
   n_page = 1;

   if(!ps_out) {
      cerr << "\n\nERROR: setup_ps_file() -> "
           << "trouble opening output PostScript file "
           << out_ps_file << "\n\n" << flush;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void build_outfile_name(unixtime valid_ut, int lead_sec,
                        const char *suffix, ConcatString &str) {
   int mon, day, yr, hr, min, sec;
   int l_hr, l_min, l_sec;
   char tmp_str[max_str_len];

   //
   // Create output file name
   //

   // Initialize
   str.clear();

   // Append the output directory and program name
   str << out_dir.text() << "/" << program_name;

   // Append the output prefix, if defined
   if(strlen(conf_info.conf.output_prefix().sval()) > 0)
      str << "_" << conf_info.conf.output_prefix().sval();

   // Append the timing information
   sec_to_hms(lead_sec, l_hr, l_min, l_sec);
   unix_to_mdyhms(valid_ut, mon, day, yr, hr, min, sec);
   sprintf(tmp_str, "%.2i%.2i%.2iL_%.4i%.2i%.2i_%.2i%.2i%.2iV",
           l_hr, l_min, l_sec, yr, mon, day, hr, min, sec);
   str << "_" << tmp_str;

   // Append the suffix
   str << suffix;

   return;
}

////////////////////////////////////////////////////////////////////////

double get_fill_value(const WrfData &wd, int i_gc) {
   int x, y, count;
   double fill_val, sum;

   //
   // If verifying precipitation, fill bad data points with zero.
   // Otherwise, fill them with the mean of the valid data.
   //
   if(is_precip_code(conf_info.gci[i_gc].code)) {
      fill_val = 0.0;
   }
   else {

      count = 0;
      sum = 0.0;
      for(x=0; x<wd.get_nx(); x++) {
         for(y=0; y<wd.get_ny(); y++) {

            if(wd.is_bad_xy(x, y)) continue;

            sum += wd.get_xy_double(x, y);
            count++;
         } // end for y
      } // end for x

      if(count > 0) fill_val = sum/count;
      else          fill_val = 0.0;
   }

   return(fill_val);
}

////////////////////////////////////////////////////////////////////////

void fill_bad_data(WrfData &wd, double fill_val) {
   int x, y, count;

   //
   // Replace any bad data values with the fill value
   //
   count = 0;
   for(x=0; x<wd.get_nx(); x++) {
      for(y=0; y<wd.get_ny(); y++) {
         if(wd.is_bad_xy(x, y)) {
            wd.put_xy_double(fill_val, x, y);
            count++;
         }
      } // end for y
   } // end for x

   if(verbosity > 1) {
      if(count > 0) {
         cout << "Replaced " << count << " bad data values out of "
              << wd.get_nx()*wd.get_ny()
              << " points with fill value of "
              << fill_val << ".\n" << flush;
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void pad_field(WrfData &wd, double pad_val) {
   int x, y, in_x, in_y;
   WrfData wd_pad;

   // Set up the WrfData object
   wd_pad.set_size(conf_info.get_tile_dim(), conf_info.get_tile_dim());
   wd_pad.set_b(wd.get_b());
   wd_pad.set_m(wd.get_m());

   // Fill the WrfData object
   for(x=0; x<wd_pad.get_nx(); x++) {
      for(y=0; y<wd_pad.get_ny(); y++) {

         // If in the region of valid data
         if(x >= conf_info.pad_bb.x_ll && x < conf_info.pad_bb.x_ur &&
            y >= conf_info.pad_bb.y_ll && y < conf_info.pad_bb.y_ur) {

            in_x = nint(x - conf_info.pad_bb.x_ll);
            in_y = nint(y - conf_info.pad_bb.y_ll);
            wd_pad.put_xy_int(wd.get_xy_int(in_x, in_y), x, y);
         }
         // Else, in the pad
         else {
            wd_pad.put_xy_double(pad_val, x, y);
         }
      } // end for y
   } // end for x

   wd = wd_pad;

   return;
}

////////////////////////////////////////////////////////////////////////

void get_tile(const WrfData &fcst_wd, const WrfData &obs_wd,
              int i_gc, int i_tile,
              NumArray &f_na, NumArray &o_na) {
   int x, y, x_ll, y_ll, x_ur, y_ur;

   //
   // Initialize the NumArray objects
   //
   f_na.clear();
   o_na.clear();

   //
   // Check the bounds to make sure this is a valid mask
   //
   x_ll = nint(conf_info.tile_xll[i_tile]);
   y_ll = nint(conf_info.tile_yll[i_tile]);
   x_ur = x_ll + conf_info.get_tile_dim();
   y_ur = y_ll + conf_info.get_tile_dim();

   if(x_ll < 0 || x_ll > fcst_wd.get_nx() ||
      y_ll < 0 || y_ll > fcst_wd.get_ny() ||
      x_ur < 0 || x_ur > fcst_wd.get_nx() ||
      y_ur < 0 || y_ur > fcst_wd.get_ny()) {

      cerr << "\n\nERROR: get_tile() -> "
           << "invalid tile extends off the grid: "
           << "(x_ll, y_ll, dim) = (" << x_ll << ", " << y_ll << ", "
           << conf_info.get_tile_dim() << ") and (nx, ny) = ("
           << fcst_wd.get_nx() << ", " << fcst_wd.get_ny()
           << ")!\n\n" << flush;
      exit(1);
   }

   if(verbosity > 1) {
      cout << "Retrieving data for tile " << i_tile+1
           << " with dimension = " << conf_info.get_tile_dim()
           << " and lower-left (x, y) = ("
           << x_ll << ", " << y_ll << ")\n" << flush;
   }

   //
   // Store the pairs in NumArray objects
   //
   for(y=y_ll; y<y_ur; y++) {
      for(x=x_ll; x<x_ur; x++) {
         f_na.add(fcst_wd.get_xy_double(x, y));
         o_na.add(obs_wd.get_xy_double(x, y));
      } // end for x
   } // end for y

   return;
}

////////////////////////////////////////////////////////////////////////

int get_tile_tot_count() {
   int x, y, nx, ny, i, count;

   // Get grid dimensions
   nx = grid.nx();
   ny = grid.ny();

   count = 0;

   // Check each point in the grid
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         // Check if the point resides in a tile
         for(i=0; i<conf_info.get_n_tile(); i++) {

            // Check if the current point is inside the current tile
            if(x >= conf_info.tile_xll[i] && x < conf_info.tile_xll[i] + conf_info.get_tile_dim() &&
               y >= conf_info.tile_yll[i] && y < conf_info.tile_yll[i] + conf_info.get_tile_dim()) {
               count++;
               break;
            }
         } // end for i
      } // end for y
   } // end for x

   return(count);
}

////////////////////////////////////////////////////////////////////////

void do_intensity_scale(const NumArray &f_na, const NumArray &o_na,
                        ISCInfo *&isc_info, int i_gc, int i_tile) {
   double *f_dat, *o_dat; // Raw and thresholded binary fields
   double *f_dwt, *o_dwt; // Discrete wavelet transformations
   double *f_scl, *o_scl; // Binary field decomposed by scale
   double *diff;          // Difference field
   double mse, fen, oen;
   int n, ns, n_isc;
   int bnd, row, col;
   int i, j, k;
   char thresh_str[max_str_len];

   // Check the NumArray lengths
   n = f_na.n_elements();
   if(n != o_na.n_elements()) {
      cerr << "\n\nERROR: do_intensity_scale() -> "
           << "the forecast and observation arrays must have equal "
           << "length.\n\n" << flush;
      exit(1);
   }

   // Check that the number of points = tile_dim * tile_dim
   if(n != (conf_info.get_tile_dim() * conf_info.get_tile_dim())) {
      cerr << "\n\nERROR: process_scores() -> "
           << "the number of points (" << n << ") should equal the "
           << "tile dimension squared ("
           << conf_info.get_tile_dim() * conf_info.get_tile_dim()
           << ").\n\n" << flush;
      exit(1);
   }

   // Get the number of scales
   ns = conf_info.get_n_scale();

   // Set up the ISCInfo thresholds and n_scale
   n_isc = conf_info.ta[i_gc].n_elements();
   for(i=0; i<n_isc; i++) {
      isc_info[i].clear();
      isc_info[i].cts_thresh = conf_info.ta[i_gc][i];
      isc_info[i].allocate_n_scale(ns);
   }

   // Allocate space
   f_dat = new double [n];
   o_dat = new double [n];
   f_dwt = new double [n];
   o_dwt = new double [n];
   f_scl = new double [n];
   o_scl = new double [n];
   diff  = new double [n];

   // Initialize f_dat and o_dat
   for(i=0; i<n; i++) {
      f_dat[i] = f_na[i];
      o_dat[i] = o_na[i];
   } // end for j

   // Write out the raw fields to NetCDF
   if(nc_flag) write_nc_raw(f_dat, o_dat, n, i_gc, i_tile);

   // Apply each threshold
   for(i=0; i<conf_info.ta[i_gc].n_elements(); i++) {

      if(verbosity > 1) {
         isc_info[i].cts_thresh.get_abbr_str(thresh_str);

         cout << "Computing ISC for " << conf_info.gci[i_gc].info_str
              << ", raw threshold of " << thresh_str << ".\n"
              << flush;
      }

      // Apply the threshold to each point to create 0/1 mask fields
      for(j=0; j<n; j++) {
         f_dat[j] = isc_info[i].cts_thresh.check(f_na[j]);
         o_dat[j] = isc_info[i].cts_thresh.check(o_na[j]);
         diff[j]  = f_dat[j] - o_dat[j];
      } // end for j

      // Compute the contingency table for the binary fields
      compute_cts(f_dat, o_dat, n, isc_info[i]);

      // Compute the MSE for the binary fields
      compute_mse(f_dat, o_dat, n, isc_info[i].mse);

      // Compute the energy for the binary fields
      compute_energy(f_dat, n, isc_info[i].fen);
      compute_energy(o_dat, n, isc_info[i].oen);

      // Compute the ISC for the binary fields
      isc_info[i].compute_isc(-1);

      // Write the thresholded binary fields to NetCDF
      if(nc_flag) write_nc_wav(f_dat, o_dat, n, i_gc, i_tile, -1,
                               isc_info[i].cts_thresh);

      // Write the thresholded binary difference field to PostScript
      if(ps_flag) plot_ps_wvlt(diff, n, i_gc, i_tile,
                               isc_info[i], -1, ns);

      // Initialize the discrete wavelet transforms
      memcpy(f_dwt, f_dat, n*sizeof(double));
      memcpy(o_dwt, o_dat, n*sizeof(double));

      // Perform the discrete wavelet transforms
      wavelet2d_transform_forward(conf_info.wvlt_ptr, f_dwt,
                                  conf_info.get_tile_dim(),
                                  conf_info.get_tile_dim(),
                                  conf_info.get_tile_dim(),
                                  conf_info.wvlt_work_ptr);
      wavelet2d_transform_forward(conf_info.wvlt_ptr, o_dwt,
                                  conf_info.get_tile_dim(),
                                  conf_info.get_tile_dim(),
                                  conf_info.get_tile_dim(),
                                  conf_info.wvlt_work_ptr);

      // Construct the decomposed forecast and observation images
      // for each scale
      for(j=0; j<=ns; j++) {

         // Compute the bound for this scale
         bnd = nint(pow(2.0, ns-j));

         // Figure out which coefficients apply to this scale
         for(k=0; k<n; k++) {

            // Compute the row and column for the current point
            row = k/conf_info.get_tile_dim();
            col = k%conf_info.get_tile_dim();

            if((row <  bnd/2 && col < bnd/2) ||
                row >= bnd ||
                col >= bnd) {
               f_scl[k] = o_scl[k] = 0.0;
            }
            else {
               f_scl[k] = f_dwt[k];
               o_scl[k] = o_dwt[k];
            }
         }

         // Compute the inverse discrete wavelet transforms
         wavelet2d_transform_inverse(conf_info.wvlt_ptr, f_scl,
                                     conf_info.get_tile_dim(),
                                     conf_info.get_tile_dim(),
                                     conf_info.get_tile_dim(),
                                     conf_info.wvlt_work_ptr);
         wavelet2d_transform_inverse(conf_info.wvlt_ptr, o_scl,
                                     conf_info.get_tile_dim(),
                                     conf_info.get_tile_dim(),
                                     conf_info.get_tile_dim(),
                                     conf_info.wvlt_work_ptr);

         // Compute the MSE for the decomposed fields
         compute_mse(f_scl, o_scl, n, mse);
         isc_info[i].mse_scale[j] = mse;

         // Compute the energy for the decomposed fields
         compute_energy(f_scl, n, fen);
         compute_energy(o_scl, n, oen);

         isc_info[i].fen_scale[j] = fen;
         isc_info[i].oen_scale[j] = oen;

         // Compute the ISC for each scale
         isc_info[i].compute_isc(j);

         // Write the decomposed fields for this scale to NetCDF
         if(nc_flag) write_nc_wav(f_scl, o_scl, n, i_gc, i_tile, j,
                                  isc_info[i].cts_thresh);

         // Compute the difference field for this scale
         for(k=0; k<n; k++) diff[k] = f_scl[k] - o_scl[k];

         // Write the decomposed difference field for this scale to PostScript
         if(ps_flag) plot_ps_wvlt(diff, n, i_gc, i_tile,
                                  isc_info[i], j, ns);

      } // end for j

      // Dump out the scores
      if(verbosity > 2) {

         cout << "FBIAS[" << thresh_str << "]\t\t= "
              << isc_info[i].fbias << "\n" << flush;
         cout << "BASER[" << thresh_str << "]\t\t= "
              << isc_info[i].baser << "\n" << flush;
         cout << "MSE[" << thresh_str << "]\t\t= "
              << isc_info[i].mse << "\n" << flush;
         cout << "ISC[" << thresh_str << "]\t\t= "
              << isc_info[i].isc << "\n" << flush;
         cout << "FEN[" << thresh_str << "]\t\t= "
              << isc_info[i].fen << "\n" << flush;
         cout << "OEN[" << thresh_str << "]\t\t= "
              << isc_info[i].oen << "\n" << flush;

         for(j=0; j<=ns; j++) {
            cout << "SCALE_" << j+1 << "[" << thresh_str
                 << "] MSE, ISC, FEN, OEN = "
                 << isc_info[i].mse_scale[j] << ", "
                 << isc_info[i].isc_scale[j] << ", "
                 << isc_info[i].fen_scale[j] << ", "
                 << isc_info[i].oen_scale[j]
                 << "\n" << flush;
         }

         cout << "MSE_SUM[" << thresh_str << "]\t= "
              << sum_array(isc_info[i].mse_scale, isc_info[i].n_scale+1)
              << "\n" << flush;
         cout << "ISC_MEAN[" << thresh_str << "]\t= "
              << mean_array(isc_info[i].isc_scale, isc_info[i].n_scale+1)
              << "\n" << flush;
         cout << "FEN_SUM[" << thresh_str << "]\t= "
              << sum_array(isc_info[i].fen_scale, isc_info[i].n_scale+1)
              << "\n" << flush;
         cout << "OEN_SUM[" << thresh_str << "]\t= "
              << sum_array(isc_info[i].oen_scale, isc_info[i].n_scale+1)
              << "\n" << flush;
      } // end if
   } // end for i

   // Deallocate memory
   if(f_dat) { delete [] f_dat; f_dat = (double *) 0; }
   if(o_dat) { delete [] o_dat; o_dat = (double *) 0; }
   if(f_dwt) { delete [] f_dwt; f_dwt = (double *) 0; }
   if(o_dwt) { delete [] o_dwt; o_dwt = (double *) 0; }
   if(f_scl) { delete [] f_scl; f_scl = (double *) 0; }
   if(o_scl) { delete [] o_scl; o_scl = (double *) 0; }
   if(diff)  { delete [] diff;  diff  = (double *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void aggregate_isc_info(ISCInfo **isc_info, int i_gc, int i_thresh,
                        ISCInfo &isc_aggr) {
   int i, j;
   int fy_oy, fy_on, fn_oy, fn_on;
   char thresh_str[max_str_len];

   // Set up the aggregated ISCInfo object
   isc_aggr = isc_info[0][i_thresh];
   isc_aggr.zero_out();

   if(verbosity > 1) {
      isc_aggr.cts_thresh.get_abbr_str(thresh_str);

      cout << "Aggregating ISC for " << conf_info.gci[i_gc].info_str
           << ", raw threshold of " << thresh_str
           << ", using " << conf_info.get_n_tile() << " tiles.\n"
           << flush;
   }

   // Initialize the Contingency Table counts
   fy_oy = fy_on = fn_oy = fn_on = 0;

   // Sum up the other ISCInfo objects
   for(i=0; i<conf_info.get_n_tile(); i++) {

      // Increment the contingency table counts
      fy_oy += isc_info[i][i_thresh].cts.fy_oy();
      fy_on += isc_info[i][i_thresh].cts.fy_on();
      fn_oy += isc_info[i][i_thresh].cts.fn_oy();
      fn_on += isc_info[i][i_thresh].cts.fn_on();

      // Sum the MSE, FEN, and OEN values
      isc_aggr.mse += isc_info[i][i_thresh].mse;
      isc_aggr.fen += isc_info[i][i_thresh].fen;
      isc_aggr.oen += isc_info[i][i_thresh].oen;

      // Sum the MSE, FEN, and OEN values for each scale
      for(j=0; j<=isc_aggr.n_scale; j++) {
         isc_aggr.mse_scale[j] += isc_info[i][i_thresh].mse_scale[j];
         isc_aggr.fen_scale[j] += isc_info[i][i_thresh].fen_scale[j];
         isc_aggr.oen_scale[j] += isc_info[i][i_thresh].oen_scale[j];
      } // end for j
   } // end for i

   // Store the aggregated contingency table counts
   isc_aggr.cts.set_fy_oy(fy_oy);
   isc_aggr.cts.set_fy_on(fy_on);
   isc_aggr.cts.set_fn_oy(fn_oy);
   isc_aggr.cts.set_fn_on(fn_on);

   // Compute the means for MSE, FEN, and OEN
   isc_aggr.mse /= conf_info.get_n_tile();
   isc_aggr.fen /= conf_info.get_n_tile();
   isc_aggr.oen /= conf_info.get_n_tile();

   // Compute the means for MSE, FEN, and OEN for each scale
   for(i=0; i<=isc_aggr.n_scale; i++) {
      isc_aggr.mse_scale[i] /= conf_info.get_n_tile();
      isc_aggr.fen_scale[i] /= conf_info.get_n_tile();
      isc_aggr.oen_scale[i] /= conf_info.get_n_tile();
   }

   // Recompute the aggregated ISC scores
   isc_aggr.compute_isc();

   // Dump out the scores
   if(verbosity > 2) {

      cout << "FBIAS[" << thresh_str << "]\t\t= "
           << isc_aggr.fbias << "\n" << flush;
      cout << "BASER[" << thresh_str << "]\t\t= "
           << isc_aggr.baser << "\n" << flush;
      cout << "MSE[" << thresh_str << "]\t\t= "
           << isc_aggr.mse << "\n" << flush;
      cout << "ISC[" << thresh_str << "]\t\t= "
           << isc_aggr.isc << "\n" << flush;
      cout << "FEN[" << thresh_str << "]\t\t= "
           << isc_aggr.fen << "\n" << flush;
      cout << "OEN[" << thresh_str << "]\t\t= "
           << isc_aggr.oen << "\n" << flush;

      for(j=0; j<=isc_aggr.n_scale; j++) {
         cout << "SCALE_" << j+1 << "[" << thresh_str
              << "] MSE, ISC, FEN, OEN = "
              << isc_aggr.mse_scale[j] << ", "
              << isc_aggr.isc_scale[j] << ", "
              << isc_aggr.fen_scale[j] << ", "
              << isc_aggr.oen_scale[j]
              << "\n" << flush;
      }

      cout << "MSE_SUM[" << thresh_str << "]\t= "
           << sum_array(isc_aggr.mse_scale, isc_aggr.n_scale+1)
           << "\n" << flush;
      cout << "ISC_MEAN[" << thresh_str << "]\t= "
           << mean_array(isc_aggr.isc_scale, isc_aggr.n_scale+1)
           << "\n" << flush;
      cout << "FEN_SUM[" << thresh_str << "]\t= "
           << sum_array(isc_aggr.fen_scale, isc_aggr.n_scale+1)
           << "\n" << flush;
      cout << "OEN_SUM[" << thresh_str << "]\t= "
           << sum_array(isc_aggr.oen_scale, isc_aggr.n_scale+1)
           << "\n" << flush;
   } // end if

   // Reset the total ISCInfo count to the union - intersection of the
   // tiles
   isc_aggr.total = get_tile_tot_count();

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute contingency table counts for binary fields
//
////////////////////////////////////////////////////////////////////////

void compute_cts(const double *f_arr, const double *o_arr, int n,
                 ISCInfo &isc_info) {
   int i, f, o;

   // Increment the contingency table counts for each grid point
   for(i=0; i<n; i++) {

      if(is_bad_data(f_arr[i]) ||
         is_bad_data(o_arr[i])) continue;

      f = nint(f_arr[i]);
      o = nint(o_arr[i]);

      if(      f &&  o) isc_info.cts.inc_fy_oy();
      else if( f && !o) isc_info.cts.inc_fy_on();
      else if(!f &&  o) isc_info.cts.inc_fn_oy();
      else if(!f && !o) isc_info.cts.inc_fn_on();

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_mse(const double *f, const double *o,
                 int n, double &mse) {
   int i, count;
   double err, sum_sq;

   for(i=0, count=0, sum_sq=0.0; i<n; i++) {

      if(is_bad_data(f[i]) ||
         is_bad_data(o[i])) continue;

      err     = f[i] - o[i];
      sum_sq += err*err;

      count++;
   } // end for i

   if(count == 0) mse = bad_data_double;
   else           mse = sum_sq/count;

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_energy(const double *arr, int n, double &en) {
   int i, count;
   double sum_sq;

   for(i=0, count=0, sum_sq=0.0; i<n; i++) {

      if(is_bad_data(arr[i])) continue;

      sum_sq += arr[i]*arr[i];

      count++;
   } // end for i

   if(count == 0) en = bad_data_double;
   else           en = sum_sq/count;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nc_raw(const double *fdata, const double *odata, int n,
                  int i_gc, int i_tile) {
   int i, d;
   float *fcst_data = (float *) 0;
   float *obs_data  = (float *) 0;
   float *diff_data = (float *) 0;
   char fcst_var_name[max_str_len];
   char obs_var_name[max_str_len];
   char diff_var_name[max_str_len];
   char tmp_str[max_str_len];

   // Build the variable names
   sprintf(fcst_var_name, "FCST_%s_%s_RAW",
           conf_info.gci[i_gc].abbr_str.text(),
           conf_info.gci[i_gc].lvl_str.text());
   sprintf(obs_var_name, "OBS_%s_%s_RAW",
           conf_info.gci[i_gc].abbr_str.text(),
           conf_info.gci[i_gc].lvl_str.text());
   sprintf(diff_var_name, "DIFF_%s_%s_RAW",
           conf_info.gci[i_gc].abbr_str.text(),
           conf_info.gci[i_gc].lvl_str.text());

   // If this is the first tile, define new variables
   if(i_tile == 0) {

      // Define the forecast and difference variables
      fcst_var = nc_out->add_var(fcst_var_name, ncFloat,
                                 tile_dim, x_dim, y_dim);
      obs_var  = nc_out->add_var(obs_var_name,  ncFloat,
                                 tile_dim, x_dim, y_dim);
      diff_var = nc_out->add_var(diff_var_name, ncFloat,
                                 tile_dim, x_dim, y_dim);

      // Add variable attributes for the forecast field
      fcst_var->add_att("type", "Forecast");
      fcst_var->add_att("name", shc.get_fcst_var());
      get_grib_code_name(conf_info.gci[i_gc].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      fcst_var->add_att("long_name", tmp_str);
      fcst_var->add_att("level", shc.get_fcst_lev());
      get_grib_code_unit(conf_info.gci[i_gc].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      fcst_var->add_att("units", tmp_str);
      fcst_var->add_att("_FillValue", bad_data_float);

      // Add variable attributes for the observation field
      obs_var->add_att("type", "Observation");
      obs_var->add_att("name", shc.get_fcst_var());
      get_grib_code_name(conf_info.gci[i_gc].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      obs_var->add_att("long_name", tmp_str);
      obs_var->add_att("level", shc.get_obs_lev());
      get_grib_code_unit(conf_info.gci[i_gc].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      obs_var->add_att("units", tmp_str);
      obs_var->add_att("_FillValue", bad_data_float);

      // Add variable attributes for the difference field
      diff_var->add_att("type", "Difference (F-O)");
      diff_var->add_att("name", shc.get_fcst_var());
      get_grib_code_name(conf_info.gci[i_gc].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      diff_var->add_att("long_name", tmp_str);
      diff_var->add_att("level", shc.get_fcst_lev());
      get_grib_code_unit(conf_info.gci[i_gc].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      diff_var->add_att("units", tmp_str);
      diff_var->add_att("_FillValue", bad_data_float);
   }
   // Otherwise, retrieve the previously defined variables
   else {

      fcst_var = nc_out->get_var(fcst_var_name);
      obs_var  = nc_out->get_var(obs_var_name);
      diff_var = nc_out->get_var(diff_var_name);
   }

   // Allocate memory for the forecast, observation, and difference
   // fields
   fcst_data = new float [n];
   obs_data  = new float [n];
   diff_data = new float [n];

   // Store the forecast, observation, and difference fields
   for(i=0; i<n; i++) {

      // Set the forecast data
      if(is_bad_data(fdata[i])) fcst_data[i] = bad_data_float;
      else                      fcst_data[i] = (float) fdata[i];

      // Set the observation data
      if(is_bad_data(odata[i])) obs_data[i]  = bad_data_float;
      else                      obs_data[i]  = (float) odata[i];

      // Set the difference data
      if(is_bad_data(fdata[i]) ||
         is_bad_data(odata[i]))
         diff_data[i] = bad_data_float;
      else
         diff_data[i] = (float) (fdata[i] - odata[i]);
   } // end for i

   // Retrieve the tile dimension
   d = conf_info.get_tile_dim();

   // Write out the forecast field
   if(!fcst_var->set_cur(i_tile, 0, 0) ||
      !fcst_var->put(&fcst_data[0], 1, d, d)) {
      cerr << "\n\nERROR: write_nc_raw() -> "
           << "error with the fcst_var->put for field "
           << shc.get_fcst_var()
           << "\n\n" << flush;
      exit(1);
   }

   // Write out the observation field
   if(!obs_var->set_cur(i_tile, 0, 0) ||
      !obs_var->put(&obs_data[0], 1, d, d)) {
      cerr << "\n\nERROR: write_nc_raw() -> "
           << "error with the obs_var->put for field "
           << shc.get_obs_var()
           << "\n\n" << flush;
      exit(1);
   }

   // Write out the difference field
   if(!diff_var->set_cur(i_tile, 0, 0) ||
      !diff_var->put(&diff_data[0], 1, d, d)) {
      cerr << "\n\nERROR: write_nc_raw() -> "
           << "error with the diff_var->put for field "
           << shc.get_fcst_var()
           << "\n\n" << flush;
      exit(1);
   }

   // Deallocate and clean up
   if(fcst_data) { delete [] fcst_data; fcst_data = (float *) 0; }
   if(obs_data)  { delete [] obs_data;  obs_data  = (float *) 0; }
   if(diff_data) { delete [] diff_data; diff_data = (float *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nc_wav(const double *fdata, const double *odata, int n,
                  int i_gc, int i_tile, int i_scale,
                  SingleThresh &st) {
   int i, d;
   float *fcst_data = (float *) 0;
   float *obs_data  = (float *) 0;
   float *diff_data = (float *) 0;
   char fcst_var_name[max_str_len];
   char obs_var_name[max_str_len];
   char diff_var_name[max_str_len];
   char thresh_str[max_str_len];
   char tmp_str[max_str_len];

   // Get the string for the threshold applied
   st.get_abbr_str(thresh_str);

   // Build the variable names
   sprintf(fcst_var_name, "FCST_%s_%s_%s",
           conf_info.gci[i_gc].abbr_str.text(),
           conf_info.gci[i_gc].lvl_str.text(),
           thresh_str);
   sprintf(obs_var_name, "OBS_%s_%s_%s",
           conf_info.gci[i_gc].abbr_str.text(),
           conf_info.gci[i_gc].lvl_str.text(),
           thresh_str);
   sprintf(diff_var_name, "DIFF_%s_%s_%s",
           conf_info.gci[i_gc].abbr_str.text(),
           conf_info.gci[i_gc].lvl_str.text(),
           thresh_str);

   // If this is the binary field, define new variables
   if(i_tile == 0 && i_scale < 0) {

      // Define the forecast and difference variables
      fcst_var = nc_out->add_var(fcst_var_name, ncFloat,
                                 tile_dim, scale_dim, x_dim, y_dim);
      obs_var  = nc_out->add_var(obs_var_name,  ncFloat,
                                 tile_dim, scale_dim, x_dim, y_dim);
      diff_var = nc_out->add_var(diff_var_name, ncFloat,
                                 tile_dim, scale_dim, x_dim, y_dim);

      // Add variable attributes for the forecast field
      fcst_var->add_att("type", "Forecast");
      fcst_var->add_att("name", shc.get_fcst_var());
      get_grib_code_name(conf_info.gci[i_gc].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      fcst_var->add_att("long_name", tmp_str);
      fcst_var->add_att("level", shc.get_fcst_lev());
      fcst_var->add_att("threshold", thresh_str);
      fcst_var->add_att("scale_0", "binary");
      fcst_var->add_att("scale_n", "scale 2^(n-1)");
      fcst_var->add_att("_FillValue", bad_data_float);

      // Add variable attributes for the observation field
      obs_var->add_att("type", "Observation");
      obs_var->add_att("name", shc.get_fcst_var());
      get_grib_code_name(conf_info.gci[i_gc].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      obs_var->add_att("long_name", tmp_str);
      obs_var->add_att("level", shc.get_obs_lev());
      obs_var->add_att("threshold", thresh_str);
      obs_var->add_att("scale_0", "binary");
      obs_var->add_att("scale_n", "scale 2^(n-1)");
      obs_var->add_att("_FillValue", bad_data_float);

      // Add variable attributes for the difference field
      diff_var->add_att("type", "Difference (F-O)");
      diff_var->add_att("name", shc.get_fcst_var());
      get_grib_code_name(conf_info.gci[i_gc].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      diff_var->add_att("long_name", tmp_str);
      diff_var->add_att("level", shc.get_obs_lev());
      diff_var->add_att("threshold", thresh_str);
      diff_var->add_att("scale_0", "binary");
      diff_var->add_att("scale_n", "scale 2^(n-1)");
      diff_var->add_att("_FillValue", bad_data_float);
   }
   // Otherwise, retrieve the previously defined variables
   else {

      fcst_var = nc_out->get_var(fcst_var_name);
      obs_var  = nc_out->get_var(obs_var_name);
      diff_var = nc_out->get_var(diff_var_name);
   }

   // Allocate memory for the forecast, observation, and difference
   // fields
   fcst_data = new float [n];
   obs_data  = new float [n];
   diff_data = new float [n];

   // Store the forecast, observation, and difference fields
   for(i=0; i<n; i++) {

      // Set the forecast data
      if(is_bad_data(fdata[i])) fcst_data[i] = bad_data_float;
      else                      fcst_data[i] = (float) fdata[i];

      // Set the observation data
      if(is_bad_data(odata[i])) obs_data[i]  = bad_data_float;
      else                      obs_data[i]  = (float) odata[i];

      // Set the difference data
      if(is_bad_data(fdata[i]) ||
         is_bad_data(odata[i]))
         diff_data[i] = bad_data_float;
      else
         diff_data[i] = (float) (fdata[i] - odata[i]);
   } // end for i

   // Retrieve the tile dimensions
   d = conf_info.get_tile_dim();

   // Write out the forecast field
   if(!fcst_var->set_cur(i_tile, i_scale+1, 0, 0) ||
      !fcst_var->put(&fcst_data[0], 1, 1, d, d)) {
      cerr << "\n\nERROR: write_nc_wav() -> "
           << "error with the fcst_var->put for field "
           << shc.get_fcst_var()
           << "\n\n" << flush;
      exit(1);
   }

   // Write out the observation field
   if(!obs_var->set_cur(i_tile, i_scale+1, 0, 0) ||
      !obs_var->put(&obs_data[0], 1, 1, d, d)) {
      cerr << "\n\nERROR: write_nc_wav() -> "
           << "error with the obs_var->put for field "
           << shc.get_obs_var()
           << "\n\n" << flush;
      exit(1);
   }

   // Write out the difference field
   if(!diff_var->set_cur(i_tile, i_scale+1, 0, 0) ||
      !diff_var->put(&diff_data[0], 1, 1, d, d)) {
      cerr << "\n\nERROR: write_nc()_wav -> "
           << "error with the diff_var->put for field "
           << shc.get_fcst_var()
           << "\n\n" << flush;
      exit(1);
   }

   // Deallocate and clean up
   if(fcst_data) { delete [] fcst_data; fcst_data = (float *) 0; }
   if(obs_data)  { delete [] obs_data;  obs_data  = (float *) 0; }
   if(diff_data) { delete [] diff_data; diff_data = (float *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void close_in_files() {

   // Close the forecast file
   if     (fcst_ftype == NcFileType) fcst_nc_file->close();
   else if(fcst_ftype == GbFileType) fcst_gb_file.close();

   // Close the observation file
   if     (obs_ftype == NcFileType)  obs_nc_file->close();
   else if(obs_ftype == GbFileType)  obs_gb_file.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void close_out_files() {

   // Write out the contents of the STAT AsciiTable and
   // close the STAT output files
   if(stat_out) {
      *stat_out << stat_at;
      close_txt_file(stat_out, stat_file, verbosity);
   }

   // Write out the contents of the ISC AsciiTable and
   // close the ISC output files
   if(conf_info.conf.output_flag(i_isc).ival() >= flag_txt_out) {
      if(isc_out) {
         *isc_out << isc_at;
         close_txt_file(isc_out, isc_file, verbosity);
      }
   }

   // Close the output NetCDF file as long as it was opened
   if(nc_out && nc_flag) {

      // List the NetCDF file after it is finished
      if(verbosity > 0) {
         cout << "Output file: "
              << out_nc_file << "\n" << flush;
      }
      nc_out->close();
      delete nc_out;
      nc_out = (NcFile *) 0;
   }

   // Close the output PSfile as long as it was opened
   if(ps_out && ps_flag) {

      // List the PostScript file after it is finished
      if(verbosity > 0) {
         cout << "Output file: "
              << out_ps_file << "\n" << flush;
      }
      ps_out->close();
      delete ps_out;
      ps_out = (PSfile *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

double sum_array(double *d, int n) {
   int i;
   double sum;

   for(i=0, sum=0.0; i<n; i++) sum += d[i];

   return(sum);
}

////////////////////////////////////////////////////////////////////////

double mean_array(double *d, int n) {
   int i;
   double sum;

   for(i=0, sum=0.0; i<n; i++) sum += d[i];

   return(sum/n);
}

////////////////////////////////////////////////////////////////////////
//
// Write the first page of the PostScript plot
//
////////////////////////////////////////////////////////////////////////

void plot_ps_raw(const WrfData &fcst_wd,
                 const WrfData &obs_wd,
                 const WrfData &fcst_wd_fill,
                 const WrfData &obs_wd_fill,
                 int i_gc) {
   char label[max_str_len], tmp_str[PATH_MAX];
   char fcst_str[max_str_len], fcst_short_str[max_str_len];
   char obs_str[max_str_len], obs_short_str[max_str_len];
   double v_tab, h_tab_a, h_tab_b;
   int i, mon, day, yr, hr, minute, sec;
   BoundingBox dim;

   //
   // Compute the min and max data values across both raw fields for use
   // in setting up the color table
   //
   raw_plot_min = min(fcst_wd.int_to_double(0),
                      obs_wd.int_to_double(0));
   raw_plot_max = max(fcst_wd.int_to_double(wrfdata_int_data_max),
                      obs_wd.int_to_double(wrfdata_int_data_max));

   //
   // Setup the x/y bounding box for the data to be plotted.
   //
   set_xy_bb();

   //
   // Setup the lat/lon bounding box based on the x/y bounding box.
   //
   set_ll_bb();

   //
   // Setup the plotting dimensions based on the x/y bounding box
   //
   set_plot_dims(nint(xy_bb.width), nint(xy_bb.height));

   //
   // Load the raw color table
   //
   replace_string(met_base_str, MET_BASE,
                  conf_info.conf.raw_color_table().sval(), tmp_str);
   if(verbosity > 1) {
      cout << "Loading raw color table: "
           << tmp_str << "\n" << flush;
   }
   raw_ct.read(tmp_str);

   //
   // Load the wavelet color table
   //
   replace_string(met_base_str, MET_BASE,
                  conf_info.conf.wvlt_color_table().sval(), tmp_str);
   if(verbosity > 1) {
      cout << "Loading wavelet color table: "
           << tmp_str << "\n" << flush;
   }
   wvlt_ct.read(tmp_str);

   //
   // If the range of the colortable is [0, 1], rescale it to the
   // raw_plot_min and raw_plot_max values.
   //
   if(is_eq(raw_ct.data_min(bad_data_double), 0.0) &&
      is_eq(raw_ct.data_max(bad_data_double), 1.0)) {
      raw_ct.rescale(raw_plot_min, raw_plot_max, bad_data_double);
   }

   //
   // If the raw_plot_min or raw_plot_max value is set in the
   // configuration file, rescale the colortable to the requested range.
   //
   if(!is_eq(conf_info.conf.raw_plot_min().dval(), 0.0) ||
      !is_eq(conf_info.conf.raw_plot_max().dval(), 0.0)) {
      raw_ct.rescale(conf_info.conf.raw_plot_min().dval(),
                     conf_info.conf.raw_plot_max().dval(),
                     bad_data_double);
   }

   //
   // Set the fill color.  If a fill value is not specified in the range
   // of the color table, use the default color.  Otherwise, use the
   // color specified in the color table.
   //
   if(bad_data_double >= raw_ct.data_min() &&
      bad_data_double <= raw_ct.data_max())
      c_raw_fill = raw_ct.nearest(bad_data_double);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Plot the raw forecast and observation fields
   //
   ////////////////////////////////////////////////////////////////////////////

   ps_out->pagenumber(n_page);

   sprintf(tmp_str, "Wavelet-Stat: %s",
           conf_info.gci[i_gc].info_str.text());

   ps_out->choose_font(31, 24.0, met_data_dir);
   ps_out->write_centered_text(1, 1, h_tab_cen, 752.0, 0.5, 0.5,
                               tmp_str);

   strcpy(fcst_str, "Forecast");
   strcpy(fcst_short_str, "Fcst");
   strcpy(obs_str, "Observation");
   strcpy(obs_short_str, "Obs");

   ps_out->choose_font(31, 18.0, met_data_dir);
   ps_out->write_centered_text(1, 1, h_tab_1, 727.0, 0.5, 0.5,
                               fcst_str);
   ps_out->write_centered_text(1, 1, h_tab_3, 727.0, 0.5, 0.5,
                               obs_str);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Draw raw forecast field
   //
   ////////////////////////////////////////////////////////////////////////////

   set_dim(dim, v_tab_1, v_tab_1 + sm_plot_height, h_tab_1);
   render_image(ps_out, fcst_wd, dim);
   draw_map(ps_out, dim);
   draw_border(ps_out, dim);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Draw the colorbar
   //
   ////////////////////////////////////////////////////////////////////////////

   draw_colorbar(ps_out, dim, 1);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Draw raw observation field
   //
   ////////////////////////////////////////////////////////////////////////////

   set_dim(dim, v_tab_1, v_tab_1 + sm_plot_height, h_tab_3);
   render_image(ps_out, obs_wd, dim);
   draw_map(ps_out, dim);
   draw_border(ps_out, dim);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Draw raw forecast fill field with the tiles overlaid
   //
   ////////////////////////////////////////////////////////////////////////////

   set_dim(dim, v_tab_2, v_tab_2 + sm_plot_height, h_tab_1);
   render_image(ps_out, fcst_wd_fill, dim);
   draw_map(ps_out, dim);
   draw_border(ps_out, dim);
   draw_tiles(ps_out, dim, 0, conf_info.get_n_tile()-1, 1);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Draw the colorbar
   //
   ////////////////////////////////////////////////////////////////////////////

   draw_colorbar(ps_out, dim, 1);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Draw raw observation fill field with the tiles overlaid
   //
   ////////////////////////////////////////////////////////////////////////////

   set_dim(dim, v_tab_2, v_tab_2 + sm_plot_height, h_tab_3);
   render_image(ps_out, obs_wd_fill, dim);
   draw_map(ps_out, dim);
   draw_border(ps_out, dim);
   draw_tiles(ps_out, dim, 0, conf_info.get_n_tile()-1, 1);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Annotate the page
   //
   ////////////////////////////////////////////////////////////////////////////

   ps_out->choose_font(31, 12.0, met_data_dir);

   v_tab = v_tab_2 - 1.0*plot_text_sep;
   h_tab_a = h_tab_1 - 0.5*dim.width;
   h_tab_b = h_tab_a + 5.0*plot_text_sep;

   //
   // Model Name
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Model Name:");
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5,
                               conf_info.conf.model().sval());
   v_tab -= plot_text_sep;

   //
   // Blank line
   //
   v_tab -= plot_text_sep;

   //
   // Initialization Time
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Init Time:");
   unix_to_mdyhms(fcst_wd.get_valid_time() - fcst_wd.get_lead_time(),
                  mon, day, yr, hr, minute, sec);
   sprintf(label, "%s %i, %i %.2i:%.2i:%.2i",
           short_month_name[mon], day, yr, hr, minute, sec);
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label);
   v_tab -= plot_text_sep;

   //
   // Valid time
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Valid Time:");
   unix_to_mdyhms(fcst_wd.get_valid_time(),
                  mon, day, yr, hr, minute, sec);
   sprintf(label, "%s %i, %i %.2i:%.2i:%.2i",
           short_month_name[mon], day, yr, hr, minute, sec);
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label);
   v_tab -= plot_text_sep;

   //
   // Lead time
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Lead Time:");
   sec_to_hms(fcst_wd.get_lead_time(), hr, minute, sec);
   sprintf(label, "%.2i:%.2i:%.2i", hr, minute, sec);
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label);
   v_tab -= plot_text_sep;

   //
   // Accumulation time
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Accum Time:");
   sec_to_hms(fcst_wd.get_accum_time(), hr, minute, sec);
   sprintf(label, "%.2i:%.2i:%.2i", hr, minute, sec);
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label);
   v_tab -= plot_text_sep;

   //
   // Blank line
   //
   v_tab -= plot_text_sep;

   //
   // Tiling method
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Tile Method:");
   strcpy(label,
      grid_decomp_str[conf_info.conf.grid_decomp_flag().ival()]);
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label);
   v_tab -= plot_text_sep;

   //
   // Tile count
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Tile Count:");
   sprintf(label, "%i", conf_info.get_n_tile());
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label);
   v_tab -= plot_text_sep;

   //
   // Tile dimension
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Tile Dim:");
   sprintf(label, "%i x %i",
           conf_info.get_tile_dim(),
           conf_info.get_tile_dim());
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label);
   v_tab -= plot_text_sep;

   //
   // Tile corners
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Tile Corner:");
   strcpy(label, "");
   for(i=0; i<conf_info.get_n_tile(); i++) {
      sprintf(tmp_str, "(%i, %i) ",
              nint(conf_info.tile_xll[i]),
              nint(conf_info.tile_yll[i]));
      sprintf(label, "%s%s", label, tmp_str);
   }
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label);
   v_tab -= plot_text_sep;

   //
   // Blank line
   //
   v_tab -= plot_text_sep;

   //
   // Mask missing flag
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Mask Missing:");
   sprintf(label, "%s",
           mask_missing_str[conf_info.conf.mask_missing_flag().ival()]);
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label);
   v_tab -= plot_text_sep;

   //
   // Wavelet type
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Wavelet(k):");
   sprintf(label, "%s (%i)",
           wavelet_str[conf_info.conf.wavelet_flag().ival()],
           conf_info.conf.wavelet_k().ival());
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label);
   v_tab -= plot_text_sep;

   ps_out->showpage();
   n_page++;

   return;
}

////////////////////////////////////////////////////////////////////////

void plot_ps_wvlt(const double *diff, int n, int i_gc, int i_tile,
                  ISCInfo &isc_info,
                  int i_scale, int n_scale) {
   char tmp_str[max_str_len], thresh_str[max_str_len];
   char field_str[max_str_len];
   BoundingBox dim;
   double v_tab, h_tab_a, h_tab_b, h_tab_c, h_tab_d;
   double p;

   //
   // Draw 2 wavelet decompositions on each page.  Figure out if this
   // data should be plotted in the top or bottom panel.
   //
   if((i_scale+1)%2 == 0) { // Top panel
      v_tab = page_height - v_margin - 1.0*plot_text_sep;
      ps_out->pagenumber(n_page);
   }
   else {                  // Bottom panel
      v_tab = v_tab_cen;
   }

   //
   // The min and max plotting values should default to [-1.0, 1.0]
   // for the decomposed wavelet difference fields.
   //
   wvlt_ct.rescale(-1.0, 1.0, bad_data_double);

   //
   // If the wvlt_plot_min or wvlt_plot_max value is set in the
   // config file, rescale the colortable to the requested range.
   //
   if(!is_eq(conf_info.conf.wvlt_plot_min().dval(), 0.0) ||
      !is_eq(conf_info.conf.wvlt_plot_max().dval(), 0.0)) {
      wvlt_ct.rescale(conf_info.conf.wvlt_plot_min().dval(),
                      conf_info.conf.wvlt_plot_max().dval(),
                      bad_data_double);
   }

   //
   // Set the fill color.  If a fill value is not specified in the range
   // of the color table, use the default color.  Otherwise, use the
   // color specified in the color table.
   //
   if(bad_data_double >= wvlt_ct.data_min() &&
      bad_data_double <= wvlt_ct.data_max())
      c_wvlt_fill = wvlt_ct.nearest(bad_data_double);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Plot the labels
   //
   ////////////////////////////////////////////////////////////////////////////

   isc_info.cts_thresh.get_str(thresh_str, 2);

   if(i_scale == -1)
      sprintf(tmp_str, "Wavelet-Stat: %s, Tile %i, %s, Binary",
              conf_info.gci[i_gc].info_str.text(),
              i_tile+1, thresh_str);
   else
      sprintf(tmp_str, "Wavelet-Stat: %s, Tile %i, %s, Scale %i",
              conf_info.gci[i_gc].info_str.text(),
              i_tile+1, thresh_str, i_scale+1);

   ps_out->choose_font(31, 24.0, met_data_dir);
   v_tab -= 1.0*plot_text_sep;
   ps_out->write_centered_text(1, 1, h_tab_cen, v_tab, 0.5, 0.5,
                               tmp_str);

   strcpy(field_str, "Difference (F-0)");

   ps_out->choose_font(31, 18.0, met_data_dir);
   v_tab -= 2.0*plot_text_sep;
   ps_out->write_centered_text(1, 1, h_tab_cen, v_tab, 0.5, 0.5,
                               field_str);
   v_tab -= 1.0*plot_text_sep;

   ////////////////////////////////////////////////////////////////////////////
   //
   // Draw wavelet difference field
   //
   ////////////////////////////////////////////////////////////////////////////

   set_dim(dim, v_tab-lg_plot_height, v_tab, h_tab_cen);
   render_tile(ps_out, diff, n, i_tile, dim);
   draw_map(ps_out, dim);
   draw_border(ps_out, dim);
   draw_tiles(ps_out, dim, i_tile, i_tile, 0);
   draw_colorbar(ps_out, dim, 0);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Write the scores for current scale
   //
   ////////////////////////////////////////////////////////////////////////////

   ps_out->choose_font(31, 12.0, met_data_dir);

   v_tab -= lg_plot_height;
   v_tab -= plot_text_sep;

   h_tab_a = 2.0*plot_text_sep;
   h_tab_b = h_tab_a + 10.0*plot_text_sep;

   h_tab_c = h_tab_cen + 2.0*plot_text_sep;;
   h_tab_d = h_tab_c + 10.0*plot_text_sep;

   //
   // Frequency Bias
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Frequency Bias:");
   sprintf(tmp_str, "%.5f", isc_info.fbias);
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5,
                               tmp_str);
   v_tab -= plot_text_sep;

   //
   // Base Rate
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Base Rate:");
   sprintf(tmp_str, "%.5f", isc_info.baser);
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5,
                               tmp_str);
   v_tab -= plot_text_sep;

   //
   // Mean-Squared Error (percentage of total)
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Mean-Squared Error (%):");
   if(i_scale < 0) {
      p = compute_percentage(isc_info.mse, isc_info.mse);
      sprintf(tmp_str, "%.5f (%.2f)", isc_info.mse, p);
   }
   else {
      p = compute_percentage(isc_info.mse_scale[i_scale], isc_info.mse);
      sprintf(tmp_str, "%.5f (%.2f)", isc_info.mse_scale[i_scale], p);
   }
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5,
                               tmp_str);
   v_tab -= plot_text_sep;

   //
   // Intensity-Scale Skill Score
   //
   v_tab += 3.0*plot_text_sep;
   ps_out->write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5,
                               "Intensity Skill Score:");
   if(i_scale < 0) sprintf(tmp_str, "%.5f", isc_info.isc);
   else           sprintf(tmp_str, "%.5f", isc_info.isc_scale[i_scale]);
   ps_out->write_centered_text(1, 1, h_tab_d, v_tab, 0.0, 0.5,
                               tmp_str);
   v_tab -= plot_text_sep;

   //
   // Forecast Energy Squared (percentage of total)
   //
   ps_out->write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5,
                               "Fcst Energy Squared (%):");
   if(i_scale < 0) {
      p = compute_percentage(isc_info.fen, isc_info.fen);
      sprintf(tmp_str, "%.5f (%.2f)", isc_info.fen, p);
   }
   else {
      p = compute_percentage(isc_info.fen_scale[i_scale], isc_info.fen);
      sprintf(tmp_str, "%.5f (%.2f)", isc_info.fen_scale[i_scale], p);
   }
   ps_out->write_centered_text(1, 1, h_tab_d, v_tab, 0.0, 0.5,
                               tmp_str);
   v_tab -= plot_text_sep;

   //
   // Observation Energy Squared (percentage of total)
   //
   ps_out->write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5,
                               "Obs Energy Squared (%):");
   if(i_scale < 0) {
      p = compute_percentage(isc_info.oen, isc_info.oen);
      sprintf(tmp_str, "%.5f (%.2f)", isc_info.oen, p);
   }
   else {
      p = compute_percentage(isc_info.oen_scale[i_scale], isc_info.oen);
      sprintf(tmp_str, "%.5f (%.2f)", isc_info.oen_scale[i_scale], p);
   }
   ps_out->write_centered_text(1, 1, h_tab_d, v_tab, 0.0, 0.5,
                               tmp_str);
   v_tab -= plot_text_sep;

   //
   // If we just filled in the bottom panel or this is the last scale
   // to be plotted start a new page
   //
   if((i_scale+1)%2 == 1 || i_scale == n_scale) {
      ps_out->showpage();
      n_page++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

double compute_percentage(double num, double den) {
   double percentage;

   if(is_bad_data(num) || is_bad_data(den) || is_eq(den, 0.0)) {
      percentage = bad_data_double;
   }
   else {
      percentage = num/den*100.0;
   }

   return(percentage);
}

////////////////////////////////////////////////////////////////////////

void set_plot_dims(int nx, int ny) {
   double grid_ar, plot_ar, w;

   w = full_pane_bb.width;

   grid_ar = (double) nx/ny;
   plot_ar = ((w - clrbar_width)/2.0)/(full_pane_bb.height/3.0);

   if(grid_ar > plot_ar) {
      sm_plot_height = full_pane_bb.height/3.0*plot_ar/grid_ar;
      lg_plot_height = full_pane_bb.height/3.0*plot_ar/grid_ar;
   }
   else {
      sm_plot_height = full_pane_bb.height/3.0;
      lg_plot_height = full_pane_bb.height/3.0;
   }

   // First plot
   h_tab_1 = full_pane_bb.x_ll + (w - clrbar_width)/4.0;
   // Colorbar plot
   h_tab_2 = full_pane_bb.x_ll + (w - clrbar_width)/4.0
             + clrbar_width;
   // Second plot
   h_tab_3 = full_pane_bb.x_ll + (w - clrbar_width)/4.0*3.0
             + clrbar_width;

   v_tab_1 = full_pane_bb.y_ur - sm_plot_height;
   v_tab_2 = v_tab_1 - sm_plot_height;
   v_tab_3 = v_tab_2 - sm_plot_height;

   return;
}

////////////////////////////////////////////////////////////////////////

void set_xy_bb() {

   //
   // Check if padding was performed
   //
   if(!is_eq(conf_info.pad_bb.x_ll, 0.0) &&
      !is_eq(conf_info.pad_bb.y_ll, 0.0)) {

      xy_bb.x_ll   = 0 - conf_info.pad_bb.x_ll;
      xy_bb.y_ll   = 0 - conf_info.pad_bb.y_ll;
      xy_bb.x_ur   = xy_bb.x_ll + conf_info.get_tile_dim();
      xy_bb.y_ur   = xy_bb.y_ll + conf_info.get_tile_dim();
      xy_bb.width  = xy_bb.x_ur - xy_bb.x_ll;
      xy_bb.height = xy_bb.y_ur - xy_bb.y_ll;
   }
   else {
      xy_bb.x_ll   = 0;
      xy_bb.y_ll   = 0;
      xy_bb.x_ur   = grid.nx();
      xy_bb.y_ur   = grid.ny();
      xy_bb.width  = xy_bb.x_ur - xy_bb.x_ll;
      xy_bb.height = xy_bb.y_ur - xy_bb.y_ll;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void set_ll_bb() {
   int x, y;

   ll_bb.x_ll = -180.0; // Initialize the max lon
   ll_bb.y_ll = 90.0;   // Initialize the min lat
   ll_bb.x_ur = 180.0;  // Initialize the min lon
   ll_bb.y_ur = -90.0;  // Initialize the max lat

   // Search the bottom edge of the grid
   for(x=nint(xy_bb.x_ll); x<xy_bb.x_ur; x++)
      check_xy_ll(x, nint(xy_bb.y_ll));

   // Search the top edge of the grid
   for(x=nint(xy_bb.x_ll); x<xy_bb.x_ur; x++)
      check_xy_ll(x, nint(xy_bb.y_ur-1));

   // Search the left side of the grid
   for(y=nint(xy_bb.y_ll); y<xy_bb.y_ur; y++)
      check_xy_ll(nint(xy_bb.x_ll), y);

   // Search the right side of the grid
   for(y=nint(xy_bb.y_ll); y<xy_bb.y_ur; y++)
      check_xy_ll(nint(xy_bb.x_ur-1), y);

   return;
}

////////////////////////////////////////////////////////////////////////

void check_xy_ll(int x, int y) {
   double lat, lon;

   // Convert to lat/lon
   grid.xy_to_latlon(x, y, lat, lon);

   // Check limits
   if(lat > ll_bb.y_ur) ll_bb.y_ur = lat;
   if(lat < ll_bb.y_ll) ll_bb.y_ll = lat;
   if(lon > ll_bb.x_ll) ll_bb.x_ll = lon;
   if(lon < ll_bb.x_ur) ll_bb.x_ur = lon;

   return;
}

////////////////////////////////////////////////////////////////////////

void set_dim(BoundingBox &dim, double y_ll, double y_ur, double x_cen) {
   double mag;

   dim.y_ll   = y_ll;
   dim.y_ur   = y_ur;
   dim.height = dim.y_ur - dim.y_ll;
   mag        = dim.height/xy_bb.height;
   dim.width  = mag*xy_bb.width;
   dim.x_ll   = x_cen - 0.5*dim.width;
   dim.x_ur   = x_cen + 0.5*dim.width;

   return;
}

////////////////////////////////////////////////////////////////////////

void draw_colorbar(PSfile *p, BoundingBox &dim, int raw) {
   int i;
   char label[max_str_len];
   double bar_width, bar_height, x_ll, y_ll, step, v;
   ColorTable *ct_ptr;
   Color c;

   //
   // Set up the pointer to the appropriate colortable
   //
   if(raw == 1) ct_ptr = &raw_ct;
   else         ct_ptr = &wvlt_ct;

   //
   // Draw colorbar in the bottom-right corner of the Bounding Box
   //
   p->gsave();
   p->setlinewidth(l_width);
   p->choose_font(28, 8.0, met_data_dir);

   bar_width = h_margin;
   bar_height = (dim.y_ur - dim.y_ll)/(n_color_bars + 1);

   x_ll = dim.x_ur;
   y_ll = dim.y_ll;

   step = (ct_ptr->data_max(bad_data_double)
           - ct_ptr->data_min(bad_data_double))/n_color_bars;
   v = ct_ptr->data_min(bad_data_double);

   for(i=0; i<=n_color_bars; i++) {

     c = ct_ptr->nearest(v);

     //
     // Color box
     //
     p->setrgbcolor(c.red()/255.0, c.green()/255.0, c.blue()/255.0);
     p->newpath();
     p->moveto(x_ll,             y_ll);
     p->lineto(x_ll,             y_ll + bar_height);
     p->lineto(x_ll + bar_width, y_ll + bar_height);
     p->lineto(x_ll + bar_width, y_ll);
     p->closepath();
     p->fill();

     //
     // Outline color box
     //
     p->setrgbcolor(0.0, 0.0, 0.0);
     p->newpath();
     p->moveto(x_ll,             y_ll);
     p->lineto(x_ll,             y_ll + bar_height);
     p->lineto(x_ll + bar_width, y_ll + bar_height);
     p->lineto(x_ll + bar_width, y_ll);
     p->closepath();
     p->stroke();

     //
     // Add text
     //
     sprintf(label, "%.1f", v);
     p->write_centered_text(2, 1,  x_ll + 0.5*bar_width,
                            y_ll + 0.5*bar_height, 0.5, 0.5, label);

     v    += step;
     y_ll += bar_height;
   }

   p->grestore();

  return;
}

////////////////////////////////////////////////////////////////////////

void draw_border(PSfile *p, BoundingBox &dim) {

   p->gsave();
   p->setlinewidth(l_width);
   p->newpath();
   p->moveto(dim.x_ll, dim.y_ll);
   p->lineto(dim.x_ur, dim.y_ll);
   p->lineto(dim.x_ur, dim.y_ur);
   p->lineto(dim.x_ll, dim.y_ur);
   p->closepath();
   p->stroke();
   p->grestore();

   return;
}

////////////////////////////////////////////////////////////////////////

void draw_map(PSfile *p, BoundingBox &dim) {

   p->gsave();
   p->setlinewidth(l_width);
   draw_world(grid, xy_bb, *p, ll_bb, dim, c_map, met_data_dir);
   draw_states(grid, xy_bb, *p, ll_bb, dim, c_map, met_data_dir);
   p->grestore();

   return;
}

////////////////////////////////////////////////////////////////////////

void draw_tiles(PSfile *p, BoundingBox &dim,
                int tile_start, int tile_end, int label_flag) {
   int i;
   double page_x, page_y;
   char label[128];
   BoundingBox tile_bb;

   p->gsave();

   // Loop through the tiles to be applied
   for(i=tile_start; i<=tile_end; i++) {

      // If padding was performed, the tile is the size of the domain
      if(conf_info.conf.grid_decomp_flag().ival() == 2) {
         tile_bb = dim;
      }
      // Find the lower-left and upper-right corners of the tile
      else {

         // Compute the page x/y coordinates for this tile
         gridxy_to_pagexy(grid,
            conf_info.tile_xll[i],
            conf_info.tile_yll[i],
            tile_bb.x_ll, tile_bb.y_ll, dim);

         gridxy_to_pagexy(grid,
            conf_info.tile_xll[i] + conf_info.get_tile_dim(),
            conf_info.tile_yll[i] + conf_info.get_tile_dim(),
            tile_bb.x_ur, tile_bb.y_ur, dim);
      }

      // Draw the border for this tile
      p->setrgbcolor(1.0, 0.0, 0.0);
      p->setlinewidth(l_width_thick);
      p->newpath();
      p->moveto(tile_bb.x_ll, tile_bb.y_ll);
      p->lineto(tile_bb.x_ur, tile_bb.y_ll);
      p->lineto(tile_bb.x_ur, tile_bb.y_ur);
      p->lineto(tile_bb.x_ll, tile_bb.y_ur);
      p->closepath();
      p->stroke();

      if(label_flag) {
         // Draw diagonals for this tile
         p->gsave();
         p->setlinewidth(l_width);
         p->file() << "[2] 2 setdash";
         p->moveto(tile_bb.x_ll, tile_bb.y_ll);
         p->lineto(tile_bb.x_ur, tile_bb.y_ur);
         p->stroke();

         p->moveto(tile_bb.x_ur, tile_bb.y_ll);
         p->lineto(tile_bb.x_ll, tile_bb.y_ur);
         p->stroke();
         p->grestore();

         // Plot the tile number in the center
         p->setlinewidth(0.0);

         page_x = (tile_bb.x_ll + tile_bb.x_ur)/2.0,
         page_y = (tile_bb.y_ll + tile_bb.y_ur)/2.0,

         p->choose_font(28, 20.0, met_data_dir);
         sprintf(label, "%i", i+1);
         p->write_centered_text(2, 1, page_x, page_y, 0.5, 0.5, label);

         // Draw outline in black
         p->setrgbcolor(0.0, 0.0, 0.0);
         p->write_centered_text(2, 0, page_x, page_y, 0.5, 0.5, label);
      } // end if
   } // end for

   p->grestore();

   return;
}

////////////////////////////////////////////////////////////////////////

void render_image(PSfile *p, const WrfData &wd, BoundingBox &dim) {
   RenderInfo render_info;
   Ppm ppm_image;
   int x, y, grid_x, grid_y;
   double mag;
   Color c;
   Color *c_fill_ptr;
   ColorTable *ct_ptr;

   //
   // Set up pointers to the appropriate colortable and fill color
   // values.
   //
   ct_ptr = &raw_ct;
   c_fill_ptr = &c_raw_fill;

   //
   // Convert the WrfData object to PPM
   //
   ppm_image.set_size_xy((int) xy_bb.width, (int) xy_bb.height);

   //
   // Check if the x,y plotting region matches the data dimensions.
   //
   if(nint(xy_bb.width)  == wd.get_nx() &&
      nint(xy_bb.height) == wd.get_ny()) {

      for(x=0; x<wd.get_nx(); x++) {
         for(y=0; y<wd.get_ny(); y++) {

            if(wd.is_bad_xy(x, y))
               c = *c_fill_ptr;
            else
               c = ct_ptr->nearest(wd.get_xy_double(x, y));

            ppm_image.putxy(c, x, y);
         }
      }
   }
   //
   // If they don't match, plot the pad
   //
   else {

      for(x=nint(xy_bb.x_ll); x<xy_bb.x_ur; x++) {
         for(y=nint(xy_bb.y_ll); y<xy_bb.y_ur; y++) {

            //
            // Compute grid relative x, y values
            //
            grid_x = nint(x + xy_bb.x_ll);
            grid_y = nint(y + xy_bb.y_ll);

            if(grid_x < 0 || grid_x >= wd.get_nx() ||
               grid_y < 0 || grid_y >= wd.get_ny()) continue;

            if(wd.is_bad_xy(grid_x, grid_y))
               c = *c_fill_ptr;
            else
               c = ct_ptr->nearest(wd.get_xy_double(grid_x, grid_y));

            ppm_image.putxy(c, x, y);
         }
      }
   }

   mag = (dim.x_ur - dim.x_ll)/xy_bb.width;

   render_info.x_ll = dim.x_ll;
   render_info.y_ll = dim.y_ll;
   render_info.magnification = mag;
   render_info.bw = 0;
   render_info.add_filter(RunLengthEncode);
   render_info.add_filter(ASCII85Encode);
   render(*p, ppm_image, render_info);

   return;
}

////////////////////////////////////////////////////////////////////////

void render_tile(PSfile *p, const double *data, int n, int i_tile,
                 BoundingBox &dim) {
   RenderInfo render_info;
   Ppm ppm_image;
   int i, x, y;
   double mag;
   Color c;
   Color *c_fill_ptr;
   ColorTable *ct_ptr;

   //
   // Set up pointers to the appropriate colortable and fill color
   // values.
   //
   ct_ptr = &wvlt_ct;
   c_fill_ptr = &c_wvlt_fill;

   //
   // Convert the WrfData object to PPM
   //
   ppm_image.set_size_xy((int) xy_bb.width, (int) xy_bb.height);

   for(i=0; i<n; i++) {

      x = nint(conf_info.tile_xll[i_tile] + i%conf_info.get_tile_dim());
      y = nint(conf_info.tile_yll[i_tile] + i/conf_info.get_tile_dim());

      if(is_bad_data(data[i]))
         c = *c_fill_ptr;
      else
         c = ct_ptr->nearest(data[i]);

      ppm_image.putxy(c, x, y);
   }

   mag = (dim.x_ur - dim.x_ll)/xy_bb.width;

   render_info.x_ll = dim.x_ll;
   render_info.y_ll = dim.y_ll;
   render_info.magnification = mag;
   render_info.bw = 0;
   render_info.add_filter(RunLengthEncode);
   render_info.add_filter(ASCII85Encode);
   render(*p, ppm_image, render_info);

   return;
}

////////////////////////////////////////////////////////////////////////

void usage(int argc, char *argv[]) {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tfcst_file\n"
        << "\tobs_file\n"
        << "\tconfig_file\n"
        << "\t[-outdir path]\n"
        << "\t[-ps]\n"
        << "\t[-nc]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"fcst_file\" is a forecast file in either Grib "
        << "or netCDF format (output of pcp_combine) containing the "
        << "field(s) to be verified (required).\n"

        << "\t\t\"obs_file\" is an observation file in either Grib "
        << "or netCDF format (output of pcp_combine) containing the "
        << "verifying field(s) (required).\n"

        << "\t\t\"config_file\" is a WaveletStatConfig file containing "
        << "the desired configuration settings (required).\n"

        << "\t\t\"-outdir path\" overrides the default output directory ("
        << out_dir << ") (optional).\n"

        << "\t\t\"-ps\" disables the PostScript output file (optional)."
        << "\n"

        << "\t\t\"-nc\" disables the NetCDF output file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << verbosity << ") (optional).\n"

        << "\n\tNOTE: The forecast and observation fields must be "
        << "on the same grid.\n\n" << flush;

   return;
}

////////////////////////////////////////////////////////////////////////
