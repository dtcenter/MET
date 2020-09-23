// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdlib>
#include <iostream>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "data_plane_util.h"
#include "interp_util.h"
#include "two_to_one.h"

#include "vx_gsl_prob.h"
#include "vx_math.h"
#include "vx_log.h"

#include "GridTemplate.h"


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
   double v, min_v, max_v;
   int x, y;

   //
   // Get the range of data values in the field
   //
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

      //
      // Divide each value by 100
      //
      for(x=0; x<dp.nx(); x++) {
         for(y=0; y<dp.ny(); y++) {

            v = dp.get(x, y);
            if(!is_bad_data(v)) dp.set(v/100.0, x, y);

         } // end for y
      } // end for x

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
                  double t, const GaussianInfo &gaussian) {
   double v = 0.0;
   int x, y;

   // Initialize the smoothed field to the raw field
   smooth_dp = dp;

   // For nearest neighbor, no work to do.
   if(width == 1 && mthd == InterpMthd_Nearest) return;

   // build the grid template
   GridTemplateFactory gtf;
   GridTemplate* gt = gtf.buildGT(shape, width);

   mlog << Debug(3)
        << "Smoothing field using the " << interpmthd_to_string(mthd)
        << "(" << gt->size() << ") " << gt->getClassName()
        << " interpolation method.\n";

   // Otherwise, apply smoothing to each grid point
   for(x=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         // Compute the smoothed value based on the interpolation method
         switch(mthd) {

            case(InterpMthd_Min):      // Minimum
               v = interp_min(dp, *gt, x, y, t);
               break;

            case(InterpMthd_Max):      // Maximum
               v = interp_max(dp, *gt, x, y, t);
               break;

            case(InterpMthd_Median):   // Median
               v = interp_median(dp, *gt, x, y, t);
               break;

            case(InterpMthd_UW_Mean):  // Unweighted Mean
               v = interp_uw_mean(dp, *gt, x, y, t);
               break;

            case(InterpMthd_Gaussian): // For Gaussian, pass the data through
               v = dp.get(x, y);
               break;

            case(InterpMthd_MaxGauss): // For Max Gaussian, compute the max
               v = interp_max(dp, *gt, x, y, 0);
               break;

            // Distance-weighted mean, area-weighted mean, least-squares
            // fit, and bilinear are omitted here since they are not
            // options for gridded data.

            default:
               mlog << Error << "\nsmooth_field() -> "
                    << "unsupported interpolation method encountered: "
                    << interpmthd_to_string(mthd) << "(" << mthd
                    << ")\n\n";
               exit(1);
               break;
         }

         // Store the smoothed value
         smooth_dp.set(v, x, y);

      } // end for y
   } // end for x

   // Apply the Gaussian smoother 
   if(mthd == InterpMthd_Gaussian ||
      mthd == InterpMthd_MaxGauss) {
      interp_gaussian_dp(smooth_dp, gaussian, t);
   }

   // Cleanup
   delete gt;

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
                       double t, const GaussianInfo &gaussian) {
   DataPlane smooth_dp;

   smooth_field(dp, smooth_dp, mthd, width, shape, t, gaussian);

   return(smooth_dp);
}

////////////////////////////////////////////////////////////////////////
//
// Convert the DataPlane field to the corresponding fractional coverage
// using the threshold critea specified.
//
////////////////////////////////////////////////////////////////////////

