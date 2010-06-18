
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __LATLON_GRID_DEFINITIONS_H__
#define  __LATLON_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


struct LatLonData {

   const char * name;

   double lat_ll_deg;
   double lon_ll_deg;

   double delta_lat_deg;
   double delta_lon_deg;

   int Nlat;
   int Nlon;

};


////////////////////////////////////////////////////////////////////////


   //
   //  LatLon grid definitions
   //


////////////////////////////////////////////////////////////////////////


#endif   /*  __LATLON_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



