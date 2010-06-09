// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cmath>
#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "grid_stat_conf_info.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class GridStatConfInfo
//
////////////////////////////////////////////////////////////////////////

GridStatConfInfo::GridStatConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

GridStatConfInfo::~GridStatConfInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void GridStatConfInfo::init_from_scratch() {

   // Initialize pointers
   fcst_gci    = (GCInfo *)      0;
   obs_gci     = (GCInfo *)      0;
   fcst_ta     = (ThreshArray *) 0;
   obs_ta      = (ThreshArray *) 0;
   interp_mthd = (InterpMthd *)  0;
   interp_wdth = (int *)         0;
   mask_wd     = (WrfData *)     0;
   mask_name   = (char **)       0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GridStatConfInfo::clear() {
   int i;

   // Set counts to zero
   n_vx          = 0;
   n_vx_scal     = 0;
   n_vx_vect     = 0;
   n_vx_prob     = 0;
   n_mask        = 0;
   n_wind_thresh = 0;
   n_interp      = 0;
   n_nbr_wdth    = 0;
   n_cov_thresh  = 0;
   n_ci_alpha    = 0;

   max_n_scal_thresh      = 0;
   max_n_prob_fcst_thresh = 0;
   max_n_prob_obs_thresh  = 0;

   fcst_wind_ta.clear();
   obs_wind_ta.clear();

   // Deallocate memory
   if(fcst_gci)    { delete [] fcst_gci;    fcst_gci    = (GCInfo *)      0; }
   if(obs_gci)     { delete [] obs_gci;     obs_gci     = (GCInfo *)      0; }
   if(fcst_ta)     { delete [] fcst_ta;     fcst_ta     = (ThreshArray *) 0; }
   if(obs_ta)      { delete [] obs_ta;      obs_ta      = (ThreshArray *) 0; }
   if(interp_mthd) { delete [] interp_mthd; interp_mthd = (InterpMthd *)  0; }
   if(interp_wdth) { delete [] interp_wdth; interp_wdth = (int *)         0; }
   if(mask_wd)     { delete [] mask_wd;     mask_wd     = (WrfData *)     0; }

   for(i=0; i<n_mask; i++) {
      if(mask_name[i]) {
         delete [] mask_name[i];
         mask_name[i] = (char *) 0;
      }
   }
   if(mask_name) { delete [] mask_name; mask_name = (char **) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void GridStatConfInfo::read_config(const char *file_name) {

   // Call the config read routine
   conf.read(file_name);

   // Process the configuration file
   process_config();

   return;
}

////////////////////////////////////////////////////////////////////////

void GridStatConfInfo::process_config() {
   int i, j, n, n_mthd, n_wdth;
   InterpMthd im;

   //
   // Conf: model
   //

   if(strlen(conf.model().sval()) == 0 ||
      check_reg_exp(ws_reg_exp, conf.model().sval()) == true) {

      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "The model name (\"" << conf.model().sval()
           << "\") must be non-empty and contain no embedded "
           << "whitespace.\n\n" << flush;
      exit(1);
   }

   //
   // Conf: fcst_field
   //

   // Parse out the GRIB codes to be verified
   n_vx = conf.n_fcst_field_elements();

   if(n_vx == 0) {

      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "At least one value must be provided "
           << "for fcst_field.\n\n" << flush;
      exit(1);
   }

   // Allocate space to store the GRIB code information
   fcst_gci = new GCInfo [n_vx];

   // Parse the fcst field information
   for(i=0; i<n_vx; i++) {
      fcst_gci[i].set_gcinfo(conf.fcst_field(i).sval(),
         conf.grib_ptv().ival());

      // No support for WDIR
      if(fcst_gci[i].code == wdir_grib_code) {

         cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
              << "the wind direction field may not be verified "
              << "using grid_stat.\n\n"
              << flush;
         exit(1);
      }
   }

   //
   // Conf: obs_field
   //

   // Check if the length of obs_field is non-zero and
   // not equal to n_vx
   if(conf.n_obs_field_elements() != 0 &&
      conf.n_obs_field_elements() != n_vx) {

      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "The length of obs_field must be the same as the "
           << "length of fcst_field.\n\n" << flush;
      exit(1);
   }

   // Allocate space to store the GRIB code information
   obs_gci = new GCInfo [n_vx];

   // Parse the obs field information
   for(i=0; i<n_vx; i++) {

      // If obs_field is empty, use fcst_field
      if(conf.n_obs_field_elements() == 0) {
         obs_gci[i].set_gcinfo(conf.fcst_field(i).sval(),
            conf.grib_ptv().ival());
      }
      else {
         obs_gci[i].set_gcinfo(conf.obs_field(i).sval(),
            conf.grib_ptv().ival());
      }

      // No support for WDIR
      if(obs_gci[i].code == wdir_grib_code) {

         cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
              << "the wind direction field may not be verified "
              << "using grid_stat.\n\n"
              << flush;
         exit(1);
      }
   }

   // Check that the observation field does not contain probabilities
   for(i=0; i<n_vx; i++) {
      if(obs_gci[i].pflag == 1) {
         cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
              << "The observation field cannot contain probabilities."
              << "\n\n" << flush;
         exit(1);
      }
   }

   // If VL1L2 is requested, check the specified GRIB codes and turn
   // on the vflag when UGRD is followed VGRD at the same level
   if(conf.output_flag(i_vl1l2).ival()  > 0) {

      for(i=0, n_vx_vect = 0; i<n_vx; i++) {

         if(i+1 < n_vx &&
            fcst_gci[i].code   == ugrd_grib_code &&
            obs_gci[i].code    == ugrd_grib_code &&
            fcst_gci[i+1].code == vgrd_grib_code &&
            obs_gci[i+1].code  == vgrd_grib_code &&
            strcmp(fcst_gci[i].lvl_str, fcst_gci[i+1].lvl_str) == 0 &&
            strcmp(obs_gci[i].lvl_str, obs_gci[i+1].lvl_str) == 0) {

            fcst_gci[i].vflag   = 1;
            obs_gci[i].vflag    = 1;
            fcst_gci[i+1].vflag = 1;
            obs_gci[i+1].vflag  = 1;

            // Increment the number of vector fields to be verified
            n_vx_vect++;
         }
      } // end for
   } // end if

   // Figure out the number of scalar and probability fields to be
   // verified.
   for(i=0, n_vx_prob = 0, n_vx_scal = 0; i<n_vx; i++) {
      if(fcst_gci[i].pflag == 1) n_vx_prob++;
      else                       n_vx_scal++;
   }

   //
   // Conf: fcst_thresh
   //

   // Check that the number of forecast threshold levels matches n_vx
   if(conf.n_fcst_thresh_elements() != n_vx) {

      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "The number fcst_thresh levels provided must match the "
           << "number of fields provided in fcst_field.\n\n"
           << flush;
      exit(1);
   }

   // Allocate space to store the threshold information
   fcst_ta = new ThreshArray [n_vx];

   // Parse the fcst threshold information
   for(i=0; i<n_vx; i++) {
      fcst_ta[i].parse_thresh_str(conf.fcst_thresh(i).sval());

      // Verifying a probability field
      if(fcst_gci[i].pflag == 1) {

         n = fcst_ta[i].n_elements();

         // Check for at least 3 thresholds beginning with 0 and ending with 1.
         if(n < 3 ||
            !is_eq(fcst_ta[i][0].thresh,   0.0) ||
            !is_eq(fcst_ta[i][n-1].thresh, 1.0)) {

            cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
                 << "When verifying a probability field, you must "
                 << "select at least 3 thresholds beginning with 0.0 "
                 << "and ending with 1.0.\n\n"
                 << flush;
            exit(1);
         }

         for(j=0; j<n; j++) {

            // Check that all threshold types are greater than or equal to
            if(fcst_ta[i][j].type != thresh_ge) {
               cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
                    << "When verifying a probability field, all "
                    << "thresholds must be set as equal to, "
                    << "using \"ge\" or \">=\".\n\n"
                    << flush;
               exit(1);
            }

            // Check that all thresholds are in [0, 1].
            if(fcst_ta[i][j].thresh < 0.0 ||
               fcst_ta[i][j].thresh > 1.0) {

               cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
                    << "When verifying a probability field, all "
                    << "thresholds must be between 0 and 1.\n\n"
                    << flush;
               exit(1);
            }
         } // end for j
      }
   } // end for i

   //
   // Conf: obs_thresh
   //

   // Check if the length of obs_thresh is non-zero and
   // not equal to n_vx
   if(conf.n_obs_thresh_elements() != 0 &&
      conf.n_obs_thresh_elements() != n_vx) {

      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "The number obs_thresh levels provided must match the "
           << "number of fields provided in obs_field.\n\n"
           << flush;
      exit(1);
   }

   // Allocate space to store the threshold information
   obs_ta = new ThreshArray [n_vx];

   // Parse the obs threshold information
   for(i=0; i<n_vx; i++) {

      // If obs_thresh is emptpy, use fcst_thresh
      if(conf.n_obs_thresh_elements() == 0) {
         obs_ta[i].parse_thresh_str(conf.fcst_thresh(i).sval());
      }
      else {
         obs_ta[i].parse_thresh_str(conf.obs_thresh(i).sval());
      }
   }

   // Check that number of thresholds specified for each field is the
   // same and compute the maximum number of thresholds
   max_n_scal_thresh      = 0;
   max_n_prob_fcst_thresh = 0;
   max_n_prob_obs_thresh  = 0;
   for(i=0; i<n_vx; i++) {

      // Only check for non-probability fields
      if(fcst_gci[i].pflag == 0 &&
         fcst_ta[i].n_elements() != obs_ta[i].n_elements()) {

         cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
              << "The number of thresholds for each field in "
              << "fcst_thresh must match the number of thresholds "
              << "for each field in obs_thresh.\n\n"
              << flush;
         exit(1);
      }

      // Verifying with multi-category contingency tables
      if(fcst_gci[i].pflag == 0 &&
         (conf.output_flag(i_mctc).ival() ||
          conf.output_flag(i_mcts).ival())) {

         // Check that the threshold values are monotonically increasing
         // and the threshold types are inequalities that remain the same
         for(j=0; j<fcst_ta[i].n_elements()-1; j++) {

            if(fcst_ta[i][j].thresh >  fcst_ta[i][j+1].thresh ||
               obs_ta[i][j].thresh  >  obs_ta[i][j+1].thresh  ||
               fcst_ta[i][j].type   != fcst_ta[i][j+1].type   ||
               obs_ta[i][j].type    != obs_ta[i][j+1].type    ||
               fcst_ta[i][j].type   == thresh_eq              ||
               fcst_ta[i][j].type   == thresh_ne              ||
               obs_ta[i][j].type    == thresh_eq              ||
               obs_ta[i][j].type    == thresh_ne) {

               cerr << "\n\nERROR: PointStatConfInfo::process_config() -> "
                    << "when verifying using multi-category contingency "
                    << "tables, the thresholds must be monotonically "
                    << "increasing and be of the same inequality type "
                    << "(lt, le, gt, or ge).\n\n" << flush;
               exit(1);
            }
         }
      }

      // Look for the maximum number of thresholds for scalar fields
      if(fcst_gci[i].pflag == 0) {

         if(fcst_ta[i].n_elements() > max_n_scal_thresh)
            max_n_scal_thresh = fcst_ta[i].n_elements();
      }
      // Look for the maximum number of thresholds for prob fields
      else {

         if(fcst_ta[i].n_elements() > max_n_prob_fcst_thresh)
            max_n_prob_fcst_thresh = fcst_ta[i].n_elements();
         if(obs_ta[i].n_elements() > max_n_prob_obs_thresh)
            max_n_prob_obs_thresh = obs_ta[i].n_elements();
      }
   }

   //
   // Conf: fcst_wind_thresh
   //

   // Parse the fcst wind threshold information
   for(i=0; i<conf.n_fcst_wind_thresh_elements(); i++) {
      fcst_wind_ta.add(conf.fcst_wind_thresh(i).sval());
   }
   n_wind_thresh = fcst_wind_ta.n_elements();

   //
   // Conf: obs_wind_thresh
   //

   // If obs_wind_thresh is emptpy, use fcst_wind_thresh
   if(conf.n_obs_wind_thresh_elements() == 0) {
     obs_wind_ta = fcst_wind_ta;
   }
   else {

      // Parse the obs wind threshold information
      for(i=0; i<conf.n_obs_wind_thresh_elements(); i++) {
         obs_wind_ta.add(conf.obs_wind_thresh(i).sval());
      }
   }

   // Check that the number of wind speed thresholds match
   if(fcst_wind_ta.n_elements() != obs_wind_ta.n_elements()) {
      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "The number of thresholds in fcst_wind_thresh must match "
           << "the number of thresholds in obs_wind_thresh.\n\n"
           << flush;
      exit(1);
   }

   //
   // Conf: ci_alpha
   //

   // Check that at least one alpha value is provided
   if((n_ci_alpha = conf.n_ci_alpha_elements()) == 0) {

      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "At least one confidence interval alpha value must be "
           << "specified.\n\n" << flush;
      exit(1);
   }

   // Check that the values for alpha are between 0 and 1
   for(i=0; i<n_ci_alpha; i++) {
      if(conf.ci_alpha(i).dval() <= 0.0 ||
         conf.ci_alpha(i).dval() >= 1.0) {

         cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
              << "All confidence interval alpha values ("
              << conf.ci_alpha(i).dval() << ") must be greater than 0 "
              << "and less than 1.\n\n" << flush;
         exit(1);
      }
   }

   //
   // Conf: boot_interval
   //

   // Check that boot_interval is set to 0 or 1
   if(conf.boot_interval().ival() != boot_bca_flag &&
      conf.boot_interval().ival() != boot_perc_flag) {

      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "The boot_interval parameter must be set to "
           << boot_bca_flag << " or "
           << boot_perc_flag << "!\n\n" << flush;
      exit(1);
   }

   //
   // Conf: boot_rep_prop
   //

   // Check that boot_rep_prop is set between 0 and 1
   if(conf.boot_rep_prop().dval() <= 0.0 ||
      conf.boot_rep_prop().dval() > 1.0) {

      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "The boot_rep_prop parameter must be set between "
           << "0 and 1!\n\n" << flush;
      exit(1);
   }

   //
   // Conf: n_boot_rep
   //

   // Check that n_boot_rep is set > 0
   if(conf.n_boot_rep().dval() < 0.0) {

      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "The number of bootstrap resamples (n_boot_rep) "
           << "must be set to a value >= 0.\n\n" << flush;
      exit(1);
   }

   //
   // Conf: interp_method
   //

   // Check that at least one interpolation method is provided
   if((n_mthd = conf.n_interp_method_elements()) <= 0) {

      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "At least one interpolation method must be provided.\n\n"
           << flush;
      exit(1);
   }

   // Check that distance-weighted mean and least squares fit are
   // not selected
   for(i=0; i<n_mthd; i++) {

      im = string_to_interpmthd(conf.interp_method(i).sval());

      if(im == im_dw_mean || im == im_ls_fit) {

         cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
              << "The interpolation method may not be set to DW_MEAN "
              << "or LS_FIT for grid_stat.\n\n" << flush;
         exit(1);
      }
   }

   //
   // Conf: interp_width
   //

   // Check that at least one interpolation width is provided
   if((n_wdth = conf.n_interp_width_elements()) <= 0) {

      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "At least one interpolation width must be provided.\n\n"
           << flush;
      exit(1);
   }

   // Do error checking and compute the total number of
   // interpolations to be performed
   n_interp = 0;
   for(i=0; i<n_wdth; i++) {

      if(conf.interp_width(i).ival() < 1 ||
         conf.interp_width(i).ival()%2 == 0) {

         cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
              << "The interpolation width must be set "
              << "to odd values greater than or equal to 1 ("
              << conf.interp_width(i).ival() << ").\n\n" << flush;
         exit(1);
      }

      if(conf.interp_width(i).ival() == 1) n_interp += 1;
      if(conf.interp_width(i).ival() >  1) n_interp += n_mthd;
   }

   // Allocate space for the interpolation methods and widths
   interp_mthd = new InterpMthd [n_interp];
   interp_wdth = new int        [n_interp];

   // Set each interpolation method and width
   for(i=0, n=0; i<n_wdth; i++) {

      // For an interpolation width of 1, set the method the method to
      // unweighted mean - which is really just nearest neighbor
      if(conf.interp_width(i).ival() == 1) {
         interp_mthd[n] = im_uw_mean;
         interp_wdth[n] = conf.interp_width(i).ival();
         n++;
         continue;
      }

      for(j=0; j<n_mthd; j++) {
         interp_mthd[n] =
            string_to_interpmthd(conf.interp_method(j).sval());
         interp_wdth[n] = conf.interp_width(i).ival();
         n++;
      }
   }

   //
   // Conf: interp_flag
   //

   // Check that the interpolation flag is set between 1 and 3.
   if(conf.interp_flag().dval() < 1.0 ||
      conf.interp_flag().dval() > 3.0) {

         cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
              << "The interpolation threshold value must be set "
              << "between 1 and 3.\n\n" << flush;
         exit(1);
   }

   //
   // Conf: interp_thresh
   //

   // Check that the interpolation threshold is set between 0 and 1.
   if(conf.interp_thresh().dval() < 0.0 ||
      conf.interp_thresh().dval() > 1.0) {

         cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
              << "The interpolation threshold value must be set "
              << "between 0 and 1.\n\n" << flush;
         exit(1);
   }

   //
   // Conf: nbr_width
   //

   // Check that at least one neighborhood width is provided
   if((n_nbr_wdth = conf.n_nbr_width_elements()) <= 0) {

      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "At least one neighborhood width must be provided.\n\n"
           << flush;
      exit(1);
   }

   // Do error checking of neighborhood widths
   for(i=0; i<n_nbr_wdth; i++) {

      if(conf.nbr_width(i).ival() < 1 ||
         conf.nbr_width(i).ival()%2 == 0) {

         cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
              << "The neighborhood width must be set "
              << "to odd values greater than or equal to 1 ("
              << conf.nbr_width(i).ival() << ").\n\n" << flush;
         exit(1);
      }
   }

   //
   // Conf: nbr_thresh
   //

   // Check that the neighborhood threshold is set between 0 and 1.
   if(conf.nbr_thresh().dval() < 0.0 ||
      conf.nbr_thresh().dval() > 1.0) {

         cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
              << "The neighborhood threshold value must be set "
              << "between 0 and 1.\n\n" << flush;
         exit(1);
   }

   //
   // Conf: cov_thresh 
   //

   // Check that at least one neighborhood fractional threshold is provided
   if((n_cov_thresh = conf.n_cov_thresh_elements()) <= 0) {

      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "At least one neighborhood coverage threshold value "
           << "must be provided.\n\n"
           << flush;
      exit(1);
   }

   // Do error checking of neighborhood coverage thresholds
   for(i=0; i<n_cov_thresh; i++) {

      // Parse the coverage threshold information
      frac_ta.add(conf.cov_thresh(i).sval());

      if(frac_ta[i].thresh < 0.0 ||
         frac_ta[i].thresh > 1.0) {

         cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
              << "The neighborhood fraction threshold value must be set "
              << "between 0 and 1.\n\n" << flush;
         exit(1);
      }
   }

   //
   // Conf: output_flag
   //

   // Make sure the output_flag is the expected length
   if(conf.n_output_flag_elements() != n_out) {

      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "Found " << conf.n_output_flag_elements()
           << " elements in the output_flag but expected " << n_out
           << ".\n\n"
           << flush;
      exit(1);
   }

   // Check that at least one output STAT type is requested
   for(i=0, n=0; i<n_txt; i++) {
      n += (conf.output_flag(i).ival() > flag_no_out);
   }

   if(n == 0) {

      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "At least one output STAT type must be requested.\n\n"
           << flush;
      exit(1);
   }

   //
   // Conf: rank_corr_flag
   //
   if(conf.rank_corr_flag().ival() != 0 &&
      conf.rank_corr_flag().ival() != 1) {

      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "The rank_corr_flag (" << conf.rank_corr_flag().ival()
           << ") must be set to 0 or 1.\n\n"
           << flush;
      exit(1);
   }

   //
   // Conf: tmp_dir
   //
   if(opendir(conf.tmp_dir().sval()) == NULL ) {
      cerr << "\n\nERROR: GridStatConfInfo::process_config() -> "
           << "Cannot access the tmp_dir temporary directory: "
           << conf.tmp_dir().sval() << "\n\n" << flush;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GridStatConfInfo::process_masks(const Grid &grid) {
   int i, n_mask_grid, n_mask_poly;

   // Get the number of masks
   n_mask_grid = conf.n_mask_grid_elements();
   n_mask_poly = conf.n_mask_poly_elements();
   n_mask      = n_mask_grid + n_mask_poly;

   // Check that at least one verification masking region is provided
   if(n_mask == 0) {

      cerr << "\n\nERROR: GridStatConfInfo::process_masks() -> "
           << "At least one grid or polyline verification masking "
           << "region must be provided.\n\n" << flush;
      exit(1);
   }

   // Allocate space to store the masking information
   mask_wd   = new WrfData [n_mask];
   mask_name = new char *  [n_mask];

   // Initialize the mask name pointers
   for(i=0; i<n_mask; i++) mask_name[i] = (char *) 0;

   // Parse out the masking grids
   for(i=0; i<n_mask_grid; i++)
      parse_grid_mask(conf.mask_grid(i).sval(), grid,
                      mask_wd[i], mask_name[i]);

   // Parse out the masking polys
   for(i=0; i<n_mask_poly; i++)
      parse_poly_mask(conf.mask_poly(i).sval(), grid,
                      mask_wd[i+n_mask_grid], mask_name[i+n_mask_grid]);

   return;
}

////////////////////////////////////////////////////////////////////////

int GridStatConfInfo::n_txt_row(int i) {
   int n;

   // Switch on the index of the line type
   switch(i) {

      case(i_fho):
         // Maximum number of FHO lines possible =
         //    Fields * Masks * Smoothing Methods * Max Thresholds
         n = n_vx_scal * n_mask * n_interp * max_n_scal_thresh;
         break;

      case(i_ctc):
         // Maximum number of CTC lines possible =
         //    Fields * Masks * Smoothing Methods * Max Thresholds
         n = n_vx_scal * n_mask * n_interp * max_n_scal_thresh;
         break;

      case(i_cts):
         // Maximum number of CTS lines possible =
         //    Fields * Masks * Smoothing Methods * Max Thresholds *
         //    Alphas
         n = n_vx_scal * n_mask * n_interp * max_n_scal_thresh *
             n_ci_alpha;
         break;

      case(i_mctc):
         // Maximum number of CTC lines possible =
         //    Fields * Masks * Smoothing Methods
         n = n_vx_scal * n_mask * n_interp;
         break;

      case(i_mcts):
         // Maximum number of CTS lines possible =
         //    Fields * Masks * Smoothing Methods * Alphas
         n = n_vx_scal * n_mask * n_interp * n_ci_alpha;
         break;

      case(i_cnt):
         // Maximum number of CNT lines possible =
         //    Fields * Masks * Smoothing Methods * Alphas
         n = n_vx_scal * n_mask * n_interp * n_ci_alpha;
         break;

      case(i_sl1l2):
         // Maximum number of SL1L2 lines possible =
         //    Fields * Masks * Smoothing Methods
         n = n_vx_scal * n_mask * n_interp;
         break;

      case(i_vl1l2):
         // Maximum number of VL1L2 lines possible =
         //    Fields * Masks * Smoothing Methods * Wind Thresholds
         n = n_vx_vect * n_mask * n_interp * n_wind_thresh;
         break;

      case(i_nbrctc):
         // Maximum number of NBRCTC lines possible =
         //    Fields * Masks * Max Thresholds *
         //    Neighborhoods * Frac Thresholds
         n = n_vx_scal * n_mask * max_n_scal_thresh * n_nbr_wdth *
             n_cov_thresh;
         break;

      case(i_nbrcts):
         // Maximum number of NBRCTS lines possible =
         //    Fields * Masks * Max Thresholds *
         //    Neighborhoods * Frac Thresholds * Alphas
         n = n_vx_scal * n_mask * max_n_scal_thresh * n_nbr_wdth *
             n_cov_thresh * n_ci_alpha;
         break;

      case(i_nbrcnt):
         // Maximum number of NBRCTS lines possible =
         //    Fields * Masks * Max Thresholds *
         //    Neighborhoods * Frac Thresholds * Alphas
         n = n_vx_scal * n_mask * max_n_scal_thresh * n_nbr_wdth *
             n_cov_thresh * n_ci_alpha;
         break;

      case(i_pct):
         // Maximum number of PCT lines possible =
         //    Fields * Masks * Smoothing Methods * Max Thresholds
         n = n_vx_prob * n_mask * n_interp * max_n_prob_obs_thresh;
         break;

      case(i_pstd):
         // Maximum number of PSTD lines possible =
         //    Fields * Masks * Smoothing Methods * Max Thresholds *
         //    Alphas
         n = n_vx_prob * n_mask * n_interp * max_n_prob_obs_thresh *
             n_ci_alpha;
         break;

      case(i_pjc):
         // Maximum number of PJC lines possible =
         //    Fields * Masks * Smoothing Methods * Max Thresholds
         n = n_vx_prob * n_mask * n_interp * max_n_prob_obs_thresh;
         break;

      case(i_prc):
         // Maximum number of PRC lines possible =
         //    Fields * Masks * Smoothing Methods *
         //    Max Thresholds
         n = n_vx_prob * n_mask * n_interp * max_n_prob_obs_thresh;
         break;

      default:
         cerr << "\n\nERROR: GridStatConfInfo::n_txt_row(int) -> "
              << "unexpected output type index value: " << i << "\n\n"
              << flush;
         exit(1);
         break;
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////

int GridStatConfInfo::n_stat_row() {
   int i, n;

   // Set the maximum number of STAT output lines by summing the counts
   // for the optional text files that have been requested
   for(i=0, n=0; i<n_txt; i++) {

      if(conf.output_flag(i).ival() > 0) n += n_txt_row(i);
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////
