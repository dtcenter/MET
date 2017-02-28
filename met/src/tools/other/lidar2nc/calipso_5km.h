
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __CALIPSO_5KM_DATA__
#define  __CALIPSO_5KM_DATA__


////////////////////////////////////////////////////////////////////////


#include <netcdf>

#include "concat_string.h"
#include "vx_cal.h"

#include "hdf.h"
#include "mfhdf.h"

#include "hdf_utils.h"


////////////////////////////////////////////////////////////////////////


static const float FILL_VALUE = -9999.f;


////////////////////////////////////////////////////////////////////////


static const int layer_base_grib_code    = 500;
static const int layer_top_grib_code     = 501;
static const int opacity_grib_code       = 502;
static const int cad_score_grib_code     = 503;
static const int fclass_grib_code        = 504;


////////////////////////////////////////////////////////////////////////


static const int hdf_max_layers = 10;


////////////////////////////////////////////////////////////////////////


struct Calipso_5km_Obs  {


   int n_layers;


   float base_height_km   [hdf_max_layers];
   float base_pressure    [hdf_max_layers];

   float top_height_km    [hdf_max_layers];
   float top_pressure     [hdf_max_layers];

   int opacity            [hdf_max_layers];

   int cad_score          [hdf_max_layers];

   unsigned short fclass  [hdf_max_layers];

      //

   Calipso_5km_Obs();

   void clear();

   void get_layer_top_record  (int hdr_id, int layer, float *);
   void get_layer_base_record (int hdr_id, int layer, float *);
   void get_opacity_record    (int hdr_id, int layer, float *);
   void get_cad_score_record  (int hdr_id, int layer, float *);
   void get_fclass_record     (int hdr_id, int layer, float *);


};


////////////////////////////////////////////////////////////////////////


struct Calipso_5km_Vars {

   int  sd_id;   //  the "handle" for the hdf input file

      //
      //  hdf variables we're interested in
      //

   HdfVarInfo  lat;
   HdfVarInfo  lon;
   HdfVarInfo  time;

   HdfVarInfo  top_layer;
   HdfVarInfo  top_pressure;

   HdfVarInfo  base_layer;
   HdfVarInfo  base_pressure;

   HdfVarInfo  opacity_flag;

   HdfVarInfo  cad_score;

   HdfVarInfo  num_layers;

   HdfVarInfo  fclass;

      //
      //  some utility members
      //

   Calipso_5km_Vars();

   void clear();

   void get_vars(const int _hdf_sd_id);


   void get_latlon(int, float & lat, float & lon) const;

   unixtime get_time(int) const;

   void get_obs(int, Calipso_5km_Obs &) const;


};


////////////////////////////////////////////////////////////////////////


#endif   /*  __CALIPSO_5KM_DATA__  */


////////////////////////////////////////////////////////////////////////



