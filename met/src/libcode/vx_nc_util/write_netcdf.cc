// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <ctime>
#include <iostream>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_log.h"
#include "vx_cal.h"
#include "vx_util.h"
#include "write_netcdf.h"
#include "grid_output.h"

///////////////////////////////////////////////////////////////////////////////

struct NcDataBuffer nc_data_buffer;
struct NcHeaderData hdr_data;
float  hdr_arr_block[NC_BUFFER_SIZE_32K][HDR_ARRAY_LEN];

///////////////////////////////////////////////////////////////////////////////

static void write_netcdf_latlon_1d(NcFile *, NcDim *, NcDim *, const Grid &);
static void write_netcdf_latlon_2d(NcFile *, NcDim *, NcDim *, const Grid &);

////////////////////////////////////////////////////////////////////////

NcHeaderData get_nc_hdr_data(NetcdfObsVars obs_vars) {
   NcHeaderData header_data;
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
   
   header_data.typ_len   = typ_len;
   header_data.sid_len   = sid_len;
   header_data.vld_len   = vld_len;
   header_data.strl_len  = strl_len;
   header_data.strll_len = strl2_len;
   if (has_array_vars) {
      header_data.typ_array.extend(nhdr_count);
      header_data.sid_array.extend(nhdr_count);
      header_data.vld_array.extend(nhdr_count);
   }
   else {
      header_data.typ_idx_array.extend(nhdr_count);
      header_data.sid_idx_array.extend(nhdr_count);
      header_data.vld_idx_array.extend(nhdr_count);

      int tmp_dim_size;
      tmp_dim_size = get_dim_size(&obs_vars.hdr_typ_dim);
      header_data.typ_array.extend(tmp_dim_size);
      tmp_dim_size = get_dim_size(&obs_vars.hdr_sid_dim);
      header_data.sid_array.extend(tmp_dim_size);
      tmp_dim_size = get_dim_size(&obs_vars.hdr_vld_dim);
      header_data.vld_array.extend(tmp_dim_size);
      mlog << Debug(7)
           << "    tbl dims: messge_type: " << get_dim_size(&obs_vars.hdr_typ_dim)
           << "  station id: " << get_dim_size(&obs_vars.hdr_sid_dim)
           << "  valid_time: " << get_dim_size(&obs_vars.hdr_vld_dim) << "\n";
   }
   header_data.lat_array.extend(nhdr_count);
   header_data.lon_array.extend(nhdr_count);
   header_data.elv_array.extend(nhdr_count);
   
   int buf_size = ((nhdr_count > NC_BUFFER_SIZE_32K)
        ? NC_BUFFER_SIZE_32K : (nhdr_count));
   
   //
   // Allocate space to store the data
   //
   char hdr_typ_str[typ_len+1];
   char hdr_sid_str[sid_len+1];
   char hdr_vld_str[vld_len+1];
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
            header_data.typ_array.add(hdr_typ_block[hIndex]);
            header_data.sid_array.add(hdr_sid_block[hIndex]);
            header_data.vld_array.add(hdr_vld_block[hIndex]);
            header_data.lat_array.add(hdr_arr_block[hIndex][0]);
            header_data.lon_array.add(hdr_arr_block[hIndex][1]);
            header_data.elv_array.add(hdr_arr_block[hIndex][2]);
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
            header_data.typ_idx_array.add(hdr_typ_idx_block[hIndex]);
            header_data.sid_idx_array.add(hdr_sid_idx_block[hIndex]);
            header_data.vld_idx_array.add(hdr_vld_idx_block[hIndex]);
            header_data.lat_array.add(hdr_lat_block[hIndex]);
            header_data.lon_array.add(hdr_lon_block[hIndex]);
            header_data.elv_array.add(hdr_elv_block[hIndex]);
         }
      }
   }
   
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
            header_data.typ_array.add(hdr_typ_block[hIndex]);
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
            header_data.sid_array.add(hdr_sid_block[hIndex]);
         }
      }

      lengths[1] = vld_len;
      tmp_dim_size = get_dim_size(&obs_vars.hdr_vld_dim);
      int buf_size = ((tmp_dim_size > NC_BUFFER_SIZE_32K)
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
            header_data.vld_array.add(hdr_vld_block[hIndex]);
         }
      }
   }
   if (!IS_INVALID_NC(obs_vars.pb_hdr_dim)) {
      get_nc_pb_hdr_data(obs_vars, &header_data);
      int last_prpt_typ = header_data.prpt_typ_array.n_elements() - 1;
      int last_irpt_typ = header_data.irpt_typ_array.n_elements() - 1; 
      int last_inst_typ = header_data.inst_typ_array.n_elements() - 1; 
      mlog << Debug(10)
           << "    prpt_typ[0,-1] " << header_data.prpt_typ_array[0]
           << "," << header_data.prpt_typ_array[last_prpt_typ]
           << "    irpt_typ[0,-1] " << header_data.irpt_typ_array[0]
           << "," << header_data.irpt_typ_array[last_irpt_typ]
           << "    inst_typ[0,-1] " << header_data.inst_typ_array[0]
           << "," << header_data.inst_typ_array[last_inst_typ] << "\n";
   }
   return header_data;
}

