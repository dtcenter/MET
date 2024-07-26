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

#include "pair_data_point.h"

#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_data2d_grib.h"
#include "vx_gsl_prob.h"
#include "vx_util.h"
#include "vx_grid.h"
#include "vx_math.h"
#include "vx_log.h"
#include "enum_as_int.hpp"

using namespace std;

////////////////////////////////////////////////////////////////////////
//
// Code for class PairDataPoint
//
////////////////////////////////////////////////////////////////////////

PairDataPoint::PairDataPoint() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PairDataPoint::~PairDataPoint() {
   clear();
}

////////////////////////////////////////////////////////////////////////

PairDataPoint::PairDataPoint(const PairDataPoint &pd) {

   init_from_scratch();

   assign(pd);
}

////////////////////////////////////////////////////////////////////////

PairDataPoint & PairDataPoint::operator=(const PairDataPoint &pd) {

   if(this == &pd) return *this;

   assign(pd);

   return *this;
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::init_from_scratch() {

   seeps_climo = nullptr;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::clear() {

   PairBase::clear();

   f_na.clear();
   for (int idx=0; idx<seeps_mpr.size(); idx++) {
      if (seeps_mpr[idx]) {
         delete seeps_mpr[idx];
         seeps_mpr[idx] = nullptr;
      }
   }
   seeps_mpr.clear();
   seeps_agg.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::erase() {

   PairBase::erase();

   f_na.erase();
   for (int idx=0; idx<seeps_mpr.size(); idx++) {
      if (seeps_mpr[idx]) {
         delete seeps_mpr[idx];
         seeps_mpr[idx] = nullptr;
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::extend(int n) {

   PairBase::extend(n);

   f_na.extend(n);
   for (int idx=seeps_mpr.size(); idx<n; idx++) {
      seeps_mpr.push_back(nullptr);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::assign(const PairDataPoint &pd) {
   int i;

   clear();

   // Allocate memory for output pairs
   extend(pd.n_obs);

   set_mask_name(pd.mask_name.c_str());
   set_mask_area_ptr(pd.mask_area_ptr);
   set_msg_typ(pd.msg_typ.c_str());
   set_msg_typ_vals(pd.msg_typ_vals);

   cdf_info_ptr = pd.cdf_info_ptr;

   set_interp_mthd(pd.interp_mthd);
   set_interp_wdth(pd.interp_wdth);
   set_interp_shape(pd.interp_shape);

   // Handle point data
   if(pd.is_point_vx()) {

      for(i=0; i<pd.n_obs; i++) {

         // Store climo data
         ClimoPntInfo cpi(pd.fcmn_na[i], pd.fcsd_na[i],
                          pd.ocmn_na[i], pd.ocsd_na[i]);

         if(add_point_pair(pd.sid_sa[i].c_str(), pd.lat_na[i], pd.lon_na[i],
               pd.x_na[i], pd.y_na[i], pd.vld_ta[i],
               pd.lvl_na[i], pd.elv_na[i],
               pd.f_na[i], pd.o_na[i], pd.o_qc_sa[i].c_str(),
               cpi, pd.wgt_na[i])) {
            if(i < pd.seeps_mpr.size()) set_seeps_score(seeps_mpr[i], i);
         }
      }
   }
   // Handle gridded data
   else {
      add_grid_pair(pd.f_na, pd.o_na, pd.fcmn_na, pd.fcsd_na,
         pd.ocmn_na, pd.ocsd_na, pd.wgt_na);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool PairDataPoint::add_point_pair(const char *sid, double lat, double lon,
                                   double x, double y, unixtime ut,
                                   double lvl, double elv,
                                   double f, double o, const char *qc,
                                   const ClimoPntInfo &cpi, double wgt) {

   if(!add_point_obs(sid, lat, lon, x, y, ut, lvl, elv, o, qc,
                     cpi, wgt)) return false;

   f_na.add(f);
   seeps_mpr.push_back(nullptr);

   return true;
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::load_seeps_climo(const ConcatString &seeps_climo_name) {
   if (nullptr == seeps_climo) seeps_climo = get_seeps_climo(seeps_climo_name);
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::set_seeps_thresh(const SingleThresh &p1_thresh) {
   if (nullptr != seeps_climo) seeps_climo->set_p1_thresh(p1_thresh);
   else mlog << Warning << "\nPairDataPoint::set_seeps_thresh() -> "
             << "ignored t1_threshold. Load SEEPS climo first\n\n";
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::set_seeps_score(SeepsScore *seeps, int index) {
   if (seeps) {
      int seeps_count = seeps_mpr.size();
      if(index < 0) index = seeps_count - 1;
      if(index < seeps_count) {
         if (seeps) {
            if (!seeps_mpr[index]) seeps_mpr[index] = new SeepsScore();
            *seeps_mpr[index] = *seeps;
         }
         else {
            if (seeps_mpr[index]) {
               delete seeps_mpr[index];
               seeps_mpr[index] = nullptr;
            }
         }
      }
      else mlog << Warning << "\nPairDataPoint::set_seeps_score("
                << index << ") is out of range " << seeps_count << "\n\n";
   }

}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::set_point_pair(int i_obs, const char *sid,
                                   double lat, double lon,
                                   double x, double y, unixtime ut,
                                   double lvl, double elv,
                                   double f, double o, const char *qc,
                                   const ClimoPntInfo &cpi,
                                   double wgt, SeepsScore *seeps) {

   if(i_obs < 0 || i_obs >= n_obs) {
      mlog << Error << "\nPairDataPoint::set_point_pair() -> "
           << "range check error: " << i_obs << " not in (0, "
           << n_obs << ").\n\n";
      exit(1);
   }

   set_point_obs(i_obs, sid, lat, lon, x, y, ut, lvl, elv,
                 o, qc, cpi, wgt);

   f_na.set(i_obs, f);
   *seeps_mpr[i_obs] = *seeps;

   return;
}

////////////////////////////////////////////////////////////////////////

bool PairDataPoint::add_grid_pair(double f, double o,
                                  const ClimoPntInfo &cpi,
                                  double wgt) {

   add_grid_obs(o, cpi, wgt);

   f_na.add(f);
   seeps_mpr.push_back(nullptr);

   return true;
}

////////////////////////////////////////////////////////////////////////

bool PairDataPoint::add_grid_pair(const NumArray &f_in,   const NumArray &o_in,
                                  const NumArray &fcmn_in, const NumArray &fcsd_in,
                                  const NumArray &ocmn_in, const NumArray &ocsd_in,
                                  const NumArray &wgt_in) {

   // Check for constant length
   if(o_in.n() != f_in.n()    ||
      o_in.n() != fcmn_in.n() ||
      o_in.n() != fcsd_in.n() ||
      o_in.n() != ocmn_in.n() ||
      o_in.n() != ocsd_in.n() ||
      o_in.n() != wgt_in.n()) {
      mlog << Error << "\nPairDataPoint::add_grid_pair() -> "
           << "arrays must all have the same length!\n\n";
      exit(1);
   }

   // Allocate enough memory
   extend(o_in.n());

   f_na.add(f_in);
   o_na.add(o_in);
   wgt_na.add(wgt_in);

   for(int i=0; i<o_in.n(); i++) {
      ClimoPntInfo cpi(fcmn_in[i], fcsd_in[i], ocmn_in[i], ocsd_in[i]);
      add_climo(o_in[i], cpi);
   }

   // Increment the number of pairs
   n_obs += o_in.n();

   return true;
}

////////////////////////////////////////////////////////////////////////

static int seeps_record_count = 0;
static int seeps_debug_level = 9;

SeepsScore *PairDataPoint::compute_seeps(const char *sid, double f,
                                         double o, unixtime ut) {
   SeepsScore *seeps = 0;
   int month, day, year, hour, minute, second;

   int sid_no = atoi(sid);
   if (sid_no && nullptr != seeps_climo) {
      unix_to_mdyhms(ut, month, day, year, hour, minute, second);
      seeps = seeps_climo->get_seeps_score(sid_no, f, o, month, hour);
      if (mlog.verbosity_level() >= seeps_debug_level
          && seeps && !is_eq(seeps->score, bad_data_double)
          && !is_eq(seeps->score, 0) && seeps_record_count < 10) {
         mlog << Debug(seeps_debug_level)
              << "PairDataPoint::compute_seeps() score = " << seeps->score << "\n";
         seeps_record_count++;
      }
   }
   return seeps;
}

////////////////////////////////////////////////////////////////////////

PairDataPoint PairDataPoint::subset_pairs_cnt_thresh(
                 const SingleThresh &ft, const SingleThresh &ot,
                 const SetLogic type) const {

   // Check for no work to be done
   if(ft.get_type() == thresh_na && ot.get_type() == thresh_na) {
      return *this;
   }

   int i;
   PairDataPoint out_pd;

   // Allocate memory for output pairs
   out_pd.extend(n_obs);
   out_pd.set_climo_cdf_info_ptr(cdf_info_ptr);

   bool fcmn_flag = set_climo_flag(f_na, fcmn_na);
   bool fcsd_flag = set_climo_flag(f_na, fcsd_na);
   bool ocmn_flag = set_climo_flag(f_na, ocmn_na);
   bool ocsd_flag = set_climo_flag(f_na, ocsd_na);
   bool wgt_flag  = set_climo_flag(f_na, wgt_na);

   // Loop over the pairs
   for(i=0; i<n_obs; i++) {

      // Check for bad data
      if(is_bad_data(f_na[i])                   ||
         is_bad_data(o_na[i])                   ||
         (fcmn_flag && is_bad_data(fcmn_na[i])) ||
         (fcsd_flag && is_bad_data(fcsd_na[i])) ||
         (ocmn_flag && is_bad_data(ocmn_na[i])) ||
         (ocsd_flag && is_bad_data(ocsd_na[i])) ||
         (wgt_flag  && is_bad_data(wgt_na[i]))) continue;

      // Store climo data
      ClimoPntInfo cpi(fcmn_na[i], fcsd_na[i], ocmn_na[i], ocsd_na[i]);

      // Keep pairs which meet the threshold criteria
      if(check_fo_thresh(f_na[i], o_na[i], cpi, ft, ot, type)) {

         // Handle point data
         if(is_point_vx()) {
            if(out_pd.add_point_pair(sid_sa[i].c_str(), lat_na[i],
                         lon_na[i], x_na[i], y_na[i],
                         vld_ta[i], lvl_na[i], elv_na[i],
                         f_na[i], o_na[i], o_qc_sa[i].c_str(),
                         cpi, wgt_na[i])) {
               out_pd.set_seeps_score(seeps_mpr[i], i);
            }
         }
         // Handle gridded data
         else {
            out_pd.add_grid_pair(f_na[i], o_na[i], cpi, wgt_na[i]);
         }
      }
   } // end for

   mlog << Debug(3)
        << "Using " << out_pd.n_obs << " of " << n_obs
        << " pairs for forecast filtering threshold " << ft.get_str()
        << ", observation filtering threshold " << ot.get_str()
        << ", and field logic " << setlogic_to_string(type) << ".\n";

   return out_pd;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class VxPairDataPoint
//
////////////////////////////////////////////////////////////////////////

VxPairDataPoint::VxPairDataPoint() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

VxPairDataPoint::~VxPairDataPoint() {
   clear();
}

////////////////////////////////////////////////////////////////////////

VxPairDataPoint::VxPairDataPoint(const VxPairDataPoint &vx_pd) {

   init_from_scratch();

   assign(vx_pd);
}

////////////////////////////////////////////////////////////////////////

VxPairDataPoint & VxPairDataPoint::operator=(const VxPairDataPoint &vx_pd) {

   if(this == &vx_pd) return *this;

   assign(vx_pd);

   return *this;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::init_from_scratch() {

   VxPairBase::init_from_scratch();

   fcst_info = (VarInfo *) nullptr;
   obs_info  = (VarInfo *) nullptr;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::clear() {

   VxPairBase::clear();

   if(fcst_info) { delete fcst_info; fcst_info = (VarInfo *) nullptr; }
   if(obs_info)  { delete obs_info;  obs_info  = (VarInfo *) nullptr; }

   pd.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::assign(const VxPairDataPoint &vx_pd) {

   clear();

   VxPairBase::assign(vx_pd);

   set_size(vx_pd.n_msg_typ, vx_pd.n_mask, vx_pd.n_interp);

   pd = vx_pd.pd;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_size(int types, int masks, int interps) {

   VxPairBase::set_size(types, masks, interps);

   // Resize the PairDataPoint vector
   pd.resize(n_vx);

   // Set PairBase pointers to the PairDataPoint objects
   for(int i=0; i<n_vx; i++) pb_ptr[i] = &pd[i];

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::add_point_obs(float *hdr_arr, const char *hdr_typ_str,
                                    const char *hdr_sid_str, unixtime hdr_ut,
                                    const char *obs_qty, float *obs_arr,
                                    Grid &gr, const char *var_name,
                                    const DataPlane *wgt_dp) {

   // Increment the number of tries count
   n_try++;

   // Point observation summary string for rejection log messages
   ConcatString pnt_obs_str;
   if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
      pnt_obs_str = point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                        hdr_ut, obs_qty, obs_arr, var_name);
   }

   // Check the station ID
   if(!is_keeper_sid(pnt_obs_str.c_str(), hdr_sid_str)) return;

   // Check observation variable
   if(!is_keeper_var(pnt_obs_str.c_str(), var_name, nint(obs_arr[1]))) return;

   // Check observation quality
   if(!is_keeper_qty(pnt_obs_str.c_str(), obs_qty)) return;

   // Check valid time
   if(!is_keeper_vld(pnt_obs_str.c_str(), hdr_ut)) return;

   // Check observation value
   double obs_v = obs_arr[4];
   if(!is_keeper_obs(pnt_obs_str.c_str(), obs_v)) return;

   // Check location
   double hdr_lat = hdr_arr[0];
   double hdr_lon = hdr_arr[1];
   double obs_x, obs_y;
   if(!is_keeper_grd(pnt_obs_str.c_str(), gr, hdr_lat, hdr_lon, obs_x, obs_y)) return;

   // Check topo
   double hdr_elv = hdr_arr[2];
   if(!is_keeper_topo(pnt_obs_str.c_str(), gr, obs_x, obs_y,
                      hdr_typ_str, hdr_elv)) return;

   // Check level
   double obs_lvl = obs_arr[2];
   double obs_hgt = obs_arr[3];
   if(!is_keeper_lvl(pnt_obs_str.c_str(), hdr_typ_str, obs_lvl, obs_hgt)) return;

   // Set flags
   bool spfh_flag   = fcst_info->is_specific_humidity() &&
                      obs_info->is_specific_humidity();
   bool precip_flag = fcst_info->is_precipitation() &&
                      obs_info->is_precipitation();
   int precip_interval = bad_data_int;
   if(precip_flag) {
      if(wgt_dp) precip_interval = wgt_dp->accum();
      else precip_interval = fcst_dpa[0].accum();
   }

   bool has_seeps = false;
   SeepsScore *seeps = nullptr;

   // When verifying a vertical level forecast against a surface message
   // type, set the observation level value to bad data so that it's not
   // used in the duplicate logic.
   if(obs_info->level().type() == LevelType_Vert &&
      msg_typ_sfc.reg_exp_match(hdr_typ_str)) {
      obs_lvl = bad_data_double;
   }

   // Loop through the message types
   for(int i_msg_typ=0; i_msg_typ<n_msg_typ; i_msg_typ++) {

      // Check message type
      if(!is_keeper_typ(pnt_obs_str.c_str(), i_msg_typ, hdr_typ_str)) continue;

      int x = nint(obs_x);
      int y = nint(obs_y);

      // Loop through the masking regions
      for(int i_mask=0; i_mask<n_mask; i_mask++) {

         // Check masking region
         if(!is_keeper_mask(pnt_obs_str.c_str(), i_msg_typ, i_mask, x, y,
                            hdr_sid_str, hdr_lat, hdr_lon)) continue;

         // Loop through the interpolation methods
         for(int i_interp=0; i_interp<n_interp; i_interp++) {

            // Check climatology values
            ClimoPntInfo cpi;
            if(!is_keeper_climo(pnt_obs_str.c_str(),
                                i_msg_typ, i_mask, i_interp,
                                gr, obs_x, obs_y, obs_v, obs_lvl, obs_hgt,
                                cpi)) continue;

            // Check forecast values
            double fcst_v;
            if(!is_keeper_fcst(pnt_obs_str.c_str(),
                               i_msg_typ, i_mask, i_interp,
                               hdr_typ_str, gr,
                               obs_x, obs_y, hdr_elv,
                               obs_v, obs_lvl, obs_hgt,
                               cpi, fcst_v)) continue;

            // Check matched pair filtering options
            ConcatString reason_cs;
            if(!check_mpr_thresh(fcst_v, obs_v, cpi,
                                 mpr_column, mpr_thresh, &reason_cs)) {

               if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
                  mlog << Debug(REJECT_DEBUG_LEVEL)
                       << "For " << fcst_info->magic_str() << " versus "
                       << obs_info->magic_str() << ", skipping observation"
                       << "due to matched pair filter since "
                       << reason_cs << ":\n"
                       << pnt_obs_str << "\n";
               }

               inc_count(rej_mpr, i_msg_typ, i_mask, i_interp);
               continue;
            }

            // Compute weight for current point
            double wgt_v = (wgt_dp == (DataPlane *) 0 ?
                            default_grid_weight :
                            wgt_dp->get(x, y));

            // Add the forecast, climatological, and observation data
            // Weight is from the nearest grid point
            int n = three_to_one(i_msg_typ, i_mask, i_interp);
            if(!pd[n].add_point_pair(hdr_sid_str,
                         hdr_lat, hdr_lon, obs_x, obs_y, hdr_ut, obs_lvl,
                         obs_hgt, fcst_v, obs_v, obs_qty, cpi, wgt_v)) {

               if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
                  mlog << Debug(REJECT_DEBUG_LEVEL)
                       << "For " << fcst_info->magic_str() << " versus "
                       << obs_info->magic_str()
                       << ", skipping observation since it is a duplicate:\n"
                       << pnt_obs_str << "\n";
               }

               inc_count(rej_dup, i_msg_typ, i_mask, i_interp);
               continue;
            }

            // Compute seeps
            if (precip_flag && precip_interval == 24*60*60) {  // 24 hour precip only
               seeps = pd[n].compute_seeps(hdr_sid_str, fcst_v, obs_v, hdr_ut);
            }
            else {
               seeps = nullptr;
            }
            pd[n].set_seeps_score(seeps);
            if (seeps) { delete seeps; seeps = nullptr; }

            if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
               mlog << Debug(REJECT_DEBUG_LEVEL)
                    << "For " << fcst_info->magic_str() << " versus "
                    << obs_info->magic_str() << ", for observation type "
                    << pd[n].msg_typ << ", over region "
                    << pd[n].mask_name << ", for interpolation method "
                    << interpmthd_to_string(pd[n].interp_mthd) << "("
                    << pd[n].interp_wdth * pd[n].interp_wdth
                    << "), using observation:\n"
                    << pnt_obs_str << "\n";
            }

         } // end for i_interp
      } // end for i_mask
   } // end for i_msg_typ

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::load_seeps_climo(const ConcatString &seeps_climo_name) {

   if(n_vx == 0) {
      mlog << Warning << "\nVxPairDataPoint::load_seeps_climo() -> "
           << "set_size() has not been called yet!\n\n";
   }

   for(vector<PairDataPoint>::iterator it = pd.begin();
       it != pd.end(); it++) {
      it->load_seeps_climo(seeps_climo_name);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_seeps_thresh(const SingleThresh &p1_thresh) {

   if(n_vx == 0) {
      mlog << Warning << "\nVxPairDataPoint::set_seeps_thresh() -> "
           << "set_size() has not been called yet!\n\n";
   }

   for(vector<PairDataPoint>::iterator it = pd.begin();
       it != pd.end(); it++) {
      it->set_seeps_thresh(p1_thresh);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Begin miscellaneous functions
//
////////////////////////////////////////////////////////////////////////

bool check_fo_thresh(double f, double o, const ClimoPntInfo &cpi,
                     const SingleThresh &ft, const SingleThresh &ot,
                     const SetLogic type) {
   bool status = true;
   bool fcheck = ft.check(f, &cpi);
   bool ocheck = ot.check(o, &cpi);
   SetLogic t  = type;

   // If either of the thresholds is NA, reset the logic to intersection
   // because an NA threshold is always true.
   if(ft.get_type() == thresh_na || ot.get_type() == thresh_na) {
      t = SetLogic::Intersection;
   }

   switch(t) {
      case SetLogic::Union:
         if(!fcheck && !ocheck) status = false;
         break;

      case SetLogic::Intersection:
         if(!fcheck || !ocheck) status = false;
         break;

      case SetLogic::SymDiff:
         if(fcheck == ocheck) status = false;
         break;

      default:
         mlog << Error << "\ncheck_fo_thresh() -> "
              << "Unexpected SetLogic value of " << enum_class_as_int(type) << ".\n\n";
         exit(1);
   }

   return status;
}

////////////////////////////////////////////////////////////////////////

bool check_mpr_thresh(double f, double o, const ClimoPntInfo &cpi,
                      const StringArray &col_sa, const ThreshArray &col_ta,
                      ConcatString *reason_ptr) {
   // Initialize
   if(reason_ptr) reason_ptr->erase();

   // Check arrays
   if(col_sa.n() == 0 || col_ta.n() == 0) return true;

   bool keep = true;
   bool absv = false;
   StringArray sa;
   ConcatString cs;
   double v, v_cur;
   int i, j;

   // Loop over all the column filter names
   for(i=0; i<col_sa.n(); i++) {

      // Check for absolute value
      if(strncasecmp(col_sa[i].c_str(), "ABS", 3) == 0) {
         absv = true;
         cs   = col_sa[i];
         sa   = cs.split("()");
         cs   = sa[1];
      }
      else {
         cs = col_sa[i];
      }

      // Split the input column name on hyphens for differences
      sa = cs.split("-");

      // Get the first value
      v = get_mpr_column_value(f, o, cpi, sa[0].c_str());

      // If multiple columns, compute the requested difference
      if(sa.n() > 1) {

         // Loop through the columns
         for(j=1; j<sa.n(); j++) {

            // Get the current column value
            v_cur = get_mpr_column_value(f, o, cpi, sa[j].c_str());

            // Compute the difference, checking for bad data
            if(is_bad_data(v) || is_bad_data(v_cur)) v  = bad_data_double;
            else                                     v -= v_cur;
         } // end for j
      }

      // Apply absolute value, if requested
      if(absv && !is_bad_data(v)) v = fabs(v);

      // Check the threshold
      if(!col_ta[i].check(v)) {
         if(reason_ptr) {
            (*reason_ptr) << cs_erase << col_sa[i] << " = " << v
                          << " is not " << col_ta[i].get_str();
         }
         keep = false;
         break;
      }
   } // end for i

   return keep;
}

////////////////////////////////////////////////////////////////////////
//
// Parse string to determine which value to use
//
////////////////////////////////////////////////////////////////////////

double get_mpr_column_value(double f, double o, const ClimoPntInfo &cpi,
                            const char *s) {
   double v;

/* #MET #2924 Replace this section
        if(strcasecmp(s, "FCST")             == 0) v = f;
   else if(strcasecmp(s, "OBS")              == 0) v = o;
   else if(strcasecmp(s, "FCST_CLIMO_MEAN")  == 0) v = cpi.fcmn;
   else if(strcasecmp(s, "FCST_CLIMO_STDEV") == 0) v = cpi.fcsd;
   else if(strcasecmp(s, "OBS_CLIMO_MEAN")   == 0) v = cpi.ocmn;
   else if(strcasecmp(s, "OBS_CLIMO_STDEV")  == 0) v = cpi.ocsd;
   else if(strcasecmp(s, "OBS_CLIMO_CDF")    == 0) {
      v = (is_bad_data(cpi.ocmn) || is_bad_data(cpi.ocsd) ?
           bad_data_double : normal_cdf(o, cpi.ocmn, cpi.ocsd));
   }
*/
        if(strcasecmp(s, "FCST")         == 0) v = f;
   else if(strcasecmp(s, "OBS")          == 0) v = o;
   else if(strcasecmp(s, "CLIMO_MEAN")   == 0) v = cpi.ocmn;
   else if(strcasecmp(s, "CLIMO_STDEV")  == 0) v = cpi.ocsd;
   else if(strcasecmp(s, "CLIMO_CDF")    == 0) {
      v = (is_bad_data(cpi.ocmn) || is_bad_data(cpi.ocsd) ?
           bad_data_double : normal_cdf(o, cpi.ocmn, cpi.ocsd));
   }
// MET #2924 End replace
   else {
      mlog << Error << "\nget_mpr_column_value() -> "
           << "unsupported matched pair column name requested in \""
           << conf_key_mpr_column << "\" (" << s << ")!\n\n";
      exit(1);
   }

   return v;
}

////////////////////////////////////////////////////////////////////////

void apply_mpr_thresh_mask(DataPlane &fcst_dp, DataPlane &obs_dp,
                           DataPlane &fcmn_dp, DataPlane &fcsd_dp,
                           DataPlane &ocmn_dp, DataPlane &ocsd_dp,
                           const StringArray &col_sa, const ThreshArray &col_ta) {

   // Check for no work to be done
   if(col_sa.n() == 0 && col_ta.n() == 0) return;

   // Check for constant length
   if(col_sa.n() != col_ta.n()) {
      mlog << Error << "\napply_mpr_thresh_mask() -> "
           << "the \"" << conf_key_mpr_column << "\" ("
           << write_css(col_sa) << ") and \"" << conf_key_mpr_thresh
           << "\" (" << write_css(col_ta)
           << ") config file entries must have the same length!\n\n";
      exit(1);
   }

   int  nxy = fcst_dp.nx() * fcst_dp.ny();
   int  n_skip = 0;
   bool fcmn_flag = !(fcmn_dp.is_empty());
   bool fcsd_flag = !(fcsd_dp.is_empty());
   bool ocmn_flag = !(ocmn_dp.is_empty());
   bool ocsd_flag = !(ocsd_dp.is_empty());

   // Loop over the pairs
   for(int i=0; i<nxy; i++) {

      // Store the climo data
      ClimoPntInfo cpi(
         (fcmn_flag ? fcmn_dp.buf()[i] : bad_data_double),
         (fcsd_flag ? fcsd_dp.buf()[i] : bad_data_double),
         (ocmn_flag ? ocmn_dp.buf()[i] : bad_data_double),
         (ocsd_flag ? ocsd_dp.buf()[i] : bad_data_double));

      // Check for bad data
      if(is_bad_data(fcst_dp.buf()[i])        ||
         is_bad_data(obs_dp.buf()[i])         ||
         (fcmn_flag && is_bad_data(cpi.fcmn)) ||
         (fcsd_flag && is_bad_data(cpi.fcsd)) ||
         (ocmn_flag && is_bad_data(cpi.ocmn)) ||
         (ocsd_flag && is_bad_data(cpi.ocsd))) continue;

      // Discard pairs which do not meet the threshold criteria
      if(!check_mpr_thresh(fcst_dp.buf()[i], obs_dp.buf()[i], cpi,
                           col_sa, col_ta)) {

         // Increment skip counter
         n_skip++;

         // Set point to bad data
         fcst_dp.buf()[i]               = bad_data_double;
         obs_dp.buf()[i]                = bad_data_double;
         if(fcmn_flag) fcmn_dp.buf()[i] = bad_data_double;
         if(fcsd_flag) fcsd_dp.buf()[i] = bad_data_double;
         if(ocmn_flag) ocmn_dp.buf()[i] = bad_data_double;
         if(ocsd_flag) ocsd_dp.buf()[i] = bad_data_double;
      }
   } // end for i

   mlog << Debug(3)
        << "Discarded " << n_skip << " of " << nxy
        << " pairs for matched pair filtering columns ("
        << write_css(col_sa) << ") and thresholds ("
        << col_ta.get_str() << ").\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void subset_wind_pairs(const PairDataPoint &pd_u, const PairDataPoint &pd_v,
                       const SingleThresh &ft, const SingleThresh &ot,
                       const SetLogic type,
                       PairDataPoint &out_pd_u, PairDataPoint &out_pd_v) {

   // Check for no work to be done
   if(ft.get_type() == thresh_na && ot.get_type() == thresh_na) {
      out_pd_u = pd_u;
      out_pd_v = pd_v;
      return;
   }

   int i;
   double fcst_wind, obs_wind, wgt;

   // Initialize and allocate memory for output pairs
   out_pd_u.erase();
   out_pd_v.erase();
   out_pd_u.extend(pd_u.n_obs);
   out_pd_v.extend(pd_v.n_obs);
   out_pd_u.set_climo_cdf_info_ptr(pd_u.cdf_info_ptr);
   out_pd_v.set_climo_cdf_info_ptr(pd_v.cdf_info_ptr);

   bool fcmn_flag = set_climo_flag(pd_u.f_na, pd_u.fcmn_na) &&
                    set_climo_flag(pd_v.f_na, pd_v.fcmn_na);
   bool fcsd_flag = set_climo_flag(pd_u.f_na, pd_u.fcsd_na) &&
                    set_climo_flag(pd_v.f_na, pd_v.fcsd_na);
   bool ocmn_flag = set_climo_flag(pd_u.f_na, pd_u.ocmn_na) &&
                    set_climo_flag(pd_v.f_na, pd_v.ocmn_na);
   bool ocsd_flag = set_climo_flag(pd_u.f_na, pd_u.ocsd_na) &&
                    set_climo_flag(pd_v.f_na, pd_v.ocsd_na);
   bool wgt_flag  = set_climo_flag(pd_u.f_na, pd_u.wgt_na);

   // Loop over the pairs
   for(i=0; i<pd_u.n_obs; i++) {
      
      wgt = (wgt_flag ? pd_u.wgt_na[i] : default_grid_weight);

      // Compute wind speeds
      fcst_wind = convert_u_v_to_wind(pd_u.f_na[i], pd_v.f_na[i]);
      obs_wind  = convert_u_v_to_wind(pd_u.o_na[i], pd_v.o_na[i]);

      ClimoPntInfo wind_cpi;
      wind_cpi.fcmn = (fcmn_flag ?
                       convert_u_v_to_wind(pd_u.fcmn_na[i], pd_v.fcmn_na[i]) :
                       bad_data_double);
      wind_cpi.fcsd = (fcsd_flag ?
                       convert_u_v_to_wind(pd_u.fcsd_na[i], pd_v.fcsd_na[i]) :
                       bad_data_double);
      wind_cpi.ocmn = (ocmn_flag ?
                       convert_u_v_to_wind(pd_u.ocmn_na[i], pd_v.ocmn_na[i]) :
                       bad_data_double);
      wind_cpi.ocsd = (ocsd_flag ?
                       convert_u_v_to_wind(pd_u.ocsd_na[i], pd_v.ocsd_na[i]) :
                       bad_data_double);

      // Check for bad data
      if(is_bad_data(fcst_wind)                    ||
         is_bad_data(obs_wind)                     ||
         (fcmn_flag && is_bad_data(wind_cpi.fcmn)) ||
         (fcsd_flag && is_bad_data(wind_cpi.fcsd)) ||
         (ocmn_flag && is_bad_data(wind_cpi.ocmn)) ||
         (ocsd_flag && is_bad_data(wind_cpi.ocsd)) ||
         (wgt_flag && is_bad_data(wgt))) continue;

      // Check wind speed thresholds
      if(check_fo_thresh(fcst_wind, obs_wind, wind_cpi,
                         ft, ot, type)) {

         // Store u/v climo data
         ClimoPntInfo u_cpi(pd_u.fcmn_na[i], pd_u.fcsd_na[i],
                            pd_u.ocmn_na[i], pd_u.ocsd_na[i]);
         ClimoPntInfo v_cpi(pd_v.fcmn_na[i], pd_v.fcsd_na[i],
                            pd_v.ocmn_na[i], pd_v.ocsd_na[i]);

         // Handle point data
         if(pd_u.is_point_vx()) {

            out_pd_u.add_point_pair(pd_u.sid_sa[i].c_str(),
                        pd_u.lat_na[i], pd_u.lon_na[i],
                        pd_u.x_na[i], pd_u.y_na[i], pd_u.vld_ta[i],
                        pd_u.lvl_na[i], pd_u.elv_na[i],
                        pd_u.f_na[i], pd_u.o_na[i],
                        pd_u.o_qc_sa[i].c_str(),
                        u_cpi, pd_u.wgt_na[i]);
            out_pd_v.add_point_pair(pd_v.sid_sa[i].c_str(),
                        pd_v.lat_na[i], pd_v.lon_na[i],
                        pd_v.x_na[i], pd_v.y_na[i], pd_v.vld_ta[i],
                        pd_v.lvl_na[i], pd_v.elv_na[i],
                        pd_v.f_na[i], pd_v.o_na[i],
                        pd_v.o_qc_sa[i].c_str(),
                        v_cpi, pd_v.wgt_na[i]);
         }
         // Handle gridded data
         else {
            out_pd_u.add_grid_pair(pd_u.f_na[i], pd_u.o_na[i],
                        u_cpi, pd_u.wgt_na[i]);
            out_pd_v.add_grid_pair(pd_v.f_na[i], pd_v.o_na[i],
                        v_cpi, pd_v.wgt_na[i]);
         }
      }
   } // end for i

   mlog << Debug(3)
        << "Using " << out_pd_u.n_obs << " of " << pd_u.n_obs
        << " vector pairs for forecast wind speed threshold "
        << ft.get_str() << ", observation wind speed threshold "
        << ot.get_str() << ", and field logic "
        << setlogic_to_string(type) << ".\n";

   return;
}

////////////////////////////////////////////////////////////////////////

PairDataPoint subset_climo_cdf_bin(const PairDataPoint &pd,
                                   const ThreshArray &ta, int i_bin) {

   // Check for no work to be done
   if(ta.n() == 0) return pd;

   int i;
   PairDataPoint out_pd;

   // Allocate memory for output pairs
   out_pd.extend(pd.n_obs);
   out_pd.set_climo_cdf_info_ptr(pd.cdf_info_ptr);

   bool fcmn_flag = set_climo_flag(pd.f_na, pd.fcmn_na);
   bool fcsd_flag = set_climo_flag(pd.f_na, pd.fcsd_na);
   bool ocmn_flag = set_climo_flag(pd.f_na, pd.ocmn_na);
   bool ocsd_flag = set_climo_flag(pd.f_na, pd.ocsd_na);
   bool wgt_flag  = set_climo_flag(pd.f_na, pd.wgt_na);

   // Loop over the pairs
   for(i=0; i<pd.n_obs; i++) {

      // Check for bad data
      if(is_bad_data(pd.f_na[i])                   ||
         is_bad_data(pd.o_na[i])                   ||
         (fcmn_flag && is_bad_data(pd.fcmn_na[i])) ||
         (fcsd_flag && is_bad_data(pd.fcsd_na[i])) ||
         (ocmn_flag && is_bad_data(pd.ocmn_na[i])) ||
         (ocsd_flag && is_bad_data(pd.ocsd_na[i])) ||
         (wgt_flag && is_bad_data(pd.wgt_na[i]))) continue;

      // Store climo data
      ClimoPntInfo cpi(pd.fcmn_na[i], pd.fcsd_na[i],
                       pd.ocmn_na[i], pd.ocsd_na[i]);

      // Keep pairs for the current bin.
      // check_bins() returns a 1-based bin value.
      if(ta.check_bins(pd.ocdf_na[i]) == (i_bin + 1)) {

         // Handle point data
         if(pd.is_point_vx()) {
            out_pd.add_point_pair(pd.sid_sa[i].c_str(), pd.lat_na[i],
                      pd.lon_na[i], pd.x_na[i], pd.y_na[i],
                      pd.vld_ta[i], pd.lvl_na[i], pd.elv_na[i],
                      pd.f_na[i], pd.o_na[i], pd.o_qc_sa[i].c_str(),
                      cpi, pd.wgt_na[i]);
         }
         // Handle gridded data
         else {
            out_pd.add_grid_pair(pd.f_na[i], pd.o_na[i], cpi, pd.wgt_na[i]);
         }
      }
   } // end for

   mlog << Debug(3)
        << "Using " << out_pd.n_obs << " of " << pd.n_obs
        << " pairs for climatology bin number " << i_bin+1 << ".\n";

   return out_pd;
}

////////////////////////////////////////////////////////////////////////
//
// End miscellaneous functions
//
////////////////////////////////////////////////////////////////////////
