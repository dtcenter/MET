// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
   vx_opt   = (EnsembleStatVxOpt *) nullptr;
   rng_ptr  = (gsl_rng *)           nullptr;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::clear() {
   int i;

   // Initialize values
   model.clear();
   grib_codes_set = false;
   obtype.clear();
   vld_ens_thresh = bad_data_double;
   vld_data_thresh = bad_data_double;
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
   if(vx_opt) { delete [] vx_opt; vx_opt = (EnsembleStatVxOpt *) nullptr; }

   // Reset counts
   n_vx          = 0;
   max_hira_size = 0;

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
                                          StringArray * ens_files,
                                          bool use_ctrl) {
   int i, j, n_ens_files;
   VarInfoFactory info_factory;
   map<STATLineType,STATOutputType>output_map;
   Dictionary *fdict = (Dictionary *) nullptr;
   Dictionary *odict  = (Dictionary *) nullptr;
   Dictionary i_fdict, i_odict;
   InterpMthd mthd;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Initialize
   clear();

   n_ens_files = ens_files->n();

   // Unset MET_ENS_MEMBER_ID in case it is set by the user
   unsetenv(met_ens_member_id);

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
   ConcatString cs = surface_msg_typ_group_str;
   if(msg_typ_group_map.count(cs) > 0) {
      msg_typ_sfc = msg_typ_group_map[cs];
   }
   else {
      msg_typ_sfc.parse_css(default_msg_typ_group_surface);
   }

   // Conf: ens_member_ids
   ens_member_ids = parse_conf_ens_member_ids(&conf);

   // Conf: control_id
   control_id = parse_conf_string(&conf, conf_key_control_id, false);

   // Error check ens_member_ids and ensemble file list
   if(ens_member_ids.n() > 1) {

      // Only a single file should be provided if using ens_member_ids
      if(ens_files->n() > 1) {
         mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
              << "the \"" << conf_key_ens_member_ids << "\" "
              << "must be empty if more than one file is provided.\n\n";
         exit(1);
      }

      // The control ID must be set when the control file is specified
      if(control_id.empty() && use_ctrl) {
         mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
              << "the control_id must be set if processing a single input "
              << "file with the -ctrl option\n\n";
         exit(1);
      }

      // If control ID is set, it cannot be found in ens_member_ids
      if(!control_id.empty() && ens_member_ids.has(control_id)) {
         mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
              << "control_id (" << control_id << ") must not be found "
              << "in ens_member_ids\n\n";
         exit(1);
      }
   }

   // If no ensemble member IDs were provided, add an empty string
   if(ens_member_ids.n() == 0) ens_member_ids.add("");

   // Conf: ens, print warning if present
   if(conf.lookup_dictionary(conf_key_ens, false, false)) {
      mlog << Warning << "\nEnsembleStatConfInfo::process_config() -> "
           << "support for ensemble product generation with the \"ens\" "
           << "dictionary has moved to the Gen-Ens-Prod tool." << "\n\n";
   }

   // Conf: fcst.ens_thresh
   vld_ens_thresh = conf.lookup_double(conf_key_fcst_ens_thresh);

   // Check that the valid ensemble threshold is between 0 and 1.
   if(vld_ens_thresh < 0.0 || vld_ens_thresh > 1.0) {
      mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
           << "The \"" << conf_key_fcst_ens_thresh << "\" parameter ("
           << vld_ens_thresh << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   // Conf: fcst.vld_thresh
   vld_data_thresh = conf.lookup_double(conf_key_fcst_vld_thresh);

   // Check that the valid data threshold is between 0 and 1.
   if(vld_data_thresh < 0.0 || vld_data_thresh > 1.0) {
      mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
           << "The \"" << conf_key_fcst_vld_thresh << "\" parameter ("
           << vld_data_thresh << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   // Conf: fcst.field and obs.field
   fdict = conf.lookup_array(conf_key_fcst_field);
   odict = conf.lookup_array(conf_key_obs_field);

   // Determine the number of fields (name/level) to be verified
   int n_fvx = parse_conf_n_vx(fdict);
   int n_ovx = parse_conf_n_vx(odict);

   // Check for a valid number of verification tasks
   if(n_fvx == 0 || n_fvx != n_ovx) {
      mlog << Error << "\nEnsembleStatConfInfo::process_config() -> "
           << "The number of \"" << conf_key_obs_field << "\" entries ("
           << n_ovx << ") must be greater than zero and match "
           << "the number of \"" << conf_key_fcst_field << "\" entries ("
           << n_fvx << ").\n\n";
      exit(1);
   }

   // Allocate memory for the verification task options
   n_vx   = n_fvx;
   vx_opt = new EnsembleStatVxOpt [n_vx];

   // Check climatology fields
   check_climo_n_vx(&conf, n_vx);

   // Parse settings for each verification task
   for(i=0,max_hira_size=0; i<n_vx; i++) {

      // Get the current dictionaries
      i_fdict = parse_conf_i_vx_dict(fdict, i);
      i_odict = parse_conf_i_vx_dict(odict, i);

      // Process the options for this verification task
      vx_opt[i].process_config(etype, i_fdict, otype, i_odict,
                               rng_ptr, grid_vx, point_vx, ens_member_ids,
                               ens_files, use_ctrl, control_id);

      // For no point verification, store obtype as the message type
      if(!point_vx) {
         vx_opt[i].msg_typ.clear();
         vx_opt[i].msg_typ.add(obtype);
      }

      // Track the maximum HiRA size
      for(j=0; j<vx_opt[i].interp_info.n_interp; j++) {
         if(string_to_interpmthd(vx_opt[i].interp_info.method[j].c_str()) == InterpMthd_HiRA) {
            GridTemplateFactory gtf;
            GridTemplate* gt = gtf.buildGT(vx_opt[i].interp_info.shape,
                                           vx_opt[i].interp_info.width[j],
                                           false);
            max_hira_size = max(max_hira_size, gt->size());
         }
      }
   }

   // Summarize output flags across all verification tasks
   process_flags();

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::process_grib_codes() {

   // Only needs to be set once
   if(grib_codes_set) return;

   mlog << Debug(3) << "Processing each \"" << conf_key_obs_field
        << "\" name as a GRIB code abbreviation since the point "
        << "observations are specified as GRIB codes.\n";

   Dictionary *odict = conf.lookup_array(conf_key_obs_field);
   Dictionary i_odict;

   // Add the GRIB code by parsing each observation dictionary
   for(int i=0; i<n_vx; i++) {
      i_odict = parse_conf_i_vx_dict(odict, i);
      vx_opt[i].vx_pd.obs_info->add_grib_code(i_odict);
   }

   // Flag to prevent processing more than once
   grib_codes_set = true;

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::process_flags() {
   int i, j;
   bool output_ascii_flag = false;

   // Initialize
   for(i=0; i<n_txt; i++) output_flag[i] = STATOutputType_None;
   nc_info.set_all_false();

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

      // Summary of nc_info flag settings
      if(vx_opt[i].nc_info.do_latlon) nc_info.do_latlon = true;
      if(vx_opt[i].nc_info.do_mean)   nc_info.do_mean   = true;
      if(vx_opt[i].nc_info.do_raw)    nc_info.do_raw    = true;
      if(vx_opt[i].nc_info.do_rank)   nc_info.do_rank   = true;
      if(vx_opt[i].nc_info.do_pit)    nc_info.do_pit    = true;
      if(vx_opt[i].nc_info.do_vld)    nc_info.do_vld    = true;
      if(vx_opt[i].nc_info.do_weight) nc_info.do_weight = true;
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
      for(j=0; j<vx_opt[i].mask_grid.n(); j++) {

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
      for(j=0; j<vx_opt[i].mask_poly.n(); j++) {

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
      for(j=0; j<vx_opt[i].mask_sid.n(); j++) {

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
      if(vx_opt[i].mask_name.n() == 0) {
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

int EnsembleStatConfInfo::get_max_n_prob_cat_thresh() const {
   int i, n;

   for(i=0,n=0; i<n_vx; i++) n = max(n, vx_opt[i].get_n_prob_cat_thresh());

   return(n);
}

////////////////////////////////////////////////////////////////////////

int EnsembleStatConfInfo::get_max_n_prob_pct_thresh() const {
   int i, n;

   for(i=0,n=0; i<n_vx; i++) n = max(n, vx_opt[i].get_n_prob_pct_thresh());

   return(n);
}

////////////////////////////////////////////////////////////////////////

int EnsembleStatConfInfo::get_max_n_eclv_points() const {
   int i, n;

   for(i=0,n=0; i<n_vx; i++) n = max(n, vx_opt[i].get_n_eclv_points());

   return(n);
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::set_vx_pd(const IntArray &ens_size, int ctrl_index) {

   // This should be called after process_masks()
   for(int i=0; i<n_vx; i++) {

      vx_opt[i].set_vx_pd(this, ctrl_index);

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
   eclv_points.clear();
   cdf_info.clear();

   ci_alpha.clear();
   interp_info.clear();

   ssvar_bin_size = bad_data_double;
   phist_bin_size = bad_data_double;

   fcat_ta.clear();
   ocat_ta.clear();
   fpct_ta.clear();

   duplicate_flag = DuplicateType_None;
   obs_summary = ObsSummary_None;
   obs_perc = bad_data_int;
   skip_const = false;
   obs_error.clear();

   for(i=0; i<n_txt; i++) output_flag[i] = STATOutputType_None;

   nc_info.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatVxOpt::process_config(GrdFileType ftype, Dictionary &fdict,
                                       GrdFileType otype, Dictionary &odict,
                                       gsl_rng *rng_ptr, bool grid_vx, bool point_vx,
                                       StringArray ens_member_ids,
                                       StringArray * ens_files,
                                       bool use_ctrl, ConcatString control_id) {
   int i, j;
   VarInfoFactory info_factory;
   map<STATLineType,STATOutputType>output_map;
   Dictionary *dict;
   VarInfo * next_var;
   InputInfo input_info;

   // Initialize
   clear();

   // Allocate new EnsVarInfo object for fcst
   vx_pd.fcst_info = new EnsVarInfo();

   // Loop over ensemble member IDs to substitute
   for(i=0; i<ens_member_ids.n(); i++) {

      // set environment variable for ens member ID
      setenv(met_ens_member_id, ens_member_ids[i].c_str(), 1);

      // Allocate new VarInfo object
      next_var = info_factory.new_var_info(ftype);

      // Set the current dictionary
      next_var->set_dict(fdict);

      input_info.var_info = next_var;
      input_info.file_index = 0;
      input_info.file_list = ens_files;
      vx_pd.fcst_info->add_input(input_info);

      // Add InputInfo to fcst info list for each ensemble file provided
      // set var_info to nullptr to note first VarInfo should be used
      int last_member_index = ens_files->n() - (use_ctrl ? 1 : 0);
      for(j=1; j<last_member_index; j++) {
         input_info.var_info = nullptr;
         input_info.file_index = j;
         input_info.file_list = ens_files;
         vx_pd.fcst_info->add_input(input_info);
      } // end for j
   } // end for i

   // Add control member as the last input
   if(use_ctrl) {

      // Set environment variable for ens member ID
      setenv(met_ens_member_id, control_id.c_str(), 1);

      // Allocate new VarInfo object
      next_var = info_factory.new_var_info(ftype);

      // Set the current dictionary
      next_var->set_dict(fdict);

      input_info.var_info = next_var;
      input_info.file_index = ens_files->n() - 1;
      input_info.file_list = ens_files;
      vx_pd.fcst_info->add_input(input_info);
   }

   // Allocate new VarInfo object for obs
   vx_pd.obs_info  = info_factory.new_var_info(otype);

   // Set the VarInfo objects
   vx_pd.obs_info->set_dict(odict);

   // Dump the contents of the current VarInfo
   if(mlog.verbosity_level() >= 5) {
      mlog << Debug(5)
           << "Parsed forecast field:\n";
      vx_pd.fcst_info->get_var_info()->dump(cout);
      mlog << Debug(5)
           << "Parsed observation field:\n";
      vx_pd.obs_info->dump(cout);
   }

   // Check the levels for the forecast and observation fields.  If the
   // forecast field is a range of pressure levels, check to see if the
   // range of observation field pressure levels is wholly contained in the
   // fcst levels.  If not, print a warning message.
   if(vx_pd.fcst_info->get_var_info()->level().type() == LevelType_Pres &&
      !is_eq(vx_pd.fcst_info->get_var_info()->level().lower(), vx_pd.fcst_info->get_var_info()->level().upper()) &&
      (vx_pd.obs_info->level().lower() < vx_pd.fcst_info->get_var_info()->level().lower() ||
       vx_pd.obs_info->level().upper() > vx_pd.fcst_info->get_var_info()->level().upper())) {

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
   if(vx_pd.fcst_info->get_var_info()->is_wind_direction() ||
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

   // Conf: eclv_points
   eclv_points = parse_conf_eclv_points(&odict);

   // Conf: climo_cdf
   cdf_info = parse_conf_climo_cdf(&odict);

   // Conf: ci_alpha
   ci_alpha = parse_conf_ci_alpha(&odict);

   // Conf: interp
   interp_info = parse_conf_interp(&odict, conf_key_interp);

   // Conf: output_flag
   output_map = parse_conf_output_flag(&odict, txt_file_type, n_txt);

   // Conf: nc_orank_flag
   parse_nc_info(odict);

   // Populate the output_flag array with map values
   for(i=0; i<n_txt; i++) output_flag[i] = output_map[txt_file_type[i]];

   // Conf: ssvar_bin_size
   ssvar_bin_size = odict.lookup_double(conf_key_ssvar_bin);

   // Conf: phist_bin_size
   phist_bin_size = odict.lookup_double(conf_key_phist_bin);

   // Conf: prob_cat_thresh
   fcat_ta = fdict.lookup_thresh_array(conf_key_prob_cat_thresh);
   ocat_ta = odict.lookup_thresh_array(conf_key_prob_cat_thresh);

   // The number of thresholds must match
   if(fcat_ta.n() != ocat_ta.n()) {
      mlog << Error << "\nEnsembleStatVxOpt::process_config() -> "
           << "The number of forecast (" << fcat_ta.n()
           << ") and observation (" << ocat_ta.n()
           << ") probability category thresholds in \""
           << conf_key_prob_cat_thresh << "\" must match.\n\n";
      exit(1);
   }

   // Conf: prob_pct_thresh
   fpct_ta = fdict.lookup_thresh_array(conf_key_prob_pct_thresh);
   fpct_ta = string_to_prob_thresh(fpct_ta.get_str().c_str());

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
   
   // Conf: obs_qty_inc
   vx_pd.set_obs_qty_inc_filt(parse_conf_obs_qty_inc(&odict));
   
   // Conf: obs_qty_exc
   vx_pd.set_obs_qty_exc_filt(parse_conf_obs_qty_exc(&odict));
   
   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatVxOpt::parse_nc_info(Dictionary &odict) {
   const DictionaryEntry * e = (const DictionaryEntry *) nullptr;

   e = odict.lookup(conf_key_nc_orank_flag);

   if(!e) {
      mlog << Error
           << "\nEnsembleStatVxOpt::parse_nc_info() -> "
           << "lookup failed for key \"" << conf_key_nc_orank_flag
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
           << "\nEnsembleStatVxOpt::parse_nc_info() -> "
           << "bad type (" << configobjecttype_to_string(type)
           << ") for key \"" << conf_key_nc_orank_flag << "\"\n\n";
      exit(1);
   }

   // Parse the various entries
   Dictionary * d = e->dict_value();

   nc_info.do_latlon = d->lookup_bool(conf_key_latlon_flag);
   nc_info.do_mean   = d->lookup_bool(conf_key_mean_flag);
   nc_info.do_raw    = d->lookup_bool(conf_key_raw_flag);
   nc_info.do_rank   = d->lookup_bool(conf_key_rank_flag);
   nc_info.do_pit    = d->lookup_bool(conf_key_pit_flag);
   nc_info.do_vld    = d->lookup_bool(conf_key_vld_count_flag);
   nc_info.do_weight = d->lookup_bool(conf_key_weight);

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatVxOpt::set_vx_pd(EnsembleStatConfInfo *conf_info, int ctrl_index) {
   int i, j, n;
   int n_msg_typ = msg_typ.n();
   int n_mask    = mask_name.n();
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

   // Store the climo CDF info
   vx_pd.set_climo_cdf_info_ptr(&cdf_info);

   // Store the list of surface message types
   vx_pd.set_msg_typ_sfc(conf_info->msg_typ_sfc);

   // Define the verifying message type name and values
   for(i=0; i<n_msg_typ; i++) {
      vx_pd.set_msg_typ(i, msg_typ[i].c_str());
      sa = conf_info->msg_typ_group_map[msg_typ[i]];
      if(sa.n() == 0) sa.add(msg_typ[i]);
      vx_pd.set_msg_typ_vals(i, sa);
   }

   // Define the masking information: grid, poly, sid

   // Define the grid masks
   for(i=0; i<mask_grid.n(); i++) {
      n = i;
      vx_pd.set_mask_area(n, mask_name[n].c_str(),
                          &(conf_info->mask_area_map[mask_name[n]]));
   }

   // Define the poly masks
   for(i=0; i<mask_poly.n(); i++) {
      n = i + mask_grid.n();
      vx_pd.set_mask_area(n, mask_name[n].c_str(),
                          &(conf_info->mask_area_map[mask_name[n]]));
   }

   // Define the station ID masks
   for(i=0; i<mask_sid.n(); i++) {
      n = i + mask_grid.n() + mask_poly.n();
      vx_pd.set_mask_sid(n, mask_name[n].c_str(),
                         &(conf_info->mask_sid_map[mask_name[n]]));
   }

   // Define the Lat/Lon point masks
   for(i=0; i<(int) mask_llpnt.size(); i++) {
      n = i + mask_grid.n() + mask_poly.n() + mask_sid.n();
      vx_pd.set_mask_llpnt(n, mask_name[n].c_str(), &mask_llpnt[i]);
   }

   // Define the interpolation methods
   for(i=0; i<interp_info.n_interp; i++) {
      vx_pd.set_interp(i, interp_info.method[i].c_str(), interp_info.width[i],
                       interp_info.shape);
      vx_pd.set_interp_thresh(interp_info.vld_thresh);
   }

   // After sizing VxPairDataEnsemble, add settings for each array element
   vx_pd.set_ssvar_bin_size(ssvar_bin_size);
   vx_pd.set_phist_bin_size(phist_bin_size);
   vx_pd.set_duplicate_flag(duplicate_flag);
   vx_pd.set_obs_summary(obs_summary);
   vx_pd.set_obs_perc_value(obs_perc);
   vx_pd.set_ctrl_index(ctrl_index);
   vx_pd.set_skip_const(skip_const);

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatVxOpt::set_perc_thresh(const PairDataEnsemble *pd_ptr) {

   //
   // Check if percentile thresholds were requested
   //
   if(!othr_ta.need_perc() &&
      !fcat_ta.need_perc() &&
      !ocat_ta.need_perc()) return;

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
   // Compute percentiles, passing the observation filtering
   // thresholds in for the fcst and obs slots.
   //
   othr_ta.set_perc(&fsort, &osort, &csort, &othr_ta, &othr_ta);
   fcat_ta.set_perc(&fsort, &osort, &csort, &fcat_ta, &ocat_ta);
   ocat_ta.set_perc(&fsort, &osort, &csort, &fcat_ta, &ocat_ta);

   return;
}

////////////////////////////////////////////////////////////////////////

int EnsembleStatVxOpt::n_txt_row(int i_txt_row) const {
   int n = 0;

   // Range check
   if(i_txt_row < 0 || i_txt_row >= n_txt) {
      mlog << Error << "\nEnsembleStatVxOpt::n_txt_row(int) -> "
           << "range check error for " << i_txt_row << "\n\n";
      exit(1);
   }

   // Check if this output line type is requested
   if(output_flag[i_txt_row] == STATOutputType_None) return(0);

   // Switch on the index of the line type
   switch(i_txt_row) {

      case(i_ecnt):
      case(i_rps):

         // Maximum number of ECNT and RPS lines possible =
         //    Point Vx: Message Types * Masks * Interpolations * Obs Thresholds * Alphas
         //     Grid Vx:                 Masks * Interpolations * Obs Thresholds * Alphas
         n = (get_n_msg_typ() + 1) * get_n_mask() * get_n_interp() *
              get_n_obs_thresh() * get_n_ci_alpha();
         break;

      case(i_rhist):
      case(i_phist):
      case(i_relp):

         // Maximum number of RHIST, PHIST, and RELP lines possible =
         //    Point Vx: Message Types * Masks * Interpolations * Obs Thresholds
         //     Grid Vx:                 Masks * Interpolations * Obs Thresholds
         n = (get_n_msg_typ() + 1) * get_n_mask() * get_n_interp() *
              get_n_obs_thresh();
         break;

      case(i_orank):

         // Compute the maximum number of matched pairs to be written
         // out by summing the number for each VxPairDataEnsemble object

         // Number of ORANK lines possible =
         //    Number of pairs * Obs Thresholds
         n = vx_pd.get_n_pair() * get_n_obs_thresh();
         break;

      case(i_ssvar):

         // Just return zero since we'll resize the output AsciiTables
         // to accomodate the SSVAR output
         n = 0;
         break;

      case(i_pct):
      case(i_pjc):
      case(i_prc):

         // Maximum number of PCT, PJC, and PRC lines possible =
         //    Point Vx: Message Types * Masks * Interpolations * Categorical Thresholds
         //     Grid Vx:                 Masks * Interpolations * Categorical Thresholds
         n = (get_n_msg_typ() + 1) * get_n_mask() * get_n_interp() *
              get_n_prob_cat_thresh();
         break;

      case(i_pstd):

         // Maximum number of PSTD lines possible =
         //    Point Vx: Message Types * Masks * Interpolations * Categorical Thresholds * Alphas
         //     Grid Vx:                 Masks * Interpolations * Categorical Thresholds * Alphas
         n = (get_n_msg_typ() + 1) * get_n_mask() * get_n_interp() *
              get_n_prob_cat_thresh() * get_n_ci_alpha();
         break;

      case(i_eclv):

         // Maximum number of ECLV lines possible =
         //    Point Vx: Message Types * Masks * Interpolations * Probability Thresholds
         //     Grid Vx:                 Masks * Interpolations * Probability Thresholds
         n = (get_n_msg_typ() + 1) * get_n_mask() * get_n_interp() *
              get_n_prob_cat_thresh() * get_n_prob_cat_thresh();
         break;

      default:
         mlog << Error << "\nEnsembleStatVxOpt::n_txt_row(int) -> "
              << "unexpected output type index value: " << i_txt_row
              << "\n\n";
         exit(1);
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////

int EnsembleStatVxOpt::get_n_prob_cat_thresh() const {

   // Probability categories can be defined by the prob_cat_thresh or
   // climo_cdf.bins configuration file options.
   return(max(fcat_ta.n(), cdf_info.cdf_ta.n()));
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

   bool status = do_latlon || do_mean || do_raw ||
                 do_rank   || do_pit  || do_vld ||
                 do_weight;

   return(!status);
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatNcOutInfo::set_all_false() {

   do_latlon = false;
   do_mean   = false;
   do_raw    = false;
   do_rank   = false;
   do_pit    = false;
   do_vld    = false;
   do_weight = false;

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatNcOutInfo::set_all_true() {

   do_latlon = true;
   do_mean   = true;
   do_raw    = true;
   do_rank   = true;
   do_pit    = true;
   do_vld    = true;
   do_weight = true;

   return;
}

////////////////////////////////////////////////////////////////////////
