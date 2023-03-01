// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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

#include <iostream>

#include <netcdf>
using namespace netCDF;

#include "vx_nc_util.h"

#include "nc_obs_util.h"

///////////////////////////////////////////////////////////////////////////////

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
// struct NcDataBuffer

NcDataBuffer::NcDataBuffer() {
   reset_counters();
}

///////////////////////////////////////////////////////////////////////////////

void NcDataBuffer::reset_counters() {
   processed_count = 0;
   obs_count = 0;
   obs_buf_size = 0;
   cur_obs_idx = 0;
   obs_data_idx = 0;
   obs_data_offset = 0;
   hdr_count = 0;
   hdr_buf_size = 0;
   cur_hdr_idx = 0;
   hdr_data_idx = 0;
   hdr_data_offset = 0;
   pb_hdr_count = 0;
   pb_hdr_data_offset = 0;
   prev_hdr_vld = 0;
}

///////////////////////////////////////////////////////////////////////////////
// struct NcPointObsData

NcPointObsData::NcPointObsData()
{
}

///////////////////////////////////////////////////////////////////////////////

bool NcPointObsData::read_obs_data_numbers(NetcdfObsVars obs_vars, bool stop) {
   bool succeed = true;
   const char* method_name = "NcPointObsData::read_obs_data_numbers()";

   clear_numbers();
   obs_cnt = obs_vars.obs_cnt;

   StringArray missing_vars;
   StringArray failed_vars;
   if (!IS_INVALID_NC(obs_vars.obs_arr_var)) {
      is_obs_array = true;
      obs_arr = new float[obs_cnt*OBS_ARRAY_LEN];
      if (!get_nc_data(&obs_vars.obs_arr_var, obs_arr)) {
         succeed = false;
         failed_vars.add(nc_var_obs_arr);
      }
   }
   else {
      if (IS_INVALID_NC(obs_vars.obs_hid_var)) {
         succeed = false;
         missing_vars.add(nc_var_obs_hid);
      }
      else {
         obs_hids = new int[obs_cnt];
         if (!get_nc_data(&obs_vars.obs_hid_var, obs_hids)) {
            succeed = false;
            failed_vars.add(nc_var_obs_hid);
         }
      }
      if (IS_INVALID_NC(obs_vars.obs_lvl_var)) {
         succeed = false;
         missing_vars.add(nc_var_obs_lvl);
      }
      else {
         obs_lvls = new float[obs_cnt];
         if (!get_nc_data(&obs_vars.obs_lvl_var, obs_lvls)) {
            succeed = false;
            failed_vars.add(nc_var_obs_lvl);
         }
      }
      if (IS_INVALID_NC(obs_vars.obs_hgt_var)) {
         succeed = false;
         missing_vars.add(nc_var_obs_hgt);
      }
      else {
         obs_hgts = new float[obs_cnt];
         if (!get_nc_data(&obs_vars.obs_hgt_var, obs_hgts)) {
            succeed = false;
            failed_vars.add(nc_var_obs_hgt);
         }
      }
      if (IS_INVALID_NC(obs_vars.obs_val_var)) {
         succeed = false;
         missing_vars.add(nc_var_obs_val);
      }
      else {
         obs_vals = new float[obs_cnt];
         if (!get_nc_data(&obs_vars.obs_val_var, obs_vals)) {
            succeed = false;
            failed_vars.add(nc_var_obs_val);
         }
      }
      if (IS_VALID_NC(obs_vars.obs_gc_var)) {
         obs_ids = new int[obs_cnt];
         if (!get_nc_data(&obs_vars.obs_gc_var, obs_ids)) {
            succeed = false;
            failed_vars.add(nc_var_obs_gc);
         }
      }
      else if (IS_VALID_NC(obs_vars.obs_vid_var)) {
         obs_ids = new int[obs_cnt];
         if (!get_nc_data(&obs_vars.obs_vid_var, obs_ids)) {
            succeed = false;
            failed_vars.add(nc_var_obs_vid);
         }
      }
      else succeed = false;

   }

   for (int idx=0; idx<missing_vars.n(); idx++) {
      mlog << Error << "\n" << method_name
           << "missing the variable " << missing_vars[idx] << "\n\n";
   }
   for (int idx=0; idx<failed_vars.n(); idx++) {
      mlog << Error << "\n" << method_name
           << "trouble getting data of " << failed_vars[idx] << "\n\n";
   }

   if (stop && !succeed) exit(1);
   return succeed;
}

///////////////////////////////////////////////////////////////////////////////

bool NcPointObsData::read_obs_data_table_lookups(NetcdfObsVars obs_vars, bool stop) {
   bool succeed = true;
   const char* method_name = "NcPointObsData::read_obs_data_table_lookups()";

   clear_strings();

   if (!IS_INVALID_NC(obs_vars.obs_arr_var)) {
      if (IS_VALID_NC(obs_vars.obs_qty_var)) {
         if (!get_nc_data_to_array(&obs_vars.obs_qty_var, &qty_names)) {
            succeed = false;
            mlog << Error << "\n" << method_name
                 << "trouble getting data of " << nc_var_obs_qty << "\n\n";
         }
      }
      else {
         mlog << Error << "\n" << method_name
              << "missing the variable " << nc_var_obs_qty << "\n\n";
      }
   }
   else {
      if (IS_VALID_NC(obs_vars.obs_var)) {
         if (!get_nc_data_to_array(&obs_vars.obs_var, &var_names)) {
            succeed = false;
            mlog << Error << "\n" << method_name
                 << "trouble getting data of " << nc_var_obs_var << "\n\n";
         }
      }
      if (IS_VALID_NC(obs_vars.obs_qty_tbl_var)) {
         if (!get_nc_data_to_array(&obs_vars.obs_qty_tbl_var, &qty_names)) {
            succeed = false;
            mlog << Error << "\n" << method_name
                 << "trouble getting data of " << nc_var_obs_qty_tbl << "\n\n";
         }
      }
   }

   if (stop && !succeed) exit(1);
   return succeed;
}


///////////////////////////////////////////////////////////////////////////////
// struc NetcdfObsVars

NetcdfObsVars::NetcdfObsVars()
{
   reset();
}

///////////////////////////////////////////////////////////////////////////////

