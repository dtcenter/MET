// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include <netcdf>

#include "get_met_grid.h"

#include "nc_utils.h"
#include "vx_log.h"

using namespace std;
using namespace netCDF;


///////////////////////////////////////////////////////////////////////////////

static void read_netcdf_grid_v3 (NcFile *, Grid &);
static void read_netcdf_grid_v2 (NcFile *, Grid &);

static LatLonData        get_latlon_data           (NcFile *);
static RotatedLatLonData get_rot_latlon_data       (NcFile *);
static LambertData       get_lambert_data          (NcFile *);
static LaeaData          get_laea_data             (NcFile *);
static StereographicData get_stereographic_data    (NcFile *);
static MercatorData      get_mercator_data         (NcFile *);
static GaussianData      get_gaussian_data         (NcFile *);
static SemiLatLonData    get_semilatlon_data       (NcFile *);
static void              get_semilatlon_var        (NcFile *, const char *, NumArray &);

static LatLonData        get_latlon_data_v2        (NcFile *);
static LambertData       get_lambert_data_v2       (NcFile *);
static StereographicData get_stereographic_data_v2 (NcFile *);
static MercatorData      get_mercator_data_v2      (NcFile *);

///////////////////////////////////////////////////////////////////////////////

void read_netcdf_grid(NcFile * f_in, Grid & gr) {

   // Parse the projection information based on the version
   if(has_att(f_in, string("MET_version"))) {
      read_netcdf_grid_v3(f_in, gr);
   }
   else {
      mlog << Warning << "\nread_netcdf_grid() -> "
           << "Applying METv2.0 grid parsing logic since the \"MET_version\" "
           << "global attribute is not present.\n\n";
      read_netcdf_grid_v2(f_in, gr);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void read_netcdf_grid_v3(NcFile * f_in, Grid & gr) {

   //
   // Parse the grid specification out of the global attributes
   //

   NcGroupAtt proj_att = GET_NC_ATT_OBJ_BY_P(f_in, "Projection");

   //
   // Parse out the grid specification depending on the projection type
   //

   if (!IS_INVALID_NC(proj_att)) {
      ConcatString proj_att_name;
      get_global_att(&proj_att, proj_att_name);

      if ( strcasecmp(proj_att_name.c_str(),
                      latlon_proj_type) == 0 )  {
         gr.set(get_latlon_data(f_in));
      } else if ( strcasecmp(proj_att_name.c_str(),
                             rotated_latlon_proj_type) == 0 )  {
         gr.set(get_rot_latlon_data(f_in));
      } else if ( strcasecmp(proj_att_name.c_str(),
                             mercator_proj_type) == 0 )  {
         gr.set(get_mercator_data(f_in));
      } else if ( strcasecmp(proj_att_name.c_str(),
                             lambert_proj_type) == 0 )  {
         gr.set(get_lambert_data(f_in));
      } else if ( strcasecmp(proj_att_name.c_str(),
                             laea_proj_type) == 0 )  {
         gr.set(get_laea_data(f_in));
      } else if ( strcasecmp(proj_att_name.c_str(),
                             stereographic_proj_type) == 0 )  {
         gr.set(get_stereographic_data(f_in));
      } else if ( strcasecmp(proj_att_name.c_str(),
                             gaussian_proj_type) == 0 )  {
         gr.set(get_gaussian_data(f_in));
      } else if ( strcasecmp(proj_att_name.c_str(),
                             semilatlon_proj_type) == 0 )  {
         gr.set(get_semilatlon_data(f_in));
      } else {   // Unsupported projection type

         mlog << Error << "\nread_netcdf_grid_v3() -> "
              << "Projection \"" << proj_att_name
              << "\" not a currently supported type ("
              << latlon_proj_type << ", "
              << rotated_latlon_proj_type << ", "
              << mercator_proj_type << ", "
              << lambert_proj_type << ", "
              << laea_proj_type << ", "
              << stereographic_proj_type << ", "
              << gaussian_proj_type << ", "
              << semilatlon_proj_type << ").\n\n";

         exit(1);

      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void read_netcdf_grid_v2(NcFile * f_in, Grid & gr)
{

   //
   // Parse the grid specification out of the global attributes
   //

   NcGroupAtt proj_att = GET_NC_ATT_OBJ_BY_P(f_in, "Projection");

   //
   // Parse out the grid specification depending on the projection type
   //

   if (!IS_INVALID_NC(proj_att)) {
      ConcatString proj_att_name;
      get_global_att(&proj_att, proj_att_name);
      if ( strcasecmp(proj_att_name.c_str(),
                      latlon_proj_type) == 0 )  {
         gr.set(get_latlon_data_v2(f_in));
      } else if ( strcasecmp(proj_att_name.c_str(),
                             mercator_proj_type) == 0 )  {
         gr.set(get_mercator_data_v2(f_in));
      } else if ( strcasecmp(proj_att_name.c_str(),
                             lambert_proj_type) == 0 )  {
         gr.set(get_lambert_data_v2(f_in));
      } else if ( strcasecmp(proj_att_name.c_str(),
                             stereographic_proj_type) == 0 )  {
         gr.set(get_stereographic_data_v2(f_in));
      } else {   // Unsupported projection type

         mlog << Error << "\nread_netcdf_grid_v2() -> "
              << "Projection \"" << proj_att_name
              << "\" not a currently supported type ("
              << latlon_proj_type << ", "
              << mercator_proj_type << ", "
              << lambert_proj_type << ", "
              << stereographic_proj_type << ").\n\n";

         exit(1);
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

int has_variable(NcFile *f_in, const char *var_name) {
   int found;

   //
   // Initialize to not found
   //
   found = (has_var(f_in, var_name) ? 1 : 0);

   return(found);
}

///////////////////////////////////////////////////////////////////////////////

LatLonData get_latlon_data(NcFile * ncfile) {

   LatLonData data;

   // Store the grid name
   data.name = latlon_proj_type;

   // Latitude of the bottom left corner
   get_global_att(ncfile, string("lat_ll"), data.lat_ll);

   // Longitude of the bottom left corner
   get_global_att(ncfile, string("lon_ll"), data.lon_ll);
   data.lon_ll *= -1.0;

   // Latitude increment
   get_global_att(ncfile, string("delta_lat"), data.delta_lat);

   // Longitude increment
   get_global_att(ncfile, string("delta_lon"), data.delta_lon);

   // Number of points in the Latitude (y) direction
   get_global_att(ncfile, string("Nlat"), data.Nlat);

   // Number of points in the Longitudinal (x) direction
   get_global_att(ncfile, string("Nlon"), data.Nlon);

   data.dump();

   return(data);
}

///////////////////////////////////////////////////////////////////////////////

RotatedLatLonData get_rot_latlon_data(NcFile * ncfile) {

   RotatedLatLonData data;

   // Store the grid name
   data.name = latlon_proj_type;

   // Latitude of the bottom left corner
   get_global_att(ncfile, string("rot_lat_ll"), data.rot_lat_ll);

   // Longitude of the bottom left corner
   get_global_att(ncfile, string("rot_lon_ll"), data.rot_lon_ll);
   data.rot_lon_ll *= -1.0;

   // Latitude increment
   get_global_att(ncfile, string("delta_rot_lat"), data.delta_rot_lat);

   // Longitude increment
   get_global_att(ncfile, string("delta_rot_lon"), data.delta_rot_lon);

   // Number of points in the Latitude (y) direction
   get_global_att(ncfile, string("Nlat"), data.Nlat);

   // Number of points in the Longitudinal (x) direction
   get_global_att(ncfile, string("Nlon"), data.Nlon);

   //  true lat/lon of south pole
   get_global_att(ncfile, string("true_lat_south_pole"), data.true_lat_south_pole);
   get_global_att(ncfile, string("true_lon_south_pole"), data.true_lon_south_pole);
   if ( !west_longitude_positive )  data.true_lon_south_pole *= -1.0;

   //  auxiliary rotation
   get_global_att(ncfile, string("aux_rotation"), data.aux_rotation);

   data.dump();

   return(data);
}

///////////////////////////////////////////////////////////////////////////////

LambertData get_lambert_data(NcFile * ncfile) {

   LambertData data;
   ConcatString att_value;

   // Store the grid name
   data.name = lambert_proj_type;

   // Hemisphere
   get_global_att(ncfile, string("hemisphere"), att_value);
   data.hemisphere = att_value.char_at(0);

   // First scale latitude
   get_global_att(ncfile, string("scale_lat_1"), data.scale_lat_1);

   // Second scale latitude
   get_global_att(ncfile, string("scale_lat_2"), data.scale_lat_2);

   // Latitude pin
   get_global_att(ncfile, string("lat_pin"), data.lat_pin);

   // Longitude pin
   get_global_att(ncfile, string("lon_pin"), data.lon_pin);
   data.lon_pin *= -1.0;

   // X pin
   get_global_att(ncfile, string("x_pin"), data.x_pin);

   // Y pin
   get_global_att(ncfile, string("y_pin"), data.y_pin);

   // Orientation longitude
   get_global_att(ncfile, string("lon_orient"), data.lon_orient);
   data.lon_orient *= -1.0;

   // Grid spacing in km
   get_global_att(ncfile, string("d_km"), data.d_km);

   // Radius of the earth
   get_global_att(ncfile, string("r_km"), data.r_km);

   // Number of points in the x-direction
   get_global_att(ncfile, string("nx"), data.nx);

   // Number of points in the y-direction
   get_global_att(ncfile, string("ny"), data.ny);

   // Rotation angle
   data.so2_angle = 0.0;

   data.dump();

   return(data);
}

///////////////////////////////////////////////////////////////////////////////

LaeaData get_laea_data(NcFile * ncfile) {

   const char * method_name = "get_laea_data() -> ";

   LaeaData data;
   ConcatString att_value;

   // Store the grid name
   data.name = laea_proj_type;

   // Spheroid name
   get_global_att(ncfile, string("spheroid_name"), att_value);
   m_strncpy(data.spheroid_name, att_value.c_str(), att_value.length(), method_name);

   // Grid spacing in km
   get_global_att(ncfile, string("radius_km"), data.radius_km);

   // Equitorial radius in km
   get_global_att(ncfile, string("equatorial_radius_km"), data.equatorial_radius_km);

   // Polar radius in km
   get_global_att(ncfile, string("polar_radius_km"), data.polar_radius_km);

   // First latitude
   get_global_att(ncfile, string("lat_first"), data.lat_first);

   // First longitude
   get_global_att(ncfile, string("lon_first"), data.lon_first);
   data.lon_first *= -1.0;

   // Standard latitude
   get_global_att(ncfile, string("standard_lat"), data.standard_lat);

   // Central longitude
   get_global_att(ncfile, string("central_lon"), data.central_lon);
   data.central_lon *= -1.0;

   // Spacing in the x-dimension
   get_global_att(ncfile, string("dx_km"), data.dx_km);

   // Spacing in the y-direction
   get_global_att(ncfile, string("dy_km"), data.dy_km);

   // Number of points in the x-direction
   get_global_att(ncfile, string("nx"), data.nx);

   // Number of points in the y-direction
   get_global_att(ncfile, string("ny"), data.ny);

   // Is sphere
   get_global_att(ncfile, string("is_sphere"), att_value);
   data.is_sphere = string_to_bool(att_value.c_str());

   data.dump();

   return(data);
}

///////////////////////////////////////////////////////////////////////////////

StereographicData get_stereographic_data(NcFile * ncfile) {

   StereographicData data;
   ConcatString att_value;

   // Store the grid name
   data.name = stereographic_proj_type;

   // Hemisphere
   get_global_att(ncfile, (string)"hemisphere", att_value);
   data.hemisphere = att_value.char_at(0);

   // Scale latitude
   get_global_att(ncfile, (string)"scale_lat", data.scale_lat);

   // Latitude pin
   get_global_att(ncfile, (string)"lat_pin", data.lat_pin);

   // Longitude pin
   get_global_att(ncfile, string("lon_pin"), data.lon_pin);
   data.lon_pin *= -1.0;

   // X pin
   get_global_att(ncfile, string("x_pin"), data.x_pin);

   // Y pin
   get_global_att(ncfile, string("y_pin"), data.y_pin);

   // Orientation longitude
   get_global_att(ncfile, string("lon_orient"), data.lon_orient);
   data.lon_orient *= -1.0;

   // Grid spacing in km
   get_global_att(ncfile, string("d_km"), data.d_km);

   // Radius of the earth
   get_global_att(ncfile, string("r_km"), data.r_km);

   // Number of points in the x-direction
   get_global_att(ncfile, string("nx"), data.nx);

   // Number of points in the y-direction
   get_global_att(ncfile, string("ny"), data.ny);

   data.eccentricity = 0.;
   data.false_east = 0.;
   data.false_north = 0.;
   data.scale_factor = 1.0;
   data.dy_km = data.d_km;

   data.dump();

   return(data);
}

///////////////////////////////////////////////////////////////////////////////

MercatorData get_mercator_data(NcFile * ncfile) {

   MercatorData data;

   // Store the grid name
   data.name = mercator_proj_type;

   // Latitude of the bottom left corner
   get_global_att(ncfile, string("lat_ll"), data.lat_ll);

   // Longitude of the bottom left corner
   get_global_att(ncfile, string("lon_ll"), data.lon_ll);
   data.lon_ll *= -1.0;

   // Latitude of the bottom left corner
   get_global_att(ncfile, string("lat_ur"), data.lat_ur);

   // Longitude of the bottom left corner
   get_global_att(ncfile, string("lon_ur"), data.lon_ur);
   data.lon_ur *= -1.0;

   // Number of points in the Latitudinal (y) direction
   get_global_att(ncfile, string("ny"), data.ny);

   // Number of points in the Longitudinal (x) direction
   get_global_att(ncfile, string("nx"), data.nx);

   data.dump();

   return(data);
}

///////////////////////////////////////////////////////////////////////////////

LatLonData get_latlon_data_v2(NcFile * ncfile) {

   LatLonData data;

   // Store the grid name
   data.name = latlon_proj_type;

   // Latitude of the bottom left corner
   get_global_att(ncfile, string("lat_ll_deg"), data.lat_ll);

   // Longitude of the bottom left corner
   get_global_att(ncfile, string("lon_ll_deg"), data.lon_ll);
   data.lon_ll *= -1.0;

   // Latitude increment
   get_global_att(ncfile, string("delta_lat_deg"), data.delta_lat);

   // Longitude increment
   get_global_att(ncfile, string("delta_lon_deg"), data.delta_lon);

   // Number of points in the Latitude (y) direction
   get_global_att(ncfile, string("Nlat"), data.Nlat);

   // Number of points in the Longitudinal (x) direction
   get_global_att(ncfile, string("Nlon"), data.Nlon);

   data.dump();

   return(data);
}

///////////////////////////////////////////////////////////////////////////////

LambertData get_lambert_data_v2(NcFile * ncfile) {

   LambertData data;

   // Store the grid name
   data.name = lambert_proj_type;

   // First scale latitude
   get_global_att(ncfile, string("p1_deg"), data.scale_lat_1);

   // Second scale latitude
   get_global_att(ncfile, string("p2_deg"), data.scale_lat_2);

   // Latitude pin
   get_global_att(ncfile, string("p0_deg"), data.lat_pin);

   // Longitude pin
   get_global_att(ncfile, string("l0_deg"), data.lon_pin);
   data.lon_pin *= -1.0;

   // X pin
   data.x_pin = 0.0;

   // Y pin
   data.y_pin = 0.0;

   // Orientation longitude
   get_global_att(ncfile, string("lcen_deg"), data.lon_orient);
   data.lon_orient *= -1.0;

   // Grid spacing in km
   get_global_att(ncfile, string("d_km"), data.d_km);

   // Radius of the earth
   get_global_att(ncfile, string("r_km"), data.r_km);

   // Number of points in the x-direction
   get_global_att(ncfile, string("nx"), data.nx);

   // Number of points in the y-direction
   get_global_att(ncfile, string("ny"), data.ny);

   // Rotation angle
   data.so2_angle = 0.0;

   data.dump();

   return(data);
}

///////////////////////////////////////////////////////////////////////////////

StereographicData get_stereographic_data_v2(NcFile * ncfile) {

   StereographicData data;

   // Store the grid name
   data.name = stereographic_proj_type;

   // Hemisphere, assume northern
   data.hemisphere = 'N';

   // Scale latitude
   get_global_att(ncfile, string("p1_deg"), data.scale_lat);

   // Latitude pin
   get_global_att(ncfile, string("p0_deg"), data.lat_pin);

   // Longitude pin
   get_global_att(ncfile, string("l0_deg"), data.lon_pin);
   data.lon_pin *= -1.0;

   // X pin
   data.x_pin = 0.0;

   // Y pin
   data.y_pin = 0.0;

   // Orientation longitude
   get_global_att(ncfile, string("lcen_deg"), data.lon_orient);
   data.lon_orient *= -1.0;

   // Grid spacing in km
   get_global_att(ncfile, string("d_km"), data.d_km);

   // Radius of the earth
   get_global_att(ncfile, string("r_km"), data.r_km);

   // Number of points in the x-direction
   get_global_att(ncfile, string("nx"), data.nx);

   // Number of points in the y-direction
   get_global_att(ncfile, string("ny"), data.ny);

   data.eccentricity = 0.;
   data.false_east = 0.;
   data.false_north = 0.;
   data.scale_factor = 1.0;
   data.dy_km = data.d_km;

   data.dump();

   return(data);
}

///////////////////////////////////////////////////////////////////////////////

MercatorData get_mercator_data_v2(NcFile * ncfile) {

   MercatorData data;

   // Store the grid name
   data.name = mercator_proj_type;

   // Latitude of the bottom left corner
   get_global_att(ncfile, string("lat_ll_deg"), data.lat_ll);

   // Longitude of the bottom left corner
   get_global_att(ncfile, string("lon_ll_deg"), data.lon_ll);
   data.lon_ll *= -1.0;

   // Latitude of the bottom left corner
   get_global_att(ncfile, string("lat_ur_deg"), data.lat_ur);

   // Longitude of the bottom left corner
   get_global_att(ncfile, string("lon_ur_deg"), data.lon_ur);
   data.lon_ur *= -1.0;

   // Number of points in the Latitudinal (y) direction
   get_global_att(ncfile, string("Nlat"), data.ny);

   // Number of points in the Longitudinal (x) direction
   get_global_att(ncfile, string("Nlon"), data.nx);

   data.dump();

   return(data);

}

///////////////////////////////////////////////////////////////////////////////

GaussianData get_gaussian_data (NcFile * ncfile) {

   GaussianData data;

   // Store the grid name
   data.name = gaussian_proj_type;

   // Longitude for x = 0
   get_global_att(ncfile, string("lon_zero"), data.lon_zero);

   // nx
   get_global_att(ncfile, string("nx"), data.nx);

   // ny
   get_global_att(ncfile, string("ny"), data.ny);

   data.dump();

   return(data);
}

///////////////////////////////////////////////////////////////////////////////

SemiLatLonData get_semilatlon_data (NcFile * ncfile) {

   SemiLatLonData data;

   // Store the grid name
   data.name = semilatlon_proj_type;
   get_semilatlon_var(ncfile, "lat",   data.lats);
   get_semilatlon_var(ncfile, "lon",   data.lons);
   get_semilatlon_var(ncfile, "level", data.levels);
   get_semilatlon_var(ncfile, "time",  data.times);

   data.dump();

   return(data);
}

///////////////////////////////////////////////////////////////////////////////

void get_semilatlon_var(NcFile *ncfile, const char * var_name, NumArray &out_na)  {

   NcVar nc_var = get_var(ncfile, var_name);

   out_na.erase();

   // Requested variable may or may not be present in the file
   if ( IS_INVALID_NC(nc_var) )  return;

   // Store the requested data in the specified NumArray object
   long count = get_data_size(&nc_var);
   double * data_values = new double[ count ];
   get_nc_data(&nc_var, data_values);
   for(int i=0; i<count; i++)  out_na.add(data_values[i]);
   if(data_values) { delete [] data_values; data_values = (double *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////////////
