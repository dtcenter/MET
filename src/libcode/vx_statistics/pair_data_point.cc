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

static const int REJECT_DEBUG_LEVEL = 9;

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

   seeps_mpr.clear();
   seeps.clear();
   seeps_climo = nullptr;
   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::clear() {

   PairBase::clear();

   f_na.clear();
   for (int idx=0; idx<seeps_mpr.size(); idx++) {
      if (seeps_mpr[idx]) delete seeps_mpr[idx];
   }
   seeps_mpr.clear();
   seeps.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::erase() {

   PairBase::erase();

   f_na.erase();
   for (int idx=0; idx<seeps_mpr.size(); idx++) {
      if (seeps_mpr[idx]) delete seeps_mpr[idx];
      seeps_mpr[idx] = nullptr;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::extend(int n) {

   PairBase::extend(n);

   f_na.extend(n);
   for (int idx=seeps_mpr.size(); idx<n; idx++) seeps_mpr.push_back(nullptr);

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
         if (add_point_pair(pd.sid_sa[i].c_str(), pd.lat_na[i], pd.lon_na[i],
                        pd.x_na[i], pd.y_na[i], pd.vld_ta[i],
                        pd.lvl_na[i], pd.elv_na[i],
                        pd.f_na[i], pd.o_na[i], pd.o_qc_sa[i].c_str(),
                        pd.cmn_na[i], pd.csd_na[i], pd.wgt_na[i])) {
            if (i < pd.seeps_mpr.size()) set_seeps_score(seeps_mpr[i], i);
         }
      }
   }
   // Handle gridded data
   else {
      add_grid_pair(pd.f_na, pd.o_na, pd.cmn_na, pd.csd_na, pd.wgt_na);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool PairDataPoint::add_point_pair(const char *sid, double lat, double lon,
                                   double x, double y, unixtime ut,
                                   double lvl, double elv,
                                   double f, double o, const char *qc,
                                   double cmn, double csd, double wgt) {

   if(!add_point_obs(sid, lat, lon, x, y, ut, lvl, elv, o, qc,
                     cmn, csd, wgt)) return false;

   f_na.add(f);
   seeps_mpr.push_back((SeepsScore *)nullptr);

   return true;
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::load_seeps_climo() {
   if (nullptr == seeps_climo) seeps_climo = get_seeps_climo();
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::set_seeps_thresh(const SingleThresh &p1_thresh) {
   if (nullptr != seeps_climo) seeps_climo->set_p1_thresh(p1_thresh);
   else mlog << Warning << "\nPairDataPoint::set_seeps_thresh() ignored t1_threshold."
             << " Load SEEPS climo first\n\n";
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
                                   double cmn, double csd, double wgt,
                                   SeepsScore *seeps) {

   if(i_obs < 0 || i_obs >= n_obs) {
      mlog << Error << "\nPairDataPoint::set_point_pair() -> "
           << "range check error: " << i_obs << " not in (0, "
           << n_obs << ").\n\n";
      exit(1);
   }

   set_point_obs(i_obs, sid, lat, lon, x, y, ut, lvl, elv,
                 o, qc, cmn, csd, wgt);

   f_na.set(i_obs, f);
   *seeps_mpr[i_obs] = *seeps;

   return;
}

////////////////////////////////////////////////////////////////////////

bool PairDataPoint::add_grid_pair(double f, double o,
                                  double cmn, double csd, double wgt) {

   add_grid_obs(o, cmn, csd, wgt);

   f_na.add(f);
   seeps_mpr.push_back(nullptr);

   return true;
}

////////////////////////////////////////////////////////////////////////

bool PairDataPoint::add_grid_pair(const NumArray &f_in,   const NumArray &o_in,
                                  const NumArray &cmn_in, const NumArray &csd_in,
                                  const NumArray &wgt_in) {

   // Check for constant length
   if(o_in.n() != f_in.n()   ||
      o_in.n() != cmn_in.n() ||
      o_in.n() != csd_in.n() ||
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
      add_climo(o_in[i], cmn_in[i], csd_in[i]);
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

   bool cmn_flag = set_climo_flag(f_na, cmn_na);
   bool csd_flag = set_climo_flag(f_na, csd_na);
   bool wgt_flag = set_climo_flag(f_na, wgt_na);

   // Loop over the pairs
   for(i=0; i<n_obs; i++) {

      // Check for bad data
      if(is_bad_data(f_na[i])                 ||
         is_bad_data(o_na[i])                 ||
         (cmn_flag && is_bad_data(cmn_na[i])) ||
         (csd_flag && is_bad_data(csd_na[i])) ||
         (wgt_flag && is_bad_data(wgt_na[i]))) continue;

      // Keep pairs which meet the threshold criteria
      if(check_fo_thresh(f_na[i], o_na[i], cmn_na[i], csd_na[i],
                         ft, ot, type)) {

         // Handle point data
         if(is_point_vx()) {
            if (out_pd.add_point_pair(sid_sa[i].c_str(), lat_na[i],
                      lon_na[i], x_na[i], y_na[i],
                      vld_ta[i], lvl_na[i], elv_na[i],
                      f_na[i], o_na[i], o_qc_sa[i].c_str(),
                      cmn_na[i], csd_na[i], wgt_na[i])) {
               out_pd.set_seeps_score(seeps_mpr[i], i);
            }
         }
         // Handle gridded data
         else {
            out_pd.add_grid_pair(f_na[i], o_na[i], cmn_na[i],
                      csd_na[i], wgt_na[i]);
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

   fcst_info    = (VarInfo *) nullptr;
   climo_info   = (VarInfo *) nullptr;
   obs_info     = (VarInfoGrib *) nullptr;

   pd           = (PairDataPoint ***) nullptr;
   rej_typ      = (int ***) nullptr;
   rej_mask     = (int ***) nullptr;
   rej_fcst     = (int ***) nullptr;
   rej_cmn      = (int ***) nullptr;
   rej_csd      = (int ***) nullptr;
   rej_mpr      = (int ***) nullptr;
   rej_dup      = (int ***) nullptr;

   n_msg_typ    = 0;
   n_mask       = 0;
   n_interp     = 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::clear() {
   int i, j, k;

   if(fcst_info)  { delete fcst_info;  fcst_info  = (VarInfo *)     nullptr; }
   if(climo_info) { delete climo_info; climo_info = (VarInfo *)     nullptr; }
   if(obs_info)   { delete obs_info;   obs_info   = (VarInfoGrib *) nullptr; }

   desc.clear();

   interp_thresh = 0;

   fcst_dpa.clear();
   climo_mn_dpa.clear();
   climo_sd_dpa.clear();
   sid_inc_filt.clear();
   sid_exc_filt.clear();
   obs_qty_inc_filt.clear();
   obs_qty_exc_filt.clear();
   mpr_column.clear();
   mpr_thresh.clear();

   fcst_ut     = (unixtime) 0;
   beg_ut      = (unixtime) 0;
   end_ut      = (unixtime) 0;

   n_try       = 0;
   rej_sid     = 0;
   rej_var     = 0;
   rej_vld     = 0;
   rej_obs     = 0;
   rej_grd     = 0;
   rej_lvl     = 0;
   rej_topo    = 0;
   rej_qty     = 0;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            pd[i][j][k].clear();
            rej_typ[i][j][k]  = 0;
            rej_mask[i][j][k] = 0;
            rej_fcst[i][j][k] = 0;
            rej_cmn[i][j][k]  = 0;
            rej_csd[i][j][k]  = 0;
            rej_mpr[i][j][k]  = 0;
            rej_dup[i][j][k]  = 0;
         }
      }
   }

   n_msg_typ     = 0;
   n_mask        = 0;
   n_interp      = 0;

   msg_typ_sfc.clear();
   msg_typ_lnd.clear();
   msg_typ_wtr.clear();

   sfc_info.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::assign(const VxPairDataPoint &vx_pd) {
   int i, j, k;

   clear();

   set_fcst_info(vx_pd.fcst_info);
   set_climo_info(vx_pd.climo_info);
   set_obs_info(vx_pd.obs_info);

   desc = vx_pd.desc;

   sid_inc_filt = vx_pd.sid_inc_filt;
   sid_exc_filt = vx_pd.sid_exc_filt;
   obs_qty_inc_filt = vx_pd.obs_qty_inc_filt;
   obs_qty_exc_filt = vx_pd.obs_qty_exc_filt;

   mpr_column = vx_pd.mpr_column;
   mpr_thresh = vx_pd.mpr_thresh;

   fcst_ut  = vx_pd.fcst_ut;
   beg_ut   = vx_pd.beg_ut;
   end_ut   = vx_pd.end_ut;

   n_try    = vx_pd.n_try;
   rej_typ  = vx_pd.rej_typ;
   rej_mask = vx_pd.rej_mask;
   rej_fcst = vx_pd.rej_fcst;
   rej_cmn  = vx_pd.rej_cmn;
   rej_csd  = vx_pd.rej_csd;
   rej_mpr  = vx_pd.rej_mpr;
   rej_dup  = vx_pd.rej_dup;

   interp_thresh = vx_pd.interp_thresh;

   fcst_dpa     = vx_pd.fcst_dpa;
   climo_mn_dpa = vx_pd.climo_mn_dpa;
   climo_sd_dpa = vx_pd.climo_sd_dpa;

   set_pd_size(vx_pd.n_msg_typ, vx_pd.n_mask, vx_pd.n_interp);

   for(i=0; i<vx_pd.n_msg_typ; i++) {
      for(j=0; j<vx_pd.n_mask; j++) {
         for(k=0; k<vx_pd.n_interp; k++) {

            pd[i][j][k]       = vx_pd.pd[i][j][k];
            rej_typ[i][j][k]  = vx_pd.rej_typ[i][j][k];
            rej_mask[i][j][k] = vx_pd.rej_mask[i][j][k];
            rej_fcst[i][j][k] = vx_pd.rej_fcst[i][j][k];
            rej_cmn[i][j][k]  = vx_pd.rej_cmn[i][j][k];
            rej_csd[i][j][k]  = vx_pd.rej_csd[i][j][k];
            rej_mpr[i][j][k]  = vx_pd.rej_mpr[i][j][k];
            rej_dup[i][j][k]  = vx_pd.rej_dup[i][j][k];
         }
      }
   }

   msg_typ_sfc = vx_pd.msg_typ_sfc;
   msg_typ_lnd = vx_pd.msg_typ_lnd;
   msg_typ_wtr = vx_pd.msg_typ_wtr;

   sfc_info = vx_pd.sfc_info;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_fcst_info(VarInfo *info) {
   VarInfoFactory f;

   // Deallocate, if necessary
   if(fcst_info) { delete fcst_info; fcst_info = (VarInfo *) nullptr; }

   // Perform a deep copy
   fcst_info = f.new_var_info(info->file_type());
   *fcst_info = *info;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_climo_info(VarInfo *info) {
   VarInfoFactory f;

   // Deallocate, if necessary
   if(climo_info) { delete climo_info; climo_info = (VarInfo *) nullptr; }

   // Perform a deep copy
   climo_info = f.new_var_info(info->file_type());
   *climo_info = *info;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_obs_info(VarInfoGrib *info) {

   // Deallocate, if necessary
   if(obs_info) { delete obs_info; obs_info = (VarInfoGrib *) nullptr; }

   // Perform a deep copy
   obs_info = new VarInfoGrib;
   *obs_info = *info;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_desc(const char *s) {

   desc = s;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_interp_thresh(double t) {

   interp_thresh = t;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_fcst_dpa(const DataPlaneArray &dpa) {

   fcst_dpa = dpa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_climo_mn_dpa(const DataPlaneArray &dpa) {

   climo_mn_dpa = dpa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_climo_sd_dpa(const DataPlaneArray &dpa) {

   climo_sd_dpa = dpa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_fcst_ut(const unixtime ut) {

   fcst_ut = ut;

   //  set the fcst_ut for all PairBase instances, used for duplicate logic
   for(int i=0; i < n_msg_typ; i++){
      for(int j=0; j < n_mask; j++){
         for(int k=0; k < n_interp; k++){
            pd[i][j][k].set_fcst_ut(ut);
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_beg_ut(const unixtime ut) {

   beg_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_end_ut(const unixtime ut) {

   end_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_sid_inc_filt(const StringArray &sa) {

   sid_inc_filt = sa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_sid_exc_filt(const StringArray &sa) {

   sid_exc_filt = sa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_obs_qty_inc_filt(const StringArray &sa) {

   obs_qty_inc_filt = sa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_obs_qty_exc_filt(const StringArray &sa) {

   obs_qty_exc_filt = sa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_pd_size(int types, int masks, int interps) {
   int i, j, k;

   // Store the dimensions for the PairDataPoint array
   n_msg_typ = types;
   n_mask    = masks;
   n_interp  = interps;

   // Allocate space for the PairDataPoint array
   pd       = new PairDataPoint ** [n_msg_typ];
   rej_typ  = new int **           [n_msg_typ];
   rej_mask = new int **           [n_msg_typ];
   rej_fcst = new int **           [n_msg_typ];
   rej_cmn  = new int **           [n_msg_typ];
   rej_csd  = new int **           [n_msg_typ];
   rej_mpr  = new int **           [n_msg_typ];
   rej_dup  = new int **           [n_msg_typ];

   for(i=0; i<n_msg_typ; i++) {
      pd[i]       = new PairDataPoint * [n_mask];
      rej_typ[i]  = new int *           [n_mask];
      rej_mask[i] = new int *           [n_mask];
      rej_fcst[i] = new int *           [n_mask];
      rej_cmn[i]  = new int *           [n_mask];
      rej_csd[i]  = new int *           [n_mask];
      rej_mpr[i]  = new int *           [n_mask];
      rej_dup[i]  = new int *           [n_mask];

      for(j=0; j<n_mask; j++) {
         pd[i][j]       = new PairDataPoint [n_interp];
         rej_typ[i][j]  = new int           [n_interp];
         rej_mask[i][j] = new int           [n_interp];
         rej_fcst[i][j] = new int           [n_interp];
         rej_cmn[i][j]  = new int           [n_interp];
         rej_csd[i][j]  = new int           [n_interp];
         rej_mpr[i][j]  = new int           [n_interp];
         rej_dup[i][j]  = new int           [n_interp];

         for(k=0; k<n_interp; k++) {
            rej_typ[i][j][k]  = 0;
            rej_mask[i][j][k] = 0;
            rej_fcst[i][j][k] = 0;
            rej_cmn[i][j][k]  = 0;
            rej_csd[i][j][k]  = 0;
            rej_mpr[i][j][k]  = 0;
            rej_dup[i][j][k]  = 0;
         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_msg_typ(int i_msg_typ, const char *name) {
   int i, j;

   for(i=0; i<n_mask; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i_msg_typ][i][j].set_msg_typ(name);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_msg_typ_vals(int i_msg_typ, const StringArray &sa) {
   int i, j;

   for(i=0; i<n_mask; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i_msg_typ][i][j].set_msg_typ_vals(sa);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_mask_area(int i_mask, const char *name,
                                    MaskPlane *mp_ptr) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i][i_mask][j].set_mask_name(name);
         pd[i][i_mask][j].set_mask_area_ptr(mp_ptr);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_mask_sid(int i_mask, const char *name,
                                   StringArray *sid_ptr) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i][i_mask][j].set_mask_name(name);
         pd[i][i_mask][j].set_mask_sid_ptr(sid_ptr);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_mask_llpnt(int i_mask, const char *name,
                                     MaskLatLon *llpnt_ptr) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i][i_mask][j].set_mask_name(name);
         pd[i][i_mask][j].set_mask_llpnt_ptr(llpnt_ptr);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_interp(int i_interp,
                                 const char *interp_mthd_str, int width,
                                 GridTemplateFactory::GridTemplates shape) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         pd[i][j][i_interp].set_interp_mthd(interp_mthd_str);
         pd[i][j][i_interp].set_interp_wdth(width);
         pd[i][j][i_interp].set_interp_shape(shape);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_interp(int i_interp,
                                 InterpMthd mthd, int width,
                                 GridTemplateFactory::GridTemplates shape) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         pd[i][j][i_interp].set_interp_mthd(mthd);
         pd[i][j][i_interp].set_interp_wdth(width);
         pd[i][j][i_interp].set_interp_shape(shape);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_mpr_thresh(const StringArray &sa, const ThreshArray &ta) {

   // Check for constant length
   if(sa.n() != ta.n()) {
      mlog << Error << "\nVxPairDataPoint::set_mpr_thresh() -> "
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

void VxPairDataPoint::set_climo_cdf_info_ptr(const ClimoCDFInfo *info) {

   for(int i=0; i<n_msg_typ; i++) {
      for(int j=0; j<n_mask; j++) {
         for(int k=0; k<n_interp; k++) {
            pd[i][j][k].set_climo_cdf_info_ptr(info);
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_msg_typ_sfc(const StringArray &sa) {

   msg_typ_sfc = sa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_msg_typ_lnd(const StringArray &sa) {

   msg_typ_lnd = sa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_msg_typ_wtr(const StringArray &sa) {

   msg_typ_wtr = sa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_sfc_info(const SurfaceInfo &si) {

   sfc_info = si;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::add_point_obs(float *hdr_arr, const char *hdr_typ_str,
                                    const char *hdr_sid_str, unixtime hdr_ut,
                                    const char *obs_qty, float *obs_arr,
                                    Grid &gr, const char *var_name,
                                    const DataPlane *wgt_dp) {
   int i, j, k, x, y;
   double hdr_lat, hdr_lon, hdr_elv;
   double obs_x, obs_y, obs_lvl, obs_hgt, to_lvl;
   double fcst_v, cmn_v, csd_v, obs_v, wgt_v;
   int f_lvl_blw, f_lvl_abv;
   int cmn_lvl_blw, cmn_lvl_abv;
   int csd_lvl_blw, csd_lvl_abv;
   ConcatString reason_cs;

   // Increment the number of tries count
   n_try++;

   // Check the station ID inclusion and exclusion lists
   if((sid_inc_filt.n() && !sid_inc_filt.has(hdr_sid_str)) ||
      (sid_exc_filt.n() &&  sid_exc_filt.has(hdr_sid_str))) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str()
              << ", skipping observation station id:\n"
              << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                     hdr_ut, obs_qty, obs_arr, var_name)
              << "\n";
      }

      rej_sid++;
      return;
   }

   // Check whether the GRIB code for the observation matches
   // the specified code
   if((var_name != 0) && (0 < strlen(var_name))) {
      if(var_name != obs_info->name()) {

         if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
            mlog << Debug(REJECT_DEBUG_LEVEL)
                 << "For " << fcst_info->magic_str() << " versus "
                 << obs_info->magic_str()
                 << ", skipping observation variable name:\n"
                 << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                        hdr_ut, obs_qty, obs_arr, var_name)
                 << "\n";
         }

         rej_var++;
         return;
      }
   }
   else if(obs_info->code() != nint(obs_arr[1])) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str()
              << ", skipping observation variable GRIB code:\n"
              << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                     hdr_ut, obs_qty, obs_arr, var_name)
              << "\n";
      }

      rej_var++;
      return;
   }

   // Check the observation quality include and exclude options
   if((obs_qty_inc_filt.n() > 0 && !obs_qty_inc_filt.has(obs_qty)) ||
      (obs_qty_exc_filt.n() > 0 &&  obs_qty_exc_filt.has(obs_qty))) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str()
              << ", skipping observation quality control string:\n"
              << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                     hdr_ut, obs_qty, obs_arr, var_name)
              << "\n";
      }

      rej_qty++;
      return;
   }

   // Check whether the observation time falls within the valid time
   // window
   if(hdr_ut < beg_ut || hdr_ut > end_ut) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str()
              << ", skipping observation valid time:\n"
              << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                     hdr_ut, obs_qty, obs_arr, var_name)
              << "\n";
      }

      rej_vld++;
      return;
   }

   bool precip_flag = fcst_info->is_precipitation() &&
                      obs_info->is_precipitation();
   int precip_interval = bad_data_int;
   if (precip_flag) {
      if (wgt_dp) precip_interval = wgt_dp->accum();
      else precip_interval = fcst_dpa[0].accum();
   }

   hdr_lat = hdr_arr[0];
   hdr_lon = hdr_arr[1];
   hdr_elv = hdr_arr[2];

   obs_lvl = obs_arr[2];
   obs_hgt = obs_arr[3];

   // Apply observation processing logic
   obs_v = pd[0][0][0].process_obs(obs_info, obs_arr[4]);

   // Check whether the observation value contains valid data
   if(is_bad_data(obs_v)) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str()
              << ", skipping observation with bad data value:\n"
              << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                     hdr_ut, obs_qty, obs_arr, var_name)
              << "\n";
      }

      rej_obs++;
      return;
   }

   // Convert the lat/lon value to x/y
   gr.latlon_to_xy(hdr_lat, -1.0*hdr_lon, obs_x, obs_y);
   x = nint(obs_x);
   y = nint(obs_y);

   // Check if the observation's lat/lon is on the grid
   if(((x < 0 || x >= gr.nx()) && !gr.wrap_lon()) ||
        y < 0 || y >= gr.ny()) {

      if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
         mlog << Debug(REJECT_DEBUG_LEVEL)
              << "For " << fcst_info->magic_str() << " versus "
              << obs_info->magic_str()
              << ", skipping observation off the grid where (x, y) = ("
              << x << ", " << y << ") and grid (nx, ny) = (" << gr.nx()
              << ", " << gr.ny() << "):\n"
              << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                     hdr_ut, obs_qty, obs_arr, var_name)
              << "\n";
      }

      rej_grd++;
      return;
   }

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
                 << obs_info->magic_str()
                 << ", skipping observation due to bad topography values "
                 << "where observation elevation = " << hdr_elv
                 << " and model topography = " << topo << ":\n"
                 << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                        hdr_ut, obs_qty, obs_arr, var_name)
                 << "\n";
         }

         rej_topo++;
         return;
      }

      // Check the topography difference threshold
      if(!sfc_info.topo_use_obs_thresh.check(topo - hdr_elv)) {

         if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
            mlog << Debug(REJECT_DEBUG_LEVEL)
                 << "For " << fcst_info->magic_str() << " versus "
                 << obs_info->magic_str()
                 << ", skipping observation due to topography difference "
                 << "where observation elevation (" << hdr_elv
                 << ") minus model topography (" << topo << ") = "
                 << topo - hdr_elv << " is not "
                 << sfc_info.topo_use_obs_thresh.get_str() << ":\n"
                 << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                        hdr_ut, obs_qty, obs_arr, var_name)
                 << "\n";
         }

         rej_topo++;
         return;
      }
   }

   // For pressure levels, check if the observation pressure level
   // falls in the requested range.
   if(obs_info->level().type() == LevelType_Pres) {

      if(obs_lvl < obs_info->level().lower() ||
         obs_lvl > obs_info->level().upper()) {

         if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
            mlog << Debug(REJECT_DEBUG_LEVEL)
                 << "For " << fcst_info->magic_str() << " versus "
                 << obs_info->magic_str()
                 << ", skipping observation pressure level value:\n"
                 << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                        hdr_ut, obs_qty, obs_arr, var_name)
                 << "\n";
         }

         rej_lvl++;
         return;
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
                 << obs_info->magic_str()
                 << ", skipping observation accumulation interval:\n"
                 << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                        hdr_ut, obs_qty, obs_arr, var_name)
                 << "\n";
         }

         rej_lvl++;
         return;
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
                 << obs_info->magic_str()
                 << ", skipping observation level value:\n"
                 << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                        hdr_ut, obs_qty, obs_arr, var_name)
                 << "\n";
         }

         rej_lvl++;
         return;
      }
   }

   // For a single forecast field
   if(fcst_dpa.n_planes() == 1) {
      f_lvl_blw = 0;
      f_lvl_abv = 0;
   }
   // For multiple forecast fields, find the levels above and below
   // the observation point.
   else {
      // Interpolate using the observation pressure level or height
      to_lvl = (fcst_info->level().type() == LevelType_Pres ?
                obs_lvl : obs_hgt);
      find_vert_lvl(fcst_dpa, to_lvl, f_lvl_blw, f_lvl_abv);
   }

   // For a single climatology mean field
   if(climo_mn_dpa.n_planes() == 1) {
      cmn_lvl_blw = 0;
      cmn_lvl_abv = 0;
   }
   // For multiple climatology mean fields, find the levels above and
   // below the observation point.
   else {
      // Interpolate using the observation pressure level or height
      to_lvl = (fcst_info->level().type() == LevelType_Pres ?
                obs_lvl : obs_hgt);
      find_vert_lvl(climo_mn_dpa, to_lvl, cmn_lvl_blw, cmn_lvl_abv);
   }

   // For a single climatology standard deviation field
   if(climo_sd_dpa.n_planes() == 1) {
      csd_lvl_blw = 0;
      csd_lvl_abv = 0;
   }
   // For multiple climatology standard deviation fields, find the
   // levels above and below the observation point.
   else {
      // Interpolate using the observation pressure level or height
      to_lvl = (fcst_info->level().type() == LevelType_Pres ?
                obs_lvl : obs_hgt);
      find_vert_lvl(climo_sd_dpa, to_lvl, csd_lvl_blw, csd_lvl_abv);
   }

   // When verifying a vertical level forecast against a surface message
   // type, set the observation level value to bad data so that it's not
   // used in the duplicate logic.
   if(obs_info->level().type() == LevelType_Vert &&
      msg_typ_sfc.reg_exp_match(hdr_typ_str)) {
      obs_lvl = bad_data_double;
   }

   // Set flag for specific humidity
   bool spfh_flag = fcst_info->is_specific_humidity() &&
                     obs_info->is_specific_humidity();

   // Look through all of the PairDataPoint objects to see if the
   // observation should be added.

   bool has_seeps = false;
   SeepsScore *seeps = 0;

   // Check the message types
   for(i=0; i<n_msg_typ; i++) {

      //
      // Check for a matching PrepBufr message type
      //
      if(!pd[i][0][0].msg_typ_vals.has(hdr_typ_str)) {

         if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
            mlog << Debug(REJECT_DEBUG_LEVEL)
                 << "For " << fcst_info->magic_str() << " versus "
                 << obs_info->magic_str()
                 << ", skipping observation message type:\n"
                 << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                        hdr_ut, obs_qty, obs_arr, var_name)
                 << "\n";
         }

         inc_count(rej_typ, i);
         continue;
      }

      // Check the masking areas and points
      for(j=0; j<n_mask; j++) {

         // Check for the obs falling within the masking region
         if(pd[i][j][0].mask_area_ptr != (MaskPlane *) 0) {
            if(!pd[i][j][0].mask_area_ptr->s_is_on(x, y)) {

               if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
                  mlog << Debug(REJECT_DEBUG_LEVEL)
                       << "For " << fcst_info->magic_str() << " versus "
                       << obs_info->magic_str()
                       << ", skipping observation based on spatial masking region:\n"
                       << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                              hdr_ut, obs_qty, obs_arr, var_name)
                       << "\n";
               }

               inc_count(rej_mask, i, j);
               continue;
            }
         }
         // Otherwise, check for the obs Station ID's presence in the
         // masking SID list
         else if(pd[i][j][0].mask_sid_ptr != (StringArray *) 0) {
            if(!pd[i][j][0].mask_sid_ptr->has(hdr_sid_str)) {

               if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
                  mlog << Debug(REJECT_DEBUG_LEVEL)
                       << "For " << fcst_info->magic_str() << " versus "
                       << obs_info->magic_str()
                       << ", skipping observation based on masking station id list:\n"
                       << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                              hdr_ut, obs_qty, obs_arr, var_name)
                       << "\n";
               }

               inc_count(rej_mask, i, j);
               continue;
            }
         }
         // Otherwise, check observation lat/lon thresholds
         else if(pd[i][j][0].mask_llpnt_ptr != (MaskLatLon *) 0) {
            if(!pd[i][j][0].mask_llpnt_ptr->lat_thresh.check(hdr_lat) ||
               !pd[i][j][0].mask_llpnt_ptr->lon_thresh.check(hdr_lon)) {

               if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
                  mlog << Debug(REJECT_DEBUG_LEVEL)
                       << "For " << fcst_info->magic_str() << " versus "
                       << obs_info->magic_str()
                       << ", skipping observation based on latitude/longitude thesholds:\n"
                       << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                              hdr_ut, obs_qty, obs_arr, var_name)
                       << "\n";
               }

               inc_count(rej_mask, i, j);
               continue;
            }
         }

         // Compute the interpolated values
         for(k=0; k<n_interp; k++) {

            // Compute the interpolated forecast value using the
            // observation pressure level or height
            to_lvl = (fcst_info->level().type() == LevelType_Pres ?
                      obs_lvl : obs_hgt);

            // Compute the interpolated climatology mean
            cmn_v = compute_interp(climo_mn_dpa, obs_x, obs_y, obs_v,
                       bad_data_double, bad_data_double,
                       pd[0][0][k].interp_mthd, pd[0][0][k].interp_wdth,
                       pd[0][0][k].interp_shape, gr.wrap_lon(),
                       interp_thresh, spfh_flag,
                       fcst_info->level().type(),
                       to_lvl, cmn_lvl_blw, cmn_lvl_abv);

            // Check for bad data
            if(climo_mn_dpa.n_planes() > 0 && is_bad_data(cmn_v)) {

               if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
                  mlog << Debug(REJECT_DEBUG_LEVEL)
                       << "For " << fcst_info->magic_str() << " versus "
                       << obs_info->magic_str()
                       << ", skipping observation based on bad climatological mean value:\n"
                       << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                              hdr_ut, obs_qty, obs_arr, var_name)
                       << "\n";
               }

               inc_count(rej_cmn, i, j, k);
               continue;
            }

            // Check for valid interpolation options
            if(climo_sd_dpa.n_planes() > 0 &&
               (pd[0][0][k].interp_mthd == InterpMthd::Min    ||
                pd[0][0][k].interp_mthd == InterpMthd::Max    ||
                pd[0][0][k].interp_mthd == InterpMthd::Median ||
                pd[0][0][k].interp_mthd == InterpMthd::Best)) {
               mlog << Warning << "\nVxPairDataPoint::add_point_obs() -> "
                    << "applying the "
                    << interpmthd_to_string(pd[0][0][k].interp_mthd)
                    << " interpolation method to climatological spread "
                    << "may cause unexpected results.\n\n";
            }

            // Compute the interpolated climatology standard deviation
            csd_v = compute_interp(climo_sd_dpa, obs_x, obs_y, obs_v,
                       bad_data_double, bad_data_double,
                       pd[0][0][k].interp_mthd, pd[0][0][k].interp_wdth,
                       pd[0][0][k].interp_shape, gr.wrap_lon(),
                       interp_thresh, spfh_flag,
                       fcst_info->level().type(),
                       to_lvl, csd_lvl_blw, csd_lvl_abv);

            // Check for bad data
            if(climo_sd_dpa.n_planes() > 0 && is_bad_data(csd_v)) {

               if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
                  mlog << Debug(REJECT_DEBUG_LEVEL)
                       << "For " << fcst_info->magic_str() << " versus "
                       << obs_info->magic_str()
                       << ", skipping observation based on bad climatological standard deviation value:\n"
                       << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                              hdr_ut, obs_qty, obs_arr, var_name)
                       << "\n";
               }

               inc_count(rej_csd, i, j, k);
               continue;
            }

            // For surface verification, apply land/sea and topo masks
            if((sfc_info.land_ptr || sfc_info.topo_ptr) &&
               (msg_typ_sfc.reg_exp_match(hdr_typ_str))) {

               bool is_land = msg_typ_lnd.has(hdr_typ_str);

               // Check for a single forecast DataPlane
               if(fcst_dpa.n_planes() != 1) {
                  mlog << Error << "\nVxPairDataPoint::add_point_obs() -> "
                       << "unexpected number of forecast levels ("
                       << fcst_dpa.n_planes()
                       << ") for surface verification! Set \"land_mask.flag\" and "
                       << "\"topo_mask.flag\" to false to disable this check.\n\n";
                  exit(1);
               }

               fcst_v = compute_sfc_interp(fcst_dpa[0], obs_x, obs_y, hdr_elv, obs_v,
                           pd[0][0][k].interp_mthd, pd[0][0][k].interp_wdth,
                           pd[0][0][k].interp_shape, gr.wrap_lon(),
                           interp_thresh, sfc_info, is_land);
            }
            // Otherwise, compute interpolated value
            else {
               fcst_v = compute_interp(fcst_dpa, obs_x, obs_y, obs_v, cmn_v, csd_v,
                           pd[0][0][k].interp_mthd, pd[0][0][k].interp_wdth,
                           pd[0][0][k].interp_shape, gr.wrap_lon(),
                           interp_thresh, spfh_flag,
                           fcst_info->level().type(),
                           to_lvl, f_lvl_blw, f_lvl_abv);
            }

            if(is_bad_data(fcst_v)) {

               if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
                  mlog << Debug(REJECT_DEBUG_LEVEL)
                       << "For " << fcst_info->magic_str() << " versus "
                       << obs_info->magic_str()
                       << ", skipping observation based due to bad data in the " 
                       << interpmthd_to_string(pd[0][0][k].interp_mthd) << "("
                       << pd[0][0][k].interp_wdth * pd[0][0][k].interp_wdth
                       << ") interpolated forecast value:\n"
                       << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                              hdr_ut, obs_qty, obs_arr, var_name)
                       << "\n";
               }

               inc_count(rej_fcst, i, j, k);
               continue;
            }

            // Check matched pair filtering options
            if(!check_mpr_thresh(fcst_v, obs_v, cmn_v, csd_v,
                                 mpr_column, mpr_thresh, &reason_cs)) {

               if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
                  mlog << Debug(REJECT_DEBUG_LEVEL)
                       << "For " << fcst_info->magic_str() << " versus "
                       << obs_info->magic_str()
                       << ", skipping observation due to matched pair filter since "
                       << reason_cs << ":\n"
                       << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                              hdr_ut, obs_qty, obs_arr, var_name)
                       << "\n";
               }

               inc_count(rej_mpr, i, j, k);
               continue;
            }

            // Compute weight for current point
            wgt_v = (wgt_dp == (DataPlane *) 0 ?
                     default_grid_weight : wgt_dp->get(x, y));

            // Add the forecast, climatological, and observation data
            // Weight is from the nearest grid point
            if(!pd[i][j][k].add_point_pair(hdr_sid_str,
                  hdr_lat, hdr_lon, obs_x, obs_y, hdr_ut, obs_lvl,
                  obs_hgt, fcst_v, obs_v, obs_qty, cmn_v, csd_v,
                  wgt_v)) {

               if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
                  mlog << Debug(REJECT_DEBUG_LEVEL)
                       << "For " << fcst_info->magic_str() << " versus "
                       << obs_info->magic_str()
                       << ", skipping observation since it is a duplicate:\n"
                       << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                              hdr_ut, obs_qty, obs_arr, var_name)
                       << "\n";
               }

               inc_count(rej_dup, i, j, k);
            }
            seeps = 0;
            if (precip_flag && precip_interval == 24*60*60) {  // 24 hour precip only
               seeps = pd[i][j][k].compute_seeps(hdr_sid_str, fcst_v, obs_v, hdr_ut);
            }
            pd[i][j][k].set_seeps_score(seeps);
            if (seeps) delete seeps;

            if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
               mlog << Debug(REJECT_DEBUG_LEVEL)
                    << "For " << fcst_info->magic_str() << " versus "
                    << obs_info->magic_str() << ", "
                    << pd[i][0][0].msg_typ << " message type, "
                    << pd[0][j][0].mask_name << " masking region, and "
                    << interpmthd_to_string(pd[0][0][k].interp_mthd) << "("
                    << pd[0][0][k].interp_wdth * pd[0][0][k].interp_wdth
                    << ") interpolation method, adding observation:\n"
                    << point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                           hdr_ut, obs_qty, obs_arr, var_name)
                    << "\n";
            }

         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

