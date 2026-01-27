// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
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

#include "pair_stat_conf_info.h"
#include "nc_obs_util.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_cal.h"
#include "vx_log.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

static ConcatString get_time_str(
                               const unixtime beg, const unixtime end,
                               const TimeArray &inc, const TimeArray &exc,
                               const IntArray &hour);

static bool is_keeper_unixtime(const unixtime, const unixtime,
                               const unixtime, const unixtime,
                               const TimeArray &, const TimeArray &,
                               const IntArray &);

static double get_ioda_mpr_val(const string &,
                               const point_pair_t &,
                               bool print_warning = false);

static string get_ioda_mpr_str(const string &,
                               const point_pair_t &,
                               bool print_warning = false);

////////////////////////////////////////////////////////////////////////
//
// Code for class PairsFormat enumeration
//
////////////////////////////////////////////////////////////////////////

PairsFormat string_to_pairsformat(const string &s) {
   PairsFormat t;
   ConcatString cs(s);
   cs.set_upper();

        if(cs == "MPR")    t = PairsFormat::MPR;
   else if(cs == "PYTHON") t = PairsFormat::Python;
   else if(cs == "IODA")   t = PairsFormat::IODA;
   else                    t = PairsFormat::None;

   return t;
}

////////////////////////////////////////////////////////////////////////

