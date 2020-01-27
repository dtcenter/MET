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
#include <cstdio>
#include <cmath>

#include "nc_utils_local.h"
#include "nc_grid.h"


////////////////////////////////////////////////////////////////////////


static bool read_nc_st_grid     (NcFile &, Grid &);
static bool read_nc_lc_grid     (NcFile &, Grid &);
static bool read_nc_latlon_grid (NcFile &, Grid &);

static void write_nc_st_grid     (NcFile &, const StereographicData &);
static void write_nc_lc_grid     (NcFile &, const LambertData &);
static void write_nc_latlon_grid (NcFile &, const LatLonData &);


////////////////////////////////////////////////////////////////////////


bool read_nc_grid(NcFile & f, Grid & g)

{

   bool status = false;
   ConcatString proj;
   //const ConcatString proj = string_att(f, "Projection");
   get_att_value_string(&f, (string)"Projection", proj);
   
   
   g.clear();
   
   
   if ( proj == "Polar Stereographic" )  {
   
      status = read_nc_st_grid(f, g);
   
   } else if ( proj == "Lambert Conformal" )  {
   
      status = read_nc_lc_grid(f, g);
   
   } else if ( proj == "LatLon" )  {
   
      status = read_nc_latlon_grid(f, g);
   
   } else {
   
      mlog << Error << "\n\n  read_nc_grid() -> haven't written code to parse \"" << proj << "\" grids yet!\n\n";
   
      return ( false );
   
   }


   //
   //  done
   //

return ( status );

}


////////////////////////////////////////////////////////////////////////


bool write_nc_grid(NcFile & f, const Grid & g)

