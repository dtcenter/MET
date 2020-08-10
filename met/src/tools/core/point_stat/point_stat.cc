// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research led(UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   point_stat.cc
//
//   Description:
//      Based on user-specified parameters, this tool compares a
//      a gridded forecast field to the point observation output of
//      the point pre-processing tools. It computes many verification
//      scores and statistics, including confidence intervals, to
//      summarize the comparison.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    04/18/07  Halley Gotway  New
//   001    12/20/07  Halley Gotway  Allow verification for level 0
//                    for verifying PRMSL
//   002    01/24/08  Halley Gotway  In compute_cnt, print a warning
//                    message when bad data values are encountered.
//   003    01/24/08  Halley Gotway  Add support for writing the matched
//                    pair data to the MPR file.
//   004    02/04/08  Halley Gotway  Modify to read new format of the
//                    intermediate NetCDF point-observation format.
//   005    02/11/08  Halley Gotway  Remove BIAS from the CNT line
//                    since it's the same as the ME.
//   006    02/12/08  Halley Gotway  Fix bug in writing COP line to
//                    write out OY and ON rather than FY and FN.
//   007    02/12/08  Halley Gotway  Enable Point-Stat to read the
//                    NetCDF output of PCP-Combine.
//   008    02/25/08  Halley Gotway  Write the output lines using the
//                    routines in the vx_met_util library.
//   009    07/01/08  Halley Gotway   Add the rank_corr_flag to the
//                    config file to disable computing rank correlations.
//   010    07/18/08  Halley Gotway   Fix bug in the write_mpr routine.
//                    Replace i_fho with i_mpr.
//   011    09/23/08  Halley Gotway  Change argument sequence for the
//                    GRIB record access routines.
//   012    11/03/08  Halley Gotway  Use the get_file_type routine to
//                    determine the input file types.
//   013    03/13/09  Halley Gotway  Add support for verifying
//                    probabilistic forecasts.
//   014    04/21/09  Halley Gotway  Fix bug for resetting obs_var.
//   015    05/07/10  Halley Gotway  Rename process_grid() to
//                    setup_first_pass() and modify its logic.
//   016    05/24/10  Halley Gotway  Rename command line options:
//                    From -ncfile to -point_obs
//                    From -valid_beg to -obs_valid_beg
//                    From -valid_end to -obs_valid_end
//   017    05/27/10  Halley Gotway  Add -fcst_valid and -fcst_lead
//                    command line options.
//   018    06/08/10  Halley Gotway  Add support for multi-category
//                    contingency tables.
//   019    06/15/10  Halley Gotway  Dump reason codes for why
//                    point observations were rejected.
//   020    06/30/10  Halley Gotway  Enhance grid equality checks.
//   021    10/20/11  Holmes         Added use of command line class to
//                    parse the command line arguments.
//   022    11/14/11  Holmes         Added code to enable reading of
//                    multiple config files.
//   023    02/02/12  Bullock        Set default output directory to "."
//   024    03/05/12  Halley Gotway  Fix bug in processing command line
//                    for setting valid end times.
//   025    04/16/12  Halley Gotway  Switch to using vx_config library.
//   026    04/27/12  Halley Gotway  Move -fcst_valid and -fcst_lead
//                    command line options to config file.
//   027    02/25/13  Halley Gotway  Add duplicates rejection counts.
//   028    08/21/13  Halley Gotway  Fix sizing of output tables for 12
//                    or more probabilstic thresholds.
//   029    07/09/14  Halley Gotway  Add station id exclusion option.
//   030    03/02/15  Halley Gotway  Add automated regridding.
//   031    07/30/15  Halley Gotway  Add conditional continuous verification.
//   032    09/11/15  Halley Gotway  Add climatology to config file.
//   033    02/27/17  Halley Gotway  Add HiRA verification.
//   034    05/15/17  Prestopnik P   Add shape for HiRA, interp and regrid.
//   035    06/16/17  Halley Gotway  Add ECLV line type.
//   036    11/01/17  Halley Gotway  Add binned climatologies.
//   037    02/06/18  Halley Gotway  Restructure config logic to make
//                    all options settable for each verification task.
//   038    08/15/18  Halley Gotway  Add mask.llpnt type.
//   039    08/24/18  Halley Gotway  Add ECNT output for HiRA.
//   040    04/01/10  Fillmore       Add FCST and OBS units.
//   041    04/08/19  Halley Gotway  Add percentile thresholds.
//   042    10/14/19  Halley Gotway  Add support for climo distribution
//                    percentile thresholds.
//   043    11/15/19  Halley Gotway  Apply climatology bins to
//                    continuous and probabilistic statistics.
//   044    01/24/20  Halley Gotway  Add HiRA RPS output.
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

#include "point_stat.h"

#include "vx_statistics.h"
#include "vx_nc_util.h"
#include "vx_regrid.h"
#include "vx_log.h"

#include "nc_obs_util.h"

////////////////////////////////////////////////////////////////////////

#define BUFFER_SIZE (DEF_NC_BUFFER_SIZE/2)

static void process_command_line    (int, char **);
static void setup_first_pass        (const DataPlane &, const Grid &);

static void setup_txt_files();
static void setup_table    (AsciiTable &);

static void build_outfile_name(unixtime, int, const char *,
                               ConcatString &);

static void process_fcst_climo_files();
static void process_obs_file(int);
static void process_scores();

static void do_cts       (CTSInfo   *&, int, const PairDataPoint *);
static void do_mcts      (MCTSInfo   &, int, const PairDataPoint *);
static void do_cnt_sl1l2 (const PointStatVxOpt &, const PairDataPoint *);
static void do_vl1l2     (VL1L2Info *&, int, const PairDataPoint *, const PairDataPoint *);
static void do_pct       (const PointStatVxOpt &, const PairDataPoint *);
static void do_hira_ens  (              int, const PairDataPoint *);
static void do_hira_prob (              int, const PairDataPoint *);


static void finish_txt_files();

static void clean_up();

