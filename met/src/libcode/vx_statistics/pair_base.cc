// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <algorithm>

#include "pair_base.h"

#include "vx_util.h"
#include "vx_grid.h"
#include "vx_gsl_prob.h"
#include "vx_data2d.h"
#include "vx_data2d_grib.h"
#include "vx_math.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static bool sort_obs(ob_val_t a, ob_val_t b) { return a.val<b.val; }

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

   mask_name.clear();
   mask_area_ptr  = (MaskPlane *)   0;  // Not allocated
   mask_sid_ptr   = (StringArray *) 0;  // Not allocated
   mask_llpnt_ptr = (MaskLatLon *)  0;  // Not allocated

   msg_typ.clear();
   msg_typ_vals.clear();

   interp_mthd = InterpMthd_None;
   interp_shape = GridTemplateFactory::GridTemplate_None;

   sid_sa.clear();
   lat_na.clear();
   lon_na.clear();
   x_na.clear();
   y_na.clear();
   wgt_na.clear();
   vld_ta.clear();
   lvl_na.clear();
   elv_na.clear();
   o_na.clear();
   o_qc_sa.clear();
   cmn_na.clear();
   csd_na.clear();
   cdf_na.clear();

   n_obs = 0;

   fcst_ut = 0;

   obs_summary = ObsSummary_None;
   obs_perc_value = bad_data_int;
   check_unique = false;

   map_key.clear();
   map_val.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::extend(int n) {

   lat_na.extend(n);
   lon_na.extend(n);
   x_na.extend(n);
   y_na.extend(n);
   wgt_na.extend(n);
   vld_ta.extend(n);
   lvl_na.extend(n);
   elv_na.extend(n);
   o_na.extend(n);
   cmn_na.extend(n);
   csd_na.extend(n);
   cdf_na.extend(n);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_mask_name(const char *c) {

   mask_name = c;

  return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_mask_area_ptr(MaskPlane *mp_ptr) {

   mask_area_ptr = mp_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_mask_sid_ptr(StringArray *sid_ptr) {

   mask_sid_ptr = sid_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_mask_llpnt_ptr(MaskLatLon *llpnt_ptr) {

   mask_llpnt_ptr = llpnt_ptr;

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

   return(status);
}

////////////////////////////////////////////////////////////////////////

void PairBase::add_climo(double obs, double cmn, double csd) {

   // Compute and store the climatology information
   cmn_na.add(cmn);
   csd_na.add(csd);
   cdf_na.add(normal_cdf(obs, cmn, csd));

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_climo(int i_obs, double obs, double cmn, double csd) {

   // Compute and store the climatology information
   cmn_na.set(i_obs, cmn);
   csd_na.set(i_obs, csd);
   cdf_na.set(i_obs, normal_cdf(obs, cmn, csd));

   return;
}

////////////////////////////////////////////////////////////////////////

bool PairBase::add_obs(const char *sid,
                       double lat, double lon, double x, double y,
                       unixtime ut, double lvl, double elv,
                       double o, const char *qc,
                       double cmn, double csd, double wgt) {

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
           if( (*o_it).ut == ut) {
              mlog << Debug(4)
                   << "Skipping duplicate observation for [lat:lon:level:elevation] = ["
                   << obs_key << "] valid at " << unix_to_yyyymmdd_hhmmss(ut)
                   << " with value = " << o << "\n";
              return false;
           }
        }
     }

     (*it).second.obs.push_back(ob_val);

   } else {
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
      val.cmn = cmn;
      val.csd = csd;

      val.obs.push_back(ob_val);
      map_key.add(obs_key.c_str());
      map_val.insert( pair<string,station_values_t>(obs_key, val) );
      ret = true;
   }

   if(obs_summary == ObsSummary_None) {
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
      add_climo(o, cmn, csd);

      // Increment the number of pairs
      n_obs += 1;
      ret = true;
   }

   return ret;
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
   double total = 0.0;
   double weight = 0.0;
   double total_weight = 0.0;
   ob_val_t out;
   station_values_t svt = map_val[obs_key];
   vector<ob_val_t>::iterator it = svt.obs.begin();
   out.qc = (*it).qc;
   for(; it != svt.obs.end(); it++) {
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

void PairBase::print_obs_summary(){

   if(obs_summary == ObsSummary_None ||
      4 > mlog.verbosity_level() ||
      ! map_val.size() ) return;

   //  iterate over ordered list map keys in the station id map
   for(int i=0; i<map_key.n_elements(); i++) {

      station_values_t svt = map_val[map_key[i]];

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

   //  iterate over the keys in the unique station id map
   for(int i=0; i<map_key.n_elements(); i++) {

      station_values_t svt = map_val[map_key[i]];

      //  parse the single key string
      char** mat = NULL;
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
         case ObsSummary_Nearest:
            ob = compute_nearest(msg_key);
            break;
         case ObsSummary_Min:
            ob = compute_min(msg_key);
            break;
         case ObsSummary_Max:
            ob = compute_max(msg_key);
            break;
         case ObsSummary_UW_Mean:
            ob = compute_uw_mean(msg_key);
            break;
         case ObsSummary_DW_Mean:
            ob = compute_dw_mean(msg_key);
            break;
         case ObsSummary_Median:
            ob = compute_percentile(msg_key, 50);
            break;
         case ObsSummary_Perc:
            ob = compute_percentile(msg_key, obs_perc_value);
            break;
         case ObsSummary_None:
         default:
            return;
      }

      // Store summarized value in the map
      svt.summary_val = ob.val;

      sid_sa.add (svt.sid.c_str());
      lat_na.add (svt.lat);
      lon_na.add (svt.lon);
      x_na.add   (svt.x);
      y_na.add   (svt.y);
      wgt_na.add (svt.wgt);
      vld_ta.add (ob.ut);
      lvl_na.add (svt.lvl);
      elv_na.add (svt.elv);
      o_na.add   (ob.val);
      o_qc_sa.add(ob.qc.c_str());
      add_climo(ob.val, svt.cmn, svt.csd);

      // Increment the number of pairs
      n_obs += 1;

   } // end for map

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::add_obs(double x, double y, double o,
                       double cmn, double csd, double wgt) {

   sid_sa.add(na_str);
   lat_na.add(bad_data_double);
   lon_na.add(bad_data_double);
   x_na.add(x);
   y_na.add(y);
   wgt_na.add(wgt);
   vld_ta.add(bad_data_int);
   lvl_na.add(bad_data_double);
   elv_na.add(bad_data_double);
   o_na.add(o);
   o_qc_sa.add(na_str);
   add_climo(o, cmn, csd);

   // Increment the number of observations
   n_obs += 1;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_obs(int i_obs, const char *sid,
                       double lat, double lon, double x, double y,
                       unixtime ut, double lvl, double elv,
                       double o, const char *qc,
                       double cmn, double csd, double wgt) {

   if(i_obs < 0 || i_obs >= n_obs) {
      mlog << Error << "\nPairBase::set_obs() -> "
           << "range check error: " << i_obs << " not in (0, "
           << n_obs << ").\n\n"
          ;
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
   set_climo(i_obs, o, cmn, csd);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_obs(int i_obs, double x, double y,
                       double o, double cmn, double csd, double wgt) {

   if(i_obs < 0 || i_obs >= n_obs) {
      mlog << Error << "\nPairBase::set_obs() -> "
           << "range check error: " << i_obs << " not in (0, "
           << n_obs << ").\n\n";
      exit(1);
   }

   sid_sa.set(i_obs, na_str);
   lat_na.set(i_obs, bad_data_double);
   lon_na.set(i_obs, bad_data_double);
   x_na.set(i_obs, x);
   y_na.set(i_obs, y);
   wgt_na.set(i_obs, wgt);
   vld_ta.set(i_obs, bad_data_int);
   lvl_na.set(i_obs, bad_data_double);
   elv_na.set(i_obs, bad_data_double);
   o_na.set(i_obs, o);
   o_qc_sa.set(i_obs, na_str);
   set_climo(i_obs, o, cmn, csd);

   return;
}

////////////////////////////////////////////////////////////////////////

double PairBase::process_obs(VarInfo *vinfo, double v) {

   if(!vinfo) return(v);

   double new_v = v;

   // Apply conversion logic.
   if(vinfo->ConvertFx.is_set()) {
      new_v = vinfo->ConvertFx(new_v);
   }

   // Apply censor logic.
   for(int i=0; i<vinfo->censor_thresh().n_elements(); i++) {

      // Break out after the first match.
      if(vinfo->censor_thresh()[i].check(new_v)) {
         new_v = vinfo->censor_val()[i];
         break;
      }
   }

   return(new_v);
}

////////////////////////////////////////////////////////////////////////
//
// Begin miscellanous utility functions
//
////////////////////////////////////////////////////////////////////////

void find_vert_lvl(const DataPlaneArray &dpa, const double obs_lvl,
                   int &i_blw, int &i_abv) {
   int i;
   double dist, dist_blw, dist_abv;

   // Check for no data
   if(dpa.n_planes() == 0) {
      i_blw = i_abv = bad_data_int;
      return;
   }

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
                      const double obs_x, const double obs_y, const double obs_v,
                      const InterpMthd method, const int width,
                      const GridTemplateFactory::GridTemplates shape,
                      const double thresh,
                      const bool spfh_flag, const LevelType lvl_typ,
                      const double to_lvl, const int i_blw, const int i_abv,
                      const SingleThresh *cat_thresh) {
   double v, v_blw, v_abv, t;

   // Check for no data
   if(dpa.n_planes() == 0) return(bad_data_double);

   v_blw = compute_horz_interp(dpa[i_blw], obs_x, obs_y, obs_v,
                               method, width, shape, thresh, cat_thresh);

   if(i_blw == i_abv) {
      v = v_blw;
   }
   else {
      v_abv = compute_horz_interp(dpa[i_abv], obs_x, obs_y, obs_v,
                                  method, width, shape, thresh, cat_thresh);

      // Check for bad data prior to vertical interpolation
      if(is_bad_data(v_blw) || is_bad_data(v_abv)) {
         return(bad_data_double);
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

   return(v);
}


////////////////////////////////////////////////////////////////////////

void get_interp_points(const DataPlaneArray &dpa,
                       const double obs_x, const double obs_y,
                       const InterpMthd method, const int width,
                       const GridTemplateFactory::GridTemplates shape,
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
   const GridTemplate* gt = gtf.buildGT(shape, width);

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

   if ( gt )  { delete gt;  gt = (const GridTemplate *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////
