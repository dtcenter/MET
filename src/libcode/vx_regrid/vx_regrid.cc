// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include "vx_regrid.h"
#include "interp_mthd.h"
#include "GridTemplate.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

DataPlane met_regrid(const DataPlane & in, const Grid & from_grid,
                     const Grid & to_grid, const RegridInfo & info) {

   DataPlane out;

   switch(info.method) {
      case InterpMthd::Min:
      case InterpMthd::Max:
      case InterpMthd::Median:
      case InterpMthd::UW_Mean:
      case InterpMthd::DW_Mean:
      case InterpMthd::LS_Fit:
      case InterpMthd::Bilin:
      case InterpMthd::Nearest:
      case InterpMthd::Upper_Left:
      case InterpMthd::Upper_Right:
      case InterpMthd::Lower_Right:
      case InterpMthd::Lower_Left:
         out = met_regrid_generic (in, from_grid, to_grid, info);
         break;

      case InterpMthd::Budget:
         out = met_regrid_budget (in, from_grid, to_grid, info);
         break;

      case InterpMthd::AW_Mean:
      case InterpMthd::AW_Mean_Cntr:
         out = met_regrid_area_weighted (in, from_grid, to_grid, info);
         break;

      case InterpMthd::Force:
         out = met_regrid_force (in, from_grid, to_grid, info);
         break;

      case InterpMthd::MaxGauss:
         out = met_regrid_maxgauss (in, from_grid, to_grid, info);
         break;

      default:
         mlog << Error << "\nmet_regrid() -> "
              << "bad interpolation method ... "
              << interpmthd_to_string(info.method) << "\n\n";
         exit(1);

   } // switch info.method

   // apply conversion logic
   out.convert(info.convert_fx);

   // apply censor logic
   out.censor(info.censor_thresh, info.censor_val);

   return out;
}

////////////////////////////////////////////////////////////////////////

DataPlane met_regrid_nearest(const DataPlane & from_data,
                             const Grid & from_grid,
                             const Grid & to_grid) {
   RegridInfo ri;
   ri.enable = true;
   ri.method = InterpMthd::Nearest;
   ri.width  = 1;
   ri.shape  = GridTemplateFactory::GridTemplates::Square;

   return met_regrid_generic(from_data, from_grid, to_grid, ri);
}

////////////////////////////////////////////////////////////////////////

DataPlane met_regrid_generic(const DataPlane & from_data,
                             const Grid & from_grid,
                             const Grid & to_grid,
                             const RegridInfo & info) {
   DataPlane to_data;

#pragma omp parallel default(none) \
   shared(from_data, from_grid, to_grid, info, to_data) 
   {

#pragma omp single
      {
         // Set the size and timing info
         to_data.set_size (to_grid.nx(), to_grid.ny());
         to_data.set_times(from_data);
      }

#pragma omp for schedule(static) \
                collapse(2)
      for(int xt=0; xt<(to_grid.nx()); xt++) {
         for(int yt=0; yt<(to_grid.ny()); yt++) {

            double lat;
            double lon;
            to_grid.xy_to_latlon(xt, yt, lat, lon);

            double x_from;
            double y_from;
            from_grid.latlon_to_xy(lat, lon, x_from, y_from);

            int xf = nint(x_from);
            int yf = nint(y_from);

            double value;
            if(((xf < 0 || xf >= from_grid.nx()) && !from_grid.wrap_lon()) ||
                 yf < 0 || yf >= from_grid.ny()) {
                value = bad_data_float;
            }
            else {
               value = compute_horz_interp(from_data, x_from, y_from,
                          bad_data_double, info.method, info.width,
                          info.shape, from_grid.wrap_lon(), info.vld_thresh);
            }

            to_data.put(value, xt, yt);

         } // for yt
      } // for xt
   } // End of omp parallel

   return to_data;
}

////////////////////////////////////////////////////////////////////////
//
// For AW_MEAN_CNTR, compute the fraction of the 1D extent [lo, hi],
// in to_grid index units, which overlaps each to_grid box, where box i
// spans [i-0.5, i+0.5]. Returns the number of boxes stored in the index
// and weight arrays, or 0 if the extent spans more than max_aw_boxes.
//
////////////////////////////////////////////////////////////////////////

