// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
#include <cmath>

#include "vx_log.h"
#include "grib_utils.h"
#include "angles.h"
#include "is_bad_data.h"


////////////////////////////////////////////////////////////////////////

   //
   //  grid types from the GDS section
   //

static const int latlon_type         = 0;
static const int mercator_type       = 1;
static const int lambert_type        = 3;
static const int stereographic_type  = 5;
static const int gaussian_type       = 4;


////////////////////////////////////////////////////////////////////////


static void gds_to_latlon         (const Section2_Header & gds, LatLonData &);
static void gds_to_rotated_latlon (const Section2_Header & gds, RotatedLatLonData &);
static void gds_to_mercator       (const Section2_Header & gds, MercatorData &);
static void gds_to_lambert        (const Section2_Header & gds, LambertData &);
static void gds_to_stereographic  (const Section2_Header & gds, StereographicData &);
static void gds_to_gaussian       (const Section2_Header & gds, GaussianData &);

static void scan_flag_to_order(const unsigned char scan_flag, int & xdir, int & ydir, int & order);

static double decode_lat_lon(const unsigned char *, int);

static bool all_bits_set(const unsigned char *, int);


////////////////////////////////////////////////////////////////////////


void gds_to_grid(const Section2_Header & gds, Grid & gr)

{

   // Structures to store projection info
LambertData         lc_data;
StereographicData   st_data;
LatLonData          ll_data;
//RotatedLatLonData  rll_data;
MercatorData        mc_data;
GaussianData         g_data;

   //
   // Check GDS for the grid type.
   // The following Projection types are supported:
   //    - Lat/Lon
   //    - Rotated Lat/Lon
   //    - Mercator
   //    - Lambert Conformal
   //    - Polar Stereographic
   //    - Gaussian
   //


if ( gds.type == latlon_type )  {

   gds_to_latlon(gds, ll_data);

   gr.set(ll_data);

// else if ( gds.type == rotated_latlon_type )  {
// 
//    gds_to_rotated_latlon(gds, rll_data);
// 
//    gr.set(rll_data);

} else if ( gds.type == mercator_type )  {

   gds_to_mercator(gds, mc_data);

   gr.set(mc_data);

} else if ( gds.type == lambert_type )  {

   gds_to_lambert(gds, lc_data);

   gr.set(lc_data);

} else if ( gds.type == stereographic_type )  {

   gds_to_stereographic(gds, st_data);

   gr.set(st_data);

} else if ( gds.type == gaussian_type )  {

   gds_to_gaussian(gds, g_data);

   gr.set(g_data);

} else {

   mlog << Error << "\ngds_to_grid() -> "
        << "Grid type " << ((int) (gds.type))
        << " not currently supported.\n\n";
   exit(1);

}

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void gds_to_order(const Section2_Header & gds, int & xdir, int & ydir, int & order)

{

   //
   // Check GDS for the grid type.
   // The following Projection types are supported:
   //    - Lat/Lon
   //    - Mercator
   //    - Lambert Conformal
   //    - Polar Stereographic
   //    - Gaussian
   //


if ( gds.type == latlon_type )  {

   scan_flag_to_order(gds.grid_type.latlon_grid.scan_flag, xdir, ydir, order);

} else if (gds.type == mercator_type )  {

   scan_flag_to_order(gds.grid_type.mercator.scan_flag, xdir, ydir, order);

} else if ( gds.type == lambert_type )  {

   scan_flag_to_order(gds.grid_type.lambert_conf.scan_flag, xdir, ydir, order);

} else if (gds.type == stereographic_type )  {

   scan_flag_to_order(gds.grid_type.stereographic.scan_flag, xdir, ydir, order);

} else if (gds.type == gaussian_type )  {

   scan_flag_to_order(gds.grid_type.gaussian.scan_flag, xdir, ydir, order);

} else {

   mlog << Error << "\ngds_to_order() -> "
        << "Grid type " << ((int) (gds.type))
        << " not currently supported.\n\n";
   exit(1);

}

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void gds_to_latlon(const Section2_Header & gds, LatLonData & data)

{

int xdir, ydir, order;
double d;

scan_flag_to_order(gds.grid_type.latlon_grid.scan_flag, xdir, ydir, order);

   // Store the grid name
data.name = latlon_proj_type;

   //
   // Multiply longitude values by -1 since the NCAR code considers
   // degrees west to be positive.
   //

   // Latitude of the bottom left corner
if ( ydir == 1 )  data.lat_ll = decode_lat_lon(gds.grid_type.latlon_grid.lat1, 3);
else              data.lat_ll = decode_lat_lon(gds.grid_type.latlon_grid.lat2, 3);

   // Longitude of the bottom left corner
if ( xdir == 0 )  data.lon_ll = -1.0*rescale_lon(decode_lat_lon(gds.grid_type.latlon_grid.lon1, 3));
else              data.lon_ll = -1.0*rescale_lon(decode_lat_lon(gds.grid_type.latlon_grid.lon2, 3));

   // Number of points in the Latitudinal (y) direction
data.Nlat = char2_to_int(gds.ny);

   // Number of points in the Longitudinal (x) direction
data.Nlon = char2_to_int(gds.nx);

   // Latitudinal increment.  If not given, compute from lat1 and lat2
if ( all_bits_set(gds.grid_type.latlon_grid.dj, 2) )  {

   d = fabs(decode_lat_lon(gds.grid_type.latlon_grid.lat1, 3)
          - decode_lat_lon(gds.grid_type.latlon_grid.lat2, 3));

   data.delta_lat = d/(data.Nlat);

} else {

   data.delta_lat = decode_lat_lon(gds.grid_type.latlon_grid.dj, 2);

}

   // Longitudinal increment.  If not given, compute from lon1 and lon2
if ( all_bits_set ( gds.grid_type.latlon_grid.di, 2) )  {

   d = fabs(decode_lat_lon(gds.grid_type.latlon_grid.lon1, 3)
          - decode_lat_lon(gds.grid_type.latlon_grid.lon2, 3));

   data.delta_lon = d/(data.Nlon);

} else {

   data.delta_lon = decode_lat_lon(gds.grid_type.latlon_grid.di, 2);

}

data.dump();

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void gds_to_rotated_latlon(const Section2_Header & gds, RotatedLatLonData & data)

{

mlog << Error
     << "gds_to_rotated_latlon() -> Rotated Lat/Lon grids are not supported in Grib Version 1.\n\n";

exit ( 1 );

return;

}


///////////////////////////////////////////////////////////////////////////////


void gds_to_mercator(const Section2_Header & gds, MercatorData & data)

{

int xdir, ydir, order;

scan_flag_to_order(gds.grid_type.mercator.scan_flag, xdir, ydir, order);

   // Store the grid name
data.name = mercator_proj_type;

   //
   // Multiply longitude values by -1 since the NCAR code considers
   // degrees west to be positive.
   //

   // Latitude of the bottom left and upper right corners
if ( ydir == 1 )  {

   data.lat_ll = decode_lat_lon(gds.grid_type.mercator.lat1, 3);
   data.lat_ur = decode_lat_lon(gds.grid_type.mercator.lat2, 3);

} else {

   data.lat_ll = decode_lat_lon(gds.grid_type.mercator.lat2, 3);
   data.lat_ur = decode_lat_lon(gds.grid_type.mercator.lat1, 3);

}

   // Longitude of the bottom left and upper right corners
if ( xdir == 0 )  {

   data.lon_ll = -1.0*rescale_lon(decode_lat_lon(gds.grid_type.mercator.lon1, 3));
   data.lon_ur = -1.0*rescale_lon(decode_lat_lon(gds.grid_type.mercator.lon2, 3));

} else {

   data.lon_ll = -1.0*rescale_lon(decode_lat_lon(gds.grid_type.mercator.lon2, 3));
   data.lon_ur = -1.0*rescale_lon(decode_lat_lon(gds.grid_type.mercator.lon1, 3));

}

   // Number of points in the Latitudinal (y) direction
data.ny = char2_to_int(gds.ny);

   // Number of points in the Longitudinal (x) direction
data.nx = char2_to_int(gds.nx);

data.dump();

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void gds_to_lambert(const Section2_Header & gds, LambertData & data)

{

unsigned char c;

   // Store the grid name
data.name = lambert_proj_type;

   //
   // Multiply longitude values by -1 since the NCAR code considers
   // degrees west to be positive.
   //

   // Hemisphere
c = gds.grid_type.lambert_conf.pc_flag;
if ( c & 128 ) { data.hemisphere = 'S'; }  // South Pole
else           { data.hemisphere = 'N'; }  // North Pole

   // First latitude
data.scale_lat_1 = decode_lat_lon(gds.grid_type.lambert_conf.latin1, 3);

   // Second latitude
data.scale_lat_2 = decode_lat_lon(gds.grid_type.lambert_conf.latin2, 3);

   // Latitude of first point
data.lat_pin = decode_lat_lon(gds.grid_type.lambert_conf.lat1, 3);

   // Longitude of first point
data.lon_pin = -1.0*rescale_lon(decode_lat_lon(gds.grid_type.lambert_conf.lon1, 3));

   // "pin" this point to the lower-left corner of the grid
data.x_pin = 0.0;
data.y_pin = 0.0;

   // Orientation longitude
data.lon_orient = -1.0*rescale_lon(decode_lat_lon(gds.grid_type.lambert_conf.lov, 3));

   // Grid spacing in km (given in the GRIB file in m)
data.d_km = (double) char3_to_int(gds.grid_type.lambert_conf.dx)/1000.0;

   // Check for dx != dy
if(!is_eq((double) char3_to_int(gds.grid_type.lambert_conf.dx)/1000.0,
          (double) char3_to_int(gds.grid_type.lambert_conf.dy)/1000.0)) {
   mlog << Warning << "\ngds_to_lambert() -> "
        << "MET does not currently support Lambert Conformal grids where dx ("
        << (double) char3_to_int(gds.grid_type.lambert_conf.dx)/1000.0 << ") != dy ("
        << (double) char3_to_int(gds.grid_type.lambert_conf.dy)/1000.0
        << ") and may produce unexpected results!\n\n";
}

   // Radius of the earth
data.r_km = grib_earth_radius_km;

   // Number of points in the x-direction
data.nx = char2_to_int(gds.nx);

   // Number of points in the y-direction
data.ny = char2_to_int(gds.ny);

   // Rotation angle
data.so2_angle = 0.0;

data.dump();

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void gds_to_stereographic(const Section2_Header & gds, StereographicData & data)

{

unsigned char c;
int parity;

   // Store the grid name
data.name = stereographic_proj_type;

   //
   // Multiply longitude values by -1 since the NCAR code considers
   // degrees west to be positive.
   //

   // Latitude where the scale factor is determined is 60.0 degrees
   // based on WMO's Guide to Grib
c = gds.grid_type.stereographic.pc_flag;
if ( c & 128 ) { parity = -1; data.hemisphere = 'S'; }  // South Pole
else           { parity =  1; data.hemisphere = 'N'; }  // North Pole
data.scale_lat = (double) 60.0*parity;

   // Latitude of first point
data.lat_pin = decode_lat_lon(gds.grid_type.stereographic.lat1, 3);

   // Longitude of first point
data.lon_pin = -1.0*rescale_lon(decode_lat_lon(gds.grid_type.stereographic.lon1, 3));

   // "pin" this point to the lower-left corner of the grid
data.x_pin = 0.0;
data.y_pin = 0.0;

   // Orientation longitude
data.lon_orient = -1.0*rescale_lon(decode_lat_lon(gds.grid_type.stereographic.lov, 3));

   // Grid spacing in km (given in the GRIB file in m)
data.d_km = (double) char3_to_int(gds.grid_type.stereographic.dx)/1000.0;

   // Check for dx != dy
if(!is_eq((double) char3_to_int(gds.grid_type.stereographic.dx)/1000.0,
          (double) char3_to_int(gds.grid_type.stereographic.dy)/1000.0)) {
   mlog << Warning << "\ngds_to_stereographic() -> "
        << "MET does not currently support Polar Stereographic grids where dx ("
        << (double) char3_to_int(gds.grid_type.stereographic.dx)/1000.0 << ") != dy ("
        << (double) char3_to_int(gds.grid_type.stereographic.dy)/1000.0
        << ") and may produce unexpected results!\n\n";
}

   // Radius of the earth
data.r_km = grib_earth_radius_km;

   // Number of points in the x-direction
data.nx = char2_to_int(gds.nx);

   // Number of points in the y-direction
data.ny = char2_to_int(gds.ny);

data.dump();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void gds_to_gaussian(const Section2_Header & gds, GaussianData & data)

{

data.name = gaussian_proj_type;

   //
   //  nx, ny
   //

data.nx = char2_to_int(gds.nx);
data.ny = char2_to_int(gds.ny);

   //
   //  lon_zero
   //

data.lon_zero = -1.0*rescale_lon(decode_lat_lon(gds.grid_type.gaussian.lon1, 3));

data.dump();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void scan_flag_to_order(const unsigned char scan_flag, int & xdir, int & ydir, int & order)

{

if ( scan_flag & 128) xdir = 1; // In the -x direction
else                  xdir = 0; // In the +x direction

if ( scan_flag & 64)  ydir = 1; // In the +y direction
else                  ydir = 0; // In the -y direction

if ( scan_flag & 32) order = 1; // Data in (x, y) order
else                 order = 0; // Data in (y, x) order

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


double decode_lat_lon(const unsigned char * p, int n)

{

int i, parity;
double answer;
unsigned char c[3];

   //
   //  For all of the lat/lon parameters, the leftmost bit indicates the
   //  parity of the value.  If the leftmost bit is on, the value is
   //  negative.
   //

for(i=0; i<n; i++) c[i] = p[i];

parity = 1;

if ( c[0] & 128 ) parity = -1;

c[0] &= 127;

     if ( n == 2 )  answer = (double) 0.001*parity*char2_to_int(c);
else if ( n == 3 )  answer = (double) 0.001*parity*char3_to_int(c);
else                answer = (double) bad_data_float;

return ( answer );

}


////////////////////////////////////////////////////////////////////////


bool all_bits_set(const unsigned char * p, int n)

{

int i;

   //
   // Check whether or not all the bits in the unsigned char array are set to 1
   //

for (i=0; i<n; i++) {

   if ( p[i] != 0xff )  return ( false );

}

return ( true );

}


////////////////////////////////////////////////////////////////////////


void instantiate_grid(GribFile & f, int rec_num, Grid & out)

{

GribRecord r;

f.seek_record(rec_num);

f >> r;

gds_to_grid( *(r.gds), out);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


