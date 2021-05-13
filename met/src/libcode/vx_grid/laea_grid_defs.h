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


   //
   //  Laea grid definitions
   //


static const LaeaData eumetnet_data = {


   "Laea",      //  name

   "WGS_84",    //  geoid

   31.7462,     // lat_LL   //  lower left
   10.4346,     // lon_LL

   67.0228,     // lat_UL   //  upper left
   39.5358,     // lon_UL

   31.9877,     // lat_LR   //  lower right
   -29.421,     // lon_LR
   
    55.0,       // lat_pole
   -10.0,       // lon_pole

   1900,        //  nx
   2200,        //  ny

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __LAEA_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////
