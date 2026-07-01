// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <iostream>
#include <math.h>
#include <string.h>
#include <unistd.h>

#ifdef _OPENMP
  #include "omp.h"
#endif

#include "config_util.h"
#include "data_plane_util.h"
#include "interp_util.h"
#include "two_to_one.h"

#include "vx_gsl_prob.h"
#include "vx_math.h"
#include "vx_log.h"
#include "enum_as_int.hpp"

#include "GridTemplate.h"

using namespace std;

////////////////////////////////////////////////////////////////////////
//
// Utility functions operating on a DataPlane
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// Check the range of probability values and make sure it's either
// [0, 1] or [0, 100].  If it's [0, 100], rescale to [0, 1].
//
////////////////////////////////////////////////////////////////////////

void rescale_probability(DataPlane &dp) {

   //
   // Get the range of data values in the field
   //
   double min_v;
   double max_v;
   dp.data_range(min_v, max_v);

   //
   // Check for a valid range of probability values.
   //
   if(min_v < 0.0-loose_tol || max_v > 100.0+loose_tol) {
      mlog << Error << "\nrescale_probability() -> "
           << "invalid range of data for a probability field: ["
           << min_v << ", " << max_v << "].\n\n";
      exit(1);
   }

   //
   // If necessary, rescale data from [0, 100] to [0, 1]
   //
   if(max_v > 1.0) {

      mlog << Debug(3)
           << "Rescaling probabilistic field from [0,100] to [0,1].\n";

#pragma omp parallel default(none) \
   shared(dp)
      {
         //
         // Divide each value by 100
         //
#pragma omp for schedule(static) \
                collapse(2) 
         for(int x=0; x<dp.nx(); x++) {
            for(int y=0; y<dp.ny(); y++) {

               double v = dp.get(x, y);
               if(!is_bad_data(v)) dp.set(v/100.0, x, y);

            } // end for y
         } // end for x
      } // End omp parallel
   }

   return;

}

////////////////////////////////////////////////////////////////////////
//
// Smooth the DataPlane values using the interpolation method and
// Grid Template specified.
//
////////////////////////////////////////////////////////////////////////

