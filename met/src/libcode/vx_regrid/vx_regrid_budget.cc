// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;

#include "vx_regrid.h"

#include "interp_mthd.h"


////////////////////////////////////////////////////////////////////////


DataPlane met_regrid_budget (const DataPlane & from_data, const Grid & from_grid, const Grid & to_grid, const RegridInfo & info)

{

DataPlane to_data;
int i, j, ixt, iyt;
int count;
double dxt, dyt, dxf, dyf;
double sum, lat, lon, value;
double fraction;

   //
   //  Hard-code the radius for budget interpolation.
   //  Consider making this configurable.
   //

const int Radius = 2;
const int N = 2*Radius + 1;
const int NN = N*N;
const double delta = 1.0/N;

to_data.set_size(to_grid.nx(), to_grid.ny());

   //
   // Copy timing info
   //

to_data.set_init  (from_data.init());
to_data.set_valid (from_data.valid());
to_data.set_lead  (from_data.lead());
to_data.set_accum (from_data.accum());

   //
   //  Do the interpolation
   //

for (ixt=0; ixt<(to_grid.nx()); ++ixt)  {

   for (iyt=0; iyt<(to_grid.ny()); ++iyt)  {

      sum = 0.0;

      count = 0;

      for (i=-Radius; i<=Radius; ++i)  {

         dxt = ixt + i*delta;

         for (j=-Radius; j<=Radius; ++j)  {

            dyt = iyt + j*delta;

            to_grid.xy_to_latlon(dxt, dyt, lat, lon);

            from_grid.latlon_to_xy(lat, lon, dxf, dyf);

            value = interp_bilin(from_data, dxf, dyf);

            if ( value != bad_data_double )  { sum += value;  ++count; }

         }   //  for j

      }   //  for i

      fraction = ((double) count)/((double) NN);

      if ( fraction >= info.vld_thresh )  value = sum/count;
      else                                value = bad_data_double;

      to_data.put(value, ixt, iyt);

   }   //  for iyt

}   //  for ixt


   //
   //  done
   //

return ( to_data );

}


////////////////////////////////////////////////////////////////////////


