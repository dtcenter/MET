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
// For AW_MEAN_CNTR, a from_grid point lying on the boundary between
// two to_grid boxes is split evenly between them. Otherwise, it is
// assigned to the nearest to_grid box. Returns the number of to_grid
// indices (1 or 2) stored in the index and weight arrays.
//
////////////////////////////////////////////////////////////////////////

static int aw_mean_split(double v, bool split, int i[2], double w[2]) {
   static const double boundary_tol = 1.0e-5;

   if(split && fabs(v - floor(v) - 0.5) < boundary_tol) {
      i[0] = (int) floor(v); w[0] = 0.5;
      i[1] = i[0] + 1;       w[1] = 0.5;
      return 2;
   }

   i[0] = nint(v); w[0] = 1.0;
   return 1;
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

            int xt[2], yt[2];
            double xw[2], yw[2];
            int n_xt = aw_mean_split(x_to, centered, xt, xw);
            int n_yt = aw_mean_split(y_to, centered, yt, yw);

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

