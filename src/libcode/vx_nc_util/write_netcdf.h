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
   NetcdfObsVars obs_vars;
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
   int   pb_hdr_count;
   int   pb_hdr_data_offset;

   char  prev_hdr_typ_buf[HEADER_STR_LEN2];
   char  prev_hdr_sid_buf[HEADER_STR_LEN2];
   char  prev_hdr_vld_buf[HEADER_STR_LEN];
   float prev_hdr_arr_buf[HDR_ARRAY_LEN];
   
   char   hdr_typ_str_buf[OBS_BUFFER_SIZE][HEADER_STR_LEN2];
   char   hdr_sid_str_buf[OBS_BUFFER_SIZE][HEADER_STR_LEN2];
   char   hdr_vld_str_buf[OBS_BUFFER_SIZE][HEADER_STR_LEN];
   float  hdr_arr_buf[OBS_BUFFER_SIZE][HDR_ARRAY_LEN];
   float obs_data_buf[OBS_BUFFER_SIZE][OBS_ARRAY_LEN];
   char  qty_data_buf[OBS_BUFFER_SIZE][HEADER_STR_LEN];
   
   //StringArray   hdr_typ_array;
   //StringArray   hdr_sid_array;
   //StringArray   hdr_vld_array;
   StringArray  qty_data_array;
   int   hdr_typ_buf[OBS_BUFFER_SIZE];
   int   hdr_sid_buf[OBS_BUFFER_SIZE];
   int   hdr_vld_buf[OBS_BUFFER_SIZE];
   float hdr_lat_buf[OBS_BUFFER_SIZE];
   float hdr_lon_buf[OBS_BUFFER_SIZE];
   float hdr_elv_buf[OBS_BUFFER_SIZE];
   int   hdr_prpt_typ_buf[OBS_BUFFER_SIZE];
   int   hdr_irpt_typ_buf[OBS_BUFFER_SIZE];
   int   hdr_inst_typ_buf[OBS_BUFFER_SIZE];
   int   obs_hid_buf[OBS_BUFFER_SIZE];
   int   obs_vid_buf[OBS_BUFFER_SIZE];
   int   qty_idx_buf[OBS_BUFFER_SIZE];
   float obs_lvl_buf[OBS_BUFFER_SIZE];
   float obs_hgt_buf[OBS_BUFFER_SIZE];
   float obs_val_buf[OBS_BUFFER_SIZE];
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

extern bool add_nc_header_all
     (const char *hdr_typ, const char *hdr_sid, const char *hdr_vld,
      const float hdr_lat, const float hdr_lon, const float hdr_elv);
extern bool add_nc_header_prepbufr (const int pb_report_type,
      const int in_report_type, const int instrument_type);
extern bool is_same_header
     (const char *hdr_typ, const char *hdr_sid, const char *hdr_vld,
      const float hdr_lat, const float hdr_lon, const float hdr_elv);
      
extern long count_nc_headers   (vector< Observation > &observations);

extern void create_nc_hdr_vars (NetcdfObsVars &, NcFile *, const int hdr_count, const int deflate_level=0);
extern void create_nc_pb_hdrs  (NetcdfObsVars &, NcFile *, const int hdr_count, const int deflate_level=0);
extern NcDim create_nc_obs_var (NetcdfObsVars &, NcFile *, int var_count, const int deflate_level);
extern void create_nc_obs_vars (NetcdfObsVars &, NcFile *, const int deflate_level=0, const bool use_var_id=true);
extern void create_nc_other_vars (NetcdfObsVars &, NcFile *, const NcDataBuffer &data_buf,
                                  const NcHeaderData &hdr_buf, const int var_count = 0,
                                  const int unit_count=0, const int deflate_level=0);

extern void init_nc_dims_vars  (NetcdfObsVars &, bool use_var_id = true);
extern void read_nc_dims_vars  (NetcdfObsVars &, NcFile *);

extern void reset_header_buffer(int buf_size, bool reset_all=false);

extern void write_nc_other_vars     (NetcdfObsVars &);
extern void write_nc_headers        (const NetcdfObsVars &);
extern void write_nc_header         (const NetcdfObsVars &);
extern void write_nc_header         (const NetcdfObsVars &,
                const char *hdr_typ, const char *hdr_sid, const char *hdr_vld,
                const float hdr_lat, const float hdr_lon, const float hdr_elv);
extern void write_header_to_nc      (const NetcdfObsVars &, NcDataBuffer &data_buf, const int buf_size, const bool is_pb = false);
extern void write_nc_obs_buffer     (NcDataBuffer &data_buf, const int buf_size);
extern int  write_nc_observations   (const NetcdfObsVars &, const vector< Observation > observations,
                const bool reset = true);
extern void write_nc_observation    (const NetcdfObsVars &, NcDataBuffer &data_buf,
                const float obs_arr[OBS_ARRAY_LEN], const char *obs_qty);
extern void write_nc_observation    (const NetcdfObsVars &, NcDataBuffer &data_buf);
extern int  write_nc_string_array   (NcVar *ncVar, StringArray &strArray, const int str_len);
      

#endif   //  __WRITE_NETCDF_H__

////////////////////////////////////////////////////////////////////////
