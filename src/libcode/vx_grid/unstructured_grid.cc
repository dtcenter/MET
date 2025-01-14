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

using PointXYZ = atlas::PointXYZ;
using PointLonLat = atlas::PointLonLat;
using Geometry = atlas::Geometry;
using IndexKDTree = atlas::util::IndexKDTree;

////////////////////////////////////////////////////////////////////////

constexpr double EARTH_RADIUS = 6378137.0;          // Radius of the Earth (in meters)
constexpr double FLAT_FACTOR = 1.0/298.257223563;   // Flattening factor WGS84 Model


static const int UGRID_DEBUG_LEVEL = 9;

static atlas::Geometry atlas_geometry;

////////////////////////////////////////////////////////////////////////

void llh_to_ecef(double lat, double lon, double alt_m,
                 double *x_km, double *y_km, double *z_km);
void check_llh_to_ecef(double lat, double lon, double alt_m,
                       double true_x_km, double true_y_km, double true_z_km,
                       const string &location);

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

   if (data.has_PointLatLon()) {
      Data.set_points(Nx, data.points_lonlat);
   }
   else {
      Data.set_points(Nx, data.points_XYZ);
   }

}

////////////////////////////////////////////////////////////////////////

void UnstructuredGrid::set_max_distance_km(double max_distance) {

   Data.max_distance_km = max_distance;

}

////////////////////////////////////////////////////////////////////////


void UnstructuredGrid::latlon_to_xy(double lat, double lon, double &x, double &y) const {

   PointLonLat point_lonlat(lon, lat);

   IndexKDTree::ValueList neighbor = Data.closest_points(lat, lon, 1);
   size_t index(neighbor[0].payload());
   double distance_km(neighbor[0].distance()/1000.);
   bool in_distance = Data.is_in_distance(distance_km);

   x = in_distance ? index : -1.0;
   y = 0;

   if(mlog.verbosity_level() >= UGRID_DEBUG_LEVEL) mlog
        << Debug(UGRID_DEBUG_LEVEL) << "UnstructuredGrid::latlon_to_xy() "
        << "input=(" << lon << ", " << lat << ") ==> (" << x << ", " << y << ") == ("
        << Data.points_lonlat[index].x() << ", " << Data.points_lonlat[index].y()
        << ") distance= " << distance_km << "km, "
        << point_lonlat.distance(Data.points_lonlat[index])
        << " degree" << (in_distance ? " " : ", rejected") << "\n";
}


////////////////////////////////////////////////////////////////////////


void UnstructuredGrid::xy_to_latlon(double x, double y, double &lat, double &lon) const {

   lat = Data.points_lonlat[nint(x)].y();
   lon = Data.points_lonlat[nint(x)].x();

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
   if (has_PointLatLon()) {
      for (int i=0; i<n_face; i++) {
         PointLonLat pointLL(points_lonlat[i].x(), points_lonlat[i].y());
         pointLL.normalise();
         kdtree->insert(pointLL, n);
         n++;
         lat_checksum += (i+1) * points_lonlat[i].y();
         lon_checksum += (i+1) * points_lonlat[i].x();
      }
   }
   else {
      double x_km;
      double y_km;
      double z_km;
      points_XYZ_km.reserve(n_face);
      for (int i=0; i<n_face; i++) {
         llh_to_ecef(points_XYZ[i].y(), points_XYZ[i].x(),
                     points_XYZ[i].z(), &x_km, &y_km, &z_km);
         points_XYZ_km.emplace_back({x_km, y_km, z_km});
         kdtree->insert(points_XYZ_km[i], n);
         n++;
         lat_checksum += (i+1) * y_km;
         lon_checksum += (i+1) * x_km;
         alt_checksum += (i+1) * z_km;
      }
   }

   kdtree->build();

   ConcatString cs;
   if (get_env("MET_TEST_UGRID_KDTREE", cs)) test_kdtree();
}

////////////////////////////////////////////////////////////////////////

