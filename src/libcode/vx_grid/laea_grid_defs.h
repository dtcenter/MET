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

/*
struct LaeaData {

   const char * name;    //  not allocated
   const char * geoid;   //  not allocated

   double lat_LL;        //  lower left
   double lon_LL;

   double lat_UL;        //  upper left
   double lon_UL;

   double lat_LR;        //  lower right
   double lon_LR;

   double lat_pole;
   double lon_pole;

   int nx;
   int ny;

   void dump() const;

};
*/

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


struct LaeaGrib2Data {  

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


   //
   //  Laea grid definitions
   //

/*
static const LaeaData laea_test_data = {


   "laea_test", //  name

   "WGS_84",    //  geoid

   31.7462,     // lat_LL   //  lower left
   10.4346,     // lon_LL

   67.0228,     // lat_UL   //  upper left
   39.5358,     // lon_UL

   31.9877,     // lat_LR   //  lower right
   -29.421,     // lon_LR
   
    55.0,       // lat_pole
   -10.0,       // lon_pole

   1900,        //  nx
   2200,        //  ny

};
*/

static const LaeaGrib2Data laea_grib2_test_data = {  


   "laea_grib2_test", 

   "Grib template 4", 

   0.0,                     //  radius_km

   0.5*6378.1370,           //  equatorial_radius_km
   0.5*6356.752314,         //       polar_radius_km

   44.5172,                 //  lat_first
   17.1171,                 //  lon_first

   54.900000,               //  standard_lat
    2.500000,               //  central_lon

    2.000000,               //  dx_km
    2.000000,               //  dy_km

   1042,                    //  nx
    970,                    //  ny

    //    970,                    //  ny
    //   1042,                    //  nx

   false                    //  is_sphere


};


////////////////////////////////////////////////////////////////////////


#endif   /*  __LAEA_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



