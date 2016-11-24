// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:  madis2nc.h
//
//   Description:
//      Parse MADIS NetCDF files containing surface point observations
//      and reformat them for use by MET.  Initial release provides
//      support for METAR and RAOB MADIS types.  Support for additional
//      MADIS types should be added.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    07-21-11  Halley Gotway  Adapted from contributed code.
//
////////////////////////////////////////////////////////////////////////

#ifndef  __MADIS2NC_H__
#define  __MADIS2NC_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

////////////////////////////////////////////////////////////////////////

#include "vx_log.h"
#include "mask_poly.h"
#include "vx_grid.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

//#define BUFFER_SIZE (128*1024)
#define BUFFER_SIZE (32*1024)


// Enumeration of possible MADIS observation types
enum MadisType {
  madis_none,
  madis_coop,
  madis_HDW,
  madis_HDW1h,
  madis_hydro,
  madis_POES,
  madis_acars,
  madis_acarsProfiles,
  madis_maritime,
  madis_metar,
  madis_mesonet,
  madis_profiler,
  madis_radiometer,
  madis_raob,
  madis_sao,
  madis_satrad,
  madis_snow
};

// Constants
static const char *program_name = "madis2nc";
static const float fill_value   = -9999.f;
static const int   strl_len     = 16; // Length of "YYYYMMDD_HHMMSS"
static const int   hdr_arr_len  = 3;  // Observation header length
static const int   obs_arr_len  = 5;  // Observation values length

////////////////////////////////////////////////////////////////////////
//
// Strings for MADIS types.
//
////////////////////////////////////////////////////////////////////////

static const char *metar_str         = "METAR";
static const char *raob_str          = "RAOB";
static const char *profiler_str      = "PROFILER";
static const char *maritime_str      = "MARITIME";
static const char *mesonet_str       = "MESONET";
static const char *acarsProfiles_str = "ACARSPROFILES";

////////////////////////////////////////////////////////////////////////
//
// Constants for reading MADIS - common for many MADIS files.
//
////////////////////////////////////////////////////////////////////////

static const char *in_recNum_str        = "recNum";
static const char *in_fillValue_str     = "_FillValue";

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

static ConcatString mdfile;
static ConcatString ncfile;
static MadisType    mtype = madis_none;
static StringArray  qc_dd_sa;
static StringArray  lvl_dim_sa;
static int          rec_beg = 0;
static int          rec_end = 0;
static ConcatString argv_str;
static Grid         grid_mask;
static MaskPoly     poly_mask;

// Counters
static int          i_obs    = 0;
static int          rej_fill = 0;
static int          rej_qc   = 0;
static int          rej_grid = 0;
static int          rej_poly = 0;

////////////////////////////////////////////////////////////////////////
//
// Variables for NetCDF file
//
////////////////////////////////////////////////////////////////////////

// Output NetCDF file
NcFile *f_out = (NcFile *) 0;

// Output NetCDF dimensions
//static NcDim *strl_dim    = (NcDim *)  0; // Maximum string length
//static NcDim *hdr_arr_dim = (NcDim *)  0; // Header array width
//static NcDim *obs_arr_dim = (NcDim *)  0; // Observation array width
//static NcDim *hdr_dim     = (NcDim *)  0; // Header array length
//static NcDim *obs_dim     = (NcDim *)  0; // Observation array length

static NcDim strl_dim    ; // Maximum string length
static NcDim hdr_arr_dim ; // Header array width
static NcDim obs_arr_dim ; // Observation array width
static NcDim hdr_dim     ; // Header array length
static NcDim obs_dim     ; // Observation array length

// Output NetCDF variables
//static NcVar *hdr_typ_var = (NcVar *)  0; // Message type
//static NcVar *hdr_sid_var = (NcVar *)  0; // Station ID
//static NcVar *hdr_vld_var = (NcVar *)  0; // Valid time
//static NcVar *hdr_arr_var = (NcVar *)  0; // Header array
//static NcVar *obs_qty_var = (NcVar *)  0; // Quality Flag
//static NcVar *obs_arr_var = (NcVar *)  0; // Observation array

static NcVar hdr_typ_var ; // Message type
static NcVar hdr_sid_var ; // Station ID
static NcVar hdr_vld_var ; // Valid time
static NcVar hdr_arr_var ; // Header array
static NcVar obs_qty_var ; // Quality Flag
static NcVar obs_arr_var ; // Observation array

int   obs_buf_size;
int   processed_count;
int   obs_data_idx;
int   obs_data_offset;
int   hdr_data_idx;
int   hdr_data_offset;

char   hdr_typ_buf[BUFFER_SIZE][strl_len];
char   hdr_sid_buf[BUFFER_SIZE][strl_len];
char   hdr_vld_buf[BUFFER_SIZE][strl_len];
float  hdr_arr_buf[BUFFER_SIZE][hdr_arr_len];
float obs_data_buf[BUFFER_SIZE][obs_arr_len];
char  qty_data_buf[BUFFER_SIZE][strl_len];

char   hdr_typ_out_buf[BUFFER_SIZE][strl_len];
char   hdr_sid_out_buf[BUFFER_SIZE][strl_len];
char   hdr_vld_out_buf[BUFFER_SIZE][strl_len];
float  hdr_arr_out_buf[BUFFER_SIZE][hdr_arr_len];
float obs_data_out_buf[BUFFER_SIZE][obs_arr_len];
char  qty_data_out_buf[BUFFER_SIZE][strl_len];


