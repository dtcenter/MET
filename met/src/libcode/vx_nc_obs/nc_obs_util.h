// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __NC_OBS_UTIL_H__
#define  __NC_OBS_UTIL_H__

#include <netcdf>
using namespace netCDF;

////////////////////////////////////////////////////////////////////////
// struct definition

struct NetcdfObsVars {
   bool  attr_agl    ;
   bool  attr_pb2nc  ;
   bool  use_var_id  ;
   int   hdr_cnt     ; // header array count (fixed dimension if hdr_cnt > 0)
   int   obs_cnt     ; // obs. array count (fixed dimension if obs_cnt > 0)
   int   raw_hdr_cnt ; // raw data (PrepBufr) header array count
   
   NcDim strl_dim    ; // header string dimension (16 bytes)
   NcDim strl2_dim   ; // header string dimension (40 bytes)
   NcDim strl3_dim   ; // header string dimension (80 bytes)
   NcDim hdr_typ_dim ; // header message type dimension
   NcDim hdr_sid_dim ; // header station id dimension
   NcDim hdr_vld_dim ; // header valid time dimension
   NcDim hdr_arr_dim ; // Header array width (V1.0, not used from V1.2)
   NcDim obs_arr_dim ; // Observation array width (V1.0, not used from V1.2)
   NcDim obs_dim     ; // Observation array length (V1.0)
   NcDim hdr_dim     ; // Header array length (V1.0)
   NcDim pb_hdr_dim  ; // PrefBufr Header array length (V1.2)
   
   NcVar hdr_typ_tbl_var ; // Message type (string) (V1.1)
   NcVar hdr_sid_tbl_var ; // Station ID (string) (V1.1)
   NcVar hdr_vld_tbl_var ; // Valid time (string) (V1.1)
   NcVar obs_qty_tbl_var ; // Quality flag (V1.0)
   NcVar hdr_typ_var ; // Message type (string to index with V1.2)
   NcVar hdr_sid_var ; // Station ID   (string to index with V1.2)
   NcVar hdr_vld_var ; // Valid time   (string to index with V1.2)
   NcVar hdr_arr_var ; // Header array (V1.0, Removed from V1.2)
   NcVar hdr_lat_var ; // Header array (latitude)  (V1.2)
   NcVar hdr_lon_var ; // Header array (longitude) (V1.2)
   NcVar hdr_elv_var ; // Header array (elevation) (V1.2)
   NcVar hdr_prpt_typ_var ; // Header array (PB report type) (V1.2)
   NcVar hdr_irpt_typ_var ; // Header array (In report type) (V1.2)
   NcVar hdr_inst_typ_var ; // Header array (instrument type) (V1.2)
   NcVar obs_qty_var ; // Quality flag (unlimited dimension) (V1.2: Changed data type to int - was string)
   NcVar obs_arr_var ; // Observation array (unlimited dimension) (V1.0, Removed from V1.2)
   NcVar obs_hid_var ; // Observation header index array (unlimited dimension) (V1.2)
   NcVar  obs_gc_var ; // Observation GRIB code array (unlimited dimension) (V1.2)
   NcVar obs_vid_var ; // Observation variable index array (unlimited dimension) (V1.2)
   NcVar obs_lvl_var ; // Observation level array (unlimited dimension) (V1.2)
   NcVar obs_hgt_var ; // Observation hight array (unlimited dimension) (V1.2)
   NcVar obs_val_var ; // Observation value array (unlimited dimension) (V1.2)
   // Optional variables
   NcVar obs_var     ; // Observation variable name (V1.1)
   NcVar unit_var    ; // The unit of the observation variable (V1.1)
   NcVar desc_var    ; // The description of the observation variable (V1.1)
};

struct NcHeaderData {
   int typ_len;
   int sid_len;
   int vld_len;
   int strl_len;  
   int strll_len;
   int min_vld_time;
   int max_vld_time;
   
   StringArray typ_array;
   StringArray sid_array;
   StringArray vld_array;
   IntArray    vld_num_array;
   IntArray    typ_idx_array;
   IntArray    sid_idx_array;
   IntArray    vld_idx_array;
   NumArray    lat_array;
   NumArray    lon_array;
   NumArray    elv_array;
   IntArray    prpt_typ_array;
   IntArray    irpt_typ_array;
   IntArray    inst_typ_array;
};

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

   int   prev_hdr_vld;
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
extern void create_nc_table_vars(NetcdfObsVars &, NcFile *, const int deflate_level=0);

extern void create_nc_pb_hdrs  (NetcdfObsVars &, NcFile *, const int hdr_count, const int deflate_level=0);

extern NcDataBuffer *get_nc_data_buffer();
extern NcHeaderData *get_hdr_data_buffer();

extern int  get_nc_hdr_cur_index();
extern int  get_nc_obs_buf_index();

extern NcHeaderData get_nc_hdr_data(NetcdfObsVars obs_vars);
extern void get_nc_pb_hdr_data (NetcdfObsVars obs_vars, NcHeaderData *header_data);

extern void init_nc_dims_vars_config(NetcdfObsVars &, bool use_var_id = true);
extern bool is_using_var_id    (const char * nc_name);

extern void nc_obs_initialize  ();

extern bool read_nc_obs_data(NetcdfObsVars obs_vars, int buf_size, int offset,
      int qty_len, float *obs_arr, int *qty_idx_arr, char *obs_qty_buf);

extern void read_nc_dims_vars  (NetcdfObsVars &, NcFile *);

extern void reset_header_buffer(int buf_size, bool reset_all=false);
extern void set_header_buffer(int buf_size, bool reset_all=false);

extern void write_header_to_nc      (const NetcdfObsVars &, NcDataBuffer &data_buf,
                                    const int buf_size, const bool is_pb = false);

extern void write_nc_table_vars     (NetcdfObsVars &);
extern void write_nc_arr_headers    (const NetcdfObsVars &);
extern void write_nc_buf_headers    (const NetcdfObsVars &);
extern void write_nc_header         (const NetcdfObsVars &,
                const char *hdr_typ, const char *hdr_sid, const time_t hdr_vld,
                const float hdr_lat, const float hdr_lon, const float hdr_elv);
extern void write_nc_obs_buffer     (const int buf_size);
extern int  write_nc_observations   (const NetcdfObsVars &, const vector< Observation > observations,
                const bool use_var_idx = true, const bool do_header = true);
extern void write_nc_observation    (const NetcdfObsVars &,
                const float obs_arr[OBS_ARRAY_LEN], const char *obs_qty);
extern void write_nc_observation    (const NetcdfObsVars &);
extern int  write_nc_string_array   (NcVar *ncVar, StringArray &strArray, const int str_len);

extern void write_obs_var_names(NetcdfObsVars &, StringArray &);
extern void write_obs_var_units(NetcdfObsVars &, StringArray &);
extern void write_obs_var_descriptions(NetcdfObsVars &, StringArray &);

#endif   /*  __NC_OBS_UTIL_H__  */


////////////////////////////////////////////////////////////////////////
