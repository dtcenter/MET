// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "conversions.h"
#include "util_constants.h"
#include "is_bad_data.h"

////////////////////////////////////////////////////////////////////////

double convert_gpm_to_msl(double gpm, double lat) {
   double msl, gphi;

   if(is_eq(gpm, bad_data_double)) {
      msl = bad_data_double;
   }
   else {

      //
      // Compute height as meters above sea level from geopotential
      // height.
      //
      gphi = gop_by_lat(lat);
      msl  = (gpm*const_gop)/gphi;
   }

   return(msl);
}

////////////////////////////////////////////////////////////////////////

double convert_p_t_z_to_prmsl(double p, double t, double z, double lat) {
   double prmsl, a, tbar, gphi;

   if(is_eq(p, bad_data_double) ||
      is_eq(t, bad_data_double) ||
      is_eq(z, bad_data_double)) {
      prmsl = bad_data_double;
   }
   else {

      //
      // Compute pressure reduced to mean sea level using the method
      // used by the U.S. National Weather Service
      //
      gphi  = gop_by_lat(lat);
      tbar  = (t + const_tmb)/2.0;
      a     = (gphi*z)/(const_rd*tbar);
      prmsl = p*exp(a);
   }

   return(prmsl);
}

////////////////////////////////////////////////////////////////////////

double convert_q_to_w(double q) {
   double w;

   if(is_eq(q, bad_data_double) || is_eq(q, 1.0))
      w = bad_data_double;
   else
      w = q/(1.0 - q);

   return(w);
}

////////////////////////////////////////////////////////////////////////

double convert_p_w_to_vp(double p, double w) {
   double vp, p_mb;

   if(is_eq(p, bad_data_double) ||
      is_eq(w, bad_data_double) ||
      is_eq(w, -0.622)) {
      vp = bad_data_double;
   }
   else {
      p_mb = p * mb_per_pa;
      vp   = (p_mb * w)/(w + 0.622);
   }

   return(vp);
}

////////////////////////////////////////////////////////////////////////

double convert_vp_to_dpt(double vp) {
   double dpt, num, den;

   if(is_eq(vp, bad_data_double)) {
      dpt = bad_data_double;
   }
   else {
      num = 1.0;
      den = const_a - (1.0/const_b)*log(vp/const_c);

      if(is_eq(den, 0.0)) dpt = bad_data_double;
      else                dpt = num/den;
   }

   return(dpt);
}

////////////////////////////////////////////////////////////////////////

double convert_t_to_svp(double t) {
   double svp;

   if(is_eq(t, bad_data_double) || is_eq(t, 0.0))
      svp = bad_data_double;
   else
      svp = const_c*(pow(vx_math_e, const_b*(const_a - 1.0/t)));

   return(svp);
}

////////////////////////////////////////////////////////////////////////

double convert_vp_svp_to_rh(double vp, double svp) {
   double rh;

   if(is_eq(vp, bad_data_double) || is_eq(svp, bad_data_double))
      rh = bad_data_double;
   else
      rh = (vp/svp)*100.0;

   return(rh);
}

////////////////////////////////////////////////////////////////////////
//
// Relative humidity derivation method used by the NCEP
// Unified-PostProcessor.  Derivation References:
//   Cuijpers, J. W. M., P. G. Duynkerke, 1993
//      Large Eddy Simulation of Trade Wind Cumulus Clouds
//      J. Atmos. Sci., 50, 3894-3908
//   Murray, F. W., 1967
//      On the Computation of Saturation Vapor Pressure
//      J. Appl. Meteor., 6, 203-204
//
////////////////////////////////////////////////////////////////////////

double convert_p_q_t_to_rh(double p, double q, double t) {
   double rh, qc;

   if(is_eq(p, bad_data_double) ||
      is_eq(q, bad_data_double) ||
      is_eq(t, bad_data_double) )
      rh = bad_data_double;
   else {
      qc = const_pq0/p*exp(const_a2*(t-const_a3)/(t-const_a4));
      rh = q/qc*100.0;
      if(rh > 100.0) rh = 100.0;
   }

   return(rh);
}

////////////////////////////////////////////////////////////////////////

double gop_by_lat(double lat) {
   double gphi;

   gphi = const_gop*(1.0 -
                     2.64e-3 * cos(2.0*lat) +
                     5.90e-10 * cos(2.0*lat) * cos(2.0*lat));

   return(gphi);
}

////////////////////////////////////////////////////////////////////////
