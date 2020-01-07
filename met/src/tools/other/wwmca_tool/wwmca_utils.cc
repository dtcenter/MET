// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


static const bool west_longitude_positive = true;


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "wwmca_utils.h"


////////////////////////////////////////////////////////////////////////


GridHemisphere find_grid_hemisphere(const Grid & grid)

{

int j;
double x, y, lat, lon;
bool nh = false;
bool sh = false;
const int Nx = grid.nx();
const int Ny = grid.ny();

   //
   //  bottom, top
   //

for (j=0; j<Nx; ++j)  {

   x = (double) j;

   grid.xy_to_latlon(x, 0.0, lat, lon);

   if ( lat > 0.0 )  nh = true;
   if ( lat < 0.0 )  sh = true;

   grid.xy_to_latlon(x, Ny - 1.0, lat, lon);

   if ( lat > 0.0 )  nh = true;
   if ( lat < 0.0 )  sh = true;

}

if ( nh && sh )  return ( both_hemispheres );

   //
   //  left, right
   //

for (j=0; j<Ny; ++j)  {

   y = (double) j;

   grid.xy_to_latlon(0.0, y, lat, lon);

   if ( lat > 0.0 )  nh = true;
   if ( lat < 0.0 )  sh = true;

   grid.xy_to_latlon(Nx - 1.0, y, lat, lon);

   if ( lat > 0.0 )  nh = true;
   if ( lat < 0.0 )  sh = true;

}

if ( nh && sh )  return ( both_hemispheres );

if ( nh )  return ( north_hemisphere );
if ( sh )  return ( south_hemisphere );

   //
   //  done
   //

return ( no_hemisphere );   //  this shouldn't happen

}


////////////////////////////////////////////////////////////////////////


Interpolator * get_interpolator(wwmca_regrid_Conf & config)

{

int width, good_percent;
int n_good_needed;
ConcatString method;
Ave_Interp ave;
Nearest_Interp nearest;
Min_Interp mini;
Max_Interp maxi;


width = config.interp_width();

good_percent = config.good_percent();

n_good_needed = nint(ceil(0.01*good_percent*width));

method = (const char *) config.interp_method();

   //
   //  get to work
   //

if ( method == "average" )  {

   ave.set_size(width);

   ave.set_ngood_needed(n_good_needed);

   return ( ave.copy() );

}


if ( method == "nearest" )  {

   nearest.set_size(width);

   nearest.set_ngood_needed(n_good_needed);

   return ( nearest.copy() );

}


if ( method == "min" )  {

   mini.set_size(width);

   mini.set_ngood_needed(n_good_needed);

   return ( mini.copy() );

}


if ( method == "max" )  {

   maxi.set_size(width);

   maxi.set_ngood_needed(n_good_needed);

   return ( maxi.copy() );

}


   //
   //  nope
   //

return ( (Interpolator *) 0 );

}


////////////////////////////////////////////////////////////////////////
