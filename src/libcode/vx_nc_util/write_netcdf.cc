// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
   snprintf(time_str, sizeof(time_str), "%.4i%.2i%.2i_%.2i%.2i%.2i",
           yr, mon, day, hr, min, sec);
   gethostname(hostname_str, max_str_len);
   snprintf(attribute_str, sizeof(attribute_str),
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

   if ( lat_data )  { delete [] lat_data;  lat_data = 0; }
   if ( lon_data )  { delete [] lon_data;  lon_data = 0; }

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

   if ( lat_data )  { delete [] lat_data;  lat_data = 0; }
   if ( lon_data )  { delete [] lon_data;  lon_data = 0; }

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

ConcatString time_str;
ConcatString s;

   // Init time
   unix_to_yyyymmdd_hhmmss(init_ut, time_str);
   add_att(var, "init_time", time_str.text());

   s = unixtime_to_string(init_ut);

   add_att(var, "init_time_ut", s.text());

   // Valid time
   unix_to_yyyymmdd_hhmmss(valid_ut, time_str);
   add_att(var, "valid_time", time_str.text());

   s = unixtime_to_string(valid_ut);

   add_att(var, "valid_time_ut", s.text());

   // Accumulation time
   if(accum_sec != 0) {
     sec_to_hhmmss(accum_sec, time_str);
      add_att(var, "accum_time", time_str.text());
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