void NetcdfObsVars::create_dimensions(NcFile *f_out) {
   const string method_name = "  create_dimensions()";
   mlog << Debug(7) << method_name << "  is called" << "\n";
   // Define netCDF dimensions
   if (IS_INVALID_NC(strl_dim))   strl_dim = add_dim(f_out, nc_dim_mxstr,  HEADER_STR_LEN);
   if (IS_INVALID_NC(strl2_dim)) strl2_dim = add_dim(f_out, nc_dim_mxstr2, HEADER_STR_LEN2);
   if (IS_INVALID_NC(strl3_dim)) strl3_dim = add_dim(f_out, nc_dim_mxstr3, HEADER_STR_LEN3);
   if (IS_INVALID_NC(hdr_dim) && hdr_cnt > 0) {
      hdr_dim = add_dim(f_out, nc_dim_nhdr, hdr_cnt);
   }
   if (IS_INVALID_NC(obs_dim)) {
      if (obs_cnt > 0) obs_dim = add_dim(f_out, nc_dim_nobs, obs_cnt);   // fixed dimension;
      else             obs_dim = add_dim(f_out, nc_dim_nobs);   // unlimited dimension;
   }
}

///////////////////////////////////////////////////////////////////////////////

void NetcdfObsVars::create_hdr_vars (NcFile *f_out, const int hdr_count) {
   const string method_name = "  create_hdr_vars()";
   mlog << Debug(7) << method_name << "  hdr_count: " << hdr_count << "\n";

   // Define netCDF dimensions
   hdr_cnt = hdr_count;
   create_dimensions(f_out);

   if (IS_INVALID_NC(hdr_dim)) {
      hdr_dim = (hdr_count > 0)
            ? add_dim(f_out, nc_dim_nhdr, hdr_count)
            : add_dim(f_out, nc_dim_nhdr);    // unlimited dimension
   }

   // Define netCDF header variables
   hdr_typ_var = add_var(f_out, nc_var_hdr_typ,   ncInt, hdr_dim, deflate_level);
   hdr_sid_var = add_var(f_out, nc_var_hdr_sid,   ncInt, hdr_dim, deflate_level);
   hdr_vld_var = add_var(f_out, nc_var_hdr_vld,   ncInt, hdr_dim, deflate_level);
   hdr_lat_var = add_var(f_out, nc_var_hdr_lat, ncFloat, hdr_dim, deflate_level);
   hdr_lon_var = add_var(f_out, nc_var_hdr_lon, ncFloat, hdr_dim, deflate_level);
   hdr_elv_var = add_var(f_out, nc_var_hdr_elv, ncFloat, hdr_dim, deflate_level);

   add_att(&hdr_typ_var,  "long_name", "index of message type");
   add_att(&hdr_typ_var, "_FillValue", (int)FILL_VALUE);
   add_att(&hdr_sid_var,  "long_name", "index of station identification");
   add_att(&hdr_sid_var, "_FillValue", (int)FILL_VALUE);
   add_att(&hdr_vld_var,  "long_name", "index of valid time");
   add_att(&hdr_vld_var, "_FillValue", (int)FILL_VALUE);

   add_att(&hdr_lat_var, "long_name",  "latitude");
   add_att(&hdr_lat_var, "_FillValue", FILL_VALUE);
   add_att(&hdr_lat_var, "units", "degrees_north");
   add_att(&hdr_lon_var, "long_name",  "longitude");
   add_att(&hdr_lon_var, "_FillValue", FILL_VALUE);
   add_att(&hdr_lon_var, "units", "degrees_east");
   add_att(&hdr_elv_var, "long_name",  "elevation");
   add_att(&hdr_elv_var, "_FillValue", FILL_VALUE);
   add_att(&hdr_elv_var, "units", "meters above sea level (msl)");
}

///////////////////////////////////////////////////////////////////////////////

void NetcdfObsVars::create_obs_vars (NcFile *f_out) {
   const char *long_name_str;
   const string method_name = "  create_obs_vars()";

   // Define netCDF dimensions
   create_dimensions(f_out);

   // Define netCDF variables
   obs_qty_var = add_var(f_out, nc_var_obs_qty,   ncInt, obs_dim, deflate_level);
   obs_hid_var = add_var(f_out, nc_var_obs_hid,   ncInt, obs_dim, deflate_level);
   if (use_var_id)
      obs_vid_var = add_var(f_out, nc_var_obs_vid,   ncInt, obs_dim, deflate_level);
   else
      obs_gc_var  = add_var(f_out, nc_var_obs_gc,    ncInt, obs_dim, deflate_level);
   obs_lvl_var = add_var(f_out, nc_var_obs_lvl, ncFloat, obs_dim, deflate_level);
   obs_hgt_var = add_var(f_out, nc_var_obs_hgt, ncFloat, obs_dim, deflate_level);
   obs_val_var = add_var(f_out, nc_var_obs_val, ncFloat, obs_dim, deflate_level);

   add_att(f_out, nc_att_obs_version, MET_NC_Obs_version);
   add_att(f_out, nc_att_use_var_id, (use_var_id ? "true" : "false"));

   // Add variable attributes
   add_att(&obs_qty_var,  "long_name", "index of quality flag");
   add_att(&obs_hid_var,  "long_name", "index of matching header data");
   add_att(&obs_hid_var, "_FillValue", (int)FILL_VALUE);
   add_att(&obs_val_var,  "long_name", "observation value");
   add_att(&obs_val_var, "_FillValue", FILL_VALUE);
   if (use_var_id) {
      long_name_str = (attr_pb2nc
            ? "index of BUFR variable corresponding to the observation type"
            : "index of variable names at var_name");
      add_att(&obs_vid_var,  "long_name", long_name_str);
      add_att(&obs_vid_var, "_FillValue", (int)FILL_VALUE);
   }
   else {
      add_att(&obs_gc_var,  "long_name", "grib code corresponding to the observation type");
      add_att(&obs_gc_var, "_FillValue", (int)FILL_VALUE);
   }

   add_att(&obs_lvl_var,  "long_name", "pressure level (hPa) or accumulation interval (sec)");
   add_att(&obs_lvl_var, "_FillValue", FILL_VALUE);
   long_name_str = (attr_agl)
         ? "height in meters above sea level or ground level (msl or agl)"
         : "height in meters above sea level (msl)";
   add_att(&obs_hgt_var,  "long_name", long_name_str);
   add_att(&obs_hgt_var, "_FillValue", FILL_VALUE);
}

///////////////////////////////////////////////////////////////////////////////

void NetcdfObsVars::create_obs_name_vars (NcFile *f_out, const int var_count, const int unit_count) {
   const string method_name = "  create_other_vars()";

   if (var_count > 0) {
      NcDim var_dim = create_var_obs_var(f_out, var_count);
      if (unit_count > 0) {
         unit_var = add_var(f_out, nc_var_unit, ncChar, var_dim, strl2_dim, deflate_level);
         desc_var = add_var(f_out, nc_var_desc, ncChar, var_dim, strl3_dim, deflate_level);

         add_att(&unit_var, "long_name", "variable units");
         add_att(&desc_var, "long_name", "variable descriptions");
      }
   }
}

