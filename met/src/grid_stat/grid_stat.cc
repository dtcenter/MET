// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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

#include "grid_stat.h"

////////////////////////////////////////////////////////////////////////

static void process_command_line  (int, char **);
static void process_fcst_obs_files();
static void process_scores        ();

static void setup_first_pass(const WrfData &, const Grid &);
static void setup_txt_files (unixtime, int);
static void setup_table     (AsciiTable &);
static void setup_nc_file   (unixtime, int);

static void build_outfile_name(unixtime, int, const char *,
                               ConcatString &);

static void do_cts   (CTSInfo *&, int,
                      const NumArray &, const NumArray &);
static void do_mcts  (MCTSInfo &, int,
                      const NumArray &, const NumArray &);
static void do_cnt   (CNTInfo &, int,
                      const NumArray &, const NumArray &);
static void do_vl1l2 (VL1L2Info *&, int,
                      const NumArray &, const NumArray &,
                      const NumArray &, const NumArray &);
static void do_pct   (PCTInfo   *&, int,
                      const NumArray &, const NumArray &);
static void do_nbrcts(NBRCTSInfo *&, int, int, int,
                      const NumArray &, const NumArray &);
static void do_nbrcnt(NBRCNTInfo &, int, int, int,
                      const NumArray &, const NumArray &);

static void write_nc(const WrfData &, const WrfData &,
                     int, InterpMthd, int);

static void finish_txt_files();

