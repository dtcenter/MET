

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
#include <cmath>

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


////////////////////////////////////////////////////////////////////////


void grid_output(const GridInfo & info, NcFile * ncfile)

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


add_att(ncfile, "Projection", "Lambert Conformal");

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


ncfile->putAtt("Projection", "LatLon");

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


ncfile->putAtt("Projection", "Polar Stereographic");

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


ncfile->putAtt("Projection", "Mercator");

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



ncfile->putAtt("Projection", "Gaussian");

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