int VxPairDataPoint::get_n_pair() const {
   int n, i, j, k;

   if(n_msg_typ == 0 || n_mask == 0 || n_interp == 0) {
      mlog << Warning << "\nVxPairDataPoint::get_n_pair() -> "
           << "set_pd_size() has not been called yet!\n\n";
   }

   for(i=0, n=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            n += pd[i][j][k].n_obs;
         }
      }
   }

   return n;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_duplicate_flag(DuplicateType duplicate_flag) {

   if(n_msg_typ == 0 || n_mask == 0 || n_interp == 0) {
      mlog << Warning << "\nVxPairDataPoint::set_duplicate_flag() -> "
           << "set_pd_size() has not been called yet!\n\n";
   }

   for(int i=0; i < n_msg_typ; i++){
      for(int j=0; j < n_mask; j++){
         for(int k=0; k < n_interp; k++){
            pd[i][j][k].set_check_unique(duplicate_flag == DuplicateType::Unique);
         }
      }
   }

}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_obs_summary(ObsSummary s) {

   if(n_msg_typ == 0 || n_mask == 0 || n_interp == 0) {
      mlog << Warning << "\nVxPairDataPoint::set_obs_summary() -> "
           << "set_pd_size() has not been called yet!\n\n";
   }

   for(int i=0; i < n_msg_typ; i++){
      for(int j=0; j < n_mask; j++){
         for(int k=0; k < n_interp; k++){
            pd[i][j][k].set_obs_summary(s);
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_obs_perc_value(int percentile) {

   if(n_msg_typ == 0 || n_mask == 0 || n_interp == 0) {
      mlog << Warning << "\nVxPairDataPoint::set_obs_perc_value() -> "
           << "set_pd_size() has not been called yet!\n\n";
   }

   for(int i=0; i < n_msg_typ; i++){
      for(int j=0; j < n_mask; j++){
         for(int k=0; k < n_interp; k++){
            pd[i][j][k].set_obs_perc_value(percentile);
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::load_seeps_climo() {
   for(int i=0; i < n_msg_typ; i++){
      for(int j=0; j < n_mask; j++){
         for(int k=0; k < n_interp; k++){
            pd[i][j][k].load_seeps_climo();
         }
      }
   }
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_seeps_thresh(const SingleThresh &p1_thresh) {
   for(int i=0; i < n_msg_typ; i++){
      for(int j=0; j < n_mask; j++){
         for(int k=0; k < n_interp; k++){
            pd[i][j][k].set_seeps_thresh(p1_thresh);
         }
      }
   }
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::print_obs_summary() {

   if(n_msg_typ == 0 || n_mask == 0 || n_interp == 0) {
      mlog << Warning << "\nVxPairDataPoint::print_obs_summary() -> "
           << "set_pd_size() has not been called yet!\n\n";
   }

   for(int i=0; i < n_msg_typ; i++){
      for(int j=0; j < n_mask; j++){
         for(int k=0; k < n_interp; k++){
            pd[i][j][k].print_obs_summary();
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::calc_obs_summary() {

   if(n_msg_typ == 0 || n_mask == 0 || n_interp == 0) {
      mlog << Warning << "\nVxPairDataPoint::calc_obs_summary() -> "
           << "set_pd_size() has not been called yet!\n\n";
   }

   for(int i=0; i < n_msg_typ; i++){
      for(int j=0; j < n_mask; j++){
         for(int k=0; k < n_interp; k++){
            pd[i][j][k].calc_obs_summary();
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::inc_count(int ***&rej, int i) {
   int j, k;

   for(j=0; j<n_mask; j++) {
      for(k=0; k<n_interp; k++) {
         rej[i][j][k]++;
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::inc_count(int ***&rej, int i, int j) {
   int k;

   for(k=0; k<n_interp; k++) {
      rej[i][j][k]++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::inc_count(int ***&rej, int i, int j, int k) {

   rej[i][j][k]++;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Begin miscellaneous functions
//
////////////////////////////////////////////////////////////////////////

bool check_fo_thresh(double f, double o, double cmn, double csd,
                     const SingleThresh &ft, const SingleThresh &ot,
                     const SetLogic type) {
   bool status = true;
   bool fcheck = ft.check(f, cmn, csd);
   bool ocheck = ot.check(o, cmn, csd);
   SetLogic t  = type;

   // If either of the thresholds is NA, reset the logic to intersection
   // because an NA threshold is always true.
   if(ft.get_type() == thresh_na || ot.get_type() == thresh_na) {
      t = SetLogic::Intersection;
   }

   switch(t) {
      case(SetLogic::Union):
         if(!fcheck && !ocheck) status = false;
         break;

      case(SetLogic::Intersection):
         if(!fcheck || !ocheck) status = false;
         break;

      case(SetLogic::SymDiff):
         if(fcheck == ocheck) status = false;
         break;

      default:
         mlog << Error << "\ncheck_fo_thresh() -> "
              << "Unexpected SetLogic value of " << enum_class_as_int(type) << ".\n\n";
         exit(1);
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool check_mpr_thresh(double f, double o, double cmn, double csd,
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
      v = get_mpr_column_value(f, o, cmn, csd, sa[0].c_str());

      // If multiple columns, compute the requested difference
      if(sa.n() > 1) {

         // Loop through the columns
         for(j=1; j<sa.n(); j++) {

            // Get the current column value
            v_cur = get_mpr_column_value(f, o, cmn, csd, sa[j].c_str());

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

double get_mpr_column_value(double f, double o, double cmn, double csd,
                            const char *s) {
   double v;

        if(strcasecmp(s, "FCST")        == 0) v = f;
   else if(strcasecmp(s, "OBS")         == 0) v = o;
   else if(strcasecmp(s, "CLIMO_MEAN")  == 0) v = cmn;
   else if(strcasecmp(s, "CLIMO_STDEV") == 0) v = csd;
   else if(strcasecmp(s, "CLIMO_CDF")   == 0) {
      v = (is_bad_data(cmn) || is_bad_data(csd) ?
           bad_data_double : normal_cdf(o, cmn, csd));
   }
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
                           DataPlane &cmn_dp, DataPlane &csd_dp,
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
   bool cmn_flag = !(cmn_dp.is_empty());
   bool csd_flag = !(csd_dp.is_empty());

   // Loop over the pairs
   for(int i=0; i<nxy; i++) {

      double cmn = (cmn_flag ? cmn_dp.buf()[i] : bad_data_double);
      double csd = (csd_flag ? csd_dp.buf()[i] : bad_data_double);

      // Check for bad data
      if(is_bad_data(fcst_dp.buf()[i])  ||
         is_bad_data(obs_dp.buf()[i])   ||
         (cmn_flag && is_bad_data(cmn)) ||
         (csd_flag && is_bad_data(csd))) continue;

      // Discard pairs which do not meet the threshold criteria
      if(!check_mpr_thresh(fcst_dp.buf()[i], obs_dp.buf()[i], cmn, csd,
                           col_sa, col_ta)) {

         // Increment skip counter
         n_skip++;

         // Set point to bad data
         fcst_dp.buf()[i] = bad_data_double;
         obs_dp.buf()[i]  = bad_data_double;
         if(cmn_flag) cmn_dp.buf()[i] = bad_data_double;
         if(csd_flag) csd_dp.buf()[i] = bad_data_double;
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
   double fcst_wind, obs_wind, cmn_wind, csd_wind, wgt;

   // Initialize and allocate memory for output pairs
   out_pd_u.erase();
   out_pd_v.erase();
   out_pd_u.extend(pd_u.n_obs);
   out_pd_v.extend(pd_v.n_obs);
   out_pd_u.set_climo_cdf_info_ptr(pd_u.cdf_info_ptr);
   out_pd_v.set_climo_cdf_info_ptr(pd_v.cdf_info_ptr);

   bool cmn_flag = set_climo_flag(pd_u.f_na, pd_u.cmn_na) &&
                   set_climo_flag(pd_v.f_na, pd_v.cmn_na);
   bool csd_flag = set_climo_flag(pd_u.f_na, pd_u.csd_na) &&
                   set_climo_flag(pd_v.f_na, pd_v.csd_na);
   bool wgt_flag = set_climo_flag(pd_u.f_na, pd_u.wgt_na);

   // Loop over the pairs
   for(i=0; i<pd_u.n_obs; i++) {
      
      wgt = (wgt_flag ? pd_u.wgt_na[i] : default_grid_weight);

      // Compute wind speeds
      fcst_wind = convert_u_v_to_wind(pd_u.f_na[i], pd_v.f_na[i]);
      obs_wind  = convert_u_v_to_wind(pd_u.o_na[i], pd_v.o_na[i]);
      cmn_wind  = (cmn_flag ?
                   convert_u_v_to_wind(pd_u.cmn_na[i], pd_v.cmn_na[i]) :
                   bad_data_double);
      csd_wind  = (csd_flag ?
                   convert_u_v_to_wind(pd_u.csd_na[i], pd_v.csd_na[i]) :
                   bad_data_double);

      // Check for bad data
      if(is_bad_data(fcst_wind)              ||
         is_bad_data(obs_wind)               ||
         (cmn_flag && is_bad_data(cmn_wind)) ||
         (csd_flag && is_bad_data(csd_wind)) ||
         (wgt_flag && is_bad_data(wgt))) continue;

      // Check wind speed thresholds
      if(check_fo_thresh(fcst_wind, obs_wind,
                         cmn_wind,  csd_wind,
                         ft, ot, type)) {

         // Handle point data
         if(pd_u.is_point_vx()) {
            out_pd_u.add_point_pair(pd_u.sid_sa[i].c_str(),
                        pd_u.lat_na[i], pd_u.lon_na[i],
                        pd_u.x_na[i], pd_u.y_na[i], pd_u.vld_ta[i],
                        pd_u.lvl_na[i], pd_u.elv_na[i],
                        pd_u.f_na[i], pd_u.o_na[i],
                        pd_u.o_qc_sa[i].c_str(), pd_u.cmn_na[i],
                        pd_u.csd_na[i], pd_u.wgt_na[i]);
            out_pd_v.add_point_pair(pd_v.sid_sa[i].c_str(),
                        pd_v.lat_na[i], pd_v.lon_na[i],
                        pd_v.x_na[i], pd_v.y_na[i], pd_v.vld_ta[i],
                        pd_v.lvl_na[i], pd_v.elv_na[i],
                        pd_v.f_na[i], pd_v.o_na[i],
                        pd_v.o_qc_sa[i].c_str(),
                        pd_v.cmn_na[i], pd_v.csd_na[i],
                        pd_v.wgt_na[i]);
         }
         // Handle gridded data
         else {
            out_pd_u.add_grid_pair(pd_u.f_na[i], pd_u.o_na[i],
                        pd_u.cmn_na[i], pd_u.csd_na[i],
                        pd_u.wgt_na[i]);
            out_pd_v.add_grid_pair(pd_v.f_na[i], pd_v.o_na[i],
                        pd_v.cmn_na[i], pd_v.csd_na[i],
                        pd_v.wgt_na[i]);
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

   bool cmn_flag = set_climo_flag(pd.f_na, pd.cmn_na);
   bool csd_flag = set_climo_flag(pd.f_na, pd.csd_na);
   bool wgt_flag = set_climo_flag(pd.f_na, pd.wgt_na);

   // Loop over the pairs
   for(i=0; i<pd.n_obs; i++) {

      // Check for bad data
      if(is_bad_data(pd.f_na[i])                 ||
         is_bad_data(pd.o_na[i])                 ||
         (cmn_flag && is_bad_data(pd.cmn_na[i])) ||
         (csd_flag && is_bad_data(pd.csd_na[i])) ||
         (wgt_flag && is_bad_data(pd.wgt_na[i]))) continue;

      // Keep pairs for the current bin.
      // check_bins() returns a 1-based bin value.
      if(ta.check_bins(pd.cdf_na[i]) == (i_bin + 1)) {

         // Handle point data
         if(pd.is_point_vx()) {
            out_pd.add_point_pair(pd.sid_sa[i].c_str(), pd.lat_na[i],
                      pd.lon_na[i], pd.x_na[i], pd.y_na[i],
                      pd.vld_ta[i], pd.lvl_na[i], pd.elv_na[i],
                      pd.f_na[i], pd.o_na[i], pd.o_qc_sa[i].c_str(),
                      pd.cmn_na[i], pd.csd_na[i], pd.wgt_na[i]);
         }
         // Handle gridded data
         else {
            out_pd.add_grid_pair(pd.f_na[i], pd.o_na[i],
                      pd.cmn_na[i], pd.csd_na[i], pd.wgt_na[i]);
         }
      }
   } // end for

   mlog << Debug(3)
        << "Using " << out_pd.n_obs << " of " << pd.n_obs
        << " pairs for climatology bin number " << i_bin+1 << ".\n";

   return out_pd;
}

////////////////////////////////////////////////////////////////////////

// Write the point observation in the MET point format for logging
ConcatString point_obs_to_string(float *hdr_arr, const char *hdr_typ_str,
                                 const char *hdr_sid_str, unixtime hdr_ut,
                                 const char *obs_qty, float *obs_arr,
                                 const char *var_name) {
   ConcatString obs_cs, name;

   if((var_name != 0) && (0 < m_strlen(var_name))) name << var_name;
   else                                            name << nint(obs_arr[1]);

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
//
// End miscellaneous functions
//
////////////////////////////////////////////////////////////////////////
