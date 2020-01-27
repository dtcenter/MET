

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include <netcdf>
using namespace netCDF;

#include "vx_log.h"
#include "vx_math.h"
#include "vx_nc_util.h"

#include "get_pinterp_grid.h"


////////////////////////////////////////////////////////////////////////


static const string proj_att_name             = "MAP_PROJ_CHAR";
static const char ps_proj_att_name          [] = "Polar Stereographic";
static const char lambert_proj_att_name     [] = "Lambert Conformal";
static const char mercator_proj_att_name    [] = "Mercator";

static const char ps_proj_var_name          [] = "Polar_Stereographic";
static const char lambert_proj_var_name     [] = "Lambert_Conformal";
static const char mercator_proj_var_name    [] = "Mercator";

static const string nx_dimension_name          = "west_east";
static const string ny_dimension_name          = "south_north";

static const char ps_default_gridname       [] = "polar";
static const char lambert_default_gridname  [] = "lambert";
static const char mercator_default_gridname [] = "mercator";

static const double default_grib_radius_km     = 6371.20;


////////////////////////////////////////////////////////////////////////


static bool   get_ps_grid       (NcFile & nc, Grid & grid);
static bool   get_lambert_grid  (NcFile & nc, Grid & grid);
static bool   get_mercator_grid (NcFile & nc, Grid & grid);

static double mercator_lon_to_u(double lon);
static double mercator_lat_to_v(double lat);

static double mercator_u_to_lon(double u);
static double mercator_v_to_lat(double v);


////////////////////////////////////////////////////////////////////////


bool get_pinterp_grid(const char * pinterp_filename, Grid & grid)

{

bool status = false;
NcFile nc(pinterp_filename, NcFile::read);


status = get_pinterp_grid(nc, grid);


return ( status );

}


////////////////////////////////////////////////////////////////////////


bool get_pinterp_grid(NcFile & nc, Grid & grid)

{

ConcatString proj_att_value;
bool status = false;


grid.clear();

if ( get_global_att(&nc, proj_att_name, proj_att_value) ) {

   //
   //  if present, parse the global projection attribute value
   //
        if ( strcasecmp(proj_att_value.c_str(),       ps_proj_att_name) == 0 )  { status = get_ps_grid       (nc, grid); }
   else if ( strcasecmp(proj_att_value.c_str(),  lambert_proj_att_name) == 0 )  { status = get_lambert_grid  (nc, grid); }
   else if ( strcasecmp(proj_att_value.c_str(), mercator_proj_att_name) == 0 )  { status = get_mercator_grid (nc, grid); }
}
else {

   //
   //  otherwise, check for the projection variable
   //
        if ( has_var(&nc,       ps_proj_var_name) ) { status = get_ps_grid       (nc, grid); }
   else if ( has_var(&nc,  lambert_proj_var_name) ) { status = get_lambert_grid  (nc, grid); }
   else if ( has_var(&nc, mercator_proj_var_name) ) { status = get_mercator_grid (nc, grid); }
}

   //
   //  done
   //

return ( status );

}


////////////////////////////////////////////////////////////////////////


bool get_ps_grid (NcFile & nc, Grid & grid)

