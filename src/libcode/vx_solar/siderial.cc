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
#include <stdlib.h>
#include <cmath>

#include "siderial.h"
#include "astro_constants.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Greenwich mean siderial time, in degrees
   //
   //
   //      Based on the algorithm given in Chapter 12 of
   //
   //      "Astronomical Algorithms", 2nd Ed., by Jean Meeus
   //


double gmt_to_gmst(unixtime gmt)

{

int month, day, year, hour, minute, second;
int mjd;
double T, theta, theta0;
double fraction;


unix_to_mdyhms(gmt, month, day, year, hour, minute, second);

mjd = date_to_mjd(month, day, year);

T = (mjd - 51544.5)/36525.0;

theta0 =   100.46061837 

         + 36000.770053608*T;

theta0 -= 360.0*floor(theta0/360.0);

theta0 += 0.000387933*T*T - (T*T*T)/38710000.0;

theta0 -= 360.0*floor(theta0/360.0);

fraction = (3600*hour + 60*minute + second)/86400.0;

theta = theta0 + eta*360.0*fraction;

theta -= 360.0*floor(theta/360.0);

   //
   //  done
   //

return ( theta );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Local mean siderial time, in degrees
   //


double lmt_to_lmst(unixtime lmt, int zone, double lon)

{

double lst, gst;
unixtime gmt;


gmt = lmt + 3600*zone;

gst = gmt_to_gmst(gmt);

lst = gst - lon;


lst -= 360.0*floor(lst/360.0);


   //
   //  done
   //

return ( lst );

}


////////////////////////////////////////////////////////////////////////



