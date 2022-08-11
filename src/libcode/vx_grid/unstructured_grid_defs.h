// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __UNSTRUCTURED_GRID_DEFINITIONS_H__
#define  __UNSTRUCTURED_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


struct UnstructuredData {

   const char * name;

   // Exactly two are non-empty

   NumArray lats;
   NumArray lons;
   NumArray levels;
   NumArray times;

   void dump();
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __UNSTRUCTURED_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////
