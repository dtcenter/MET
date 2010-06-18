
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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

   double scale_lat;

   double lat_pin;
   double lon_pin;

   double x_pin;
   double y_pin;

   double lcen;

   double d_km;

   double r_km;

   int nx;
   int ny;

};


////////////////////////////////////////////////////////////////////////


struct StereoType2Data {

   const char * name;

   char hemisphere;   //  'N' or 'S'

   double scale_lat;

   double lat_pin;
   double lon_pin;

   double x_pin;
   double y_pin;

   double lcen;

   double d_km;

   double r_km;

   int nx;
   int ny;

};


////////////////////////////////////////////////////////////////////////


struct StereoType3Data {

   const char * name;

   char hemisphere;   //  'N' or 'S'

   double lcen;

   double alpha;

   double bx;
   double by;

   int nx;
   int ny;

};


////////////////////////////////////////////////////////////////////////


   //
   //  Stereographic grid definitions
   //


////////////////////////////////////////////////////////////////////////


#endif   /*  __STEREOGRAPHIC_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