ConcatString pairsformat_to_string(const PairsFormat t) {
   ConcatString s;

   switch(t) {
      case PairsFormat::MPR:    s = "mpr";    break;
      case PairsFormat::Python: s = "python"; break;
      case PairsFormat::IODA:   s = "ioda";   break;
      default:                  s = na_str;   break;
   }

   return s;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class PairStatConfInfo
//
////////////////////////////////////////////////////////////////////////

PairStatConfInfo::PairStatConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

void PairStatConfInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairStatConfInfo::clear() {

   // Initialize values
   model.clear();
   vx_opt.clear();
   mask_area_map.clear();
   mask_sid_map.clear();
   point_weight_flag = PointWeightType::None;
   tmp_dir.clear();
   version.clear();
   seeps_climo_name.clear();
   seeps_p1_thresh.clear();

   // Set count to zero
   n_vx = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairStatConfInfo::read_config(const StringArray &sa) {

   // Read each specified config file in order
   for(int i=0; i<sa.n(); i++) {
      mlog << Debug(1) << "Reading Config: " << sa[i] << "\n";
      conf.read(sa[i].c_str());
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairStatConfInfo::process_config(PairsFormat ftype) {
   Dictionary *fdict = nullptr;
   Dictionary *odict = nullptr;
   Dictionary i_fdict;
   Dictionary i_odict;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Initialize
   clear();

   // Conf: version
   version = parse_conf_version(&conf);

   // Conf: model
   model = parse_conf_string(&conf, conf_key_model, false);

   // Conf: point_weight_flag
   point_weight_flag = parse_conf_point_weight_flag(&conf);

   // Conf: tmp_dir
   tmp_dir = parse_conf_tmp_dir(&conf);

   // Conf: fcst.pairs and obs.pairs
   fdict = conf.lookup_array(conf_key_fcst_pairs);
   odict = conf.lookup_array(conf_key_obs_pairs);

   // Determine the number of fields (name/level) to be verified
   int n_fvx = parse_conf_n_vx(fdict);
   int n_ovx = parse_conf_n_vx(odict);

   // Check for a valid number of verification tasks
   if(n_fvx == 0 || n_fvx != n_ovx) {
      mlog << Error << "\nPairStatConfInfo::process_config() -> "
           << "The number of verification tasks in \""
           << conf_key_obs_pairs << "\" (" << n_ovx
           << ") must be non-zero and match the number in \""
           << conf_key_fcst_pairs << "\" (" << n_fvx << ").\n\n";
      exit(1);
   }

   // Allocate memory for the verification task options
   n_vx   = n_fvx;
   vx_opt.resize(n_vx);

   // Check for consistent number of climatology fields
   check_climo_n_vx(fdict, n_vx);
   check_climo_n_vx(odict, n_vx);

   // Conf: threshold for SEEPS p1
   seeps_p1_thresh = conf.lookup_thresh(conf_key_seeps_p1_thresh);

   // Conf: SEEPS climo filename
   seeps_climo_name = conf.lookup_string(conf_key_seeps_point_climo_name, false);

   // Parse settings for each verification task
   for(int i=0; i<n_vx; i++) {

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
   if(output_flag[i_vl1l2]  != STATOutputType::None ||
      output_flag[i_val1l2] != STATOutputType::None ||
      output_flag[i_vcnt]   != STATOutputType::None) {

      for(int i=0; i<n_vx; i++) {

         // Process u-wind
         if(vx_opt[i].vx_pd.fcst_info->is_u_wind() &&
            vx_opt[i].vx_pd.obs_info->is_u_wind()) {

            // Search for corresponding v-wind
            for(int j=0; j<n_vx; j++) {
               if(vx_opt[j].vx_pd.fcst_info->is_v_wind() &&
                  vx_opt[j].vx_pd.obs_info->is_v_wind()  &&
                  vx_opt[i].is_uv_match(vx_opt[j])) {

                  mlog << Debug(3) << "U-wind field array entry " << i+1
                       << " matches V-wind field array entry " << j+1 << ".\n";

                  // Print warning about multiple matches
                  if(vx_opt[i].vx_pd.fcst_info->uv_index() >= 0 ||
                     vx_opt[i].vx_pd.obs_info->uv_index()  >= 0) {
                     mlog << Warning << "\nPairStatConfInfo::process_config() -> "
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
            for(int j=0; j<n_vx; j++) {
               if(vx_opt[j].vx_pd.fcst_info->is_u_wind() &&
                  vx_opt[j].vx_pd.obs_info->is_u_wind()  &&
                  vx_opt[i].is_uv_match(vx_opt[j])) {

                  mlog << Debug(3) << "V-wind field array entry " << i+1
                       << " matches U-wind field array entry " << j+1 << ".\n";

                  // Print warning about multiple matches
                  if(vx_opt[i].vx_pd.fcst_info->uv_index() >= 0 ||
                     vx_opt[i].vx_pd.obs_info->uv_index()  >= 0) {
                     mlog << Warning << "\nPairStatConfInfo::process_config() -> "
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

void PairStatConfInfo::process_flags() {
   bool output_ascii_flag = false;

   // Initialize
   for(int i=0; i<n_txt; i++) output_flag[i] = STATOutputType::None;

   // Loop over the verification tasks
   for(int i=0; i<n_vx; i++) {

      // Summary of output_flag settings
      for(int j=0; j<n_txt; j++) {

         if(vx_opt[i].output_flag[j] == STATOutputType::Both) {
            output_flag[j] = STATOutputType::Both;
            output_ascii_flag = true;
         }
         else if(vx_opt[i].output_flag[j] == STATOutputType::Stat &&
                           output_flag[j] == STATOutputType::None) {
            output_flag[j] = STATOutputType::Stat;
            output_ascii_flag = true;
         }
      } //  for j
   } //  for i

   // Check for at least one output line type
   if(!output_ascii_flag) {
      mlog << Error << "\nPairStatVxOpt::process_config() -> "
           << "At least one output STAT type must be requested in \""
           << conf_key_output_flag << "\".\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairStatConfInfo::process_masks() {
   MaskPlane mp;
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
   for(auto &vx : vx_opt) {

      // Initialize
      vx.mask_name.clear();

      // MET #3298 Add the FULL grid, if needed
      if(vx.mask_grid.n() + vx.mask_poly.n() +
         vx.mask_sid.n() + (int) vx.mask_llpnt.size() == 0) {
         mlog << Debug(3) << "Adding grid = " << full_domain_str
              << " since no masking regions were specified.\n";
         vx.mask_grid.add(full_domain_str);
      }

      // Parse the masking grids
      for(int i=0; i<vx.mask_grid.n(); i++) {

         // Process new grid masks
         if(grid_map.count(vx.mask_grid[i]) == 0) {
            mlog << Debug(3)
                 << "Processing grid mask: "
                 << vx.mask_grid[i] << "\n";
            parse_grid_mask(vx.mask_grid[i], grid_mask, mp, name);
            grid_map[vx.mask_grid[i]] = name;
            mask_area_map[name] = mp;
         }

         // Store the name for the current grid mask
         vx.mask_name.add(grid_map[vx.mask_grid[i]]);

      } // end for i

      // Parse the masking polylines
      for(int i=0; i<vx.mask_poly.n(); i++) {

         // Process new poly mask
         if(poly_map.count(vx.mask_poly[i]) == 0) {
            mlog << Debug(3)
                 << "Processing poly mask: "
                 << vx.mask_poly[i] << "\n";
            parse_poly_mask(vx.mask_poly[i], grid_mask, mp, name);
            poly_map[vx.mask_poly[i]] = name;
            mask_area_map[name] = mp;
         }

         // Store the name for the current poly mask
         vx.mask_name.add(poly_map[vx.mask_poly[i]]);

      } // end for i 

      // Parse the masking station ID's
      for(int i=0; i<vx.mask_sid.n(); i++) {

         // Process new station ID mask
         if(sid_map.count(vx.mask_sid[i]) == 0) {
            mlog << Debug(3)
                 << "Processing station ID mask: "
                 << vx.mask_sid[i] << "\n";
            MaskSID ms = parse_sid_mask(vx.mask_sid[i]);
            sid_map[vx.mask_sid[i]] = ms.name();
            mask_sid_map[ms.name()] = ms;
         }

         // Store the name for the current station ID mask
         vx.mask_name.add(sid_map[vx.mask_sid[i]]);

      } // end for i  

      // Parse the Lat/Lon point masks
      for(int i=0; i<(int) vx.mask_llpnt.size(); i++) {

         // Process new point masks -- no real work to do
         if(point_map.count(vx.mask_llpnt[i].name) == 0) {
            mlog << Debug(3)
                 << "Processing Lat/Lon point mask: "
                 << vx.mask_llpnt[i].name << "\n";
            point_map[vx.mask_llpnt[i].name] = vx.mask_llpnt[i];
         }

         // Store the name for the current Lat/Lon point mask
         vx.mask_name.add(vx.mask_llpnt[i].name);

      } // end for i 

      // Check for unique mask names
      check_mask_names(vx.mask_name);

   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairStatConfInfo::set_vx_pd() {

   // This should be called after process_masks()
   for(auto &vx : vx_opt) vx.set_vx_pd(this);
}

////////////////////////////////////////////////////////////////////////

int PairStatConfInfo::n_txt_row(int i_txt_row) const {
   int n = 0;

   // Loop over the tasks and sum the line counts for this line type
   for(auto &vx : vx_opt) n += vx.n_txt_row(i_txt_row);

   return n;
}

////////////////////////////////////////////////////////////////////////

int PairStatConfInfo::n_stat_row() const {
   int n = 0;

   // Loop over the line types and sum the line counts
   for(int i=0; i<n_txt; i++) n += n_txt_row(i);

   return n;
}

////////////////////////////////////////////////////////////////////////

int PairStatConfInfo::get_max_n_cat_thresh() const {
   int n = 0;

   for(auto &vx : vx_opt) n = max(n, vx.get_n_cat_thresh());

   return n;
}

////////////////////////////////////////////////////////////////////////

int PairStatConfInfo::get_max_n_cnt_thresh() const {
   int n = 0;

   for(auto &vx : vx_opt) n = max(n, vx.get_n_cnt_thresh());

   return n;
}

////////////////////////////////////////////////////////////////////////

int PairStatConfInfo::get_max_n_wind_thresh() const {
   int n = 0;

   for(auto &vx : vx_opt) n = max(n, vx.get_n_wind_thresh());

   return n;
}

////////////////////////////////////////////////////////////////////////

int PairStatConfInfo::get_max_n_fprob_thresh() const {
   int n = 0;

   for(auto &vx : vx_opt) n = max(n, vx.get_n_fprob_thresh());

   return n;
}

////////////////////////////////////////////////////////////////////////

int PairStatConfInfo::get_max_n_oprob_thresh() const {
   int n = 0;

   for(auto &vx : vx_opt) n = max(n, vx.get_n_oprob_thresh());

   return n;
}

////////////////////////////////////////////////////////////////////////

int PairStatConfInfo::get_max_n_eclv_points() const {
   int n = 0;

   for(auto &vx : vx_opt) n = max(n, vx.get_n_eclv_points());

   return n;
}

////////////////////////////////////////////////////////////////////////

bool PairStatConfInfo::get_vflag() const {
   bool vflag = false;

   // Vector output must be requested
   if(output_flag[i_vl1l2]  == STATOutputType::None &&
      output_flag[i_val1l2] == STATOutputType::None) {
      return false;
   }

   // Vector components must be requested
   for(auto &vx : vx_opt) {

      if(!vx.vx_pd.fcst_info || !vx.vx_pd.obs_info) continue;

      if((vx.vx_pd.fcst_info->is_u_wind() &&
          vx.vx_pd.obs_info->is_u_wind()) ||
         (vx.vx_pd.fcst_info->is_v_wind() &&
          vx.vx_pd.obs_info->is_v_wind())) {
         vflag = true;
         break;
      }
   }

   return vflag;
}

////////////////////////////////////////////////////////////////////////

bool PairStatConfInfo::add_mpr_line(STATLine l) {
   bool keep = false;

   // Attempt to add line to each verification task
   for(auto &vx : vx_opt) {
      if(vx.add_mpr_line(l)) keep = true;
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class PairStatVxOpt
//
////////////////////////////////////////////////////////////////////////

PairStatVxOpt::PairStatVxOpt() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

void PairStatVxOpt::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairStatVxOpt::clear() {

   // Initialize values
   vx_pd.clear();
   vx_hdr.clear();
   convert_censor_flag = false;

   fcat_ta.clear();
   ocat_ta.clear();

   fcnt_ta.clear();
   ocnt_ta.clear();
   cnt_logic = SetLogic::None;

   fwind_ta.clear();
   owind_ta.clear();
   wind_logic = SetLogic::None;

   mask_grid.clear();
   mask_poly.clear();
   mask_sid.clear();
   mask_llpnt.clear();

   mpr_thr_inc_map.clear();
   mpr_str_inc_map.clear();
   mpr_str_exc_map.clear();

   fcst_lead.clear();
   obs_lead.clear();

   fcst_valid_beg = 0;
   fcst_valid_end = 0;
   fcst_valid_inc.clear();
   fcst_valid_exc.clear();
   fcst_valid_hour.clear();

   obs_valid_beg = 0;
   obs_valid_end = 0;
   obs_valid_inc.clear();
   obs_valid_exc.clear();
   obs_valid_hour.clear();

   fcst_init_beg = 0;
   fcst_init_end = 0;
   fcst_init_inc.clear();
   fcst_init_exc.clear();
   fcst_init_hour.clear();

   obs_init_beg = 0;
   obs_init_end = 0;
   obs_init_inc.clear();
   obs_init_exc.clear();
   obs_init_hour.clear();

   mask_name.clear();

   eclv_points.clear();

   cdf_info.clear();

   ci_alpha.clear();

   boot_info.clear();

   hss_ec_value = bad_data_double;
   rank_corr_flag = false;

   for(int i=0; i<n_txt; i++) output_flag[i] = STATOutputType::None;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Check the settings that would impact the number of matched pairs
// when searching for U/V matches.
//
////////////////////////////////////////////////////////////////////////

bool PairStatVxOpt::is_uv_match(const PairStatVxOpt &v) const {
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
   //    boot_info, hss_ec_value,
   //    rank_corr_flag, output_flag
   //

   if(!is_req_level_match(  vx_pd.fcst_info->req_level_name(),
                          v.vx_pd.fcst_info->req_level_name()) ||
      !is_req_level_match(  vx_pd.obs_info->req_level_name(),
                          v.vx_pd.obs_info->req_level_name()) ||
      !(mask_grid  == v.mask_grid     ) ||
      !(mask_poly  == v.mask_poly     ) ||
      !(mask_sid   == v.mask_sid      ) ||
      !(mask_llpnt == v.mask_llpnt    ) ||
      !(mask_name  == v.mask_name     )) match = false;

   return match;
}

////////////////////////////////////////////////////////////////////////

void PairStatVxOpt::process_config(PairsFormat ftype,
        Dictionary &fdict, Dictionary &odict) {
   const char *method_name = "PairStatVxOpt::process_config() -> ";

   // Initialize
   clear();

   // Allocate new VarInfo objects
   vx_pd.set_fcst_info(VarInfoFactory::new_var_info(FileType_Pairs));
   vx_pd.set_obs_info(VarInfoFactory::new_var_info(FileType_Pairs));

   // Set the VarInfo objects
   vx_pd.fcst_info->set_dict(fdict);
   vx_pd.obs_info->set_dict(odict);

   // Set the conversion and/or censoring flag
   if(vx_pd.fcst_info->ConvertFx.is_set()      ||
      vx_pd.fcst_info->censor_thresh().n() > 0 ||
      vx_pd.obs_info->ConvertFx.is_set()       ||
      vx_pd.obs_info->censor_thresh().n()  > 0) {
      convert_censor_flag = true;
   }

   // Dump the contents of the current VarInfo
   if(mlog.verbosity_level() >= 5) {
      mlog << Debug(5)
           << "Parsed forecast field:\n";
      vx_pd.fcst_info->dump(cout);
      mlog << Debug(5)
           << "Parsed observation field:\n";
      vx_pd.obs_info->dump(cout);
   }

   // No support for wind direction
   if(vx_pd.fcst_info->is_wind_direction() ||
      vx_pd.obs_info->is_wind_direction()) {
      mlog << Error << "\n" << method_name
           << "wind direction may not be verified using pair_stat.\n\n";
      exit(1);
   }

   // Check that the observation field does not contain probabilities
   if(vx_pd.obs_info->is_prob()) {
      mlog << Error << "\n" << method_name
           << "the observation field cannot contain probabilities.\n\n";
      exit(1);
   }

   // Conf: output_flag
   map<STATLineType,STATOutputType> output_map = 
      parse_conf_output_flag(&odict, txt_file_type, n_txt);

   // Populate the output_flag array with map values
   for(int i=0; i<n_txt; i++) output_flag[i] = output_map[txt_file_type[i]];

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
   StringArray mpr_sa(odict.lookup_string_array(conf_key_mpr_column));
   ThreshArray mpr_ta(odict.lookup_thresh_array(conf_key_mpr_thresh));

   // Check for the same length
   if(mpr_sa.n() != mpr_ta.n()) {
      mlog << Error << "\n" << method_name
           << "The length of \"" << conf_key_mpr_column << "\" and \""
           << conf_key_mpr_thresh << "\" must match (" << mpr_sa.n()
           << " != " << mpr_ta.n() << ")!\n\n";
      exit(1);
   }

   // Store in map
   for(int i=0; i<mpr_sa.n(); i++) {
      if(mpr_thr_inc_map.count(mpr_sa[i]) == 0) {
         ThreshArray ta;
         mpr_thr_inc_map[mpr_sa[i]] = ta;
      }
      mpr_thr_inc_map[mpr_sa[i]].add(mpr_ta[i]);
   }

   // Conf: mpr_str_inc
   parse_add_conf_key_values_map(&odict, conf_key_mpr_str_inc,
      &mpr_str_inc_map, method_name);

   // Conf: mpr_str_exc
   parse_add_conf_key_values_map(&odict, conf_key_mpr_str_exc,
      &mpr_str_exc_map, method_name);

   // Print warnings about unexpected IODA MPR column names
   if(ftype == PairsFormat::IODA) {
      point_pair_t p;
      for(const auto &m : mpr_thr_inc_map) get_ioda_mpr_val(m.first, p, true);
      for(const auto &m : mpr_str_inc_map) get_ioda_mpr_str(m.first, p, true);
      for(const auto &m : mpr_str_exc_map) get_ioda_mpr_str(m.first, p, true);
   }

   // Parse time filtering options
   fcst_lead       = odict.lookup_seconds_array(conf_key_fcst_lead);
   obs_lead        = odict.lookup_seconds_array(conf_key_obs_lead);

   fcst_valid_beg  = odict.lookup_unixtime(conf_key_fcst_valid_beg);
   fcst_valid_end  = odict.lookup_unixtime(conf_key_fcst_valid_end);
   fcst_valid_inc  = odict.lookup_unixtime_array(conf_key_fcst_valid_inc);
   fcst_valid_exc  = odict.lookup_unixtime_array(conf_key_fcst_valid_exc);
   fcst_valid_hour = odict.lookup_seconds_array(conf_key_fcst_valid_hour);

   obs_valid_beg   = odict.lookup_unixtime(conf_key_obs_valid_beg);
   obs_valid_end   = odict.lookup_unixtime(conf_key_obs_valid_end);
   obs_valid_inc   = odict.lookup_unixtime_array(conf_key_obs_valid_inc);
   obs_valid_exc   = odict.lookup_unixtime_array(conf_key_obs_valid_exc);
   obs_valid_hour  = odict.lookup_seconds_array(conf_key_obs_valid_hour);

   fcst_init_beg   = odict.lookup_unixtime(conf_key_fcst_init_beg);
   fcst_init_end   = odict.lookup_unixtime(conf_key_fcst_init_end);
   fcst_init_inc   = odict.lookup_unixtime_array(conf_key_fcst_init_inc);
   fcst_init_exc   = odict.lookup_unixtime_array(conf_key_fcst_init_exc);
   fcst_init_hour  = odict.lookup_seconds_array(conf_key_fcst_init_hour);

   obs_init_beg    = odict.lookup_unixtime(conf_key_obs_init_beg);
   obs_init_end    = odict.lookup_unixtime(conf_key_obs_init_end);
   obs_init_inc    = odict.lookup_unixtime_array(conf_key_obs_init_inc);
   obs_init_exc    = odict.lookup_unixtime_array(conf_key_obs_init_exc);
   obs_init_hour   = odict.lookup_seconds_array(conf_key_obs_init_hour);

   // Dump the contents of the current thresholds
   if(mlog.verbosity_level() >= 5) {
      mlog << Debug(5)
           << "Parsed thresholds:\n"
           << "Matched pair filter columns:     " << write_css(mpr_sa) << "\n"
           << "Matched pair filter thresholds:  " << mpr_ta.get_str() << "\n"
           << "Matched pair fcst lead time:     " << write_css_hhmmss(fcst_lead) << "\n"
           << "Matched pair obs lead time:      " << write_css_hhmmss(obs_lead) << "\n"
           << "Matched pair fcst valid time:    " << get_time_str(fcst_valid_beg, fcst_valid_end,
                                                                  fcst_valid_inc, fcst_valid_exc,
                                                                  fcst_valid_hour) << "\n"
           << "Matched pair obs valid time:     " << get_time_str(obs_valid_beg, obs_valid_end,
                                                                  obs_valid_inc, obs_valid_exc,
                                                                  obs_valid_hour) << "\n"
           << "Matched pair fcst init time:     " << get_time_str(fcst_init_beg, fcst_init_end,
                                                                  fcst_init_inc, fcst_init_exc,
                                                                  fcst_init_hour) << "\n"
           << "Matched pair obs init time:      " << get_time_str(obs_init_beg, obs_init_end,
                                                                  obs_init_inc, obs_init_exc,
                                                                  obs_init_hour) << "\n"
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

      mlog << Error << "\n" << method_name
           << "The number of thresholds for each field in \"fcst."
           << conf_key_cat_thresh
           << "\" must match the number of thresholds for each "
           << "field in \"obs." << conf_key_cat_thresh << "\".\n\n";
      exit(1);
   }

   // Add default continuous thresholds until the counts match
   int n = max(fcnt_ta.n(), ocnt_ta.n());
   while(fcnt_ta.n() < n) fcnt_ta.add(na_str);
   while(ocnt_ta.n() < n) ocnt_ta.add(na_str);

   // Add default wind speed thresholds until the counts match
   n = max(fwind_ta.n(), owind_ta.n());
   while(fwind_ta.n() < n) fwind_ta.add(na_str);
   while(owind_ta.n() < n) owind_ta.add(na_str);

   // Verifying with multi-category contingency tables
   if(!vx_pd.fcst_info->is_prob() &&
      (output_flag[i_mctc] != STATOutputType::None ||
       output_flag[i_mcts] != STATOutputType::None)) {
      check_mctc_thresh(fcat_ta);
      check_mctc_thresh(ocat_ta);
   }

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

   // Conf: hss_ec_value
   hss_ec_value = odict.lookup_double(conf_key_hss_ec_value);

   // Conf: rank_corr_flag
   rank_corr_flag = odict.lookup_bool(conf_key_rank_corr_flag);

   // Conf: desc
   vx_pd.set_desc(parse_conf_string(&odict, conf_key_desc, false).c_str());

   return;
}

////////////////////////////////////////////////////////////////////////

void PairStatVxOpt::set_vx_pd(PairStatConfInfo *conf_info) {
   ConcatString cs;
   StringArray sa;

   // Setup the VxPairDataPoint object for each mask

   // Check for at least one masking region
   if(get_n_mask() == 0) {
      mlog << Error << "\nPairStatVxOpt::set_vx_pd() -> "
           << "At least one output masking region must be requested in \""
           << conf_key_mask_grid  << "\", \""
           << conf_key_mask_poly  << "\", \""
           << conf_key_mask_sid   << "\", or \""
           << conf_key_mask_llpnt << "\".\n\n";
      exit(1);
   }

   // Define the dimensions with n_msg_typ = n_interp = 1
   vx_pd.set_size(1, get_n_mask(), 1);

   // Size the header objects 
   vx_hdr.resize(get_n_mask());

   // Store the MPR filtering maps
   vx_pd.set_mpr_thr_inc_map(mpr_thr_inc_map);
   vx_pd.set_mpr_str_inc_map(mpr_str_inc_map);
   vx_pd.set_mpr_str_exc_map(mpr_str_exc_map);

   // Store the climo CDF info
   vx_pd.set_climo_cdf_info_ptr(&cdf_info);

   // Set interpolation options for climatology data as nearest neighbor
   vx_pd.set_interp(0, InterpMthd::Nearest, 1,
                    GridTemplateFactory::GridTemplates::Square);

   // Define the masking information: grid, poly, sid, point
   int n;

   // Define the grid masks
   for(int i=0; i<mask_grid.n(); i++) {
      n = i;
      vx_pd.set_mask_area(n, mask_name[n].c_str(),
                          &(conf_info->mask_area_map[mask_name[n]]));
   }

   // Define the poly masks
   for(int i=0; i<mask_poly.n(); i++) {
      n = i + mask_grid.n();
      vx_pd.set_mask_area(n, mask_name[n].c_str(),
                          &(conf_info->mask_area_map[mask_name[n]]));
   }

   // Define the station ID masks
   for(int i=0; i<mask_sid.n(); i++) {
      n = i + mask_grid.n() + mask_poly.n();
      vx_pd.set_mask_sid(n, mask_name[n].c_str(),
                         &(conf_info->mask_sid_map[mask_name[n]]));
   }

   // Define the Lat/Lon point masks
   for(int i=0; i<(int) mask_llpnt.size(); i++) {
      n = i + mask_grid.n() + mask_poly.n() + mask_sid.n();
      vx_pd.set_mask_llpnt(n, mask_name[n].c_str(), &mask_llpnt[i]);
   }

   // After sizing VxPairDataPoint, add settings for each array element
   if(output_flag[i_seeps_mpr] != STATOutputType::None ||
      output_flag[i_seeps]     != STATOutputType::None) {
     vx_pd.load_seeps_climo(conf_info->seeps_climo_name);
     vx_pd.set_seeps_thresh(conf_info->seeps_p1_thresh);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairStatVxOpt::set_perc_thresh(const PairDataPoint *pd_ptr) {

   //
   // Compute percentiles for forecast and observation thresholds,
   // but not for wind speed or climatology CDF thresholds.
   //
   if(!fcat_ta.need_perc() && !ocat_ta.need_perc() &&
      !fcnt_ta.need_perc() && !ocnt_ta.need_perc()) return;

   //
   // Sort the input arrays
   //
   NumArray f_sort    = pd_ptr->f_na;
   NumArray o_sort    = pd_ptr->o_na;
   NumArray fcmn_sort = pd_ptr->fcmn_na;
   NumArray ocmn_sort = pd_ptr->ocmn_na;
   f_sort.sort_array();
   o_sort.sort_array();
   fcmn_sort.sort_array();
   ocmn_sort.sort_array();

   //
   // Compute percentiles
   //
   fcat_ta.set_perc(&f_sort, &o_sort, &fcmn_sort, &ocmn_sort, &fcat_ta, &ocat_ta);
   ocat_ta.set_perc(&f_sort, &o_sort, &fcmn_sort, &ocmn_sort, &fcat_ta, &ocat_ta);
   fcnt_ta.set_perc(&f_sort, &o_sort, &fcmn_sort, &ocmn_sort, &fcnt_ta, &ocnt_ta);
   ocnt_ta.set_perc(&f_sort, &o_sort, &fcmn_sort, &ocmn_sort, &fcnt_ta, &ocnt_ta);

   return;
}

////////////////////////////////////////////////////////////////////////

int PairStatVxOpt::n_txt_row(int i_txt_row) const {
   int n = 0;
   int n_bin;
   int n_pd = get_n_mask();
   const char *method_name = "PairStatVxOpt::n_txt_row(int) -> ";

   // Range check
   if(i_txt_row < 0 || i_txt_row >= n_txt) {
      mlog << Error << "\n" << method_name
           << "range check error for " << i_txt_row << "\n\n";
      exit(1);
   }

   // Check if this output line type is requested
   if(output_flag[i_txt_row] == STATOutputType::None) return 0;

   bool prob_flag = vx_pd.fcst_info->is_prob();
   bool vect_flag = vx_pd.fcst_info->is_v_wind() &&
                    vx_pd.fcst_info->uv_index() >= 0;

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

      case i_fho:
      case i_ctc:
         // Number of FHO or CTC lines =
         //    Message Types * Masks * Interpolations * Thresholds
         n = (prob_flag ? 0 : n_pd * get_n_cat_thresh());
         break;

      case i_cts:
         // Number of CTS lines =
         //    Message Types * Masks * Interpolations * Thresholds *
         //    Alphas
         n = (prob_flag ? 0 : n_pd * get_n_cat_thresh() *
              get_n_ci_alpha());
         break;

      case i_mctc:
         // Number of MCTC lines =
         //    Message Types * Masks * Interpolations
         n = (prob_flag ? 0 : n_pd);
         break;

      case i_mcts:
         // Number of MCTS lines =
         //    Message Types * Masks * Interpolations * Alphas
         n = (prob_flag ? 0 : n_pd * get_n_ci_alpha());
         break;

      case i_cnt:
         // Number of CNT lines =
         //    Message Types * Masks * Interpolations * Thresholds *
         //    Climo Bins * Alphas
         n = (prob_flag ? 0 : n_pd * get_n_cnt_thresh() * n_bin *
              get_n_ci_alpha());
         break;

      case i_sl1l2:
      case i_sal1l2:
         // Number of SL1L2 and SAL1L2 lines =
         //    Message Types * Masks * Interpolations * Thresholds *
         //    Climo Bins
         n = (prob_flag ? 0 : n_pd * get_n_cnt_thresh() * n_bin);
         break;

      case i_vcnt:
         // Number of VCNT lines =
         //    Message Types * Masks * Interpolations * Thresholds *
         //    Alphas
         n = (!vect_flag ? 0 : n_pd *
              get_n_wind_thresh() * get_n_ci_alpha());
         break;

      case i_vl1l2:
      case i_val1l2:
         // Number of VL1L2 or VAL1L2 lines =
         //    Message Types * Masks * Interpolations * Thresholds
         n = (!vect_flag ? 0 : n_pd *
              get_n_wind_thresh());
         break;

      case i_pct:
      case i_pjc:
      case i_prc:
         // Number of PCT, PJC, or PRC lines possible =
         //    Message Types * Masks * Interpolations * Thresholds *
         //    Climo Bins
         n = (!prob_flag ? 0 : n_pd * get_n_oprob_thresh() * n_bin);
         break;

      case i_pstd:
         // Number of PSTD lines =
         //    Message Types * Masks * Interpolations * Thresholds *
         //    Alphas * Climo Bins
         n = (!prob_flag ? 0 : n_pd *
              get_n_oprob_thresh() * get_n_ci_alpha() * n_bin);
         break;

      case i_eclv:
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

      case i_mpr:
         // Compute the number of matched pairs to be written
         n = vx_pd.get_n_pair();
         break;

      case i_seeps_mpr:
         // Compute the number of matched pairs to be written
         n = vx_pd.get_n_pair();
         break;

      case i_seeps:
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

int PairStatVxOpt::get_n_cnt_thresh() const {
   return (!vx_pd.fcst_info || vx_pd.fcst_info->is_prob()) ?
          0 : fcnt_ta.n();
}

////////////////////////////////////////////////////////////////////////

int PairStatVxOpt::get_n_cat_thresh() const {
   return (!vx_pd.fcst_info || vx_pd.fcst_info->is_prob()) ?
          0 : fcat_ta.n();
}

////////////////////////////////////////////////////////////////////////

int PairStatVxOpt::get_n_wind_thresh() const {
   return (!vx_pd.fcst_info || vx_pd.fcst_info->is_prob()) ?
          0 : fwind_ta.n();
}

////////////////////////////////////////////////////////////////////////

int PairStatVxOpt::get_n_fprob_thresh() const {
   return (!vx_pd.fcst_info || !vx_pd.fcst_info->is_prob()) ?
          0 : fcat_ta.n();
}

////////////////////////////////////////////////////////////////////////

int PairStatVxOpt::get_n_oprob_thresh() const {
   return (!vx_pd.fcst_info || !vx_pd.fcst_info->is_prob()) ?
           0 : ocat_ta.n();
}

////////////////////////////////////////////////////////////////////////

bool PairStatVxOpt::is_keeper_mpr(const STATLine &l) const {

   // Check name and level strings
   if(l.fcst_var() != vx_pd.fcst_info->name_attr()  || 
      l.fcst_lev() != vx_pd.fcst_info->level_attr() || 
      l.obs_var()  != vx_pd.obs_info->name_attr()   || 
      l.obs_lev()  != vx_pd.obs_info->level_attr()) return false;

   // Check MPR thresholds
   for(const auto &m : mpr_thr_inc_map) {
      if(!m.second.check_dbl(atof(l.get_item(m.first.c_str())))) return false;
   }

   // Check MPR string inclusions
   for(const auto &m : mpr_str_inc_map) {
      if(!m.second.has(l.get_item(m.first.c_str()))) return false;
   }

   // Check MPR string exclusions
   for(const auto &m : mpr_str_exc_map) {
      if(m.second.has(l.get_item(m.first.c_str()))) return false;
   }

   // Check lead time filters
   if(!is_keeper_lead_time(
      l.fcst_lead(), l.obs_lead())) return false;

   // Check forecast valid time filters
   if(!is_keeper_fcst_valid_time(
      l.fcst_valid_beg(), l.fcst_valid_end())) return false;

   // Check observation valid time filters
   if(!is_keeper_obs_valid_time(
      l.obs_valid_beg(), l.obs_valid_end())) return false;

   // Check forecast init time filters
   if(!is_keeper_fcst_init_time(
      l.fcst_init_beg(), l.fcst_init_end())) return false;

   // Check observation init time filters
   if(!is_keeper_obs_init_time(
      l.obs_init_beg(), l.obs_init_end())) return false;

   return true;
}

////////////////////////////////////////////////////////////////////////

bool PairStatVxOpt::is_keeper_ioda(const point_pair_t &p) const {

   // Check for bad data in required spots
   if(is_bad_data(p.lat)  || is_bad_data(p.lon) ||
      is_bad_data(p.fval) || is_bad_data(p.oval)) return false;

   // Check MPR thresholds
   for(auto &m : mpr_thr_inc_map) {
      if(!m.second.check_dbl(get_ioda_mpr_val(m.first, p))) return false;
   }

   // Check MPR string inclusions
   for(auto &m : mpr_str_inc_map) {
      if(!m.second.has(get_ioda_mpr_str(m.first, p))) return false;
   }

   // Check MPR string exclusions
   for(auto &m : mpr_str_exc_map) {
      if(m.second.has(get_ioda_mpr_str(m.first, p))) return false;
   }

   // Check timing filters for IODA data.
   // Assume an observation lead time of 0 and observation
   // initialization time equal to the valid time.

   // Check lead time filters
   if(!is_keeper_lead_time(p.lead, 0)) return false;

   // Check forecast valid time filters
   if(!is_keeper_fcst_valid_time(p.ut, p.ut)) return false;

   // Check observation valid time filters
   if(!is_keeper_obs_valid_time(p.ut, p.ut)) return false;

   // Check forecast init time filters
   unixtime fcst_init_ut = (p.ut == 0 || is_bad_data(p.lead) ?
                            0 : p.ut - p.lead);
   if(!is_keeper_fcst_init_time(fcst_init_ut, fcst_init_ut)) return false;

   // Check observation init time filters
   if(!is_keeper_obs_init_time(p.ut, p.ut)) return false;

   return true;
}

////////////////////////////////////////////////////////////////////////

bool PairStatVxOpt::is_keeper_lead_time(const int f_sec,
                                        const int o_sec) const {
   // Check lead times
   if((fcst_lead.n() > 0 && !fcst_lead.has(f_sec)) ||
      (obs_lead.n() > 0 && !obs_lead.has(o_sec))) return false;

   return true;
}

////////////////////////////////////////////////////////////////////////

bool PairStatVxOpt::is_keeper_fcst_valid_time(const unixtime beg_ut,
                                              const unixtime end_ut) const {
   return is_keeper_unixtime(beg_ut, end_ut,
                             fcst_valid_beg, fcst_valid_end,
                             fcst_valid_inc, fcst_valid_exc,
                             fcst_valid_hour);
}

////////////////////////////////////////////////////////////////////////

bool PairStatVxOpt::is_keeper_obs_valid_time(const unixtime beg_ut,
                                             const unixtime end_ut) const {
   return is_keeper_unixtime(beg_ut, end_ut,
                             obs_valid_beg, obs_valid_end,
                             obs_valid_inc, obs_valid_exc,
                             obs_valid_hour);
}

////////////////////////////////////////////////////////////////////////

bool PairStatVxOpt::is_keeper_fcst_init_time(const unixtime beg_ut,
                                             const unixtime end_ut) const {
   return is_keeper_unixtime(beg_ut, end_ut,
                             fcst_init_beg, fcst_init_end,
                             fcst_init_inc, fcst_init_exc,
                             fcst_init_hour);

}

////////////////////////////////////////////////////////////////////////

bool PairStatVxOpt::is_keeper_obs_init_time(const unixtime beg_ut,
                                            const unixtime end_ut) const {
   return is_keeper_unixtime(beg_ut, end_ut,
                             obs_init_beg, obs_init_end,
                             obs_init_inc, obs_init_exc,
                             obs_init_hour);
}

////////////////////////////////////////////////////////////////////////

bool PairStatVxOpt::add_mpr_line(STATLine l) {
   bool keep = false;

   // Apply conversion and censoring logic
   if(convert_censor_flag) apply_convert_censor(l);

   // Check filtering options
   if(is_keeper_mpr(l)) {

      // Store values
      // Use OBS_ELV for both the station elevation and
      // the observation height
      double obs_lat = atof(l.get_item("OBS_LAT"));
      double obs_lon = -1.0*atof(l.get_item("OBS_LON"));
      double obs_elv = atof(l.get_item("OBS_ELV"));
      double obs_val = atof(l.get_item("OBS"));
      double obs_lvl = atof(l.get_item("OBS_LVL"));
      double obs_hgt = atof(l.get_item("OBS_ELV"));

      // Climo data for this pair
      ClimoPntInfo cpi;

      // Parse climo data from the config file
      if(grid_climo.nxy() > 0) {
         double obs_x;
         double obs_y;
         grid_climo.latlon_to_xy(obs_lat, obs_lon, obs_x, obs_y);
         cpi = vx_pd.get_climo_pnt_info(vx_pd.pb_ptr[0],
                                        grid_climo, obs_x, obs_y,
                                        obs_val, obs_lvl, obs_hgt);
      }
      // Otherwise, parse from the input line
      else {

         // In met-6.1 and later:
         // - CLIMO was replaced by CLIMO_MEAN
         if(l.has("CLIMO")) {
            double cmn = atof(l.get_item("CLIMO"));
            double csd = bad_data_double;
            cpi.set(cmn, csd, cmn, csd);
         }
         // In met-12.0.0 and later:
         // - CLIMO_MEAN was replaced by OBS_CLIMO_MEAN
         // - CLIMO_STDEV was replaced by OBS_CLIMO_STDEV
         // - CLIMO_CDF was replaced by OBS_CLIMO_CDF
         else if(l.has("CLIMO_MEAN")) {
            double cmn = atof(l.get_item("CLIMO_MEAN"));
            double csd = atof(l.get_item("CLIMO_STDEV"));
            cpi.set(cmn, csd, cmn, csd);
         }
         else {
            cpi.set(atof(l.get_item("FCST_CLIMO_MEAN")),
                    atof(l.get_item("FCST_CLIMO_STDEV")),
                    atof(l.get_item("OBS_CLIMO_MEAN")),
                    atof(l.get_item("OBS_CLIMO_STDEV")));
         }
      }

      // Attempt to add to each masking region
      for(int i_mask=0; i_mask<get_n_mask(); i_mask++) {

         // Convert lat/lon to x/y
         double obs_x;
         double obs_y;
         grid_mask.latlon_to_xy(obs_lat, obs_lon, obs_x, obs_y);

         // Check masking region
         if(!vx_pd.is_keeper_mask(
               l.get_line(), 0, i_mask,
               nint(obs_x),
               nint(obs_y),
               l.get_item("OBS_SID"),
               obs_lat,
               obs_lon)) continue;

         // Add the pair 
         if(vx_pd.pd[i_mask].add_point_pair(
               l.obtype(),
               l.get_item("OBS_SID"),
               obs_lat, obs_lon, obs_elv,
               bad_data_double, bad_data_double,
               timestring_to_sec(l.get_item("FCST_LEAD")),
               timestring_to_unix(l.get_item("OBS_VALID_BEG")),
               obs_lvl, obs_hgt,
               atof(l.get_item("FCST")),
               obs_val,
               l.get_item("OBS_QC"),
               cpi,
               1.0)) { 

            // Using this line for at least one masking region
            keep = true;

            // Track the unique headers
            vx_hdr[i_mask].add(l);
         }
      } // end for i_mask
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////

bool PairStatVxOpt::add_ioda_pair(point_pair_t p) {
   bool keep = false;

   // Apply conversion and censoring logic
   if(convert_censor_flag) apply_convert_censor(p);

   // Check filtering options
   if(is_keeper_ioda(p)) {

      // Store values
      double obs_lat = p.lat;
      double obs_lon = rescale_lon(-1.0*p.lon);
      double obs_elv = p.elv;
      double obs_val = p.oval;
      double obs_lvl = (is_bad_data(p.lvl) ? bad_data_double : p.lvl/100.0);
      double obs_hgt = p.hgt;

      // Parse climo data from the config file
      ClimoPntInfo cpi;
      if(grid_climo.nxy() > 0) {
         double obs_x;
         double obs_y;
         grid_climo.latlon_to_xy(obs_lat, obs_lon, obs_x, obs_y);
         cpi = vx_pd.get_climo_pnt_info(vx_pd.pb_ptr[0],
                                        grid_climo, obs_x, obs_y,
                                        obs_val, obs_lvl, obs_hgt);
      }

      // Attempt to add to each masking region
      for(int i_mask=0; i_mask<get_n_mask(); i_mask++) {

         // Convert lat/lon to x/y
         double obs_x;
         double obs_y;
         grid_mask.latlon_to_xy(obs_lat, obs_lon, obs_x, obs_y);

         // Check masking region
         if(!vx_pd.is_keeper_mask(
               "IODA pair data", 0, i_mask,
               nint(obs_x),
               nint(obs_y),
               p.sid.c_str(),
               obs_lat,
               obs_lon)) continue;

         // Add the pair
	 // - Convert pressure from Pa to HPa
         if(vx_pd.pd[i_mask].add_point_pair(
               p.typ.c_str(), p.sid.c_str(),
               obs_lat, obs_lon, obs_elv,
               bad_data_double, bad_data_double,
               p.lead, p.ut, obs_lvl, obs_hgt,
               p.fval, obs_val, na_str, cpi, 1.0)) {

            // Using this pair for at least one masking region
            keep = true;

            // Track the unique headers
            vx_hdr[i_mask].add(p.lead, p.ut,
               vx_pd.fcst_info->name_attr(),
               vx_pd.obs_info->name_attr(),
               p.typ);
         }
      } // end for i_mask
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////

void PairStatVxOpt::apply_convert_censor(STATLine &l) const {

   // Apply logic to the forecast and observation values
   double fval = atof(l.get_item("FCST"));
   apply_convert_censor(vx_pd.fcst_info, fval);

   double oval = atof(l.get_item("OBS"));
   apply_convert_censor(vx_pd.obs_info, oval);

   // Store the result
   l.set_item(l.get_offset("FCST"), to_string(fval));
   l.set_item(l.get_offset("OBS"),  to_string(oval));

   return;
}

////////////////////////////////////////////////////////////////////////

void PairStatVxOpt::apply_convert_censor(point_pair_t &p) const {

   // Apply logic to the forecast and observation values
   apply_convert_censor(vx_pd.fcst_info, p.fval);
   apply_convert_censor(vx_pd.obs_info,  p.oval);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairStatVxOpt::apply_convert_censor(const VarInfo *var_info,
                                         double &v) const {
   // Check for null
   if(!var_info) return;

   // Apply conversion logic
   if(var_info->ConvertFx.is_set()) {
     v = var_info->ConvertFx(v);
   }

   // Apply censoring logic
   for(int i=0; i<var_info->censor_thresh().n(); i++) {

      // Break out after the first match
      if(var_info->censor_thresh()[i].check(v)) {
         v = var_info->censor_val()[i];
         break;
      }
   }

   return;
}
 
////////////////////////////////////////////////////////////////////////
//
// Begin utility functions
//
////////////////////////////////////////////////////////////////////////

static ConcatString get_time_str(
                       const unixtime beg, const unixtime end,
                       const TimeArray &inc, const TimeArray &exc,
                       const IntArray &hr) {
   ConcatString cs;
   cs << "("  << (beg == 0 ? na_str : unix_to_yyyymmdd_hhmmss(beg))
      << ", " << (end == 0 ? na_str : unix_to_yyyymmdd_hhmmss(end))
      << "), inc: "  << (inc.n() == 0 ? "(nul)" : write_css(inc))
      <<  ", exc: "  << (exc.n() == 0 ? "(nul)" : write_css(exc))
      <<  ", hour: " << (hr.n()  == 0 ? "(nul)" : write_css_hhmmss(hr));

   return cs;
}

////////////////////////////////////////////////////////////////////////

static bool is_keeper_unixtime(const unixtime beg_ut, const unixtime end_ut,
                               const unixtime conf_beg_ut, const unixtime conf_end_ut,
                               const TimeArray &conf_inc, const TimeArray &conf_exc,
                               const IntArray &conf_hour) {

   // Check configuration time limits
   if((conf_beg_ut != 0 && beg_ut < conf_beg_ut) ||
      (conf_end_ut != 0 && beg_ut > conf_end_ut)) return false;

   // Check inclusion list
   if(conf_inc.n() > 0 &&
      !conf_inc.has(beg_ut) && !conf_inc.has(end_ut)) return false;

   // Check exclusion list
   if(conf_exc.n() > 0 &&
      (conf_exc.has(beg_ut) || conf_exc.has(end_ut))) return false;

   // Check hour
   if(conf_hour.n() > 0 &&
      !conf_hour.has(unix_to_sec_of_day(beg_ut)) &&
      !conf_hour.has(unix_to_sec_of_day(end_ut))) return false;

   return true;
}

////////////////////////////////////////////////////////////////////////

static double get_ioda_mpr_val(const string &mpr_col,
                               const point_pair_t &p,
                               bool print_warning) {
   const char *method_name = "get_ioda_mpr_val() -> ";
   double val = bad_data_double;

   // Get the numeric value
   if(mpr_col == "OBS_LAT") {
      val = p.lat;
   }
   else if(mpr_col == "OBS_LON") {
      val = p.lon;
   }
   else if(mpr_col == "FCST_VALID_BEG" ||
           mpr_col == "FCST_VALID_END" ||
           mpr_col == "OBS_VALID_BEG"  ||
           mpr_col == "OBS_VALID_END") {
      val = (double) p.ut;
   }
   else if(mpr_col == "OBS_LEV") {
      // Convert Pa to HPa
      val = p.lvl/100.0;
   }
   else if(mpr_col == "OBS_ELV") {
      val = p.hgt;
   }
   else if(mpr_col == "FCST") {
      val = p.fval;
   }
   else if(mpr_col == "OBS") {
      val = p.oval;
   }
   else if(print_warning) {
      mlog << Warning << "\n" << method_name
           << "non-numeric column (" << mpr_col
           << ") requested in the \"" << conf_key_mpr_column
           << "\" configuration option.\n\n";
   }

   return val;
}

////////////////////////////////////////////////////////////////////////

static string get_ioda_mpr_str(const string &mpr_col,
                               const point_pair_t &p,
                               bool print_warning) {
   const char *method_name = "get_ioda_mpr_str() -> ";
   string str(na_str);

   // Get the string value
   if(mpr_col == "OBTYPE") {
      str = p.typ;
   }
   else if(mpr_col == "OBS_SID") {
      str = p.sid;
   }
   else if(mpr_col == "FCST_VALID_BEG" ||
           mpr_col == "FCST_VALID_END" ||
           mpr_col == "OBS_VALID_BEG"  ||
           mpr_col == "OBS_VALID_END") {
      str = unix_to_yyyymmdd_hhmmss(p.ut);
   }
   else if(print_warning) {
      mlog << Warning << "\n" << method_name
           << "non-string column (" << mpr_col
           << ") requested in the \"" << conf_key_mpr_str_inc
           << "\" or \"" << conf_key_mpr_str_exc
           << "\" configuration option.\n\n";
   }

   return str;
}

////////////////////////////////////////////////////////////////////////
