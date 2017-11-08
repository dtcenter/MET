// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __WRITE_NETCDF_H__
#define  __WRITE_NETCDF_H__

////////////////////////////////////////////////////////////////////////

#include <netcdf>
using namespace netCDF;

#include "vx_grid.h"
#include "vx_config.h"
#include "observation.h"

////////////////////////////////////////////////////////////////////////
#define HDR_ARRAY_LEN    3   // Observation header length
#define OBS_ARRAY_LEN    5   // Observation values length
#define HEADER_STR_LEN   16  // Maximum length for header string
#define HEADER_STR_LEN_L 40  // Maximum length for header string (for time summary)

#define OBS_BUFFER_SIZE  (128 * 1024)

static const float FILL_VALUE = -9999.f;

////////////////////////////////////////////////////////////////////////

struct NetcdfObsVars {
   bool  use_var_id  ;
   int   hdr_len     ; // header array length (fixed dimension if hdr_len > 0)
   int   hdr_str_len ; // header string length
   NcDim strl_dim    ; // header string dimension
   NcDim strll_dim   ; // header string dimension (bigger dimension)
   NcDim hdr_arr_dim ; // Header array width
   NcDim obs_arr_dim ; // Observation array width
   NcDim obs_dim     ; // Observation array length
   NcDim hdr_dim     ; // Header array length
   //NcDim var_dim     ;
   NcVar hdr_typ_var ; // Message type
   NcVar hdr_sid_var ; // Station ID
   NcVar hdr_vld_var ; // Valid time
   NcVar hdr_arr_var ; // Header array
   NcVar obs_qty_var ; // Quality flag (unlimited dimension)
   NcVar obs_arr_var ; // Observation array (unlimited dimension)
};

struct NcHeaderArrays {
   StringArray typ_sa;  // Message type
   StringArray sid_sa;  // Station ID
   StringArray vld_sa;  // Valid time
   NumArray    lat_na;  // Latitude
   NumArray    lon_na;  // Longitude
   NumArray    elv_na;  // Elevation
};

////////////////////////////////////////////////////////////////////////

//extern int   cur_hdr_idx;
//extern int   hdr_data_idx;
//extern int   hdr_data_offset;
//extern int   obs_data_idx;
//extern int   obs_data_offset;
//extern float  hdr_arr_buf[OBS_BUFFER_SIZE][HDR_ARRAY_LEN];
//extern float obs_data_buf[OBS_BUFFER_SIZE][OBS_ARRAY_LEN];

extern void write_netcdf_global     (NcFile *, const char *, const char *,
                                     const char *model_name = (const char *) 0,
                                     const char *desc       = (const char *) 0,
                                     const char *obtype     = (const char *) 0);
extern void write_netcdf_proj       (NcFile *, const Grid &);
extern void write_netcdf_latlon     (NcFile *, NcDim *, NcDim *, const Grid &);
extern void write_netcdf_grid_weight(NcFile *, NcDim *, NcDim *, const GridWeightType, const DataPlane &); 
extern void write_netcdf_var_times  (NcVar *, const DataPlane &);
extern void write_netcdf_var_times  (NcVar *, const unixtime, const unixtime, const int);

// Observations to NetCDF
extern int get_nc_header_index      ();
extern int get_nc_hdr_buf_count     ();
extern int get_nc_obs_buf_count     ();

extern void nc_obs_initialize       ();

extern void add_nc_header
     (const char *hdr_typ, const char *hdr_sid, const char *hdr_vld,
      const float hdr_lat, const float hdr_lon, const float hdr_elv);
extern void add_and_write_nc_observation
     (const NetcdfObsVars &obsVars,
      const float obs_arr[OBS_ARRAY_LEN], const char *obs_qty);

extern void create_nc_hdr_vars (NetcdfObsVars &obsVars, NcFile *, const int hdr_count, const int deflate_level=0);
extern void create_nc_obs_vars (NetcdfObsVars &obsVars, NcFile *, const int deflate_level=0, const bool use_var_id=true);

extern void read_nc_dims_vars  (NetcdfObsVars &obsVars, NcFile *);

extern void reset_header_buffer(int buf_size);

extern void write_nc_headers        (const NetcdfObsVars &obsVars);
extern void write_nc_header_buffer  (const NetcdfObsVars &obsVars, const int buf_size);
extern void write_nc_obs_buffer     (const NetcdfObsVars &obsVars, const int buf_size);
extern bool write_nc_observations   (const NetcdfObsVars &obsVars, const vector< Observation > observations);
      

#endif   //  __WRITE_NETCDF_H__

////////////////////////////////////////////////////////////////////////
