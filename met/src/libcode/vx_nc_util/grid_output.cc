

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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


////////////////////////////////////////////////////////////////////////


static void lambert_grid_output       (const GridInfo &, NcFile * ncfile);
static void latlon_grid_output        (const GridInfo &, NcFile * ncfile);
static void stereographic_grid_output (const GridInfo &, NcFile * ncfile);
static void mercator_grid_output      (const GridInfo &, NcFile * ncfile);


////////////////////////////////////////////////////////////////////////


void grid_output(const GridInfo & info, NcFile * ncfile)

{


if ( !(info.ok()) )  {

   mlog << Error << "\ngrid_output(const GridInfo &, NcFile *) -> bad grid info!\n\n";

   exit ( 1 );

}

     if ( info.lc )  lambert_grid_output       (info, ncfile);
else if ( info.st )  stereographic_grid_output (info, ncfile);
else if ( info.ll )  latlon_grid_output        (info, ncfile);
else if ( info.m  )  mercator_grid_output      (info, ncfile);
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


ncfile->add_att("Projection", "Lambert Conformal");

   //
   //  scale_lat_1
   //

sprintf(junk, "%.6f", data.scale_lat_1);

ncfile->add_att("scale_lat_1", junk);

   //
   //  scale_lat_2
   //

sprintf(junk, "%.6f", data.scale_lat_2);

ncfile->add_att("scale_lat_2", junk);


   //
   //  lat_pin
   //

sprintf(junk, "%.6f", data.lat_pin);

ncfile->add_att("lat_pin", junk);

   //
   //  lon_pin
   //

t = data.lon_pin;

if ( !west_longitude_positive )  t = -t;

sprintf(junk, "%.6f", t);

ncfile->add_att("lon_pin", junk);

   //
   //  x_pin
   //

sprintf(junk, "%.6f", data.x_pin);

ncfile->add_att("x_pin", junk);

   //
   //  y_pin
   //

sprintf(junk, "%.6f", data.y_pin);

ncfile->add_att("y_pin", junk);

   //
   //  lon_orient
   //

t = data.lon_orient;

if ( !west_longitude_positive )  t = -t;

sprintf(junk, "%.6f", t);

ncfile->add_att("lon_orient", junk);

   //
   //  d_km
   //

sprintf(junk, "%.6f", data.d_km);

ncfile->add_att("d_km", junk);

   //
   //  r_km
   //

sprintf(junk, "%.6f", data.r_km);

ncfile->add_att("r_km", junk);

   //
   //  nx
   //

sprintf(junk, "%d", data.nx);

ncfile->add_att("nx", junk);

   //
   //  ny
   //

sprintf(junk, "%d grid_points", data.ny);

ncfile->add_att("ny", junk);

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


ncfile->add_att("Projection", "LatLon");

   //
   //  lat_ll
   //

sprintf(junk, "%f degrees_north", data.lat_ll);

ncfile->add_att("lat_ll", junk);

   //
   //  lon_ll
   //

t = data.lon_ll;

if ( !west_longitude_positive )  t = -t;

sprintf(junk, "%f degrees_east", t);

ncfile->add_att("lon_ll", junk);

   //
   //  delta_lat
   //

sprintf(junk, "%f degrees", data.delta_lat);

ncfile->add_att("delta_lat", junk);

   //
   //  delta_lon
   //

t = data.delta_lon;

// if ( !west_longitude_positive )  t = -t;

sprintf(junk, "%f degrees", t);

ncfile->add_att("delta_lon", junk);

   //
   //  Nlat
   //

sprintf(junk, "%d grid_points", data.Nlat);

ncfile->add_att("Nlat", junk);

   //
   //  Nlon
   //

sprintf(junk, "%d grid_points", data.Nlon);

ncfile->add_att("Nlon", junk);

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


ncfile->add_att("Projection", "Polar Stereographic");

   //
   //  hemisphere
   //

sprintf(junk, "%c", data.hemisphere);

ncfile->add_att("hemisphere", junk);

   //
   //  scale_lat
   //

sprintf(junk, "%f degrees_north", data.scale_lat);

ncfile->add_att("scale_lat", junk);

   //
   //  lat_pin
   //

sprintf(junk, "%.6f", data.lat_pin);

ncfile->add_att("lat_pin", junk);

   //
   //  lon_pin
   //

t = data.lon_pin;

if ( !west_longitude_positive )  t = -t;

sprintf(junk, "%.6f", t);

ncfile->add_att("lon_pin", junk);

   //
   //  x_pin
   //

sprintf(junk, "%.6f", data.x_pin);

ncfile->add_att("x_pin", junk);

   //
   //  y_pin
   //

sprintf(junk, "%.6f", data.y_pin);

ncfile->add_att("y_pin", junk);

   //
   //  lon_orient
   //

t = data.lon_orient;

if ( !west_longitude_positive )  t = -t;

sprintf(junk, "%.6f", t);

ncfile->add_att("lon_orient", junk);

   //
   //  d_km
   //

sprintf(junk, "%.6f km", data.d_km);

ncfile->add_att("d_km", junk);

   //
   //  r_km
   //

sprintf(junk, "%.6f km", data.r_km);

ncfile->add_att("r_km", junk);

   //
   //  nx
   //

sprintf(junk, "%d", data.nx);

ncfile->add_att("nx", junk);

   //
   //  ny
   //

sprintf(junk, "%d", data.ny);

ncfile->add_att("ny", junk);

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


ncfile->add_att("Projection", "Mercator");

   //
   //  lat_ll
   //

sprintf(junk, "%f degrees_north", data.lat_ll);

ncfile->add_att("lat_ll", junk);

   //
   //  lon_ll
   //

t = data.lon_ll;

if ( !west_longitude_positive )  t = -t;

sprintf(junk, "%f degrees_east", t);

ncfile->add_att("lon_ll", junk);

   //
   //  lat_ur
   //

sprintf(junk, "%f degrees_north", data.lat_ur);

ncfile->add_att("lat_ur", junk);

   //
   //  lon_ur
   //

t = data.lon_ur;

if ( !west_longitude_positive )  t = -t;

sprintf(junk, "%f degrees_east", t);

ncfile->add_att("lon_ur", junk);

   //
   //  nx
   //

sprintf(junk, "%d", data.nx);

ncfile->add_att("nx", junk);

   //
   //  ny
   //

sprintf(junk, "%d", data.ny);

ncfile->add_att("ny", junk);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////



