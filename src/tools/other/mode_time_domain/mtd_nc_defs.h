// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __MTD_FILE_NETCDF_DEFINES_H__
#define  __MTD_FILE_NETCDF_DEFINES_H__

////////////////////////////////////////////////////////////////////////

static const std::string         nx_dim_name = "lon";
static const std::string         ny_dim_name = "lat";
static const std::string         nt_dim_name = "time";

static const std::string       nobj_dim_name = "Nobjects";

static const std::string     data_field_name = "MtdData";
static const std::string        volumes_name = "Volumes";
static const std::string      intensity_name = "MaxConvIntensity";

static const std::string            lat_name = "lat";
static const std::string            lon_name = "lon";

static const std::string       fcst_raw_name = "fcst_raw";
static const std::string        obs_raw_name = "obs_raw";

static const std::string    fcst_obj_id_name = "fcst_object_id";
static const std::string     obs_obj_id_name = "obs_object_id";

static const std::string   fcst_clus_id_name = "fcst_cluster_id";
static const std::string    obs_clus_id_name = "obs_cluster_id";

static const std::string start_time_att_name = "StartTime";
static const std::string    delta_t_att_name = "DeltaT";
static const std::string     radius_att_name = "Radius";
static const std::string  threshold_att_name = "Threshold";
static const std::string   time_beg_att_name = "Time_Beg";
static const std::string   time_end_att_name = "Time_End";
static const std::string  min_value_att_name = "MinDataValue";
static const std::string  max_value_att_name = "MaxDataValue";
static const std::string   is_split_att_name = "is_split";

////////////////////////////////////////////////////////////////////////

static const std::string   filetype_att_name    = "filetype";
static const std::string   filetype_raw_name    = "raw";
static const std::string   filetype_conv_name   = "conv";
static const std::string   filetype_mask_name   = "mask";
static const std::string   filetype_object_name = "object";

////////////////////////////////////////////////////////////////////////

#endif   /*  __MTD_FILE_NETCDF_DEFINES_H__  */

////////////////////////////////////////////////////////////////////////

