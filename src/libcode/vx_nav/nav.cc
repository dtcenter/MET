// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


//////////////////////////////////////////////////////////////////


#include <stdlib.h>
#include <cmath>

#include "vx_math.h"
#include "nav.h"


using namespace std;


//////////////////////////////////////////////////////////////////


struct Vector3D {
   double x, y, z;
};

static Vector3D latlon_to_xyz(double lat, double lon);
static void xyz_to_latlon(Vector3D v, double &lat, double &lon);
static Vector3D cross_product(Vector3D v1, Vector3D v2);
   

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

double lat1_radians = lat1 * rad_per_deg;
double lat2_radians = lat2 * rad_per_deg;
double lon1_radians = lon1 * rad_per_deg;
double lon2_radians = lon2 * rad_per_deg;

double dp = (lat1_radians - lat2_radians);
double dl = (lon1_radians - lon2_radians);

double x = haversine(dp) + cos(lat1_radians)*cos(lat2_radians)*haversine(dl);

return earth_radius_km*ahaversine(x);

}


//////////////////////////////////////////////////////////////////


   //
   //  gc_angle
   //
   //  Calculates the angle in degrees
   //     between the points (lat1, lon1),
   //     (lat2, lon2) (in degrees) with the
   //     center of the earth as the vertex
   //


//////////////////////////////////////////////////////////////////


double gc_angle(double lat1, double lon1, double lat2, double lon2)

{

double lat1_radians = lat1 * rad_per_deg;
double lat2_radians = lat2 * rad_per_deg;
double lon1_radians = lon1 * rad_per_deg;
double lon2_radians = lon2 * rad_per_deg;

double dp = (lat1_radians - lat2_radians);
double dl = (lon1_radians - lon2_radians);

double x = haversine(dp) + cos(lat1_radians)*cos(lat2_radians)*haversine(dl);

return ahaversine(x);

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

return 2.0*asin(sqrt(t));

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

double d;

if ( fabs(lat1 - lat2) < 0.0001 )  {

   d = earth_radius_km*cosd(lat1)*(lon1 - lon2) * rad_per_deg;

   return fabs(d);

}

double beta = rl_bearing(lat1, lon1, lat2, lon2);

d = earth_radius_km*( rad_per_deg * (lat1 - lat2)/cosd(beta) );

return fabs(d);

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

double mp1 = meridional_parts(lat1);
double mp2 = meridional_parts(lat2);

return atan2d( rad_per_deg * (lon1 - lon2), mp2 - mp1);

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

return log(tan(piover4 + (0.5*a)));

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

double dl = lon1 - lon2;

double x = cosd(lat1)*sind(lat2) - sind(lat1)*cosd(lat2)*cosd(dl);

double y = cosd(lat2)*sind(dl);

double beta = atan2d(y, x);

return beta;

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

double theta = gc_dist(lat1, lon1, lat2, lon2)/earth_radius_km;

double t = dist/earth_radius_km;

double sth = sin(theta);

double st = sin(t);

double stmt = sin(theta - t);

double sp1 = sind(lat1);
double sp2 = sind(lat2);

double sl1 = sind(lon1);
double sl2 = sind(lon2);

double cp1 = cosd(lat1);
double cp2 = cosd(lat2);

double cl1 = cosd(lon1);
double cl2 = cosd(lon2);

double x = cp1*cl1*stmt + cp2*cl2*st;

double y = cp1*sl1*stmt + cp2*sl2*st;

double z = (sp1*stmt + sp2*st)/sth;

lat = asind(z);

if ( (fabs(x) + fabs(y)) < 1.0e-6 )
   lon = 0.0;
else
   lon = atan2d(y, x);

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

double t = dist/earth_radius_km;

double sp = sind(lat1); 
double cp = cosd(lat1);

double sl = sind(lon1);
double cl = cosd(lon1);

double sb = sind(bear);
double cb = cosd(bear);

double st = sin(t);
double ct = cos(t);

double x = cp*sl*ct - sp*sl*cb*st - cl*sb*st;

double y = cp*cl*ct - sp*cl*cb*st + sl*sb*st;

double z = sp*ct + cp*cb*st;

lat = asind(z);

if ( (fabs(x) + fabs(y)) < 1.0e-6 )
   lon = 0.0;
else
   lon = atan2d(x, y);

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

double sb = sind(bear);
double cb = cosd(bear);

double t = dist/earth_radius_km;

lat = lat1 + deg_per_rad * (t*cb);

if ( fabs(cb) < 1.0e-5 )
   lon = lon1 * rad_per_deg - t*( sb/cosd(lat1) );
else {

   double tb = tand(bear);

   double mp = meridional_parts(lat);

   double mp1 = meridional_parts(lat1);

   lon = lon1 * rad_per_deg - tb*( mp - mp1 );

}

lon += twopi*floor( 0.5 - (lon/twopi) );

lon = lon * deg_per_rad;

return;

}


