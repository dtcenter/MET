// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <iostream>
#include <limits>

#include <math.h>
#include <string.h>
#include <unistd.h>

#include "config_util.h"
#include "interp_util.h"
#include "GridTemplate.h"
#include "RectangularTemplate.h"

#include "vx_math.h"
#include "vx_log.h"
#include "enum_as_int.hpp"

using namespace std;


////////////////////////////////////////////////////////////////////////
//
// Code for struct SurfaceInfo
//
////////////////////////////////////////////////////////////////////////

void SurfaceInfo::clear() {
   land_ptr = (MaskPlane *) nullptr;
   topo_ptr = (DataPlane *) nullptr;
   topo_use_obs_thresh.clear();
   topo_interp_fcst_thresh.clear();
}

////////////////////////////////////////////////////////////////////////

SurfaceInfo::SurfaceInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////
//
// Utility functions for horizontal interpolation on a DataPlane
//
////////////////////////////////////////////////////////////////////////

NumArray interp_points(const DataPlane &dp, const GridTemplate &gt, double x_dbl, double y_dbl) {
   int x = nint(x_dbl);
   int y = nint(y_dbl);

   // if width is even, push center to lower left instead of nearest
   if((gt.getWidth() % 2) == 0) {
      x = floor(x_dbl);
      y = floor(y_dbl);
   }

   return interp_points(dp, gt, x, y);
}

////////////////////////////////////////////////////////////////////////

NumArray interp_points(const DataPlane &dp, const GridTemplate &gt, int x, int y) {
   NumArray points;

   // Search the neighborhood, storing any points off the grid as bad data
   GridPoint *gp = nullptr;
   for(gp = gt.getFirst(x, y, dp.nx(), dp.ny());
       gp != nullptr; gp = gt.getNext()) {
      if(gp->x < 0 || gp->x >= dp.nx() ||
         gp->y < 0 || gp->y >= dp.ny()) {
         points.add(bad_data_double);
      }
      else {
         points.add(dp.get(gp->x, gp->y));
      }
   }

   return points;
}

////////////////////////////////////////////////////////////////////////

double interp_min(const DataPlane &dp, const GridTemplate &gt,
                  int x, int y, double t, const MaskPlane *mp) {

   int num_good_points = 0;
   int num_points = gt.size();
   double min_v = bad_data_double;

   // Search the neighborhood
   GridPoint *gp = nullptr;
   for(gp = gt.getFirstInGrid(x, y, dp.nx(), dp.ny());
       gp != nullptr; gp = gt.getNextInGrid()) {

      // Check the optional mask
      if(mp) {
         if(!(*mp)(gp->x, gp->y)) continue;
      }

      double v = dp.get(gp->x, gp->y);
      if(is_bad_data(v)) continue;

      if(is_bad_data(min_v) || v < min_v) {
         min_v = v;
      }
      num_good_points++;

   }

   // Check whether enough valid grid points were found to trust
   // the value computed
   if((num_points == 0) ||
      ((static_cast<double>(num_good_points) / num_points) < t))
      min_v = bad_data_double;

   return min_v;
}

////////////////////////////////////////////////////////////////////////

double interp_min_ll(const DataPlane &dp, int x_ll, int y_ll, int wdth, double t) {
   int x, y, count;
   double v, min_v;

   // Search the neighborhood for the minimum value
   count = 0;
   min_v = 1.0e30;
   for(x=x_ll; x<x_ll+wdth; x++) {
      if(x < 0 || x >= dp.nx()) continue;

      for(y=y_ll; y<y_ll+wdth; y++) {
         if(y < 0 || y >= dp.ny())     continue;
         if(is_bad_data(dp.get(x, y))) continue;

         v = dp.get(x, y);
         if(v < min_v) min_v = v;
         count++;
      } // end for y
   } // end for x

   // Check whether enough valid grid points were found to trust
   // the minimum value computed
   if((double) count/(wdth*wdth) < t || count == 0) {
      min_v = bad_data_double;
   }

   return min_v;
}

////////////////////////////////////////////////////////////////////////

double interp_max(const DataPlane &dp, const GridTemplate &gt,
                  int x, int y, double t, const MaskPlane *mp) {

   int num_good_points = 0;
   int num_points = gt.size();
   double max_v = bad_data_double;

   // Search the neighborhood
   for(GridPoint *gp = gt.getFirstInGrid(x, y, dp.nx(), dp.ny());
       gp != nullptr; gp = gt.getNextInGrid()) {

      // Check the optional mask
      if(mp) {
         if(!(*mp)(gp->x, gp->y)) continue;
      }

      double v = dp.get(gp->x, gp->y);
      if(is_bad_data(v)) continue;

      if(is_bad_data(max_v) || v > max_v) {
         max_v = v;
      }
      num_good_points++;
   }

   // Check whether enough valid grid points were found to trust
   // the value computed
   if((num_points == 0) ||
      ((static_cast<double>(num_good_points) / num_points) < t))
      max_v = bad_data_double;

   return max_v;
}

////////////////////////////////////////////////////////////////////////

double interp_max_ll(const DataPlane &dp, int x_ll, int y_ll, int wdth, double t) {
   int x, y, count;
   double v, max_v;

   // Search the neighborhood for the maximum value
   count = 0;
   max_v = -1.0e30;
   for(x=x_ll; x<x_ll+wdth; x++) {
      if(x < 0 || x >= dp.nx()) continue;

      for(y=y_ll; y<y_ll+wdth; y++) {
         if(y < 0 || y >= dp.ny())     continue;
         if(is_bad_data(dp.get(x, y))) continue;

         v = dp.get(x, y);
         if(v > max_v) max_v = v;
         count++;
      } // end for y
   } // end for x

   // Check whether enough valid grid points were found to trust
   // the maximum value computed
   if((double) count/(wdth*wdth) < t || count == 0) {
      max_v = bad_data_double;
   }

   return max_v;
}

