// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   grid_stat.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    04/18/07  Halley Gotway   New
//   001    10/05/07  Halley Gotway   Add checks to only compute
//                    confidence intervals for sufficiently large
//                    sample sizes.
//   002    10/19/07  Halley Gotway   Add ability to smooth the forecast
//                    field using the interp_method and interp_width
//                    parameters
//   003    01/11/08  Halley Gotway   Modify sprintf statements which
//                    use the GRIB code abbreviation string
//   004    02/06/08  Halley Gotway   Modify to read the updated NetCDF
//                    output of PCP-Combine
//   005    02/11/08  Halley Gotway   Remove BIAS from the CNT line
//                    since it's the same as the ME.
//   006    02/12/08  Halley Gotway   Fix bug in writing COP line to
//                    write out OY and ON rather than FY and FN.
//   007    02/14/08  Halley Gotway   Apply the smoothing operation to
//                    both forecast and observation fields rather than
//                    just the forecast.
//   008    02/25/08  Halley Gotway   Write the STAT lines using the
//                    routines in the vx_met_util library.
//   009    02/25/08  Halley Gotway   Add support for the NBRCTS and
//                    NBRCNT line types.
//   010    07/01/08  Halley Gotway   Add the rank_corr_flag to the
//                    config file to disable computing rank
//                    correlations.
//   011    09/23/08  Halley Gotway  Change argument sequence for the
//                    get_grib_record routine.
//   012    10/31/08  Halley Gotway  Use the get_file_type routine to
//                    determine the input file types.
//   013    11/14/08  Halley Gotway  Restructure organization of Grid-
//                    Stat and move several functions to libraries.
//   014    03/13/09  Halley Gotway  Add support for verifying
//                    probabilistic forecasts.
//   015    10/22/09  Halley Gotway  Fix output_flag cut and paste bug.
//   016    05/03/10  Halley Gotway  Don't write duplicate NetCDF
//                    matched pair variables or probabilistic
//                    difference fields.
//   017    05/06/10  Halley Gotway  Add interp_flag config parameter.
//   018    05/27/10  Halley Gotway  Add -fcst_valid, -fcst_lead,
//                    -obs_valid, and -obs_lead command line options.
//   019    06/08/10  Halley Gotway  Add support for multi-category
//                    contingency tables.
//   020    06/30/10  Halley Gotway  Enhance grid equality checks.
//   021    07/27/10  Halley Gotway  Add lat/lon variables to NetCDF.
//   022    10/28/11  Holmes         Added use of command line class to
//                    parse the command line arguments.
//   023    11/14/11  Holmes         Added code to enable reading of
//                    multiple config files.
//   024    12/09/11  Halley Gotway  When zero matched pairs are found
//                    print the status message before continuing.
//   025    02/02/12  Bullock        Changed default output directory to "."
//   026    04/30/12  Halley Gotway  Switch to using vx_config library.
//   027    04/30/12  Halley Gotway  Move -fcst_valid, -fcst_lead,
//                    -obs_valid, and -obs_lead command line options
//                    to config file.
//   028    08/21/13  Halley Gotway  Fix sizing of output tables for 12
//                    or more probabilstic thresholds.
//   029    05/20/14  Halley Gotway  Add AFSS, UFSS, F_RATE, and O_RATE
//                    to the NBRCNT line type.
//   030    11/12/14  Halley Gotway  Pass the obtype entry from the
//                    from the config file to the output files.
//   031    02/19/15  Halley Gotway  Add automated regridding.
//   032    08/04/15  Halley Gotway  Add conditional continuous verification.
//   033    09/04/15  Halley Gotway  Add climatology and SAL1L2 and VAL1L2
//                                   output line types.
//   034    05/10/16  Halley Gotway  Add grid weighting.
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

#include "grid_stat.h"

#include "vx_statistics.h"
#include "vx_nc_util.h"
#include "vx_regrid.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void setup_first_pass    (const DataPlane &);

static void setup_txt_files(unixtime, int);
static void setup_table    (AsciiTable &);
static void setup_nc_file  (const GridStatNcOutInfo &, unixtime, int);

static void build_outfile_name(unixtime, int, const char *,
                               ConcatString &);

static void process_scores();

static void do_cts   (CTSInfo *&, int,
                      const NumArray &, const NumArray &);
static void do_mcts  (MCTSInfo &, int,
                      const NumArray &, const NumArray &);
static void do_cnt   (CNTInfo *&, int,
                      const NumArray &, const NumArray &, const NumArray &);
static void do_sl1l2 (SL1L2Info *&, int,
                      const NumArray &, const NumArray &, const NumArray &);
static void do_vl1l2 (VL1L2Info *&, int,
                      const NumArray &, const NumArray &,
                      const NumArray &, const NumArray &,
                      const NumArray &, const NumArray &);
static void do_pct   (PCTInfo *&, int,
                      const NumArray &, const NumArray &, const NumArray &);
static void do_nbrcts(NBRCTSInfo *&, int, int, int,
                      const NumArray &, const NumArray &);
static void do_nbrcnt(NBRCNTInfo &, int, int, int,
                      const NumArray &, const NumArray &,
                      const NumArray &, const NumArray &);

static void write_nc(const GridStatNcOutInfo &,
                     const DataPlane &, const DataPlane &,
                     const DataPlane &, int, InterpMthd, int);

static void write_nbrhd_nc(const GridStatNcOutInfo &,
                           const DataPlane &, const DataPlane &, int,
                           const SingleThresh &, const SingleThresh &,
                           int);

static void add_var_att(NcVar *, const char *, const char *);

static void finish_txt_files();

static void clean_up();

static void usage();
static void set_outdir(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);

