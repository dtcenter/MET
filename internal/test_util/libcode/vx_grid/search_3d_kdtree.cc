// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

// ./search_3d_kdtree /d1/personal/dadriaan/projects/NRL/PyIRI/pyiri_f4_2020.nc -23.114 334.77 100.
// ./search_3d_kdtree /d1/personal/dadriaan/projects/NRL/PyIRI/pyiri_f4_2020.nc 34.0522 -118.40806 0. glat glon zalt
//
// lat=-23.114, lon=334.77, alt=100 (meters)
//                     ==> X: 5309.352 km, Y: -2501.788 km, Z: -2488.375 km
// index=0    -9.59874, 287.326, 100] to [812813, 6.22979e+06, 1.09673e+06])
//                     ==> X: 1873.072 km, Y: -6004.143 km, Z: -1056.531 km
// index=1152 -27.9711, 107.326, 100] to [-5.30871e+06, -2.98241e+06, -1.89224e+06])
//                     ==> X: -1678.837 km, Y: 5381.522 km, Z: -2973.724 km
// index=2303  75.6264, 287.326, 100] to [-803942, -6.16181e+06, 1.43314e+06])
//                     ==> X: 473.024 km, Y: -1516.283 km, Z: 6156.59 km

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include <netcdf>

#include "atlas/grid/Grid.h"    // PointXYZ
#include "atlas/util/Geometry.h"
#include "atlas/util/KDTree.h"

#include "vx_util.h"
#include "vx_grid.h"
#include "nav.h"
#include "nc_utils_core.h"
#include "nc_utils.h"

using namespace std;
using namespace netCDF;

using PointXYZ = atlas::PointXYZ;
using PointLonLat = atlas::PointLonLat;
using Geometry = atlas::Geometry;
using IndexKDTree = atlas::util::IndexKDTree;

////////////////////////////////////////////////////////////////////////

static Geometry atlas_geometry;

IndexKDTree *build_tree(vector<double> lat, vector<double> lon, vector<double> alt);
void llh_to_ecef(double lat, double lon, double alt, double *x_km, double *y_km, double *z_km);
void llh_to_ecef_radian(double lat_r, double lon_r, double alt, double *x_km, double *y_km, double *z_km);

////////////////////////////////////////////////////////////////////////