////////////////////////////////////////////////////////////////////////

double interp_median(const DataPlane &dp, const GridTemplate &gt,
                     int x, int y, double t, const MaskPlane *mp) {

   double *data = (double *) nullptr;
   int num_good_points = 0;
   int num_points = gt.size();
   double median_v;

   // Allocate space to store the data points for sorting
   data = new double [gt.size()];

   // Search the neighborhood
   for(GridPoint *gp = gt.getFirstInGrid(x, y, dp.nx(), dp.ny());
       gp != nullptr; gp = gt.getNextInGrid()) {

      // Check the optional mask
      if(mp) {
         if(!(*mp)(gp->x, gp->y)) continue;
      }

      double v = dp.get(gp->x, gp->y);
      if(is_bad_data(v)) continue;

      data[num_good_points] = v;
      num_good_points++;
   }

   // Check whether enough valid grid points were found to trust
   // the value computed
   if((num_points == 0) ||
      ((static_cast<double>(num_good_points) / num_points) < t )) {
      median_v = bad_data_double;
   }
   else {
      sort(data, num_good_points);
      median_v = percentile(data, num_good_points, 0.50);
   }

   delete[] data;

   return median_v;
}


////////////////////////////////////////////////////////////////////////

double interp_median_ll(const DataPlane &dp, int x_ll, int y_ll, int wdth, double t) {
   double *data = (double *) nullptr;
   int x, y, count;
   double v, median_v;

   // Allocate space to store the data points
   data = new double [wdth*wdth];

   // Search the neighborhood for valid data points
   count = 0;
   for(x=x_ll; x<x_ll+wdth; x++) {
      if(x < 0 || x >= dp.nx()) continue;

      for(y=y_ll; y<y_ll+wdth; y++) {
         if(y < 0 || y >= dp.ny())     continue;
         if(is_bad_data(dp.get(x, y))) continue;

         v = dp.get(x, y);
         data[count] = v;
         count++;
      } // end for y
   } // end for x

   // Check whether enough valid grid points were found to compute
   // a median value
   if((double) count/(wdth*wdth) < t || count == 0) {
      median_v = bad_data_double;
   }
   else {
      sort(data, count);
      median_v = percentile(data, count, 0.50);
   }

   if(data) { delete [] data; data = (double *) nullptr; }

   return median_v;
}

////////////////////////////////////////////////////////////////////////

double interp_uw_mean(const DataPlane &dp, const GridTemplate &gt,
                      int x, int y, double t, const MaskPlane *mp) {

   double sum = 0;
   int num_good_points = 0;
   int num_points = gt.size();
   double uw_mean_v;

   // Sum the valid data in the neighborhood
   for(GridPoint *gp = gt.getFirstInGrid(x, y, dp.nx(), dp.ny());
       gp != nullptr; gp = gt.getNextInGrid()) {

      // Check the optional mask
      if(mp) {
         if(!(*mp)(gp->x, gp->y)) continue;
      }

      double v = dp.get(gp->x, gp->y);
      if (is_bad_data(v)) continue;
      sum += v;
      num_good_points++;
   }

   // Check whether enough valid grid points were found to trust
   // the value computed
   if((num_points == 0) ||
      ((static_cast<double>(num_good_points) / num_points) < t )) {
      uw_mean_v = bad_data_double;
   }
   else {
      uw_mean_v = sum / num_good_points;
   }

   return uw_mean_v;
}


////////////////////////////////////////////////////////////////////////

double interp_uw_mean_ll(const DataPlane &dp, int x_ll, int y_ll, int wdth, double t) {
   int x, y, count;
   double v, sum, uw_mean_v;

   // Sum up the valid data in the neighborhood
   count = 0;
   sum = 0.0;
   for(x=x_ll; x<x_ll+wdth; x++) {
      if(x < 0 || x >= dp.nx()) continue;

      for(y=y_ll; y<y_ll+wdth; y++) {
         if(y < 0 || y >= dp.ny())     continue;
         if(is_bad_data(dp.get(x, y))) continue;

         v = dp.get(x, y);
         sum += v;
         count++;
      } // end for y
   } // end for x

   // Check whether enough valid grid points were found to compute
   // a mean value
   if((double) count/(wdth*wdth) < t || count == 0) {
      uw_mean_v = bad_data_double;
   }
   else {
      uw_mean_v = sum/count;
   }

   return uw_mean_v;
}

////////////////////////////////////////////////////////////////////////
//
// Compute the distance-weighted mean using Shepards Method
//
////////////////////////////////////////////////////////////////////////

