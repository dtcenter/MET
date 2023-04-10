// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include <netcdf>
using namespace netCDF;

#include "grid_output.h"
#include "vx_log.h"
#include "nc_utils.h"


////////////////////////////////////////////////////////////////////////


static void lambert_grid_output        (const GridInfo &, NcFile * ncfile);
static void latlon_grid_output         (const GridInfo &, NcFile * ncfile);
static void rotated_latlon_grid_output (const GridInfo &, NcFile * ncfile);
static void stereographic_grid_output  (const GridInfo &, NcFile * ncfile);
static void mercator_grid_output       (const GridInfo &, NcFile * ncfile);
static void gaussian_grid_output       (const GridInfo &, NcFile * ncfile);
static void laea_grid_output           (const GridInfo &, NcFile * ncfile);
static void laea_grib2_grid_output     (const GridInfo &, NcFile * ncfile);
static void semilatlon_grid_output     (const GridInfo &, NcFile * ncfile, NcDim &, NcDim &);
static void write_semilatlon_var       (NcFile * ncfile, const char *,
                                        NcDim *, const NumArray &, const char *,
                                        const char *, const char *);


////////////////////////////////////////////////////////////////////////


void grid_output(const GridInfo & info, NcFile * ncfile,
                 NcDim & lat_dim, NcDim & lon_dim)