// float  hdr_lat_arr[BUFFER_SIZE];
// float  hdr_lon_arr[BUFFER_SIZE];
// float  hdr_elv_arr[BUFFER_SIZE];
// double tmp_dbl_arr[BUFFER_SIZE];

//float pressure_arr[BUFFER_SIZE];

// //float seaLevelPress_arr[BUFFER_SIZE];
// float visibility_arr[BUFFER_SIZE];
// float temperature_arr[BUFFER_SIZE];
// float dewpoint_arr[BUFFER_SIZE];
// float windDir_arr[BUFFER_SIZE];
// float windSpeed_arr[BUFFER_SIZE];
// float windGust_arr[BUFFER_SIZE];
// float minTemp24Hour_arr[BUFFER_SIZE];
// float maxTemp24Hour_arr[BUFFER_SIZE];
// float precip1Hour_arr[BUFFER_SIZE];
// float precip3Hour_arr[BUFFER_SIZE];
// float precip6Hour_arr[BUFFER_SIZE];
// float precip24Hour_arr[BUFFER_SIZE];
// float snowCover_arr[BUFFER_SIZE];
// 
// char seaLevelPressQty_arr[BUFFER_SIZE];
// char visibilityQty_arr[BUFFER_SIZE];
// char temperatureQty_arr[BUFFER_SIZE];
// char dewpointQty_arr[BUFFER_SIZE];
// char windDirQty_arr[BUFFER_SIZE];
// char windSpeedQty_arr[BUFFER_SIZE];
// char windGustQty_arr[BUFFER_SIZE];
// char minTemp24HourQty_arr[BUFFER_SIZE];
// char maxTemp24HourQty_arr[BUFFER_SIZE];
// char precip1HourQty_arr[BUFFER_SIZE];
// char precip3HourQty_arr[BUFFER_SIZE];
// char precip6HourQty_arr[BUFFER_SIZE];
// char precip24HourQty_arr[BUFFER_SIZE];
// char snowCoverQty_arr[BUFFER_SIZE];
// 
// //float temperature_arr[BUFFER_SIZE];
// //float dewpoint_arr[BUFFER_SIZE];
// float relHumidity_arr[BUFFER_SIZE];
// float stationPressure_arr[BUFFER_SIZE];
// float seaLevelPressure_arr[BUFFER_SIZE];
// fl//oat windDir_arr[BUFFER_SIZE];
// //float windSpeed_arr[BUFFER_SIZE];
// //float windGust_arr[BUFFER_SIZE];
// //float visibility_arr[BUFFER_SIZE];
// float precipRate_arr[BUFFER_SIZE];
// float solarRadiation_arr[BUFFER_SIZE];
// float seaSurfaceTemp_arr[BUFFER_SIZE];
// float totalColumnPWV_arr[BUFFER_SIZE];
// float soilTemperature_arr[BUFFER_SIZE];
// //float minTemp24Hour_arr[BUFFER_SIZE];
// //float maxTemp24Hour_arr[BUFFER_SIZE];
// float precip3hr_arr[BUFFER_SIZE];
// float precip6hr_arr[BUFFER_SIZE];
// float precip12hr_arr[BUFFER_SIZE];
// float precip10min_arr[BUFFER_SIZE];
// float precip1min_arr[BUFFER_SIZE];
// float windDir10_arr[BUFFER_SIZE];
// float windSpeed10_arr[BUFFER_SIZE];
// 
// //char temperatureQty_arr[BUFFER_SIZE];
// //char dewpointQty_arr[BUFFER_SIZE];
// char relHumidityQty_arr[BUFFER_SIZE];
// char stationPressureQty_arr[BUFFER_SIZE];
// char seaLevelPressureQty_arr[BUFFER_SIZE];
// //char windDirQty_arr[BUFFER_SIZE];
// //char windSpeedQty_arr[BUFFER_SIZE];
// //char windGustQty_arr[BUFFER_SIZE];
// //char visibilityQty_arr[BUFFER_SIZE];
// char precipRateQty_arr[BUFFER_SIZE];
// char solarRadiationQty_arr[BUFFER_SIZE];
// char seaSurfaceTempQty_arr[BUFFER_SIZE];
// char totalColumnPWVQty_arr[BUFFER_SIZE];
// char soilTemperatureQty_arr[BUFFER_SIZE];
// //char minTemp24HourQty_arr[BUFFER_SIZE];
// //char maxTemp24HourQty_arr[BUFFER_SIZE];
// char precip3hrQty_arr[BUFFER_SIZE];
// char precip6hrQty_arr[BUFFER_SIZE];
// char precip12hrQty_arr[BUFFER_SIZE];
// char precip10minQty_arr[BUFFER_SIZE];
// char precip1minQty_arr[BUFFER_SIZE];
// char windDir10Qty_arr[BUFFER_SIZE];
// char windSpeed10Qty_arr[BUFFER_SIZE];

////////////////////////////////////////////////////////////////////////

#endif   //  __MADIS2NC_H__

////////////////////////////////////////////////////////////////////////
