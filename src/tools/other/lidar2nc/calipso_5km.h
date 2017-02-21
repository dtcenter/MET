
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

#include "hdf.h"
#include "mfhdf.h"

#include "hdf_utils.h"


////////////////////////////////////////////////////////////////////////


struct Calipso_5km_data {

   int hdf_sd_id;

   HdfVarInfo lat_info;
   HdfVarInfo lon_info;
   HdfVarInfo time_info;

   HdfVarInfo top_layer_info;
   HdfVarInfo top_pressure_info;

   HdfVarInfo base_layer_info;
   HdfVarInfo base_pressure_info;

   HdfVarInfo opacity_flag_info;

   HdfVarInfo cad_score_info;

   HdfVarInfo num_layers_info;

      //

   Calipso_5km_data();

   void clear();

   void get_var_info(const int _hdf_sd_id);

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __CALIPSO_5KM_DATA__  */


////////////////////////////////////////////////////////////////////////