////////////////////////////////////////////////////////////////////////

void get_nc_pb_hdr_data(NetcdfObsVars obs_vars, NcHeaderData *header_data) {

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
         header_data->prpt_typ_array.add(hdr_prpt_typ_block[hIndex]);
         header_data->irpt_typ_array.add(hdr_irpt_typ_block[hIndex]);
         header_data->inst_typ_array.add(hdr_inst_typ_block[hIndex]);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

void init_nc_dims_vars(NetcdfObsVars &obs_vars, bool use_var_id) {
   obs_vars.attr_agl    = false;
   obs_vars.attr_pb2nc  = false;
   obs_vars.use_var_id  = use_var_id;
   obs_vars.hdr_cnt     = 0;     // header array length (fixed dimension if hdr_cnt > 0)
   obs_vars.obs_cnt     = 0;     // obs. array length (fixed dimension if obs_cnt > 0)
   //obs_vars.hdr_str_len = 0;    // string length for header (message) type header
   nc_data_buffer.obs_vars = obs_vars;
}

///////////////////////////////////////////////////////////////////////////////

void read_nc_dims_vars(NetcdfObsVars &obs_vars, NcFile *f_in) {
   
   NcVar ncVar;
   bool version_less_than_1_02 = is_version_less_than_1_02(f_in);
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

void write_netcdf_global(NcFile * f_out, const char *file_name,
                         const char *program_name, const char *model_name,
                         const char *obtype, const char *desc)
{
   int yr, mon, day, hr, min, sec;
   char attribute_str[PATH_MAX];
   char hostname_str[max_str_len];
   char time_str[max_str_len];
   
   unix_to_mdyhms(time(NULL), mon, day, yr, hr, min, sec);
   sprintf(time_str, "%.4i%.2i%.2i_%.2i%.2i%.2i",
           yr, mon, day, hr, min, sec);
   gethostname(hostname_str, max_str_len);
   sprintf(attribute_str,
           "File %s generated %s UTC on host %s by the MET %s tool",
           file_name, time_str, hostname_str, program_name);
   f_out->putAtt("FileOrigins", attribute_str);
   f_out->putAtt("MET_version", met_version);
   f_out->putAtt("MET_tool", program_name);
   //f_out->putAtt("Conventions", "CF-1.6");
   if(model_name) f_out->putAtt("model",  model_name);
   if(obtype)     f_out->putAtt("obtype", obtype);
   if(desc)       f_out->putAtt("desc",   desc);
   
   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_proj(NcFile * f_out, const Grid & grid)
{

const GridInfo info = grid.info();

grid_output(info, f_out);

return;

}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_latlon(NcFile *f_out, NcDim *lat_dim, NcDim *lon_dim,
                         const Grid &grid) {

   // Write 1-dimensional arrays for lat/lon grids and 2-d for all others
   if(grid.info().ll != 0) {
      write_netcdf_latlon_1d(f_out, lat_dim, lon_dim, grid);
   }
   else {
      write_netcdf_latlon_2d(f_out, lat_dim, lon_dim, grid);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_latlon_1d(NcFile *f_out, NcDim *lat_dim, NcDim *lon_dim,
                            const Grid &grid) {
   int i;
   double lat, lon;
   //NcVar *lat_var  = (NcVar *) 0;
   //NcVar *lon_var  = (NcVar *) 0;
   NcVar lat_var  ;
   NcVar lon_var  ;
   float *lat_data = (float *) 0;
   float *lon_data = (float *) 0;

   // Define Variables
   lat_var = f_out->addVar("lat", ncFloat, *lat_dim);
   lon_var = f_out->addVar("lon", ncFloat, *lon_dim);
   
   //lat_var = &lat_var_T;
   //lon_var = &lon_var_T;

   // Add variable attributes
   add_att(&lat_var, "long_name", "latitude");
   add_att(&lat_var, "units", "degrees_north");
   add_att(&lat_var, "standard_name", "latitude");

   add_att(&lon_var, "long_name", "longitude");
   add_att(&lon_var, "units", "degrees_east");
   add_att(&lon_var, "standard_name", "longitude");

   // Allocate space for lat/lon values
   lat_data = new float [grid.ny()];
   lon_data = new float [grid.nx()];

   // Compute latitude values
   for(i=0; i<grid.ny(); i++) {
      grid.xy_to_latlon(0, i, lat, lon);
      lat_data[i] = (float) lat;
   }

   // Compute longitude values (convert degrees west to east)
   for(i=0; i<grid.nx(); i++) {
      grid.xy_to_latlon(i, 0, lat, lon);
      lon_data[i] = (float) -1.0*lon;
   }

   // Write the lat data
   put_nc_data(&lat_var, &lat_data[0], lat_dim->getSize(), 0);

   // Write the lon data
   put_nc_data(&lon_var, &lon_data[0], lon_dim->getSize(), 0);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_latlon_2d(NcFile *f_out, NcDim *lat_dim, NcDim *lon_dim,
                            const Grid &grid) {
   int i, x, y;
   double lat, lon;
   //NcVar *lat_var  = (NcVar *) 0;
   //NcVar *lon_var  = (NcVar *) 0;
   NcVar lat_var  ;
   NcVar lon_var  ;
   float *lat_data = (float *) 0;
   float *lon_data = (float *) 0;
   std::vector<NcDim> dims;
   long  counts[2] = {grid.ny(), grid.nx()};
   long offsets[2] = {0 , 0};

   // Define Variables
   dims.push_back(*lat_dim);
   dims.push_back(*lon_dim);
   lat_var = add_var(f_out, "lat", ncFloat, dims);
   lon_var = add_var(f_out, "lon", ncFloat, dims);

   // Add variable attributes
   add_att(&lat_var, "long_name", "latitude");
   add_att(&lat_var, "units", "degrees_north");
   add_att(&lat_var, "standard_name", "latitude");
   //add_att(&lat_var, "axis", "Y");

   add_att(&lon_var, "long_name", "longitude");
   add_att(&lon_var, "units", "degrees_east");
   add_att(&lon_var, "standard_name", "longitude");
   //add_att(&lon_var, "axis", "X");

   // Allocate space for lat/lon values
   lat_data = new float [grid.nx()*grid.ny()];
   lon_data = new float [grid.nx()*grid.ny()];

   // Compute lat/lon values
   for(x=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         grid.xy_to_latlon(x, y, lat, lon);
         i = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);

         // Multiple by -1.0 to convert from degrees west to degrees east
         lat_data[i] = (float) lat;
         lon_data[i] = (float) -1.0*lon;
      }
   }

   // Write the lat data
   put_nc_data(&lat_var, &lat_data[0], counts, offsets);

   // Write the lon data
   put_nc_data(&lon_var, &lon_data[0], counts, offsets);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_grid_weight(NcFile *f_out, NcDim *lat_dim, NcDim *lon_dim,
                              const GridWeightType t, const DataPlane &wgt_dp) {
   int i, x, y;
   //NcVar *wgt_var  = (NcVar *) 0;
   NcVar wgt_var  ;
   float *wgt_data = (float *) 0;
   std::vector<NcDim> dims;
   std::vector<size_t> count;

   // Define Variables
   dims.push_back(*lat_dim);
   dims.push_back(*lon_dim);
   wgt_var = add_var(f_out, "grid_weight", ncFloat, dims);

   // Add variable attributes
   add_att(&wgt_var, "standard_name", "weight");

   switch(t) {

      case GridWeightType_Cos_Lat:
         add_att(&wgt_var, "long_name", "cosine latitude grid weight");
         add_att(&wgt_var, "units", "NA");
         break;

      case GridWeightType_Area:
         add_att(&wgt_var, "long_name", "true area grid weight");
         add_att(&wgt_var, "units", "km^2");
         break;

      default:
         add_att(&wgt_var, "long_name", "default grid weight");
         add_att(&wgt_var, "units", "NA");
         break;
   }

   // Allocate space for weight values
   wgt_data = new float [wgt_dp.nx()*wgt_dp.ny()];

   // Store weight values
   for(x=0; x<wgt_dp.nx(); x++) {
      for(y=0; y<wgt_dp.ny(); y++) {
         i = DefaultTO.two_to_one(wgt_dp.nx(), wgt_dp.ny(), x, y);
         wgt_data[i] = (float) wgt_dp(x, y);
      }
   }

   // Write the weights
   count.push_back(wgt_dp.ny());
   count.push_back(wgt_dp.nx());
   put_nc_data_with_dims(&wgt_var, &wgt_data[0], wgt_dp.ny(), wgt_dp.nx());

   // Clean up
   if(wgt_data) { delete [] wgt_data; wgt_data = (float *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_var_times(NcVar *var, const DataPlane &dp) {

   write_netcdf_var_times(var, dp.init(), dp.valid(), dp.accum());

   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_var_times(NcVar *var, const unixtime init_ut,
                            const unixtime valid_ut, const int accum_sec)

{

char time_str[max_str_len];
ConcatString s;

   // Init time
   unix_to_yyyymmdd_hhmmss(init_ut, time_str);
   add_att(var, "init_time", time_str);

   s = unixtime_to_string(init_ut);

   add_att(var, "init_time_ut", s.text());

   // Valid time
   unix_to_yyyymmdd_hhmmss(valid_ut, time_str);
   add_att(var, "valid_time", time_str);

   s = unixtime_to_string(valid_ut);

   add_att(var, "valid_time_ut", s.text());

   // Accumulation time
   if(accum_sec != 0) {
      sec_to_hhmmss(accum_sec, time_str);
      add_att(var, "accum_time", time_str);
      var->putAtt("accum_time_sec", ncInt, accum_sec);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

//bool is_same_header (const char *hdr_typ, const char *hdr_sid, const char *hdr_vld,
//      const float hdr_lat, const float hdr_lon, const float hdr_elv) {
//   bool new_header =
//         !is_eq(nc_data_buffer.prev_hdr_arr_buf[0],hdr_lat) ||
//         !is_eq(nc_data_buffer.prev_hdr_arr_buf[1], hdr_lon) ||
//         !is_eq(nc_data_buffer.prev_hdr_arr_buf[2], hdr_elv) ||
//         0 != strcmp(nc_data_buffer.prev_hdr_typ_buf, hdr_typ) ||
//         0 != strcmp(nc_data_buffer.prev_hdr_sid_buf, hdr_sid) ||
//         0 != strcmp(nc_data_buffer.prev_hdr_vld_buf, hdr_vld);
//   if (new_header) {
//      strcpy(nc_data_buffer.prev_hdr_typ_buf, hdr_typ);
//      strcpy(nc_data_buffer.prev_hdr_sid_buf, hdr_sid);
//      strcpy(nc_data_buffer.prev_hdr_vld_buf, hdr_vld);
//      nc_data_buffer.prev_hdr_arr_buf[0] = hdr_lat;
//      nc_data_buffer.prev_hdr_arr_buf[1] = hdr_lon;
//      nc_data_buffer.prev_hdr_arr_buf[2] = hdr_elv;
//   }
//   return new_header;
//}
//
//bool is_same_header (const char *hdr_typ, const char *hdr_sid, const unixtime hdr_vld,
//      const float hdr_lat, const float hdr_lon, const float hdr_elv) {
//   bool new_header =
//         !is_eq(nc_data_buffer.prev_hdr_arr_buf[0],hdr_lat) ||
//         !is_eq(nc_data_buffer.prev_hdr_arr_buf[1], hdr_lon) ||
//         !is_eq(nc_data_buffer.prev_hdr_arr_buf[2], hdr_elv) ||
//         0 != strcmp(nc_data_buffer.prev_hdr_typ_buf, hdr_typ) ||
//         0 != strcmp(nc_data_buffer.prev_hdr_sid_buf, hdr_sid) ||
//         !is_eq(nc_data_buffer.prev_hdr_vld, hdr_vld);
//   if (new_header) {
//      strcpy(nc_data_buffer.prev_hdr_typ_buf, hdr_typ);
//      strcpy(nc_data_buffer.prev_hdr_sid_buf, hdr_sid);
//      nc_data_buffer.prev_hdr_vld = hdr_vld;
//      nc_data_buffer.prev_hdr_arr_buf[0] = hdr_lat;
//      nc_data_buffer.prev_hdr_arr_buf[1] = hdr_lon;
//      nc_data_buffer.prev_hdr_arr_buf[2] = hdr_elv;
//   }
//   return new_header;
//}

///////////////////////////////////////////////////////////////////////////////
