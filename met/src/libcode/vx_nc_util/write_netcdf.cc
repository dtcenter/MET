// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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

extern void write_netcdf_latlon_1d(NcFile *, NcDim *, NcDim *, const Grid &);
extern void write_netcdf_latlon_2d(NcFile *, NcDim *, NcDim *, const Grid &);

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
f_out->add_att("FileOrigins", attribute_str);
f_out->add_att("MET_version", met_version);
f_out->add_att("MET_tool", program_name);
if(model_name) f_out->add_att("model",  model_name);
if(obtype)     f_out->add_att("obtype", obtype);
if(desc)       f_out->add_att("desc",   desc);

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
   NcVar *lat_var  = (NcVar *) 0;
   NcVar *lon_var  = (NcVar *) 0;
   float *lat_data = (float *) 0;
   float *lon_data = (float *) 0;

   // Define Variables
   lat_var = f_out->add_var("lat", ncFloat, lat_dim);
   lon_var = f_out->add_var("lon", ncFloat, lon_dim);

   // Add variable attributes
   lat_var->add_att("long_name", "latitude");
   lat_var->add_att("units", "degrees_north");
   lat_var->add_att("standard_name", "latitude");

   lon_var->add_att("long_name", "longitude");
   lon_var->add_att("units", "degrees_east");
   lon_var->add_att("standard_name", "longitude");

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
   if(!lat_var->put(&lat_data[0], lat_dim->size())) {
      mlog << Error << "\nwrite_netcdf_latlon() -> "
           << "error with lat_var->put\n\n";
      exit(1);
   }

   // Write the lon data
   if(!lon_var->put(&lon_data[0], lon_dim->size())) {
      mlog << Error << "\nwrite_netcdf_latlon() -> "
           << "error with lon_var->put\n\n";
      exit(1);
   }

   // Clean up
   if(lat_data) { delete [] lat_data; lat_data = (float *) 0; }
   if(lon_data) { delete [] lon_data; lon_data = (float *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_latlon_2d(NcFile *f_out, NcDim *lat_dim, NcDim *lon_dim,
                            const Grid &grid) {
   int i, x, y;
   double lat, lon;
   NcVar *lat_var  = (NcVar *) 0;
   NcVar *lon_var  = (NcVar *) 0;
   float *lat_data = (float *) 0;
   float *lon_data = (float *) 0;

   // Define Variables
   lat_var = f_out->add_var("lat", ncFloat, lat_dim, lon_dim);
   lon_var = f_out->add_var("lon", ncFloat, lat_dim, lon_dim);

   // Add variable attributes
   lat_var->add_att("long_name", "latitude");
   lat_var->add_att("units", "degrees_north");
   lat_var->add_att("standard_name", "latitude");

   lon_var->add_att("long_name", "longitude");
   lon_var->add_att("units", "degrees_east");
   lon_var->add_att("standard_name", "longitude");

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
   if(!lat_var->put(&lat_data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_netcdf_latlon() -> "
           << "error with lat_var->put\n\n";
      exit(1);
   }

   // Write the lon data
   if(!lon_var->put(&lon_data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_netcdf_latlon() -> "
           << "error with lon_var->put\n\n";
      exit(1);
   }

   // Clean up
   if(lat_data) { delete [] lat_data; lat_data = (float *) 0; }
   if(lon_data) { delete [] lon_data; lon_data = (float *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_grid_weight(NcFile *f_out, NcDim *lat_dim, NcDim *lon_dim,
                              const GridWeightType t, const DataPlane &wgt_dp) {
   int i, x, y;
   NcVar *wgt_var  = (NcVar *) 0;
   float *wgt_data = (float *) 0;

   // Define variable
   wgt_var = f_out->add_var("grid_weight", ncFloat, lat_dim, lon_dim);

   // Add variable attributes
   wgt_var->add_att("standard_name", "weight");

   switch(t) {

      case GridWeightType_Cos_Lat:
         wgt_var->add_att("long_name", "cosine latitude grid weight");
         wgt_var->add_att("units", "NA");
         break;

      case GridWeightType_Area:
         wgt_var->add_att("long_name", "true area grid weight");
         wgt_var->add_att("units", "km^2");
         break;

      default:
         wgt_var->add_att("long_name", "default grid weight");
         wgt_var->add_att("units", "NA");
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
   if(!wgt_var->put(&wgt_data[0], wgt_dp.ny(), wgt_dp.nx())) {
      mlog << Error << "\nwrite_netcdf_grid_weight() -> "
           << "error with wgt_var->put\n\n";
      exit(1);
   }

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
   var->add_att("init_time", time_str);

   s = unixtime_to_string(init_ut);

   var->add_att("init_time_ut", s.text());

   // Valid time
   unix_to_yyyymmdd_hhmmss(valid_ut, time_str);
   var->add_att("valid_time", time_str);

   s = unixtime_to_string(valid_ut);

   var->add_att("valid_time_ut", s.text());

   // Accumulation time
   if(accum_sec > 0) {
      sec_to_hhmmss(accum_sec, time_str);
      var->add_att("accum_time", time_str);
      var->add_att("accum_time_sec", accum_sec);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
