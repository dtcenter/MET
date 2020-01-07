
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


////////////////////////////////////////////////////////////////////////


static int hdf_start  [2];
static int hdf_stride [2];
static int hdf_edge   [2];

static float ff[2];


////////////////////////////////////////////////////////////////////////


#include "nint.h"
#include "write_netcdf.h"

#include "calipso_5km.h"


////////////////////////////////////////////////////////////////////////


inline float km_to_meters(float km) {

   if ( km == FILL_VALUE )  return ( FILL_VALUE );

   return ( km*1000.f );

}


////////////////////////////////////////////////////////////////////////


   //
   //  bit manipulation
   //

   //
   //  bits numbered from 1 to 16 inclusive.
   //

static const unsigned short mask_3 = (unsigned short) 7;   //  2^3 - 1
static const unsigned short mask_2 = (unsigned short) 3;   //  2^2 - 1
static const unsigned short mask_1 = (unsigned short) 1;   //  2^1 - 1

   //
   //  mask sizes don't depend on bit order
   //

static const unsigned short type_mask               = mask_3;
static const unsigned short type_qa_mask            = mask_2;
static const unsigned short ice_water_mask          = mask_2;
static const unsigned short ice_water_qa_mask       = mask_2;
static const unsigned short subtype_mask            = mask_3;
static const unsigned short cloud_aerosol_qa_mask   = mask_1;
static const unsigned short h_average_mask          = mask_3;

   //
   //  shifts for bit #1 = highest order bit
   //

/*
static const int            type_shift              = 13;
static const int            type_qa_shift           = 11;
static const int            ice_water_shift         =  9;
static const int            ice_water_qa_shift      =  7;
static const int            subtype_shift           =  4;
static const int            cloud_aerosol_qa_shift  =  3;
static const int            h_average_shift         =  0;
*/

   //
   //  shifts for bit #1 = lowest order bit
   //

static const int            type_shift              =  0;
static const int            type_qa_shift           =  3;
static const int            ice_water_shift         =  5;
static const int            ice_water_qa_shift      =  7;
static const int            subtype_shift           =  9;
static const int            cloud_aerosol_qa_shift  = 12;
static const int            h_average_shift         = 13;


////////////////////////////////////////////////////////////////////////


   //
   //  masking 3 high bits
   //
/*
inline unsigned short fclass_mask(unsigned short u)

{

return ( u >> 13 );

}
*/

   //
   //  masking 3 low bits
   //

inline unsigned short fclass_mask(unsigned short u)

{

return ( u & 7 );

}


////////////////////////////////////////////////////////////////////////


static const unixtime jan_1_1993 = mdyhms_to_unix(1, 1, 1993, 0, 0, 0);


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

static int extract_bits(unsigned short u, const unsigned short mask, const int shift);


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

}


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

int j;
unsigned char u [hdf_max_layers];
signed char   c [hdf_max_layers];


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

obs.n_layers = (int) (u[0]);

if ( obs.n_layers == 0 )  return;

   //
   //  layer base
   //

hdf_stride[0] = 1;
hdf_stride[1] = 1;

hdf_edge[0]   = 1;
hdf_edge[1]   = hdf_max_layers;

hdf_start[0]  = n;
hdf_start[1]  = 0;

if ( SDreaddata(base_pressure.id, hdf_start, hdf_stride, hdf_edge, &(obs.base_pressure)) < 0 )  {

   mlog << Error
        << "\n\n  Calipso_5km_Vars::get_obs() -> SDreaddata failed for layer base pressure\n\n";

   exit ( 1 );

}