static void usage();
static void set_point_obs(const StringArray &);
static void set_ncfile(const StringArray &);
static void set_obs_valid_beg_time(const StringArray &);
static void set_obs_valid_end_time(const StringArray &);
static void set_outdir(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   int i;

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the forecast and climo files
   process_fcst_climo_files();

   // Process each observation netCDF file
   for(i=0; i<obs_file.n(); i++) {
      process_obs_file(i);
   }

   // Calculate and print observation summaries
   for(i=0; i<conf_info.get_n_vx(); i++) {
      conf_info.vx_opt[i].vx_pd.calc_obs_summary();
      conf_info.vx_opt[i].vx_pd.print_obs_summary();
   }

   // Compute the scores and write them out
   process_scores();

   // Close the text files and deallocate memory
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;
   int i;
   GrdFileType ftype;
   ConcatString default_config_file;

   out_dir = ".";

   // Check for zero arguments
   if(argc == 1) usage();

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_point_obs,          "-point_obs",     1);
   cline.add(set_ncfile,             "-ncfile",        1);
   cline.add(set_obs_valid_beg_time, "-obs_valid_beg", 1);
   cline.add(set_obs_valid_end_time, "-obs_valid_end", 1);
   cline.add(set_outdir,             "-outdir",        1);
   cline.add(set_logfile,            "-log",           1);
   cline.add(set_verbosity,          "-v",             1);

   // Parse the command line
   cline.parse();

   // Check for error. There should be three arguments left:
   // forecast, observation, and config filenames
   if(cline.n() != 3) usage();

   // Check that the end_ut >= beg_ut
   if(obs_valid_beg_ut != (unixtime) 0 &&
      obs_valid_end_ut != (unixtime) 0 &&
      obs_valid_beg_ut > obs_valid_end_ut) {

      mlog << Error << "\nprocess_command_line() -> "
           << "the ending time ("
           << unix_to_yyyymmdd_hhmmss(obs_valid_end_ut)
           << ") must be greater than the beginning time ("
           << unix_to_yyyymmdd_hhmmss(obs_valid_beg_ut)
           << ").\n\n";
      exit(1);
   }

   // Store the input forecast and observation file names
   fcst_file = cline[0];
   obs_file.insert(0, cline[1].c_str());
   config_file = cline[2];

   // Create the default config file names
   default_config_file = replace_path(default_config_filename);

   // List the config files
   mlog << Debug(1)
        << "Default Config File: " << default_config_file << "\n"
        << "User Config File: "    << config_file << "\n";

   // Read the config files
   conf_info.read_config(default_config_file.c_str(), config_file.c_str());

   // Get the forecast file type from config, if present
   ftype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_fcst));

   // Read forecast file
   if(!(fcst_mtddf = mtddf_factory.new_met_2d_data_file(fcst_file.c_str(), ftype))) {
      mlog << Error << "\nTrouble reading forecast file \""
           << fcst_file << "\"\n\n";
      exit(1);
   }

   // Use a variable index from var_name instead of GRIB code
   bool use_var_id = is_using_var_id(obs_file[0].c_str());

   // Store the forecast file type
   ftype = fcst_mtddf->file_type();

   // Process the configuration
   conf_info.process_config(ftype, use_var_id);

   // Set the model name
   shc.set_model(conf_info.model.c_str());

   // Use the first verification task to set the random number generator
   // and seed value for bootstrap confidence intervals
   rng_set(rng_ptr,
           conf_info.vx_opt[0].boot_info.rng.c_str(),
           conf_info.vx_opt[0].boot_info.seed.c_str());

   // List the input files
   mlog << Debug(1)
        << "Forecast File: " << fcst_file << "\n";

   for(i=0; i<obs_file.n(); i++) {
      mlog << Debug(1)
           << "Observation File: " << obs_file[i] << "\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_first_pass(const DataPlane &dp, const Grid &data_grid) {

   // Unset the flag
   is_first_pass = false;

   // Determine the verification grid
   grid = parse_vx_grid(conf_info.vx_opt[0].vx_pd.fcst_info->regrid(),
                        &(data_grid), &(data_grid));

   // Process the masks
   conf_info.process_masks(grid);

   // Process the geography data
   conf_info.process_geog(grid, fcst_file.c_str());

   // Setup the VxPairDataPoint objects
   conf_info.set_vx_pd();

   // Store the lead and valid times
   if(fcst_valid_ut == (unixtime) 0) fcst_valid_ut = dp.valid();
   if(is_bad_data(fcst_lead_sec))    fcst_lead_sec = dp.lead();

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_txt_files() {
   int i, max_col, max_prob_col, max_mctc_col, n_prob, n_cat, n_eclv;
   ConcatString base_name;

   // Create output file names for the stat file and optional text files
   build_outfile_name(fcst_valid_ut, fcst_lead_sec, "", base_name);

   /////////////////////////////////////////////////////////////////////
   //
   // Setup the output STAT file
   //
   /////////////////////////////////////////////////////////////////////

   // Get the maximum number of data columns
   n_prob = conf_info.get_max_n_fprob_thresh();
   n_cat  = conf_info.get_max_n_cat_thresh() + 1;
   n_eclv = conf_info.get_max_n_eclv_points();

   // Check for HiRA output
   for(i=0; i<conf_info.get_n_vx(); i++) {
      if(conf_info.vx_opt[i].hira_info.flag) {
         n_prob = max(n_prob, conf_info.vx_opt[i].hira_info.cov_ta.n());
      }
   }

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
   open_txt_file(stat_out, stat_file.c_str());

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
         open_txt_file(txt_out[i], txt_file[i].c_str());

         // Get the maximum number of columns for this line type
         switch(i) {

            case(i_mctc):
               max_col = get_n_mctc_columns(n_cat) + n_header_columns + 1;
               break;

            case(i_pct):
               max_col = get_n_pct_columns(n_prob) + n_header_columns + 1;
               break;

            case(i_pstd):
               max_col = get_n_pstd_columns(n_prob) + n_header_columns + 1;
               break;

            case(i_pjc):
               max_col = get_n_pjc_columns(n_prob) + n_header_columns + 1;
               break;

            case(i_prc):
               max_col = get_n_prc_columns(n_prob) + n_header_columns + 1;
               break;

            case(i_eclv):
               max_col = get_n_eclv_columns(n_eclv) + n_header_columns + 1;
               break;

            default:
               max_col = n_txt_columns[i] + n_header_columns + 1;
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

            case(i_eclv):
               write_eclv_header_row(1, n_eclv, txt_at[i], 0, 0);
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

void process_fcst_climo_files() {
   int i, j;
   int n_fcst;
   DataPlaneArray fcst_dpa, cmn_dpa, csd_dpa;
   unixtime file_ut, beg_ut, end_ut;

   // Loop through each of the fields to be verified and extract
   // the forecast and climatological fields for verification
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Read the gridded data from the input forecast file
      n_fcst = fcst_mtddf->data_plane_array(
                  *conf_info.vx_opt[i].vx_pd.fcst_info, fcst_dpa);

      mlog << Debug(2)
           << "\n" << sep_str << "\n\n"
           << "Reading data for "
           << conf_info.vx_opt[i].vx_pd.fcst_info->magic_str()
           << ".\n";

      // Check for zero fields
      if(n_fcst == 0) {
         mlog << Warning << "\nprocess_fcst_climo_files() -> "
              << "no fields matching "
              << conf_info.vx_opt[i].vx_pd.fcst_info->magic_str()
              << " found in file: "
              << fcst_file << "\n\n";
         continue;
      }

      // Setup the first pass through the data
      if(is_first_pass) setup_first_pass(fcst_dpa[0], fcst_mtddf->grid());

      // Regrid, if necessary
      if(!(fcst_mtddf->grid() == grid)) {
         mlog << Debug(1)
              << "Regridding " << fcst_dpa.n_planes()
              << " forecast field(s) for "
              << conf_info.vx_opt[i].vx_pd.fcst_info->magic_str()
              << " to the verification grid.\n";

         // Loop through the forecast fields
         for(j=0; j<fcst_dpa.n_planes(); j++) {
            fcst_dpa[j] = met_regrid(fcst_dpa[j], fcst_mtddf->grid(), grid,
                                     conf_info.vx_opt[i].vx_pd.fcst_info->regrid());
         }
      }

      // Rescale probabilities from [0, 100] to [0, 1]
      if(conf_info.vx_opt[i].vx_pd.fcst_info->p_flag()) {
         for(j=0; j<fcst_dpa.n_planes(); j++) {
            rescale_probability(fcst_dpa[j]);
         }
      } // end for j

      // Read climatology data
      cmn_dpa = read_climo_data_plane_array(
                   conf_info.conf.lookup_array(conf_key_climo_mean_field, false),
                   i, fcst_dpa[0].valid(), grid);
      csd_dpa = read_climo_data_plane_array(
                   conf_info.conf.lookup_array(conf_key_climo_stdev_field, false),
                   i, fcst_dpa[0].valid(), grid);

      // Store data for the current verification task
      conf_info.vx_opt[i].vx_pd.set_fcst_dpa(fcst_dpa);
      conf_info.vx_opt[i].vx_pd.set_climo_mn_dpa(cmn_dpa);
      conf_info.vx_opt[i].vx_pd.set_climo_sd_dpa(csd_dpa);

      // Get the valid time for the first field
      file_ut = fcst_dpa[0].valid();

      // If obs_valid_beg_ut and obs_valid_end_ut were set on the command
      // line, use them.  If not, use beg_ds and end_ds.
      if(obs_valid_beg_ut != (unixtime) 0 ||
         obs_valid_end_ut != (unixtime) 0) {
         beg_ut = obs_valid_beg_ut;
         end_ut = obs_valid_end_ut;
      }
      else {
         beg_ut = file_ut + conf_info.vx_opt[i].beg_ds;
         end_ut = file_ut + conf_info.vx_opt[i].end_ds;
      }

      // Store the valid times for this VxPairDataPoint object
      conf_info.vx_opt[i].vx_pd.set_fcst_ut(fcst_valid_ut);
      conf_info.vx_opt[i].vx_pd.set_beg_ut(beg_ut);
      conf_info.vx_opt[i].vx_pd.set_end_ut(end_ut);

      // Dump out the number of levels found
      mlog << Debug(2)
           << "For " << conf_info.vx_opt[i].vx_pd.fcst_info->magic_str()
           << " found " << n_fcst << " forecast levels, "
           << cmn_dpa.n_planes() << " climatology mean levels, and "
           << csd_dpa.n_planes() << " climatology standard deviation levels.\n";

   } // end for i

   // Check for no data
   if(is_first_pass) {
      mlog << Error << "\nprocess_fcst_climo_files() -> "
           << "no requested forecast data found!  Exiting...\n\n";
      exit(1);
   }

   mlog << Debug(2)
        << "\n" << sep_str << "\n\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void process_obs_file(int i_nc) {
   int j, i_obs;
   float obs_arr[OBS_ARRAY_LEN], hdr_arr[hdr_arr_len];
   float prev_obs_arr[OBS_ARRAY_LEN];
   char hdr_typ_str[max_str_len];
   char hdr_sid_str[max_str_len];
   char hdr_vld_str[max_str_len];
   char obs_qty_str[max_str_len];
   unixtime hdr_ut;
   NcFile *obs_in = (NcFile *) 0;

   // Set flags for vectors
   bool vflag = conf_info.get_vflag();
   bool is_ugrd, is_vgrd;

   // Open the observation file as a NetCDF file.
   // The observation file must be in NetCDF format as the
   // output of the PB2NC or ASCII2NC tool.
   obs_in = open_ncfile(obs_file[i_nc].c_str());

   if(IS_INVALID_NC_P(obs_in)) {
      delete obs_in;
      obs_in = (NcFile *) 0;

      mlog << Warning << "\nprocess_obs_file() -> "
           << "can't open observation netCDF file: "
           << obs_file[i_nc] << "\n\n";
      return;
   }

   // Read the dimensions and variables
   NetcdfObsVars obs_vars;
   read_nc_dims_vars(obs_vars, obs_in);

   bool use_var_id = obs_vars.use_var_id;
   if (use_var_id) {
      NcDim var_dim = get_nc_dim(obs_in,nc_dim_nvar);
      get_dim_size(&var_dim);
   }

   int exit_code = check_nc_dims_vars(obs_vars);
   if(exit_code == exit_code_no_dim) {
      mlog << Error << "\nprocess_obs_file() -> "
           << "can't read \"mxstr\", \"nobs\" or \"nmsg\" "
           << "dimensions from netCDF file: "
           << obs_file[i_nc] << "\n\n";
      exit(1);
   }

   if(exit_code == exit_code_no_hdr_vars) {
      mlog << Error << "\nprocess_obs_file() -> "
           << "can't read \"hdr_typ\", \"hdr_sid\", "
           << "or \"hdr_vld\" variables from netCDF file: "
           << obs_file[i_nc] << "\n\n";
      exit(1);
   }

   if(exit_code == exit_code_no_loc_vars) {
      mlog << Error << "\nprocess_obs_file() -> "
           << "can't read \"hdr_arr\" or \"hdr_lat\" "
           << "variables from netCDF file: "
           << obs_file[i_nc] << "\n\n";
      exit(1);
   }
   if(exit_code == exit_code_no_obs_vars) {
      mlog << Error << "\nprocess_obs_file() -> "
           << "can't read \"obs_arr\" or \"obs_val\" "
           << "variables from netCDF file: "
           << obs_file[i_nc] << "\n\n";
      exit(1);
   }

   if(IS_INVALID_NC(obs_vars.obs_qty_var))
      mlog << Debug(3) << "Quality marker information not found input file\n";

   int obs_count = get_dim_size(&obs_vars.obs_dim);
   int hdr_count = get_dim_size(&obs_vars.hdr_dim);
   int var_name_len = get_nc_string_length(obs_in, obs_vars.obs_var, nc_var_obs_var);

   mlog << Debug(2)
        << "Searching " << obs_count
        << " observations from " << hdr_count
        << " messages.\n";

   StringArray var_names;
   ConcatString var_name("");
   if (use_var_id) {
      if (!get_nc_data_to_array(obs_in, nc_var_obs_var, &var_names)) {
         mlog << Error << "\nprocess_obs_file() -> "
              << "trouble getting variable names from "
              << nc_var_obs_var << "\n\n";
         exit(1);
      }
   }

   bool use_arr_vars = !IS_INVALID_NC(obs_vars.obs_arr_var);

   int buf_size = ((obs_count > BUFFER_SIZE) ? BUFFER_SIZE : (obs_count));
   NcHeaderData header_data = get_nc_hdr_data(obs_vars);
   int typ_len = header_data.typ_len;
   int sid_len = header_data.sid_len;
   int vld_len = header_data.vld_len;
   int qty_len = get_nc_string_length(obs_in, obs_vars.obs_qty_tbl_var,
                    (use_arr_vars ? nc_var_obs_qty : nc_var_obs_qty_tbl));


   int   obs_qty_idx_block[buf_size];
   float obs_arr_block[buf_size][OBS_ARRAY_LEN];
   char  obs_qty_block[buf_size][qty_len];
   StringArray obs_qty_array;

   if (!IS_INVALID_NC(obs_vars.obs_qty_tbl_var)) {
      if (!get_nc_data_to_array(&obs_vars.obs_qty_tbl_var, &obs_qty_array)) {
         mlog << Error << "\nprocess_obs_file() -> "
              << "trouble getting obs_qty\n\n";
         exit(1);
      }
   }

   // Process each observation in the file
   int str_length, block_size;
   for(int i_block_start_idx=0; i_block_start_idx<obs_count; i_block_start_idx+=block_size) {
      block_size = (obs_count - i_block_start_idx);
      if (block_size > BUFFER_SIZE) block_size = BUFFER_SIZE;

      if (!read_nc_obs_data(obs_vars, block_size, i_block_start_idx, qty_len,
            (float *)obs_arr_block, obs_qty_idx_block, (char *)obs_qty_block)) {
         exit(1);
      }

      int hdr_idx;
      strcpy(obs_qty_str, "");
      for(int i_block_idx=0; i_block_idx<block_size; i_block_idx++) {
         i_obs = i_block_start_idx + i_block_idx;

         for (j=0; j<OBS_ARRAY_LEN; j++) {
            obs_arr[j] = obs_arr_block[i_block_idx][j];
         }

         if (use_arr_vars) {
            strcpy(obs_qty_str, obs_qty_block[i_block_idx]);
         }
         else {
            strcpy(obs_qty_str, obs_qty_array[obs_qty_idx_block[i_block_idx]].c_str());
         }

         int headerOffset = obs_arr[0];

         // Range check the header offset
         if(headerOffset < 0 || headerOffset >= hdr_count) {
            mlog << Warning << "\nprocess_obs_file() -> "
                 << "range check error for header index " << headerOffset
                 << " from observation number " << i_obs
                 << " of point observation file: " << obs_file[i_nc]
                 << "\n\n";
            continue;
         }

         // Read the corresponding header array for this observation
         hdr_arr[0] = header_data.lat_array[headerOffset];
         hdr_arr[1] = header_data.lon_array[headerOffset];
         hdr_arr[2] = header_data.elv_array[headerOffset];

         // Read the corresponding header type for this observation
         hdr_idx = use_arr_vars ? headerOffset : header_data.typ_idx_array[headerOffset];
         str_length = header_data.typ_array[hdr_idx].length();
         if (str_length > typ_len) str_length = typ_len;
         strncpy(hdr_typ_str, header_data.typ_array[hdr_idx].c_str(), str_length);
         hdr_typ_str[str_length] = bad_data_char;

         // Read the corresponding header Station ID for this observation
         hdr_idx = use_arr_vars ? headerOffset : header_data.sid_idx_array[headerOffset];
         str_length = header_data.sid_array[hdr_idx].length();
         if (str_length > sid_len) str_length = sid_len;
         strncpy(hdr_sid_str, header_data.sid_array[hdr_idx].c_str(), str_length);
         hdr_sid_str[str_length] = bad_data_char;

         // Read the corresponding valid time for this observation
         hdr_idx = use_arr_vars ? headerOffset : header_data.vld_idx_array[headerOffset];
         str_length = header_data.vld_array[hdr_idx].length();
         if (str_length > vld_len) str_length = vld_len;
         strncpy(hdr_vld_str, header_data.vld_array[hdr_idx].c_str(), str_length);
         hdr_vld_str[str_length] = bad_data_char;

         // Store the variable name
         int grib_code = obs_arr[1];
         if (use_var_id && grib_code < var_names.n()) {
            var_name   = var_names[grib_code];
            obs_arr[1] = bad_data_int;
         }
         else {
            var_name = "";
         }

         // Check for wind components
         is_ugrd = ( use_var_id &&         var_name == ugrd_abbr_str ) ||
                   (!use_var_id && nint(obs_arr[1]) == ugrd_grib_code);
         is_vgrd = ( use_var_id &&         var_name == vgrd_abbr_str ) ||
                   (!use_var_id && nint(obs_arr[1]) == vgrd_grib_code);

         // If the current observation is UGRD, save it as the
         // previous.  If vector winds are to be computed, UGRD
         // must be followed by VGRD
         if(vflag && is_ugrd) {
            for(j=0; j<4; j++) prev_obs_arr[j] = obs_arr[j];
         }

         // If the current observation is VGRD and vector
         // winds are to be computed.  Make sure that the
         // previous observation was UGRD with the same header
         // and at the same vertical level.
         if(vflag && is_vgrd) {

            if(!is_eq(obs_arr[0], prev_obs_arr[0]) ||
               !is_eq(obs_arr[2], prev_obs_arr[2]) ||
               !is_eq(obs_arr[3], prev_obs_arr[3])) {
               mlog << Error << "\nprocess_obs_file() -> "
                    << "for observation index " << i_obs
                    << ", when computing VL1L2 and/or VAL1L2 vector winds "
                    << "each UGRD observation must be followed by a VGRD "
                    << "observation with the same header and at the same "
                    << "level.\n\n";
               exit(1);
            }
         }

         // Convert string to a unixtime
         hdr_ut = timestring_to_unix(hdr_vld_str);

         // Check each conf_info.vx_pd object to see if this observation
         // should be added
         for(j=0; j<conf_info.get_n_vx(); j++) {

            // Check for no forecast fields
            if(conf_info.vx_opt[j].vx_pd.fcst_dpa.n_planes() == 0) continue;

            // Attempt to add the observation to the conf_info.vx_pd object
            conf_info.vx_opt[j].vx_pd.add_point_obs(hdr_arr,
                                         hdr_typ_str, hdr_sid_str,
                                         hdr_ut, obs_qty_str, obs_arr,
                                         grid, var_name.c_str());
         }

         obs_arr[1] = grib_code;
      }

   } // end for i_block_start_idx

   // Deallocate and clean up
   if(obs_in) {
      delete obs_in;
      obs_in = (NcFile *) 0;
   }
   clear_header_data(&header_data);

   return;
}

////////////////////////////////////////////////////////////////////////

void process_scores() {
   int i, j, k, l, m;
   int n_cat, n_wind;
   ConcatString cs;

   // Initialize pointers
   PairDataPoint *pd_ptr     = (PairDataPoint *) 0;
   CTSInfo       *cts_info   = (CTSInfo *)       0;
   MCTSInfo       mcts_info;
   VL1L2Info     *vl1l2_info = (VL1L2Info *)     0;

   mlog << Debug(2)
        << "\n" << sep_str << "\n\n";

   // Setup the output text files as requested in the config file
   setup_txt_files();

   // Store the maximum number of each threshold type
   n_cat  = conf_info.get_max_n_cat_thresh();
   n_wind = conf_info.get_max_n_wind_thresh();

   // Allocate space for output statistics types
   cts_info   = new CTSInfo   [n_cat];
   vl1l2_info = new VL1L2Info [n_wind];

   // Compute scores for each PairData object and write output
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Check for no forecast fields
      if(conf_info.vx_opt[i].vx_pd.fcst_dpa.n_planes() == 0) continue;

      // Store the description
      shc.set_desc(conf_info.vx_opt[i].vx_pd.desc.c_str());

      // Store the forecast variable name
      shc.set_fcst_var(conf_info.vx_opt[i].vx_pd.fcst_info->name_attr());

      // Store the forecast variable units
      shc.set_fcst_units(conf_info.vx_opt[i].vx_pd.fcst_info->units_attr());

      // Set the forecast level name
      shc.set_fcst_lev(conf_info.vx_opt[i].vx_pd.fcst_info->level_attr().c_str());

      // Store the observation variable name
      shc.set_obs_var(conf_info.vx_opt[i].vx_pd.obs_info->name_attr());

      // Store the observation variable units
      cs = conf_info.vx_opt[i].vx_pd.obs_info->units_attr();
      if(cs.empty()) cs = na_string;
      shc.set_obs_units(cs);

      // Set the observation level name
      shc.set_obs_lev(conf_info.vx_opt[i].vx_pd.obs_info->level_attr().c_str());

      // Set the forecast lead time
      shc.set_fcst_lead_sec(conf_info.vx_opt[i].vx_pd.fcst_dpa[0].lead());

      // Set the forecast valid time
      shc.set_fcst_valid_beg(conf_info.vx_opt[i].vx_pd.fcst_dpa[0].valid());
      shc.set_fcst_valid_end(conf_info.vx_opt[i].vx_pd.fcst_dpa[0].valid());

      // Set the observation lead time
      shc.set_obs_lead_sec(0);

      // Set the observation valid time
      shc.set_obs_valid_beg(conf_info.vx_opt[i].vx_pd.beg_ut);
      shc.set_obs_valid_end(conf_info.vx_opt[i].vx_pd.end_ut);

      // Loop through the message types
      for(j=0; j<conf_info.vx_opt[i].get_n_msg_typ(); j++) {

         // Store the message type in the obtype column
         shc.set_obtype(conf_info.vx_opt[i].msg_typ[j].c_str());

         // Loop through the verification masking regions
         for(k=0; k<conf_info.vx_opt[i].get_n_mask(); k++) {

            // Store the verification masking region
            shc.set_mask(conf_info.vx_opt[i].mask_name[k].c_str());

            // Loop through the interpolation methods
            for(l=0; l<conf_info.vx_opt[i].get_n_interp(); l++) {

               // Store the interpolation method and width being applied
               shc.set_interp_mthd(conf_info.vx_opt[i].interp_info.method[l],
                                   conf_info.vx_opt[i].interp_info.shape);
               shc.set_interp_wdth(conf_info.vx_opt[i].interp_info.width[l]);

               pd_ptr = &conf_info.vx_opt[i].vx_pd.pd[j][k][l];

               mlog << Debug(2)
                    << "Processing "
                    << conf_info.vx_opt[i].vx_pd.fcst_info->magic_str()
                    << " versus "
                    << conf_info.vx_opt[i].vx_pd.obs_info->magic_str()
                    << ", for observation type " << pd_ptr->msg_typ
                    << ", over region " << pd_ptr->mask_name
                    << ", for interpolation method "
                    << shc.get_interp_mthd() << "("
                    << shc.get_interp_pnts_str()
                    << "), using " << pd_ptr->n_obs << " matched pairs.\n";

               // Dump out detailed information about why observations were rejected
               mlog << Debug(3)
                    << "Number of matched pairs   = " << pd_ptr->n_obs << "\n"
                    << "Observations processed    = " << conf_info.vx_opt[i].vx_pd.n_try << "\n"
                    << "Rejected: station id      = " << conf_info.vx_opt[i].vx_pd.rej_sid << "\n"
                    << "Rejected: obs type        = " << conf_info.vx_opt[i].vx_pd.rej_gc << "\n"
                    << "Rejected: valid time      = " << conf_info.vx_opt[i].vx_pd.rej_vld << "\n"
                    << "Rejected: bad obs value   = " << conf_info.vx_opt[i].vx_pd.rej_obs << "\n"
                    << "Rejected: off the grid    = " << conf_info.vx_opt[i].vx_pd.rej_grd << "\n"
                    << "Rejected: topography      = " << conf_info.vx_opt[i].vx_pd.rej_topo << "\n"
                    << "Rejected: level mismatch  = " << conf_info.vx_opt[i].vx_pd.rej_lvl << "\n"
                    << "Rejected: quality marker  = " << conf_info.vx_opt[i].vx_pd.rej_qty << "\n"
                    << "Rejected: message type    = " << conf_info.vx_opt[i].vx_pd.rej_typ[j][k][l] << "\n"
                    << "Rejected: masking region  = " << conf_info.vx_opt[i].vx_pd.rej_mask[j][k][l] << "\n"
                    << "Rejected: bad fcst value  = " << conf_info.vx_opt[i].vx_pd.rej_fcst[j][k][l] << "\n"
                    << "Rejected: bad climo mean  = " << conf_info.vx_opt[i].vx_pd.rej_cmn[j][k][l] << "\n"
                    << "Rejected: bad climo stdev = " << conf_info.vx_opt[i].vx_pd.rej_csd[j][k][l] << "\n"
                    << "Rejected: duplicates      = " << conf_info.vx_opt[i].vx_pd.rej_dup[j][k][l] << "\n";

               // Continue for no matched pairs
               if(pd_ptr->n_obs == 0) continue;

               // Process percentile thresholds
               conf_info.vx_opt[i].set_perc_thresh(pd_ptr);

               // Write out the MPR lines
               if(conf_info.vx_opt[i].output_flag[i_mpr] != STATOutputType_None) {
                  write_mpr_row(shc, pd_ptr,
                     conf_info.vx_opt[i].output_flag[i_mpr],
                     stat_at, i_stat_row,
                     txt_at[i_mpr], i_txt_row[i_mpr]);

                  // Reset the observation valid time
                  shc.set_obs_valid_beg(conf_info.vx_opt[i].vx_pd.beg_ut);
                  shc.set_obs_valid_end(conf_info.vx_opt[i].vx_pd.end_ut);
               }

               // Compute CTS scores
               if(!conf_info.vx_opt[i].vx_pd.fcst_info->is_prob()                 &&
                   conf_info.vx_opt[i].fcat_ta.n() > 0                            &&
                  (conf_info.vx_opt[i].output_flag[i_fho]  != STATOutputType_None ||
                   conf_info.vx_opt[i].output_flag[i_ctc]  != STATOutputType_None ||
                   conf_info.vx_opt[i].output_flag[i_cts]  != STATOutputType_None ||
                   conf_info.vx_opt[i].output_flag[i_eclv] != STATOutputType_None)) {

                  // Initialize
                  for(m=0; m<n_cat; m++) cts_info[m].clear();

                  // Compute CTS Info
                  do_cts(cts_info, i, pd_ptr);

                  // Loop through the categorical thresholds
                  for(m=0; m<conf_info.vx_opt[i].fcat_ta.n(); m++) {

                     if(cts_info[m].cts.n() == 0) continue;

                     // Write out FHO
                     if(conf_info.vx_opt[i].output_flag[i_fho] != STATOutputType_None) {
                        write_fho_row(shc, cts_info[m],
                           conf_info.vx_opt[i].output_flag[i_fho],
                           stat_at, i_stat_row,
                           txt_at[i_fho], i_txt_row[i_fho]);
                     }

                     // Write out CTC
                     if(conf_info.vx_opt[i].output_flag[i_ctc] != STATOutputType_None) {
                        write_ctc_row(shc, cts_info[m],
                           conf_info.vx_opt[i].output_flag[i_ctc],
                           stat_at, i_stat_row,
                           txt_at[i_ctc], i_txt_row[i_ctc]);
                     }

                     // Write out CTS
                     if(conf_info.vx_opt[i].output_flag[i_cts] != STATOutputType_None) {
                        write_cts_row(shc, cts_info[m],
                           conf_info.vx_opt[i].output_flag[i_cts],
                           stat_at, i_stat_row,
                           txt_at[i_cts], i_txt_row[i_cts]);
                     }

                     // Write out ECLV
                     if(conf_info.vx_opt[i].output_flag[i_eclv] != STATOutputType_None) {
                        write_eclv_row(shc, cts_info[m], conf_info.vx_opt[i].eclv_points,
                           conf_info.vx_opt[i].output_flag[i_eclv],
                           stat_at, i_stat_row,
                           txt_at[i_eclv], i_txt_row[i_eclv]);
                     }
                  } // end for m
               } // end Compute CTS scores

               // Compute MCTS scores
               if(!conf_info.vx_opt[i].vx_pd.fcst_info->is_prob()                 &&
                   conf_info.vx_opt[i].fcat_ta.n() > 1                            &&
                  (conf_info.vx_opt[i].output_flag[i_mctc] != STATOutputType_None ||
                   conf_info.vx_opt[i].output_flag[i_mcts] != STATOutputType_None)) {

                  // Initialize
                  mcts_info.clear();

                  // Compute MCTS Info
                  do_mcts(mcts_info, i, pd_ptr);

                  // Write out MCTC
                  if(conf_info.vx_opt[i].output_flag[i_mctc] != STATOutputType_None &&
                     mcts_info.cts.total() > 0) {

                     write_mctc_row(shc, mcts_info,
                        conf_info.vx_opt[i].output_flag[i_mctc],
                        stat_at, i_stat_row,
                        txt_at[i_mctc], i_txt_row[i_mctc]);
                  }

                  // Write out MCTS
                  if(conf_info.vx_opt[i].output_flag[i_mcts] != STATOutputType_None &&
                     mcts_info.cts.total() > 0) {

                     write_mcts_row(shc, mcts_info,
                        conf_info.vx_opt[i].output_flag[i_mcts],
                        stat_at, i_stat_row,
                        txt_at[i_mcts], i_txt_row[i_mcts]);
                  }
               } // end Compute MCTS scores

               // Compute CNT, SL1L2, and SAL1L2 scores
               if(!conf_info.vx_opt[i].vx_pd.fcst_info->is_prob() &&
                  (conf_info.vx_opt[i].output_flag[i_cnt]    != STATOutputType_None ||
                   conf_info.vx_opt[i].output_flag[i_sl1l2]  != STATOutputType_None ||
                   conf_info.vx_opt[i].output_flag[i_sal1l2] != STATOutputType_None)) {
                  do_cnt_sl1l2(conf_info.vx_opt[i], pd_ptr);
               }

               // Compute VL1L2 and VAL1L2 partial sums for UGRD,VGRD
               if(!conf_info.vx_opt[i].vx_pd.fcst_info->is_prob() &&
                   conf_info.vx_opt[i].vx_pd.fcst_info->is_v_wind() &&
                   conf_info.vx_opt[i].vx_pd.fcst_info->uv_index() >= 0  &&
                  (conf_info.vx_opt[i].output_flag[i_vl1l2]  != STATOutputType_None ||
                   conf_info.vx_opt[i].output_flag[i_val1l2] != STATOutputType_None) ) {

                  // Store the forecast variable name
                  shc.set_fcst_var(ugrd_vgrd_abbr_str);

                  // Store the observation variable name
                  shc.set_obs_var(ugrd_vgrd_abbr_str);

                  // Initialize
                  for(m=0; m<n_wind; m++) vl1l2_info[m].clear();

                  // Get the index of the matching u-component
                  int ui = conf_info.vx_opt[i].vx_pd.fcst_info->uv_index();

                  // Check to make sure message types, masking regions,
                  // and interpolation methods match
                  if(conf_info.vx_opt[i].get_n_msg_typ()  !=
                     conf_info.vx_opt[ui].get_n_msg_typ() ||
                     conf_info.vx_opt[i].get_n_mask()     !=
                     conf_info.vx_opt[ui].get_n_mask()    ||
                     conf_info.vx_opt[i].get_n_interp()   !=
                     conf_info.vx_opt[ui].get_n_interp()) {
                     mlog << Warning << "\nprocess_scores() -> "
                          << "when computing VL1L2 and/or VAL1L2 vector "
                          << "partial sums, the U and V components must "
                          << "be processed using the same set of message "
                          << "types, masking regions, and interpolation "
                          << "methods. Failing to do so will cause "
                          << "unexpected results!\n\n";
                  }

                  // Compute VL1L2 and VAL1L2
                  do_vl1l2(vl1l2_info, i,
                           &conf_info.vx_opt[ui].vx_pd.pd[j][k][l],
                           &conf_info.vx_opt[i].vx_pd.pd[j][k][l]);

                  // Loop through all of the wind speed thresholds
                  for(m=0; m<conf_info.vx_opt[i].fwind_ta.n(); m++) {

                     // Write out VL1L2
                     if(conf_info.vx_opt[i].output_flag[i_vl1l2] != STATOutputType_None &&
                        vl1l2_info[m].vcount > 0) {
                        write_vl1l2_row(shc, vl1l2_info[m],
                           conf_info.vx_opt[i].output_flag[i_vl1l2],
                           stat_at, i_stat_row,
                           txt_at[i_vl1l2], i_txt_row[i_vl1l2]);
                     }

                     // Write out VAL1L2
                     if(conf_info.vx_opt[i].output_flag[i_val1l2] != STATOutputType_None &&
                        vl1l2_info[m].vacount > 0) {
                        write_val1l2_row(shc, vl1l2_info[m],
                           conf_info.vx_opt[i].output_flag[i_val1l2],
                           stat_at, i_stat_row,
                           txt_at[i_val1l2], i_txt_row[i_val1l2]);
                     }


                    // Write out VCNT
                     if(conf_info.vx_opt[i].output_flag[i_vcnt] != STATOutputType_None &&
                        vl1l2_info[m].vcount > 0) {
                        write_vcnt_row(shc, vl1l2_info[m],
                           conf_info.vx_opt[i].output_flag[i_vcnt],
                           stat_at, i_stat_row,
                           txt_at[i_vcnt], i_txt_row[i_vcnt]);
                     }


                  } // end for m

                  // Reset the forecast variable name
                  shc.set_fcst_var(conf_info.vx_opt[i].vx_pd.fcst_info->name_attr());

                  // Reset the observation variable name
                  shc.set_obs_var(conf_info.vx_opt[i].vx_pd.obs_info->name_attr());

               } // end Compute VL1L2 and VAL1L2

               // Compute PCT counts and scores
               if(conf_info.vx_opt[i].vx_pd.fcst_info->is_prob() &&
                  (conf_info.vx_opt[i].output_flag[i_pct]  != STATOutputType_None ||
                   conf_info.vx_opt[i].output_flag[i_pstd] != STATOutputType_None ||
                   conf_info.vx_opt[i].output_flag[i_pjc]  != STATOutputType_None ||
                   conf_info.vx_opt[i].output_flag[i_prc]  != STATOutputType_None ||
                   conf_info.vx_opt[i].output_flag[i_eclv] != STATOutputType_None)) {
                  do_pct(conf_info.vx_opt[i], pd_ptr);
               }

               // Reset the verification masking region
               shc.set_mask(conf_info.vx_opt[i].mask_name[k].c_str());

            } // end for l

            // Apply HiRA ensemble verification logic
            if(!conf_info.vx_opt[i].vx_pd.fcst_info->is_prob() &&
                conf_info.vx_opt[i].hira_info.flag             &&
               (conf_info.vx_opt[i].output_flag[i_ecnt] != STATOutputType_None ||
                conf_info.vx_opt[i].output_flag[i_rps]  != STATOutputType_None)) {

               pd_ptr = &conf_info.vx_opt[i].vx_pd.pd[j][k][0];

               // Process percentile thresholds
               conf_info.vx_opt[i].set_perc_thresh(pd_ptr);

               // Appy HiRA verification and write ensemble output
               do_hira_ens(i, pd_ptr);

            } // end HiRA for probabilities

            // Apply HiRA probabilistic verification logic
            if(!conf_info.vx_opt[i].vx_pd.fcst_info->is_prob() &&
                conf_info.vx_opt[i].hira_info.flag             &&
               (conf_info.vx_opt[i].output_flag[i_mpr]  != STATOutputType_None ||
                conf_info.vx_opt[i].output_flag[i_pct]  != STATOutputType_None ||
                conf_info.vx_opt[i].output_flag[i_pstd] != STATOutputType_None ||
                conf_info.vx_opt[i].output_flag[i_pjc]  != STATOutputType_None ||
                conf_info.vx_opt[i].output_flag[i_prc]  != STATOutputType_None)) {

               pd_ptr = &conf_info.vx_opt[i].vx_pd.pd[j][k][0];

               // Process percentile thresholds
               conf_info.vx_opt[i].set_perc_thresh(pd_ptr);

               // Apply HiRA verification and write probabilistic output
               do_hira_prob(i, pd_ptr);

            } // end HiRA for probabilities

         } // end for k
      } // end for j

      mlog << Debug(2)
           << "\n" << sep_str << "\n\n";
   } // end for i

   // Deallocate memory
   if(cts_info)   { delete [] cts_info;   cts_info   = (CTSInfo *)   0; }
   if(vl1l2_info) { delete [] vl1l2_info; vl1l2_info = (VL1L2Info *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_cts(CTSInfo *&cts_info, int i_vx, const PairDataPoint *pd_ptr) {
   int i, j, n_cat;

   mlog << Debug(2)
        << "Computing Categorical Statistics.\n";

   //
   // Set up the CTSInfo thresholds and alpha values
   //
   n_cat = conf_info.vx_opt[i_vx].fcat_ta.n();
   for(i=0; i<n_cat; i++) {
      cts_info[i].fthresh = conf_info.vx_opt[i_vx].fcat_ta[i];
      cts_info[i].othresh = conf_info.vx_opt[i_vx].ocat_ta[i];
      cts_info[i].allocate_n_alpha(conf_info.vx_opt[i_vx].get_n_ci_alpha());

      for(j=0; j<conf_info.vx_opt[i_vx].get_n_ci_alpha(); j++) {
         cts_info[i].alpha[j] = conf_info.vx_opt[i_vx].ci_alpha[j];
      }
   }

   //
   // If there are no matched pairs to process, return
   //
   if(pd_ptr->f_na.n() == 0 || pd_ptr->o_na.n() == 0) return;

   //
   // Compute the stats, normal confidence intervals, and bootstrap
   // bootstrap confidence intervals
   //
   if(conf_info.vx_opt[i_vx].boot_info.interval == boot_bca_flag) {
      compute_cts_stats_ci_bca(rng_ptr, *pd_ptr,
         conf_info.vx_opt[i_vx].boot_info.n_rep,
         cts_info, n_cat,
         conf_info.vx_opt[i_vx].output_flag[i_cts] != STATOutputType_None,
         conf_info.vx_opt[i_vx].rank_corr_flag,
         conf_info.tmp_dir.c_str());
   }
   else {
      compute_cts_stats_ci_perc(rng_ptr, *pd_ptr,
         conf_info.vx_opt[i_vx].boot_info.n_rep,
         conf_info.vx_opt[i_vx].boot_info.rep_prop,
         cts_info, n_cat,
         conf_info.vx_opt[i_vx].output_flag[i_cts] != STATOutputType_None,
         conf_info.vx_opt[i_vx].rank_corr_flag,
         conf_info.tmp_dir.c_str());
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_mcts(MCTSInfo &mcts_info, int i_vx, const PairDataPoint *pd_ptr) {
   int i;

   mlog << Debug(2)
        << "Computing Multi-Category Statistics.\n";

   //
   // Set up the MCTSInfo size, thresholds, and alpha values
   //
   mcts_info.cts.set_size(conf_info.vx_opt[i_vx].fcat_ta.n() + 1);
   mcts_info.set_fthresh(conf_info.vx_opt[i_vx].fcat_ta);
   mcts_info.set_othresh(conf_info.vx_opt[i_vx].ocat_ta);
   mcts_info.allocate_n_alpha(conf_info.vx_opt[i_vx].get_n_ci_alpha());

   for(i=0; i<conf_info.vx_opt[i_vx].get_n_ci_alpha(); i++) {
      mcts_info.alpha[i] = conf_info.vx_opt[i_vx].ci_alpha[i];
   }

   //
   // If there are no matched pairs to process, return
   //
   if(pd_ptr->f_na.n() == 0 || pd_ptr->o_na.n() == 0) return;

   //
   // Compute the stats, normal confidence intervals, and bootstrap
   // bootstrap confidence intervals
   //
   if(conf_info.vx_opt[i_vx].boot_info.interval == boot_bca_flag) {
      compute_mcts_stats_ci_bca(rng_ptr, *pd_ptr,
         conf_info.vx_opt[i_vx].boot_info.n_rep,
         mcts_info,
         conf_info.vx_opt[i_vx].output_flag[i_mcts] != STATOutputType_None,
         conf_info.vx_opt[i_vx].rank_corr_flag,
         conf_info.tmp_dir.c_str());
   }
   else {
      compute_mcts_stats_ci_perc(rng_ptr, *pd_ptr,
         conf_info.vx_opt[i_vx].boot_info.n_rep,
         conf_info.vx_opt[i_vx].boot_info.rep_prop,
         mcts_info,
         conf_info.vx_opt[i_vx].output_flag[i_mcts] != STATOutputType_None,
         conf_info.vx_opt[i_vx].rank_corr_flag,
         conf_info.tmp_dir.c_str());
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_cnt_sl1l2(const PointStatVxOpt &vx_opt, const PairDataPoint *pd_ptr) {
   int i, j, k, n_bin;
   PairDataPoint pd_thr, pd;
   SL1L2Info *sl1l2_info = (SL1L2Info *) 0;
   CNTInfo   *cnt_info   = (CNTInfo *)   0;

   mlog << Debug(2)
        << "Computing Scalar Partial Sums and Continuous Statistics.\n";

   // Determine the number of climo CDF bins
   n_bin = (pd_ptr->cmn_na.n_valid() > 0 && pd_ptr->csd_na.n_valid() > 0 ?
            vx_opt.get_n_cdf_bin() : 1);

   if(n_bin > 1) {
      mlog << Debug(2)
           << "Applying " << n_bin << " climatology bins.\n";
   }

   // Set flags
   bool do_sl1l2    = (vx_opt.output_flag[i_sl1l2]  != STATOutputType_None ||
                       vx_opt.output_flag[i_sal1l2] != STATOutputType_None);
   bool do_cnt      = (vx_opt.output_flag[i_cnt]    != STATOutputType_None);
   bool precip_flag = (vx_opt.vx_pd.fcst_info->is_precipitation() &&
                       vx_opt.vx_pd.obs_info->is_precipitation());

   // Allocate memory
   cnt_info   = new CNTInfo   [n_bin];
   sl1l2_info = new SL1L2Info [n_bin];

   // Process each continuous filtering threshold
   for(i=0; i<vx_opt.fcnt_ta.n(); i++) {

      // Apply continuous filtering thresholds to subset pairs
      pd_thr = subset_pairs(*pd_ptr, vx_opt.fcnt_ta[i],
                            vx_opt.ocnt_ta[i], vx_opt.cnt_logic);

      // Check for no matched pairs to process
      if(pd_thr.n_obs == 0) continue;

      // Process the climo CDF bins
      for(j=0; j<n_bin; j++) {

         // Initialize
         if(do_sl1l2) sl1l2_info[j].clear();
         if(do_cnt)   cnt_info[j].clear();

         // Apply climo CDF bins logic to subset pairs
         if(n_bin > 1) pd = subset_climo_cdf_bin(pd_thr,
                               vx_opt.cdf_info.cdf_ta, j);
         else          pd = pd_thr;

         // Check for no matched pairs to process
         if(pd.n_obs == 0) continue;

         // Compute and write SL1L2 and SAL1L2 output
         if(do_sl1l2) {

            // Store thresholds
            sl1l2_info[j].fthresh = vx_opt.fcnt_ta[i];
            sl1l2_info[j].othresh = vx_opt.ocnt_ta[i];
            sl1l2_info[j].logic   = vx_opt.cnt_logic;

            // Compute partial sums
            sl1l2_info[j].set(pd);

            // Write out SL1L2
            if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
               vx_opt.output_flag[i_sl1l2] != STATOutputType_None &&
               sl1l2_info[j].scount > 0) {

               write_sl1l2_row(shc, sl1l2_info[j],
                  vx_opt.output_flag[i_sl1l2],
                  j, n_bin, stat_at, i_stat_row,
                  txt_at[i_sl1l2], i_txt_row[i_sl1l2]);
            }

            // Write out SAL1L2
            if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
               vx_opt.output_flag[i_sal1l2] != STATOutputType_None &&
               sl1l2_info[j].sacount > 0) {

               write_sal1l2_row(shc, sl1l2_info[j],
                  vx_opt.output_flag[i_sal1l2],
                  j, n_bin, stat_at, i_stat_row,
                  txt_at[i_sal1l2], i_txt_row[i_sal1l2]);
            }
         } // end do_sl1l2

         // Compute and write CNT output
         if(do_cnt) {

            // Store thresholds
            cnt_info[j].fthresh = vx_opt.fcnt_ta[i];
            cnt_info[j].othresh = vx_opt.ocnt_ta[i];
            cnt_info[j].logic   = vx_opt.cnt_logic;

            // Setup the CNTInfo alpha values
            cnt_info[j].allocate_n_alpha(vx_opt.get_n_ci_alpha());
            for(k=0; k<vx_opt.get_n_ci_alpha(); k++) {
               cnt_info[j].alpha[k] = vx_opt.ci_alpha[k];
            }

            // Compute the stats, normal confidence intervals, and
            // bootstrap confidence intervals
            if(vx_opt.boot_info.interval == BootIntervalType_BCA) {
               compute_cnt_stats_ci_bca(rng_ptr, pd,
                  precip_flag, vx_opt.rank_corr_flag,
                  vx_opt.boot_info.n_rep,
                  cnt_info[j], conf_info.tmp_dir.c_str());
            }
            else {
               compute_cnt_stats_ci_perc(rng_ptr, pd,
                  precip_flag, vx_opt.rank_corr_flag,
                  vx_opt.boot_info.n_rep,
                  vx_opt.boot_info.rep_prop,
                  cnt_info[j], conf_info.tmp_dir.c_str());
            }

            // Write out CNT
            if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
               vx_opt.output_flag[i_cnt] != STATOutputType_None &&
               cnt_info[j].n > 0) {

               write_cnt_row(shc, cnt_info[j],
                  vx_opt.output_flag[i_cnt], j, n_bin,
                  stat_at, i_stat_row, txt_at[i_cnt], i_txt_row[i_cnt]);
            }
         } // end if do_cnt
      } // end for j (n_bin)

      // Write the mean of the climo CDF bins
      if(n_bin > 1) {

         // Compute SL1L2 climo CDF bin means
         if(vx_opt.output_flag[i_sl1l2]  != STATOutputType_None ||
            vx_opt.output_flag[i_sal1l2] != STATOutputType_None) {

            SL1L2Info sl1l2_mean;
            compute_sl1l2_mean(sl1l2_info, n_bin, sl1l2_mean);

            // Write out SL1L2
            if(vx_opt.output_flag[i_sl1l2]  != STATOutputType_None &&
               sl1l2_mean.scount > 0) {

               write_sl1l2_row(shc, sl1l2_mean,
                  vx_opt.output_flag[i_sl1l2],
                  -1, n_bin, stat_at, i_stat_row,
                  txt_at[i_sl1l2], i_txt_row[i_sl1l2]);
            }

            // Write out SAL1L2
            if(vx_opt.output_flag[i_sal1l2] != STATOutputType_None &&
               sl1l2_mean.sacount > 0) {

               write_sal1l2_row(shc, sl1l2_mean,
                  vx_opt.output_flag[i_sal1l2],
                  -1, n_bin, stat_at, i_stat_row,
                  txt_at[i_sal1l2], i_txt_row[i_sal1l2]);
            }
         }

         // Compute CNT climo CDF bin means
         if(vx_opt.output_flag[i_cnt] != STATOutputType_None) {

            CNTInfo cnt_mean;
            compute_cnt_mean(cnt_info, n_bin, cnt_mean);

            if(cnt_mean.n > 0) {

               write_cnt_row(shc, cnt_mean,
                  vx_opt.output_flag[i_cnt],
                  -1, n_bin, stat_at, i_stat_row,
                  txt_at[i_cnt], i_txt_row[i_cnt]);
            }
         }
      } // end if n_bin > 1

   } // end for i (fcnt_ta)

   // Dealloate memory
   if(sl1l2_info) { delete [] sl1l2_info; sl1l2_info = (SL1L2Info *) 0; }
   if(cnt_info)   { delete [] cnt_info;   cnt_info   = (CNTInfo *)   0;  }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_vl1l2(VL1L2Info *&v_info, int i_vx,
              const PairDataPoint *pd_u_ptr, const PairDataPoint *pd_v_ptr) {
   int i;

   //
   // Check that the number of pairs are the same
   //
   if(pd_u_ptr->n_obs != pd_v_ptr->n_obs) {
      mlog << Error << "\ndo_vl1l2() -> "
           << "unequal number of UGRD and VGRD pairs ("
           << pd_u_ptr->n_obs << " != " << pd_v_ptr->n_obs
           << ")\n\n";
      exit(1);
   }

   //
   // Set all of the VL1L2Info objects
   //
   for(i=0; i<conf_info.vx_opt[i_vx].fwind_ta.n(); i++) {

      //
      // Store thresholds
      //
      v_info[i].zero_out();
      v_info[i].fthresh = conf_info.vx_opt[i_vx].fwind_ta[i];
      v_info[i].othresh = conf_info.vx_opt[i_vx].owind_ta[i];
      v_info[i].logic   = conf_info.vx_opt[i_vx].wind_logic;

      //
      // Compute partial sums
      //
      v_info[i].set(*pd_u_ptr, *pd_v_ptr);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_pct(const PointStatVxOpt &vx_opt, const PairDataPoint *pd_ptr) {
   int i, j, k, n_bin;
   PairDataPoint pd;
   PCTInfo *pct_info = (PCTInfo *) 0;

   mlog << Debug(2)
        << "Computing Probabilistic Statistics.\n";

   // Determine the number of climo CDF bins
   n_bin = (pd_ptr->cmn_na.n_valid() > 0 && pd_ptr->csd_na.n_valid() > 0 ?
            vx_opt.get_n_cdf_bin() : 1);

   if(n_bin > 1) {
      mlog << Debug(2)
           << "Applying " << n_bin << " climatology bins.\n";
   }

   // Allocate memory
   pct_info = new PCTInfo [n_bin];

   // Process each probabilistic observation threshold
   for(i=0; i<vx_opt.ocat_ta.n(); i++) {

      // Process the climo CDF bins
      for(j=0; j<n_bin; j++) {

         // Initialize
         pct_info[j].clear();

         // Apply climo CDF bins logic to subset pairs
         if(n_bin > 1) pd = subset_climo_cdf_bin(*pd_ptr,
                               vx_opt.cdf_info.cdf_ta, j);
         else          pd = *pd_ptr;

         // Store thresholds
         pct_info[j].fthresh = vx_opt.fcat_ta;
         pct_info[j].othresh = vx_opt.ocat_ta[i];
         pct_info[j].allocate_n_alpha(vx_opt.get_n_ci_alpha());

         for(k=0; k<vx_opt.get_n_ci_alpha(); k++) {
            pct_info[j].alpha[k] = vx_opt.ci_alpha[k];
         }

         // Compute the probabilistic counts and statistics
         compute_pctinfo(pd, vx_opt.output_flag[i_pstd], pct_info[j]);

         // Check for no matched pairs to process
         if(pd.n_obs == 0) continue;

         // Write out PCT
         if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
            vx_opt.output_flag[i_pct] != STATOutputType_None) {
            write_pct_row(shc, pct_info[j],
               vx_opt.output_flag[i_pct],
               j, n_bin, stat_at, i_stat_row,
               txt_at[i_pct], i_txt_row[i_pct]);
         }

         // Write out PSTD
         if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
            vx_opt.output_flag[i_pstd] != STATOutputType_None) {
            write_pstd_row(shc, pct_info[j],
               vx_opt.output_flag[i_pstd],
               j, n_bin, stat_at, i_stat_row,
               txt_at[i_pstd], i_txt_row[i_pstd]);
         }

         // Write out PJC
         if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
            vx_opt.output_flag[i_pjc] != STATOutputType_None) {
            write_pjc_row(shc, pct_info[j],
               vx_opt.output_flag[i_pjc],
               j, n_bin, stat_at, i_stat_row,
               txt_at[i_pjc], i_txt_row[i_pjc]);
         }

         // Write out PRC
         if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
            vx_opt.output_flag[i_prc] != STATOutputType_None) {
            write_prc_row(shc, pct_info[j],
               vx_opt.output_flag[i_prc],
               j, n_bin, stat_at, i_stat_row,
               txt_at[i_prc], i_txt_row[i_prc]);
         }

         // Write out ECLV
         if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
            vx_opt.output_flag[i_eclv] != STATOutputType_None) {
            write_eclv_row(shc, pct_info[j], vx_opt.eclv_points,
               vx_opt.output_flag[i_eclv],
               j, n_bin, stat_at, i_stat_row,
               txt_at[i_eclv], i_txt_row[i_eclv]);
         }
      } // end for j (n_bin)

      // Write the mean of the climo CDF bins
      if(n_bin > 1) {

         PCTInfo pct_mean;
         compute_pct_mean(pct_info, n_bin, pct_mean);

         // Write out PSTD
         if(vx_opt.output_flag[i_pstd] != STATOutputType_None) {
            write_pstd_row(shc, pct_mean,
               vx_opt.output_flag[i_pstd],
               -1, n_bin, stat_at, i_stat_row,
               txt_at[i_pstd], i_txt_row[i_pstd]);
         }
      } // end if n_bin > 1

   } // end for i (ocnt_ta)

   // Dealloate memory
   if(pct_info) { delete [] pct_info; pct_info = (PCTInfo *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_hira_ens(int i_vx, const PairDataPoint *pd_ptr) {
   PairDataEnsemble hira_pd;
   int i, j, k, lvl_blw, lvl_abv;
   NumArray f_ens;

   // Set flag for specific humidity
   bool spfh_flag = conf_info.vx_opt[i_vx].vx_pd.fcst_info->is_specific_humidity() &&
                    conf_info.vx_opt[i_vx].vx_pd.obs_info->is_specific_humidity();

   shc.set_interp_mthd(InterpMthd_Nbrhd,
                       conf_info.vx_opt[i_vx].hira_info.shape);

   // Loop over the HiRA widths
   for(i=0; i<conf_info.vx_opt[i_vx].hira_info.width.n(); i++) {

      shc.set_interp_wdth(conf_info.vx_opt[i_vx].hira_info.width[i]);

      // Determine the number of points in the area
      GridTemplateFactory gtf;
      GridTemplate* gt = gtf.buildGT(conf_info.vx_opt[i_vx].hira_info.shape,
                                     conf_info.vx_opt[i_vx].hira_info.width[i]);

      // Initialize
      hira_pd.clear();
      hira_pd.extend(pd_ptr->n_obs);
      hira_pd.set_ens_size(gt->size());
      f_ens.extend(gt->size());

      // Process each observation point
      for(j=0; j<pd_ptr->n_obs; j++) {

         // Determine the forecast level values
         find_vert_lvl(conf_info.vx_opt[i_vx].vx_pd.fcst_dpa,
                       pd_ptr->lvl_na[j], lvl_blw, lvl_abv);

         // Get the nearby forecast values
         get_interp_points(conf_info.vx_opt[i_vx].vx_pd.fcst_dpa,
            pd_ptr->x_na[j], pd_ptr->y_na[j],
            InterpMthd_Nbrhd, conf_info.vx_opt[i_vx].hira_info.width[i],
            conf_info.vx_opt[i_vx].hira_info.shape,
            conf_info.vx_opt[i_vx].hira_info.vld_thresh, spfh_flag,
            conf_info.vx_opt[i_vx].vx_pd.fcst_info->level().type(),
            pd_ptr->lvl_na[j], lvl_blw, lvl_abv, f_ens);

         // Check for values
         if(f_ens.n() == 0) continue;

         // Skip points where climatology has been specified but is bad data
         if(conf_info.vx_opt[i_vx].vx_pd.climo_mn_dpa.n_planes() > 0 &&
            is_bad_data(pd_ptr->cmn_na[j])) continue;

         // Store the observation value
         hira_pd.add_point_obs(pd_ptr->sid_sa[j].c_str(),
            pd_ptr->lat_na[j], pd_ptr->lon_na[j],
            pd_ptr->x_na[j], pd_ptr->y_na[j], pd_ptr->vld_ta[j],
            pd_ptr->lvl_na[j], pd_ptr->elv_na[j],
            pd_ptr->o_na[j], pd_ptr->o_qc_sa[j].c_str(),
            pd_ptr->cmn_na[j], pd_ptr->csd_na[j],
            pd_ptr->wgt_na[j]);

         // Store the ensemble mean and member values
         hira_pd.mn_na.add(f_ens.mean());
         for(k=0; k<f_ens.n(); k++) {
            hira_pd.add_ens(k, f_ens[k]);
            hira_pd.add_ens_var_sums(hira_pd.n_obs-1, f_ens[k]);
         }

      } // end for j

      mlog << Debug(2)
           << "Processing "
           << conf_info.vx_opt[i_vx].vx_pd.fcst_info->magic_str()
           << " versus "
           << conf_info.vx_opt[i_vx].vx_pd.obs_info->magic_str()
           << ", for observation type " << pd_ptr->msg_typ
           << ", over region " << pd_ptr->mask_name
           << ", for interpolation method HiRA Ensemble NBRHD("
           << shc.get_interp_pnts_str()
           << "), using " << hira_pd.n_obs << " matched pairs.\n";

      // Check for zero matched pairs
      if(hira_pd.o_na.n() == 0) {
         if(gt) { delete gt; gt = 0; }
         continue;
      }

      // Write out the ECNT line
      if(conf_info.vx_opt[i_vx].output_flag[i_ecnt] != STATOutputType_None) {

         // Compute ensemble statistics
         hira_pd.compute_pair_vals(rng_ptr);
         ECNTInfo ecnt_info;
         ecnt_info.set(hira_pd);

         write_ecnt_row(shc, ecnt_info,
            conf_info.vx_opt[i_vx].output_flag[i_ecnt],
            0, 1, stat_at, i_stat_row,
            txt_at[i_ecnt], i_txt_row[i_ecnt]);
      } // end if ECNT

      // Write out the RPS line
      if(conf_info.vx_opt[i_vx].output_flag[i_rps] != STATOutputType_None) {

         // Store ensemble RPS thresholds
         RPSInfo rps_info;
         rps_info.set_prob_cat_thresh(conf_info.vx_opt[i_vx].hira_info.prob_cat_ta);

         // If prob_cat_thresh is empty, try to select other thresholds
         if(rps_info.fthresh.n() == 0) {

            // Use climo data, if avaiable
            if(hira_pd.cmn_na.n_valid()                   > 0 &&
               hira_pd.csd_na.n_valid()                   > 0 &&
               conf_info.vx_opt[i_vx].cdf_info.cdf_ta.n() > 0) {
               mlog << Debug(3) << "Resetting the empty HiRA \""
                    << conf_key_prob_cat_thresh << "\" thresholds to "
                    << "climatological distribution thresholds.\n";
               rps_info.set_cdp_thresh(conf_info.vx_opt[i_vx].cdf_info.cdf_ta);
            }
            // Otherwise, use categorical observation thresholds
            else {
               mlog << Debug(3) << "Resetting the empty HiRA \""
                    << conf_key_prob_cat_thresh << "\" thresholds to the "
                    << "observed categorical thresholds.\n";
               rps_info.set_prob_cat_thresh(conf_info.vx_opt[i_vx].ocat_ta);
            }
         }

         // Check for no thresholds
         if(rps_info.fthresh.n() == 0) {
            mlog << Debug(3) << "Skipping HiRA RPS output since no "
                 << "\"" << conf_key_prob_cat_thresh << "\" thresholds are "
                 << "defined in the \"" << conf_key_hira
                 << "\" dictionary.\n";
            if(gt) { delete gt; gt = 0; }
            break;
         }

         // Compute ensemble RPS statistics
         rps_info.set(hira_pd);

         write_rps_row(shc, rps_info,
                       conf_info.vx_opt[i_vx].output_flag[i_rps],
                       stat_at, i_stat_row,
                       txt_at[i_rps], i_txt_row[i_rps]);
      } // end if RPS

      if(gt) { delete gt; gt = 0; }

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_hira_prob(int i_vx, const PairDataPoint *pd_ptr) {
   PairDataPoint hira_pd;
   int i, j, k, lvl_blw, lvl_abv;
   double f_cov, cmn_cov;
   NumArray cmn_cov_na;
   SingleThresh cat_thresh;
   PCTInfo pct_info;

   // Set flag for specific humidity
   bool spfh_flag = conf_info.vx_opt[i_vx].vx_pd.fcst_info->is_specific_humidity() &&
                    conf_info.vx_opt[i_vx].vx_pd.obs_info->is_specific_humidity();

   shc.set_interp_mthd(InterpMthd_Nbrhd,
                       conf_info.vx_opt[i_vx].hira_info.shape);

   // Loop over categorical thresholds and HiRA widths
   for(i=0; i<conf_info.vx_opt[i_vx].fcat_ta.n(); i++) {

      cat_thresh = conf_info.vx_opt[i_vx].fcat_ta[i];

      shc.set_cov_thresh(cat_thresh);

      for(j=0; j<conf_info.vx_opt[i_vx].hira_info.width.n(); j++) {

         shc.set_interp_wdth(conf_info.vx_opt[i_vx].hira_info.width[j]);

         // Initialize
         hira_pd.clear();
         pct_info.clear();
         cmn_cov_na.erase();

         // Loop through matched pairs and replace the forecast value
         // with the HiRA fractional coverage.
         for(k=0; k<pd_ptr->n_obs; k++) {

            // Compute the fractional coverage forecast value using the
            // observation level value
            find_vert_lvl(conf_info.vx_opt[i_vx].vx_pd.fcst_dpa,
                          pd_ptr->lvl_na[k], lvl_blw, lvl_abv);

            f_cov = compute_interp(conf_info.vx_opt[i_vx].vx_pd.fcst_dpa,
                       pd_ptr->x_na[k], pd_ptr->y_na[k], pd_ptr->o_na[k],
                       pd_ptr->cmn_na[k], pd_ptr->csd_na[k],
                       InterpMthd_Nbrhd, conf_info.vx_opt[i_vx].hira_info.width[j],
                       conf_info.vx_opt[i_vx].hira_info.shape,
                       conf_info.vx_opt[i_vx].hira_info.vld_thresh, spfh_flag,
                       conf_info.vx_opt[i_vx].vx_pd.fcst_info->level().type(),
                       pd_ptr->lvl_na[k], lvl_blw, lvl_abv, &cat_thresh);

            // Check for bad data
            if(is_bad_data(f_cov)) continue;

            // Compute the fractional coverage for the climatological mean
            if(conf_info.vx_opt[i_vx].vx_pd.climo_mn_dpa.n_planes() > 0) {

               // Interpolate to the observation level
               find_vert_lvl(conf_info.vx_opt[i_vx].vx_pd.climo_mn_dpa,
                             pd_ptr->lvl_na[k], lvl_blw, lvl_abv);

               cmn_cov = compute_interp(conf_info.vx_opt[i_vx].vx_pd.climo_mn_dpa,
                            pd_ptr->x_na[k], pd_ptr->y_na[k], pd_ptr->o_na[k],
                            pd_ptr->cmn_na[k], pd_ptr->csd_na[k],
                            InterpMthd_Nbrhd, conf_info.vx_opt[i_vx].hira_info.width[j],
                            conf_info.vx_opt[i_vx].hira_info.shape,
                            conf_info.vx_opt[i_vx].hira_info.vld_thresh, spfh_flag,
                            conf_info.vx_opt[i_vx].vx_pd.fcst_info->level().type(),
                            pd_ptr->lvl_na[k], lvl_blw, lvl_abv, &cat_thresh);

               // Check for bad data
               if(is_bad_data(cmn_cov)) continue;
               else                     cmn_cov_na.add(cmn_cov);
            }

            // Store the fractional coverage pair
            hira_pd.add_point_pair(pd_ptr->sid_sa[k].c_str(),
               pd_ptr->lat_na[k], pd_ptr->lon_na[k],
               pd_ptr->x_na[k], pd_ptr->y_na[k], pd_ptr->vld_ta[k],
               pd_ptr->lvl_na[k], pd_ptr->elv_na[k],
               f_cov, pd_ptr->o_na[k], pd_ptr->o_qc_sa[k].c_str(),
               pd_ptr->cmn_na[k], pd_ptr->csd_na[k], pd_ptr->wgt_na[k]);

         } // end for k

         mlog << Debug(2)
              << "Processing "
              << conf_info.vx_opt[i_vx].vx_pd.fcst_info->magic_str()
              << conf_info.vx_opt[i_vx].fcat_ta[i].get_str()
              << " versus "
              << conf_info.vx_opt[i_vx].vx_pd.obs_info->magic_str()
              << conf_info.vx_opt[i_vx].ocat_ta[i].get_str()
              << ", for observation type " << pd_ptr->msg_typ
              << ", over region " << pd_ptr->mask_name
              << ", for interpolation method HiRA Probability NBRHD("
              << shc.get_interp_pnts_str()
              << "), using " << hira_pd.n_obs << " matched pairs.\n";

         // Check for zero matched pairs
         if(hira_pd.f_na.n() == 0 || hira_pd.o_na.n() == 0) continue;

         // Set up the PCTInfo thresholds and alpha values
         pct_info.fthresh = conf_info.vx_opt[i_vx].hira_info.cov_ta;
         pct_info.othresh = conf_info.vx_opt[i_vx].ocat_ta[i];
         pct_info.allocate_n_alpha(conf_info.vx_opt[i_vx].get_n_ci_alpha());

         for(k=0; k<conf_info.vx_opt[i_vx].get_n_ci_alpha(); k++) {
            pct_info.alpha[k] = conf_info.vx_opt[i_vx].ci_alpha[k];
         }

         // Compute the probabilistic counts and statistics
         compute_pctinfo(hira_pd, conf_info.vx_opt[i_vx].output_flag[i_pstd],
                         pct_info, &cmn_cov_na);

         // Set the contents of the output threshold columns
         shc.set_fcst_thresh (conf_info.vx_opt[i_vx].fcat_ta[i]);
         shc.set_obs_thresh  (conf_info.vx_opt[i_vx].ocat_ta[i]);
         shc.set_thresh_logic(SetLogic_None);
         shc.set_cov_thresh  (na_str);

         // Write out the MPR lines
         if(conf_info.vx_opt[i_vx].output_flag[i_mpr] != STATOutputType_None) {
            write_mpr_row(shc, &hira_pd,
               conf_info.vx_opt[i_vx].output_flag[i_mpr],
               stat_at, i_stat_row,
               txt_at[i_mpr], i_txt_row[i_mpr], false);

            // Reset the observation valid time
            shc.set_obs_valid_beg(conf_info.vx_opt[i_vx].vx_pd.beg_ut);
            shc.set_obs_valid_end(conf_info.vx_opt[i_vx].vx_pd.end_ut);
         }

         // Set cov_thresh column using the HiRA coverage thresholds
         shc.set_cov_thresh(conf_info.vx_opt[i_vx].hira_info.cov_ta);

         // Write out PCT
         if(conf_info.vx_opt[i_vx].output_flag[i_pct] != STATOutputType_None) {
            write_pct_row(shc, pct_info,
               conf_info.vx_opt[i_vx].output_flag[i_pct],1, 1,
               stat_at, i_stat_row,
               txt_at[i_pct], i_txt_row[i_pct], false);
         }

         // Write out PSTD
         if(conf_info.vx_opt[i_vx].output_flag[i_pstd] != STATOutputType_None) {
            write_pstd_row(shc, pct_info,
               conf_info.vx_opt[i_vx].output_flag[i_pstd], 1, 1,
               stat_at, i_stat_row,
               txt_at[i_pstd], i_txt_row[i_pstd], false);
         }

         // Write out PJC
         if(conf_info.vx_opt[i_vx].output_flag[i_pjc] != STATOutputType_None) {
            write_pjc_row(shc, pct_info,
               conf_info.vx_opt[i_vx].output_flag[i_pjc], 1, 1,
               stat_at, i_stat_row,
               txt_at[i_pjc], i_txt_row[i_pjc], false);
         }

         // Write out PRC
         if(conf_info.vx_opt[i_vx].output_flag[i_prc] != STATOutputType_None) {
            write_prc_row(shc, pct_info,
               conf_info.vx_opt[i_vx].output_flag[i_prc], 1, 1,
               stat_at, i_stat_row,
               txt_at[i_prc], i_txt_row[i_prc], false);
         }

      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void finish_txt_files() {
   int i;

   // Write out the contents of the STAT AsciiTable and
   // close the STAT output files
   if(stat_out) {
      *stat_out << stat_at;
      close_txt_file(stat_out, stat_file.c_str());
   }

   // Finish up each of the optional text files
   for(i=0; i<n_txt; i++) {

      // Only write the table if requested in the config file
      if(conf_info.output_flag[i] == STATOutputType_Both) {

         // Write the AsciiTable to a file
         if(txt_out[i]) {
            *txt_out[i] << txt_at[i];
            close_txt_file(txt_out[i], txt_file[i].c_str());
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {

   // Close the output text files that were open for writing
   finish_txt_files();

   // Deallocate memory for data files
   if(fcst_mtddf) { delete fcst_mtddf; fcst_mtddf = (Met2dDataFile *) 0; }

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
        << "\t[-point_obs file]\n"
        << "\t[-obs_valid_beg time]\n"
        << "\t[-obs_valid_end time]\n"
        << "\t[-outdir path]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"fcst_file\" is a gridded forecast file containing "
        << "the field(s) to be verified (required).\n"

        << "\t\t\"obs_file\" is an observation file in NetCDF format "
        << "containing the verifying point observations (required).\n"

        << "\t\t\"config_file\" is a PointStatConfig file containing "
        << "the desired configuration settings (required).\n"

        << "\t\t\"-point_obs file\" specifies additional NetCDF point "
        << "observation files to be used (optional).\n"

        << "\t\t\"-obs_valid_beg time\" in YYYYMMDD[_HH[MMSS]] sets the "
        << "beginning of the matching time window (optional).\n"

        << "\t\t\"-obs_valid_end time\" in YYYYMMDD[_HH[MMSS]] sets the "
        << "end of the matching time window (optional).\n"

        << "\t\t\"-outdir path\" overrides the default output "
        << "directory (" << out_dir << ") (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n" << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_point_obs(const StringArray & a)
{
   obs_file.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_ncfile(const StringArray & a)
{
   obs_file.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_obs_valid_beg_time(const StringArray & a)
{
   obs_valid_beg_ut = timestring_to_unix(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_obs_valid_end_time(const StringArray & a)
{
   obs_valid_end_ut = timestring_to_unix(a[0].c_str());
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