static void clean_up();

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

   out_dir << MET_BASE << "/out/grid_stat";

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

      if(strcmp(argv[i], "-fcst_valid") == 0) {
         fcst_valid_ut = timestring_to_unix(argv[i+1]);
         i++;
      }
      else if(strcmp(argv[i], "-fcst_lead") == 0) {
         fcst_lead_sec = timestring_to_sec(argv[i+1]);
         i++;
      }
      else if(strcmp(argv[i], "-obs_valid") == 0) {
         obs_valid_ut = timestring_to_unix(argv[i+1]);
         i++;
      }
      else if(strcmp(argv[i], "-obs_lead") == 0) {
         obs_lead_sec = timestring_to_sec(argv[i+1]);
         i++;
      }
      else if(strcmp(argv[i], "-outdir") == 0) {
         out_dir = argv[i+1];
         i++;
      }
      else if(strcmp(argv[i], "-v") == 0) {
         verbosity = atoi(argv[i+1]);
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

   // Set the model name
   shc.set_model(conf_info.conf.model().sval());

   // Set the random number generator and seed value to be used when
   // computing bootstrap confidence intervals
   rng_set(rng_ptr, conf_info.conf.boot_rng().sval(),
           conf_info.conf.boot_seed().sval());

   // List the input files
   if(verbosity > 0) {
      cout << "Forecast File: "      << fcst_file   << "\n"
           << "Observation File: "   << obs_file    << "\n"
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
              << "unsupported forecast file type: "
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
   int i, j, k, m, n, status;
   int max_scal_t, max_prob_t, wind_t, frac_t;

   WrfData fcst_wd,        obs_wd;
   WrfData fcst_wd_smooth, obs_wd_smooth;

   GribRecord fcst_r, obs_r;
   Grid fcst_grid, obs_grid;
   char tmp_str[max_str_len], tmp2_str[max_str_len];
   char fcst_thresh_str[max_str_len], obs_thresh_str[max_str_len];
   NumArray f_na, o_na;

   // Objects to handle vector winds
   WrfData fcst_ugrd_wd,        obs_ugrd_wd;
   WrfData fcst_ugrd_wd_smooth, obs_ugrd_wd_smooth;
   NumArray f_ugrd_na, o_ugrd_na;

   CNTInfo     cnt_info;
   CTSInfo    *cts_info;
   MCTSInfo    mcts_info;
   VL1L2Info  *vl1l2_info;
   NBRCNTInfo  nbrcnt_info;
   NBRCTSInfo *nbrcts_info;
   PCTInfo    *pct_info;

   // Allocate enough space for the CTSInfo objects
   max_scal_t  = conf_info.get_max_n_scal_thresh();
   cts_info    = new CTSInfo [max_scal_t];

   // Allocate enough space for the VL1L2Info objects
   wind_t      = conf_info.get_n_wind_thresh();
   vl1l2_info  = new VL1L2Info [wind_t];

   // Allocate enough space for the NBRCTS objects
   frac_t      = conf_info.frac_ta.n_elements();
   nbrcts_info = new NBRCTSInfo [frac_t];

   // Allocate enough space for the PCTInfo objects
   max_prob_t  = conf_info.get_max_n_prob_obs_thresh();
   pct_info    = new PCTInfo [max_prob_t];

   // Loop through each of the fields to be verified
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Continue based on the forecast file type
      if(fcst_ftype == GbFileType) {

         status = get_grib_record(fcst_gb_file, fcst_r,
                                  conf_info.fcst_gci[i],
                                  fcst_valid_ut, fcst_lead_sec,
                                  fcst_wd, fcst_grid, verbosity);

         if(status != 0) {
            cout << "***WARNING***: process_scores() -> "
                 << conf_info.fcst_gci[i].info_str
                 << " not found in GRIB file: " << fcst_file
                 << "\n" << flush;
            continue;
         }
      }
      // fcst_ftype == NcFileType
      else {

         read_netcdf(fcst_nc_file,
                     conf_info.fcst_gci[i].abbr_str.text(),
                     tmp_str,
                     fcst_wd, fcst_grid, verbosity);
      }

      // Store the forecast lead and valid times
      if(fcst_valid_ut == (unixtime) 0) fcst_valid_ut = fcst_wd.get_valid_time();
      if(is_bad_data(fcst_lead_sec))    fcst_lead_sec = fcst_wd.get_lead_time();

      // For probability fields, check to see if they need to be
      // rescaled from [0, 100] to [0, 1]
      if(conf_info.fcst_gci[i].pflag == 1) rescale_probability(fcst_wd);

      // Set the forecast lead time
      shc.set_fcst_lead_sec(fcst_lead_sec);

      // Set the forecast valid time
      shc.set_fcst_valid_beg(fcst_valid_ut);
      shc.set_fcst_valid_end(fcst_valid_ut);

      // Continue based on the forecast file type
      if(obs_ftype == GbFileType) {

         status = get_grib_record(obs_gb_file, obs_r,
                                  conf_info.obs_gci[i],
                                  obs_valid_ut, obs_lead_sec,
                                  obs_wd, obs_grid, verbosity);

         if(status != 0) {
            cout << "***WARNING***: process_scores() -> "
                 << conf_info.obs_gci[i].info_str
                 << " not found in GRIB file: " << obs_file
                 << "\n" << flush;
            continue;
         }
      }
      // obs_ftype == NcFileType
      else {

         read_netcdf(obs_nc_file,
                     conf_info.obs_gci[i].abbr_str.text(),
                     tmp_str,
                     obs_wd, obs_grid, verbosity);
      }

      // Store the observation lead and valid times
      if(obs_valid_ut == (unixtime) 0) obs_valid_ut = obs_wd.get_valid_time();
      if(is_bad_data(obs_lead_sec))    obs_lead_sec = obs_wd.get_lead_time();

      // Set the observation lead time
      shc.set_obs_lead_sec(obs_lead_sec);

      // Set the observation valid time
      shc.set_obs_valid_beg(obs_valid_ut);
      shc.set_obs_valid_end(obs_valid_ut);

      //
      // Check that the grids match
      //
      if(!(fcst_grid == obs_grid)) {
         cerr << "\n\nERROR: main() -> "
              << "The forecast and observation grids do not match "
              << "for field " << i+1 << ".\n\n" << flush;
         exit(1);
      }

      // Check that the valid times match
      if(fcst_valid_ut != obs_valid_ut) {

         unix_to_yyyymmdd_hhmmss(fcst_valid_ut, tmp_str);
         unix_to_yyyymmdd_hhmmss(obs_valid_ut, tmp2_str);

         cout << "***WARNING***: process_scores() -> "
              << "Forecast and observation valid times do not match "
              << tmp_str << " != " << tmp2_str << " for "
              << conf_info.fcst_gci[i].info_str << " versus "
              << conf_info.obs_gci[i].info_str << ".\n\n" << flush;
      }

      // Check that the accumulation intervals match
      if(conf_info.fcst_gci[i].lvl_type == AccumLevel &&
         conf_info.obs_gci[i].lvl_type  == AccumLevel &&
         fcst_wd.get_accum_time()       != obs_wd.get_accum_time()) {

         sec_to_hhmmss(fcst_wd.get_accum_time(), tmp_str);
         sec_to_hhmmss(obs_wd.get_accum_time(), tmp2_str);

         cout << "***WARNING***: process_scores() -> "
              << "Forecast and observation accumulation times "
              << "do not match " << tmp_str << " != " << tmp2_str
              << " for " << conf_info.fcst_gci[i].info_str << " versus "
              << conf_info.obs_gci[i].info_str << ".\n\n" << flush;
      }

      // This is the first pass through the loop and grid is unset
      if(grid.nx() == 0 && grid.ny() == 0) {

         // Setup the first pass through the data
         setup_first_pass(fcst_wd, fcst_grid);
      }
      else {

         // For multiple verification fields, check to make sure that
         // the grids don't change
         if(!(fcst_grid == grid) || !(obs_grid == grid)) {
            cerr << "\n\nERROR: process_scores() -> "
                 << "The grid must remain the same for all fields.\n\n"
                 << flush;
            exit(1);
         }
      }

      // Setup strings for the GRIB code
      if(conf_info.fcst_gci[i].code == apcp_grib_code &&
         conf_info.obs_gci[i].code  == apcp_grib_code) {
         shc.set_msg_typ("MC_PCP");
      }
      else {
         shc.set_msg_typ("ANALYS");
      }

      // Store the forecast variable name
      shc.set_fcst_var(conf_info.fcst_gci[i].abbr_str.text());

      // Set the forecast level name
      shc.set_fcst_lev(conf_info.fcst_gci[i].lvl_str.text());

      // Store the observation variable name
      shc.set_obs_var(conf_info.obs_gci[i].abbr_str.text());

      // Set the observation level name
      shc.set_obs_lev(conf_info.obs_gci[i].lvl_str.text());

      if(verbosity > 1) {
         cout << "\n" << sep_str << "\n\n" << flush;
      }

      // If verifying vector winds, store the UGRD fields
      if(conf_info.fcst_gci[i].code  == ugrd_grib_code &&
         conf_info.obs_gci[i].code   == ugrd_grib_code &&
         conf_info.fcst_gci[i].vflag == 1 &&
         conf_info.obs_gci[i].vflag  == 1) {

         fcst_ugrd_wd = fcst_wd;
         obs_ugrd_wd  = obs_wd;
      }

      // Loop through and apply each of the smoothing operations
      for(j=0; j<conf_info.get_n_interp(); j++) {

         // Store the interpolation method and width being applied
         shc.set_interp_mthd(conf_info.interp_mthd[j]);
         shc.set_interp_wdth(conf_info.interp_wdth[j]);

         // If requested in the config file, smooth the forecast field
         if(conf_info.conf.interp_flag().ival() == 1 ||
            conf_info.conf.interp_flag().ival() == 3) {
            fcst_wd_smooth = smooth_field(fcst_wd,
                                conf_info.interp_mthd[j],
                                conf_info.interp_wdth[j],
                                conf_info.conf.interp_thresh().dval());
         }
         // Do not smooth the forecast field
         else {
            fcst_wd_smooth = fcst_wd;
         }

         // If requested in the config file, smooth the observation field
         if(conf_info.conf.interp_flag().ival() == 2 ||
            conf_info.conf.interp_flag().ival() == 3) {
            obs_wd_smooth  = smooth_field(obs_wd,
                                conf_info.interp_mthd[j],
                                conf_info.interp_wdth[j],
                                conf_info.conf.interp_thresh().dval());
         }
         // Do not smooth the observation field
         else {
            obs_wd_smooth = obs_wd;
         }

         // Loop through the masks to be applied
         for(k=0; k<conf_info.get_n_mask(); k++) {

            // Apply the current mask to the fields
            apply_mask(fcst_wd_smooth, obs_wd_smooth,
                       conf_info.mask_wd[k],
                       f_na, o_na);

            // Continue if no pairs were found
            if(f_na.n_elements() == 0) continue;

            // Set the mask name
            shc.set_mask(conf_info.mask_name[k]);

            if(verbosity > 1) {

               cout << "Processing "
                    << conf_info.fcst_gci[i].info_str
                    << " versus "
                    << conf_info.obs_gci[i].info_str
                    << ", for interpolation method "
                    << shc.get_interp_mthd_str() << "("
                    << shc.get_interp_pnts_str()
                    << "), over region " << shc.get_mask()
                    << ", using " << f_na.n_elements() << " pairs.\n"
                    << flush;
            }

            // Compute CTS scores
            if(conf_info.fcst_gci[i].pflag == 0 &&
               conf_info.fcst_ta[i].n_elements() > 0 &&
               (conf_info.conf.output_flag(i_fho).ival() ||
                conf_info.conf.output_flag(i_ctc).ival() ||
                conf_info.conf.output_flag(i_cts).ival())) {

               // Initialize
               for(m=0; m<max_scal_t; m++) cts_info[m].clear();

               // Compute CTS
               do_cts(cts_info, i, f_na, o_na);

               // Loop through all of the thresholds
               for(m=0; m<conf_info.fcst_ta[i].n_elements(); m++) {

                  // Write out FHO
                  if(conf_info.conf.output_flag(i_fho).ival() &&
                     cts_info[m].cts.n() > 0) {

                     write_fho_row(shc, cts_info[m],
                        conf_info.conf.output_flag(i_fho).ival(),
                        stat_at, i_stat_row,
                        txt_at[i_fho], i_txt_row[i_fho]);
                  }

                  // Write out CTC
                  if(conf_info.conf.output_flag(i_ctc).ival() &&
                     cts_info[m].cts.n() > 0) {

                     write_ctc_row(shc, cts_info[m],
                        conf_info.conf.output_flag(i_ctc).ival(),
                        stat_at, i_stat_row,
                        txt_at[i_ctc], i_txt_row[i_ctc]);
                  }

                  // Write out CTS
                  if(conf_info.conf.output_flag(i_cts).ival() &&
                     cts_info[m].cts.n() > 0) {

                     write_cts_row(shc, cts_info[m],
                        conf_info.conf.output_flag(i_cts).ival(),
                        stat_at, i_stat_row,
                        txt_at[i_cts], i_txt_row[i_cts]);
                  }
               } // end for m
            } // end Compute CTS

            // Compute MCTS scores
            if(conf_info.fcst_gci[i].pflag == 0 &&
               conf_info.fcst_ta[i].n_elements() > 1 &&
               (conf_info.conf.output_flag(i_mctc).ival() ||
                conf_info.conf.output_flag(i_mcts).ival())) {

               // Initialize
               mcts_info.clear();

               // Compute MCTS
               do_mcts(mcts_info, i, f_na, o_na);

               // Write out MCTC
               if(conf_info.conf.output_flag(i_mctc).ival() &&
                  mcts_info.cts.total() > 0) {

                  write_mctc_row(shc, mcts_info,
                     conf_info.conf.output_flag(i_mctc).ival(),
                     stat_at, i_stat_row,
                     txt_at[i_mctc], i_txt_row[i_mctc]);
               }

               // Write out MCTS
               if(conf_info.conf.output_flag(i_mcts).ival() &&
                  mcts_info.cts.total() > 0) {

                  write_mcts_row(shc, mcts_info,
                     conf_info.conf.output_flag(i_mcts).ival(),
                     stat_at, i_stat_row,
                     txt_at[i_mcts], i_txt_row[i_mcts]);
               }
            } // end Compute MCTS

            // Compute CNT scores
            if(conf_info.fcst_gci[i].pflag == 0 &&
               (conf_info.conf.output_flag(i_cnt).ival() ||
                conf_info.conf.output_flag(i_sl1l2).ival())) {

               // Initialize
               cnt_info.clear();

               // Compute CNT
               do_cnt(cnt_info, i, f_na, o_na);

               // Write out CNT
               if(conf_info.conf.output_flag(i_cnt).ival() &&
                  cnt_info.n > 0) {

                  write_cnt_row(shc, cnt_info,
                     conf_info.conf.output_flag(i_cnt).ival(),
                     stat_at, i_stat_row,
                     txt_at[i_cnt], i_txt_row[i_cnt]);
               }

               // Write out SL1L2 as long as the vflag is not set
               if(conf_info.fcst_gci[i].vflag == 0 &&
                  conf_info.conf.output_flag(i_sl1l2).ival() &&
                  cnt_info.n > 0) {

                  write_sl1l2_row(shc, cnt_info,
                     conf_info.conf.output_flag(i_sl1l2).ival(),
                     stat_at, i_stat_row,
                     txt_at[i_sl1l2], i_txt_row[i_sl1l2]);
               }
            } // end Compute CNT

            // Compute VL1L2 partial sums for UGRD,VGRD
            if(conf_info.fcst_gci[i].pflag == 0 &&
               conf_info.fcst_gci[i].vflag == 1 &&
               conf_info.conf.output_flag(i_vl1l2).ival() &&
               i > 0 &&
               conf_info.fcst_gci[i].code   == vgrd_grib_code &&
               conf_info.fcst_gci[i-1].code == ugrd_grib_code) {

               // Store the forecast variable name
               shc.set_fcst_var(ugrd_vgrd_abbr_str);

               // Store the observation variable name
               shc.set_obs_var(ugrd_vgrd_abbr_str);

               // Initialize
               for(m=0; m<wind_t; m++) vl1l2_info[m].clear();

               // Apply current smoothing method to the UGRD fields
               fcst_ugrd_wd_smooth =
                  smooth_field(fcst_ugrd_wd,
                               conf_info.interp_mthd[j],
                               conf_info.interp_wdth[j],
                               conf_info.conf.interp_thresh().dval());

               obs_ugrd_wd_smooth  =
                  smooth_field(obs_ugrd_wd,
                               conf_info.interp_mthd[j],
                               conf_info.interp_wdth[j],
                               conf_info.conf.interp_thresh().dval());

               // Apply the current mask to the UGRD fields
               apply_mask(fcst_ugrd_wd_smooth, obs_ugrd_wd_smooth,
                          conf_info.mask_wd[k],
                          f_ugrd_na, o_ugrd_na);

               // Compute VL1L2
               do_vl1l2(vl1l2_info, i,
                        f_na, o_na, f_ugrd_na, o_ugrd_na);

               // Loop through all of the wind speed thresholds
               for(m=0; m<conf_info.fcst_wind_ta.n_elements(); m++) {

                  // Write out VL1L2
                  if(conf_info.conf.output_flag(i_vl1l2).ival() &&
                     vl1l2_info[m].vcount > 0) {

                     write_vl1l2_row(shc, vl1l2_info[m],
                        conf_info.conf.output_flag(i_vl1l2).ival(),
                        stat_at, i_stat_row,
                        txt_at[i_vl1l2], i_txt_row[i_vl1l2]);
                  }
               } // end for m

               // Reset the forecast variable name
               shc.set_fcst_var(conf_info.fcst_gci[i].abbr_str.text());

               // Reset the observation variable name
               shc.set_obs_var(conf_info.obs_gci[i].abbr_str.text());

            } // end Compute VL1L2

            // Compute PCT counts and scores
            if(conf_info.fcst_gci[i].pflag == 1 &&
               (conf_info.conf.output_flag(i_pct).ival()  ||
                conf_info.conf.output_flag(i_pstd).ival() ||
                conf_info.conf.output_flag(i_pjc).ival()  ||
                conf_info.conf.output_flag(i_prc).ival())) {

               // Initialize
               for(m=0; m<max_prob_t; m++) pct_info[m].clear();

               // Compute PCT
               do_pct(pct_info, i, f_na, o_na);

               // Loop through all of the thresholds
               for(m=0; m<max_prob_t; m++) {

                  // Write out PCT
                  if(conf_info.conf.output_flag(i_pct).ival() &&
                     pct_info[m].pct.n() > 0) {

                     write_pct_row(shc, pct_info[m],
                        conf_info.conf.output_flag(i_pct).ival(),
                        stat_at, i_stat_row,
                        txt_at[i_pct], i_txt_row[i_pct]);
                  }

                  // Write out PSTD
                  if(conf_info.conf.output_flag(i_pstd).ival() &&
                     pct_info[m].pct.n() > 0) {

                     write_pstd_row(shc, pct_info[m],
                        conf_info.conf.output_flag(i_pstd).ival(),
                        stat_at, i_stat_row,
                        txt_at[i_pstd], i_txt_row[i_pstd]);
                  }

                  // Write out PJC
                  if(conf_info.conf.output_flag(i_pjc).ival() &&
                     pct_info[m].pct.n() > 0) {

                     write_pjc_row(shc, pct_info[m],
                        conf_info.conf.output_flag(i_pjc).ival(),
                        stat_at, i_stat_row,
                        txt_at[i_pjc], i_txt_row[i_pjc]);
                  }

                  // Write out PRC
                  if(conf_info.conf.output_flag(i_prc).ival() &&
                     pct_info[m].pct.n() > 0) {

                     write_prc_row(shc, pct_info[m],
                        conf_info.conf.output_flag(i_prc).ival(),
                        stat_at, i_stat_row,
                        txt_at[i_prc], i_txt_row[i_prc]);
                  }
               }
            } // end Compute PCT

         } // end for k

         // Write out the smoothed forecast, observation, and difference
         // fields in netCDF format for each GRIB code if requested in
         // the config file
         if(conf_info.conf.output_flag(i_nc).ival())
            write_nc(fcst_wd_smooth, obs_wd_smooth, i,
                     conf_info.interp_mthd[j],
                     conf_info.interp_wdth[j]);

      } // end for j

      // Loop through and apply the Neighborhood methods for each of the
      // neighborhood widths if requested in the config file
      if(conf_info.fcst_gci[i].pflag == 0 &&
         (conf_info.conf.output_flag(i_nbrctc).ival() ||
          conf_info.conf.output_flag(i_nbrcts).ival() ||
          conf_info.conf.output_flag(i_nbrcnt).ival())) {

         // Initialize
         for(j=0; j<frac_t; j++) nbrcts_info[j].clear();

         // Loop through and apply each of the neighborhood widths
         for(j=0; j<conf_info.get_n_nbr_wdth(); j++) {

            // Store the interpolation method and width being applied
            shc.set_interp_mthd(im_nbrhd);
            shc.set_interp_wdth(conf_info.conf.nbr_width(j).ival());

            // Loop through and apply each of the raw threshold values
            for(k=0; k<conf_info.fcst_ta[i].n_elements(); k++) {

               // Compute the fractional coverage based on the
               // threshold
               fcst_wd_smooth = fractional_coverage(fcst_wd,
                                   conf_info.conf.nbr_width(j).ival(),
                                   conf_info.fcst_ta[i][k],
                                   conf_info.conf.nbr_thresh().dval());

               obs_wd_smooth  = fractional_coverage(obs_wd,
                                   conf_info.conf.nbr_width(j).ival(),
                                   conf_info.obs_ta[i][k],
                                   conf_info.conf.nbr_thresh().dval());

               // Loop through the masks to be applied
               for(m=0; m<conf_info.get_n_mask(); m++) {

                  // Apply the current mask to the fields
                  apply_mask(fcst_wd_smooth, obs_wd_smooth,
                             conf_info.mask_wd[m],
                             f_na, o_na);

                  // Continue if no pairs were found
                  if(f_na.n_elements() == 0) continue;

                  // Set the mask name
                  shc.set_mask(conf_info.mask_name[m]);

                  if(verbosity > 1) {

                     conf_info.fcst_ta[i][k].get_str(
                        fcst_thresh_str);
                     conf_info.obs_ta[i][k].get_str(
                        obs_thresh_str);

                     cout << "Processing "
                          << conf_info.fcst_gci[i].info_str
                          << " versus "
                          << conf_info.obs_gci[i].info_str
                          << ", for interpolation method "
                          << shc.get_interp_mthd_str() << "("
                          << shc.get_interp_pnts_str()
                          << "), raw thresholds of "
                          << fcst_thresh_str << " and "
                          << obs_thresh_str
                          << ", over region " << shc.get_mask()
                          << ", using " << f_na.n_elements()
                          << " pairs.\n" << flush;
                  }

                  // Compute NBRCTS scores
                  if(conf_info.conf.output_flag(i_nbrctc).ival() ||
                     conf_info.conf.output_flag(i_nbrcts).ival()) {

                     // Initialize
                     for(n=0; n<conf_info.frac_ta.n_elements(); n++) {
                        nbrcts_info[n].clear();
                     }

                     do_nbrcts(nbrcts_info, i, j, k, f_na, o_na);

                     // Loop through all of the thresholds
                     for(n=0; n<conf_info.frac_ta.n_elements(); n++) {

                        // Only write out if n > 0
                        if(nbrcts_info[n].cts_info.cts.n() > 0) {

                           // Write out NBRCTC
                           if(conf_info.conf.output_flag(i_nbrctc).ival()) {

                              write_nbrctc_row(shc, nbrcts_info[n],
                                 conf_info.conf.output_flag(i_nbrctc).ival(),
                                 stat_at, i_stat_row,
                                 txt_at[i_nbrctc], i_txt_row[i_nbrctc]);
                           }

                           // Write out NBRCTS
                           if(conf_info.conf.output_flag(i_nbrcts).ival()) {

                              write_nbrcts_row(shc, nbrcts_info[n],
                                 conf_info.conf.output_flag(i_nbrcts).ival(),
                                 stat_at, i_stat_row,
                                 txt_at[i_nbrcts], i_txt_row[i_nbrcts]);
                           }
                        } // end if
                     } // end for n
                  } // end compute NBRCTS

                  // Compute NBRCNT scores
                  if(conf_info.conf.output_flag(i_nbrcnt).ival()) {

                     // Initialize
                     nbrcnt_info.clear();

                     do_nbrcnt(nbrcnt_info, i, j, k, f_na, o_na);

                     // Write out NBRCNT
                     if(conf_info.conf.output_flag(i_nbrcnt).ival() &&
                        nbrcnt_info.cnt_info.n > 0) {

                        write_nbrcnt_row(shc, nbrcnt_info,
                           conf_info.conf.output_flag(i_nbrcnt).ival(),
                           stat_at, i_stat_row,
                           txt_at[i_nbrcnt], i_txt_row[i_nbrcnt]);
                     }
                  } // end compute NBRCNT
               } // end for m
            } // end for k
         } // end for j
      } // end if
   } // end for i

   if(verbosity > 1) {
      cout << "\n" << sep_str << "\n\n" << flush;
   }

   // Deallocate memory
   if(cts_info)    { delete [] cts_info;    cts_info    = (CTSInfo *)    0; }
   if(vl1l2_info)  { delete [] vl1l2_info;  vl1l2_info  = (VL1L2Info *)  0; }
   if(nbrcts_info) { delete [] nbrcts_info; nbrcts_info = (NBRCTSInfo *) 0; }
   if(pct_info)    { delete [] pct_info;    pct_info    = (PCTInfo *)    0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_first_pass(const WrfData &wd, const Grid &fcst_grid) {

   // Store the grid to be used through the verification
   grid = fcst_grid;

   // Process masks Grids and Polylines in the config file
   conf_info.process_masks(grid);

   // Create output text files as requested in the config file
   setup_txt_files(wd.get_valid_time(), wd.get_lead_time());

   // If requested, create a NetCDF file to store the matched pairs and
   // difference fields for each GRIB code and masking region
   if(conf_info.conf.output_flag(i_nc).ival() != flag_no_out) {
      setup_nc_file(wd.get_valid_time(), wd.get_lead_time());
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_txt_files(unixtime valid_ut, int lead_sec) {
   int  i, max_col, max_prob_col, max_mctc_col, n_prob, n_cat;
   ConcatString tmp_str;

   // Create output file names for the stat file and optional text files
   build_outfile_name(valid_ut, lead_sec, "", tmp_str);

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

   // Maximum number of standard STAT columns
   max_col = max_stat_col + n_header_columns + 1;

   // Maximum number of probabilistic columns
   if(max_prob_col > max_col)
      max_col = max_prob_col + n_header_columns + 1;

   // Maximum number of multi-category contingency table columns
   if(max_mctc_col > max_stat_col)
      max_col = max_mctc_col + n_header_columns + 1;

   // Initialize file stream
   stat_out   = (ofstream *) 0;

   // Build the file name
   stat_file << tmp_str << stat_file_ext;

   // Create the output STAT file
   open_txt_file(stat_out, stat_file, verbosity);

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
      if(conf_info.conf.output_flag(i).ival() >= flag_txt_out) {

         // Initialize file stream
         txt_out[i] = (ofstream *) 0;

         // Build the file name
         txt_file[i] << tmp_str << "_" << txt_file_abbr[i]
                     << txt_file_ext;

         // Create the output text file
         open_txt_file(txt_out[i], txt_file[i], verbosity);

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
   int mon, day, yr, hr, min, sec;
   char attribute_str[PATH_MAX], time_str[max_str_len];
   char hostname_str[max_str_len];
   unixtime ut;

   // Create output NetCDF file name
   build_outfile_name(valid_ut, lead_sec, "_pairs.nc", out_nc_file);

   // Create a new NetCDF file and open it
   nc_out = new NcFile(out_nc_file, NcFile::Replace);

   if(!nc_out->is_valid()) {
      cerr << "\n\nERROR: setup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_nc_file << "\n\n" << flush;
      exit(1);
   }

   ut = time(NULL);
   unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
   sprintf(time_str, "%.4i-%.2i-%.2i %.2i:%.2i:%.2i",
           yr, mon, day, hr, min, sec);

   gethostname(hostname_str, max_str_len);

   sprintf(attribute_str, "File %s generated %s UTC on host %s",
           out_nc_file.text(), time_str, hostname_str);
   nc_out->add_att("FileOrigins", attribute_str);
   nc_out->add_att("MET_version", met_version);
   nc_out->add_att("Difference", "Forecast Value - Observation Value");

   // Define Dimensions
   lat_dim = nc_out->add_dim("lat", (long) grid.ny());
   lon_dim = nc_out->add_dim("lon", (long) grid.nx());

   // Add the projection information
   write_netcdf_proj(nc_out, grid);

   // Add the lat/lon variables
   write_netcdf_latlon(nc_out, lat_dim, lon_dim, grid);

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

void do_cts(CTSInfo *&cts_info, int i_gc,
            const NumArray &f_na, const NumArray &o_na) {
   int i, j, n_cts;

   //
   // Set up the CTSInfo thresholds and alpha values
   //
   n_cts = conf_info.fcst_ta[i_gc].n_elements();
   for(i=0; i<n_cts; i++) {
      cts_info[i].cts_fcst_thresh = conf_info.fcst_ta[i_gc][i];
      cts_info[i].cts_obs_thresh  = conf_info.obs_ta[i_gc][i];
      cts_info[i].allocate_n_alpha(conf_info.get_n_ci_alpha());

      for(j=0; j<conf_info.get_n_ci_alpha(); j++) {
         cts_info[i].alpha[j] = conf_info.conf.ci_alpha(j).dval();
      }
   }

   if(verbosity > 1) {
      cout << "Computing Categorical Statistics.\n" << flush;
   }

   //
   // Compute the counts, stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(conf_info.conf.boot_interval().ival() == boot_bca_flag) {
      compute_cts_stats_ci_bca(rng_ptr, f_na, o_na,
         conf_info.conf.n_boot_rep().ival(),
         cts_info, n_cts,
         conf_info.conf.output_flag(i_cts).ival(),
         conf_info.conf.rank_corr_flag().ival(),
         conf_info.conf.tmp_dir().sval());
   }
   else {
      compute_cts_stats_ci_perc(rng_ptr, f_na, o_na,
         conf_info.conf.n_boot_rep().ival(),
         conf_info.conf.boot_rep_prop().dval(),
         cts_info, n_cts,
         conf_info.conf.output_flag(i_cts).ival(),
         conf_info.conf.rank_corr_flag().ival(),
         conf_info.conf.tmp_dir().sval());
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_mcts(MCTSInfo &mcts_info, int i_gc,
             const NumArray &f_na, const NumArray &o_na) {
   int i;

   //
   // Set up the MCTSInfo size, thresholds, and alpha values
   //
   mcts_info.cts.set_size(conf_info.fcst_ta[i_gc].n_elements() + 1);
   mcts_info.cts_fcst_ta = conf_info.fcst_ta[i_gc];
   mcts_info.cts_obs_ta  = conf_info.obs_ta[i_gc];
   mcts_info.allocate_n_alpha(conf_info.get_n_ci_alpha());

   for(i=0; i<conf_info.get_n_ci_alpha(); i++) {
      mcts_info.alpha[i] = conf_info.conf.ci_alpha(i).dval();
   }

   if(verbosity > 1) {
      cout << "Computing Multi-Category Statistics.\n" << flush;
   }

   //
   // If there are no matched pairs to process, return
   //
   if(f_na.n_elements() == 0 || o_na.n_elements() == 0) return;

   //
   // Compute the stats, normal confidence intervals, and bootstrap
   // bootstrap confidence intervals
   //
   if(conf_info.conf.boot_interval().ival() == boot_bca_flag) {
      compute_mcts_stats_ci_bca(rng_ptr, f_na, o_na,
         conf_info.conf.n_boot_rep().ival(),
         mcts_info,
         conf_info.conf.output_flag(i_mcts).ival(),
         conf_info.conf.rank_corr_flag().ival(),
         conf_info.conf.tmp_dir().sval());
   }
   else {
      compute_mcts_stats_ci_perc(rng_ptr, f_na, o_na,
         conf_info.conf.n_boot_rep().ival(),
         conf_info.conf.boot_rep_prop().dval(),
         mcts_info,
         conf_info.conf.output_flag(i_mcts).ival(),
         conf_info.conf.rank_corr_flag().ival(),
         conf_info.conf.tmp_dir().sval());
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_cnt(CNTInfo &cnt_info, int i_gc,
            const NumArray &f_na, const NumArray &o_na) {
   int i;

   //
   // Set up the CNTInfo alpha values
   //
   cnt_info.allocate_n_alpha(conf_info.get_n_ci_alpha());
   for(i=0; i<conf_info.get_n_ci_alpha(); i++) {
      cnt_info.alpha[i] = conf_info.conf.ci_alpha(i).dval();
   }

   if(verbosity > 1) {
      cout << "Computing Continuous Statistics.\n" << flush;
   }

   //
   // Compute the stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(conf_info.conf.boot_interval().ival() == boot_bca_flag) {
      compute_cnt_stats_ci_bca(rng_ptr, f_na, o_na,
         conf_info.fcst_gci[i_gc].code,
         conf_info.obs_gci[i_gc].code,
         conf_info.conf.n_boot_rep().ival(),
         cnt_info,
         conf_info.conf.output_flag(i_cnt).ival(),
         conf_info.conf.rank_corr_flag().ival(),
         conf_info.conf.tmp_dir().sval());
   }
   else {
      compute_cnt_stats_ci_perc(rng_ptr, f_na, o_na,
         conf_info.fcst_gci[i_gc].code,
         conf_info.obs_gci[i_gc].code,
         conf_info.conf.n_boot_rep().ival(),
         conf_info.conf.boot_rep_prop().dval(),
         cnt_info,
         conf_info.conf.output_flag(i_cnt).ival(),
         conf_info.conf.rank_corr_flag().ival(),
         conf_info.conf.tmp_dir().sval());
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_vl1l2(VL1L2Info *&v_info, int i_gc,
              const NumArray &vf_na, const NumArray &vo_na,
              const NumArray &uf_na, const NumArray &uo_na) {
   int i, j, n_thresh;
   double uf, vf, uo, vo, fwind, owind;
   SingleThresh fst, ost;
   char fcst_str[max_str_len], obs_str[max_str_len];

   // Check that the number of pairs are the same
   if(vf_na.n_elements() != vo_na.n_elements() ||
      uf_na.n_elements() != uo_na.n_elements() ||
      vf_na.n_elements() != uf_na.n_elements()) {

      cerr << "\n\nERROR: do_vl1l2() -> "
           << "the number of UGRD pairs != the number of VGRD pairs: "
           << vf_na.n_elements() << " != " << uf_na.n_elements()
           << "\n\n" << flush;
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
   for(i=0; i<vf_na.n_elements(); i++) {

      // Retrieve the U,V values
      uf = uf_na[i];
      vf = vf_na[i];
      uo = uo_na[i];
      vo = vo_na[i];

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

         // Add this pair to the VL1L2 counts

         // VL1L2 sums
         v_info[j].vcount  += 1;
         v_info[j].ufbar   += uf;
         v_info[j].vfbar   += vf;
         v_info[j].uobar   += uo;
         v_info[j].vobar   += vo;
         v_info[j].uvfobar += uf*uo+vf*vo;
         v_info[j].uvffbar += uf*uf+vf*vf;
         v_info[j].uvoobar += uo*uo+vo*vo;
      } // end for j
   } // end for i

   // Compute means for the VL1L2Info objects
   for(i=0; i<n_thresh; i++) {

      if(verbosity > 1) {

         // Retrieve the threshold abbreviations
         v_info[i].wind_fcst_thresh.get_str(fcst_str);
         v_info[i].wind_obs_thresh.get_str(obs_str);

         cout << "Computing Vector Partial Sums, "
              << "for forecast wind speed " << fcst_str
              << ", for observation wind speed " << obs_str
              << ", using " << v_info[i].vcount << " pairs.\n"
              << flush;
      }

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
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_pct(PCTInfo *&pct_info, int i_gc,
            const NumArray &f_na, const NumArray &o_na) {
   int i, j, n_pct;

   if(verbosity > 1) {
      cout << "Computing Probabilistic Statistics.\n" << flush;
   }

   //
   // If there are no matched pairs to process, return
   //
   if(f_na.n_elements() == 0 || o_na.n_elements() == 0) return;

   //
   // Set up the PCTInfo thresholds and alpha values
   //
   n_pct = conf_info.obs_ta[i_gc].n_elements();
   for(i=0; i<n_pct; i++) {

      // Use all of the selected forecast thresholds
      pct_info[i].pct_fcst_thresh = conf_info.fcst_ta[i_gc];

      // Process the observation thresholds one at a time
      pct_info[i].pct_obs_thresh  = conf_info.obs_ta[i_gc][i];

      pct_info[i].allocate_n_alpha(conf_info.get_n_ci_alpha());

      for(j=0; j<conf_info.get_n_ci_alpha(); j++) {
         pct_info[i].alpha[j] = conf_info.conf.ci_alpha(j).dval();
      }

      //
      // Compute the probabilistic counts and statistics
      //
      compute_pctinfo(f_na, o_na,
            conf_info.conf.output_flag(i_pstd).ival(),
            pct_info[i]);
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_nbrcts(NBRCTSInfo *&nbrcts_info,
               int i_gc, int i_wdth, int i_thresh,
               const NumArray &f_na, const NumArray &o_na) {
   int i, j, n_nbrcts;

   //
   // Set up the NBRCTSInfo thresholds and alpha values
   //
   n_nbrcts = conf_info.frac_ta.n_elements();
   for(i=0; i<n_nbrcts; i++) {

      nbrcts_info[i].raw_fcst_thresh =
         conf_info.fcst_ta[i_gc][i_thresh];
      nbrcts_info[i].raw_obs_thresh =
         conf_info.obs_ta[i_gc][i_thresh];
      nbrcts_info[i].frac_thresh =
         conf_info.frac_ta[i];

      nbrcts_info[i].cts_info.cts_fcst_thresh =
         conf_info.frac_ta[i];
      nbrcts_info[i].cts_info.cts_obs_thresh =
         conf_info.frac_ta[i];
      nbrcts_info[i].allocate_n_alpha(conf_info.get_n_ci_alpha());

      for(j=0; j<conf_info.get_n_ci_alpha(); j++) {
         nbrcts_info[i].cts_info.alpha[j] =
            conf_info.conf.ci_alpha(j).dval();
      }
   }

   //
   // Keep track of the neighborhood width
   //
   for(i=0; i<n_nbrcts; i++) {
      nbrcts_info[i].nbr_wdth =
         conf_info.conf.nbr_width(i_wdth).ival();
   }

   if(verbosity > 1) {
      cout << "Computing Neighborhood Categorical Statistics.\n"
           << flush;
   }

   //
   // Compute the stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(conf_info.conf.boot_interval().ival() == boot_bca_flag) {
      compute_nbrcts_stats_ci_bca(rng_ptr, f_na, o_na,
         conf_info.conf.n_boot_rep().ival(),
         nbrcts_info, n_nbrcts,
         conf_info.conf.output_flag(i_nbrcts).ival(),
         conf_info.conf.tmp_dir().sval());
   }
   else {
      compute_nbrcts_stats_ci_perc(rng_ptr, f_na, o_na,
         conf_info.conf.n_boot_rep().ival(),
         conf_info.conf.boot_rep_prop().dval(),
         nbrcts_info, n_nbrcts,
         conf_info.conf.output_flag(i_nbrcts).ival(),
         conf_info.conf.tmp_dir().sval());
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_nbrcnt(NBRCNTInfo &nbrcnt_info,
               int i_gc, int i_wdth, int i_thresh,
               const NumArray &f_na, const NumArray &o_na) {
   int i;

   //
   // Set up the NBRCNTInfo threshold and alpha values
   //
   nbrcnt_info.raw_fcst_thresh =
      conf_info.fcst_ta[i_gc][i_thresh];
   nbrcnt_info.raw_obs_thresh =
      conf_info.obs_ta[i_gc][i_thresh];

   nbrcnt_info.allocate_n_alpha(conf_info.get_n_ci_alpha());
   for(i=0; i<conf_info.get_n_ci_alpha(); i++) {
      nbrcnt_info.cnt_info.alpha[i] = conf_info.conf.ci_alpha(i).dval();
   }

   //
   // Keep track of the neighborhood width
   //
   nbrcnt_info.nbr_wdth =
      conf_info.conf.nbr_width(i_wdth).ival();

   if(verbosity > 1) {
      cout << "Computing Neighborhood Continuous Statistics.\n"
           << flush;
   }

   //
   // Compute the stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(conf_info.conf.boot_interval().ival() == boot_bca_flag) {
      compute_nbrcnt_stats_ci_bca(rng_ptr, f_na, o_na,
         conf_info.conf.n_boot_rep().ival(),
         nbrcnt_info,
         conf_info.conf.output_flag(i_nbrcnt).ival(),
         conf_info.conf.tmp_dir().sval());
   }
   else {
      compute_nbrcnt_stats_ci_perc(rng_ptr, f_na, o_na,
         conf_info.conf.n_boot_rep().ival(),
         conf_info.conf.boot_rep_prop().dval(),
         nbrcnt_info,
         conf_info.conf.output_flag(i_nbrcnt).ival(),
         conf_info.conf.tmp_dir().sval());
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nc(const WrfData &fcst_wd, const WrfData &obs_wd,
              int i_gc, InterpMthd mthd, int wdth) {
   int i, n, x, y, mon, day, yr, hr, min, sec;
   int fcst_flag, obs_flag, diff_flag;
   float *fcst_data = (float *) 0;
   float *obs_data  = (float *) 0;
   float *diff_data = (float *) 0;
   double fval, oval;
   unixtime ut;
   NcVar *fcst_var, *obs_var, *diff_var;
   char fcst_var_name[max_str_len], obs_var_name[max_str_len];
   char mthd_str[max_str_len];
   char diff_var_name[max_str_len];
   char time_str[max_str_len];
   char tmp_str[max_str_len];

   // Get the interpolation method string
   interpmthd_to_string(mthd, mthd_str);

   // Allocate memory for the forecast, observation, and difference
   // fields
   fcst_data = new float [grid.nx()*grid.ny()];
   obs_data  = new float [grid.nx()*grid.ny()];
   diff_data = new float [grid.nx()*grid.ny()];

   // Compute the difference field for each of the masking regions
   for(i=0; i<conf_info.get_n_mask(); i++) {

      // If the neighborhood width is greater than 1 and the forecast
      // field was smoothed
      if((wdth > 1) &&
         (conf_info.conf.interp_flag().ival() == 1 ||
          conf_info.conf.interp_flag().ival() == 3)) {
         sprintf(fcst_var_name, "FCST_%s_%s_%s_%s_%i",
                 conf_info.fcst_gci[i_gc].abbr_str.text(),
                 conf_info.fcst_gci[i_gc].lvl_str.text(),
                 conf_info.mask_name[i],
                 mthd_str, wdth*wdth);
      }
      // Forecast field was not smoothed
      else {
         sprintf(fcst_var_name, "FCST_%s_%s_%s",
                 conf_info.fcst_gci[i_gc].abbr_str.text(),
                 conf_info.fcst_gci[i_gc].lvl_str.text(),
                 conf_info.mask_name[i]);
      }

      // If the neighborhood width is greater than 1 and the observation
      // field was smoothed
      if((wdth > 1) &&
         (conf_info.conf.interp_flag().ival() == 2 ||
          conf_info.conf.interp_flag().ival() == 3)) {
         sprintf(obs_var_name, "OBS_%s_%s_%s_%s_%i",
                 conf_info.obs_gci[i_gc].abbr_str.text(),
                 conf_info.obs_gci[i_gc].lvl_str.text(),
                 conf_info.mask_name[i],
                 mthd_str, wdth*wdth);
      }
      // Observation field was not smoothed
      else {
         sprintf(obs_var_name, "OBS_%s_%s_%s",
                 conf_info.obs_gci[i_gc].abbr_str.text(),
                 conf_info.obs_gci[i_gc].lvl_str.text(),
                 conf_info.mask_name[i]);
      }

      // If any smoothing was performed
      if(wdth > 1) {
         sprintf(diff_var_name, "DIFF_%s_%s_%s_%s_%s_%s_%i",
                 conf_info.fcst_gci[i_gc].abbr_str.text(),
                 conf_info.fcst_gci[i_gc].lvl_str.text(),
                 conf_info.obs_gci[i_gc].abbr_str.text(),
                 conf_info.obs_gci[i_gc].lvl_str.text(),
                 conf_info.mask_name[i],
                 mthd_str, wdth*wdth);
      }
      // Smoothing was not performed
      else {
         sprintf(diff_var_name, "DIFF_%s_%s_%s_%s_%s",
                 conf_info.fcst_gci[i_gc].abbr_str.text(),
                 conf_info.fcst_gci[i_gc].lvl_str.text(),
                 conf_info.obs_gci[i_gc].abbr_str.text(),
                 conf_info.obs_gci[i_gc].lvl_str.text(),
                 conf_info.mask_name[i]);
      }

      // Figure out if the forecast, observation, and difference
      // fields should be written out.
      fcst_flag = !fcst_var_sa.has(fcst_var_name);
      obs_flag  = !obs_var_sa.has(obs_var_name);

      // Don't write the difference field for probability forecasts
      diff_flag = (!diff_var_sa.has(diff_var_name) &&
                   !conf_info.fcst_gci[i_gc].pflag);

      // Set up the forecast variable if not already defined
      if(fcst_flag) {

         // Define the forecast variable
         fcst_var = nc_out->add_var(fcst_var_name, ncFloat,
                                    lat_dim, lon_dim);

         // Add to the list of previously defined variables
         fcst_var_sa.add(fcst_var_name);

         // Add variable attributes for the forecast field
         fcst_var->add_att("name", shc.get_fcst_var());
         get_grib_code_unit(conf_info.fcst_gci[i_gc].code,
                            conf_info.conf.grib_ptv().ival(),
                            tmp_str);
         fcst_var->add_att("units", tmp_str);
         get_grib_code_name(conf_info.fcst_gci[i_gc].code,
                            conf_info.conf.grib_ptv().ival(),
                            tmp_str);
         fcst_var->add_att("long_name", tmp_str);
         fcst_var->add_att("i_level", shc.get_fcst_lev());

         fcst_var->add_att("_FillValue", bad_data_float);

         ut = fcst_wd.get_valid_time();
         unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
         sprintf(time_str, "%.4i-%.2i-%.2i %.2i:%.2i:%.2i",
                 yr, mon, day, hr, min, sec);
         fcst_var->add_att("valid_time", time_str);
         fcst_var->add_att("valid_time_ut", (long int) ut);

         fcst_var->add_att("masking_region", conf_info.mask_name[i]);
         fcst_var->add_att("smoothing_method", mthd_str);
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
         obs_var->add_att("name", shc.get_obs_var());
         get_grib_code_unit(conf_info.obs_gci[i_gc].code,
                            conf_info.conf.grib_ptv().ival(),
                            tmp_str);
         obs_var->add_att("units", tmp_str);
         get_grib_code_name(conf_info.obs_gci[i_gc].code,
                            conf_info.conf.grib_ptv().ival(),
                            tmp_str);
         obs_var->add_att("long_name", tmp_str);
         obs_var->add_att("i_level", shc.get_obs_lev());

         obs_var->add_att("_FillValue", bad_data_float);

         ut = obs_wd.get_valid_time();
         unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
         sprintf(time_str, "%.4i-%.2i-%.2i %.2i:%.2i:%.2i",
                 yr, mon, day, hr, min, sec);
         obs_var->add_att("valid_time", time_str);
         obs_var->add_att("valid_time_ut", (long int) ut);

         obs_var->add_att("masking_region", conf_info.mask_name[i]);
         obs_var->add_att("smoothing_method", mthd_str);
         obs_var->add_att("smoothing_neighborhood", wdth*wdth);
      } // end obs_flag

      // Set up the observation variable if not already defined
      if(diff_flag) {

         // Define the difference variable
         diff_var = nc_out->add_var(diff_var_name, ncFloat,
                                    lat_dim, lon_dim);

         // Add to the list of previously defined variables
         diff_var_sa.add(diff_var_name);

         // Add variable attributes for the difference field
         sprintf(tmp_str, "%s_minus_%s",
            shc.get_fcst_var(), shc.get_obs_var());
         diff_var->add_att("name", tmp_str);
         get_grib_code_unit(conf_info.fcst_gci[i_gc].code,
                            conf_info.conf.grib_ptv().ival(),
                            tmp_str);
         diff_var->add_att("fcst_units", tmp_str);
         get_grib_code_name(conf_info.fcst_gci[i_gc].code,
                            conf_info.conf.grib_ptv().ival(),
                            tmp_str);
         diff_var->add_att("fcst_long_name", tmp_str);
         diff_var->add_att("fcst_i_level", shc.get_fcst_lev());
         get_grib_code_unit(conf_info.obs_gci[i_gc].code,
                            conf_info.conf.grib_ptv().ival(),
                            tmp_str);
         diff_var->add_att("obs_units", tmp_str);
         get_grib_code_name(conf_info.obs_gci[i_gc].code,
                            conf_info.conf.grib_ptv().ival(),
                            tmp_str);
         diff_var->add_att("obs_long_name", tmp_str);
         diff_var->add_att("obs_i_level", shc.get_obs_lev());

         diff_var->add_att("_FillValue", bad_data_float);

         ut = fcst_wd.get_valid_time();
         unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
         sprintf(time_str, "%.4i-%.2i-%.2i %.2i:%.2i:%.2i",
                 yr, mon, day, hr, min, sec);
         diff_var->add_att("fcst_valid_time", time_str);
         diff_var->add_att("fcst_valid_time_ut", (long int) ut);

         ut = obs_wd.get_valid_time();
         unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
         sprintf(time_str, "%.4i-%.2i-%.2i %.2i:%.2i:%.2i",
                 yr, mon, day, hr, min, sec);
         diff_var->add_att("obs_valid_time", time_str);
         diff_var->add_att("obs_valid_time_ut", (long int) ut);

         diff_var->add_att("masking_region", conf_info.mask_name[i]);
         diff_var->add_att("smoothing_method", mthd_str);
         diff_var->add_att("smoothing_neighborhood", wdth*wdth);
      } // end diff_flag

      // Store the forecast, observation, and difference values
      for(x=0; x<grid.nx(); x++) {
         for(y=0; y<grid.ny(); y++) {

            n = two_to_one(grid.nx(), grid.ny(), x, y);

            // Check whether the mask is on or off for this point
            if(!conf_info.mask_wd[i].s_is_on(x, y)) {
               fcst_data[n] = bad_data_float;
               obs_data[n]  = bad_data_float;
               diff_data[n] = bad_data_float;
               continue;
            }

            // Retrieve the forecast and observation values
            fval = fcst_wd.get_xy_double(x, y);
            oval = obs_wd.get_xy_double(x, y);

            // Set the forecast data
            if(is_bad_data(fval)) fcst_data[n] = bad_data_float;
            else                  fcst_data[n] = fval;

            // Set the observation data
            if(is_bad_data(oval)) obs_data[n]  = bad_data_float;
            else                  obs_data[n]  = oval;

            // Set the difference data
            if(is_bad_data(fval) ||
               is_bad_data(oval)) diff_data[n] = bad_data_float;
            else                  diff_data[n] = fval - oval;
         } // end for y
      } // end for x

      // Write out the forecast field
      if(fcst_flag) {
         if(!fcst_var->put(&fcst_data[0], grid.ny(), grid.nx())) {
            cerr << "\n\nERROR: write_nc() -> "
                 << "error with the fcst_var->put for fields "
                 << shc.get_fcst_var() << " and " << shc.get_obs_var()
                 << " and masking region " << conf_info.mask_name[i]
                 << "\n\n" << flush;
            exit(1);
         }
      }

      // Write out the observation field
      if(obs_flag) {
         if(!obs_var->put(&obs_data[0], grid.ny(), grid.nx())) {
            cerr << "\n\nERROR: write_nc() -> "
                 << "error with the obs_var->put for fields "
                 << shc.get_fcst_var() << " and " << shc.get_obs_var()
                 << " and masking region " << conf_info.mask_name[i]
                 << "\n\n" << flush;
            exit(1);
         }
      }

      // Write out the difference field
      if(diff_flag) {
         if(!diff_var->put(&diff_data[0], grid.ny(), grid.nx())) {
            cerr << "\n\nERROR: write_nc() -> "
                 << "error with the diff_var->put for fields "
                 << shc.get_fcst_var() << " and " << shc.get_obs_var()
                 << " and masking region " << conf_info.mask_name[i]
                 << "\n\n" << flush;
            exit(1);
         }
      }

   } // end for i

   // Deallocate and clean up
   if(fcst_data) { delete [] fcst_data; fcst_data = (float *) 0; }
   if(obs_data)  { delete [] obs_data;  obs_data  = (float *) 0; }
   if(diff_data) { delete [] diff_data; diff_data = (float *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void finish_txt_files() {
   int i;

   // Write out the contents of the STAT AsciiTable and
   // close the STAT output files
   if(stat_out) {
      *stat_out << stat_at;
      close_txt_file(stat_out, stat_file, verbosity);
   }

   // Finish up each of the optional text files
   for(i=0; i<n_txt; i++) {

      // Only write the table if requested in the config file
      if(conf_info.conf.output_flag(i).ival() >= flag_txt_out) {

         // Write the AsciiTable to a file
         if(txt_out[i]) {
            *txt_out[i] << txt_at[i];
            close_txt_file(txt_out[i], txt_file[i], verbosity);
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {

   // Close the output text files that were open for writing
   finish_txt_files();

   // Close the output NetCDF file as long as it was opened
   if(nc_out && conf_info.conf.output_flag(i_nc).ival()) {

      // List the NetCDF file after it is finished
      if(verbosity > 0) {
         cout << "Output file: "
              << out_nc_file << "\n" << flush;
      }
      nc_out->close();
      delete nc_out;
      nc_out = (NcFile *) 0;
   }

   // Close the forecast file
   if(fcst_ftype == NcFileType) {
      fcst_nc_file->close();
      delete fcst_nc_file;
      fcst_nc_file = (NcFile *) 0;
   }
   else if(fcst_ftype == GbFileType) {
      fcst_gb_file.close();
   }

   // Close the observation file
   if(obs_ftype == NcFileType) {
      obs_nc_file->close();
      delete obs_nc_file;
      obs_nc_file = (NcFile *) 0;
   }
   else if(obs_ftype == GbFileType) {
      obs_gb_file.close();
   }

   // Deallocate memory for the random number generator
   rng_free(rng_ptr);

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
        << "\t[-fcst_valid time]\n"
        << "\t[-fcst_lead time]\n"
        << "\t[-obs_valid time]\n"
        << "\t[-obs_lead time]\n"
        << "\t[-outdir path]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"fcst_file\" is a forecast file in either GRIB "
        << "or NetCDF format containing the field(s) to be verified "
        << "(required).\n"

        << "\t\t\"obs_file\" is an observation file in either GRIB "
        << "or NetCDF format containing the verifying field(s) "
        << "(required).\n"

        << "\t\t\"config_file\" is a GridStatConfig file containing "
        << "the desired configuration settings (required).\n"

        << "\t\t\"-fcst_valid time\" in YYYYMMDD[_HH[MMSS]] format "
        << "sets the forecast valid time to be verified (optional).\n"

        << "\t\t\"-fcst_lead time\" in HH[MMSS] format sets "
        << "the forecast lead time to be verified (optional).\n"

        << "\t\t\"-obs_valid time\" in YYYYMMDD[_HH[MMSS]] format "
        << "sets the observation valid time to be used (optional).\n"

        << "\t\t\"-obs_lead time\" in HH[MMSS] format sets "
        << "the observation lead time to be used (optional).\n"

        << "\t\t\"-outdir path\" overrides the default output directory "
        << "(" << out_dir << ") (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << verbosity << ") (optional).\n"

        << "\n\tNOTE: The forecast and observation fields must be "
        << "on the same grid.\n\n" << flush;

   return;
}

////////////////////////////////////////////////////////////////////////
