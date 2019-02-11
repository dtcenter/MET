// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_FILE_NETCDF_DEFINES_H__
#define  __MTD_FILE_NETCDF_DEFINES_H__


////////////////////////////////////////////////////////////////////////


// static const char         nx_dim_name [] = "Nx";
// static const char         ny_dim_name [] = "Ny";
// static const char         nt_dim_name [] = "Nt";
static const char         nx_dim_name [] = "lon";
static const char         ny_dim_name [] = "lat";
static const char         nt_dim_name [] = "time";

static const char       nobj_dim_name [] = "Nobjects";

static const char     data_field_name [] = "MtdData";
static const char        volumes_name [] = "Volumes";
static const char      intensity_name [] = "MaxConvIntensity";

static const char            lat_name [] = "lat";
static const char            lon_name [] = "lon";

static const char       fcst_raw_name [] = "fcst_raw";
static const char        obs_raw_name [] = "obs_raw";

static const char    fcst_obj_id_name [] = "fcst_object_id";
static const char     obs_obj_id_name [] = "obs_object_id";

static const char   fcst_clus_id_name [] = "fcst_cluster_id";
static const char    obs_clus_id_name [] = "obs_cluster_id";

static const char start_time_att_name [] = "StartTime";
static const char    delta_t_att_name [] = "DeltaT";
static const char     radius_att_name [] = "Radius";
static const char  threshold_att_name [] = "Threshold";
static const char  min_value_att_name [] = "MinDataValue";
static const char  max_value_att_name [] = "MaxDataValue";
static const char   is_split_att_name [] = "is_split";


////////////////////////////////////////////////////////////////////////


static const char   filetype_att_name    [] = "filetype";

static const char   filetype_raw_name    [] = "raw";
static const char   filetype_conv_name   [] = "conv";
static const char   filetype_mask_name   [] = "mask";
static const char   filetype_object_name [] = "object";


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_FILE_NETCDF_DEFINES_H__  */


////////////////////////////////////////////////////////////////////////


