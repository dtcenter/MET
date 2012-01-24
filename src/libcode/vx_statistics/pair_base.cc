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

#include "pair_base.h"

#include "vx_util.h"
#include "vx_grid.h"
#include "vx_data2d.h"
#include "vx_data2d_grib.h"
#include "vx_math.h"
#include "vx_log.h"

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

   msg_typ.clear();
   mask_name.clear();

   mask_dp_ptr = (DataPlane *) 0;  // Not allocated

   interp_mthd = InterpMthd_None;
   interp_dpth = bad_data_int;

   sid_sa.clear();
   lat_na.clear();
   lon_na.clear();
   x_na.clear();
   y_na.clear();
   vld_ta.clear();
   lvl_na.clear();
   elv_na.clear();
   o_na.clear();

   n_obs = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_mask_name(const char *c) {

   mask_name = c;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_mask_dp_ptr(DataPlane *wd_ptr) {

   mask_dp_ptr = wd_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_msg_typ(const char *c) {

   msg_typ = c;

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

void PairBase::set_interp_dpth(int n) {

   interp_dpth = n;

   return;
}

////////////////////////////////////////////////////////////////////////

int PairBase::has_obs_rec(const char *sid,
                          double lat, double lon,
                          double x, double y,
                          double lvl, double elv, int &i_obs) {
   int i, status = 0;

   //
   // Check for an existing record of this observation
   //
   for(i=0, i_obs=-1; i<n_obs; i++) {

      if(strcmp(sid_sa[i], sid) == 0 &&
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

void PairBase::add_obs(const char *sid,
                       double lat, double lon,
                       double x, double y, unixtime ut,
                       double lvl, double elv,
                       double o) {

   sid_sa.add(sid);
   lat_na.add(lat);
   lon_na.add(lon);
   x_na.add(x);
   y_na.add(y);
   vld_ta.add(ut);
   lvl_na.add(lvl);
   elv_na.add(elv);
   o_na.add(o);

   // Increment the number of pairs
   n_obs += 1;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::add_obs(double x, double y, double o) {

   sid_sa.add(na_str);
   lat_na.add(bad_data_double);
   lon_na.add(bad_data_double);
   x_na.add(x);
   y_na.add(y);
   vld_ta.add(bad_data_int);
   lvl_na.add(bad_data_double);
   elv_na.add(bad_data_double);
   o_na.add(o);

   // Increment the number of observations
   n_obs += 1;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_obs(int i_obs, const char *sid,
                       double lat, double lon,
                       double x, double y, unixtime ut,
                       double lvl, double elv,
                       double o) {

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
   vld_ta.set(i_obs, ut);
   lvl_na.set(i_obs, lvl);
   elv_na.set(i_obs, elv);
   o_na.set(i_obs, o);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_obs(int i_obs, double x, double y, double o) {

   if(i_obs < 0 || i_obs >= n_obs) {
      mlog << Error << "\nPairBase::set_obs() -> "
           << "range check error: " << i_obs << " not in (0, "
           << n_obs << ").\n\n"
          ;
      exit(1);
   }

   sid_sa.set(i_obs, na_str);
   lat_na.set(i_obs, bad_data_double);
   lon_na.set(i_obs, bad_data_double);
   x_na.set(i_obs, x);
   y_na.set(i_obs, y);
   vld_ta.set(i_obs, bad_data_int);
   lvl_na.set(i_obs, bad_data_double);
   elv_na.set(i_obs, bad_data_double);
   o_na.set(i_obs, o);

   return;
}

////////////////////////////////////////////////////////////////////////
