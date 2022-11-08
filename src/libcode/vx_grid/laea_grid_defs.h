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
//
// LaeaGrid, as defined in GRIB2
//
////////////////////////////////////////////////////////////////////////


struct LaeaData {

   const char * name;

   const char * geoid;

   double lat_1;
   double lon_1;

   double lat_std;
   double lon_cen;

   double dx_m;
   double dy_m;

   int nx;
   int ny;

   void dump() const;

};


////////////////////////////////////////////////////////////////////////
//
// LaeaGrid defined by the location of 3 corner points
//
////////////////////////////////////////////////////////////////////////


struct LaeaCornerData {

   const char * name;

   const char * geoid;

   double lat_ll;   //  lower left
   double lon_ll;

   double lat_ul;   //  upper left
   double lon_ul;

   double lat_lr;   //  lower right
   double lon_lr;

   double lat_pole;
   double lon_pole;

   int nx;
   int ny;

   void dump() const;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __LAEA_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////