void fractional_coverage(const DataPlane &dp, DataPlane &frac_dp,
        int width, const GridTemplateFactory::GridTemplates shape,
        SingleThresh t, double vld_t) {
   GridPoint *gp = NULL;
   int x, y;
   int n_vld = 0;
   int n_thr = 0;
   double v;

   // Check that width is set to 1 or greater
   if(width < 1) {
      mlog << Error << "\nfractional_coverage() -> "
           << "Grid must have at least one point in it. \n\n";
      exit(1);
   }

   // Build the grid template
   GridTemplateFactory gtf;
   GridTemplate* gt = gtf.buildGT(shape, width);

   mlog << Debug(3)
        << "Computing fractional coverage field using the "
        << t.get_str() << " threshold and the "
        << interpmthd_to_string(InterpMthd_Nbrhd) << "(" << gt->size()
        << ") " << gt->getClassName() << " interpolation method.\n";

   // Initialize the fractional coverage field
   frac_dp = dp;
   frac_dp.set_constant(bad_data_double);

   // Compute the fractional coverage meeting the threshold criteria
   for(x=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         // For a new column, reset the grid template and counts.
         if(y == 0) {

            // Initialize counts
            n_vld = n_thr = 0;

            // Sum all the points
            for(gp  = gt->getFirstInGrid(x, y, dp.nx(), dp.ny());
                gp != NULL;
                gp  = gt->getNextInGrid()) {
               if(is_bad_data(v = dp.get(gp->x, gp->y))) continue;
               n_vld++;
               if(t.check(v)) n_thr++;
            }
         }
         // Subtract off the bottom edge, shift up, and add the top.
         else {

            // Subtract points from the the bottom edge
            for(gp  = gt->getFirstInBotEdge();
                gp != NULL;
                gp  = gt->getNextInBotEdge()) {
               if(is_bad_data(v = dp.get(gp->x, gp->y))) continue;
               n_vld--;
               if(t.check(v)) n_thr--;
            }

            // Increment Y
            gt->incBaseY(1);

            // Add points from the the top edge
            for(gp  = gt->getFirstInTopEdge();
                gp != NULL;
                gp  = gt->getNextInTopEdge()) {
               if(is_bad_data(v = dp.get(gp->x, gp->y))) continue;
               n_vld++;
               if(t.check(v)) n_thr++;
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

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Convert the DataPlane field to the corresponding fractional coverage
// using the threshold critea specified.
//
////////////////////////////////////////////////////////////////////////

void fractional_coverage_square(const DataPlane &dp, DataPlane &frac_dp,
        int width, SingleThresh t, double vld_t) {
   int i, j, k, n, x, y, x_ll, y_ll, y_ur, xx, yy, half_width;
   double v;
   int count_vld = 0;
   int count_thr = 0;
   NumArray box_na;

   mlog << Debug(3)
        << "Computing fractional coverage field using the "
        << t.get_str() << " threshold and the "
        << interpmthd_to_string(InterpMthd_Nbrhd)
        << "(" << width*width << ") interpolation method.\n";

   // Check that width is set to 1 or greater
   if(width < 1) {
      mlog << Error << "\nfractional_coverage_square() -> "
           << "width must be set to a value of 1 or greater.\n\n";
      exit(1);
   }

   // Initialize the fractional coverage field
   frac_dp = dp;
   frac_dp.set_constant(bad_data_double);

   // Compute the box half-width
   half_width = (width - 1)/2;

   // Initialize the box
   for(i=0; i<width*width; i++) box_na.add(bad_data_int);

   // Compute the fractional coverage meeting the threshold criteria
   for(x=0; x<dp.nx(); x++) {

      // Find the lower-left x-coordinate of the neighborhood
      x_ll = x - half_width;

      for(y=0; y<dp.ny(); y++) {

         // Find the lower-left y-coordinate of the neighborhood
         y_ll = y - half_width;
         y_ur = y + half_width;

         // Initialize the box for this new column
         if(y == 0) {

            // Initialize counts
            count_vld = count_thr = 0;

            for(i=0; i<width; i++) {

               xx = x_ll + i;

               for(j=0; j<width; j++) {

                  yy = y_ll + j;

                  n = DefaultTO.two_to_one(width, width, i, j);

                  // Check for being off the grid
                  if(xx < 0 || xx >= dp.nx() ||
                     yy < 0 || yy >= dp.ny()) {
                     k = bad_data_int;
                  }
                  // Check v to see if it meets the threshold criteria
                  else {
                     v = dp.get(xx, yy);
                     if(is_bad_data(v))  k = bad_data_int;
                     else if(t.check(v)) k = 1;
                     else                k = 0;
                  }
                  box_na.set(n, k);

                  // Increment the counts
                  if(!is_bad_data(k)) {
                     count_vld += 1;
                     count_thr += k;
                  }

               } // end for j
            } // end for i
         } // end if

         // Otherwise, update one row of the box
         else {

            // Compute the row of the neighborhood box to be updated
            j = (y - 1) % width;

            for(i=0; i<width; i++) {

               // Index into the box
               n = DefaultTO.two_to_one(width, width, i, j);

               // Get x and y values to be checked
               xx = x_ll + i;
               yy = y_ur;

               // Decrement counts for data to be replaced
               k = nint(box_na[n]);
               if(!is_bad_data(k)) {
                  count_vld -= 1;
                  count_thr -= k;
               }

               // Check for being off the grid
               if(xx < 0 || xx >= dp.nx() ||
                  yy < 0 || yy >= dp.ny()) {
                  k = bad_data_int;
               }
               // Check v to see if it meets the threshold criteria
               else {
                  v = dp.get(xx, yy);
                  if(is_bad_data(v))  k = bad_data_int;
                  else if(t.check(v)) k = 1;
                  else                k = 0;
               }
               box_na.set(n, k);

               // Increment the counts
               if(!is_bad_data(k)) {
                  count_vld += 1;
                  count_thr += k;
               }

            } // end for i
         } // end else

         // Check whether enough valid grid points were found
         if((double) count_vld/(width*width) < vld_t ||
            count_vld == 0) {
            v = bad_data_double;
         }
         // Compute the fractional coverage
         else {
            v = (double) count_thr/count_vld;
         }

         // Store the fractional coverage value
         frac_dp.set(v, x, y);

      } // end for y
   } // end for x

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
           << "grid dimensions do not match\n\n";
      exit(1);
   }

   // Initialize the NumArray object
   na.erase();

   int Nxy = mask.nx() * mask.ny();

   for(int i=0; i<Nxy; i++) {

      // Store the values where the mask in on
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
           << "grid dimensions do not match\n\n";
      exit(1);
   }

   int Nxy = mask.nx() * mask.ny();

   for(int i=0; i<Nxy; i++) {
      if(!is_bad_data(in.data()[i]) && !mask.data()[i]) {
         in.buf()[i] = bad_data_double;
      }
   }

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
           << "grid dimensions do not match\n\n";
      exit(1);
   }

   int Nxy = mask.nx() * mask.ny();

   for(int i=0; i<Nxy; i++) {
      if(!mask.data()[i]) in.buf()[i] = false;
   }

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
           << "grid dimensions do not match\n\n";
      exit(1);
   }

   int Nxy = mask_dp.nx() * mask_dp.ny();

   for(int i=0; i<Nxy; i++) {
      if(is_bad_data(mask_dp.data()[i])) {
         dp.buf()[i] = v;
      }
   }

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
           << "grid dimensions do not match\n\n";
      exit(1);
   }

   int Nxy = dp.nx() * dp.ny();

   for(int i=0; i<Nxy; i++) {
      if(is_bad_data(dp.data()[i])) {
         mp.buf()[i] = false;
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

DataPlane subtract(const DataPlane &dp1, const DataPlane &dp2) {
   DataPlane diff = dp1;
   double v;

   if(dp1.nx() != dp2.nx() || dp1.ny() != dp2.ny()) {
      mlog << Error << "\nsubtract() -> "
           << "grid dimensions do not match\n\n";
      exit(1);
   }

   for(int x=0; x<dp1.nx(); x++) {
      for(int y=0; y<dp1.ny(); y++) {
         v = (is_bad_data(dp1.get(x,y)) || is_bad_data(dp2.get(x,y)) ?
              bad_data_double : dp1.get(x,y) - dp2.get(x,y));
         diff.set(v, x, y);
      }
   }

   return(diff);
}

////////////////////////////////////////////////////////////////////////

DataPlane normal_cdf(const DataPlane &dp, const DataPlane &mn,
                     const DataPlane &sd) {
   DataPlane cdf = dp;
   double v;

   // Check grid dimensions
   if(dp.nx() != mn.nx() || dp.ny() != mn.ny() ||
      dp.nx() != sd.nx() || dp.ny() != sd.ny()) {
      mlog << Error << "\nnormal_cdf() -> "
           << "grid dimensions do not match\n\n";
      exit(1);
   }

   // Compute the normal CDF for each grid point
   for(int x=0; x<dp.nx(); x++) {
      for(int y=0; y<dp.ny(); y++) {
         v = (is_bad_data(dp.get(x,y)) ||
              is_bad_data(mn.get(x,y)) ||
              is_bad_data(sd.get(x,y)) ?
              bad_data_double :
              normal_cdf(dp.get(x,y), mn.get(x,y), sd.get(x,y)));
         cdf.set(v, x, y);
      }
   }

   return(cdf);
}

////////////////////////////////////////////////////////////////////////

DataPlane normal_cdf_inv(const double area, const DataPlane &mn,
                         const DataPlane &sd) {
   DataPlane cdf_inv = mn;
   double v;

   // Check grid dimensions
   if(mn.nx() != sd.nx() || mn.ny() != sd.ny()) {
      mlog << Error << "\nnormal_cdf_inv() -> "
           << "grid dimensions do not match\n\n";
      exit(1);
   }

   // Range check area value
   if(area <= 0.0 || area >= 1.0) {
      mlog << Error << "\nnormal_cdf_inv() -> "
           << "requested area (" << area
           << ") must be between 0 and 1.\n\n";
      exit(1);
   }

   // Compute the inverse of the normal CDF for each grid point
   for(int x=0; x<mn.nx(); x++) {
      for(int y=0; y<mn.ny(); y++) {
         v = (is_bad_data(mn.get(x,y)) ||
              is_bad_data(sd.get(x,y)) ?
              bad_data_double :
              normal_cdf_inv(area, mn.get(x,y), sd.get(x,y)));
         cdf_inv.set(v, x, y);
      }
   }

   return(cdf_inv);
}

////////////////////////////////////////////////////////////////////////

DataPlane gradient(const DataPlane &dp, int dim, int delta) {
   int x, y, x1, y1;
   double v, v1, gr;
   DataPlane grad_dp;

   if(dim != 0 && dim != 1) {
      mlog << Error << "\ngradient() -> "
           << "dimension must be set to 0 (x-dim) or 1 (y-dim)!\n\n";
      exit(1);
   }

   // Initialize to bad data values
   grad_dp = dp;
   grad_dp.set_constant(bad_data_double);

   for(x=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         // dim: 0 for x-dimension, 1 for y-dimension
         x1 = (dim == 0 ? x+delta : x  );
         y1 = (dim == 0 ? y       : y+delta);
         v1 = (x1 < 0 || x1 >= dp.nx() ||
               y1 < 0 || y1 >= dp.ny() ?
               bad_data_double : dp.get(x1, y1));
         v  = dp.get(x, y);
         gr = (is_bad_data(v1) || is_bad_data(v) ?
               bad_data_double : v1 - v);
         grad_dp.set(gr, x, y);
      }
   }

   return(grad_dp);
}

////////////////////////////////////////////////////////////////////////

int meijster_sep(int u_index, int i_index, double u_distance, double i_distance) {
   return ((u_index*u_index - i_index*i_index + u_distance*u_distance - i_distance*i_distance)
         / (2 * (u_index-i_index)));
}

double euclide_distance(int x, int y) {
   return sqrt(x*x + y*y);
}

////////////////////////////////////////////////////////////////////////

DataPlane distance_map(const DataPlane &dp) {
   DataPlane g_distance, dm;
   bool debug_g_distance = false;
   double distance_value;
   int ix, iy;
   int nx = dp.nx();
   int ny = dp.ny();
   int max_distance = nx + ny;

   // Initialize to the maximum distance   
   g_distance = dp;
   g_distance.set_constant(max_distance);
   dm = dp;
   dm.set_constant(max_distance);
   
   int event_count = 0;
   // Meijster first phase
   for (ix=0; ix<nx; ix++) {
      // Meijster scan 1
      iy = 0;
      if (0 < dp.get(ix, iy)) {
         g_distance.set(0.0, ix, iy);
         event_count++;
      }
      
      for (iy = 1; iy<ny; iy++) {
         if (0 < dp.get(ix, iy)) {
            distance_value = 0.0;
            event_count++;
         }
         else {
            distance_value = (1.0 + g_distance.get(ix, (iy-1)));
         }
         g_distance.set(distance_value, ix, iy);
      }
      
      // Meijster scan 2
      for (iy = ny-2; iy>=0; iy--) {
         distance_value = g_distance.get(ix, (iy+1));
         if (distance_value < g_distance.get(ix, iy)) {
            g_distance.set((1.0 + distance_value), ix, iy);
         }
      }
   }
   
   // Meijster second phase
   if (0 < event_count) {
      int iq, iw;
      int s[nx], t[nx];
      
      // Initialize s and t array
      for (ix=0; ix<nx; ix++) {
         s[ix] = t[ix] = 0;
      }
      
      for (iy = 0; iy<ny; iy++) {
         iq = 0;
         s[iq] = t[iq] = 0;
      
         // Meijster Scan 3
         for (ix=1; ix<nx; ix++) {
            while ((0 <= iq)
                 && euclide_distance((t[iq]-s[iq]), g_distance.get(s[iq], iy))
                    > euclide_distance((t[iq]-ix), g_distance.get(ix, iy)))
               iq--;
                
            if (0 > iq) {
               iq = 0;
               s[0] = ix;
            }
            else {
               iw = 1 + meijster_sep(ix, s[iq],
                     g_distance.get(ix, iy), g_distance.get(s[iq], iy));
               if (iw < nx) {
                  iq++;
                  s[iq] = ix;
                  t[iq] = iw;
               }
            }
         }
         
         // Meijster Scan 4
         for (ix=nx-1; ix>=0; ix--) {
            distance_value = euclide_distance((ix-s[iq]), g_distance.get(s[iq],iy));
            dm.set(distance_value,ix,iy);
            if (ix == t[iq]) iq--;
         }
      }
   }
   
   int debug_level = 7;
   if(mlog.verbosity_level() >= debug_level) {
      if (debug_g_distance) {
         for (ix=0; ix<nx; ix++) {
            ConcatString message;
            message << " g_distance: " ;
            for (iy = 0; iy<ny; iy++) {
               message << "  " << g_distance.get(ix, iy);
            }
            mlog << Debug(debug_level) << message << "\n";
         }
      }
      for (ix=0; ix<nx; ix++) {
         ConcatString message;
         message << " distance: " ;
         for (iy = 0; iy<ny; iy++) {
            message << "  " << dm.get(ix, iy);
         }
         mlog << Debug(debug_level) << message << "\n";
      }
   }

   // Mask the distance map with bad data values of the input field
   mask_bad_data(dm, dp);

   return(dm);
}

////////////////////////////////////////////////////////////////////////