IndexKDTree::ValueList UnstructuredData::closest_points(const double &lat, const double &lon,
                                                        const size_t &k, const double &alt_m) const {
   const string method_name = "UnstructuredData::closest_points() --> ";
   if (has_PointLatLon()) {
      PointLonLat point_lonlat(lon, lat);
      if (!is_eq(alt_m, bad_data_double)) {
         mlog << Debug(4) << method_name << "ignored the altitude (" << alt_m << ")\n";
      }
      return kdtree->closestPoints(point_lonlat, k);
   }
   else {
      double x_km;
      double y_km;
      double z_km;
      double _alt_m = alt_m;
      if (is_eq(_alt_m, bad_data_double)) {
         _alt_m = EARTH_RADIUS;
         mlog << Debug(4) << method_name << "Set the altitude to earth radius (" << EARTH_RADIUS << ")\n";
      }
      llh_to_ecef(lat, lon, _alt_m, &x_km, &y_km, &z_km);
      PointXYZ point_XYZ(x_km, y_km, z_km);
      return kdtree->closestPoints(point_XYZ, k);
   }
};

////////////////////////////////////////////////////////////////////////

void UnstructuredData::copy_from(const UnstructuredData &us_data) {
   if (us_data.has_PointLatLon()) {
     set_points(us_data.n_face, us_data.points_lonlat);
   }
   else {
     set_points(us_data.n_face, us_data.points_XYZ);
   }
   n_edge = us_data.n_edge;
   n_node = us_data.n_node;
   max_distance_km = us_data.max_distance_km;
}

////////////////////////////////////////////////////////////////////////

void UnstructuredData::copy_from(const UnstructuredData *us_data) {
   if (us_data->has_PointLatLon()) {
      set_points(us_data->n_face, us_data->points_lonlat);
   }
   else {
      set_points(us_data->n_face, us_data->points_XYZ);
   }
   n_edge = us_data->n_edge;
   n_node = us_data->n_node;
   max_distance_km = us_data->max_distance_km;
}

////////////////////////////////////////////////////////////////////////

