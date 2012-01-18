// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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

#include "point_stat_conf_info.h"

#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class PointStatConfInfo
//
////////////////////////////////////////////////////////////////////////

PointStatConfInfo::PointStatConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PointStatConfInfo::~PointStatConfInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::init_from_scratch() {

   // Initialize pointers
   vx_pd       = (VxPairDataPoint *)  0;
   fcst_ta     = (ThreshArray *)      0;
   obs_ta      = (ThreshArray *)      0;
   msg_typ     = (ConcatString *)     0;
   interp_mthd = (InterpMthd *)       0;
   interp_wdth = (int *)              0;
   mask_dp     = (DataPlane *)        0;
   mask_name   = (ConcatString *)     0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::clear() {

   // Set counts to zero
   n_vx          = 0;
   n_vx_scal     = 0;
   n_vx_vect     = 0;
   n_vx_prob     = 0;
   n_msg_typ     = 0;
   n_mask        = 0;
   n_mask_area   = 0;
   n_mask_sid    = 0;
   n_wind_thresh = 0;
   n_interp      = 0;
   n_ci_alpha    = 0;

   max_n_scal_thresh      = 0;
   max_n_prob_fcst_thresh = 0;
   max_n_prob_obs_thresh  = 0;

   fcst_wind_ta.clear();
   obs_wind_ta.clear();

   // Deallocate memory
   if(vx_pd)       { delete [] vx_pd;       vx_pd       = (VxPairDataPoint *)  0; }
   if(fcst_ta)     { delete [] fcst_ta;     fcst_ta     = (ThreshArray *)      0; }
   if(obs_ta)      { delete [] obs_ta;      obs_ta      = (ThreshArray *)      0; }
   if(msg_typ)     { delete [] msg_typ;     msg_typ     = (ConcatString *)     0; }
   if(interp_mthd) { delete [] interp_mthd; interp_mthd = (InterpMthd *)       0; }
   if(interp_wdth) { delete [] interp_wdth; interp_wdth = (int *)              0; }
   if(mask_dp)     { delete [] mask_dp;     mask_dp     = (DataPlane *)        0; }
   if(mask_name)   { delete [] mask_name;   mask_name   = (ConcatString *)     0; }

   mask_sid.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::read_config(const char *default_file_name,
                                    const char *user_file_name,
                                    GrdFileType ftype, unixtime fcst_valid_ut, int fcst_lead_sec) {

   // Read the default config file
   conf.read(default_file_name);

   // Read the user-specified config file
   conf.read(user_file_name);

   // Process the configuration file
   process_config(ftype, fcst_valid_ut, fcst_lead_sec);

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::process_config(GrdFileType ftype, unixtime fcst_valid_ut, int fcst_lead_sec) {
   int i, j, n, n_mthd, n_wdth, a, b;
   ConcatString grib_ptv_str;
   InterpMthd method;
   VarInfoFactory info_factory;

   //
   // Conf: version
   //

   check_met_version(conf.version().sval());

   //
   // Conf: model
   //

   if(strlen(conf.model().sval()) == 0 ||
      check_reg_exp(ws_reg_exp, conf.model().sval()) == true) {

      mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
           << "The model name (\"" << conf.model().sval()
           << "\") must be non-empty and contain no embedded "
           << "whitespace.\n\n";
      exit(1);
   }

   //
   // Conf: grib_ptv
   //

   // Store the grib_ptv as a string to be passed to the VarInfo objects
   grib_ptv_str << conf.grib_ptv().ival();

   //
   // Conf: fcst_field
   //

   // Parse out the GRIB codes to be verified
   n_vx = conf.n_fcst_field_elements();

   if(n_vx == 0) {

      mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
           << "At least one value must be provided "
           << "for fcst_field.\n\n";
      exit(1);
   }

   // Allocate space to store the GRIB code information
   vx_pd = new VxPairDataPoint [n_vx];

   // Parse the fcst field information
   for(i=0; i<n_vx; i++) {

      // Get new VarInfo object
      vx_pd[i].fcst_info = info_factory.new_var_info(ftype);

      // Set the GRIB parameter table version number and pass the magic string
      vx_pd[i].fcst_info->set_pair(CONFIG_GRIB_PTV, grib_ptv_str);
      vx_pd[i].fcst_info->set_magic(conf.fcst_field(i).sval());

      // Set the requested timing information
      if(fcst_valid_ut > 0)           vx_pd[i].fcst_info->set_valid(fcst_valid_ut);
      if(!is_bad_data(fcst_lead_sec)) vx_pd[i].fcst_info->set_lead(fcst_lead_sec);

      // No support for wind direction
      if(vx_pd[i].fcst_info->is_wind_direction()) {

         mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
              << "the wind direction field may not be verified "
              << "using point_stat.\n\n";
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

      mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
           << "The length of obs_field must be the same as the "
           << "length of fcst_field.\n\n";
      exit(1);
   }

   // Parse the obs field information
   for(i=0; i<n_vx; i++) {

      // Allocate a new VarInfoGrib object
      vx_pd[i].obs_info = new VarInfoGrib;
     
      // If obs_field is empty and the forecast file type is not GRIB1, error out
      if(conf.n_obs_field_elements() == 0 && ftype != FileType_Gb1) {
         mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
           << "When the forecast file is not GRIB1, \"obs_field\" cannot be blank.  "
           << "You must specify the verifying observations following the GRIB1 convention.\n\n";
         exit(1);
      }

      // Set the GRIB parameter table version number
      vx_pd[i].obs_info->set_pair(CONFIG_GRIB_PTV, grib_ptv_str);

      // If obs_field is empty, use fcst_field
      if(conf.n_obs_field_elements() == 0) {
         vx_pd[i].obs_info->set_magic(conf.fcst_field(i).sval());
      }
      else {
         vx_pd[i].obs_info->set_magic(conf.obs_field(i).sval());
      }

      // No support for wind direction
      if(vx_pd[i].obs_info->is_wind_direction()) {

         mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
              << "the wind direction field may not be verified "
              << "using point_stat.\n\n";
         exit(1);
      }

      // Check the levels for the fcst and obs fields.  If the
      // fcst_field is a range of pressure levels, check to see if the
      // range of obs_field pressure levels is wholly contained in the
      // fcst levels.  If not, print a warning message.
      if(vx_pd[i].fcst_info->level().type()  == LevelType_Pres &&
         vx_pd[i].fcst_info->level().lower() != vx_pd[i].fcst_info->level().upper() &&
         (vx_pd[i].obs_info->level().lower() <  vx_pd[i].fcst_info->level().lower() ||
          vx_pd[i].obs_info->level().upper() >  vx_pd[i].fcst_info->level().upper())) {

         mlog << Warning << "\n\n  PointStatConfInfo::process_config() -> "
              << "The range of requested observation pressure levels "
              << "is not contained within the range of requested "
              << "forecast pressure levels.  No vertical interpolation "
              << "will be performed for observations falling outside "
              << "the range of forecast levels.  Instead, they will be "
              << "matched to the single nearest forecast level.\n\n";
      }
   }

   // Check that the observation field does not contain probabilities
   for(i=0; i<n_vx; i++) {
      if(vx_pd[i].obs_info->p_flag()) {
         mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
              << "The observation field cannot contain probabilities."
              << "\n\n";
         exit(1);
      }
   }

   // If VL1L2 or VAL1L2 is requested, check the specified fields and turn
   // on the vflag when UGRD is followed by VGRD at the same level
   if(conf.output_flag(i_vl1l2).ival()  > 0 ||
      conf.output_flag(i_val1l2).ival() > 0) {

      for(i=0, n_vx_vect = 0; i<n_vx; i++) {

         if(i+1 < n_vx                        &&
            vx_pd[i].fcst_info->is_u_wind()   &&
            vx_pd[i].obs_info->is_u_wind()    &&
            vx_pd[i+1].fcst_info->is_v_wind() &&
            vx_pd[i+1].obs_info->is_v_wind()  &&
            vx_pd[i].fcst_info->req_level_name() == vx_pd[i+1].fcst_info->req_level_name() &&
            vx_pd[i].obs_info->req_level_name()  == vx_pd[i+1].obs_info->req_level_name()) {

            vx_pd[i].fcst_info->set_v_flag(true);
            vx_pd[i].obs_info->set_v_flag(true);
            vx_pd[i+1].fcst_info->set_v_flag(true);
            vx_pd[i+1].obs_info->set_v_flag(true);

            // Increment the number of vector fields to be verified
            n_vx_vect++;
         }
      } // end for
   } // end if

   // Compute the number of scalar and probability fields to be verified.
   for(i=0, n_vx_prob = 0, n_vx_scal = 0; i<n_vx; i++) {
      if(vx_pd[i].fcst_info->p_flag()) n_vx_prob++;
      else                             n_vx_scal++;
   }

   // Allocate space to store the threshold information
   fcst_ta = new ThreshArray [n_vx];
   obs_ta  = new ThreshArray [n_vx];

   //
   // Only sanity check thresholds for thresholded line types
   //
   if(conf.output_flag(i_fho).ival()  ||
      conf.output_flag(i_ctc).ival()  ||
      conf.output_flag(i_cts).ival()  ||
      conf.output_flag(i_mctc).ival() ||
      conf.output_flag(i_mcts).ival() ||
      conf.output_flag(i_pct).ival()  ||
      conf.output_flag(i_pstd).ival() ||
      conf.output_flag(i_pjc).ival()  ||
      conf.output_flag(i_prc).ival()) {

      //
      // Conf: fcst_thresh
      //

      // Check that the number of forecast threshold levels matches n_vx
      if(conf.n_fcst_thresh_elements() != n_vx) {

         mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
              << "The number fcst_thresh entries provided must match the "
              << "number of fields provided in fcst_field.\n\n";
         exit(1);
      }

      // Parse the fcst threshold information
      for(i=0; i<n_vx; i++) {
         fcst_ta[i].parse_thresh_str(conf.fcst_thresh(i).sval());

         // Verifying a probability field
         if(vx_pd[i].fcst_info->p_flag() == 1) {

            n = fcst_ta[i].n_elements();

            // Check for at least 3 thresholds beginning with 0 and ending with 1.
            if(n < 3 ||
               !is_eq(fcst_ta[i][0].thresh,   0.0) ||
               !is_eq(fcst_ta[i][n-1].thresh, 1.0)) {

               mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
                    << "When verifying a probability field, you must "
                    << "select at least 3 thresholds beginning with 0.0 "
                    << "and ending with 1.0.\n\n";
               exit(1);
            }

            for(j=0; j<n; j++) {

               // Check that all threshold types are greater than or equal to
               if(fcst_ta[i][j].type != thresh_ge) {
                  mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
                       << "When verifying a probability field, all "
                       << "thresholds must be set as equal to, "
                       << "using \"ge\" or \">=\".\n\n";
                  exit(1);
               }

               // Check that all thresholds are in [0, 1].
               if(fcst_ta[i][j].thresh < 0.0 ||
                  fcst_ta[i][j].thresh > 1.0) {

                  mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
                       << "When verifying a probability field, all "
                       << "thresholds must be between 0 and 1.\n\n";
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

         mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
              << "The number obs_thresh entries provided must match the "
              << "number of fields provided in obs_field.\n\n";
         exit(1);
      }

      // Parse the obs threshold information
      for(i=0; i<n_vx; i++) {

         // If obs_thresh is empty, use fcst_thresh
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
         if(vx_pd[i].fcst_info->p_flag() == 0 &&
            fcst_ta[i].n_elements() != obs_ta[i].n_elements()) {

            mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
                 << "The number of thresholds for each field in "
                 << "fcst_thresh must match the number of thresholds "
                 << "for each field in obs_thresh.\n\n";
            exit(1);
         }

         // Verifying with multi-category contingency tables
         if(vx_pd[i].fcst_info->p_flag() == 0 &&
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

                  mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
                       << "when verifying using multi-category contingency "
                       << "tables, the thresholds must be monotonically "
                       << "increasing and be of the same inequality type "
                       << "(lt, le, gt, or ge).\n\n";
                  exit(1);
               }
            }
         }

         // Look for the maximum number of thresholds for scalar fields
         if(vx_pd[i].fcst_info->p_flag() == 0) {

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

   // If obs_wind_thresh is empty, use fcst_wind_thresh
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
      mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
           << "The number of thresholds in fcst_wind_thresh must match "
           << "the number of thresholds in obs_wind_thresh.\n\n";
      exit(1);
   }

   //
   // Conf: message_type
   //

   // Check that at least one PrepBufr message type is provided
   if((n_msg_typ = conf.n_message_type_elements()) <= 0) {
      mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
           << "At least one PrepBufr message type must be provided.\n\n";
      exit(1);
   }

   // Store the message types
   msg_typ = new ConcatString [n_msg_typ];
   for(i=0; i<n_msg_typ; i++) msg_typ[i] = conf.message_type(i).sval();

   // Check that each PrepBufr message type provided is valid
   for(i=0; i<n_msg_typ; i++) {

      if(strstr(vld_msg_typ_str, msg_typ[i]) == NULL) {
         mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
              << "Invalid message type string provided ("
              << conf.message_type(i).sval() << ").\n\n";
         exit(1);
      }
   }

   //
   // Conf: ci_alpha
   //

   // Check that at least one alpha value is provided
   if((n_ci_alpha = conf.n_ci_alpha_elements()) == 0) {

      mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
           << "At least one confidence interval alpha value must be "
           << "specified.\n\n";
      exit(1);
   }

   // Check that the values for alpha are between 0 and 1
   for(i=0; i<n_ci_alpha; i++) {
      if(conf.ci_alpha(i).dval() <= 0.0 ||
         conf.ci_alpha(i).dval() >= 1.0) {

         mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
              << "All confidence interval alpha values ("
              << conf.ci_alpha(i).dval() << ") must be greater than 0 "
              << "and less than 1.\n\n";
         exit(1);
      }
   }

   //
   // Conf: boot_interval
   //

   // Check that boot_interval is set to 0 or 1
   if(conf.boot_interval().ival() != boot_bca_flag &&
      conf.boot_interval().ival() != boot_perc_flag) {

      mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
           << "The boot_interval parameter must be set to "
           << boot_bca_flag << " or "
           << boot_perc_flag << "!\n\n";
      exit(1);
   }

   //
   // Conf: boot_rep_prop
   //

   // Check that boot_rep_prop is set between 0 and 1
   if(conf.boot_rep_prop().dval() <= 0.0 ||
      conf.boot_rep_prop().dval() > 1.0) {

      mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
           << "The boot_rep_prop parameter must be set between "
           << "0 and 1!\n\n";
      exit(1);
   }

   //
   // Conf: n_boot_rep
   //

   // Check that n_boot_rep is set > 0
   if(conf.n_boot_rep().dval() < 0.0) {

      mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
           << "The number of bootstrap resamples (n_boot_rep) "
           << "must be set to a value >= 0.\n\n";
      exit(1);
   }

   //
   // Conf: interp_method
   //

   // Check that at least one interpolation method is provided
   if((n_mthd = conf.n_interp_method_elements()) <= 0) {

      mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
           << "At least one interpolation method must be provided.\n\n";
      exit(1);
   }

   //
   // Conf: interp_width
   //

   // Check that at least one interpolation width is provided
   if((n_wdth = conf.n_interp_width_elements()) <= 0) {

      mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
           << "At least one interpolation width must be provided.\n\n";
      exit(1);
   }

   // Compute the number of interpolation methods to be used
   n_interp = 0;

   // Check for nearest neighbor special case
   for(i=0, a=n_wdth; i<n_wdth; i++) {

      if(conf.interp_width(i).ival() == 1) {
         a--;
         n_interp++;
      }

      // Perform error checking on widths
      if(conf.interp_width(i).ival() < 1) {
         mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
              << "The interpolation width values must be set "
              << "greater than or equal to 1 ("
              << conf.interp_width(i).ival() << ").\n\n";
         exit(1);
      }
   }

   // Check for bilinear interpolation special case
   for(i=0, b=n_mthd; i<n_mthd; i++) {

      method = string_to_interpmthd(conf.interp_method(i).sval());

      if(method == InterpMthd_Bilin) {
         b--;
         n_interp++;
      }
   }

   // Compute n_interp
   n_interp += a*b;

   // Allocate space for the interpolation methods and widths
   interp_mthd = new InterpMthd [n_interp];
   interp_wdth = new int        [n_interp];

   // Initialize the interpolation method count
   n = 0;

   // Check for the nearest neighbor special case
   for(i=0; i<n_wdth; i++) {
      if(conf.interp_width(i).ival() == 1) {
         interp_mthd[n] = InterpMthd_UW_Mean;
         interp_wdth[n] = 1;
         n++;
      }
   }

   // Check for the bilinear interpolation special case
   for(i=0; i<n_mthd; i++) {
      method = string_to_interpmthd(conf.interp_method(i).sval());
      if(method == InterpMthd_Bilin) {
         interp_mthd[n] = InterpMthd_Bilin;
         interp_wdth[n] = 2;
         n++;
      }
   }

   // Loop through the interpolation widths
   for(i=0; i<n_wdth; i++) {

      // Skip the nearest neighbor case
      if(conf.interp_width(i).ival() == 1) continue;

      // Loop through the interpolation methods
      for(j=0; j<n_mthd; j++) {
         method = string_to_interpmthd(conf.interp_method(j).sval());

         // Skip the bilinear interpolation case
         if(method == InterpMthd_Bilin) continue;

         // Store the interpolation method and width
         interp_mthd[n] = method;
         interp_wdth[n] = conf.interp_width(i).ival();
         n++;
      }
   }

   //
   // Conf: interp_thresh
   //

   // Check that the interpolation threshold is set between
   // 0 and 1.
   if(conf.interp_thresh().dval() < 0.0 ||
      conf.interp_thresh().dval() > 1.0) {

         mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
              << "The interpolation threshold value must be set "
              << "between 0 and 1.\n\n";
         exit(1);
   }

   //
   // Conf: output_flag
   //

   // Make sure the output_flag is the expected length
   if(conf.n_output_flag_elements() != n_out) {

      mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
           << "Found " << conf.n_output_flag_elements()
           << " elements in the output_flag but expected " << n_out
           << ".\n\n";
      exit(1);
   }

   // Check that at least one output STAT type is requested
   for(i=0, n=0; i<n_txt; i++) {
      n += (conf.output_flag(i).ival() > 0);
   }

   if(n == 0) {

      mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
           << "At least one output STAT type must be requested.\n\n";
      exit(1);
   }

   //
   // Conf: rank_corr_flag
   //
   if(conf.rank_corr_flag().ival() != 0 &&
      conf.rank_corr_flag().ival() != 1) {

      mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
           << "The rank_corr_flag (" << conf.rank_corr_flag().ival()
           << ") must be set to 0 or 1.\n\n";
      exit(1);
   }

   //
   // Conf: tmp_dir
   //
   if(opendir(conf.tmp_dir().sval()) == NULL ) {
      mlog << Error << "\n\n  PointStatConfInfo::process_config() -> "
           << "Cannot access the tmp_dir temporary directory: "
           << conf.tmp_dir().sval() << "\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::process_masks(const Grid &grid) {
   int i, n_mask_grid, n_mask_poly;
   StringArray mask_sid_sa;

   // Get the number of masking areas
   n_mask_grid = conf.n_mask_grid_elements();
   n_mask_poly = conf.n_mask_poly_elements();
   n_mask_area = n_mask_grid + n_mask_poly;

   // Get the number of masking station ID's
   parse_sid_mask(conf.mask_sid().sval(), mask_sid_sa);
   n_mask_sid = mask_sid_sa.n_elements();

   // Save the total number masks as a sum of the masking areas and
   // masking points
   n_mask = n_mask_area + n_mask_sid;

   // Check that at least one verification masking region is provided
   if(n_mask == 0) {
      mlog << Error << "\n\n  PointStatConfInfo::process_masks() -> "
           << "At least one grid, polyline, or station ID "
           << "masking region must be provided.\n\n";
      exit(1);
   }

   // Allocate space to store the masking information
   mask_dp   = new DataPlane    [n_mask_area];
   mask_name = new ConcatString [n_mask];

   // Parse out the masking grid areas
   for(i=0; i<n_mask_grid; i++) {
      parse_grid_mask(conf.mask_grid(i).sval(), grid,
                      mask_dp[i], mask_name[i]);
   }

   // Parse out the masking poly areas
   for(i=0; i<n_mask_poly; i++) {
      parse_poly_mask(conf.mask_poly(i).sval(), grid,
                      mask_dp[i+n_mask_grid], mask_name[i+n_mask_grid]);
   }

   // Store the masking station ID points
   for(i=0; i<n_mask_sid; i++) {
      mask_name[i+n_mask_area] = mask_sid_sa[i];
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::set_vx_pd() {
   int i, j;

   // PairData is stored in the vx_pd objects in the following order:
   // [n_msg_typ][n_mask][n_interp]
   for(i=0; i<n_vx; i++) {

      // Set up the dimensions for the vx_pd object
      vx_pd[i].set_pd_size(n_msg_typ, n_mask, n_interp);

      // Add the verifying message type to the vx_pd objects
      for(j=0; j<n_msg_typ; j++)
         vx_pd[i].set_msg_typ(j, msg_typ[j]);

      // Add the masking information to the vx_pd objects
      for(j=0; j<n_mask; j++) {

         // If this is a masking area
         if(j<n_mask_area)
            vx_pd[i].set_mask_dp(j, mask_name[j], &mask_dp[j]);
         // Otherwise this is a masking StationID
         else
            vx_pd[i].set_mask_dp(j, mask_name[j], (DataPlane *) 0);
      }

      // Add the interpolation methods to the vx_pd objects
      for(j=0; j<n_interp; j++)
         vx_pd[i].set_interp(j, interp_mthd[j], interp_wdth[j]);
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

int PointStatConfInfo::n_txt_row(int i_txt_row) {
   int i, n;

   // Switch on the index of the line type
   switch(i_txt_row) {

      case(i_fho):
         // Maximum number of FHO lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds
         n = n_vx_scal * n_msg_typ * n_mask * n_interp *
             max_n_scal_thresh;
         break;

      case(i_ctc):
         // Maximum number of CTC lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds
         n = n_vx_scal * n_msg_typ * n_mask * n_interp *
             max_n_scal_thresh;
         break;

      case(i_cts):
         // Maximum number of CTS lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds * Alphas
         n = n_vx_scal * n_msg_typ * n_mask * n_interp *
             max_n_scal_thresh * n_ci_alpha;
         break;

      case(i_mctc):
         // Maximum number of MCTC lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods
         n = n_vx_scal * n_msg_typ * n_mask * n_interp;
         break;

      case(i_mcts):
         // Maximum number of MCTS lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Alphas
         n = n_vx_scal * n_msg_typ * n_mask * n_interp *
             n_ci_alpha;
         break;

      case(i_cnt):
         // Maximum number of CNT lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Alphas
         n = n_vx_scal * n_msg_typ * n_mask * n_interp * n_ci_alpha;
         break;

      case(i_sl1l2):
         // Maximum number of SL1L2 lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods
         n = n_vx_scal * n_msg_typ * n_mask * n_interp;
         break;

      case(i_sal1l2):
         // Maximum number of SAL1L2 lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods
         n = n_vx_scal * n_msg_typ * n_mask * n_interp;
         break;

      case(i_vl1l2):
         // Maximum number of VL1L2 lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Wind Thresholds
         n = n_vx_vect * n_msg_typ * n_mask * n_interp * n_wind_thresh;
         break;

      case(i_val1l2):
         // Maximum number of VAL1L2 lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Wind Thresholds
         n = n_vx_vect * n_msg_typ * n_mask * n_interp * n_wind_thresh;
         break;

      case(i_pct):
         // Maximum number of PCT lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds
         n = n_vx_prob * n_msg_typ * n_mask * n_interp *
             max_n_prob_obs_thresh;
         break;

      case(i_pstd):
         // Maximum number of PSTD lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds * Alphas
         n = n_vx_prob * n_msg_typ * n_mask * n_interp *
             max_n_prob_obs_thresh * n_ci_alpha;
         break;

      case(i_pjc):
         // Maximum number of PJC lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds
         n = n_vx_prob * n_msg_typ * n_mask * n_interp *
             max_n_prob_obs_thresh;
         break;

      case(i_prc):
         // Maximum number of PRC lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds
         n = n_vx_prob * n_msg_typ * n_mask * n_interp *
             max_n_prob_obs_thresh;
         break;

      case(i_mpr):
         // Compute the maximum number of matched pairs to be written
         // out by summing the number for each GCPairData object
         for(i=0, n=0; i<n_vx; i++) {
            n += vx_pd[i].get_n_pair();
         }
         break;

      default:
         mlog << Error << "\n\n  PointStatConfInfo::n_txt_row(int) -> "
              << "unexpected output type index value: " << i_txt_row
              << "\n\n";
         exit(1);
         break;
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////

int PointStatConfInfo::n_stat_row() {
   int i, n;

   // Set the maximum number of STAT output lines by summing the counts
   // for the optional text files that have been requested
   for(i=0, n=0; i<n_txt; i++) {

      if(conf.output_flag(i).ival() > 0) n += n_txt_row(i);
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////
