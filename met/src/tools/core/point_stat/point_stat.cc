// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2014
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   point_stat.cc
//
//   Description:
//      Based on user specified parameters, this tool compares a
//      a gridded forecast field to the point observation output of
//      the PB2NC or ASCII2NC tool.  It computes many verification
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
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static void process_command_line    (int, char **);
static void setup_first_pass        (const DataPlane &, const Grid &);

static void setup_txt_files();
static void setup_table    (AsciiTable &);

static void build_outfile_name(unixtime, int, const char *,
                               ConcatString &);

static void process_fcst_climo_files();
static void process_obs_file(int);
static void process_scores();

static void do_cts  (CTSInfo   *&, int, PairDataPoint *);
static void do_mcts (MCTSInfo   &, int, PairDataPoint *);
static void do_cnt  (CNTInfo    &, int, PairDataPoint *);
static void do_sl1l2(SL1L2Info  &, int, PairDataPoint *);
static void do_vl1l2(VL1L2Info *&, int, PairDataPoint *,
                                   int, PairDataPoint *);
static void do_pct  (PCTInfo   *&, int, PairDataPoint *);

static void finish_txt_files();

static void clean_up();

static void usage();
static void set_climo_file(const StringArray &);
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
   for(i=0; i<obs_file.n_elements(); i++) {
      process_obs_file(i);
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

   //
   // check for zero arguments
   //
   if(argc == 1) usage();

   climo_file = "none";

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
   cline.add(set_climo_file, "-climo", 1);
   cline.add(set_point_obs, "-point_obs", 1);
   cline.add(set_ncfile, "-ncfile", 1);
   cline.add(set_obs_valid_beg_time, "-obs_valid_beg", 1);
   cline.add(set_obs_valid_end_time, "-obs_valid_end", 1);
   cline.add(set_outdir, "-outdir", 1);
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
   obs_file.insert(0, cline[1]);
   config_file = cline[2];

   // Create the default config file names
   default_config_file = replace_path(default_config_filename);

   // List the config files
   mlog << Debug(1)
        << "Default Config File: " << default_config_file << "\n"
        << "User Config File: "    << config_file << "\n";

   // Read the config files
   conf_info.read_config(default_config_file, config_file);

   // Get the forecast file type from config, if present
   ftype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_fcst));

   // Read forecast file
   if(!(fcst_mtddf = mtddf_factory.new_met_2d_data_file(fcst_file, ftype))) {
      mlog << Error << "\nTrouble reading forecast file \""
           << fcst_file << "\"\n\n";
      exit(1);
   }

   // Store the forecast file type
   ftype = fcst_mtddf->file_type();

   // Read the climo file
   if(climo_flag) {
     
      // Read climatology file
      if(!(climo_mtddf = mtddf_factory.new_met_2d_data_file(climo_file, ftype))) {
         mlog << Error << "\nTrouble reading climatology file \""
              << climo_file << "\"\n\n";
         exit(1);
      }
   }

   // Process the configuration
   conf_info.process_config(ftype);   

   // Set the model name
   shc.set_model(conf_info.model);

   // Set the random number generator and seed value to be used when
   // computing bootstrap confidence intervals
   rng_set(rng_ptr, conf_info.boot_rng, conf_info.boot_seed);

   // List the input files
   mlog << Debug(1)
        << "Forecast File: "      << fcst_file << "\n"
        << "Climatology File: "   << climo_file << "\n";

   for(i=0; i<obs_file.n_elements(); i++) {
      mlog << Debug(1)
           << "Observation File: " << obs_file[i] << "\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_first_pass(const DataPlane &dp, const Grid &data_grid) {

   // Unset the flag
   is_first_pass = false;
  
   // Store the grid
   grid = data_grid;

   // Process the masks
   conf_info.process_masks(grid);

   // Setup the VxPairDataPoint objects
   conf_info.set_vx_pd();

   // Store the lead and valid times
   if(fcst_valid_ut == (unixtime) 0) fcst_valid_ut = dp.valid();
   if(is_bad_data(fcst_lead_sec))    fcst_lead_sec = dp.lead();

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_txt_files() {
   int  i, max_col, max_prob_col, max_mctc_col, n_prob, n_cat;
   ConcatString base_name;

   // Create output file names for the stat file and optional text files
   build_outfile_name(fcst_valid_ut, fcst_lead_sec, "", base_name);

   /////////////////////////////////////////////////////////////////////
   //
   // Setup the output STAT file
   //
   /////////////////////////////////////////////////////////////////////

   // Get the maximum number of data columns
   n_prob = conf_info.get_max_n_prob_fcst_thresh();
   n_cat  = conf_info.get_max_n_scal_thresh() + 1;

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

void process_fcst_climo_files() {
   int i, j;
   int n_fcst, n_climo;
   DataPlaneArray fcst_dpa, climo_dpa;
   NumArray fcst_lvl_na, climo_lvl_na;
   unixtime file_ut, beg_ut, end_ut;
   
   // Loop through each of the fields to be verified and extract
   // the forecast and climatological fields for verification
   for(i=0; i<conf_info.get_n_vx(); i++) {

      mlog << Debug(2)
           << "\n" << sep_str << "\n\n"
           << "Reading data for "
           << conf_info.vx_pd[i].fcst_info->magic_str()
           << ".\n";

      // Read the gridded data from the input forecast file
      n_fcst = fcst_mtddf->data_plane_array(*conf_info.vx_pd[i].fcst_info, fcst_dpa);

      // Check for zero fields
      if(n_fcst == 0) {
         mlog << Warning << "\nprocess_fcst_climo_files() -> "
              << "no fields matching "
              << conf_info.vx_pd[i].fcst_info->magic_str()
              << " found in file: "
              << fcst_file << "\n\n";
         continue;
      }

      // Setup the first pass through the data
      if(is_first_pass) {
         setup_first_pass(fcst_dpa[0], fcst_mtddf->grid());
      }
      // Check to make sure that the grid has not changed
      else {

         if(!(fcst_mtddf->grid() == grid)) {
            mlog << Error << "\nprocess_fcst_climo_files() -> "
                 << "The forecast grid has changed for field "
                 << conf_info.vx_pd[i].fcst_info->magic_str() << ".\n\n";
            exit(1);
         }
      }

      // Process the climo file if specified
      if(climo_flag) {

         // Read the gridded data from the input climo file
         n_climo = climo_mtddf->data_plane_array(*conf_info.vx_pd[i].fcst_info, climo_dpa);

         // Check that the grid has not changed
         if(!(climo_mtddf->grid() == grid)) {
            mlog << Error << "\nprocess_fcst_climo_files() -> "
                 << "The climatology grid has changed for field "
                 << conf_info.vx_pd[i].fcst_info->magic_str() << ".\n\n";
            exit(1);
         }
      }
      // No climo file specified
      else {
         n_climo = 0;
      }

      // For probability fields, check to see if they need to be
      // rescaled from [0, 100] to [0, 1]
      if(conf_info.vx_pd[i].fcst_info->p_flag()) {
         for(j=0; j<fcst_dpa.n_planes(); j++) {
            rescale_probability(fcst_dpa[j]);
         }
      } // end for j

      // Store information for the raw forecast fields
      conf_info.vx_pd[i].set_fcst_dpa(fcst_dpa);

      // Store information for the raw climo fields
      conf_info.vx_pd[i].set_climo_dpa(climo_dpa);

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
         beg_ut = file_ut + conf_info.beg_ds;
         end_ut = file_ut + conf_info.end_ds;
      }

      // Store the valid times for this VxPairDataPoint object
      conf_info.vx_pd[i].set_fcst_ut(fcst_valid_ut);
      conf_info.vx_pd[i].set_beg_ut(beg_ut);
      conf_info.vx_pd[i].set_end_ut(end_ut);

      // Dump out the number of levels found
      mlog << Debug(2)
           << "For " << conf_info.vx_pd[i].fcst_info->magic_str()
           << " found " << n_fcst << " forecast levels and "
           << n_climo << " climatology levels.\n";

   } // end for i

   mlog << Debug(2)
        << "\n" << sep_str << "\n\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void process_obs_file(int i_nc) {
   int j, i_obs;
   float obs_arr[obs_arr_len], hdr_arr[hdr_arr_len];
   float prev_obs_arr[obs_arr_len];
   char hdr_typ_str[max_str_len];
   char hdr_sid_str[max_str_len];
   char hdr_vld_str[max_str_len];
   char obs_qty_str[max_str_len];
   unixtime hdr_ut;
   NcFile *obs_in = (NcFile *) 0;

   // Open the observation file as a NetCDF file.
   // The observation file must be in NetCDF format as the
   // output of the PB2NC or ASCII2NC tool.
   obs_in = new NcFile(obs_file[i_nc]);

   if(!obs_in->is_valid()) {
      obs_in->close();
      delete obs_in;
      obs_in = (NcFile *) 0;

      mlog << Warning << "\nprocess_obs_file() -> "
           << "can't open observation netCDF file: "
           << obs_file[i_nc] << "\n\n";
      return;
   }

   // Define dimensions
   NcDim *strl_dim    = (NcDim *) 0; // Maximum string length
   NcDim *obs_dim     = (NcDim *) 0; // Number of observations
   NcDim *hdr_dim     = (NcDim *) 0; // Number of messages

   // Define variables
   NcVar *obs_qty_var = (NcVar *) 0;
   NcVar *obs_arr_var = (NcVar *) 0;
   NcVar *hdr_typ_var = (NcVar *) 0;
   NcVar *hdr_sid_var = (NcVar *) 0;
   NcVar *hdr_vld_var = (NcVar *) 0;
   NcVar *hdr_arr_var = (NcVar *) 0;

   // Read the dimensions
   strl_dim = obs_in->get_dim("mxstr");
   obs_dim  = obs_in->get_dim("nobs");
   hdr_dim  = obs_in->get_dim("nhdr");

   if(!strl_dim || !strl_dim->is_valid() ||
      !obs_dim  || !obs_dim->is_valid()  ||
      !hdr_dim  || !hdr_dim->is_valid()) {
      mlog << Error << "\nprocess_obs_file() -> "
           << "can't read \"mxstr\", \"nobs\" or \"nmsg\" "
           << "dimensions from netCDF file: "
           << obs_file[i_nc] << "\n\n";
      exit(1);
   }

   // Read the variables
   obs_arr_var = obs_in->get_var("obs_arr");
   hdr_typ_var = obs_in->get_var("hdr_typ");
   hdr_sid_var = obs_in->get_var("hdr_sid");
   hdr_vld_var = obs_in->get_var("hdr_vld");
   hdr_arr_var = obs_in->get_var("hdr_arr");
   obs_qty_var = has_var(obs_in, "obs_qty");

   if(!obs_arr_var || !obs_arr_var->is_valid() ||
      !hdr_typ_var || !hdr_typ_var->is_valid() ||
      !hdr_sid_var || !hdr_sid_var->is_valid() ||
      !hdr_vld_var || !hdr_vld_var->is_valid() ||
      !hdr_arr_var || !hdr_arr_var->is_valid()) {
      mlog << Error << "\nprocess_obs_file() -> "
           << "can't read \"obs_arr\", \"hdr_typ\", \"hdr_sid\", "
           << "\"hdr_vld\", or \"hdr_arr\" variables from netCDF file: "
           << obs_file[i_nc] << "\n\n";
      exit(1);
   }

   if(!obs_qty_var || !obs_qty_var->is_valid())
      mlog << Debug(3) << "Quality marker information not found input file\n";

   mlog << Debug(2)
        << "Searching " << obs_dim->size()
           << " observations from " << hdr_dim->size()
           << " messages.\n";

   // Process each observation in the file
   for(i_obs=0; i_obs<obs_dim->size(); i_obs++) {

      // Read the current observation message
      if(!obs_arr_var->set_cur((long) i_obs) ||
         !obs_arr_var->get(obs_arr, 1, obs_arr_len)) {
         mlog << Error << "\nprocess_obs_file() -> "
              << "can't read the record for observation "
              << "index " << i_obs << "\n\n";
         exit(1);
      }

      // If the current observation is UGRD, save it as the
      // previous.  If vector winds are to be computed, UGRD
      // must be followed by VGRD
      if(conf_info.get_vflag() && nint(obs_arr[1]) == ugrd_grib_code) {
         for(j=0; j<4; j++) prev_obs_arr[j] = obs_arr[j];
      }

      // If the current observation is VGRD and vector
      // winds are to be computed.  Make sure that the
      // previous observation was UGRD with the same header
      // and at the same vertical level.
      if(conf_info.get_vflag() && nint(obs_arr[1]) == vgrd_grib_code) {

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

      // Read the corresponding header array for this observation
      if(!hdr_arr_var->set_cur((long) (obs_arr[0])) ||
         !hdr_arr_var->get(hdr_arr, 1, hdr_arr_len)) {
         mlog << Error << "\nprocess_obs_file() -> "
              << "for observation index " << i_obs
              << ", can't read the header array record for header "
              << "number " << obs_arr[0] << "\n\n";
         exit(1);
      }

      // Read the corresponding header type for this observation
      if(!hdr_typ_var->set_cur((long) (obs_arr[0])) ||
         !hdr_typ_var->get(hdr_typ_str, 1, strl_dim->size())) {
         mlog << Error << "\nprocess_obs_file() -> "
              << "for observation index " << i_obs
              << ", can't read the message type record for header "
              << "number " << obs_arr[0] << "\n\n";
         exit(1);
      }

      // Read the corresponding header Station ID for this observation
      if(!hdr_sid_var->set_cur((long) (obs_arr[0])) ||
         !hdr_sid_var->get(hdr_sid_str, 1, strl_dim->size())) {
         mlog << Error << "\nprocess_obs_file() -> "
              << "for observation index " << i_obs
              << ", can't read the station ID record for header "
              << "number " << obs_arr[0] << "\n\n";
         exit(1);
      }

      // Read the corresponding valid time for this observation
      if(!hdr_vld_var->set_cur((long) (obs_arr[0])) ||
         !hdr_vld_var->get(hdr_vld_str, 1, strl_dim->size())) {
         mlog << Error << "\nprocess_obs_file() -> "
              << "for observation index " << i_obs
              << ", can't read the valid time for header "
              << "number " << obs_arr[0] << "\n\n";
         exit(1);
      }

      // Read the current observation quality flag
      strcpy(obs_qty_str, "");
      if( ( obs_qty_var && obs_qty_var->is_valid() ) &&
          ( !obs_qty_var->set_cur((long) i_obs) ||
            !obs_qty_var->get(obs_qty_str, 1, strl_dim->size()) )
        ) {
         mlog << Error << "\nprocess_obs_file() -> "
              << "can't read the quality flag for observation "
              << "index " << i_obs << "\n\n";
         exit(1);
      }

      // Convert string to a unixtime
      hdr_ut = timestring_to_unix(hdr_vld_str);

      // Check each conf_info.vx_pd object to see if this observation
      // should be added
      for(j=0; j<conf_info.get_n_vx(); j++) {

         // Check for no forecast fields
         if(conf_info.vx_pd[j].fcst_dpa.n_planes() == 0) continue;
      
         // Attempt to add the observation to the conf_info.vx_pd object
         conf_info.vx_pd[j].add_obs(hdr_arr, hdr_typ_str, hdr_sid_str,
                                    hdr_ut, obs_qty_str, obs_arr, grid);
      }

   } // end for i_obs

   // Print the duplicate report
   for(j=0; j<conf_info.get_n_vx(); j++) {
      conf_info.vx_pd[j].print_duplicate_report();
   }

   // Deallocate and clean up
   obs_in->close();
   delete obs_in;
   obs_in = (NcFile *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void process_scores() {
   int i, j, k, l, m, max_scal_t, max_prob_t, wind_t;
   PairDataPoint  *pd_ptr = (PairDataPoint *) 0;
   CTSInfo   *cts_info = (CTSInfo *) 0;
   MCTSInfo   mcts_info;
   CNTInfo    cnt_info;
   SL1L2Info  sl1l2_info;
   VL1L2Info *vl1l2_info = (VL1L2Info *) 0;
   PCTInfo   *pct_info = (PCTInfo *) 0;

   mlog << Debug(2)
        << "\n" << sep_str << "\n\n";

   // Setup the output text files as requested in the config file
   setup_txt_files();

   // Allocate enough space for the CTSInfo objects
   max_scal_t = conf_info.get_max_n_scal_thresh();
   cts_info   = new CTSInfo [max_scal_t];

   // Allocate enough space for the VL1L2Info objects
   wind_t     = conf_info.get_n_wind_thresh();
   vl1l2_info = new VL1L2Info [wind_t];

   // Allocate enough space for the PCTInfo objects
   max_prob_t = conf_info.get_max_n_prob_obs_thresh();
   pct_info   = new PCTInfo [max_prob_t];

   // Loop through the Pair Data objects, compute the scores requested,
   // and write the output.
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Check for no forecast fields
      if(conf_info.vx_pd[i].fcst_dpa.n_planes() == 0) continue;

      // Store the forecast variable name
      shc.set_fcst_var(conf_info.vx_pd[i].fcst_info->name());

      // Set the forecast level name
      shc.set_fcst_lev(conf_info.vx_pd[i].fcst_info->level_name());

      // Store the observation variable name
      shc.set_obs_var(conf_info.vx_pd[i].obs_info->name());

      // Set the observation level name
      shc.set_obs_lev(conf_info.vx_pd[i].obs_info->level_name());

      // Set the forecast lead time
      shc.set_fcst_lead_sec(conf_info.vx_pd[i].fcst_dpa[0].lead());

      // Set the forecast valid time
      shc.set_fcst_valid_beg(conf_info.vx_pd[i].fcst_dpa[0].valid());
      shc.set_fcst_valid_end(conf_info.vx_pd[i].fcst_dpa[0].valid());

      // Set the observation lead time
      shc.set_obs_lead_sec(0);

      // Set the observation valid time
      shc.set_obs_valid_beg(conf_info.vx_pd[i].beg_ut);
      shc.set_obs_valid_end(conf_info.vx_pd[i].end_ut);

      // Loop through the message types
      for(j=0; j<conf_info.get_n_msg_typ(i); j++) {

         // Store the message type
         shc.set_msg_typ(conf_info.msg_typ[i][j]);

         // Loop through the verification masking regions
         for(k=0; k<conf_info.get_n_mask(); k++) {

            // Store the verification masking region
            shc.set_mask(conf_info.mask_name[k]);

            // Loop through the interpolation methods
            for(l=0; l<conf_info.get_n_interp(); l++) {

               // Store the interpolation method and width being applied
               shc.set_interp_mthd(conf_info.interp_mthd[l]);
               shc.set_interp_wdth(conf_info.interp_wdth[l]);

               pd_ptr = &conf_info.vx_pd[i].pd[j][k][l];

               mlog << Debug(2)
                    << "Processing "
                    << conf_info.vx_pd[i].fcst_info->magic_str()
                    << " versus "
                    << conf_info.vx_pd[i].obs_info->magic_str()
                    << ", for observation type " << pd_ptr->msg_typ
                    << ", over region " << pd_ptr->mask_name
                    << ", for interpolation method "
                    << shc.get_interp_mthd_str() << "("
                    << shc.get_interp_pnts_str()
                    << "), using " << pd_ptr->n_pair << " pairs.\n";

               // Dump out detailed information about why observations were rejected
               mlog << Debug(3)
                    << "Number of matched pairs  = " << pd_ptr->n_pair << "\n"
                    << "Observations processed   = " << conf_info.vx_pd[i].n_try << "\n"
                    << "Rejected: SID exclusion  = " << conf_info.vx_pd[i].rej_sid_exc << "\n"
                    << "Rejected: GRIB code      = " << conf_info.vx_pd[i].rej_gc << "\n"
                    << "Rejected: valid time     = " << conf_info.vx_pd[i].rej_vld << "\n"
                    << "Rejected: bad obs value  = " << conf_info.vx_pd[i].rej_obs << "\n"
                    << "Rejected: off the grid   = " << conf_info.vx_pd[i].rej_grd << "\n"
                    << "Rejected: level mismatch = " << conf_info.vx_pd[i].rej_lvl << "\n"
                    << "Rejected: quality marker = " << conf_info.vx_pd[i].rej_qty << "\n"
                    << "Rejected: message type   = " << conf_info.vx_pd[i].rej_typ[j][k][l] << "\n"
                    << "Rejected: masking region = " << conf_info.vx_pd[i].rej_mask[j][k][l] << "\n"
                    << "Rejected: bad fcst value = " << conf_info.vx_pd[i].rej_fcst[j][k][l] << "\n"
                    << "Rejected: duplicates     = " << conf_info.vx_pd[i].rej_dup[j][k][l] << "\n";


               // Continue if the number of points is <= 1
               if(pd_ptr->n_pair == 0) continue;

               // Write out the MPR lines
               if(conf_info.output_flag[i_mpr] != STATOutputType_None) {

                  write_mpr_row(shc, pd_ptr,
                     conf_info.output_flag[i_mpr] == STATOutputType_Both,
                     stat_at, i_stat_row,
                     txt_at[i_mpr], i_txt_row[i_mpr]);

                  // Reset the observation valid time
                  shc.set_obs_valid_beg(conf_info.vx_pd[i].beg_ut);
                  shc.set_obs_valid_end(conf_info.vx_pd[i].end_ut);
               }

               // Compute CTS scores
               if(!conf_info.vx_pd[i].fcst_info->p_flag() &&
                   conf_info.fcst_ta[i].n_elements() > 0 &&
                  (conf_info.output_flag[i_fho] != STATOutputType_None ||
                   conf_info.output_flag[i_ctc] != STATOutputType_None ||
                   conf_info.output_flag[i_cts] != STATOutputType_None)) {

                  // Initialize
                  for(m=0; m<max_scal_t; m++) cts_info[m].clear();

                  // Compute CTS Info
                  do_cts(cts_info, i, pd_ptr);

                  // Loop through all of the thresholds
                  for(m=0; m<conf_info.fcst_ta[i].n_elements(); m++) {

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
               } // end Compute CTS scores

               // Compute MCTS scores
               if(!conf_info.vx_pd[i].fcst_info->p_flag() &&
                   conf_info.fcst_ta[i].n_elements() > 1 &&
                  (conf_info.output_flag[i_mctc] != STATOutputType_None ||
                   conf_info.output_flag[i_mcts] != STATOutputType_None)) {

                  // Initialize
                  mcts_info.clear();

                  // Compute MCTS Info
                  do_mcts(mcts_info, i, pd_ptr);

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
               } // end Compute MCTS scores

               // Compute CNT scores
               if(!conf_info.vx_pd[i].fcst_info->p_flag() &&
                   conf_info.output_flag[i_cnt] != STATOutputType_None) {

                  // Initialize
                  cnt_info.clear();

                  // Compute CNT
                  do_cnt(cnt_info, i, pd_ptr);

                  // Write out CNT
                  if(conf_info.output_flag[i_cnt] != STATOutputType_None &&
                     cnt_info.n > 0) {

                     write_cnt_row(shc, cnt_info,
                        conf_info.output_flag[i_cnt] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_cnt], i_txt_row[i_cnt]);
                  }
               } // end Compute CNT

               // Compute SL1L2 and SAL1L2 scores as long as the
               // vflag is not set
               if(!conf_info.vx_pd[i].fcst_info->p_flag() &&
                  !conf_info.vx_pd[i].fcst_info->v_flag() &&
                  (conf_info.output_flag[i_sl1l2]  != STATOutputType_None ||
                   conf_info.output_flag[i_sal1l2] != STATOutputType_None)) {

                  // Initialize
                  sl1l2_info.zero_out();

                  // Compute SL1L2 and SAL1L2
                  do_sl1l2(sl1l2_info, i, pd_ptr);

                  // Write out SL1L2
                  if(conf_info.output_flag[i_sl1l2] != STATOutputType_None &&
                     sl1l2_info.scount > 0) {

                     write_sl1l2_row(shc, sl1l2_info,
                        conf_info.output_flag[i_sl1l2] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_sl1l2], i_txt_row[i_sl1l2]);
                  }

                  // Write out SAL1L2
                  if(conf_info.output_flag[i_sal1l2] != STATOutputType_None &&
                     sl1l2_info.sacount > 0) {

                     write_sal1l2_row(shc, sl1l2_info,
                        conf_info.output_flag[i_sal1l2] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_sal1l2], i_txt_row[i_sal1l2]);
                  }
               }  // end Compute SL1L2 and SAL1L2

               // Compute VL1L2 and VAL1L2 partial sums for UGRD,VGRD
               if(!conf_info.vx_pd[i].fcst_info->p_flag() &&
                   conf_info.vx_pd[i].fcst_info->v_flag() &&
                  (conf_info.output_flag[i_vl1l2]  != STATOutputType_None ||
                   conf_info.output_flag[i_val1l2] != STATOutputType_None) &&
                   i > 0 &&
                   conf_info.vx_pd[i].fcst_info->is_v_wind() &&
                   conf_info.vx_pd[i-1].fcst_info->is_u_wind()) {

                  // Store the forecast variable name
                  shc.set_fcst_var(ugrd_vgrd_abbr_str);

                  // Store the observation variable name
                  shc.set_obs_var(ugrd_vgrd_abbr_str);

                  // Initialize
                  for(m=0; m<wind_t; m++) vl1l2_info[m].clear();

                  // Compute VL1L2 and VAL1L2
                  do_vl1l2(vl1l2_info,
                           i-1, &conf_info.vx_pd[i-1].pd[j][k][l],
                           i,   &conf_info.vx_pd[i].pd[j][k][l]);

                  // Loop through all of the wind speed thresholds
                  for(m=0; m<conf_info.fcst_wind_ta.n_elements(); m++) {

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
                  shc.set_fcst_var(conf_info.vx_pd[i].fcst_info->name());

                  // Reset the observation variable name
                  shc.set_obs_var(conf_info.vx_pd[i].obs_info->name());

               } // end Compute VL1L2 and VAL1L2

               // Compute PCT counts and scores
               if(conf_info.vx_pd[i].fcst_info->p_flag() &&
                  (conf_info.output_flag[i_pct]  != STATOutputType_None ||
                   conf_info.output_flag[i_pstd] != STATOutputType_None ||
                   conf_info.output_flag[i_pjc]  != STATOutputType_None ||
                   conf_info.output_flag[i_prc]  != STATOutputType_None)) {

                  // Initialize
                  for(m=0; m<max_prob_t; m++) pct_info[m].clear();

                  // Compute PCT
                  do_pct(pct_info, i, pd_ptr);

                  // Loop through all of the thresholds
                  for(m=0; m<max_prob_t; m++) {

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
                  }
               } // end Compute PCT

            } // end for l
         } // end for k
      } // end for j

      mlog << Debug(2)
           << "\n" << sep_str << "\n\n";
   } // end for i

   // Deallocate memory
   if(cts_info)   { delete [] cts_info;   cts_info   = (CTSInfo *)   0; }
   if(vl1l2_info) { delete [] vl1l2_info; vl1l2_info = (VL1L2Info *) 0; }
   if(pct_info)   { delete [] pct_info;   pct_info   = (PCTInfo *)   0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_cts(CTSInfo *&cts_info, int i_vx, PairDataPoint *pd_ptr) {
   int i, j, n_cts;
   NumArray f_na, o_na;

   //
   // Set up the CTSInfo thresholds and alpha values
   //
   n_cts = conf_info.fcst_ta[i_vx].n_elements();
   for(i=0; i<n_cts; i++) {
      cts_info[i].cts_fcst_thresh = conf_info.fcst_ta[i_vx][i];
      cts_info[i].cts_obs_thresh  = conf_info.obs_ta[i_vx][i];
      cts_info[i].allocate_n_alpha(conf_info.get_n_ci_alpha());

      for(j=0; j<conf_info.get_n_ci_alpha(); j++) {
         cts_info[i].alpha[j] = conf_info.ci_alpha[j];
      }
   }

   mlog << Debug(2)
        << "Computing Categorical Statistics.\n";

   //
   // If there are no matched pairs to process, return
   //
   if(pd_ptr->f_na.n_elements() == 0 ||
      pd_ptr->o_na.n_elements() == 0) return;

   //
   // Compute the stats, normal confidence intervals, and bootstrap
   // bootstrap confidence intervals
   //
   if(conf_info.boot_interval == boot_bca_flag) {
      compute_cts_stats_ci_bca(rng_ptr, pd_ptr->f_na, pd_ptr->o_na,
         conf_info.n_boot_rep,
         cts_info, n_cts,
         conf_info.output_flag[i_cts] != STATOutputType_None,
         conf_info.rank_corr_flag,
         conf_info.tmp_dir);
   }
   else {
      compute_cts_stats_ci_perc(rng_ptr, pd_ptr->f_na, pd_ptr->o_na,
         conf_info.n_boot_rep,
         conf_info.boot_rep_prop,
         cts_info, n_cts,
         conf_info.output_flag[i_cts] != STATOutputType_None,
         conf_info.rank_corr_flag,
         conf_info.tmp_dir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_mcts(MCTSInfo &mcts_info, int i_vx, PairDataPoint *pd_ptr) {
   int i;
   NumArray f_na, o_na;

   //
   // Set up the MCTSInfo size, thresholds, and alpha values
   //
   mcts_info.cts.set_size(conf_info.fcst_ta[i_vx].n_elements() + 1);
   mcts_info.cts_fcst_ta = conf_info.fcst_ta[i_vx];
   mcts_info.cts_obs_ta  = conf_info.obs_ta[i_vx];
   mcts_info.allocate_n_alpha(conf_info.get_n_ci_alpha());

   for(i=0; i<conf_info.get_n_ci_alpha(); i++) {
      mcts_info.alpha[i] = conf_info.ci_alpha[i];
   }

   mlog << Debug(2)
        << "Computing Multi-Category Statistics.\n";

   //
   // If there are no matched pairs to process, return
   //
   if(pd_ptr->f_na.n_elements() == 0 ||
      pd_ptr->o_na.n_elements() == 0) return;

   //
   // Compute the stats, normal confidence intervals, and bootstrap
   // bootstrap confidence intervals
   //
   if(conf_info.boot_interval == boot_bca_flag) {
      compute_mcts_stats_ci_bca(rng_ptr, pd_ptr->f_na, pd_ptr->o_na,
         conf_info.n_boot_rep,
         mcts_info,
         conf_info.output_flag[i_mcts] != STATOutputType_None,
         conf_info.rank_corr_flag,
         conf_info.tmp_dir);
   }
   else {
      compute_mcts_stats_ci_perc(rng_ptr, pd_ptr->f_na, pd_ptr->o_na,
         conf_info.n_boot_rep,
         conf_info.boot_rep_prop,
         mcts_info,
         conf_info.output_flag[i_mcts] != STATOutputType_None,
         conf_info.rank_corr_flag,
         conf_info.tmp_dir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_cnt(CNTInfo &cnt_info, int i_vx, PairDataPoint *pd_ptr) {
   int i;

   //
   // Set up the CNTInfo alpha values
   //
   cnt_info.allocate_n_alpha(conf_info.get_n_ci_alpha());
   for(i=0; i<conf_info.get_n_ci_alpha(); i++) {
      cnt_info.alpha[i] = conf_info.ci_alpha[i];
   }

   mlog << Debug(2)
        << "Computing Continuous Statistics.\n";

   //
   // If there are no matched pairs to process, return
   //
   if(pd_ptr->f_na.n_elements() == 0 ||
      pd_ptr->o_na.n_elements() == 0) return;

   //
   // Compute the stats, normal confidence intervals, and bootstrap
   // bootstrap confidence intervals
   //
   if(conf_info.boot_interval == boot_bca_flag) {
      compute_cnt_stats_ci_bca(rng_ptr, pd_ptr->f_na, pd_ptr->o_na,
         conf_info.vx_pd[i_vx].fcst_info->is_precipitation() &
         conf_info.vx_pd[i_vx].obs_info->is_precipitation(),
         conf_info.n_boot_rep,
         cnt_info,
         conf_info.output_flag[i_cnt] != STATOutputType_None,
         conf_info.rank_corr_flag,
         conf_info.tmp_dir);
   }
   else {
      compute_cnt_stats_ci_perc(rng_ptr, pd_ptr->f_na, pd_ptr->o_na,
         conf_info.vx_pd[i_vx].fcst_info->is_precipitation() &
         conf_info.vx_pd[i_vx].obs_info->is_precipitation(),
         conf_info.n_boot_rep,
         conf_info.boot_rep_prop,
         cnt_info,
         conf_info.output_flag[i_cnt],
         conf_info.rank_corr_flag,
         conf_info.tmp_dir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_sl1l2(SL1L2Info &s_info, int i_vx, PairDataPoint *pd_ptr) {
   int i;
   double f, o, c;
   double f_sum, o_sum, fo_sum, ff_sum, oo_sum;
   double fa_sum, oa_sum, foa_sum, ffa_sum, ooa_sum;
   double abs_err_sum;

   mlog << Debug(2)
        << "Computing Scalar Partial Sums.\n";

   // Initialize the counts and sums
   s_info.zero_out();
   f_sum  = o_sum  = fo_sum  = ff_sum  = oo_sum  = 0.0;
   fa_sum = oa_sum = foa_sum = ffa_sum = ooa_sum = 0.0;
   abs_err_sum = 0.0;

   // Loop through the pair data and compute sums
   for(i=0; i<pd_ptr->n_pair; i++) {

      f = pd_ptr->f_na[i];
      o = pd_ptr->o_na[i];
      c = pd_ptr->c_na[i];

      // Skip bad data values in the forecast or observation fields
      if(is_bad_data(f) || is_bad_data(o)) continue;

      // SL1L2 sums
      f_sum       += f;
      o_sum       += o;
      fo_sum      += f*o;
      ff_sum      += f*f;
      oo_sum      += o*o;
      abs_err_sum += abs(f-o);

      s_info.scount++;

      // SAL1L2 sums
      if(!is_bad_data(c)) {

         fa_sum  += f-c;
         oa_sum  += o-c;
         foa_sum += (f-c)*(o-c);
         ffa_sum += (f-c)*(f-c);
         ooa_sum += (o-c)*(o-c);

         s_info.sacount++;
      }
   }

   if(s_info.scount == 0) {
      mlog << Error << "\ndo_sl1l2() -> "
           << "count is zero!\n\n";
      exit(1);
   }

   // Compute the mean SL1L2 values
   s_info.fbar  = f_sum/s_info.scount;
   s_info.obar  = o_sum/s_info.scount;
   s_info.fobar = fo_sum/s_info.scount;
   s_info.ffbar = ff_sum/s_info.scount;
   s_info.oobar = oo_sum/s_info.scount;
   s_info.mae   = abs_err_sum/s_info.scount;

   // Compute the mean SAL1L2 values
   if(s_info.sacount > 0) {
      s_info.fabar  = fa_sum/s_info.sacount;
      s_info.oabar  = oa_sum/s_info.sacount;
      s_info.foabar = foa_sum/s_info.sacount;
      s_info.ffabar = ffa_sum/s_info.sacount;
      s_info.ooabar = ooa_sum/s_info.sacount;
   }
   else {
      s_info.fabar  = bad_data_double;
      s_info.oabar  = bad_data_double;
      s_info.foabar = bad_data_double;
      s_info.ffabar = bad_data_double;
      s_info.ooabar = bad_data_double;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_vl1l2(VL1L2Info *&v_info,
              int i_ugrd, PairDataPoint *ugrd_pd_ptr,
              int i_vgrd, PairDataPoint *vgrd_pd_ptr) {
   int i, j, n_thresh;
   double uf, vf, uc, vc, uo, vo, fwind, owind;
   SingleThresh fst, ost;

   // Check that the number of pairs are the same
   if(ugrd_pd_ptr->n_pair != vgrd_pd_ptr->n_pair) {
      mlog << Error << "\ndo_vl1l2() -> "
           << "the number of UGRD pairs != the number of VGRD pairs: "
           << ugrd_pd_ptr->n_pair << " != " << vgrd_pd_ptr->n_pair
           << "\n\n";
      exit(1);
   }

   // Initialize all of the VL1L2Info objects
   n_thresh = conf_info.get_n_wind_thresh();
   for(i=0; i<n_thresh; i++) {
      v_info[i].zero_out();
      v_info[i].wind_fcst_thresh = conf_info.fcst_wind_ta[i];
      v_info[i].wind_obs_thresh  = conf_info.obs_wind_ta[i];
   }

   // Loop through the pair data and compute sums
   for(i=0; i<ugrd_pd_ptr->n_pair; i++) {

      // Retrieve the U,V values
      uf = ugrd_pd_ptr->f_na[i];
      vf = vgrd_pd_ptr->f_na[i];
      uc = ugrd_pd_ptr->c_na[i];
      vc = vgrd_pd_ptr->c_na[i];
      uo = ugrd_pd_ptr->o_na[i];
      vo = vgrd_pd_ptr->o_na[i];

      // Compute wind speeds
      fwind = convert_u_v_to_wind(uf, vf);
      owind = convert_u_v_to_wind(uo, vo);

      // Skip bad data values in the forecast or observation fields
      if(is_bad_data(uf)    || is_bad_data(vf) ||
         is_bad_data(uo)    || is_bad_data(vo) ||
         is_bad_data(fwind) || is_bad_data(owind)) continue;

      // Loop through each of wind speed thresholds to be used
      for(j=0; j<n_thresh; j++) {

         fst = v_info[j].wind_fcst_thresh;
         ost = v_info[j].wind_obs_thresh;

         // Apply both wind speed thresholds
         if(fst.type != thresh_na && ost.type  != thresh_na) {
            if(!fst.check(fwind) || !ost.check(owind)) continue;
         }
         // Apply only the fcst wind speed threshold
         else if(fst.type != thresh_na && ost.type  == thresh_na) {
            if(!fst.check(fwind)) continue;
         }
         // Apply only the obs wind speed threshold
         else if(fst.type == thresh_na && ost.type  != thresh_na) {
            if(!ost.check(owind)) continue;
         }

         // Add this pair to the VL1L2 and VAL1L2 counts

         // VL1L2 sums
         v_info[j].vcount  += 1;
         v_info[j].ufbar   += uf;
         v_info[j].vfbar   += vf;
         v_info[j].uobar   += uo;
         v_info[j].vobar   += vo;
         v_info[j].uvfobar += uf*uo+vf*vo;
         v_info[j].uvffbar += uf*uf+vf*vf;
         v_info[j].uvoobar += uo*uo+vo*vo;

         // VAL1L2 sums
         if(!is_bad_data(uc) && !is_bad_data(vc)) {

            v_info[j].vacount  += 1;
            v_info[j].ufabar   += uf-uc;
            v_info[j].vfabar   += vf-vc;
            v_info[j].uoabar   += uo-uc;
            v_info[j].voabar   += vo-vc;
            v_info[j].uvfoabar += (uf-uc)*(uo-uc)+(vf-vc)*(vo-vc);
            v_info[j].uvffabar += (uf-uc)*(uf-uc)+(vf-vc)*(vf-vc);
            v_info[j].uvooabar += (uo-uc)*(uo-uc)+(vo-vc)*(vo-vc);
         }
      } // end for j
   } // end for i

   // Compute means for the VL1L2Info objects
   for(i=0; i<n_thresh; i++) {

      mlog << Debug(2)
           << "Computing Vector Partial Sums, for forecast wind speed "
           << v_info[i].wind_fcst_thresh.get_str()
           << ", for observation wind speed "
           << v_info[i].wind_obs_thresh.get_str()
           << ", using " << v_info[i].vcount << " pairs.\n";

      if(v_info[i].vcount == 0) {
         // Set to bad data
         v_info[i].ufbar   = bad_data_double;
         v_info[i].vfbar   = bad_data_double;
         v_info[i].uobar   = bad_data_double;
         v_info[i].vobar   = bad_data_double;
         v_info[i].uvfobar = bad_data_double;
         v_info[i].uvffbar = bad_data_double;
         v_info[i].uvoobar = bad_data_double;
      }
      else {
         // Compute the mean VL1L2 values
         v_info[i].ufbar   /= v_info[i].vcount;
         v_info[i].vfbar   /= v_info[i].vcount;
         v_info[i].uobar   /= v_info[i].vcount;
         v_info[i].vobar   /= v_info[i].vcount;
         v_info[i].uvfobar /= v_info[i].vcount;
         v_info[i].uvffbar /= v_info[i].vcount;
         v_info[i].uvoobar /= v_info[i].vcount;
      }

      if(v_info[i].vacount == 0) {
         // Set to bad data
         v_info[i].ufabar   = bad_data_double;
         v_info[i].vfabar   = bad_data_double;
         v_info[i].uoabar   = bad_data_double;
         v_info[i].voabar   = bad_data_double;
         v_info[i].uvfoabar = bad_data_double;
         v_info[i].uvffabar = bad_data_double;
         v_info[i].uvooabar = bad_data_double;
      }
      else {
         // Compute the mean VAL1L2 values
         v_info[i].ufabar   /= v_info[i].vacount;
         v_info[i].vfabar   /= v_info[i].vacount;
         v_info[i].uoabar   /= v_info[i].vacount;
         v_info[i].voabar   /= v_info[i].vacount;
         v_info[i].uvfoabar /= v_info[i].vacount;
         v_info[i].uvffabar /= v_info[i].vacount;
         v_info[i].uvooabar /= v_info[i].vacount;
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_pct(PCTInfo *&pct_info, int i_vx, PairDataPoint *pd_ptr) {
   int i, j, n_pct;
   NumArray f_na, o_na;

   mlog << Debug(2)
        << "Computing Probabilistic Statistics.\n";

   //
   // If there are no matched pairs to process, return
   //
   if(pd_ptr->f_na.n_elements() == 0 ||
      pd_ptr->o_na.n_elements() == 0) return;

   //
   // Set up the PCTInfo thresholds and alpha values
   //
   n_pct = conf_info.obs_ta[i_vx].n_elements();
   for(i=0; i<n_pct; i++) {

      // Use all of the selected forecast thresholds
      pct_info[i].pct_fcst_thresh = conf_info.fcst_ta[i_vx];

      // Process the observation thresholds one at a time
      pct_info[i].pct_obs_thresh  = conf_info.obs_ta[i_vx][i];

      pct_info[i].allocate_n_alpha(conf_info.get_n_ci_alpha());

      for(j=0; j<conf_info.get_n_ci_alpha(); j++) {
         pct_info[i].alpha[j] = conf_info.ci_alpha[j];
      }

      //
      // Compute the probabilistic counts and statistics
      //
      compute_pctinfo(pd_ptr->f_na, pd_ptr->o_na,
            conf_info.output_flag[i_pstd],
            pct_info[i]);
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

   // Deallocate memory for data files
   if(fcst_mtddf)  { delete fcst_mtddf;  fcst_mtddf  = (Met2dDataFile *) 0; }
   if(climo_mtddf) { delete climo_mtddf; climo_mtddf = (Met2dDataFile *) 0; }

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
        << "\t[-climo climo_file]\n"
        << "\t[-point_obs file]\n"
        << "\t[-obs_valid_beg time]\n"
        << "\t[-obs_valid_end time]\n"
        << "\t[-outdir path]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"fcst_file\" is a forecast file in either GRIB "
        << "or NetCDF format containing the field(s) to be verified "
        << "(required).\n"

        << "\t\t\"obs_file\" is an observation file in NetCDF format "
        << "(output of PB2NC or ASCII2NC) containing the verifying "
        << "observation data points (required).\n"

        << "\t\t\"config_file\" is a PointStatConfig file containing "
        << "the desired configuration settings (required).\n"

        << "\t\t\"-climo climo_file\" is a climatological file in "
        << "either Grid or NetCDF format on the same grid as the "
        << "forecast file to be\n"
        << "\t\tused when computing scalar and vector anomaly "
        << "measures.  If not provided, scalar and vector "
        << "anomaly values will not be computed (optional).\n"

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
        << mlog.verbosity_level() << ") (optional).\n\n";

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_climo_file(const StringArray & a)
{
   climo_file = a[0];
   climo_flag = true;
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
   obs_valid_beg_ut = timestring_to_unix(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_obs_valid_end_time(const StringArray & a)
{
   obs_valid_end_ut = timestring_to_unix(a[0]);
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

