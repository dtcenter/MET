// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MET_POINT_DATA_H__
#define  __MET_POINT_DATA_H__


////////////////////////////////////////////////////////////////////////


#include <ostream>

#include "nc_utils.h"


////////////////////////////////////////////////////////////////////////

struct MetPointHeader {
   int typ_len;
   int sid_len;
   int vld_len;
   int strl_len;
   int strll_len;
   int min_vld_time;
   int max_vld_time;
   int hdr_count;
   int hdr_type_count;

   StringArray typ_array;
   StringArray sid_array;
   StringArray vld_array;       // string array for valid time
   IntArray    vld_num_array;   // number array for valid time
   IntArray    typ_idx_array;
   IntArray    sid_idx_array;
   IntArray    vld_idx_array;   // index to vld_array/vld_num_array
   NumArray    lat_array;
   NumArray    lon_array;
   NumArray    elv_array;
   IntArray    prpt_typ_array;
   IntArray    irpt_typ_array;
   IntArray    inst_typ_array;

   MetPointHeader();
   void assign(MetPointHeader &h);
   void clear();
   void reset_counters();
};

////////////////////////////////////////////////////////////////////////

struct MetPointObsData {
   int obs_cnt;
   bool is_obs_array;
   
   int *obs_ids;       // grib_code or var_id
   int *obs_hids;
   int *obs_qids;
   float *obs_lvls;
   float *obs_hgts;
   float *obs_vals;
   float *obs_arr;       // nobs * 5
   StringArray var_names;
   StringArray qty_names;
   
   MetPointObsData();
   void allocate();
   void assign(MetPointObsData &d);
   void clear();
   void clear_numbers();
   void clear_strings();
   bool fill_obs_buf(int buf_size, int offset, float *obs_arr, int *qty_idx_arr);
   float get_obs_val(int index);
};


class MetPointData {

   protected:

      int nobs;
      int nhdr;
      int qty_length;
      bool use_var_id;
      bool use_arr_vars;

      MetPointHeader header_data;
      MetPointObsData *obs_data = 0;

      void init_from_scratch();

   public:

      MetPointData();
     ~MetPointData();

      //void close();
      void clear();

      int get_buf_size();
      int get_hdr_cnt();
      int get_grib_code_or_var_index(const float obs_arr[OBS_ARRAY_LEN]);
      MetPointHeader *get_header_data();
      bool get_header(int header_offset, float hdr_arr[HDR_ARRAY_LEN],
                      ConcatString &hdr_typ_str, ConcatString &hdr_sid_str,
                      ConcatString &hdr_vld_str);
      int get_header_offset(const float obs_arr[OBS_ARRAY_LEN]);
      bool get_header_type(int header_offset, int hdr_typ_arr[HDR_TYPE_ARR_LEN]);
      bool get_lats(float *hdr_lats);
      bool get_lons(float *hdr_lons);
      int get_obs_cnt();
      MetPointObsData *get_point_obs_data();
      StringArray get_qty_data();
      StringArray get_var_names();

      bool is_same_obs_values(const float obs_arr1[OBS_ARRAY_LEN], const float obs_arr2[OBS_ARRAY_LEN]);
      bool is_using_var_id();

      void set_grib_code_or_var_index(float obs_arr[OBS_ARRAY_LEN], int grib_code);
      void set_hdr_cnt(int hdr_cnt);
      void set_obs_cnt(int obs_cnt);
      //  variables

      //  data

};  // MetPointDataBase



class MetPointDataPython : public MetPointData {

   protected:

   public:

      MetPointDataPython();
      MetPointDataPython(MetPointDataPython &d);
     ~MetPointDataPython();

      void allocate(int obs_cnt);
      void set_use_var_id(bool new_use_var_id);

      //  variables

      //  data

};  // MetPointData

////////////////////////////////////////////////////////////////////////

inline MetPointHeader *MetPointData::get_header_data() { return &header_data; }
inline int MetPointData::get_buf_size() { return OBS_BUFFER_SIZE; }
inline int MetPointData::get_grib_code_or_var_index(const float obs_arr[OBS_ARRAY_LEN]) { return obs_arr[1]; };
inline int MetPointData::get_hdr_cnt() { return nhdr; }
inline int MetPointData::get_header_offset(const float obs_arr[OBS_ARRAY_LEN]) { return obs_arr[0]; };
inline int MetPointData::get_obs_cnt() { return nobs; }
inline MetPointObsData *MetPointData::get_point_obs_data() { return obs_data; }
inline StringArray MetPointData::get_qty_data() { return obs_data->qty_names; }
inline StringArray MetPointData::get_var_names() { return obs_data->var_names; }
inline bool MetPointData::is_using_var_id() { return use_var_id; }
inline void MetPointData::set_grib_code_or_var_index(float obs_arr[OBS_ARRAY_LEN], int grib_code) { obs_arr[1] = grib_code; }
inline void MetPointDataPython::set_use_var_id(bool new_use_var_id) { use_var_id = new_use_var_id; }

////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_POINT_DATA_H__  */


////////////////////////////////////////////////////////////////////////

