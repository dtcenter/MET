// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __MET_NC_POINT_OBS_H__
#define  __MET_NC_POINT_OBS_H__


////////////////////////////////////////////////////////////////////////


#include <ostream>

#include "observation.h"
#include "nc_utils.h"
#include "nc_obs_util.h"
#include "nc_var_info.h"


////////////////////////////////////////////////////////////////////////


class MetNcPointObs {

   private:

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

      MetNcPointObs(const MetNcPointObs &);
      MetNcPointObs & operator=(const MetNcPointObs &);

      void init_from_scratch();

   public:

      MetNcPointObs();
     ~MetNcPointObs();

      bool open(const char * filename);
      void close();
      bool check_nc(const char *nc_name, const char *caller=empty_name);
      bool read_dim_headers();
      bool read_obs_data();
      bool read_obs_data_numbers();
      bool read_obs_data_strings();
      bool set_netcdf(NcFile *nc_file, bool _keep_nc=false);

      int get_buf_size();
      //NetcdfObsVars get_obs_vars();
      int get_hdr_cnt();
      int get_obs_cnt();
      int get_qty_length();
      
      bool is_using_var_id();
      bool is_using_obs_arr();
      
      NcHeaderData get_header_data();
      bool get_header(int header_offset, float hdr_arr[HDR_ARRAY_LEN],
                      ConcatString &hdr_typ_str, ConcatString &hdr_sid_str,
                      ConcatString &hdr_vld_str);
      //int  get_hdr_arr_len();
      bool get_header_type(int header_offset, int hdr_typ_arr[HDR_TYPE_ARR_LEN]);
      bool get_lats(float *hdr_lats);
      bool get_lons(float *hdr_lons);
      //int  get_obs_arr_len();
      bool read_obs_data(int buf_size, int start, float *obs_arr_block,
                         int *obs_qty_idx_block, char *obs_qty_str_block);
      NcPointObsData get_point_obs_data();
      StringArray get_qty_data();
      StringArray get_var_names();

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


#endif   /*  __MET_NC_POINT_OBS_H__  */


////////////////////////////////////////////////////////////////////////


