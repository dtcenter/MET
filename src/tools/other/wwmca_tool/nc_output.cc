// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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
#include "write_netcdf.h"


////////////////////////////////////////////////////////////////////////


static const float fill_value = bad_data_float;


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::do_output(const char * output_filename)

{

NcFile * ncfile   = (NcFile *) 0;
NcDim  * lat_dim  = (NcDim *)  0;
NcDim  * lon_dim  = (NcDim *)  0;
NcVar  * data_var = (NcVar *)  0;
unixtime init_time  = (unixtime) 0;
unixtime valid_time = (unixtime) 0;
int accum_time, x, y;
float f;
InterpolationValue iv;
ConcatString s;
const int Nx = ToGrid->nx();
const int Ny = ToGrid->ny();

   //
   //  list output file name
   //

mlog << Debug(1)
     << "Writing output file: " << output_filename << "\n";


   //
   //  open the netcdf file
   //

ncfile = new NcFile (output_filename, NcFile::Replace);

if ( !(ncfile->is_valid()) )  {

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

lat_dim = ncfile->add_dim("lat", ToGrid->ny());
lon_dim = ncfile->add_dim("lon", ToGrid->nx());

   //
   //  lat/lon variables
   //

write_netcdf_latlon(ncfile, lat_dim, lon_dim, *ToGrid);

   //
   //  variable attributes
   //

s = Config->lookup_string(conf_key_variable_name);

data_var = ncfile->add_var((const char *) s, ncFloat, lat_dim, lon_dim);

s = Config->lookup_string(conf_key_units);

data_var->add_att("units", (const char *) s);

s = Config->lookup_string(conf_key_long_name);

data_var->add_att("long_name", (const char *) s);

s = Config->lookup_string(conf_key_level);

data_var->add_att("level", (const char *) s);

data_var->add_att("_FillValue", fill_value);

   //
   //  variable timing attributes
   //

s = Config->lookup_string("valid_time");

valid_time = ( s.length() > 0 ?
               timestring_to_unix((const char *) s) :
               cp_nh->valid() );

if ( valid_time == (unixtime) 0 )  {

   mlog << Warning << "\nWwmcaRegridder::do_output(const char * output_filename) -> "
        << "valid time not defined in the filename or configuration file, writing 0.\n\n";

   exit ( 1 );

}

s = Config->lookup_string("init_time");

init_time = ( s.length() > 0 ?
              timestring_to_unix((const char *) s) :
              valid_time );

s = Config->lookup_string("accum_time");

accum_time = ( s.length() > 0 ?
               timestring_to_sec((const char *) s) :
               0);

write_netcdf_var_times(data_var, init_time, valid_time, accum_time);

   //
   //  global attributes
   //

grid_output(ToGrid->info(), ncfile);

   //
   //  fill in data values
   //

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      iv = get_interpolated_value(x, y);

      if ( iv.ok )  f = (float) (iv.value);
      else          f = fill_value;

      data_var->set_cur(y, x);

      data_var->put(&f, 1, 1);

   }

}

   //
   //  done
   //

if ( ncfile )  { delete ncfile;  ncfile = (NcFile *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////