double interp_dw_mean(const DataPlane &dp, const GridTemplate &gt,
                      double obs_x, double obs_y, int i_pow, double t,
                      const MaskPlane *mp) {

   // Search the neighborhood for valid data points
   double count = 0;
   double wght_sum = 0;
   double numerator = 0;

   int x = nint(obs_x);
   int y = nint(obs_y);

   // if width is even, push center to lower left instead of nearest
   if((gt.getWidth() % 2 ) == 0) {
      x = floor(obs_x);
      y = floor(obs_y);
   }

   for(GridPoint *gp = gt.getFirstInGrid(x, y, dp.nx(), dp.ny());
       gp != nullptr; gp = gt.getNextInGrid()) {

      // Check the optional mask
      if(mp) {
         if(!(*mp)(gp->x, gp->y)) continue;
      }

      x = gp->x;
      y = gp->y;
      double data = dp.get(x, y);

      if(is_bad_data(data)) continue;

      double dist = sqrt(pow((obs_x-x), 2.0) + pow((obs_y-y), 2.0));

      // if the distance is tiny, just use the value at this point.
      if(dist <= 0.001) {
         return data;
      }

      // Otherwise, compute the weight and accumulate numerator and denominator
      double weight = pow(dist, -1*i_pow);
      wght_sum  += weight;
      numerator += (weight * data);
      count++;
   }

   // Check whether enough valid grid points were found to compute
   // a distance weighted mean value
   if((double) count/(gt.size()) < t || count == 0) {
      return bad_data_double;
   }

   return numerator/wght_sum;
}

////////////////////////////////////////////////////////////////////////
//
// Compute a least-squares fit interpolation
//
////////////////////////////////////////////////////////////////////////

double interp_ls_fit(const DataPlane &dp, const GridTemplate &gt,
                     double obs_x, double obs_y, double t,
                     const MaskPlane *mp) {

   // DEV NOTE: We are restricting the GridTemplate to a square,
   // because the LS methods in this function are designed around a square shape.
   // Because we have chosen not to refactor this function to be more general,
   // I am going to simply pull out the relevant values from the GT object,
   // and leave the rest of the function to work the same way as before.
   const RectangularTemplate* tmpGT = dynamic_cast<const RectangularTemplate*>(&gt);
   if((tmpGT == nullptr ) || (tmpGT->getHeight() != tmpGT->getWidth())) {
      mlog << Error << "\ninterp_ls_fit() -> "
           << "Least Squares Interpolation only supports SQUARE shapes.\n\n";
      exit(1);
   }

   int wdth = tmpGT->getWidth();
   int x_ll, y_ll;
   get_xy_ll(obs_x, obs_y, wdth, wdth, x_ll, y_ll);

   //-------------------------------------------------------------------------------

   int i, j, x, y, count;
   const int N  = wdth;
   const int N2 = N*N;
   const double alpha     = (N2*(N2 - 1.0))/12.0;
   const double beta      = 0.5*(N - 1.0);
   const double x_center  = x_ll + beta;
   const double y_center  = y_ll + beta;
   const double u_test    = obs_x - x_center;
   const double v_test    = obs_y - y_center;
   double A, B, C;
   double suz, svz, sz;
   double u, v, z;

   if(N < 2) {
      mlog << Error << "\ninterp_ls_fit() -> "
           << "the interpolation width (" << N
           << ") must be set >= 2\n\n";

      exit (1);
   }
   suz = svz = sz = 0.0;

   // Search the neighborhood
   count = 0;
   for(i=0; i<N; i++) {

      u = i - beta;
      x = x_ll + i;

      if(x < 0 || x >= dp.nx()) continue;

      for(j=0; j<N; j++) {

         v = j - beta;
         y = y_ll + j;

         if(y < 0 || y >= dp.ny())     continue;
         if(is_bad_data(dp.get(x, y))) continue;

         // Check the optional mask
         if(mp) {
            if(!(*mp)(x, y)) continue;
         }

         z = dp.get(x, y);
         count++;

         suz += u*z;
         svz += v*z;
         sz  += z;
      }
   }

   A = suz/alpha;
   B = svz/alpha;
   C = sz/N2;

   z = A*u_test + B*v_test + C;

   // Check for not enough valid data
   if((double) count/N2 < t || count == 0) {
      z = bad_data_double;
   }

   return z;
}

////////////////////////////////////////////////////////////////////////

void interp_gaussian_dp(DataPlane &dp, const GaussianInfo &gaussian, double t) {
   int idx_x, idx_y;
   double value;
   int max_r = gaussian.max_r;
   int g_nx = max_r * 2 + 1;
   int nx = dp.nx();
   int ny = dp.ny();
   DataPlane d_dp;
   DataPlane g_dp;
   
   if (max_r <= 0 || gaussian.weights == (double *)0) {
      mlog << Error << "\ninterp_gaussian_dp() -> "
           << "the gaussian weights were not computed (max_r: " << max_r << ").\n\n";
      exit(1);
   }
   
   g_dp.set_size(g_nx, g_nx);
   d_dp.set_size(nx, ny);
   
   int index = 0;
   for(idx_x=0; idx_x<g_nx; idx_x++) {
      for(idx_y=0; idx_y<g_nx; idx_y++) {
         g_dp.set(gaussian.weights[index++], idx_x, idx_y);
      } // end for y
   } // end for x
   for(idx_x=0; idx_x<nx; idx_x++) {
      for(idx_y=0; idx_y<ny; idx_y++) {
         d_dp.set(dp.get(idx_x, idx_y), idx_x, idx_y);
      } // end for y
   } // end for x
   for(idx_x=0; idx_x<nx; idx_x++) {
      for(idx_y=0; idx_y<ny; idx_y++) {
         value = interp_gaussian(d_dp, g_dp, (double)idx_x,
                                 (double)idx_y, max_r, t);
         dp.set(value, idx_x, idx_y);
      } // end for y
   } // end for x

   mlog << Debug(5) << "interp_gaussian_dp() "
        << "weight_sum: " << gaussian.weight_sum
        << " weight_cnt: " << gaussian.weight_cnt
        << " nx: " << nx << ", ny: " << ny << " max_r: " << max_r
        << "\n";
}

