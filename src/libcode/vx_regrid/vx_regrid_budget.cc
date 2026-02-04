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

using namespace std;

////////////////////////////////////////////////////////////////////////

DataPlane met_regrid_budget(const DataPlane & from_data,
                            const Grid & from_grid,
                            const Grid & to_grid,
                            const RegridInfo & info) {
   DataPlane to_data;
   int count;
   double value;
   double sum;

   // Hard-code the radius for budget interpolation.
   // This could be made configurable.
   const int Radius = 2;
   const int N = 2*Radius + 1;
   const int NN = N*N;
   const double delta = 1.0/N;

#pragma omp parallel default(none) \
   shared(from_data, from_grid, to_grid, info, to_data) \
   shared(Radius, N, NN, delta) \
   private(count, value, sum)
   {

#pragma omp single
      {
         // Set the size and timing info 
         to_data.set_size (to_grid.nx(), to_grid.ny());
         to_data.set_times(from_data);
      }

#pragma omp for schedule(static) \
                collapse(2)
      for(int ixt=0; ixt<(to_grid.nx()); ixt++) {
         for(int iyt=0; iyt<(to_grid.ny()); iyt++) {

            // Initialize
            count = 0;
            sum = 0.0;

            for(int i=-Radius; i<=Radius; i++) {

               double dxt = ixt + i*delta;

               for(int j=-Radius; j<=Radius; j++) {

                  double dyt = iyt + j*delta;

                  double lat;
                  double lon;
                  to_grid.xy_to_latlon(dxt, dyt, lat, lon);

                  double dxf;
                  double dyf;
                  from_grid.latlon_to_xy(lat, lon, dxf, dyf);

                  value = interp_bilin(from_data, from_grid.wrap_lon(), dxf, dyf);

                  // Increment sum and valid data count
                  if(value != bad_data_double) {
                     sum += value;
                     count++;
                  }
               } // for j
            } // for i

            double fraction = ((double) count)/((double) NN);

            if(fraction >= info.vld_thresh) value = sum/count;
            else                            value = bad_data_double;

            to_data.put(value, ixt, iyt);

         } // for iyt
      } // for ixt
   } // End of omp parallel

   return to_data;
}

////////////////////////////////////////////////////////////////////////
