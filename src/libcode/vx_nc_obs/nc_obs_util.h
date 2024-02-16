// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __NC_OBS_UTIL_H__
#define  __NC_OBS_UTIL_H__

#include <netcdf>

#include "nc_summary.h"
#include "met_point_data.h"

////////////////////////////////////////////////////////////////////////


static const char empty_name[] = "";


////////////////////////////////////////////////////////////////////////
// struct definition

struct NcDataBuffer {
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

   int   prev_hdr_vld;
   char  prev_hdr_typ_buf[HEADER_STR_LEN2];
   char  prev_hdr_sid_buf[HEADER_STR_LEN2];
   char  prev_hdr_vld_buf[HEADER_STR_LEN];
   float prev_hdr_arr_buf[HDR_ARRAY_LEN];

   char  hdr_typ_str_buf[OBS_BUFFER_SIZE][HEADER_STR_LEN2];
   char  hdr_sid_str_buf[OBS_BUFFER_SIZE][HEADER_STR_LEN2];
   char  hdr_vld_str_buf[OBS_BUFFER_SIZE][HEADER_STR_LEN];
   float hdr_arr_buf[OBS_BUFFER_SIZE][HDR_ARRAY_LEN];
   float obs_data_buf[OBS_BUFFER_SIZE][OBS_ARRAY_LEN];
   char  qty_data_buf[OBS_BUFFER_SIZE][HEADER_STR_LEN];

   StringArray qty_data_array;
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

   NcDataBuffer();
   void reset_counters();
};

////////////////////////////////////////////////////////////////////////

struct NetcdfObsVars {
   bool  attr_agl    ;
   bool  attr_pb2nc  ;
   bool  use_var_id  ;
   int   hdr_cnt     ; // header array count (fixed dimension if hdr_cnt > 0)
   int   obs_cnt     ; // obs. array count (fixed dimension if obs_cnt > 0)
   int   raw_hdr_cnt ; // raw data (PrepBufr) header array count
   int   deflate_level;

   netCDF::NcDim strl_dim    ; // header string dimension (16 bytes)
   netCDF::NcDim strl2_dim   ; // header string dimension (40 bytes)
   netCDF::NcDim strl3_dim   ; // header string dimension (80 bytes)
   netCDF::NcDim hdr_typ_dim ; // header message type dimension
   netCDF::NcDim hdr_sid_dim ; // header station id dimension
   netCDF::NcDim hdr_vld_dim ; // header valid time dimension
   netCDF::NcDim hdr_arr_dim ; // Header array width (V1.0, not used from V1.2)
   netCDF::NcDim obs_arr_dim ; // Observation array width (V1.0, not used from V1.2)
   netCDF::NcDim obs_dim     ; // Observation array length (V1.0)
   netCDF::NcDim hdr_dim     ; // Header array length (V1.0)
   netCDF::NcDim pb_hdr_dim  ; // PrepfBufr Header array length (V1.2)

   netCDF::NcVar hdr_typ_tbl_var ; // Message type (string) (V1.1)
   netCDF::NcVar hdr_sid_tbl_var ; // Station ID (string) (V1.1)
   netCDF::NcVar hdr_vld_tbl_var ; // Valid time (string) (V1.1)
   netCDF::NcVar obs_qty_tbl_var ; // Quality flag (V1.0)
   netCDF::NcVar hdr_typ_var ; // Message type (string to index with V1.2)
   netCDF::NcVar hdr_sid_var ; // Station ID   (string to index with V1.2)
   netCDF::NcVar hdr_vld_var ; // Valid time   (string to index with V1.2)
   netCDF::NcVar hdr_arr_var ; // Header array (V1.0, Removed from V1.2)
   netCDF::NcVar hdr_lat_var ; // Header array (latitude)  (V1.2)
   netCDF::NcVar hdr_lon_var ; // Header array (longitude) (V1.2)
   netCDF::NcVar hdr_elv_var ; // Header array (elevation) (V1.2)
   netCDF::NcVar hdr_prpt_typ_var ; // Header array (PB report type) (V1.2)
   netCDF::NcVar hdr_irpt_typ_var ; // Header array (In report type) (V1.2)
   netCDF::NcVar hdr_inst_typ_var ; // Header array (instrument type) (V1.2)
   netCDF::NcVar obs_qty_var ; // Quality flag (unlimited dimension) (V1.2: Changed data type to int - was string)
   netCDF::NcVar obs_arr_var ; // Observation array (unlimited dimension) (V1.0, Removed from V1.2)
   netCDF::NcVar obs_hid_var ; // Observation header index array (unlimited dimension) (V1.2)
   netCDF::NcVar  obs_gc_var ; // Observation GRIB code array (unlimited dimension) (V1.2)
   netCDF::NcVar obs_vid_var ; // Observation variable index array (unlimited dimension) (V1.2)
   netCDF::NcVar obs_lvl_var ; // Observation level array (unlimited dimension) (V1.2)
   netCDF::NcVar obs_hgt_var ; // Observation hight array (unlimited dimension) (V1.2)
   netCDF::NcVar obs_val_var ; // Observation value array (unlimited dimension) (V1.2)
   // Optional variables
   netCDF::NcVar obs_var     ; // Observation variable name (V1.1)
   netCDF::NcVar unit_var    ; // The unit of the observation variable (V1.1)
   netCDF::NcVar desc_var    ; // The description of the observation variable (V1.1)

