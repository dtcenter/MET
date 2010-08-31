

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2010
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

#include <netcdf.hh>

#include "vx_math/vx_math.h"

#include "vx_gdata/get_pinterp_grid.h"


////////////////////////////////////////////////////////////////////////


static const char       ps_target_name      [] = "Polar_Stereographic";
static const char  lambert_target_name      [] = "Lambert_Conformal";
static const char mercator_target_name      [] = "Mercator";

static const char nx_dimension_name         [] = "west_east";
static const char ny_dimension_name         [] = "south_north";

static const char ps_default_gridname       [] = "polar";
static const char lambert_default_gridname  [] = "lambert";
static const char mercator_default_gridname [] = "lambert";

static const double default_grib_radius_km     = 6367.47;


////////////////////////////////////////////////////////////////////////


static bool   get_ps_grid       (NcFile & nc, Grid & grid);
static bool   get_lambert_grid  (NcFile & nc, Grid & grid);
static bool   get_mercator_grid (NcFile & nc, Grid & grid);

static int    get_dimension(NcFile &, const char * name);

static double get_att_as_double(NcFile &, const char * name);

static double mercator_lon_to_u(double lon);
static double mercator_lat_to_v(double lat);

static double mercator_u_to_lon(double u);
static double mercator_v_to_lat(double v);


////////////////////////////////////////////////////////////////////////


bool get_pinterp_grid(const char * pinterp_filename, Grid & grid)

{

bool status = false;
NcFile nc(pinterp_filename);


status = get_pinterp_grid(nc, grid);


return ( status );

}


////////////////////////////////////////////////////////////////////////


bool get_pinterp_grid(NcFile & nc, Grid & grid)

{

int j, n;
NcVar * var = (NcVar *) 0;
bool status = false;


grid.clear();

   //
   //  figure out which projection this is
   //

n = nc.num_vars();

status = false;

for (j=0; j<n; ++j)  {

   var = nc.get_var(j);

   if ( strcmp(var->name(),       ps_target_name) == 0 )  { status = get_ps_grid       (nc, grid);  break; }
   if ( strcmp(var->name(),  lambert_target_name) == 0 )  { status = get_lambert_grid  (nc, grid);  break; }
   if ( strcmp(var->name(), mercator_target_name) == 0 )  { status = get_mercator_grid (nc, grid);  break; }

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

data.scale_lat = get_att_as_double(nc, "TRUELAT1");

   //
   //  hemisphere ... assume north?
   //

if ( data.scale_lat < 0.0 )  data.hemisphere = 'S';
else                         data.hemisphere = 'N';

   //
   //  Nx, Ny
   //

data.nx = get_dimension(nc, nx_dimension_name);
data.ny = get_dimension(nc, ny_dimension_name);

   //
   //  pin point
   //

data.lat_pin =   get_att_as_double(nc, "CEN_LAT");
data.lon_pin = -(get_att_as_double(nc, "CEN_LON"));

data.x_pin = 0.5*(data.nx - 1.0);
data.y_pin = 0.5*(data.ny - 1.0);

   //
   //  orientation longitude
   //

data.lon_orient = -(get_att_as_double(nc, "STAND_LON"));

   //
   //  D, R
   //

data.d_km = 0.001*(get_att_as_double(nc, "DX"));

data.r_km = default_grib_radius_km;

   //
   //  done
   //

grid.set(data);

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

data.scale_lat_1 = get_att_as_double(nc, "TRUELAT1");
data.scale_lat_2 = get_att_as_double(nc, "TRUELAT2");

   //
   //  Nx, Ny
   //

data.nx = get_dimension(nc, nx_dimension_name);
data.ny = get_dimension(nc, ny_dimension_name);

   //
   //  pin point
   //

data.lat_pin =   get_att_as_double(nc, "CEN_LAT");
data.lon_pin = -(get_att_as_double(nc, "CEN_LON"));

data.x_pin = 0.5*(data.nx - 1.0);
data.y_pin = 0.5*(data.ny - 1.0);

   //
   //  orientation longitude
   //

data.lon_orient = -(get_att_as_double(nc, "STAND_LON"));

   //
   //  D, R
   //

data.d_km = 0.001*(get_att_as_double(nc, "DX"));

data.r_km = default_grib_radius_km;

   //
   //  done
   //

grid.set(data);

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

data.nx = get_dimension(nc, nx_dimension_name);
data.ny = get_dimension(nc, ny_dimension_name);

   //
   //  center lat, lon
   //

lat_center =   get_att_as_double(nc, "CEN_LAT");
lon_center = -(get_att_as_double(nc, "CEN_LON"));

   //
   //  D_km
   //

D_km = 0.001*(get_att_as_double(nc, "DX"));

   //
   //  scale latitude
   //

scale_lat = get_att_as_double(nc, "TRUELAT1");

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

return ( true );

}


////////////////////////////////////////////////////////////////////////


int get_dimension(NcFile & nc, const char * name)

{

int d = 0;
NcDim * dim = (NcDim *) 0;

dim = nc.get_dim(name);

d = (int) (dim->size());

   //
   //  done
   //

return ( d );

}


////////////////////////////////////////////////////////////////////////


double get_att_as_double(NcFile & nc, const char * name)

{

double value = 0.0;
NcAtt * att = (NcAtt *) 0;

att = nc.get_att(name);


if ( (att->type() != ncDouble) && (att->type() != ncFloat) )  {

   cerr << "\n\n  get_att_as_double(NcFile &, const char * name) -> can't get attribute \"" << name << "\"\n\n";

   exit ( 1 );

}

value = att->as_double(0);

   //
   //  done
   //

return ( value );

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


