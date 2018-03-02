// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
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
                  InterpMthd mthd, int width, const GridTemplateFactory::GridTemplates shape, double t) {
   double v;
   int x, y;

   // Initialize the smoothed field to the raw field
   smooth_dp = dp;

   // Check that grid template is at least 1 point
   if(width == 1 || mthd == InterpMthd_Nearest) return;

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

            case(InterpMthd_Min):     // Minimum
               v = interp_min(dp, *gt, x, y, t);
               break;

            case(InterpMthd_Max):     // Maximum
               v = interp_max(dp, *gt, x, y, t);
               break;

            case(InterpMthd_Median):  // Median
               v = interp_median(dp, *gt, x, y, t);
               break;

            case(InterpMthd_UW_Mean): // Unweighted Mean
               v = interp_uw_mean(dp, *gt, x, y, t);
               break;

            // Distance-weighted mean, least-squares fit, and bilinear
            // interpolation are omitted here since they are not
            // options for gridded data

            default:
               mlog << Error << "\nsmooth_field() -> "
                    << "unexpected interpolation method encountered: "
                    << mthd << "\n\n";
               exit(1);
               break;
         }

         // Store the smoothed value
         smooth_dp.set(v, x, y);

      } // end for y
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

void fractional_coverage(const DataPlane &dp, DataPlane &frac_dp,
                          int width, const GridTemplateFactory::GridTemplates shape, SingleThresh t, double vld_t) {

   // Check that width is set to 1 or greater
   if(width < 1) {
      mlog << Error << "\nfractional_coverage() -> "
           << "Grid must have at one point in it. \n\n";
      exit(1);
   }

   // Build the grid template
   GridTemplateFactory gtf;
   GridTemplate* gt = gtf.buildGT(shape, width);

   mlog << Debug(3)
        << "Computing fractional coverage field using the " << t.get_str()
        << " threshold and the " << interpmthd_to_string(InterpMthd_Nbrhd)
      << "(" << gt->size() << ")  " << gt->getClassName() << " interpolation method.\n";


   // Initialize the fractional coverage field
   frac_dp = dp;
   frac_dp.set_constant(bad_data_double);

   // Compute the fractional coverage meeting the threshold criteria
   for(int x=0; x<dp.nx(); x++) {
      for(int y=0; y<dp.ny(); y++) {

         int count_vld = 0;
         int count_thr = 0;

         GridPoint *gp = NULL;
         for(gp = gt->getFirstInGrid(x, y, dp.nx(), dp.ny() );
             gp != NULL; gp = gt->getNextInGrid()){

            double v = dp.get(gp->x,gp->y);

            if (is_bad_data(v)) continue;

            count_vld++;

            if(t.check(v)) {
               count_thr++;
            }
         }

         // Check whether enough valid grid points were found or leave it as bad_data
         if( static_cast<double>(count_vld)/gt->size() >= vld_t &&
             count_vld != 0) {

            // Compute and store the fractional coverage
            double frac = (double) count_thr/count_vld;
            frac_dp.set(frac, x, y);
         }
      }
   }

   delete gt;
}

////////////////////////////////////////////////////////////////////////
//
// Convert the DataPlane field to the corresponding fractional coverage
// using the threshold critea specified.
//
////////////////////////////////////////////////////////////////////////

void fractional_coverage(const DataPlane &dp, DataPlane &frac_dp,
                         int wdth, SingleThresh t, double vld_t) {
   int i, j, k, n, x, y, x_ll, y_ll, y_ur, xx, yy, half_width;
   double v;
   int count_vld, count_thr;
   NumArray box_na;

   mlog << Debug(3)
        << "Computing fractional coverage field using the " << t.get_str()
        << " threshold and the " << interpmthd_to_string(InterpMthd_Nbrhd)
        << "(" << wdth*wdth << ") interpolation method.\n";

   // Check that width is set to 1 or greater
   if(wdth < 1) {
      mlog << Error << "\nfractional_coverage() -> "
           << "width must be set to a value of 1 or greater.\n\n";
      exit(1);
   }

   // Initialize the fractional coverage field
   frac_dp = dp;
   frac_dp.set_constant(bad_data_double);

   // Compute the box half-width
   half_width = (wdth - 1)/2;

   // Initialize the box
   for(i=0; i<wdth*wdth; i++) box_na.add(bad_data_int);

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

            for(i=0; i<wdth; i++) {

               xx = x_ll + i;

               for(j=0; j<wdth; j++) {

                  yy = y_ll + j;

                  n = DefaultTO.two_to_one(wdth, wdth, i, j);

                  // Check for being off the grid
                  if(xx < 0 || xx >= dp.nx() ||
                     yy < 0 || yy >= dp.ny()) {
                     k = bad_data_int;
                  }
                  // Check the value of v to see if it meets the threshold criteria
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
            j = (y - 1) % wdth;

            for(i=0; i<wdth; i++) {

               // Index into the box
               n = DefaultTO.two_to_one(wdth, wdth, i, j);

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
               // Check the value of v to see if it meets the threshold criteria
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
         if((double) count_vld/(wdth*wdth) < vld_t ||
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

void apply_mask(const DataPlane &dp, const DataPlane &mask_dp,
                NumArray &na) {
   int x, y;
   double v;

   // Initialize the NumArray object
   na.erase();

   // Loop through the mask data points
   for(x=0; x<mask_dp.nx(); x++) {
      for(y=0; y<mask_dp.ny(); y++) {

         // Skip points where the mask is off
         if(is_bad_data(mask_dp.get(x, y)) ||
            !mask_dp.s_is_on(x, y)) continue;

         // Get the current value
         v = (dp.nx() == 0 && dp.ny() == 0 ?
              bad_data_double : dp.get(x, y));

         // Store the value
         na.add(v);
      } // end for y
   } // end for x

   return;
}

///////////////////////////////////////////////////////////////////////////////

void apply_mask(DataPlane &dp, const DataPlane &mask_dp) {
   int x, y;

   if(dp.nx() != mask_dp.nx() ||
      dp.ny() != mask_dp.ny() ) {

      mlog << Error << "\napply_mask() -> "
           << "grid dimensions do not match\n\n";
      exit(1);
   }

   for(x=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         // Put bad data everywhere the mask is bad data or turned off
         if(is_bad_data(mask_dp(x, y)) || !mask_dp(x, y)) dp.set(bad_data_double, x, y);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void mask_bad_data(DataPlane &dp, const DataPlane &mask_dp, double v) {
   int x, y;

   if(dp.nx() != mask_dp.nx() ||
      dp.ny() != mask_dp.ny() ) {

      mlog << Error << "\nmask_bad_data() -> "
           << "grid dimensions do not match\n\n";
      exit(1);
   }

   for(x=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         if(is_bad_data(mask_dp.get(x, y)))
            dp.set(v, x, y);
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
           << "requested area (" << area << ") must be between 0 and 1.\n\n";
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

DataPlane gradient(const DataPlane &dp, int dim) {
   int x, y, x1, y1;
   double v, v1, gr;
   DataPlane grad_dp;

   if(dim != 0 && dim != 1) {
      mlog << Error << "\ngradient() -> "
           << "dimension must be set to 0 (x-dim) or 1 (y-dim)!\n\n";
      exit(1);
   }

   // Initialize to bad data values
   grad_dp.set_size(dp.nx(), dp.ny());

   for(x=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         // dim: 0 for x-dimension, 1 for y-dimension
         x1 = (dim == 0 ? x+1 : x  );
         y1 = (dim == 0 ? y   : y+1);
         v1 = (x1 >= dp.nx() || y1 >= dp.ny() ?
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
