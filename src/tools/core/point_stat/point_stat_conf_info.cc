// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

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

using namespace std;


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
   vx_opt = (PointStatVxOpt *) nullptr;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::clear() {

   // Initialize values
   model.clear();
   grib_codes_set = false;
   land_mask.clear();
   topo_dp.clear();
   topo_use_obs_thresh.clear();
   topo_interp_fcst_thresh.clear();
   msg_typ_group_map.clear();
   mask_area_map.clear();
   mask_sid_map.clear();
   tmp_dir.clear();
   output_prefix.clear();
   version.clear();
#ifdef WITH_UGRID
   ugrid_nc.clear();
   ugrid_dataset.clear();
   ugrid_map_config.clear();
   ugrid_max_distance_km = bad_data_double;
#endif
   // Deallocate memory
   if(vx_opt) { delete [] vx_opt; vx_opt = (PointStatVxOpt *) nullptr; }

   // Set count to zero
   n_vx = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::read_config(const char *default_file_name,
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

void PointStatConfInfo::read_configs(StringArray user_file_names) {

   const char *file_name;
   for (int i=0; i<user_file_names.n_elements(); i++) {
      file_name = replace_path(user_file_names[i].c_str()).c_str();
      if (file_exists(file_name)) conf.read(file_name);
      else mlog << Warning << "\nPointStatConfInfo::read_configs(StringArray) -> "
                << "The configuration file \"" << user_file_names[i]<< "\" does not exist.\n\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::process_config(GrdFileType ftype) {
   int i, j, n_fvx, n_ovx;
   Dictionary *fdict = (Dictionary *) nullptr;
   Dictionary *odict = (Dictionary *) nullptr;
   Dictionary i_fdict, i_odict;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Initialize
   clear();

   // Conf: version
   version = parse_conf_version(&conf);

   // Conf: model
   model = parse_conf_string(&conf, conf_key_model);

   // Conf: tmp_dir
   tmp_dir = parse_conf_tmp_dir(&conf);

#ifdef WITH_UGRID
   // Conf: ugrid_dataset
   ugrid_dataset = parse_conf_ugrid_dataset(&conf);

   // Conf: ugrid_nc
   ugrid_nc = parse_conf_ugrid_coordinates_file(&conf);

   // Conf: ugrid_map_config
   ugrid_map_config = parse_conf_ugrid_map_config(&conf);

   // Conf: ugrid_max_distance_km
   ugrid_max_distance_km = parse_conf_ugrid_max_distance_km(&conf);
#endif

   // Conf: output_prefix
   output_prefix = conf.lookup_string(conf_key_output_prefix);

   // Conf: message_type_group_map
   msg_typ_group_map = parse_conf_message_type_group_map(&conf);

   // Conf: fcst.field and obs.field
   fdict = conf.lookup_array(conf_key_fcst_field);
   odict = conf.lookup_array(conf_key_obs_field);

   // Determine the number of fields (name/level) to be verified
   n_fvx = parse_conf_n_vx(fdict);
   n_ovx = parse_conf_n_vx(odict);

   // Check for a valid number of verification tasks
   if(n_fvx == 0 || n_fvx != n_ovx) {
      mlog << Error << "\nPointStatConfInfo::process_config() -> "
           << "The number of verification tasks in \""
           << conf_key_obs_field << "\" (" << n_ovx
           << ") must be non-zero and match the number in \""
           << conf_key_fcst_field << "\" (" << n_fvx << ").\n\n";
      exit(1);
   }

   // Allocate memory for the verification task options
   n_vx   = n_fvx;
   vx_opt = new PointStatVxOpt [n_vx];

   // Check for consistent number of climatology fields
   check_climo_n_vx(&conf, n_vx);

   // Parse settings for each verification task
   for(i=0; i<n_vx; i++) {

      // Get the current dictionaries
      i_fdict = parse_conf_i_vx_dict(fdict, i);
      i_odict = parse_conf_i_vx_dict(odict, i);

      // Process the options for this verification task
      vx_opt[i].process_config(ftype, i_fdict, i_odict);
   }

   // Summarize output flags across all verification tasks
   process_flags();

   // If VL1L2, VAL1L2, or VCNT is requested, set the uv_index.
   // When processing vectors, need to make sure the message types,
   // masking regions, and interpolation methods are consistent.
   if(output_flag[i_vl1l2]  != STATOutputType_None ||
      output_flag[i_val1l2] != STATOutputType_None ||
      output_flag[i_vcnt]   != STATOutputType_None) {

      for(i=0; i<n_vx; i++) {

         // Process u-wind
         if(vx_opt[i].vx_pd.fcst_info->is_u_wind() &&
            vx_opt[i].vx_pd.obs_info->is_u_wind()) {

            // Search for corresponding v-wind
            for(j=0; j<n_vx; j++) {
               if(vx_opt[j].vx_pd.fcst_info->is_v_wind() &&
                  vx_opt[j].vx_pd.obs_info->is_v_wind()  &&
                  vx_opt[i].is_uv_match(vx_opt[j])) {

                  mlog << Debug(3) << "U-wind field array entry " << i+1
                       << " matches V-wind field array entry " << j+1 << ".\n";

                  // Print warning about multiple matches
                  if(vx_opt[i].vx_pd.fcst_info->uv_index() >= 0 ||
                     vx_opt[i].vx_pd.obs_info->uv_index()  >= 0) {
                     mlog << Warning << "\nPointStatConfInfo::process_config() -> "
                          << "For U-wind, found multiple matching V-wind field array entries! "
                          << "Using the first match found. Set the \"level\" strings to "
                          << "differentiate between them.\n\n";
                  }
                  // Use the first match
                  else {
                     vx_opt[i].vx_pd.fcst_info->set_uv_index(j);
                     vx_opt[i].vx_pd.obs_info->set_uv_index(j);
                  }
               }
            }

            // No match found
            if(vx_opt[i].vx_pd.fcst_info->uv_index() < 0 ||
               vx_opt[i].vx_pd.obs_info->uv_index()  < 0) {
               mlog << Debug(3) << "U-wind field array entry " << i+1
                    << " has no matching V-wind field array entry.\n";
            }

         }
         // Process v-wind
         else if(vx_opt[i].vx_pd.fcst_info->is_v_wind() &&
                 vx_opt[i].vx_pd.obs_info->is_v_wind()) {

            // Search for corresponding u-wind
            for(j=0; j<n_vx; j++) {
               if(vx_opt[j].vx_pd.fcst_info->is_u_wind() &&
                  vx_opt[j].vx_pd.obs_info->is_u_wind()  &&
                  vx_opt[i].is_uv_match(vx_opt[j])) {

                  mlog << Debug(3) << "V-wind field array entry " << i+1
                       << " matches U-wind field array entry " << j+1 << ".\n";

                  // Print warning about multiple matches
                  if(vx_opt[i].vx_pd.fcst_info->uv_index() >= 0 ||
                     vx_opt[i].vx_pd.obs_info->uv_index()  >= 0) {
                     mlog << Warning << "\nPointStatConfInfo::process_config() -> "
                          << "For U-wind, found multiple matching V-wind field array entries! "
                          << "Using the first match found. Set the \"level\" strings to "
                          << "differentiate between them.\n\n";
                  }
                  // Use the first match
                  else {
                     vx_opt[i].vx_pd.fcst_info->set_uv_index(j);
                     vx_opt[i].vx_pd.obs_info->set_uv_index(j);
                  }
               }
            }

            // No match found
            if(vx_opt[i].vx_pd.fcst_info->uv_index() < 0 ||
               vx_opt[i].vx_pd.obs_info->uv_index()  < 0) {
               mlog << Debug(3) << "V-wind field array entry " << i+1
                    << " has no matching U-wind field array entry.\n";
            }

         }
      } // end for i
   } // end if

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::process_grib_codes() {

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

void PointStatConfInfo::process_flags() {
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
      } //  for j
   } //  for i

   // Check for at least one output line type
   if(!output_ascii_flag) {
      mlog << Error << "\nPointStatVxOpt::process_config() -> "
           << "At least one output STAT type must be requested in \""
           << conf_key_output_flag << "\".\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::process_masks(const Grid &grid) {
   int i, j;
   MaskPlane mp;
   StringArray sid;
   ConcatString name;

   mlog << Debug(2)
        << "Processing masking regions.\n";

   // Mapping of grid definition strings to mask names
   map<ConcatString,ConcatString> grid_map;
   map<ConcatString,ConcatString> poly_map;
   map<ConcatString,ConcatString> sid_map;
   map<ConcatString,MaskLatLon>   point_map;

   // Initiailize
   mask_area_map.clear();
   mask_sid_map.clear();

   // Process the masks for each vx task
   for(i=0; i<n_vx; i++) {

      // Initialize
      vx_opt[i].mask_name.clear();

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

         // Store the name for the current grid mask
         vx_opt[i].mask_name.add(grid_map[vx_opt[i].mask_grid[j]]);

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

         // Store the name for the current poly mask
         vx_opt[i].mask_name.add(poly_map[vx_opt[i].mask_poly[j]]);

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

         // Store the name for the current station ID mask
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

      // Check for unique mask names
      check_mask_names(vx_opt[i].mask_name);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::process_geog(const Grid &grid,
                                     const char *fcst_file) {
   int i;
   bool land, topo;
   Dictionary *dict;
   DataPlane geog_dp;
   SurfaceInfo sfc_info;

   // Loop over the verification tasks and check flags
   for(i=0, land = topo = false; i<n_vx; i++) {

      // Set to true if requested by any sub-task
      if(vx_opt[i].land_flag) land = true;
      if(vx_opt[i].topo_flag) topo = true;
   }

   mlog << Debug(2)
        << "Processing geography data.\n";

   // Conf: land
   if(land) {
      dict      = conf.lookup_dictionary(conf_key_land_mask);
      geog_dp   = parse_geog_data(dict, grid, fcst_file);
      geog_dp.threshold(dict->lookup_thresh(conf_key_thresh));
      land_mask = geog_dp.mask_plane();

      // Conf: message_type_group_map for LANDSF and WATERSF
      if(msg_typ_group_map.count((string)landsf_msg_typ_group_str) == 0 ||
         msg_typ_group_map.count((string)watersf_msg_typ_group_str) == 0 ) {
         mlog << Error << "\nPointStatConfInfo::process_geog() -> "
              << "when \"" << conf_key_land_mask_flag << "\" is true, \""
              << conf_key_message_type_group_map
              << "\" must contain entries for \""
              << landsf_msg_typ_group_str << "\" and \""
              << watersf_msg_typ_group_str << "\".\n\n";
         exit(1);
      }
   }

   // Conf: topo
   if(topo) {
      dict                    = conf.lookup_dictionary(conf_key_topo_mask);
      topo_dp                 = parse_geog_data(dict, grid, fcst_file);
      topo_use_obs_thresh     = dict->lookup_thresh(conf_key_use_obs_thresh);
      topo_interp_fcst_thresh = dict->lookup_thresh(conf_key_interp_fcst_thresh);

      // Conf: message_type_group_map for SURFACE
      if(msg_typ_group_map.count((string)surface_msg_typ_group_str) == 0) {
         mlog << Error << "\nPointStatConfInfo::process_geog() -> "
              << "when \"" << conf_key_topo_mask_flag << "\" is true, \""
              << conf_key_message_type_group_map
              << "\" must contain an entry for \""
              << surface_msg_typ_group_str << "\".\n\n";
         exit(1);
      }
   }

   // Loop over the verification tasks and set the geography info
   for(i=0; i<n_vx; i++) {
      sfc_info.clear();
      if(vx_opt[i].land_flag) {
         sfc_info.land_ptr = &land_mask;
      }
      else {
         sfc_info.land_ptr = nullptr;
      }
      if(vx_opt[i].topo_flag) {
         sfc_info.topo_ptr = &topo_dp;
         sfc_info.topo_use_obs_thresh = topo_use_obs_thresh;
         sfc_info.topo_interp_fcst_thresh = topo_interp_fcst_thresh;
      }
      else {
         sfc_info.topo_ptr = nullptr;
      }
      vx_opt[i].vx_pd.set_sfc_info(sfc_info);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatConfInfo::set_vx_pd() {

   // This should be called after process_masks()
   for(int i=0; i<n_vx; i++) vx_opt[i].set_vx_pd(this);
}

////////////////////////////////////////////////////////////////////////

int PointStatConfInfo::n_txt_row(int i_txt_row) const {
   int n = 0;

   // Loop over the tasks and sum the line counts for this line type
   for(int i=0; i<n_vx; i++) n += vx_opt[i].n_txt_row(i_txt_row);

   return n;
}

////////////////////////////////////////////////////////////////////////

int PointStatConfInfo::n_stat_row() const {
   int n = 0;

   // Loop over the line types and sum the line counts
   for(int i=0; i<n_txt; i++) n += n_txt_row(i);

   return n;
}

////////////////////////////////////////////////////////////////////////

int PointStatConfInfo::get_max_n_cat_thresh() const {
   int n = 0;

   for(int i=0; i<n_vx; i++) n = max(n, vx_opt[i].get_n_cat_thresh());

   return n;
}

////////////////////////////////////////////////////////////////////////

int PointStatConfInfo::get_max_n_cnt_thresh() const {
   int n = 0;

   for(int i=0; i<n_vx; i++) n = max(n, vx_opt[i].get_n_cnt_thresh());

   return n;
}

////////////////////////////////////////////////////////////////////////

int PointStatConfInfo::get_max_n_wind_thresh() const {
   int n = 0;

   for(int i=0; i<n_vx; i++) n = max(n, vx_opt[i].get_n_wind_thresh());

   return n;
}

////////////////////////////////////////////////////////////////////////

int PointStatConfInfo::get_max_n_fprob_thresh() const {
   int n = 0;

   for(int i=0; i<n_vx; i++) n = max(n, vx_opt[i].get_n_fprob_thresh());

   return n;
}

////////////////////////////////////////////////////////////////////////

int PointStatConfInfo::get_max_n_oprob_thresh() const {
   int n = 0;

   for(int i=0; i<n_vx; i++) n = max(n, vx_opt[i].get_n_oprob_thresh());

   return n;
}

////////////////////////////////////////////////////////////////////////

int PointStatConfInfo::get_max_n_eclv_points() const {
   int n = 0;

   for(int i=0; i<n_vx; i++) n = max(n, vx_opt[i].get_n_eclv_points());

   return n;
}

////////////////////////////////////////////////////////////////////////

int PointStatConfInfo::get_max_n_hira_ens() const {
   int n = 0;

   for(int i=0; i<n_vx; i++) n = max(n, vx_opt[i].get_n_hira_ens());

   return n;
}

////////////////////////////////////////////////////////////////////////

int PointStatConfInfo::get_max_n_hira_prob() const {
   int n = 0;

   for(int i=0; i<n_vx; i++) n = max(n, vx_opt[i].get_n_hira_prob());

   return n;
}

////////////////////////////////////////////////////////////////////////

bool PointStatConfInfo::get_vflag() const {
   bool vflag = false;

   // Vector output must be requested
   if(output_flag[i_vl1l2]  == STATOutputType_None &&
      output_flag[i_val1l2] == STATOutputType_None) {
      return false;
   }

   // Vector components must be requested
   for(int i=0; i<n_vx; i++) {

      if(!vx_opt[i].vx_pd.fcst_info || !vx_opt[i].vx_pd.obs_info) continue;

      if((vx_opt[i].vx_pd.fcst_info->is_u_wind() &&
          vx_opt[i].vx_pd.obs_info->is_u_wind()) ||
         (vx_opt[i].vx_pd.fcst_info->is_v_wind() &&
          vx_opt[i].vx_pd.obs_info->is_v_wind())) {
         vflag = true;
         break;
      }
   }

   return vflag;
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class PointStatVxOpt
//
////////////////////////////////////////////////////////////////////////

PointStatVxOpt::PointStatVxOpt() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PointStatVxOpt::~PointStatVxOpt() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void PointStatVxOpt::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatVxOpt::clear() {
   int i;

   // Initialize values
   vx_pd.clear();

   beg_ds = end_ds = bad_data_int;

   fcat_ta.clear();
   ocat_ta.clear();

   fcnt_ta.clear();
   ocnt_ta.clear();
   cnt_logic = SetLogic_None;

   fwind_ta.clear();
   owind_ta.clear();
   wind_logic = SetLogic_None;

   land_flag = false;
   topo_flag = false;

   mask_grid.clear();
   mask_poly.clear();
   mask_sid.clear();
   mask_llpnt.clear();

   mpr_sa.clear();
   mpr_ta.clear();

   mask_name.clear();

   eclv_points.clear();

   cdf_info.clear();

   ci_alpha.clear();

   boot_info.clear();
   interp_info.clear();
   hira_info.clear();

   hss_ec_value = bad_data_double;
   rank_corr_flag = false;

   msg_typ.clear();

   seeps_p1_thresh.clear();

   duplicate_flag = DuplicateType_None;
   obs_summary = ObsSummary_None;
   obs_perc = bad_data_int;

   for(i=0; i<n_txt; i++) output_flag[i] = STATOutputType_None;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Check the settings that would impact the number of matched pairs
// when searching for U/V matches.
//
////////////////////////////////////////////////////////////////////////

bool PointStatVxOpt::is_uv_match(const PointStatVxOpt &v) const {
   bool match = true;

   //
   // Check that requested forecast and observation levels match.
   // Requested levels are optional for python embedding and may be empty.
   // Check that several other config options also match.
   //
   // The following do not impact matched pairs:
   //    fcat_ta, ocat_ta,
   //    fcnt_ta, ocnt_ta, cnt_logic,
   //    fwind_ta, owind_ta, wind_logic,
   //    eclv_points, cdf_info, ci_alpha
   //    boot_info, hira_info, hss_ec_value,
   //    rank_corr_flag, output_flag
   //

   if(!is_req_level_match(  vx_pd.fcst_info->req_level_name(),
                          v.vx_pd.fcst_info->req_level_name()) ||
      !is_req_level_match(  vx_pd.obs_info->req_level_name(),
                          v.vx_pd.obs_info->req_level_name()) ||
      !(beg_ds         == v.beg_ds        ) ||
      !(end_ds         == v.end_ds        ) ||
      !(land_flag      == v.land_flag     ) ||
      !(topo_flag      == v.topo_flag     ) ||
      !(mask_grid      == v.mask_grid     ) ||
      !(mask_poly      == v.mask_poly     ) ||
      !(mask_sid       == v.mask_sid      ) ||
      !(mask_llpnt     == v.mask_llpnt    ) ||
      !(mask_name      == v.mask_name     ) ||
      !(interp_info    == v.interp_info   ) ||
      !(msg_typ        == v.msg_typ       ) ||
      !(duplicate_flag == v.duplicate_flag) ||
      !(obs_summary    == v.obs_summary   ) ||
      !(obs_perc       == v.obs_perc      )) match = false;

   return match;
}

////////////////////////////////////////////////////////////////////////

void PointStatVxOpt::process_config(GrdFileType ftype,
        Dictionary &fdict, Dictionary &odict) {
   int n;
   VarInfoFactory info_factory;
   map<STATLineType,STATOutputType>output_map;
   Dictionary *dict;

   // Initialize
   clear();

   // Allocate new VarInfo objects
   vx_pd.fcst_info = info_factory.new_var_info(ftype);
   vx_pd.obs_info  = new VarInfoGrib;

   // Set the VarInfo objects
   vx_pd.fcst_info->set_dict(fdict);
   vx_pd.obs_info->set_dict(odict);

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
           << "\nPointStatVxOpt::process_config() -> "
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
      mlog << Error << "\nPointStatVxOpt::process_config() -> "
           << "wind direction may not be verified using point_stat.\n\n";
      exit(1);
   }

   // Check that the observation field does not contain probabilities
   if(vx_pd.obs_info->is_prob()) {
      mlog << Error << "\nPointStatVxOpt::process_config() -> "
           << "the observation field cannot contain probabilities.\n\n";
      exit(1);
   }

   // Conf: output_flag
   output_map = parse_conf_output_flag(&odict, txt_file_type, n_txt);

   // Populate the output_flag array with map values
   for(int i=0; i<n_txt; i++) output_flag[i] = output_map[txt_file_type[i]];

   // Conf: beg_ds and end_ds
   dict = odict.lookup_dictionary(conf_key_obs_window);
   parse_conf_range_int(dict, beg_ds, end_ds);

   // Conf: cat_thresh
   fcat_ta = fdict.lookup_thresh_array(conf_key_cat_thresh);
   ocat_ta = odict.lookup_thresh_array(conf_key_cat_thresh);

   // Conf: cnt_thresh
   fcnt_ta = process_perc_thresh_bins(
                fdict.lookup_thresh_array(conf_key_cnt_thresh));
   ocnt_ta = process_perc_thresh_bins(
                odict.lookup_thresh_array(conf_key_cnt_thresh));

   // Conf: cnt_logic
   cnt_logic = check_setlogic(
      int_to_setlogic(fdict.lookup_int(conf_key_cnt_logic)),
      int_to_setlogic(odict.lookup_int(conf_key_cnt_logic)));

   // Conf: wind_thresh
   fwind_ta = process_perc_thresh_bins(
                 fdict.lookup_thresh_array(conf_key_wind_thresh));
   owind_ta = process_perc_thresh_bins(
                 odict.lookup_thresh_array(conf_key_wind_thresh));

   // Conf: wind_logic
   wind_logic = check_setlogic(
      int_to_setlogic(fdict.lookup_int(conf_key_wind_logic)),
      int_to_setlogic(odict.lookup_int(conf_key_wind_logic)));

   // Conf: mpr_column and mpr_thresh
   mpr_sa = odict.lookup_string_array(conf_key_mpr_column);
   mpr_ta = odict.lookup_thresh_array(conf_key_mpr_thresh);

   // Dump the contents of the current thresholds
   if(mlog.verbosity_level() >= 5) {
      mlog << Debug(5)
           << "Parsed thresholds:\n"
           << "Matched pair filter columns:     " << write_css(mpr_sa) << "\n"
           << "Matched pair filter thresholds:  " << mpr_ta.get_str() << "\n"
           << "Forecast categorical thresholds: " << fcat_ta.get_str() << "\n"
           << "Observed categorical thresholds: " << ocat_ta.get_str() << "\n"
           << "Forecast continuous thresholds:  " << fcnt_ta.get_str() << "\n"
           << "Observed continuous thresholds:  " << ocnt_ta.get_str() << "\n"
           << "Continuous threshold logic:      " << setlogic_to_string(cnt_logic) << "\n"
           << "Forecast wind speed thresholds:  " << fwind_ta.get_str() << "\n"
           << "Observed wind speed thresholds:  " << owind_ta.get_str() << "\n"
           << "Wind speed threshold logic:      " << setlogic_to_string(wind_logic) << "\n";
   }

   // Verifying a probability field
   if(vx_pd.fcst_info->is_prob()) {
      fcat_ta = string_to_prob_thresh(fcat_ta.get_str().c_str());
   }

   // Check for equal threshold length for non-probability fields
   if(!vx_pd.fcst_info->is_prob() &&
      fcat_ta.n() != ocat_ta.n()) {

      mlog << Error << "\nPointStatVxOpt::process_config() -> "
           << "The number of thresholds for each field in \"fcst."
           << conf_key_cat_thresh
           << "\" must match the number of thresholds for each "
           << "field in \"obs." << conf_key_cat_thresh << "\".\n\n";
      exit(1);
   }

   // Add default continuous thresholds until the counts match
   n = max(fcnt_ta.n(), ocnt_ta.n());
   while(fcnt_ta.n() < n) fcnt_ta.add(na_str);
   while(ocnt_ta.n() < n) ocnt_ta.add(na_str);

   // Add default wind speed thresholds until the counts match
   n = max(fwind_ta.n(), owind_ta.n());
   while(fwind_ta.n() < n) fwind_ta.add(na_str);
   while(owind_ta.n() < n) owind_ta.add(na_str);

   // Verifying with multi-category contingency tables
   if(!vx_pd.fcst_info->is_prob() &&
      (output_flag[i_mctc] != STATOutputType_None ||
       output_flag[i_mcts] != STATOutputType_None)) {
      check_mctc_thresh(fcat_ta);
      check_mctc_thresh(ocat_ta);
   }

   // Conf: land.flag
   land_flag = odict.lookup_bool(conf_key_land_mask_flag);

   // Conf: topo.flag
   topo_flag = odict.lookup_bool(conf_key_topo_mask_flag);

   // Conf: mask_grid
   mask_grid = odict.lookup_string_array(conf_key_mask_grid);

   // Conf: mask_poly
   mask_poly = odict.lookup_string_array(conf_key_mask_poly);

   // Conf: mask_sid
   mask_sid = odict.lookup_string_array(conf_key_mask_sid);

   // Conf: mask_llpnt
   mask_llpnt = parse_conf_llpnt_mask(&odict);

   // Conf: eclv_points
   eclv_points = parse_conf_eclv_points(&odict);

   // Conf: climo_cdf
   cdf_info = parse_conf_climo_cdf(&odict);

   // Conf: ci_alpha
   ci_alpha = parse_conf_ci_alpha(&odict);

   // Conf: boot
   boot_info = parse_conf_boot(&odict);

   // Conf: interp
   interp_info = parse_conf_interp(&odict, conf_key_interp);

   // Conf: hira
   hira_info = parse_conf_hira(&odict);

   // Conf: hss_ec_value
   hss_ec_value = odict.lookup_double(conf_key_hss_ec_value);

   // Conf: rank_corr_flag
   rank_corr_flag = odict.lookup_bool(conf_key_rank_corr_flag);

   // Conf: threshold for SEEPS p1
   seeps_p1_thresh = odict.lookup_thresh(conf_key_seeps_p1_thresh);

   // Conf: message_type
   msg_typ = parse_conf_message_type(&odict);

   // Conf: duplicate_flag
   duplicate_flag = parse_conf_duplicate_flag(&odict);

   // Conf: obs_summary
   obs_summary = parse_conf_obs_summary(&odict);

   // Conf: obs_perc_value
   obs_perc = parse_conf_percentile(&odict);

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

void PointStatVxOpt::set_vx_pd(PointStatConfInfo *conf_info) {
   int i, n;
   int n_msg_typ = msg_typ.n();
   int n_mask    = mask_name.n();
   int n_interp  = interp_info.n_interp;
   ConcatString cs;
   StringArray sa;

   // Setup the VxPairDataPoint object with these dimensions:
   // [n_msg_typ][n_mask][n_interp]

   // Check for at least one message type
   if(n_msg_typ == 0) {
      mlog << Error << "\nPointStatVxOpt::set_vx_pd() -> "
           << "At least one output message type must be requested in \""
           << conf_key_message_type << "\".\n\n";
      exit(1);
   }

   // Check for at least one masking region
   if(n_mask == 0) {
      mlog << Error << "\nPointStatVxOpt::set_vx_pd() -> "
           << "At least one output masking region must be requested in \""
           << conf_key_mask_grid  << "\", \""
           << conf_key_mask_poly  << "\", \""
           << conf_key_mask_sid   << "\", or \""
           << conf_key_mask_llpnt << "\".\n\n";
      exit(1);
   }

   // Check for at least one interpolation method
   if(n_interp == 0) {
      mlog << Error << "\nPointStatVxOpt::set_vx_pd() -> "
           << "At least one interpolation method must be requested in \""
           << conf_key_interp << "\".\n\n";
      exit(1);
   }

   // Define the dimensions
   vx_pd.set_pd_size(n_msg_typ, n_mask, n_interp);

   // Store the MPR filter threshold
   vx_pd.set_mpr_thresh(mpr_sa, mpr_ta);

   // Store the climo CDF info
   vx_pd.set_climo_cdf_info_ptr(&cdf_info);

   // Store the surface message type group
   cs = surface_msg_typ_group_str;
   if(conf_info->msg_typ_group_map.count(cs) > 0) {
      vx_pd.set_msg_typ_sfc(conf_info->msg_typ_group_map[cs]);
   }
   else {
      sa.parse_css(default_msg_typ_group_surface);
      vx_pd.set_msg_typ_sfc(sa);
   }

   // Store the surface land message type group
   cs = landsf_msg_typ_group_str;
   if(conf_info->msg_typ_group_map.count(cs) > 0) {
      vx_pd.set_msg_typ_lnd(conf_info->msg_typ_group_map[cs]);
   }
   else {
      sa.parse_css(default_msg_typ_group_landsf);
      vx_pd.set_msg_typ_lnd(sa);
   }

   // Store the surface water message type group
   cs = watersf_msg_typ_group_str;
   if(conf_info->msg_typ_group_map.count(cs) > 0) {
      vx_pd.set_msg_typ_wtr(conf_info->msg_typ_group_map[cs]);
   }
   else {
      sa.parse_css(default_msg_typ_group_watersf);
      vx_pd.set_msg_typ_wtr(sa);
   }

   // Define the verifying message type name and values
   for(i=0; i<n_msg_typ; i++) {
      vx_pd.set_msg_typ(i, msg_typ[i].c_str());
      sa = conf_info->msg_typ_group_map[msg_typ[i]];
      if(sa.n() == 0) sa.add(msg_typ[i]);
      vx_pd.set_msg_typ_vals(i, sa);
   }

   // Define the masking information: grid, poly, sid, point

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
   for(i=0; i<n_interp; i++) {
      vx_pd.set_interp(i, interp_info.method[i].c_str(), interp_info.width[i],
                       interp_info.shape);
      vx_pd.set_interp_thresh(interp_info.vld_thresh);
   }

   // After sizing VxPairDataPoint, add settings for each array element
   vx_pd.set_duplicate_flag(duplicate_flag);
   vx_pd.set_obs_summary(obs_summary);
   vx_pd.set_obs_perc_value(obs_perc);
   if (output_flag[i_seeps_mpr] != STATOutputType_None
       || output_flag[i_seeps] != STATOutputType_None) {
     vx_pd.load_seeps_climo();
     vx_pd.set_seeps_thresh(seeps_p1_thresh);
   }
   return;
}

////////////////////////////////////////////////////////////////////////

void PointStatVxOpt::set_perc_thresh(const PairDataPoint *pd_ptr) {

   //
   // Compute percentiles for forecast and observation thresholds,
   // but not for wind speed or climatology CDF thresholds.
   //
   if(!fcat_ta.need_perc() && !ocat_ta.need_perc() &&
      !fcnt_ta.need_perc() && !ocnt_ta.need_perc()) return;

   //
   // Sort the input arrays
   //
   NumArray fsort = pd_ptr->f_na;
   NumArray osort = pd_ptr->o_na;
   NumArray csort = pd_ptr->cmn_na;
   fsort.sort_array();
   osort.sort_array();
   csort.sort_array();

   //
   // Compute percentiles
   //
   fcat_ta.set_perc(&fsort, &osort, &csort, &fcat_ta, &ocat_ta);
   ocat_ta.set_perc(&fsort, &osort, &csort, &fcat_ta, &ocat_ta);
   fcnt_ta.set_perc(&fsort, &osort, &csort, &fcnt_ta, &ocnt_ta);
   ocnt_ta.set_perc(&fsort, &osort, &csort, &fcnt_ta, &ocnt_ta);

   return;
}

////////////////////////////////////////////////////////////////////////

int PointStatVxOpt::n_txt_row(int i_txt_row) const {
   int n = 0;
   int n_bin;
   const char *method_name = "PointStatVxOpt::n_txt_row(int) -> ";

   // Range check
   if(i_txt_row < 0 || i_txt_row >= n_txt) {
      mlog << Error << "\n" << method_name
           << "range check error for " << i_txt_row << "\n\n";
      exit(1);
   }

   // Check if this output line type is requested
   if(output_flag[i_txt_row] == STATOutputType_None) return 0;

   bool prob_flag = vx_pd.fcst_info->is_prob();
   bool vect_flag = vx_pd.fcst_info->is_v_wind() &&
                    vx_pd.fcst_info->uv_index() >= 0;

   int n_pd = get_n_msg_typ() * get_n_mask() * get_n_interp();

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

      case(i_fho):
      case(i_ctc):
         // Number of FHO or CTC lines =
         //    Message Types * Masks * Interpolations * Thresholds
         n = (prob_flag ? 0 : n_pd * get_n_cat_thresh());
         break;

      case(i_cts):
         // Number of CTS lines =
         //    Message Types * Masks * Interpolations * Thresholds *
         //    Alphas
         n = (prob_flag ? 0 : n_pd * get_n_cat_thresh() *
              get_n_ci_alpha());
         break;

      case(i_mctc):
         // Number of MCTC lines =
         //    Message Types * Masks * Interpolations
         n = (prob_flag ? 0 : n_pd);
         break;

      case(i_mcts):
         // Number of MCTS lines =
         //    Message Types * Masks * Interpolations * Alphas
         n = (prob_flag ? 0 : n_pd * get_n_ci_alpha());
         break;

      case(i_cnt):
         // Number of CNT lines =
         //    Message Types * Masks * Interpolations * Thresholds *
         //    Climo Bins * Alphas
         n = (prob_flag ? 0 : n_pd * get_n_cnt_thresh() * n_bin *
              get_n_ci_alpha());
         break;

      case(i_sl1l2):
      case(i_sal1l2):
         // Number of SL1L2 and SAL1L2 lines =
         //    Message Types * Masks * Interpolations * Thresholds *
         //    Climo Bins
         n = (prob_flag ? 0 : n_pd * get_n_cnt_thresh() * n_bin);
         break;

      case(i_vl1l2):
      case(i_val1l2):
         // Number of VL1L2 or VAL1L2 lines =
         //    Message Types * Masks * Interpolations * Thresholds
         n = (!vect_flag ? 0 : n_pd *
              get_n_wind_thresh());
         break;

      case(i_vcnt):
         // Number of VCNT lines =
         //    Message Types * Masks * Interpolations * Thresholds *
         //    Alphas
         n = (!vect_flag ? 0 : n_pd *
              get_n_wind_thresh() * get_n_ci_alpha());
         break;

      case(i_pct):
      case(i_pjc):
      case(i_prc):
         // Number of PCT, PJC, or PRC lines possible =
         //    Message Types * Masks * Interpolations * Thresholds *
         //    Climo Bins
         n = (!prob_flag ? 0 : n_pd * get_n_oprob_thresh() * n_bin);

         // Number of HiRA PCT, PJC, or PRC lines =
         //    Message Types * Masks * HiRA widths * Thresholds
         if(hira_info.flag) {
            n += (prob_flag ? 0 : n_pd * get_n_cat_thresh() *
                  hira_info.width.n());
         }

         break;

      case(i_pstd):
         // Number of PSTD lines =
         //    Message Types * Masks * Interpolations * Thresholds *
         //    Alphas * Climo Bins
         n = (!prob_flag ? 0 : n_pd *
              get_n_oprob_thresh() * get_n_ci_alpha() * n_bin);

         // Number of HiRA PSTD lines =
         //    Message Types * Masks * HiRA widths * Thresholds *
         //    Alphas
         if(hira_info.flag) {
            n += (prob_flag ? 0 : n_pd *
                  get_n_cat_thresh() * hira_info.width.n() *
                  get_n_ci_alpha());
         }

         break;

      case(i_ecnt):
      case(i_rps):
         // Number of HiRA ECNT and RPS lines =
         //    Message Types * Masks * HiRA widths *
         //    Alphas
         if(hira_info.flag) {
            n = n_pd * hira_info.width.n() * get_n_ci_alpha();
         }
         else {
            n = 0;
         }

         break;

      case(i_orank):
         // Number of HiRA ORANK lines possible =
         //    Number of pairs * Categorical Thresholds *
         //    HiRA widths
         if(hira_info.flag) {
            n = vx_pd.get_n_pair() * get_n_cat_thresh() *
                hira_info.width.n();
         }
         else {
            n = 0;
         }

         break;

      case(i_eclv):
         // Number of CTC -> ECLV lines =
         //    Message Types * Masks * Interpolations * Thresholds *
         //    Climo Bins
         n = (prob_flag ? 0 : n_pd * get_n_cat_thresh() * n_bin);

         // Number of PCT -> ECLV lines =
         //    Message Types * Masks * Interpolations *
         //    Observation Probability Thresholds *
         //    Forecast Probability Thresholds * Climo Bins
         n += (!prob_flag ? 0 : n_pd *
               get_n_oprob_thresh() * get_n_fprob_thresh() * n_bin);

         break;

      case(i_mpr):
         // Compute the number of matched pairs to be written
         n = vx_pd.get_n_pair();

         // Maximum number of HiRA MPR lines possible =
         //    Number of pairs * Max Scalar Categorical Thresholds *
         //    HiRA widths
         if(hira_info.flag) {
            n += (prob_flag ? 0 :
                  vx_pd.get_n_pair() * get_n_cat_thresh() *
                  hira_info.width.n());
         }

         break;

      case(i_seeps_mpr):
         // Compute the number of matched pairs to be written
         n = vx_pd.get_n_pair();

         break;

      case(i_seeps):
         // Compute the number of matched pairs to be written
         n = vx_pd.get_n_pair();

         break;

      default:
         mlog << Error << "\n" << method_name
              << "unexpected output type index value: " << i_txt_row
              << "\n\n";
         exit(1);
   }
   return n;
}

////////////////////////////////////////////////////////////////////////

int PointStatVxOpt::get_n_cnt_thresh() const {
   return (!vx_pd.fcst_info || vx_pd.fcst_info->is_prob()) ?
          0 : fcnt_ta.n();
}

////////////////////////////////////////////////////////////////////////

int PointStatVxOpt::get_n_cat_thresh() const {
   return (!vx_pd.fcst_info || vx_pd.fcst_info->is_prob()) ?
          0 : fcat_ta.n();
}

////////////////////////////////////////////////////////////////////////

int PointStatVxOpt::get_n_wind_thresh() const {
   return (!vx_pd.fcst_info || vx_pd.fcst_info->is_prob()) ?
          0 : fwind_ta.n();
}

////////////////////////////////////////////////////////////////////////

int PointStatVxOpt::get_n_fprob_thresh() const {
   return (!vx_pd.fcst_info || !vx_pd.fcst_info->is_prob()) ?
          0 : fcat_ta.n();
}

////////////////////////////////////////////////////////////////////////

int PointStatVxOpt::get_n_oprob_thresh() const {
   return (!vx_pd.fcst_info || !vx_pd.fcst_info->is_prob()) ?
           0 : ocat_ta.n();
}

////////////////////////////////////////////////////////////////////////

int PointStatVxOpt::get_n_hira_ens() const {
   int n = (hira_info.flag ? hira_info.width.max() : 0);
   return n*n;
}

////////////////////////////////////////////////////////////////////////

int PointStatVxOpt::get_n_hira_prob() const {
   return hira_info.flag ? hira_info.cov_ta.n() : 0;
}

////////////////////////////////////////////////////////////////////////
