// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
   double max_distance_km;  // This should be set after calling set_points()
   double lat_checksum;
   double lon_checksum;

   std::vector<atlas::PointLonLat> pointLonLat;
   atlas::util::IndexKDTree *kdtree;

   UnstructuredData();
   ~UnstructuredData();

   void build_tree();
   bool is_in_distance(double distance_km) const;
   void set_points(int count, double *_lon, double *_lat);
   void set_points(int count, const std::vector<atlas::PointLonLat> &);
   void copy_from(const UnstructuredData *);
   void copy_from(const UnstructuredData &);
   void clear();
   void clear_data();

   void dump() const;
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __UNSTRUCTURTED_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////