////////////////////////////////////////////////////////////////////////

void NetcdfObsVars::create_pb_hdrs (NcFile *f_out, const int hdr_count) {
   const string method_name = "  create_pb_hdrs()";
   mlog << Debug(7) << method_name << "  hdr_count: " << hdr_count << "\n";

   // Define netCDF dimensions
   if (IS_INVALID_NC(pb_hdr_dim)) pb_hdr_dim = add_dim(f_out, nc_dim_npbhdr, hdr_count);

   raw_hdr_cnt = hdr_count;
   hdr_prpt_typ_var = add_var(f_out, nc_var_hdr_prpt_typ, ncInt, pb_hdr_dim, deflate_level);
   hdr_irpt_typ_var = add_var(f_out, nc_var_hdr_irpt_typ, ncInt, pb_hdr_dim, deflate_level);
   hdr_inst_typ_var = add_var(f_out, nc_var_hdr_inst_typ, ncInt, pb_hdr_dim, deflate_level);
   add_att(&hdr_prpt_typ_var, "long_name",  "PB report type");
   add_att(&hdr_prpt_typ_var, "_FillValue", (int)FILL_VALUE);
   add_att(&hdr_irpt_typ_var, "long_name",  "In report type");
   add_att(&hdr_irpt_typ_var, "_FillValue", (int)FILL_VALUE);
   add_att(&hdr_inst_typ_var, "long_name",  "instrument type");
   add_att(&hdr_inst_typ_var, "_FillValue", (int)FILL_VALUE);
}

///////////////////////////////////////////////////////////////////////////////

void NetcdfObsVars::create_table_vars (NcFile *f_out, MetPointHeader &hdr_data,
                                       NcDataBuffer &data_buffer) {
   const string method_name = "  create_table_vars()";

   // Define netCDF dimensions
   NcDim hdr_typ_dim = add_dim(f_out, nc_dim_nhdr_typ, hdr_data.typ_array.n_elements());
   NcDim hdr_sid_dim = add_dim(f_out, nc_dim_nhdr_sid, hdr_data.sid_array.n_elements());
   NcDim hdr_vld_dim = add_dim(f_out, nc_dim_nhdr_vld, hdr_data.vld_array.n_elements());
   NcDim obs_qty_dim = add_dim(f_out, nc_dim_nqty,     data_buffer.qty_data_array.n_elements());

   // Define netCDF header variables
   hdr_typ_tbl_var = add_var(f_out, nc_var_hdr_typ_tbl, ncChar, hdr_typ_dim, strl2_dim, deflate_level);
   hdr_sid_tbl_var = add_var(f_out, nc_var_hdr_sid_tbl, ncChar, hdr_sid_dim, strl2_dim, deflate_level);
   hdr_vld_tbl_var = add_var(f_out, nc_var_hdr_vld_tbl, ncChar, hdr_vld_dim, strl_dim,  deflate_level);
   obs_qty_tbl_var = add_var(f_out, nc_var_obs_qty_tbl, ncChar, obs_qty_dim, strl_dim,  deflate_level);

   add_att(&obs_qty_tbl_var, "long_name", "quality flag");
   add_att(&hdr_typ_tbl_var, "long_name", "message type");
   add_att(&hdr_sid_tbl_var, "long_name", "station identification");
   add_att(&hdr_vld_tbl_var, "long_name", "valid time");
   add_att(&hdr_vld_tbl_var, "units", "YYYYMMDD_HHMMSS UTC");
}

///////////////////////////////////////////////////////////////////////////////

NcDim NetcdfObsVars::create_var_obs_var (NcFile *f_out, int var_count) {
   NcDim var_dim  = add_dim(f_out, nc_dim_nvar, var_count);
   // If the string length is modified, update nc_tools.cc, too.
   if (IS_INVALID_NC(strl2_dim)) strl2_dim = add_dim(f_out, nc_dim_mxstr2, HEADER_STR_LEN2);

   obs_var = add_var(f_out, nc_var_obs_var, ncChar, var_dim, strl2_dim, deflate_level);
   add_att(&obs_var,  "long_name", "variable names");
   return var_dim;
}

////////////////////////////////////////////////////////////////////////

bool NetcdfObsVars::is_valid(bool exit_on_error) {
   bool valid = true;
   const char* method_name = "NetcdfObsVars::is_valid()";

   StringArray missing_dims;
   StringArray missing_vars;

   if(IS_INVALID_NC(hdr_dim)) missing_dims.add(nc_dim_nhdr);
   if(IS_INVALID_NC(obs_dim)) missing_dims.add(nc_dim_nobs);
   if(IS_INVALID_NC(strl_dim)) missing_dims.add("mxstr");

   if(IS_INVALID_NC(hdr_typ_var)) missing_vars.add("hdr_typ");
   if(IS_INVALID_NC(hdr_sid_var)) missing_vars.add("hdr_sid");
   if(IS_INVALID_NC(hdr_vld_var)) missing_vars.add("hdr_vld");
   if(IS_INVALID_NC(hdr_arr_var) && IS_INVALID_NC(hdr_lat_var)) missing_vars.add("hdr_lat/hdr_arr");
   if(IS_INVALID_NC(obs_arr_var) && IS_INVALID_NC(obs_val_var)) missing_vars.add("obs_val/obs_arr");

   if (0 < missing_dims.n()) {
      valid = false;
      for (int idx=0; idx<missing_dims.n() ;idx++) {
         mlog << Error << "\n" << method_name << " -> "
              << "can not find the dimension\"" << missing_dims[idx] << "\".\n\n";
      }
   }

   if (0 < missing_vars.n()) {
      valid = false;
      for (int idx=0; idx<missing_vars.n() ;idx++) {
         mlog << Error << "\n" << method_name << " -> "
              << "can't read \"" << missing_vars[idx]
              << "\" variable from the netCDF file\n\n";
      }
   }

   if (!valid && exit_on_error) exit(1);

   if(IS_INVALID_NC(obs_qty_var))
      mlog << Debug(3) << "Quality marker information not found input file.\n";

   return valid;
}

///////////////////////////////////////////////////////////////////////////////

