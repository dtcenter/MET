// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   summary_util.cc
//
//   Description:
//      Common routines for time summary (into NetCDF).
//

using namespace std;

//#include <algorithm>
#include <iostream>

//#include "vx_math.h"
#include "vx_nc_util.h"

#include "nc_obs_util.h"

///////////////////////////////////////////////////////////////////////////////

struct NcDataBuffer nc_data_buffer;
struct NcHeaderData hdr_data;
float  hdr_arr_block[NC_BUFFER_SIZE_32K][HDR_ARRAY_LEN];

///////////////////////////////////////////////////////////////////////////////

long hdrNum;
long obsNum;

static const string err_msg_message_type =
      "error writing the message type string to the netCDF file\n\n";
static const string err_msg_station_id =
      "error writing the station id string to the netCDF file\n\n";
static const string err_msg_valid_time =
      "error writing the valid time to the netCDF file\n\n";
static const string err_msg_hdr_arr =
      "error writing the header array to the netCDF file\n\n";

///////////////////////////////////////////////////////////////////////////////

bool add_nc_header_to_array (const char *hdr_typ, const char *hdr_sid,
      const time_t hdr_vld, const float hdr_lat,
      const float hdr_lon, const float hdr_elv)
{
   bool added = false;
   bool new_vld = false;
   
   // Can't filter duplicated one because header index was
   // assigned before checking
   int hdr_index;
   if (!hdr_data.typ_array.has(hdr_typ, hdr_index)) {
      hdr_index = hdr_data.typ_array.n_elements();
      hdr_data.typ_array.add(hdr_typ);          // Message type
   }
   hdr_data.typ_idx_array.add(hdr_index);       // Index of Message type
   
   if (!hdr_data.sid_array.has(hdr_sid, hdr_index)) {
      hdr_index = hdr_data.sid_array.n_elements();
      hdr_data.sid_array.add(hdr_sid);          // Station ID
   }
   hdr_data.sid_idx_array.add(hdr_index);       // Index of Station ID
 
   if (hdr_data.min_vld_time == -1 || hdr_data.min_vld_time > hdr_vld) {
      if (hdr_data.min_vld_time == -1) hdr_data.max_vld_time = hdr_vld; 
      hdr_data.min_vld_time = hdr_vld;
      new_vld = true;
   }
   else if (hdr_data.max_vld_time < hdr_vld) {
      hdr_data.max_vld_time = hdr_vld;
      new_vld = true;
   }
   if (new_vld || !hdr_data.vld_num_array.has(hdr_vld, hdr_index)) {
      hdr_index = hdr_data.vld_array.n_elements();
      hdr_data.vld_array.add(unix_to_yyyymmdd_hhmmss(hdr_vld)); // Valid time
      hdr_data.vld_num_array.add(hdr_vld);   // Valid time
   }
   hdr_data.vld_idx_array.add(hdr_index);       // Index of Valid time
   
   hdr_data.lat_array.add(hdr_lat);  // Latitude
   hdr_data.lon_array.add(hdr_lon);  // Longitude
   hdr_data.elv_array.add(hdr_elv);  // Elevation
   nc_data_buffer.cur_hdr_idx++;
   added = true;
   return added;
}

///////////////////////////////////////////////////////////////////////////////

bool add_nc_header_prepbufr (const int pb_report_type,
      const int in_report_type, const int instrument_type)
{
   bool added = true;
   // Can't filter duplicated one because header index was
   // assigned before checking
   hdr_data.prpt_typ_array.add(pb_report_type);
   hdr_data.irpt_typ_array.add(in_report_type);
   hdr_data.inst_typ_array.add(instrument_type);
   return added;
}

////////////////////////////////////////////////////////////////////////

int check_nc_dims_vars(const NetcdfObsVars obs_vars) {
   int exit_code = exit_code_no_error;
   if(IS_INVALID_NC(obs_vars.strl_dim) ||
      IS_INVALID_NC(obs_vars.obs_dim)  ||
      IS_INVALID_NC(obs_vars.hdr_dim)) {
      exit_code = exit_code_no_dim;
   }
   else if(IS_INVALID_NC(obs_vars.hdr_typ_var) ||
      IS_INVALID_NC(obs_vars.hdr_sid_var) ||
      IS_INVALID_NC(obs_vars.hdr_vld_var)) {
      exit_code = exit_code_no_hdr_vars;
   }
   else if((IS_INVALID_NC(obs_vars.obs_arr_var) && IS_INVALID_NC(obs_vars.obs_val_var))) {
      exit_code = exit_code_no_obs_vars;
   }
   else if((IS_INVALID_NC(obs_vars.hdr_arr_var) && IS_INVALID_NC(obs_vars.hdr_lat_var))) {
      exit_code = exit_code_no_loc_vars;
   }

   return exit_code;
}

////////////////////////////////////////////////////////////////////////

void clear_header_data(NcHeaderData *my_hdr_data) {
   my_hdr_data->min_vld_time = -1;
   my_hdr_data->max_vld_time = -1;
   
   my_hdr_data->typ_array.clear();
   my_hdr_data->sid_array.clear();
   my_hdr_data->vld_array.clear();
   my_hdr_data->vld_num_array.clear();
   my_hdr_data->typ_idx_array.clear();
   my_hdr_data->sid_idx_array.clear();
   my_hdr_data->vld_idx_array.clear();
   my_hdr_data->lat_array.clear();
   my_hdr_data->lon_array.clear();
   my_hdr_data->elv_array.clear();
   my_hdr_data->prpt_typ_array.clear();
   my_hdr_data->irpt_typ_array.clear();
   my_hdr_data->inst_typ_array.clear();
}

///////////////////////////////////////////////////////////////////////////////

long count_nc_headers(vector< Observation > &observations)
{
   long nhdr = 0;
   
   string prev_header_type = "";
   string prev_station_id = "";
   time_t prev_valid_time = 0;
   double prev_latitude = bad_data_double;
   double prev_longitude = bad_data_double;
   double prev_elevation = bad_data_double;
   const string method_name = "  count_nc_headers()";

   for (vector< Observation >::iterator obs = observations.begin();
        obs != observations.end(); ++obs)
   {
      if (obs->getHeaderType() != prev_header_type    ||
          obs->getStationId()  != prev_station_id     ||
          obs->getValidTime()  != prev_valid_time     ||
          !is_eq(obs->getLatitude(),  prev_latitude)  ||
          !is_eq(obs->getLongitude(), prev_longitude) ||
          !is_eq(obs->getElevation(), prev_elevation))
      {
        nhdr++;
      
        prev_header_type = obs->getHeaderType();
        prev_station_id  = obs->getStationId();
        prev_valid_time  = obs->getValidTime();
        prev_latitude    = obs->getLatitude();
        prev_longitude   = obs->getLongitude();
        prev_elevation   = obs->getElevation();
      }
      //else mlog << Debug(7) << method_name 
      //     << "  FFF obs->getHeaderIndex(): " << obs->getHeaderIndex()
      //     << ", nhdr: " << nhdr << " count: " << count
      //     << "\n";
      obs->setHeaderIndex(nhdr-1);
   } /* endfor - obs */
   
   return nhdr;
}

///////////////////////////////////////////////////////////////////////////////

void create_nc_dimensions(NetcdfObsVars &obs_vars, NcFile *f_out) {
   const string method_name = "  create_nc_dimensions()";
   mlog << Debug(7) << method_name << "  is called" << "\n";
   // Define netCDF dimensions
   if (IS_INVALID_NC(obs_vars.strl_dim))   obs_vars.strl_dim = add_dim(f_out, nc_dim_mxstr,  HEADER_STR_LEN);
   if (IS_INVALID_NC(obs_vars.strl2_dim)) obs_vars.strl2_dim = add_dim(f_out, nc_dim_mxstr2, HEADER_STR_LEN2);
   if (IS_INVALID_NC(obs_vars.strl3_dim)) obs_vars.strl3_dim = add_dim(f_out, nc_dim_mxstr3, HEADER_STR_LEN3);
   if (IS_INVALID_NC(obs_vars.hdr_dim) && obs_vars.hdr_cnt > 0) {
      obs_vars.hdr_dim = add_dim(f_out, nc_dim_nhdr, obs_vars.hdr_cnt);
   }
   if (IS_INVALID_NC(obs_vars.obs_dim)) {
      if (obs_vars.obs_cnt > 0) obs_vars.obs_dim   = add_dim(f_out, nc_dim_nobs, obs_vars.obs_cnt);   // fixed dimension;
      else                      obs_vars.obs_dim   = add_dim(f_out, nc_dim_nobs);   // unlimited dimension;
   }
   nc_data_buffer.obs_vars = obs_vars;
}

///////////////////////////////////////////////////////////////////////////////

