// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __LAEA_GRID_DEFINITIONS_H__
#define  __LAEA_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


struct LaeaNetcdfData {

   const char * name;

   double prime_meridian_lon;

   double semi_major_axis_km;
   double semi_minor_axis_km;

   double proj_origin_lat;
   double proj_origin_lon;

   double  x_pin;
   double  y_pin;

   double dx_km;
   double dy_km;

   int nx;
   int ny;

   void dump() const;

};


////////////////////////////////////////////////////////////////////////


   //
   //  all lengths in kilometers
   //
   //  all angles in degrees
   //
   //  conventions for latitude:  + north, - south
   //
   //  conventions for longitude: + west, - east
   //


struct LaeaData {  

   const char * name;             //  not allocated
   const char * spheroid_name;    //  not allocated

   double radius_km;              //  for spherical Earth

   double equatorial_radius_km;   //  for ellipsoidal Earth
   double      polar_radius_km;   //  for ellipsoidal Earth

   double lat_first;              //   latitude of first grid point
   double lon_first;              //  longitude of first grid point

   double standard_lat;           //  standard parallel
   double central_lon;            //  central longitude

   double dx_km;                  //  x-direction grid length
   double dy_km;                  //  x-direction grid length

   int nx;                        //  number of points along x-axis
   int ny;                        //  number of points along y-axis

   bool is_sphere;                //  spherical earth, or ellipsoidal?

   void dump() const;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __LAEA_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////

