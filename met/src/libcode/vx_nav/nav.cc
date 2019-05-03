// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


using namespace std;


//////////////////////////////////////////////////////////////////


#include <cmath>

#include "vx_math.h"
#include "nav.h"


//////////////////////////////////////////////////////////////////


//static const double cf      = 57.2957795130823208768;


//////////////////////////////////////////////////////////////////


   //
   //  gc_dist
   //
   //  Calculates the distance (in kilometers)
   //     between the points (lat1, lon1),
   //     (lat2, lon2) (in degrees) along
   //     a great circle arc
   //


//////////////////////////////////////////////////////////////////


double gc_dist(double lat1, double lon1, double lat2, double lon2)

{

double x, dp, dl;
double lat1_radians, lat2_radians;
double lon1_radians, lon2_radians;

lat1_radians = lat1 * rad_per_deg;
lat2_radians = lat2 * rad_per_deg;
lon1_radians = lon1 * rad_per_deg;
lon2_radians = lon2 * rad_per_deg;
//lat1_radians = deg_to_rad(lat1);
//lat2_radians = deg_to_rad(lat2);
//lon1_radians = deg_to_rad(lon1);
//lon2_radians = deg_to_rad(lon2);

dp = (lat1_radians - lat2_radians);
dl = (lon1_radians - lon2_radians);

//dp = (lat1 - lat2)/cf;
//dl = (lon1 - lon2)/cf;

x = haversine(dp) + cos(lat1_radians)*cos(lat2_radians)*haversine(dl);
//x = haversine(dp) + cos(lat1/cf)*cos(lat2/cf)*haversine(dl);

x = earth_radius_km*ahaversine(x);

return ( x );

}


//////////////////////////////////////////////////////////////////


   //
   //  haversine
   //
   //  Calculates the haversine of an
   //     angle a in radians
   //


//////////////////////////////////////////////////////////////////


double haversine(double a)

{

double t = sin(0.5*a);

return ( t*t );

}


//////////////////////////////////////////////////////////////////


   //
   //  ahaversine
   //
   //  Calculates the angle (in radians)
   //     whose haversine is t
   //


//////////////////////////////////////////////////////////////////


double ahaversine(double t)

{

double a = 2.0*asin(sqrt(t));

return ( a );

}


//////////////////////////////////////////////////////////////////


   //
   //  rl_dist
   //
   //  Calculates the distance (in kilometers)
   //     between the points (lat1, lon1),
   //     (lat2, lon2) (in degrees) along
   //     a rhumbline
   //


//////////////////////////////////////////////////////////////////


double rl_dist(double lat1, double lon1, double lat2, double lon2)

{

double d, beta;

if ( fabs(lat1 - lat2) < 0.0001 )  {

   d = earth_radius_km*cosd(lat1)*(lon1 - lon2) * rad_per_deg;
//   d = earth_radius_km*cos(lat1/cf)*((lon1 - lon2)/cf);

   return ( fabs(d) );

}

beta = rl_bearing(lat1, lon1, lat2, lon2);

d = earth_radius_km*( rad_per_deg * (lat1 - lat2)/cosd(beta) );
//d = earth_radius_km*( ((lat1 - lat2)/cf)/cos(beta/cf) );

return ( fabs(d) );

}


//////////////////////////////////////////////////////////////////


   //
   //  rl_bearing
   //
   //  Calculates the bearing of point (lat2, lon2)
   //     from the point (lat1, lon1) (both in degrees)
   //     along a rhumbline.
   //
   //  Answer is returned in degrees.
   //


//////////////////////////////////////////////////////////////////


double rl_bearing(double lat1, double lon1, double lat2, double lon2)

{

double mp1, mp2, beta;

mp1 = meridional_parts(lat1);
mp2 = meridional_parts(lat2);

beta = atan2d( rad_per_deg * (lon1 - lon2), mp2 - mp1);
//beta = cf*atan2((lon1 - lon2)/cf, mp2 - mp1);

return ( beta );

}


//////////////////////////////////////////////////////////////////


   //
   //  meridional_parts
   //
   //  Calculates meridional parts
   //     for the angle a (in degrees)
   //


//////////////////////////////////////////////////////////////////


double meridional_parts(double a)

{

a = a * rad_per_deg;
//a /= cf;

return ( log(tan(piover4 + (0.5*a))) );

}


//////////////////////////////////////////////////////////////////


   //
   //  gc_bearing
   //
   //  Calculates the initial bearing of point (lat2, lon2)
   //     from the point (lat1, lon1) (both in degrees)
   //     along a great circle.
   //
   //  Answer is returned in degrees.
   //


//////////////////////////////////////////////////////////////////


double gc_bearing(double lat1, double lon1, double lat2, double lon2)

{

double x, y, dl, beta;

//lat1 /= cf; lon1 /= cf;

//lat2 /= cf; lon2 /= cf;

dl = lon1 - lon2;

x = cosd(lat1)*sind(lat2) - sind(lat1)*cosd(lat2)*cosd(dl);
//x = cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(dl);

y = cosd(lat2)*sind(dl);
//y = cos(lat2)*sin(dl);

beta = atan2d(y, x);
//beta = cf*atan2(y, x);

return ( beta );

}


//////////////////////////////////////////////////////////////////


   //
   //  gc_point_v1
   //
   //  Calculates the point (lat, lon) on the great
   //     circle arc connecting (lat1, lon1) and
   //     (lat2, lon2) that is a distance dist from
   //     (lat1, lon1) in the direction of (lat2, lon2)
   //
   //  Units for latitudes and longitudes are degrees
   //
   //  Units for dist are kilometers
   //


