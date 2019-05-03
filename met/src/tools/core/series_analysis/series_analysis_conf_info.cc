// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
   desc.clear();
   obtype.clear();
   fcat_ta.clear();
   ocat_ta.clear();
   fcnt_ta.clear();
   ocnt_ta.clear();
   cnt_logic = SetLogic_None;
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
   mask_area.clear();
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
   conf.read(replace_path(config_const_filename).c_str());

   // Read the default config file
   conf.read(default_file_name);

   // Read the user-specified config file
   conf.read(user_file_name);

   return;
}

////////////////////////////////////////////////////////////////////////

void SeriesAnalysisConfInfo::process_config(GrdFileType ftype,
                                            GrdFileType otype) {
   int i, n;
   ConcatString s;
   StringArray sa;
   VarInfoFactory info_factory;
   Dictionary *fdict = (Dictionary *) 0;
   Dictionary *odict = (Dictionary *) 0;
   Dictionary i_fdict, i_odict;
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

   // Conf: desc
   desc = parse_conf_string(&conf, conf_key_desc);

   // Conf: obtype
   obtype = parse_conf_string(&conf, conf_key_obtype);

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
   fdict = conf.lookup_array(conf_key_fcst_field);
   odict = conf.lookup_array(conf_key_obs_field);

   // Determine the number of fields (name/level) to be verified
   n_fcst = parse_conf_n_vx(fdict);
   n_obs  = parse_conf_n_vx(odict);

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

   // Check climatology fields
   check_climo_n_vx(&conf, n_fcst);

   // Allocate space based on the number of verification tasks
   fcst_info = new VarInfo * [n_fcst];
   obs_info  = new VarInfo * [n_obs];

   // Initialize pointers
   for(i=0; i<n_fcst; i++) fcst_info[i] = (VarInfo *) 0;
   for(i=0; i<n_obs;  i++) obs_info[i]  = (VarInfo *) 0;

   // Parse the fcst field information
   for(i=0; i<n_fcst; i++) {

      // Allocate new VarInfo objects
      fcst_info[i] = info_factory.new_var_info(ftype);

      // Get the current dictionaries
      i_fdict = parse_conf_i_vx_dict(fdict, i);

      // Set the current dictionaries
      fcst_info[i]->set_dict(i_fdict);

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
      i_odict = parse_conf_i_vx_dict(odict, i);

      // Set the current dictionaries
      obs_info[i]->set_dict(i_odict);

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
      if(obs_info[i]->is_prob()) {
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

   if(vld_data_thresh < 0.0 || vld_data_thresh > 1.0) {
      mlog << Error << "\nSeriesAnalysisConfInfo::process_config() -> "
           << "The \"" << conf_key_vld_thresh << "\" parameter ("
           << vld_data_thresh << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   // Sanity check categorical thresholds
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
      fcat_ta = fdict->lookup_thresh_array(conf_key_cat_thresh);

      // Conf: obs.cat_thresh
      ocat_ta = odict->lookup_thresh_array(conf_key_cat_thresh);

      mlog << Debug(5)
           << "Parsed forecast categorical thresholds: "
           << fcat_ta.get_str() << "\n"
           << "Parsed observed categorical thresholds: "
           << ocat_ta.get_str() << "\n";

      // Verifying a probability field
      if(fcst_info[0]->is_prob()) {
         fcat_ta = string_to_prob_thresh(fcat_ta.get_str().c_str());
      }

      // Verifying non-probability fields
      if(!fcst_info[0]->is_prob() &&
         fcat_ta.n_elements() != ocat_ta.n_elements()) {

         mlog << Error << "\nSeriesAnalysisConfInfo::process_config() -> "
              << "The number of thresholds in \"fcst.cat_thresh\" must "
              << "match the number of thresholds in \"obs.cat_thresh\".\n\n";
         exit(1);
      }

      // Verifying with multi-category contingency tables
      if(!fcst_info[0]->is_prob() &&
         (output_stats[stat_mctc].n_elements() > 0 ||
          output_stats[stat_mcts].n_elements() > 0)) {
         check_mctc_thresh(fcat_ta);
         check_mctc_thresh(ocat_ta);
      }
   } // end if categorical

   // Sanity check continuous thresholds
   if((output_stats[stat_sl1l2].n_elements()  +
       output_stats[stat_sal1l2].n_elements() +
       output_stats[stat_cnt].n_elements()) > 0) {

      // Conf: fcst.cnt_thresh
      fcnt_ta = fdict->lookup_thresh_array(conf_key_cnt_thresh);

      // Conf: obs.cnt_thresh
      ocnt_ta = odict->lookup_thresh_array(conf_key_cnt_thresh);

      // Conf: cnt_logic
      cnt_logic = check_setlogic(
                     int_to_setlogic(i_fdict.lookup_int(conf_key_cnt_logic)),
                     int_to_setlogic(i_odict.lookup_int(conf_key_cnt_logic)));

      mlog << Debug(5)
           << "Parsed forecast continuous thresholds: "  << fcnt_ta.get_str() << "\n"
           << "Parsed observed continuous thresholds: "  << ocnt_ta.get_str() << "\n"
           << "Parsed continuous threshold logic: "       << setlogic_to_string(cnt_logic) << "\n";

      // Add default continuous thresholds until the counts match
      n = max(fcnt_ta.n_elements(), ocnt_ta.n_elements());
      while(fcnt_ta.n_elements() < n) fcnt_ta.add(na_str);
      while(ocnt_ta.n_elements() < n) ocnt_ta.add(na_str);

   } // end if continuous

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
   MaskPlane mask_grid, mask_poly;
   ConcatString name;

   mlog << Debug(2)
        << "Processing masking regions.\n";

   // Initialize the mask to all points on
   mask_area.set_size(grid.nx(), grid.ny(), true);

   // Conf: mask.grid
   mask_grid_file = conf.lookup_string(conf_key_mask_grid);

   // Conf: mask.poly
   mask_poly_file = conf.lookup_string(conf_key_mask_poly);

   // Parse out the masking grid
   if(mask_grid_file.length() > 0) {
      mlog << Debug(3)
           << "Processing grid mask: " << mask_grid_file << "\n";
      parse_grid_mask(mask_grid_file, grid, mask_grid, mask_grid_name);
      apply_mask(mask_area, mask_grid);
   }

   // Parse out the masking polyline
   if(mask_poly_file.length() > 0) {
      mlog << Debug(3)
           << "Processing poly mask: " << mask_poly_file << "\n";
      parse_poly_mask(mask_poly_file, grid, mask_poly, mask_poly_name);
      apply_mask(mask_area, mask_poly);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