//////////////////////////////////////////////////////////////////


   //
   //  gc_dist_to_line
   //
   //  Calculates the great circle arc distance from the point
   //     (lat3, lon3) to the great circle arc connecting
   //     (lat1, lon1) and (lat2, lon2).
   //
   //  Units for latitudes and longitudes are degrees
   //
   //  Units for dist are kilometers
   //


//////////////////////////////////////////////////////////////////


double gc_dist_to_line(double lat1, double lon1,
                       double lat2, double lon2,
                       double lat3, double lon3) {

   // Convert to cartesian coordinates
   Vector3D a = latlon_to_xyz(lat1, lon1);
   Vector3D b = latlon_to_xyz(lat2, lon2);
   Vector3D c = latlon_to_xyz(lat3, lon3);

   // Compute intersection of arc AB with perpendicular from C
   Vector3D g = cross_product(a, b);
   Vector3D f = cross_product(c, g);
   Vector3D t = cross_product(g, f);

   // Convert intersection point from cartesian back to lat/lon
   double lat4;
   double lon4;
   xyz_to_latlon(t, lat4, lon4);
   
   // Length of the arc segment
   double dist12 = gc_dist(lat1, lon1, lat2, lon2);
   
   // Distance from intersection point to end points
   double dist14 = gc_dist(lat1, lon1, lat4, lon4);
   double dist24 = gc_dist(lat2, lon2, lat4, lon4);
   
   // If intersection is between the end points,
   // use the perpendicular distance
   double dist;
   if(dist14 <= dist12 && dist24 <= dist12) {
      dist = gc_dist(lat3, lon3, lat4, lon4);
   }
   // Otherwise, use the minimum distance to the end points
   else {
      dist = min(gc_dist(lat1, lon1, lat3, lon3),
                 gc_dist(lat2, lon2, lat3, lon3));
   }
   
   return dist;
}

//////////////////////////////////////////////////////////////////

Vector3D latlon_to_xyz(double lat, double lon) {
   Vector3D v;
   
   v.x = cosd(lat) * cosd(lon) * earth_radius_km;
   v.y = cosd(lat) * sind(lon) * earth_radius_km;
   v.z = sind(lat) * earth_radius_km;
   
   return v;
}

//////////////////////////////////////////////////////////////////

void xyz_to_latlon(Vector3D v, double &lat, double &lon) {
   
   // Normalize to unit vector
   double length = sqrt((v.x*v.x)+(v.y*v.y)+(v.z*v.z));
   v.x /= length;
   v.y /= length;
   v.z /= length;

   // Convert to lat/lon
   lat = asind(v.z);
   lon = atan2d(v.y, v.x);

   return;
}

//////////////////////////////////////////////////////////////////

Vector3D cross_product(Vector3D v1, Vector3D v2) {
   Vector3D v;
   
   v.x = v1.y*v2.z - v2.y*v1.z;
   v.y = v2.x*v1.z - v1.x*v2.z;
   v.z = v1.x*v2.y - v1.y*v2.x; 
   
   return v;
}

//////////////////////////////////////////////////////////////////
