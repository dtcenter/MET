
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


////////////////////////////////////////////////////////////////////////


static int hdf_start[2];
static int hdf_stride[2];
static int hdf_edge[2];

static float ff[2];


////////////////////////////////////////////////////////////////////////


#include "nint.h"

#include "calipso_5km.h"


////////////////////////////////////////////////////////////////////////


inline float km_to_meters(float km) {

   if ( km == FILL_VALUE )  return ( FILL_VALUE );

   return ( km*1000.f );

}


////////////////////////////////////////////////////////////////////////


static const unixtime jan_1_1993 = mdyhms_to_unix(1, 1, 1993, 0, 0, 0);

static const int  level_index    = 0;   //  use the first cloud level

static const int default_grib_code  = 500;

   //
   //  indices into the 5-element obs_arr record
   //

static const int    hdr_id_index = 0;
static const int grib_code_index = 1;
static const int  pressure_index = 2;
static const int    height_index = 3;
static const int       obs_index = 4;


////////////////////////////////////////////////////////////////////////


static void clear_float_buf(float *);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for struct Calipso_5km_Vars
   //


////////////////////////////////////////////////////////////////////////


Calipso_5km_Vars::Calipso_5km_Vars()

{

sd_id = -1;

// clear();

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Vars::clear()

{

sd_id = -1;

lat.clear();
lon.clear();
time.clear();

top_layer.clear();
top_pressure.clear();

base_layer.clear();
base_pressure.clear();

opacity_flag.clear();

cad_score.clear();

num_layers.clear();

fclass.clear();


      //

return;

};


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Vars::get_vars(const int _hdf_sd_id)

{

clear();

sd_id = _hdf_sd_id;

::get_hdf_var_info(sd_id, hdf_lat_name,   lat);
::get_hdf_var_info(sd_id, hdf_lon_name,   lon);
::get_hdf_var_info(sd_id, hdf_time_name, time);


::get_hdf_var_info(sd_id, hdf_top_layer_name,          top_layer);
::get_hdf_var_info(sd_id, hdf_top_pressure_name,    top_pressure);

::get_hdf_var_info(sd_id, hdf_base_layer_name,        base_layer);
::get_hdf_var_info(sd_id, hdf_base_pressure_name,  base_pressure);

::get_hdf_var_info(sd_id, hdf_opacity_flag_name,    opacity_flag);

::get_hdf_var_info(sd_id, hdf_cad_score_name,          cad_score);

::get_hdf_var_info(sd_id, hdf_num_layers_name,        num_layers);

::get_hdf_var_info(sd_id, hdf_fclass_name,                fclass);


      //

return;

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Vars::get_latlon(int n, float & _lat, float & _lon) const

{

hdf_stride[0] = 1;
hdf_stride[1] = 1;

hdf_edge[0]   = 1;
hdf_edge[1]   = 1;

hdf_start[0]  = n;
hdf_start[1]  = 1;   //  take the middle value

if ( SDreaddata(lat.id, hdf_start, hdf_stride, hdf_edge, ff) < 0 )  {

   mlog << Error
        << "\n\n  Calipso_5km_Vars::get_latlon() -> SDreaddata for latitude failed\n\n";

   exit ( 1 );

}

_lat = ff[0];


if ( SDreaddata(lon.id, hdf_start, hdf_stride, hdf_edge, ff) < 0 )  {

   mlog << Error
        << "\n\n  Calipso_5km_Vars::get_latlon() -> SDreaddata for longitude failed\n\n";

   exit ( 1 );

}

_lon = ff[0];

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


unixtime Calipso_5km_Vars::get_time(int n) const

{

int k;
unixtime t;
double dd;


hdf_stride[0] = 1;
hdf_stride[1] = 1;

hdf_edge[0] = 1;
hdf_edge[1] = 1;

hdf_start[0] = n;
hdf_start[1] = 1;   //  take the middle value


if ( SDreaddata(time.id, hdf_start, hdf_stride, hdf_edge, &dd) < 0 )  {

   mlog << Error
        << "\n\n  Calipso_5km_Vars::get_time() -> SDreaddata failed\n\n";

   exit ( 1 );

}

k = nint(dd);

t = k + jan_1_1993;


   //
   //  done
   //

return ( t );

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Vars::get_obs(int n, Calipso_5km_Obs & obs) const

{

unsigned char u;
signed char c;


obs.clear();

   //
   //  num layers
   //

hdf_stride [0] = 1;
hdf_stride [1] = 1;

hdf_edge   [0] = 1;
hdf_edge   [1] = 1;

hdf_start  [0] = n;
hdf_start  [1] = 0;

if ( SDreaddata(num_layers.id, hdf_start, hdf_stride, hdf_edge, &u) < 0 )  {

   mlog << Error
        << "\n\n  Calipso_5km_Vars::get_obs() -> SDreaddata failed for num layers\n\n";

   exit ( 1 );

}

obs.n_layers = (int) u;

if ( obs.n_layers == 0 )  return;

   //
   //  layer base
   //

hdf_stride[0] = 1;
hdf_stride[1] = 1;

hdf_edge[0]   = 1;
hdf_edge[1]   = 1;

hdf_start[0]  = n;
hdf_start[1]  = level_index;

if ( SDreaddata(base_pressure.id, hdf_start, hdf_stride, hdf_edge, &(obs.base_pressure)) < 0 )  {

   mlog << Error
        << "\n\n  Calipso_5km_Vars::get_obs() -> SDreaddata failed for layer base pressure\n\n";

   exit ( 1 );

}

if ( SDreaddata(base_layer.id, hdf_start, hdf_stride, hdf_edge, &(obs.base_height)) < 0 )  {

   mlog << Error
        << "\n\n  Calipso_5km_Vars::get_obs() -> SDreaddata failed for layer base height\n\n";

   exit ( 1 );

}


   //
   //  layer top
   //

hdf_stride[0] = 1;
hdf_stride[1] = 1;

hdf_edge[0]   = 1;
hdf_edge[1]   = 1;

hdf_start[0]  = n;
hdf_start[1]  = level_index;

if ( SDreaddata(top_pressure.id, hdf_start, hdf_stride, hdf_edge, &(obs.top_pressure)) < 0 )  {

   mlog << Error
        << "\n\n  Calipso_5km_Vars::get_obs() -> SDreaddata failed for layer top pressure\n\n";

   exit ( 1 );

}

if ( SDreaddata(top_layer.id, hdf_start, hdf_stride, hdf_edge, &(obs.top_height)) < 0 )  {

   mlog << Error
        << "\n\n  Calipso_5km_Vars::get_obs() -> SDreaddata failed for layer top height\n\n";

   exit ( 1 );

}


   //
   //  opacity
   //

hdf_stride[0] = 1;
hdf_stride[1] = 1;

hdf_edge[0]   = 1;
hdf_edge[1]   = 1;

hdf_start[0]  = n;
hdf_start[1]  = level_index;

if ( SDreaddata(opacity_flag.id, hdf_start, hdf_stride, hdf_edge, &u) < 0 )  {

   mlog << Error
        << "\n\n  Calipso_5km_Vars::get_obs() -> SDreaddata failed for layer top pressure\n\n";

   exit ( 1 );

}

obs.opacity = (int) u;

   //
   //  cad score
   //

hdf_stride[0] = 1;
hdf_stride[1] = 1;

hdf_edge[0]   = 1;
hdf_edge[1]   = 1;

hdf_start[0]  = n;
hdf_start[1]  = level_index;

if ( SDreaddata(cad_score.id, hdf_start, hdf_stride, hdf_edge, &c) < 0 )  {

   mlog << Error
        << "\n\n  Calipso_5km_Vars::get_obs() -> SDreaddata failed for layer top pressure\n\n";

   exit ( 1 );

}

obs.cad_score = (int) c;

   //
   //  feature classification flags (fclass)
   //

hdf_stride[0] = 1;
hdf_stride[1] = 1;

hdf_edge[0]   = 1;
hdf_edge[1]   = 1;

hdf_start[0]  = n;
hdf_start[1]  = level_index;

if ( SDreaddata(fclass.id, hdf_start, hdf_stride, hdf_edge, &(obs.fclass)) < 0 )  {

   mlog << Error
        << "\n\n  Calipso_5km_Vars::get_obs() -> SDreaddata failed for layer top pressure\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for struct Calipso_5km_Obs
   //


////////////////////////////////////////////////////////////////////////


Calipso_5km_Obs::Calipso_5km_Obs()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::clear()

{

n_layers = 0;

base_height   = FILL_VALUE;
base_pressure = FILL_VALUE;

top_height    = FILL_VALUE;
top_pressure  = FILL_VALUE;

opacity = 0;

cad_score = 0;

fclass = 0;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_layer_base_record(int hdr_id, float * record)

{

clear_float_buf(record);

if ( n_layers == 0 )  return;

record [    hdr_id_index ] = (float) hdr_id;
record [ grib_code_index ] = (float) (default_grib_code + 1);
record [  pressure_index ] = base_pressure;
record [    height_index ] = km_to_meters(base_height);
record [       obs_index ] = km_to_meters(base_height);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_layer_top_record(int hdr_id, float * record)

{

clear_float_buf(record);

if ( n_layers == 0 )  return;

record [    hdr_id_index ] = (float) hdr_id;
record [ grib_code_index ] = (float) default_grib_code;
record [  pressure_index ] = top_pressure;
record [    height_index ] = km_to_meters(top_height);
record [       obs_index ] = km_to_meters(top_height);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_opacity_record(int hdr_id, float * record)

{

clear_float_buf(record);

if ( n_layers == 0 )  return;


record [    hdr_id_index ] = (float) hdr_id;
record [ grib_code_index ] = (float) (default_grib_code + 2);
record [  pressure_index ] = base_pressure;
record [    height_index ] = km_to_meters(base_height);
record [       obs_index ] = (float) opacity;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_cad_score_record(int hdr_id, float * record)

{

clear_float_buf(record);

if ( n_layers == 0 )  return;


record [    hdr_id_index ] = (float) hdr_id;
record [ grib_code_index ] = (float) (default_grib_code + 3);
record [  pressure_index ] = base_pressure;
record [    height_index ] = km_to_meters(base_height);
record [       obs_index ] = (float) cad_score;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_fclass_record(int hdr_id, float * record)

{

clear_float_buf(record);

if ( n_layers == 0 )  return;


record [    hdr_id_index ] = (float) hdr_id;
record [ grib_code_index ] = (float) (default_grib_code + 3);
record [  pressure_index ] = base_pressure;
record [    height_index ] = km_to_meters(base_height);
record [       obs_index ] = (float) fclass;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void clear_float_buf(float * f)

{

for (int j=0; j<5; ++j)  f[j] = FILL_VALUE;

return;

}


////////////////////////////////////////////////////////////////////////



