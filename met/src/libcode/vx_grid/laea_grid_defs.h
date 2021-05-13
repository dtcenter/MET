// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2021
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __LAEA_GRID_DEFINITIONS_H__
#define  __LAEA_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


struct LaeaData {

   const char * name;

   const char * geoid;

   double lat_LL;   //  lower left
   double lon_LL;

   double lat_UL;   //  upper left
   double lon_UL;

   double lat_LR;   //  lower right
   double lon_LR;

   double lat_pole;
   double lon_pole;

   int nx;
   int ny;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __LAEA_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////