static const int max_aw_boxes = 16;

static int aw_mean_overlap(double lo, double hi,
                           int i[max_aw_boxes], double w[max_aw_boxes]) {
   static const double min_width = 1.0e-5;

   // Treat a degenerate extent as a point
   if(hi - lo < min_width) {
      i[0] = nint(0.5*(lo + hi)); w[0] = 1.0;
      return 1;
   }

   int i_beg = nint(lo);
   int i_end = nint(hi);
   if(i_end - i_beg + 1 > max_aw_boxes) return 0;

   int n = 0;
   for(int k=i_beg; k<=i_end; k++) {
      double overlap = min(hi, k + 0.5) - max(lo, k - 0.5);
      if(overlap <= 0.0) continue;
      i[n] = k;
      w[n] = overlap / (hi - lo);
      n++;
   }

   return n;
}

////////////////////////////////////////////////////////////////////////

DataPlane met_regrid_area_weighted(const DataPlane & from_data,
                                   const Grid & from_grid,
                                   const Grid & to_grid,
                                   const RegridInfo & info) {

   //
   //  The interpolation width and shape do not apply here.  The output
   //  value for each to_grid box is computed as a weighted average of
   //  the from_grid data points falling inside that box, where the
   //  weights are determined by the area of the from_grid boxes.
   //

   DataPlane to_data;
   vector<double> to_data_sum(to_grid.nxy(), 0.0);
   vector<double> wt_data_sum(to_grid.nxy(), 0.0);

   //
   // MET #3206 Reduction of vectors needed to prevent data races
   //           when updating to_data values 
   //

#pragma omp declare reduction(vec_dbl_plus : vector<double> :             \
                              transform(omp_out.begin(), omp_out.end(),   \
                                         omp_in.begin(), omp_out.begin(), \
                                        plus<double>()))                  \
                    initializer(omp_priv = decltype(omp_orig)(omp_orig.size()))

#pragma omp parallel default(none) \
   shared(from_data, from_grid, to_grid, info, to_data) \
   shared(to_data_sum, wt_data_sum)
   { 

#pragma omp single
      {
         // Set the size and timinig info
         to_data.set_size (to_grid.nx(), to_grid.ny(), 0.0);
         to_data.set_times(from_data);
      }

      // Loop over the from grid to accumulate sums and area weights
#pragma omp for schedule(static) \
                collapse(2) \
                reduction(vec_dbl_plus : to_data_sum, wt_data_sum)
      for(int xf=0; xf<(from_grid.nx()); xf++) {
         for(int yf=0; yf<(from_grid.ny()); yf++) {

            double value = from_data(xf, yf);
            if(is_bad_data(value)) continue;

            double lat;
            double lon;
            from_grid.xy_to_latlon(xf, yf, lat, lon);

            double x_to;
            double y_to;
            to_grid.latlon_to_xy(lat, lon, x_to, y_to);

            bool centered = (info.method == InterpMthd::AW_Mean_Cntr);
            double weight = from_grid.calc_area(xf, yf, centered);

            int xt[max_aw_boxes], yt[max_aw_boxes];
            double xw[max_aw_boxes], yw[max_aw_boxes];
            int n_xt = 0;
            int n_yt = 0;

            if(centered) {

               // Map the from_grid box edge midpoints to the to_grid
               const double dx[4] = { -0.5, 0.5, 0.0, 0.0 };
               const double dy[4] = { 0.0, 0.0, -0.5, 0.5 };
               double x_min = x_to, x_max = x_to;
               double y_min = y_to, y_max = y_to;
               for(int k=0; k<4; k++) {
                  double lat_e, lon_e, x_e, y_e;
                  from_grid.xy_to_latlon(xf + dx[k], yf + dy[k], lat_e, lon_e);
                  to_grid.latlon_to_xy(lat_e, lon_e, x_e, y_e);

                  // Unwrap relative to the box center
                  if(to_grid.wrap_lon()) {
                     while(x_e - x_to >  0.5*to_grid.nx()) x_e -= to_grid.nx();
                     while(x_e - x_to < -0.5*to_grid.nx()) x_e += to_grid.nx();
                  }

                  x_min = min(x_min, x_e); x_max = max(x_max, x_e);
                  y_min = min(y_min, y_e); y_max = max(y_max, y_e);
               }
               n_xt = aw_mean_overlap(x_min, x_max, xt, xw);
               n_yt = aw_mean_overlap(y_min, y_max, yt, yw);
            }

            // Otherwise, assign to the nearest to_grid box
            if(n_xt == 0 || n_yt == 0) {
               n_xt = n_yt = 1;
               xt[0] = nint(x_to); xw[0] = 1.0;
               yt[0] = nint(y_to); yw[0] = 1.0;
            }

            for(int i=0; i<n_xt; i++) {

               // Wrap longitudes for AW_MEAN_CNTR
               int x = xt[i];
               if(centered && to_grid.wrap_lon()) {
                  x = ((x % to_grid.nx()) + to_grid.nx()) % to_grid.nx();
               }
               if(x < 0 || x >= to_grid.nx()) continue;

               for(int j=0; j<n_yt; j++) {
                  if(yt[j] < 0 || yt[j] >= to_grid.ny()) continue;

                  double w = weight*xw[i]*yw[j];
                  int n = to_data.two_to_one(x, yt[j]);
                  to_data_sum[n] += value*w;
                  wt_data_sum[n] += w;
               }
            }
         } // for yf
      } // for xf

      // Loop over the to grid to compute the area weighted average
#pragma omp for schedule(static)
      for(int n=0; n<to_data_sum.size(); n++) {

         int xt;
         int yt;
         to_data.one_to_two(n, xt, yt);

         if(is_eq(wt_data_sum[n], 0.0)) {
            to_data.set(bad_data_double, xt, yt);
         }
         else {
            to_data.set(to_data_sum[n] / wt_data_sum[n], xt, yt);
         }
      } // for n
   } // End of omp parallel

   return to_data;
}

