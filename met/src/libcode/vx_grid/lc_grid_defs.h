

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __LAMBERT_GRID_DEFINITIONS_H__
#define  __LAMBERT_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


struct LambertData {

   const char * name;

   char hemisphere;        //  'N' or 'S'

   double scale_lat_1;     //  scale latitude #1
   double scale_lat_2;     //  scale latitude #2

   double lat_pin;
   double lon_pin;

   double x_pin;
   double y_pin;

   double lon_orient;      //  alignment longitude

   double d_km;
   double r_km;

   int nx;
   int ny;

   double so2_angle;       //  rotation about pin point

   // LambertData() { so2_angle = 0.0; };

   void dump();

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __LAMBERT_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