void create_nc_hdr_vars (NetcdfObsVars &obs_vars, NcFile *f_out,
      const int hdr_count, const int deflate_level) {
   const string method_name = "  create_nc_hdr_vars()";
   mlog << Debug(7) << method_name << "  hdr_count: " << hdr_count << "\n";
   
   // Define netCDF dimensions
   create_nc_dimensions(obs_vars, f_out);
   
   NcDim hdr_dim;
   obs_vars.hdr_cnt = hdr_count;
   if (!IS_INVALID_NC(obs_vars.hdr_dim)) {
      hdr_dim = obs_vars.hdr_dim;
   }
   else {
      hdr_dim = (hdr_count > 0)
            ? add_dim(f_out, nc_dim_nhdr, hdr_count)
            : add_dim(f_out, nc_dim_nhdr);    // unlimited dimension
      obs_vars.hdr_dim = hdr_dim;
   }

   // Define netCDF header variables
   obs_vars.hdr_typ_var = add_var(f_out, nc_var_hdr_typ,   ncInt, hdr_dim, deflate_level);
   obs_vars.hdr_sid_var = add_var(f_out, nc_var_hdr_sid,   ncInt, hdr_dim, deflate_level);
   obs_vars.hdr_vld_var = add_var(f_out, nc_var_hdr_vld,   ncInt, hdr_dim, deflate_level);
   obs_vars.hdr_lat_var = add_var(f_out, nc_var_hdr_lat, ncFloat, hdr_dim, deflate_level);
   obs_vars.hdr_lon_var = add_var(f_out, nc_var_hdr_lon, ncFloat, hdr_dim, deflate_level);
   obs_vars.hdr_elv_var = add_var(f_out, nc_var_hdr_elv, ncFloat, hdr_dim, deflate_level);

   add_att(&obs_vars.hdr_typ_var,  "long_name", "index of message type");
   add_att(&obs_vars.hdr_typ_var, "_FillValue", (int)FILL_VALUE);
   add_att(&obs_vars.hdr_sid_var,  "long_name", "index of station identification");
   add_att(&obs_vars.hdr_sid_var, "_FillValue", (int)FILL_VALUE);
   add_att(&obs_vars.hdr_vld_var,  "long_name", "index of valid time");
   add_att(&obs_vars.hdr_vld_var, "_FillValue", (int)FILL_VALUE);

   add_att(&obs_vars.hdr_lat_var, "long_name",  "latitude");
   add_att(&obs_vars.hdr_lat_var, "_FillValue", FILL_VALUE);
   add_att(&obs_vars.hdr_lat_var, "units", "degrees_north");
   add_att(&obs_vars.hdr_lon_var, "long_name",  "longitude");
   add_att(&obs_vars.hdr_lon_var, "_FillValue", FILL_VALUE);
   add_att(&obs_vars.hdr_lon_var, "units", "degrees_east");
   add_att(&obs_vars.hdr_elv_var, "long_name",  "elevation");
   add_att(&obs_vars.hdr_elv_var, "_FillValue", FILL_VALUE);
   add_att(&obs_vars.hdr_elv_var, "units", "meters above sea level (msl)");
   nc_data_buffer.obs_vars = obs_vars;
}

///////////////////////////////////////////////////////////////////////////////

NcDim create_nc_obs_var_var (NetcdfObsVars &obs_vars, NcFile *f_out,
                         int var_count, const int deflate_level) {
   NcDim var_dim  = add_dim(f_out, nc_dim_nvar, var_count);
   // If the string length is modified, update nc_tools.cc, too.
   if (IS_INVALID_NC(obs_vars.strl2_dim)) obs_vars.strl2_dim = add_dim(f_out, nc_dim_mxstr2, HEADER_STR_LEN2);
   
   obs_vars.obs_var = add_var(f_out, nc_var_obs_var, ncChar, var_dim, obs_vars.strl2_dim, deflate_level);
   add_att(&obs_vars.obs_var,  "long_name", "variable names");
   return var_dim;
}

///////////////////////////////////////////////////////////////////////////////

void create_nc_obs_vars (NetcdfObsVars &obs_vars, NcFile *f_out,
                         const int deflate_level, bool use_var_id) {
   const char *long_name_str;
   const string method_name = "  create_nc_obs_vars()";

   // Define netCDF dimensions
   create_nc_dimensions(obs_vars, f_out);
   
   // Define netCDF variables
   obs_vars.obs_qty_var = add_var(f_out, nc_var_obs_qty,   ncInt, obs_vars.obs_dim, deflate_level);
   obs_vars.obs_hid_var = add_var(f_out, nc_var_obs_hid,   ncInt, obs_vars.obs_dim, deflate_level);
   if (use_var_id)
      obs_vars.obs_vid_var = add_var(f_out, nc_var_obs_vid,   ncInt, obs_vars.obs_dim, deflate_level);
   else
      obs_vars.obs_gc_var  = add_var(f_out, nc_var_obs_gc,    ncInt, obs_vars.obs_dim, deflate_level);
   obs_vars.obs_lvl_var = add_var(f_out, nc_var_obs_lvl, ncFloat, obs_vars.obs_dim, deflate_level);
   obs_vars.obs_hgt_var = add_var(f_out, nc_var_obs_hgt, ncFloat, obs_vars.obs_dim, deflate_level);
   obs_vars.obs_val_var = add_var(f_out, nc_var_obs_val, ncFloat, obs_vars.obs_dim, deflate_level);

   add_att(f_out, nc_att_obs_version, MET_NC_Obs_version);
   add_att(f_out, nc_att_use_var_id, (use_var_id ? "true" : "false"));

   // Add variable attributes
   add_att(&obs_vars.obs_qty_var,  "long_name", "index of quality flag");
   add_att(&obs_vars.obs_hid_var,  "long_name", "index of matching header data");
   add_att(&obs_vars.obs_hid_var, "_FillValue", (int)FILL_VALUE);
   add_att(&obs_vars.obs_val_var,  "long_name", "observation value");
   add_att(&obs_vars.obs_val_var, "_FillValue", FILL_VALUE);
   if (use_var_id) {
      long_name_str = (obs_vars.attr_pb2nc
            ? "index of BUFR variable corresponding to the observation type"
            : "index of variable names at var_name");
      add_att(&obs_vars.obs_vid_var,  "long_name", long_name_str);
      add_att(&obs_vars.obs_vid_var, "_FillValue", (int)FILL_VALUE);
   }
   else {
      add_att(&obs_vars.obs_gc_var,  "long_name", "grib code corresponding to the observation type");
      add_att(&obs_vars.obs_gc_var, "_FillValue", (int)FILL_VALUE);
   }

   add_att(&obs_vars.obs_lvl_var,  "long_name", "pressure level (hPa) or accumulation interval (sec)");
   add_att(&obs_vars.obs_lvl_var, "_FillValue", FILL_VALUE);
   long_name_str = (obs_vars.attr_agl)
         ? "height in meters above sea level or ground level (msl or agl)"
         : "height in meters above sea level (msl)";
   add_att(&obs_vars.obs_hgt_var,  "long_name", long_name_str);
   add_att(&obs_vars.obs_hgt_var, "_FillValue", FILL_VALUE);
   nc_data_buffer.obs_vars = obs_vars;
}

///////////////////////////////////////////////////////////////////////////////

