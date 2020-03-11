// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
#include "configobjecttype_to_string.h"

#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_log.h"

#include "GridTemplate.h"

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
   ens_info = (VarInfo **)          0;
   ens_ta   = (ThreshArray *)       0;
   vx_opt   = (EnsembleStatVxOpt *) 0;
   rng_ptr  = (gsl_rng *)           0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::clear() {
   int i;

   // Initialize values
   ens_var_str.clear();
   model.clear();
   obtype.clear();
   vld_ens_thresh = bad_data_double;
   vld_data_thresh = bad_data_double;
   nbrhd_prob.clear();
   nmep_smooth.clear();
   msg_typ_group_map.clear();
   msg_typ_sfc.clear();
   mask_area_map.clear();
   mask_sid_map.clear();
   grid_weight_flag = GridWeightType_None;
   tmp_dir.clear();
   output_prefix.clear();
   version.clear();

   // Deallocate memory for the random number generator
   if(rng_ptr) rng_free(rng_ptr);

   for(i=0; i<n_txt; i++) output_flag[i] = STATOutputType_None;

   nc_info.clear();

   // Deallocate memory
   if(vx_opt) { delete [] vx_opt; vx_opt = (EnsembleStatVxOpt *) 0; }

   if(ens_info) {
      for(i=0; i<n_ens_var; i++) {
         if(ens_info[i]) { delete ens_info[i]; ens_info[i] = (VarInfo *) 0; }
      }
      delete ens_info; ens_info = (VarInfo **) 0;
   }

   // Reset counts
   n_ens_var    = 0;
   max_n_thresh = 0;
   n_nbrhd      = 0;
   n_vx         = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::read_config(const ConcatString default_file_name,
                                       const ConcatString user_file_name) {

   // Read the config file constants
   conf.read(replace_path(config_const_filename).c_str());

   // Read the default config file
   conf.read(default_file_name.c_str());

   // Read the user-specified config file
   conf.read(user_file_name.c_str());

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::process_config(GrdFileType etype,
                                          GrdFileType otype,
                                          bool grid_vx, bool point_vx,
                                          bool use_var_id) {
   int i;
   VarInfoFactory info_factory;
   map<STATLineType,STATOutputType>output_map;
   Dictionary *edict  = (Dictionary *) 0;
   Dictionary *fdict = (Dictionary *) 0;
   Dictionary *odict  = (Dictionary *) 0;
   Dictionary i_edict, i_fdict, i_odict;
   InterpMthd mthd;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Initialize
   clear();

   // Conf: version
   version = parse_conf_version(&conf);

   // Conf: model
   model = parse_conf_string(&conf, conf_key_model);

   // Conf: obtype
   obtype = parse_conf_string(&conf, conf_key_obtype, grid_vx);

   // Conf: rng_type and rng_seed
   ConcatString rng_type, rng_seed;
   rng_type = conf.lookup_string(conf_key_rng_type);
   rng_seed = conf.lookup_string(conf_key_rng_seed);

   // Set the random number generator and seed value
   rng_set(rng_ptr, rng_type.c_str(), rng_seed.c_str());

   // Conf: grid_weight_flag
   grid_weight_flag = parse_conf_grid_weight_flag(&conf);

   // Conf: output_prefix
   output_prefix = conf.lookup_string(conf_key_output_prefix);

   // Conf: message_type_group_map
   msg_typ_group_map = parse_conf_message_type_group_map(&conf);

   // Conf: message_type_group_map(SURFACE)
   if(msg_typ_group_map.count((string)surface_msg_typ_group_str) == 0) {
      mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
           << "\"" << conf_key_message_type_group_map
           << "\" must contain an entry for \""
           << surface_msg_typ_group_str << "\".\n\n";
      exit(1);
   }
   else {
     msg_typ_sfc = msg_typ_group_map[(string)surface_msg_typ_group_str];
   }

   // Conf: ensemble_flag
   parse_nc_info();

   // Conf: ens.field
   edict = conf.lookup_array(conf_key_ens_field);

   // Determine the number of ensemble fields to be processed
   n_ens_var = parse_conf_n_vx(edict);

   // Allocate space based on the number of ensemble fields
   if(n_ens_var > 0) {
      ens_info = new VarInfo *   [n_ens_var];
      ens_ta   = new ThreshArray [n_ens_var];
   }

   // Initialize pointers
   for(i=0; i<n_ens_var; i++) ens_info[i] = (VarInfo *) 0;

    // Parse the ensemble field information
   for(i=0,max_n_thresh=0; i<n_ens_var; i++) {

      // Allocate new VarInfo object
      ens_info[i] = info_factory.new_var_info(etype);

      // Get the current dictionary
      i_edict = parse_conf_i_vx_dict(edict, i);

      // Set the current dictionary
      ens_info[i]->set_dict(i_edict);

      // Dump the contents of the current VarInfo
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed ensemble field number " << i+1 << ":\n";
         ens_info[i]->dump(cout);
      }

      // Conf: ens_nc_var_str
      ens_var_str.add(parse_conf_string(&i_edict, conf_key_nc_var_str, false));

      // Conf: ens_nc_pairs
      // Only parse thresholds if probabilities are requested
      if(nc_info.do_freq || nc_info.do_nep || nc_info.do_nmep) {

         // Conf: cat_thresh
         ens_ta[i] = i_edict.lookup_thresh_array(conf_key_cat_thresh);

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
   if(vld_ens_thresh < 0.0 || vld_ens_thresh > 1.0) {
      mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
           << "The \"" << conf_key_ens_ens_thresh << "\" parameter ("
           << vld_ens_thresh << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   // Conf: ens.vld_thresh
   vld_data_thresh = conf.lookup_double(conf_key_ens_vld_thresh);

   // Check that the valid data threshold is between 0 and 1.
   if(vld_data_thresh < 0.0 || vld_data_thresh > 1.0) {
      mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
           << "The \"" << conf_key_ens_vld_thresh << "\" parameter ("
           << vld_data_thresh << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   // Conf: nbrhd_prob
   nbrhd_prob = parse_conf_nbrhd(edict, conf_key_nbrhd_prob);
   n_nbrhd = nbrhd_prob.width.n();

   // Conf: nmep_smooth 
   nmep_smooth = parse_conf_interp(edict, conf_key_nmep_smooth);

   // Loop through the neighborhood probability smoothing options
   for(i=0; i<nmep_smooth.n_interp; i++) {

      mthd = string_to_interpmthd(nmep_smooth.method[i].c_str());

      // Check for unsupported neighborhood probability smoothing methods
      if(mthd == InterpMthd_DW_Mean ||
         mthd == InterpMthd_LS_Fit  ||
         mthd == InterpMthd_Bilin) {
         mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
              << "Neighborhood probability smoothing methods DW_MEAN, "
              << "LS_FIT, and BILIN are not supported for \""
              << conf_key_nmep_smooth << "\".\n\n";
         exit(1);
      }

      // Check for valid neighborhood probability interpolation widths
      if(nmep_smooth.width[i] < 1 || nmep_smooth.width[i]%2 == 0) {
         mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
              << "Neighborhood probability smoothing widths must be set "
              << "to odd values greater than or equal to 1 ("
              << nmep_smooth.width[i] << ") for \""
              << conf_key_nmep_smooth << "\".\n\n";
         exit(1);
      }
   } // end for i

   // Conf: fcst.field and obs.field
   fdict = conf.lookup_array(conf_key_fcst_field);
   odict = conf.lookup_array(conf_key_obs_field);

   // Determine the number of fields to be verified
   n_vx = parse_conf_n_vx(fdict);

   // Check for a valid number of verification tasks
   if(parse_conf_n_vx(odict) != n_vx) {
      mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
           << "The number of verification tasks in \""
           << conf_key_obs_field
           << "\" must match the number in \""
           << conf_key_fcst_field << "\".\n\n";
      exit(1);
   }

   if(n_vx > 0) {

      // Allocate memory for the verification task options
      vx_opt = new EnsembleStatVxOpt [n_vx];

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

      // Parse settings for each verification task
      for(i=0; i<n_vx; i++) {

         // Get the current dictionaries
         i_fdict = parse_conf_i_vx_dict(fdict, i);
         i_odict = parse_conf_i_vx_dict(odict, i);

         // Process the options for this verification task
         vx_opt[i].process_config(etype, i_fdict, otype, i_odict,
                                  rng_ptr, grid_vx, point_vx,
                                  use_var_id);

         // For no point verification, store obtype as the message type
         if(!point_vx) {
            vx_opt[i].msg_typ.clear();
            vx_opt[i].msg_typ.add(obtype);
         }
      }

      // Summarize output flags across all verification tasks
      process_flags();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::parse_nc_info() {
   const DictionaryEntry * e = (const DictionaryEntry *) 0;

   e = conf.lookup(conf_key_ensemble_flag);

   if(!e) {
      mlog << Error
           << "\nEnsembleStatConfInfo::parse_nc_info() -> "
           << "lookup failed for key \"" << conf_key_ensemble_flag
           << "\"\n\n";
      exit(1);
   }

   const ConfigObjectType type = e->type();

   if(type == BooleanType) {
      bool value = e->b_value();

      if(!value) nc_info.set_all_false();

      return;
   }

   // It should be a dictionary
   if(type != DictionaryType) {
      mlog << Error
           << "\nEnsembleStatConfInfo::parse_nc_info() -> "
           << "bad type (" << configobjecttype_to_string(type)
           << ") for key \"" << conf_key_ensemble_flag << "\"\n\n";
      exit(1);
   }

   // Parse the various entries
   Dictionary * d = e->dict_value();

   nc_info.do_latlon = d->lookup_bool(conf_key_latlon_flag);
   nc_info.do_mean   = d->lookup_bool(conf_key_mean_flag);
   nc_info.do_stdev  = d->lookup_bool(conf_key_stdev_flag);
   nc_info.do_minus  = d->lookup_bool(conf_key_minus_flag);
   nc_info.do_plus   = d->lookup_bool(conf_key_plus_flag);
   nc_info.do_min    = d->lookup_bool(conf_key_min_flag);
   nc_info.do_max    = d->lookup_bool(conf_key_max_flag);
   nc_info.do_range  = d->lookup_bool(conf_key_range_flag);
   nc_info.do_vld    = d->lookup_bool(conf_key_vld_count_flag);
   nc_info.do_freq   = d->lookup_bool(conf_key_frequency_flag);
   nc_info.do_nep    = d->lookup_bool(conf_key_nep_flag);
   nc_info.do_nmep   = d->lookup_bool(conf_key_nmep_flag);
   nc_info.do_orank  = d->lookup_bool(conf_key_rank_flag);
   nc_info.do_weight = d->lookup_bool(conf_key_weight);

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::process_flags() {
   int i, j;
   bool output_ascii_flag = false;

   // Initialize
   for(i=0; i<n_txt; i++) output_flag[i] = STATOutputType_None;

   // Loop over the verification tasks
   for(i=0; i<n_vx; i++) {

      // Summary of output_flag settings
      for(j=0; j<n_txt; j++) {
         if(vx_opt[i].output_flag[j] == STATOutputType_Both) {
            output_flag[j] = STATOutputType_Both;
            output_ascii_flag = true;
         }
         else if(vx_opt[i].output_flag[j] == STATOutputType_Stat &&
                           output_flag[j] == STATOutputType_None) {
            output_flag[j] = STATOutputType_Stat;
            output_ascii_flag = true;
         }
      }
   }

   // Check output_ascii_flag
   if(!output_ascii_flag) {
       mlog << Debug(3)
            <<"\nNo STAT output types requested, "
            << "proceeding with ASCII output flag set to false.\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::process_masks(const Grid &grid) {
   int i, j;
   MaskPlane mp;
   StringArray sid;
   ConcatString name;

   mlog << Debug(2)
        << "Processing masking regions.\n";

   // Mapping of grid definition strings to mask names
   map<ConcatString,ConcatString> grid_map;
   map<ConcatString,ConcatString> poly_map;
   map<ConcatString,ConcatString>  sid_map;
   map<ConcatString,MaskLatLon>   point_map;

   // Initialize
   mask_area_map.clear();
   mask_sid_map.clear();

   // Process the masks for each vx task
   for(i=0; i<n_vx; i++) {

      // Initialize
      vx_opt[i].mask_name.clear();
      vx_opt[i].mask_name_area.clear();

      // Parse the masking grids
      for(j=0; j<vx_opt[i].mask_grid.n_elements(); j++) {

         // Process new grid masks
         if(grid_map.count(vx_opt[i].mask_grid[j]) == 0) {
            mlog << Debug(3)
                 << "Processing grid mask: "
                 << vx_opt[i].mask_grid[j] << "\n";
            parse_grid_mask(vx_opt[i].mask_grid[j], grid, mp, name);
            grid_map[vx_opt[i].mask_grid[j]] = name;
            mask_area_map[name] = mp;
         }

         // Store the name for gridded and point verification
         vx_opt[i].mask_name.add(grid_map[vx_opt[i].mask_grid[j]]);
         vx_opt[i].mask_name_area.add(grid_map[vx_opt[i].mask_grid[j]]);

      } // end for j

      // Parse the masking polylines
      for(j=0; j<vx_opt[i].mask_poly.n_elements(); j++) {

         // Process new poly mask
         if(poly_map.count(vx_opt[i].mask_poly[j]) == 0) {
            mlog << Debug(3)
                 << "Processing poly mask: "
                 << vx_opt[i].mask_poly[j] << "\n";
            parse_poly_mask(vx_opt[i].mask_poly[j], grid, mp, name);
            poly_map[vx_opt[i].mask_poly[j]] = name;
            mask_area_map[name] = mp;
         }

         // Store the name for gridded and point verification
         vx_opt[i].mask_name.add(poly_map[vx_opt[i].mask_poly[j]]);
         vx_opt[i].mask_name_area.add(poly_map[vx_opt[i].mask_poly[j]]);

      } // end for j

      // Parse the masking station ID's
      for(j=0; j<vx_opt[i].mask_sid.n_elements(); j++) {

         // Process new station ID mask
         if(sid_map.count(vx_opt[i].mask_sid[j]) == 0) {
            mlog << Debug(3)
                 << "Processing station ID mask: "
                 << vx_opt[i].mask_sid[j] << "\n";
            parse_sid_mask(vx_opt[i].mask_sid[j], sid, name);
            sid_map[vx_opt[i].mask_sid[j]] = name;
            mask_sid_map[name] = sid;
         }

         // Store the name only for point verification
         vx_opt[i].mask_name.add(sid_map[vx_opt[i].mask_sid[j]]);

      } // end for j

      // Parse the Lat/Lon point masks
      for(j=0; j<(int) vx_opt[i].mask_llpnt.size(); j++) {

         // Process new point masks -- no real work to do
         if(point_map.count(vx_opt[i].mask_llpnt[j].name) == 0) {
            mlog << Debug(3)
                 << "Processing Lat/Lon point mask: "
                 << vx_opt[i].mask_llpnt[j].name << "\n";
            point_map[vx_opt[i].mask_llpnt[j].name] = vx_opt[i].mask_llpnt[j];
         }

         // Store the name for the current Lat/Lon point mask
         vx_opt[i].mask_name.add(vx_opt[i].mask_llpnt[j].name);

      } // end for j

      // Check that at least one verification masking region is provided
      if(vx_opt[i].mask_name.n_elements() == 0) {
         mlog << Error << "\nEnsembleStatConfInfo::process_masks() -> "
              << "At least one grid, polyline or station ID masking "
              << "region must be provided for verification task number "
              << i+1 << ".\n\n";
         exit(1);
      }

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

int EnsembleStatConfInfo::n_txt_row(int i_txt_row) const {
   int i, n;

   // Loop over the tasks and sum the line counts for this line type
   for(i=0, n=0; i<n_vx; i++) n += vx_opt[i].n_txt_row(i_txt_row);

   return(n);
}

////////////////////////////////////////////////////////////////////////

int EnsembleStatConfInfo::n_stat_row() const {
   int i, n;

   // Loop over the line types and sum the line counts
   for(i=0, n=0; i<n_txt; i++) n += n_txt_row(i);

   return(n);
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::set_vx_pd(const IntArray &ens_size) {

   // This should be called after process_masks()
   for(int i=0; i<n_vx; i++) {

      vx_opt[i].set_vx_pd(this);

      // Set up the ensemble size
      vx_opt[i].vx_pd.set_ens_size(ens_size[i]);

   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class EnsembleStatVxOpt
//
////////////////////////////////////////////////////////////////////////

EnsembleStatVxOpt::EnsembleStatVxOpt() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

EnsembleStatVxOpt::~EnsembleStatVxOpt() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatVxOpt::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatVxOpt::clear() {
   int i;

   // Initialize values
   vx_pd.clear();
   var_str.clear();
   beg_ds = end_ds = bad_data_int;
   mask_grid.clear();
   mask_poly.clear();
   mask_sid.clear();
   mask_llpnt.clear();
   mask_name.clear();
   mask_name_area.clear();
   msg_typ.clear();
   othr_ta.clear();
   cdf_info.clear();
   ci_alpha.clear();
   interp_info.clear();

   ssvar_bin_size = bad_data_double;
   phist_bin_size = bad_data_double;
   prob_cat_ta.clear();

   duplicate_flag = DuplicateType_None;
   obs_summary = ObsSummary_None;
   obs_perc = bad_data_int;
   skip_const = false;
   obs_error.clear();

   for(i=0; i<n_txt; i++) output_flag[i] = STATOutputType_None;

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatVxOpt::process_config(GrdFileType ftype, Dictionary &fdict,
                                       GrdFileType otype, Dictionary &odict,
                                       gsl_rng *rng_ptr, bool grid_vx,
                                       bool point_vx, bool use_var_id) {
   int i;
   VarInfoFactory info_factory;
   map<STATLineType,STATOutputType>output_map;
   Dictionary *dict;

   // Initialize
   clear();

   // Allocate new VarInfo objects
   vx_pd.fcst_info = info_factory.new_var_info(ftype);
   vx_pd.obs_info  = info_factory.new_var_info(otype);

   // Set the VarInfo objects
   vx_pd.fcst_info->set_dict(fdict);
   vx_pd.obs_info->set_dict(odict);

   // Set the GRIB code for point observations
   if(!use_var_id) vx_pd.obs_info->add_grib_code(odict);

   // Dump the contents of the current VarInfo
   if(mlog.verbosity_level() >= 5) {
      mlog << Debug(5)
           << "Parsed forecast field:\n";
      vx_pd.fcst_info->dump(cout);
      mlog << Debug(5)
           << "Parsed observation field:\n";
      vx_pd.obs_info->dump(cout);
   }

   // Check the levels for the forecast and observation fields.  If the
   // forecast field is a range of pressure levels, check to see if the
   // range of observation field pressure levels is wholly contained in the
   // fcst levels.  If not, print a warning message.
   if(vx_pd.fcst_info->level().type() == LevelType_Pres &&
      !is_eq(vx_pd.fcst_info->level().lower(), vx_pd.fcst_info->level().upper()) &&
      (vx_pd.obs_info->level().lower() < vx_pd.fcst_info->level().lower() ||
       vx_pd.obs_info->level().upper() > vx_pd.fcst_info->level().upper())) {

      mlog << Warning
           << "\nEnsembleStatVxOpt::process_config() -> "
           << "The range of requested observation pressure levels "
           << "is not contained within the range of requested "
           << "forecast pressure levels.  No vertical interpolation "
           << "will be performed for observations falling outside "
           << "the range of forecast levels.  Instead, they will be "
           << "matched to the single nearest forecast level.\n\n";
   }

   // No support for wind direction
   if(vx_pd.fcst_info->is_wind_direction() ||
      vx_pd.obs_info->is_wind_direction()) {
      mlog << Error << "\nEnsembleStatVxOpt::process_config() -> "
           << "wind direction may not be verified using grid_stat.\n\n";
      exit(1);
   }

   // Check that the observation field does not contain probabilities
   if(vx_pd.obs_info->is_prob()) {
      mlog << Error << "\nEnsembleStatVxOpt::process_config() -> "
           << "the observation field cannot contain probabilities.\n\n";
      exit(1);
   }

   // Conf: nc_var_str
   var_str = parse_conf_string(&odict, conf_key_nc_var_str, false);

   // Conf: beg_ds and end_ds
   dict = odict.lookup_dictionary(conf_key_obs_window);
   parse_conf_range_int(dict, beg_ds, end_ds);

   // Conf: mask_grid
   mask_grid = odict.lookup_string_array(conf_key_mask_grid);

   // Conf: mask_poly
   mask_poly = odict.lookup_string_array(conf_key_mask_poly);

   // Conf: mask_sid
   mask_sid = odict.lookup_string_array(conf_key_mask_sid);

   // Conf: mask_llpnt
   mask_llpnt = parse_conf_llpnt_mask(&odict);

   // Conf: message_type
   msg_typ = parse_conf_message_type(&odict, point_vx);

   // Conf: othr_thresh
   othr_ta = process_perc_thresh_bins(
                odict.lookup_thresh_array(conf_key_obs_thresh));

   // Conf: climo_cdf
   cdf_info = parse_conf_climo_cdf(&odict);

   // Conf: ci_alpha
   ci_alpha = parse_conf_ci_alpha(&odict);

   // Conf: interp
   interp_info = parse_conf_interp(&odict, conf_key_interp);

   // Conf: output_flag
   output_map = parse_conf_output_flag(&odict, txt_file_type, n_txt);

   // Populate the output_flag array with map values
   for(i=0; i<n_txt; i++) output_flag[i] = output_map[txt_file_type[i]];

   // Conf: ssvar_bin_size
   ssvar_bin_size = odict.lookup_double(conf_key_ssvar_bin);

   // Conf: phist_bin_size
   phist_bin_size = odict.lookup_double(conf_key_phist_bin);

   // Conf: prob_cat_thresh
   prob_cat_ta = fdict.lookup_thresh_array(conf_key_prob_cat_thresh);

   // Conf: duplicate_flag
   duplicate_flag = parse_conf_duplicate_flag(&odict);

   // Conf: obs_summary
   obs_summary = parse_conf_obs_summary(&odict);

   // Conf: obs_perc_value
   obs_perc = parse_conf_percentile(&odict);

   // Conf: skip_const
   skip_const = odict.lookup_bool(conf_key_skip_const);

   // Conf: obs_error
   obs_error = parse_conf_obs_error(&odict, rng_ptr);
   vx_pd.obs_error_info = &obs_error;

   // Initialize the global instance of obs_error_table
   if(obs_error.flag && !obs_error_table.is_set()) {
      obs_error_table.initialize();
   }

   // Print debug information
   if(obs_error.entry.dist_type != DistType_None) {
      mlog << Debug(3)
           << "Observation error for point verification is "
           << "defined in the configuration file.\n";
   }
   else {
      mlog << Debug(3)
           << "Observation error for point verification is "
           << "defined by a table lookup for each observation.\n";
   }

   // Conf: desc
   vx_pd.set_desc(parse_conf_string(&odict, conf_key_desc).c_str());

   // Conf: sid_inc
   vx_pd.set_sid_inc_filt(parse_conf_sid_list(&odict, conf_key_sid_inc));

   // Conf: sid_exc
   vx_pd.set_sid_exc_filt(parse_conf_sid_list(&odict, conf_key_sid_exc));

   // Conf: obs_qty
   vx_pd.set_obs_qty_filt(parse_conf_obs_qty(&odict));

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatVxOpt::set_vx_pd(EnsembleStatConfInfo *conf_info) {
   int i, n;
   int n_msg_typ = msg_typ.n_elements();
   int n_mask    = mask_name.n_elements();
   int n_interp  = interp_info.n_interp;
   StringArray sa;

   // Setup the VxPairDataEnsemble object with these dimensions:
   // [n_msg_typ][n_mask][n_interp]

   // Check for at least one message type
   if(n_msg_typ == 0) {
      mlog << Error << "\nEnsembleStatVxOpt::set_vx_pd() -> "
           << "At least one output message type must be requested in \""
           << conf_key_message_type << "\".\n\n";
      exit(1);
   }

   // Check for at least one masking region
   if(n_mask == 0) {
      mlog << Error << "\nEnsembleStatVxOpt::set_vx_pd() -> "
           << "At least one output masking region must be requested in \""
           << conf_key_mask_grid  << "\", \""
           << conf_key_mask_poly  << "\", \""
           << conf_key_mask_sid   << "\", or \""
           << conf_key_mask_llpnt << "\".\n\n";
      exit(1);
   }

   // Check for at least one interpolation method
   if(n_interp == 0) {
      mlog << Error << "\nEnsembleStatVxOpt::set_vx_pd() -> "
           << "At least one interpolation method must be requested in \""
           << conf_key_interp << "\".\n\n";
      exit(1);
   }

   // Define the dimensions
   vx_pd.set_pd_size(n_msg_typ, n_mask, n_interp);

   // Store the list of surface message types
   vx_pd.set_msg_typ_sfc(conf_info->msg_typ_sfc);

   // Define the verifying message type name and values
   for(i=0; i<n_msg_typ; i++) {
      vx_pd.set_msg_typ(i, msg_typ[i].c_str());
      sa = conf_info->msg_typ_group_map[msg_typ[i]];
      if(sa.n_elements() == 0) sa.add(msg_typ[i]);
      vx_pd.set_msg_typ_vals(i, sa);
   }

   // Define the masking information: grid, poly, sid

   // Define the grid masks
   for(i=0; i<mask_grid.n_elements(); i++) {
      n = i;
      vx_pd.set_mask_area(n, mask_name[n].c_str(),
                          &(conf_info->mask_area_map[mask_name[n]]));
   }

   // Define the poly masks
   for(i=0; i<mask_poly.n_elements(); i++) {
      n = i + mask_grid.n_elements();
      vx_pd.set_mask_area(n, mask_name[n].c_str(),
                          &(conf_info->mask_area_map[mask_name[n]]));
   }

   // Define the station ID masks
   for(i=0; i<mask_sid.n_elements(); i++) {
      n = i + mask_grid.n_elements() + mask_poly.n_elements();
      vx_pd.set_mask_sid(n, mask_name[n].c_str(),
                         &(conf_info->mask_sid_map[mask_name[n]]));
   }

   // Define the Lat/Lon point masks
   for(i=0; i<(int) mask_llpnt.size(); i++) {
      n = i + mask_grid.n_elements() + mask_poly.n_elements() + mask_sid.n_elements();
      vx_pd.set_mask_llpnt(n, mask_name[n].c_str(), &mask_llpnt[i]);
   }

   // Define the interpolation methods
   for(i=0; i<n_interp; i++) {
      vx_pd.set_interp(i, interp_info.method[i].c_str(), interp_info.width[i],
                       interp_info.shape);
   }

   // After sizing VxPairDataEnsemble, add settings for each array element
   vx_pd.set_ssvar_bin_size(ssvar_bin_size);
   vx_pd.set_phist_bin_size(phist_bin_size);
   vx_pd.set_duplicate_flag(duplicate_flag);
   vx_pd.set_obs_summary(obs_summary);
   vx_pd.set_obs_perc_value(obs_perc);
   vx_pd.set_skip_const(skip_const);

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatVxOpt::set_perc_thresh(const PairDataEnsemble *pd_ptr) {

   if(!othr_ta.need_perc()) return;

   //
   // Sort the input arrays
   //
   NumArray fsort;
   for(int i=0; i<pd_ptr->n_ens; i++) fsort.add(pd_ptr->e_na[i]);
   NumArray osort = pd_ptr->o_na;
   NumArray csort = pd_ptr->cmn_na;
   fsort.sort_array();
   osort.sort_array();
   csort.sort_array();

   //
   // Compute percentiles, passing the observation thresholds in for
   // the fcst and obs slots.
   //
   othr_ta.set_perc(&fsort, &osort, &csort, &othr_ta, &othr_ta);

   return;
}

////////////////////////////////////////////////////////////////////////

int EnsembleStatVxOpt::n_txt_row(int i_txt_row) const {
   int n = 0;
   int n_bin;

   // Range check
   if(i_txt_row < 0 || i_txt_row >= n_txt) {
      mlog << Error << "\nEnsembleStatVxOpt::n_txt_row(int) -> "
           << "range check error for " << i_txt_row << "\n\n";
      exit(1);
   }

   // Check if this output line type is requested
   if(output_flag[i_txt_row] == STATOutputType_None) return(0);

   // Determine row multiplier for climatology bins
   if(cdf_info.write_bins) {
      n_bin = get_n_cdf_bin();
      if(n_bin > 1) n_bin++;
   }
   else {
      n_bin = 1;
   }

   // Switch on the index of the line type
   switch(i_txt_row) {

      case(i_ecnt):
      case(i_rps):

         // Maximum number of ECNT and RPS lines possible =
         //    Point Vx: Message Types * Masks * Interpolations *
         //                              Obs Thresholds * Climo Bins
         //     Grid Vx:                 Masks * Interpolations *
         //                              Obs Thresholds * Climo Bins
         n = (get_n_msg_typ() + 1) * get_n_mask() * get_n_interp() *
              get_n_o_thresh() * n_bin;
         break;

      case(i_rhist):
      case(i_phist):
      case(i_relp):

         // Maximum number of RHIST, PHIST, and RELP lines possible =
         //    Point Vx: Message Types * Masks * Interpolations * Obs Thresholds
         //     Grid Vx:                 Masks * Interpolations * Obs Thresholds
         n = (get_n_msg_typ() + 1) * get_n_mask() * get_n_interp() *
              get_n_o_thresh();
         break;

      case(i_orank):

         // Compute the maximum number of matched pairs to be written
         // out by summing the number for each VxPairDataEnsemble object
         n = vx_pd.get_n_pair() * get_n_o_thresh();
         break;

      case(i_ssvar):

         // Just return zero since we'll resize the output AsciiTables
         // to accomodate the SSVAR output
         n = 0;
         break;

      default:
         mlog << Error << "\nEnsembleStatVxOpt::n_txt_row(int) -> "
              << "unexpected output type index value: " << i_txt_row
              << "\n\n";
         exit(1);
         break;
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for struct EnsembleStatNcOutInfo
//
////////////////////////////////////////////////////////////////////////

EnsembleStatNcOutInfo::EnsembleStatNcOutInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatNcOutInfo::clear() {

   set_all_true();

   return;
}

////////////////////////////////////////////////////////////////////////

bool EnsembleStatNcOutInfo::all_false() const {

   bool status = do_latlon || do_mean || do_stdev || do_minus ||
                 do_plus   || do_min  || do_max   || do_range ||
                 do_vld    || do_freq || do_nep   || do_nmep  ||
                 do_orank  || do_weight;

   return(!status);
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatNcOutInfo::set_all_false() {

   do_latlon = false;
   do_mean   = false;
   do_stdev  = false;
   do_minus  = false;
   do_plus   = false;
   do_min    = false;
   do_max    = false;
   do_range  = false;
   do_vld    = false;
   do_freq   = false;
   do_nep    = false;
   do_nmep   = false;
   do_orank  = false;
   do_weight = false;

   return;
}


////////////////////////////////////////////////////////////////////////

void EnsembleStatNcOutInfo::set_all_true() {

   do_latlon = true;
   do_mean   = true;
   do_stdev  = true;
   do_minus  = true;
   do_plus   = true;
   do_min    = true;
   do_max    = true;
   do_range  = true;
   do_vld    = true;
   do_freq   = true;
   do_nep    = true;
   do_nmep   = true;
   do_orank  = true;
   do_weight = true;

   return;
}

////////////////////////////////////////////////////////////////////////

