// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_log.h"
#include "vx_cal.h"

#include "wwmca_ref.h"

#include "grid_output.h"
#include "nc_utils.h"
#include "write_netcdf.h"


////////////////////////////////////////////////////////////////////////

extern int get_compress();

////////////////////////////////////////////////////////////////////////


static const float fill_value = bad_data_float;


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::do_output(const char * output_filename)

{

   NcFile * ncfile   = (NcFile *) 0;
   NcDim   lat_dim  ;
   NcDim   lon_dim  ;
   NcVar   data_var ;
   unixtime init_time  = (unixtime) 0;
   unixtime valid_time = (unixtime) 0;
   int accum_time, x, y;
   float f;
   double v;
   ConcatString s;
   const int Nx = ToGrid->nx();
   const int Ny = ToGrid->ny();

   //
   //  open the netcdf file
   //

   ncfile = open_ncfile(output_filename, true);
   if ( IS_INVALID_NC_P(ncfile) )  {
      mlog << Error << "\nWwmcaRegridder::do_output(const char * output_filename) -> Netcdf file is not valid!\n\n";
      exit ( 1 );
   }

   //
   //  global attributes
   //

   write_netcdf_global(ncfile, output_filename, "wwmca_regrid");

   //
   //  dimensions
   //

   lat_dim = add_dim(ncfile, "lat", ToGrid->ny());
   lon_dim = add_dim(ncfile, "lon", ToGrid->nx());

   //
   //  lat/lon variables
   //

   write_netcdf_latlon(ncfile, &lat_dim, &lon_dim, *ToGrid);

   //
   //  variable attributes
   //

   int deflate_level = get_compress();
   if (deflate_level < 0) deflate_level = Config->nc_compression();

   s = Config->lookup_string(conf_key_variable_name);
   
   data_var = add_var(ncfile, s.c_str(), ncFloat, lat_dim, lon_dim, deflate_level);
   
   s = Config->lookup_string(conf_key_units);
   
   add_att(&data_var, "units", s.c_str());
   
   s = Config->lookup_string(conf_key_long_name);
   
   add_att(&data_var, "long_name", s.c_str());
   
   s = Config->lookup_string(conf_key_level);
   
   add_att(&data_var, "level", s.c_str());
   
   add_att(&data_var, "_FillValue", fill_value);

   //
   //  variable timing attributes
   //

   s = Config->lookup_string("valid_time");
   
   if ( s.length() > 0 )  {
   
      valid_time = timestring_to_unix(s.c_str());
   
      mlog << Debug(2) << "Parsed valid time ("
           << unix_to_yyyymmdd_hhmmss(valid_time)
           << ") from config file.\n";
   
      if ( cp_nh->valid() != (unixtime) 0 &&
           cp_nh->valid() != valid_time )  {
   
         mlog << Warning << "\nWwmcaRegridder::do_output(const char * output_filename) -> "
              << "the config file valid time ("
              << unix_to_yyyymmdd_hhmmss(valid_time)
              << ") and data file valid time ("
              << unix_to_yyyymmdd_hhmmss(cp_nh->valid())
              << ") do not match, using config file time.\n\n";
   
      }
   
   }
   else  {
      valid_time = cp_nh->valid();
   }
   
   if ( valid_time == (unixtime) 0 )  {
   
      mlog << Warning << "\nWwmcaRegridder::do_output(const char * output_filename) -> "
           << "valid time not defined in the data file or config file, writing "
           << unix_to_yyyymmdd_hhmmss((unixtime) 0) << ".\n\n";
   
   }
   
   s = Config->lookup_string("init_time");
   
   if ( s.length() > 0 )  {
   
      init_time = timestring_to_unix(s.c_str());
   
      mlog << Debug(2) << "Parsed initialization time ("
           << unix_to_yyyymmdd_hhmmss(init_time)
           << ") from config file.\n";
   
      if ( cp_nh->init() != (unixtime) 0 &&
           cp_nh->init() != init_time )  {
   
         mlog << Warning << "\nWwmcaRegridder::do_output(const char * output_filename) -> "
              << "the config file initialization time ("
              << unix_to_yyyymmdd_hhmmss(init_time)
              << ") and data file initialization time ("
              << unix_to_yyyymmdd_hhmmss(cp_nh->init())
              << ") do not match, using config file time.\n\n";
   
      }
   
   }
   else  {
      init_time = cp_nh->init();
   }
   
   s = Config->lookup_string("accum_time");
   
   accum_time = ( s.length() > 0 ?
                  timestring_to_sec(s.c_str()) :
                  0);
   
   write_netcdf_var_times(&data_var, init_time, valid_time, accum_time);

   //
   //  global attributes
   //

   grid_output(ToGrid->info(), ncfile);

   //
   //  fill in data values
   //

   DataPlane dp;
   
   get_interpolated_data(dp);
   
   
   long offsets[2] = {0,0};
   long lengths[2] = {1,1};
   for (x=0; x<Nx; ++x)  {
   
      for (y=0; y<Ny; ++y)  {
   
         v = dp(x, y);
   
         if ( is_bad_data(v) ) f = fill_value;
         else                  f = (float) v;
   
         offsets[0] = y;
         offsets[1] = x;
         put_nc_data(&data_var, &f, lengths, offsets);
   
      }
   
   }

   //
   //  done
   //

   if ( ncfile )  { delete ncfile;  ncfile = (NcFile *) 0; }

   //
   //  list output file name
   //

   mlog << Debug(1)
        << "Writing output file: " << output_filename << "\n";
   
   return;

}


////////////////////////////////////////////////////////////////////////