{


if ( !(info.ok()) )  {

   mlog << Error << "\ngrid_output(const GridInfo &, NcFile *) -> bad grid info!\n\n";

   exit ( 1 );

}

     if ( info.lc  )  lambert_grid_output         (info, ncfile);
else if ( info.st  )  stereographic_grid_output   (info, ncfile);
else if ( info.ll  )  latlon_grid_output          (info, ncfile);
else if ( info.rll )  rotated_latlon_grid_output  (info, ncfile);
else if ( info.m   )  mercator_grid_output        (info, ncfile);
else if ( info.g   )  gaussian_grid_output        (info, ncfile);
else if ( info.la  )  laea_grid_output            (info, ncfile);
else if ( info.lg  )  laea_grib2_grid_output      (info, ncfile);
else if ( info.sl  )  semilatlon_grid_output      (info, ncfile, lat_dim, lon_dim);
else {

   mlog << Error << "\ngrid_output(const GridInfo &, NcFile *) -> can't determine projection!\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void lambert_grid_output(const GridInfo & info, NcFile * ncfile)

{

char junk[256];
double t;
const LambertData & data = *(info.lc);


add_att(ncfile, "Projection", lambert_proj_type);

   //
   //  hemisphere
   //

snprintf(junk, sizeof(junk), "%c", data.hemisphere);

ncfile->putAtt("hemisphere", junk);

   //
   //  scale_lat_1
   //

snprintf(junk, sizeof(junk), "%.6f", data.scale_lat_1);

ncfile->putAtt("scale_lat_1", junk);

   //
   //  scale_lat_2
   //

snprintf(junk, sizeof(junk), "%.6f", data.scale_lat_2);

ncfile->putAtt("scale_lat_2", junk);


   //
   //  lat_pin
   //

snprintf(junk, sizeof(junk), "%.6f", data.lat_pin);

ncfile->putAtt("lat_pin", junk);

   //
   //  lon_pin
   //

t = data.lon_pin;

if ( !west_longitude_positive )  t = -t;

snprintf(junk, sizeof(junk), "%.6f", t);

ncfile->putAtt("lon_pin", junk);

   //
   //  x_pin
   //

snprintf(junk, sizeof(junk), "%.6f", data.x_pin);

ncfile->putAtt("x_pin", junk);

   //
   //  y_pin
   //

snprintf(junk, sizeof(junk), "%.6f", data.y_pin);

ncfile->putAtt("y_pin", junk);

   //
   //  lon_orient
   //

t = data.lon_orient;

if ( !west_longitude_positive )  t = -t;

snprintf(junk, sizeof(junk), "%.6f", t);

ncfile->putAtt("lon_orient", junk);

   //
   //  d_km
   //

snprintf(junk, sizeof(junk), "%.6f", data.d_km);

ncfile->putAtt("d_km", junk);

   //
   //  r_km
   //

snprintf(junk, sizeof(junk), "%.6f", data.r_km);

ncfile->putAtt("r_km", junk);

   //
   //  nx
   //

snprintf(junk, sizeof(junk), "%d", data.nx);

ncfile->putAtt("nx", junk);

   //
   //  ny
   //

snprintf(junk, sizeof(junk), "%d grid_points", data.ny);

ncfile->putAtt("ny", junk);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void latlon_grid_output(const GridInfo & info, NcFile * ncfile)

{

char junk[256];
double t;
const LatLonData & data = *(info.ll);


ncfile->putAtt("Projection", latlon_proj_type);

   //
   //  lat_ll
   //

snprintf(junk, sizeof(junk), "%f degrees_north", data.lat_ll);

ncfile->putAtt("lat_ll", junk);

   //
   //  lon_ll
   //

t = data.lon_ll;

if ( !west_longitude_positive )  t = -t;

snprintf(junk, sizeof(junk), "%f degrees_east", t);

ncfile->putAtt("lon_ll", junk);

   //
   //  delta_lat
   //

snprintf(junk, sizeof(junk), "%f degrees", data.delta_lat);

ncfile->putAtt("delta_lat", junk);

   //
   //  delta_lon
   //

t = data.delta_lon;

snprintf(junk, sizeof(junk), "%f degrees", t);

ncfile->putAtt("delta_lon", junk);

   //
   //  Nlat
   //

snprintf(junk, sizeof(junk), "%d grid_points", data.Nlat);

ncfile->putAtt("Nlat", junk);

   //
   //  Nlon
   //

snprintf(junk, sizeof(junk), "%d grid_points", data.Nlon);

ncfile->putAtt("Nlon", junk);

   //
   // done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void rotated_latlon_grid_output(const GridInfo & info, NcFile * ncfile)

{

char junk[256];
double t;
const RotatedLatLonData & data = *(info.rll);

ncfile->putAtt("Projection", rotated_latlon_proj_type);


   //
   //  rot_lat_ll
   //

snprintf(junk, sizeof(junk), "%f degrees_north", data.rot_lat_ll);

ncfile->putAtt("rot_lat_ll", junk);

   //
   //  rot_lon_ll
   //

t = data.rot_lon_ll;

if ( !west_longitude_positive )  t = -t;

snprintf(junk, sizeof(junk), "%f degrees_east", t);

ncfile->putAtt("rot_lon_ll", junk);

   //
   //  delta_rot_lat
   //

snprintf(junk, sizeof(junk), "%f degrees", data.delta_rot_lat);

ncfile->putAtt("delta_rot_lat", junk);

   //
   //  delta_rot_lon
   //

t = data.delta_rot_lon;

snprintf(junk, sizeof(junk), "%f degrees", t);

ncfile->putAtt("delta_rot_lon", junk);

   //
   //  Nlat
   //

snprintf(junk, sizeof(junk), "%d grid_points", data.Nlat);

ncfile->putAtt("Nlat", junk);

   //
   //  Nlon
   //

snprintf(junk, sizeof(junk), "%d grid_points", data.Nlon);

ncfile->putAtt("Nlon", junk);

   //
   //  true_lat_south_pole
   //

snprintf(junk, sizeof(junk), "%f degrees_north", data.true_lat_south_pole);

ncfile->putAtt("true_lat_south_pole", junk);

   //
   //  true_lon_south_pole
   //

t = data.true_lon_south_pole;

if ( !west_longitude_positive )  t = -t;

snprintf(junk, sizeof(junk), "%f degrees_east", t);

ncfile->putAtt("true_lon_south_pole", junk);

   //
   //  auxilliary rotation
   //

snprintf(junk, sizeof(junk), "%f degrees", data.aux_rotation);

ncfile->putAtt("aux_rotation", junk);


   //
   // done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void stereographic_grid_output(const GridInfo & info, NcFile * ncfile)

{

char junk[256];
double t;
const StereographicData & data = *(info.st);


ncfile->putAtt("Projection", stereographic_proj_type);

   //
   //  hemisphere
   //

snprintf(junk, sizeof(junk), "%c", data.hemisphere);

ncfile->putAtt("hemisphere", junk);

   //
   //  scale_lat
   //

snprintf(junk, sizeof(junk), "%f degrees_north", data.scale_lat);

ncfile->putAtt("scale_lat", junk);

   //
   //  lat_pin
   //

snprintf(junk, sizeof(junk), "%.6f", data.lat_pin);

ncfile->putAtt("lat_pin", junk);

   //
   //  lon_pin
   //

t = data.lon_pin;

if ( !west_longitude_positive )  t = -t;

snprintf(junk, sizeof(junk), "%.6f", t);

ncfile->putAtt("lon_pin", junk);

   //
   //  x_pin
   //

snprintf(junk, sizeof(junk), "%.6f", data.x_pin);

ncfile->putAtt("x_pin", junk);

   //
   //  y_pin
   //

snprintf(junk, sizeof(junk), "%.6f", data.y_pin);

ncfile->putAtt("y_pin", junk);

   //
   //  lon_orient
   //

t = data.lon_orient;

if ( !west_longitude_positive )  t = -t;

snprintf(junk, sizeof(junk), "%.6f", t);

ncfile->putAtt("lon_orient", junk);

   //
   //  d_km
   //

snprintf(junk, sizeof(junk), "%.6f km", data.d_km);

ncfile->putAtt("d_km", junk);

   //
   //  r_km
   //

snprintf(junk, sizeof(junk), "%.6f km", data.r_km);

ncfile->putAtt("r_km", junk);

   //
   //  nx
   //

snprintf(junk, sizeof(junk), "%d", data.nx);

ncfile->putAtt("nx", junk);

   //
   //  ny
   //

snprintf(junk, sizeof(junk), "%d", data.ny);

ncfile->putAtt("ny", junk);

   //
   // done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void mercator_grid_output(const GridInfo & info, NcFile * ncfile)

{

char junk[256];
double t;
const MercatorData & data = *(info.m);


ncfile->putAtt("Projection", mercator_proj_type);

   //
   //  lat_ll
   //

snprintf(junk, sizeof(junk), "%f degrees_north", data.lat_ll);

ncfile->putAtt("lat_ll", junk);

   //
   //  lon_ll
   //

t = data.lon_ll;

if ( !west_longitude_positive )  t = -t;

snprintf(junk, sizeof(junk), "%f degrees_east", t);

ncfile->putAtt("lon_ll", junk);

   //
   //  lat_ur
   //

snprintf(junk, sizeof(junk), "%f degrees_north", data.lat_ur);

ncfile->putAtt("lat_ur", junk);

   //
   //  lon_ur
   //

t = data.lon_ur;

if ( !west_longitude_positive )  t = -t;

snprintf(junk, sizeof(junk), "%f degrees_east", t);

ncfile->putAtt("lon_ur", junk);

   //
   //  nx
   //

snprintf(junk, sizeof(junk), "%d", data.nx);

ncfile->putAtt("nx", junk);

   //
   //  ny
   //

snprintf(junk, sizeof(junk), "%d", data.ny);

ncfile->putAtt("ny", junk);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void gaussian_grid_output(const GridInfo & info, NcFile * ncfile)

{

char junk[256];
const GaussianData & data = *(info.g);



ncfile->putAtt("Projection", gaussian_proj_type);

   //
   //  Lon_Zero
   //

snprintf(junk, sizeof(junk), "%.3f degrees_east", -(data.lon_zero));

ncfile->putAtt("lon_zero", junk);

   //
   //  nx
   //

snprintf(junk, sizeof(junk), "%d", data.nx);

ncfile->putAtt("nx", junk);

   //
   //  ny
   //

snprintf(junk, sizeof(junk), "%d", data.ny);

ncfile->putAtt("ny", junk);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void laea_grid_output(const GridInfo & info, NcFile * ncfile)

{

char junk[256];
double t;
const LaeaData & data = *(info.la);

ncfile->putAtt("Projection", "Lambert Azimuthal Equal Area");

if(data.geoid) ncfile->putAtt("geoid", data.geoid);

   //
   //  lat_LL
   //

snprintf(junk, sizeof(junk), "%f degrees_north", data.lat_LL);

ncfile->putAtt("lat_LL", junk);

   //
   //  lon_LL
   //

t = data.lon_LL;

if ( !west_longitude_positive )  t = -t;

snprintf(junk, sizeof(junk), "%f degrees_east", t);

ncfile->putAtt("lon_LL", junk);

   //
   //  lat_UL
   //

snprintf(junk, sizeof(junk), "%f degrees_north", data.lat_UL);

ncfile->putAtt("lat_UL", junk);

   //
   //  lon_UL
   //

t = data.lon_UL;

if ( !west_longitude_positive )  t = -t;

snprintf(junk, sizeof(junk), "%f degrees_east", t);

ncfile->putAtt("lon_UL", junk);

   //
   //  lat_LR
   //

snprintf(junk, sizeof(junk), "%f degrees_north", data.lat_LR);

ncfile->putAtt("lat_LR", junk);

   //
   //  lon_LR
   //

t = data.lon_LR;

if ( !west_longitude_positive )  t = -t;

snprintf(junk, sizeof(junk), "%f degrees_east", t);

ncfile->putAtt("lon_LR", junk);

   //
   //  lat_pole
   //

snprintf(junk, sizeof(junk), "%f degrees_north", data.lat_pole);

ncfile->putAtt("lat_pole", junk);

   //
   //  lon_pole
   //

t = data.lon_pole;

if ( !west_longitude_positive )  t = -t;

snprintf(junk, sizeof(junk), "%f degrees_east", t);

ncfile->putAtt("lon_pole", junk);

   //
   //  nx
   //

snprintf(junk, sizeof(junk), "%d", data.nx);

ncfile->putAtt("nx", junk);

   //
   //  ny
   //

snprintf(junk, sizeof(junk), "%d", data.ny);

ncfile->putAtt("ny", junk);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void laea_grib2_grid_output(const GridInfo & info, NcFile * ncfile)

{

char junk[256];
double t;
const LaeaGrib2Data & data = *(info.lg);

ncfile->putAtt("Projection", "Grib2 Lambert Azimuthal Equal Area");

ncfile->putAtt("spheroid_name", data.spheroid_name);

   //
   //  radius_km
   //

snprintf(junk, sizeof(junk), "%f km", data.radius_km);

ncfile->putAtt("radius_km", junk);

   //
   //  equatorial_radius_km
   //

snprintf(junk, sizeof(junk), "%f km", data.equatorial_radius_km);

ncfile->putAtt("equatorial_radius_km", junk);

   //
   //  polar_radius_km
   //

snprintf(junk, sizeof(junk), "%f km", data.polar_radius_km);

ncfile->putAtt("polar_radius_km", junk);

   //
   //  lat_first
   //

snprintf(junk, sizeof(junk), "%f degrees_north", data.lat_first);

ncfile->putAtt("lat_first", junk);

   //
   //  lon_first
   //

t = data.lon_first;

if ( !west_longitude_positive )  t = -t;

snprintf(junk, sizeof(junk), "%f degrees_east", t);

ncfile->putAtt("lon_first", junk);

   //
   //  standard_lat
   //

snprintf(junk, sizeof(junk), "%f degrees_north", data.standard_lat);

ncfile->putAtt("standard_lat", junk);

   //
   //  central_lon
   //

t = data.central_lon;

if ( !west_longitude_positive )  t = -t;

snprintf(junk, sizeof(junk), "%f degrees_east", t);

ncfile->putAtt("central_lon", junk);

   //
   //  dx_km
   //

snprintf(junk, sizeof(junk), "%.6f", data.dx_km);

ncfile->putAtt("dx_km", junk);

   //
   //  dy_km
   //

snprintf(junk, sizeof(junk), "%.6f", data.dy_km);

ncfile->putAtt("dy_km", junk);

   //
   //  nx
   //

snprintf(junk, sizeof(junk), "%d", data.nx);

ncfile->putAtt("nx", junk);

   //
   //  ny
   //

snprintf(junk, sizeof(junk), "%d", data.ny);

ncfile->putAtt("ny", junk);

   //
   //  is_sphere
   //

snprintf(junk, sizeof(junk), "%s", bool_to_string(data.is_sphere));

ncfile->putAtt("is_sphere", junk);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void semilatlon_grid_output(const GridInfo & info, NcFile * ncfile,
                            NcDim & lat_dim, NcDim & lon_dim)

{

const SemiLatLonData & data = *(info.sl);

ncfile->putAtt("Projection", semilatlon_proj_type);

NcDim nc_dim, new_lat_dim, new_lon_dim;

   //
   //  lat and lon data
   //  create new lat/lon dimensions in case the inputs
   //  are already defined
   //

if ( data.lats.n() == data.lons.n() && data.lats.n() > 0 )  {
   new_lon_dim = add_dim(ncfile, "latlon", (long) data.lats.n());
   write_semilatlon_var(ncfile, "lat", &new_lon_dim, data.lats,
                        "latitude", "degrees_north", "latitude");
   write_semilatlon_var(ncfile, "lon", &new_lon_dim, data.lons,
                        "longitude", "degrees_east", "longitude");
}
else if ( data.lats.n() > 0 )  {
   new_lat_dim = add_dim(ncfile, "lat", (long) data.lats.n());
   write_semilatlon_var(ncfile, "lat", &new_lat_dim, data.lats,
                        "latitude", "degrees_north", "latitude");
}
else if ( data.lons.n() > 0 )  {
   new_lon_dim = add_dim(ncfile, "lon", (long) data.lons.n());
   write_semilatlon_var(ncfile, "lon", &new_lon_dim, data.lons,
                        "longitude", "degrees_east", "longitude");
}
else {
   mlog << Error << "\nsemilatlon_grid_output(const GridInfo & info, NcFile * ncfile) -> "
        << "lat and lon arrays should not both be empty!\n\n";
   exit ( 1 );
}

   //
   //  level and time data
   //

if ( data.levels.n() > 0 )  {
   nc_dim = add_dim(ncfile, "level", (long) data.levels.n());
   write_semilatlon_var(ncfile, "level", &nc_dim, data.levels,
                        "level", 0, 0);
}
else if ( data.times.n() > 0 )  {
   nc_dim = add_dim(ncfile, "time", (long) data.times.n());
   write_semilatlon_var(ncfile, "times", &nc_dim, data.times,
                        "time", 0, 0);
}
else {
   mlog << Error << "\nsemilatlon_grid_output(const GridInfo & info, NcFile * ncfile) -> "
        << "level and time arrays should not both be empty!\n\n";
   exit ( 1 );
}

   //
   //  store the second dimension
   //

     if ( new_lat_dim.isNull() )  new_lat_dim = nc_dim;
else if ( new_lon_dim.isNull() )  new_lon_dim = nc_dim;

   //
   //  save the newly created dimensions
   //

lat_dim = new_lat_dim;
lon_dim = new_lon_dim;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void write_semilatlon_var(NcFile * ncfile, const char * var_name,
                          NcDim * nc_dim, const NumArray &var,
                          const char * long_name_str, const char * units_str,
                          const char * standard_name_str) {

NcVar nc_var = ncfile->addVar(var_name, ncFloat, *nc_dim);
float * var_data = new float [var.n()];
for (int i=0; i<var.n(); i++)  var_data[i] = var[i];

   //
   //  add attributes
   //

if ( long_name_str )      add_att(&nc_var, long_name_att_name, long_name_str);
if ( units_str )          add_att(&nc_var, units_att_name, units_str);
if ( standard_name_str )  add_att(&nc_var, standard_name_att_name, standard_name_str);

   //
   //  write data and cleanup
   //

put_nc_data(&nc_var, &var_data[0], nc_dim->getSize(), 0);

if ( var_data )  { delete [] var_data; var_data = (float *) 0; }

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////
