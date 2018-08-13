

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include "trig.h"

#include "latlon_xyz.h"


////////////////////////////////////////////////////////////////////////


static const double near_pole = 89.9999;


////////////////////////////////////////////////////////////////////////


void latlon_to_xyz(double lat, double lon, double & x, double & y, double & z)

{

const double cos_lat = cosd(lat);   //  this is needed twice, so we'll store it


x = cos_lat*sind(lon);

y = cos_lat*cosd(lon);

z = sind(lat);


return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  we assume that x^2 + y^2 + z^2 = 1
   //


void xyz_to_latlon(double x, double y, double z, double & lat, double & lon)

{

lat = asind(z);

if ( fabs(lat) >= near_pole )  lon = 0.0;
else                           lon = atan2d(x, y);   //  NOT atan2d(y, x)


return;

}


////////////////////////////////////////////////////////////////////////


