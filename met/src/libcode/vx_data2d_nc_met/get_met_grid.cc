// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "get_met_grid.h"

#include "nc_utils.h"
#include "vx_log.h"


///////////////////////////////////////////////////////////////////////////////


static void read_netcdf_grid_v3       (NcFile *, Grid &);
static void read_netcdf_grid_v2       (NcFile *, Grid &);

static void get_latlon_data_v3        (NcFile *, LatLonData &);
static void get_rot_latlon_data_v3    (NcFile *, RotatedLatLonData &);
static void get_lambert_data_v3       (NcFile *, LambertData &);
static void get_stereographic_data_v3 (NcFile *, StereographicData &);
static void get_mercator_data_v3      (NcFile *, MercatorData &);

static void get_latlon_data_v2        (NcFile *, LatLonData &);
static void get_lambert_data_v2       (NcFile *, LambertData &);
static void get_stereographic_data_v2 (NcFile *, StereographicData &);
static void get_mercator_data_v2      (NcFile *, MercatorData &);

static void get_gaussian_data         (NcFile *, GaussianData &);


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
      mlog << Warning << "\nread_netcdf_grid() -> "
           << "Applying METv2.0 grid parsing logic since the \"MET_version\" "
           << "global attribute is not present.\n\n";
      read_netcdf_grid_v2(f_in, gr);
   }

   return;
}


///////////////////////////////////////////////////////////////////////////////


void read_netcdf_grid_v3(NcFile * f_in, Grid & gr)
{

   NcGroupAtt proj_att;

   // Structures to store projection info
   LambertData        lc_data;
   StereographicData  st_data;
   LatLonData         ll_data;
   RotatedLatLonData rll_data;
   MercatorData       mc_data;
   GaussianData        g_data;

   //
   // Parse the grid specification out of the global attributes
   //

   proj_att = GET_NC_ATT_OBJ_BY_P(f_in, "Projection");

   //
   // Parse out the grid specification depending on the projection type
   // The following Projection types are supported:
   //    - Lat/Lon
   //    - Mercator
   //    - Lambert Conformal
   //    - Polar Stereographic
   //    - Gaussian
   //

   if (!IS_INVALID_NC(proj_att)) {
      ConcatString proj_att_name;
      get_global_att(&proj_att, proj_att_name);

      if ( strcasecmp(proj_att_name.c_str(), latlon_proj_type) == 0 )  {

         get_latlon_data_v3(f_in, ll_data);
         gr.set(ll_data);

      } else if ( strcasecmp(proj_att_name.c_str(), rotated_latlon_proj_type) == 0 )  {

         get_rot_latlon_data_v3(f_in, rll_data);
         gr.set(rll_data);

      } else if ( strcasecmp(proj_att_name.c_str(), mercator_proj_type) == 0 )  {

         get_mercator_data_v3(f_in, mc_data);
         gr.set(mc_data);

      } else if ( strcasecmp(proj_att_name.c_str(), lambert_proj_type) == 0 )  {

         get_lambert_data_v3(f_in, lc_data);
         gr.set(lc_data);

      } else if ( strcasecmp(proj_att_name.c_str(), stereographic_proj_type) == 0 )  {

         get_stereographic_data_v3(f_in, st_data);
         gr.set(st_data);

      } else if ( strcasecmp(proj_att_name.c_str(), gaussian_proj_type) == 0 )  {

         get_gaussian_data(f_in, g_data);
         gr.set(g_data);

      } else {   // Unsupported projection type

         mlog << Error << "\nread_netcdf_grid_v3() -> "
              << "Projection \"" << proj_att_name
              << "\" not a currently supported type (\""
              << latlon_proj_type << "\", \"" << mercator_proj_type << "\", \""
              << lambert_proj_type << "\", \"" << stereographic_proj_type
              << "\").\n\n";

         exit(1);

      }
   }

   //
   //  done
   //

return;

}

///////////////////////////////////////////////////////////////////////////////


void read_netcdf_grid_v2(NcFile * f_in, Grid & gr)
{

   NcGroupAtt  proj_att;

   // Structures to store projection info
   LambertData       lc_data;
   StereographicData st_data;
   LatLonData        ll_data;
   MercatorData      mc_data;

   //
   // Parse the grid specification out of the global attributes
   //

   proj_att = GET_NC_ATT_OBJ_BY_P(f_in, "Projection");

   //
   // Parse out the grid specification depending on the projection type
   // The following 4 Projection types are supported:
   //    - Lat/Lon
   //    - Mercator
   //    - Lambert Conformal
   //    - Polar Stereographic
   //

   if (!IS_INVALID_NC(proj_att)) {
      ConcatString proj_att_name;
      get_global_att(&proj_att, proj_att_name);
      if ( strcasecmp(proj_att_name.c_str(), latlon_proj_type) == 0 )  {
         get_latlon_data_v2(f_in, ll_data);
         gr.set(ll_data);
      } else if ( strcasecmp(proj_att_name.c_str(), mercator_proj_type) == 0 )  {
         get_mercator_data_v2(f_in, mc_data);
         gr.set(mc_data);
      } else if ( strcasecmp(proj_att_name.c_str(), lambert_proj_type) == 0 )  {
         get_lambert_data_v2(f_in, lc_data);
         gr.set(lc_data);
      } else if ( strcasecmp(proj_att_name.c_str(), stereographic_proj_type) == 0 )  {
         get_stereographic_data_v2(f_in, st_data);
         gr.set(st_data);
      } else {   // Unsupported projection type

         mlog << Error << "\nread_netcdf_grid_v2() -> "
              << "Projection \"" << proj_att_name
              << "\" not a currently supported type (\""
              << latlon_proj_type << "\", \"" << mercator_proj_type << "\", \""
              << lambert_proj_type << "\", \"" << stereographic_proj_type
              << "\").\n\n";

         exit(1);
      }
   }

   //
   //  done
   //

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


void get_latlon_data_v3(NcFile * ncfile, LatLonData & data)
{

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

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_rot_latlon_data_v3(NcFile * ncfile, RotatedLatLonData & data)
{

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

   //  auxilliary rotation

 get_global_att(ncfile, string("aux_rotation"), data.aux_rotation);



data.dump();

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_lambert_data_v3(NcFile * ncfile, LambertData & data)
{

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

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_stereographic_data_v3(NcFile * ncfile, StereographicData & data)
{

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

data.dump();

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_mercator_data_v3(NcFile * ncfile, MercatorData & data)

{

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

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_latlon_data_v2(NcFile * ncfile, LatLonData & data)

{

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

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_lambert_data_v2(NcFile * ncfile, LambertData & data)

{

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

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_stereographic_data_v2(NcFile * ncfile, StereographicData & data)

{

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

data.dump();

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_mercator_data_v2(NcFile * ncfile, MercatorData & data)

{

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

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_gaussian_data (NcFile * ncfile, GaussianData & data)

{


data.name = gaussian_proj_type;

   //
   //  Longitude for x = 0
   //

 get_global_att(ncfile, string("lon_zero"), data.lon_zero);

   //
   //  nx
   //

 get_global_att(ncfile, string("nx"), data.nx);

   //
   //  ny
   //

 get_global_att(ncfile, string("ny"), data.ny);

data.dump();

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////
