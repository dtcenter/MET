// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
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
#include "nc_utils.h"
#include "observation.h"

////////////////////////////////////////////////////////////////////////
static const float FILL_VALUE = -9999.f;

////////////////////////////////////////////////////////////////////////

struct NcDataBuffer {
   NetcdfObsVars obsVars;
   int   processed_count;
   int   obs_count;
   int   obs_buf_size;
   int   cur_obs_idx;
   int   obs_data_idx;
   int   obs_data_offset;
   int   hdr_count;
   int   hdr_buf_size;
   int   cur_hdr_idx;
   int   hdr_data_idx;
   int   hdr_data_offset;
   
   char   hdr_typ_buf[OBS_BUFFER_SIZE][HEADER_STR_LEN_L];
   char   hdr_sid_buf[OBS_BUFFER_SIZE][HEADER_STR_LEN_L];
   char   hdr_vld_buf[OBS_BUFFER_SIZE][HEADER_STR_LEN];
   float  hdr_arr_buf[OBS_BUFFER_SIZE][HDR_ARRAY_LEN];
   float obs_data_buf[OBS_BUFFER_SIZE][OBS_ARRAY_LEN];
   char  qty_data_buf[OBS_BUFFER_SIZE][HEADER_STR_LEN];
};

////////////////////////////////////////////////////////////////////////

extern void write_netcdf_global     (NcFile *, const char *, const char *,
                                     const char *model_name = (const char *) 0,
                                     const char *obtype     = (const char *) 0,
                                     const char *desc       = (const char *) 0);
extern void write_netcdf_proj       (NcFile *, const Grid &);
extern void write_netcdf_latlon     (NcFile *, NcDim *, NcDim *, const Grid &);
extern void write_netcdf_grid_weight(NcFile *, NcDim *, NcDim *, const GridWeightType, const DataPlane &); 
extern void write_netcdf_var_times  (NcVar *, const DataPlane &);
extern void write_netcdf_var_times  (NcVar *, const unixtime, const unixtime, const int);

extern void nc_obs_initialize       ();

extern void add_nc_header_full
     (const char *hdr_typ, const char *hdr_sid, const char *hdr_vld,
      const float hdr_lat, const float hdr_lon, const float hdr_elv);

extern long count_nc_headers   (vector< Observation > &observations);

extern void create_nc_hdr_vars (NetcdfObsVars &obsVars, NcFile *, const int hdr_count, const int deflate_level=0);
extern void create_nc_obs_vars (NetcdfObsVars &obsVars, NcFile *, const int deflate_level=0, const bool use_var_id=true);

extern void init_nc_dims_vars  (NetcdfObsVars &obsVars, bool use_var_id = true);
extern void read_nc_dims_vars  (NetcdfObsVars &obsVars, NcFile *);

extern void reset_header_buffer(int buf_size);

extern void write_nc_headers        (const NetcdfObsVars &obsVars);
//extern void write_nc_header         (const NetcdfObsVars &obsVars, NcDataBuffer data_buf);
extern void write_nc_header         (const NetcdfObsVars &obsVars);
extern void write_nc_header         (const NetcdfObsVars &obsVars,
                const char *hdr_typ, const char *hdr_sid, const char *hdr_vld,
                const float hdr_lat, const float hdr_lon, const float hdr_elv);
extern void write_header_to_nc      (const NetcdfObsVars &obsVars, NcDataBuffer &data_buf, const int buf_size);
extern void write_nc_obs_buffer     (NcDataBuffer &data_buf, const int buf_size);
extern int  write_nc_observations   (const NetcdfObsVars &obsVars, const vector< Observation > observations);
extern void write_nc_observation    (const NetcdfObsVars &obsVars, NcDataBuffer &data_buf,
                const float obs_arr[OBS_ARRAY_LEN], const char *obs_qty);
extern void write_nc_observation    (const NetcdfObsVars &obsVars, NcDataBuffer &data_buf);
      

#endif   //  __WRITE_NETCDF_H__

////////////////////////////////////////////////////////////////////////
