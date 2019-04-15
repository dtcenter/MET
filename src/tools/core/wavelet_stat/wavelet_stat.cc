// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
//   001    11/05/09  Halley Gotway   Generalize to compare two
//                    different fcst and obs fields.
//   002    05/27/10  Halley Gotway   Add -fcst_valid, -fcst_lead,
//                    -obs_valid, and -obs_lead command line options.
//   003    06/30/10  Halley Gotway   Enhance grid equality checks.
//   004    08/09/10  Halley Gotway   Add valid time variable attributes
//                    to NetCDF output.
//   005    10/20/11  Holmes          Added use of command line class to
//                    parse the command line arguments.
//   006    11/14/11  Holmes          Added code to enable reading of
//                    multiple config files.
//   007    05/01/12  Halley Gotway   Switch to using vx_config library.
//   008    05/01/12  Halley Gotway   Move -fcst_valid, -fcst_lead,
//                    -obs_valid, -obs_lead, -ps, -nc command line
//                    options to config file.
//   009    11/12/14  Halley Gotway   Pass the obtype entry from the
//                    from the config file to the output files.
//   010    02/25/15  Halley Gotway   Add automated regridding.
//   011    05/15/17  Prestopnik P.   Add shape to regrid options.
//   012    04/08/19  Halley Gotway   Add percentile thresholds.
//   012    04/01/19  Fillmore       Add FCST and OBS units.
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

#include "wavelet_stat.h"

#include "vx_statistics.h"
#include "vx_nc_util.h"
#include "vx_regrid.h"
#include "vx_log.h"
#include "vx_plot_util.h"

////////////////////////////////////////////////////////////////////////


static const bool use_flate = true;
static int compress_level = -1;


////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_scores();
static void clean_up();

static void setup_first_pass(const DataPlane &);
static void setup_txt_files (unixtime, int);
static void setup_table     (AsciiTable &);
static void setup_nc_file   (const WaveletStatNcOutInfo &, unixtime, int);
static void setup_ps_file   (unixtime, int);

static void build_outfile_name(unixtime, int, const char *,
                               ConcatString &);
static double get_fill_value(const DataPlane &, int);
static void fill_bad_data(DataPlane &, double);
static void pad_field(DataPlane &, double);
static void get_tile(const DataPlane &, const DataPlane &, int, int,
                     NumArray &, NumArray &);
static int  get_tile_tot_count();

static void do_intensity_scale(const NumArray &, const NumArray &,
                               ISCInfo *&, int, int);

static void aggregate_isc_info(ISCInfo **, int, int, ISCInfo &);

static void compute_cts(const double *, const double *, int, ISCInfo &);
static void compute_mse(const double *, const double *, int, double &);
static void compute_energy(const double *, int, double &);

static void write_nc_raw(const WaveletStatNcOutInfo &, const double *, const double *,
                         int, int, int);
static void write_nc_wav(const WaveletStatNcOutInfo &, const double *, const double *,
                         int, int, int, int,
                         SingleThresh &, SingleThresh &);
static void add_var_att_local(NcVar *, const char *, const char *);

static void close_out_files();

static double sum_array(double *, int);
static double mean_array(double *, int);

static void plot_ps_raw(const DataPlane &, const DataPlane &,
                        const DataPlane &, const DataPlane &, int);
static void plot_ps_wvlt(const double *, int, int, int, ISCInfo &,
                         int, int);
static double compute_percentage(double, double);

static void set_plot_dims(int, int);
static void set_xy_bb();
static void set_dim(Box &, double, double, double);
static void draw_colorbar(PSfile *, Box &, int, int);
static void draw_border(PSfile *, Box &);
static void draw_map(PSfile *, Box &);
static void draw_tiles(PSfile *, Box &, int, int, int);
static void render_image(PSfile *, const DataPlane &, Box &, int);
static void render_tile(PSfile *, const double *, int, int, Box &);

