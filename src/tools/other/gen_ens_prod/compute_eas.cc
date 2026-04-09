// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include "compute_eas.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

static double compute_eas_dist(const std::vector<DataPlane> &,
                               const GridTemplate *, double, int, int,
                               double &);
static double compute_frac_cov(const DataPlane &,
                               const GridTemplate *, double, int, int);

////////////////////////////////////////////////////////////////////////

extern void compute_eas(const std::vector<DataPlane> &thresh_dp,
                        const EASProbInfo &eas_info,
                        const Grid &grid,
                        DataPlane &width_dp,
                        DataPlane &prob_dp,
                        DataPlane &smooth_dp) {

   // Check for empty input
   if(thresh_dp.empty()) return;

   const int n_eas = eas_info.width.n();

   // Initialize output
   width_dp.set_size(grid.nx(), grid.ny());
   prob_dp.set_size(grid.nx(), grid.ny());

   // Process each grid point
#pragma omp parallel default(none) \
   shared(thresh_dp, width_dp, prob_dp) \
   shared(grid, n_eas, eas_info)
   {

      // Build GridTemplate for each EAS width
      GridTemplateFactory gtf;
      vector<GridTemplate *> eas_gt;
      for(int i_eas=0; i_eas<n_eas; i_eas++) {
         eas_gt.emplace_back(gtf.buildGT(eas_info.shape,
                                         eas_info.width[i_eas],
                                         grid.wrap_lon()));
      }

#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<grid.nx(); x++) {
         for(int y=0; y<grid.ny(); y++) {

            // Loop over candidate EAS widths
            for(int i_eas=0; i_eas<n_eas; i_eas++) {

               double frac_cov_mean;
               double dist_mean = compute_eas_dist(thresh_dp,
                                     eas_gt[i_eas],
                                     eas_info.vld_thresh,
                                     x, y, frac_cov_mean);

               // Check for bad data
               if(is_bad_data(dist_mean)) {
                  width_dp.set(bad_data_double, x, y);
                  prob_dp.set(bad_data_double, x, y);
                  break;
               }
               // Check for average distance less than alpha or the last width
               else if(dist_mean <= eas_info.alpha ||
                       i_eas+1 == n_eas) {
                  width_dp.set((double) eas_gt[i_eas]->getWidth(), x, y);
                  prob_dp.set(frac_cov_mean, x, y);
                  break;
               }
            } // end for i_eas
         } // end for y
      } // end for x

      // Clean up
      for(auto &gt : eas_gt) delete gt;

   } // end of omp parallel

   // Apply the Gaussian smoother
   smooth_dp = prob_dp; 
   interp_gaussian_dp(smooth_dp, eas_info.gaussian, eas_info.vld_thresh);

   return;
}

////////////////////////////////////////////////////////////////////////

static double compute_eas_dist(const std::vector<DataPlane> &thresh_dp,
                               const GridTemplate *gt, double vld_t,
                               int x, int y,
                               double &frac_cov_mean) {
   NumArray frac_cov;
   frac_cov.extend((int) thresh_dp.size());

   // Compute fractional coverage for each ensemble member
   for(auto &dp : thresh_dp) {
      frac_cov.add(compute_frac_cov(dp, gt, vld_t, x, y));
   }

   // Compute the average fractional coverage distance
   int n_dist = 0;
   double dist_sum = 0.0;
   for(int i=0; i<frac_cov.n(); i++) {
      for(int j=i+1; j<frac_cov.n(); j++) {

         double v1 = frac_cov.buf()[i];
         double v2 = frac_cov.buf()[j];

         // Check for bad data
         if(is_bad_data(v1) || is_bad_data(v2)) continue;

         // Increment distance sum
         if(is_eq(v1, 0.0) || is_eq(v2, 0.0)) {
            dist_sum += 1.0;
         }
         else {
            dist_sum += ((v1 - v2)*(v1 - v2))/(v1*v1 + v2*v2);
         }

         // Increment distance counter
         n_dist++;

      } // end for j
   } // end for i

   // Compute the mean fractional coverage
   frac_cov_mean = frac_cov.mean();

   // Compute the mean fractional coverage distance
   double dist_mean = (n_dist > 0 ? dist_sum/n_dist : bad_data_double);

   return dist_mean;
}

////////////////////////////////////////////////////////////////////////

static double compute_frac_cov(const DataPlane &dp,
                               const GridTemplate *gt, double vld_t,
                               int x, int y) {

   // Initialize counts
   int n_vld = 0;
   double sum = 0.0;

   // Sum the neighborhood points
   for(GridPoint *gp = gt->getFirstInGrid(x, y, dp.nx(), dp.ny());
       gp != nullptr;
       gp = gt->getNextInGrid()) {

      double v = dp.get(gp->x, gp->y);
      if(!is_bad_data(v)) {
         n_vld++;
         sum += v;
      }
   }

   // Check for enough valid data and compute fractional coverage
   double frac_cov = bad_data_double;
   if((double) (n_vld)/gt->size() >= vld_t && n_vld != 0) {
      frac_cov = sum/n_vld;
   }

   return frac_cov;
}

////////////////////////////////////////////////////////////////////////