void create_nc_obs_name_vars (NetcdfObsVars &obs_vars, NcFile *f_out, 
      const int var_count, const int unit_count, const int deflate_level) {
   const string method_name = "  create_nc_other_vars()";
   
   if (var_count > 0) {
      NcDim var_dim = create_nc_obs_var_var(obs_vars, f_out, var_count, deflate_level);
      if (unit_count > 0) {
         obs_vars.unit_var = add_var(f_out,    nc_var_unit, ncChar, var_dim, obs_vars.strl2_dim, deflate_level);
         obs_vars.desc_var = add_var(f_out,    nc_var_desc, ncChar, var_dim, obs_vars.strl3_dim, deflate_level);
         
         add_att(&obs_vars.unit_var, "long_name", "variable units");
         add_att(&obs_vars.desc_var, "long_name", "variable descriptions");
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

void create_nc_table_vars (NetcdfObsVars &obs_vars, NcFile *f_out, const int deflate_level) {
   const string method_name = "  create_nc_table_vars()";
   
   // Define netCDF dimensions
   NcDim hdr_typ_dim = add_dim(f_out, nc_dim_nhdr_typ, hdr_data.typ_array.n_elements());
   NcDim hdr_sid_dim = add_dim(f_out, nc_dim_nhdr_sid, hdr_data.sid_array.n_elements());
   NcDim hdr_vld_dim = add_dim(f_out, nc_dim_nhdr_vld, hdr_data.vld_array.n_elements());
   NcDim obs_qty_dim = add_dim(f_out, nc_dim_nqty,     nc_data_buffer.qty_data_array.n_elements());

   // Define netCDF header variables
   obs_vars.hdr_typ_tbl_var = add_var(f_out, nc_var_hdr_typ_tbl, ncChar, hdr_typ_dim, obs_vars.strl2_dim, deflate_level);
   obs_vars.hdr_sid_tbl_var = add_var(f_out, nc_var_hdr_sid_tbl, ncChar, hdr_sid_dim, obs_vars.strl2_dim, deflate_level);
   obs_vars.hdr_vld_tbl_var = add_var(f_out, nc_var_hdr_vld_tbl, ncChar, hdr_vld_dim, obs_vars.strl_dim,  deflate_level);
   obs_vars.obs_qty_tbl_var = add_var(f_out, nc_var_obs_qty_tbl, ncChar, obs_qty_dim, obs_vars.strl_dim,  deflate_level);

   add_att(&obs_vars.obs_qty_tbl_var, "long_name", "quality flag");
   add_att(&obs_vars.hdr_typ_tbl_var, "long_name", "message type");
   add_att(&obs_vars.hdr_sid_tbl_var, "long_name", "station identification");
   add_att(&obs_vars.hdr_vld_tbl_var, "long_name", "valid time");
   add_att(&obs_vars.hdr_vld_tbl_var, "units", "YYYYMMDD_HHMMSS UTC");

}

////////////////////////////////////////////////////////////////////////

void create_nc_pb_hdrs (NetcdfObsVars &obs_vars, NcFile *f_out,
      const int hdr_count, const int deflate_level) {
   const string method_name = "  create_nc_pb_hdrs()";
   mlog << Debug(7) << method_name << "  hdr_count: " << hdr_count << "\n";
   
   // Define netCDF dimensions
   if (IS_INVALID_NC(obs_vars.pb_hdr_dim)) obs_vars.pb_hdr_dim = add_dim(f_out, nc_dim_npbhdr, hdr_count);
   
   obs_vars.raw_hdr_cnt = hdr_count;
   obs_vars.hdr_prpt_typ_var = add_var(f_out, nc_var_hdr_prpt_typ, ncInt, obs_vars.pb_hdr_dim, deflate_level);
   obs_vars.hdr_irpt_typ_var = add_var(f_out, nc_var_hdr_irpt_typ, ncInt, obs_vars.pb_hdr_dim, deflate_level);
   obs_vars.hdr_inst_typ_var = add_var(f_out, nc_var_hdr_inst_typ, ncInt, obs_vars.pb_hdr_dim, deflate_level);
   add_att(&obs_vars.hdr_prpt_typ_var, "long_name",  "PB report type");
   add_att(&obs_vars.hdr_prpt_typ_var, "_FillValue", (int)FILL_VALUE);
   add_att(&obs_vars.hdr_irpt_typ_var, "long_name",  "In report type");
   add_att(&obs_vars.hdr_irpt_typ_var, "_FillValue", (int)FILL_VALUE);
   add_att(&obs_vars.hdr_inst_typ_var, "long_name",  "instrument type");
   add_att(&obs_vars.hdr_inst_typ_var, "_FillValue", (int)FILL_VALUE);
}


////////////////////////////////////////////////////////////////////////

NcDataBuffer *get_nc_data_buffer() {
   return &nc_data_buffer;
}

////////////////////////////////////////////////////////////////////////

NcHeaderData *get_hdr_data_buffer() {
   return &hdr_data;
}

////////////////////////////////////////////////////////////////////////

NcHeaderData get_nc_hdr_data(NetcdfObsVars obs_vars) {
   NcHeaderData my_hdr_data;
   long nhdr_count  = get_dim_size(&obs_vars.hdr_dim);
   int  strl_len    = get_dim_size(&obs_vars.strl_dim);
   int  strl2_len   = strl_len;
   int  typ_len = strl_len;
   int  sid_len = strl_len;
   int  vld_len = strl_len;
   int  hdr_arr_len = IS_INVALID_NC(obs_vars.hdr_arr_dim)
         ? 0 : get_dim_size(&obs_vars.hdr_arr_dim);
   bool has_array_vars = IS_INVALID_NC(obs_vars.hdr_typ_tbl_var);

   if (!IS_INVALID_NC(obs_vars.strl2_dim)) {
      NcDim str_dim;
      strl2_len = get_dim_size(&obs_vars.strl2_dim);
      string dim_name = GET_NC_NAME(obs_vars.strl2_dim);
      str_dim = get_nc_dim((IS_INVALID_NC(obs_vars.hdr_typ_tbl_var)
            ? &obs_vars.hdr_typ_var : &obs_vars.hdr_typ_tbl_var), dim_name);
      if (!IS_INVALID_NC(str_dim)) typ_len = strl2_len;
      
      str_dim = get_nc_dim((IS_INVALID_NC(obs_vars.hdr_sid_tbl_var)
            ? &obs_vars.hdr_sid_var : &obs_vars.hdr_sid_tbl_var), dim_name);
      if (!IS_INVALID_NC(str_dim)) sid_len = strl2_len;
      
      str_dim = get_nc_dim((IS_INVALID_NC(obs_vars.hdr_vld_tbl_var)
            ? &obs_vars.hdr_vld_var : &obs_vars.hdr_vld_tbl_var), dim_name);
      if (!IS_INVALID_NC(str_dim)) vld_len = strl2_len;
   }
   
   my_hdr_data.typ_len   = typ_len;
   my_hdr_data.sid_len   = sid_len;
   my_hdr_data.vld_len   = vld_len;
   my_hdr_data.strl_len  = strl_len;
   my_hdr_data.strll_len = strl2_len;
   
   int buf_size = ((nhdr_count > NC_BUFFER_SIZE_32K)
        ? NC_BUFFER_SIZE_32K : (nhdr_count));
   
   //
   // Allocate space to store the data
   //
   char hdr_typ_block[buf_size][typ_len];
   char hdr_sid_block[buf_size][sid_len];
   char hdr_vld_block[buf_size][vld_len];
   int  *hdr_typ_idx_block = new int[buf_size];
   int  *hdr_sid_idx_block = new int[buf_size];
   int  *hdr_vld_idx_block = new int[buf_size];
   float *hdr_lat_block    = new float[buf_size];
   float *hdr_lon_block    = new float[buf_size];
   float *hdr_elv_block    = new float[buf_size];

   long offsets[2] = { 0, 0 };
   long lengths[2] = { 1, 1 };
   long offsets_1D[1] = { 0 };
   long lengths_1D[1] = { 1 };
   
   //lengths[0] = buf_size;
   //lengths[1] = strl_len;
   for(int i_start=0; i_start<nhdr_count; i_start+=buf_size) {
      buf_size = ((nhdr_count-i_start) > NC_BUFFER_SIZE_32K)
            ? NC_BUFFER_SIZE_32K : (nhdr_count-i_start);
      
      offsets[0] = i_start;
      lengths[0] = buf_size;
      offsets_1D[0] = i_start;
      lengths_1D[0] = buf_size;
   
      //
      // Get the corresponding header message type
      //
      if (has_array_vars) {
         lengths[1] = typ_len;
         if(!get_nc_data(&obs_vars.hdr_typ_var,
               (char *)&hdr_typ_block[0], lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_typ\n\n";
            exit(1);
         }
      
         //
         // Get the corresponding header station id
         //
         lengths[1] = sid_len;
         if(!get_nc_data(&obs_vars.hdr_sid_var,
               (char *)&hdr_sid_block[0], lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_sid\n\n";
            exit(1);
         }
         
         //
         // Get the corresponding header valid time
         //
         lengths[1] = vld_len;
         if(!get_nc_data(&obs_vars.hdr_vld_var,
               (char *)&hdr_vld_block[0], lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_vld\n\n";
            exit(1);
         }
         
         //
         // Get the header for this observation
         //
         lengths[1] = hdr_arr_len;
         if(!get_nc_data(&obs_vars.hdr_arr_var,
               (float *)&hdr_arr_block[0], lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_arr\n\n";
            exit(1);
         }
         for (int hIndex = 0; hIndex < buf_size; hIndex++) {
            my_hdr_data.typ_array.add(hdr_typ_block[hIndex]);
            my_hdr_data.sid_array.add(hdr_sid_block[hIndex]);
            my_hdr_data.vld_array.add(hdr_vld_block[hIndex]);
            my_hdr_data.lat_array.add(hdr_arr_block[hIndex][0]);
            my_hdr_data.lon_array.add(hdr_arr_block[hIndex][1]);
            my_hdr_data.elv_array.add(hdr_arr_block[hIndex][2]);
         }
      }
      else {
         // Get the corresponding header message type (index, not string)
         if(!get_nc_data(&obs_vars.hdr_typ_var,
               hdr_typ_idx_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_typ\n\n";
            exit(1);
         }
      
         // Get the corresponding header station id (index, not string)
         if(!get_nc_data(&obs_vars.hdr_sid_var,
               hdr_sid_idx_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_sid\n\n";
            exit(1);
         }
         
         // Get the corresponding header valid time (index, not string)
         if(!get_nc_data(&obs_vars.hdr_vld_var,
               hdr_vld_idx_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_vld\n\n";
            exit(1);
         }
         
         //
         // Get the header for this observation
         //
         if(!get_nc_data(&obs_vars.hdr_lat_var,
               hdr_lat_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_lat\n\n";
            exit(1);
         }
         if(!get_nc_data(&obs_vars.hdr_lon_var,
               hdr_lon_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_lon\n\n";
            exit(1);
         }
         if(!get_nc_data(&obs_vars.hdr_elv_var,
               hdr_elv_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_elv\n\n";
            exit(1);
         }
         for (int hIndex = 0; hIndex < buf_size; hIndex++) {
            my_hdr_data.typ_idx_array.add(hdr_typ_idx_block[hIndex]);
            my_hdr_data.sid_idx_array.add(hdr_sid_idx_block[hIndex]);
            my_hdr_data.vld_idx_array.add(hdr_vld_idx_block[hIndex]);
            my_hdr_data.lat_array.add(hdr_lat_block[hIndex]);
            my_hdr_data.lon_array.add(hdr_lon_block[hIndex]);
            my_hdr_data.elv_array.add(hdr_elv_block[hIndex]);
         }
      }
   }

   delete[] hdr_typ_idx_block;
   delete[] hdr_sid_idx_block;
   delete[] hdr_vld_idx_block;
   delete[] hdr_lat_block;
   delete[] hdr_lon_block;
   delete[] hdr_elv_block;
   
   if (!has_array_vars) {
      int tmp_dim_size;
      
      lengths[1] = typ_len;
      tmp_dim_size = get_dim_size(&obs_vars.hdr_typ_dim);
      buf_size = ((tmp_dim_size > NC_BUFFER_SIZE_32K)
           ? NC_BUFFER_SIZE_32K : (tmp_dim_size));
      for(int i_start=0; i_start<tmp_dim_size; i_start+=buf_size) {
         buf_size = ((tmp_dim_size-i_start) > NC_BUFFER_SIZE_32K)
               ? NC_BUFFER_SIZE_32K : (tmp_dim_size-i_start);
         offsets[0] = i_start;
         lengths[0] = buf_size;
      
         // Get the corresponding header message type (string)
         if(!get_nc_data(&obs_vars.hdr_typ_tbl_var,
               (char *)&hdr_typ_block[0], lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_typ\n\n";
            exit(1);
         }
         for (int hIndex = 0; hIndex < buf_size; hIndex++) {
            my_hdr_data.typ_array.add(hdr_typ_block[hIndex]);
         }
      }
      
      lengths[1] = sid_len;
      tmp_dim_size = get_dim_size(&obs_vars.hdr_sid_dim);
      buf_size = ((tmp_dim_size > NC_BUFFER_SIZE_32K)
           ? NC_BUFFER_SIZE_32K : (tmp_dim_size));
      for(int i_start=0; i_start<tmp_dim_size; i_start+=buf_size) {
         buf_size = ((tmp_dim_size-i_start) > NC_BUFFER_SIZE_32K)
               ? NC_BUFFER_SIZE_32K : (tmp_dim_size-i_start);
         offsets[0] = i_start;
         lengths[0] = buf_size;
      
         // Get the corresponding header station id (string)
         if(!get_nc_data(&obs_vars.hdr_sid_tbl_var,
               (char *)&hdr_sid_block[0], lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_typ\n\n";
            exit(1);
         }
         for (int hIndex = 0; hIndex < buf_size; hIndex++) {
            my_hdr_data.sid_array.add(hdr_sid_block[hIndex]);
         }
      }

      lengths[1] = vld_len;
      tmp_dim_size = get_dim_size(&obs_vars.hdr_vld_dim);
      buf_size = ((tmp_dim_size > NC_BUFFER_SIZE_32K)
            ? NC_BUFFER_SIZE_32K : (tmp_dim_size));
      for(int i_start=0; i_start<tmp_dim_size; i_start+=buf_size) {
         buf_size = ((tmp_dim_size-i_start) > NC_BUFFER_SIZE_32K)
               ? NC_BUFFER_SIZE_32K : (tmp_dim_size-i_start);
         offsets[0] = i_start;
         lengths[0] = buf_size;
      
         // Get the corresponding header valid time (string)
         if(!get_nc_data(&obs_vars.hdr_vld_tbl_var,
               (char *)&hdr_vld_block[0], lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_typ\n\n";
            exit(1);
         }
         for (int hIndex = 0; hIndex < buf_size; hIndex++) {
            my_hdr_data.vld_array.add(hdr_vld_block[hIndex]);
         }
      }
   }
   if (!IS_INVALID_NC(obs_vars.pb_hdr_dim)) {
      get_nc_pb_hdr_data(obs_vars, &my_hdr_data);
      int last_prpt_typ = my_hdr_data.prpt_typ_array.n_elements() - 1;
      int last_irpt_typ = my_hdr_data.irpt_typ_array.n_elements() - 1; 
      int last_inst_typ = my_hdr_data.inst_typ_array.n_elements() - 1; 
      mlog << Debug(10)
           << "    prpt_typ[0,-1] " << my_hdr_data.prpt_typ_array[0]
           << "," << my_hdr_data.prpt_typ_array[last_prpt_typ]
           << "    irpt_typ[0,-1] " << my_hdr_data.irpt_typ_array[0]
           << "," << my_hdr_data.irpt_typ_array[last_irpt_typ]
           << "    inst_typ[0,-1] " << my_hdr_data.inst_typ_array[0]
           << "," << my_hdr_data.inst_typ_array[last_inst_typ] << "\n";
   }
   return my_hdr_data;
}

////////////////////////////////////////////////////////////////////////

int get_nc_hdr_cur_index() {
   //return (nc_data_buffer.cur_hdr_idx < 0) ? 0 : nc_data_buffer.cur_hdr_idx;
   return nc_data_buffer.cur_hdr_idx;
}

////////////////////////////////////////////////////////////////////////

int get_nc_obs_buf_index() {
   return nc_data_buffer.obs_data_idx;
}

////////////////////////////////////////////////////////////////////////

void get_nc_pb_hdr_data(NetcdfObsVars obs_vars, NcHeaderData *my_hdr_data) {

   long offsets[1] = { 0 };
   long lengths[1] = { 1 };
   int pb_hdr_count = get_dim_size(&obs_vars.pb_hdr_dim);
   
   // Read PB report type
   int buf_size = ((pb_hdr_count > NC_BUFFER_SIZE_32K)
         ? NC_BUFFER_SIZE_32K : (pb_hdr_count));
   int *hdr_prpt_typ_block = new int[buf_size];
   int *hdr_irpt_typ_block = new int[buf_size];
   int *hdr_inst_typ_block = new int[buf_size];
   for(int i_start=0; i_start<pb_hdr_count; i_start+=buf_size) {
      buf_size = pb_hdr_count - i_start;
      if (buf_size > NC_BUFFER_SIZE_32K) buf_size = NC_BUFFER_SIZE_32K;
      offsets[0] = i_start;
      lengths[0] = buf_size;
      
      if (!IS_INVALID_NC(obs_vars.hdr_prpt_typ_var)) {
         // Get the corresponding header PB message type (string)
         if(!get_nc_data(&obs_vars.hdr_prpt_typ_var,
               hdr_prpt_typ_block, lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_prpt_typ\n\n";
            exit(1);
         }
      }
         
      if (!IS_INVALID_NC(obs_vars.hdr_irpt_typ_var)) {
         // Get the corresponding header In message type (string)
         if(!get_nc_data(&obs_vars.hdr_irpt_typ_var,
               hdr_irpt_typ_block, lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_irpt_typ\n\n";
            exit(1);
         }
      }
         
      if (!IS_INVALID_NC(obs_vars.hdr_inst_typ_var)) {
         // Get the corresponding header instrument type (string)
         if(!get_nc_data(&obs_vars.hdr_inst_typ_var,
               hdr_inst_typ_block, lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_inst_typ\n\n";
            exit(1);
         }
      }
         
      for (int hIndex = 0; hIndex < buf_size; hIndex++) {
         my_hdr_data->prpt_typ_array.add(hdr_prpt_typ_block[hIndex]);
         my_hdr_data->irpt_typ_array.add(hdr_irpt_typ_block[hIndex]);
         my_hdr_data->inst_typ_array.add(hdr_inst_typ_block[hIndex]);
      }
   }

   delete[] hdr_prpt_typ_block;
   delete[] hdr_irpt_typ_block;
   delete[] hdr_inst_typ_block;

}

///////////////////////////////////////////////////////////////////////////////

void init_nc_dims_vars_config(NetcdfObsVars &obs_vars, bool use_var_id) {
   obs_vars.attr_agl    = false;
   obs_vars.attr_pb2nc  = false;
   obs_vars.use_var_id  = use_var_id;
   obs_vars.hdr_cnt     = 0;     // header array length (fixed dimension if hdr_cnt > 0)
   obs_vars.obs_cnt     = 0;     // obs. array length (fixed dimension if obs_cnt > 0)
   //obs_vars.hdr_str_len = 0;    // string length for header (message) type header
   nc_data_buffer.obs_vars = obs_vars;
}

////////////////////////////////////////////////////////////////////////

bool is_using_var_id(const char * nc_name) {
   bool use_var_id = false;
   ConcatString attr_name = nc_att_use_var_id;
   if (!get_global_att(nc_name, nc_att_use_var_id, use_var_id)) {
      use_var_id = false;
   }
   return use_var_id;
}

///////////////////////////////////////////////////////////////////////////////

void nc_obs_initialize() {
   nc_data_buffer.obs_data_idx    = 0;
   nc_data_buffer.obs_data_offset = 0;
   nc_data_buffer.hdr_data_idx    = 0;
   nc_data_buffer.hdr_data_offset = 0;
   nc_data_buffer.pb_hdr_data_offset = 0;

   strcpy(nc_data_buffer.prev_hdr_typ_buf, "NotDefined");
   strcpy(nc_data_buffer.prev_hdr_sid_buf, "NotDefined");
   strcpy(nc_data_buffer.prev_hdr_vld_buf, "NotDefined");
   for (int index=0; index<HDR_ARRAY_LEN; index++)
      nc_data_buffer.prev_hdr_arr_buf[index] = 0.0;
   
   clear_header_data(&hdr_data);
}

////////////////////////////////////////////////////////////////////////

bool read_nc_obs_data(NetcdfObsVars obs_vars, int buf_size, int offset,
      int qty_len, float *obs_arr, int *qty_idx_arr, char *obs_qty_buf) {
   bool result = true;
   long offsets[2] = { offset, 0 };
   long lengths[2] = { buf_size, 1 };
   
   if (!IS_INVALID_NC(obs_vars.obs_arr_var)) {
      // Read the current observation message
      lengths[1] = OBS_ARRAY_LEN;
      if(!get_nc_data(&obs_vars.obs_arr_var, (float *)obs_arr, lengths, offsets)) {
         mlog << Error << "\nread_nc_obs_array() -> trouble getting obs_arr\n\n";
         result = false;
      }

      lengths[1] = qty_len;
      if(!get_nc_data(&obs_vars.obs_qty_var, obs_qty_buf, lengths, offsets)) {
         mlog << Error << "\nprocess_point_obs() -> trouble getting obs_qty\n\n";
         result = false;
      }
   }
   else {
      int   *obs_hid_buf = new   int[buf_size];
      int   *obs_vid_buf = new   int[buf_size];
      float *obs_lvl_buf = new float[buf_size];
      float *obs_hgt_buf = new float[buf_size];
      float *obs_val_buf = new float[buf_size];
      
      lengths[1] = 1;
      
      if(!get_nc_data(&obs_vars.obs_hid_var, obs_hid_buf, lengths, offsets)) {
         mlog << Error << "\nread_nc_obs_array() -> "
              << "can't read the record for observation "
              << "index " << offset << "\n\n";
         result = false;
      }
      if(!get_nc_data((IS_INVALID_NC(obs_vars.obs_gc_var) ? &obs_vars.obs_vid_var : &obs_vars.obs_gc_var),
            obs_vid_buf, lengths, offsets)) {
         mlog << Error << "\nread_nc_obs_array() -> "
              << "can't read the record (vid or gc) for observation "
              << "index " << offset << "\n\n";
         result = false;
      }
      if(!get_nc_data(&obs_vars.obs_lvl_var, obs_lvl_buf, lengths, offsets)) {
         mlog << Error << "\nread_nc_obs_array() -> "
              << "can't read the record (lvl) for observation "
              << "index " << offset << "\n\n";
         result = false;
      }
      if(!get_nc_data(&obs_vars.obs_hgt_var, obs_hgt_buf, lengths, offsets)) {
         mlog << Error << "\nread_nc_obs_array() -> "
              << "can't read the record (hgt) for observation "
              << "index " << offset << "\n\n";
         result = false;
      }
      if(!get_nc_data(&obs_vars.obs_val_var, obs_val_buf, lengths, offsets)) {
         mlog << Error << "\nread_nc_obs_array() -> "
              << "can't read the record (val) for observation "
              << "index " << offset << "\n\n";
         result = false;
      }

      if (!get_nc_data(&obs_vars.obs_qty_var, qty_idx_arr, lengths, offsets)) {
         mlog << Error << "\nread_nc_obs_array() -> "
              << "can't read the index of quality flag for observation "
              << "index " << offset << "\n\n";
         result = false;
      }
      
      if (result) {
         float *tmp_obs_arr = obs_arr;
         for(int index=0; index<buf_size; index++) {
            *tmp_obs_arr++ = (float)obs_hid_buf[index];
            *tmp_obs_arr++ = (float)obs_vid_buf[index];
            *tmp_obs_arr++ = obs_lvl_buf[index];
            *tmp_obs_arr++ = obs_hgt_buf[index];
            *tmp_obs_arr++ = obs_val_buf[index];
         }
      }
   
      delete[] obs_hid_buf;
      delete[] obs_vid_buf;
      delete[] obs_lvl_buf;
      delete[] obs_hgt_buf;
      delete[] obs_val_buf;
   }
   return result;
}

///////////////////////////////////////////////////////////////////////////////

void read_nc_dims_vars(NetcdfObsVars &obs_vars, NcFile *f_in) {
   
   NcVar ncVar;
   // Define netCDF dimensions
   //obs_vars.hdr_cnt     ; // header array length (fixed dimension if hdr_cnt > 0)
   obs_vars.strl_dim    = get_nc_dim(f_in, nc_dim_mxstr);       // header string dimension
   if (has_dim(f_in, nc_dim_mxstr2))
      obs_vars.strl2_dim   = get_nc_dim(f_in, nc_dim_mxstr2);   // header string dimension (bigger dimension)
   if (has_dim(f_in, nc_dim_mxstr3))
      obs_vars.strl3_dim   = get_nc_dim(f_in, nc_dim_mxstr3);   // header string dimension (bigger dimension)

   if (has_dim(f_in, nc_dim_hdr_arr)) {
      obs_vars.hdr_arr_dim = get_nc_dim(f_in, nc_dim_hdr_arr);  // Header array width
      obs_vars.obs_arr_dim = get_nc_dim(f_in, nc_dim_obs_arr);  // Observation array width
   }
   else {
      if (has_dim(f_in, nc_dim_nhdr_typ))
         obs_vars.hdr_typ_dim = get_nc_dim(f_in, nc_dim_nhdr_typ); // header dimension for message type
      if (has_dim(f_in, nc_dim_nhdr_sid))
         obs_vars.hdr_sid_dim = get_nc_dim(f_in, nc_dim_nhdr_sid); // header dimension for station id
      if (has_dim(f_in, nc_dim_nhdr_vld))
         obs_vars.hdr_vld_dim = get_nc_dim(f_in, nc_dim_nhdr_vld); // header dimension for valid time
      if (has_dim(f_in, nc_dim_npbhdr))
         obs_vars.pb_hdr_dim  = get_nc_dim(f_in, nc_dim_npbhdr);   // header dimension for PB headers
   }
   obs_vars.obs_dim     = get_nc_dim(f_in, nc_dim_nobs);     // Observation array length
   obs_vars.hdr_dim     = get_nc_dim(f_in, nc_dim_nhdr);     // Header array length

   // Get netCDF header variables
   obs_vars.hdr_typ_var = get_var(f_in, nc_var_hdr_typ);     // Message type (String or int)
   obs_vars.hdr_sid_var = get_var(f_in, nc_var_hdr_sid);     // Station ID (String or int)
   obs_vars.hdr_vld_var = get_var(f_in, nc_var_hdr_vld);     // Valid time (String or int)
   
   ncVar = get_var(f_in, nc_var_hdr_lat);
   if (IS_INVALID_NC(ncVar)) {
      obs_vars.hdr_arr_var = get_var(f_in, nc_var_hdr_arr);     // Header array
   } else {
      obs_vars.hdr_lat_var = ncVar;                                  // Header array 
      obs_vars.hdr_lon_var = get_var(f_in, nc_var_hdr_lon);          // Header array 
      obs_vars.hdr_elv_var = get_var(f_in, nc_var_hdr_elv);          // Header array 
      obs_vars.hdr_typ_tbl_var = get_var(f_in, nc_var_hdr_typ_tbl);  // Message type (String)
      obs_vars.hdr_sid_tbl_var = get_var(f_in, nc_var_hdr_sid_tbl);  // Station ID (String)
      obs_vars.hdr_vld_tbl_var = get_var(f_in, nc_var_hdr_vld_tbl);  // Valid time (String)
   }

   // Get netCDF variables
   ncVar = get_var(f_in, nc_var_obs_hid);
   if (IS_INVALID_NC(ncVar)) {
      obs_vars.obs_arr_var = get_var(f_in, nc_var_obs_arr);
   } else {
      obs_vars.obs_hid_var = ncVar;                             // Obs. header id array 
      ncVar = get_var(f_in, nc_var_obs_gc);
      if (!IS_INVALID_NC(ncVar)) obs_vars.obs_gc_var  = ncVar;  // Obs. grib code array 
      ncVar = get_var(f_in, nc_var_obs_vid);
      if (!IS_INVALID_NC(ncVar)) obs_vars.obs_vid_var = ncVar;  // Obs. variable id array 
      obs_vars.obs_lvl_var = get_var(f_in, nc_var_obs_lvl);     // Obs. pressure level array 
      obs_vars.obs_hgt_var = get_var(f_in, nc_var_obs_hgt);     // Obs. highth array 
      obs_vars.obs_val_var = get_var(f_in, nc_var_obs_val);     // Obs. value array 
   }
   ncVar = get_var(f_in, nc_var_obs_qty);
   if (!IS_INVALID_NC(ncVar)) obs_vars.obs_qty_var = ncVar;
   ncVar = get_var(f_in, nc_var_obs_qty_tbl);
   if (!IS_INVALID_NC(ncVar)) obs_vars.obs_qty_tbl_var = ncVar;

   // PrepBufr only headers
   ncVar = get_var(f_in, nc_var_hdr_prpt_typ);
   if (!IS_INVALID_NC(ncVar)) obs_vars.hdr_prpt_typ_var = ncVar;
   ncVar = get_var(f_in, nc_var_hdr_irpt_typ);
   if (!IS_INVALID_NC(ncVar)) obs_vars.hdr_irpt_typ_var = ncVar;
   ncVar = get_var(f_in, nc_var_hdr_inst_typ);
   if (!IS_INVALID_NC(ncVar)) obs_vars.hdr_inst_typ_var = ncVar;
   
   bool use_var_id = false;
   if (!get_global_att(f_in, nc_att_use_var_id, use_var_id)) use_var_id = false;

   obs_vars.use_var_id = use_var_id;
   nc_data_buffer.obs_vars = obs_vars;
}

///////////////////////////////////////////////////////////////////////////////

void reset_header_buffer(int buf_size, bool reset_all) {
   for (int i=0; i<buf_size; i++) {
      for (int j=0; j<HEADER_STR_LEN; j++) {
         nc_data_buffer.hdr_typ_str_buf[i][j] = bad_data_char;
         nc_data_buffer.hdr_sid_str_buf[i][j] = bad_data_char;
         nc_data_buffer.hdr_vld_str_buf[i][j] = bad_data_char;
      }
      for (int j=HEADER_STR_LEN; j<HEADER_STR_LEN2; j++) {
         nc_data_buffer.hdr_typ_str_buf[i][j] = bad_data_char;
         nc_data_buffer.hdr_sid_str_buf[i][j] = bad_data_char;
      }
      for (int j=0; j<HDR_ARRAY_LEN; j++) {
         nc_data_buffer.hdr_arr_buf[i][j] = FILL_VALUE;
      }
      
      nc_data_buffer.hdr_lat_buf[i] = FILL_VALUE;
      nc_data_buffer.hdr_lon_buf[i] = FILL_VALUE;
      nc_data_buffer.hdr_elv_buf[i] = FILL_VALUE;
   }
   
   if (reset_all) {
      nc_data_buffer.cur_hdr_idx = 0;
      
      hdr_data.min_vld_time = -1;
      hdr_data.max_vld_time = -1;
      hdr_data.typ_array.clear();
      hdr_data.sid_array.clear();
      hdr_data.vld_array.clear();
      hdr_data.vld_num_array.clear();
      hdr_data.typ_idx_array.clear();
      hdr_data.sid_idx_array.clear();
      hdr_data.vld_idx_array.clear();
      hdr_data.lat_array.clear();
      hdr_data.lon_array.clear();
      hdr_data.elv_array.clear();
      hdr_data.prpt_typ_array.clear();
      hdr_data.irpt_typ_array.clear();
      hdr_data.inst_typ_array.clear();
   }
}

///////////////////////////////////////////////////////////////////////////////

void write_header_to_nc(const NetcdfObsVars &obs_vars,
      NcDataBuffer &data_buf, const int buf_size, const bool is_pb)
{
   long offsets[2] = { data_buf.hdr_data_offset, 0 };
   long lengths[1] = { buf_size } ;
   const string method_name = "  write_header_to_nc()";

   mlog << Debug(7) << method_name << "  buf_size: " << buf_size << "\n";
   
   //lengths[1] = HEADER_STR_LEN2;
   if(!put_nc_data((NcVar *)&obs_vars.hdr_typ_var, (int *)data_buf.hdr_typ_buf, lengths, offsets)) {
      mlog << Error << err_msg_message_type;
      exit(1);
   }
   
   // Station ID
   if(!put_nc_data((NcVar *)&obs_vars.hdr_sid_var, (int *)data_buf.hdr_sid_buf, lengths, offsets)) {
      mlog << Error << err_msg_station_id;
      exit(1);
   }
   
   // Valid Time
   if(!put_nc_data((NcVar *)&obs_vars.hdr_vld_var, (int *)data_buf.hdr_vld_buf, lengths, offsets)) {
      mlog << Error << err_msg_valid_time;
      exit(1);
   }
   
   // Write the header array which consists of the following:
   //    LAT LON ELV
   
   if(!put_nc_data((NcVar *)&obs_vars.hdr_lat_var, (float *)data_buf.hdr_lat_buf, lengths, offsets)) {
      mlog << Error << err_msg_hdr_arr;
      exit(1);
   }
   if(!put_nc_data((NcVar *)&obs_vars.hdr_lon_var, (float *)data_buf.hdr_lon_buf, lengths, offsets)) {
      mlog << Error << err_msg_hdr_arr;
      exit(1);
   }
   if(!put_nc_data((NcVar *)&obs_vars.hdr_elv_var, (float *)data_buf.hdr_elv_buf, lengths, offsets)) {
      mlog << Error << err_msg_hdr_arr;
      exit(1);
   }
   
   //for(int hi=0; hi<buf_size; hi++) {
   //   for(int hj=0; hj<HEADER_STR_LEN; hj++) {
   //      hdr_typ_buf[hi][hj] = bad_data_char;
   //      hdr_sid_buf[hi][hj] = bad_data_char;
   //   }
   //}
   if (is_pb && data_buf.hdr_data_offset == data_buf.pb_hdr_data_offset) {
      int save_len = lengths[0];
      int pb_hdr_len = obs_vars.raw_hdr_cnt - offsets[0];
      if (pb_hdr_len > buf_size) pb_hdr_len = buf_size;
      
      lengths[0] = pb_hdr_len;
      if(!put_nc_data((NcVar *)&obs_vars.hdr_prpt_typ_var, data_buf.hdr_prpt_typ_buf, lengths, offsets)) {
         mlog << Error << "error writing the pb message type to the netCDF file\n\n";
         exit(1);
      }
      if(!put_nc_data((NcVar *)&obs_vars.hdr_irpt_typ_var, data_buf.hdr_irpt_typ_buf, lengths, offsets)) {
         mlog << Error << "error writing the in message type to the netCDF file\n\n";
         exit(1);
      }
      if(!put_nc_data((NcVar *)&obs_vars.hdr_inst_typ_var, data_buf.hdr_inst_typ_buf, lengths, offsets)) {
         mlog << Error << "error writing the instrument type to the netCDF file\n\n";
         exit(1);
      }
      lengths[0] = save_len;
      data_buf.pb_hdr_data_offset += pb_hdr_len;
   }
   
   data_buf.hdr_data_offset += buf_size;
   data_buf.hdr_data_idx = 0;
}

///////////////////////////////////////////////////////////////////////////////

// Saves the headers at NcHeaderData hdr_data
//
void write_nc_arr_headers(const NetcdfObsVars &obs_vars)
{
   int cur_hdr_idx = nc_data_buffer.cur_hdr_idx;
   int buf_size = (cur_hdr_idx > OBS_BUFFER_SIZE) ? OBS_BUFFER_SIZE : cur_hdr_idx;
   const string method_name = "  write_nc_arr_headers()";
   
   mlog << Debug(5) << method_name << "  hdr_count: " << cur_hdr_idx
        << ", typ_idx_array: " << hdr_data.typ_idx_array.n_elements()
        << ", sid_idx_array: " << hdr_data.sid_idx_array.n_elements()
        << ", vld_idx_array: " << hdr_data.vld_idx_array.n_elements()
        << ", lat_array: " << hdr_data.lat_array.n_elements()
        << ", lon_array: " << hdr_data.lon_array.n_elements()
        << ", elv_array: " << hdr_data.elv_array.n_elements()
        << "\n";

   int hdr_data_idx = 0;
   bool is_pb_hdr = (0 < hdr_data.prpt_typ_array.n_elements())
         && !IS_INVALID_NC(obs_vars.hdr_prpt_typ_var);
   nc_data_buffer.obs_vars = obs_vars;
   nc_data_buffer.hdr_buf_size = buf_size;
   nc_data_buffer.hdr_data_idx = hdr_data_idx;
   for (int index=0; index<cur_hdr_idx; index++) {
      // Message type
      nc_data_buffer.hdr_typ_buf[hdr_data_idx] = hdr_data.typ_idx_array[index];
      
      // Station ID
      nc_data_buffer.hdr_sid_buf[hdr_data_idx] = hdr_data.sid_idx_array[index];
      
      // Valid Time
      nc_data_buffer.hdr_vld_buf[hdr_data_idx] = hdr_data.vld_idx_array[index];
      
      // Write the header array which consists of the following:
      //    LAT LON ELV
      nc_data_buffer.hdr_lat_buf[hdr_data_idx] = (float) hdr_data.lat_array[index];
      nc_data_buffer.hdr_lon_buf[hdr_data_idx] = (float) hdr_data.lon_array[index];
      nc_data_buffer.hdr_elv_buf[hdr_data_idx] = (float) hdr_data.elv_array[index];
      
      if (is_pb_hdr && index < obs_vars.raw_hdr_cnt) {
         nc_data_buffer.hdr_prpt_typ_buf[hdr_data_idx] = (float) hdr_data.prpt_typ_array[index];
         nc_data_buffer.hdr_irpt_typ_buf[hdr_data_idx] = (float) hdr_data.irpt_typ_array[index];
         nc_data_buffer.hdr_inst_typ_buf[hdr_data_idx] = (float) hdr_data.inst_typ_array[index];
      }
      
      hdr_data_idx++;
      nc_data_buffer.hdr_data_idx = hdr_data_idx;
      
      if (hdr_data_idx >= buf_size) {
         write_header_to_nc(obs_vars, nc_data_buffer, hdr_data_idx, is_pb_hdr);
         hdr_data_idx = nc_data_buffer.hdr_data_idx;
      }
   }

   write_nc_buf_headers(obs_vars);
}

///////////////////////////////////////////////////////////////////////////////

void write_nc_buf_headers (const NetcdfObsVars &obs_vars)
{
   if (0 < nc_data_buffer.hdr_data_idx) {
      write_header_to_nc(obs_vars, nc_data_buffer, nc_data_buffer.hdr_data_idx);
   }
   write_nc_table_vars((NetcdfObsVars &)obs_vars);
}
       
///////////////////////////////////////////////////////////////////////////////

void write_nc_header (const NetcdfObsVars &obs_vars,
      const char *hdr_typ, const char *hdr_sid, const time_t hdr_vld,
      const float hdr_lat, const float hdr_lon, const float hdr_elv)
{
   // Can't filter duplicated one because header index was
   // assigned before checking
   int hdr_index;
   bool new_vld = false;
   int hdr_data_idx = nc_data_buffer.hdr_data_idx;

   // Message type
   if (!hdr_data.typ_array.has(hdr_typ, hdr_index)) {
      hdr_index = hdr_data.typ_array.n_elements();
      hdr_data.typ_array.add(hdr_typ);
   }
   nc_data_buffer.hdr_typ_buf[hdr_data_idx] = hdr_index;
   
   // Station ID
   if (!hdr_data.sid_array.has(hdr_sid, hdr_index)) {
      hdr_index = hdr_data.sid_array.n_elements();
      hdr_data.sid_array.add(hdr_sid);
   }
   nc_data_buffer.hdr_sid_buf[hdr_data_idx] = hdr_index;
   
   // Valid Time
   if (hdr_data.min_vld_time == -1 || hdr_data.min_vld_time > hdr_vld) {
      if (hdr_data.min_vld_time == -1) hdr_data.max_vld_time = hdr_vld; 
      hdr_data.min_vld_time = hdr_vld;
      new_vld = true;
   }
   else if (hdr_data.max_vld_time < hdr_vld) {
      hdr_data.max_vld_time = hdr_vld;
      new_vld = true;
   }
   
   if (new_vld || !hdr_data.vld_num_array.has(hdr_vld, hdr_index)) {
      hdr_index = hdr_data.vld_array.n_elements();
      hdr_data.vld_array.add(unix_to_yyyymmdd_hhmmss(hdr_vld));
      hdr_data.vld_num_array.add(hdr_vld);
   }
   nc_data_buffer.hdr_vld_buf[hdr_data_idx] = hdr_index;
   
   // Write the header array which consists of the following:
   //    LAT LON ELV
   nc_data_buffer.hdr_lat_buf[hdr_data_idx] = (float) hdr_lat;
   nc_data_buffer.hdr_lon_buf[hdr_data_idx] = (float) hdr_lon;
   nc_data_buffer.hdr_elv_buf[hdr_data_idx] = (float) hdr_elv;
   
   hdr_data_idx++;
   nc_data_buffer.hdr_data_idx = hdr_data_idx;
   nc_data_buffer.cur_hdr_idx++;
   
   if (hdr_data_idx >= OBS_BUFFER_SIZE) {
      write_header_to_nc(nc_data_buffer.obs_vars, nc_data_buffer, OBS_BUFFER_SIZE);
   }
}
      
///////////////////////////////////////////////////////////////////////////////

void write_nc_observation(const NetcdfObsVars &obs_vars)
{
   if (0 < nc_data_buffer.obs_data_idx){
      nc_data_buffer.obs_vars = obs_vars;
      write_nc_obs_buffer(nc_data_buffer.obs_data_idx);
   }
}

///////////////////////////////////////////////////////////////////////////////

void write_nc_observation(const NetcdfObsVars &obs_vars,
      const float obs_arr[OBS_ARRAY_LEN], const char *obs_qty)
{
   int qty_index;
   int obs_data_idx = nc_data_buffer.obs_data_idx;
   if (!nc_data_buffer.qty_data_array.has(obs_qty, qty_index)) {
      qty_index = nc_data_buffer.qty_data_array.n_elements();
      nc_data_buffer.qty_data_array.add(obs_qty);
   }
   nc_data_buffer.qty_idx_buf[obs_data_idx] = qty_index;
      
   //for (int idx=0; idx<OBS_ARRAY_LEN; idx++) {
   //   nc_data_buffer.obs_data_buf[obs_data_idx][idx] = obs_arr[idx];
   //}
   nc_data_buffer.obs_hid_buf[obs_data_idx] = obs_arr[0];
   nc_data_buffer.obs_vid_buf[obs_data_idx] = obs_arr[1];
   nc_data_buffer.obs_lvl_buf[obs_data_idx] = obs_arr[2];
   nc_data_buffer.obs_hgt_buf[obs_data_idx] = obs_arr[3];
   nc_data_buffer.obs_val_buf[obs_data_idx] = obs_arr[4];
   nc_data_buffer.obs_data_idx++;
   nc_data_buffer.cur_obs_idx++;
   
   if (nc_data_buffer.obs_data_idx >= OBS_BUFFER_SIZE) {
      write_nc_obs_buffer(OBS_BUFFER_SIZE);
   }
}

///////////////////////////////////////////////////////////////////////////////

void write_nc_table_vars (NetcdfObsVars &obs_vars)
{
   mlog << Debug(5) << "    write_nc_table_vars() is called. valid hdr_typ_tbl_var: "
        << !IS_INVALID_NC(obs_vars.hdr_typ_tbl_var) << "\n";
   if (IS_INVALID_NC(obs_vars.hdr_typ_tbl_var))
      mlog << Warning << "\nwrite_nc_table_vars() is called without creating variables\n\n";
   if (!IS_INVALID_NC(obs_vars.hdr_typ_tbl_var))
      write_nc_string_array (&obs_vars.hdr_typ_tbl_var, hdr_data.typ_array, HEADER_STR_LEN2);
   if (!IS_INVALID_NC(obs_vars.hdr_sid_tbl_var))
      write_nc_string_array (&obs_vars.hdr_sid_tbl_var, hdr_data.sid_array, HEADER_STR_LEN2);
   if (!IS_INVALID_NC(obs_vars.hdr_vld_tbl_var))
      write_nc_string_array (&obs_vars.hdr_vld_tbl_var, hdr_data.vld_array, HEADER_STR_LEN);
   if (!IS_INVALID_NC(obs_vars.obs_qty_tbl_var))
      write_nc_string_array (&obs_vars.obs_qty_tbl_var, nc_data_buffer.qty_data_array, HEADER_STR_LEN);
}

///////////////////////////////////////////////////////////////////////////////

int write_nc_string_array (NcVar *ncVar, StringArray &strArray, const int str_len)
{
   //float obs_arr[obs_arr_len];
   const string method_name = "  write_nc_string_array() ";
   int data_count = strArray.n_elements();
   int max_buf_size = (1024 * 8);
   int buf_size = (data_count > max_buf_size ? max_buf_size : data_count);
   char data_buf[buf_size][str_len];
   long offsets[2] = { 0, 0 };
   long lengths[2] = { buf_size, str_len } ;

   mlog << Debug(7) << method_name << " " << GET_NC_NAME_P(ncVar)
        << "  data count: " << data_count << "\n";
   
   // Initialize data_buf
   for (int indexX=0; indexX<buf_size; indexX++)
      for (int indexY=0; indexY<str_len; indexY++)
        data_buf[indexX][indexY] = bad_data_char;

   int buf_index = 0;
   int processed_count = 0;
   for (int index=0; index<data_count; index++) {
      int len, len2;
      const string string_data= strArray[index];
      
      processed_count++;
      len  = string_data.length();
      len2 = strnlen(data_buf[buf_index], str_len);
      if (len2 < len) len2 = len;
      strncpy(data_buf[buf_index], string_data.c_str(), len);
      for (int idx=len; idx<len2; idx++)
         data_buf[buf_index][idx] = bad_data_char;

      buf_index++;
      if (buf_index >= buf_size) {
         mlog << Debug(7) << method_name << " save to NetCDF. index: " << index
              << "  buf_index: " << buf_index << "  offsets: "
              << offsets[0] << " lengths: " << lengths[0] << "\n";
         if(!put_nc_data(ncVar, (char*)data_buf[0], lengths, offsets)) {
            mlog << Error << "\n" << method_name << "-> "
                 << "error writing the variable " << GET_NC_NAME_P(ncVar)
                 << " to the netCDF file\n\n";
            exit(1);
         }
         offsets[0] += buf_size;
         buf_index = 0;
      }
   }
   
   if (buf_index > 0) {
      lengths[0] = (data_count <= max_buf_size) ? data_count : (data_count % buf_size);
      mlog << Debug(7) << method_name << " Save to NetCDF. offsets: " << offsets[0]
           << " lengths: " << lengths[0] << "\n";
      if(!put_nc_data(ncVar, (char*)data_buf[0], lengths, offsets)) {
         mlog << Error << "\n" << method_name << "-> "
              << "error writing the variable " << GET_NC_NAME_P(ncVar)
              << " to the netCDF file\n\n";
         exit(1);
      }
   }

   return processed_count;
}

///////////////////////////////////////////////////////////////////////////////

void write_nc_obs_buffer(const int buf_size)
{
   const NetcdfObsVars &obs_vars = nc_data_buffer.obs_vars;
   long offsets[2] = { nc_data_buffer.obs_data_offset, 0 };
   long lengths[1] = { buf_size} ;
   const string method_name = "  write_nc_obs_buffer()";

   mlog << Debug(7) << method_name << " offset: "
        << offsets[0] << ", " << offsets[1] << "  buf_size: " << buf_size << "\n";
   mlog << Debug(7) << "       obs_qty_var:  " << GET_NC_NAME(obs_vars.obs_qty_var) << "\n";
   
   //lengths[1] = HEADER_STR_LEN;
   if(!put_nc_data((NcVar *)&obs_vars.obs_qty_var, (int*)nc_data_buffer.qty_idx_buf, lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the quality flag to the "
           << "netCDF file\n\n";
      exit(1);
   }
   //lengths[1] = OBS_ARRAY_LEN;
   if(!put_nc_data((NcVar *)&obs_vars.obs_hid_var, (int*)nc_data_buffer.obs_hid_buf, lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the observation header index array to the "
           << "netCDF file\n\n";
      exit(1);
   }
   bool use_var_id = !IS_INVALID_NC(obs_vars.obs_vid_var);
   bool result = use_var_id
         ? put_nc_data((NcVar *)&obs_vars.obs_vid_var, (int*)nc_data_buffer.obs_vid_buf, lengths, offsets)
         : put_nc_data((NcVar *)&obs_vars.obs_gc_var,  (int*)nc_data_buffer.obs_vid_buf, lengths, offsets);
   if(!result) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the observation "
           << (use_var_id ? "variable_index" : "grib_code")
           << " array to the netCDF file\n\n";
      exit(1);
   }
   if(!put_nc_data((NcVar *)&obs_vars.obs_lvl_var, (float*)nc_data_buffer.obs_lvl_buf, lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the observation level array to the "
           << "netCDF file\n\n";
      exit(1);
   }
   if(!put_nc_data((NcVar *)&obs_vars.obs_hgt_var, (float*)nc_data_buffer.obs_hgt_buf, lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the observation hight array to the "
           << "netCDF file\n\n";
      exit(1);
   }
   if(!put_nc_data((NcVar *)&obs_vars.obs_val_var, (float*)nc_data_buffer.obs_val_buf, lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the observation data array to the "
           << "netCDF file\n\n";
      exit(1);
   }
   
   nc_data_buffer.obs_data_offset += buf_size;
   nc_data_buffer.obs_data_idx = 0;
}
      
///////////////////////////////////////////////////////////////////////////////

int write_nc_observations(const NetcdfObsVars &obs_vars,
                          const vector< Observation > observations,
                          const bool use_var_id, const bool do_header)
{
   int prev_hdr_idx = -1;
   string prev_header_type = "";
   string prev_station_id = "";
   ConcatString obs_qty;
   int headerOffset = nc_data_buffer.cur_hdr_idx;
   //float obs_arr[obs_arr_len];
   const string method_name = "  write_nc_observations()";

   int obs_buf_size = observations.size();
   mlog << Debug(7) << method_name << "  obs_count: " << obs_buf_size << "\n";
   if (obs_buf_size > OBS_BUFFER_SIZE) obs_buf_size = OBS_BUFFER_SIZE;
   
   //if (reset) {
   //   nc_data_buffer.obs_vars = obs_vars;
   //   nc_data_buffer.obs_buf_size = obs_buf_size;
   //   nc_data_buffer.obs_data_idx = 0;
   //   nc_data_buffer.obs_data_offset = 0;
   //   nc_data_buffer.hdr_data_idx = 0;
   //   nc_data_buffer.hdr_data_offset = 0;
   //   
   //   nc_data_buffer.processed_count =0;
   //}
   float obs_arr[OBS_ARRAY_LEN];
   bool header_to_vector = IS_INVALID_NC(obs_vars.hdr_arr_var)
         || IS_INVALID_NC(obs_vars.hdr_lat_var);
   mlog << Debug(5) << method_name << "  do_header: " << do_header
        << "  header_to_vector: " << header_to_vector << "\n";
   for (vector< Observation >::const_iterator obs = observations.begin();
        obs != observations.end(); ++obs)
   {
      nc_data_buffer.processed_count++;
      
      if (do_header) {
         if (obs->getHeaderIndex() != prev_hdr_idx) {
            mlog << Debug(9) << method_name << "  obs->getHeaderIndex(): "
                 << obs->getHeaderIndex() << " at obs " << nc_data_buffer.processed_count << "\n";
            prev_hdr_idx = obs->getHeaderIndex();
            if (header_to_vector) {
               add_nc_header_to_array(
                     obs->getHeaderType().c_str(),
                     obs->getStationId().c_str(),
                     obs->getValidTime(),
                     obs->getLatitude(),
                     obs->getLongitude(),
                     obs->getElevation());
            }
            else {
               write_nc_header(
                  obs_vars,
                     obs->getHeaderType().c_str(),
                     obs->getStationId().c_str(),
                     obs->getValidTime(),
                     obs->getLatitude(),
                     obs->getLongitude(),
                     obs->getElevation());
            }
         }
      }
      
      obs_arr[0] = obs->getHeaderIndex();
      obs_arr[1] = (use_var_id ? obs->getVarCode() : obs->getGribCode());
      obs_arr[2] = obs->getPressureLevel();
      obs_arr[3] = obs->getHeight();
      obs_arr[4] = obs->getValue();
      obs_qty = (obs->getQualityFlag().length() == 0 ? na_str : obs->getQualityFlag().c_str());
      if (do_header) obs_arr[0] += headerOffset;

      write_nc_observation(obs_vars, obs_arr, obs_qty.text());
      
   } /* endfor - obs */
   
   if (nc_data_buffer.obs_data_idx > 0) {
      write_nc_obs_buffer(nc_data_buffer.obs_data_idx);
   }

   //Caller handles writing headers

   return nc_data_buffer.processed_count;
}

////////////////////////////////////////////////////////////////////////

void write_obs_var_names(NetcdfObsVars &obs_vars, StringArray &obs_names) {
   write_nc_string_array (&obs_vars.obs_var,  obs_names, HEADER_STR_LEN2);
}

////////////////////////////////////////////////////////////////////////

void write_obs_var_units(NetcdfObsVars &obs_vars, StringArray &units) {
   write_nc_string_array (&obs_vars.unit_var,  units, HEADER_STR_LEN2);
}

////////////////////////////////////////////////////////////////////////

void write_obs_var_descriptions(NetcdfObsVars &obs_vars, StringArray &descriptions) {
   write_nc_string_array (&obs_vars.desc_var,  descriptions, HEADER_STR_LEN3);
}

////////////////////////////////////////////////////////////////////////
