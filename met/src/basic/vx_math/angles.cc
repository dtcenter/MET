// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


using namespace std;


//////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "angles.h"

#include "trig.h"
#include "is_bad_data.h"
#include "vx_log.h"


//////////////////////////////////////////////////////////////////


   //
   // Rescale the longitude value provided to be in the range from
   // -180 to 180
   //

double rescale_lon(double lon)

{

double new_lon;

   // Check for bad data

if( is_bad_data(lon) )  return ( bad_data_double );

new_lon = lon;

if ( new_lon < -180.0 )  {

   while ( new_lon < -180.0 )  new_lon += 360.0;

}

if ( new_lon > 180.0 )  {

   while ( new_lon >  180.0 )  new_lon -= 360.0;

}

return ( new_lon );

}


//////////////////////////////////////////////////////////////////

   //
   // Rescale the degrees provided to be in the range specified
   //

double rescale_deg(double deg, double lower, double upper)

{

double new_deg;

   // Check for bad data

if ( is_bad_data(deg) )  return ( bad_data_double );

if ( ! is_eq(fabs(upper - lower), 360.0) || lower > upper )  {

   mlog << Error << "\nrescale_deg() -> invalid upper and lower limits supplied.\n\n";

   exit(1);

}

new_deg = deg;

if ( new_deg < lower )  {

   while ( new_deg < lower )  new_deg += 360.0;

}

if ( new_deg > upper )  {

   while ( new_deg > upper )  new_deg -= 360.0;

}

return ( new_deg );

}

//////////////////////////////////////////////////////////////////

   //
   // Compute the number of degrees between two angles
   //

double angle_between(double a, double b)

{

double d, d1, d2;

   // Check for bad data

if ( is_bad_data(a) || is_bad_data(b) )  return(bad_data_double);

   // Compute the difference between the angles, and rescale them from
   // 0 to 360

d1 = rescale_deg(a - b, 0.0, 360.0);
d2 = rescale_deg(b - a, 0.0, 360.0);

if ( min(d1, d2) < 90.0 )  d = min(d1, d2);
else                       d = max(d1, d2) - 180.0;

return ( d );

}


//////////////////////////////////////////////////////////////////


   //
   // Compute the directed angle difference between two vectors
   //

double angle_difference(double uf, double vf, double uo, double vo)

{

double d, a, b;

   // Check for bad data

if ( is_bad_data(uf) || is_bad_data(vf) || is_bad_data(uo) || is_bad_data(vo) )  return ( bad_data_double );

   // Normalize the vectors to unit length

convert_u_v_to_unit(uf, vf);
convert_u_v_to_unit(uo, vo);

   // Check for bad data in the normalized vectors

if ( is_bad_data(uf) || is_bad_data(vf) || is_bad_data(uo) || is_bad_data(vo) )  return ( bad_data_double );

   // Compute sums

a = vo*uf - uo*vf;
b = uo*uf + vo*vf;

   // Only compute the angle between if both terms are non-zero

if(is_eq(a, 0.0) && is_eq(b, 0.0)) d = bad_data_double;
else                               d = atan2d(a, b);

return ( d );

}


//////////////////////////////////////////////////////////////////


double convert_u_v_to_wdir(double u, double v)

{

double wdir;

     if ( is_eq(u, bad_data_double) || is_eq(v, bad_data_double) )  wdir = bad_data_double;
else if ( is_eq(u, 0.0) && is_eq(v, 0.0) )                          wdir = bad_data_double;
else  {

   wdir = rescale_deg(atan2d(-u, -v), 0.0, 360.0);

}

return ( wdir );

}


//////////////////////////////////////////////////////////////////


double convert_u_v_to_wind(double u, double v)

{

double wind;

if ( is_eq(u, bad_data_double) || is_eq(v, bad_data_double) )  wind = bad_data_double;
else {

   wind = sqrt(u*u + v*v);

}

return ( wind );

}


//////////////////////////////////////////////////////////////////


void convert_u_v_to_unit(double u, double v, double & u_unit, double & v_unit)

{

double l = sqrt(u*u + v*v);

if( is_eq(u, bad_data_double) || is_eq(v, bad_data_double) || is_eq(l, 0.0) )  {

   u_unit = bad_data_double;
   v_unit = bad_data_double;

} else {

   u_unit = u/l;
   v_unit = v/l;

}

return;

}


//////////////////////////////////////////////////////////////////


void convert_u_v_to_unit(double & u, double & v)

{

double u_tmp, v_tmp;

convert_u_v_to_unit(u, v, u_tmp, v_tmp);

u = u_tmp;
v = v_tmp;

return;

}


//////////////////////////////////////////////////////////////////