////////////////////////////////////////////////////////////////////////

double interp_gaussian(const DataPlane &dp, const DataPlane &g_dp,
                       double obs_x, double obs_y, int max_r, double t) {
   int count, count_vld;
   double value, gaussian_value, gaussian_weight, weight_sum;

   int x = nint(obs_x);
   int y = nint(obs_y);
   int nx = dp.nx();
   int ny = dp.ny();
   int g_nx = g_dp.nx();

   count = count_vld = 0;
   gaussian_value = weight_sum = 0.0;
   for(int x_idx=0; x_idx<g_nx; x_idx++) {
      int ix = x - max_r + x_idx;
      if (0 > ix || ix >= nx) continue;
      for(int y_idx=0; y_idx<g_nx; y_idx++) {
         int iy = y - max_r + y_idx;
         if (0 > iy || iy >= ny) continue;

         gaussian_weight = g_dp.get(x_idx, y_idx);
         if(gaussian_weight <= 0.) continue;
         value = dp.get(ix, iy);
         count++;
         if(is_bad_data(value)) continue;
         gaussian_value += value * gaussian_weight;
         weight_sum += gaussian_weight;
         count_vld++;
      }
   }

   // Check whether enough valid grid points were found
   if(0 == count || (double)count_vld/count < t || count_vld == 0) {
      gaussian_value = bad_data_double;
   }
   else {
      gaussian_value /= weight_sum;
   }

   return gaussian_value;
}

////////////////////////////////////////////////////////////////////////
//
// Search the interpolation region for the nearest grid point where
// the mask is valid.
//
////////////////////////////////////////////////////////////////////////

double interp_geog_match(const DataPlane &dp, const GridTemplate &gt,
                         double obs_x, double obs_y, double obs_v,
                         const MaskPlane *mp) {

   // Search the neighborhood for valid data points
   int interp_x, interp_y;
   double d, interp_d;
   double dx, dy, v, interp_v;
   int x = nint(obs_x);
   int y = nint(obs_y);

   // if width is even, push center to lower left instead of nearest
   if((gt.getWidth() % 2 ) == 0) {
      x = floor(obs_x);
      y = floor(obs_y);
   }

   // Initialize
   interp_x = interp_y = bad_data_int;
   interp_d = interp_v = bad_data_double;

   for(GridPoint *gp = gt.getFirstInGrid(x, y, dp.nx(), dp.ny());
       gp != nullptr; gp = gt.getNextInGrid()) {

      // Check the optional mask
      if(mp) {
         if(!(*mp)(gp->x, gp->y)) continue;
      }

      // Get the current value
      x = gp->x;
      y = gp->y;
      v = dp.get(x, y);

      // Skip bad data
      if(is_bad_data(v)) continue;

      // Compute the distance
      dx = obs_x - x;
      dy = obs_y - y;
      d  = sqrt(dx*dx + dy*dy);

      if(is_bad_data(interp_d) || d < interp_d) {
         interp_d = d;
         interp_x = x;
         interp_y = y;
         interp_v = v;
      }
   }

   if(!is_bad_data(interp_v)) {
      mlog << Debug(5)
           << "For observation value " << obs_v << " at grid (x, y) = ("
           << obs_x << ", " << obs_y << ") found forecast value "
           << interp_v << " at nearest matching geography point ("
           << interp_x << ", " << interp_y << ").\n";
   }

   return interp_v;
}

////////////////////////////////////////////////////////////////////////
//
// Compute neighborhood fractional coverage.
//
////////////////////////////////////////////////////////////////////////

double interp_nbrhd(const DataPlane &dp, const GridTemplate &gt, int x, int y,
                    double t, const SingleThresh *st, double cmn, double csd,
                    const MaskPlane *mp) {
   int count, count_thr;

   // Compute the ratio of events within the neighborhood
   count = count_thr = 0;

   for(GridPoint *gp = gt.getFirstInGrid(x, y, dp.nx(), dp.ny());
       gp != nullptr; gp = gt.getNextInGrid()) {

      // Check the optional mask
      if(mp) {
         if(!(*mp)(gp->x, gp->y)) continue;
      }

      double data = dp.get(gp->x, gp->y);
      if(is_bad_data(data)) continue;

      count++;
      if(st->check(data, cmn, csd)) count_thr++;
   }

   // Check whether enough valid grid points were found
   if((double) count/(gt.size()) < t || count == 0) {
      return bad_data_double;
   }

   return (double) count_thr/count;
}

////////////////////////////////////////////////////////////////////////
//
// Compute bilinear interpolation.
//
////////////////////////////////////////////////////////////////////////

