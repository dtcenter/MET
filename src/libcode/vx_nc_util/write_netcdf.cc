// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////


#include <cstdio>
#include <ctime>
#include <iostream>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include <netcdf>

#include "vx_log.h"
#include "vx_cal.h"
#include "vx_util.h"
#include "write_netcdf.h"
#include "grid_output.h"

using namespace std;
using namespace netCDF;

///////////////////////////////////////////////////////////////////////////////

static void write_netcdf_latlon_1d(NcFile *, NcDim *, NcDim *, const Grid &);
static void write_netcdf_latlon_2d(NcFile *, NcDim *, NcDim *, const Grid &);
static void write_netcdf_grid_data(NcFile *, NcDim *, NcDim *,
                                   const char *, const char *,
                                   const char *, const char *,
                                   const DataPlane &);

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_global(NcFile * f_out, const char *file_name,
                         const char *program_name, const char *model_name,
                         const char *obtype, const char *desc)
{
   int yr, mon, day, hr, min, sec;
   char attribute_str[PATH_MAX];
   char hostname_str[max_str_len];
   char time_str[max_str_len];

   unix_to_mdyhms(time(nullptr), mon, day, yr, hr, min, sec);
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

void write_netcdf_proj(NcFile * f_out, const Grid & grid, NcDim & lat_dim, NcDim & lon_dim)
{

const GridInfo info = grid.info();

   //
   //  add range and azimuth dimensions for Range/Azimuth grids
   //

if ( info.ra )  {

   lat_dim = add_dim(f_out, "range",   (long) grid.ny() );
   lon_dim = add_dim(f_out, "azimuth", (long) grid.nx() );

}

   //
   //  add lat and lon dimensions for non-SemiLatLon grids
   //

else if ( !info.sl )  {

   lat_dim = add_dim(f_out, "lat", (long) grid.ny() );
   lon_dim = add_dim(f_out, "lon", (long) grid.nx() );

}

grid_output(info, f_out, lat_dim, lon_dim);

return;

}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_latlon(NcFile *f_out, NcDim *lat_dim, NcDim *lon_dim,
                         const Grid &grid) {
   // Write 1-dimensional arrays for lat/lon grids
   if(grid.info().ll != 0) {
      write_netcdf_latlon_1d(f_out, lat_dim, lon_dim, grid);
   }
   // Write 2-dimensional arrays for all others, except SemiLatLon
   else if(grid.info().sl == 0) {
      write_netcdf_latlon_2d(f_out, lat_dim, lon_dim, grid);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_latlon_1d(NcFile *f_out, NcDim *lat_dim, NcDim *lon_dim,
                            const Grid &grid) {
   double lat;
   double lon;
   NcVar lat_var;
   NcVar lon_var;

   // Allocate space for lat/lon values
   vector<float> lat_data(grid.ny());
   vector<float> lon_data(grid.nx());

   // Define Variables
   lat_var = f_out->addVar("lat", ncFloat, *lat_dim);
   lon_var = f_out->addVar("lon", ncFloat, *lon_dim);

   // Add variable attributes
   add_att(&lat_var, long_name_att_name, "latitude");
   add_att(&lat_var, units_att_name, "degrees_north");
   add_att(&lat_var, standard_name_att_name, "latitude");

   add_att(&lon_var, long_name_att_name, "longitude");
   add_att(&lon_var, units_att_name, "degrees_east");
   add_att(&lon_var, standard_name_att_name, "longitude");

#pragma omp parallel default(none) \
   shared(grid, lat_data, lon_data) \
   private(lat, lon)
   {

      // Compute latitude values
#pragma omp for schedule(static)
      for(int i=0; i<grid.ny(); i++) {
         grid.xy_to_latlon(0, i, lat, lon);
         lat_data[i] = (float) lat;
      }

      // Compute longitude values (convert degrees west to east)
#pragma omp for schedule(static)
      for(int i=0; i<grid.nx(); i++) {
         grid.xy_to_latlon(i, 0, lat, lon);
         lon_data[i] = (float) -1.0*lon;
      }
   } // End omp parallel

   // Write the lat data
   put_nc_data(&lat_var, lat_data.data(), lat_dim->getSize(), 0);

   // Write the lon data
   put_nc_data(&lon_var, lon_data.data(), lon_dim->getSize(), 0);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_latlon_2d(NcFile *f_out, NcDim *lat_dim, NcDim *lon_dim,
                            const Grid &grid) {
   NcVar lat_var;
   NcVar lon_var;
   vector<NcDim> dims;
   long  counts[2] = {grid.ny(), grid.nx()};
   long offsets[2] = {0 , 0};

   // Allocate space for lat/lon values
   vector<float> lat_data(grid.nx()*grid.ny());
   vector<float> lon_data(grid.nx()*grid.ny());

   // Define Variables
   dims.emplace_back(*lat_dim);
   dims.emplace_back(*lon_dim);
   lat_var = add_var(f_out, "lat", ncFloat, dims);
   lon_var = add_var(f_out, "lon", ncFloat, dims);

   // Add variable attributes
   add_att(&lat_var, long_name_att_name, "latitude");
   add_att(&lat_var, units_att_name, "degrees_north");
   add_att(&lat_var, standard_name_att_name, "latitude");

   add_att(&lon_var, long_name_att_name, "longitude");
   add_att(&lon_var, units_att_name, "degrees_east");
   add_att(&lon_var, standard_name_att_name, "longitude");

#pragma omp parallel default(none) \
   shared(grid, lat_data, lon_data, DefaultTO)
   {

      // Compute lat/lon values
#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<grid.nx(); x++) {
         for(int y=0; y<grid.ny(); y++) {

            double lat;
            double lon;
            grid.xy_to_latlon(x, y, lat, lon);
            int i = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);

            // Multiple by -1.0 to convert from degrees west to degrees east
            lat_data[i] = (float) lat;
            lon_data[i] = (float) -1.0*lon;
         }
      }
   } // End omp parallel

   // Write the lat data
   put_nc_data(&lat_var, lat_data.data(), counts, offsets);

   // Write the lon data
   put_nc_data(&lon_var, lon_data.data(), counts, offsets);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_grid_weight(NcFile *f_out, NcDim *lat_dim, NcDim *lon_dim,
                              const GridWeightType t, const DataPlane &wgt_dp,
                              const DataPlane *area_dp) {

   // Write the grid weights
   switch(t) {

      case GridWeightType::Cos_Lat:
         write_netcdf_grid_data(f_out, lat_dim, lon_dim, "grid_weight",
                                "weight", "cosine latitude grid weight",
                                "NA", wgt_dp);
         break;

      case GridWeightType::Area:
         write_netcdf_grid_data(f_out, lat_dim, lon_dim, "grid_weight",
                                "weight", "normalized true area grid weight",
                                "NA", wgt_dp);
         break;

      default:
         write_netcdf_grid_data(f_out, lat_dim, lon_dim, "grid_weight",
                                "weight", "default grid weight",
                                "NA", wgt_dp);
         break;
   }

   // Write the true grid box areas, if provided
   if(t == GridWeightType::Area && area_dp && area_dp->nxy() > 0) {
      write_netcdf_grid_data(f_out, lat_dim, lon_dim, "grid_area",
                             "cell_area", "true grid box area",
                             "km^2", *area_dp);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_grid_data(NcFile *f_out, NcDim *lat_dim, NcDim *lon_dim,
                            const char *var_name, const char *standard_name,
                            const char *long_name, const char *units,
                            const DataPlane &dp) {
   NcVar nc_var;
   vector<NcDim> dims;

   // Allocate space for the data values
   vector<float> nc_data(dp.nx()*dp.ny());

   // Define Variables
   dims.emplace_back(*lat_dim);
   dims.emplace_back(*lon_dim);
   nc_var = add_var(f_out, var_name, ncFloat, dims);

   // Add variable attributes
   add_att(&nc_var, "standard_name", standard_name);
   add_att(&nc_var, long_name_att_name, long_name);
   add_att(&nc_var, units_att_name, units);

#pragma omp parallel default(none) \
   shared(dp, nc_data, DefaultTO)
   {

      // Store data values
#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<dp.nx(); x++) {
         for(int y=0; y<dp.ny(); y++) {
            int i = DefaultTO.two_to_one(dp.nx(), dp.ny(), x, y);
            nc_data[i] = (float) dp(x, y);
         }
      }
   } // End omp parallel

   // Write the data
   put_nc_data_with_dims(&nc_var, nc_data.data(), dp.ny(), dp.nx());

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
   add_att(var, init_time_att_name, time_str.text());

   s = unixtime_to_string(init_ut);

   add_att(var, init_time_ut_att_name, s.text());

   // Valid time
   unix_to_yyyymmdd_hhmmss(valid_ut, time_str);
   add_att(var, valid_time_att_name, time_str.text());

   s = unixtime_to_string(valid_ut);

   add_att(var, valid_time_ut_att_name, s.text());

   // Accumulation time
   if(accum_sec != 0) {
     sec_to_hhmmss(accum_sec, time_str);
      add_att(var, accum_time_att_name, time_str.text());
      var->putAtt(accum_time_sec_att_name, ncInt, accum_sec);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

