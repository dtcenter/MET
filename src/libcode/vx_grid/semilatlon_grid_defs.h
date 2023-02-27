// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __SEMILATLON_GRID_DEFINITIONS_H__
#define  __SEMILATLON_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


struct SemiLatLonData {

   const char * name;

   // Arrays to define SemiLatLon dimensions

   NumArray lats;
   NumArray lons;
   NumArray levels;
   NumArray times;

   void dump();
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __SEMILATLON_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////