double interp_bilin(const DataPlane &dp, bool wrap_lon,
                    double obs_x, double obs_y, const MaskPlane *mp) {
   int x, xp1, y;
   double bilin_v, dx, dy;
   double wtsw, wtse, wtnw, wtne;

   x = nint(floor(obs_x));
   y = nint(floor(obs_y));

   // Apply global wrap logic to x and x+1
   x   = (wrap_lon ? positive_modulo(x,   dp.nx()) : x);
   xp1 = (wrap_lon ? positive_modulo(x+1, dp.nx()) : x+1);

   // Check the optional mask
   if(mp) {
      if(!(*mp)(x,   y  ) ||
         !(*mp)(xp1, y  ) ||
         !(*mp)(x,   y+1) ||
         !(*mp)(xp1, y+1)) return bad_data_double;
   }

   // Compute dx and dy
   dx = obs_x - nint(floor(obs_x));
   dy = obs_y - nint(floor(obs_y));

   // Compute weights for 4 corner points
   wtsw = (1.0-dx) * (1.0-dy);
   wtse = dx * (1.0-dy);
   wtnw = (1.0-dx) * dy;
   wtne = dx * dy;

   // On the grid boundary, check each corner point
   if(x < 0 || xp1 >= dp.nx() || y < 0 || y+1 >= dp.ny()) {

      // Initialize weighted sum
      bilin_v = 0.0;

      // Southwest Corner
      if(!is_bad_data(bilin_v) && !is_eq(wtsw, 0.0)) {
         if(x < 0 || x >= dp.nx() || y < 0 || y >= dp.ny())
            bilin_v = bad_data_double;
         else if(is_bad_data(dp.get(x, y)))
            bilin_v = bad_data_double;
         else
            bilin_v += wtsw * dp.get(x, y);
      }

      // Southeast Corner
      if(!is_bad_data(bilin_v) && !is_eq(wtse, 0.0)) {
         if(xp1 < 0 || xp1 >= dp.nx() || y < 0 || y >= dp.ny())
            bilin_v = bad_data_double;
         else if(is_bad_data(dp.get(xp1, y)))
            bilin_v = bad_data_double;
         else
            bilin_v += wtse * dp.get(xp1, y);
      }

      // Northwest Corner
      if(!is_bad_data(bilin_v) && !is_eq(wtnw, 0.0)) {
         if(x < 0 || x >= dp.nx() || y+1 < 0 || y+1 >= dp.ny())
            bilin_v = bad_data_double;
         else if(is_bad_data(dp.get(x, y+1)))
            bilin_v = bad_data_double;
         else
            bilin_v += wtnw * dp.get(x, y+1);
      }

      // Northeast Corner
      if(!is_bad_data(bilin_v) && !is_eq(wtne, 0.0)) {
         if(xp1 < 0 || xp1 >= dp.nx() || y+1 < 0 || y+1 >= dp.ny())
            bilin_v = bad_data_double;
         else if(is_bad_data(dp.get(xp1, y+1)))
            bilin_v = bad_data_double;
         else
            bilin_v += wtne * dp.get(xp1, y+1);
      }
   }
   // On the grid interior, compute weighted average
   else {
      if(is_bad_data(dp.get(x,   y  )) ||
         is_bad_data(dp.get(xp1, y  )) ||
         is_bad_data(dp.get(x,   y+1)) ||
         is_bad_data(dp.get(xp1, y+1))) {
         bilin_v = bad_data_double;
      }
      else {
         bilin_v = wtsw * dp.get(x,   y)   +
                   wtse * dp.get(xp1, y)   +
                   wtnw * dp.get(x,   y+1) +
                   wtne * dp.get(xp1, y+1);
      }
   }

   return bilin_v;
}

////////////////////////////////////////////////////////////////////////
//
// Select a single point but check the grid dimensions.
//
////////////////////////////////////////////////////////////////////////

double interp_xy(const DataPlane &dp, bool wrap_lon, int x, int y,
                 const MaskPlane *mp) {
   double v;

   // Apply global wrap logic
   x = (wrap_lon ? positive_modulo(x, dp.nx()) : x);

   // Check the optional mask
   if(mp) {
      if(!(*mp)(x, y)) return bad_data_double;
   }

   if(x < 0 || x >= dp.nx() || y < 0 || y >= dp.ny()) v = bad_data_double;
   else                                               v = dp.get(x, y);

   return v;
}

////////////////////////////////////////////////////////////////////////
//
// Search the interpolation box for the best match.
//
////////////////////////////////////////////////////////////////////////

double interp_best(const DataPlane &dp, const GridTemplate &gt,
                   int x, int y, double obs_v, double t,
                   const MaskPlane *mp) {
   int count;
   double v, min_d, min_v;

   // Search the neighborhood for the best match to the observation
   count = 0;
   min_d = min_v = bad_data_double;
   for(GridPoint *gp = gt.getFirstInGrid(x, y, dp.nx(), dp.ny());
       gp != nullptr; gp = gt.getNextInGrid()) {

      // Check the optional mask
      if(mp) {
         if(!(*mp)(x, y)) continue;
      }

      v = dp.get(gp->x, gp->y);
      if (is_bad_data(v)) continue;

      if(is_bad_data(min_d) || fabs(v - obs_v) < min_d) {
         min_d = fabs(v - obs_v);
         min_v = v;
      }
      count++;
   }

   // Check whether enough valid grid points were found to trust
   // the maximum value computed
   if((static_cast<double>(count)/gt.size()) < t || count == 0) {
      return bad_data_double;
   }

   return min_v;
}

////////////////////////////////////////////////////////////////////////
//
// Determine the lower-left (x, y) based on height and width parity.
//
////////////////////////////////////////////////////////////////////////

