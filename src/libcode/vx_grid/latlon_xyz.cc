

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "trig.h"
#include "vx_log.h"
#include "vx_math.h"

#include "latlon_xyz.h"


////////////////////////////////////////////////////////////////////////


static const double near_pole = 89.9999;

static const double normalize_tol = 1.0e-5;


////////////////////////////////////////////////////////////////////////


void grid_latlon_to_xyz(double lat, double lon, double & x, double & y, double & z)

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


void grid_xyz_to_latlon(double x, double y, double z, double & lat, double & lon)

{
// account for numerical errors in which z is actually 1.0 or -1.0 but goes out of bounds
if (is_eq(z, 1.0, 1.0e-5))
    z = 1.0;
else if (is_eq(z, -1.0, 1.0e-5))
    z = -1.0;

lat = asind(z);

if ( fabs(lat) >= near_pole )  lon = 0.0;
else                           lon = atan2d(x, y);   //  NOT atan2d(y, x)


return;

}


////////////////////////////////////////////////////////////////////////


void normalize(double & ax, double & ay, double & az)

{

double t = ax*ax + ay*ay + az*az;

if ( fabs(t) < normalize_tol )  {

   mlog << Error
        << "\n\n  normalize() -> can't normalize zero vector!\n\n";

   exit ( 1 );

}

t = 1.0/sqrt(t);

ax *= t;
ay *= t;
az *= t;

return;

}


////////////////////////////////////////////////////////////////////////


