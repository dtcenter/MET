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

#include "GridTemplate.h"


////////////////////////////////////////////////////////////////////////


DataPlane met_regrid (const DataPlane & in, const Grid & from_grid, const Grid & to_grid, const RegridInfo & info)

{

DataPlane out;


switch ( info.method )  {

   case InterpMthd_Min:
   case InterpMthd_Max:
   case InterpMthd_Median:
   case InterpMthd_UW_Mean:
   case InterpMthd_DW_Mean:
   case InterpMthd_LS_Fit:
   case InterpMthd_Bilin:
   case InterpMthd_Nearest:
   case InterpMthd_Gaussian:
      out = met_regrid_generic (in, from_grid, to_grid, info);
      break;

   case InterpMthd_Budget:
      out = met_regrid_budget (in, from_grid, to_grid, info);
      break;

   case InterpMthd_AW_Mean:
      out = met_regrid_area_weighted (in, from_grid, to_grid, info);
      break;

   case InterpMthd_Force:
      out = met_regrid_force (in, from_grid, to_grid, info);
      break;

   default:
      mlog << Error << "\nmet_regrid() -> "
           << "bad interpolation method ... "
           << interpmthd_to_string(info.method) << "\n\n";
      exit(1);
      break;

}   //  switch info.method

   //
   //  done
   //

return ( out );

}


////////////////////////////////////////////////////////////////////////


DataPlane met_regrid_generic (const DataPlane & from_data, const Grid & from_grid, const Grid & to_grid, const RegridInfo & info)

{

int xt, yt;
int xf, yf;
double value, lat, lon;
double x_from, y_from;
DataPlane to_data;

to_data.set_size(to_grid.nx(), to_grid.ny());

   //
   //  copy timing info
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

      if ( (xf < 0) || (xf >= from_grid.nx()) || (yf < 0) || (yf >= from_grid.ny()) )  {

         value = bad_data_float;

      } else {
         value = compute_horz_interp(from_data, x_from, y_from, bad_data_double,
                                     info.method, info.width, info.shape, info.vld_thresh);

      }

      to_data.put(value, xt, yt);

   }   //  for yt

}   //  for xt

if (info.method == InterpMthd_Gaussian) {
   interp_gaussian_dp(to_data, info.gaussian_radius, info.gaussian_dx);
}


   //
   //  done
   //

return ( to_data );

}


////////////////////////////////////////////////////////////////////////


DataPlane met_regrid_area_weighted (const DataPlane & from_data, const Grid & from_grid, const Grid & to_grid, const RegridInfo & info)

{

int xt, yt;
int xf, yf;
double value, weight, lat, lon;
double x_to, y_to;
DataPlane to_data, wt_data;

   //
   //  The interpolation width and shape do not apply here.  The output
   //  value for each to_grid box is computed as a weighted average of
   //  the from_grid data points falling inside that box, where the
   //  weights are determined by the area of the from_grid boxes.
   //

   //
   //  set output size and initialize to 0
   //

to_data.set_size(to_grid.nx(), to_grid.ny());
wt_data.set_size(to_grid.nx(), to_grid.ny());

to_data.set_constant(0.0);
wt_data.set_constant(0.0);

   //
   //  copy timing info
   //

to_data.set_init  (from_data.init());
to_data.set_valid (from_data.valid());
to_data.set_lead  (from_data.lead());
to_data.set_accum (from_data.accum());

   //
   //  loop over the from grid to accumulate sums and area weights
   //

for (xf=0; xf<(from_grid.nx()); ++xf)  {

   for (yf=0; yf<(from_grid.ny()); ++yf)  {

      from_grid.xy_to_latlon(xf, yf, lat, lon);

      to_grid.latlon_to_xy(lat, lon, x_to, y_to);

      xt = nint(x_to);
      yt = nint(y_to);

      if ( (xt < 0) || (xt >= to_grid.nx()) || (yt < 0) || (yt >= to_grid.ny()) )  {

         continue;

      } else {

         if ( is_bad_data(value = from_data(xf, yf)) )  continue;
         weight = from_grid.calc_area(xf, yf);

         to_data.set(to_data(xt, yt) + value*weight, xt, yt);
         wt_data.set(wt_data(xt, yt) + weight,       xt, yt);

      }

   }   //  for yf

}   //  for xf

   //
   //  loop over the to grid to compute the area weighted average
   //

for (xt=0; xt<(to_grid.nx()); ++xt)  {

   for (yt=0; yt<(to_grid.ny()); ++yt)  {

      if ( is_eq(wt_data(xt, yt), 0.0) )  {

         to_data.set(bad_data_double, xt, yt);

      } else {

         to_data.set(to_data(xt, yt) / wt_data(xt, yt), xt, yt);

      }

   }   //  for yt

}   //  for xt

   //
   //  done
   //

return ( to_data );

}


////////////////////////////////////////////////////////////////////////


DataPlane met_regrid_force (const DataPlane & from_data, const Grid & from_grid, const Grid & to_grid, const RegridInfo & info)

{

   //
   //  check grid dimensions
   //

if ( from_grid.nx() != to_grid.nx() || from_grid.ny() != to_grid.ny() ) {

   mlog << Error << "\nmet_regrid_force() -> "
        << "the " << interpmthd_to_string(info.method)
        << " interpolation method may only be used when the grid "
        << "dimensions match: ("
        << from_grid.nx() << ", " << from_grid.ny() << ") != ("
        << to_grid.nx() << ", " << to_grid.ny() << ")\n\n";
   exit(1);

}

return ( from_data );

}

////////////////////////////////////////////////////////////////////////
