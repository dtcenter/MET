// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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

#include "vx_met_util/conversions.h"
#include "vx_met_util/constants.h"
#include "vx_wrfdata/vx_wrfdata.h"
#include "vx_math/vx_math.h"

////////////////////////////////////////////////////////////////////////
//
// Check the range of probability values and make sure it's either
// [0, 1] or [0, 100].  If it's [0, 100], rescale to [0, 1].
//
////////////////////////////////////////////////////////////////////////

void rescale_probability(WrfData &wd) {
   double v, min_v, max_v;
   int x, y;
   WrfData tmp_wd;

   // Get the min/max values in the field
   min_v = wd.int_to_double(0);
   max_v = wd.int_to_double(wrfdata_int_data_max);

   // Check for a valid range of values
   if(min_v < 0.0 || max_v > 100.0) {
      cerr << "\n\nERROR: rescale_probability() -> "
           << "invalid range of data for a probability field: ["
           << min_v << ", " << max_v << "].\n\n" << flush;
      exit(1);

   }

   // Check to see if the data needs to be rescaled from
   // [0, 100] to [0, 1]
   if(max_v > 1.0) {

      // Initialize
      tmp_wd = wd;

      // Set the m and b values
      tmp_wd.set_m(1.0/(double) wrfdata_int_data_max);
      tmp_wd.set_b(0.0);

      // Divide each of the values by 100
      for(x=0; x<wd.get_nx(); x++) {
         for(y=0; y<wd.get_ny(); y++) {

            // Check for bad data
            if(wd.is_bad_xy(x, y)) {
               tmp_wd.put_xy_int(bad_data_flag, x, y);
            }
            else {
               v = wd.get_xy_double(x, y);
               tmp_wd.put_xy_double(v/100.0, x, y);
            }
         } // end for y
      } // end for x

      wd = tmp_wd;
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Rescale the longitude value provided to be in the range from
// -180 to 180
//
////////////////////////////////////////////////////////////////////////

double rescale_lon(double lon) {
   double new_lon;

   // Check for bad data
   if(is_bad_data(lon))
      return(bad_data_double);

   new_lon = lon;
   if(new_lon < -180.0) {
      while(new_lon < -180.0) new_lon += 360.0;
   }
   if(new_lon > 180.0) {
      while(new_lon >  180.0) new_lon -= 360.0;
   }

   return(new_lon);
}

////////////////////////////////////////////////////////////////////////
//
// Rescale the degrees provided to be in the range specified
//
////////////////////////////////////////////////////////////////////////

double rescale_deg(double deg, double lower, double upper) {
   double new_deg;

   // Check for bad data
   if(is_bad_data(deg))
      return(bad_data_double);

   if(!is_eq(abs(upper - lower), 360.0) || lower > upper) {
      cerr << "\n\nERROR: rescale_deg() -> "
           << "invalid upper and lower limits supplied.\n\n"
           << flush;
      exit(1);
   }

   new_deg = deg;
   if(new_deg < lower) {
      while(new_deg < lower) new_deg += 360.0;
   }
   if(new_deg > upper) {
      while(new_deg > upper) new_deg -= 360.0;
   }

   return(new_deg);
}

////////////////////////////////////////////////////////////////////////
//
// Compute the number of degrees between two angles
//
////////////////////////////////////////////////////////////////////////

double angle_between(double a, double b) {
   double d, d1, d2;

   // Check for bad data
   if(is_bad_data(a) || is_bad_data(b))
      return(bad_data_double);

   // Compute the difference between the angles, and rescale them from
   // 0 to 360
   d1 = rescale_deg(a - b, 0.0, 360.0);
   d2 = rescale_deg(b - a, 0.0, 360.0);

   if(min(d1, d2) < 90.0) d = min(d1, d2);
   else                   d = max(d1, d2) - 180.0;

   return(d);
}

////////////////////////////////////////////////////////////////////////
//
// Compute the directed angle difference between two vectors
//
////////////////////////////////////////////////////////////////////////

double angle_difference(double uf, double vf, double uo, double vo) {
   double d, a, b;

   // Check for bad data
   if(is_bad_data(uf) || is_bad_data(vf) ||
      is_bad_data(uo) || is_bad_data(vo))
      return(bad_data_double);

   // Normalize the vectors to unit length
   convert_u_v_to_unit(uf, vf);
   convert_u_v_to_unit(uo, vo);

   // Compute sums
   a = vf*uo - uf*vo;
   b = uf*uo + vf*vo;

   // Only compute the angle between if both terms are non-zero
   if(is_eq(a, 0.0) && is_eq(b, 0.0)) d = bad_data_double;
   else                               d = atan2d(a, b);

   return(d);
}

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
      svp = const_c*(pow(e, const_b*(const_a - 1.0/t)));

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

double convert_u_v_to_wdir(double u, double v) {
   double wdir;

   if(is_eq(u, bad_data_double) || is_eq(v, bad_data_double))
      wdir = bad_data_double;
   else if(is_eq(u, 0.0) && is_eq(v, 0.0))
      wdir = bad_data_double;
   else
      wdir = rescale_deg(atan2d(-1.0*u, -1.0*v), 0.0, 360.0);

   return(wdir);
}

////////////////////////////////////////////////////////////////////////

double convert_u_v_to_wind(double u, double v) {
   double wind;

   if(is_eq(u, bad_data_double) || is_eq(v, bad_data_double))
      wind = bad_data_double;
   else
      wind = sqrt(u*u + v*v);

   return(wind);
}

////////////////////////////////////////////////////////////////////////

void convert_u_v_to_unit(double u,       double v,
                         double &u_unit, double &v_unit) {
   double l = sqrt(u*u + v*v);

   if(is_eq(u, bad_data_double) ||
      is_eq(v, bad_data_double) ||
      is_eq(l, 0.0)) {
      u_unit = bad_data_double;
      v_unit = bad_data_double;
   }
   else {
      u_unit = u/l;
      v_unit = v/l;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void convert_u_v_to_unit(double &u, double &v) {
   double u_tmp, v_tmp;

   convert_u_v_to_unit(u, v, u_tmp, v_tmp);
   u = u_tmp;
   v = v_tmp;

   return;
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