   NetcdfObsVars();
   bool is_valid(bool exit_on_error=false);
   void reset(bool _use_var_id = true);

   void create_dimensions(netCDF::NcFile *f_out);
   void create_hdr_vars (netCDF::NcFile *f_out, const int hdr_count);
   void create_obs_vars (netCDF::NcFile *f_out);
   void create_obs_name_vars (netCDF::NcFile *f_out, const int var_count, const int unit_count);
   void create_table_vars (netCDF::NcFile *f_out, MetPointHeader &hdr_data, NcDataBuffer &data_buffer);
   void create_pb_hdrs (netCDF::NcFile *f_out, const int hdr_count);
   netCDF::NcDim create_var_obs_var (netCDF::NcFile *f_out, int var_count);

   int get_hdr_index();
   int get_obs_index();

   void read_dims_vars(netCDF::NcFile *f_in);
   void read_header_data(MetPointHeader &hdr_data);
   bool read_obs_data(int buf_size, int offset, int qty_len, float *obs_arr,
                      int *qty_idx_arr, char *obs_qty_buf);
   void read_pb_hdr_data(MetPointHeader &hdr_data);

   void write_header_to_nc(NcDataBuffer &data_buf, const int buf_size, const bool is_pb = false);
   void write_obs_buffer(NcDataBuffer &data_buffer, const int buf_size);
   void write_obs_var_names(StringArray &obs_names);
   void write_obs_var_units(StringArray &units);
   void write_obs_var_descriptions(StringArray &descriptions);
   void write_table_vars(MetPointHeader &hdr_data, NcDataBuffer &data_buffer);

};  // NetcdfObsVars

////////////////////////////////////////////////////////////////////////

struct NcPointObsData : public MetPointObsData {
   
   NcPointObsData();
   bool read_obs_data_numbers(NetcdfObsVars obs_vars, bool stop=true);
   bool read_obs_data_table_lookups(NetcdfObsVars obs_vars, bool stop=true);
};

////////////////////////////////////////////////////////////////////////

// extern variables

////////////////////////////////////////////////////////////////////////

extern bool add_nc_header_prepbufr (const int pb_report_type,
                                    const int in_report_type, const int instrument_type);
      
extern long count_nc_headers   (std::vector< Observation > &observations);

extern int  get_nc_hdr_cur_index();
extern int  get_nc_obs_buf_index();

extern bool is_using_var_id    (const char * nc_name);
extern bool is_using_var_id    (netCDF::NcFile * nc_file);

extern void reset_header_buffer(int buf_size, bool reset_all=false);
extern void set_header_buffer  (int buf_size, bool reset_all=false);

extern std::string seconds_to_time_string(const int secs);

extern void write_nc_obs_buffer     (const int buf_size);
extern int  write_nc_string_array   (netCDF::NcVar *ncVar, StringArray &strArray,
                                     const int str_len);

#endif   /*  __NC_OBS_UTIL_H__  */


////////////////////////////////////////////////////////////////////////