bool UnstructuredData::has_PointLatLon() const {
   return (!points_lonlat.empty());
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

void UnstructuredData::set_points(int count, const double *_lon, const double *_lat) {

   clear_data();

   n_face = count;
   points_lonlat.reserve(count);
   for (int i=0; i<count; i++) {
      points_lonlat.emplace_back({_lon[i], _lat[i]});
   }
   if(mlog.verbosity_level() >= UGRID_DEBUG_LEVEL) mlog
        << Debug(UGRID_DEBUG_LEVEL) << "UnstructuredData::set_points(int, double *, double *) first ("
        << points_lonlat[0].x() << ", " << points_lonlat[0].y() << ") and last ("
        << points_lonlat[count-1].x() << ", " << points_lonlat[count-1].y() << ") from ("
        << _lon[0] << ", " << _lat[0] << ") and ("
        << _lon[count-1] << ", " << _lat[count-1] << ")\n";

   build_tree();

}

////////////////////////////////////////////////////////////////////////

void UnstructuredData::set_points(int count, const double *_lon, const double *_lat, const double *_alt) {

   clear_data();

   n_face = count;
   points_XYZ.reserve(count);
   for (int i=0; i<count; i++) {
      points_XYZ.emplace_back({_lon[i], _lat[i], _alt[i]});
   }
   if(mlog.verbosity_level() >= UGRID_DEBUG_LEVEL) mlog
        << Debug(UGRID_DEBUG_LEVEL) << "UnstructuredData::set_points(int, double *lon, double *lat, double *alt) first ("
        << points_XYZ[0].x() << ", " << points_XYZ[0].y() << ", " << points_XYZ[0].z() << ") and last ("
        << points_XYZ[count-1].x() << ", " << points_XYZ[count-1].y() << ") from ("
        << _lon[0] << ", " << _lat[0] << ", " << _alt[0] << ") and ("
        << _lon[count-1] << ", " << _lat[count-1] << ")\n";

   build_tree();

}

////////////////////////////////////////////////////////////////////////

void UnstructuredData::set_points(int count, const std::vector<PointLonLat> &pointsLL) {
   static const string method_name
      = "UnstructuredData::set_points(int, std::vector<PointLonLat> &) -> ";

   clear_data();

   if (count != pointsLL.size()) {
      mlog << Warning << "\n" << method_name
           << " The count argument (" << count << ") does not match with vector ("
           << pointsLL.size() << ")\n\n";
   }

   n_face = count;
   points_lonlat.reserve(count);
   for (int i=0; i<count; i++) {
      points_lonlat.emplace_back({pointsLL[i].x(), pointsLL[i].y()});
   }
   if(mlog.verbosity_level() >= UGRID_DEBUG_LEVEL) mlog
        << Debug(UGRID_DEBUG_LEVEL) << method_name << " first: ("
        << points_lonlat[0].x() << ", " << points_lonlat[0].y() << ") and last ("
        << points_lonlat[count-1].x() << ", " << points_lonlat[count-1].y() << ") from ("
        << pointsLL[0].x() << ", " << pointsLL[0].y() << ") and ("
        << pointsLL[count-1].x() << ", " << pointsLL[count-1].y() << ")\n";

   build_tree();

}

////////////////////////////////////////////////////////////////////////

void UnstructuredData::set_points(int count, const std::vector<PointXYZ> &pointsXYZ) {
   static const string method_name
      = "UnstructuredData::set_points(int, std::vector<PointXYZ> &) -> ";

   clear_data();

   if (count != pointsXYZ.size()) {
      mlog << Warning << "\n" << method_name
           << " The count argument (" << count << ") does not match with vector ("
           << pointsXYZ.size() << ")\n\n";
   }

   n_face = count;
   points_XYZ.reserve(count);
   for (int i=0; i<count; i++) {
      points_XYZ.emplace_back({pointsXYZ[i].x(), pointsXYZ[i].y(), pointsXYZ[i].z()});
   }
   if(mlog.verbosity_level() >= UGRID_DEBUG_LEVEL) {
      int last_i = count - 1;
      mlog << Debug(UGRID_DEBUG_LEVEL) << method_name
           << "first: (" << points_XYZ[0].x() << ", " << points_XYZ[0].y() << ", "
           << points_XYZ[0].z() << ") and last (" << points_XYZ[last_i].x() << ", "
           << points_XYZ[last_i].y() << ", " << points_XYZ[last_i].z() << ") from ("
           << pointsXYZ[0].x() << ", " << pointsXYZ[0].y() << ", " << pointsXYZ[0].z()
           << ") and (" << pointsXYZ[last_i].x() << ", " << pointsXYZ[last_i].y()
           << ", " << pointsXYZ[last_i].z() << ")\n";
   }

   build_tree();

}

////////////////////////////////////////////////////////////////////////

void UnstructuredData::test_kdtree() {
   static const string method_name
      = "UnstructuredData::test_kdtree() -> ";

   int closest_n = 5;
   double lat;
   double lon;
   double distance_km;
   vector<int> indices  = {0, n_face/2, (n_face - 1)};
   if (has_PointLatLon()) {
      if (n_face != points_lonlat.size()) {
         mlog << Warning << "\n" << method_name
              << " The count argument (" << n_face << ") does not match with the vector (latlon="
              << points_lonlat.size() << ", XYZ=" << points_XYZ.size() << ")\n\n";
      }
      for (int idx : indices) {
         lat = points_lonlat[idx].y();
         lon = points_lonlat[idx].x();
         cout << " - search index=" << idx << " (" << lat << ", " << lon << ")\n";
         IndexKDTree::ValueList neighbor = closest_points(lon, lat, closest_n);
         for (const auto &x : neighbor) {
            int index = x.payload();
            distance_km = x.distance() / 1000.;
            cout << "  + closest index=" << index << "  distance=" << distance_km << " from ("
                 << points_lonlat[index].y() << ", " << points_lonlat[index].x() << ")\n";
         }
         cout << "\n";
      }
   }
   else {
      double x_km;
      double y_km;
      double z_km;
      double alt_m;
      if (n_face != points_XYZ.size()) {
         mlog << Warning << "\n" << method_name
              << " The count argument (" << n_face << ") does not match with the vector (XYZ="
              << points_XYZ.size() << ", latlon=" << points_lonlat.size() << ")\n\n";
      }
      for (int idx : indices) {
         lat = points_XYZ[idx].y();
         lon = points_XYZ[idx].x();
         alt_m = points_XYZ[idx].z();
         cout << " - search index=" << idx << " (" << lat << ", " << lon << ", " << alt_m << ")\n";
         IndexKDTree::ValueList neighbor = closest_points(lat, lon, closest_n, alt_m);
         for (const auto &x : neighbor) {
            int index = x.payload();
            distance_km = x.distance() / 1000.;
            llh_to_ecef(lat, lon, alt_m, &x_km, &y_km, &z_km);
            cout << "  + closest index=" << index << "  distance=" << distance_km << " from ("
                 << points_XYZ[index].y() << ", " << points_XYZ[index].x() << ", " << points_XYZ[index].z()
                 << ")  km: [" << x_km << ", " << y_km << ", " << z_km << "]\n";
         }
         cout << "\n";
      }
   }

}

////////////////////////////////////////////////////////////////////////
// Called by internal/test_util/libcode/vx_grid/search_3d_kdtree_api.cc
// Input: /d1/personal/dadriaan/projects/NRL/PyIRI/pyiri_f4_2020.nc
// - search index=   0 (-9.59874, 287.326, 100) ==> ( 1873.072, -6004.143, -1056.531) km
// - search index=1152 (-27.9711, 107.326, 100) ==> (-1678.837,  5381.522, -2973.724) km
// - search index=2303 ( 75.6264, 287.326, 100) ==> (  473.024, -1516.283,  6156.59 ) km

void UnstructuredData::test_llh_to_ecef() const {
   check_llh_to_ecef( 34.0522, -118.40806, 0., -2516.715, -4653.003,  3551.245, "           LA");
   check_llh_to_ecef(-9.59874, 287.326, 100.,   1873.072, -6004.143, -1056.531, "  First Point");
   check_llh_to_ecef(-27.9711, 107.326, 100.,  -1678.837,  5381.522, -2973.724, " Middle Point");
   check_llh_to_ecef( 75.6264, 287.326, 100.,    473.024, -1516.283,  6156.59,  "   Last Point");
}

////////////////////////////////////////////////////////////////////////

void llh_to_ecef(double lat, double lon, double alt_m, double *x_km, double *y_km, double *z_km) {
   const double lat_r = lat*rad_per_deg;
   const double lon_r = lon*rad_per_deg;
   double cosLat = cos(lat_r);
   double sinLat = sin(lat_r);
   double FF     = (1.0-FLAT_FACTOR)*(1.0-FLAT_FACTOR);
   double C      = 1./sqrt(cosLat*cosLat + FF * sinLat*sinLat);
   double S      = C * FF;

   *x_km = (EARTH_RADIUS * C + alt_m) * cosLat * cos(lon_r) / 1000.;
   *y_km = (EARTH_RADIUS * C + alt_m) * cosLat * sin(lon_r) / 1000.;
   *z_km = (EARTH_RADIUS * S + alt_m) * sinLat / 1000.;
}

////////////////////////////////////////////////////////////////////////

void check_llh_to_ecef(double lat, double lon, double alt_m, double true_x_km, double true_y_km, double true_z_km, const string &location) {
   double x_km;
   double y_km;
   double z_km;

   llh_to_ecef(lat, lon, alt_m, &x_km, &y_km, &z_km);
   cout << location << ":   (" << lat << ", " << lon << ", " << alt_m
        << ") => (" << x_km << ", " << y_km << ", " << z_km
        << ") Diff: (" << (true_x_km - x_km) << ", " << (true_y_km - y_km) << ", " << (true_z_km - z_km) << ")\n";

}

////////////////////////////////////////////////////////////////////////
