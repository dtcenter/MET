// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
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


// static GridInfo create_lambert_grid       (const StringArray &);
// static GridInfo create_latlon_grid        (const StringArray &);
// static GridInfo create_stereographic_grid (const StringArray &);
// static GridInfo create_mercator_grid      (const StringArray &);


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

/*
GridInfo get_grid(const char * gridinfo_string)

{

int n;
char * line = (char *) 0;
char * c = (char *) 0;
char * p = (char *) 0;
const char delim [] = " ";
ConcatString s;
const Grid * G = (const Grid *) 0;
GridInfo info;



a.clear();

type = no_grid_projection;

n = 1 + strlen(gridinfo_string);

line = new char [n];

memset(line, 0, n);

strcpy(line, gridinfo_string);

p = line;

while ( (c = strtok(p, delim)) != 0 )  {

   a.add(c);

   p = (char *) 0;

}   //  while

s = a[0];


if ( s == "lambert" )  {

   info = create_lambert_grid(a);

   return ( info );

}


if ( s == "stereo" )  {

   info = create_stereographic_grid(a);

   return ( info );

}


if ( s == "latlon" )  {

   info = create_latlon_grid(a);

   return ( info );

}


if ( s == "mercator" )  {

   info = create_mercator_grid(a);

   return ( info );

}


   //
   //  nope
   //

return ( info );

}
*/

////////////////////////////////////////////////////////////////////////

/*
GridInfo create_lambert_grid(const StringArray & a)

{

LambertData data;
const int N = a.n_elements();
GridInfo info;


if ( (N < 9) || (N > 10) )  return ( info );


info.lc = new LambertData;

LambertData & data = *(info.lc);

data.name = "To (Lambert)";

data.scale_lat_1 = atof(a[8]);

if ( a.n_elements() == 10 )  data.scale_lat_2 = atof(a[9]);
else                         data.scale_lat_2 = data.scale_lat_1;

data.lat_pin = atof(a[3]);
data.lon_pin = atof(a[4]);

data.x_pin = 0.0;
data.y_pin = 0.0;

data.lcen = atof(a[5]);

data.d_km = atof(a[6]);
data.r_km = atof(a[7]);

data.nx = atof(a[1]);
data.ny = atof(a[2]);

data.so2_angle = 0.0;

if ( west_longitude_positive )  {

   data.lon_pin *= -1.0;
   data.lcen    *= -1.0;

}

return ( info );

}
*/

////////////////////////////////////////////////////////////////////////


