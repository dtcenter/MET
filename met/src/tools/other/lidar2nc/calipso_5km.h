
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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


//static const float FILL_VALUE = -9999.f;


////////////////////////////////////////////////////////////////////////

static const int n_layers_grib_code       = 500;
static const int layer_base_grib_code     = 501;
static const int layer_top_grib_code      = 502;
static const int opacity_grib_code        = 503;
static const int cad_score_grib_code      = 504;
static const int base_base_grib_code      = 505;
static const int top_top_grib_code        = 506;

   //
   //  these are from the feature classification bits
   //

static const int ftype_grib_code             = 600;
static const int ice_water_grib_code         = 601;
static const int subtype_grib_code           = 602;
static const int cloud_aerosol_qa_grib_code  = 603;
static const int h_average_grib_code         = 604;


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

   unsigned short fclass  [hdf_max_layers];    //  feature classification bits

      //

   Calipso_5km_Obs();

   void clear();

   void get_n_layers_record   (int hdr_id,            float *);

   void get_layer_top_record  (int hdr_id, int layer, float *);
   void get_layer_base_record (int hdr_id, int layer, float *);
   void get_opacity_record    (int hdr_id, int layer, float *);
   void get_cad_score_record  (int hdr_id, int layer, float *);

   void get_base_base_record  (int hdr_id,            float *);
   void get_top_top_record    (int hdr_id,            float *);

      //
      //  these things are all obtained from the feature classification bits
      //

   void get_feature_type_record       (int hdr_id, int layer, float *);
   // void get_feature_type_qa_record    (int hdr_id, int layer, float *);
   int  get_feature_type_qa_value     (int layer);

   void get_ice_water_record          (int hdr_id, int layer, float *);
   // void get_ice_water_qa_record       (int hdr_id, int layer, float *);
   int  get_ice_water_qa_value        (int layer);

   void get_subtype_record            (int hdr_id, int layer, float *);

   void get_cloud_aerosol_qa_record   (int hdr_id, int layer, float *);
   int  get_cloud_aerosol_qa_value    (int layer);

   void get_h_average_record          (int hdr_id, int layer, float *);

      //
      //  fills in all but the last float in a feature classification record
      //

   void fclass_record_header          (int hdr_id, int layer, int grib_code, float *);

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



