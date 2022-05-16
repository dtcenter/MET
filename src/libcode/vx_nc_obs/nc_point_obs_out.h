// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#ifndef  __NC_POINT_OBS_OUT_H__
#define  __NC_POINT_OBS_OUT_H__

////////////////////////////////////////////////////////////////////////

#include <ostream>

#include "observation.h"
#include "nc_utils.h"
#include "nc_obs_util.h"
#include "nc_point_obs.h"
#include "nc_var_info.h"
#include "vx_summary.h"


////////////////////////////////////////////////////////////////////////

class MetNcPointObsOut : public MetNcPointObs {

   protected:
      int raw_hdr_cnt;
      bool reset_hdr_buffer;

      NcDataBuffer data_buffer;
      NcObsOutputData out_data;

      void init_from_scratch();

   public:

      MetNcPointObsOut();
     ~MetNcPointObsOut();

      bool add_header(const char *hdr_typ, const char *hdr_sid, const time_t hdr_vld,
                      const float hdr_lat, const float hdr_lon, const float hdr_elv);
      bool add_header_prepbufr (const int pb_report_type, const int in_report_type,
                                const int instrument_type);
      bool add_header_strings(const char *hdr_typ, const char *hdr_sid);
      bool add_header_vld(const char *hdr_vld);

      void create_pb_hdrs(int pb_hdr_count);
         
      int get_buf_size();
      void get_dim_counts(int *obs_cnt, int *hdr_cnt);
      int get_hdr_index();
      int get_obs_index();
      NcObsOutputData *get_output_data();
      NetcdfObsVars *get_obs_vars();

      void init_buffer();
      void init_obs_vars(bool using_var_id, int deflate_level, bool attr_agl=false);
      bool init_netcdf(int obs_count, int hdr_count, string program_name);

      void reset_header_buffer(int buf_size, bool reset_all);
      void set_nc_out_data(vector<Observation> observations,
                           SummaryObs *summary_obs, TimeSummaryInfo summary_info,
                           int processed_hdr_cnt=0);
      void set_using_var_id(bool using_var_id);

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

};  // MetNcPointObsOut


////////////////////////////////////////////////////////////////////////

inline int MetNcPointObsOut::get_hdr_index() { return data_buffer.cur_hdr_idx; }
inline int MetNcPointObsOut::get_obs_index() { return data_buffer.obs_data_idx; }
inline NcObsOutputData *MetNcPointObsOut::get_output_data() { return &out_data; }
inline NetcdfObsVars *MetNcPointObsOut::get_obs_vars() { return &obs_vars; }

////////////////////////////////////////////////////////////////////////


#endif   /*  __NC_POINT_OBS_OUT_H__  */


////////////////////////////////////////////////////////////////////////

