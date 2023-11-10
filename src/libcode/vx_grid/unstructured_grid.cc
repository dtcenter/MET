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
   Nx = data.Nface;

   Data.Nface = Nx;
   Data.Nedge = data.Nedge;
   Data.Nnode = data.Nnode;
   Data.max_distance_km = data.max_distance_km;

   Data.set_points(Nx, data.pointLonLat);

}

////////////////////////////////////////////////////////////////////////

void UnstructuredGrid::set_max_distance_km(double max_distance) {

   Data.max_distance_km = max_distance;

}

////////////////////////////////////////////////////////////////////////


void UnstructuredGrid::latlon_to_xy(double lat, double lon, double &x, double &y) const {

   PointLonLat _pointLonLat(lon, lat);

   IndexKDTree::ValueList neighbor = Data.kdtree->closestPoints(_pointLonLat, 1);
   size_t index(neighbor[0].payload());
   double distance_km(neighbor[0].distance()/1000.);
   bool is_rejected = (!is_eq(Data.max_distance_km, bad_data_double)
                       && Data.max_distance_km > 0
                       && Data.max_distance_km < distance_km);

   x = is_rejected ? -1.0 : index;
   y = 0;

   //PointLonLat r_lonlat;
   //if(_distance > 180.0) atlas_geometry.xyz2lonlat(neighbor[0].point(), r_lonlat);
   //else r_lonlat.assign(neighbor[0].point()[0], neighbor[0].point()[1]);

   if(mlog.verbosity_level() >= UGRID_DEBUG_LEVEL) mlog
        << Debug(UGRID_DEBUG_LEVEL) << "UnstructuredGrid::latlon_to_xy() "
        << "input=(" << lon << ", " << lat << ") ==> (" << x << ", " << y << ") == ("
        << Data.pointLonLat[index].x() << ", " << Data.pointLonLat[index].y()
        << ") distance= " << distance_km << "km, "
        << _pointLonLat.distance(Data.pointLonLat[index])
        << " degree, is_rejected: " << is_rejected
        << ", max_distanc=" << Data.max_distance_km<< "\n";
}


////////////////////////////////////////////////////////////////////////


void UnstructuredGrid::xy_to_latlon(double x, double y, double &lat, double &lon) const {

   lat = Data.pointLonLat[x].y();
   lon = Data.pointLonLat[x].x();

   if(mlog.verbosity_level() >= UGRID_DEBUG_LEVEL) mlog
        << Debug(UGRID_DEBUG_LEVEL) << "UnstructuredGrid::xy_to_latlon() "
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
   kdtree->reserve(Nface);
   for (int i=0; i<Nface; i++) {
      PointLonLat pointLL(pointLonLat[i].x(), pointLonLat[i].y());
      pointLL.normalise();
      kdtree->insert(pointLL, n++);
      lat_checksum += (i+1) * pointLonLat[i].y();
      lon_checksum += (i+1) * pointLonLat[i].x();
   }

   kdtree->build();

}

////////////////////////////////////////////////////////////////////////

void UnstructuredData::copy_from(const UnstructuredData &us_data) {
   set_points(us_data.Nface, us_data.pointLonLat);
   Nedge = us_data.Nedge;
   Nnode = us_data.Nnode;
   max_distance_km = us_data.max_distance_km;
}

////////////////////////////////////////////////////////////////////////

void UnstructuredData::copy_from(const UnstructuredData *us_data) {
   set_points(us_data->Nface, us_data->pointLonLat);
   Nedge = us_data->Nedge;
   Nnode = us_data->Nnode;
   max_distance_km = us_data->max_distance_km;
}

////////////////////////////////////////////////////////////////////////

void UnstructuredData::set_points(int count, double *_lon, double *_lat) {

   clear_data();

   Nface = count;
   pointLonLat.reserve(count);
   for (int i=0; i<count; i++) {
      pointLonLat[i] = {_lon[i], _lat[i]};
   }
   if(mlog.verbosity_level() >= UGRID_DEBUG_LEVEL) mlog
        << Debug(UGRID_DEBUG_LEVEL) << "UnstructuredData::set_points(int, double *, double *) first ("
        << pointLonLat[0].x() << ", " << pointLonLat[0].y() << ") and last ("
        << pointLonLat[count-1].x() << ", " << pointLonLat[count-1].y() << ") from ("
        << _lon[0] << ", " << _lat[0] << ") and ("
        << _lon[count-1] << ", " << _lat[count-1] << ")\n";

   build_tree();

}

////////////////////////////////////////////////////////////////////////

void UnstructuredData::set_points(int count, const std::vector<PointLonLat> &ptLonLat) {

   clear_data();

   Nface = count;
   pointLonLat.reserve(count);
   for (int i=0; i<count; i++) {
      pointLonLat[i] = {(ptLonLat)[i].x(), (ptLonLat)[i].y()};
   }
   if(mlog.verbosity_level() >= UGRID_DEBUG_LEVEL) mlog
        << Debug(UGRID_DEBUG_LEVEL) << "UnstructuredData::set_points(std::vector<PointLonLat> &) first: ("
        << pointLonLat[0].x() << ", " << pointLonLat[0].y() << ") and last ("
        << pointLonLat[count-1].x() << ", " << pointLonLat[count-1].y() << ") from ("
        << ptLonLat[0].x() << ", " << ptLonLat[0].y() << ") and ("
        << ptLonLat[count-1].x() << ", " << ptLonLat[count-1].y() << ")\n";

   build_tree();

}

////////////////////////////////////////////////////////////////////////