void NetcdfObsVars::read_dims_vars(NcFile *f_in) {

   NcVar ncVar;
   // Define netCDF dimensions
   strl_dim    = get_nc_dim(f_in, nc_dim_mxstr);       // header string dimension
   if (has_dim(f_in, nc_dim_mxstr2))
      strl2_dim   = get_nc_dim(f_in, nc_dim_mxstr2);   // header string dimension (bigger dimension)
   if (has_dim(f_in, nc_dim_mxstr3))
      strl3_dim   = get_nc_dim(f_in, nc_dim_mxstr3);   // header string dimension (bigger dimension)

   if (has_dim(f_in, nc_dim_hdr_arr)) {
      hdr_arr_dim = get_nc_dim(f_in, nc_dim_hdr_arr);  // Header array width
      obs_arr_dim = get_nc_dim(f_in, nc_dim_obs_arr);  // Observation array width
   }
   else {
      if (has_dim(f_in, nc_dim_nhdr_typ))
         hdr_typ_dim = get_nc_dim(f_in, nc_dim_nhdr_typ); // header dimension for message type
      if (has_dim(f_in, nc_dim_nhdr_sid))
         hdr_sid_dim = get_nc_dim(f_in, nc_dim_nhdr_sid); // header dimension for station id
      if (has_dim(f_in, nc_dim_nhdr_vld))
         hdr_vld_dim = get_nc_dim(f_in, nc_dim_nhdr_vld); // header dimension for valid time
      if (has_dim(f_in, nc_dim_npbhdr))
         pb_hdr_dim  = get_nc_dim(f_in, nc_dim_npbhdr);   // header dimension for PB headers
   }
   obs_dim     = get_nc_dim(f_in, nc_dim_nobs);     // Observation array length
   hdr_dim     = get_nc_dim(f_in, nc_dim_nhdr);     // Header array length

   use_var_id = false;
   get_global_att(f_in, nc_att_use_var_id, use_var_id);

   // Get netCDF header variables
   hdr_typ_var = get_var(f_in, nc_var_hdr_typ);     // Message type (String or int)
   hdr_sid_var = get_var(f_in, nc_var_hdr_sid);     // Station ID (String or int)
   hdr_vld_var = get_var(f_in, nc_var_hdr_vld);     // Valid time (String or int)

   ncVar = get_var(f_in, nc_var_hdr_lat);
   if (IS_INVALID_NC(ncVar)) {
      hdr_arr_var = get_var(f_in, nc_var_hdr_arr);     // Header array
   } else {
      hdr_lat_var = ncVar;                                  // Header array
      hdr_lon_var = get_var(f_in, nc_var_hdr_lon);          // Header array
      hdr_elv_var = get_var(f_in, nc_var_hdr_elv);          // Header array
      hdr_typ_tbl_var = get_var(f_in, nc_var_hdr_typ_tbl);  // Message type (String)
      hdr_sid_tbl_var = get_var(f_in, nc_var_hdr_sid_tbl);  // Station ID (String)
      hdr_vld_tbl_var = get_var(f_in, nc_var_hdr_vld_tbl);  // Valid time (String)
   }

   // Get netCDF variables
   ncVar = get_var(f_in, nc_var_obs_hid);
   if (IS_INVALID_NC(ncVar)) {
      obs_arr_var = get_var(f_in, nc_var_obs_arr);
   } else {
      obs_hid_var = ncVar;                             // Obs. header id array
      if (use_var_id) {
         ncVar = get_var(f_in, nc_var_obs_vid);
         if (!IS_INVALID_NC(ncVar)) obs_vid_var = ncVar;  // Obs. variable id array
      }
      else {
         ncVar = get_var(f_in, nc_var_obs_gc);
         if (!IS_INVALID_NC(ncVar)) obs_gc_var  = ncVar;  // Obs. grib code array
      }
      obs_lvl_var = get_var(f_in, nc_var_obs_lvl);     // Obs. pressure level array
      obs_hgt_var = get_var(f_in, nc_var_obs_hgt);     // Obs. highth array
      obs_val_var = get_var(f_in, nc_var_obs_val);     // Obs. value array
   }
   ncVar = get_var(f_in, nc_var_obs_qty);
   if (!IS_INVALID_NC(ncVar)) obs_qty_var = ncVar;
   ncVar = get_var(f_in, nc_var_obs_qty_tbl);
   if (!IS_INVALID_NC(ncVar)) obs_qty_tbl_var = ncVar;

   if (use_var_id) {
      ncVar = get_var(f_in, nc_var_obs_var);
      if (!IS_INVALID_NC(ncVar)) obs_var = ncVar;
      ncVar = get_var(f_in, nc_var_unit);
      if (!IS_INVALID_NC(ncVar)) unit_var = ncVar;
      ncVar = get_var(f_in, nc_var_desc);
      if (!IS_INVALID_NC(ncVar)) desc_var = ncVar;
   }

   // PrepBufr only headers
   ncVar = get_var(f_in, nc_var_hdr_prpt_typ);
   if (!IS_INVALID_NC(ncVar)) hdr_prpt_typ_var = ncVar;
   ncVar = get_var(f_in, nc_var_hdr_irpt_typ);
   if (!IS_INVALID_NC(ncVar)) hdr_irpt_typ_var = ncVar;
   ncVar = get_var(f_in, nc_var_hdr_inst_typ);
   if (!IS_INVALID_NC(ncVar)) hdr_inst_typ_var = ncVar;

}

////////////////////////////////////////////////////////////////////////

