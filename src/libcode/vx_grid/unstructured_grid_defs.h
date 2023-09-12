// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __UNSTRUCTURTED_GRID_DEFINITIONS_H__
#define  __UNSTRUCTURTED_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "atlas/grid/Grid.h"    // PointLonLat
#include "atlas/util/Geometry.h"
#include "atlas/util/KDTree.h"


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////




struct UnstructuredData {

   const char * name;   //  not allocated

   int Nface;
   int Nedge;
   int Nnode;

   std::vector<atlas::PointLonLat> pointLonLat;
   atlas::util::IndexKDTree *kdtree;

   UnstructuredData();
   ~UnstructuredData();

   void build_tree();
   void set_points(int count, double *_lon, double *_lat);
   void set_points(int count, const std::vector<atlas::PointLonLat> &);
   void clear();

   void dump() const;
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __UNSTRUCTURTED_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////
