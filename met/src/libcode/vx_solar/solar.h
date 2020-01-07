// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef __SOLAR_H__
#define __SOLAR_H__


////////////////////////////////////////////////////////////////////////


#include "vx_cal.h"


////////////////////////////////////////////////////////////////////////


extern void solar_altaz(unixtime gmt, double lat, double lon, double & alt, double & azi);

   //
   //  calculates the altitude and azimuth of the sun at the given gmt
   //  for the given location.
   //
   //
   //  Input:   gmt, greenwich mean time expressed as unix time
   //
   //           lat, latitude (degrees) of given location (+ north, - south)
   //
   //           lon, longitude (degrees) of given location (+ west, - east)
   //
   //
   //  Output:  alt, sun's altitude in degrees (angle between line 
   //                of sight to sun and plane of horizon)
   //
   //           azi, sun'a azimuth in degrees, 0 north, 90 east, etc.
   //


////////////////////////////////////////////////////////////////////////


extern void solar_radec(unixtime gmt, double & Ra, double & Dec);

   //
   //  calculates the sun's right ascension and declination
   //  for the given greenwich mean time, accurate to a few
   //  minutes of arc
   //
   //
   //  Input:  gmt, greenwich mean time expressed as unix time
   //
   //  Output: Ra, the sun's right ascension in degrees
   //
   //          Dec, the sun's declination in degrees
   //


////////////////////////////////////////////////////////////////////////


extern void solar_latlon(unixtime gmt, double & lat, double & lon);


   //
   //  calculates the latitude and longitude of the sub-solar point
   //
   //  Input:  gmt, greenwich mean time expressed as unix time
   //
   //  Output: lat, lon
   //


////////////////////////////////////////////////////////////////////////


extern void dh_to_aa(double lat, double Dec, double lha, double &alt, double &azi);

   //
   //  performs an equatorial to horizon coordinate conversion
   //
   //
   //  Input:  lat, latitude in degrees of location (+ north, - south)
   //
   //          Dec, declination of object in degrees
   //
   //          lha, local hour angle of object in degrees
   //
   //
   //  Output:  alt, object's altitude in degrees (angle between line 
   //                of sight to object and plane of horizon)
   //
   //           azi, objects'a azimuth in degrees, 0 north, 90 east, etc.
   //


////////////////////////////////////////////////////////////////////////


#endif   /*  __SOLAR_H__  */


////////////////////////////////////////////////////////////////////////


