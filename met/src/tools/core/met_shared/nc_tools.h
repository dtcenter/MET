// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __NC_TOOLS_H__
#define  __NC_TOOLS_H__

#include <netcdf>
using namespace netCDF;
#include "nc_utils.h"


////////////////////////////////////////////////////////////////////////
// struct definition

////////////////////////////////////////////////////////////////////////
// extern variables

////////////////////////////////////////////////////////////////////////

extern bool add_nc_header_to_array
     (const char *hdr_typ, const char *hdr_sid, const time_t hdr_vld,
      const float hdr_lat, const float hdr_lon, const float hdr_elv);
extern bool add_nc_header_prepbufr (const int pb_report_type,
      const int in_report_type, const int instrument_type);
//extern bool is_same_header
//     (const char *hdr_typ, const char *hdr_sid, const unixtime hdr_vld,
//      const float hdr_lat, const float hdr_lon, const float hdr_elv);
//extern bool is_same_header
//     (const char *hdr_typ, const char *hdr_sid, const char *hdr_vld,
//      const float hdr_lat, const float hdr_lon, const float hdr_elv);
      
extern int check_nc_dims_vars(const NetcdfObsVars obs_vars);

extern void clear_header_data(NcHeaderData *);

extern long count_nc_headers   (vector< Observation > &observations);

extern void create_nc_hdr_vars (NetcdfObsVars &, NcFile *, const int hdr_count, const int deflate_level=0);
extern void create_nc_name_vars(NetcdfObsVars &, NcFile *, const int var_count = 0,
                                const int unit_count=0, const int deflate_level=0);
extern void create_nc_obs_name_vars(NetcdfObsVars &, NcFile *,  
                                    const int var_count, const int unit_count, const int deflate_level=0);
extern NcDim create_nc_obs_var_var(NetcdfObsVars &, NcFile *, int var_count, const int deflate_level);
extern void create_nc_obs_vars (NetcdfObsVars &, NcFile *, const int deflate_level=0, const bool use_var_id=true);
extern void create_nc_table_vars(NetcdfObsVars &, NcFile *, const NcDataBuffer &data_buf,
                                 const NcHeaderData &hdr_buf, const int deflate_level=0);

extern void create_nc_pb_hdrs  (NetcdfObsVars &, NcFile *, const int hdr_count, const int deflate_level=0);

extern NcHeaderData get_nc_hdr_data(NetcdfObsVars obs_vars);
extern void get_nc_pb_hdr_data (NetcdfObsVars obs_vars, NcHeaderData *header_data);

extern bool is_using_var_id    (const char * nc_name);

extern void nc_obs_initialize  ();

extern bool read_nc_obs_data(NetcdfObsVars obs_vars, int buf_size, int offset,
      int qty_len, float *obs_arr, int *qty_idx_arr, char *obs_qty_buf);

extern void reset_header_buffer(int buf_size, bool reset_all=false);

extern void write_header_to_nc      (const NetcdfObsVars &, NcDataBuffer &data_buf,
                                    const int buf_size, const bool is_pb = false);

extern void write_nc_table_vars     (NetcdfObsVars &);
extern void write_nc_arr_headers    (const NetcdfObsVars &);
extern void write_nc_buf_headers    (const NetcdfObsVars &);
extern void write_nc_header         (const NetcdfObsVars &,
                const char *hdr_typ, const char *hdr_sid, const time_t hdr_vld,
                const float hdr_lat, const float hdr_lon, const float hdr_elv);
extern void write_nc_obs_buffer     (NcDataBuffer &data_buf, const int buf_size);
extern int  write_nc_observations   (const NetcdfObsVars &, const vector< Observation > observations,
                const bool use_var_idx = true, const bool reset = false, const bool include_header = true);
extern void write_nc_observation    (const NetcdfObsVars &, NcDataBuffer &data_buf,
                const float obs_arr[OBS_ARRAY_LEN], const char *obs_qty);
extern void write_nc_observation    (const NetcdfObsVars &, NcDataBuffer &data_buf);
extern int  write_nc_string_array   (NcVar *ncVar, StringArray &strArray, const int str_len);

extern void write_obs_var_names(NetcdfObsVars &, StringArray &);
extern void write_obs_var_units(NetcdfObsVars &, StringArray &);
extern void write_obs_var_descriptions(NetcdfObsVars &, StringArray &);

#endif   /*  __NC_TOOLS_H__  */


////////////////////////////////////////////////////////////////////////
