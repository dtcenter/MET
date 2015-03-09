// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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


DataPlane met_regrid_nearest_nbr (const DataPlane & from_data, const Grid & from_grid, const Grid & to_grid, const RegridInfo & info)

{

int xt, yt;
int xf, yf;
double value, lat, lon;
double x_from, y_from;
DataPlane to_data;

to_data.set_size(to_grid.nx(), to_grid.ny());

   //
   // Copy timing info
   //

to_data.set_init  (from_data.init());
to_data.set_valid (from_data.valid());
to_data.set_lead  (from_data.lead());
to_data.set_accum (from_data.accum());

   //
   //  copy data
   //

for (xt=0; xt<(to_grid.nx()); ++xt)  {

   for (yt=0; yt<(to_grid.ny()); ++yt)  {

      to_grid.xy_to_latlon(xt, yt, lat, lon);

      from_grid.latlon_to_xy(lat, lon, x_from, y_from);

      xf = nint(x_from);
      yf = nint(y_from);

      if ( (xf < 0) || (xf >= from_grid.nx()) || (yf < 0) || (yf >= from_grid.ny()) )  value = bad_data_float;
      else                                                                             value = from_data(xf, yf);

      to_data.put(value, xt, yt);

   }   //  for yt

}   //  for xt

   //
   //  done
   //

return ( to_data );

}


////////////////////////////////////////////////////////////////////////