static void usage();
static void set_outdir(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);
static void set_compress(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Compute the scores and write them out
   process_scores();

   // Close the text files and deallocate memory
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;
   GrdFileType ftype, otype;
   ConcatString default_config_file;
   DataPlane dp;

   // Set the default output directory
   out_dir = replace_path(default_out_dir);

   // Check for zero arguments
   if(argc == 1) usage();

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_outdir, "-outdir", 1);
   cline.add(set_logfile, "-log", 1);
   cline.add(set_verbosity, "-v", 1);
   cline.add(set_compress,  "-compress",  1);

   // Parse the command line
   cline.parse();

   // Check for error. There should be three arguments left:
   // forecast, observation, and config filenames
   if(cline.n() != 3) usage();

   // Store the input file names
   fcst_file   = cline[0];
   obs_file    = cline[1];
   config_file = cline[2];

   // Create the default config file name
   default_config_file = replace_path(default_config_filename);

   // List the config files
   mlog << Debug(1)
        << "Default Config File: " << default_config_file << "\n"
        << "User Config File: "    << config_file << "\n";

   // Read the config files
   conf_info.read_config(default_config_file.c_str(), config_file.c_str());

   // Get the forecast and observation file types from config, if present
   ftype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_fcst));
   otype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_obs));

   // Read forecast file
   if(!(fcst_mtddf = mtddf_factory.new_met_2d_data_file(fcst_file.c_str(), ftype))) {
      mlog << Error << "\nTrouble reading forecast file \""
           << fcst_file << "\"\n\n";
      exit(1);
   }

   // Read observation file
   if(!(obs_mtddf = mtddf_factory.new_met_2d_data_file(obs_file.c_str(), otype))) {
      mlog << Error << "\nTrouble reading observation file \""
           << obs_file << "\"\n\n";
      exit(1);
   }

   // Store the input data file types
   ftype = fcst_mtddf->file_type();
   otype = obs_mtddf->file_type();

   // Process the configuration
   conf_info.process_config(ftype, otype);

   // For python types read the first field to set the grid
   if(ftype == FileType_Python_Numpy ||
      ftype == FileType_Python_Xarray) {
      if(!fcst_mtddf->data_plane(*conf_info.fcst_info[0], dp)) {
         mlog << Error << "\nTrouble reading data from forecast file \""
              << fcst_file << "\"\n\n";
         exit(1);
      }
   }

   if(otype == FileType_Python_Numpy ||
      otype == FileType_Python_Xarray) {
      if(!obs_mtddf->data_plane(*conf_info.obs_info[0], dp)) {
         mlog << Error << "\nTrouble reading data from observation file \""
              << obs_file << "\"\n\n";
         exit(1);
      }
   }

   // Determine the verification grid
   grid = parse_vx_grid(conf_info.fcst_info[0]->regrid(),
                        &(fcst_mtddf->grid()), &(obs_mtddf->grid()));

   // Set the model name
   shc.set_model(conf_info.model.c_str());

   // Set the obtype column
   shc.set_obtype(conf_info.obtype.c_str());

   // List the input files
   mlog << Debug(1)
        << "Forecast File: " << fcst_file   << "\n"
        << "Observation File: " << obs_file    << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void process_scores() {
   int i, j, k;
   bool status;
   double fcst_fill, obs_fill;

   DataPlane fcst_dp,      obs_dp;
   DataPlane fcst_dp_fill, obs_dp_fill;

   NumArray f_na, o_na;
   ISCInfo **isc_info = (ISCInfo **) 0, isc_aggr;
   Grid fcst_grid, obs_grid;

   // Loop through each of the fields to be verified
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Read the gridded data from the input forecast file
      status = fcst_mtddf->data_plane(*conf_info.fcst_info[i], fcst_dp);

      if(!status) {
         mlog << Warning << "\nprocess_scores() -> "
              << conf_info.fcst_info[i]->magic_str()
              << " not found in file: " << fcst_file << "\n";
         continue;
      }

      // Regrid, if necessary
      if(!(fcst_mtddf->grid() == grid)) {
         mlog << Debug(1)
              << "Regridding forecast " << conf_info.fcst_info[i]->magic_str()
              << " to the verification grid.\n";
         fcst_dp = met_regrid(fcst_dp, fcst_mtddf->grid(), grid,
                              conf_info.fcst_info[i]->regrid());
      }

      // Set the forecast lead time
      shc.set_fcst_lead_sec(fcst_dp.lead());

      // Set the forecast valid time
      shc.set_fcst_valid_beg(fcst_dp.valid());
      shc.set_fcst_valid_end(fcst_dp.valid());

      // Read the gridded data from the input observation file
      status = obs_mtddf->data_plane(*conf_info.obs_info[i], obs_dp);

      if(!status) {
         mlog << Warning << "\nprocess_scores() -> "
              << conf_info.obs_info[i]->magic_str()
              << " not found in file: " << obs_file << "\n\n";
         continue;
      }

      // Regrid, if necessary
      if(!(obs_mtddf->grid() == grid)) {
         mlog << Debug(1)
              << "Regridding observation " << conf_info.obs_info[i]->magic_str()
              << " to the verification grid.\n";
         obs_dp = met_regrid(obs_dp, obs_mtddf->grid(), grid,
                             conf_info.obs_info[i]->regrid());
      }

      // Set the observation lead time
      shc.set_obs_lead_sec(obs_dp.lead());

      // Set the observation valid time
      shc.set_obs_valid_beg(obs_dp.valid());
      shc.set_obs_valid_end(obs_dp.valid());

      // Check that the valid times match
      if(fcst_dp.valid() != obs_dp.valid()) {

         mlog << Warning << "\nprocess_scores() -> "
              << "Forecast and observation valid times do not match "
              << unix_to_yyyymmdd_hhmmss(fcst_dp.valid()) << " != " <<
              unix_to_yyyymmdd_hhmmss(obs_dp.valid()) << " for "
              << conf_info.fcst_info[i]->magic_str() << " versus "
              << conf_info.obs_info[i]->magic_str() << ".\n";
      }

      // Check that the accumulation intervals match
      if(conf_info.fcst_info[i]->level().type() == LevelType_Accum &&
         conf_info.obs_info[i]->level().type()  == LevelType_Accum &&
         fcst_dp.accum()       != obs_dp.accum()) {

         mlog << Warning << "\nprocess_scores() -> "
              << "Forecast and observation accumulation times "
              << "do not match " << sec_to_hhmmss(fcst_dp.accum())
              << " != " << sec_to_hhmmss(obs_dp.accum())
              << " for " << conf_info.fcst_info[i]->magic_str() << " versus "
              << conf_info.obs_info[i]->magic_str() << ".\n";
      }

      // Setup the first pass through the data
      if(is_first_pass) setup_first_pass(fcst_dp);

      // Store the description
      shc.set_desc(conf_info.desc[i].c_str());

      // Store the forecast variable name
      shc.set_fcst_var(conf_info.fcst_info[i]->name());

      // Store the forecast variable units
      shc.set_fcst_units(conf_info.fcst_info[i]->units());

      // Set the forecast level name
      shc.set_fcst_lev(conf_info.fcst_info[i]->level_name().c_str());

      // Store the observation variable name
      shc.set_obs_var(conf_info.obs_info[i]->name());

      // Store the observation variable units
      shc.set_obs_units(conf_info.obs_info[i]->units());

      // Set the observation level name
      shc.set_obs_lev(conf_info.obs_info[i]->level_name().c_str());

      mlog << Debug(2) << "\n" << sep_str << "\n\n"
           << "Processing " << conf_info.fcst_info[i]->magic_str()
           << " versus " << conf_info.obs_info[i]->magic_str() << ".\n";

      // Mask out the missing data between fields
      if(conf_info.mask_missing_flag == FieldType_Fcst ||
         conf_info.mask_missing_flag == FieldType_Both)
         mask_bad_data(fcst_dp, obs_dp);

      // Mask out the missing data between fields
      if(conf_info.mask_missing_flag == FieldType_Obs ||
         conf_info.mask_missing_flag == FieldType_Both)
         mask_bad_data(obs_dp, fcst_dp);

      // Get the fill data value to be used for each field
      fcst_fill = get_fill_value(fcst_dp, i);
      obs_fill  = get_fill_value(obs_dp, i);

      // Initialize the fill fields
      fcst_dp_fill = fcst_dp;
      obs_dp_fill  = obs_dp;

      // Replace any bad data in the fields with a fill value
      mlog << Debug(2) << "Forecast field: ";
      fill_bad_data(fcst_dp_fill, fcst_fill);

      mlog << Debug(2) << "Observation field: ";
      fill_bad_data(obs_dp_fill,  obs_fill);

      // Pad the fields out to the nearest power of two if requsted
      if(conf_info.grid_decomp_flag == GridDecompType_Pad) {
         mlog << Debug(2) << "Padding the fields out to the nearest integer "
              << "power of two.\n";
         pad_field(fcst_dp_fill, fcst_fill);
         pad_field(obs_dp_fill,  obs_fill);
      }

      // Write out the raw fields to PostScript
      if(conf_info.ps_plot_flag) {
         plot_ps_raw(fcst_dp, obs_dp, fcst_dp_fill, obs_dp_fill, i);
      }

      // Allocate memory for ISCInfo objects sized as [n_tile][n_thresh]
      isc_info = new ISCInfo * [conf_info.get_n_tile()];
      for(j=0; j<conf_info.get_n_tile(); j++) {
         isc_info[j] = new ISCInfo [conf_info.fcat_ta[i].n_elements()];
      }

      // Process percentile thresholds
      conf_info.set_perc_thresh(fcst_dp, obs_dp);

      // Loop through the tiles to be applied
      for(j=0; j<conf_info.get_n_tile(); j++) {

         // Set the mask name
         ConcatString mask = (string)"TILE_TOT";
         if(conf_info.get_n_tile() > 1) mask.format("TILE%i", j+1);
         shc.set_mask(mask.text());

         // Apply the current tile to the fields
         get_tile(fcst_dp_fill, obs_dp_fill, i, j, f_na, o_na);

         // Compute Intensity-Scale scores
         if(conf_info.output_flag[i_isc] != STATOutputType_None) {

            // Do the intensity-scale decomposition
            do_intensity_scale(f_na, o_na, isc_info[j], i, j);

            // Write out the ISC statistics
            for(k=0; k<conf_info.fcat_ta[i].n_elements(); k++) {

               // Store the tile definition parameters
               isc_info[j][k].tile_dim = conf_info.get_tile_dim();
               isc_info[j][k].tile_xll = nint(conf_info.tile_xll[j]);
               isc_info[j][k].tile_yll = nint(conf_info.tile_xll[j]);

               // Set the forecast and observation thresholds
               shc.set_fcst_thresh(conf_info.fcat_ta[i][k]);
               shc.set_obs_thresh(conf_info.ocat_ta[i][k]);

               write_isc_row(shc, isc_info[j][k],
                  conf_info.output_flag[i_isc] == STATOutputType_Both,
                  stat_at, i_stat_row, isc_at, i_isc_row);
            } // end for k
         } // end if
      } // end for j

      // Aggregate the scores across tiles
      if(conf_info.get_n_tile() > 1) {

         // Set the mask name
         shc.set_mask("TILE_TOT");

         for(j=0; j<conf_info.fcat_ta[i].n_elements(); j++) {

            // Set the forecast and observation thresholds
            shc.set_fcst_thresh(conf_info.fcat_ta[i][j]);
            shc.set_obs_thresh(conf_info.ocat_ta[i][j]);

            // Aggregate the tiles for the current threshold
            aggregate_isc_info(isc_info, i, j, isc_aggr);

            write_isc_row(shc, isc_aggr,
               conf_info.output_flag[i_isc] == STATOutputType_Both,
               stat_at, i_stat_row, isc_at, i_isc_row);
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

   mlog << Debug(2) << "\n" << sep_str << "\n\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {

   // Close the output files
   close_out_files();

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_first_pass(const DataPlane &dp) {

   // Unset the flag
   is_first_pass = false;

   // Setup the tiles to be used
   conf_info.process_tiles(grid);

   // Create output text files as requested in the configuration file
   setup_txt_files(dp.valid(), dp.lead());

   // If requested, create a NetCDF file
   if ( ! (conf_info.nc_info.all_false()) ) {
      setup_nc_file(conf_info.nc_info, dp.valid(), dp.lead());
   }

   // If requested, create a PostScript file
   if(conf_info.ps_plot_flag) {
      setup_ps_file(dp.valid(), dp.lead());
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
   open_txt_file(stat_out, stat_file.c_str());

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

   if(conf_info.output_flag[i_isc] != STATOutputType_None) {


      // Initialize file stream
      isc_out   = (ofstream *) 0;

      // Build the file name
      isc_file << tmp_str << "_" << isc_file_abbr << txt_file_ext;

      // Create the output STAT file
      open_txt_file(isc_out, isc_file.c_str());

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

   // Justify the STAT AsciiTable objects
   justify_stat_cols(at);

   // Set the precision
   at.set_precision(conf_info.conf.output_precision());

   // Set the bad data value
   at.set_bad_data_value(bad_data_double);

   // Set the bad data string
   at.set_bad_data_str(na_str);

   // Don't write out trailing blank rows
   at.set_delete_trailing_blank_rows(1);

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_nc_file(const WaveletStatNcOutInfo & nc_info, unixtime valid_ut, int lead_sec) {
   int i, x, y;

   // Create output NetCDF file name
   build_outfile_name(valid_ut, lead_sec, ".nc", out_nc_file);

   // Create a new NetCDF file and open it
   nc_out = open_ncfile(out_nc_file.c_str(), true);

   if(!nc_out || IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_nc_file << "\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, out_nc_file.c_str(), program_name,
                       conf_info.model.c_str(), conf_info.obtype.c_str());
   if ( nc_info.do_diff )  add_att(nc_out, "Difference", "Forecast Value - Observation Value");

   // Set the NetCDF dimensions
   x_dim = add_dim(nc_out, "x", conf_info.get_tile_dim());

   y_dim = add_dim(nc_out, "y", conf_info.get_tile_dim());

   scale_dim = add_dim(nc_out, "scale", conf_info.get_n_scale()+2);

   tile_dim  = add_dim(nc_out, "tile", conf_info.get_n_tile());

   // Add the x_ll and y_ll variables
   NcVar x_ll_var ;
   NcVar y_ll_var ;

   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = conf_info.get_compression_level();

   x_ll_var = add_var(nc_out, "x_ll", ncInt, tile_dim, deflate_level);
   y_ll_var = add_var(nc_out, "y_ll", ncInt, tile_dim, deflate_level);

   for(i=0; i<conf_info.get_n_tile(); i++) {

      x = nint(conf_info.tile_xll[i]);
      y = nint(conf_info.tile_yll[i]);

      // Write the x_ll value
      if(!put_nc_data(&x_ll_var, &x, 1, i)) {

         mlog << Error << "\nsetup_nc_file() -> "
              << "error with the x_ll_var->put"
              << "\n\n";
         exit(1);
      }

      // Write the y_ll value
      if(!put_nc_data(&y_ll_var, &y, 1, i)) {

         mlog << Error << "\nsetup_nc_file() -> "
              << "error with the y_ll_var->put"
              << "\n\n";
         exit(1);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_ps_file(unixtime valid_ut, int lead_sec) {

   // Create output PostScript file name
   build_outfile_name(valid_ut, lead_sec, ".ps", out_ps_file);

   // Create a new PostScript file and open it
   ps_out = new PSfile;
   ps_out->open(out_ps_file.c_str());
   n_page = 1;

   if(!ps_out) {
      mlog << Error << "\nsetup_ps_file() -> "
           << "trouble opening output PostScript file "
           << out_ps_file << "\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void build_outfile_name(unixtime valid_ut, int lead_sec,
                        const char *suffix, ConcatString &str) {

   //
   // Create output file name
   //

   // Append the output directory and program name
   str << cs_erase << out_dir << "/" << program_name;

   // Append the output prefix, if defined
   if(conf_info.output_prefix.nonempty())
      str << "_" << conf_info.output_prefix;

   // Append the timing information
   str << "_"
       << sec_to_hhmmss(lead_sec) << "L_"
       << unix_to_yyyymmdd_hhmmss(valid_ut) << "V";

   // Append the suffix
   str << suffix;

   return;
}

////////////////////////////////////////////////////////////////////////

double get_fill_value(const DataPlane &dp, int i_vx) {
   int x, y, count;
   double fill_val, sum;

   //
   // If verifying precipitation, fill bad data points with zero.
   // Otherwise, fill them with the mean of the valid data.
   //
   if(conf_info.fcst_info[i_vx]->is_precipitation() ||
      conf_info.obs_info[i_vx]->is_precipitation()) {
      fill_val = 0.0;
   }
   else {

      count = 0;
      sum = 0.0;
      for(x=0; x<dp.nx(); x++) {
         for(y=0; y<dp.ny(); y++) {

            if(is_bad_data(dp.get(x, y))) continue;

            sum += dp.get(x, y);
            count++;
         } // end for y
      } // end for x

      if(count > 0) fill_val = sum/count;
      else          fill_val = 0.0;
   }

   return(fill_val);
}

////////////////////////////////////////////////////////////////////////

void fill_bad_data(DataPlane &dp, double fill_val) {
   int x, y, count;

   //
   // Replace any bad data values with the fill value
   //
   count = 0;
   for(x=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {
         if(is_bad_data(dp.get(x, y))) {
            dp.set(fill_val, x, y);
            count++;
         }
      } // end for y
   } // end for x

   if(count > 0) {
      mlog << "Replaced " << count << " bad data values out of "
           << dp.nx()*dp.ny()
           << " points with fill value of "
           << fill_val << ".\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void pad_field(DataPlane &dp, double pad_val) {
   int x, y, in_x, in_y;
   DataPlane dp_pad;

   // Set up the DataPlane object
   dp_pad.set_size(conf_info.get_tile_dim(), conf_info.get_tile_dim());

   // Fill the DataPlane object
   for(x=0; x<dp_pad.nx(); x++) {
      for(y=0; y<dp_pad.ny(); y++) {

         // If in the region of valid data
         if(x >= conf_info.pad_bb.x_ll() && x < conf_info.pad_bb.x_ur() &&
            y >= conf_info.pad_bb.y_ll() && y < conf_info.pad_bb.y_ur()) {

            in_x = nint(x - conf_info.pad_bb.x_ll());
            in_y = nint(y - conf_info.pad_bb.y_ll());
            dp_pad.set(dp.get(in_x, in_y), x, y);
         }
         // Else, in the pad
         else {
            dp_pad.set(pad_val, x, y);
         }
      } // end for y
   } // end for x

   dp = dp_pad;

   return;
}

////////////////////////////////////////////////////////////////////////

void get_tile(const DataPlane &fcst_dp, const DataPlane &obs_dp,
              int i_vx, int i_tile,
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

   if(x_ll < 0 || x_ll > fcst_dp.nx() ||
      y_ll < 0 || y_ll > fcst_dp.ny() ||
      x_ur < 0 || x_ur > fcst_dp.nx() ||
      y_ur < 0 || y_ur > fcst_dp.ny()) {

      mlog << Error << "\nget_tile() -> "
           << "invalid tile extends off the grid: "
           << "(x_ll, y_ll, dim) = (" << x_ll << ", " << y_ll << ", "
           << conf_info.get_tile_dim() << ") and (nx, ny) = ("
           << fcst_dp.nx() << ", " << fcst_dp.ny() << ")!\n\n";
      exit(1);
   }

   mlog << Debug(2) << "Retrieving data for tile " << i_tile+1
        << " with dimension = " << conf_info.get_tile_dim()
        << " and lower-left (x, y) = ("
        << x_ll << ", " << y_ll << ")\n";

   //
   // Store the pairs in NumArray objects
   //
   for(y=y_ll; y<y_ur; y++) {
      for(x=x_ll; x<x_ur; x++) {
         f_na.add(fcst_dp.get(x, y));
         o_na.add(obs_dp.get(x, y));
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
                        ISCInfo *&isc_info, int i_vx, int i_tile) {
   double *f_dat = (double *) 0, *o_dat = (double *) 0; // Raw and thresholded binary fields
   double *f_dwt = (double *) 0, *o_dwt = (double *) 0; // Discrete wavelet transformations
   double *f_scl = (double *) 0, *o_scl = (double *) 0; // Binary field decomposed by scale
   double *diff = (double *) 0;                         // Difference field
   double mse, fen, oen;
   int n, ns, n_isc;
   int bnd, row, col;
   int i, j, k;
   ConcatString fcst_thresh_str, obs_thresh_str;

   // Check the NumArray lengths
   n = f_na.n_elements();
   if(n != o_na.n_elements()) {
      mlog << Error << "\ndo_intensity_scale() -> "
           << "the forecast and observation arrays must have equal "
           << "length.\n\n";
      exit(1);
   }

   // Check that the number of points = tile_dim * tile_dim
   if(n != (conf_info.get_tile_dim() * conf_info.get_tile_dim())) {
      mlog << Error << "\nprocess_scores() -> "
           << "the number of points (" << n << ") should equal the "
           << "tile dimension squared ("
           << conf_info.get_tile_dim() * conf_info.get_tile_dim()
           << ").\n\n";
      exit(1);
   }

   // Get the number of scales
   ns = conf_info.get_n_scale();

   // Set up the ISCInfo thresholds and n_scale
   n_isc = conf_info.fcat_ta[i_vx].n_elements();
   for(i=0; i<n_isc; i++) {
      isc_info[i].clear();
      isc_info[i].fthresh = conf_info.fcat_ta[i_vx][i];
      isc_info[i].othresh = conf_info.ocat_ta[i_vx][i];
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
   if( conf_info.nc_info.do_raw || conf_info.nc_info.do_diff ) {
      write_nc_raw(conf_info.nc_info, f_dat, o_dat, n, i_vx, i_tile);
   }

   // Apply each threshold
   for(i=0; i<conf_info.fcat_ta[i_vx].n_elements(); i++) {

      fcst_thresh_str = isc_info[i].fthresh.get_abbr_str();
      obs_thresh_str  = isc_info[i].othresh.get_abbr_str();

      mlog << Debug(2) << "Computing Intensity-Scale decomposition for "
           << conf_info.fcst_info[i_vx]->magic_str() << " "
           << fcst_thresh_str << " versus "
           << conf_info.obs_info[i_vx]->magic_str() << " "
           << obs_thresh_str << ".\n";

      // Apply the threshold to each point to create 0/1 mask fields
      for(j=0; j<n; j++) {
         f_dat[j] = isc_info[i].fthresh.check(f_na[j]);
         o_dat[j] = isc_info[i].othresh.check(o_na[j]);
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
      if ( conf_info.nc_info.do_raw || conf_info.nc_info.do_diff )  {
         write_nc_wav(conf_info.nc_info, f_dat, o_dat, n, i_vx, i_tile, -1,
                      isc_info[i].fthresh,
                      isc_info[i].othresh);
      }

      // Write the thresholded binary difference field to PostScript
      if ( ! (conf_info.nc_info.all_false()) ) {
         plot_ps_wvlt(diff, n, i_vx, i_tile, isc_info[i], -1, ns);
      }

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
         if ( ! (conf_info.nc_info.all_false()) ) {
            write_nc_wav(conf_info.nc_info,
                         f_scl, o_scl, n, i_vx, i_tile, j,
                         isc_info[i].fthresh,
                         isc_info[i].othresh);
         }

         // Compute the difference field for this scale
         for(k=0; k<n; k++) diff[k] = f_scl[k] - o_scl[k];

         // Write the decomposed difference field for this scale to PostScript
         if(conf_info.ps_plot_flag) {
            plot_ps_wvlt(diff, n, i_vx, i_tile, isc_info[i], j, ns);
         }

      } // end for j

      // Dump out the scores
      ConcatString msg;
      ConcatString thresh_str;
      thresh_str << cs_erase << fcst_thresh_str << ", " << obs_thresh_str;

      msg << "FBIAS[" << thresh_str << "]\t\t= "
          << isc_info[i].fbias << "\n"
          << "BASER[" << thresh_str << "]\t\t= "
          << isc_info[i].baser << "\n"
          << "MSE[" << thresh_str << "]\t\t= "
          << isc_info[i].mse << "\n"
          << "ISC[" << thresh_str << "]\t\t= "
          << isc_info[i].isc << "\n"
          << "FEN[" << thresh_str << "]\t\t= "
          << isc_info[i].fen << "\n"
          << "OEN[" << thresh_str << "]\t\t= "
          << isc_info[i].oen << "\n";

      for(j=0; j<=ns; j++) {
         msg << "SCALE_" << j+1 << "[" << thresh_str
             << "] MSE, ISC, FEN, OEN = "
             << isc_info[i].mse_scale[j] << ", "
             << isc_info[i].isc_scale[j] << ", "
             << isc_info[i].fen_scale[j] << ", "
             << isc_info[i].oen_scale[j] << "\n";
      }

      msg << "MSE_SUM[" << thresh_str << "]\t= "
          << sum_array(isc_info[i].mse_scale, isc_info[i].n_scale+1) << "\n"
          << "ISC_MEAN[" << thresh_str << "]\t= "
          << mean_array(isc_info[i].isc_scale, isc_info[i].n_scale+1) << "\n"
          << "FEN_SUM[" << thresh_str << "]\t= "
          << sum_array(isc_info[i].fen_scale, isc_info[i].n_scale+1) << "\n"
          << "OEN_SUM[" << thresh_str << "]\t= "
          << sum_array(isc_info[i].oen_scale, isc_info[i].n_scale+1) << "\n";

      mlog << Debug(3) << msg;

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

void aggregate_isc_info(ISCInfo **isc_info, int i_vx, int i_thresh,
                        ISCInfo &isc_aggr) {
   int i, j;
   int fy_oy, fy_on, fn_oy, fn_on;
   ConcatString fcst_thresh_str, obs_thresh_str;

   // Set up the aggregated ISCInfo object
   isc_aggr = isc_info[0][i_thresh];
   isc_aggr.zero_out();

   fcst_thresh_str = isc_aggr.fthresh.get_abbr_str();
   obs_thresh_str  = isc_aggr.othresh.get_abbr_str();

   mlog << Debug(2) << "Aggregating ISC for "
        << conf_info.fcst_info[i_vx]->magic_str() << " " << fcst_thresh_str
        << " versus "
        << conf_info.obs_info[i_vx]->magic_str() << " " << obs_thresh_str
        << " using " << conf_info.get_n_tile() << " tiles.\n";

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
   ConcatString msg;
   msg << "FBIAS[" << fcst_thresh_str << ", " << obs_thresh_str << "]\t\t= "
        << isc_aggr.fbias << "\n"
        << "BASER[" << fcst_thresh_str << ", " << obs_thresh_str << "]\t\t= "
        << isc_aggr.baser << "\n"
        << "MSE[" << fcst_thresh_str << ", " << obs_thresh_str << "]\t\t= "
        << isc_aggr.mse << "\n"
        << "ISC[" << fcst_thresh_str << ", " << obs_thresh_str << "]\t\t= "
        << isc_aggr.isc << "\n"
        << "FEN[" << fcst_thresh_str << ", " << obs_thresh_str << "]\t\t= "
        << isc_aggr.fen << "\n"
        << "OEN[" << fcst_thresh_str << ", " << obs_thresh_str << "]\t\t= "
        << isc_aggr.oen << "\n";

   for(j=0; j<=isc_aggr.n_scale; j++) {
      msg << "SCALE_" << j+1 << "["
          << fcst_thresh_str<< ", " << obs_thresh_str
          << "] MSE, ISC, FEN, OEN = "
          << isc_aggr.mse_scale[j] << ", "
          << isc_aggr.isc_scale[j] << ", "
          << isc_aggr.fen_scale[j] << ", "
          << isc_aggr.oen_scale[j] << "\n";
   }

   msg << "MSE_SUM[" << fcst_thresh_str << ", " << obs_thresh_str << "]\t= "
        << sum_array(isc_aggr.mse_scale, isc_aggr.n_scale+1) << "\n"
        << "ISC_MEAN[" << fcst_thresh_str << ", " << obs_thresh_str << "]\t= "
        << mean_array(isc_aggr.isc_scale, isc_aggr.n_scale+1) << "\n"
        << "FEN_SUM[" << fcst_thresh_str << ", " << obs_thresh_str << "]\t= "
        << sum_array(isc_aggr.fen_scale, isc_aggr.n_scale+1) << "\n"
        << "OEN_SUM[" << fcst_thresh_str << ", " << obs_thresh_str << "]\t= "
        << sum_array(isc_aggr.oen_scale, isc_aggr.n_scale+1) << "\n";

   mlog << Debug(2) << msg;

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

void write_nc_raw(const WaveletStatNcOutInfo & nc_info, const double *fdata, const double *odata, int n,
                  int i_vx, int i_tile) {
   int i, d;
   float *fcst_data = (float *) 0;
   float *obs_data  = (float *) 0;
   float *diff_data = (float *) 0;
   ConcatString fcst_var_name;
   ConcatString obs_var_name;
   ConcatString diff_var_name;
   ConcatString val;

   // Build the variable names

   if ( nc_info.do_raw)  {
      fcst_var_name.format("FCST_%s_%s_%s_%s_RAW",
              conf_info.fcst_info[i_vx]->name().text(),
              conf_info.fcst_info[i_vx]->level_name().text(),
              conf_info.obs_info[i_vx]->name().text(),
              conf_info.obs_info[i_vx]->level_name().text());
      obs_var_name.format("OBS_%s_%s_%s_%s_RAW",
              conf_info.fcst_info[i_vx]->name().text(),
              conf_info.fcst_info[i_vx]->level_name().text(),
              conf_info.obs_info[i_vx]->name().text(),
              conf_info.obs_info[i_vx]->level_name().text());
   }

   if ( nc_info.do_diff )  {
      diff_var_name.format("DIFF_%s_%s_%s_%s_RAW",
              conf_info.fcst_info[i_vx]->name().text(),
              conf_info.fcst_info[i_vx]->level_name().text(),
              conf_info.obs_info[i_vx]->name().text(),
              conf_info.obs_info[i_vx]->level_name().text());

   }

   // If this is the first tile, define new variables
   if(i_tile == 0) {

      // Define the forecast and difference variables

      int deflate_level = compress_level;
      if (deflate_level < 0) deflate_level = conf_info.get_compression_level();

      if ( nc_info.do_raw )  {
         fcst_var = add_var(nc_out, (string)fcst_var_name, ncFloat, tile_dim, x_dim, y_dim, deflate_level);
         obs_var  = add_var(nc_out, (string)obs_var_name,  ncFloat, tile_dim, x_dim, y_dim, deflate_level);
      }
      if ( nc_info.do_diff )  diff_var = add_var(nc_out, (string)diff_var_name, ncFloat, tile_dim, x_dim, y_dim, deflate_level);

      // Add variable attributes for the observation field

      if ( nc_info.do_raw )  {
         add_var_att_local(&obs_var, "type", "Observation");
         add_var_att_local(&obs_var, "name", shc.get_obs_var().c_str());
         val.format("%s at %s",
                 conf_info.obs_info[i_vx]->name().text(),
                 conf_info.obs_info[i_vx]->level_name().text());
         add_var_att_local(&obs_var, "long_name", val.c_str());
         add_var_att_local(&obs_var, "level", shc.get_obs_lev().c_str());
         add_var_att_local(&obs_var, "units", conf_info.fcst_info[i_vx]->units().text());
         add_att(&obs_var, "_FillValue", bad_data_float);
         write_netcdf_var_times(&obs_var,
            shc.get_obs_valid_beg() - shc.get_obs_lead_sec(),
            shc.get_obs_valid_beg(), 0);
         add_var_att_local(&obs_var, "desc", conf_info.desc[i_vx].c_str());

         // Add variable attributes for the forecast field
         add_var_att_local(&fcst_var, "type", "Forecast");
         add_var_att_local(&fcst_var, "name", shc.get_fcst_var().c_str());
         val.format("%s at %s",
                 conf_info.fcst_info[i_vx]->name().text(),
                 conf_info.fcst_info[i_vx]->level_name().text());
         add_var_att_local(&fcst_var, "long_name", val.c_str());
         add_var_att_local(&fcst_var, "level", shc.get_fcst_lev().c_str());
         add_var_att_local(&fcst_var, "units", conf_info.fcst_info[i_vx]->units().text());
         add_att(&fcst_var, "_FillValue", bad_data_float);
         write_netcdf_var_times(&fcst_var,
            shc.get_fcst_valid_beg() - shc.get_fcst_lead_sec(),
            shc.get_fcst_valid_beg(), 0);
         add_var_att_local(&fcst_var, "desc", conf_info.desc[i_vx].c_str());

      }

      if ( nc_info.do_diff )  {

         // Add variable attributes for the difference field
         add_var_att_local(&diff_var, "type", "Difference (F-O)");
         val.format("Forecast %s minus Observed %s",
                 shc.get_fcst_var().text(),
                 shc.get_obs_var().text());
         add_var_att_local(&diff_var, "name", val.c_str());
         val.format("%s at %s and %s at %s",
                 conf_info.fcst_info[i_vx]->name().text(),
                 conf_info.fcst_info[i_vx]->level_name().text(),
                 conf_info.obs_info[i_vx]->name().text(),
                 conf_info.obs_info[i_vx]->level_name().text());
         add_var_att_local(&diff_var, "long_name", val.c_str());
         val.format("%s and %s",
                 shc.get_fcst_lev().text(),
                 shc.get_obs_lev().text());
         add_var_att_local(&diff_var, "level", val.c_str());
         val.format("%s and %s",
                 conf_info.fcst_info[i_vx]->units().text(),
                 conf_info.obs_info[i_vx]->units().text());
         add_var_att_local(&diff_var, "units", val.c_str());
         add_att(&diff_var, "_FillValue", bad_data_float);
         write_netcdf_var_times(&diff_var,
            shc.get_fcst_valid_beg() - shc.get_fcst_lead_sec(),
            shc.get_fcst_valid_beg(), 0);
         add_var_att_local(&diff_var, "desc", conf_info.desc[i_vx].c_str());

      }

   }
   // Otherwise, retrieve the previously defined variables
   else {

      if ( nc_info.do_raw )  {
         fcst_var = get_var(nc_out, fcst_var_name.c_str());
         obs_var  = get_var(nc_out, obs_var_name.c_str());
      }
      if ( nc_info.do_diff )  diff_var = get_var(nc_out, diff_var_name.c_str());
   }

   // Allocate memory for the forecast, observation, and difference
   // fields
   if ( nc_info.do_raw )  {
      fcst_data = new float [n];
      obs_data  = new float [n];
   }
   if ( nc_info.do_diff )  diff_data = new float [n];

   // Store the forecast, observation, and difference fields
   for(i=0; i<n; i++) {

      if ( nc_info.do_raw && fcst_data != NULL && obs_data != NULL)  {

         // Set the forecast data
         if(fdata == NULL || is_bad_data(fdata[i])) fcst_data[i] = bad_data_float;
         else                      fcst_data[i] = (float) fdata[i];

         // Set the observation data
         if(odata == NULL || is_bad_data(odata[i])) obs_data[i]  = bad_data_float;
         else                      obs_data[i]  = (float) odata[i];

      }

      if ( nc_info.do_diff && fdata != NULL && odata != NULL && diff_data != NULL)  {
         // Set the difference data
         if(is_bad_data(fdata[i]) ||
            is_bad_data(odata[i]))
            diff_data[i] = bad_data_float;
         else
            diff_data[i] = (float) (fdata[i] - odata[i]);
      }

   } // end for i

   // Retrieve the tile dimension
   d = conf_info.get_tile_dim();

   int dim_count = get_dim_count(&fcst_var);
   long lengths[dim_count];
   long offsets[dim_count];

   offsets[0] = i_tile;
   offsets[1] = 0;
   offsets[2] = 0;

   lengths[0] = 1;
   lengths[1] = d;
   lengths[2] = d;

   if ( nc_info.do_raw )  {
      // Write out the forecast field
      if(!put_nc_data(&fcst_var, &fcst_data[0], lengths, offsets)) {
         mlog << Error << "\nwrite_nc_raw() -> "
              << "error with the fcst_var->put for field "
              << shc.get_fcst_var() << "\n\n";
         exit(1);
      }

      // Write out the observation field
      if(!put_nc_data(&obs_var, &obs_data[0], lengths, offsets)) {
         mlog << Error << "\nwrite_nc_raw() -> "
              << "error with the obs_var->put for field "
              << shc.get_obs_var() << "\n\n";
         exit(1);
      }

   }

   if ( nc_info.do_diff )  {

      // Write out the difference field
      if(!put_nc_data(&diff_var, &diff_data[0], lengths, offsets)) {
         mlog << Error << "\nwrite_nc_raw() -> "
              << "error with the diff_var->put for field "
              << shc.get_fcst_var() << "\n\n";
         exit(1);
      }

   }

   // Deallocate and clean up
   if(fcst_data) { delete [] fcst_data; fcst_data = (float *) 0; }
   if(obs_data)  { delete [] obs_data;  obs_data  = (float *) 0; }
   if(diff_data) { delete [] diff_data; diff_data = (float *) 0; }

   return;

}

////////////////////////////////////////////////////////////////////////

void write_nc_wav(const WaveletStatNcOutInfo & nc_info, const double *fdata, const double *odata, int n,
                  int i_vx, int i_tile, int i_scale,
                  SingleThresh &fcst_st, SingleThresh &obs_st) {
   int i, d;
   float *fcst_data = (float *) 0;
   float *obs_data  = (float *) 0;
   float *diff_data = (float *) 0;
   ConcatString fcst_var_name, obs_var_name, diff_var_name;
   ConcatString fcst_thresh_str, obs_thresh_str;
   ConcatString val;

   // Get the string for the threshold applied
   fcst_thresh_str = fcst_st.get_abbr_str();
   obs_thresh_str  = obs_st.get_abbr_str();

   if ( nc_info.do_raw )  {

      // Build the variable names
      fcst_var_name.format("FCST_%s_%s_%s_%s_%s_%s",
              conf_info.fcst_info[i_vx]->name().text(),
              conf_info.fcst_info[i_vx]->level_name().text(),
              fcst_thresh_str.text(),
              conf_info.obs_info[i_vx]->name().text(),
              conf_info.obs_info[i_vx]->level_name().text(),
              obs_thresh_str.text());
      obs_var_name.format("OBS_%s_%s_%s_%s_%s_%s",
              conf_info.fcst_info[i_vx]->name().text(),
              conf_info.fcst_info[i_vx]->level_name().text(),
              fcst_thresh_str.text(),
              conf_info.obs_info[i_vx]->name().text(),
              conf_info.obs_info[i_vx]->level_name().text(),
              obs_thresh_str.text());
   }

   if ( nc_info.do_diff )  {

      diff_var_name.format("DIFF_%s_%s_%s_%s_%s_%s",
              conf_info.fcst_info[i_vx]->name().text(),
              conf_info.fcst_info[i_vx]->level_name().text(),
              fcst_thresh_str.text(),
              conf_info.obs_info[i_vx]->name().text(),
              conf_info.obs_info[i_vx]->level_name().text(),
              obs_thresh_str.text());

   }

   // If this is the binary field, define new variables
   if(i_tile == 0 && i_scale < 0) {

      int deflate_level = compress_level;
      if (deflate_level < 0) deflate_level = conf_info.get_compression_level();

      // Define the forecast and difference variables

      if ( nc_info.do_raw )  {
         fcst_var = add_var(nc_out, (string)fcst_var_name, ncFloat,
                            tile_dim, scale_dim, x_dim, y_dim, deflate_level);
         obs_var  = add_var(nc_out, (string)obs_var_name,  ncFloat,
                            tile_dim, scale_dim, x_dim, y_dim, deflate_level);
      }

      if ( nc_info.do_diff )  diff_var = add_var(nc_out, (string)diff_var_name, ncFloat,
                                                 tile_dim, scale_dim, x_dim, y_dim, deflate_level);

      if ( nc_info.do_raw )  {

         // Add variable attributes for the observation field
         add_var_att_local(&obs_var, "type", "Observation");
         add_var_att_local(&obs_var, "name", shc.get_obs_var().c_str());
         val.format("%s at %s",
                 conf_info.obs_info[i_vx]->name().text(),
                 conf_info.obs_info[i_vx]->level_name().text());
         add_var_att_local(&obs_var, "long_name", val.c_str());
         add_var_att_local(&obs_var, "level", shc.get_obs_lev().c_str());
         add_var_att_local(&obs_var, "units", conf_info.fcst_info[i_vx]->units().text());
         add_var_att_local(&obs_var, "threshold", fcst_thresh_str.c_str());
         add_var_att_local(&obs_var, "scale_0", "binary");
         add_var_att_local(&obs_var, "scale_n", "scale 2^(n-1)");
         add_att(&obs_var, "_FillValue", bad_data_float);
         write_netcdf_var_times(&obs_var,
            shc.get_obs_valid_beg() - shc.get_obs_lead_sec(),
            shc.get_obs_valid_beg(), 0);
         add_var_att_local(&obs_var, "desc", conf_info.desc[i_vx].c_str());

         // Add variable attributes for the forecast field
         add_var_att_local(&fcst_var, "type", "Forecast");
         add_var_att_local(&fcst_var, "name", shc.get_fcst_var().c_str());
         val.format("%s at %s",
                 conf_info.fcst_info[i_vx]->name().text(),
                 conf_info.fcst_info[i_vx]->level_name().text());
         add_var_att_local(&fcst_var, "long_name", val.c_str());
         add_var_att_local(&fcst_var, "level", shc.get_fcst_lev().c_str());
         add_var_att_local(&fcst_var, "units", conf_info.fcst_info[i_vx]->units().text());
         add_var_att_local(&fcst_var, "threshold", fcst_thresh_str.c_str());
         add_var_att_local(&fcst_var, "scale_0", "binary");
         add_var_att_local(&fcst_var, "scale_n", "scale 2^(n-1)");
         add_att(&fcst_var, "_FillValue", bad_data_float);
         write_netcdf_var_times(&fcst_var,
            shc.get_fcst_valid_beg() - shc.get_fcst_lead_sec(),
            shc.get_fcst_valid_beg(), 0);
         add_var_att_local(&fcst_var, "desc", conf_info.desc[i_vx].c_str());

      }

      if ( nc_info.do_diff )  {

         // Add variable attributes for the difference field
         add_var_att_local(&diff_var, "type", "Difference (F-O)");
         val.format("Forecast %s minus Observed %s",
                 shc.get_fcst_var().text(),
                 shc.get_obs_var().text());
         add_var_att_local(&diff_var, "name", val.c_str());
         val.format("%s at %s and %s at %s",
                 conf_info.fcst_info[i_vx]->name().text(),
                 conf_info.fcst_info[i_vx]->level_name().text(),
                 conf_info.obs_info[i_vx]->name().text(),
                 conf_info.obs_info[i_vx]->level_name().text());
         add_var_att_local(&diff_var, "long_name", val.c_str());
         val.format("%s and %s",
                 shc.get_fcst_lev().text(),
                 shc.get_obs_lev().text());
         add_var_att_local(&diff_var, "level", val.c_str());
         val.format("%s and %s",
                 conf_info.fcst_info[i_vx]->units().text(),
                 conf_info.obs_info[i_vx]->units().text());
         add_var_att_local(&diff_var, "units", val.c_str());
         val.format("%s and %s",
                 fcst_thresh_str.text(),
                 obs_thresh_str.text());
         add_var_att_local(&diff_var, "threshold", val.c_str());
         add_var_att_local(&diff_var, "scale_0", "binary");
         add_var_att_local(&diff_var, "scale_n", "scale 2^(n-1)");
         add_att(&diff_var, "_FillValue", bad_data_float);
         write_netcdf_var_times(&diff_var,
            shc.get_fcst_valid_beg() - shc.get_fcst_lead_sec(),
            shc.get_fcst_valid_beg(), 0);
         add_var_att_local(&diff_var, "desc", conf_info.desc[i_vx].c_str());
      }

   }
   // Otherwise, retrieve the previously defined variables
   else {

      if ( nc_info.do_raw )  {

         fcst_var = get_var(nc_out, fcst_var_name.c_str());
         obs_var  = get_var(nc_out, obs_var_name.c_str());

      }

      if ( nc_info.do_diff )  diff_var = get_var(nc_out, diff_var_name.c_str());

   }

   // Allocate memory for the forecast, observation, and difference
   // fields

   if ( nc_info.do_raw )  {
      fcst_data = new float [n];
      obs_data  = new float [n];
   }
   if ( nc_info.do_diff )  diff_data = new float [n];

   // Store the forecast, observation, and difference fields
   for(i=0; i<n; i++) {

      if ( nc_info.do_raw && fcst_data != NULL && obs_data != NULL)  {

         // Set the forecast data
         if(fdata == NULL || is_bad_data(fdata[i])) fcst_data[i] = bad_data_float;
         else                      fcst_data[i] = (float) fdata[i];

         // Set the observation data
         if(odata == NULL || is_bad_data(odata[i])) obs_data[i]  = bad_data_float;
         else                      obs_data[i]  = (float) odata[i];

      }

      if ( nc_info.do_diff && fdata != NULL && odata != NULL && diff_data != NULL)  {

         // Set the difference data
         if(is_bad_data(fdata[i]) ||
            is_bad_data(odata[i]))
            diff_data[i] = bad_data_float;
         else
            diff_data[i] = (float) (fdata[i] - odata[i]);

      }

   } // end for i

   // Retrieve the tile dimensions
   d = conf_info.get_tile_dim();

   int dim_count = get_dim_count(&fcst_var);
   long lengths[dim_count];
   long offsets[dim_count];

   offsets[0] = i_tile;
   offsets[1] = i_scale+1;
   offsets[2] = 0;
   offsets[3] = 0;
   lengths[0] = 1;
   lengths[1] = 1;
   lengths[2] = d;
   lengths[3] = d;
   if ( nc_info.do_raw )  {

      // Write out the forecast field
      if(!put_nc_data(&fcst_var, &fcst_data[0], lengths, offsets)) {
         mlog << Error << "\nwrite_nc_wav() -> "
              << "error with the fcst_var->put for field "
              << shc.get_fcst_var() << "\n\n";
         exit(1);
      }

      // Write out the observation field
      if(!put_nc_data(&obs_var, &obs_data[0], lengths, offsets)) {
         mlog << Error << "\nwrite_nc_wav() -> "
              << "error with the obs_var->put for field "
              << shc.get_obs_var() << "\n\n";
         exit(1);
      }

   }

   if ( nc_info.do_diff )  {


      // Write out the difference field
      if(!put_nc_data(&diff_var, &diff_data[0], lengths, offsets)) {
         mlog << Error << "\nwrite_nc()_wav -> "
              << "error with the diff_var->put for field "
              << shc.get_fcst_var() << "\n\n";
         exit(1);
      }

   }

   // Deallocate and clean up
   if(fcst_data) { delete [] fcst_data; fcst_data = (float *) 0; }
   if(obs_data)  { delete [] obs_data;  obs_data  = (float *) 0; }
   if(diff_data) { delete [] diff_data; diff_data = (float *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void add_var_att_local(NcVar *var, const char *att_name, const char *att_value) {

   if(att_value) add_att(var, att_name, att_value);
   else          add_att(var, att_name, na_str);

   return;
}

////////////////////////////////////////////////////////////////////////

void close_out_files() {

   // Write out the contents of the STAT AsciiTable and
   // close the STAT output files
   if(stat_out) {
      *stat_out << stat_at;
      close_txt_file(stat_out, stat_file.c_str());
   }

   // Write out the contents of the ISC AsciiTable and
   // close the ISC output files
   if(conf_info.output_flag[i_isc] == STATOutputType_Both) {
      if(isc_out) {
         *isc_out << isc_at;
         close_txt_file(isc_out, isc_file.c_str());
      }
   }

   // Close the output NetCDF file as long as it was opened
   if ( nc_out && !(conf_info.nc_info.all_false()) )  {

      // List the NetCDF file after it is finished
      mlog << Debug(1) << "Output file: " << out_nc_file << "\n";
      //nc_out->close();
      delete nc_out;
      nc_out = (NcFile *) 0;
   }

   // Close the output PSfile as long as it was opened
   if(ps_out && conf_info.ps_plot_flag) {

      // List the PostScript file after it is finished
      mlog << Debug(1) << "Output file: " << out_ps_file << "\n";
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

void plot_ps_raw(const DataPlane &fcst_dp,
                 const DataPlane &obs_dp,
                 const DataPlane &fcst_dp_fill,
                 const DataPlane &obs_dp_fill,
                 int i_vx) {
   ConcatString label;
   ConcatString tmp_str;
   ConcatString fcst_str, fcst_short_str;
   ConcatString obs_str, obs_short_str;
   double v_tab, h_tab_a, h_tab_b;
   double data_min, data_max;
   int i, mon, day, yr, hr, minute, sec;
   Box dim;

   //
   // Compute the min and max data values across both raw fields for use
   // in setting up the color table
   //
   double fcst_min, fcst_max, obs_min, obs_max;
   fcst_dp.data_range(fcst_min, fcst_max);
   obs_dp.data_range(obs_min, obs_max);

   if     (!is_bad_data(fcst_min) && !is_bad_data(obs_min)) raw_plot_min = min(fcst_min, obs_min);
   else if(!is_bad_data(fcst_min) &&  is_bad_data(obs_min)) raw_plot_min = fcst_min;
   else if( is_bad_data(fcst_min) && !is_bad_data(obs_min)) raw_plot_min = obs_min;

   if     (!is_bad_data(fcst_max) && !is_bad_data(obs_max)) raw_plot_max = max(fcst_max, obs_max);
   else if(!is_bad_data(fcst_max) &&  is_bad_data(obs_max)) raw_plot_max = fcst_max;
   else if( is_bad_data(fcst_max) && !is_bad_data(obs_max)) raw_plot_max = obs_max;

   //
   // Setup the x/y bounding box for the data to be plotted.
   //
   set_xy_bb();

   //
   // Setup the plotting dimensions based on the x/y bounding box
   //
   set_plot_dims(nint(xy_bb.width()), nint(xy_bb.height()));

   //
   // Load the raw forecast color table
   //
   tmp_str = replace_path(conf_info.fcst_raw_pi.color_table.c_str());
   mlog << Debug(2) << "Loading forecast raw color table: " << tmp_str << "\n";
   fcst_ct.read(tmp_str.c_str());

   //
   // Load the raw observation color table
   //
   tmp_str = replace_path(conf_info.obs_raw_pi.color_table.c_str());
   mlog << Debug(2) << "Loading observation raw color table: " << tmp_str << "\n";
   obs_ct.read(tmp_str.c_str());

   //
   // Load the wavelet color table
   //
   tmp_str = replace_path(conf_info.wvlt_pi.color_table.c_str());
   mlog << Debug(2) << "Loading wavelet color table: " << tmp_str << "\n";
   wvlt_ct.read(tmp_str.c_str());

   //
   // Compute the min and max data values across both raw fields for use
   // in setting up the color table
   //
   data_min = raw_plot_min;
   data_max = raw_plot_max;

   //
   // If the forecast and observation fields are the same and if the range
   // of both colortables is [0, 1], rescale both colortables to the
   // data_min and data_max values
   //
   if(conf_info.fcst_info[i_vx] == conf_info.obs_info[i_vx] &&
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

         fcst_ct.rescale(fcst_min, fcst_max, bad_data_double);
      }
      if(is_eq(obs_ct.data_min(bad_data_double), 0.0) &&
         is_eq(obs_ct.data_max(bad_data_double), 1.0)) {

         obs_ct.rescale(obs_min, obs_max, bad_data_double);
      }
   }

   //
   // If the fcst_raw_plot_min or fcst_raw_plot_max value is set in the
   // config file, rescale the forecast colortable to the requested range
   //
   if(!is_eq(conf_info.fcst_raw_pi.plot_min, 0.0) ||
      !is_eq(conf_info.fcst_raw_pi.plot_max, 0.0)) {
      fcst_ct.rescale(conf_info.fcst_raw_pi.plot_min,
                      conf_info.fcst_raw_pi.plot_max,
                      bad_data_double);
   }

   //
   // If the obs_raw_plot_min or obs_raw_plot_max value is set in the
   // config file, rescale the observation colortable to the requested range
   //
   if(!is_eq(conf_info.obs_raw_pi.plot_min, 0.0) ||
      !is_eq(conf_info.obs_raw_pi.plot_max, 0.0)) {
      obs_ct.rescale(conf_info.obs_raw_pi.plot_min,
                     conf_info.obs_raw_pi.plot_max,
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

   ////////////////////////////////////////////////////////////////////////////
   //
   // Plot the raw forecast and observation fields
   //
   ////////////////////////////////////////////////////////////////////////////

   ps_out->pagenumber(n_page);

   label.format("Wavelet-Stat: %s vs %s",
                conf_info.fcst_info[i_vx]->magic_str().text(),
                conf_info.obs_info[i_vx]->magic_str().text());

   ps_out->choose_font(31, 24.0);
   ps_out->write_centered_text(1, 1, h_tab_cen, 752.0, 0.5, 0.5, label.c_str());

   fcst_str       = "Forecast";
   fcst_short_str = "Fcst";
   obs_str        = "Observation";
   obs_short_str  = "Obs";

   ps_out->choose_font(31, 18.0);
   ps_out->write_centered_text(1, 1, h_tab_1, 727.0, 0.5, 0.5,
                               fcst_str.c_str());
   ps_out->write_centered_text(1, 1, h_tab_3, 727.0, 0.5, 0.5,
                               obs_str.c_str());

   ////////////////////////////////////////////////////////////////////////////
   //
   // Draw raw forecast field
   //
   ////////////////////////////////////////////////////////////////////////////

   set_dim(dim, v_tab_1, v_tab_1 + sm_plot_height, h_tab_1);
   render_image(ps_out, fcst_dp, dim, 1);
   draw_map(ps_out, dim);
   draw_border(ps_out, dim);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Draw the colorbar
   //
   ////////////////////////////////////////////////////////////////////////////

   draw_colorbar(ps_out, dim, 1, 1);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Draw raw observation field
   //
   ////////////////////////////////////////////////////////////////////////////

   set_dim(dim, v_tab_1, v_tab_1 + sm_plot_height, h_tab_3);
   render_image(ps_out, obs_dp, dim, 0);
   draw_map(ps_out, dim);
   draw_border(ps_out, dim);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Draw raw forecast fill field with the tiles overlaid
   //
   ////////////////////////////////////////////////////////////////////////////

   set_dim(dim, v_tab_2, v_tab_2 + sm_plot_height, h_tab_1);
   render_image(ps_out, fcst_dp_fill, dim, 1);
   draw_map(ps_out, dim);
   draw_border(ps_out, dim);
   draw_tiles(ps_out, dim, 0, conf_info.get_n_tile()-1, 1);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Draw the colorbar
   //
   ////////////////////////////////////////////////////////////////////////////

   draw_colorbar(ps_out, dim, 0, 1);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Draw raw observation fill field with the tiles overlaid
   //
   ////////////////////////////////////////////////////////////////////////////

   set_dim(dim, v_tab_2, v_tab_2 + sm_plot_height, h_tab_3);
   render_image(ps_out, obs_dp_fill, dim, 0);
   draw_map(ps_out, dim);
   draw_border(ps_out, dim);
   draw_tiles(ps_out, dim, 0, conf_info.get_n_tile()-1, 1);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Annotate the page
   //
   ////////////////////////////////////////////////////////////////////////////

   ps_out->choose_font(31, 12.0);

   v_tab = v_tab_2 - 1.0*plot_text_sep;
   h_tab_a = h_tab_1 - 0.5*dim.width();
   h_tab_b = h_tab_a + 5.0*plot_text_sep;

   //
   // Model Name
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Model Name:");
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5,
                               conf_info.model.c_str());
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
   unix_to_mdyhms(fcst_dp.valid() - fcst_dp.lead(),
                  mon, day, yr, hr, minute, sec);
   label.format("%s %i, %i %.2i:%.2i:%.2i",
           short_month_name[mon], day, yr, hr, minute, sec);
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label.c_str());
   v_tab -= plot_text_sep;

   //
   // Valid time
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Valid Time:");
   unix_to_mdyhms(fcst_dp.valid(),
                  mon, day, yr, hr, minute, sec);
   label.format("%s %i, %i %.2i:%.2i:%.2i",
           short_month_name[mon], day, yr, hr, minute, sec);
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label.c_str());
   v_tab -= plot_text_sep;

   //
   // Lead time
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Lead Time:");
   label = sec_to_hhmmss_colon(fcst_dp.lead());
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label.c_str());
   v_tab -= plot_text_sep;

   //
   // Accumulation time
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Accum Time:");
   label = sec_to_hhmmss_colon(fcst_dp.accum());
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label.c_str());
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
   label = griddecomptype_to_string(conf_info.grid_decomp_flag);
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label.c_str());
   v_tab -= plot_text_sep;

   //
   // Tile count
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Tile Count:");
   label.format("%i", conf_info.get_n_tile());
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label.c_str());
   v_tab -= plot_text_sep;

   //
   // Tile dimension
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Tile Dim:");
   label.format("%i x %i",
           conf_info.get_tile_dim(),
           conf_info.get_tile_dim());
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label.c_str());
   v_tab -= plot_text_sep;

   //
   // Tile corners
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Tile Corner:");
   for(i=0; i<conf_info.get_n_tile(); i++) {
      label.format("(%i, %i) ",
              nint(conf_info.tile_xll[i]),
              nint(conf_info.tile_yll[i]));
   }
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label.c_str());
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
   label = fieldtype_to_string(conf_info.mask_missing_flag);
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label.c_str());
   v_tab -= plot_text_sep;

   //
   // Wavelet type
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Wavelet(k):");
   label = wavelettype_to_string(conf_info.wvlt_type);
   label << "(" << conf_info.wvlt_member << ")";
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5, label.c_str());
   v_tab -= plot_text_sep;

   ps_out->showpage();
   n_page++;

   return;
}

////////////////////////////////////////////////////////////////////////

void plot_ps_wvlt(const double *diff, int n, int i_vx, int i_tile,
                  ISCInfo &isc_info,
                  int i_scale, int n_scale) {
   ConcatString label;
   ConcatString fcst_thresh_str, obs_thresh_str;
   Box dim;
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
   if(!is_eq(conf_info.wvlt_pi.plot_min, 0.0) ||
      !is_eq(conf_info.wvlt_pi.plot_max, 0.0)) {
      wvlt_ct.rescale(conf_info.wvlt_pi.plot_min,
                      conf_info.wvlt_pi.plot_max,
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

   fcst_thresh_str = isc_info.fthresh.get_str();
   obs_thresh_str  = isc_info.othresh.get_str();

   label.format("%s %s vs %s %s",
                conf_info.fcst_info[i_vx]->magic_str().text(),
                fcst_thresh_str.text(),
                conf_info.obs_info[i_vx]->magic_str().text(),
                obs_thresh_str.text());

   ps_out->choose_font(31, 24.0);
   v_tab -= 1.0*plot_text_sep;
   ps_out->write_centered_text(1, 1, h_tab_cen, v_tab, 0.5, 0.5, label.c_str());
   if(i_scale == -1)
      label.format("Tile %i, Binary, Difference (F-0)",
              i_tile+1);
   else
      label.format("Tile %i, Scale %i, Difference (F-0)",
              i_tile+1, i_scale+1);

   v_tab -= 2.0*plot_text_sep;
   ps_out->write_centered_text(1, 1, h_tab_cen, v_tab, 0.5, 0.5,
                               label.c_str());

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
   draw_colorbar(ps_out, dim, 0, 0);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Write the scores for current scale
   //
   ////////////////////////////////////////////////////////////////////////////

   ps_out->choose_font(31, 12.0);

   v_tab -= lg_plot_height;
   v_tab -= plot_text_sep;

   h_tab_a = 2.0*plot_text_sep;
   h_tab_b = h_tab_a + 10.0*plot_text_sep;

   h_tab_c = h_tab_cen + 2.0*plot_text_sep;
   h_tab_d = h_tab_c + 10.0*plot_text_sep;

   //
   // Frequency Bias
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Frequency Bias:");
   label.format("%.5f", isc_info.fbias);
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5,
                               label.c_str());
   v_tab -= plot_text_sep;

   //
   // Base Rate
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Base Rate:");
   label.format("%.5f", isc_info.baser);
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5,
                               label.c_str());
   v_tab -= plot_text_sep;

   //
   // Mean-Squared Error (percentage of total)
   //
   ps_out->write_centered_text(1, 1, h_tab_a, v_tab, 0.0, 0.5,
                               "Mean-Squared Error (%):");
   if(i_scale < 0) {
      p = compute_percentage(isc_info.mse, isc_info.mse);
      label.format("%.5f (%.2f)", isc_info.mse, p);
   }
   else {
      p = compute_percentage(isc_info.mse_scale[i_scale], isc_info.mse);
      label.format("%.5f (%.2f)", isc_info.mse_scale[i_scale], p);
   }
   ps_out->write_centered_text(1, 1, h_tab_b, v_tab, 0.0, 0.5,
                               label.c_str());
   v_tab -= plot_text_sep;

   //
   // Intensity-Scale Skill Score
   //
   v_tab += 3.0*plot_text_sep;
   ps_out->write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5,
                               "Intensity Skill Score:");
   if(i_scale < 0) label.format("%.5f", isc_info.isc);
   else            label.format("%.5f", isc_info.isc_scale[i_scale]);
   ps_out->write_centered_text(1, 1, h_tab_d, v_tab, 0.0, 0.5,
                               label.c_str());
   v_tab -= plot_text_sep;

   //
   // Forecast Energy Squared (percentage of total)
   //
   ps_out->write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5,
                               "Fcst Energy Squared (%):");
   if(i_scale < 0) {
      p = compute_percentage(isc_info.fen, isc_info.fen);
      label.format("%.5f (%.2f)", isc_info.fen, p);
   }
   else {
      p = compute_percentage(isc_info.fen_scale[i_scale], isc_info.fen);
      label.format("%.5f (%.2f)", isc_info.fen_scale[i_scale], p);
   }
   ps_out->write_centered_text(1, 1, h_tab_d, v_tab, 0.0, 0.5,
                               label.c_str());
   v_tab -= plot_text_sep;

   //
   // Observation Energy Squared (percentage of total)
   //
   ps_out->write_centered_text(1, 1, h_tab_c, v_tab, 0.0, 0.5,
                               "Obs Energy Squared (%):");
   if(i_scale < 0) {
      p = compute_percentage(isc_info.oen, isc_info.oen);
      label.format("%.5f (%.2f)", isc_info.oen, p);
   }
   else {
      p = compute_percentage(isc_info.oen_scale[i_scale], isc_info.oen);
      label.format("%.5f (%.2f)", isc_info.oen_scale[i_scale], p);
   }
   ps_out->write_centered_text(1, 1, h_tab_d, v_tab, 0.0, 0.5,
                               label.c_str());
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

   w = full_pane_bb.width();

   grid_ar = (double) nx/ny;
   plot_ar = ((w - clrbar_width)/2.0)/(full_pane_bb.height()/3.0);

   if(grid_ar > plot_ar) {
      sm_plot_height = full_pane_bb.height()/3.0*plot_ar/grid_ar;
      lg_plot_height = full_pane_bb.height()/3.0*plot_ar/grid_ar;
   }
   else {
      sm_plot_height = full_pane_bb.height()/3.0;
      lg_plot_height = full_pane_bb.height()/3.0;
   }

   // First plot
   h_tab_1 = full_pane_bb.x_ll() + (w - clrbar_width)/4.0;
   // Colorbar plot
   h_tab_2 = full_pane_bb.x_ll() + (w - clrbar_width)/4.0
             + clrbar_width;
   // Second plot
   h_tab_3 = full_pane_bb.x_ll() + (w - clrbar_width)/4.0*3.0
             + clrbar_width;

   v_tab_1 = full_pane_bb.y_ur() - sm_plot_height;
   v_tab_2 = v_tab_1 - sm_plot_height;
   v_tab_3 = v_tab_2 - sm_plot_height;

   return;
}

////////////////////////////////////////////////////////////////////////

void set_xy_bb() {

   //
   // Check if padding was performed
   //
   if(!is_eq(conf_info.pad_bb.x_ll(), 0.0) &&
      !is_eq(conf_info.pad_bb.y_ll(), 0.0)) {

      double x_ll = 0 - conf_info.pad_bb.x_ll();
      double y_ll = 0 - conf_info.pad_bb.y_ll();

      xy_bb.set_lrbt ( x_ll, x_ll + conf_info.get_tile_dim(),
                       y_ll, y_ll + conf_info.get_tile_dim() );

   }
   else {

      xy_bb.set_lrbt(0, grid.nx(), 0, grid.ny());

   }

   return;
}

////////////////////////////////////////////////////////////////////////

void set_dim(Box &dim, double y_ll, double y_ur, double x_cen) {
   double width;

   /*
   dim.y_ll   = y_ll;
   dim.y_ur   = y_ur;
   dim.height = dim.y_ur - dim.y_ll;
   mag        = dim.height/xy_bb.height;
   dim.width  = mag*xy_bb.width;
   dim.x_ll   = x_cen - 0.5*dim.width;
   dim.x_ur   = x_cen + 0.5*dim.width;
   */

   width = ( (y_ur - y_ll) / xy_bb.height() ) * xy_bb.width();
   dim.set_lrbt(x_cen - 0.5*width, x_cen + 0.5*width, y_ll, y_ur);

   return;
}

////////////////////////////////////////////////////////////////////////

void draw_colorbar(PSfile *p, Box &dim, int fcst, int raw) {
   int i;
   char label[max_str_len];
   double bar_width, bar_height, x_ll, y_ll, step, v;
   ColorTable *ct_ptr = (ColorTable *) 0;
   Color c;

   //
   // Set up the pointer to the appropriate colortable
   //
   if     (raw == 1 && fcst == 1) ct_ptr = &fcst_ct;
   else if(raw == 1 && fcst == 0) ct_ptr = &obs_ct;
   else                           ct_ptr = &wvlt_ct;

   //
   // Draw colorbar in the bottom-right corner of the Bounding Box
   //

   if ( use_flate )  p->begin_flate();

   p->gsave();
   p->setlinewidth(l_width);
   p->choose_font(28, 8.0);

   bar_width = h_margin;
   bar_height = (dim.y_ur() - dim.y_ll())/(n_color_bars + 1);

   x_ll = dim.x_ur();
   y_ll = dim.y_ll();

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
     snprintf(label, sizeof(label), "%.1f", v);
     p->write_centered_text(2, 1,  x_ll + 0.5*bar_width,
                            y_ll + 0.5*bar_height, 0.5, 0.5, label);

     v    += step;
     y_ll += bar_height;
   }

   p->grestore();

   if ( use_flate )  p->end_flate();

  return;
}

////////////////////////////////////////////////////////////////////////

void draw_border(PSfile *p, Box &dim) {

   p->gsave();
   p->setlinewidth(l_width);
   p->newpath();
   p->moveto(dim.x_ll(), dim.y_ll());
   p->lineto(dim.x_ur(), dim.y_ll());
   p->lineto(dim.x_ur(), dim.y_ur());
   p->lineto(dim.x_ll(), dim.y_ur());
   p->closepath();
   p->stroke();
   p->grestore();

   return;
}

////////////////////////////////////////////////////////////////////////

void draw_map(PSfile *p, Box &dim) {

   if ( use_flate )  p->begin_flate();

   p->gsave();
   p->setlinewidth(l_width);
   draw_map(grid, xy_bb, *p, dim, &conf_info.conf);
   p->grestore();

   if ( use_flate )  p->end_flate();

   return;
}

////////////////////////////////////////////////////////////////////////

void draw_tiles(PSfile *p, Box &dim,
                int tile_start, int tile_end, int label_flag) {
   int i;
   double page_x, page_y;
   char label[128];
   Box tile_bb;
   double bb_x, bb_y;

   p->gsave();

   // Loop through the tiles to be applied
   for(i=tile_start; i<=tile_end; i++) {

      // If padding was performed, the tile is the size of the domain
      if(conf_info.grid_decomp_flag == GridDecompType_Pad) {
         tile_bb = dim;
      }
      // Find the lower-left and upper-right corners of the tile
      else {

         // Compute the page x/y coordinates for this tile
         gridxy_to_pagexy(grid, xy_bb,
            conf_info.tile_xll[i],
            conf_info.tile_yll[i],
            bb_x, bb_y, dim);
         tile_bb.set_llwh(bb_x, bb_y, tile_bb.width(), tile_bb.height());

         gridxy_to_pagexy(grid, xy_bb,
            conf_info.tile_xll[i] + conf_info.get_tile_dim(),
            conf_info.tile_yll[i] + conf_info.get_tile_dim(),
            bb_x, bb_y, dim);
         tile_bb.set_lrbt(tile_bb.x_ll(), bb_x, tile_bb.y_ll(), bb_y);
      }

      // Draw the border for this tile
      p->setrgbcolor(1.0, 0.0, 0.0);
      p->setlinewidth(l_width_thick);
      p->newpath();
      p->moveto(tile_bb.x_ll(), tile_bb.y_ll());
      p->lineto(tile_bb.x_ur(), tile_bb.y_ll());
      p->lineto(tile_bb.x_ur(), tile_bb.y_ur());
      p->lineto(tile_bb.x_ll(), tile_bb.y_ur());
      p->closepath();
      p->stroke();

      if(label_flag) {
         // Draw diagonals for this tile
         p->gsave();
         p->setlinewidth(l_width);
         p->file() << "[2] 2 setdash";
         p->moveto(tile_bb.x_ll(), tile_bb.y_ll());
         p->lineto(tile_bb.x_ur(), tile_bb.y_ur());
         p->stroke();

         p->moveto(tile_bb.x_ur(), tile_bb.y_ll());
         p->lineto(tile_bb.x_ll(), tile_bb.y_ur());
         p->stroke();
         p->grestore();

         // Plot the tile number in the center
         p->setlinewidth(0.0);

         page_x = (tile_bb.x_ll() + tile_bb.x_ur())/2.0,
         page_y = (tile_bb.y_ll() + tile_bb.y_ur())/2.0,

         p->choose_font(28, 20.0);
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

void render_image(PSfile *p, const DataPlane &dp, Box &dim, int fcst) {
   RenderInfo render_info;
   Ppm ppm_image;
   int x, y, grid_x, grid_y;
   double mag;
   Color c;
   Color *c_fill_ptr = (Color *) 0;
   ColorTable *ct_ptr = (ColorTable *) 0;

   //
   // Set up pointers to the appropriate colortable and fill color
   // values.
   //
   if(fcst == 1) {
      ct_ptr     = &fcst_ct;
      c_fill_ptr = &c_fcst_fill;
   }
   else {
      ct_ptr     = &obs_ct;
      c_fill_ptr = &c_obs_fill;
   }

   //
   // Convert the DataPlane object to PPM
   //
   ppm_image.set_size_xy((int) xy_bb.width(), (int) xy_bb.height());

   //
   // Check if the x,y plotting region matches the data dimensions.
   //
   if(nint(xy_bb.width())  == dp.nx() &&
      nint(xy_bb.height()) == dp.ny()) {

      for(x=0; x<dp.nx(); x++) {
         for(y=0; y<dp.ny(); y++) {

            if(is_bad_data(dp.get(x, y)))
               c = *c_fill_ptr;
            else
               c = ct_ptr->nearest(dp.get(x, y));

            ppm_image.putxy(c, x, y);
         }
      }
   }
   //
   // If they don't match, plot the pad
   //
   else {

      for(x=nint(xy_bb.x_ll()); x<xy_bb.x_ur(); x++) {
         for(y=nint(xy_bb.y_ll()); y<xy_bb.y_ur(); y++) {

            //
            // Compute grid relative x, y values
            //
            grid_x = nint(x + xy_bb.x_ll());
            grid_y = nint(y + xy_bb.y_ll());

            if(grid_x < 0 || grid_x >= dp.nx() ||
               grid_y < 0 || grid_y >= dp.ny()) continue;

            if(is_bad_data(dp.get(grid_x, grid_y)))
               c = *c_fill_ptr;
            else
               c = ct_ptr->nearest(dp.get(grid_x, grid_y));

            ppm_image.putxy(c, x, y);
         }
      }
   }

   mag = (dim.x_ur() - dim.x_ll())/xy_bb.width();

   render_info.set_ll(dim.x_ll(), dim.y_ll());
   render_info.set_mag(mag);
   render_info.set_color();

   if ( use_flate )  render_info.add_filter(FlateEncode);
   else              render_info.add_filter(RunLengthEncode);

   render_info.add_filter(ASCII85Encode);
   render(*p, ppm_image, render_info);

   return;
}

////////////////////////////////////////////////////////////////////////

void render_tile(PSfile *p, const double *data, int n, int i_tile,
                 Box &dim) {
   RenderInfo render_info;
   Ppm ppm_image;
   int i, x, y;
   double mag;
   Color c;
   Color *c_fill_ptr = (Color *) 0;
   ColorTable *ct_ptr = (ColorTable *) 0;

   //
   // Set up pointers to the appropriate colortable and fill color
   // values.
   //
   ct_ptr = &wvlt_ct;
   c_fill_ptr = &c_wvlt_fill;

   //
   // Convert the DataPlane object to PPM
   //
   ppm_image.set_size_xy((int) xy_bb.width(), (int) xy_bb.height());

   for(i=0; i<n; i++) {

      x = nint(conf_info.tile_xll[i_tile] + i%conf_info.get_tile_dim());
      y = nint(conf_info.tile_yll[i_tile] + i/conf_info.get_tile_dim());

      if(is_bad_data(data[i]))
         c = *c_fill_ptr;
      else
         c = ct_ptr->nearest(data[i]);

      ppm_image.putxy(c, x, y);
   }

   mag = (dim.x_ur() - dim.x_ll())/xy_bb.width();

   render_info.set_ll(dim.x_ll(), dim.y_ll());
   render_info.set_mag(mag);
   render_info.set_color();

   if ( use_flate )  render_info.add_filter(FlateEncode);
   else              render_info.add_filter(RunLengthEncode);

   render_info.add_filter(ASCII85Encode);
   render(*p, ppm_image, render_info);

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tfcst_file\n"
        << "\tobs_file\n"
        << "\tconfig_file\n"
        << "\t[-outdir path]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"fcst_file\" is a gridded forecast file containing "
        << "the field(s) to be verified (required).\n"

        << "\t\t\"obs_file\" is a gridded observation file containing "
        << "the verifying field(s) (required).\n"

        << "\t\t\"config_file\" is a WaveletStatConfig file containing "
        << "the desired configuration settings (required).\n"

        << "\t\t\"-outdir path\" overrides the default output directory ("
        << default_out_dir << ") (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t\"-compress level\" overrides the compression level of NetCDF variable ("
        << conf_info.get_compression_level() << ") (optional).\n\n" << flush;

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_outdir(const StringArray & a)
{
   out_dir = a[0];
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