//////////////////////////////////////////////////////////////////


void gc_point_v1(double lat1, double lon1, double lat2, double lon2,
                 double dist, double &lat, double &lon)

{

double sp1, sp2, sl1, sl2;
double cp1, cp2, cl1, cl2;
double t, st, sth, stmt, theta;
double x, y, z;

theta = gc_dist(lat1, lon1, lat2, lon2)/earth_radius_km;

t = dist/earth_radius_km;

sth = sin(theta);

st = sin(t);

stmt = sin(theta - t);

//lat1 /= cf;    lon1 /= cf;

//lat2 /= cf;    lon2 /= cf;

sp1 = sind(lat1); sp2 = sind(lat2);
//sp1 = sin(lat1); sp2 = sin(lat2);

sl1 = sind(lon1); sl2 = sind(lon2);
//sl1 = sin(lon1); sl2 = sin(lon2);

cp1 = cosd(lat1); cp2 = cosd(lat2);
//cp1 = cos(lat1); cp2 = cos(lat2);

cl1 = cosd(lon1); cl2 = cosd(lon2);
//cl1 = cos(lon1); cl2 = cos(lon2);

x = cp1*cl1*stmt + cp2*cl2*st;

y = cp1*sl1*stmt + cp2*sl2*st;

z = (sp1*stmt + sp2*st)/sth;

lat = asind(z);
//lat = cf*asin(z);

if ( (fabs(x) + fabs(y)) < 1.0e-6 )
   lon = 0.0;
else
   lon = atan2d(y, x);
//   lon = cf*atan2(y, x);

return;

}


//////////////////////////////////////////////////////////////////


   //
   //  gc_point_v2
   //
   //  Calculates the point (lat, lon) on the great
   //     circle arc passing through (lat1, lon1) in
   //     in the direction bear that is a distance
   //     dist from (lat1, lon1)
   //
   //  Units for lat1, lon1, bear are degrees
   //
   //  Units for dist are kilometers
   //
   //  Units for lat, lon are degrees
   //


//////////////////////////////////////////////////////////////////


void gc_point_v2(double lat1, double lon1, double bear, double dist,
                 double &lat, double &lon)

{

double x, y, z, t;
double sp, cp, cl, sl, sb, cb, st, ct;

//lat1 /= cf;
//lon1 /= cf;

//bear /= cf;

t = dist/earth_radius_km;

sp = sind(lat1); cp = cosd(lat1);
//sp = sin(lat1); cp = cos(lat1);

sl = sind(lon1); cl = cosd(lon1);
//sl = sin(lon1); cl = cos(lon1);

sb = sind(bear); cb = cosd(bear);
//sb = sin(bear); cb = cos(bear);

st = sin(t); ct = cos(t);

x = cp*sl*ct - sp*sl*cb*st - cl*sb*st;

y = cp*cl*ct - sp*cl*cb*st + sl*sb*st;

z = sp*ct + cp*cb*st;

lat = asind(z);
//lat = cf*asin(z);

if ( (fabs(x) + fabs(y)) < 1.0e-6 )
   lon = 0.0;
else
   lon = atan2d(x, y);
//   lon = cf*atan2(x, y);

return;

}


//////////////////////////////////////////////////////////////////


   //
   //  rl_point_v1
   //
   //  Calculates the point (lat, lon) on the rhumbline
   //     connecting (lat1, lon1) and (lat2, lon2) that
   //     is a distance dist from (lat1, lon1) in the
   //     direction of (lat2, lon2)
   //
   //  Units for latitudes and longitudes are degrees
   //
   //  Units for dist are kilometers
   //


//////////////////////////////////////////////////////////////////


void rl_point_v1(double lat1, double lon1, double lat2, double lon2,
                 double dist, double &lat, double &lon)

{

double bear = rl_bearing(lat1, lon1, lat2, lon2);

rl_point_v2(lat1, lon1, bear, dist, lat, lon);

return;

}


//////////////////////////////////////////////////////////////////


   //
   //  rl_point_v2
   //
   //  Calculates the point (lat, lon) on the rhumbline
   //     passing through (lat1, lon1) in the direction
   //     bear that is a distance dist from (lat1, lon1)
   //
   //  Units for lat1, lon1, bear are degrees
   //
   //  Units for dist are kilometers
   //
   //  Units for lat, lon are degrees
   //


//////////////////////////////////////////////////////////////////


void rl_point_v2(double lat1, double lon1, double bear, double dist,
                 double &lat, double &lon)

{

double t, sb, cb, tb, mp, mp1;

//bear /= cf;

sb = sind(bear); cb = cosd(bear);
//sb = sin(bear); cb = cos(bear);

t = dist/earth_radius_km;

lat = lat1 + deg_per_rad * (t*cb);
//lat = lat1 + cf*t*cb;

if ( fabs(cb) < 1.0e-5 )
   lon = lon1 * rad_per_deg - t*( sb/cosd(lat1) );
//   lon = (lon1/cf) - t*( sb/cos(lat1/cf) );
else {

   tb = tand(bear);
//   tb = tan(bear);

   mp = meridional_parts(lat);

   mp1 = meridional_parts(lat1);

   lon = lon1 * rad_per_deg - tb*( mp - mp1 );
//   lon = (lon1/cf) - tb*( mp - mp1 );

}

lon += twopi*floor( 0.5 - ((lon)/twopi) );

lon = lon * deg_per_rad;
//lon *= cf;

return;

}


//////////////////////////////////////////////////////////////////