int main(int argc, char *argv[])
{

   string lat_name = "glat";
   string lon_name = "glon";
   string alt_name = "zalt";

   if(argc < 2) cerr << "\n" << argv[0]
           << ": Must specify input NetCDF name to process!\n\n";
   if(argc < 3) cerr << "\n" << argv[0]
           << ": Must specify latitude to search\n\n";
   if(argc < 4) cerr << "\n" << argv[0]
           << ": Must specify longitude to search\n\n";
   if(argc < 5) cerr << "\n" << argv[0]
           << ": Must specify altitude to search\n\n";

   if(argc < 5) {
      cerr << "\n  Usage: " << argv[0]
           << " netcdf_name lat_deg lon_deg alt_meters <lat_var_name> <lon_var_name> <alt_var_name>\n";
      cerr << "             def_lat_var_name=" << lat_name
           << ", def_lon_var_name=" << lon_name
           << ", def_alt_var_name=" << alt_name << "\n\n";
      exit(1);
   }

NcFile * _ncFile = open_ncfile(argv[1]);
if (IS_INVALID_NC_P(_ncFile)) {
  if (_ncFile) {
    delete _ncFile;
    _ncFile = (NcFile *)nullptr;
  }
   exit(1);
}

double lat = std::stod(argv[2]);
double lon = std::stod(argv[3]);
double alt = std::stod(argv[4]);

if(argc > 5) lat_name = argv[5];
if(argc > 6) lon_name = argv[6];
if(argc > 7) alt_name = argv[7];

cout << "Requested lat=" << lat << ", lon=" << lon << ", alt=" << alt
     << ",  lat_name=" << lat_name << ", lon_name=" << lon_name
     << ", alt_name=" << alt_name << "\n\n";

NcVar lat_var = get_nc_var(_ncFile, lat_name);
NcVar lon_var = get_nc_var(_ncFile, lon_name);
NcVar alt_var = get_nc_var(_ncFile, alt_name);


const int nlat = get_data_size(&lat_var);
const int nlon = get_data_size(&lon_var);
const int nalt = get_data_size(&alt_var);
vector<double> lat_values(nlat);
vector<double> lon_values(nlon);
vector<double> alt_values(nalt);

if (!get_nc_data(&lat_var, lat_values.data())) {
   cerr << "\n"
        << " Fail to read " << lat_name << "\n\n";
}
if (!get_nc_data(&lon_var, lon_values.data())) {
   cerr << "\n"
        << " Fail to read " << lon_name << "\n\n";
}
if (!get_nc_data(&alt_var, alt_values.data())) {
   cerr << "\n"
        << " Fail to read " << alt_name << "\n\n";
}

double x,y,z;
double x_f,y_f,z_f;
double x_m,y_m,z_m;
double x_l,y_l,z_l;

llh_to_ecef(lat, lon, alt, &x, &y, &z);
PointXYZ target_point(x, y, z);

int idx = 0;
llh_to_ecef(lat_values[idx], lon_values[idx], alt_values[idx], &x_f, &y_f, &z_f);
PointXYZ target_point_first(x_f, y_f, z_f);

idx = lat_values.size()/2;
llh_to_ecef(lat_values[idx], lon_values[idx], alt_values[idx], &x_m, &y_m, &z_m);
PointXYZ target_point_mid(x_m, y_m, z_m);

idx = lat_values.size() - 1;
llh_to_ecef(lat_values[idx], lon_values[idx], alt_values[idx], &x_l, &y_l, &z_l);
PointXYZ target_point_last(x_l, y_l, z_l);

//bool in_distance;
int closest_n = 5;
double distance_km;
IndexKDTree *kdtree = build_tree(lat_values, lon_values, alt_values);

idx = 0;
cout << "The first point from the input file: "
     << " index=" << idx << " " << lat_values[idx] << ", " << lon_values[idx] << ", " << alt_values[idx] << "] to [" << x_f << ", " << y_f << ", " << z_f << "])\n";
IndexKDTree::ValueList neighbor = kdtree->closestPoints(target_point_first, closest_n);
for (size_t i=0; i < neighbor.size(); i++) {
   size_t index(neighbor[i].payload());
   int index2 = (int)neighbor[i].payload();
   distance_km = neighbor[i].distance() / 1000.;
   //in_distance = is_in_distance(distance_km);
   llh_to_ecef(lat_values[index], lon_values[index], alt_values[index], &x, &y, &z);
   cout << " - " << i << ": index=" << index << "  distance=" << distance_km << " from ["
        << lat_values[index] << ", " << lon_values[index] << ", " << alt_values[index] << "] ("
        << x << ", " << y << ", " << z << ")\n";
}
cout << "\n";

idx = lat_values.size()/2;
cout << "The mid point from the input file: "
     << " index=" << idx << " " << lat_values[idx] << ", " << lon_values[idx] << ", " << alt_values[idx] << "] to [" << x_m << ", " << y_m << ", " << z_m << "])\n";
neighbor = kdtree->closestPoints(target_point_mid, closest_n);
for (size_t i=0; i < neighbor.size(); i++) {
   size_t index(neighbor[i].payload());
   int index2 = (int)neighbor[i].payload();
   distance_km = neighbor[i].distance() / 1000.;
   //in_distance = is_in_distance(distance_km);
   llh_to_ecef(lat_values[index], lon_values[index], alt_values[index], &x, &y, &z);
   cout << " - " << i << ": index=" << index << "  distance=" << distance_km << " from ["
        << lat_values[index] << ", " << lon_values[index] << ", " << alt_values[index] << "] ("
        << x << ", " << y << ", " << z << ")\n";
}
cout << "\n";

idx = lat_values.size() - 1;
cout << "The last point from the input file: "
     << " index=" << idx << " " << lat_values[idx] << ", " << lon_values[idx] << ", " << alt_values[idx] << "] to [" << x_m << ", " << y_m << ", " << z_m << "])\n";
neighbor = kdtree->closestPoints(target_point_last, closest_n);
for (size_t i=0; i < neighbor.size(); i++) {
   size_t index(neighbor[i].payload());
   int index2 = (int)neighbor[i].payload();
   distance_km = neighbor[i].distance() / 1000.;
   //in_distance = is_in_distance(distance_km);
   llh_to_ecef(lat_values[index], lon_values[index], alt_values[index], &x, &y, &z);
   cout << " - " << i << ": index=" << index << "  distance=" << distance_km << " from ["
        << lat_values[index] << ", " << lon_values[index] << ", " << alt_values[index] << "] ("
        << x << ", " << y << ", " << z << ")\n";
}
cout << "\n";

cout << "The requested point: "
     << lat << ", " << lon << ", " << alt
     << "] to [" << x << ", " << y << ", " << z << "])\n";
neighbor = kdtree->closestPoints(target_point, closest_n);
for (size_t i=0; i < neighbor.size(); i++) {
   size_t index(neighbor[i].payload());
   int index2 = (int)neighbor[i].payload();
   distance_km = neighbor[i].distance() / 1000.;
   //in_distance = is_in_distance(distance_km);
   llh_to_ecef(lat_values[index], lon_values[index], alt_values[index], &x, &y, &z);
   cout << " - " << i << ": index=" << index << "  distance=" << distance_km << " from ["
        << lat_values[index] << ", " << lon_values[index] << ", " << alt_values[index] << "] ("
        << x << ", " << y << ", " << z << ")\n";
}
cout << "\n";


if (_ncFile) {
   delete _ncFile;
   _ncFile = (NcFile *)nullptr;
}

   //
   //  done
   //

   return 0;

}