void get_xy_ll(double x, double y, int w, int h, int &x_ll, int &y_ll) {

   // Range check
   if(w <= 0 || h <= 0) {
      mlog << Error << "\nget_xy_ll() -> "
           << "the height (" << h << ") and width (" << w
           << ") must be set >= 0\n\n";
      exit(1);
   }

   // Define left-most x based on the parity of width
   if(w%2 == 1) x_ll = nint(x) - (w - 1)/2;
   else         x_ll = nint(floor(x) - (w/2 - 1));

   // Define lower-most y based on the parity of height
   if(h%2 == 1) y_ll = nint(y) - (h - 1)/2;
   else         y_ll = nint(floor(y) - (h/2 - 1));

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Do horizontal interpolation at the surface
//
////////////////////////////////////////////////////////////////////////

double compute_sfc_interp(const DataPlane &dp,
                          double obs_x,   double obs_y,
                          double obs_elv, double obs_v,
                          const InterpMthd mthd, const int width,
                          const GridTemplateFactory::GridTemplates shape,
                          bool wrap_lon, double interp_thresh,
                          const SurfaceInfo &sfc_info, bool is_land_obs) {
   double v = bad_data_double;
   int x = nint(obs_x);
   int y = nint(obs_y);

   // if width is even, push center to lower left point instead of nearest
   if((width % 2 ) == 0) {
      x = static_cast<int>(floor(obs_x));
      y = static_cast<int>(floor(obs_y));
   }

   GridTemplateFactory gtf;
   const GridTemplate* gt = gtf.buildGT(shape, width, wrap_lon);

   MaskPlane sfc_mask = compute_sfc_mask(*gt, x, y, sfc_info, is_land_obs, obs_elv);

   // Compute the interpolated value for the fields above and below
   switch(mthd) {

      case InterpMthd::Min:         // Minimum
         v = interp_min(dp, *gt, x, y, interp_thresh, &sfc_mask);
         break;

      case InterpMthd::Max:         // Maximum
         v = interp_max(dp, *gt, x, y, interp_thresh, &sfc_mask);
         break;

      case InterpMthd::Median:      // Median
         v = interp_median(dp, *gt, x, y, interp_thresh, &sfc_mask);
         break;

      case InterpMthd::UW_Mean:     // Unweighted Mean
         v = interp_uw_mean(dp, *gt, x, y, interp_thresh, &sfc_mask);
         break;

      case InterpMthd::DW_Mean:     // Distance-Weighted Mean
         v = interp_dw_mean(dp, *gt, obs_x, obs_y,
                            dw_mean_pow, interp_thresh, &sfc_mask);
         break;

      case InterpMthd::LS_Fit:      // Least-squares fit
         v = interp_ls_fit(dp, *gt, obs_x, obs_y,
                           interp_thresh, &sfc_mask);
         break;

      case InterpMthd::Bilin:       // Bilinear interpolation
         v = interp_bilin(dp, wrap_lon, obs_x, obs_y, &sfc_mask);
         break;

      case InterpMthd::Nearest:     // Nearest Neighbor
         v = interp_xy(dp, wrap_lon, x, y, &sfc_mask);
         break;

      case InterpMthd::Best:        // Best Match
         v = interp_best(dp, *gt, x, y, obs_v, interp_thresh, &sfc_mask);
         break;

      case InterpMthd::Upper_Left:  // Upper Left corner of the grid box
         v = interp_xy(dp, wrap_lon, floor(obs_x), ceil(obs_y), &sfc_mask);
         break;

      case InterpMthd::Upper_Right: // Upper Right corner of the grid box
         v = interp_xy(dp, wrap_lon, ceil(obs_x), ceil(obs_y), &sfc_mask);
         break;

      case InterpMthd::Lower_Right: // Lower Right corner of the grid box
         v = interp_xy(dp, wrap_lon, ceil(obs_x), floor(obs_y), &sfc_mask);
         break;

      case InterpMthd::Lower_Left:  // Lower Left corner of the grid box
         v = interp_xy(dp, wrap_lon, floor(obs_x), floor(obs_y), &sfc_mask);
         break;

      case InterpMthd::Geog_Match:  // Geography Match for surface point verification
         v = interp_geog_match(dp, *gt, obs_x, obs_y, obs_v, &sfc_mask);
         break;

      default:
         mlog << Error << "\ncompute_sfc_interp() -> "
              << "unsupported interpolation method encountered: "
              << interpmthd_to_string(mthd) << "(" << enum_class_as_int(mthd) << ")\n\n";
         exit(1);
   }

   delete gt;
   return v;
}

////////////////////////////////////////////////////////////////////////

MaskPlane compute_sfc_mask(const GridTemplate &gt, int x, int y,
                           const SurfaceInfo &sfc_info,
                           bool is_land_obs, double obs_elv) {
   MaskPlane mp;
   int nx = 0;
   int ny = 0;
   bool land_ok, topo_ok;

   // Initialize the mask
   if(sfc_info.land_ptr) {
      nx = sfc_info.land_ptr->nx();
      ny = sfc_info.land_ptr->ny();
   }
   else if(sfc_info.topo_ptr) {
      nx = sfc_info.topo_ptr->nx();
      ny = sfc_info.topo_ptr->ny();
   }
   else {
      mlog << Error << "\ncompute_sfc_mask() -> "
           << "the land and topography pointers are not set!\n\n";
      exit(1);
   }

   //
   // Search the neighborhood
   //
   mp.set_size(nx, ny, false);
   GridPoint *gp = nullptr;
   for(gp = gt.getFirstInGrid(x, y, nx, ny);
       gp != nullptr; gp = gt.getNextInGrid()) {

      // Check the land mask
      if(sfc_info.land_ptr) {
         bool is_land_grid = (*sfc_info.land_ptr)(gp->x, gp->y);
         land_ok = (( is_land_obs &&  is_land_grid) ||
                    (!is_land_obs && !is_land_grid));
      }
      else {
         land_ok = true;
      }

      // Check the topo mask
      if(sfc_info.topo_ptr) {
         double topo_grid = sfc_info.topo_ptr->get(gp->x, gp->y);
         topo_ok = sfc_info.topo_interp_fcst_thresh.check(topo_grid - obs_elv);
      }
      else {
         topo_ok = true;
      }

      mp.put((land_ok & topo_ok), gp->x, gp->y);
   }

   return mp;
}

////////////////////////////////////////////////////////////////////////
//
// Utility functions for horizontal and vertical interpolation
//
////////////////////////////////////////////////////////////////////////

double compute_horz_interp(const DataPlane &dp,
                           double obs_x, double obs_y, double obs_v,
                           const InterpMthd mthd, const int width,
                           const GridTemplateFactory::GridTemplates shape,
                           bool wrap_lon, double interp_thresh,
                           const SingleThresh *cat_thresh) {
   return compute_horz_interp(dp, obs_x, obs_y, obs_v, bad_data_double,
             bad_data_double, mthd, width, shape, wrap_lon,
             interp_thresh, cat_thresh);
}

////////////////////////////////////////////////////////////////////////

double compute_horz_interp(const DataPlane &dp,
                           double obs_x, double obs_y,
                           double obs_v, double cmn, double csd,
                           const InterpMthd mthd, const int width,
                           const GridTemplateFactory::GridTemplates shape,
                           bool wrap_lon, double interp_thresh,
                           const SingleThresh *cat_thresh) {
   double v = bad_data_double;
   int x = nint(obs_x);
   int y = nint(obs_y);

   // if width is even, push center to lower left point instead of nearest
   if((width % 2) == 0) {
      x = static_cast<int>(floor(obs_x));
      y = static_cast<int>(floor(obs_y));
   }

   GridTemplateFactory gtf;
   const GridTemplate* gt = gtf.buildGT(shape, width, wrap_lon);

   // Compute the interpolated value for the fields above and below
   switch(mthd) {

      case InterpMthd::Min:         // Minimum
         v = interp_min(dp, *gt, x, y, interp_thresh);
         break;

      case InterpMthd::Max:         // Maximum
         v = interp_max(dp, *gt, x, y, interp_thresh);
         break;

      case InterpMthd::Median:      // Median
         v = interp_median(dp, *gt, x, y, interp_thresh);
         break;

      case InterpMthd::UW_Mean:     // Unweighted Mean
         v = interp_uw_mean(dp, *gt, x, y, interp_thresh);
         break;

      case InterpMthd::DW_Mean:     // Distance-Weighted Mean
         v = interp_dw_mean(dp, *gt, obs_x, obs_y,
                            dw_mean_pow, interp_thresh);
         break;

      case InterpMthd::LS_Fit:      // Least-squares fit
         v = interp_ls_fit(dp, *gt, obs_x, obs_y,
                           interp_thresh);
         break;

      case InterpMthd::Nbrhd:       // Neighborhood fractional coverage
         v = interp_nbrhd(dp, *gt, x, y,
                          interp_thresh, cat_thresh, cmn, csd);
         break;

      case InterpMthd::Bilin:       // Bilinear interpolation
         v = interp_bilin(dp, wrap_lon, obs_x, obs_y);
         break;

      case InterpMthd::Nearest:     // Nearest Neighbor
         v = interp_xy(dp, wrap_lon, x, y);
         break;

      case InterpMthd::Best:        // Best Match
         v = interp_best(dp, *gt, x, y, obs_v, interp_thresh);
         break;

      case InterpMthd::Upper_Left:  // Upper Left corner of the grid box
         v = interp_xy(dp, wrap_lon, floor(obs_x), ceil(obs_y));
         break;

      case InterpMthd::Upper_Right: // Upper Right corner of the grid box
         v = interp_xy(dp, wrap_lon, ceil(obs_x), ceil(obs_y));
         break;

      case InterpMthd::Lower_Right: // Lower Right corner of the grid box
         v = interp_xy(dp, wrap_lon, ceil(obs_x), floor(obs_y));
         break;

      case InterpMthd::Lower_Left:  // Lower Left corner of the grid box
         v = interp_xy(dp, wrap_lon, floor(obs_x), floor(obs_y));
         break;

      case InterpMthd::Geog_Match:  // Geography Match for surface point verification
         v = interp_geog_match(dp, *gt, obs_x, obs_y, obs_v);
         break;

      default:
         mlog << Error << "\ncompute_horz_interp() -> "
              << "unsupported interpolation method encountered: "
              << interpmthd_to_string(mthd) << "(" << enum_class_as_int(mthd) << ")\n\n";
         exit(1);
   }

   delete gt;
   return v;
}

////////////////////////////////////////////////////////////////////////
//
// Interpolate lineary in the log of pressure between values "v1" and
// "v2" at pressure levels "prs1" and "prs2" to pressure level "to_prs".
//
////////////////////////////////////////////////////////////////////////

double compute_vert_pinterp(double v1, double prs1,
                            double v2, double prs2,
                            double to_prs) {
   double v_interp;

   if(prs1 <= 0.0 || prs2 <= 0.0 || to_prs <= 0.0) {
      mlog << Error << "\ncompute_vert_pinterp() -> "
           << "pressure shouldn't be <= zero!\n\n";
      exit(1);
   }

   // Check that the to_prs falls between the limits
   if( !(to_prs >= prs1 && to_prs <= prs2) &&
       !(to_prs <= prs1 && to_prs >= prs2) ) {
      mlog << Warning << "\ncompute_vert_pinterp() -> "
           << "the interpolation pressure, " << to_prs
           << ", should fall between the pressure limits, "
           << prs1 << " and " << prs2 << "\n\n";
   }

   v_interp = v1 + ((v2-v1)*log(prs1/to_prs)/log(prs1/prs2));

   return v_interp;
}

////////////////////////////////////////////////////////////////////////
//
// Linearly interpolate between values "v1" and "v2" at height levels
// "lvl1" and "lvl2" to height level "to_lvl".
//
////////////////////////////////////////////////////////////////////////

double compute_vert_zinterp(double v1, double lvl1,
                            double v2, double lvl2,
                            double to_lvl) {
   double d1, d2, v_interp;

   if(lvl1 <= 0.0 || lvl2 <= 0.0 || to_lvl <= 0.0) {
      mlog << Error << "\ncompute_vert_zinterp() -> "
           << "level shouldn't be <= zero!\n\n";
      exit(1);
   }

   // Check that the to_lvl falls between the limits
   if( !(to_lvl >= lvl1 && to_lvl <= lvl2) &&
       !(to_lvl <= lvl1 && to_lvl >= lvl2) ) {
      mlog << Warning << "\ncompute_vert_zinterp() -> "
           << "the interpolation level, " << to_lvl
           << ", should fall between the level limits, "
           << lvl1 << " and " << lvl2 << "\n\n";
   }

   d1 = fabs(lvl1 - to_lvl);
   d2 = fabs(lvl2 - to_lvl);

   // Linearly interpolate betwen lvl_1 and lvl_2
   v_interp = v1*(1.0 - d1/(d1+d2)) + v2*(1.0 - d2/(d1+d2));

   return v_interp;
}

////////////////////////////////////////////////////////////////////////

DataPlane valid_time_interp(const DataPlane &in1, const DataPlane &in2,
                            const unixtime to_ut, InterpMthd mthd) {
   DataPlane dp, dp1, dp2;
   int x, y;
   bool use_min;
   double v, v1, v2;
   double w1 = bad_data_double;
   double w2 = bad_data_double;

   // Store min and max valid times
   dp1 = (in1.valid() <= in2.valid() ? in1 : in2);
   dp2 = (in1.valid() >  in2.valid() ? in1 : in2);

   // Check for matching valid times
   if(dp1.valid() == dp2.valid()) return dp1;

   // Range check the times
   if(dp1.valid() > to_ut || dp2.valid() < to_ut) {
      mlog << Error << "\ntime_interp() -> "
           << "the interpolation time " << unix_to_yyyymmdd_hhmmss(to_ut)
           << " must fall between the input times: "
           << unix_to_yyyymmdd_hhmmss(dp1.valid()) << " and "
           << unix_to_yyyymmdd_hhmmss(dp2.valid()) << ".\n\n";
      exit(1);
   }

   // Check grid dimensions
   if(dp1.nx() != dp2.nx() || dp1.ny() != dp2.ny()) {
      mlog << Error << "\nvalid_time_interp() -> "
           << "the grid dimensions don't match: "
           << dp1.nx() << " x " << dp1.ny() << " != "
           << dp2.nx() << " x " << dp2.ny() << ".\n\n";
      exit(1);
   }

   // Compute interpolation weights
   switch(mthd) {
      case InterpMthd::Min:     // Minimum
      case InterpMthd::Max:     // Maximum
         w1 = w2 = bad_data_double;
         break;

      case InterpMthd::UW_Mean: // Unweighted Mean
         w1 = w2 = 0.5;
         break;

      case InterpMthd::DW_Mean: // Distance-Weighted Mean
         w1 = (double) (dp2.valid() - to_ut) /
                       (dp2.valid() - dp1.valid());
         w2 = (double) (to_ut - dp1.valid()) /
                       (dp2.valid() - dp1.valid());
         break;

      case InterpMthd::Nearest: // Nearest Neighbor
         use_min = ((to_ut - dp1.valid()) <=
                    (dp2.valid() - to_ut));
         w1 = (use_min ? 1.0 : 0.0);
         w2 = (use_min ? 0.0 : 1.0);
         break;

      case InterpMthd::AW_Mean: // Area-Weighted Mean
      case InterpMthd::Median:  // Median
      case InterpMthd::LS_Fit:  // Least-squares fit
      case InterpMthd::Bilin:   // Bilinear interpolation
      default:
         mlog << Error << "\nvalid_time_interp() -> "
              << "unsupported interpolation method encountered: "
              << interpmthd_to_string(mthd) << "(" << enum_class_as_int(mthd) << ")\n\n";
         exit(1);
   }

   // Initialize
   dp.clear();
   dp.set_size(dp1.nx(), dp1.ny());
   dp.set_valid(to_ut);

   // Compute interpolated value for each point
   for(x=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         // Get current values
         v1 = dp1.get(x, y);
         v2 = dp2.get(x, y);
         v  = bad_data_double;

         // Check for bad data
         if(!is_bad_data(v1) && !is_bad_data(v2)) {

            // Minimum
                 if(mthd == InterpMthd::Min) v = min(v1, v2);
            // Maximum
            else if(mthd == InterpMthd::Max) v = max(v1, v2);
            // Apply weights
            else                             v = w1*v1 + w2*v2;
         }

         // Store interpolated value
         dp.set(v, x, y);
      } // end for y
   } // end for x

   return dp;
}

////////////////////////////////////////////////////////////////////////