{

StereographicData data;

   //
   //  name ... ?
   //

data.name = ps_default_gridname;

   //
   //  scale latitude
   //

get_global_att_double(&nc, (string)"TRUELAT1", data.scale_lat, true);

   //
   //  hemisphere ... assume north?
   //

if ( data.scale_lat < 0.0 )  data.hemisphere = 'S';
else                         data.hemisphere = 'N';

   //
   //  Nx, Ny
   //

get_dim(&nc, nx_dimension_name, data.nx, true);
get_dim(&nc, ny_dimension_name, data.ny, true);

   //
   //  pin point
   //

get_global_att_double(&nc, (string)"CEN_LAT", data.lat_pin, true);
get_global_att_double(&nc, (string)"CEN_LON", data.lon_pin, true);
data.lon_pin *= -1.0;

data.x_pin = 0.5*(data.nx - 1.0);
data.y_pin = 0.5*(data.ny - 1.0);

   //
   //  orientation longitude
   //

get_global_att_double(&nc, (string)"STAND_LON", data.lon_orient, true);
data.lon_orient *= -1.0;

   //
   //  D, R
   //

get_global_att_double(&nc, (string)"DX", data.d_km, true);
data.d_km *= 0.001;

data.r_km = default_grib_radius_km;

   //
   //  done
   //

grid.set(data);

data.dump();

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool get_lambert_grid (NcFile & nc, Grid & grid)

{

LambertData data;


   //
   //  name ... ?
   //

data.name = lambert_default_gridname;

   //
   //  scale latitude(s)
   //

get_global_att_double(&nc, (string)"TRUELAT1", data.scale_lat_1, true);
get_global_att_double(&nc, (string)"TRUELAT2", data.scale_lat_2, true);

   //
   //  hemisphere ... assume north?
   //

if ( data.scale_lat_1 < 0.0 )  data.hemisphere = 'S';
else                           data.hemisphere = 'N';

   //
   //  Nx, Ny
   //

get_dim(&nc, nx_dimension_name, data.nx, true);
get_dim(&nc, ny_dimension_name, data.ny, true);

   //
   //  pin point
   //

get_global_att_double(&nc, (string)"CEN_LAT", data.lat_pin, true);
get_global_att_double(&nc, (string)"CEN_LON", data.lon_pin, true);
data.lon_pin *= -1.0;

data.x_pin = 0.5*(data.nx - 1.0);
data.y_pin = 0.5*(data.ny - 1.0);

   //
   //  orientation longitude
   //

get_global_att_double(&nc, (string)"STAND_LON", data.lon_orient, true);
data.lon_orient *= -1.0;

   //
   //  D, R
   //

get_global_att_double(&nc, (string)"DX", data.d_km, true);
data.d_km *= 0.001;

data.r_km = default_grib_radius_km;

   //
   //  rotation angle
   //

data.so2_angle = 0.0;

   //
   //  done
   //

grid.set(data);

data.dump();

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool get_mercator_grid (NcFile & nc, Grid & grid)

{

double D_km;
double scale_factor, scale_lat;
double lat_center, lon_center;
double tx, ty;
double x, y, u, v, lat, lon;
MercatorData data;

   //
   //  name
   //

data.name = mercator_default_gridname;

   //
   //  nx, ny
   //

get_dim(&nc, nx_dimension_name, data.nx, true);
get_dim(&nc, ny_dimension_name, data.ny, true);

   //
   //  center lat, lon
   //

get_global_att_double(&nc, (string)"CEN_LAT", lat_center, true);
get_global_att_double(&nc, (string)"CEN_LON", lon_center, true);
lon_center *= -1.0;

   //
   //  D_km
   //

get_global_att_double(&nc, (string)"DX", D_km, true);
D_km *= 0.001;

   //
   //  scale latitude
   //

get_global_att_double(&nc, (string)"TRUELAT1", scale_lat, true);

   //
   //  do some calculations
   //

scale_factor = (default_grib_radius_km/D_km)*cosd(scale_lat);

x = 0.5*(data.nx - 1);
y = 0.5*(data.ny - 1);

lat = lat_center;
lon = lon_center;

u = mercator_lon_to_u(lon);
v = mercator_lat_to_v(lat);

tx = x - scale_factor*u;
ty = y - scale_factor*v;

   //
   //  lat_ll, lon_ll
   //

x = 0.0;
y = 0.0;

u = (x - tx)/scale_factor;
v = (y - ty)/scale_factor;

data.lat_ll = mercator_v_to_lat(v);
data.lon_ll = mercator_u_to_lon(u);

   //
   //  lat_ur, lon_ur
   //

x = data.nx - 1.0;
y = data.ny - 1.0;

u = (x - tx)/scale_factor;
v = (y - ty)/scale_factor;

data.lat_ur = mercator_v_to_lat(v);
data.lon_ur = mercator_u_to_lon(u);

   //
   //  done
   //

grid.set(data);

data.dump();

return ( true );

}


////////////////////////////////////////////////////////////////////////


double mercator_lon_to_u(double lon)

{

double u, lon_rad;

lon_rad = lon*rad_per_deg;

u = -lon_rad;

return ( u );

}


////////////////////////////////////////////////////////////////////////


double mercator_lat_to_v(double lat)

{

double v;

v = log(tand(45.0 + 0.5*lat));

return ( v );

}


////////////////////////////////////////////////////////////////////////


double mercator_u_to_lon(double u)

{

double lon_rad, lon_deg;

lon_rad = -u;

lon_deg = lon_rad*deg_per_rad;

return ( lon_deg );

}


////////////////////////////////////////////////////////////////////////


double mercator_v_to_lat(double v)

{

double lat;

lat = 2.0*atand(exp(v)) - 90.0;

return ( lat );

}


////////////////////////////////////////////////////////////////////////