////////////////////////////////////////////////////////////////////////

DataPlane met_regrid_force(const DataPlane & from_data,
                           const Grid & from_grid,
                           const Grid & to_grid,
                           const RegridInfo & info) {

   // Check grid dimensions
   if(from_grid.nx() != to_grid.nx() ||
      from_grid.ny() != to_grid.ny()) {

      mlog << Error << "\nmet_regrid_force() -> "
           << "the " << interpmthd_to_string(info.method)
           << " interpolation method may only be used when the grid "
           << "dimensions match: ("
           << from_grid.nx() << ", " << from_grid.ny() << ") != ("
           << to_grid.nx() << ", " << to_grid.ny() << ")\n\n";
      exit(1);
   }

   return from_data;
}

////////////////////////////////////////////////////////////////////////

DataPlane met_regrid_maxgauss(const DataPlane & from_data,
                              const Grid & from_grid,
                              const Grid & to_grid,
                              const RegridInfo & info) {
   DataPlane to_data;

#pragma omp parallel default(none) \
   shared(from_data, from_grid, to_grid, info, to_data)
   {

#pragma omp single
      {
         // Set the size and timing info
         to_data.set_size (to_grid.nx(), to_grid.ny());
         to_data.set_times(from_data);
      }

#pragma omp for schedule(static) \
                collapse(2)
      for(int xt=0; xt<(to_grid.nx()); xt++) {
         for(int yt=0; yt<(to_grid.ny()); yt++) {

            double lat;
            double lon;
            to_grid.xy_to_latlon(xt, yt, lat, lon);

            double x_from;
            double y_from;
            from_grid.latlon_to_xy(lat, lon, x_from, y_from);

            int xf = nint(x_from);
            int yf = nint(y_from);

            double value;
            if(((xf < 0 || xf >= from_grid.nx()) && !from_grid.wrap_lon()) ||
                 yf < 0 || yf >= from_grid.ny()) {
               value = bad_data_float;
            }
            else {
               value = compute_horz_interp(from_data, x_from, y_from,
                          bad_data_double, InterpMthd::Max, info.width,
                          info.shape, from_grid.wrap_lon(), info.vld_thresh);
            }

            to_data.put(value, xt, yt);

         } // for yt
      } // for xt
   } // End of omp parallel

   interp_gaussian_dp(to_data, info.gaussian, info.vld_thresh);

   return to_data;
}

////////////////////////////////////////////////////////////////////////

