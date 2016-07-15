

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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

   const char * name;   //  not allocated

   double lat_ll;
   double lon_ll;

   double delta_lat;
   double delta_lon;

   int Nlat;
   int Nlon;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __LATLON_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