static void show_version(const StringArray &);

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

   // Set the default output directory
   out_dir = replace_path(default_out_dir);

   // Check for zero arguments
   if(argc == 1) usage();

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_outdir,    "-outdir", 1);
   cline.add(set_logfile,   "-log",    1);
   cline.add(set_verbosity, "-v",      1);

   cline.add(show_version,  "-version", 0);

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
   conf_info.read_config(default_config_file, config_file);

   // Get the forecast and observation file types from config, if present
   ftype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_fcst));
   otype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_obs));

   // Read forecast file
   if(!(fcst_mtddf = mtddf_factory.new_met_2d_data_file(fcst_file, ftype))) {
      mlog << Error << "\nTrouble reading forecast file \""
           << fcst_file << "\"\n\n";
      exit(1);
   }

   // Read observation file
   if(!(obs_mtddf = mtddf_factory.new_met_2d_data_file(obs_file, otype))) {
      mlog << Error << "\nTrouble reading observation file \""
           << obs_file << "\"\n\n";
      exit(1);
   }

   // Store the input data file types
   ftype = fcst_mtddf->file_type();
   otype = obs_mtddf->file_type();

   // Process the configuration
   conf_info.process_config(ftype, otype);

   // Determine the verification grid
   grid = parse_vx_grid(conf_info.regrid_info,
                        &(fcst_mtddf->grid()), &(obs_mtddf->grid()));

   // Compute weight for each grid point
   parse_grid_wgt(grid, wgt_dp);

   // Set the model name
   shc.set_model(conf_info.model);

   // Set the obtype column
   shc.set_obtype(conf_info.obtype);

   // Set the random number generator and seed value to be used when
   // computing bootstrap confidence intervals
   rng_set(rng_ptr, conf_info.boot_rng, conf_info.boot_seed);

   // List the input files
   mlog << Debug(1)
        << "Forecast File: "    << fcst_file << "\n"
        << "Observation File: " << obs_file  << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_first_pass(const DataPlane &dp) {

   // Unset the flag
   is_first_pass = false;

   // Process masks grids and polylines in the config file
   conf_info.process_masks(grid);

   // Create output text files as requested in the config file
   setup_txt_files(dp.valid(), dp.lead());

   // If requested, create a NetCDF file to store the matched pairs and
   // difference fields for each GRIB code and masking region
   if(!(conf_info.nc_info.all_false())) {
      setup_nc_file(conf_info.nc_info, dp.valid(), dp.lead());
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_txt_files(unixtime valid_ut, int lead_sec) {
   int  i, max_col, max_prob_col, max_mctc_col, n_prob, n_cat;
   ConcatString base_name;

   // Create output file names for the stat file and optional text files
   build_outfile_name(valid_ut, lead_sec, "", base_name);

   /////////////////////////////////////////////////////////////////////
   //
   // Setup the output STAT file
   //
   /////////////////////////////////////////////////////////////////////

   // Get the maximum number of data columns
   n_prob = conf_info.get_max_n_fprob_thresh();
   n_cat  = conf_info.get_max_n_cat_thresh() + 1;

   max_prob_col = get_n_pjc_columns(n_prob);
   max_mctc_col = get_n_mctc_columns(n_cat);

   // Determine the maximum number of data columns
   max_col = ( max_prob_col > max_stat_col ? max_prob_col : max_stat_col );
   max_col = ( max_mctc_col > max_col      ? max_mctc_col : max_col );

   // Add the header columns
   max_col += n_header_columns + 1;

   // Initialize file stream
   stat_out = (ofstream *) 0;

   // Build the file name
   stat_file << base_name << stat_file_ext;

   // Create the output STAT file
   open_txt_file(stat_out, stat_file);

   // Setup the STAT AsciiTable
   stat_at.set_size(conf_info.n_stat_row() + 1, max_col);
   setup_table(stat_at);

   // Write the text header row
   write_header_row((const char **) 0, 0, 1, stat_at, 0, 0);

   // Initialize the row index to 1 to account for the header
   i_stat_row = 1;

   /////////////////////////////////////////////////////////////////////
   //
   // Setup each of the optional output text files
   //
   /////////////////////////////////////////////////////////////////////

   // Loop through output file type
   for(i=0; i<n_txt; i++) {

      // Only set it up if requested in the config file
      if(conf_info.output_flag[i] == STATOutputType_Both) {

         // Initialize file stream
         txt_out[i] = (ofstream *) 0;

         // Build the file name
         txt_file[i] << base_name << "_" << txt_file_abbr[i]
                     << txt_file_ext;

         // Create the output text file
         open_txt_file(txt_out[i], txt_file[i]);

         // Get the maximum number of columns for this line type
         switch(i) {

            case(i_mctc):
               max_col = get_n_mctc_columns(n_cat) + n_header_columns + 1;
               break;

            case(i_pct):
               max_col = get_n_pct_columns(n_prob)  + n_header_columns + 1;
               break;

            case(i_pstd):
               max_col = get_n_pstd_columns(n_prob) + n_header_columns + 1;
               break;

            case(i_pjc):
               max_col = get_n_pjc_columns(n_prob)  + n_header_columns + 1;
               break;

            case(i_prc):
               max_col = get_n_prc_columns(n_prob)  + n_header_columns + 1;
               break;

            default:
               max_col = n_txt_columns[i]  + n_header_columns + 1;
               break;
         } // end switch

         // Setup the text AsciiTable
         txt_at[i].set_size(conf_info.n_txt_row(i) + 1, max_col);
         setup_table(txt_at[i]);

         // Write the text header row
         switch(i) {

            case(i_mctc):
               write_mctc_header_row(1, n_cat, txt_at[i], 0, 0);
               break;

            case(i_pct):
               write_pct_header_row(1, n_prob, txt_at[i], 0, 0);
               break;

            case(i_pstd):
               write_pstd_header_row(1, n_prob, txt_at[i], 0, 0);
               break;

            case(i_pjc):
               write_pjc_header_row(1, n_prob, txt_at[i], 0, 0);
               break;

            case(i_prc):
               write_prc_header_row(1, n_prob, txt_at[i], 0, 0);
               break;

            default:
               write_header_row(txt_columns[i], n_txt_columns[i], 1,
                  txt_at[i], 0, 0);
               break;
         } // end switch

         // Initialize the row index to 1 to account for the header
         i_txt_row[i] = 1;
      }
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

void setup_nc_file(const GridStatNcOutInfo & nc_info,
                   unixtime valid_ut, int lead_sec) {

   // Create output NetCDF file name
   build_outfile_name(valid_ut, lead_sec, "_pairs.nc", out_nc_file);

   // Create a new NetCDF file and open it
   nc_out = new NcFile(out_nc_file, NcFile::Replace);

   if(!nc_out->is_valid()) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_nc_file << "\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, out_nc_file, program_name, conf_info.model, conf_info.obtype);
   if(nc_info.do_diff) {
      nc_out->add_att("Difference", "Forecast Value - Observation Value");
   }

   // Add the projection information
   write_netcdf_proj(nc_out, grid);

   // Define Dimensions
   lat_dim = nc_out->add_dim("lat", (long) grid.ny());
   lon_dim = nc_out->add_dim("lon", (long) grid.nx());

   // Add the lat/lon variables
   if(nc_info.do_latlon) {
      write_netcdf_latlon(nc_out, lat_dim, lon_dim, grid);
   }

   // Add grid weight variable
   if(nc_info.do_weight) {
      write_netcdf_grid_wgt(nc_out, lat_dim, lon_dim, wgt_dp);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void build_outfile_name(unixtime valid_ut, int lead_sec,
                        const char *suffix, ConcatString &str) {
   int mon, day, yr, hr, min, sec;
   int l_hr, l_min, l_sec;
   ConcatString date_str;

   //
   // Create output file name
   //

   // Append the output directory and program name
   str << cs_erase << out_dir << "/" << program_name;

   // Append the output prefix, if defined
   if(conf_info.output_prefix.nonempty())
      str << "_" << conf_info.output_prefix;

   // Append the timing information
   sec_to_hms(lead_sec, l_hr, l_min, l_sec);
   unix_to_mdyhms(valid_ut, mon, day, yr, hr, min, sec);
   date_str.format("%.2i%.2i%.2iL_%.4i%.2i%.2i_%.2i%.2i%.2iV",
           l_hr, l_min, l_sec, yr, mon, day, hr, min, sec);
   str << "_" << date_str;

   // Append the suffix
   str << suffix;

   return;
}

////////////////////////////////////////////////////////////////////////

void process_scores() {
   int i, j, k, m, n;
   bool status;
   int n_cat, n_cnt, n_wind, n_prob, n_cov;

   // Forecast and observation fields
   DataPlane mask_dp;
   DataPlane fcst_dp,        obs_dp;
   DataPlane fcst_dp_smooth, obs_dp_smooth;
   DataPlane fcst_dp_thresh, obs_dp_thresh;

   // Climatology mean
   DataPlane cmn_dp;

   // Forecast, observation, climatology, and weights
   NumArray f_na, o_na, c_na, w_na;

   // Thresholded fractional coverage pairs
   NumArray fthr_na, othr_na;

   // Objects to handle vector winds
   DataPlane fu_dp, ou_dp;
   DataPlane fu_dp_smooth, ou_dp_smooth;
   DataPlane cmnu_dp;
   NumArray fu_na, ou_na, cmnu_na;

   CTSInfo    *cts_info    = (CTSInfo *) 0;
   MCTSInfo    mcts_info;
   CNTInfo    *cnt_info    = (CNTInfo *) 0;
   SL1L2Info  *sl1l2_info  = (SL1L2Info *) 0;
   VL1L2Info  *vl1l2_info  = (VL1L2Info *) 0;
   NBRCNTInfo  nbrcnt_info;
   NBRCTSInfo *nbrcts_info = (NBRCTSInfo *) 0;
   PCTInfo    *pct_info    = (PCTInfo *) 0;

   // Store the maximum number of each threshold type
   n_cnt  = conf_info.get_max_n_cnt_thresh();
   n_cat  = conf_info.get_max_n_cat_thresh();
   n_wind = conf_info.get_max_n_wind_thresh();
   n_prob = conf_info.get_max_n_oprob_thresh();
   n_cov  = conf_info.get_n_cov_thresh();

   // Allocate space for output statistics types
   cts_info    = new CTSInfo    [n_cat];
   cnt_info    = new CNTInfo    [n_cnt];
   sl1l2_info  = new SL1L2Info  [n_cnt];
   vl1l2_info  = new VL1L2Info  [n_wind];
   nbrcts_info = new NBRCTSInfo [n_cov];
   pct_info    = new PCTInfo    [n_prob];

   // Compute scores for each verification task and write output_flag
   for(i=0; i<conf_info.get_n_vx(); i++) {

      mlog << Debug(3)
           << "Reading forecast data for "
           << conf_info.fcst_info[i]->magic_str() << ".\n";

      // Read the gridded data from the input forecast file
      status = fcst_mtddf->data_plane(*conf_info.fcst_info[i], fcst_dp);

      if(!status) {
         mlog << Warning << "\nprocess_scores() -> "
              << conf_info.fcst_info[i]->magic_str()
              << " not found in file: " << fcst_file
              << "\n\n";
         continue;
      }

      // Regrid, if necessary
      if(!(fcst_mtddf->grid() == grid)) {
         mlog << Debug(1)
              << "Regridding forecast "
              << conf_info.fcst_info[i]->magic_str()
              << " to the verification grid.\n";
         fcst_dp = met_regrid(fcst_dp, fcst_mtddf->grid(), grid,
                              conf_info.regrid_info);
      }

      // For probability fields, check to see if they need to be
      // rescaled from [0, 100] to [0, 1]
      if(conf_info.fcst_info[i]->p_flag()) rescale_probability(fcst_dp);

      // Set the forecast lead time
      shc.set_fcst_lead_sec(fcst_dp.lead());

      // Set the forecast valid time
      shc.set_fcst_valid_beg(fcst_dp.valid());
      shc.set_fcst_valid_end(fcst_dp.valid());

      mlog << Debug(3)
           << "Reading observation data for "
           << conf_info.obs_info[i]->magic_str() << ".\n";

      // Read the gridded data from the input observation file
      status = obs_mtddf->data_plane(*conf_info.obs_info[i], obs_dp);

      if(!status) {
         mlog << Warning << "\nprocess_scores() -> "
              << conf_info.obs_info[i]->magic_str()
              << " not found in file: " << obs_file
              << "\n\n";
         continue;
      }

      // Regrid, if necessary
      if(!(obs_mtddf->grid() == grid)) {
         mlog << Debug(1)
              << "Regridding observation "
              << conf_info.obs_info[i]->magic_str()
              << " to the verification grid.\n";
         obs_dp = met_regrid(obs_dp, obs_mtddf->grid(), grid,
                             conf_info.regrid_info);
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
              << unix_to_yyyymmdd_hhmmss(fcst_dp.valid()) << " != "
              << unix_to_yyyymmdd_hhmmss(obs_dp.valid()) << " for "
              << conf_info.fcst_info[i]->magic_str() << " versus "
              << conf_info.obs_info[i]->magic_str() << ".\n\n";
      }

      // Check that the accumulation intervals match
      if(conf_info.fcst_info[i]->level().type() == LevelType_Accum &&
         conf_info.obs_info[i]->level().type()  == LevelType_Accum &&
         fcst_dp.accum() != obs_dp.accum()) {

         mlog << Warning << "\nprocess_scores() -> "
              << "Forecast and observation accumulation times "
              << "do not match " << sec_to_hhmmss(fcst_dp.accum())
              << " != " << sec_to_hhmmss(obs_dp.accum())
              << " for " << conf_info.fcst_info[i]->magic_str() << " versus "
              << conf_info.obs_info[i]->magic_str() << ".\n\n";
      }

      // Read climatology data
      cmn_dp = read_climo_data_plane(
                  conf_info.conf.lookup_array(conf_key_climo_mean_field, false),
                  i, fcst_dp.valid(), grid);

      mlog << Debug(3)
           << "Found " << (cmn_dp.nx() == 0 ? 0 : 1)
           << " climatology mean field(s) for forecast "
           << conf_info.fcst_info[i]->magic_str() << ".\n";

      // Setup the first pass through the data
      if(is_first_pass) setup_first_pass(fcst_dp);

      // Store the forecast variable name
      shc.set_fcst_var(conf_info.fcst_info[i]->name());

      // Set the forecast level name
      shc.set_fcst_lev(conf_info.fcst_info[i]->level_name());

      // Store the observation variable name
      shc.set_obs_var(conf_info.obs_info[i]->name());

      // Set the observation level name
      shc.set_obs_lev(conf_info.obs_info[i]->level_name());

      mlog << Debug(2) << "\n" << sep_str << "\n\n";

      // If verifying vector winds, store the U-wind fields
      if(conf_info.fcst_info[i]->is_u_wind() &&
         conf_info.obs_info[i]->is_u_wind() &&
         conf_info.fcst_info[i]->v_flag() &&
         conf_info.obs_info[i]->v_flag()) {

         fu_dp   = fcst_dp;
         ou_dp   = obs_dp;
         cmnu_dp = cmn_dp;
      }

      // Loop through and apply each of the smoothing operations
      for(j=0; j<conf_info.get_n_interp(); j++) {

         // Store the interpolation method and width being applied
         shc.set_interp_mthd(conf_info.interp_mthd[j]);
         shc.set_interp_wdth(conf_info.interp_wdth[j]);

         // If requested in the config file, smooth the forecast field
         if(conf_info.interp_field == FieldType_Fcst ||
            conf_info.interp_field == FieldType_Both) {
            smooth_field(fcst_dp, fcst_dp_smooth,
                         conf_info.interp_mthd[j],
                         conf_info.interp_wdth[j],
                         conf_info.interp_thresh);
         }
         // Do not smooth the forecast field
         else {
            fcst_dp_smooth = fcst_dp;
         }

         // If requested in the config file, smooth the observation field
         if(conf_info.interp_field == FieldType_Obs ||
            conf_info.interp_field == FieldType_Both) {
            smooth_field(obs_dp, obs_dp_smooth,
                         conf_info.interp_mthd[j],
                         conf_info.interp_wdth[j],
                         conf_info.interp_thresh);
         }
         // Do not smooth the observation field
         else {
            obs_dp_smooth = obs_dp;
         }

         // Loop through the masks to be applied
         for(k=0; k<conf_info.get_n_mask(); k++) {

            // Store the current mask
            mask_dp = conf_info.mask_dp[k];

            // Turn off the mask for bad forecast or observation values
            mask_bad_data(mask_dp, fcst_dp_smooth, 0.0);
            mask_bad_data(mask_dp, obs_dp_smooth,  0.0);

            // Apply the current mask to the current fields
            apply_mask(fcst_dp_smooth, mask_dp, f_na);
            apply_mask(obs_dp_smooth,  mask_dp, o_na);
            apply_mask(cmn_dp,         mask_dp, c_na);
            apply_mask(wgt_dp,         mask_dp, w_na);

            // Set the mask name
            shc.set_mask(conf_info.mask_name[k]);

            mlog << Debug(2)
                 << "Processing " << conf_info.fcst_info[i]->magic_str()
                 << " versus " << conf_info.obs_info[i]->magic_str()
                 << ", for interpolation method "
                 << shc.get_interp_mthd() << "("
                 << shc.get_interp_pnts_str()
                 << "), over region " << shc.get_mask()
                 << ", using " << f_na.n_elements() << " pairs.\n";

            // Continue if no pairs were found
            if(f_na.n_elements() == 0) continue;

            // Compute CTS scores
            if(!conf_info.fcst_info[i]->p_flag()      &&
                conf_info.fcat_ta[i].n_elements() > 0 &&
               (conf_info.output_flag[i_fho] != STATOutputType_None ||
                conf_info.output_flag[i_ctc] != STATOutputType_None ||
                conf_info.output_flag[i_cts] != STATOutputType_None)) {

               // Initialize
               for(m=0; m<n_cat; m++) cts_info[m].clear();

               // Compute CTS
               do_cts(cts_info, i, f_na, o_na);

               // Loop through all of the thresholds
               for(m=0; m<conf_info.fcat_ta[i].n_elements(); m++) {

                  // Write out FHO
                  if(conf_info.output_flag[i_fho] != STATOutputType_None &&
                     cts_info[m].cts.n() > 0) {

                     write_fho_row(shc, cts_info[m],
                        conf_info.output_flag[i_fho] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_fho], i_txt_row[i_fho]);
                  }

                  // Write out CTC
                  if(conf_info.output_flag[i_ctc] != STATOutputType_None &&
                     cts_info[m].cts.n() > 0) {

                     write_ctc_row(shc, cts_info[m],
                        conf_info.output_flag[i_ctc] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_ctc], i_txt_row[i_ctc]);
                  }

                  // Write out CTS
                  if(conf_info.output_flag[i_cts] != STATOutputType_None &&
                     cts_info[m].cts.n() > 0) {

                     write_cts_row(shc, cts_info[m],
                        conf_info.output_flag[i_cts] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_cts], i_txt_row[i_cts]);
                  }
               } // end for m
            } // end Compute CTS

            // Compute MCTS scores
            if(!conf_info.fcst_info[i]->p_flag()      &&
                conf_info.fcat_ta[i].n_elements() > 1 &&
               (conf_info.output_flag[i_mctc] != STATOutputType_None ||
                conf_info.output_flag[i_mcts] != STATOutputType_None)) {

               // Initialize
               mcts_info.clear();

               // Compute MCTS
               do_mcts(mcts_info, i, f_na, o_na);

               // Write out MCTC
               if(conf_info.output_flag[i_mctc] != STATOutputType_None &&
                  mcts_info.cts.total() > 0) {

                  write_mctc_row(shc, mcts_info,
                     conf_info.output_flag[i_mctc] == STATOutputType_Both,
                     stat_at, i_stat_row,
                     txt_at[i_mctc], i_txt_row[i_mctc]);
               }

               // Write out MCTS
               if(conf_info.output_flag[i_mcts] != STATOutputType_None &&
                  mcts_info.cts.total() > 0) {

                  write_mcts_row(shc, mcts_info,
                     conf_info.output_flag[i_mcts] == STATOutputType_Both,
                     stat_at, i_stat_row,
                     txt_at[i_mcts], i_txt_row[i_mcts]);
               }
            } // end Compute MCTS

            // Compute CNT scores
            if(!conf_info.fcst_info[i]->p_flag() &&
               conf_info.output_flag[i_cnt] != STATOutputType_None) {

               // Initialize
               for(m=0; m<n_cnt; m++) cnt_info[m].clear();

               // Compute CNT
               do_cnt(cnt_info, i, f_na, o_na, c_na);

               // Loop through the continuous thresholds
               for(m=0; m<conf_info.fcnt_ta[i].n_elements(); m++) {

                  // Write out CNT
                  if(conf_info.output_flag[i_cnt] != STATOutputType_None &&
                     cnt_info[m].n > 0) {

                     write_cnt_row(shc, cnt_info[m],
                        conf_info.output_flag[i_cnt] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_cnt], i_txt_row[i_cnt]);
                  }
               } // end for m
            } // end Compute CNT

            // Compute SL1L2 and SAL1L2 scores as long as the
            // vflag is not set
            if(!conf_info.fcst_info[i]->p_flag() &&
               !conf_info.fcst_info[i]->v_flag() &&
               (conf_info.output_flag[i_sl1l2]  != STATOutputType_None ||
                conf_info.output_flag[i_sal1l2] != STATOutputType_None)) {

               // Initialize
               for(m=0; m<n_cnt; m++) sl1l2_info[m].clear();

               // Compute SL1L2 and SAL1L2
               do_sl1l2(sl1l2_info, i, f_na, o_na, c_na);

               // Loop through the continuous thresholds
               for(m=0; m<conf_info.fcnt_ta[i].n_elements(); m++) {

                  // Write out SL1L2
                  if(conf_info.output_flag[i_sl1l2] != STATOutputType_None &&
                     sl1l2_info[m].scount > 0) {

                     write_sl1l2_row(shc, sl1l2_info[m],
                        conf_info.output_flag[i_sl1l2] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_sl1l2], i_txt_row[i_sl1l2]);
                  }

                  // Write out SAL1L2
                  if(conf_info.output_flag[i_sal1l2] != STATOutputType_None &&
                     sl1l2_info[m].sacount > 0) {

                     write_sal1l2_row(shc, sl1l2_info[m],
                        conf_info.output_flag[i_sal1l2] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_sal1l2], i_txt_row[i_sal1l2]);
                  }
               } // end for m
            }  // end Compute SL1L2 and SAL1L2

            // Compute VL1L2 and VAL1L2 partial sums for UGRD,VGRD
            if(!conf_info.fcst_info[i]->p_flag() &&
                conf_info.fcst_info[i]->v_flag() &&
               (conf_info.output_flag[i_vl1l2]  != STATOutputType_None ||
                conf_info.output_flag[i_val1l2] != STATOutputType_None) &&
               i > 0 &&
               conf_info.fcst_info[i]->is_v_wind() &&
               conf_info.fcst_info[i-1]->is_u_wind()) {

               // Store the forecast variable name
               shc.set_fcst_var(ugrd_vgrd_abbr_str);

               // Store the observation variable name
               shc.set_obs_var(ugrd_vgrd_abbr_str);

               // Initialize
               for(m=0; m<n_wind; m++) vl1l2_info[m].clear();

               // If requested in the config file, smooth the forecast
               // and climatology U-wind fields
               if(conf_info.interp_field == FieldType_Fcst ||
                  conf_info.interp_field == FieldType_Both) {
                  smooth_field(fu_dp, fu_dp_smooth,
                               conf_info.interp_mthd[j],
                               conf_info.interp_wdth[j],
                               conf_info.interp_thresh);
               }
               // Do not smooth the forecast field
               else {
                  fu_dp_smooth = fu_dp;
               }

               // If requested in the config file, smooth the observation
               // U-wind field
               if(conf_info.interp_field == FieldType_Obs ||
                  conf_info.interp_field == FieldType_Both) {
                  smooth_field(ou_dp, ou_dp_smooth,
                               conf_info.interp_mthd[j],
                               conf_info.interp_wdth[j],
                               conf_info.interp_thresh);
               }
               // Do not smooth the observation field
               else {
                  ou_dp_smooth = ou_dp;
               }

               // Apply the current mask to the U-wind fields
               apply_mask(fu_dp_smooth,   mask_dp, fu_na);
               apply_mask(ou_dp_smooth,   mask_dp, ou_na);
               apply_mask(cmnu_dp,        mask_dp, cmnu_na);
               apply_mask(wgt_dp,         mask_dp, w_na);

               // Compute VL1L2
               do_vl1l2(vl1l2_info, i, fu_na, f_na, ou_na, o_na, cmnu_na, c_na);

               // Loop through all of the wind speed thresholds
               for(m=0; m<conf_info.fwind_ta[i].n_elements(); m++) {

                  // Write out VL1L2
                  if(conf_info.output_flag[i_vl1l2] != STATOutputType_None &&
                     vl1l2_info[m].vcount > 0) {

                     write_vl1l2_row(shc, vl1l2_info[m],
                        conf_info.output_flag[i_vl1l2] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_vl1l2], i_txt_row[i_vl1l2]);
                  }

                  // Write out VAL1L2
                  if(conf_info.output_flag[i_val1l2] != STATOutputType_None &&
                     vl1l2_info[m].vacount > 0) {

                     write_val1l2_row(shc, vl1l2_info[m],
                        conf_info.output_flag[i_val1l2] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_val1l2], i_txt_row[i_val1l2]);
                  }
               } // end for m

               // Reset the forecast variable name
               shc.set_fcst_var(conf_info.fcst_info[i]->name());

               // Reset the observation variable name
               shc.set_obs_var(conf_info.obs_info[i]->name());

            } // end Compute VL1L2

            // Compute PCT counts and scores
            if(conf_info.fcst_info[i]->p_flag() &&
               (conf_info.output_flag[i_pct] != STATOutputType_None  ||
                conf_info.output_flag[i_pstd] != STATOutputType_None ||
                conf_info.output_flag[i_pjc] != STATOutputType_None  ||
                conf_info.output_flag[i_prc] != STATOutputType_None)) {

               // Initialize
               for(m=0; m<n_prob; m++) pct_info[m].clear();

               // Compute PCT
               do_pct(pct_info, i, f_na, o_na, c_na);

               // Loop through all of the thresholds
               for(m=0; m<n_prob; m++) {

                  // Write out PCT
                  if(conf_info.output_flag[i_pct] != STATOutputType_None &&
                     pct_info[m].pct.n() > 0) {

                     write_pct_row(shc, pct_info[m],
                        conf_info.output_flag[i_pct] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_pct], i_txt_row[i_pct]);
                  }

                  // Write out PSTD
                  if(conf_info.output_flag[i_pstd] != STATOutputType_None &&
                     pct_info[m].pct.n() > 0) {

                     write_pstd_row(shc, pct_info[m],
                        conf_info.output_flag[i_pstd] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_pstd], i_txt_row[i_pstd]);
                  }

                  // Write out PJC
                  if(conf_info.output_flag[i_pjc] != STATOutputType_None &&
                     pct_info[m].pct.n() > 0) {

                     write_pjc_row(shc, pct_info[m],
                        conf_info.output_flag[i_pjc] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_pjc], i_txt_row[i_pjc]);
                  }

                  // Write out PRC
                  if(conf_info.output_flag[i_prc] != STATOutputType_None &&
                     pct_info[m].pct.n() > 0) {

                     write_prc_row(shc, pct_info[m],
                        conf_info.output_flag[i_prc] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_prc], i_txt_row[i_prc]);
                  }
               } // end for m
            } // end Compute PCT

         } // end for k

         // Write out the smoothed forecast, observation, and difference
         // fields in netCDF format for each verification task if requested in
         // the config file
         if(!(conf_info.nc_info.all_false())) {
            write_nc(conf_info.nc_info,
                     fcst_dp_smooth, obs_dp_smooth, cmn_dp, i,
                     conf_info.interp_mthd[j],
                     conf_info.interp_wdth[j]);
         }
      } // end for j

      // Loop through and apply the Neighborhood methods for each of the
      // neighborhood widths if requested in the config file
      if(!conf_info.fcst_info[i]->p_flag() &&
         (conf_info.output_flag[i_nbrctc] != STATOutputType_None ||
          conf_info.output_flag[i_nbrcts] != STATOutputType_None ||
          conf_info.output_flag[i_nbrcnt] != STATOutputType_None)) {

         // Initialize
         for(j=0; j<n_cov; j++) nbrcts_info[j].clear();

         // Loop through and apply each of the neighborhood widths
         for(j=0; j<conf_info.get_n_nbrhd_wdth(); j++) {

            // Store the interpolation method and width being applied
            shc.set_interp_mthd(InterpMthd_Nbrhd);
            shc.set_interp_wdth(conf_info.nbrhd_wdth[j]);

            // Loop through and apply each of the raw threshold values
            for(k=0; k<conf_info.fcat_ta[i].n_elements(); k++) {

               // Compute the binary thresholded fields
               fcst_dp_thresh = fcst_dp;
               fcst_dp_thresh.threshold(conf_info.fcat_ta[i][k]);

               obs_dp_thresh = obs_dp;
               obs_dp_thresh.threshold(conf_info.ocat_ta[i][k]);

               // Compute the thresholded fractional coverage field
               fractional_coverage(fcst_dp, fcst_dp_smooth,
                                   conf_info.nbrhd_wdth[j],
                                   conf_info.fcat_ta[i][k],
                                   conf_info.nbrhd_thresh);

               fractional_coverage(obs_dp, obs_dp_smooth,
                                   conf_info.nbrhd_wdth[j],
                                   conf_info.ocat_ta[i][k],
                                   conf_info.nbrhd_thresh);

               // Loop through the masks to be applied
               for(m=0; m<conf_info.get_n_mask(); m++) {

                  // Store the current mask
                  mask_dp = conf_info.mask_dp[m];

                  // Turn off the mask for bad forecast or observation values
                  mask_bad_data(mask_dp, fcst_dp_smooth, 0.0);
                  mask_bad_data(mask_dp, fcst_dp_thresh, 0.0);
                  mask_bad_data(mask_dp, obs_dp_smooth,  0.0);
                  mask_bad_data(mask_dp, obs_dp_thresh,  0.0);

                  // Apply the current mask to the fractional coverage
                  // and thresholded fields
                  apply_mask(fcst_dp_smooth, mask_dp, f_na);
                  apply_mask(fcst_dp_thresh, mask_dp, fthr_na);
                  apply_mask(obs_dp_smooth,  mask_dp, o_na);
                  apply_mask(obs_dp_thresh,  mask_dp, othr_na);
                  apply_mask(wgt_dp,         mask_dp, w_na);

                  mlog << Debug(2) << "Processing "
                       << conf_info.fcst_info[i]->magic_str()
                       << " versus "
                       << conf_info.obs_info[i]->magic_str()
                       << ", for interpolation method "
                       << shc.get_interp_mthd() << "("
                       << shc.get_interp_pnts_str()
                       << "), raw thresholds of "
                       << conf_info.fcat_ta[i][k].get_str()
                       << " and "
                       << conf_info.ocat_ta[i][k].get_str()
                       << ", over region " << shc.get_mask()
                       << ", using " << f_na.n_elements()
                       << " pairs.\n";

                  // Continue if no pairs were found
                  if(f_na.n_elements() == 0) continue;

                  // Set the mask name
                  shc.set_mask(conf_info.mask_name[m]);

                  // Compute NBRCTS scores
                  if(conf_info.output_flag[i_nbrctc] != STATOutputType_None ||
                     conf_info.output_flag[i_nbrcts] != STATOutputType_None) {

                     // Initialize
                     for(n=0; n<conf_info.nbrhd_cov_ta.n_elements(); n++) {
                        nbrcts_info[n].clear();
                     }

                     do_nbrcts(nbrcts_info, i, j, k, f_na, o_na);

                     // Loop through all of the thresholds
                     for(n=0; n<conf_info.nbrhd_cov_ta.n_elements(); n++) {

                        // Only write out if n > 0
                        if(nbrcts_info[n].cts_info.cts.n() > 0) {

                           // Write out NBRCTC
                           if(conf_info.output_flag[i_nbrctc] != STATOutputType_None) {

                              write_nbrctc_row(shc, nbrcts_info[n],
                                 conf_info.output_flag[i_nbrctc] == STATOutputType_Both,
                                 stat_at, i_stat_row,
                                 txt_at[i_nbrctc], i_txt_row[i_nbrctc]);
                           }

                           // Write out NBRCTS
                           if(conf_info.output_flag[i_nbrcts] != STATOutputType_None) {

                              write_nbrcts_row(shc, nbrcts_info[n],
                                 conf_info.output_flag[i_nbrcts] == STATOutputType_Both,
                                 stat_at, i_stat_row,
                                 txt_at[i_nbrcts], i_txt_row[i_nbrcts]);
                           }
                        } // end if
                     } // end for n
                  } // end compute NBRCTS

                  // Compute NBRCNT scores
                  if(conf_info.output_flag[i_nbrcnt] != STATOutputType_None) {

                     // Initialize
                     nbrcnt_info.clear();

                     do_nbrcnt(nbrcnt_info, i, j, k, f_na, o_na,
                               fthr_na, othr_na);

                     // Write out NBRCNT
                     if(nbrcnt_info.sl1l2_info.scount > 0 &&
                        conf_info.output_flag[i_nbrcnt]) {

                        write_nbrcnt_row(shc, nbrcnt_info,
                           conf_info.output_flag[i_nbrcnt] == STATOutputType_Both,
                           stat_at, i_stat_row,
                           txt_at[i_nbrcnt], i_txt_row[i_nbrcnt]);
                     }
                  } // end compute NBRCNT
               } // end for m

               // Write out the neighborhood fractional coverage fields
               // if requested in the config file
               if(conf_info.nc_info.do_nbrhd) {
                  write_nbrhd_nc(conf_info.nc_info,
                     fcst_dp_smooth, obs_dp_smooth, i,
                     conf_info.fcat_ta[i][k], conf_info.ocat_ta[i][k],
                     conf_info.nbrhd_wdth[j]);
               }

            } // end for k
         } // end for j
      } // end if
   } // end for i

   mlog << Debug(2) << "\n" << sep_str << "\n\n";

   // Deallocate memory
   if(cts_info)    { delete [] cts_info;    cts_info    = (CTSInfo *)    0; }
   if(cnt_info)    { delete [] cnt_info;    cnt_info    = (CNTInfo *)    0; }
   if(sl1l2_info)  { delete [] sl1l2_info;  sl1l2_info  = (SL1L2Info *)  0; }
   if(vl1l2_info)  { delete [] vl1l2_info;  vl1l2_info  = (VL1L2Info *)  0; }
   if(nbrcts_info) { delete [] nbrcts_info; nbrcts_info = (NBRCTSInfo *) 0; }
   if(pct_info)    { delete [] pct_info;    pct_info    = (PCTInfo *)    0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_cts(CTSInfo *&cts_info, int i_vx,
            const NumArray &f_na, const NumArray &o_na) {
   int i, j, n_cts;

   //
   // Set up the CTSInfo thresholds and alpha values
   //
   n_cts = conf_info.fcat_ta[i_vx].n_elements();
   for(i=0; i<n_cts; i++) {
      cts_info[i].fthresh = conf_info.fcat_ta[i_vx][i];
      cts_info[i].othresh = conf_info.ocat_ta[i_vx][i];
      cts_info[i].allocate_n_alpha(conf_info.get_n_ci_alpha());

      for(j=0; j<conf_info.get_n_ci_alpha(); j++) {
         cts_info[i].alpha[j] = conf_info.ci_alpha[j];
      }
   }

   mlog << Debug(2) << "Computing Categorical Statistics.\n";

   //
   // Compute the counts, stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(conf_info.boot_interval == BootIntervalType_BCA) {
      compute_cts_stats_ci_bca(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep,
         cts_info, n_cts,
         conf_info.output_flag[i_cts] != STATOutputType_None,
         conf_info.rank_corr_flag, conf_info.tmp_dir);
   }
   else {
      compute_cts_stats_ci_perc(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep, conf_info.boot_rep_prop,
         cts_info, n_cts,
         conf_info.output_flag[i_cts] != STATOutputType_None,
         conf_info.rank_corr_flag, conf_info.tmp_dir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_mcts(MCTSInfo &mcts_info, int i_vx,
             const NumArray &f_na, const NumArray &o_na) {
   int i;

   //
   // Set up the MCTSInfo size, thresholds, and alpha values
   //
   mcts_info.cts.set_size(conf_info.fcat_ta[i_vx].n_elements() + 1);
   mcts_info.fthresh = conf_info.fcat_ta[i_vx];
   mcts_info.othresh = conf_info.ocat_ta[i_vx];
   mcts_info.allocate_n_alpha(conf_info.get_n_ci_alpha());

   for(i=0; i<conf_info.get_n_ci_alpha(); i++) {
      mcts_info.alpha[i] = conf_info.ci_alpha[i];
   }

   mlog << Debug(2) << "Computing Multi-Category Statistics.\n";

   //
   // If there are no matched pairs to process, return
   //
   if(f_na.n_elements() == 0 || o_na.n_elements() == 0) return;

   //
   // Compute the counts, stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(conf_info.boot_interval == BootIntervalType_BCA) {
      compute_mcts_stats_ci_bca(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep,
         mcts_info,
         conf_info.output_flag[i_mcts] != STATOutputType_None,
         conf_info.rank_corr_flag, conf_info.tmp_dir);
   }
   else {
      compute_mcts_stats_ci_perc(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep, conf_info.boot_rep_prop,
         mcts_info,
         conf_info.output_flag[i_mcts] != STATOutputType_None,
         conf_info.rank_corr_flag, conf_info.tmp_dir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_cnt(CNTInfo *&cnt_info, int i_vx,
            const NumArray &f_na, const NumArray &o_na,
            const NumArray &c_na) {
   int i, j;
   NumArray ff_na, oo_na, cc_na;

   mlog << Debug(2) << "Computing Continuous Statistics.\n";

   //
   // Process each filtering threshold
   //
   for(i=0; i<conf_info.fcnt_ta[i_vx].n_elements(); i++) {

      //
      // Store thresholds
      //
      cnt_info[i].fthresh = conf_info.fcnt_ta[i_vx][i];
      cnt_info[i].othresh = conf_info.ocnt_ta[i_vx][i];
      cnt_info[i].logic   = conf_info.cnt_logic[i_vx];

      //
      // Setup the CNTInfo alpha values
      //
      cnt_info[i].allocate_n_alpha(conf_info.get_n_ci_alpha());
      for(j=0; j<conf_info.get_n_ci_alpha(); j++) {
         cnt_info[i].alpha[j] = conf_info.ci_alpha[j];
      }

      //
      // Apply continuous filtering thresholds
      //
      subset_pairs(f_na, cnt_info[i].fthresh,
                   o_na, cnt_info[i].othresh,
                   c_na, cnt_info[i].logic,
                   ff_na, oo_na, cc_na);

      //
      // Check for no matched pairs to process
      //
      if(ff_na.n_elements() == 0) continue;

      //
      // Compute the stats, normal confidence intervals, and
      // bootstrap confidence intervals
      //
      int precip_flag = (conf_info.fcst_info[i_vx]->is_precipitation() &&
                         conf_info.obs_info[i_vx]->is_precipitation());

      if(conf_info.boot_interval == BootIntervalType_BCA) {
         compute_cnt_stats_ci_bca(rng_ptr, ff_na, oo_na, cc_na,
            precip_flag, conf_info.rank_corr_flag,
            conf_info.n_boot_rep,
            cnt_info[i], conf_info.tmp_dir);
      }
      else {
         compute_cnt_stats_ci_perc(rng_ptr, ff_na, oo_na, cc_na,
            precip_flag, conf_info.rank_corr_flag,
            conf_info.n_boot_rep, conf_info.boot_rep_prop,
            cnt_info[i], conf_info.tmp_dir);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_sl1l2(SL1L2Info *&s_info, int i_vx,
              const NumArray &f_na, const NumArray &o_na,
              const NumArray &c_na) {
   int i;

   mlog << Debug(2)
        << "Computing Scalar Partial Sums.\n";

   //
   // Process each filtering threshold
   //
   for(i=0; i<conf_info.fcnt_ta[i_vx].n_elements(); i++) {

      //
      // Store thresholds
      //
      s_info[i].fthresh = conf_info.fcnt_ta[i_vx][i];
      s_info[i].othresh = conf_info.ocnt_ta[i_vx][i];
      s_info[i].logic   = conf_info.cnt_logic[i_vx];

      //
      // Compute partial sums
      //
      s_info[i].set(f_na, o_na, c_na);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_vl1l2(VL1L2Info *&v_info, int i_vx,
              const NumArray &uf_na, const NumArray &vf_na,
              const NumArray &uo_na, const NumArray &vo_na,
              const NumArray &uc_na, const NumArray &vc_na) {
   int i;

   // Check that the number of pairs are the same
   if(uf_na.n_elements() != uo_na.n_elements() ||
      uf_na.n_elements() != vf_na.n_elements() ||
      vf_na.n_elements() != vo_na.n_elements()) {
      mlog << Error << "\ndo_vl1l2() -> "
           << "unequal number of UGRD and VGRD pairs ("
           << uf_na.n_elements() << " != " << uo_na.n_elements()
           << ")\n\n";
      exit(1);
   }

   // Set all of the VL1L2Info objects
   for(i=0; i<conf_info.fwind_ta[i_vx].n_elements(); i++) {

      //
      // Store thresholds
      //
      v_info[i].zero_out();
      v_info[i].fthresh = conf_info.fwind_ta[i_vx][i];
      v_info[i].othresh = conf_info.owind_ta[i_vx][i];
      v_info[i].logic   = conf_info.wind_logic[i_vx];

      //
      // Compute partial sums
      //
      v_info[i].set(uf_na, vf_na, uo_na, vo_na, uc_na, vc_na);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_pct(PCTInfo *&pct_info, int i_vx,
            const NumArray &f_na, const NumArray &o_na,
            const NumArray &c_na) {
   int i, j, n_pct;

   mlog << Debug(2) << "Computing Probabilistic Statistics.\n";

   //
   // If there are no matched pairs to process, return
   //
   if(f_na.n_elements() == 0 || o_na.n_elements() == 0) return;

   //
   // Set up the PCTInfo thresholds and alpha values
   //
   n_pct = conf_info.ocat_ta[i_vx].n_elements();
   for(i=0; i<n_pct; i++) {

      // Use all of the selected forecast thresholds
      pct_info[i].fthresh = conf_info.fcat_ta[i_vx];

      // Process the observation thresholds one at a time
      pct_info[i].othresh = conf_info.ocat_ta[i_vx][i];

      pct_info[i].allocate_n_alpha(conf_info.get_n_ci_alpha());

      for(j=0; j<conf_info.get_n_ci_alpha(); j++) {
         pct_info[i].alpha[j] = conf_info.ci_alpha[j];
      }

      //
      // Compute the probabilistic counts and statistics
      //
      compute_pctinfo(f_na, o_na, c_na, conf_info.output_flag[i_pstd],
                      pct_info[i]);
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_nbrcts(NBRCTSInfo *&nbrcts_info,
               int i_vx, int i_wdth, int i_thresh,
               const NumArray &f_na, const NumArray &o_na) {
   int i, j, n_nbrcts;

   //
   // Set up the NBRCTSInfo thresholds and alpha values
   //
   n_nbrcts = conf_info.nbrhd_cov_ta.n_elements();
   for(i=0; i<n_nbrcts; i++) {

      nbrcts_info[i].fthresh =
         conf_info.fcat_ta[i_vx][i_thresh];
      nbrcts_info[i].othresh =
         conf_info.ocat_ta[i_vx][i_thresh];
      nbrcts_info[i].cthresh =
         conf_info.nbrhd_cov_ta[i];

      nbrcts_info[i].cts_info.fthresh =
         conf_info.nbrhd_cov_ta[i];
      nbrcts_info[i].cts_info.othresh =
         conf_info.nbrhd_cov_ta[i];
      nbrcts_info[i].allocate_n_alpha(conf_info.get_n_ci_alpha());

      for(j=0; j<conf_info.get_n_ci_alpha(); j++) {
         nbrcts_info[i].cts_info.alpha[j] = conf_info.ci_alpha[j];
      }
   }

   //
   // Keep track of the neighborhood width
   //
   for(i=0; i<n_nbrcts; i++) {
      nbrcts_info[i].nbr_wdth = conf_info.nbrhd_wdth[i_wdth];
   }

   mlog << Debug(2) << "Computing Neighborhood Categorical Statistics.\n";

   //
   // Compute the stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(conf_info.boot_interval == BootIntervalType_BCA) {
      compute_nbrcts_stats_ci_bca(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep,
         nbrcts_info, n_nbrcts,
         conf_info.output_flag[i_nbrcts] != STATOutputType_None,
         conf_info.tmp_dir);
   }
   else {
      compute_nbrcts_stats_ci_perc(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep, conf_info.boot_rep_prop,
         nbrcts_info, n_nbrcts,
         conf_info.output_flag[i_nbrcts] != STATOutputType_None,
         conf_info.tmp_dir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_nbrcnt(NBRCNTInfo &nbrcnt_info,
               int i_vx, int i_wdth, int i_thresh,
               const NumArray &f_na, const NumArray &o_na,
               const NumArray &fthr_na, const NumArray &othr_na) {
   int i;

   //
   // Set up the NBRCNTInfo threshold and alpha values
   //
   nbrcnt_info.fthresh =
      conf_info.fcat_ta[i_vx][i_thresh];
   nbrcnt_info.othresh =
      conf_info.ocat_ta[i_vx][i_thresh];

   nbrcnt_info.allocate_n_alpha(conf_info.get_n_ci_alpha());
   for(i=0; i<conf_info.get_n_ci_alpha(); i++) {
      nbrcnt_info.alpha[i] = conf_info.ci_alpha[i];
   }

   //
   // Keep track of the neighborhood width
   //
   nbrcnt_info.nbr_wdth = conf_info.nbrhd_wdth[i_wdth];

   mlog << Debug(2) << "Computing Neighborhood Continuous Statistics.\n";

   //
   // Compute the stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(conf_info.boot_interval == BootIntervalType_BCA) {
      compute_nbrcnt_stats_ci_bca(rng_ptr, f_na, o_na,
         fthr_na, othr_na,
         conf_info.n_boot_rep,
         nbrcnt_info,
         conf_info.output_flag[i_nbrcnt] != STATOutputType_None,
         conf_info.tmp_dir);
   }
   else {
      compute_nbrcnt_stats_ci_perc(rng_ptr, f_na, o_na,
         fthr_na, othr_na,
         conf_info.n_boot_rep, conf_info.boot_rep_prop,
         nbrcnt_info,
         conf_info.output_flag[i_nbrcnt] != STATOutputType_None,
         conf_info.tmp_dir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nc(const GridStatNcOutInfo & nc_info,
              const DataPlane &fcst_dp, const DataPlane &obs_dp,
              const DataPlane &cmn_dp,
              int i_vx, InterpMthd mthd, int wdth) {
   int i, n, x, y;
   int fcst_flag, obs_flag, diff_flag, cmn_flag;
   double fval, oval, cmnval;
   ConcatString fcst_var_name, obs_var_name, diff_var_name;
   ConcatString cmn_var_name;
   ConcatString att_str, mthd_str, smooth_str, cs;

   float *fcst_data = (float *) 0;
   float *obs_data  = (float *) 0;
   float *diff_data = (float *) 0;
   float *cmn_data  = (float *) 0;

   NcVar *fcst_var = (NcVar *) 0;
   NcVar *obs_var  = (NcVar *) 0;
   NcVar *diff_var = (NcVar *) 0;
   NcVar *cmn_var  = (NcVar *) 0;

   // Get the interpolation strings
   mthd_str = interpmthd_to_string(mthd);
   if(wdth > 1) smooth_str << "_" << mthd_str << "_" << wdth*wdth;

   // Allocate memory for the forecast, observation, and difference
   // fields
   fcst_data = new float [grid.nx()*grid.ny()];
   obs_data  = new float [grid.nx()*grid.ny()];
   diff_data = new float [grid.nx()*grid.ny()];
   cmn_data  = new float [grid.nx()*grid.ny()];

   // Compute the difference field for each of the masking regions
   for(i=0; i<conf_info.get_n_mask(); i++) {

      // Build the forecast variable name
      cs << cs_erase
         << conf_info.fcst_info[i_vx]->name() << "_"
         << conf_info.fcst_info[i_vx]->level_name() << "_"
         << conf_info.mask_name[i];

      fcst_var_name << cs_erase << "FCST_" << cs;
      if(conf_info.interp_field == FieldType_Fcst ||
         conf_info.interp_field == FieldType_Both) {
         fcst_var_name << smooth_str;
      }

      // Build the climatology variable names (no smoothing)
      cmn_var_name  << cs_erase << "CLIMO_MEAN_" << cs;

      // Build the observation variable name
      obs_var_name << cs_erase << "OBS_"
                   << conf_info.obs_info[i_vx]->name() << "_"
                   << conf_info.obs_info[i_vx]->level_name() << "_"
                   << conf_info.mask_name[i];
      if(conf_info.interp_field == FieldType_Obs ||
         conf_info.interp_field == FieldType_Both) {
         obs_var_name << smooth_str;
      }

      // Build the difference variable name
      diff_var_name << cs_erase << "DIFF_"
                    << conf_info.fcst_info[i_vx]->name() << "_"
                    << conf_info.fcst_info[i_vx]->level_name() << "_"
                    << conf_info.obs_info[i_vx]->name() << "_"
                    << conf_info.obs_info[i_vx]->level_name() << "_"
                    << conf_info.mask_name[i] << smooth_str;

      // Figure out which fields should be written
      fcst_flag = !fcst_var_sa.has(fcst_var_name);
      obs_flag  = !obs_var_sa.has(obs_var_name);
      diff_flag = !diff_var_sa.has(diff_var_name);
      cmn_flag  = !cmn_var_sa.has(cmn_var_name);

      // Check the config file flags
      if(!(nc_info.do_raw)) {
         fcst_flag = false;
         obs_flag  = false;
      }
      if(!(nc_info.do_climo)) {
         cmn_flag  = false;
      }
      if(!(nc_info.do_diff)) {
         diff_flag = false;
      }

      // Skip differences for probability forecasts
      if(conf_info.fcst_info[i_vx]->p_flag()) diff_flag = false;

      // Skip empty climatology mean
      if(cmn_dp.nx() == 0 && cmn_dp.ny() == 0) cmn_flag = false;

      // Add the forecast variable
      if(fcst_flag) {

         // Define the forecast variable
         fcst_var = nc_out->add_var(fcst_var_name, ncFloat,
                                    lat_dim, lon_dim);

         // Add to the list of previously defined variables
         fcst_var_sa.add(fcst_var_name);

         // Add variable attributes for the forecast field
         add_var_att(fcst_var, "name", shc.get_fcst_var());
         att_str << cs_erase << conf_info.fcst_info[i_vx]->name()
                 << " at "
                 << conf_info.fcst_info[i_vx]->level_name();
         add_var_att(fcst_var, "long_name", att_str);
         add_var_att(fcst_var, "level", shc.get_fcst_lev());
         add_var_att(fcst_var, "units", conf_info.fcst_info[i_vx]->units());
         write_netcdf_var_times(fcst_var, fcst_dp);
         fcst_var->add_att("_FillValue", bad_data_float);
         add_var_att(fcst_var, "masking_region", conf_info.mask_name[i]);
         add_var_att(fcst_var, "smoothing_method", mthd_str);
         fcst_var->add_att("smoothing_neighborhood", wdth*wdth);
      } // end fcst_flag

      // Set up the observation variable if not already defined
      if(obs_flag) {

         // Define the observation variable
         obs_var  = nc_out->add_var(obs_var_name,  ncFloat,
                                    lat_dim, lon_dim);

         // Add to the list of previously defined variables
         obs_var_sa.add(obs_var_name);

         // Add variable attributes for the observation field
         add_var_att(obs_var, "name", shc.get_obs_var());
         att_str << cs_erase << conf_info.obs_info[i_vx]->name()
                 << " at "
                 << conf_info.obs_info[i_vx]->level_name();
         add_var_att(obs_var, "long_name", att_str);
         add_var_att(obs_var, "level", shc.get_obs_lev());
         add_var_att(obs_var, "units", conf_info.obs_info[i_vx]->units());
         write_netcdf_var_times(obs_var, obs_dp);
         obs_var->add_att("_FillValue", bad_data_float);
         add_var_att(obs_var, "masking_region", conf_info.mask_name[i]);
         add_var_att(obs_var, "smoothing_method", mthd_str);
         obs_var->add_att("smoothing_neighborhood", wdth*wdth);
      } // end obs_flag

      // Set up the difference variable if not already defined
      if(diff_flag) {

         // Define the difference variable
         diff_var = nc_out->add_var(diff_var_name, ncFloat,
                                    lat_dim, lon_dim);

         // Add to the list of previously defined variables
         diff_var_sa.add(diff_var_name);

         // Add variable attributes for the difference field
         att_str << cs_erase << "Forecast " << shc.get_fcst_var()
                 << " minus Observed " << shc.get_obs_var();
         add_var_att(diff_var, "name", att_str);
         att_str << cs_erase << conf_info.fcst_info[i_vx]->name() << " at "
                 << conf_info.fcst_info[i_vx]->level_name() << " and "
                 << conf_info.obs_info[i_vx]->name() << " at "
                 << conf_info.obs_info[i_vx]->level_name();
         add_var_att(diff_var, "long_name", att_str);
         att_str << cs_erase << shc.get_fcst_lev() << " and " << shc.get_obs_lev();
         add_var_att(diff_var, "level", att_str);
         att_str << cs_erase << conf_info.fcst_info[i_vx]->units() << " and "
                 << conf_info.obs_info[i_vx]->units();
         add_var_att(diff_var, "units", att_str);
         diff_var->add_att("_FillValue", bad_data_float);
         write_netcdf_var_times(diff_var, fcst_dp);
         diff_var->add_att("_FillValue", bad_data_float);
         add_var_att(diff_var, "masking_region", conf_info.mask_name[i]);
         add_var_att(diff_var, "smoothing_method", mthd_str);
         diff_var->add_att("smoothing_neighborhood", wdth*wdth);
      } // end diff_flag

      // Add the climatology mean variable
      if(cmn_flag) {

         // Define the forecast variable
         cmn_var = nc_out->add_var(cmn_var_name, ncFloat,
                                   lat_dim, lon_dim);

         // Add to the list of previously defined variables
         cmn_var_sa.add(cmn_var_name);

         // Add variable attributes for the climatology mean field
         add_var_att(cmn_var, "name", shc.get_fcst_var());
         att_str << cs_erase << conf_info.fcst_info[i_vx]->name()
                 << " at "
                 << conf_info.fcst_info[i_vx]->level_name();
         add_var_att(cmn_var, "long_name", att_str);
         add_var_att(cmn_var, "level", shc.get_fcst_lev());
         add_var_att(cmn_var, "units", conf_info.fcst_info[i_vx]->units());
         write_netcdf_var_times(cmn_var, cmn_dp);
         cmn_var->add_att("_FillValue", bad_data_float);
         add_var_att(cmn_var, "masking_region", conf_info.mask_name[i]);
         add_var_att(cmn_var, "smoothing_method", mthd_str);
         cmn_var->add_att("smoothing_neighborhood", wdth*wdth);
      } // end cmn_flag

      // Store the forecast, observation, difference, and climatology values
      for(x=0; x<grid.nx(); x++) {
         for(y=0; y<grid.ny(); y++) {

            n = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);

            // Check whether the mask is on or off for this point
            if(!conf_info.mask_dp[i].s_is_on(x, y)) {
               fcst_data[n] = bad_data_float;
               obs_data[n]  = bad_data_float;
               diff_data[n] = bad_data_float;
               cmn_data[n]  = bad_data_float;
               continue;
            }

            // Retrieve the data values
            fval   = fcst_dp.get(x, y);
            oval   = obs_dp.get(x, y);
            cmnval = (cmn_flag ? cmn_dp.get(x, y) : bad_data_double);

            // Store the values
            fcst_data[n] = (is_bad_data(fval)   ? bad_data_float : fval);
            obs_data[n]  = (is_bad_data(oval)   ? bad_data_float : oval);
            diff_data[n] = (is_bad_data(fval) || is_bad_data(oval) ?
                            bad_data_float : fval - oval);
            cmn_data[n]  = (is_bad_data(cmnval) ? bad_data_float : cmnval);

         } // end for y
      } // end for x

      // Write out the forecast field
      if(fcst_flag) {
         if(!fcst_var->put(&fcst_data[0], grid.ny(), grid.nx())) {
            mlog << Error << "\nwrite_nc() -> "
                 << "error with the fcst_var->put for fields "
                 << shc.get_fcst_var() << " and " << shc.get_obs_var()
                 << " and masking region " << conf_info.mask_name[i]
                 << "\n\n";
            exit(1);
         }
      }

      // Write out the observation field
      if(obs_flag) {
         if(!obs_var->put(&obs_data[0], grid.ny(), grid.nx())) {
            mlog << Error << "\nwrite_nc() -> "
                 << "error with the obs_var->put for fields "
                 << shc.get_fcst_var() << " and " << shc.get_obs_var()
                 << " and masking region " << conf_info.mask_name[i]
                 << "\n\n";
            exit(1);
         }
      }

      // Write out the difference field
      if(diff_flag) {
         if(!diff_var->put(&diff_data[0], grid.ny(), grid.nx())) {
            mlog << Error << "\nwrite_nc() -> "
                 << "error with the diff_var->put for fields "
                 << shc.get_fcst_var() << " and " << shc.get_obs_var()
                 << " and masking region " << conf_info.mask_name[i]
                 << "\n\n";
            exit(1);
         }
      }

      // Write out the climatology mean field
      if(cmn_flag) {
         if(!cmn_var->put(&cmn_data[0], grid.ny(), grid.nx())) {
            mlog << Error << "\nwrite_nc() -> "
                 << "error with the cmn_var->put for fields "
                 << shc.get_fcst_var() << " and " << shc.get_obs_var()
                 << " and masking region " << conf_info.mask_name[i]
                 << "\n\n";
            exit(1);
         }
      }

   } // end for i

   // Deallocate and clean up
   if(fcst_data) { delete [] fcst_data; fcst_data = (float *) 0; }
   if(obs_data)  { delete [] obs_data;  obs_data  = (float *) 0; }
   if(diff_data) { delete [] diff_data; diff_data = (float *) 0; }
   if(cmn_data)  { delete [] cmn_data;  cmn_data  = (float *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

static void write_nbrhd_nc(const GridStatNcOutInfo & nc_info,
               const DataPlane &fcst_dp, const DataPlane &obs_dp, int i_vx,
               const SingleThresh &fcst_st, const SingleThresh &obs_st,
               int wdth) {
   int i, n, x, y;
   int fcst_flag, obs_flag;
   double fval, oval;
   ConcatString fcst_var_name, obs_var_name;
   ConcatString att_str, mthd_str, nbrhd_str;

   float *fcst_data = (float *) 0;
   float *obs_data  = (float *) 0;

   NcVar *fcst_var = (NcVar *) 0;
   NcVar *obs_var  = (NcVar *) 0;

   // Get the interpolation strings
   mthd_str = interpmthd_to_string(InterpMthd_Nbrhd);
   if(wdth > 1) nbrhd_str << "_" << mthd_str << "_" << wdth*wdth;

   // Allocate memory for the forecast and observation fields
   fcst_data = new float [grid.nx()*grid.ny()];
   obs_data  = new float [grid.nx()*grid.ny()];

   // Process each of the masking regions
   for(i=0; i<conf_info.get_n_mask(); i++) {

      // Build the forecast variable name
      fcst_var_name << cs_erase << "FCST_"
                    << conf_info.fcst_info[i_vx]->name() << "_"
                    << conf_info.fcst_info[i_vx]->level_name() << "_"
                    << conf_info.mask_name[i] << "_"
                    << fcst_st.get_abbr_str() << nbrhd_str;

      // Build the observation variable name
      obs_var_name << cs_erase << "OBS_"
                   << conf_info.obs_info[i_vx]->name() << "_"
                   << conf_info.obs_info[i_vx]->level_name() << "_"
                   << conf_info.mask_name[i] << "_"
                   << obs_st.get_abbr_str() << nbrhd_str;

      // Figure out which fields should be written
      fcst_flag = !fcst_var_sa.has(fcst_var_name);
      obs_flag  = !obs_var_sa.has(obs_var_name);

      // Check for nothing to do
      if(!fcst_flag && !obs_flag) continue;

      // Add the forecast variable
      if(fcst_flag) {

         // Define the forecast variable
         fcst_var = nc_out->add_var(fcst_var_name, ncFloat,
                                    lat_dim, lon_dim);

         // Add to the list of previously defined variables
         fcst_var_sa.add(fcst_var_name);

         // Add variable attributes for the forecast field
         add_var_att(fcst_var, "name", shc.get_fcst_var());
         att_str << cs_erase << conf_info.fcst_info[i_vx]->name()
                 << " at "
                 << conf_info.fcst_info[i_vx]->level_name();
         add_var_att(fcst_var, "long_name", att_str);
         add_var_att(fcst_var, "level", shc.get_fcst_lev());
         add_var_att(fcst_var, "units", conf_info.fcst_info[i_vx]->units());
         write_netcdf_var_times(fcst_var, fcst_dp);
         fcst_var->add_att("_FillValue", bad_data_float);
         add_var_att(fcst_var, "masking_region", conf_info.mask_name[i]);
         add_var_att(fcst_var, "smoothing_method", mthd_str);
         add_var_att(fcst_var, "threshold", fcst_st.get_abbr_str());
         fcst_var->add_att("smoothing_neighborhood", wdth*wdth);
      } // end fcst_flag

      // Add the observation variable
      if(obs_flag) {

         // Define the observation variable
         obs_var  = nc_out->add_var(obs_var_name,  ncFloat,
                                    lat_dim, lon_dim);

         // Add to the list of previously defined variables
         obs_var_sa.add(obs_var_name);

         // Add variable attributes for the observation field
         add_var_att(obs_var, "name", shc.get_obs_var());
         att_str << cs_erase << conf_info.obs_info[i_vx]->name()
                 << " at "
                 << conf_info.obs_info[i_vx]->level_name();
         add_var_att(obs_var, "long_name", att_str);
         add_var_att(obs_var, "level", shc.get_obs_lev());
         add_var_att(obs_var, "units", conf_info.obs_info[i_vx]->units());
         write_netcdf_var_times(obs_var, obs_dp);
         obs_var->add_att("_FillValue", bad_data_float);
         add_var_att(obs_var, "masking_region", conf_info.mask_name[i]);
         add_var_att(obs_var, "smoothing_method", mthd_str);
         add_var_att(obs_var, "threshold", obs_st.get_abbr_str());
         obs_var->add_att("smoothing_neighborhood", wdth*wdth);
      } // end obs_flag

      // Store the forecast and observation values
      for(x=0; x<grid.nx(); x++) {
         for(y=0; y<grid.ny(); y++) {

            n = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);

            // Check whether the mask is on or off for this point
            if(!conf_info.mask_dp[i].s_is_on(x, y)) {
               fcst_data[n] = bad_data_float;
               obs_data[n]  = bad_data_float;
               continue;
            }

            // Retrieve the data values
            fval   = fcst_dp.get(x, y);
            oval   = obs_dp.get(x, y);

            // Store the values
            fcst_data[n] = (is_bad_data(fval)   ? bad_data_float : fval);
            obs_data[n]  = (is_bad_data(oval)   ? bad_data_float : oval);

         } // end for y
      } // end for x

      // Write out the forecast field
      if(fcst_flag) {
         if(!fcst_var->put(&fcst_data[0], grid.ny(), grid.nx())) {
            mlog << Error << "\nwrite_nbrhd_nc() -> "
                 << "error with the fcst_var->put for forecast variable "
                 << fcst_var_name << "\n\n";
            exit(1);
         }
      }

      // Write out the observation field
      if(obs_flag) {
         if(!obs_var->put(&obs_data[0], grid.ny(), grid.nx())) {
            mlog << Error << "\nwrite_nbrhd_nc() -> "
                 << "error with the obs_var->put for observation variable "
                 << obs_var_name << "\n\n";
            exit(1);
         }
      }

   } // end for i

   // Deallocate and clean up
   if(fcst_data) { delete [] fcst_data; fcst_data = (float *) 0; }
   if(obs_data)  { delete [] obs_data;  obs_data  = (float *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void add_var_att(NcVar *var, const char *att_name, const char *att_value) {

   if(att_value) var->add_att(att_name, att_value);
   else          var->add_att(att_name, na_str);

   return;
}

////////////////////////////////////////////////////////////////////////

void finish_txt_files() {
   int i;

   // Write out the contents of the STAT AsciiTable and
   // close the STAT output files
   if(stat_out) {
      *stat_out << stat_at;
      close_txt_file(stat_out, stat_file);
   }

   // Finish up each of the optional text files
   for(i=0; i<n_txt; i++) {

      // Only write the table if requested in the config file
      if(conf_info.output_flag[i] == STATOutputType_Both) {

         // Write the AsciiTable to a file
         if(txt_out[i]) {
            *txt_out[i] << txt_at[i];
            close_txt_file(txt_out[i], txt_file[i]);
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {

   // Close the output text files that were open for writing
   finish_txt_files();

   // Close the output NetCDF file
   if(nc_out) {

      // List the NetCDF file after it is finished
      mlog << Debug(1) << "Output file: " << out_nc_file << "\n";

      nc_out->close();
      delete nc_out;
      nc_out = (NcFile *) 0;
   }

   // Deallocate memory for data files
   if(fcst_mtddf) { delete fcst_mtddf; fcst_mtddf = (Met2dDataFile *) 0; }
   if(obs_mtddf)  { delete obs_mtddf;  obs_mtddf  = (Met2dDataFile *) 0; }

   // Deallocate memory for the random number generator
   rng_free(rng_ptr);

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
        << "\t[-v level]\n\n"

        << "\twhere\t\"fcst_file\" is a gridded forecast file containing "
        << "the field(s) to be verified (required).\n"

        << "\t\t\"obs_file\" is a gridded observation file containing "
        << "the verifying field(s) (required).\n"

        << "\t\t\"config_file\" is a GridStatConfig file containing "
        << "the desired configuration settings (required).\n"

        << "\t\t\"-outdir path\" overrides the default output directory "
        << "(" << out_dir << ") (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n" << flush;

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
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////


void show_version(const StringArray &)

{

cerr << "\n\n  " << met_version << "\n\n";

exit ( 0 );

return;

}


////////////////////////////////////////////////////////////////////////

