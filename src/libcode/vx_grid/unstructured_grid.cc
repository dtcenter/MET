// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_math.h"
#include "vx_util.h"
#include "vx_log.h"
#include "unstructured_grid.h"

#include "atlas/grid/Grid.h"    // PointLonLat
#include "atlas/util/Geometry.h"
#include "atlas/util/KDTree.h"

using namespace std;

using PointLonLat = atlas::PointLonLat;
using Geometry = atlas::Geometry;
using IndexKDTree = atlas::util::IndexKDTree;

////////////////////////////////////////////////////////////////////////

static const int UGRID_DEBUG_LEVEL = 9;

static atlas::Geometry atlas_geometry;

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class UnstructuredGrid
   //


////////////////////////////////////////////////////////////////////////


UnstructuredGrid::UnstructuredGrid() {
   clear();
}


////////////////////////////////////////////////////////////////////////


UnstructuredGrid::~UnstructuredGrid() {
   clear();
}


////////////////////////////////////////////////////////////////////////


void UnstructuredGrid::clear() {

   Name.clear();

   Nx = 0;
   wrapLon = false;
   //pt_distance = -1.;  // Disabled

   Data.clear(); 
   return;

}


////////////////////////////////////////////////////////////////////////


UnstructuredGrid::UnstructuredGrid(const UnstructuredData & data) {

   set_from_data(data);

}


////////////////////////////////////////////////////////////////////////

void UnstructuredGrid::set_from_data(const UnstructuredData &data) {

   clear();

   if (data.name) Name = data.name;
   Nx = data.n_face;

   Data.n_face = Nx;
   Data.n_edge = data.n_edge;
   Data.n_node = data.n_node;
   Data.max_distance_km = data.max_distance_km;

   Data.set_points(Nx, data.point_lonlat);

}

////////////////////////////////////////////////////////////////////////

void UnstructuredGrid::set_max_distance_km(double max_distance) {

   Data.max_distance_km = max_distance;

}

////////////////////////////////////////////////////////////////////////


void UnstructuredGrid::latlon_to_xy(double lat, double lon, double &x, double &y) const {

   PointLonLat _point_lonlat(lon, lat);

   IndexKDTree::ValueList neighbor = Data.kdtree->closestPoints(_point_lonlat, 1);
   size_t index(neighbor[0].payload());
   double distance_km(neighbor[0].distance()/1000.);
   bool in_distance = Data.is_in_distance(distance_km);

   x = in_distance ? index : -1.0;
   y = 0;

   if(mlog.verbosity_level() >= UGRID_DEBUG_LEVEL) mlog
        << Debug(UGRID_DEBUG_LEVEL) << "UnstructuredGrid::latlon_to_xy() "
        << "input=(" << lon << ", " << lat << ") ==> (" << x << ", " << y << ") == ("
        << Data.point_lonlat[index].x() << ", " << Data.point_lonlat[index].y()
        << ") distance= " << distance_km << "km, "
        << _point_lonlat.distance(Data.point_lonlat[index])
        << " degree" << (in_distance ? " " : ", rejected") << "\n";
}


////////////////////////////////////////////////////////////////////////


void UnstructuredGrid::xy_to_latlon(double x, double y, double &lat, double &lon) const {

   lat = Data.point_lonlat[x].y();
   lon = Data.point_lonlat[x].x();

   if(mlog.verbosity_level() >= UGRID_DEBUG_LEVEL) mlog
        << Debug(UGRID_DEBUG_LEVEL) << "UnstructuredGrid::xy_to_latlon() "
        << "(" << x << ", " << y << ") ==> (" << lon << ", " << lat << ").\n";

}


////////////////////////////////////////////////////////////////////////

double UnstructuredGrid::calc_area(int x, int y) const {

   double area = 0.;

   return area;

}

////////////////////////////////////////////////////////////////////////


int UnstructuredGrid::nx() const {

   return Nx;

}


////////////////////////////////////////////////////////////////////////


int UnstructuredGrid::ny() const {

   return 1;

}


////////////////////////////////////////////////////////////////////////


ConcatString UnstructuredGrid::name() const {

   return Name;

}


////////////////////////////////////////////////////////////////////////


