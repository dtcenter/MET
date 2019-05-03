// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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
   c_na.clear();

   n_pair = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::assign(const PairDataPoint &pd) {
   int i;

   clear();

   set_mask_name(pd.mask_name);
   set_mask_dp_ptr(pd.mask_dp_ptr);
   set_msg_typ(pd.msg_typ);

   set_interp_mthd(pd.interp_mthd);
   set_interp_dpth(pd.interp_dpth);

   for(i=0; i<pd.n_pair; i++) {
      add_pair(pd.sid_sa[i], pd.lat_na[i], pd.lon_na[i],
               pd.x_na[i], pd.y_na[i], pd.vld_ta[i],
               pd.lvl_na[i], pd.elv_na[i],
               pd.f_na[i], pd.c_na[i], pd.o_na[i]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::add_pair(const char *sid, double lat, double lon,
                             double x, double y, unixtime ut,
                             double lvl, double elv,
                             double f, double c, double o) {

   PairBase::add_obs(sid, lat, lon, x, y, ut, lvl, elv, o);

   f_na.add(f);
   c_na.add(c);

   // Increment the number of pairs
   n_pair += 1;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataPoint::set_pair(int i_pair, const char *sid,
                             double lat, double lon,
                             double x, double y, unixtime ut,
                             double lvl, double elv,
                             double f, double c, double o) {

   if(i_pair < 0 || i_pair >= n_pair) {
      mlog << Error << "\nPairDataPoint::set_pair() -> "
           << "range check error: " << i_pair << " not in (0, "
           << n_pair << ").\n\n"
          ;
      exit(1);
   }

   PairBase::set_obs(i_pair, sid, lat, lon, x, y, ut, lvl, elv, o);

   f_na.set(i_pair, f);
   c_na.set(i_pair, c);

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
   obs_info     = (VarInfoGrib *) 0;
  
   pd           = (PairDataPoint ***) 0;
   rej_typ      = (int ***) 0;
   rej_mask     = (int ***) 0;
   rej_fcst     = (int ***) 0;

   n_msg_typ    = 0;
   n_mask       = 0;
   n_interp     = 0;
   
   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::clear() {
   int i, j, k;

   if(fcst_info) { delete fcst_info; fcst_info = (VarInfo *)     0; }
   if(obs_info)  { delete obs_info;  obs_info  = (VarInfoGrib *) 0; }

   interp_thresh = 0;

   fcst_dpa.clear();
   climo_dpa.clear();
   
   fcst_ut       = (unixtime) 0;
   beg_ut        = (unixtime) 0;
   end_ut        = (unixtime) 0;

   n_try         = 0;
   rej_gc        = 0;
   rej_vld       = 0;
   rej_obs       = 0;
   rej_grd       = 0;
   rej_lvl       = 0;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            pd[i][j][k].clear();
            rej_typ[i][j][k]  = 0;
            rej_mask[i][j][k] = 0;
            rej_fcst[i][j][k] = 0;
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
   set_obs_info(vx_pd.obs_info);

   fcst_ut  = vx_pd.fcst_ut;
   beg_ut   = vx_pd.beg_ut;
   end_ut   = vx_pd.end_ut;

   n_try    = vx_pd.n_try;
   rej_typ  = vx_pd.rej_typ;
   rej_mask = vx_pd.rej_mask;
   rej_fcst = vx_pd.rej_fcst;

   interp_thresh = vx_pd.interp_thresh;

   fcst_dpa  = vx_pd.fcst_dpa;
   climo_dpa = vx_pd.climo_dpa;

   set_pd_size(vx_pd.n_msg_typ, vx_pd.n_mask, vx_pd.n_interp);

   for(i=0; i<vx_pd.n_msg_typ; i++) {
      for(j=0; j<vx_pd.n_mask; j++) {
         for(k=0; k<vx_pd.n_interp; k++) {

            pd[i][j][k]       = vx_pd.pd[i][j][k];
            rej_typ[i][j][k]  = vx_pd.rej_typ[i][j][k];
            rej_mask[i][j][k] = vx_pd.rej_mask[i][j][k];
            rej_fcst[i][j][k] = vx_pd.rej_fcst[i][j][k];
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

void VxPairDataPoint::set_obs_info(VarInfoGrib *info) {

   // Deallocate, if necessary
   if(obs_info) { delete obs_info; obs_info = (VarInfoGrib *) 0; }

   // Perform a deep copy
   obs_info = new VarInfoGrib;
   *obs_info = *info;

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

void VxPairDataPoint::set_climo_dpa(const DataPlaneArray &dpa) {

   climo_dpa = dpa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::set_fcst_ut(const unixtime ut) {

   fcst_ut = ut;

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

   for(i=0; i<n_msg_typ; i++) {
      pd[i]       = new PairDataPoint * [n_mask];
      rej_typ[i]  = new int *           [n_mask];
      rej_mask[i] = new int *           [n_mask];
      rej_fcst[i] = new int *           [n_mask];

      for(j=0; j<n_mask; j++) {
         pd[i][j]       = new PairDataPoint [n_interp];
         rej_typ[i][j]  = new int           [n_interp];
         rej_mask[i][j] = new int           [n_interp];
         rej_fcst[i][j] = new int           [n_interp];

         for(k=0; k<n_interp; k++) {
            rej_typ[i][j][k]  = 0;
            rej_mask[i][j][k] = 0;
            rej_fcst[i][j][k] = 0;
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

void VxPairDataPoint::add_obs(float *hdr_arr,     char *hdr_typ_str,
                              char  *hdr_sid_str, unixtime hdr_ut,
                              float *obs_arr,     Grid &gr) {
   int i, j, k, x, y;
   double hdr_lat, hdr_lon;
   double obs_x, obs_y, obs_lvl, obs_hgt;
   double fcst_v, climo_v, obs_v;
   int fcst_lvl_below, fcst_lvl_above;
   int climo_lvl_below, climo_lvl_above;

   // Increment the number of tries count
   n_try++;
   
   // Check whether the GRIB code for the observation matches
   // the specified code
   if(obs_info->code() != nint(obs_arr[1])) {
      rej_gc++;
      return;
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
      fcst_lvl_below = 0;
      fcst_lvl_above = 0;
   }
   // For multiple forecast fields, find the levels above and below
   // the observation point.
   else {

      // Interpolate using the pressure value
      if(fcst_info->level().type() == LevelType_Pres) {
         find_fcst_vert_lvl(obs_lvl, fcst_lvl_below, fcst_lvl_above);
      }
      // Interpolate using the height value
      else {
         find_fcst_vert_lvl(obs_hgt, fcst_lvl_below, fcst_lvl_above);
      }
   }

   // For a single climatology field
   if(climo_dpa.n_planes() == 1) {
      climo_lvl_below = 0;
      climo_lvl_above = 0;
   }
   // For multiple climatology fields, find the levels above and below
   // the observation point.
   else {

      // Interpolate using the pressure value
      if(fcst_info->level().type() == LevelType_Pres) {
         find_climo_vert_lvl(obs_lvl, climo_lvl_below, climo_lvl_above);
      }
      // Interpolate using the height value
      else {
         find_climo_vert_lvl(obs_hgt, climo_lvl_below, climo_lvl_above);
      }
   }

   // Look through all of the PairDataPoint objects to see if the observation
   // should be added.

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
         // Otherwise, check for the obs Station ID matching the
         // masking SID
         else {
            if(strcmp(hdr_sid_str, pd[i][j][0].mask_name) != 0) {
               inc_count(rej_mask, i, j);
               continue;
            }
         }

         // Compute the interpolated values
         for(k=0; k<n_interp; k++) {

            // Compute the interpolated forecast value using the pressure level
            if(fcst_info->level().type() == LevelType_Pres) {
               fcst_v = compute_fcst_interp(obs_x, obs_y, k,
                           obs_lvl, fcst_lvl_below, fcst_lvl_above);
            }
            // Interpolate using the height value
            else {
               fcst_v = compute_fcst_interp(obs_x, obs_y, k,
                           obs_hgt, fcst_lvl_below, fcst_lvl_above);
            }

            if(is_bad_data(fcst_v)) {
               inc_count(rej_fcst, i, j, k);
               continue;
            }

            // Compute the interpolated climatological value using the pressure level
            if(fcst_info->level().type() == LevelType_Pres) {
               climo_v = compute_climo_interp(obs_x, obs_y, k,
                            obs_lvl, climo_lvl_below, climo_lvl_above);
            }
            // Interpolate using the height value
            else {
               climo_v = compute_climo_interp(obs_x, obs_y, k,
                            obs_hgt, climo_lvl_below, climo_lvl_above);
            }

            // Add the forecast, climatological, and observation data
            pd[i][j][k].add_pair(hdr_sid_str, hdr_lat, hdr_lon,
                                 obs_x, obs_y, hdr_ut, obs_lvl, obs_hgt,
                                 fcst_v, climo_v, obs_v);

         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::find_fcst_vert_lvl(double obs_lvl,
                                         int &i_below, int &i_above) {
   int i;
   double dist, dist_below, dist_above;

   // Check for no data
   if(fcst_dpa.n_planes() == 0) {
      i_below = i_above = bad_data_int;
      return;
   }

   // Find the closest pressure levels above and below the observation
   dist_below = dist_above = 1.0e30;
   for(i=0; i<fcst_dpa.n_planes(); i++) {

      dist = obs_lvl - fcst_dpa.lower(i);

      // Check for the closest level below.
      // Levels below contain higher values of pressure.
      if(dist <= 0 && abs((long double) dist) < dist_below) {
         dist_below = abs((long double) dist);
         i_below = i;
      }

      // Check for the closest level above.
      // Levels above contain lower values of pressure.
      if(dist >= 0 && abs((long double) dist) < dist_above) {
         dist_above = abs((long double) dist);
         i_above = i;
      }
   }

   // Check if the observation is above the forecast range
   if(is_eq(dist_below, 1.0e30) && !is_eq(dist_above, 1.0e30)) {

      // Set the index below to the index above and perform
      // no vertical interpolation
      i_below = i_above;
   }
   // Check if the observation is below the forecast range
   else if(!is_eq(dist_below, 1.0e30) && is_eq(dist_above, 1.0e30)) {

      // Set the index above to the index below and perform
      // no vertical interpolation
      i_above = i_below;
   }
   // Check if an error occurred
   else if(is_eq(dist_below, 1.0e30) && is_eq(dist_above, 1.0e30)) {

      mlog << Error << "\nVxPairDataPoint::find_fcst_vert_lvl() -> "
           << "could not find a level above and/or below the "
           << "observation level of " << obs_lvl << ".\n\n"
          ;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataPoint::find_climo_vert_lvl(double obs_lvl,
                                         int &i_below, int &i_above) {
   int i;
   double dist, dist_below, dist_above;

   // Check for no data
   if(climo_dpa.n_planes() == 0) {
      i_below = i_above = bad_data_int;
      return;
   }

   // Find the closest pressure levels above and below the observation
   dist_below = dist_above = 1.0e30;
   for(i=0; i<climo_dpa.n_planes(); i++) {

      dist = obs_lvl - climo_dpa.lower(i);

      // Check for the closest level below.
      // Levels below contain higher values of pressure.
      if(dist <= 0 && abs((long double) dist) < dist_below) {
         dist_below = abs((long double) dist);
         i_below = i;
      }

      // Check for the closest level above.
      // Levels above contain lower values of pressure.
      if(dist >= 0 && abs((long double) dist) < dist_above) {
         dist_above = abs((long double) dist);
         i_above = i;
      }
   }

   // Check if the observation is above the forecast range
   if(is_eq(dist_below, 1.0e30) && !is_eq(dist_above, 1.0e30)) {

      // Set the index below to the index above and perform
      // no vertical interpolation
      i_below = i_above;
   }
   // Check if the observation is below the forecast range
   else if(!is_eq(dist_below, 1.0e30) && is_eq(dist_above, 1.0e30)) {

      // Set the index above to the index below and perform
      // no vertical interpolation
      i_above = i_below;
   }
   // Check if an error occurred
   else if(is_eq(dist_below, 1.0e30) && is_eq(dist_above, 1.0e30)) {

      mlog << Error << "\nVxPairDataPoint::find_climo_vert_lvl() -> "
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

            n += pd[i][j][k].n_pair;
         }
      }
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////

double VxPairDataPoint::compute_fcst_interp(double obs_x, double obs_y,
                                            int i_interp, double to_lvl,
                                            int i_below, int i_above) {
   double v, v_below, v_above, t;

   // Check for no data
   if(fcst_dpa.n_planes() == 0) return(bad_data_double);

   v_below = compute_horz_interp(fcst_dpa[i_below], obs_x, obs_y,
                                 pd[0][0][i_interp].interp_mthd,
                                 pd[0][0][i_interp].interp_dpth,
                                 interp_thresh);

   if(i_below == i_above) {
      v = v_below;
   }
   else {
      v_above = compute_horz_interp(fcst_dpa[i_above], obs_x, obs_y,
                                    pd[0][0][i_interp].interp_mthd,
                                    pd[0][0][i_interp].interp_dpth,
                                    interp_thresh);

      // If verifying specific humidity, do vertical interpolation in
      // the natural log of q
      if(fcst_info->is_specific_humidity() &&
         obs_info->is_specific_humidity()) {
         t = compute_vert_pinterp(log(v_below), fcst_dpa.lower(i_below),
                                  log(v_above), fcst_dpa.lower(i_above),
                                  to_lvl);
         v = exp(t);
      }
      // Vertically interpolate to the observation pressure level
      else if(fcst_info->level().type() == LevelType_Pres) {
         v = compute_vert_pinterp(v_below, fcst_dpa.lower(i_below),
                                  v_above, fcst_dpa.lower(i_above),
                                  to_lvl);
      }
      // Vertically interpolate to the observation height
      else {
         v = compute_vert_zinterp(v_below, fcst_dpa.lower(i_below),
                                  v_above, fcst_dpa.lower(i_above),
                                  to_lvl);
      }
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////

double VxPairDataPoint::compute_climo_interp(double obs_x, double obs_y,
                                            int i_interp, double to_lvl,
                                            int i_below, int i_above) {
   double v, v_below, v_above, t;

   // Check for no data
   if(climo_dpa.n_planes() == 0) return(bad_data_double);

   v_below = compute_horz_interp(climo_dpa[i_below], obs_x, obs_y,
                                 pd[0][0][i_interp].interp_mthd,
                                 pd[0][0][i_interp].interp_dpth,
                                 interp_thresh);

   if(i_below == i_above) {
      v = v_below;
   }
   else {
      v_above = compute_horz_interp(climo_dpa[i_above], obs_x, obs_y,
                                    pd[0][0][i_interp].interp_mthd,
                                    pd[0][0][i_interp].interp_dpth,
                                    interp_thresh);

      // If verifying specific humidity, do vertical interpolation in
      // the natural log of q
      if(fcst_info->is_specific_humidity() &&
         obs_info->is_specific_humidity()) {
         t = compute_vert_pinterp(log(v_below), climo_dpa.lower(i_below),
                                  log(v_above), climo_dpa.lower(i_above),
                                  to_lvl);
         v = exp(t);
      }
      // Vertically interpolate to the observation pressure level
      else if(fcst_info->level().type() == LevelType_Pres) {
         v = compute_vert_pinterp(v_below, climo_dpa.lower(i_below),
                                  v_above, climo_dpa.lower(i_above),
                                  to_lvl);
      }
      // Vertically interpolate to the observation height
      else {
         v = compute_vert_zinterp(v_below, climo_dpa.lower(i_below),
                                  v_above, climo_dpa.lower(i_above),
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
