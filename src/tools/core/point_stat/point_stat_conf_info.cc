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
   fcst_ta     = (ThreshArray *)     0;
   obs_ta      = (ThreshArray *)     0;
   msg_typ     = (StringArray *)     0;
   mask_dp     = (DataPlane *)       0;
   interp_mthd = (InterpMthd *)      0;
   vx_pd       = (VxPairDataPoint *) 0;
   
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
   n_mask        = 0;
   n_mask_area   = 0;
   n_interp      = 0;

   max_n_scal_thresh      = 0;
   max_n_prob_fcst_thresh = 0;
   max_n_prob_obs_thresh  = 0;

   // Initialize values
   model.clear();
   beg_ds = end_ds = bad_data_int;
   fcst_field.clear();
   obs_field.clear();
   fcst_thresh.clear();
   obs_thresh.clear();
   fcst_wind_ta.clear();
   obs_wind_ta.clear();
   mask_name.clear();
   mask_sid.clear();
   ci_alpha.clear();
   duplicate_flag = DuplicateType_None;
   boot_interval = BootIntervalType_None;
   boot_rep_prop = bad_data_double;
   n_boot_rep = bad_data_int;
   boot_rng.clear();
   boot_seed.clear();
   interp_thresh = bad_data_double;
   rank_corr_flag = false;
   tmp_dir.clear();
   output_prefix.clear();
   version.clear();

   interp_wdth.clear();

   // Deallocate memory
   if(fcst_ta)     { delete [] fcst_ta;     fcst_ta     = (ThreshArray *)      0; }
   if(obs_ta)      { delete [] obs_ta;      obs_ta      = (ThreshArray *)      0; }
   if(interp_mthd) { delete [] interp_mthd; interp_mthd = (InterpMthd *)       0; }
   if(msg_typ)     { delete [] msg_typ;     msg_typ     = (StringArray *)      0; }
   if(mask_dp)     { delete [] mask_dp;     mask_dp     = (DataPlane *)        0; }
   if(vx_pd)       { delete [] vx_pd;       vx_pd       = (VxPairDataPoint *)  0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::read_config(const char *default_file_name,
                                    const char *user_file_name) {

   // Read the config file constants
   conf.read(replace_path(config_const_filename));
  
   // Read the default config file
   conf.read(default_file_name);

   // Read the user-specified config file
   conf.read(user_file_name);

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::process_config(GrdFileType ftype) {
   int i, j, n;
   ConcatString s;
   StringArray sa;
   VarInfoFactory info_factory;
   map<STATLineType,STATOutputType>output_map;
   Dictionary *fcst_dict = (Dictionary *) 0;
   Dictionary *obs_dict  = (Dictionary *) 0;
   Dictionary i_fcst_dict, i_obs_dict;
   BootInfo boot_info;
   InterpInfo interp_info;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);
   
   // Initialize
   clear();

   // Conf: version
   version = parse_conf_version(&conf);

   // Conf: model
   model = parse_conf_model(&conf);

   // Conf: beg_ds and end_ds
   beg_ds = conf.lookup_int("obs_window.beg_ds");
   end_ds = conf.lookup_int("obs_window.end_ds");
   if(beg_ds > end_ds) {
      mlog << Error << "\nPointStatConfInfo::process_config() -> "
           << "\"beg_ds\" cannot be greater than \"end_ds\": "
           << beg_ds << " > " << end_ds << "\n\n";
      exit(1);
   }

   // Conf: output_flag
   output_map = parse_conf_output_flag(&conf);

   // Make sure the output_flag is the expected size
   if((signed int) output_map.size() != n_txt) {
      mlog << Error << "\nPointStatConfInfo::process_config() -> "
           << "Unexpected number of entries found in \""
           << conf_key_output_flag << "\" ("
           << (signed int) output_map.size()
           << " != " << n_txt << ").\n\n";
      exit(1);
   }

   // Populate the output_flag array with map values
   for(i=0,n=0; i<n_txt; i++) {
      output_flag[i] = output_map[txt_file_type[i]];
      if(output_flag[i] != STATOutputType_None) n++;
   }

   // Check for at least one output line type
   if(n == 0) {
      mlog << Error << "\nPointStatConfInfo::process_config() -> "
           << "At least one output STAT type must be requested.\n\n";
      exit(1);
   }

   // Conf: fcst.field and obs.field
   fcst_dict = conf.lookup_array(conf_key_fcst_field);
   obs_dict  = conf.lookup_array(conf_key_obs_field);

   // Determine the number of fields (name/level) to be verified
   n_vx = parse_conf_n_vx(fcst_dict);

   // Check for a valid number of verification tasks
   if(n_vx == 0 || parse_conf_n_vx(obs_dict) != n_vx) {
      mlog << Error << "\nPointStatConfInfo::process_config() -> "
           << "The number of verification tasks in \"" << conf_key_obs_field
           << "\" must be non-zero and match the number in \""
           << conf_key_fcst_field << "\".\n\n";
      exit(1);
   }

   // Allocate space to store the GRIB code and threshold information
   vx_pd   = new VxPairDataPoint [n_vx];
   fcst_ta = new ThreshArray     [n_vx];
   obs_ta  = new ThreshArray     [n_vx];   
   msg_typ = new StringArray     [n_vx];
   
   // Parse the fcst field information
   for(i=0; i<n_vx; i++) {
     
      // Allocate new VarInfo objects
      vx_pd[i].fcst_info = info_factory.new_var_info(ftype);
      vx_pd[i].obs_info  = new VarInfoGrib;      

      // Get the current dictionaries
      i_fcst_dict = parse_conf_i_vx_dict(fcst_dict, i);
      i_obs_dict  = parse_conf_i_vx_dict(obs_dict, i);

      // Conf: msg_typ
      msg_typ[i] = parse_conf_message_type(&i_fcst_dict);
      
      // Set the current dictionaries
      vx_pd[i].fcst_info->set_dict(i_fcst_dict);
      vx_pd[i].obs_info->set_dict(i_obs_dict);

      // Dump the contents of the current VarInfo
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed forecast field number " << i+1 << ":\n";
         vx_pd[i].fcst_info->dump(cout);        
         mlog << Debug(5)
              << "Parsed observation field number " << i+1 << ":\n";
         vx_pd[i].obs_info->dump(cout);
      }
      
      // No support for wind direction
      if(vx_pd[i].fcst_info->is_wind_direction() ||
         vx_pd[i].obs_info->is_wind_direction()) {
         mlog << Error << "\nPointStatConfInfo::process_config() -> "
              << "the wind direction field may not be verified "
              << "using point_stat.\n\n";
         exit(1);
      }

      // Check the levels for the forecast and observation fields.  If the
      // forecast field is a range of pressure levels, check to see if the
      // range of observation field pressure levels is wholly contained in the
      // fcst levels.  If not, print a warning message.
      if(vx_pd[i].fcst_info->level().type() == LevelType_Pres &&
         !is_eq(vx_pd[i].fcst_info->level().lower(), vx_pd[i].fcst_info->level().upper()) &&
         (vx_pd[i].obs_info->level().lower() <  vx_pd[i].fcst_info->level().lower() ||
          vx_pd[i].obs_info->level().upper() >  vx_pd[i].fcst_info->level().upper())) {

         mlog << Warning
              << "\nPointStatConfInfo::process_config() -> "
              << "The range of requested observation pressure levels "
              << "is not contained within the range of requested "
              << "forecast pressure levels.  No vertical interpolation "
              << "will be performed for observations falling outside "
              << "the range of forecast levels.  Instead, they will be "
              << "matched to the single nearest forecast level.\n\n";
      }

      // Check that the observation field does not contain probabilities
      if(vx_pd[i].obs_info->p_flag()) {
         mlog << Error << "\nPointStatConfInfo::process_config() -> "
              << "The observation field cannot contain probabilities."
              << "\n\n";
         exit(1);
      }
   } // end for i

   // If VL1L2 or VAL1L2 is requested, check the specified fields and turn
   // on the vflag when UGRD is followed by VGRD at the same level
   if(output_flag[i_vl1l2]  != STATOutputType_None ||
      output_flag[i_val1l2] != STATOutputType_None) {

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

   // Only sanity check thresholds for thresholded line types
   if(output_flag[i_fho]  != STATOutputType_None ||
      output_flag[i_ctc]  != STATOutputType_None ||
      output_flag[i_cts]  != STATOutputType_None ||
      output_flag[i_mctc] != STATOutputType_None ||
      output_flag[i_mcts] != STATOutputType_None ||
      output_flag[i_pct]  != STATOutputType_None ||
      output_flag[i_pstd] != STATOutputType_None ||
      output_flag[i_pjc]  != STATOutputType_None ||
      output_flag[i_prc]) {

      // Loop over the fields to be verified
      for(i=0; i<n_vx; i++) {

         // Get the current dictionaries
         i_fcst_dict = parse_conf_i_vx_dict(fcst_dict, i);
         i_obs_dict  = parse_conf_i_vx_dict(obs_dict, i);
        
         // Conf: thresh
         fcst_ta[i] = i_fcst_dict.lookup_thresh_array(conf_key_cat_thresh);
         obs_ta[i]  = i_obs_dict.lookup_thresh_array(conf_key_cat_thresh);
         
         // Dump the contents of the current thresholds
         if(mlog.verbosity_level() >= 5) {
            mlog << Debug(5)
                 << "Parsed thresholds for forecast field number " << i+1 << ":\n";
            fcst_ta[i].dump(cout);
            mlog << Debug(5)
                 << "Parsed thresholds for observation field number " << i+1 << ":\n";
            obs_ta[i].dump(cout);
         }

         // Verifying a probability field
         if(vx_pd[i].fcst_info->p_flag() == 1) check_prob_thresh(fcst_ta[i]);
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

            mlog << Error << "\nPointStatConfInfo::process_config() -> "
                 << "The number of thresholds for each field in "
                 << "\"fcst.thresh\" must match the number of thresholds "
                 << "for each field in \"obs.thresh\".\n\n";
            exit(1);
         }

         // Verifying with multi-category contingency tables
         if(vx_pd[i].fcst_info->p_flag() == 0 &&
            (output_flag[i_mctc] != STATOutputType_None ||
             output_flag[i_mcts] != STATOutputType_None)) {

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

                  mlog << Error << "\nPointStatConfInfo::process_config() -> "
                       << "when verifying using multi-category contingency "
                       << "tables, the thresholds must be monotonically "
                       << "increasing and be of the same inequality type "
                       << "(<, <=, >, or >=).\n\n";
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

   // Conf: fcst.wind_thresh
   fcst_wind_ta = conf.lookup_thresh_array(conf_key_fcst_wind_thresh);

   // Conf: obs.wind_thresh
   obs_wind_ta = conf.lookup_thresh_array(conf_key_obs_wind_thresh);

   // Check that the number of wind speed thresholds match
   if(fcst_wind_ta.n_elements() != obs_wind_ta.n_elements()) {
      mlog << Error << "\nPointStatConfInfo::process_config() -> "
           << "The number of thresholds in \"fcst_wind_thresh\" must "
           << "match the number of thresholds in \"obs_wind_thresh\".\n\n";
      exit(1);
   }

   // Conf: ci_alpha
   ci_alpha = parse_conf_ci_alpha(&conf);
   
   // Conf: boot
   boot_info     = parse_conf_boot(&conf);
   boot_interval = boot_info.interval;
   boot_rep_prop = boot_info.rep_prop;
   n_boot_rep    = boot_info.n_rep;
   boot_rng      = boot_info.rng;
   boot_seed     = boot_info.seed;

   // Conf: interp
   interp_info   = parse_conf_interp(&conf);
   interp_thresh = interp_info.vld_thresh;
   n_interp      = interp_info.n_interp;
   interp_wdth   = interp_info.width;

   // Allocate memory to store the interpolation methods
   interp_mthd = new InterpMthd [n_interp];
   for(i=0; i<n_interp; i++)
      interp_mthd[i] = string_to_interpmthd(interp_info.method[i]);

   // Conf: duplicate_flag
   duplicate_flag = parse_conf_duplicate_flag(&conf);

   // Conf: rank_corr_flag
   rank_corr_flag = conf.lookup_bool(conf_key_rank_corr_flag);

   // Conf: tmp_dir
   tmp_dir = parse_conf_tmp_dir(&conf);

   // Conf: output_prefix
   output_prefix = conf.lookup_string(conf_key_output_prefix);

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::process_masks(const Grid &grid) {
   int i, j;
   StringArray mask_grid, mask_poly;
   ConcatString sid_file, s;

   // Retrieve the area masks
   mask_grid = conf.lookup_string_array(conf_key_mask_grid);
   mask_poly = conf.lookup_string_array(conf_key_mask_poly);
   n_mask_area = mask_grid.n_elements() + mask_poly.n_elements();

   // Retrieve the station masks
   sid_file = conf.lookup_string(conf_key_mask_sid);
   parse_sid_mask(sid_file, mask_sid);

   // Save the total number masks as a sum of the masking areas and
   // masking points
   n_mask = n_mask_area + mask_sid.n_elements();

   // Check that at least one verification masking region is provided
   if(n_mask == 0) {
      mlog << Error << "\nPointStatConfInfo::process_masks() -> "
           << "At least one grid, polyline, or station ID "
           << "masking region must be provided.\n\n";
      exit(1);
   }

   // Allocate space to store the masking information
   mask_dp = new DataPlane [n_mask_area];

   // Parse out the masking grid areas
   for(i=0; i<mask_grid.n_elements(); i++) {
      parse_grid_mask(mask_grid[i], grid, mask_dp[i], s);
      mask_name.add(s);
   }

   // Parse out the masking poly areas
   for(i=0,j=mask_grid.n_elements(); i<mask_poly.n_elements(); i++,j++) {
      parse_poly_mask(mask_poly[i], grid, mask_dp[j], s);
      mask_name.add(s);                    
   }

   // Store the masking station ID points
   for(i=0; i<mask_sid.n_elements(); i++) mask_name.add(mask_sid[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::set_vx_pd() {
   int i, j, n_msg_typ;

   // PairData is stored in the vx_pd objects in the following order:
   // [n_msg_typ][n_mask][n_interp]
   for(i=0; i<n_vx; i++) {

      // Get the message types for the current verification task
      n_msg_typ = get_n_msg_typ(i);
     
      // Set up the dimensions for the vx_pd object
      vx_pd[i].set_pd_size(n_msg_typ, n_mask, n_interp);

      // Add the verifying message type to the vx_pd objects
      for(j=0; j<n_msg_typ; j++)
         vx_pd[i].set_msg_typ(j, msg_typ[i][j]);

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

   //  set the duplicate flag for each pair data object
   for(j=0; j < n_vx; j++) vx_pd[j].set_duplicate_flag(duplicate_flag);

   return;
}

////////////////////////////////////////////////////////////////////////

int PointStatConfInfo::get_n_msg_typ(int i) const {

   if(i < 0 || i >= n_vx) {
      mlog << Error << "\nPointStatConfInfo::get_n_msg_typ(int i) -> "
           << "range check error for i = " << i << "\n\n";
      exit(1);
   }
   
   return(msg_typ[i].n_elements());
}
   
////////////////////////////////////////////////////////////////////////

int PointStatConfInfo::n_txt_row(int i_txt_row) {
   int i, n, max_n_msg_typ;

   // Determine the maximum number of message types being used
   for(i=0, max_n_msg_typ=0; i<n_vx; i++)
      if(get_n_msg_typ(i) > max_n_msg_typ)
         max_n_msg_typ = get_n_msg_typ(i);

   // Switch on the index of the line type
   switch(i_txt_row) {

      case(i_fho):
         // Maximum number of FHO lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds
         n = n_vx_scal * max_n_msg_typ * n_mask * n_interp *
             max_n_scal_thresh;
         break;

      case(i_ctc):
         // Maximum number of CTC lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds
         n = n_vx_scal * max_n_msg_typ * n_mask * n_interp *
             max_n_scal_thresh;
         break;

      case(i_cts):
         // Maximum number of CTS lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds * Alphas
         n = n_vx_scal * max_n_msg_typ * n_mask * n_interp *
             max_n_scal_thresh * get_n_ci_alpha();
         break;

      case(i_mctc):
         // Maximum number of MCTC lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods
         n = n_vx_scal * max_n_msg_typ * n_mask * n_interp;
         break;

      case(i_mcts):
         // Maximum number of MCTS lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Alphas
         n = n_vx_scal * max_n_msg_typ * n_mask * n_interp *
             get_n_ci_alpha();
         break;

      case(i_cnt):
         // Maximum number of CNT lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Alphas
         n = n_vx_scal * max_n_msg_typ * n_mask * n_interp * get_n_ci_alpha();
         break;

      case(i_sl1l2):
         // Maximum number of SL1L2 lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods
         n = n_vx_scal * max_n_msg_typ * n_mask * n_interp;
         break;

      case(i_sal1l2):
         // Maximum number of SAL1L2 lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods
         n = n_vx_scal * max_n_msg_typ * n_mask * n_interp;
         break;

      case(i_vl1l2):
         // Maximum number of VL1L2 lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Wind Thresholds
         n = n_vx_vect * max_n_msg_typ * n_mask * n_interp * get_n_wind_thresh();
         break;

      case(i_val1l2):
         // Maximum number of VAL1L2 lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Wind Thresholds
         n = n_vx_vect * max_n_msg_typ * n_mask * n_interp * get_n_wind_thresh();
         break;

      case(i_pct):
         // Maximum number of PCT lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds
         n = n_vx_prob * max_n_msg_typ * n_mask * n_interp *
             max_n_prob_obs_thresh;
         break;

      case(i_pstd):
         // Maximum number of PSTD lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds * Alphas
         n = n_vx_prob * max_n_msg_typ * n_mask * n_interp *
             max_n_prob_obs_thresh * get_n_ci_alpha();
         break;

      case(i_pjc):
         // Maximum number of PJC lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds
         n = n_vx_prob * max_n_msg_typ * n_mask * n_interp *
             max_n_prob_obs_thresh;
         break;

      case(i_prc):
         // Maximum number of PRC lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds
         n = n_vx_prob * max_n_msg_typ * n_mask * n_interp *
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
         mlog << Error << "\nPointStatConfInfo::n_txt_row(int) -> "
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

      if(output_flag[i] != STATOutputType_None) n += n_txt_row(i);
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////
