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

long hdrNum;
long obsNum;

struct NcDataBuffer nc_data_buffer;
struct NcHeaderData hdr_data;

static const string err_msg_message_type =
      "error writing the message type string to the netCDF file\n\n";
static const string err_msg_station_id =
      "error writing the station id string to the netCDF file\n\n";
static const string err_msg_valid_time =
      "error writing the valid time to the netCDF file\n\n";
static const string err_msg_hdr_arr =
      "error writing the header array to the netCDF file\n\n";


///////////////////////////////////////////////////////////////////////////////

static void write_netcdf_latlon_1d(NcFile *, NcDim *, NcDim *, const Grid &);
static void write_netcdf_latlon_2d(NcFile *, NcDim *, NcDim *, const Grid &);

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

   add_att(&lon_var, "long_name", "longitude");
   add_att(&lon_var, "units", "degrees_east");
   add_att(&lon_var, "standard_name", "longitude");

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

bool add_nc_header_all (const char *hdr_typ, const char *hdr_sid, const char *hdr_vld,
      const float hdr_lat, const float hdr_lon, const float hdr_elv)
{
   bool added = false;
   // Can't filter duplicated one because header index was
   // assigned before checking
   int hdr_index;
   if (!hdr_data.typ_array.has(hdr_typ, hdr_index)) {
      hdr_index = hdr_data.typ_array.n_elements();
      hdr_data.typ_array.add(hdr_typ);       // Message type
   }
   hdr_data.typ_idx_array.add(hdr_index);    // Index of Message type
   if (!hdr_data.sid_array.has(hdr_sid, hdr_index)) {
      hdr_index = hdr_data.sid_array.n_elements();
      hdr_data.sid_array.add(hdr_sid);       // Station ID
   }
   hdr_data.sid_idx_array.add(hdr_index);    // Index of Station ID
   if (!hdr_data.vld_array.has(hdr_vld, hdr_index)) {
      hdr_index = hdr_data.vld_array.n_elements();
      hdr_data.vld_array.add(hdr_vld);       // Valid time
   }
   hdr_data.vld_idx_array.add(hdr_index);    // Index of Valid time
   
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

///////////////////////////////////////////////////////////////////////////////

bool is_same_header (const char *hdr_typ, const char *hdr_sid, const char *hdr_vld,
      const float hdr_lat, const float hdr_lon, const float hdr_elv) {
   bool new_header =
         !is_eq(nc_data_buffer.prev_hdr_arr_buf[0],hdr_lat) ||
         !is_eq(nc_data_buffer.prev_hdr_arr_buf[1], hdr_lon) ||
         !is_eq(nc_data_buffer.prev_hdr_arr_buf[2], hdr_elv) ||
         0 != strcmp(nc_data_buffer.prev_hdr_typ_buf, hdr_typ) ||
         0 != strcmp(nc_data_buffer.prev_hdr_sid_buf, hdr_sid) ||
         0 != strcmp(nc_data_buffer.prev_hdr_vld_buf, hdr_vld);
   if (new_header) {
      strcpy(nc_data_buffer.prev_hdr_typ_buf, hdr_typ);
      strcpy(nc_data_buffer.prev_hdr_sid_buf, hdr_sid);
      strcpy(nc_data_buffer.prev_hdr_vld_buf, hdr_vld);
      nc_data_buffer.prev_hdr_arr_buf[0] = hdr_lat;
      nc_data_buffer.prev_hdr_arr_buf[1] = hdr_lon;
      nc_data_buffer.prev_hdr_arr_buf[2] = hdr_elv;
   }
   return new_header;
}

///////////////////////////////////////////////////////////////////////////////

void write_nc_other_vars (NetcdfObsVars &obs_vars)
{
   mlog << Debug(7) << "    write_nc_other_vars() is called. valid hdr_typ_tbl_var: "
        << !IS_INVALID_NC(obs_vars.hdr_typ_tbl_var) << "\n";
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

void write_nc_header (const NetcdfObsVars &obs_vars)
{
   if (0 < nc_data_buffer.hdr_data_idx) {
      write_header_to_nc(obs_vars, nc_data_buffer, nc_data_buffer.hdr_data_idx);
   }
   write_nc_other_vars((NetcdfObsVars &)obs_vars);
}
       
///////////////////////////////////////////////////////////////////////////////

void write_nc_header (const NetcdfObsVars &obs_vars,
      const char *hdr_typ, const char *hdr_sid, const char *hdr_vld,
      const float hdr_lat, const float hdr_lon, const float hdr_elv)
{
   // Can't filter duplicated one because header index was
   // assigned before checking
   int hdr_index;
   //int hdr_str_len, hdr_str_len2;
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
   if (!hdr_data.vld_array.has(hdr_vld, hdr_index)) {
      hdr_index = hdr_data.vld_array.n_elements();
      hdr_data.vld_array.add(hdr_vld);
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

void write_nc_observation(const NetcdfObsVars &obs_vars, NcDataBuffer &data_buf)
{
   if (0 < data_buf.obs_data_idx){
      data_buf.obs_vars = obs_vars;
      write_nc_obs_buffer(data_buf, data_buf.obs_data_idx);
   }
}

///////////////////////////////////////////////////////////////////////////////

void write_nc_observation(const NetcdfObsVars &obs_vars, NcDataBuffer &data_buf,
      const float obs_arr[OBS_ARRAY_LEN], const char *obs_qty)
{
   int qty_index;
   int obs_data_idx = data_buf.obs_data_idx;
   if (!data_buf.qty_data_array.has(obs_qty, qty_index)) {
      qty_index = data_buf.qty_data_array.n_elements();
      data_buf.qty_data_array.add(obs_qty);
   }
   data_buf.qty_idx_buf[obs_data_idx] = qty_index;
      
   //for (int idx=0; idx<OBS_ARRAY_LEN; idx++) {
   //   data_buf.obs_data_buf[obs_data_idx][idx] = obs_arr[idx];
   //}
   data_buf.obs_hid_buf[obs_data_idx] = obs_arr[0];
   data_buf.obs_vid_buf[obs_data_idx] = obs_arr[1];
   data_buf.obs_lvl_buf[obs_data_idx] = obs_arr[2];
   data_buf.obs_hgt_buf[obs_data_idx] = obs_arr[3];
   data_buf.obs_val_buf[obs_data_idx] = obs_arr[4];
   data_buf.obs_data_idx++;
   data_buf.cur_obs_idx++;
   
   if (data_buf.obs_data_idx >= OBS_BUFFER_SIZE) {
      write_nc_obs_buffer(data_buf, OBS_BUFFER_SIZE);
   }
}

///////////////////////////////////////////////////////////////////////////////

int write_nc_string_array (NcVar *ncVar, StringArray &strArray, const int str_len)
{
   //float obs_arr[obs_arr_len];
   const string method_name = "write_nc_string_array()";
   int data_count = strArray.n_elements();
   int max_buf_size = (1024 * 8);
   int buf_size = (data_count > max_buf_size ? max_buf_size : data_count);
   char data_buf[buf_size][str_len];
   long offsets[2] = { 0, 0 };
   long lengths[2] = { buf_size, str_len } ;

   mlog << Debug(7) << "    " << method_name << "  data count : "
        << data_count << " (" << GET_NC_NAME_P(ncVar) << ")\n";
   
   // Initialize data_buf
   for (int indexX=0; indexX<buf_size; indexX++)
      for (int indexY=0; indexY<str_len; indexY++)
        data_buf[indexX][indexY] = bad_data_char;

   int buf_index = 0;
   int processed_count = 0;
   for (int index=0; index<data_count; index++) {
      int len, len2;
      const char* string_data;
      
      processed_count++;
      string_data = strArray[index];
      len  = strlen(string_data);
      len2 = strlen(data_buf[buf_index]);
      if (len2 < len) len2 = len;
      strncpy(data_buf[buf_index], string_data, len);
      for (int idx=len; idx<len2; idx++)
         data_buf[buf_index][idx] = bad_data_char;

      buf_index++;
      if (buf_index >= buf_size) {
         mlog << Debug(7) << "    " << method_name << "  index : " << index
              << "  buf_index: " << buf_index << "  offsets: " << offsets[0] << " lengths " << lengths[0] << "\n";
         if(!put_nc_data(ncVar, (char*)data_buf[0], lengths, offsets)) {
            mlog << Error << "\n" << method_name << " -> "
                 << "error writing the variable " << GET_NC_NAME_P(ncVar)
                 << " to the netCDF file\n\n";
            exit(1);
         }
         offsets[0] += buf_size;
         buf_index = 0;
         mlog << Debug(7) << "    " << method_name << "  for loop\n";
      }
   }
   
   if (buf_index > 0) {
      lengths[0] = (data_count <= max_buf_size) ? data_count : (data_count % buf_size);
      mlog << Debug(7) << "    " << method_name << " offsets: " << offsets[0]
                       << " lengths " << lengths[0] << "\n";
      if(!put_nc_data(ncVar, (char*)data_buf[0], lengths, offsets)) {
         mlog << Error << "\n" << method_name << " -> "
              << "error writing the variable " << GET_NC_NAME_P(ncVar)
              << " to the netCDF file\n\n";
         exit(1);
      }
   }

   return processed_count;
}

///////////////////////////////////////////////////////////////////////////////

void create_nc_dimensions(NetcdfObsVars &obs_vars, NcFile *f_out) {
   const char *method_name = "create_nc_dimensions()";
   mlog << Debug(7) << "    " << method_name << "  is called" << "\n";
   // Define netCDF dimensions
   if (IS_INVALID_NC(obs_vars.strl_dim))   obs_vars.strl_dim = add_dim(f_out, nc_dim_mxstr,  HEADER_STR_LEN);
   if (IS_INVALID_NC(obs_vars.strl2_dim)) obs_vars.strl2_dim = add_dim(f_out, nc_dim_mxstr2, HEADER_STR_LEN2);
   if (IS_INVALID_NC(obs_vars.strl3_dim)) obs_vars.strl3_dim = add_dim(f_out, nc_dim_mxstr3, HEADER_STR_LEN3);
   if (IS_INVALID_NC(obs_vars.hdr_dim) && obs_vars.hdr_cnt > 0) {
      obs_vars.hdr_dim = add_dim(f_out, nc_dim_nhdr, obs_vars.hdr_cnt);
   }
   if (IS_INVALID_NC(obs_vars.obs_dim))   obs_vars.obs_dim   = add_dim(f_out, nc_dim_nobs);   // unlimited dimension;
   nc_data_buffer.obs_vars = obs_vars;
}

///////////////////////////////////////////////////////////////////////////////

void create_nc_hdr_vars (NetcdfObsVars &obs_vars, NcFile *f_out,
      const int hdr_count, const int deflate_level) {
   const char *method_name = "create_nc_hdr_vars()";
   mlog << Debug(7) << "    " << method_name << "  hdr_count: " << hdr_count << "\n";
   
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

void create_nc_pb_hdrs (NetcdfObsVars &obs_vars, NcFile *f_out,
      const int hdr_count, const int deflate_level) {
   const char *method_name = "create_nc_pb_hdrs()";
   mlog << Debug(7) << "    " << method_name << "  hdr_count: " << hdr_count << "\n";
   
   // Define netCDF dimensions
   if (IS_INVALID_NC(obs_vars.pb_hdr_dim)) obs_vars.pb_hdr_dim = add_dim(f_out, nc_dim_npbhdr, hdr_count);
   
   obs_vars.pb_hdr_cnt = hdr_count;
   obs_vars.hdr_prpt_typ_var = add_var(f_out, nc_var_hdr_prpt_typ, ncInt, obs_vars.pb_hdr_dim, deflate_level);
   obs_vars.hdr_irpt_typ_var = add_var(f_out, nc_var_hdr_irpt_typ, ncInt, obs_vars.pb_hdr_dim, deflate_level);
   obs_vars.hdr_inst_typ_var = add_var(f_out, nc_var_hdr_inst_typ, ncInt, obs_vars.pb_hdr_dim, deflate_level);
   add_att(&obs_vars.hdr_prpt_typ_var, "long_name",  "PB report type");
   add_att(&obs_vars.hdr_prpt_typ_var, "_FillValue", (int)FILL_VALUE);
   add_att(&obs_vars.hdr_irpt_typ_var, "long_name",  "In report type");
   add_att(&obs_vars.hdr_irpt_typ_var, "_FillValue", (int)FILL_VALUE);
   add_att(&obs_vars.hdr_inst_typ_var, "long_name",  "instrument type");
   add_att(&obs_vars.hdr_inst_typ_var, "_FillValue", (int)FILL_VALUE);
   nc_data_buffer.obs_vars = obs_vars;
}

///////////////////////////////////////////////////////////////////////////////

void create_nc_obs_vars (NetcdfObsVars &obs_vars, NcFile *f_out, const int deflate_level, bool use_var_id) {
   const char *long_name_str;
   const char *method_name = "create_nc_obs_vars()";

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

void create_nc_other_vars (NetcdfObsVars &obs_vars, NcFile *f_out, 
      const NcDataBuffer &data_buf, const NcHeaderData &hdr_buf,
      const int var_count, const int unit_count, const int deflate_level) {
   const char *method_name = "create_nc_other_vars()";
   
   // Define netCDF dimensions
   NcDim hdr_typ_dim = add_dim(f_out, nc_dim_nhdr_typ, hdr_buf.typ_array.n_elements());
   NcDim hdr_sid_dim = add_dim(f_out, nc_dim_nhdr_sid, hdr_buf.sid_array.n_elements());
   NcDim hdr_vld_dim = add_dim(f_out, nc_dim_nhdr_vld, hdr_buf.vld_array.n_elements());
   NcDim obs_qty_dim = add_dim(f_out, nc_dim_nqty,     data_buf.qty_data_array.n_elements());

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

   if (var_count > 0) {
      NcDim var_dim  = add_dim(f_out, nc_dim_nvar, var_count);

      obs_vars.obs_var  = add_var(f_out, nc_var_obs_var, ncChar, var_dim, obs_vars.strl_dim,  deflate_level);
      add_att(&obs_vars.obs_var,  "long_name", "variable names");
      if (unit_count > 0) {
         obs_vars.unit_var = add_var(f_out,    nc_var_unit, ncChar, var_dim, obs_vars.strl2_dim, deflate_level);
         obs_vars.desc_var = add_var(f_out,    nc_var_desc, ncChar, var_dim, obs_vars.strl3_dim, deflate_level);
         
         add_att(&obs_vars.unit_var, "long_name", "variable units");
         add_att(&obs_vars.desc_var, "long_name", "variable descriptions");
      }
   }
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
   const char *method_name = "count_nc_headers()";

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
      //else mlog << Debug(7) << "    " << method_name 
      //     << "  FFF obs->getHeaderIndex(): " << obs->getHeaderIndex()
      //     << ", nhdr: " << nhdr << " count: " << count
      //     << "\n";
      obs->setHeaderIndex(nhdr-1);
   } /* endfor - obs */
   
   return nhdr;
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

///////////////////////////////////////////////////////////////////////////////

void init_nc_dims_vars(NetcdfObsVars &obs_vars, bool use_var_id) {
   obs_vars.attr_agl    = false;
   obs_vars.attr_pb2nc  = false;
   obs_vars.use_var_id  = use_var_id;
   obs_vars.hdr_cnt     = 0;     // header array length (fixed dimension if hdr_cnt > 0)
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

void reset_header_buffer(int buf_size) {
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
}

///////////////////////////////////////////////////////////////////////////////

// Saves the headers at NcHeaderData hdr_data
//
void write_nc_headers(const NetcdfObsVars &obs_vars)
{
   int hdr_str_len, hdr_str_len2;
   int cur_hdr_idx = nc_data_buffer.cur_hdr_idx;
   int buf_size = (cur_hdr_idx > OBS_BUFFER_SIZE) ? OBS_BUFFER_SIZE : cur_hdr_idx;
   const char *method_name = "write_nc_headers()";
   
   mlog << Debug(7) << "    " << method_name << "  hdr_count: " << cur_hdr_idx << "\n";

   int hdr_data_idx = 0;
   bool is_pb_hdr = (0 < hdr_data.prpt_typ_array.n_elements())
         && !IS_INVALID_NC(obs_vars.hdr_prpt_typ_var);
   nc_data_buffer.obs_vars = obs_vars;
   nc_data_buffer.hdr_buf_size = buf_size;
   nc_data_buffer.hdr_data_idx = hdr_data_idx;
   for (int index=0; index<cur_hdr_idx; index++) {
      // PrepBufr Message type
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
      
      if (is_pb_hdr && index < obs_vars.pb_hdr_cnt) {
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

   write_nc_header(obs_vars);
}

///////////////////////////////////////////////////////////////////////////////

void write_header_to_nc(const NetcdfObsVars &obs_vars,
      NcDataBuffer &data_buf, const int buf_size, const bool is_pb)
{
   long offsets[2] = { data_buf.hdr_data_offset, 0 };
   long lengths[1] = { buf_size } ;
   const char *method_name = "write_header_to_nc()";

   mlog << Debug(7) << "    " << method_name << "  buf_size: " << buf_size << "\n";
   
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
      int pb_hdr_len = obs_vars.pb_hdr_cnt - offsets[0];
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

void write_nc_obs_buffer(NcDataBuffer &data_buf, const int buf_size)
{
   const NetcdfObsVars &obs_vars = data_buf.obs_vars;
   long offsets[2] = { data_buf.obs_data_offset, 0 };
   long lengths[1] = { buf_size} ;
   const string method_name = "write_nc_obs_buffer()";

   mlog << Debug(7) << "    " << method_name << " offset: "
        << offsets[0] << ", " << offsets[1] << "  buf_size: " << buf_size << "\n";
   mlog << Debug(7) << "       obs_qty_var:  " << GET_NC_NAME(obs_vars.obs_qty_var) << "\n";
   
   //lengths[1] = HEADER_STR_LEN;
   if(!put_nc_data((NcVar *)&obs_vars.obs_qty_var, (int*)data_buf.qty_idx_buf, lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the quality flag to the "
           << "netCDF file\n\n";
      exit(1);
   }
   //lengths[1] = OBS_ARRAY_LEN;
   if(!put_nc_data((NcVar *)&obs_vars.obs_hid_var, (int*)data_buf.obs_hid_buf, lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the observation header index array to the "
           << "netCDF file\n\n";
      exit(1);
   }
   bool use_var_id = !IS_INVALID_NC(obs_vars.obs_vid_var);
   bool result = use_var_id
         ? put_nc_data((NcVar *)&obs_vars.obs_vid_var, (int*)data_buf.obs_vid_buf, lengths, offsets)
         : put_nc_data((NcVar *)&obs_vars.obs_gc_var,  (int*)data_buf.obs_vid_buf, lengths, offsets);
   if(!result) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the observation "
           << (use_var_id ? "variable_index" : "grib_code")
           << " array to the netCDF file\n\n";
      exit(1);
   }
   if(!put_nc_data((NcVar *)&obs_vars.obs_lvl_var, (float*)data_buf.obs_lvl_buf, lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the observation level array to the "
           << "netCDF file\n\n";
      exit(1);
   }
   if(!put_nc_data((NcVar *)&obs_vars.obs_hgt_var, (float*)data_buf.obs_hgt_buf, lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the observation hight array to the "
           << "netCDF file\n\n";
      exit(1);
   }
   if(!put_nc_data((NcVar *)&obs_vars.obs_val_var, (float*)data_buf.obs_val_buf, lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the observation data array to the "
           << "netCDF file\n\n";
      exit(1);
   }
   
   data_buf.obs_data_offset += buf_size;
   data_buf.obs_data_idx = 0;
}
      
///////////////////////////////////////////////////////////////////////////////

int write_nc_observations(const NetcdfObsVars &obs_vars,
                          const vector< Observation > observations,
                          const bool reset)
{
   int prev_hdr_idx = -1;
   string prev_header_type = "";
   string prev_station_id = "";
   ConcatString obs_qty;

   //float obs_arr[obs_arr_len];
   const string method_name = "write_nc_observations()";

   int obs_buf_size = observations.size();
   mlog << Debug(7) << "    " << method_name << "  obs_count: " << obs_buf_size << "\n";
   if (obs_buf_size > OBS_BUFFER_SIZE) obs_buf_size = OBS_BUFFER_SIZE;
   
   if (reset) {
      nc_data_buffer.obs_vars = obs_vars;
      nc_data_buffer.obs_buf_size = obs_buf_size;
      nc_data_buffer.obs_data_idx = 0;
      nc_data_buffer.obs_data_offset = 0;
      nc_data_buffer.hdr_data_idx = 0;
      nc_data_buffer.hdr_data_offset = 0;
      
      nc_data_buffer.processed_count =0;
   }
   float obs_arr[OBS_ARRAY_LEN];
   bool header_to_vector = IS_INVALID_NC(obs_vars.hdr_arr_var) || IS_INVALID_NC(obs_vars.hdr_lat_var);
   mlog << Debug(7) << "    " << method_name << "  header_to_vector: "
        << header_to_vector << "\n";
   for (vector< Observation >::const_iterator obs = observations.begin();
        obs != observations.end(); ++obs)
   {
      nc_data_buffer.processed_count++;
      
      if (obs->getHeaderIndex() != prev_hdr_idx) {
         mlog << Debug(9) << "    " << method_name << "  obs->getHeaderIndex(): "
              << obs->getHeaderIndex() << " at obs " << nc_data_buffer.processed_count << "\n";
         prev_hdr_idx = obs->getHeaderIndex();
         if (header_to_vector) {
            add_nc_header_all(obs->getHeaderType().c_str(),
                              obs->getStationId().c_str(),
                              obs->getValidTimeString().c_str(),
                              obs->getLatitude(),
                              obs->getLongitude(),
                              obs->getElevation());
         }
         else {
            write_nc_header(obs_vars,
                            obs->getHeaderType().c_str(),
                            obs->getStationId().c_str(),
                            obs->getValidTimeString().c_str(),
                            obs->getLatitude(),
                            obs->getLongitude(),
                            obs->getElevation());
         }
      }
      
      obs_arr[0] = obs->getHeaderIndex();
      obs_arr[1] = obs->getVarCode();
      obs_arr[2] = obs->getPressureLevel();
      obs_arr[3] = obs->getHeight();
      obs_arr[4] = obs->getValue();
      obs_qty = (obs->getQualityFlag().length() == 0 ? na_str : obs->getQualityFlag().c_str());
      write_nc_observation(obs_vars, nc_data_buffer, obs_arr, obs_qty.text());
      
      //if (nc_data_buffer.obs_data_idx >= nc_data_buffer.obs_buf_size) {
      //   write_nc_obs_buffer(nc_data_buffer, nc_data_buffer.obs_data_idx);
      //}
      
   } /* endfor - obs */
   
   if (nc_data_buffer.obs_data_idx > 0) {
      write_nc_obs_buffer(nc_data_buffer, nc_data_buffer.obs_data_idx);
   }

   //Caller handles writing headers

   return nc_data_buffer.processed_count;
}

///////////////////////////////////////////////////////////////////////////////