void smooth_field(const DataPlane &dp, DataPlane &smooth_dp,
                  InterpMthd mthd, int width,
                  const GridTemplateFactory::GridTemplates shape,
                  bool wrap_lon, double t, const GaussianInfo &gaussian) {
   double v;

   // Initialize the smoothed field to the raw field
   smooth_dp = dp;

   // For nearest neighbor, no work to do.
   if(width == 1 && mthd == InterpMthd::Nearest) return;

#pragma omp parallel default(shared) \
   shared(mlog, Error, dp, smooth_dp) \
   shared(mthd, width, wrap_lon, t, gaussian) \
   private(v)
   {

      // build the grid template
      GridTemplateFactory gtf;
      GridTemplate* gt = gtf.buildGT(shape, width, wrap_lon);

#pragma omp single
   {
      mlog << Debug(3)
           << "Smoothing " << (wrap_lon ? "global" : "non-global")
           << " field using the " << interpmthd_to_string(mthd)
           << "(" << gt->size() << ") " << gt->getClassName()
           << " interpolation method.\n";
   } 

      // Otherwise, apply smoothing to each grid point
#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<dp.nx(); x++) {
         for(int y=0; y<dp.ny(); y++) {

            // Compute the smoothed value based on the interpolation method
            switch(mthd) {

               case InterpMthd::Min:      // Minimum
                  v = interp_min(dp, *gt, x, y, t);
                  break;

               case InterpMthd::Max:      // Maximum
                  v = interp_max(dp, *gt, x, y, t);
                  break;

               case InterpMthd::Median:   // Median
                  v = interp_median(dp, *gt, x, y, t);
                  break;

               case InterpMthd::UW_Mean:  // Unweighted Mean
                  v = interp_uw_mean(dp, *gt, x, y, t);
                  break;

               case InterpMthd::Gaussian: // For Gaussian, pass the data through
                  v = dp.get(x, y);
                  break;

               case InterpMthd::MaxGauss: // For Max Gaussian, compute the max
                  v = interp_max(dp, *gt, x, y, 0);
                  break;

               // Distance-weighted mean, area-weighted mean, least-squares
               // fit, and bilinear are omitted here since they are not
               // options for gridded data.
               default:
                  mlog << Error << "\nsmooth_field() -> "
                       << "unsupported interpolation method encountered: "
                       << interpmthd_to_string(mthd) << "(" << enum_class_as_int(mthd)
                       << ")\n\n";
                  exit(1);
            }

            // Store the smoothed value
            smooth_dp.set(v, x, y);

         } // end for y
      } // end for x

      // Cleanup
      delete gt;

   } // End omp parallel

   // Apply the Gaussian smoother 
   if(mthd == InterpMthd::Gaussian ||
      mthd == InterpMthd::MaxGauss) {
      interp_gaussian_dp(smooth_dp, gaussian, t);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Smooth the DataPlane values using the interpolation method and
// Grid Template specified.
//
////////////////////////////////////////////////////////////////////////

DataPlane smooth_field(const DataPlane &dp,
                       InterpMthd mthd, int width,
                       const GridTemplateFactory::GridTemplates shape,
                       bool wrap_lon, double t, const GaussianInfo &gaussian) {
   DataPlane smooth_dp;

   smooth_field(dp, smooth_dp, mthd, width, shape, wrap_lon, t, gaussian);

   return smooth_dp;
}

////////////////////////////////////////////////////////////////////////
//
// Convert the DataPlane field to the corresponding fractional coverage
// using the threshold critea specified.
//
////////////////////////////////////////////////////////////////////////

void fractional_coverage(const DataPlane &dp, DataPlane &frac_dp,
        int width, GridTemplateFactory::GridTemplates shape,
        bool wrap_lon, SingleThresh t,
        const DataPlane *fcmn, const DataPlane *fcsd,
        const DataPlane *ocmn, const DataPlane *ocsd,
        double vld_t) {
   GridPoint *gp = nullptr;
   int n_vld = 0;
   int n_thr = 0;
   double v;
   double bad = bad_data_double;
   bool use_climo = false;

   // Check that width is set to 1 or greater
   if(width < 1) {
      mlog << Error << "\nfractional_coverage() -> "
           << "grid must have at least one point in it. \n\n";
      exit(1);
   }

   // Check climatology data, if needed
   if(fcmn && !fcmn->is_empty() &&
      fcsd && !fcsd->is_empty() &&
      ocmn && !ocmn->is_empty() &&
      ocsd && !ocsd->is_empty()) use_climo = true;

   // Check climatology dimensions
   if(use_climo) {

      // Check dimensions
      if(fcmn->nx() != dp.nx() || fcmn->ny() != dp.ny()) {
         mlog << Error << "\nfractional_coverage() -> "
           << "forecast climatology mean dimension ("
           << fcmn->nx() << ", " << fcmn->ny()
           << ") does not match the data dimension ("
           << dp.nx() << ", " << dp.ny() << ")!\n\n";
         exit(1);
      }
      if(fcsd->nx() != dp.nx() || fcsd->ny() != dp.ny()) {
         mlog << Error << "\nfractional_coverage() -> "
           << "forecast climatology standard deviation dimension ("
           << fcsd->nx() << ", " << fcsd->ny()
           << ") does not match the data dimension ("
           << dp.nx() << ", " << dp.ny() << ")!\n\n";
         exit(1);
      }
      if(ocmn->nx() != dp.nx() || ocmn->ny() != dp.ny()) {
         mlog << Error << "\nfractional_coverage() -> "
           << "observation climatology mean dimension ("
           << ocmn->nx() << ", " << ocmn->ny()
           << ") does not match the data dimension ("
           << dp.nx() << ", " << dp.ny() << ")!\n\n";
         exit(1);
      }
      if(ocsd->nx() != dp.nx() || ocsd->ny() != dp.ny()) {
         mlog << Error << "\nfractional_coverage() -> "
           << "observation climatology standard deviation dimension ("
           << ocsd->nx() << ", " << ocsd->ny()
           << ") does not match the data dimension ("
           << dp.nx() << ", " << dp.ny() << ")!\n\n";
         exit(1);
      }
   }

#pragma omp parallel default(none) \
   shared(mlog, dp, frac_dp, shape, width, wrap_lon, t) \
   shared(use_climo, fcmn, fcsd, ocmn, ocsd, vld_t, bad) \
   private(n_vld, n_thr, gp, v)
   {

     // Build the grid template
     GridTemplateFactory gtf;
     GridTemplate* gt = gtf.buildGT(shape, width, wrap_lon);

#pragma omp single
     {
       mlog << Debug(3)
            << "Computing fractional coverage field using the "
            << t.get_str() << " threshold and the "
            << interpmthd_to_string(InterpMthd::Nbrhd) << "(" << gt->size()
            << ") " << gt->getClassName() << " interpolation method.\n";

       // Initialize the fractional coverage field
       frac_dp = dp;
       frac_dp.set_constant(bad_data_double);
     }

     // Compute the fractional coverage meeting the threshold criteria
#pragma omp for schedule(static)
     for(int x=0; x<dp.nx(); x++) {
        for(int y=0; y<dp.ny(); y++) {

           // For a new column, reset the grid template and counts.
           if(y == 0) {

              // Initialize counts
              n_vld = n_thr = 0;

              // Sum all the points
              for(gp  = gt->getFirstInGrid(x, y, dp.nx(), dp.ny());
                  gp != nullptr;
                  gp  = gt->getNextInGrid()) {
                 if(is_bad_data(v = dp.get(gp->x, gp->y))) continue;
                 n_vld++;
                 ClimoPntInfo cpi;
                 if(use_climo) {
                    cpi.set(fcmn->get(gp->x, gp->y),
                            fcsd->get(gp->x, gp->y),
                            ocmn->get(gp->x, gp->y),
                            ocsd->get(gp->x, gp->y));
                 }
                 if(t.check(v, &cpi)) n_thr++;
              }
           }
           // Subtract off the bottom edge, shift up, and add the top.
           else {

              // Subtract points from the the bottom edge
              for(gp  = gt->getFirstInBotEdge();
                  gp != nullptr;
                  gp  = gt->getNextInBotEdge()) {
                 if(is_bad_data(v = dp.get(gp->x, gp->y))) continue;
                 n_vld--;
                 ClimoPntInfo cpi;
                 if(use_climo) {
                    cpi.set(fcmn->get(gp->x, gp->y),
                            fcsd->get(gp->x, gp->y),
                            ocmn->get(gp->x, gp->y),
                            ocsd->get(gp->x, gp->y));
                 }
                 if(t.check(v, &cpi)) n_thr--;
              }

              // Increment Y
              gt->incBaseY(1);

              // Add points from the the top edge
              for(gp  = gt->getFirstInTopEdge();
                  gp != nullptr;
                  gp  = gt->getNextInTopEdge()) {
                 if(is_bad_data(v = dp.get(gp->x, gp->y))) continue;
                 n_vld++;
                 ClimoPntInfo cpi;
                 if(use_climo) {
                    cpi.set(fcmn->get(gp->x, gp->y),
                            fcsd->get(gp->x, gp->y),
                            ocmn->get(gp->x, gp->y),
                            ocsd->get(gp->x, gp->y));
                 }
                 if(t.check(v, &cpi)) n_thr++;
              }
           }

           // Check for enough valid data and compute fractional coverage
           if((double)(n_vld)/gt->size() >= vld_t && n_vld != 0) {
              frac_dp.set((double) n_thr/n_vld, x, y);
           }

        } // end for y

        // Increment X
        if(x < (dp.nx() - 1)) gt->incBaseX(1);

     } // end for x

     delete gt;

   } // End of omp parallel

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Select points inside the mask and write them to a NumArray.
// For an empty input field, write all bad data values.
//
////////////////////////////////////////////////////////////////////////

void apply_mask(const DataPlane &in, const MaskPlane &mask,
                NumArray &na) {

   if((in.nx() != mask.nx() || in.ny() != mask.ny()) &&
       in.nx() != 0         && in.ny() != 0) {
      mlog << Error << "\napply_mask() -> "
           << "grid dimensions do not match ("
           << in.nx() << ", " << in.ny() << ") != ("
           << mask.nx() << ", " << mask.ny() << ")!\n\n";
      exit(1);
   }

   // Initialize the NumArray object
   na.erase();

   int Nxy = mask.nx() * mask.ny();

   // Do not parallelize to preserve output order
   for(int i=0; i<Nxy; i++) {

      // Store the values where the mask is on
      if(mask.data()[i]) {
         na.add(in.nx() == 0 && in.ny() == 0 ?
                bad_data_double : in.data()[i]);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Write bad data everywhere the mask is turned off.
//
////////////////////////////////////////////////////////////////////////

void apply_mask(DataPlane &in, const MaskPlane &mask) {

   if(in.nx() != mask.nx() || in.ny() != mask.ny()) {
      mlog << Error << "\napply_mask() -> "
           << "grid dimensions do not match ("
           << in.nx() << ", " << in.ny() << ") != ("
           << mask.nx() << ", " << mask.ny() << ")!\n\n";
      exit(1);
   }

   int Nxy = mask.nx() * mask.ny();

#pragma omp parallel default(none) \
   shared(in, mask, Nxy)
   {

#pragma omp for schedule(static)
      for(int i=0; i<Nxy; i++) {
         if(!is_bad_data(in.data()[i]) && !mask.data()[i]) {
            in.buf()[i] = bad_data_double;
         }
      }
   } // End omp parallel

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Turn input field off everywhere the mask is off.
//
////////////////////////////////////////////////////////////////////////

void apply_mask(MaskPlane &in, const MaskPlane &mask) {

   if(in.nx() != mask.nx() || in.ny() != mask.ny() ) {
      mlog << Error << "\napply_mask() -> "
           << "grid dimensions do not match ("
           << in.nx() << ", " << in.ny() << ") != ("
           << mask.nx() << ", " << mask.ny() << ")!\n\n";
      exit(1);
   }

   int Nxy = mask.nx() * mask.ny();

#pragma omp parallel default(none) \
   shared(in, mask, Nxy)
   {

#pragma omp for schedule(static)
      for(int i=0; i<Nxy; i++) {
         if(!mask.data()[i]) in.buf()[i] = false;
      }
   } // End omp parallel

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Turn off the mask at any grid points containing missing data.
//
////////////////////////////////////////////////////////////////////////

void mask_bad_data(DataPlane &dp, const DataPlane &mask_dp, double v) {

   if(dp.nx() != mask_dp.nx() || dp.ny() != mask_dp.ny()) {
      mlog << Error << "\nmask_bad_data() -> "
           << "grid dimensions do not match ("
           << dp.nx() << ", " << dp.ny() << ") != ("
           << mask_dp.nx() << ", " << mask_dp.ny() << ")!\n\n";
      exit(1);
   }

   int Nxy = mask_dp.nx() * mask_dp.ny();

#pragma omp parallel default(none) \
   shared(dp, mask_dp, v, Nxy)
   {

#pragma omp for schedule(static)
      for(int i=0; i<Nxy; i++) {
         if(is_bad_data(mask_dp.data()[i])) {
            dp.buf()[i] = v;
         }
      }
   } // End omp parallel

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Turn off the mask at any grid points containing missing data.
//
////////////////////////////////////////////////////////////////////////

void mask_bad_data(MaskPlane &mp, const DataPlane &dp) {

   if(mp.nx() != dp.nx() || mp.ny() != dp.ny()) {
      mlog << Error << "\nmask_bad_data() -> "
           << "grid dimensions do not match ("
           << mp.nx() << ", " << mp.ny() << ") != ("
           << dp.nx() << ", " << dp.ny() << ")!\n\n";
      exit(1);
   }

   int Nxy = dp.nx() * dp.ny();

#pragma omp parallel default(none) \
   shared(mp, dp, Nxy)
   {

#pragma omp for schedule(static)
      for(int i=0; i<Nxy; i++) {
         if(is_bad_data(dp.data()[i])) {
            mp.buf()[i] = false;
         }
      }
   } // End omp parallel

   return;
}

////////////////////////////////////////////////////////////////////////

DataPlane subtract(const DataPlane &dp1, const DataPlane &dp2) {
   DataPlane diff(dp1);

   if(dp1.nx() != dp2.nx() || dp1.ny() != dp2.ny()) {
      mlog << Error << "\nsubtract() -> "
           << "grid dimensions do not match ("
           << dp1.nx() << ", " << dp1.ny() << ") != ("
           << dp2.nx() << ", " << dp2.ny() << ")!\n\n";
      exit(1);
   }

#pragma omp parallel default(none) \
   shared(dp1, dp2, diff)
   {

#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<dp1.nx(); x++) {
         for(int y=0; y<dp1.ny(); y++) {
            double v = (is_bad_data(dp1.get(x,y)) ||
                        is_bad_data(dp2.get(x,y)) ?
                        bad_data_double :
                        dp1.get(x,y) - dp2.get(x,y));
            diff.set(v, x, y);
         }
      }
   } // End omp parallel

   return diff;
}

////////////////////////////////////////////////////////////////////////

DataPlane normal_cdf(const DataPlane &dp, const DataPlane &mn,
                     const DataPlane &sd) {
   DataPlane cdf(mn);

   // Check grid dimensions
   if(dp.nx() != mn.nx() || dp.ny() != mn.ny() ||
      dp.nx() != sd.nx() || dp.ny() != sd.ny()) {
      mlog << Error << "\nnormal_cdf() -> "
           << "grid dimensions do not match ("
           << dp.nx() << ", " << dp.ny() << ") != ("
           << mn.nx() << ", " << mn.ny() << ") or ("
           << sd.nx() << ", " << sd.ny() << ")!\n\n";
      exit(1);
   }

#pragma omp parallel default(none) \
   shared(dp, mn, sd, cdf)
   {

   // Compute the normal CDF for each grid point
#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<dp.nx(); x++) {
         for(int y=0; y<dp.ny(); y++) {
            double v;
            if(is_bad_data(dp.get(x,y)) ||
               is_bad_data(mn.get(x,y)) ||
               is_bad_data(sd.get(x,y))) {
               v = bad_data_double;
            }
            else {
               v = normal_cdf(dp.get(x,y), mn.get(x,y), sd.get(x,y));
            }
            cdf.set(v, x, y);
         }
      }
   } // End omp parallel

   return cdf;
}

////////////////////////////////////////////////////////////////////////

DataPlane normal_cdf_inv(const double area, const DataPlane &mn,
                         const DataPlane &sd) {
   DataPlane cdf_inv(mn);

   // Check grid dimensions
   if(mn.nx() != sd.nx() || mn.ny() != sd.ny()) {
      mlog << Error << "\nnormal_cdf_inv() -> "
           << "grid dimensions do not match ("
           << mn.nx() << ", " << mn.ny() << ") != ("
           << sd.nx() << ", " << sd.ny() << ")!\n\n";
      exit(1);
   }

   // Range check area value
   if(area <= 0.0 || area >= 1.0) {
      mlog << Error << "\nnormal_cdf_inv() -> "
           << "requested area (" << area
           << ") must be between 0 and 1.\n\n";
      exit(1);
   }

#pragma omp parallel default(shared) \
   shared(mn, sd, cdf_inv)
   {

   // Compute the inverse of the normal CDF for each grid point
#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<mn.nx(); x++) {
         for(int y=0; y<mn.ny(); y++) {
            double v;
            if(is_bad_data(mn.get(x,y)) ||
               is_bad_data(sd.get(x,y))) {
               v = bad_data_double;
            }
            else {
               v = normal_cdf_inv(area, mn.get(x,y), sd.get(x,y));
            }
            cdf_inv.set(v, x, y);
         }
      }
   } // End omp parallel

   return cdf_inv;
}

////////////////////////////////////////////////////////////////////////

DataPlane gradient(const DataPlane &dp, int dim, int delta) {
   DataPlane grad_dp(dp);

   if(dim != 0 && dim != 1) {
      mlog << Error << "\ngradient() -> "
           << "dimension must be set to 0 (x-dim) or 1 (y-dim)!\n\n";
      exit(1);
   }

   // Initialize to bad data values
   grad_dp.set_constant(bad_data_double);

#pragma omp parallel default(none) \
   shared(dp, dim, delta, grad_dp)
   {

      // Compute the gradient for each grid point
#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<dp.nx(); x++) {
         for(int y=0; y<dp.ny(); y++) {

            // dim: 0 for x-dimension, 1 for y-dimension
            int    x1 = (dim == 0 ? x+delta : x      );
            int    y1 = (dim == 0 ? y       : y+delta);
            double v1 = (x1 < 0 || x1 >= dp.nx() ||
                         y1 < 0 || y1 >= dp.ny() ?
                         bad_data_double : dp.get(x1, y1));
            double v  = dp.get(x, y);
            double gr = (is_bad_data(v1) || is_bad_data(v) ?
                         bad_data_double : v1 - v);
            grad_dp.set(gr, x, y);
         }
      }
   } // End omp parallel

   return grad_dp;
}

////////////////////////////////////////////////////////////////////////

int meijster_sep(int u_index, int i_index, double u_distance, double i_distance) {
   return ((u_index*u_index - i_index*i_index + u_distance*u_distance - i_distance*i_distance)
         / (2 * (u_index-i_index)));
}

////////////////////////////////////////////////////////////////////////

double euclide_distance(int x, int y) {
   return sqrt(x*x + y*y);
}

////////////////////////////////////////////////////////////////////////

DataPlane distance_map(const DataPlane &dp) {
   double distance_value;
   int nx = dp.nx();
   int ny = dp.ny();

   // Initialize to the maximum distance   
   DataPlane g_distance(dp);
   g_distance.set_constant(nx + ny);
   DataPlane dm(dp);
   dm.set_constant(nx + ny);
   
   int event_count = 0;

#pragma omp parallel default(none) \
   shared(dp, nx, ny, g_distance, dm, event_count) \
   private(distance_value)
   {

      // Meijster first phase
#pragma omp for schedule(static)
      for(int ix=0; ix<nx; ix++) {

         // Meijster scan 1
         int iy = 0;
         if(dp.get(ix, iy) > 0) {
            g_distance.set(0.0, ix, iy);
            event_count++;
         }
      
         for(iy=1; iy<ny; iy++) {
            if(dp.get(ix, iy) > 0) {
               distance_value = 0.0;
               event_count++;
            }
            else {
               distance_value = (1.0 + g_distance.get(ix, (iy-1)));
            }
            g_distance.set(distance_value, ix, iy);
         } // end for iy
      
         // Meijster scan 2
         for(iy=ny-2; iy>=0; iy--) {
            distance_value = g_distance.get(ix, (iy+1));
            if(g_distance.get(ix, iy) > distance_value) {
               g_distance.set((1.0 + distance_value), ix, iy);
            }
         } // end for iy
      } // end for ix

      // Meijster second phase
      if(event_count > 0) {

#pragma omp for schedule(static)
         for(int iy=0; iy<ny; iy++) {

            // Initialize s and t arrays
            int iq = 0;
            vector<int> s(nx, 0);
            vector<int> t(nx, 0);

            // Meijster Scan 3
            for(int ix=1; ix<nx; ix++) {
               while(iq >= 0 &&
                     euclide_distance((t[iq]-s[iq]),
                        nint(g_distance.get(s[iq], iy))) >
                     euclide_distance((t[iq]-ix),
                        nint(g_distance.get(ix, iy)))) iq--;

               if(iq < 0) {
                  iq = 0;
                  s[0] = ix;
               }
               else {
                  int iw = 1 + meijster_sep(ix, s[iq],
                                  g_distance.get(ix, iy),
                                  g_distance.get(s[iq], iy));
                  if(iw < nx) {
                     iq++;
                     s[iq] = ix;
                     t[iq] = iw;
                  }
               }
            } // end for ix

            // Meijster Scan 4
            for(int ix=nx-1; ix>=0; ix--) {
               distance_value = euclide_distance((ix-s[iq]),
                                   nint(g_distance.get(s[iq],iy)));
               dm.set(distance_value,ix,iy);
               if(ix == t[iq]) iq--;
            } // end for ix
         } // end for iy
      }
   } // End omp parallel

   // Mask the distance map with bad data values of the input field
   mask_bad_data(dm, dp);

   return dm;
}

////////////////////////////////////////////////////////////////////////

extern vector<double> radial_energy(const DataPlane &dp) {

   // Check for empty input
   if(dp.is_empty()) {
      mlog << Error << "\nradial_energy() -> "
           << "empty input!\n\n";
      exit(1);
   }

   // Define the number of bins as the smaller dimension
   int nx = dp.nx();
   int ny = dp.ny();
   int n_bins = min(nx, ny);
   vector<double> re(n_bins, 0.0);

   // Maximum euclidean distance from (0,0)
   double max_dist = sqrt((nx-1)*(nx-1) + (ny-1)*(ny-1));

   // Accumulate the radial energy for each bin
   for(int x=0; x<nx; x++) {
      for(int y=0; y<ny; y++) {

         // Energy is the value squared divided by the number of points
         double energy = dp(x,y)*dp(x,y)/dp.nxy();

         // Euclidean distance from (0,0)
         double dist = sqrt(x*x + y*y);

         // Map distance to a bin index [0, n_bins - 1]
         auto bin = (int)((dist / max_dist) * (n_bins - 1));

         // Accumulate energy for each bin
         if(bin < n_bins) re[bin] += energy;

      } // end for y
   } // end for x

   return re;
}

////////////////////////////////////////////////////////////////////////

