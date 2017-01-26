// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
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

#include "pair_data_point.h"

#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_data2d_grib.h"
#include "vx_util.h"
#include "vx_grid.h"
#include "vx_math.h"
#include "vx_log.h"

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

   if(this == &pd) return(*this);

   assign(pd);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::clear() {

   PairBase::clear();

   f_na.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::extend(int n) {

   PairBase::extend(n);

   f_na.extend(n);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::assign(const PairDataPoint &pd) {
   int i;

   clear();

   // Allocate memory for output pairs
   extend(pd.n_obs);

   set_mask_name(pd.mask_name);
   set_mask_dp_ptr(pd.mask_dp_ptr);
   set_msg_typ(pd.msg_typ);

   set_interp_mthd(pd.interp_mthd);
   set_interp_dpth(pd.interp_dpth);

   // Handle point data
   if(pd.sid_sa.n_elements() == pd.n_obs) {

      for(i=0; i<pd.n_obs; i++) {
         add_pair(pd.sid_sa[i], pd.lat_na[i], pd.lon_na[i],
                  pd.x_na[i], pd.y_na[i], pd.vld_ta[i],
                  pd.lvl_na[i], pd.elv_na[i],
                  pd.f_na[i], pd.o_na[i], pd.o_qc_sa[i],
                  pd.cmn_na[i], pd.csd_na[i], pd.wgt_na[i]);
      }
   }
   // Handle gridded data
   else {
      add_pair(pd.f_na, pd.o_na, pd.cmn_na, pd.wgt_na);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool PairDataPoint::add_pair(const char *sid, double lat, double lon,
                             double x, double y, unixtime ut,
                             double lvl, double elv,
                             double f, double o, const char *qc,
                             double cmn, double csd, double wgt) {

   if(!PairBase::add_obs(sid, lat, lon, x, y, ut, lvl, elv,
                         o, qc, cmn, csd, wgt) ) return(false);

   f_na.add(f);

   return(true);
}

////////////////////////////////////////////////////////////////////////

bool PairDataPoint::add_pair(double f, double o, double c, double w) {

   f_na.add(f);
   o_na.add(o);
   cmn_na.add(c);
   wgt_na.add(w);
   n_obs++;

   return(true);
}

////////////////////////////////////////////////////////////////////////

bool PairDataPoint::add_pair(const NumArray f_in, const NumArray o_in,
                             const NumArray c_in, const NumArray w_in) {

   // Check for constant length
   if(o_in.n_elements() != f_in.n_elements() ||
      o_in.n_elements() != c_in.n_elements() ||
      o_in.n_elements() != w_in.n_elements()) {
      mlog << Error << "\nPairDataPoint::add_pair() -> "
           << "arrays must all have the same length!\n\n";
      exit(1);
   }

   f_na.add(f_in);
   o_na.add(o_in);
   cmn_na.add(c_in);
   wgt_na.add(w_in);

   // Increment the number of pairs
   n_obs += o_in.n_elements();

   return(true);
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::set_pair(int i_obs, const char *sid,
                             double lat, double lon,
                             double x, double y, unixtime ut,
                             double lvl, double elv,
                             double f, double o, const char *qc,
                             double cmn, double csd, double wgt) {

   if(i_obs < 0 || i_obs >= n_obs) {
      mlog << Error << "\nPairDataPoint::set_pair() -> "
           << "range check error: " << i_obs << " not in (0, "
           << n_obs << ").\n\n"
          ;
      exit(1);
   }

   PairBase::set_obs(i_obs, sid, lat, lon, x, y, ut, lvl, elv,
                     o, qc, cmn, csd, wgt);

   f_na.set(i_obs, f);

   return;
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

   if(this == &vx_pd) return(*this);

   assign(vx_pd);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::init_from_scratch() {

   fcst_info    = (VarInfo *) 0;
   climo_info   = (VarInfo *) 0;
   obs_info     = (VarInfoGrib *) 0;

   pd           = (PairDataPoint ***) 0;
   rej_typ      = (int ***) 0;
   rej_mask     = (int ***) 0;
   rej_fcst     = (int ***) 0;
   rej_dup      = (int ***) 0;

   n_msg_typ    = 0;
   n_mask       = 0;
   n_interp     = 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::clear() {
   int i, j, k;

   if(fcst_info)  { delete fcst_info;  fcst_info  = (VarInfo *)     0; }
   if(climo_info) { delete climo_info; climo_info = (VarInfo *)     0; }
   if(obs_info)   { delete obs_info;   obs_info   = (VarInfoGrib *) 0; }

   desc.clear();

   interp_thresh = 0;

   fcst_dpa.clear();
   climo_mn_dpa.clear();
   climo_sd_dpa.clear();
   sid_exc_filt.clear();
   obs_qty_filt.clear();

   fcst_ut       = (unixtime) 0;
   beg_ut        = (unixtime) 0;
   end_ut        = (unixtime) 0;

   n_try         = 0;
   rej_sid_exc   = 0;
   rej_gc        = 0;
   rej_vld       = 0;
   rej_obs       = 0;
   rej_grd       = 0;
   rej_lvl       = 0;
   rej_qty       = 0;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            pd[i][j][k].clear();
            rej_typ[i][j][k]  = 0;
            rej_mask[i][j][k] = 0;
            rej_fcst[i][j][k] = 0;
            rej_dup[i][j][k]  = 0;
         }
      }
   }

   n_msg_typ     = 0;
   n_mask        = 0;
   n_interp      = 0;

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

   sid_exc_filt = vx_pd.sid_exc_filt;
   obs_qty_filt = vx_pd.obs_qty_filt;

   fcst_ut  = vx_pd.fcst_ut;
   beg_ut   = vx_pd.beg_ut;
   end_ut   = vx_pd.end_ut;

   n_try       = vx_pd.n_try;
   rej_typ     = vx_pd.rej_typ;
   rej_mask    = vx_pd.rej_mask;
   rej_fcst    = vx_pd.rej_fcst;
   rej_dup     = vx_pd.rej_dup;

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
            rej_dup[i][j][k]  = vx_pd.rej_dup[i][j][k];
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_fcst_info(VarInfo *info) {
   VarInfoFactory f;

   // Deallocate, if necessary
   if(fcst_info) { delete fcst_info; fcst_info = (VarInfo *) 0; }

   // Perform a deep copy
   fcst_info = f.new_var_info(info->file_type());
   *fcst_info = *info;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_climo_info(VarInfo *info) {
   VarInfoFactory f;

   // Deallocate, if necessary
   if(climo_info) { delete climo_info; climo_info = (VarInfo *) 0; }

   // Perform a deep copy
   climo_info = f.new_var_info(info->file_type());
   *climo_info = *info;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_obs_info(VarInfoGrib *info) {

   // Deallocate, if necessary
   if(obs_info) { delete obs_info; obs_info = (VarInfoGrib *) 0; }

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

void VxPairDataPoint::set_sid_exc_filt(const StringArray se) {

   sid_exc_filt = se;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_obs_qty_filt(const StringArray q) {

   obs_qty_filt = q;

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
   rej_dup  = new int **           [n_msg_typ];

   for(i=0; i<n_msg_typ; i++) {
      pd[i]       = new PairDataPoint * [n_mask];
      rej_typ[i]  = new int *           [n_mask];
      rej_mask[i] = new int *           [n_mask];
      rej_fcst[i] = new int *           [n_mask];
      rej_dup[i]  = new int *           [n_mask];

      for(j=0; j<n_mask; j++) {
         pd[i][j]       = new PairDataPoint [n_interp];
         rej_typ[i][j]  = new int           [n_interp];
         rej_mask[i][j] = new int           [n_interp];
         rej_fcst[i][j] = new int           [n_interp];
         rej_dup[i][j]  = new int           [n_interp];

         for(k=0; k<n_interp; k++) {
            rej_typ[i][j][k]  = 0;
            rej_mask[i][j][k] = 0;
            rej_fcst[i][j][k] = 0;
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

void VxPairDataPoint::set_mask_dp(int i_mask, const char *name,
                                  DataPlane *dp_ptr) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i][i_mask][j].set_mask_name(name);
         pd[i][i_mask][j].set_mask_dp_ptr(dp_ptr);
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

void VxPairDataPoint::set_interp(int i_interp,
                                 const char *interp_mthd_str,
                                 int wdth) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         pd[i][j][i_interp].set_interp_mthd(interp_mthd_str);
         pd[i][j][i_interp].set_interp_dpth(wdth);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_interp(int i_interp, InterpMthd mthd,
                                 int wdth) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         pd[i][j][i_interp].set_interp_mthd(mthd);
         pd[i][j][i_interp].set_interp_dpth(wdth);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::add_obs(float *hdr_arr, const char *hdr_typ_str,
                              const char *hdr_sid_str, unixtime hdr_ut,
                              const char *obs_qty, float *obs_arr,
                              Grid &gr, const DataPlane *wgt_dp) {
   int i, j, k, x, y;
   double hdr_lat, hdr_lon;
   double obs_x, obs_y, obs_lvl, obs_hgt, to_lvl;
   double fcst_v, cmn_v, csd_v, obs_v, wgt_v;
   int f_lvl_blw, f_lvl_abv;
   int cmn_lvl_blw, cmn_lvl_abv;
   int csd_lvl_blw, csd_lvl_abv;

   // Increment the number of tries count
   n_try++;

   // Check the station ID exclusion list
   if(sid_exc_filt.n_elements() && sid_exc_filt.has(hdr_sid_str)) {
      rej_sid_exc++;
      return;
   }

   // Check whether the GRIB code for the observation matches
   // the specified code
   if(obs_info->code() != nint(obs_arr[1])) {
      rej_gc++;
      return;
   }

   // Check if the observation quality flag is included in the list
   if( obs_qty_filt.n_elements() && strcmp(obs_qty, "") ) {
      bool qty_match = false;
      for(i=0; i < obs_qty_filt.n_elements() && !qty_match; i++)
         if( 0 == strcmp(obs_qty, obs_qty_filt[i]) ) qty_match = true;

      if( !qty_match ){
         rej_qty++;
         return;
      }
   }

   // Check whether the observation time falls within the valid time
   // window
   if(hdr_ut < beg_ut || hdr_ut > end_ut) {
      rej_vld++;
      return;
   }

   hdr_lat = hdr_arr[0];
   hdr_lon = hdr_arr[1];

   obs_lvl = obs_arr[2];
   obs_hgt = obs_arr[3];
   obs_v   = obs_arr[4];

   // Check whether the observation value contains valid data
   if(is_bad_data(obs_v)) {
      rej_obs++;
      return;
   }

   // Convert the lat/lon value to x/y
   gr.latlon_to_xy(hdr_lat, -1.0*hdr_lon, obs_x, obs_y);
   x = nint(obs_x);
   y = nint(obs_y);

   // Check if the observation's lat/lon is on the grid
   if(x < 0 || x >= gr.nx() ||
      y < 0 || y >= gr.ny()) {
      rej_grd++;
      return;
   }

   // For pressure levels, check if the observation pressure level
   // falls in the requsted range.
   if(obs_info->level().type() == LevelType_Pres) {

      if(obs_lvl < obs_info->level().lower() ||
         obs_lvl > obs_info->level().upper()) {
         rej_lvl++;
         return;
      }
   }
   // For accumulations, check if the observation accumulation interval
   // matches the requested interval.
   else if(obs_info->level().type() == LevelType_Accum) {

      if(obs_lvl < obs_info->level().lower() ||
         obs_lvl > obs_info->level().upper()) {
         rej_lvl++;
         return;
      }
   }
   // For vertical levels, check for a surface message type or if the
   // observation height falls within the requested range.
   else if(obs_info->level().type() == LevelType_Vert) {

      if(strstr(onlysf_msg_typ_str, hdr_typ_str) == NULL &&
         (obs_hgt < obs_info->level().lower() ||
          obs_hgt > obs_info->level().upper())) {
         rej_lvl++;
         return;
      }
   }
   // For all other level types (RecNumber, NoLevel), check
   // for a surface message type or if the observation height falls
   // within the requested range.
   else {

      if(strstr(onlysf_msg_typ_str, hdr_typ_str) == NULL &&
         (obs_hgt < obs_info->level().lower() ||
          obs_hgt > obs_info->level().upper())) {
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
      strstr(onlysf_msg_typ_str, hdr_typ_str) != NULL) {
      obs_lvl = bad_data_double;
   }

   // Look through all of the PairDataPoint objects to see if the
   // observation should be added.

   // Check the message types
   for(i=0; i<n_msg_typ; i++) {

      //
      // Check for a matching PrepBufr message type
      //

      // Handle ANYAIR
      if(strcmp(anyair_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(anyair_msg_typ_str, hdr_typ_str) == NULL ) {
            inc_count(rej_typ, i);
            continue;
         }
      }

      // Handle ANYSFC
      else if(strcmp(anysfc_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(anysfc_msg_typ_str, hdr_typ_str) == NULL) {
            inc_count(rej_typ, i);
            continue;
         }
      }

      // Handle ONLYSF
      else if(strcmp(onlysf_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(onlysf_msg_typ_str, hdr_typ_str) == NULL) {
            inc_count(rej_typ, i);
            continue;
         }
      }

      // Handle all other message types
      else {
         if(strcmp(hdr_typ_str, pd[i][0][0].msg_typ) != 0) {
            inc_count(rej_typ, i);
            continue;
         }
      }

      // Check the masking areas and points
      for(j=0; j<n_mask; j++) {

         // Check for the obs falling within the masking region
         if(pd[i][j][0].mask_dp_ptr != (DataPlane *) 0) {
            if(!pd[i][j][0].mask_dp_ptr->s_is_on(x, y)) {
               inc_count(rej_mask, i, j);
               continue;
            }
         }
         // Otherwise, check for the obs Station ID's presence in the
         // masking SID list
         else if(pd[i][j][0].mask_sid_ptr != (StringArray *) 0) {
            if(!pd[i][j][0].mask_sid_ptr->has(hdr_sid_str)) {
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
            fcst_v = compute_interp(fcst_dpa, obs_x, obs_y, obs_v, k,
                        to_lvl, f_lvl_blw, f_lvl_abv);

            if(is_bad_data(fcst_v)) {
               inc_count(rej_fcst, i, j, k);
               continue;
            }

            // Compute the interpolated climatology mean
            cmn_v = compute_interp(climo_mn_dpa, obs_x, obs_y, obs_v, k,
                      to_lvl, cmn_lvl_blw, cmn_lvl_abv);

            // Check for valid interpolation options
            if(climo_sd_dpa.n_planes() > 0 &&
               (pd[0][0][k].interp_mthd == InterpMthd_Min    ||
                pd[0][0][k].interp_mthd == InterpMthd_Max    ||
                pd[0][0][k].interp_mthd == InterpMthd_Median ||
                pd[0][0][k].interp_mthd == InterpMthd_Best)) {
               mlog << Warning << "\nVxPairDataPoint::add_obs() -> "
                    << "applying the "
                    << interpmthd_to_string(pd[0][0][k].interp_mthd)
                    << " interpolation method to climatological spread "
                    << "may cause unexpected results.\n\n";
            }

            // Compute the interpolated climatology standard deviation
            csd_v = compute_interp(climo_sd_dpa, obs_x, obs_y, obs_v, k,
                      to_lvl, csd_lvl_blw, csd_lvl_abv);

            // Compute weight for current point
            wgt_v = ( wgt_dp == (DataPlane *) 0 ?
                      default_grid_weight : wgt_dp->get(x, y) );

            // Add the forecast, climatological, and observation data
            // Weight is from the nearest grid point
            if(!pd[i][j][k].add_pair(hdr_sid_str, hdr_lat, hdr_lon,
                  obs_x, obs_y, hdr_ut, obs_lvl, obs_hgt,
                  fcst_v, obs_v, obs_qty, cmn_v, csd_v, wgt_v)) {
               inc_count(rej_dup, i, j, k);
            }

         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::find_vert_lvl(const DataPlaneArray &dpa, double obs_lvl,
                                    int &i_blw, int &i_abv) {
   int i;
   double dist, dist_blw, dist_abv;

   // Check for no data
   if(dpa.n_planes() == 0) {
      i_blw = i_abv = bad_data_int;
      return;
   }

   // Find the closest pressure levels above and below the observation
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

      mlog << Error << "\nVxPairDataPoint::find_vert_lvl() -> "
           << "could not find a level above and/or below the "
           << "observation level of " << obs_lvl << ".\n\n"
          ;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

int VxPairDataPoint::get_n_pair() {
   int n, i, j, k;

   for(i=0, n=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            n += pd[i][j][k].n_obs;
         }
      }
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_duplicate_flag(DuplicateType duplicate_flag) {

   for(int i=0; i < n_msg_typ; i++){
      for(int j=0; j < n_mask; j++){
         for(int k=0; k < n_interp; k++){
            pd[i][j][k].set_check_unique(duplicate_flag == DuplicateType_Unique);
            pd[i][j][k].set_check_single(duplicate_flag == DuplicateType_Single);
         }
      }
   }

}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::print_duplicate_report() {

   for(int i=0; i < n_msg_typ; i++){
      for(int j=0; j < n_mask; j++){
         for(int k=0; k < n_interp; k++){
            pd[i][j][k].print_duplicate_report();
         }
      }
   }

}

////////////////////////////////////////////////////////////////////////

double VxPairDataPoint::compute_interp(const DataPlaneArray &dpa,
                                       double obs_x, double obs_y,
                                       double obs_v, int i_interp,
                                       double to_lvl,
                                       int i_blw, int i_abv) {
   double v, v_blw, v_abv, t;

   // Check for no data
   if(dpa.n_planes() == 0) return(bad_data_double);

   v_blw = compute_horz_interp(dpa[i_blw], obs_x, obs_y, obs_v,
                               pd[0][0][i_interp].interp_mthd,
                               pd[0][0][i_interp].interp_dpth,
                               interp_thresh);

   if(i_blw == i_abv) {
      v = v_blw;
   }
   else {
      v_abv = compute_horz_interp(dpa[i_abv], obs_x, obs_y, obs_v,
                                  pd[0][0][i_interp].interp_mthd,
                                  pd[0][0][i_interp].interp_dpth,
                                  interp_thresh);

      // Check for bad data prior to vertical interpolation
      if(is_bad_data(v_blw) || is_bad_data(v_abv)) {
         return(bad_data_double);
      }

      // If verifying specific humidity, do vertical interpolation in
      // the natural log of q
      if(fcst_info->is_specific_humidity() &&
         obs_info->is_specific_humidity()) {
         t = compute_vert_pinterp(log(v_blw), dpa.lower(i_blw),
                                  log(v_abv), dpa.lower(i_abv),
                                  to_lvl);
         v = exp(t);
      }
      // Vertically interpolate to the observation pressure level
      else if(fcst_info->level().type() == LevelType_Pres) {
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

PairDataPoint subset_pairs(const PairDataPoint &pd,
                           const SingleThresh &ft, const SingleThresh &ot,
                           const SetLogic type) {

   // Check for no work to be done
   if(ft.get_type() == thresh_na && ot.get_type() == thresh_na) {
      return(pd);
   }

   int i;
   PairDataPoint out_pd;

   // Allocate memory for output pairs
   out_pd.extend(pd.n_obs);

   bool cflag = set_climo_flag(pd.f_na, pd.cmn_na);
   bool wflag = set_climo_flag(pd.f_na, pd.wgt_na);

   // Loop over the pairs
   for(i=0; i<pd.n_obs; i++) {

      // Check for bad data
      if(is_bad_data(pd.f_na[i]) ||
         is_bad_data(pd.o_na[i]) ||
         (cflag && is_bad_data(pd.cmn_na[i])) ||
         (wflag && is_bad_data(pd.wgt_na[i]))) continue;

      // Keep pairs which meet the threshold criteria
      if(check_fo_thresh(pd.f_na[i], ft, pd.o_na[i], ot, type)) {

         // Handle point data
         if(pd.sid_sa.n_elements() == pd.n_obs) {
            out_pd.add_pair(pd.sid_sa[i], pd.lat_na[i], pd.lon_na[i],
                            pd.x_na[i], pd.y_na[i], pd.vld_ta[i],
                            pd.lvl_na[i], pd.elv_na[i],
                            pd.f_na[i], pd.o_na[i], pd.o_qc_sa[i],
                            pd.cmn_na[i], pd.csd_na[i], pd.wgt_na[i]);
         }
         // Handle gridded data
         else {
            out_pd.add_pair(pd.f_na[i], pd.o_na[i],
                            pd.cmn_na[i], pd.wgt_na[i]);
         }
      }
   } // end for

   mlog << Debug(3)
        << "Using " << out_pd.n_obs << " of " << pd.n_obs
        << " pairs for forecast filtering threshold " << ft.get_str()
        << ", observation filtering threshold " << ot.get_str()
        << ", and field logic " << setlogic_to_string(type) << ".\n";

   return(out_pd);
}

////////////////////////////////////////////////////////////////////////

bool set_climo_flag(const NumArray &f_na, const NumArray &c_na) {

   // The climo values must have non-zero, consistent length and
   // cannot all be bad data
   if(c_na.n_elements() != f_na.n_elements() ||
      c_na.n_elements() < 1 ||
      is_bad_data(c_na.max())) return(false);

   return(true);
}

////////////////////////////////////////////////////////////////////////
//
// End miscellaneous functions
//
////////////////////////////////////////////////////////////////////////
