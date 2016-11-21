// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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

static const int grid_debug_level = 4;

///////////////////////////////////////////////////////////////////////////////


//static void get_global_att(NcFile *, NcGroupAtt * & att, const char * name);
//static bool has_att(NcFile *, const char * name);

static void read_netcdf_grid_v3       (NcFile *, Grid &);
static void read_netcdf_grid_v2       (NcFile *, Grid &);

static void get_latlon_data_v3        (NcFile *, LatLonData &);
static void get_lambert_data_v3       (NcFile *, LambertData &);
static void get_stereographic_data_v3 (NcFile *, StereographicData &);
static void get_mercator_data_v3      (NcFile *, MercatorData &);

static void get_latlon_data_v2        (NcFile *, LatLonData &);
static void get_lambert_data_v2       (NcFile *, LambertData &);
static void get_stereographic_data_v2 (NcFile *, StereographicData &);
static void get_mercator_data_v2      (NcFile *, MercatorData &);


///////////////////////////////////////////////////////////////////////////////


void read_netcdf_grid(NcFile * f_in, Grid & gr) {

   // Parse the projection information based on the version
   if(has_att(f_in, "MET_version")) {
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

   //NcGroupAtt * proj_att = (NcGroupAtt *) 0;
   NcGroupAtt proj_att;

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
      if ( strcmp(proj_att_name, latlon_proj_type) == 0 )  {
         get_latlon_data_v3(f_in, ll_data);
         gr.set(ll_data);
      } else if ( strcmp(proj_att_name, mercator_proj_type) == 0 )  {
         get_mercator_data_v3(f_in, mc_data);
         gr.set(mc_data);
      } else if ( strcmp(proj_att_name, lambert_proj_type) == 0 )  {
         get_lambert_data_v3(f_in, lc_data);
         gr.set(lc_data);
      } else if ( strcmp(proj_att_name, stereographic_proj_type) == 0 )  {
         get_stereographic_data_v3(f_in, st_data);
         gr.set(st_data);
      } else {   // Unsupported projection type
      
         mlog << Error << "\nread_netcdf_grid_v3() -> "
              << "Projection type " << proj_att_name
              << " not currently supported.\n\n";
      
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

   //NcGroupAtt * proj_att = (NcGroupAtt *) 0;
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
//   if (get_global_att(f_in, proj_att, "Projection")) {
   
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
      if ( strcmp(proj_att_name, latlon_proj_type) == 0 )  {
         get_latlon_data_v2(f_in, ll_data);
         gr.set(ll_data);
      } else if ( strcmp(proj_att_name, mercator_proj_type) == 0 )  {
         get_mercator_data_v2(f_in, mc_data);
         gr.set(mc_data);
      } else if ( strcmp(proj_att_name, lambert_proj_type) == 0 )  {
         get_lambert_data_v2(f_in, lc_data);
         gr.set(lc_data);
      } else if ( strcmp(proj_att_name, stereographic_proj_type) == 0 )  {
         get_stereographic_data_v2(f_in, st_data);
         gr.set(st_data);
      } else {   // Unsupported projection type
      
         mlog << Error << "\nread_netcdf_grid_v2() -> "
              << "Projection type " << proj_att_name
              << " not currently supported.\n\n";
      
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
   int n_var, i, found;
   //NcVar *nc_var = (NcVar *) 0;

   //
   // Initialize to not found
   //
   found = (has_var(f_in, var_name) ? 1 : 0);

   //
   // Retrieve the number of variables
   //
   //nc_var = get_var(f_in, var_name);

   //
   // Check if this is the variable name requested
   //
   //if(!IS_INVALID_NC(nc_var)) found = 1;

   return(found);
}

///////////////////////////////////////////////////////////////////////////////
// Moved to nc_utils.cc
//void get_global_att(NcFile * ncfile, NcGroupAtt * & att, const char * name)
//{
//if ( !has_att(ncfile, name) ) {
//   mlog << Error << "\nget_global_att() -> "
//        << "global NetCDF attribute \"" << name << "\" not found.\n\n";
//   exit ( 1 );
//}
//att = ncfile->get_global_att(name);
//return;
//}


///////////////////////////////////////////////////////////////////////////////
// Moved to nc_utils.cc
//bool has_att(NcFile * ncfile, const char * att_name)
//{
//int i, n;
//bool status = false;
//NcGroupAtt *att = (NcGroupAtt *) 0;
//
//n = ncfile->num_atts();
//for ( i=0; i<n; i++ )  {
//   att = ncfile->get_global_att(i);
//   if ( !att )  {
//      mlog << Error << "\nhas_att() -> "
//           << "can't read attribute number " << i << ".\n\n";
//      exit ( 1 );
//   }
//   if ( strcmp(att->getName(), att_name) == 0 )  {
//      status = true;
//      break;
//   }
//}
//
//return(status);
//
//}


///////////////////////////////////////////////////////////////////////////////


void get_latlon_data_v3(NcFile * ncfile, LatLonData & data)
{

NcGroupAtt * att = (NcGroupAtt *) 0;
double att_double_value;
   // Store the grid name
data.name = latlon_proj_type;

   // Latitude of the bottom left corner
get_global_att(ncfile, "lat_ll", data.lat_ll);
//data.lat_ll = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lat_ll);

   // Longitude of the bottom left corner
get_global_att(ncfile, "lon_ll", data.lon_ll);
//data.lon_ll = atof(att->getValues(att->as_string(0));
data.lon_ll *= -1.0;

   // Latitude increment
get_global_att(ncfile, "delta_lat", data.delta_lat);
//data.delta_lat = atof(att->getValues(att->as_string(0));
//att->getValues(&data.delta_lat);

   // Longitude increment
get_global_att(ncfile, "delta_lon", data.delta_lon);
//data.delta_lon = atof(att->getValues(att->as_string(0));
//att->getValues(&data.delta_lon);

   // Number of points in the Latitude (y) direction
get_global_att(ncfile, "Nlat", data.Nlat);
//data.Nlat = atoi(att->getValues(att->as_string(0));
//att->getValues(&data.Nlat);

   // Number of points in the Longitudinal (x) direction
get_global_att(ncfile, "Nlon", data.Nlon);
//data.Nlon = atoi(att->getValues(att->as_string(0));
//att->getValues(&data.Nlon);

mlog << Debug(grid_debug_level)
     << "Latitude/Longitude Grid Data:\n"
     << "lat_ll = " << data.lat_ll << "\n"
     << "lon_ll = " << data.lon_ll << "\n"
     << "delta_lat = " << data.delta_lat << "\n"
     << "delta_lon = " << data.delta_lon << "\n"
     << "Nlat = " << data.Nlat << "\n"
     << "Nlon = " << data.Nlon << "\n";

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_lambert_data_v3(NcFile * ncfile, LambertData & data)
{

NcGroupAtt * att = (NcGroupAtt *) 0;

   // Store the grid name
data.name = lambert_proj_type;

   // First scale latitude
get_global_att(ncfile, "scale_lat_1", data.scale_lat_1);
//data.scale_lat_1 = atof(att->getValues(att->as_string(0));
//att->getValues(&data.scale_lat_1);

   // Second scale latitude
get_global_att(ncfile, "scale_lat_2", data.scale_lat_2);
//data.scale_lat_2 = atof(att->getValues(att->as_string(0));
//att->getValues(&data.scale_lat_2);

   // Latitude pin
get_global_att(ncfile, "lat_pin", data.lat_pin);
//data.lat_pin = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lat_pin);

   // Longitude pin
get_global_att(ncfile, "lon_pin", data.lon_pin);
//data.lon_pin = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lon_pin);
data.lon_pin *= -1.0;

   // X pin
get_global_att(ncfile, "x_pin", data.x_pin);
//data.x_pin = atof(att->getValues(att->as_string(0));
//att->getValues(&data.x_pin);

   // Y pin
get_global_att(ncfile, "y_pin", data.y_pin);
//data.y_pin = atof(att->getValues(att->as_string(0));
//att->getValues(&data.y_pin);

   // Orientation longitude
get_global_att(ncfile, "lon_orient", data.lon_orient);
//data.lon_orient = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lon_orient);
data.lon_orient *= -1.0;

   // Grid spacing in km
get_global_att(ncfile, "d_km", data.d_km);
//data.d_km = atof(att->getValues(att->as_string(0));
//att->getValues(&data.d_km);
cout << "   get_lambert_data_v3: data.d_km: " << data.d_km << "\n";
   // Radius of the earth
get_global_att(ncfile, "r_km", data.r_km);
//data.r_km = atof(att->getValues(att->as_string(0));
//att->getValues(&data.r_km);

   // Number of points in the x-direction
get_global_att(ncfile, "nx", data.nx);
//data.nx = atoi(att->getValues(att->as_string(0));
//att->getValues(&data.nx);

   // Number of points in the y-direction
get_global_att(ncfile, "ny", data.ny);
//data.ny = atoi(att->getValues(att->as_string(0));
//att->getValues(&data.ny);

mlog << Debug(grid_debug_level)
     << "Lambert Conformal Grid Data:\n"
     << "scale_lat_1 = " << data.scale_lat_1 << "\n"
     << "scale_lat_2 = " << data.scale_lat_2 << "\n"
     << "lat_pin = " << data.lat_pin << "\n"
     << "lon_pin = " << data.lon_pin << "\n"
     << "x_pin = " << data.x_pin << "\n"
     << "y_pin = " << data.y_pin << "\n"
     << "lon_orient = " << data.lon_orient << "\n"
     << "d_km = " << data.d_km << "\n"
     << "r_km = " << data.r_km << "\n"
     << "nx = " << data.nx << "\n"
     << "ny = " << data.ny << "\n";

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////

void get_stereographic_data_v3(NcFile * ncfile, StereographicData & data)
{

NcGroupAtt * att = (NcGroupAtt *) 0;
const char * c = (const char *) 0;
ConcatString att_value;

   // Store the grid name
data.name = stereographic_proj_type;

   // Hemisphere
get_global_att(ncfile, "hemisphere", att_value);
data.hemisphere = att_value.char_at(0);

   // Scale latitude
get_global_att(ncfile, "scale_lat", data.scale_lat);
//data.scale_lat = atof(att->getValues(att->as_string(0));
//att->getValues(&data.scale_lat);

   // Latitude pin
get_global_att(ncfile, "lat_pin", data.lat_pin);
//data.lat_pin = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lat_pin);

   // Longitude pin
get_global_att(ncfile, "lon_pin", data.lon_pin);
//data.lon_pin = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lon_pin);
data.lon_pin *= -1.0;

   // X pin
get_global_att(ncfile, "x_pin", data.x_pin);
//data.x_pin = atof(att->getValues(att->as_string(0));
//att->getValues(&data.x_pin);

   // Y pin
get_global_att(ncfile, "y_pin", data.y_pin);
//data.y_pin = atof(att->getValues(att->as_string(0));
//att->getValues(&data.y_pin);

   // Orientation longitude
get_global_att(ncfile, "lon_orient", data.lon_orient);
//data.lon_orient = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lon_orient);
data.lon_orient *= -1.0;

   // Grid spacing in km
get_global_att(ncfile, "d_km", data.d_km);
//data.d_km = atof(att->getValues(att->as_string(0));
//att->getValues(&data.d_km);

   // Radius of the earth
get_global_att(ncfile, "r_km", data.r_km);
//data.r_km = atof(att->getValues(att->as_string(0));
//att->getValues(&data.r_km);

   // Number of points in the x-direction
get_global_att(ncfile, "nx", data.nx);
//data.nx = atoi(att->getValues(att->as_string(0));
//att->getValues(&data.nx);

   // Number of points in the y-direction
get_global_att(ncfile, "ny", data.ny);
//data.ny = atoi(att->getValues(att->as_string(0));
//att->getValues(&data.ny);

mlog << Debug(grid_debug_level)
     << "Stereographic Grid Data:\n"
     << "hemisphere = " << data.hemisphere << "\n"
     << "scale_lat = " << data.scale_lat << "\n"
     << "lat_pin = " << data.lat_pin << "\n"
     << "lon_pin = " << data.lon_pin << "\n"
     << "x_pin = " << data.x_pin << "\n"
     << "y_pin = " << data.y_pin << "\n"
     << "lon_orient = " << data.lon_orient << "\n"
     << "d_km = " << data.d_km << "\n"
     << "r_km = " << data.r_km << "\n"
     << "nx = " << data.nx << "\n"
     << "ny = " << data.ny << "\n";

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_mercator_data_v3(NcFile * ncfile, MercatorData & data)

{

//NcGroupAtt * att = (NcGroupAtt *) 0;

   // Store the grid name
data.name = mercator_proj_type;

   // Latitude of the bottom left corner
get_global_att(ncfile, "lat_ll", data.lat_ll);
//data.lat_ll = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lat_ll);

   // Longitude of the bottom left corner
get_global_att(ncfile, "lon_ll", data.lon_ll);
//data.lon_ll = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lon_ll);
data.lon_ll *= -1.0;

   // Latitude of the bottom left corner
get_global_att(ncfile, "lat_ur", data.lat_ur);
//data.lat_ur = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lat_ur);

   // Longitude of the bottom left corner
get_global_att(ncfile, "lon_ur", data.lon_ur);
//data.lon_ur = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lon_ur);
data.lon_ur *= -1.0;

   // Number of points in the Latitudinal (y) direction
get_global_att(ncfile, "ny", data.ny);
//data.ny = atoi(att->getValues(att->as_string(0));
//att->getValues(&data.ny);

   // Number of points in the Longitudinal (x) direction
get_global_att(ncfile, "nx", data.nx);
//data.nx = atoi(att->getValues(att->as_string(0));
//att->getValues(&data.nx);

mlog << Debug(grid_debug_level)
     << "Mercator Data:\n"
     << "lat_ll = " << data.lat_ll << "\n"
     << "lon_ll = " << data.lon_ll << "\n"
     << "lat_ur = " << data.lat_ur << "\n"
     << "lon_ur = " << data.lon_ur << "\n"
     << "ny = " << data.ny << "\n"
     << "nx = " << data.nx << "\n";

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_latlon_data_v2(NcFile * ncfile, LatLonData & data)

{

//NcGroupAtt * att = (NcGroupAtt *) 0;

   // Store the grid name
data.name = latlon_proj_type;

   // Latitude of the bottom left corner
get_global_att(ncfile, "lat_ll_deg", data.lat_ll);
//data.lat_ll = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lat_ll);

   // Longitude of the bottom left corner
get_global_att(ncfile, "lon_ll_deg", data.lon_ll);
//data.lon_ll = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lon_ll);
data.lon_ll *= -1.0;

   // Latitude increment
get_global_att(ncfile, "delta_lat_deg", data.delta_lat);
//data.delta_lat = atof(att->getValues(att->as_string(0));
//att->getValues(&data.delta_lat);

   // Longitude increment
get_global_att(ncfile, "delta_lon_deg", data.delta_lon);
//data.delta_lon = atof(att->getValues(att->as_string(0));
//att->getValues(&data.delta_lon);

   // Number of points in the Latitude (y) direction
get_global_att(ncfile, "Nlat", data.Nlat);
//data.Nlat = atoi(att->getValues(att->as_string(0));
//att->getValues(&data.Nlat);

   // Number of points in the Longitudinal (x) direction
get_global_att(ncfile, "Nlon", data.Nlon);
//data.Nlon = atoi(att->getValues(att->as_string(0));
//att->getValues(&data.Nlon);

mlog << Debug(grid_debug_level)
     << "Latitude/Longitude Grid Data:\n"
     << "lat_ll = " << data.lat_ll << "\n"
     << "lon_ll = " << data.lon_ll << "\n"
     << "delta_lat = " << data.delta_lat << "\n"
     << "delta_lon = " << data.delta_lon << "\n"
     << "Nlat = " << data.Nlat << "\n"
     << "Nlon = " << data.Nlon << "\n";

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_lambert_data_v2(NcFile * ncfile, LambertData & data)

{

//NcGroupAtt * att = (NcGroupAtt *) 0;

   // Store the grid name
data.name = lambert_proj_type;

   // First scale latitude
get_global_att(ncfile, "p1_deg", data.scale_lat_1);
//data.scale_lat_1 = atof(att->getValues(att->as_string(0));
//att->getValues(&data.scale_lat_1);

   // Second scale latitude
get_global_att(ncfile, "p2_deg", data.scale_lat_2);
//data.scale_lat_2 = atof(att->getValues(att->as_string(0));
//att->getValues(&data.scale_lat_2);

   // Latitude pin
get_global_att(ncfile, "p0_deg", data.lat_pin);
//data.lat_pin = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lat_pin);

   // Longitude pin
get_global_att(ncfile, "l0_deg", data.lon_pin);
//data.lon_pin = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lon_pin);
data.lon_pin *= -1.0;

   // X pin
data.x_pin = 0.0;

   // Y pin
data.y_pin = 0.0;

   // Orientation longitude
get_global_att(ncfile, "lcen_deg", data.lon_orient);
//data.lon_orient = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lon_orient);
data.lon_orient *= -1.0;

   // Grid spacing in km
get_global_att(ncfile, "d_km", data.d_km);
//data.d_km = atof(att->getValues(att->as_string(0));
//att->getValues(&data.d_km);

   // Radius of the earth
get_global_att(ncfile, "r_km", data.r_km);
//data.r_km = atof(att->getValues(att->as_string(0));
//att->getValues(&data.r_km);

   // Number of points in the x-direction
get_global_att(ncfile, "nx", data.nx);
//data.nx = atoi(att->getValues(att->as_string(0));
//att->getValues(&data.nx);

   // Number of points in the y-direction
get_global_att(ncfile, "ny", data.ny);
//data.ny = atoi(att->getValues(att->as_string(0));
//att->getValues(&data.ny);

mlog << Debug(grid_debug_level)
     << "Lambert Conformal Grid Data:\n"
     << "scale_lat_1 = " << data.scale_lat_1 << "\n"
     << "scale_lat_2 = " << data.scale_lat_2 << "\n"
     << "lat_pin = " << data.lat_pin << "\n"
     << "lon_pin = " << data.lon_pin << "\n"
     << "x_pin = " << data.x_pin << "\n"
     << "y_pin = " << data.y_pin << "\n"
     << "lon_orient = " << data.lon_orient << "\n"
     << "d_km = " << data.d_km << "\n"
     << "r_km = " << data.r_km << "\n"
     << "nx = " << data.nx << "\n"
     << "ny = " << data.ny << "\n";

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_stereographic_data_v2(NcFile * ncfile, StereographicData & data)

{

//NcGroupAtt * att = (NcGroupAtt *) 0;

   // Store the grid name
data.name = stereographic_proj_type;

   // Hemisphere, assume northern
data.hemisphere = 'N';

   // Scale latitude
get_global_att(ncfile, "p1_deg", data.scale_lat);
//data.scale_lat = atof(att->getValues(att->as_string(0));
//att->getValues(&data.scale_lat);

   // Latitude pin
get_global_att(ncfile, "p0_deg", data.lat_pin);
//data.lat_pin = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lat_pin);

   // Longitude pin
get_global_att(ncfile, "l0_deg", data.lon_pin);
//data.lon_pin = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lon_pin);
data.lon_pin *= -1.0;

   // X pin
data.x_pin = 0.0;

   // Y pin
data.y_pin = 0.0;

   // Orientation longitude
get_global_att(ncfile, "lcen_deg", data.lon_orient);
//data.lon_orient = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lon_orient);
data.lon_orient *= -1.0;

   // Grid spacing in km
get_global_att(ncfile, "d_km", data.d_km);
//data.d_km = atof(att->getValues(att->as_string(0));
//att->getValues(&data.d_km);

   // Radius of the earth
get_global_att(ncfile, "r_km", data.r_km);
//data.r_km = atof(att->getValues(att->as_string(0));
//att->getValues(&data.r_km);

   // Number of points in the x-direction
get_global_att(ncfile, "nx", data.nx);
//data.nx = atoi(att->getValues(att->as_string(0));
//att->getValues(&data.nx);

   // Number of points in the y-direction
get_global_att(ncfile, "ny", data.ny);
//data.ny = atoi(att->getValues(att->as_string(0));
//att->getValues(&data.ny);

mlog << Debug(grid_debug_level)
     << "Stereographic Grid Data:\n"
     << "hemisphere = " << data.hemisphere << "\n"
     << "scale_lat = " << data.scale_lat << "\n"
     << "lat_pin = " << data.lat_pin << "\n"
     << "lon_pin = " << data.lon_pin << "\n"
     << "x_pin = " << data.x_pin << "\n"
     << "y_pin = " << data.y_pin << "\n"
     << "lon_orient = " << data.lon_orient << "\n"
     << "d_km = " << data.d_km << "\n"
     << "r_km = " << data.r_km << "\n"
     << "nx = " << data.nx << "\n"
     << "ny = " << data.ny << "\n";

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_mercator_data_v2(NcFile * ncfile, MercatorData & data)

{

//NcGroupAtt * att = (NcGroupAtt *) 0;

   // Store the grid name
data.name = mercator_proj_type;

   // Latitude of the bottom left corner
get_global_att(ncfile, "lat_ll_deg", data.lat_ll);
//data.lat_ll = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lat_ll);

   // Longitude of the bottom left corner
get_global_att(ncfile, "lon_ll_deg", data.lon_ll);
//data.lon_ll = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lon_ll);
data.lon_ll *= -1.0;

   // Latitude of the bottom left corner
get_global_att(ncfile, "lat_ur_deg", data.lat_ur);
//data.lat_ur = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lat_ur);

   // Longitude of the bottom left corner
get_global_att(ncfile, "lon_ur_deg", data.lon_ur);
//data.lon_ur = atof(att->getValues(att->as_string(0));
//att->getValues(&data.lon_ur);
data.lon_ur *= -1.0;

   // Number of points in the Latitudinal (y) direction
get_global_att(ncfile, "Nlat", data.ny);
//data.ny = atoi(att->getValues(att->as_string(0));
//att->getValues(&data.ny);

   // Number of points in the Longitudinal (x) direction
get_global_att(ncfile, "Nlon", data.nx);
//data.nx = atoi(att->getValues(att->as_string(0));
//att->getValues(&data.nx);

mlog << Debug(grid_debug_level)
     << "Mercator Data:\n"
     << "lat_ll = " << data.lat_ll << "\n"
     << "lon_ll = " << data.lon_ll << "\n"
     << "lat_ur = " << data.lat_ur << "\n"
     << "lon_ur = " << data.lon_ur << "\n"
     << "ny = " << data.ny << "\n"
     << "nx = " << data.nx << "\n";

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////
