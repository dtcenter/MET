

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __MERCATOR_GRID_DEFINITIONS_H__
#define  __MERCATOR_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


struct MercatorData {

   const char * name;

   double lat_ll;
   double lon_ll;

   double lat_ur;
   double lon_ur;

   int nx;
   int ny;

   void dump();
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MERCATOR_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



