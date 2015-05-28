// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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

#include "series_analysis_conf_info.h"

#include "vx_data2d_factory.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class SeriesAnalysisConfInfo
//
////////////////////////////////////////////////////////////////////////

SeriesAnalysisConfInfo::SeriesAnalysisConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

SeriesAnalysisConfInfo::~SeriesAnalysisConfInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void SeriesAnalysisConfInfo::init_from_scratch() {

   // Initialize pointers
   fcst_info = (VarInfo **)    0;
   obs_info  = (VarInfo **)    0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void SeriesAnalysisConfInfo::clear() {
   int i;

   // Initialize values
   model.clear();
   obtype.clear();
   regrid_info.clear();
   fcst_cat_ta.clear();
   obs_cat_ta.clear();
   ci_alpha.clear();
   boot_interval = BootIntervalType_None;
   boot_rep_prop = bad_data_double;
   n_boot_rep = bad_data_int;
   boot_rng.clear();
   boot_seed.clear();
   mask_grid_file.clear();
   mask_grid_name.clear();
   mask_poly_file.clear();
   mask_poly_name.clear();
   mask_dp.clear();
   block_size = bad_data_int;
   vld_data_thresh = bad_data_double;
   rank_corr_flag = false;
   tmp_dir.clear();
   version.clear();

   output_stats.clear();

   // Clear fcst_info
   if(fcst_info) {
      for(i=0; i<n_fcst; i++)
         if(fcst_info[i]) { delete fcst_info[i]; fcst_info[i] = (VarInfo *) 0; }
      delete fcst_info; fcst_info = (VarInfo **) 0;
   }

   // Clear obs_info
   if(obs_info) {
      for(i=0; i<n_obs; i++)
         if(obs_info[i]) { delete obs_info[i]; obs_info[i] = (VarInfo *) 0; }
      delete obs_info; obs_info = (VarInfo **) 0;
   }

   // Reset counts
   n_fcst = 0;
   n_obs  = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void SeriesAnalysisConfInfo::read_config(const char *default_file_name,
                                   const char *user_file_name) {

   // Read the config file constants
   conf.read(replace_path(config_const_filename));

   // Read the default config file
   conf.read(default_file_name);

   // Read the user-specified config file
   conf.read(user_file_name);

   return;
}

// ////////////////////////////////////////////////////////////////////////

void SeriesAnalysisConfInfo::process_config(GrdFileType ftype,
                                            GrdFileType otype) {
   int i;
   ConcatString s;
   StringArray sa;
   VarInfoFactory info_factory;
   Dictionary *fcst_dict = (Dictionary *) 0;
   Dictionary *obs_dict  = (Dictionary *) 0;
   Dictionary i_fcst_dict, i_obs_dict;
   BootInfo boot_info;
   map<STATLineType,StringArray>::iterator it;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Initialize
   clear();

   // Conf: version
   version = parse_conf_version(&conf);

   // Conf: model
   model = parse_conf_string(&conf, conf_key_model);

   // Conf: obtype
   obtype = parse_conf_string(&conf, conf_key_obtype);

   // Conf: regrid
   regrid_info = parse_conf_regrid(&conf);

   // Conf: output_stats
   output_stats = parse_conf_output_stats(&conf);

   // Count the number of statistics requested
   for(i = 0, it = output_stats.begin(); it != output_stats.end(); it++) {
      i += it->second.n_elements();
   } // end for it

   // Check for at least one output statistics
   if(i == 0) {
      mlog << Error << "\nSeriesAnalysisConfInfo::process_config() -> "
           << "At least one output statistic must be requested.\n\n";
      exit(1);
   }

   // Conf: fcst.field and obs.field
   fcst_dict = conf.lookup_array(conf_key_fcst_field);
   obs_dict  = conf.lookup_array(conf_key_obs_field);

   // Determine the number of fields (name/level) to be verified
   n_fcst = parse_conf_n_vx(fcst_dict);
   n_obs  = parse_conf_n_vx(obs_dict);

   // Check for empty fcst and obs settings
   if(n_fcst == 0 || n_obs == 0) {
      mlog << Error << "\nSeriesAnalysisConfInfo::process_config() -> "
           << "the \"fcst\" and \"obs\" settings may not be empty.\n\n";
      exit(1);
   }

   // Check for matching lengths
   if(n_fcst > 1 && n_obs > 1 && n_fcst != n_obs) {
      mlog << Error << "\nSeriesAnalysisConfInfo::process_config() -> "
           << "if the \"fcst.field\" and \"obs.field\" settings "
           << "have length greater than 1, they must be equal.\n\n";
      exit(1);
   }

   // Allocate space based on the number of verification tasks
   fcst_info = new VarInfo * [n_fcst];
   obs_info  = new VarInfo * [n_obs];

   // Initialize pointers
   for(i=0; i<n_fcst; i++) fcst_info[i] = obs_info[i] = (VarInfo *) 0;

   // Parse the fcst field information
   for(i=0; i<n_fcst; i++) {
    
      // Allocate new VarInfo objects
      fcst_info[i] = info_factory.new_var_info(ftype);

      // Get the current dictionaries
      i_fcst_dict = parse_conf_i_vx_dict(fcst_dict, i);

      // Set the current dictionaries
      fcst_info[i]->set_dict(i_fcst_dict);

      // Dump the contents of the current VarInfo
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed forecast field number " << i+1 << ":\n";
         fcst_info[i]->dump(cout);
      }

      // No support for wind direction
      if(fcst_info[i]->is_wind_direction()) {
         mlog << Error << "\nSeriesAnalysisConfInfo::process_config() -> "
              << "the wind direction field may not be verified "
              << "using series_analysis.\n\n";
         exit(1);
      }
   } // end for i

   // Parse the obs field information
   for(i=0; i<n_obs; i++) {

      // Allocate new VarInfo objects
      obs_info[i] = info_factory.new_var_info(otype);

      // Get the current dictionaries
      i_obs_dict = parse_conf_i_vx_dict(obs_dict, i);

      // Set the current dictionaries
      obs_info[i]->set_dict(i_obs_dict);

      // Dump the contents of the current VarInfo
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed observation field number " << i+1 << ":\n";
         obs_info[i]->dump(cout);
      }

      // No support for wind direction
      if(obs_info[i]->is_wind_direction()) {
         mlog << Error << "\nSeriesAnalysisConfInfo::process_config() -> "
              << "the wind direction field may not be verified "
              << "using series_analysis.\n\n";
         exit(1);
      }

      // Check that the observation field does not contain probabilities
      if(obs_info[i]->p_flag()) {
         mlog << Error << "\nSeriesAnalysisConfInfo::process_config() -> "
              << "The observation field cannot contain probabilities.\n\n";
         exit(1);
      }
   } // end for i

   // Conf: block_size
   block_size = conf.lookup_int(conf_key_block_size);
   
   if(block_size <= 0.0) {
      mlog << Error << "\nSeriesAnalysisConfInfo::process_config() -> "
           << "The \"" << conf_key_block_size << "\" parameter ("
           << block_size << ") must be greater than 0.\n\n";
      exit(1);
   }

   // Conf: vld_thresh
   vld_data_thresh = conf.lookup_double(conf_key_vld_thresh);

   if(vld_data_thresh <= 0.0 || vld_data_thresh > 1.0) {
      mlog << Error << "\nSeriesAnalysisConfInfo::process_config() -> "
           << "The \"" << conf_key_vld_thresh << "\" parameter ("
           << vld_data_thresh << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   // Only sanity check thresholds for thresholded statistics
   if((output_stats[stat_fho].n_elements()  +
       output_stats[stat_ctc].n_elements()  +
       output_stats[stat_cts].n_elements()  +
       output_stats[stat_mctc].n_elements() +
       output_stats[stat_mcts].n_elements() +
       output_stats[stat_pct].n_elements()  +
       output_stats[stat_pstd].n_elements() +
       output_stats[stat_pjc].n_elements()  +
       output_stats[stat_prc].n_elements()) > 0) {

      // Conf: fcst.cat_thresh
      fcst_cat_ta = fcst_dict->lookup_thresh_array(conf_key_cat_thresh);

      // Conf: obs.cat_thresh
      obs_cat_ta = obs_dict->lookup_thresh_array(conf_key_cat_thresh);

      // Dump the contents of the current thresholds
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed forecast thresholds:\n";
              fcst_cat_ta.dump(cout);
         mlog << Debug(5)
              << "Parsed observation thresholds:\n";
              obs_cat_ta.dump(cout);
      }

      // Verifying a probability field
      if(fcst_info[0]->p_flag()) check_prob_thresh(fcst_cat_ta);

   } // end if categorical

   // Verifying non-probability fields
   if(!fcst_info[0]->p_flag() &&
      fcst_cat_ta.n_elements() != obs_cat_ta.n_elements()) {

      mlog << Error << "\nSeriesAnalysisConfInfo::process_config() -> "
           << "The number of thresholds in \"fcst.cat_thresh\" must "
           << "match the number of thresholds in \"obs.cat_thresh\".\n\n";
      exit(1);
   }

   // Verifying with multi-category contingency tables
   if(!fcst_info[0]->p_flag() &&
      (output_stats[stat_mctc].n_elements() > 0 ||
       output_stats[stat_mcts].n_elements() > 0)) {

      // Check that the threshold values are monotonically increasing
      // and the threshold types are inequalities that remain the same
      for(i=0; i<fcst_cat_ta.n_elements()-1; i++) {

         if(fcst_cat_ta[i].thresh >  fcst_cat_ta[i+1].thresh ||
            obs_cat_ta[i].thresh  >  obs_cat_ta[i+1].thresh  ||
            fcst_cat_ta[i].get_type()   != fcst_cat_ta[i+1].get_type()   ||
            obs_cat_ta[i].get_type()    != obs_cat_ta[i+1].get_type()    ||
            fcst_cat_ta[i].get_type()   == thresh_eq           ||
            fcst_cat_ta[i].get_type()   == thresh_ne           ||
            obs_cat_ta[i].get_type()    == thresh_eq           ||
            obs_cat_ta[i].get_type()    == thresh_ne) {

            mlog << Error << "\nSeriesAnalysisConfInfo::process_config() -> "
                 << "when verifying using multi-category contingency "
                 << "tables, the thresholds must be monotonically "
                 << "increasing and be of the same inequality type "
                 << "(lt, le, gt, or ge).\n\n";
            exit(1);
         }
      } // end for i
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

   // Conf: rank_corr_flag
   rank_corr_flag = conf.lookup_bool(conf_key_rank_corr_flag);

   // Conf: tmp_dir
   tmp_dir = parse_conf_tmp_dir(&conf);

   return;
}

////////////////////////////////////////////////////////////////////////

void SeriesAnalysisConfInfo::process_masks(const Grid &grid) {
   DataPlane mask_grid_dp, mask_poly_dp;
   ConcatString name;

   // Initialize the mask to all points on
   mask_dp.set_size(grid.nx(), grid.ny());
   mask_dp.set_constant(mask_on_value);
   
   // Conf: mask.grid
   mask_grid_file = conf.lookup_string(conf_key_mask_grid);

   // Conf: mask.poly
   mask_poly_file = conf.lookup_string(conf_key_mask_poly);

   // Parse out the masking grid
   if(mask_grid_file.length() > 0) {
      parse_grid_mask(mask_grid_file, grid, mask_grid_dp, mask_grid_name);
      apply_mask(mask_dp, mask_grid_dp);
   }

   // Parse out the masking polyline
   if(mask_poly_file.length() > 0) {
      parse_poly_mask(mask_poly_file, grid, mask_poly_dp, mask_poly_name);
      apply_mask(mask_dp, mask_poly_dp);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
