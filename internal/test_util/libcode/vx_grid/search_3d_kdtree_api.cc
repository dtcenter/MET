// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

// ./search_3d_kdtree_api /d1/personal/dadriaan/projects/NRL/PyIRI/pyiri_f4_2020.nc -23.114 334.77 100.
// ./search_3d_kdtree_api /d1/personal/dadriaan/projects/NRL/PyIRI/pyiri_f4_2020.nc 34.0522 -118.40806 0. glat glon zalt
//
// lat=-23.114, lon=334.77, alt=100 (meters)
//                     ==> X: 5309.352 km, Y: -2501.788 km, Z: -2488.375 km
// lat=34.0522, lon=-118.40806, alt=0. (meters)
//                     ==> X: -2516.715 km, Y: -4653.00 km, Z: 3551.245 km
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

#include "atlas/util/KDTree.h"

#include "vx_util.h"
#include "vx_grid.h"
#include "nav.h"
#include "nc_utils_core.h"
#include "nc_utils.h"

using namespace std;
using namespace netCDF;

using IndexKDTree = atlas::util::IndexKDTree;

////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////

extern void llh_to_ecef(double lat, double lon, double alt_m, double *x_km, double *y_km, double *z_km);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{

   string lat_name = "glat";
   string lon_name = "glon";
   string alt_name = "zalt";

   if(argc < 2) cerr << argv[0] << ": Must specify input NetCDF name to process!\n";
   if(argc < 3) cerr << argv[0] << ": Must specify latitude to search\n";
   if(argc < 4) cerr << argv[0] << ": Must specify longitude to search\n";
   if(argc < 5) {
      cerr << argv[0] << ": Must specify altitude to search\n";

      cerr << "\n  Usage: " << argv[0]
           << " netcdf_name lat_deg lon_deg alt_meters <lat_var_name> <lon_var_name> <alt_var_name>\n";
      cerr << "             def_lat_var_name=" << lat_name
           << ", def_lon_var_name=" << lon_name
           << ", def_alt_var_name=" << alt_name << "\n";

      if(argc < 2) {
         UnstructuredData ugrid_data1;
         ugrid_data1.test_llh_to_ecef();
      }
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

   UnstructuredData ugrid_data;
   ugrid_data.set_points(nlat, lon_values.data(), lat_values.data(), alt_values.data());
   ugrid_data.test_kdtree();

   double x,y,z;
   double distance_km;
   int closest_n = 5;

   cout << "The requested point: "
        << lat << ", " << lon << ", " << alt
        << "] to [" << x << ", " << y << ", " << z << "])\n";
   IndexKDTree::ValueList neighbor = ugrid_data.closest_points(lat, lon, closest_n, alt);
   for (size_t i=0; i < neighbor.size(); i++) {
      size_t index(neighbor[i].payload());
      int index2 = (int)neighbor[i].payload();
      distance_km = neighbor[i].distance() / 1000.;
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
