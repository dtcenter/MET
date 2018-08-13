

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_LATLON_TO_XYZ_H__
#define  __VX_LATLON_TO_XYZ_H__


////////////////////////////////////////////////////////////////////////

   //
   //  latitude and longitude are in degrees, west longitude is positive.
   //
   //   (x, y, z) is a point on the unit sphere x^2 + y^2 + z^2 = 1
   //

extern void latlon_to_xyz(double lat, double lon, double & x, double & y, double & z);

extern void xyz_to_latlon(double x, double y, double z, double & lat, double & lon);


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_LATLON_TO_XYZ_H__  */


////////////////////////////////////////////////////////////////////////


