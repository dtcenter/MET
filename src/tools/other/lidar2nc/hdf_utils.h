
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __HDF_UTILS_H__
#define  __HDF_UTILS_H__


////////////////////////////////////////////////////////////////////////


#include <netcdf>

#include "logger.h"
#include "concat_string.h"

#include "hdf.h"
#include "mfhdf.h"


////////////////////////////////////////////////////////////////////////


static const char hdf_lat_name            [] = "Latitude";
static const char hdf_lon_name            [] = "Longitude";
static const char hdf_time_name           [] = "Profile_Time";
static const char hdf_num_layers_name     [] = "Number_Layers_Found";
static const char hdf_opacity_flag_name   [] = "Opacity_Flag";
static const char hdf_cad_score_name      [] = "CAD_Score";
static const char hdf_fclass_name         [] = "Feature_Classification_Flags";

static const char hdf_base_layer_name     [] = "Layer_Base_Altitude";
static const char hdf_base_pressure_name  [] = "Layer_Base_Pressure";

static const char hdf_top_layer_name      [] = "Layer_Top_Altitude";
static const char hdf_top_pressure_name   [] = "Layer_Top_Pressure";


////////////////////////////////////////////////////////////////////////


struct HdfVarInfo {

   ConcatString hdf_name;

   int hdf_index;

   int hdf_id;

   int hdf_rank;

   int hdf_type;

   int hdf_atts;

   int hdf_dimsizes[MAX_VAR_DIMS];

      //

   HdfVarInfo();

   void clear();

};


////////////////////////////////////////////////////////////////////////


extern void get_hdf_var_info(const int hdf_sd_id, const char * hdf_name, HdfVarInfo &);

extern int sizeof_hdf_type(const int type);


////////////////////////////////////////////////////////////////////////


#endif   /*  __HDF_UTILS_H__  */


////////////////////////////////////////////////////////////////////////



