// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <cmath>

#include "trig.h"
#include "siderial.h"
#include "solar.h"
#include "astro_constants.h"


////////////////////////////////////////////////////////////////////////


void solar_altaz(unixtime gmt, double lat, double lon, double & alt, double & azi)

{

double lha, Ra, Dec;

   //
   //  right ascension and declination
   //

solar_radec(gmt, Ra, Dec);

   //
   //  local hour angle
   //

lha = gmt_to_gmst(gmt) - lon - Ra;

   //
   //  equatorial to horizon conversion
   //

dh_to_aa(lat, Dec, lha, alt, azi);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void dh_to_aa(double lat, double Dec, double lha, double & alt, double & azi)

{

double x, y, z, d;
double cl, sl, cd, sd, ch, sh;
const double tol = 1.0e-6;


   //
   //  calculate some stuff
   //

cl = cosd(lat);  sl = sind(lat);
cd = cosd(Dec);  sd = sind(Dec);
ch = cosd(lha);  sh = sind(lha);

   //
   //  rectangular coordinates
   //

x = sd*cl - cd*sl*ch;

y = -cd*sh;

z = sl*sd + cl*cd*ch;

   //
   //  altitude and azimuth (range of -180 to 180)
   //

d = fabs(x) + fabs(y);

azi = ( (d < tol) ? 0.0 : atan2d(y, x) );

azi += 180.0;

azi -= 360.0*floor(azi/360.0);

azi -= 180.0;

alt = asind(z);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Calculate the sun's right ascension and declination
   //
   //    For the intended purpose (calculating zenith angles)
   //    and precision ( +/- 0.01 degree ) we don't need to
   //    worry what epoch the coordinates are referred to.
   //
   //    Reference: Chapter 24 of "Astronomical Algorithms"
   //    by Jean Meeus, published by Willman-Bell in 1991
   //    ISBN 0-943396-35-2
   //


void solar_radec(unixtime gmt, double & Ra, double & Dec)

{

double t, lzero, m, eps;
double omega, lambda;
double x, y, z, d, c, theta;
const double tol = 1.0e-6;


   //
   //  scale and offset adjustment for time
   //

t = ((gmt/86400.0) - 10957.5)/36525.0;

   //
   //  geometric mean longitude of the sun,
   //    referred to the mean equinox of date
   //

lzero = 280.46645 + 36000.76983*t + 0.0003032*t*t;

lzero -= 360.0*floor(lzero/360.0);

   //
   //  the sun's mean anomaly
   //

m = 357.52910 + 35999.05030*t - 0.0001559*t*t - 0.00000048*t*t*t;

m -= 360.0*floor(m/360.0);

   //
   //  obliquity of the ecliptic
   //

eps = 23.439291111111 - (46.815*t + 0.00059*t*t - 0.001813*t*t*t)/3600.0;

   //
   //  the eccentricity of the earth's orbit
   //

// double ecc = 0.016708617 - 0.000042037*t - 0.0000001236*t*t;

   //
   //  the sun's equation of center
   //

c =   (1.9146 - 0.004817*t - 0.000014*t*t)*sind(m)
        + (0.019933 - 0.000101*t)*sind(2.0*m)
        + 0.00029*sind(3.0*m);

   //
   //  the sun's true longitude
   //

theta = lzero + c;

   //
   //  the sun's apparent longitude
   //

omega = 125.04 - 1934.136*t;

omega -= 360.0*floor(omega/360.0);

lambda = theta - 0.00569 - 0.00478*sind(omega);

   //
   //  correction to obliquity
   //

eps += 0.00256*cosd(omega);

   //
   //  rectangular coordinates
   //

x = cosd(lambda);

y = cosd(eps)*sind(lambda);

z = sind(eps)*sind(lambda);

   //
   //  declination and right ascension
   //

d = fabs(x) + fabs(y);

Dec = asind(z);

Ra = ( (d < tol) ? 0.0 : atan2d(y, x) );

Ra -= 360.0*floor(Ra/360.0);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void solar_latlon(unixtime gmt, double & lat, double & lon)

{

double s;
double Ra, Dec;


s = gmt_to_gmst(gmt);

solar_radec(gmt, Ra, Dec);

lat = Dec;

lon = s - Ra;

lon += 180.0;

lon -= 360.0*floor(lon/360.0);

lon -= 180.0;


return;

}


////////////////////////////////////////////////////////////////////////


