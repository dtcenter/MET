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

#include "ensemble_stat_conf_info.h"

#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class EnsembleStatConfInfo
//
////////////////////////////////////////////////////////////////////////

EnsembleStatConfInfo::EnsembleStatConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

EnsembleStatConfInfo::~EnsembleStatConfInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::init_from_scratch() {

   // Initialize pointers
   ens_info    = (VarInfo **)           0;
   ens_ta      = (ThreshArray *)        0;
   vx_pd       = (VxPairDataEnsemble *) 0;
   msg_typ     = (StringArray *)        0;
   sid_exc     = (StringArray *)        0;
   obs_qty     = (StringArray *)        0;
   mask_dp     = (DataPlane *)          0;
   interp_mthd = (InterpMthd *)         0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::clear() {
   int i;

   // Set counts to zero
   max_n_thresh = 0;
   n_vx         = 0;
   n_interp     = 0;
   n_mask       = 0;
   n_mask_area  = 0;
   n_mask_sid   = 0;

   // Initialize values
   model.clear();
   obtype.clear();
   regrid_info.clear();
   beg_ds = end_ds = bad_data_int;
   vld_ens_thresh = vld_data_thresh = bad_data_double;
   mask_name.clear();
   ci_alpha.clear();
   interp_field = FieldType_None;
   interp_thresh = bad_data_double;
   interp_wdth.clear();
   rng_type.clear();
   rng_seed.clear();
   duplicate_flag = DuplicateType_None;
   grid_weight_flag = GridWeightType_None;
   output_prefix.clear();
   version.clear();

   for(i=0; i<n_txt; i++) output_flag[i]   = STATOutputType_None;
   for(i=0; i<n_nc;  i++) ensemble_flag[i] = false;

   // Deallocate memory
   if(ens_info)    { delete [] ens_info;    ens_info    = (VarInfo **)           0; }
   if(ens_ta)      { delete [] ens_ta;      ens_ta      = (ThreshArray *)        0; }
   if(vx_pd)       { delete [] vx_pd;       vx_pd       = (VxPairDataEnsemble *) 0; }
   if(msg_typ)     { delete [] msg_typ;     msg_typ     = (StringArray *)        0; }
   if(sid_exc)     { delete [] sid_exc;     sid_exc     = (StringArray *)        0; }
   if(obs_qty)     { delete [] obs_qty;     obs_qty     = (StringArray *)        0; }
   if(mask_dp)     { delete [] mask_dp;     mask_dp     = (DataPlane *)          0; }
   if(mask_sid)    { delete [] mask_sid;    mask_sid    = (StringArray *)        0; }
   if(interp_mthd) { delete [] interp_mthd; interp_mthd = (InterpMthd *)         0; }

   // Clear ens_info
   if(ens_info) {
      for(i=0; i<n_vx; i++)
         if(ens_info[i]) { delete ens_info[i]; ens_info[i] = (VarInfo *) 0; }
      delete ens_info; ens_info = (VarInfo **) 0;
   }

   // Reset count
   n_ens_var = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::read_config(const char *default_file_name,
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

void EnsembleStatConfInfo::process_config(GrdFileType etype,
                                          GrdFileType otype,
                                          bool point_vx) {
   int i;
   VarInfoFactory info_factory;
   map<STATLineType,STATOutputType>output_map;
   Dictionary *ens_dict  = (Dictionary *) 0;
   Dictionary *fcst_dict = (Dictionary *) 0;
   Dictionary *obs_dict  = (Dictionary *) 0;
   Dictionary i_ens_dict, i_fcst_dict, i_obs_dict;
   InterpInfo interp_info;

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

   // Conf: beg_ds and end_ds
   ens_dict = conf.lookup_dictionary(conf_key_obs_window);
   parse_conf_range_int(ens_dict, beg_ds, end_ds);

   // Conf: output_flag
   output_map = parse_conf_output_flag(&conf);

   // Make sure the output_flag is the expected size
   if((signed int) output_map.size() != n_txt) {
      mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
           << "Unexpected number of entries found in \""
           << conf_key_output_flag << "\" ("
           << (signed int) output_map.size()
           << " != " << n_txt << ").\n\n";
      exit(1);
   }

   // Populate the output_flag array with map values
   for(i=0; i<n_txt; i++) {
      output_flag[i] = output_map[txt_file_type[i]];
   }

   // Conf: ensemble_flag
   ens_dict = conf.lookup_dictionary(conf_key_ensemble_flag);

   // Populate the ensemble_flag array
   for(i=0; i<n_nc; i++) {
      ensemble_flag[i] = ens_dict->lookup_bool(conf_key_ensemble_flag_entries[i]);
   }

   // Conf: ens.field
   ens_dict = conf.lookup_array(conf_key_ens_field);

   // Determine the number of ensemble fields (name/level) to be processed
   n_ens_var = parse_conf_n_vx(ens_dict);

   // Check for a valid number of ensemble fields
   if(n_ens_var == 0) {
      mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
           << "At least one field must be specified for \""
           << conf_key_ens_field << "\".\n\n";
      exit(1);
   }

   // Allocate space based on the number of ensemble fields
   ens_info = new VarInfo * [n_ens_var];
   ens_ta   = new ThreshArray [n_ens_var];

   // Initialize pointers
   for(i=0; i<n_ens_var; i++) ens_info[i] = (VarInfo *) 0;

    // Parse the ensemble field information
   for(i=0,max_n_thresh=0; i<n_ens_var; i++) {

      // Allocate new VarInfo object
      ens_info[i] = info_factory.new_var_info(etype);

      // Get the current dictionary
      i_ens_dict = parse_conf_i_vx_dict(ens_dict, i);

      // Set the current dictionary
      ens_info[i]->set_dict(i_ens_dict);

      // Dump the contents of the current VarInfo
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed ensemble field number " << i+1 << ":\n";
         ens_info[i]->dump(cout);
      }

      // Only parse thresholds if relative frequencies are requested
      if(ensemble_flag[i_nc_freq]) {

         // Conf: cat_thresh
         ens_ta[i] = i_ens_dict.lookup_thresh_array(conf_key_cat_thresh);

         // Dump the contents of the current thresholds
         if(mlog.verbosity_level() >= 5) {
            mlog << Debug(5)
                 << "Parsed thresholds for ensemble field number " << i+1 << ":\n";
            ens_ta[i].dump(cout);
         }

         // Keep track of the maximum number of thresholds
         if(ens_ta[i].n_elements() > max_n_thresh) {
            max_n_thresh = ens_ta[i].n_elements();
         }
      }
   }

   // Conf: ens.ens_thresh
   vld_ens_thresh = conf.lookup_double(conf_key_ens_ens_thresh);

   // Check that the valid ensemble threshold is between 0 and 1.
   if(vld_ens_thresh <= 0.0 || vld_ens_thresh > 1.0) {
      mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
           << "The \"" << conf_key_ens_ens_thresh << "\" parameter ("
           << vld_ens_thresh << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   // Conf: ens.vld_thresh
   vld_data_thresh = conf.lookup_double(conf_key_ens_vld_thresh);

   // Check that the valid data threshold is between 0 and 1.
   if(vld_data_thresh <= 0.0 || vld_data_thresh > 1.0) {
      mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
           << "The \"" << conf_key_ens_vld_thresh << "\" parameter ("
           << vld_data_thresh << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   // Conf: fcst.field and obs.field
   fcst_dict = conf.lookup_array(conf_key_fcst_field);
   obs_dict  = conf.lookup_array(conf_key_obs_field);

   // Determine the number of fields (name/level) to be verified
   n_vx = parse_conf_n_vx(fcst_dict);

   // Check for a valid number of verification tasks
   if(parse_conf_n_vx(obs_dict) != n_vx) {
      mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
           << "The number of verification tasks in \""
           << conf_key_obs_field
           << "\" must match the number in \""
           << conf_key_fcst_field << "\".\n\n";
      exit(1);
   }

   if(n_vx > 0) {

      // Check climatology fields
      check_climo_n_vx(&conf, n_vx);

      // Check to make sure the observation file type is defined
      if(otype == FileType_None) {
         mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
              << "When \"fcst.field\" is non-empty, you must use "
              << "\"-point_obs\" and/or \"-grid_obs\" to specify the "
              << "verifying observations.\n\n";
         exit(1);
      }

      // Allocate space based on the number of verification tasks
      vx_pd   = new VxPairDataEnsemble [n_vx];
      msg_typ = new StringArray        [n_vx];
      sid_exc = new StringArray        [n_vx];
      obs_qty = new StringArray        [n_vx];

      // Parse the fcst field information
      for(i=0; i<n_vx; i++) {

         // Allocate new VarInfo objects
         vx_pd[i].fcst_info = info_factory.new_var_info(etype);

         // Point observations are specified following GRIB conventions
         if(point_vx) vx_pd[i].obs_info = new VarInfoGrib;
         else         vx_pd[i].obs_info = info_factory.new_var_info(otype);

         // Get the current dictionaries
         i_fcst_dict = parse_conf_i_vx_dict(fcst_dict, i);
         i_obs_dict  = parse_conf_i_vx_dict(obs_dict, i);

         // Conf: message_type
         msg_typ[i] = parse_conf_message_type(&i_obs_dict);

         // Conf: sid_exc
         sid_exc[i] = parse_conf_sid_exc(&i_obs_dict);
         vx_pd[i].set_sid_exc_filt(sid_exc[i]);

         // Conf: obs_qty
         obs_qty[i] = parse_conf_obs_qty(&i_obs_dict);
         vx_pd[i].set_obs_qty_filt(obs_qty[i]);

         // Conf: ssvar_bin_size
         ens_ssvar_bin_size.add(i_obs_dict.lookup_double(conf_key_ssvar_bin));

         // Conf: phist_bin_size
         ens_phist_bin_size.add(i_obs_dict.lookup_double(conf_key_phist_bin));

         // Set the current dictionaries
         vx_pd[i].fcst_info->set_dict(i_fcst_dict);
         vx_pd[i].obs_info->set_dict(i_obs_dict);

         // Set the GRIB code for point observations
         if(point_vx) vx_pd[i].obs_info->add_grib_code(i_obs_dict);

         // Dump the contents of the current VarInfo
         if(mlog.verbosity_level() >= 5) {
            mlog << Debug(5)
                 << "Parsed forecast field number " << i+1 << ":\n";
            vx_pd[i].fcst_info->dump(cout);
            mlog << Debug(5)
                 << "Parsed observation field number " << i+1 << ":\n";
            vx_pd[i].obs_info->dump(cout);
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
                 << "\nEnsembleStatConfInfo::process_config() -> "
                 << "The range of requested observation pressure levels "
                 << "is not contained within the range of requested "
                 << "forecast pressure levels.  No vertical interpolation "
                 << "will be performed for observations falling outside "
                 << "the range of forecast levels.  Instead, they will be "
                 << "matched to the single nearest forecast level.\n\n";
         }
      }
   } // end if n_vx > 0

   // Conf: ci_alpha
   ci_alpha = parse_conf_ci_alpha(&conf);

   // Conf: interp
   interp_info   = parse_conf_interp(&conf);
   interp_field  = interp_info.field;
   interp_thresh = interp_info.vld_thresh;
   n_interp      = interp_info.n_interp;
   interp_wdth   = interp_info.width;

   // Allocate memory to store the interpolation methods
   interp_mthd = new InterpMthd [n_interp];
   for(i=0; i<n_interp; i++)
      interp_mthd[i] = string_to_interpmthd(interp_info.method[i]);

   // Conf: rng_type and rng_seed
   rng_type = conf.lookup_string(conf_key_rng_type);
   rng_seed = conf.lookup_string(conf_key_rng_seed);

   // Conf: grid_weight_flag
   grid_weight_flag = parse_conf_grid_weight_flag(&conf);

   // Conf: duplicate_flag
   duplicate_flag = parse_conf_duplicate_flag(&conf);

   // Conf: output_prefix
   output_prefix = conf.lookup_string(conf_key_output_prefix);

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::process_masks(const Grid &grid) {
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
      mlog << Error << "\nEnsembleStatConfInfo::process_masks() -> "
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

void EnsembleStatConfInfo::set_vx_pd(const IntArray &n_ens_vld) {
   int i, j, n_msg_typ;

   // EnsPairData is stored in the vx_pd objects in the following order:
   // [n_msg_typ][n_mask][n_interp]
   for(i=0; i<n_vx; i++) {

      // Get the message types for the current verification task
      n_msg_typ = get_n_msg_typ(i);

      // Set up the dimensions for the vx_pd object
      vx_pd[i].set_pd_size(n_msg_typ, n_mask, n_interp);

      // Set up the ensemble size
      vx_pd[i].set_ens_size(n_ens_vld[i]);

      // Set the ensemble spread/skill information
      vx_pd[i].ens_ssvar_flag = ens_ssvar_flag;
      vx_pd[i].ens_ssvar_mean = ens_ssvar_mean;
      vx_pd[i].set_ssvar_bin_size(ens_ssvar_bin_size[i]);

      // Set the PIT histogram bin size
      vx_pd[i].set_phist_bin_size(ens_phist_bin_size[i]);

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
   for(i=0; i<n_vx; i++) vx_pd[i].set_duplicate_flag((DuplicateType) duplicate_flag);

   return;
}

////////////////////////////////////////////////////////////////////////

int EnsembleStatConfInfo::get_n_msg_typ(int i) const {

   if(i < 0 || i >= n_vx) {
      mlog << Error << "\nEnsembleStatConfInfo::get_n_msg_typ(int i) -> "
           << "range check error for i = " << i << "\n\n";
      exit(1);
   }

   return(msg_typ[i].n_elements());
}

////////////////////////////////////////////////////////////////////////

int EnsembleStatConfInfo::n_txt_row(int i_txt_row) {
   int i, n, max_n_msg_typ;

   // Determine the maximum number of message types being used
   for(i=0, max_n_msg_typ=0; i<n_vx; i++)
      if(get_n_msg_typ(i) > max_n_msg_typ)
         max_n_msg_typ = get_n_msg_typ(i);

   // Switch on the index of the line type
   switch(i_txt_row) {

      case(i_rhist):
      case(i_phist):
         // Maximum number of Rank and PIT Histogram lines possible =
         //    Fields * Masks * Interpolations * Message Type [Point Obs]
         //    Fields * Masks * Interpolations [Grid Obs]
         n =   n_vx * n_mask * n_interp * max_n_msg_typ
             + n_vx * n_mask * n_interp;
         break;

      case(i_orank):

         // Compute the maximum number of matched pairs to be written
         // out by summing the number for each VxPairDataEnsemble object
         for(i=0, n=0; i<n_vx; i++) {
            n += vx_pd[i].get_n_pair();
         }
         break;

      case(i_ssvar):

         // Just return zero since we'll resize the output AsciiTables
         // to accomodate the SSVAR output
         n = 0;
         break;

      default:
         mlog << Error << "\nEnsembleStatConfInfo::n_txt_row(int) -> "
              << "unexpected output type index value: " << i_txt_row
              << "\n\n";
         exit(1);
         break;
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////

int EnsembleStatConfInfo::n_stat_row() {
   int i, n;

   // Set the maximum number of STAT output lines by summing the counts
   // for the optional text files that have been requested
   for(i=0, n=0; i<n_txt; i++) {

      if(output_flag[i] != STATOutputType_None) n += n_txt_row(i);
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////