if ( SDreaddata(base_layer.id, hdf_start, hdf_stride, hdf_edge, &(obs.base_height_km)) < 0 )  {

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
hdf_edge[1]   = hdf_max_layers;

hdf_start[0]  = n;
hdf_start[1]  = 0;

if ( SDreaddata(top_pressure.id, hdf_start, hdf_stride, hdf_edge, &(obs.top_pressure)) < 0 )  {

   mlog << Error
        << "\n\n  Calipso_5km_Vars::get_obs() -> SDreaddata failed for layer top pressure\n\n";

   exit ( 1 );

}

if ( SDreaddata(top_layer.id, hdf_start, hdf_stride, hdf_edge, &(obs.top_height_km)) < 0 )  {

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
hdf_edge[1]   = hdf_max_layers;

hdf_start[0]  = n;
hdf_start[1]  = 0;

if ( SDreaddata(opacity_flag.id, hdf_start, hdf_stride, hdf_edge, &u) < 0 )  {

   mlog << Error
        << "\n\n  Calipso_5km_Vars::get_obs() -> SDreaddata failed for layer top pressure\n\n";

   exit ( 1 );

}

for (j=0; j<hdf_max_layers; ++j)  {

   obs.opacity[j] = (int) (u[j]);

}

   //
   //  cad score
   //

hdf_stride[0] = 1;
hdf_stride[1] = 1;

hdf_edge[0]   = 1;
hdf_edge[1]   = hdf_max_layers;

hdf_start[0]  = n;
hdf_start[1]  = 0;

if ( SDreaddata(cad_score.id, hdf_start, hdf_stride, hdf_edge, &c) < 0 )  {

   mlog << Error
        << "\n\n  Calipso_5km_Vars::get_obs() -> SDreaddata failed for layer top pressure\n\n";

   exit ( 1 );

}

for (j=0; j<hdf_max_layers; ++j)  {

   obs.cad_score[j] = (int) (c[j]);

}

   //
   //  feature classification flags (fclass)
   //

hdf_stride[0] = 1;
hdf_stride[1] = 1;

hdf_edge[0]   = 1;
hdf_edge[1]   = hdf_max_layers;

hdf_start[0]  = n;
hdf_start[1]  = 0;

if ( SDreaddata(fclass.id, hdf_start, hdf_stride, hdf_edge, &(obs.fclass)) < 0 )  {

   mlog << Error
        << "\n\n  Calipso_5km_Vars::get_obs() -> SDreaddata failed for layer top pressure\n\n";

   exit ( 1 );

}

// for (j=0; j<hdf_max_layers; ++j)  {
//
//    // obs.fclass[j] = fclass_mask(obs.fclass[j]);
//
// }

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

int j;

for (j=0; j<hdf_max_layers; ++j)  {

   base_height_km [j] = FILL_VALUE;
   base_pressure  [j] = FILL_VALUE;

   top_height_km  [j] = FILL_VALUE;
   top_pressure   [j] = FILL_VALUE;

   opacity        [j] = 0;

   cad_score      [j] = 0;

   fclass         [j] = 0;

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_n_layers_record(int hdr_id, float * record)

{

clear_float_buf(record);

record [    hdr_id_index ] = (float) hdr_id;
record [ grib_code_index ] = (float) n_layers_grib_code;
record [  pressure_index ] = FILL_VALUE;
record [    height_index ] = (float) 0;
record [       obs_index ] = (float) n_layers;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_layer_base_record(int hdr_id, int layer, float * record)

{

clear_float_buf(record);

record [    hdr_id_index ] = (float) hdr_id;
record [ grib_code_index ] = (float) layer_base_grib_code;
record [  pressure_index ] = base_pressure[layer];
record [    height_index ] = km_to_meters(base_height_km[layer]);
record [       obs_index ] = km_to_meters(base_height_km[layer]);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_layer_top_record(int hdr_id, int layer, float * record)

{

clear_float_buf(record);

record [    hdr_id_index ] = (float) hdr_id;
record [ grib_code_index ] = (float) layer_top_grib_code;
record [  pressure_index ] = top_pressure[layer];
record [    height_index ] = km_to_meters(top_height_km[layer]);
record [       obs_index ] = km_to_meters(top_height_km[layer]);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_opacity_record(int hdr_id, int layer, float * record)

{

clear_float_buf(record);

if ( n_layers == 0 )  return;


record [    hdr_id_index ] = (float) hdr_id;
record [ grib_code_index ] = (float) opacity_grib_code;
record [  pressure_index ] = base_pressure[layer];
record [    height_index ] = km_to_meters(base_height_km[layer]);
record [       obs_index ] = (float) (opacity[layer]);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_cad_score_record(int hdr_id, int layer, float * record)

{

clear_float_buf(record);

if ( n_layers == 0 )  return;


record [    hdr_id_index ] = (float) hdr_id;
record [ grib_code_index ] = (float) cad_score_grib_code;
record [  pressure_index ] = base_pressure[layer];
record [    height_index ] = km_to_meters(base_height_km[layer]);
record [       obs_index ] = (float) (cad_score[layer]);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_base_base_record(int hdr_id, float * record)

{

clear_float_buf(record);

if ( n_layers == 0 )  return;

// const int layer = 0;
const int layer = n_layers - 1;

Calipso_5km_Obs::get_layer_base_record(hdr_id, layer, record);

record [ grib_code_index ] = (float) base_base_grib_code;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_top_top_record(int hdr_id, float * record)

{

clear_float_buf(record);

if ( n_layers == 0 )  return;

// const int layer = n_layers - 1;
const int layer = 0;

Calipso_5km_Obs::get_layer_top_record(hdr_id, layer, record);

record [ grib_code_index ] = (float) top_top_grib_code;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::fclass_record_header(int hdr_id, int layer, int grib_code, float * record)

{

clear_float_buf(record);

if ( n_layers == 0 )  return;

record [    hdr_id_index ] = (float) hdr_id;
record [ grib_code_index ] = (float) grib_code;
record [  pressure_index ] = base_pressure[layer];
record [    height_index ] = km_to_meters(base_height_km[layer]);

return;

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_feature_type_record(int hdr_id, int layer, float * record)

{

if ( n_layers == 0 )  return;

fclass_record_header(hdr_id, layer, ftype_grib_code, record);

record [ obs_index ] = (float) extract_bits(fclass[layer], type_mask, type_shift);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////

/*
void Calipso_5km_Obs::get_feature_type_qa_record(int hdr_id, int layer, float * record)

{

if ( n_layers == 0 )  return;

fclass_record_header(hdr_id, layer, ftype_qa_grib_code, record);

record [ obs_index ] = (float) extract_bits(fclass[layer], type_qa_mask, type_qa_shift);

   //
   //  done
   //

return;

}
*/

////////////////////////////////////////////////////////////////////////


int Calipso_5km_Obs::get_feature_type_qa_value(int layer)

{

if ( n_layers == 0 )  return ( -1 );

int k;

k = extract_bits(fclass[layer], type_qa_mask, type_qa_shift);

return ( k );

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_ice_water_record(int hdr_id, int layer, float * record)

{

if ( n_layers == 0 )  return;

fclass_record_header(hdr_id, layer, ice_water_grib_code, record);

record [ obs_index ] = (float) extract_bits(fclass[layer], ice_water_mask, ice_water_shift);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////

/*
void Calipso_5km_Obs::get_ice_water_qa_record(int hdr_id, int layer, float * record)

{

if ( n_layers == 0 )  return;

fclass_record_header(hdr_id, layer, ice_water_qa_grib_code, record);

record [ obs_index ] = (float) extract_bits(fclass[layer], ice_water_qa_mask, ice_water_qa_shift);

   //
   //  done
   //

return;

}
*/

////////////////////////////////////////////////////////////////////////


int Calipso_5km_Obs::get_ice_water_qa_value(int layer)

{

if ( n_layers == 0 )  return ( -1 );

int k;

k = extract_bits(fclass[layer], ice_water_qa_mask, ice_water_qa_shift);

return ( k );

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_subtype_record(int hdr_id, int layer, float * record)

{

if ( n_layers == 0 )  return;

fclass_record_header(hdr_id, layer, subtype_grib_code, record);

record [ obs_index ] = (float) extract_bits(fclass[layer], subtype_mask, subtype_shift);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_cloud_aerosol_qa_record(int hdr_id, int layer, float * record)

{

if ( n_layers == 0 )  return;

fclass_record_header(hdr_id, layer, cloud_aerosol_qa_grib_code, record);

record [ obs_index ] = (float) get_cloud_aerosol_qa_value(layer);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int Calipso_5km_Obs::get_cloud_aerosol_qa_value(int layer)

{

if ( n_layers == 0 )  return ( -1 );

int k;

k = extract_bits(fclass[layer], cloud_aerosol_qa_mask, cloud_aerosol_qa_shift);

return ( k );

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_Obs::get_h_average_record(int hdr_id, int layer, float * record)

{

if ( n_layers == 0 )  return;

fclass_record_header(hdr_id, layer, h_average_grib_code, record);

record [ obs_index ] = (float) extract_bits(fclass[layer], h_average_mask, h_average_shift);

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


int extract_bits(unsigned short u, const unsigned short mask, const int shift)


{

if ( shift > 0 )  u >>= shift;

u &= mask;


   //
   //  done
   //

return ( (int) u );

}


////////////////////////////////////////////////////////////////////////



