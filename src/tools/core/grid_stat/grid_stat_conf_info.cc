// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <cmath>

#include "grid_stat_conf_info.h"

#include "vx_data2d_factory.h"
#include "vx_log.h"

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
   fcst_info   = (VarInfo **)     0;
   obs_info    = (VarInfo **)     0;
   fcst_ta     = (ThreshArray *)  0;
   obs_ta      = (ThreshArray *)  0;
   mask_dp     = (DataPlane *)    0;   
   interp_mthd = (InterpMthd *)   0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GridStatConfInfo::clear() {
   int i;

   // Set counts to zero
   n_vx_scal     = 0;
   n_vx_vect     = 0;
   n_vx_prob     = 0;
   n_mask        = 0;
   n_interp      = 0;

   max_n_scal_thresh      = 0;
   max_n_prob_fcst_thresh = 0;
   max_n_prob_obs_thresh  = 0;

   // Initialize values
   model.clear();
   fcst_wind_ta.clear();
   obs_wind_ta.clear();
   mask_name.clear();
   ci_alpha.clear();
   boot_interval = BootIntervalType_None;
   boot_rep_prop = bad_data_double;
   n_boot_rep = bad_data_int;
   boot_rng.clear();
   boot_seed.clear();
   interp_field = FieldType_None;
   interp_thresh = bad_data_double;
   interp_wdth.clear();
   nc_pairs_flag = false;
   rank_corr_flag = false;
   tmp_dir.clear();
   output_prefix.clear();
   version.clear();

   for(i=0; i<n_txt; i++) output_flag[i] = STATOutputType_None;
   
   // Deallocate memory
   if(fcst_ta)     { delete [] fcst_ta;     fcst_ta     = (ThreshArray *)  0; }
   if(obs_ta)      { delete [] obs_ta;      obs_ta      = (ThreshArray *)  0; }
   if(mask_dp)     { delete [] mask_dp;     mask_dp     = (DataPlane *)    0; }   
   if(interp_mthd) { delete [] interp_mthd; interp_mthd = (InterpMthd *)   0; }

   // Clear fcst_info
   if(fcst_info) {
      for(i=0; i<n_vx; i++)
         if(fcst_info[i]) { delete fcst_info[i]; fcst_info[i] = (VarInfo *) 0; }
      delete fcst_info; fcst_info = (VarInfo **) 0;
   }

   // Clear obs_info
   if(obs_info) {
      for(i=0; i<n_vx; i++)
         if(obs_info[i]) { delete obs_info[i]; obs_info[i] = (VarInfo *) 0; }
      delete obs_info; obs_info = (VarInfo **) 0;
   }

   // Reset count
   n_vx = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void GridStatConfInfo::read_config(const char *default_file_name,
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

void GridStatConfInfo::process_config(GrdFileType ftype,
                                      GrdFileType otype) {
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
   NbrhdInfo nbrhd_info;
   
   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Initialize
   clear();

   // Conf: version
   version = parse_conf_version(&conf);

   // Conf: model
   model = parse_conf_model(&conf);

   // Conf: output_flag
   output_map = parse_conf_output_flag(&conf);

   // Make sure the output_flag is the expected size
   if((signed int) output_map.size() != n_txt) {
      mlog << Error << "\nGridStatConfInfo::process_config() -> "
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
      mlog << Error << "\nGridStatConfInfo::process_config() -> "
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
      mlog << Error << "\nGridStatConfInfo::process_config() -> "
           << "The number of verification tasks in \""
           << conf_key_obs_field
           << "\" must be non-zero and match the number in \""
           << conf_key_fcst_field << "\".\n\n";
      exit(1);
   }

   // Allocate space based on the number of verification tasks
   fcst_info = new VarInfo *   [n_vx];
   obs_info  = new VarInfo *   [n_vx];
   fcst_ta   = new ThreshArray [n_vx];
   obs_ta    = new ThreshArray [n_vx];

   // Parse the fcst and obs field information
   for(i=0; i<n_vx; i++) {
    
      // Allocate new VarInfo objects
      fcst_info[i] = info_factory.new_var_info(ftype);
      obs_info[i]  = info_factory.new_var_info(otype);

      // Get the current dictionaries
      i_fcst_dict = parse_conf_i_vx_dict(fcst_dict, i);
      i_obs_dict  = parse_conf_i_vx_dict(obs_dict, i);

      // Set the current dictionaries
      fcst_info[i]->set_dict(i_fcst_dict);
      obs_info[i]->set_dict(i_obs_dict);

      // Dump the contents of the current VarInfo
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed forecast field number " << i+1 << ":\n";
         fcst_info[i]->dump(cout);
         mlog << Debug(5)
              << "Parsed observation field number " << i+1 << ":\n";
         obs_info[i]->dump(cout);
      }

      // No support for wind direction
      if(fcst_info[i]->is_wind_direction() ||
         obs_info[i]->is_wind_direction()) {
         mlog << Error << "\nGridStatConfInfo::process_config() -> "
              << "the wind direction field may not be verified "
              << "using grid_stat.\n\n";
         exit(1);
      }

      // Check that the observation field does not contain probabilities
      if(obs_info[i]->p_flag()) {
         mlog << Error << "\nGridStatConfInfo::process_config() -> "
              << "The observation field cannot contain probabilities.\n\n";
         exit(1);
      }
   } // end for i   

   // If VL1L2 is requested, check the specified fields and turn
   // on the vflag when UGRD is followed by VGRD at the same level
   if(output_flag[i_vl1l2] != STATOutputType_None) {

      for(i=0, n_vx_vect = 0; i<n_vx; i++) {

         if(i+1 < n_vx                  &&
            fcst_info[i]->is_u_wind()   &&
            obs_info[i]->is_u_wind()    &&
            fcst_info[i+1]->is_v_wind() &&
            obs_info[i+1]->is_v_wind()  &&
            fcst_info[i]->req_level_name() == fcst_info[i+1]->req_level_name() &&
            obs_info[i]->req_level_name()  == obs_info[i+1]->req_level_name()) {

            fcst_info[i]->set_v_flag(true);
            obs_info[i]->set_v_flag(true);
            fcst_info[i+1]->set_v_flag(true);
            obs_info[i+1]->set_v_flag(true);

            // Increment the number of vector fields to be verified
            n_vx_vect++;
         }
      } // end for
   } // end if

   // Compute the number of scalar and probability fields to be verified.
   for(i=0, n_vx_prob = 0, n_vx_scal = 0; i<n_vx; i++) {
      if(fcst_info[i]->p_flag()) n_vx_prob++;
      else                       n_vx_scal++;
   }

   // Only sanity check thresholds for thresholded line types
   if(output_flag[i_fho]    != STATOutputType_None ||
      output_flag[i_ctc]    != STATOutputType_None ||
      output_flag[i_cts]    != STATOutputType_None ||
      output_flag[i_mctc]   != STATOutputType_None ||
      output_flag[i_mcts]   != STATOutputType_None ||
      output_flag[i_pct]    != STATOutputType_None ||
      output_flag[i_pstd]   != STATOutputType_None ||
      output_flag[i_pjc]    != STATOutputType_None ||
      output_flag[i_prc]    != STATOutputType_None ||
      output_flag[i_nbrctc] != STATOutputType_None ||
      output_flag[i_nbrcts] != STATOutputType_None ||
      output_flag[i_nbrcnt] != STATOutputType_None) {

      // Loop over the fields to be verified
      for(i=0; i<n_vx; i++) {

         // Get the current dictionaries
         i_fcst_dict = parse_conf_i_vx_dict(fcst_dict, i);
         i_obs_dict  = parse_conf_i_vx_dict(obs_dict, i);

         // Conf: cat_thresh
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
         if(fcst_info[i]->p_flag() == 1) check_prob_thresh(fcst_ta[i]);
      }

      // Check that number of thresholds specified for each field is the
      // same and compute the maximum number of thresholds
      max_n_scal_thresh      = 0;
      max_n_prob_fcst_thresh = 0;
      max_n_prob_obs_thresh  = 0;
      for(i=0; i<n_vx; i++) {

         // Only check for non-probability fields
         if(!fcst_info[i]->p_flag() &&
            fcst_ta[i].n_elements() != obs_ta[i].n_elements()) {

            mlog << Error << "\nGridStatConfInfo::process_config() -> "
                 << "The number of thresholds for each field in \"fcst."
                 << conf_key_cat_thresh
                 << "\" must match the number of thresholds for each "
                 << "field in \"obs." << conf_key_cat_thresh << "\".\n\n";
            exit(1);
         }

         // Verifying with multi-category contingency tables
         if(!fcst_info[i]->p_flag() &&
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

                  mlog << Error << "\nGridStatConfInfo::process_config() -> "
                       << "when verifying using multi-category contingency "
                       << "tables, the thresholds must be monotonically "
                       << "increasing and be of the same inequality type "
                       << "(lt, le, gt, or ge).\n\n";
                  exit(1);
               }
            }
         }

         // Look for the maximum number of thresholds for scalar fields
         if(!fcst_info[i]->p_flag()) {

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
      mlog << Error << "\nGridStatConfInfo::process_config() -> "
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
   interp_field  = interp_info.field;
   interp_thresh = interp_info.vld_thresh;
   n_interp      = interp_info.n_interp;
   interp_wdth   = interp_info.width;

   // Allocate memory to store the interpolation methods
   interp_mthd = new InterpMthd [n_interp];
   for(i=0; i<n_interp; i++) {
      interp_mthd[i] = string_to_interpmthd(interp_info.method[i]);

      // Check for DW_MEAN, LS_FIT, and BILIN
      if(interp_mthd[i] == InterpMthd_DW_Mean ||
         interp_mthd[i] == InterpMthd_LS_Fit  ||
         interp_mthd[i] == InterpMthd_Bilin) {

         mlog << Error << "\nGridStatConfInfo::process_config() -> "
              << "The interpolation method may not be set to DW_MEAN, "
              << "LS_FIT, or BILIN for grid_stat.\n\n";
         exit(1);
      }

      // Check for valid interpolation width
      if(interp_wdth[i] < 1 || interp_wdth[i]%2 == 0) {

         mlog << Error << "\nGridStatConfInfo::process_config() -> "
              << "The interpolation width must be set to odd values "
              << "greater than or equal to 1 ("
              << interp_wdth[i] << ").\n\n";
         exit(1);
      }
   } // end for i

   // Conf: nbrhd
   nbrhd_info   = parse_conf_nbrhd(&conf);
   nbrhd_thresh = nbrhd_info.vld_thresh;
   nbrhd_wdth   = nbrhd_info.width;
   nbrhd_cov_ta = nbrhd_info.cov_ta;

   // Conf: nc_pairs_flag
   nc_pairs_flag = conf.lookup_bool(conf_key_nc_pairs_flag);

   // Conf: rank_corr_flag
   rank_corr_flag = conf.lookup_bool(conf_key_rank_corr_flag);

   // Conf: tmp_dir
   tmp_dir = parse_conf_tmp_dir(&conf);

   // Conf: output_prefix
   output_prefix = conf.lookup_string(conf_key_output_prefix);

   return;
}

////////////////////////////////////////////////////////////////////////

void GridStatConfInfo::process_masks(const Grid &grid) {
   int i;
   StringArray mask_grid, mask_poly;
   ConcatString name;

   // Retrieve the mask names
   mask_grid = conf.lookup_string_array(conf_key_mask_grid);
   mask_poly = conf.lookup_string_array(conf_key_mask_poly);

   // Get the number of masks
   n_mask = mask_grid.n_elements() + mask_poly.n_elements();

   // Check that at least one verification masking region is provided
   if(n_mask == 0) {

      mlog << Error << "\nGridStatConfInfo::process_masks() -> "
           << "At least one grid or polyline verification masking "
           << "region must be provided.\n\n";
      exit(1);
   }

   // Allocate space to store the masking information
   mask_dp = new DataPlane [n_mask];

   // Parse out the masking grids
   for(i=0; i<mask_grid.n_elements(); i++) {
      parse_grid_mask(mask_grid[i], grid, mask_dp[i], name);
      mask_name.add(name);
   }

   // Parse out the masking polylines
   for(i=0; i<mask_poly.n_elements(); i++) {
      parse_poly_mask(mask_poly[i], grid,
                      mask_dp[i+mask_grid.n_elements()], name);
      mask_name.add(name);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

int GridStatConfInfo::n_txt_row(int i_txt_row) {
   int n;

   // Switch on the index of the line type
   switch(i_txt_row) {

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
             get_n_ci_alpha();
         break;

      case(i_mctc):
         // Maximum number of CTC lines possible =
         //    Fields * Masks * Smoothing Methods
         n = n_vx_scal * n_mask * n_interp;
         break;

      case(i_mcts):
         // Maximum number of CTS lines possible =
         //    Fields * Masks * Smoothing Methods * Alphas
         n = n_vx_scal * n_mask * n_interp * get_n_ci_alpha();
         break;

      case(i_cnt):
         // Maximum number of CNT lines possible =
         //    Fields * Masks * Smoothing Methods * Alphas
         n = n_vx_scal * n_mask * n_interp * get_n_ci_alpha();
         break;

      case(i_sl1l2):
         // Maximum number of SL1L2 lines possible =
         //    Fields * Masks * Smoothing Methods
         n = n_vx_scal * n_mask * n_interp;
         break;

      case(i_vl1l2):
         // Maximum number of VL1L2 lines possible =
         //    Fields * Masks * Smoothing Methods * Wind Thresholds
         n = n_vx_vect * n_mask * n_interp *
             get_n_wind_thresh();
         break;

      case(i_nbrctc):
         // Maximum number of NBRCTC lines possible =
         //    Fields * Masks * Max Thresholds *
         //    Neighborhoods * Frac Thresholds
         n = n_vx_scal * n_mask * max_n_scal_thresh *
             get_n_nbrhd_wdth() * get_n_cov_thresh();
         break;

      case(i_nbrcts):
         // Maximum number of NBRCTS lines possible =
         //    Fields * Masks * Max Thresholds *
         //    Neighborhoods * Frac Thresholds * Alphas
         n = n_vx_scal * n_mask * max_n_scal_thresh *
             get_n_nbrhd_wdth() * get_n_cov_thresh() *
             get_n_ci_alpha();
         break;

      case(i_nbrcnt):
         // Maximum number of NBRCTS lines possible =
         //    Fields * Masks * Max Thresholds *
         //    Neighborhoods * Frac Thresholds * Alphas
         n = n_vx_scal * n_mask * max_n_scal_thresh *
             get_n_nbrhd_wdth() * get_n_cov_thresh() *
             get_n_ci_alpha();
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
             get_n_ci_alpha();
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
         mlog << Error << "\nGridStatConfInfo::n_txt_row(int) -> "
              << "unexpected output type index value: " << i_txt_row
              << "\n\n";
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

      if(output_flag[i] != STATOutputType_None) n += n_txt_row(i);
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////
