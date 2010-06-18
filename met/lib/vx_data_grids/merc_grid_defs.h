
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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

   double lat_ll_deg;
   double lon_ll_deg;

   double lat_ur_deg;
   double lon_ur_deg;

   // double delta_i;
   // double delta_j;

   int nx;
   int ny;

};


////////////////////////////////////////////////////////////////////////


   //
   //  Mercator grid definitions
   //


////////////////////////////////////////////////////////////////////////


#endif   /*  __MERCATOR_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



