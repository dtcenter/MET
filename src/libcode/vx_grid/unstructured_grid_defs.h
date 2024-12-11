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

   const char *name;   //  not allocated

   int n_face;
   int n_edge;
   int n_node;
   double max_distance_km;  // This should be set after calling set_points()
   double lat_checksum;
   double lon_checksum;
   double alt_checksum;

   std::vector<atlas::PointLonLat> points_lonlat;
   std::vector<atlas::PointXYZ> points_XYZ;     // lat_deg, lon_der, alt_meters
   std::vector<atlas::PointXYZ> points_XYZ_km;  // x_km, y_km, z_km
   atlas::util::IndexKDTree *kdtree;

   UnstructuredData();
   ~UnstructuredData();

   void build_tree();
   bool is_in_distance(double distance_km) const;
   void set_points(int count, double *_lon, double *_lat);
   void set_points(int count, const std::vector<atlas::PointLonLat> &);
   void set_points(int count, double *_lon, double *_lat, double *_alt);
   void set_points(int count, const std::vector<atlas::PointXYZ> &);
   void copy_from(const UnstructuredData *);
   void copy_from(const UnstructuredData &);
   void clear();
   void clear_data();
   bool has_PointLatLon() const;
   void test_kdtree();
   void test_llh_to_ecef();
   atlas::util::IndexKDTree::ValueList closest_points(
        const double &lat, const double &lon, const size_t &k,
        const double &alt_m=bad_data_double) const;


   void dump() const;
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __UNSTRUCTURTED_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////
