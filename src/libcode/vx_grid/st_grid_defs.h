

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __STEREOGRAPHIC_GRID_DEFINITIONS_H__
#define  __STEREOGRAPHIC_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


struct StereographicData {

   const char * name;

   char hemisphere;     //  'N' or 'S' from latitude_of_projection_origin

   double scale_lat;    //latitude_of_projection_origin

   double lat_pin;
   double lon_pin;

   double x_pin;
   double y_pin;

   double lon_orient;   // -straight_vertical_longitude_from_pole

   double d_km;

   double r_km;     // semi_major_axis

   int nx;
   int ny;

   // ellipsoidal earth
   double eccentricity; // 0 for shperical earth
   double false_east;
   double false_north;
   double scale_factor;
   double dy_km;

   void dump();
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __STEREOGRAPHIC_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