void NetcdfObsVars::read_header_data(MetPointHeader &hdr_data) {
   bool is_valid_obs_nc = true;
   long nhdr_count  = get_dim_size(&hdr_dim);
   int  strl_len    = get_dim_size(&strl_dim);
   int  strl2_len   = strl_len;
   int  typ_len = strl_len;
   int  sid_len = strl_len;
   int  vld_len = strl_len;
   int  hdr_arr_len = IS_INVALID_NC(hdr_arr_dim)
         ? 0 : get_dim_size(&hdr_arr_dim);
   bool has_array_vars = IS_INVALID_NC(hdr_typ_tbl_var);
   const char *method_name = "get_nc_header() -> ";

   if (!IS_INVALID_NC(strl2_dim)) {
      NcDim str_dim;
      strl2_len = get_dim_size(&strl2_dim);
      string dim_name = GET_NC_NAME(strl2_dim);
      str_dim = get_nc_dim((IS_INVALID_NC(hdr_typ_tbl_var)
            ? &hdr_typ_var : &hdr_typ_tbl_var), dim_name);
      if (!IS_INVALID_NC(str_dim)) typ_len = strl2_len;

      str_dim = get_nc_dim((IS_INVALID_NC(hdr_sid_tbl_var)
            ? &hdr_sid_var : &hdr_sid_tbl_var), dim_name);
      if (!IS_INVALID_NC(str_dim)) sid_len = strl2_len;

      str_dim = get_nc_dim((IS_INVALID_NC(hdr_vld_tbl_var)
            ? &hdr_vld_var : &hdr_vld_tbl_var), dim_name);
      if (!IS_INVALID_NC(str_dim)) vld_len = strl2_len;
   }

   hdr_data.typ_len   = typ_len;
   hdr_data.sid_len   = sid_len;
   hdr_data.vld_len   = vld_len;
   hdr_data.strl_len  = strl_len;
   hdr_data.strll_len = strl2_len;

   hdr_data.lat_array.extend(nhdr_count);
   hdr_data.lon_array.extend(nhdr_count);
   hdr_data.elv_array.extend(nhdr_count);
   hdr_data.typ_idx_array.extend(nhdr_count);
   hdr_data.sid_idx_array.extend(nhdr_count);
   hdr_data.vld_idx_array.extend(nhdr_count);

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

   for(int i_start=0; i_start<nhdr_count; i_start+=buf_size) {
      buf_size = ((nhdr_count-i_start) > NC_BUFFER_SIZE_32K)
            ? NC_BUFFER_SIZE_32K : (nhdr_count-i_start);

      offsets[0] = offsets_1D[0] = i_start;
      lengths[0] = lengths_1D[0] = buf_size;

      //
      // Get the corresponding header message type
      //
      if (has_array_vars) {
         lengths[1] = typ_len;
         if(!get_nc_data(&hdr_typ_var,
               (char *)&hdr_typ_block[0], lengths, offsets)) {
            mlog << Error << "\n" << method_name
                 << "trouble getting hdr_typ\n\n";
            exit(1);
         }

         //
         // Get the corresponding header station id
         //
         lengths[1] = sid_len;
         if(!get_nc_data(&hdr_sid_var,
               (char *)&hdr_sid_block[0], lengths, offsets)) {
            mlog << Error << "\n" << method_name
                 << "trouble getting hdr_sid\n\n";
            exit(1);
         }

         //
         // Get the corresponding header valid time
         //
         lengths[1] = vld_len;
         if(!get_nc_data(&hdr_vld_var,
               (char *)&hdr_vld_block[0], lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_vld\n\n";
            exit(1);
         }

         //
         // Get the header for this observation
         //
         lengths[1] = hdr_arr_len;
         if(!get_nc_data(&hdr_arr_var,
               (float *)&hdr_arr_block[0], lengths, offsets)) {
            mlog << Error << "\n" << method_name
                 << "trouble getting hdr_arr\n\n";
            exit(1);
         }
         for (int hIndex = 0; hIndex < buf_size; hIndex++) {
            hdr_data.typ_array.add(hdr_typ_block[hIndex]);
            hdr_data.sid_array.add(hdr_sid_block[hIndex]);
            hdr_data.vld_array.add(hdr_vld_block[hIndex]);
            hdr_data.lat_array.add(hdr_arr_block[hIndex][0]);
            hdr_data.lon_array.add(hdr_arr_block[hIndex][1]);
            hdr_data.elv_array.add(hdr_arr_block[hIndex][2]);
         }
      }
      else {
         // Get the corresponding header message type (index, not string)
         if(!get_nc_data(&hdr_typ_var,
               hdr_typ_idx_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\n" << method_name
                 << "trouble getting hdr_typ\n\n";
            exit(1);
         }

         // Get the corresponding header station id (index, not string)
         if(!get_nc_data(&hdr_sid_var,
               hdr_sid_idx_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\n" << method_name
                 << "trouble getting hdr_sid\n\n";
            exit(1);
         }

         // Get the corresponding header valid time (index, not string)
         if(!get_nc_data(&hdr_vld_var,
               hdr_vld_idx_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\n" << method_name
                 << "trouble getting hdr_vld\n\n";
            exit(1);
         }

         //
         // Get the header for this observation
         //
         if(!get_nc_data(&hdr_lat_var,
               hdr_lat_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\n" << method_name
                 << "trouble getting hdr_lat\n\n";
            exit(1);
         }
         if(!get_nc_data(&hdr_lon_var,
               hdr_lon_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\n" << method_name
                 << "trouble getting hdr_lon\n\n";
            exit(1);
         }
         if(!get_nc_data(&hdr_elv_var,
               hdr_elv_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\n" << method_name
                 << "trouble getting hdr_elv\n\n";
            exit(1);
         }
         for (int hIndex = 0; hIndex < buf_size; hIndex++) {
            hdr_data.typ_idx_array.add(hdr_typ_idx_block[hIndex]);
            hdr_data.sid_idx_array.add(hdr_sid_idx_block[hIndex]);
            hdr_data.vld_idx_array.add(hdr_vld_idx_block[hIndex]);
            hdr_data.lat_array.add(hdr_lat_block[hIndex]);
            hdr_data.lon_array.add(hdr_lon_block[hIndex]);
            hdr_data.elv_array.add(hdr_elv_block[hIndex]);
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
      tmp_dim_size = get_dim_size(&hdr_typ_dim);
      for(int i_start=0; i_start<tmp_dim_size; i_start+=buf_size) {
         buf_size = ((tmp_dim_size-i_start) > NC_BUFFER_SIZE_32K)
               ? NC_BUFFER_SIZE_32K : (tmp_dim_size-i_start);
         offsets[0] = i_start;
         lengths[0] = buf_size;

         // Get the corresponding header message type (string)
         if(!get_nc_data(&hdr_typ_tbl_var,
               (char *)&hdr_typ_block[0], lengths, offsets)) {
            mlog << Error << "\n" << method_name
                 << "trouble getting hdr_typ\n\n";
            exit(1);
         }
         for (int hIndex = 0; hIndex < buf_size; hIndex++) {
            hdr_data.typ_array.add(hdr_typ_block[hIndex]);
         }
      }

      lengths[1] = sid_len;
      tmp_dim_size = get_dim_size(&hdr_sid_dim);
      for(int i_start=0; i_start<tmp_dim_size; i_start+=buf_size) {
         buf_size = ((tmp_dim_size-i_start) > NC_BUFFER_SIZE_32K)
               ? NC_BUFFER_SIZE_32K : (tmp_dim_size-i_start);
         offsets[0] = i_start;
         lengths[0] = buf_size;

         // Get the corresponding header station id (string)
         if(!get_nc_data(&hdr_sid_tbl_var,
               (char *)&hdr_sid_block[0], lengths, offsets)) {
            mlog << Error << "\n" << method_name
                 << "trouble getting hdr_typ\n\n";
            exit(1);
         }
         for (int hIndex = 0; hIndex < buf_size; hIndex++) {
            hdr_data.sid_array.add(hdr_sid_block[hIndex]);
         }
      }

      lengths[1] = vld_len;
      tmp_dim_size = get_dim_size(&hdr_vld_dim);
      for(int i_start=0; i_start<tmp_dim_size; i_start+=buf_size) {
         buf_size = ((tmp_dim_size-i_start) > NC_BUFFER_SIZE_32K)
               ? NC_BUFFER_SIZE_32K : (tmp_dim_size-i_start);
         offsets[0] = i_start;
         lengths[0] = buf_size;

         // Get the corresponding header valid time (string)
         if(!get_nc_data(&hdr_vld_tbl_var,
               (char *)&hdr_vld_block[0], lengths, offsets)) {
            mlog << Error << "\n" << method_name
                 << "trouble getting hdr_typ\n\n";
            exit(1);
         }
         for (int hIndex = 0; hIndex < buf_size; hIndex++) {
            hdr_data.vld_array.add(hdr_vld_block[hIndex]);
         }
      }
   }

   if (!IS_INVALID_NC(pb_hdr_dim)) {
      read_pb_hdr_data(hdr_data);
      int last_prpt_typ = hdr_data.prpt_typ_array.n_elements() - 1;
      int last_irpt_typ = hdr_data.irpt_typ_array.n_elements() - 1;
      int last_inst_typ = hdr_data.inst_typ_array.n_elements() - 1;
      if (0 > last_prpt_typ) mlog << Debug(7) << "    prpt_typ is empty\n";
      else if (0 > last_irpt_typ) mlog << Debug(7) << "    irpt_typ is empty\n";
      else if (0 > last_inst_typ) mlog << Debug(7) << "    inst_typ is empty\n";
      else {
         mlog << Debug(10)
              << "    prpt_typ[0,-1] " << hdr_data.prpt_typ_array[0]
              << "," << hdr_data.prpt_typ_array[last_prpt_typ]
              << "    irpt_typ[0,-1] " << hdr_data.irpt_typ_array[0]
              << "," << hdr_data.irpt_typ_array[last_irpt_typ]
              << "    inst_typ[0,-1] " << hdr_data.inst_typ_array[0]
              << "," << hdr_data.inst_typ_array[last_inst_typ] << "\n";
      }
   }
}

////////////////////////////////////////////////////////////////////////

bool NetcdfObsVars::read_obs_data(int buf_size, int offset,
      int qty_len, float *obs_arr, int *qty_idx_arr, char *obs_qty_buf) {
   bool result = true;
   long offsets[2] = { offset, 0 };
   long lengths[2] = { buf_size, 1 };
   const char *method_name = "read_obs_data() -> ";

   if (IS_VALID_NC(obs_arr_var)) {
      // Read the current observation message
      lengths[1] = OBS_ARRAY_LEN;
      if(!get_nc_data(&obs_arr_var, (float *)obs_arr, lengths, offsets)) {
         mlog << Error << "\n" << method_name << "trouble getting obs_arr\n\n";
         result = false;
      }

      if (0 != obs_qty_buf) {
         lengths[1] = qty_len;
         if(!get_nc_data(&obs_qty_var, obs_qty_buf, lengths, offsets)) {
            mlog << Error << "\n" << method_name << "trouble getting obs_qty\n\n";
            result = false;
         }
      }
   }
   else {
      int   *obs_hid_buf = new   int[buf_size];
      int   *obs_vid_buf = new   int[buf_size];
      float *obs_lvl_buf = new float[buf_size];
      float *obs_hgt_buf = new float[buf_size];
      float *obs_val_buf = new float[buf_size];

      lengths[1] = 1;

      if(!get_nc_data(&obs_hid_var, obs_hid_buf, lengths, offsets)) {
         mlog << Error << "\n" << method_name
              << "can't read the record for observation "
              << "index " << offset << "\n\n";
         result = false;
      }
      if(!get_nc_data((IS_INVALID_NC(obs_gc_var) ? &obs_vid_var : &obs_gc_var),
            obs_vid_buf, lengths, offsets)) {
         mlog << Error << "\n" << method_name
              << "can't read the record (vid or gc) for observation "
              << "index " << offset << "\n\n";
         result = false;
      }
      if(!get_nc_data(&obs_lvl_var, obs_lvl_buf, lengths, offsets)) {
         mlog << Error << "\n" << method_name
              << "can't read the record (lvl) for observation "
              << "index " << offset << "\n\n";
         result = false;
      }
      if(!get_nc_data(&obs_hgt_var, obs_hgt_buf, lengths, offsets)) {
         mlog << Error << "\n" << method_name
              << "can't read the record (hgt) for observation "
              << "index " << offset << "\n\n";
         result = false;
      }
      if(!get_nc_data(&obs_val_var, obs_val_buf, lengths, offsets)) {
         mlog << Error << "\n" << method_name
              << "can't read the record (val) for observation "
              << "index " << offset << "\n\n";
         result = false;
      }

      if (!get_nc_data(&obs_qty_var, qty_idx_arr, lengths, offsets)) {
         mlog << Error << "\n" << method_name
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

////////////////////////////////////////////////////////////////////////

void NetcdfObsVars::read_pb_hdr_data(MetPointHeader &hdr_data) {
   const char *method_name = "get_nc_pb_hdr_data() -> ";

   int pb_hdr_count = get_dim_size(&pb_hdr_dim);
   if (pb_hdr_count < 0) {
      mlog << Warning << "\n" << method_name
           << "No extra header for PREPBUFR\n\n";
      return;
   }

   long offsets[1] = { 0 };
   long lengths[1] = { 1 };
   bool has_hdr_prpt_typ_var = !IS_INVALID_NC(hdr_prpt_typ_var);
   bool has_hdr_irpt_typ_var = !IS_INVALID_NC(hdr_irpt_typ_var);
   bool has_hdr_inst_typ_var = !IS_INVALID_NC(hdr_inst_typ_var);

   if (has_hdr_prpt_typ_var) hdr_data.prpt_typ_array.extend(pb_hdr_count);
   if (has_hdr_irpt_typ_var) hdr_data.irpt_typ_array.extend(pb_hdr_count);
   if (has_hdr_inst_typ_var) hdr_data.inst_typ_array.extend(pb_hdr_count);

   // Read PB report type
   int buf_size = ((pb_hdr_count > NC_BUFFER_SIZE_32K)
         ? NC_BUFFER_SIZE_32K : (pb_hdr_count));
   int *hdr_prpt_typ_block = new int[buf_size];
   int *hdr_irpt_typ_block = new int[buf_size];
   int *hdr_inst_typ_block = new int[buf_size];
   for(int i_start=0; i_start<pb_hdr_count; i_start+=buf_size) {
      int buf_size2 = pb_hdr_count - i_start;
      if (buf_size2 > NC_BUFFER_SIZE_32K) buf_size2 = NC_BUFFER_SIZE_32K;
      offsets[0] = i_start;
      lengths[0] = buf_size2;

      if (has_hdr_prpt_typ_var) {
         // Get the corresponding header PB message type (string)
         if(!get_nc_data(&hdr_prpt_typ_var,
               hdr_prpt_typ_block, lengths, offsets)) {
            mlog << Error << "\n" << method_name
                 << "trouble getting hdr_prpt_typ\n\n";
            exit(1);
         }
      }

      if (has_hdr_irpt_typ_var) {
         // Get the corresponding header In message type (string)
         if(!get_nc_data(&hdr_irpt_typ_var,
               hdr_irpt_typ_block, lengths, offsets)) {
            mlog << Error << "\n" << method_name
                 << "trouble getting hdr_irpt_typ\n\n";
            exit(1);
         }
      }

      if (has_hdr_inst_typ_var) {
         // Get the corresponding header instrument type (string)
         if(!get_nc_data(&hdr_inst_typ_var,
               hdr_inst_typ_block, lengths, offsets)) {
            mlog << Error << "\n" << method_name
                 << "trouble getting hdr_inst_typ\n\n";
            exit(1);
         }
      }

      for (int hIndex = 0; hIndex < buf_size2; hIndex++) {
         hdr_data.prpt_typ_array.add(hdr_prpt_typ_block[hIndex]);
         hdr_data.irpt_typ_array.add(hdr_irpt_typ_block[hIndex]);
         hdr_data.inst_typ_array.add(hdr_inst_typ_block[hIndex]);
      }
   }

   delete[] hdr_prpt_typ_block;
   delete[] hdr_irpt_typ_block;
   delete[] hdr_inst_typ_block;

}

///////////////////////////////////////////////////////////////////////////////

void NetcdfObsVars::reset(bool _use_var_id) {
   attr_agl    = false;
   attr_pb2nc  = false;
   use_var_id  = _use_var_id;
   deflate_level = 0;
   hdr_cnt     = 0;     // header array length (fixed dimension if hdr_cnt > 0)
   obs_cnt     = 0;     // obs. array length (fixed dimension if obs_cnt > 0)
   raw_hdr_cnt = 0;
   //hdr_str_len = 0;    // string length for header (message) type header
}

///////////////////////////////////////////////////////////////////////////////

void NetcdfObsVars::write_header_to_nc(NcDataBuffer &data_buf,
                                       const int buf_size, const bool is_pb)
{
   int dim_size;
   bool is_prepbufr = is_pb || attr_pb2nc;
   long offsets[2] = { data_buf.hdr_data_offset, 0 };
   long lengths[1] = { buf_size } ;
   const string method_name = "  NetcdfObsVars::write_header_to_nc() ";

   ConcatString log_message;
   if (is_prepbufr) {
      log_message.add(", pb_hdr_data_offset: ");
      log_message.add(str_format("%i", data_buf.pb_hdr_data_offset));
      log_message.add(", raw_hdr_cnt: ");
      log_message.add(str_format("%i", raw_hdr_cnt));
   }

   mlog << Debug(7) << method_name << " buf_size: " << buf_size
        << "  is_prepbufr: " << is_prepbufr << ", hdr_data_offset: "
        << data_buf.hdr_data_offset << log_message << "\n";

   //lengths[1] = HEADER_STR_LEN2;
   dim_size = get_dim_size(&hdr_typ_var, 0);
   if ((0 < dim_size) &&(dim_size < lengths[0])) {
      mlog << Error << "\n" << method_name << " mismatching dimensions: allocated="
           << dim_size << ", data size=" << lengths[0] << " (hdr_typ)\n\n";
      exit(1);
   }
   else if(!put_nc_data((NcVar *)&hdr_typ_var, (int *)data_buf.hdr_typ_buf, lengths, offsets)) {
      mlog << Error << err_msg_message_type;
      exit(1);
   }

   // Station ID
   if(!put_nc_data((NcVar *)&hdr_sid_var, (int *)data_buf.hdr_sid_buf, lengths, offsets)) {
      mlog << Error << err_msg_station_id;
      exit(1);
   }

   // Valid Time
   if(!put_nc_data((NcVar *)&hdr_vld_var, (int *)data_buf.hdr_vld_buf, lengths, offsets)) {
      mlog << Error << err_msg_valid_time;
      exit(1);
   }

   // Write the header array which consists of the following: LAT LON ELV
   dim_size = get_dim_size(&hdr_lat_var, 0);
   if ((0 < dim_size) &&(dim_size < lengths[0])) {
      mlog << Error << "\n" << method_name << " mismatching dimensions: allocated="
           << dim_size << ", data size=" << lengths[0] << " (hdr_lat)\n\n";
      exit(1);
   }
   else if(!put_nc_data((NcVar *)&hdr_lat_var, (float *)data_buf.hdr_lat_buf, lengths, offsets)) {
      mlog << Error << err_msg_hdr_arr;
      exit(1);
   }
   if(!put_nc_data((NcVar *)&hdr_lon_var, (float *)data_buf.hdr_lon_buf, lengths, offsets)) {
      mlog << Error << err_msg_hdr_arr;
      exit(1);
   }
   if(!put_nc_data((NcVar *)&hdr_elv_var, (float *)data_buf.hdr_elv_buf, lengths, offsets)) {
      mlog << Error << err_msg_hdr_arr;
      exit(1);
   }

   if (is_prepbufr) {
      if (0 == raw_hdr_cnt) {
          mlog << Debug(6) << method_name
               << "No header for PREPBUFR report/instrument\n";
      }
      else if (data_buf.hdr_data_offset == data_buf.pb_hdr_data_offset) {
         int save_len = lengths[0];
         int pb_hdr_len = raw_hdr_cnt - offsets[0];
         if (pb_hdr_len > 0) {
            if (pb_hdr_len > buf_size) pb_hdr_len = buf_size;

            lengths[0] = pb_hdr_len;
            if(IS_VALID_NC(hdr_prpt_typ_var) && !put_nc_data((NcVar *)&hdr_prpt_typ_var,
                                                             data_buf.hdr_prpt_typ_buf, lengths, offsets)) {
               mlog << Error << "error writing the pb message type to the netCDF file\n\n";
               exit(1);
            }
            if(IS_VALID_NC(hdr_irpt_typ_var) && !put_nc_data((NcVar *)&hdr_irpt_typ_var,
                                                             data_buf.hdr_irpt_typ_buf, lengths, offsets)) {
               mlog << Error << "error writing the in message type to the netCDF file\n\n";
               exit(1);
            }
            if(IS_VALID_NC(hdr_inst_typ_var) && !put_nc_data((NcVar *)&hdr_inst_typ_var,
                                                             data_buf.hdr_inst_typ_buf, lengths, offsets)) {
               mlog << Error << "error writing the instrument type to the netCDF file\n\n";
               exit(1);
            }
            lengths[0] = save_len;
            data_buf.pb_hdr_data_offset += pb_hdr_len;
         }
      }
      else {
          mlog << Debug(6) << method_name
               << "Does not match header offsets for report/instrument: " << data_buf.hdr_data_offset
               << " : " << data_buf.pb_hdr_data_offset << "\n";
      }
   }

   data_buf.hdr_data_offset += buf_size;
   data_buf.hdr_data_idx = 0;
}

///////////////////////////////////////////////////////////////////////////////

void NetcdfObsVars::write_table_vars (MetPointHeader &hdr_data, NcDataBuffer &data_buffer)
{
   mlog << Debug(5) << "write_table_vars() is called. valid hdr_typ_tbl_var: "
        << !IS_INVALID_NC(hdr_typ_tbl_var) << "\n";
   if (IS_INVALID_NC(hdr_typ_tbl_var))
      mlog << Warning << "\nwrite_table_vars() is called without creating variables\n\n";
   if (!IS_INVALID_NC(hdr_typ_tbl_var))
      write_nc_string_array (&hdr_typ_tbl_var, hdr_data.typ_array, HEADER_STR_LEN2);
   if (!IS_INVALID_NC(hdr_sid_tbl_var))
      write_nc_string_array (&hdr_sid_tbl_var, hdr_data.sid_array, HEADER_STR_LEN2);
   if (!IS_INVALID_NC(hdr_vld_tbl_var))
      write_nc_string_array (&hdr_vld_tbl_var, hdr_data.vld_array, HEADER_STR_LEN);
   if (!IS_INVALID_NC(obs_qty_tbl_var))
      write_nc_string_array (&obs_qty_tbl_var, data_buffer.qty_data_array, HEADER_STR_LEN);
}

///////////////////////////////////////////////////////////////////////////////

void NetcdfObsVars::write_obs_buffer(NcDataBuffer &data_buffer, const int buf_size)
{
   long offsets[2] = { data_buffer.obs_data_offset, 0 };
   long lengths[1] = { buf_size} ;
   const string method_name = "  write_obs_buffer()";

   mlog << Debug(7) << method_name << " offset: "
        << offsets[0] << ", " << offsets[1] << "  buf_size: " << buf_size << "\n";
   mlog << Debug(7) << "       obs_qty_var:  " << GET_NC_NAME(obs_qty_var) << "\n";

   if(!put_nc_data((NcVar *)&obs_qty_var, (int*)data_buffer.qty_idx_buf, lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the quality flag to the "
           << "netCDF file\n\n";
      exit(1);
   }
   if(!put_nc_data((NcVar *)&obs_hid_var, (int*)data_buffer.obs_hid_buf, lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the observation header index array to the "
           << "netCDF file\n\n";
      exit(1);
   }
   bool use_var_id = !IS_INVALID_NC(obs_vid_var);
   bool result = use_var_id
         ? put_nc_data((NcVar *)&obs_vid_var, (int*)data_buffer.obs_vid_buf, lengths, offsets)
         : put_nc_data((NcVar *)&obs_gc_var,  (int*)data_buffer.obs_vid_buf, lengths, offsets);
   if(!result) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the observation "
           << (use_var_id ? "variable_index" : "grib_code")
           << " array to the netCDF file\n\n";
      exit(1);
   }
   if(!put_nc_data((NcVar *)&obs_lvl_var, (float*)data_buffer.obs_lvl_buf, lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the observation level array to the "
           << "netCDF file\n\n";
      exit(1);
   }
   if(!put_nc_data((NcVar *)&obs_hgt_var, (float*)data_buffer.obs_hgt_buf, lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the observation hight array to the "
           << "netCDF file\n\n";
      exit(1);
   }
   if(!put_nc_data((NcVar *)&obs_val_var, (float*)data_buffer.obs_val_buf, lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the observation data array to the "
           << "netCDF file\n\n";
      exit(1);
   }

   data_buffer.obs_data_offset += buf_size;
   data_buffer.obs_data_idx = 0;
}

////////////////////////////////////////////////////////////////////////

void NetcdfObsVars::write_obs_var_names(StringArray &obs_names) {
   write_nc_string_array (&obs_var, obs_names, HEADER_STR_LEN2);
}

////////////////////////////////////////////////////////////////////////

void NetcdfObsVars::write_obs_var_units(StringArray &units) {
   write_nc_string_array (&unit_var, units, HEADER_STR_LEN2);
}

////////////////////////////////////////////////////////////////////////

void NetcdfObsVars::write_obs_var_descriptions(StringArray &descriptions) {
   write_nc_string_array (&desc_var, descriptions, HEADER_STR_LEN3);
}

////////////////////////////////////////////////////////////////////////
// End of NetcdfObsVars

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
      obs->setHeaderIndex(nhdr-1);
   } /* endfor - obs */

   return nhdr;
}


///////////////////////////////////////////////////////////////////////////////

bool is_using_var_id(const char * nc_name) {
   bool use_var_id = false;
   if (!get_global_att(nc_name, nc_att_use_var_id, use_var_id)) {
      use_var_id = false;
   }
   return use_var_id;
}

///////////////////////////////////////////////////////////////////////////////

bool is_using_var_id(NcFile *nc_file) {
   bool use_var_id = false;
   if (!get_global_att(nc_file, nc_att_use_var_id, use_var_id)) {
      use_var_id = false;
   }
   return use_var_id;
}

///////////////////////////////////////////////////////////////////////////////

int write_nc_string_array (NcVar *ncVar, StringArray &strArray, const int str_len)
{
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
        data_buf[indexX][indexY] = 0;

   int buf_index = 0;
   int processed_count = 0;
   for (int index=0; index<data_count; index++) {
      int len_n, len_p;
      const string string_data= strArray[index];

      processed_count++;
      len_n = string_data.length();
      len_p = strnlen(data_buf[buf_index], str_len);
      if (len_n > str_len) len_n = str_len;
      m_strncpy(data_buf[buf_index], string_data.c_str(), len_n, method_name.c_str());
      for (int idx=len_n; idx<len_p; idx++)
         data_buf[buf_index][idx] = 0;  // erase previous data

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

////////////////////////////////////////////////////////////////////////
