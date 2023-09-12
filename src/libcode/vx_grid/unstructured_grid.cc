// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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

   Data.clear(); 
   return;

}


////////////////////////////////////////////////////////////////////////


UnstructuredGrid::UnstructuredGrid(const UnstructuredData & data) {

   set_from_data(data);

}


////////////////////////////////////////////////////////////////////////

void UnstructuredGrid::set_from_data(const UnstructuredData &data) {

   Data.clear();

   if (data.name) Name = data.name;
   Nx = data.Nface;;

   Data.Nface = Nx;
   Data.Nedge = data.Nedge;
   Data.Nnode = data.Nnode;

   Data.set_points(Nx, data.pointLonLat);

   return;

}

////////////////////////////////////////////////////////////////////////


void UnstructuredGrid::latlon_to_xy(double lat, double lon, double &x, double &y) const {

   PointLonLat _pointLonLat(lon, lat);
   y = 0;

   IndexKDTree::ValueList neighbor = Data.kdtree->closestPoints(_pointLonLat, 1);
   size_t index(neighbor[0].payload());
   x = index;

   mlog << Debug(7) << "UnstructuredGrid::latlon_to_xy() "
        << "(" << lon << ", " << lat << ") ==> (" << x << ", " << y << ")\n";

}


////////////////////////////////////////////////////////////////////////


void UnstructuredGrid::xy_to_latlon(double x, double y, double &lat, double &lon) const {

   lat = Data.pointLonLat[x].y();
   lon = Data.pointLonLat[x].x();

   mlog << Debug(7) << "UnstructuredGrid::xy_to_latlon() "
        << "(" << x << ", " << y << ") ==> (" << lon << ", " << lat << ").\n";

}


////////////////////////////////////////////////////////////////////////

double UnstructuredGrid::calc_area(int x, int y) const {

   double area = 0.;

   return ( area );

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

out << prefix << "Nface       = "  << Nx << "\n";

   //
   //  done
   //
out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString UnstructuredGrid::serialize(const char *sep) const {

ConcatString a;
char junk[256];


a << "Projection: UnstructuredGrid" << sep;

a << "Nface: " << Nx << sep;
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
   for (int i=0; i<Nface; i++) {
      PointLonLat pointLL(pointLonLat[i].x(), pointLonLat[i].y());
      pointLL.normalise();
      kdtree->insert(pointLL, n++);
   }

   kdtree->build();

}

////////////////////////////////////////////////////////////////////////

void UnstructuredData::set_points(int count, double *_lon, double *_lat) {

   clear();

   Nface = count;
   pointLonLat.reserve(Nface);
   for (int i=0; i<count; i++) {
      pointLonLat[i] = {_lon[i], _lat[i]};
   }
   mlog << Debug(7) << "UnstructuredData::set_points(int, double *, double *) ("
        << pointLonLat[0].x() << ", " << pointLonLat[0].y() << ") and ("
        << pointLonLat[count-1].x() << ", " << pointLonLat[count-1].y() << ") from ("
        << _lon[0] << ", " << _lat[0] << ") and ("
        << _lon[count-1] << ", " << _lat[count-1] << ")\n";

   build_tree();

}

////////////////////////////////////////////////////////////////////////

void UnstructuredData::set_points(int count, const std::vector<PointLonLat> &_pointLonLat) {

   clear();

   Nface = count;
   pointLonLat.reserve(Nface);

   for (int i=0; i<count; i++) {
      pointLonLat[i] = {(_pointLonLat)[i].x(), (_pointLonLat)[i].y()};
   }
   mlog << Debug(7) << "UnstructuredData::set_points(std::vector<PointLonLat> &) ("
        << pointLonLat[0].x() << ", " << pointLonLat[0].y() << ") and ("
        << pointLonLat[count-1].x() << ", " << pointLonLat[count-1].y() << ") from ("
        << _pointLonLat[0].x() << ", " << _pointLonLat[0].y() << ") and ("
        << _pointLonLat[count-1].x() << ", " << _pointLonLat[count-1].y() << ")\n";

   build_tree();

}

////////////////////////////////////////////////////////////////////////