void UnstructuredGrid::dump(ostream & out, int depth) const {

Indent prefix(depth);

out << prefix << "Name         = ";

if ( Name.length() > 0 )  out << '\"' << Name << '\"';
else                      out << "(nul)\n";

out << '\n';

out << prefix << "n_face       = "  << Nx << "\n";

   //
   //  done
   //
out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString UnstructuredGrid::serialize(const char *sep) const {

ConcatString a;

a << "Projection: UnstructuredGrid" << sep;

a << "n_face: " << Nx << sep;
   //
   //  done
   //

return a;

}


////////////////////////////////////////////////////////////////////////


GridInfo UnstructuredGrid::info() const {

GridInfo i;

i.set( Data );

return i;

}


////////////////////////////////////////////////////////////////////////

double UnstructuredGrid::rot_grid_to_earth(int x, int y) const

{

//
// The rotation angle from grid relative to earth relative is zero
// for the PlateCarreeGrids that are defined north and east.  This may
// need to be changed when support is added for GRIB2.
//

return 0.0;

}

////////////////////////////////////////////////////////////////////////

void UnstructuredGrid::shift_right(int N)
{

   mlog << Warning << "\nUnstructuredGrid::shift_right(int) -> not implemented\n\n";

}

////////////////////////////////////////////////////////////////////////


GridRep * UnstructuredGrid::copy() const {

  UnstructuredGrid *p = new UnstructuredGrid (Data);

  p->Name = Name;

  return p;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


Grid::Grid(const UnstructuredData &data) {

   init_from_scratch();

   set(data);

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const UnstructuredData &data) {
   clear();

   rep = new UnstructuredGrid ( data );
   if ( !rep )  {
      mlog << Error << "\nGrid::set(const Unstructured &) -> memory allocation error\n\n";
      exit ( 1 );
   }
}


////////////////////////////////////////////////////////////////////////


UnstructuredData::UnstructuredData() {
   kdtree = nullptr;
   max_distance_km = bad_data_double;  // disable distance
   clear();
}

////////////////////////////////////////////////////////////////////////

UnstructuredData::~UnstructuredData() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void UnstructuredData::build_tree() {

   atlas::idx_t n = 0;
   kdtree = new IndexKDTree(atlas_geometry);
   kdtree->reserve(n_face);
   for (int i=0; i<n_face; i++) {
      PointLonLat pointLL(point_lonlat[i].x(), point_lonlat[i].y());
      pointLL.normalise();
      kdtree->insert(pointLL, n++);
      lat_checksum += (i+1) * point_lonlat[i].y();
      lon_checksum += (i+1) * point_lonlat[i].x();
   }

   kdtree->build();

}

////////////////////////////////////////////////////////////////////////

void UnstructuredData::copy_from(const UnstructuredData &us_data) {
   set_points(us_data.n_face, us_data.point_lonlat);
   n_edge = us_data.n_edge;
   n_node = us_data.n_node;
   max_distance_km = us_data.max_distance_km;
}

////////////////////////////////////////////////////////////////////////

void UnstructuredData::copy_from(const UnstructuredData *us_data) {
   set_points(us_data->n_face, us_data->point_lonlat);
   n_edge = us_data->n_edge;
   n_node = us_data->n_node;
   max_distance_km = us_data->max_distance_km;
}

////////////////////////////////////////////////////////////////////////

bool UnstructuredData::is_in_distance(double distance_km) const {
   bool in_distance = is_eq(max_distance_km, bad_data_double)
                      || (max_distance_km <= 0)
                      || (max_distance_km >= distance_km);
   //if (!in_distance) rejectedCount++;
   //totalCount++;
   return in_distance;
}

////////////////////////////////////////////////////////////////////////

void UnstructuredData::set_points(int count, double *_lon, double *_lat) {

   clear_data();

   n_face = count;
   point_lonlat.reserve(count);
   for (int i=0; i<count; i++) {
      point_lonlat[i] = {_lon[i], _lat[i]};
   }
   if(mlog.verbosity_level() >= UGRID_DEBUG_LEVEL) mlog
        << Debug(UGRID_DEBUG_LEVEL) << "UnstructuredData::set_points(int, double *, double *) first ("
        << point_lonlat[0].x() << ", " << point_lonlat[0].y() << ") and last ("
        << point_lonlat[count-1].x() << ", " << point_lonlat[count-1].y() << ") from ("
        << _lon[0] << ", " << _lat[0] << ") and ("
        << _lon[count-1] << ", " << _lat[count-1] << ")\n";

   build_tree();

}

////////////////////////////////////////////////////////////////////////

void UnstructuredData::set_points(int count, const std::vector<PointLonLat> &ptLonLat) {

   clear_data();

   n_face = count;
   point_lonlat.reserve(count);
   for (int i=0; i<count; i++) {
      point_lonlat[i] = {(ptLonLat)[i].x(), (ptLonLat)[i].y()};
   }
   if(mlog.verbosity_level() >= UGRID_DEBUG_LEVEL) mlog
        << Debug(UGRID_DEBUG_LEVEL) << "UnstructuredData::set_points(int, std::vector<PointLonLat> &) first: ("
        << point_lonlat[0].x() << ", " << point_lonlat[0].y() << ") and last ("
        << point_lonlat[count-1].x() << ", " << point_lonlat[count-1].y() << ") from ("
        << ptLonLat[0].x() << ", " << ptLonLat[0].y() << ") and ("
        << ptLonLat[count-1].x() << ", " << ptLonLat[count-1].y() << ")\n";

   build_tree();

}

////////////////////////////////////////////////////////////////////////