{

GridInfo info = g.info();

if ( !(info.ok()) )  {

   mlog << Error << "\n\n  write_nc_grid(NcFile &, const Grid &) -> can't get information from grid!\n\n";

   exit ( 1 );

}

     if ( info.st )  write_nc_st_grid     (f, *(info.st));
else if ( info.lc )  write_nc_lc_grid     (f, *(info.lc));
else if ( info.ll )  write_nc_latlon_grid (f, *(info.ll));
else {

   mlog << Error << "\n\n  bool write_nc_grid(NcFile &, const Grid &) -> unsupported projection type\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool read_nc_st_grid(NcFile & f, Grid & g)

{

   StereographicData data;
   ConcatString c;

   //
   //  name
   //

   data.name = "Unknown stereographic";

   //
   //  hemisphere
   //

   //c = string_att(f, "hemisphere");
   get_att_value_string(&f, (string)"hemisphere", c);

data.hemisphere = c[0];

   //
   //  scale latitude
   //

data.scale_lat = string_att_as_double(f, "scale_lat");

   //
   //  lat/lon pin
   //

data.lat_pin = string_att_as_double(f, "lat_pin");
data.lon_pin = string_att_as_double(f, "lon_pin");

data.lon_pin *= -1.0;

   //
   //  x/y pin
   //

data.x_pin = string_att_as_double(f, "x_pin");
data.y_pin = string_att_as_double(f, "y_pin");

   //
   //  orientation longitude
   //

data.lon_orient = string_att_as_double(f, "lon_orient");

data.lon_orient *= -1.0;

   //
   //  D, R
   //

data.d_km = string_att_as_double(f, "d_km");
data.r_km = string_att_as_double(f, "r_km");

   //
   //  Nx, Ny
   //

data.nx = string_att_as_int(f, "nx");
data.ny = string_att_as_int(f, "ny");


   //
   //  done
   //

g.set(data);

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool read_nc_lc_grid(NcFile & f, Grid & g)

{

LambertData data;
g.clear();

   //
   //  name
   //

data.name = "Unknown lambert";

   //
   //  scale latitudes
   //

data.scale_lat_1 = string_att_as_double(f, "scale_lat_1");
data.scale_lat_2 = string_att_as_double(f, "scale_lat_2");

   //
   //  lat/lon pin
   //

data.lat_pin = string_att_as_double(f, "lat_pin");
data.lon_pin = string_att_as_double(f, "lon_pin");

data.lon_pin *= -1.0;

   //
   //  x/y pin
   //

data.x_pin = string_att_as_double(f, "x_pin");
data.y_pin = string_att_as_double(f, "y_pin");

   //
   //  orientation longitude
   //

data.lon_orient = string_att_as_double(f, "lon_orient");

data.lon_orient *= -1.0;

   //
   //  D, R
   //

data.d_km = string_att_as_double(f, "d_km");
data.r_km = string_att_as_double(f, "r_km");

   //
   //  Nx, Ny
   //

data.nx = string_att_as_int(f, "nx");
data.ny = string_att_as_int(f, "ny");

   //
   //  Rotation angle
   //

data.so2_angle = 0.0;

   //
   //  done
   //

g.set(data);

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool read_nc_latlon_grid(NcFile & f, Grid & g)

{

LatLonData data;

g.clear();

   //
   //  name
   //

data.name = "Unknown latlon";

   //
   //  lower-left lat/lon
   //

data.lat_ll = string_att_as_double(f, "lat_ll");
data.lon_ll = string_att_as_double(f, "lon_ll");

data.lon_ll = -(data.lon_ll);

   //
   //  lat/lon deltas
   //

data.delta_lat = string_att_as_double(f, "delta_lat");
data.delta_lon = string_att_as_double(f, "delta_lon");

   //
   //  grid size
   //

data.Nlat = string_att_as_int(f, "Nlat");
data.Nlon = string_att_as_int(f, "Nlon");


   //
   //  done
   //

g.set(data);

return ( true );

}


////////////////////////////////////////////////////////////////////////


void write_nc_st_grid(NcFile & f, const StereographicData & data)

{

ConcatString junk;
ConcatString j2;

   //
   //  name
   //

add_att(&f, "Projection", "Polar Stereographic");

   //
   //  hemisphere
   //

junk = data.hemisphere;

add_att(&f, "hemisphere", junk);

   //
   //  scale latitude
   //

 j2.format("%.5f", data.scale_lat);

fix_float(j2);

 junk.format("%s degrees_north", j2.c_str());

add_att(&f, "scale_lat", junk);

   //
   //  lat/lon pin point
   //

junk.format("%.5f", data.lat_pin);

fix_float(junk);

add_att(&f, "lat_pin", junk);


 junk.format("%.5f", -(data.lon_pin));

fix_float(junk);

add_att(&f, "lon_pin", junk);

   //
   //  x/y pin point
   //

 junk.format("%.5f", data.x_pin);

fix_float(junk);

add_att(&f, "x_pin", junk);


 junk.format("%.5f", data.y_pin);

fix_float(junk);

add_att(&f, "y_pin", junk);

   //
   //  orientation longitude
   //

 junk.format("%.5f", -(data.lon_orient));

fix_float(junk);

add_att(&f, "lon_orient", junk);

   //
   //  D and R
   //

 j2.format("%.5f", data.d_km);

fix_float(j2);

 junk.format("%s km", j2.c_str());

add_att(&f, "d_km", junk);


 j2.format("%.5f", data.r_km);

fix_float(j2);

 junk.format("%s km", j2.c_str());

add_att(&f, "r_km", junk);

   //
   //  nx and ny
   //

 junk.format("%d", data.nx);

add_att(&f, "nx", junk);


 junk.format("%d", data.ny);

add_att(&f, "ny", junk);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void write_nc_lc_grid(NcFile & f, const LambertData & data)

{

ConcatString junk;
ConcatString j2;

   //
   //  name
   //

add_att(&f, "Projection", "Lambert Conformal");

   //
   //  scale latitudes
   //

 junk.format("%.5f", data.scale_lat_1);

fix_float(junk);

add_att(&f, "scale_lat_1", junk);


 junk.format("%.5f", data.scale_lat_2);

fix_float(junk);

add_att(&f, "scale_lat_2", junk);

   //
   //  lat/lon pin point
   //

 junk.format("%.5f", data.lat_pin);

fix_float(junk);

add_att(&f, "lat_pin", junk);


 junk.format("%.5f", -(data.lon_pin));

fix_float(junk);

add_att(&f, "lon_pin", junk);

   //
   //  x/y pin point
   //

 junk.format("%.5f", data.x_pin);

fix_float(junk);

add_att(&f, "x_pin", junk);


 junk.format("%.5f", data.y_pin);

fix_float(junk);

add_att(&f, "y_pin", junk);

   //
   //  orientation longitude
   //

 junk.format("%.5f", -(data.lon_orient));

fix_float(junk);

add_att(&f, "lon_orient", junk);

   //
   //  D and R
   //

 j2.format("%.5f", data.d_km);

fix_float(j2);

 junk.format("%s km", j2.c_str());

add_att(&f, "d_km", junk);


 j2.format("%.5f", data.r_km);

fix_float(j2);

 junk.format("%s km", j2.c_str());

add_att(&f, "r_km", junk);

   //
   //  nx and ny
   //

 junk.format("%d", data.nx);

add_att(&f, "nx", junk);


 junk.format("%d", data.ny);

add_att(&f, "ny", junk);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void write_nc_latlon_grid (NcFile & f, const LatLonData & data)

{

ConcatString junk;


   //
   //  name
   //

add_att(&f, "Projection", "LatLon");

   //
   //  lower left point
   //

 junk.format("%.5f", data.lat_ll);

fix_float(junk);

add_att(&f, "lat_ll", junk);


 junk.format("%.5f", -(data.lon_ll));

fix_float(junk);

add_att(&f, "lon_ll", junk);

   //
   //  lat/lon deltas
   //

 junk.format("%.5f", data.delta_lat);

fix_float(junk);

add_att(&f, "delta_lat", junk);


 junk.format("%.5f", data.delta_lon);

fix_float(junk);

add_att(&f, "delta_lon", junk);

   //
   //  grid size
   //

 junk.format("%d", data.Nlat);

fix_float(junk);

add_att(&f, "Nlat", junk);


 junk.format("%d", data.Nlon);

fix_float(junk);

add_att(&f, "Nlon", junk);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


