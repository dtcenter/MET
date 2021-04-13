// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __NC_POINT_OBS_H__
#define  __NC_POINT_OBS_H__


////////////////////////////////////////////////////////////////////////


#include <ostream>

#include "observation.h"
#include "nc_utils.h"
#include "nc_obs_util.h"
#include "nc_var_info.h"
#include "nc_summary.h"


////////////////////////////////////////////////////////////////////////


class MetNcPointObs {

   protected:

      int nobs;
      int nhdr;
      int qty_length;
      bool keep_nc;
      bool use_var_id;
      bool use_arr_vars;
      NcFile *obs_nc;      //  allocated

      NetcdfObsVars obs_vars;
      NcPointObsData obs_data;
      NcHeaderData header_data;

      //MetNcPointObs(const MetNcPointObs &);
      //MetNcPointObs & operator=(const MetNcPointObs &);

      void init_from_scratch();

   public:

      MetNcPointObs();
     ~MetNcPointObs();

      bool open(const char * filename);
      void close();
      bool check_nc(const char *nc_name, const char *caller=empty_name);
      bool set_netcdf(NcFile *nc_file, bool _keep_nc=false);

      int get_buf_size();
      int get_hdr_cnt();
      NcHeaderData get_header_data();
      bool get_header(int header_offset, float hdr_arr[HDR_ARRAY_LEN],
                      ConcatString &hdr_typ_str, ConcatString &hdr_sid_str,
                      ConcatString &hdr_vld_str);
      //int  get_hdr_arr_len();
      bool get_header_type(int header_offset, int hdr_typ_arr[HDR_TYPE_ARR_LEN]);
      bool get_lats(float *hdr_lats);
      bool get_lons(float *hdr_lons);
      //int  get_obs_arr_len();
      int get_obs_cnt();
      NcPointObsData get_point_obs_data();
      StringArray get_qty_data();
      int get_qty_length();
      StringArray get_var_names();

      bool is_using_var_id();
      bool is_using_obs_arr();

      bool read_dim_headers();
      bool read_obs_data();
      bool read_obs_data(int buf_size, int start, float *obs_arr_block,
                         int *obs_qty_idx_block, char *obs_qty_str_block);
      bool read_obs_data_numbers();
      bool read_obs_data_strings();

      void set_using_var_id(bool using_var_id);

      //  variables

      //  data

};  // MetNcPointObs

////////////////////////////////////////////////////////////////////////

//inline NetcdfObsVars MetNcPointObs::get_obs_vars() { return obs_vars; }
inline NcHeaderData MetNcPointObs::get_header_data() { return header_data; }
inline int MetNcPointObs::get_buf_size() { return OBS_BUFFER_SIZE; }
inline int MetNcPointObs::get_hdr_cnt() { return nhdr; }
inline int MetNcPointObs::get_obs_cnt() { return nobs; }
inline NcPointObsData MetNcPointObs::get_point_obs_data() { return obs_data; }
inline StringArray MetNcPointObs::get_qty_data() { return obs_data.qty_names; }
inline StringArray MetNcPointObs::get_var_names() { return obs_data.var_names; }
inline bool MetNcPointObs::is_using_obs_arr() { return use_arr_vars; }
inline bool MetNcPointObs::is_using_var_id() { return use_var_id; }

////////////////////////////////////////////////////////////////////////

class MetNcPointObs2Write : public MetNcPointObs {

   protected:
      int raw_hdr_cnt;
      bool reset_hdr_buffer;

      NcDataBuffer data_buffer;
      NcObsOutputData out_data;

      //MetNcPointObs2Write(const MetNcPointObs2Write &);
      //MetNcPointObs2Write & operator=(const MetNcPointObs2Write &);

      void init_from_scratch();

   public:

      MetNcPointObs2Write();
     ~MetNcPointObs2Write();

      bool add_header(const char *hdr_typ, const char *hdr_sid, const time_t hdr_vld,
                      const float hdr_lat, const float hdr_lon, const float hdr_elv);
      bool add_header_prepbufr (const int pb_report_type, const int in_report_type,
                                const int instrument_type);
      bool add_header_strings(const char *hdr_typ, const char *hdr_sid);
      bool add_header_vld(const char *hdr_vld);

      void create_pb_hdrs(int pb_hdr_count);
         
      int get_buf_size();
      void get_dim_counts(int *obs_cnt, int *hdr_cnt);
//      NcHeaderData *get_header_data();
      int get_hdr_index();
//      int get_obs_cnt();
      int get_obs_index();
      NcObsOutputData *get_output_data();
      NetcdfObsVars *get_obs_vars();

      void init_buffer();
      void init_obs_vars(bool using_var_id, int deflate_level, bool attr_agl=false);
      bool init_netcdf(int obs_count, int hdr_count, string program_name);

      void reset_header_buffer(int buf_size, bool reset_all);
//      void set_nc_out_data(vector<Observation> observations, SummaryObs *summary_obs,
//                           TimeSummaryInfo summary_info);
//      bool set_reset_hdr_buffer(bool reset_buffer);

      void write_arr_headers();
      void write_buf_headers ();
      void write_header (const char *hdr_typ, const char *hdr_sid, const time_t hdr_vld,
                         const float hdr_lat, const float hdr_lon, const float hdr_elv);
      void write_observation();
      void write_observation(const float obs_arr[OBS_ARRAY_LEN], const char *obs_qty);
      void write_obs_data();
      int  write_obs_data(const vector< Observation > observations,
                          const bool do_header = true);
      bool write_to_netcdf(StringArray obs_names, StringArray obs_units,
                           StringArray obs_descs);

      //  variables

      //  data

};  // MetNcPointObs2Write


////////////////////////////////////////////////////////////////////////

inline int MetNcPointObs2Write::get_hdr_index() { return data_buffer.cur_hdr_idx; }
inline int MetNcPointObs2Write::get_obs_index() { return data_buffer.obs_data_idx; }
inline NcObsOutputData *MetNcPointObs2Write::get_output_data() { return &out_data; }
inline NetcdfObsVars *MetNcPointObs2Write::get_obs_vars() { return &obs_vars; }
//inline bool MetNcPointObs2Write::set_reset_hdr_buffer(bool reset_buffer) { reset_hdr_buffer = reset_buffer; }

////////////////////////////////////////////////////////////////////////


#endif   /*  __NC_POINT_OBS_H__  */


////////////////////////////////////////////////////////////////////////


