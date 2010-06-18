
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __EXPONENTIAL_GRID_DEFINITIONS_H__
#define  __EXPONENTIAL_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


struct ExpData {

   const char * name;

   double lat_origin_deg;
   double lon_origin_deg;

   double lat_2_deg;
   double lon_2_deg;

   double x_scale;   //  grid-units per fake-kilometer
   double y_scale;

   double x_offset;   //  grid-units
   double y_offset;

   int nx;
   int ny;

};


////////////////////////////////////////////////////////////////////////


   //
   //  Exp grid definitions
   //


////////////////////////////////////////////////////////////////////////


#endif   /*  __EXPONENTIAL_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



