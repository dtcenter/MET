// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <algorithm>

#include "pair_base.h"

#include "vx_data2d_factory.h"
#include "vx_util.h"
#include "vx_grid.h"
#include "vx_gsl_prob.h"
#include "vx_data2d.h"
#include "vx_data2d_grib.h"
#include "vx_math.h"
#include "vx_log.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

static bool sort_obs(ob_val_t a, ob_val_t b) { return a.val<b.val; }

////////////////////////////////////////////////////////////////////////
//
// Code for struct station_values_t
//
////////////////////////////////////////////////////////////////////////

void station_values_t::clear() {
   sid.clear();
   lat = lon = x = y = wgt = bad_data_double;
   ut = (unixtime) 0;
   lvl = elv = bad_data_double;
   fcmn = fcsd = ocmn = ocsd = bad_data_double;
   summary_val = bad_data_double;
   obs.clear();
}

////////////////////////////////////////////////////////////////////////
//
// Code for class PairBase
//
////////////////////////////////////////////////////////////////////////

PairBase::PairBase() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PairBase::~PairBase() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void PairBase::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::clear() {

   IsPointVx = false;

   mask_name.clear();
   mask_area_ptr  = (MaskPlane *)  nullptr;  // Not allocated
   mask_sid_ptr   = (MaskSID *)    nullptr;  // Not allocated
   mask_llpnt_ptr = (MaskLatLon *) nullptr;  // Not allocated

   cdf_info_ptr = (const ClimoCDFInfo *) nullptr;  // Not allocated

   msg_typ.clear();
   msg_typ_vals.clear();

   interp_wdth = 0;
   interp_mthd = InterpMthd::None;
   interp_shape = GridTemplateFactory::GridTemplates::None;

   o_na.clear();
   x_na.clear();
   y_na.clear();
   wgt_na.clear();      
   fcmn_na.clear();
   fcsd_na.clear();
   ocmn_na.clear();
   ocsd_na.clear();
   ocdf_na.clear();

   sid_sa.clear();
   lat_na.clear();
   lon_na.clear();
   vld_ta.clear();
   lvl_na.clear();
   elv_na.clear();
   o_qc_sa.clear();

   n_obs = 0;

   fcst_ut = 0;

   obs_summary = ObsSummary::None;
   obs_perc_value = bad_data_int;
   check_unique = false;

   map_key.clear();
   map_val.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::erase() {

   IsPointVx = false;

   mask_name.erase();
   mask_area_ptr  = (MaskPlane *)  nullptr;  // Not allocated
   mask_sid_ptr   = (MaskSID *)    nullptr;  // Not allocated
   mask_llpnt_ptr = (MaskLatLon *) nullptr;  // Not allocated

   cdf_info_ptr = (const ClimoCDFInfo *) nullptr;  // Not allocated

   msg_typ.clear();
   msg_typ_vals.clear();

   interp_mthd = InterpMthd::None;
   interp_shape = GridTemplateFactory::GridTemplates::None;

   o_na.erase();
   x_na.erase();
   y_na.erase();
   wgt_na.erase();      
   fcmn_na.erase();
   fcsd_na.erase();
   ocmn_na.erase();
   ocsd_na.erase();
   ocdf_na.erase();

   sid_sa.clear();  // no erase option
   lat_na.erase();
   lon_na.erase();
   vld_ta.erase();
   lvl_na.erase();
   elv_na.erase();
   o_qc_sa.clear(); // no erase option

   n_obs = 0;

   fcst_ut = 0;

   obs_summary = ObsSummary::None;
   obs_perc_value = bad_data_int;
   check_unique = false;

   map_key.clear();
   map_val.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::extend(int n) {

   o_na.extend  (n);
   x_na.extend  (n);
   y_na.extend  (n);
   wgt_na.extend(n);

   fcmn_na.extend(n);
   fcsd_na.extend(n);
   ocmn_na.extend(n);
   ocsd_na.extend(n);
   ocdf_na.extend(n);

   if(IsPointVx) {
      lat_na.extend(n);
      lon_na.extend(n);
      vld_ta.extend(n);
      lvl_na.extend(n);
      elv_na.extend(n);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_mask_name(const string &s) {

   mask_name = s;

  return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_mask_area_ptr(MaskPlane *mp_ptr) {

   mask_area_ptr = mp_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_mask_sid_ptr(MaskSID *ms_ptr) {

   mask_sid_ptr = ms_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_mask_llpnt_ptr(MaskLatLon *llpnt_ptr) {

   mask_llpnt_ptr = llpnt_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_climo_cdf_info_ptr(const ClimoCDFInfo *info_ptr) {

   cdf_info_ptr = info_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_msg_typ(const char *c) {

   msg_typ = c;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_msg_typ_vals(const StringArray &sa) {

   msg_typ_vals = sa;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_interp_mthd(const char *str) {

   interp_mthd = string_to_interpmthd(str);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_interp_mthd(InterpMthd m) {

   interp_mthd = m;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_interp_wdth(int n) {

   interp_wdth = n;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_interp_shape(GridTemplateFactory::GridTemplates shape) {

   interp_shape = shape;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_fcst_ut(unixtime ut){

   fcst_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_check_unique(bool check){

   check_unique = check;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_obs_summary(ObsSummary s) {

  obs_summary = s;

  return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_obs_perc_value(int i) {

  obs_perc_value = i;

  return;
}

////////////////////////////////////////////////////////////////////////

int PairBase::has_obs_rec(const char *sid, double lat, double lon,
                          double x, double y, double lvl, double elv,
                          int &i_obs) {
   int i, status = 0;

   //
   // Only valid for point data
   //
   if(!IsPointVx) return false;

   //
   // Check for an existing record of this observation
   //
   for(i=0, i_obs=-1; i<n_obs; i++) {

      if(sid_sa[i] == sid &&
         is_eq(lat_na[i], lat) &&
         is_eq(lon_na[i], lon) &&
         is_eq(lvl_na[i], lvl) &&
         is_eq(elv_na[i], elv)) {
         status = 1;
         i_obs = i;
         break;
      }
   } // end for

   return status;
}

////////////////////////////////////////////////////////////////////////

void PairBase::add_climo(double obs,
                         const ClimoPntInfo &cpi) {

   // Compute and store the climatology information
   fcmn_na.add(cpi.fcmn);
   fcsd_na.add(cpi.fcsd);
   ocmn_na.add(cpi.ocmn);
   ocsd_na.add(cpi.ocsd);
   ocdf_na.add(normal_cdf(obs, cpi.ocmn, cpi.ocsd));

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_climo(int i_obs, double obs,
                         const ClimoPntInfo &cpi) {

   // Compute and store the climatology information
   fcmn_na.set(i_obs, cpi.fcmn);
   fcsd_na.set(i_obs, cpi.fcsd);
   ocmn_na.set(i_obs, cpi.ocmn);
   ocsd_na.set(i_obs, cpi.ocsd);
   ocdf_na.set(i_obs, normal_cdf(obs, cpi.ocmn, cpi.ocsd));

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::compute_climo_cdf() {
   int i;

   // The o_na, ocmn_na, and ocsd_na have already been populated
   if(o_na.n() != ocmn_na.n() || o_na.n() != ocsd_na.n()) {
      mlog << Error << "\nPairBase::compute_climo_cdf() -> "
           << "the observation, climo mean, and climo stdev arrays "
           << "must all have the same length (" << o_na.n() << ").\n\n";
      exit(1);
   }

   ocdf_na.extend(o_na.n());

   for(i=0; i<o_na.n(); i++) {
      ocdf_na.add(is_bad_data(o_na[i])   ||
                  is_bad_data(ocmn_na[i]) ||
                  is_bad_data(ocsd_na[i]) ?
                  bad_data_double :
                  normal_cdf(o_na[i], ocmn_na[i], ocsd_na[i]));
   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool PairBase::add_point_obs(const char *sid,
                             double lat, double lon, double x, double y,
                             unixtime ut, double lvl, double elv,
                             double o, const char *qc,
                             const ClimoPntInfo &cpi, double wgt) {

   //
   // Set or check the IsPointVx flag
   //
   if(n_obs == 0) {
      IsPointVx = true;
   }
   else if(!IsPointVx) {
      mlog << Error << "\nPairBase::add_point_obs() -> "
           << "should not be called for gridded verification!\n\n";
      exit(1);
   }

   bool ret = false;

   //  build a uniqueness test key
   string obs_key = str_format("%.3f:%.3f:%.2f:%.2f",
                       lat,         //  lat
                       lon,         //  lon
                       lvl,         //  level
                       elv).text(); //  elevation

   //  add a single value reporting string to the reporting map
   ob_val_t ob_val;
   ob_val.ut  = ut;
   ob_val.val = o;
   ob_val.qc  = qc;

   //  if key exists, add new observation to vector, else add new pair
   map<string,station_values_t>::iterator it = map_val.find(obs_key);
   if(it != map_val.end()) {

     if(check_unique) {
        vector<ob_val_t>::iterator o_it = (*it).second.obs.begin();
        for(;o_it != (*it).second.obs.end(); o_it++) {
           if((*o_it).ut == ut) return false;
        }
     }

     (*it).second.obs.push_back(ob_val);

   }
   else {
      station_values_t val;
      val.sid = string(sid);
      val.lat = lat;
      val.lon = lon;
      val.x   = x;
      val.y   = y;
      val.wgt = wgt;
      val.ut  = fcst_ut;
      val.lvl = lvl;
      val.elv = elv;
      val.fcmn = cpi.fcmn;
      val.fcsd = cpi.fcsd;
      val.ocmn = cpi.ocmn;
      val.ocsd = cpi.ocsd;

      val.obs.push_back(ob_val);
      map_key.add(obs_key.c_str());
      map_val.insert( pair<string,station_values_t>(obs_key, val) );
      ret = true;
   }

   if(obs_summary == ObsSummary::None) {
      sid_sa.add(sid);
      lat_na.add(lat);
      lon_na.add(lon);
      x_na.add(x);
      y_na.add(y);
      wgt_na.add(wgt);
      vld_ta.add(ut);
      lvl_na.add(lvl);
      elv_na.add(elv);
      o_na.add(o);
      o_qc_sa.add(qc);
      add_climo(o, cpi);

      // Increment the number of pairs
      n_obs += 1;
      ret = true;
   }

   return ret;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_point_obs(int i_obs, const char *sid,
                             double lat, double lon, double x, double y,
                             unixtime ut, double lvl, double elv,
                             double o, const char *qc,
                             const ClimoPntInfo &cpi,
                             double wgt) {

   if(i_obs < 0 || i_obs >= n_obs) {
      mlog << Error << "\nPairBase::set_point_obs() -> "
           << "range check error: " << i_obs << " not in (0, "
           << n_obs << ").\n\n";
      exit(1);
   }

   sid_sa.set(i_obs, sid);
   lat_na.set(i_obs, lat);
   lon_na.set(i_obs, lon);
   x_na.set(i_obs, x);
   y_na.set(i_obs, y);
   wgt_na.set(i_obs, wgt);
   vld_ta.set(i_obs, ut);
   lvl_na.set(i_obs, lvl);
   elv_na.set(i_obs, elv);
   o_na.set(i_obs, o);
   o_qc_sa.set(i_obs, qc);
   set_climo(i_obs, o, cpi);

   return;
}

////////////////////////////////////////////////////////////////////////

ob_val_t PairBase::compute_nearest(string obs_key) {
   station_values_t svt = map_val[obs_key];
   vector<ob_val_t>::iterator it = svt.obs.begin();
   ob_val_t out = (*it);
   int ut_diff = labs(svt.ut - (*it).ut);
   for(; it != svt.obs.end(); it++) {
      int new_diff = labs(svt.ut - (*it).ut);
      if( ut_diff > new_diff) {
         ut_diff = new_diff;
         out = (*it);
      }
   }

   return out;
}

////////////////////////////////////////////////////////////////////////

ob_val_t PairBase::compute_min(string obs_key) {
   station_values_t svt = map_val[obs_key];
   vector<ob_val_t>::iterator it = svt.obs.begin();
   ob_val_t out = (*it);
   for(; it != svt.obs.end(); it++) {
      if( out.val > (*it).val) {
         out = (*it);
      }
   }

   return out;
}

////////////////////////////////////////////////////////////////////////

ob_val_t PairBase::compute_max(string obs_key) {
   station_values_t svt = map_val[obs_key];
   vector<ob_val_t>::iterator it = svt.obs.begin();
   ob_val_t out = (*it);
   for(; it != svt.obs.end(); it++) {
     if( out.val < (*it).val) {
       out = (*it);
      }
   }

   return out;
}

////////////////////////////////////////////////////////////////////////

ob_val_t PairBase::compute_uw_mean(string obs_key) {
   double total = 0.0;
   int count = 0;
   ob_val_t out;
   station_values_t svt = map_val[obs_key];
   vector<ob_val_t>::iterator it = svt.obs.begin();
   out.qc = (*it).qc;
   for(; it != svt.obs.end(); it++) {
     total += (*it).val;
     if( (*it).qc != out.qc ) {
       out.qc = "NA";
     }
     count++;
   }
   out.ut  = svt.ut;
   out.val = total / double(count);
   return out;
}

////////////////////////////////////////////////////////////////////////

ob_val_t PairBase::compute_dw_mean(string obs_key) {
   double total, weight, total_weight;
   ob_val_t out;
   station_values_t svt = map_val[obs_key];

   vector<ob_val_t>::iterator it = svt.obs.begin();
   out.qc = (*it).qc;
   for(total=total_weight=0.0; it != svt.obs.end(); it++) {
     if( svt.ut == (*it).ut) return *it;
     weight = 1.0 / pow( labs( svt.ut - (*it).ut ), 2.0);

     total_weight += weight;
     total += (*it).val * weight;
     if( (*it).qc != out.qc ) {
       out.qc = "NA";
     }
   }

   out.ut  = svt.ut;
   out.val = total / total_weight;
   return out;
}


////////////////////////////////////////////////////////////////////////

ob_val_t PairBase::compute_percentile(string obs_key, int perc) {
   station_values_t svt = map_val[obs_key];
   vector<ob_val_t> obs = svt.obs;
   std::sort(obs.begin(), obs.end(), sort_obs);

   int index = ceil((perc/100.0) * (obs.size()-1));

   return obs[index];
}

////////////////////////////////////////////////////////////////////////

void PairBase::print_obs_summary() const {

   if(!IsPointVx) return;

   if(obs_summary == ObsSummary::None ||
      mlog.verbosity_level() < 4 ||
      !map_val.size()) return;

   //  iterate over ordered list map keys in the station id map
   for(int i=0; i<map_key.n(); i++) {

      station_values_t svt = map_val.at(map_key[i]);

      mlog << Debug(4)
           << "Computed " << obssummary_to_string(obs_summary, obs_perc_value)
           << " of " << (int) svt.obs.size()
           << " observations = " << svt.summary_val
           << " for [lat:lon:level:elevation] = ["
           << map_key[i] << "]\n";

      //  parse and print the point obs information for the current key
      vector<ob_val_t>::iterator o_it = svt.obs.begin();

      for(; o_it != svt.obs.end(); o_it++) {
         mlog << Debug(4) << "  "
              << "[sid: " << svt.sid.c_str()
              << " vld: " << unix_to_yyyymmdd_hhmmss( (*o_it).ut )
              << " obs: " << (*o_it).val << "]\n";
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::calc_obs_summary(){

   if(!IsPointVx) return;

   //  iterate over the keys in the unique station id map
   for(int i=0; i<map_key.n(); i++) {

      station_values_t svt = map_val[map_key[i]];

      //  parse the single key string
      char** mat = nullptr;
      if( 5 != regex_apply("^([^:]+):([^:]+):([^:]+):([^:]+)$", 5,
                           map_key[i].c_str(), mat) ){
         mlog << Error << "\nPairBase::calc_obs_summary() -> "
              << "regex_apply failed to parse '"
              << map_key[i] << "'\n\n";
         exit(1);
      }

      string msg_key = str_format("%s:%s:%s:%s", mat[1], mat[2], mat[3],
                                  mat[4]).text();

      ob_val_t ob;

      regex_clean(mat);

      switch(obs_summary) {
         case ObsSummary::Nearest:
            ob = compute_nearest(msg_key);
            break;
         case ObsSummary::Min:
            ob = compute_min(msg_key);
            break;
         case ObsSummary::Max:
            ob = compute_max(msg_key);
            break;
         case ObsSummary::UW_Mean:
            ob = compute_uw_mean(msg_key);
            break;
         case ObsSummary::DW_Mean:
            ob = compute_dw_mean(msg_key);
            break;
         case ObsSummary::Median:
            ob = compute_percentile(msg_key, 50);
            break;
         case ObsSummary::Perc:
            ob = compute_percentile(msg_key, obs_perc_value);
            break;
         case ObsSummary::None:
         default:
            return;
      }

      // Store summarized value in the map
      svt.summary_val = ob.val;

      sid_sa.add    (svt.sid.c_str());
      lat_na.add    (svt.lat);
      lon_na.add    (svt.lon);
      x_na.add      (svt.x);
      y_na.add      (svt.y);
      wgt_na.add    (svt.wgt);
      vld_ta.add    (ob.ut);
      lvl_na.add    (svt.lvl);
      elv_na.add    (svt.elv);
      o_na.add      (ob.val);
      o_qc_sa.add   (ob.qc.c_str());
      ClimoPntInfo cpi(svt.fcmn, svt.fcsd, svt.ocmn, svt.ocsd);
      add_climo(ob.val, cpi);

      // Increment the number of pairs
      n_obs += 1;

   } // end for map

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_point_weight(const PointWeightType wgt_flag) {

   const char *method_name = "PairBase::set_point_weight() -> ";

   if(!IsPointVx || wgt_flag == PointWeightType::None) return;

   // Apply the SID point weight type
   if(wgt_flag == PointWeightType::SID &&
      mask_sid_ptr != nullptr) {

      mlog << Debug(4)
           << "Applying point weights for the \""
           << mask_sid_ptr->name << "\" station ID masking region.\n";

      // Print warning if no weights are provided
      if(!mask_sid_ptr->has_weights) {
         mlog << Warning << "\n" << method_name
              << "station ID point weighting requested but no weights "
              << "were defined in the \"" << mask_sid_ptr->name
              << "\" station ID mask. Using default weights of "
              << default_weight << ".\n\n";
      }

      // Loop through the point observations
      for(int i_obs=0; i_obs<n_obs; i_obs++) {

         pair<string,double> *item_ptr = nullptr;
         if(mask_sid_ptr->has(sid_sa[i_obs], item_ptr)) {
            wgt_na.set(i_obs, item_ptr->second);
         }
         else {
            mlog << Warning << "\n" << method_name
                 << "no match found for station id: "
                 << sid_sa[i_obs] << "\n\n";
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::add_grid_obs(double o,
                            const ClimoPntInfo &cpi,
                            double wgt) {

   //
   // Set or check the IsPointVx flag
   //
   if(n_obs == 0) {
      IsPointVx = false;
   }
   else if(IsPointVx) {
      mlog << Error << "\nPairBase::add_grid_obs() -> "
           << "should not be called for point verification!\n\n";
      exit(1);
   }

   o_na.add(o);
   wgt_na.add(wgt);
   add_climo(o, cpi);

   // Increment the number of observations
   n_obs += 1;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::add_grid_obs(double x, double y, double o,
                            const ClimoPntInfo &cpi,
                            double wgt) {

   add_grid_obs(o, cpi, wgt);

   x_na.add(x);
   y_na.add(y);

   return;
}

////////////////////////////////////////////////////////////////////////

double PairBase::process_obs(const VarInfo *vinfo, double v) const {

   if(!vinfo) return v;

   double new_v = v;

   // Apply conversion logic.
   if(vinfo->ConvertFx.is_set()) {
      new_v = vinfo->ConvertFx(new_v);
   }

   // Apply censor logic.
   for(int i=0; i<vinfo->censor_thresh().n(); i++) {

      // Break out after the first match.
      if(vinfo->censor_thresh()[i].check(new_v)) {
         new_v = vinfo->censor_val()[i];
         break;
      }
   }

   return new_v;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class VxPairBase
//
////////////////////////////////////////////////////////////////////////

VxPairBase::VxPairBase() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

VxPairBase::~VxPairBase() {
   clear();
}

////////////////////////////////////////////////////////////////////////

VxPairBase::VxPairBase(const VxPairBase &vx_pb) {

   init_from_scratch();

   assign(vx_pb);
}

////////////////////////////////////////////////////////////////////////

VxPairBase & VxPairBase::operator=(const VxPairBase &vx_pb) {

   if(this == &vx_pb) return *this;

   assign(vx_pb);

   return *this;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::init_from_scratch() {

   fcst_info = (VarInfo *) nullptr;
   obs_info  = (VarInfo *) nullptr;

   fclm_info = (VarInfo *) nullptr;
   oclm_info = (VarInfo *) nullptr;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::clear() {

   if(fcst_info) { delete fcst_info; fcst_info = (VarInfo *) nullptr; }
   if(obs_info)  { delete obs_info;  obs_info  = (VarInfo *) nullptr; }

   if(fclm_info) { delete fclm_info; fclm_info = (VarInfo *) nullptr; }
   if(oclm_info) { delete oclm_info; oclm_info = (VarInfo *) nullptr; }

   desc.clear();

   interp_thresh = 0;

   fcst_dpa.clear();
   fcmn_dpa.clear();
   fcsd_dpa.clear();
   ocmn_dpa.clear();
   ocsd_dpa.clear();

   sid_inc_filt.clear();
   sid_exc_filt.clear();
   obs_qty_inc_filt.clear();
   obs_qty_exc_filt.clear();

   mpr_column.clear();
   mpr_thresh.clear();

   fcst_ut = (unixtime) 0;
   beg_ut  = (unixtime) 0;
   end_ut  = (unixtime) 0;

   msg_typ_sfc.clear();
   msg_typ_lnd.clear();
   msg_typ_wtr.clear();

   sfc_info.clear();

   n_msg_typ = 0;
   n_mask    = 0;
   n_interp  = 0;
   n_vx      = 0;

   pb_ptr.clear();

   n_try    = 0;
   rej_sid  = 0;
   rej_var  = 0;
   rej_vld  = 0;
   rej_obs  = 0;
   rej_grd  = 0;
   rej_lvl  = 0;
   rej_topo = 0;
   rej_qty  = 0;

   rej_typ.clear();
   rej_mask.clear();
   rej_fcst.clear();
   rej_cmn.clear();
   rej_csd.clear();
   rej_mpr.clear();
   rej_dup.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::assign(const VxPairBase &vx_pb) {

   clear();

   set_fcst_info(vx_pb.fcst_info);
   set_obs_info(vx_pb.obs_info);

   set_fcst_climo_info(vx_pb.fclm_info);
   set_obs_climo_info(vx_pb.oclm_info);

   desc = vx_pb.desc;

   interp_thresh = vx_pb.interp_thresh;

   fcst_dpa = vx_pb.fcst_dpa;
   fcmn_dpa = vx_pb.fcmn_dpa;
   fcsd_dpa = vx_pb.fcsd_dpa;
   ocmn_dpa = vx_pb.ocmn_dpa;
   ocsd_dpa = vx_pb.ocsd_dpa;

   fcst_ut  = vx_pb.fcst_ut;
   beg_ut   = vx_pb.beg_ut;
   end_ut   = vx_pb.end_ut;

   sid_inc_filt = vx_pb.sid_inc_filt;
   sid_exc_filt = vx_pb.sid_exc_filt;
   obs_qty_inc_filt = vx_pb.obs_qty_inc_filt;
   obs_qty_exc_filt = vx_pb.obs_qty_exc_filt;

   mpr_column = vx_pb.mpr_column;
   mpr_thresh = vx_pb.mpr_thresh;

   msg_typ_sfc = vx_pb.msg_typ_sfc;
   msg_typ_lnd = vx_pb.msg_typ_lnd;
   msg_typ_wtr = vx_pb.msg_typ_wtr;

   sfc_info = vx_pb.sfc_info;

   set_size(vx_pb.n_msg_typ, vx_pb.n_mask, vx_pb.n_interp);

   pb_ptr = vx_pb.pb_ptr;

   n_try    = vx_pb.n_try;
   rej_typ  = vx_pb.rej_typ;
   rej_mask = vx_pb.rej_mask;
   rej_fcst = vx_pb.rej_fcst;
   rej_cmn  = vx_pb.rej_cmn;
   rej_csd  = vx_pb.rej_csd;
   rej_mpr  = vx_pb.rej_mpr;
   rej_dup  = vx_pb.rej_dup;
   rej_typ  = vx_pb.rej_typ;
   rej_mask = vx_pb.rej_mask;
   rej_fcst = vx_pb.rej_fcst;
   rej_cmn  = vx_pb.rej_cmn;
   rej_csd  = vx_pb.rej_csd;
   rej_mpr  = vx_pb.rej_mpr;
   rej_dup  = vx_pb.rej_dup;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::copy_var_info(const VarInfo *info, VarInfo *&copy) {
   VarInfoFactory f;

   // Deallocate, if necessary
   if(copy) { delete copy; copy = (VarInfo *) nullptr; }

   // Perform a deep copy
   copy = f.new_var_info(info->file_type());
   *copy = *info;

   return;
}

////////////////////////////////////////////////////////////////////////

int VxPairBase::three_to_one(int i_msg_typ, int i_mask, int i_interp) const {

   int n = (i_interp * n_mask + i_mask)*n_msg_typ + i_msg_typ;

   if(n < 0 || n >= n_vx) {
      mlog << Error << "\nVxPairBase::three_to_one() -> "
           << "range check error for n (" << n << " < 0 or n >= " << n_vx
           << ") for i_msg_typ (" << i_msg_typ << "), i_mask ("
           << i_mask << "), i_interp (" << i_interp << "), and n_msg_typ ("
           << n_msg_typ << "), n_mask (" << n_mask << "), n_interp ("
           << n_interp << ")!\n\n";
      exit(1);
   }

   return n;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_fcst_info(const VarInfo *info) {

   copy_var_info(info, fcst_info);

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_obs_info(const VarInfo *info) {

   copy_var_info(info, obs_info);

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_fcst_climo_info(const VarInfo *info) {

   copy_var_info(info, fclm_info);

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_obs_climo_info(const VarInfo *info) {

   copy_var_info(info, oclm_info);

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_desc(const char *s) {

   desc = s;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_interp_thresh(double t) {

   interp_thresh = t;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_fcst_dpa(const DataPlaneArray &dpa) {

   fcst_dpa = dpa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_fcst_climo_mn_dpa(const DataPlaneArray &dpa) {

   fcmn_dpa = dpa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_fcst_climo_sd_dpa(const DataPlaneArray &dpa) {

   fcsd_dpa = dpa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_obs_climo_mn_dpa(const DataPlaneArray &dpa) {

   ocmn_dpa = dpa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_obs_climo_sd_dpa(const DataPlaneArray &dpa) {

   ocsd_dpa = dpa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_fcst_ut(const unixtime ut) {

   fcst_ut = ut;

   // Set for all PairBase instances, used for duplicate logic
   for(auto &x : pb_ptr) x->set_fcst_ut(ut);

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_beg_ut(const unixtime ut) {

   beg_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_end_ut(const unixtime ut) {

   end_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_sid_inc_filt(const StringArray &sa) {

   sid_inc_filt = sa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_sid_exc_filt(const StringArray &sa) {

   sid_exc_filt = sa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_obs_qty_inc_filt(const StringArray &sa) {

   obs_qty_inc_filt = sa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_obs_qty_exc_filt(const StringArray &sa) {

   obs_qty_exc_filt = sa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_size(int types, int masks, int interps) {

   // Store the dimensions for the PairBase array
   n_msg_typ = types;
   n_mask    = masks;
   n_interp  = interps;
   n_vx      = types * masks * interps;

   // Resize the PairBase pointer vector
   pb_ptr.resize(n_vx);

   // Initialize 3-D rejection count vectors
   vector<int> rej_counts(n_vx, 0);
   rej_typ  = rej_counts;
   rej_mask = rej_counts;
   rej_fcst = rej_counts;
   rej_cmn  = rej_counts;
   rej_csd  = rej_counts;
   rej_mpr  = rej_counts;
   rej_dup  = rej_counts;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_msg_typ(int i_msg_typ, const char *name) {

   for(int i_mask=0; i_mask<n_mask; i_mask++) {
      for(int i_interp=0; i_interp<n_interp; i_interp++) {
         int n = three_to_one(i_msg_typ, i_mask, i_interp);
         pb_ptr[n]->set_msg_typ(name);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_msg_typ_vals(int i_msg_typ, const StringArray &sa) {

   for(int i_mask=0; i_mask<n_mask; i_mask++) {
      for(int i_interp=0; i_interp<n_interp; i_interp++) {
         int n = three_to_one(i_msg_typ, i_mask, i_interp);
         pb_ptr[n]->set_msg_typ_vals(sa);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_mask_area(int i_mask, const char *name,
                               MaskPlane *mp_ptr) {

   for(int i_msg_typ=0; i_msg_typ<n_msg_typ; i_msg_typ++) {
      for(int i_interp=0; i_interp<n_interp; i_interp++) {
         int n = three_to_one(i_msg_typ, i_mask, i_interp);
         pb_ptr[n]->set_mask_name(name);
         pb_ptr[n]->set_mask_area_ptr(mp_ptr);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_mask_sid(int i_mask, const char *name,
                              MaskSID *ms_ptr) {

   if(!ms_ptr) return;

   for(int i_msg_typ=0; i_msg_typ<n_msg_typ; i_msg_typ++) {
      for(int i_interp=0; i_interp<n_interp; i_interp++) {
         int n = three_to_one(i_msg_typ, i_mask, i_interp);
         pb_ptr[n]->set_mask_name(name);
         pb_ptr[n]->set_mask_sid_ptr(ms_ptr);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_mask_llpnt(int i_mask, const char *name,
                                MaskLatLon *llpnt_ptr) {

   for(int i_msg_typ=0; i_msg_typ<n_msg_typ; i_msg_typ++) {
      for(int i_interp=0; i_interp<n_interp; i_interp++) {
         int n = three_to_one(i_msg_typ, i_mask, i_interp);
         pb_ptr[n]->set_mask_name(name);
         pb_ptr[n]->set_mask_llpnt_ptr(llpnt_ptr);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_interp(int i_interp,
                            const char *interp_mthd_str, int width,
                            GridTemplateFactory::GridTemplates shape) {

   for(int i_msg_typ=0; i_msg_typ<n_msg_typ; i_msg_typ++) {
      for(int i_mask=0; i_mask<n_mask; i_mask++) {
         int n = three_to_one(i_msg_typ, i_mask, i_interp);
         pb_ptr[n]->set_interp_mthd(interp_mthd_str);
         pb_ptr[n]->set_interp_wdth(width);
         pb_ptr[n]->set_interp_shape(shape);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_interp(int i_interp,
                            InterpMthd mthd, int width,
                            GridTemplateFactory::GridTemplates shape) {

   for(int i_msg_typ=0; i_msg_typ<n_msg_typ; i_msg_typ++) {
      for(int i_mask=0; i_mask<n_mask; i_mask++) {
         int n = three_to_one(i_msg_typ, i_mask, i_interp);
         pb_ptr[n]->set_interp_mthd(mthd);
         pb_ptr[n]->set_interp_wdth(width);
         pb_ptr[n]->set_interp_shape(shape);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_mpr_thresh(const StringArray &sa, const ThreshArray &ta) {

   // Check for constant length
   if(sa.n() != ta.n()) {
      mlog << Error << "\nVxPairBase::set_mpr_thresh() -> "
           << "the \"" << conf_key_mpr_column << "\" ("
           << write_css(sa) << ") and \"" << conf_key_mpr_thresh
           << "\" (" << write_css(ta)
           << ") config file entries must have the same length!\n\n";
      exit(1);
   }

   mpr_column = sa;
   mpr_thresh = ta;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_climo_cdf_info_ptr(const ClimoCDFInfo *info) {

   for(auto &x : pb_ptr) x->set_climo_cdf_info_ptr(info);

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_msg_typ_sfc(const StringArray &sa) {

   msg_typ_sfc = sa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_msg_typ_lnd(const StringArray &sa) {

   msg_typ_lnd = sa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_msg_typ_wtr(const StringArray &sa) {

   msg_typ_wtr = sa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_sfc_info(const SurfaceInfo &si) {

   sfc_info = si;

   return;
}

////////////////////////////////////////////////////////////////////////

int VxPairBase::get_n_pair() const {

   if(n_vx == 0) {
      mlog << Warning << "\nVxPairBase::get_n_pair() -> "
           << "set_size() has not been called yet!\n\n";
   }

   int n = 0;

   for(auto &x : pb_ptr) n += x->n_obs;

   return n;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_duplicate_flag(DuplicateType duplicate_flag) {

   if(n_vx == 0) {
      mlog << Warning << "\nVxPairBase::set_duplicate_flag() -> "
           << "set_size() has not been called yet!\n\n";
   }

   for(auto &x : pb_ptr) x->set_check_unique(duplicate_flag == DuplicateType::Unique);

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_obs_summary(ObsSummary s) {

   if(n_vx == 0) {
      mlog << Warning << "\nVxPairBase::set_obs_summary() -> "
           << "set_size() has not been called yet!\n\n";
   }

   for(auto &x : pb_ptr) x->set_obs_summary(s);

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_obs_perc_value(int percentile) {

   if(n_vx == 0) {
      mlog << Warning << "\nVxPairBase::set_obs_perc_value() -> "
           << "set_size() has not been called yet!\n\n";
   }

   for(auto &x : pb_ptr) x->set_obs_perc_value(percentile);

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::print_obs_summary() const {

   if(n_vx == 0) {
      mlog << Warning << "\nVxPairBase::print_obs_summary() -> "
           << "set_size() has not been called yet!\n\n";
   }

   for(auto &x : pb_ptr) x->print_obs_summary();

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::calc_obs_summary() {

   if(n_vx == 0) {
      mlog << Warning << "\nVxPairBase::calc_obs_summary() -> "
           << "set_size() has not been called yet!\n\n";
   }

   for(auto &x : pb_ptr) x->calc_obs_summary();

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::set_point_weight(const PointWeightType wgt_flag) {

   if(n_vx == 0) {
      mlog << Warning << "\nVxPairBase::set_point_weight() -> "
           << "set_size() has not been called yet!\n\n";
   }

   for(auto &x : pb_ptr) x->set_point_weight(wgt_flag);

   return;
}

////////////////////////////////////////////////////////////////////////

bool VxPairBase::is_keeper_sid(
        const char *pnt_obs_str, const char *hdr_sid_str) {
   bool keep = true;

   // Check the station ID inclusion and exclusion lists
   if((sid_inc_filt.n() && !sid_inc_filt.has(hdr_sid_str)) ||
      (sid_exc_filt.n() &&  sid_exc_filt.has(hdr_sid_str))) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str() << ", skipping observation "
              << "station id:\n"
              << pnt_obs_str << "\n";
      }

      rej_sid++;
      keep = false;
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////

bool VxPairBase::is_keeper_var(
        const char *pnt_obs_str, const char *var_name, int grib_code) {
   bool keep = true;

   const auto obs_info_grib = (VarInfoGrib *) obs_info;

   // Check for matching variable name or GRIB code
   if((var_name != nullptr) && (m_strlen(var_name) > 0)) {

      if(var_name != obs_info->name()) {

         if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
            mlog << Debug(REJECT_DEBUG_LEVEL)
                 << "For " << fcst_info->magic_str() << " versus "
                 << obs_info->magic_str() << ", skipping observation "
                 << "variable name:\n"
                 << pnt_obs_str << "\n";
         }

         rej_var++;
         keep = false;
      }
   }
   else if(obs_info_grib && obs_info_grib->code() != nint(grib_code)) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str() << ", skipping observation "
              << "GRIB code:\n"
              << pnt_obs_str << "\n";
      }

      rej_var++;
      keep = false;
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////

bool VxPairBase::is_keeper_qty(
        const char *pnt_obs_str, const char *obs_qty) {
   bool keep = true;

   // Check the observation quality include and exclude options
   if((obs_qty_inc_filt.n() > 0 && !obs_qty_inc_filt.has(obs_qty)) ||
      (obs_qty_exc_filt.n() > 0 &&  obs_qty_exc_filt.has(obs_qty))) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str() << ", skipping observation "
              << "quality control string:\n"
              << pnt_obs_str << "\n";
      }

      rej_qty++;
      keep = false;
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////

bool VxPairBase::is_keeper_vld(
        const char *pnt_obs_str, unixtime hdr_ut) {
   bool keep = true;

   // Check the observation valid time
   if(hdr_ut < beg_ut || hdr_ut > end_ut) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str() << ", skipping observation "
              << "valid time:\n"
              << pnt_obs_str << "\n";
      }

      rej_vld++;
      keep = false;
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////

bool VxPairBase::is_keeper_obs(
        const char *pnt_obs_str, double &obs_v) {
   bool keep = true;

   // Apply observation processing logic
   obs_v = pb_ptr[0]->process_obs(obs_info, obs_v);

   // Check whether the observation value contains valid data
   if(is_bad_data(obs_v)) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str() << ", skipping observation "
              << "with bad data value:\n"
              << pnt_obs_str << "\n";
      }

      rej_obs++;
      keep = false;
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////

bool VxPairBase::is_keeper_grd(
        const char *pnt_obs_str, const Grid &gr,
        double hdr_lat, double hdr_lon,
        double &obs_x, double &obs_y) {
   bool keep = true;

   // Convert the lat/lon value to x/y
   gr.latlon_to_xy(hdr_lat, -1.0*hdr_lon, obs_x, obs_y);
   int x = nint(obs_x);
   int y = nint(obs_y);

   // Check if the observation's lat/lon is on the grid
   if(((x < 0 || x >= gr.nx()) && !gr.wrap_lon()) ||
        y < 0 || y >= gr.ny()) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str() << ", skipping observation "
              << "off the grid where (x, y) = (" << x << ", " << y
              << ") and grid (nx, ny) = (" << gr.nx() << ", " << gr.ny() << "):\n"
              << pnt_obs_str << "\n";
      }

      rej_grd++;
      keep = false;
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////

bool VxPairBase::is_keeper_topo(
        const char *pnt_obs_str, const Grid &gr,
        double obs_x, double obs_y,
        const char *hdr_typ_str, double hdr_elv) {
   bool keep = true;

   // Check for a large topography difference
   if(sfc_info.topo_ptr && msg_typ_sfc.reg_exp_match(hdr_typ_str)) {

      // Interpolate model topography to observation location
      double topo = compute_horz_interp(
                       *sfc_info.topo_ptr, obs_x, obs_y, hdr_elv,
                       InterpMthd::Bilin, 2,
                       GridTemplateFactory::GridTemplates::Square,
                       gr.wrap_lon(), 1.0);

      // Skip bad topography values
      if(is_bad_data(hdr_elv) || is_bad_data(topo)) {

         if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
            mlog << Debug(REJECT_DEBUG_LEVEL)
                 << "For " << fcst_info->magic_str() << " versus "
                 << obs_info->magic_str() << ", skipping observation "
                 << "due to bad topography values where observation elevation = "
                 << hdr_elv << " and model topography = " << topo << ":\n"
                 << pnt_obs_str << "\n";
         }

         rej_topo++;
         keep = false;
      }

      // Check the topography difference threshold
      else if(!sfc_info.topo_use_obs_thresh.check(topo - hdr_elv)) {

         if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
            mlog << Debug(REJECT_DEBUG_LEVEL)
                 << "For " << fcst_info->magic_str() << " versus "
                 << obs_info->magic_str() << ", skipping observation "
                 << "due to topography difference where observation elevation ("
                 << hdr_elv << ") minus model topography (" << topo << ") = "
                 << topo - hdr_elv << " is not "
                 << sfc_info.topo_use_obs_thresh.get_str() << ":\n"
                 << pnt_obs_str << "\n";
         }

         rej_topo++;
         keep = false;
      }
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////

bool VxPairBase::is_keeper_lvl(
        const char *pnt_obs_str, const char *hdr_typ_str,
        double obs_lvl, double obs_hgt) {
   bool keep = true;

   // For pressure levels, check if the observation pressure level
   // falls in the requested range.
   if(obs_info->level().type() == LevelType_Pres) {

      if(obs_lvl < obs_info->level().lower() ||
         obs_lvl > obs_info->level().upper()) {

         if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
            mlog << Debug(REJECT_DEBUG_LEVEL)
                 << "For " << fcst_info->magic_str() << " versus "
                 << obs_info->magic_str() << ", skipping observation "
                 << "pressure level value:\n"
                 << pnt_obs_str << "\n";
         }

         rej_lvl++;
         keep = false;
      }
   }
   // For accumulations, check if the observation accumulation interval
   // matches the requested interval.
   else if(obs_info->level().type() == LevelType_Accum) {

      if(obs_lvl < obs_info->level().lower() ||
         obs_lvl > obs_info->level().upper()) {

         if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
            mlog << Debug(REJECT_DEBUG_LEVEL)
                 << "For " << fcst_info->magic_str() << " versus "
                 << obs_info->magic_str() << ", skipping observation "
                 << "accumulation interval:\n"
                 << pnt_obs_str << "\n";
         }

         rej_lvl++;
         keep = false;
      }
   }
   // For all other level types (VertLevel, RecNumber, NoLevel),
   // check for a surface message type or if the observation height
   // falls within the requested range.
   else {

      if(!msg_typ_sfc.reg_exp_match(hdr_typ_str) &&
         (obs_hgt < obs_info->level().lower() ||
          obs_hgt > obs_info->level().upper())) {

         if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
            mlog << Debug(REJECT_DEBUG_LEVEL)
                 << "For " << fcst_info->magic_str() << " versus "
                 << obs_info->magic_str() << ", skipping observation "
                 << "level value:\n"
                 << pnt_obs_str << "\n";
         }

         rej_lvl++;
         keep = false;
      }
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////

bool VxPairBase::is_keeper_typ(
        const char *pnt_obs_str, int i_msg_typ,
        const char *hdr_typ_str) {
   bool keep = true;

   int n = three_to_one(i_msg_typ, 0, 0);

   // Check for a matching message type
   if(!pb_ptr[n]->msg_typ_vals.has(hdr_typ_str)) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str() << ", skipping observation "
              << "message type:\n"
              << pnt_obs_str << "\n";
      }

      inc_count(rej_typ, i_msg_typ);
      keep = false;
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////

bool VxPairBase::is_keeper_mask(
        const char *pnt_obs_str, int i_msg_typ, int i_mask, int x, int y,
        const char *hdr_sid_str, double hdr_lat, double hdr_lon) {
   bool keep = true;

   int n = three_to_one(i_msg_typ, i_mask, 0);

   // Check for the obs falling within the masking region
   if( pb_ptr[n]->mask_area_ptr != nullptr &&
      !pb_ptr[n]->mask_area_ptr->s_is_on(x, y)) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str() << ", skipping observation "
              << "based on spatial masking region:\n"
              << pnt_obs_str << "\n";
         }

      inc_count(rej_mask, i_msg_typ, i_mask);
      keep = false;
   }
   // Otherwise, check for the masking SID list
   else if( pb_ptr[n]->mask_sid_ptr != nullptr &&
           !pb_ptr[n]->mask_sid_ptr->has(hdr_sid_str)) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str() << ", skipping observation "
              << "based on masking station id list:\n"
              << pnt_obs_str << "\n";
      }

      inc_count(rej_mask, i_msg_typ, i_mask);
      keep = false;
   }
   // Otherwise, check observation lat/lon thresholds
   else if(  pb_ptr[n]->mask_llpnt_ptr != nullptr &&
           (!pb_ptr[n]->mask_llpnt_ptr->lat_thresh.check(hdr_lat) ||
            !pb_ptr[n]->mask_llpnt_ptr->lon_thresh.check(hdr_lon))) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str() << ", skipping observation "
              << "based on latitude/longitude thesholds:\n"
              << pnt_obs_str << "\n";
      }

      inc_count(rej_mask, i_msg_typ, i_mask);
      keep = false;
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////

bool VxPairBase::is_keeper_climo(
        const char *pnt_obs_str,
        int i_msg_typ, int i_mask, int i_interp,
        const Grid &gr, double obs_x, double obs_y,
        double obs_v, double obs_lvl, double obs_hgt,
        ClimoPntInfo &cpi) {
   bool keep = true;

   int n = three_to_one(i_msg_typ, i_mask, i_interp);

   bool spfh_flag = fcst_info->is_specific_humidity() &&
                    obs_info->is_specific_humidity();

   // Compute the interpolated forecast value using the
   // observation pressure level or height
   double to_lvl = (fcst_info->level().type() == LevelType_Pres ?
                    obs_lvl : obs_hgt);
   int lvl_blw, lvl_abv;

   // Initialize
   cpi.clear();

   // Forecast climatology mean
   if(keep && fcmn_dpa.n_planes() > 0) {

      find_vert_lvl(fcmn_dpa, to_lvl, lvl_blw, lvl_abv);

      cpi.fcmn = compute_interp(fcmn_dpa, obs_x, obs_y, obs_v, nullptr,
                    pb_ptr[n]->interp_mthd, pb_ptr[n]->interp_wdth,
                    pb_ptr[n]->interp_shape, gr.wrap_lon(),
                    interp_thresh, spfh_flag,
                    fcst_info->level().type(),
                    to_lvl, lvl_blw, lvl_abv);

      // Check for bad data
      if(is_bad_data(cpi.fcmn)) {

         if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
            mlog << Debug(REJECT_DEBUG_LEVEL)
                 << "For " << fcst_info->magic_str() << " versus "
                 << obs_info->magic_str() << ", skipping observation "
                 << "based on bad forecast climatological mean value:\n"
                 << pnt_obs_str << "\n";
         }

         inc_count(rej_cmn, i_msg_typ, i_mask, i_interp);
         keep = false;
      }
   }

   // Observation climatology mean
   if(keep && ocmn_dpa.n_planes() > 0) {

      find_vert_lvl(ocmn_dpa, to_lvl, lvl_blw, lvl_abv);

      cpi.ocmn = compute_interp(ocmn_dpa, obs_x, obs_y, obs_v, nullptr,
                    pb_ptr[n]->interp_mthd, pb_ptr[n]->interp_wdth,
                    pb_ptr[n]->interp_shape, gr.wrap_lon(),
                    interp_thresh, spfh_flag,
                    fcst_info->level().type(),
                    to_lvl, lvl_blw, lvl_abv);

      // Check for bad data
      if(is_bad_data(cpi.ocmn)) {

         if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
            mlog << Debug(REJECT_DEBUG_LEVEL)
                 << "For " << fcst_info->magic_str() << " versus "
                 << obs_info->magic_str() << ", skipping observation "
                 << "based on bad observation climatological mean value:\n"
                 << pnt_obs_str << "\n";
         }

         inc_count(rej_cmn, i_msg_typ, i_mask, i_interp);
         keep = false;
      }
   }

   // Check for valid interpolation options
   if((fcsd_dpa.n_planes() > 0                 ||
       ocsd_dpa.n_planes() > 0)                &&
      (pb_ptr[n]->interp_mthd == InterpMthd::Min    ||
       pb_ptr[n]->interp_mthd == InterpMthd::Max    ||
       pb_ptr[n]->interp_mthd == InterpMthd::Median ||
       pb_ptr[n]->interp_mthd == InterpMthd::Best)) {
      mlog << Warning << "\nVxPairBase::add_point_obs() -> "
           << "applying the " << interpmthd_to_string(pb_ptr[n]->interp_mthd)
           << " interpolation method to climatological spread "
           << "may cause unexpected results.\n\n";
   }

   // Forecast climatology spread
   if(keep && fcsd_dpa.n_planes() > 0) {

      find_vert_lvl(fcsd_dpa, to_lvl, lvl_blw, lvl_abv);

      cpi.fcsd = compute_interp(fcsd_dpa, obs_x, obs_y, obs_v, nullptr,
                    pb_ptr[n]->interp_mthd, pb_ptr[n]->interp_wdth,
                    pb_ptr[n]->interp_shape, gr.wrap_lon(),
                    interp_thresh, spfh_flag,
                    fcst_info->level().type(),
                    to_lvl, lvl_blw, lvl_abv);

      // Check for bad data
      if(is_bad_data(cpi.fcsd)) {

         if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
            mlog << Debug(REJECT_DEBUG_LEVEL)
                 << "For " << fcst_info->magic_str() << " versus "
                 << obs_info->magic_str() << ", skipping observation "
                 << "based on bad forecast climatological spread value:\n"
                 << pnt_obs_str << "\n";
         }

         inc_count(rej_csd, i_msg_typ, i_mask, i_interp);
         keep = false;
      }
   }

   // Observation climatology spread
   if(keep && ocsd_dpa.n_planes() > 0) {

      find_vert_lvl(ocsd_dpa, to_lvl, lvl_blw, lvl_abv);

      cpi.ocsd = compute_interp(ocsd_dpa, obs_x, obs_y, obs_v, nullptr,
                    pb_ptr[n]->interp_mthd, pb_ptr[n]->interp_wdth,
                    pb_ptr[n]->interp_shape, gr.wrap_lon(),
                    interp_thresh, spfh_flag,
                    fcst_info->level().type(),
                    to_lvl, lvl_blw, lvl_abv);

      // Check for bad data
      if(is_bad_data(cpi.ocsd)) {

         if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
            mlog << Debug(REJECT_DEBUG_LEVEL)
                 << "For " << fcst_info->magic_str() << " versus "
                 << obs_info->magic_str() << ", skipping observation "
                 << "based on bad observation climatological spread value:\n"
                 << pnt_obs_str << "\n";
         }

         inc_count(rej_csd, i_msg_typ, i_mask, i_interp);
         keep = false;
      }
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////

bool VxPairBase::is_keeper_fcst(
        const char *pnt_obs_str,
        int i_msg_typ, int i_mask, int i_interp,
        const char *hdr_typ_str, const Grid &gr,
        double obs_x, double obs_y, double hdr_elv,
        double obs_v, double obs_lvl, double obs_hgt,
        const ClimoPntInfo &cpi, double &fcst_v) {
   bool keep = true;

   int n = three_to_one(i_msg_typ, i_mask, i_interp);

   // For surface verification, apply land/sea and topo masks
   if((sfc_info.land_ptr || sfc_info.topo_ptr) &&
      (msg_typ_sfc.reg_exp_match(hdr_typ_str))) {

      bool is_land = msg_typ_lnd.has(hdr_typ_str);

      // Check for a single forecast DataPlane
      if(fcst_dpa.n_planes() != 1) {
         mlog << Error << "\nVxPairBase::add_point_obs() -> "
              << "unexpected number of forecast levels ("
              << fcst_dpa.n_planes()
              << ") for surface verification! Set \"land_mask.flag\" and "
              << "\"topo_mask.flag\" to false to disable this check.\n\n";
         exit(1);
      }

      fcst_v = compute_sfc_interp(fcst_dpa[0], obs_x, obs_y, hdr_elv, obs_v,
                  pb_ptr[n]->interp_mthd, pb_ptr[n]->interp_wdth,
                  pb_ptr[n]->interp_shape, gr.wrap_lon(),
                  interp_thresh, sfc_info, is_land);
   }
   // Otherwise, compute interpolated value
   else {

      bool spfh_flag = fcst_info->is_specific_humidity() &&
                       obs_info->is_specific_humidity();

      // Compute the interpolated forecast value using the
      // observation pressure level or height
      double to_lvl = (fcst_info->level().type() == LevelType_Pres ?
                       obs_lvl : obs_hgt);
      int lvl_blw, lvl_abv;

      find_vert_lvl(fcst_dpa, to_lvl, lvl_blw, lvl_abv);

      fcst_v = compute_interp(fcst_dpa, obs_x, obs_y, obs_v, &cpi,
                  pb_ptr[n]->interp_mthd, pb_ptr[n]->interp_wdth,
                  pb_ptr[n]->interp_shape, gr.wrap_lon(),
                  interp_thresh, spfh_flag,
                  fcst_info->level().type(),
                  to_lvl, lvl_blw, lvl_abv);
   }

   // Check for bad data
   if(is_bad_data(fcst_v)) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str() << ", skipping observation "
              << "based on bad data in the "
              << interpmthd_to_string(pb_ptr[n]->interp_mthd) << "("
              << pb_ptr[n]->interp_wdth * pb_ptr[n]->interp_wdth
              << ") interpolated forecast value:\n"
              << pnt_obs_str << "\n";
      }

      inc_count(rej_fcst, i_msg_typ, i_mask, i_interp);
      keep = false;
   }

   return keep;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::inc_count(vector<int> &rej, int i_msg_typ) {

   for(int i_mask=0; i_mask<n_mask; i_mask++) {
      inc_count(rej, i_msg_typ, i_mask);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::inc_count(vector<int> &rej, int i_msg_typ, int i_mask) {

   for(int i_interp=0; i_interp<n_interp; i_interp++) {
      inc_count(rej, i_msg_typ, i_mask, i_interp);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairBase::inc_count(vector<int> &rej, int i_msg_typ, int i_mask, int i_interp) {

   rej[three_to_one(i_msg_typ, i_mask, i_interp)]++;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Begin miscellaneous utility functions
//
////////////////////////////////////////////////////////////////////////

void find_vert_lvl(const DataPlaneArray &dpa, const double obs_lvl,
                   int &i_blw, int &i_abv) {
   int i;
   double dist, dist_blw, dist_abv;

   // Initialize
   i_blw = i_abv = bad_data_int;

   // Check for no data
   if(dpa.n_planes() == 0) return;

   // Find the closest levels above and below the observation
   dist_blw = dist_abv = 1.0e30;
   for(i=0; i<dpa.n_planes(); i++) {

      dist = obs_lvl - dpa.lower(i);

      // Check for the closest level below.
      // Levels below contain higher values of pressure.
      if(dist <= 0 && fabs(dist) < dist_blw) {
         dist_blw = fabs(dist);
         i_blw = i;
      }

      // Check for the closest level above.
      // Levels above contain lower values of pressure.
      if(dist >= 0 && fabs(dist) < dist_abv) {
         dist_abv = fabs(dist);
         i_abv = i;
      }
   }

   // Check if the observation is above the forecast range
   if(is_eq(dist_blw, 1.0e30) && !is_eq(dist_abv, 1.0e30)) {

      // Set the index below to the index above and perform no vertical
      // interpolation
      i_blw = i_abv;
   }
   // Check if the observation is below the forecast range
   else if(!is_eq(dist_blw, 1.0e30) && is_eq(dist_abv, 1.0e30)) {

      // Set the index above to the index below and perform no vertical
      // interpolation
      i_abv = i_blw;
   }
   // Check if an error occurred
   else if(is_eq(dist_blw, 1.0e30) && is_eq(dist_abv, 1.0e30)) {

      mlog << Error << "\nfind_vert_lvl() -> "
           << "could not find a level above and/or below the "
           << "observation level of " << obs_lvl << ".\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

double compute_interp(const DataPlaneArray &dpa,
                      const double obs_x, const double obs_y,
                      const double obs_v, const ClimoPntInfo *cpi,
                      const InterpMthd method, const int width,
                      const GridTemplateFactory::GridTemplates shape,
                      const bool wrap_lon,
                      const double thresh,
                      const bool spfh_flag, const LevelType lvl_typ,
                      const double to_lvl, const int i_blw, const int i_abv,
                      const SingleThresh *cat_thresh) {
   double v, v_blw, v_abv, t;

   // Check for no data
   if(dpa.n_planes() == 0) return bad_data_double;

   v_blw = compute_horz_interp(dpa[i_blw], obs_x, obs_y, obs_v, cpi,
                               method, width, shape, wrap_lon,
                               thresh, cat_thresh);

   if(i_blw == i_abv) {
      v = v_blw;
   }
   else {
      v_abv = compute_horz_interp(dpa[i_abv], obs_x, obs_y, obs_v, cpi,
                                  method, width, shape, wrap_lon,
                                  thresh, cat_thresh);

      // Check for bad data prior to vertical interpolation
      if(is_bad_data(v_blw) || is_bad_data(v_abv)) {
         return bad_data_double;
      }

      // If verifying specific humidity, do vertical interpolation in
      // the natural log of q
      if(spfh_flag) {
         t = compute_vert_pinterp(log(v_blw), dpa.lower(i_blw),
                                  log(v_abv), dpa.lower(i_abv),
                                  to_lvl);
         v = exp(t);
      }
      // Vertically interpolate to the observation pressure level
      else if(lvl_typ == LevelType_Pres) {
         v = compute_vert_pinterp(v_blw, dpa.lower(i_blw),
                                  v_abv, dpa.lower(i_abv),
                                  to_lvl);
      }
      // Vertically interpolate to the observation height
      else {
         v = compute_vert_zinterp(v_blw, dpa.lower(i_blw),
                                  v_abv, dpa.lower(i_abv),
                                  to_lvl);
      }
   }

   return v;
}


////////////////////////////////////////////////////////////////////////

void get_interp_points(const DataPlaneArray &dpa,
                       const double obs_x, const double obs_y,
                       const InterpMthd method, const int width,
                       const GridTemplateFactory::GridTemplates shape,
                       const bool wrap_lon,
                       const double thresh, const bool spfh_flag,
                       const LevelType lvl_typ, const double to_lvl,
                       const int i_blw, const int i_abv,
                       NumArray &interp_pnts) {

   // Initialize
   interp_pnts.erase();

   // Check for no data
   if(dpa.n_planes() == 0) return;

   double v;
   int i, n_vld;
   NumArray pts_blw, pts_abv;
   GridTemplateFactory gtf;
   const GridTemplate* gt = gtf.buildGT(shape, width, wrap_lon);

   // Get interpolation points below the observation
   pts_blw = interp_points(dpa[i_blw], *gt, obs_x, obs_y);

   // For multiple levels, get interpolation points above
   if(i_blw != i_abv) {
      pts_abv = interp_points(dpa[i_abv], *gt, obs_x, obs_y);

      if(pts_abv.n() != pts_blw.n()) {
         mlog << Error << "\nget_interp_points() -> "
              << "the number of interpolation points above ("
              << pts_abv.n() << ") and below (" << pts_blw.n()
              << ") should match!\n\n";
         exit(1);
      }
   }

   // Interpolate each point vertically
   for(i=0, n_vld=0; i<pts_blw.n(); i++) {

      // Check for a single level
      if(i_blw == i_abv) {
         v = pts_blw[i];
      }
      // For specific humidity, interpolate in the natural log of q
      else if(spfh_flag) {
         v = exp(compute_vert_pinterp(log(pts_blw[i]), dpa.lower(i_blw),
                                      log(pts_abv[i]), dpa.lower(i_abv),
                                      to_lvl));
      }
      // Vertically interpolate to the observation pressure level
      else if(lvl_typ == LevelType_Pres) {
         v = compute_vert_pinterp(pts_blw[i], dpa.lower(i_blw),
                                  pts_abv[i], dpa.lower(i_abv), to_lvl);
      }
      // Vertically interpolate to the observation height
      else {
         v = compute_vert_zinterp(pts_blw[i], dpa.lower(i_blw),
                                  pts_abv[i], dpa.lower(i_abv), to_lvl);
      }

      // Keep track of valid data count
      if(!is_bad_data(v)) n_vld++;
 
      // Store the current value     
      interp_pnts.add(v);

   } // end for i

   // Check for enough valid data
   if(((double) n_vld)/((double) gt->size()) < thresh) {
      interp_pnts.erase();
   }

   if ( gt )  { delete gt;  gt = (const GridTemplate *) nullptr; }

   return;
}

////////////////////////////////////////////////////////////////////////

bool set_climo_flag(const NumArray &f_na, const NumArray &c_na) {

   // The climo values must have non-zero, consistent length and
   // cannot all be bad data
   if(c_na.n() != f_na.n() || c_na.n() < 1 || is_bad_data(c_na.max())) {
      return false;
   }

   return true;
}

////////////////////////////////////////////////////////////////////////

void derive_climo_vals(const ClimoCDFInfo *cdf_info_ptr,
                       double m, double s,
                       NumArray &climo_vals) {

   // Initialize
   climo_vals.erase();

   // Check for no work to do
   if(!cdf_info_ptr) return;

   // cdf_info_ptr->cdf_ta starts with >=0.0 and ends with >=1.0.
   // The number of bins is the number of thresholds minus 1.

   // Check for bad mean value
   if(is_bad_data(m) || cdf_info_ptr->cdf_ta.n() < 2) {
      return;
   }
   // Single climo bin
   else if(cdf_info_ptr->cdf_ta.n() == 2) {
      climo_vals.add(m);
   }
   // Check for bad standard deviation value
   else if(is_bad_data(s)) {
      return;
   }
   // Extract climo distribution values
   else {

      // Skip the first and last thresholds
      for(int i=1; i<cdf_info_ptr->cdf_ta.n()-1; i++) {
         climo_vals.add(
            normal_cdf_inv(cdf_info_ptr->cdf_ta[i].get_value(), m, s));
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

NumArray derive_climo_prob(const ClimoCDFInfo *cdf_info_ptr,
                           const NumArray &mn_na, const NumArray &sd_na,
                           const SingleThresh &othresh) {
   int i, n_mn, n_sd;
   NumArray climo_prob, climo_vals;
   double prob;

   // Number of valid climo mean and standard deviation
   n_mn = mn_na.n_valid();
   n_sd = sd_na.n_valid();

   // Check for constant climo probability
   prob = othresh.get_obs_climo_prob();
   if(!is_bad_data(prob)) {

      mlog << Debug(4)
           << "For threshold " << othresh.get_str()
           << ", using a constant climatological probability value of "
           << prob << ".\n";

      climo_prob.add_const(prob, n_mn);
   }
   // If both mean and standard deviation were provided, use them to
   // derive normal climatological probabilities for the current event
   // threshold
   else if(n_mn > 0 && n_sd > 0) {

      // Need cdf_info_ptr set to proceed
      if(!cdf_info_ptr) return climo_prob;

      // Derive climatological probabilities directly
      if(cdf_info_ptr->direct_prob) {

         mlog << Debug(4)
              << "Deriving climatological probabilities for threshold "
              << othresh.get_str() << " directly from the normal "
              << "climatological distribution.\n";

         // Directly derive the climatological probability
         for(i=0; i<mn_na.n(); i++) {

            // Check for bad data
            if(is_bad_data(mn_na[i]) || is_bad_data(sd_na[i])) {
               prob = bad_data_double;
            }
            else {

               // Probability is the area to the left of the threshold value
               prob = normal_cdf(othresh.get_value(), mn_na[i], sd_na[i]);

               // Adjust the probability based on the threshold type
               switch(othresh.get_type()) {
                  case thresh_lt:
                  case thresh_le:
                     break;
                  case thresh_gt:
                  case thresh_ge: prob = 1.0 - prob;
                     break;
                  case thresh_eq: prob = 0.0;
                     break;
                  case thresh_ne: prob = 1.0;
                     break;
                  default:
                     break;
               } // switch
               climo_prob.add(prob);
            }
         }
      }
      // Derive climatological probabilities by sampling from the distribution
      else {

         if(cdf_info_ptr->cdf_ta.n() == 0) {
            mlog << Error << "\nderive_climo_prob() -> "
                 << "No climatological probability thresholds defined!\n\n";
            exit(1);
         }

         // The first (>=0.0) and last (>=1.0) climo thresholds are omitted
         mlog << Debug(4)
              << "Deriving climatological probabilities for threshold "
              << othresh.get_str() << " by sampling "
              << cdf_info_ptr->cdf_ta.n()-2
              << " values from the normal climatological distribution.\n";

         // Compute the probability by sampling from the climo distribution
         // and deriving the event frequency
         for(i=0; i<mn_na.n(); i++) {
            derive_climo_vals(cdf_info_ptr, mn_na[i], sd_na[i], climo_vals);
            climo_prob.add(derive_prob(climo_vals, othresh));
         }
      }
   }
   // If only climatological mean was provided, it should already
   // contain probabilities. Range check the data to be sure.
   else {

      // Range check climatological probability mean values
      if(n_mn > 0 && n_sd == 0) {
         if(mn_na.min() < 0.0 || mn_na.max() > 1.0) {
            mlog << Error << "\nderive_climo_prob() -> "
                 << "The range of climatological probability values ["
                 << mn_na.min() << ", " << mn_na.max() << "] falls "
                 << "outside the expected range of [0, 1].\n"
                 << "When verifying a probabilistic forecast using "
                 << "climatology data, either supply a probabilistic "
                 << "climo_mean field or non-probabilistic\n"
                 << "climo_mean and climo_stdev fields from which a "
                 << "normal approximation of the climatological "
                 << "probabilities should be derived.\n\n";
            exit(1);
         }
      }

      climo_prob = mn_na;
   }

   return climo_prob;
}

////////////////////////////////////////////////////////////////////////

double derive_prob(const NumArray &na, const SingleThresh &st) {
   int i, n_vld, n_event;
   double prob;

   // Count up the number of events
   for(i=0,n_vld=0,n_event=0; i<na.n(); i++) {
      if(is_bad_data(na[i])) continue;
      n_vld++;
      if(st.check(na[i])) n_event++;
   }

   if(n_vld == 0) prob = bad_data_double;
   else           prob = (double) n_event / n_vld;

   return prob;
}

////////////////////////////////////////////////////////////////////////

// Write the point observation in the MET point format for logging
ConcatString point_obs_to_string(const float *hdr_arr, const char *hdr_typ_str,
                                 const char *hdr_sid_str, unixtime hdr_ut,
                                 const char *obs_qty, const float *obs_arr,
                                 const char *var_name) {
   ConcatString obs_cs, name;

   if((var_name != nullptr) && (0 < m_strlen(var_name))) name << var_name;
   else                                                  name << nint(obs_arr[1]);

   //
   // Write the 11-column MET point format:
   //   Message_Type Station_ID Valid_Time(YYYYMMDD_HHMMSS)
   //   Lat(Deg North) Lon(Deg East) Elevation(msl)
   //   Var_Name(or GRIB_Code) Level Height(msl or agl)
   //   QC_String Observation_Value
   //
   obs_cs << "   "
          << hdr_typ_str << " " << hdr_sid_str << " "
          << unix_to_yyyymmdd_hhmmss(hdr_ut) << " "
          << hdr_arr[0] << " " << -1.0*hdr_arr[1] << " "
          << hdr_arr[2] << " " << name << " "
          << obs_arr[2] << " " << obs_arr[3] << " "
          << obs_qty    << " " << obs_arr[4];

   return obs_cs;
}

////////////////////////////////////////////////////////////////////////