IndexKDTree *build_tree(vector<double> lat, vector<double> lon, vector<double> alt) {

   double x;
   double y;
   double z;
   int count = lat.size();
   atlas::idx_t n = 0;
   IndexKDTree *kdtree = new IndexKDTree(atlas_geometry);
   kdtree->reserve(count);
   for (int i=0; i<count; i++) {
      //llh_to_ecef(lat[i]*rad_per_deg, lon[i]*rad_per_deg, alt[i], &x, &y, &z);
      llh_to_ecef(lat[i], lon[i], 0, &x, &y, &z);
      PointXYZ point(x, y, z);
      kdtree->insert(point, n++);
   }

   kdtree->build();

   return kdtree;
}

void llh_to_ecef(double lat, double lon, double alt, double *x_km, double *y_km, double *z_km) {
   llh_to_ecef_radian(lat*rad_per_deg, lon*rad_per_deg, alt, x_km, y_km, z_km);
}

void llh_to_ecef_radian(double lat_r, double lon_r, double alt, double *x_km, double *y_km, double *z_km) {
   const double radius = 6378137.0;                 // Radius of the Earth (in meters)
   const double flat_factor = 1.0/298.257223563;    // Flattening factor WGS84 Model
   double cosLat = cos(lat_r);
   double sinLat = sin(lat_r);
   double FF     = (1.0-flat_factor)*(1.0-flat_factor);
   double C      = 1./sqrt(cosLat*cosLat + FF * sinLat*sinLat);
   double S      = C * FF;

   *x_km = (radius * C + alt) * cosLat * cos(lon_r) / 1000.;
   *y_km = (radius * C + alt) * cosLat * sin(lon_r) / 1000.;
   *z_km = (radius * S + alt) * sinLat / 1000.;
}


////////////////////////////////////////////////////////////////////////

/*
https://stackoverflow.com/questions/10473852/convert-latitude-and-longitude-to-point-in-3d-space

def llarToWorld(lat, lon, alt, rad):
    # see: http://www.mathworks.de/help/toolbox/aeroblks/llatoecefposition.html
    f  = 0                              # flattening
    ls = atan((1 - f)**2 * tan(lat))    # lambda

    x = rad * cos(ls) * cos(lon) + alt * cos(lat) * cos(lon)
    y = rad * cos(ls) * sin(lon) + alt * cos(lat) * sin(lon)
    z = rad * sin(ls) + alt * sin(lat)

    return c4d.Vector(x, y, z)

def LLHtoECEF(lat, lon, alt):
    # see http://www.mathworks.de/help/toolbox/aeroblks/llatoecefposition.html

    rad = np.float64(6378137.0)        # Radius of the Earth (in meters)
    f = np.float64(1.0/298.257223563)  # Flattening factor WGS84 Model
    cosLat = np.cos(lat)
    sinLat = np.sin(lat)
    FF     = (1.0-f)**2
    C      = 1/np.sqrt(cosLat**2 + FF * sinLat**2)
    S      = C * FF

    x = (rad * C + alt)*cosLat * np.cos(lon)
    y = (rad * C + alt)*cosLat * np.sin(lon)
    z = (rad * S + alt)*sinLat

    return (x, y, z)


Comparison output: finding ECEF for Los Angeles, CA (34.0522, -118.40806, 0 elevation)
My code:
X = -2516715.36114 meters or -2516.715 km
Y = -4653003.08089 meters or -4653.003 km
Z = 3551245.35929 meters or 3551.245 km

Your Code:
X = -2514072.72181 meters or -2514.072 km
Y = -4648117.26458 meters or -4648.117 km
Z = 3571424.90261 meters or 3571.424 km

Although in your earth rotation environment your function will produce right geographic region for display, it will NOT give the right ECEF equivalent coordinates. As you can see some of the parameters vary by as much as 20 KM which is rather a large error.

Flattening factor, f depends on the model you assume for your conversion. Typical, model is WGS 84; however, there are other models.

Personally, I like to use this link to Naval Postgraduate School for sanity checks on my conversions.

https://www.oc.nps.edu/oc2902w/coord/llhxyz.html

https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_geodetic_to_ECEF_coordinates

*/
