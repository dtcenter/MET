// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#include <string.h>


#include "vx_log.h"
#include "vx_python_utils.h"

#include "grid_from_python_dict.h"


////////////////////////////////////////////////////////////////////////

   //
   //  taken from src/libcode/vx_nc_util/grid_output.cc
   //

static const char             lc_string [] = "Lambert Conformal";
static const char             st_string [] = "Polar Stereographic";
static const char           merc_string [] = "Mercator";
static const char         latlon_string [] = "LatLon";
static const char rotated_latlon_string [] = "Rotated LatLon";


////////////////////////////////////////////////////////////////////////


static void get_lc_grid             (PyObject * dict, Grid & g);
static void get_st_grid             (PyObject * dict, Grid & g);
static void get_merc_grid           (PyObject * dict, Grid & g);
static void get_latlon_grid         (PyObject * dict, Grid & g);
static void get_rotated_latlon_grid (PyObject * dict, Grid & g);

static void set_string(const char * & dest, const ConcatString & src);


////////////////////////////////////////////////////////////////////////


inline void toggle_sign(double & x) { x = -x;  return; }


////////////////////////////////////////////////////////////////////////


void grid_from_python_dict(PyObject * dict, Grid & g)

{

ConcatString proj_type;

g.clear();

   //
   //  get projection type
   //

proj_type = dict_lookup_string(dict, "type");

     if ( proj_type ==             lc_string )  get_lc_grid             (dict, g);
else if ( proj_type ==             st_string )  get_st_grid             (dict, g);
else if ( proj_type ==           merc_string )  get_merc_grid           (dict, g);
else if ( proj_type ==         latlon_string )  get_latlon_grid         (dict, g);
else if ( proj_type == rotated_latlon_string )  get_rotated_latlon_grid (dict, g);
else {

   mlog << Error
        << "\ngrid_from_python_dict() -> bad projection type: \""
        << proj_type << "\"\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////

   //
   //  name                            (string)
   //
   //  hemisphere                      (string: "N" or "S")
   //
   //  scale_lat_1, scale_lat_2        (double)
   //
   //  lat_pin, lon_pin, x_pin, y_pin  (double)
   //
   //  lon_orient                      (double)
   //
   //  d_km, r_km                      (double)
   //
   //  nx, ny                          (int)
   //

void get_lc_grid     (PyObject * dict, Grid & g)

{

LambertData data;
ConcatString s;

s = dict_lookup_string(dict, "name");

set_string(data.name, s);

s = dict_lookup_string(dict, "hemisphere");

data.hemisphere = s[0];

data.scale_lat_1 = dict_lookup_double(dict, "scale_lat_1");
data.scale_lat_2 = dict_lookup_double(dict, "scale_lat_2");

data.lat_pin = dict_lookup_double(dict, "lat_pin");
data.lon_pin = rescale_lon(dict_lookup_double(dict, "lon_pin"));

data.x_pin = dict_lookup_double(dict, "x_pin");
data.y_pin = dict_lookup_double(dict, "y_pin");

data.lon_orient = rescale_lon(dict_lookup_double(dict, "lon_orient"));

data.d_km = dict_lookup_double(dict, "d_km");
data.r_km = dict_lookup_double(dict, "r_km");

data.nx = dict_lookup_int(dict, "nx");
data.ny = dict_lookup_int(dict, "ny");

data.so2_angle = 0.0;

   ////////////////

if ( ! west_longitude_positive )  {

   toggle_sign(data.lon_orient);
   toggle_sign(data.lon_pin);

}

   //
   //  done
   //

g.set(data);

if ( data.name )  { delete [] data.name;  data.name = (const char *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////

   //
   //  name                             (string)
   //
   //  hemisphere                       (string: "N" or "S")
   //
   //  scale_lat                        (double)
   //
   //  lat_pin, lon_pin, x_pin, y_pin   (double)
   //
   //  lon_orient                       (double)
   //
   //  d_km, r_km                       (double)
   //
   //  nx, ny                           (int)
   //


void get_st_grid     (PyObject * dict, Grid & g)

{

StereographicData data;
ConcatString s;

s = dict_lookup_string(dict, "name");

set_string(data.name, s);

s = dict_lookup_string(dict, "hemisphere");

data.hemisphere = s[0];

data.scale_lat = dict_lookup_double(dict, "scale_lat");

data.lat_pin = dict_lookup_double(dict, "lat_pin");
data.lon_pin = rescale_lon(dict_lookup_double(dict, "lon_pin"));

data.x_pin = dict_lookup_double(dict, "x_pin");
data.y_pin = dict_lookup_double(dict, "y_pin");

data.lon_orient = rescale_lon(dict_lookup_double(dict, "lon_orient"));

data.d_km = dict_lookup_double(dict, "d_km");
data.r_km = dict_lookup_double(dict, "r_km");

data.nx = dict_lookup_int(dict, "nx");
data.ny = dict_lookup_int(dict, "ny");


   ////////////////

if ( ! west_longitude_positive )  {

   toggle_sign(data.lon_orient);
   toggle_sign(data.lon_pin);

}

   //
   //  done
   //

g.set(data);

if ( data.name )  { delete [] data.name;  data.name = (const char *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////

   //
   //  name        (string)
   //
   //  lat_ll      (double)
   //
   //  lon_ll      (double)
   //
   //  lat_ur      (double)
   //
   //  lon_ur      (double)
   //
   //  nx, ny      (int)
   //

void get_merc_grid   (PyObject * dict, Grid & g)

{

MercatorData data;
ConcatString s;

s = dict_lookup_string(dict, "name");

set_string(data.name, s);

data.lat_ll = dict_lookup_double(dict, "lat_ll");
data.lon_ll = rescale_lon(dict_lookup_double(dict, "lon_ll"));

data.lat_ur = dict_lookup_double(dict, "lat_ur");
data.lon_ur = rescale_lon(dict_lookup_double(dict, "lon_ur"));

   ////////////////

if ( ! west_longitude_positive )  {

   toggle_sign(data.lon_ll);
   toggle_sign(data.lon_ur);

}

data.nx = dict_lookup_int(dict, "nx");
data.ny = dict_lookup_int(dict, "ny");

   //
   //  done
   //

g.set(data);

if ( data.name )  { delete [] data.name;  data.name = (const char *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////

   //
   // name                    (string)
   //
   // lat_ll, lon_ll          (double)
   //
   // delta_lat, delta_lon    (double)
   //
   // Nlat, Nlon              (int)
   //

void get_latlon_grid (PyObject * dict, Grid & g)

{

LatLonData data;
ConcatString s;

s = dict_lookup_string(dict, "name");

set_string(data.name, s);

data.lat_ll = dict_lookup_double(dict, "lat_ll");
data.lon_ll = rescale_lon(dict_lookup_double(dict, "lon_ll"));

data.delta_lat = dict_lookup_double(dict, "delta_lat");
data.delta_lon = dict_lookup_double(dict, "delta_lon");

data.Nlat = dict_lookup_int(dict, "Nlat");
data.Nlon = dict_lookup_int(dict, "Nlon");

if ( ! west_longitude_positive )  {

   toggle_sign(data.lon_ll);

}

   //
   //  done
   //

g.set(data);

if ( data.name )  { delete [] data.name;  data.name = (const char *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////

   //
   // name                                      (string)
   //
   // rot_lat_ll, rot_lon_ll                    (double)
   //
   // delta_rot_lat, delta_rot_lon              (double)
   //
   // Nlat, Nlon                                (int)
   //
   // true_lat_south_pole, true_lon_south_pole  (double)
   //
   // aux_rotation                              (double)
   //

void get_rotated_latlon_grid (PyObject * dict, Grid & g)

{

RotatedLatLonData data;
ConcatString s;

s = dict_lookup_string(dict, "name");

set_string(data.name, s);

data.rot_lat_ll = dict_lookup_double(dict, "rot_lat_ll");
data.rot_lon_ll = rescale_lon(dict_lookup_double(dict, "rot_lon_ll"));

data.delta_rot_lat = dict_lookup_double(dict, "delta_rot_lat");
data.delta_rot_lon = dict_lookup_double(dict, "delta_rot_lon");

data.Nlat = dict_lookup_int(dict, "Nlat");
data.Nlon = dict_lookup_int(dict, "Nlon");

data.true_lat_south_pole = dict_lookup_double(dict, "true_lat_south_pole");
data.true_lon_south_pole = rescale_lon(dict_lookup_double(dict, "true_lon_south_pole"));

data.aux_rotation = dict_lookup_double(dict, "aux_rotation");

if ( ! west_longitude_positive )  {

   toggle_sign(data.rot_lon_ll);
   toggle_sign(data.true_lon_south_pole);

}

   //
   //  done
   //

g.set(data);

if ( data.name )  { delete [] data.name;  data.name = (const char *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////

   //
   //  the fact that our destination is "const char *" rather than
   //
   //    just "char *" is the only thing that makes this tricky
   //

void set_string(const char * & dest, const ConcatString & src)

{

char * s = 0;
const int L = src.length();

s = new char [1 + L];

memcpy(s, src.text(), L);

s[L] = (char) 0;

dest = s;

return;

}


////////////////////////////////////////////////////////////////////////



