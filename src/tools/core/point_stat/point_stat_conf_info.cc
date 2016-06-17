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
   fcat_ta     = (ThreshArray *)     0;
   ocat_ta     = (ThreshArray *)     0;
   fcnt_ta     = (ThreshArray *)     0;
   ocnt_ta     = (ThreshArray *)     0;
   cnt_logic   = (SetLogic *)        0;
   fwind_ta    = (ThreshArray *)     0;
   owind_ta    = (ThreshArray *)     0;
   wind_logic  = (SetLogic *)        0;
   msg_typ     = (StringArray *)     0;
   sid_exc     = (StringArray *)     0;
   obs_qty     = (StringArray *)     0;
   mask_dp     = (DataPlane *)       0;
   interp_mthd = (InterpMthd *)      0;
   vx_pd       = (VxPairDataPoint *) 0;
   
   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::clear() {
   int i;

   // Set counts to zero
   n_vx        = 0;
   n_vx_scal   = 0;
   n_vx_vect   = 0;
   n_vx_prob   = 0;
   n_mask      = 0;
   n_mask_area = 0;
   n_mask_sid  = 0;
   n_interp    = 0;

   max_n_cat_thresh   = 0;
   max_n_cnt_thresh   = 0;
   max_n_wind_thresh  = 0;
   max_n_fprob_thresh = 0;
   max_n_oprob_thresh = 0;

   // Initialize values
   model.clear();
   regrid_info.clear();
   beg_ds = end_ds = bad_data_int;
   mask_name.clear();
   ci_alpha.clear();
   boot_interval = BootIntervalType_None;
   boot_rep_prop = bad_data_double;
   n_boot_rep = bad_data_int;
   boot_rng.clear();
   boot_seed.clear();
   interp_thresh = bad_data_double;
   interp_wdth.clear();
   duplicate_flag = DuplicateType_None;   
   rank_corr_flag = false;
   tmp_dir.clear();
   output_prefix.clear();
   version.clear();

   for(i=0; i<n_txt; i++) output_flag[i] = STATOutputType_None;

   // Deallocate memory
   if(fcat_ta)     { delete [] fcat_ta;     fcat_ta     = (ThreshArray *)      0; }
   if(ocat_ta)     { delete [] ocat_ta;     ocat_ta     = (ThreshArray *)      0; }
   if(fcnt_ta)     { delete [] fcnt_ta;     fcnt_ta     = (ThreshArray *)      0; }
   if(ocnt_ta)     { delete [] ocnt_ta;     ocnt_ta     = (ThreshArray *)      0; }
   if(cnt_logic)   { delete [] cnt_logic;   cnt_logic   = (SetLogic *)         0; }
   if(fwind_ta)    { delete [] fwind_ta;    fwind_ta    = (ThreshArray *)      0; }
   if(owind_ta)    { delete [] owind_ta;    owind_ta    = (ThreshArray *)      0; }
   if(wind_logic)  { delete [] wind_logic;  wind_logic  = (SetLogic *)         0; }
   if(interp_mthd) { delete [] interp_mthd; interp_mthd = (InterpMthd *)       0; }
   if(msg_typ)     { delete [] msg_typ;     msg_typ     = (StringArray *)      0; }
   if(sid_exc)     { delete [] sid_exc;     sid_exc     = (StringArray *)      0; }
   if(obs_qty)     { delete [] obs_qty;     obs_qty     = (StringArray *)      0; }
   if(mask_dp)     { delete [] mask_dp;     mask_dp     = (DataPlane *)        0; }
   if(mask_sid)    { delete [] mask_sid;    mask_sid    = (StringArray *)      0; }
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
   int i, n;
   ConcatString s;
   StringArray sa;
   VarInfoFactory info_factory;
   map<STATLineType,STATOutputType>output_map;
   Dictionary *fdict = (Dictionary *) 0;
   Dictionary *odict = (Dictionary *) 0;
   Dictionary i_fdict, i_odict;
   BootInfo boot_info;
   InterpInfo interp_info;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);
   
   // Initialize
   clear();

   // Conf: version
   version = parse_conf_version(&conf);

   // Conf: model
   model = parse_conf_string(&conf, conf_key_model);

   // Conf: regrid
   regrid_info = parse_conf_regrid(&conf);

   // Conf: beg_ds and end_ds
   fdict = conf.lookup_dictionary(conf_key_obs_window);
   parse_conf_range_int(fdict, beg_ds, end_ds);

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

   // Conf: fcst.field, obs.field, and climo.field
   fdict = conf.lookup_array(conf_key_fcst_field);
   odict = conf.lookup_array(conf_key_obs_field);

   // Determine the number of fields (name/level) to be verified
   n_vx = parse_conf_n_vx(fdict);

   // Check for a valid number of verification tasks
   if(n_vx == 0 || parse_conf_n_vx(odict) != n_vx) {
      mlog << Error << "\nPointStatConfInfo::process_config() -> "
           << "The number of verification tasks in \""
           << conf_key_obs_field
           << "\" must be non-zero and match the number in \""
           << conf_key_fcst_field << "\".\n\n";
      exit(1);
   }

   // Check climatology fields
   check_climo_n_vx(&conf, n_vx);

   // Allocate space based on the number of verification tasks
   vx_pd      = new VxPairDataPoint [n_vx];
   fcat_ta    = new ThreshArray     [n_vx];
   ocat_ta    = new ThreshArray     [n_vx];
   fcnt_ta    = new ThreshArray     [n_vx];
   ocnt_ta    = new ThreshArray     [n_vx];
   cnt_logic  = new SetLogic        [n_vx];
   fwind_ta   = new ThreshArray     [n_vx];
   owind_ta   = new ThreshArray     [n_vx];
   wind_logic = new SetLogic        [n_vx];
   msg_typ    = new StringArray     [n_vx];
   sid_exc    = new StringArray     [n_vx];
   obs_qty    = new StringArray     [n_vx];
   
   // Parse the fcst and obs field information
   for(i=0; i<n_vx; i++) {
     
      // Allocate new VarInfo objects
      vx_pd[i].fcst_info = info_factory.new_var_info(ftype);
      vx_pd[i].obs_info  = new VarInfoGrib;      

      // Get the current dictionaries
      i_fdict = parse_conf_i_vx_dict(fdict, i);
      i_odict = parse_conf_i_vx_dict(odict, i);

      // Conf: message_type
      msg_typ[i] = parse_conf_message_type(&i_odict);

      // Conf: sid_exc
      sid_exc[i] = parse_conf_sid_exc(&i_odict);
      vx_pd[i].set_sid_exc_filt(sid_exc[i]);

      // Conf: obs_qty
      obs_qty[i] = parse_conf_obs_qty(&i_odict);
      vx_pd[i].set_obs_qty_filt(obs_qty[i]);

      // Set the current dictionaries
      vx_pd[i].fcst_info->set_dict(i_fdict);
      vx_pd[i].obs_info->set_dict(i_odict);

      //fill in code data
      vx_pd[i].obs_info->add_grib_code (i_odict);

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
         (vx_pd[i].obs_info->level().lower() < vx_pd[i].fcst_info->level().lower() ||
          vx_pd[i].obs_info->level().upper() > vx_pd[i].fcst_info->level().upper())) {

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
              << "The observation field cannot contain probabilities.\n\n";
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

   // Initialize maximum threshold counts
   max_n_cat_thresh   = 0;
   max_n_cnt_thresh   = 0;
   max_n_wind_thresh  = 0;
   max_n_fprob_thresh = 0;
   max_n_oprob_thresh = 0;

   // Parse and sanity check thresholds   
   for(i=0; i<n_vx; i++) {

      // Get the current dictionaries
      i_fdict = parse_conf_i_vx_dict(fdict, i);
      i_odict = parse_conf_i_vx_dict(odict, i);

      // Conf: cat_thresh
      fcat_ta[i] = i_fdict.lookup_thresh_array(conf_key_cat_thresh);
      ocat_ta[i] = i_odict.lookup_thresh_array(conf_key_cat_thresh);

      // Conf: cnt_thresh
      fcnt_ta[i] = i_fdict.lookup_thresh_array(conf_key_cnt_thresh);
      ocnt_ta[i] = i_odict.lookup_thresh_array(conf_key_cnt_thresh);

      // Conf: cnt_logic
      cnt_logic[i] = check_setlogic(
                        int_to_setlogic(i_fdict.lookup_int(conf_key_cnt_logic)),
                        int_to_setlogic(i_odict.lookup_int(conf_key_cnt_logic)));

      // Conf: wind_thresh
      fwind_ta[i] = i_fdict.lookup_thresh_array(conf_key_wind_thresh);
      owind_ta[i] = i_odict.lookup_thresh_array(conf_key_wind_thresh);

      // Conf: wind_logic
      wind_logic[i] = check_setlogic(
                         int_to_setlogic(i_fdict.lookup_int(conf_key_wind_logic)),
                         int_to_setlogic(i_odict.lookup_int(conf_key_wind_logic)));

      // Dump the contents of the current thresholds
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed threshold settings for field number " << i+1 << "...\n"
              << "Forecast categorical thresholds: "  << fcat_ta[i].get_str() << "\n"
              << "Observed categorical thresholds: "  << ocat_ta[i].get_str() << "\n"
              << "Forecast continuous thresholds: "   << fcnt_ta[i].get_str() << "\n"
              << "Observed continuous thresholds: "   << ocnt_ta[i].get_str() << "\n"
              << "Continuous threshold logic: "       << setlogic_to_string(cnt_logic[i]) << "\n"
              << "Forecast wind speed thresholds: "   << fwind_ta[i].get_str() << "\n"
              << "Observed wind speed thresholds: "   << owind_ta[i].get_str() << "\n"
              << "Wind speed threshold logic: "       << setlogic_to_string(wind_logic[i]) << "\n";
      }

      // Verifying a probability field
      if(vx_pd[i].fcst_info->p_flag()) check_prob_thresh(fcat_ta[i]);

      // Check for equal threshold length for non-probability fields
      if(!vx_pd[i].fcst_info->p_flag() &&
         fcat_ta[i].n_elements() != ocat_ta[i].n_elements()) {

         mlog << Error << "\nPointStatConfInfo::process_config() -> "
              << "The number of thresholds for each field in \"fcst."
              << conf_key_cat_thresh
              << "\" must match the number of thresholds for each "
              << "field in \"obs." << conf_key_cat_thresh << "\".\n\n";
         exit(1);
      }

      // Add default continuous thresholds until the counts match
      n = max(fcnt_ta[i].n_elements(), ocnt_ta[i].n_elements());
      while(fcnt_ta[i].n_elements() < n) fcnt_ta[i].add(na_str);
      while(ocnt_ta[i].n_elements() < n) ocnt_ta[i].add(na_str);

      // Add default wind speed thresholds until the counts match
      n = max(fwind_ta[i].n_elements(), owind_ta[i].n_elements());
      while(fwind_ta[i].n_elements() < n) fwind_ta[i].add(na_str);
      while(owind_ta[i].n_elements() < n) owind_ta[i].add(na_str);

      // Verifying with multi-category contingency tables
      if(!vx_pd[i].fcst_info->p_flag() &&
         (output_flag[i_mctc] != STATOutputType_None ||
          output_flag[i_mcts] != STATOutputType_None)) {

         check_mctc_thresh(fcat_ta[i]);
         check_mctc_thresh(ocat_ta[i]);
      }

      // Look for the maximum number of thresholds
      if(!vx_pd[i].fcst_info->p_flag()) {

         if(fcat_ta[i].n_elements() > max_n_cat_thresh)
            max_n_cat_thresh = fcat_ta[i].n_elements();
         if(fcnt_ta[i].n_elements() > max_n_cnt_thresh)
            max_n_cnt_thresh = fcnt_ta[i].n_elements();
         if(fwind_ta[i].n_elements() > max_n_wind_thresh)
            max_n_wind_thresh = fwind_ta[i].n_elements();
      }
      // Look for the maximum number of thresholds for prob fields
      else {

         if(fcat_ta[i].n_elements() > max_n_fprob_thresh)
            max_n_fprob_thresh = fcat_ta[i].n_elements();
         if(ocat_ta[i].n_elements() > max_n_oprob_thresh)
            max_n_oprob_thresh = ocat_ta[i].n_elements();
      }
   } // end for i

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
   StringArray mask_grid, mask_poly, sid_list;
   ConcatString s;

   // Retrieve the area masks
   mask_grid = conf.lookup_string_array(conf_key_mask_grid);
   mask_poly = conf.lookup_string_array(conf_key_mask_poly);
   n_mask_area = mask_grid.n_elements() + mask_poly.n_elements();

   // Retrieve the station masks
   sid_list = conf.lookup_string_array(conf_key_mask_sid);
   n_mask_sid = sid_list.n_elements();

   // Save the total number masks as a sum of the masking areas and
   // masking points
   n_mask = n_mask_area + n_mask_sid;

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

   // Allocate space to store the station ID masks
   mask_sid = new StringArray [n_mask_sid];

   // Parse out the station ID masks
   for(i=0; i<sid_list.n_elements(); i++) {
      parse_sid_mask(sid_list[i], mask_sid[i], s);
      mask_name.add(s);
   }

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
         if(j<n_mask_area) {
            vx_pd[i].set_mask_dp(j, mask_name[j], &mask_dp[j]);
         }
         // Otherwise this is a masking StationID
         else {
            vx_pd[i].set_mask_sid(j, mask_name[j], &mask_sid[j-n_mask_area]);
         }
      }

      // Add the interpolation methods to the vx_pd objects
      for(j=0; j<n_interp; j++)
         vx_pd[i].set_interp(j, interp_mthd[j], interp_wdth[j]);
   } // end for i

   // Set the duplicate flag for each pair data object
   for(i=0; i<n_vx; i++) vx_pd[i].set_duplicate_flag(duplicate_flag);

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
      case(i_ctc):
         // Maximum number of FHO or CTC lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds
         n = n_vx_scal * max_n_msg_typ * n_mask * n_interp *
             max_n_cat_thresh;
         break;

      case(i_cts):
         // Maximum number of CTS lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds * Alphas
         n = n_vx_scal * max_n_msg_typ * n_mask * n_interp *
             max_n_cat_thresh * get_n_ci_alpha();
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
         //    Max Thresholds * Alphas
         n = n_vx_scal * max_n_msg_typ * n_mask * n_interp *
             max_n_cnt_thresh * get_n_ci_alpha();
         break;

      case(i_sl1l2):
      case(i_sal1l2):
         // Maximum number of SL1L2 or SAL1L2 lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds
         n = n_vx_scal * max_n_msg_typ * n_mask * n_interp *
             max_n_cnt_thresh;
         break;

      case(i_vl1l2):
      case(i_val1l2):
         // Maximum number of VL1L2 or VAL1L2 lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds
         n = n_vx_vect * max_n_msg_typ * n_mask * n_interp *
             max_n_wind_thresh;
         break;

      case(i_pct):
      case(i_pjc):
      case(i_prc):
         // Maximum number of PCT, PJC, or PRC lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds
         n = n_vx_prob * max_n_msg_typ * n_mask * n_interp *
             max_n_oprob_thresh;
         break;

      case(i_pstd):
         // Maximum number of PSTD lines possible =
         //    Fields * Message Types * Masks * Smoothing Methods *
         //    Max Thresholds * Alphas
         n = n_vx_prob * max_n_msg_typ * n_mask * n_interp *
             max_n_oprob_thresh * get_n_ci_alpha();
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
