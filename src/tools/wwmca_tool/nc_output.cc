

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "wwmca_ref.h"

#include "grid_output.h"
#include "write_netcdf.h"


////////////////////////////////////////////////////////////////////////


static const double fill_value = -9999.0;


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::do_output(const char * output_filename)

{

NcFile * ncfile   = (NcFile *) 0;
NcDim  * lat_dim  = (NcDim *)  0;
NcDim  * lon_dim  = (NcDim *)  0;
NcVar  * lat_var  = (NcVar *)  0;
NcVar  * lon_var  = (NcVar *)  0;
NcVar  * data_var = (NcVar *)  0;
unixtime valid_time = 0LL;
unixtime issue_time = 0LL;
int month, day, year, hour, minute, second;
int j, x, y;
float f[2];
double lat, lon;
InterpolationValue iv;
char junk[256];
ConcatString s;
Result r;
// const LambertData & ldata = *(ginfo.lc);
const int Nx = ToGrid->nx();
const int Ny = ToGrid->ny();


valid_time = cp_nh->valid();

issue_time = valid_time;

   //
   //  open the netcdf file
   //

ncfile = new NcFile (output_filename, NcFile::Replace);

if ( !(ncfile->is_valid()) )  {

   cerr << "\n\n  WwmcaRegridder::do_lambert_output(const char * output_filename) -> Netcdf file is not valid!\n\n";

   exit ( 1 );

}

   //
   //  global attributes
   //

write_netcdf_global(ncfile, output_filename, "wwmca_regrid");

   //
   //  dimensions
   //

ncfile->add_dim("lat", ToGrid->ny());
ncfile->add_dim("lon", ToGrid->nx());

lat_dim = ncfile->get_dim("lat");
lon_dim = ncfile->get_dim("lon");

ncfile->add_var("lat",  ncFloat, lat_dim, lon_dim);
lat_var = ncfile->get_var("lat");



lat_var->add_att("units",      "degrees_north");
lat_var->add_att("long_name",  "Latitude");
lat_var->add_att("_FillValue", -9999.f);   //  don't use NC_FILL_FLOAT, I guess ...

ncfile->add_var("lon",  ncFloat, lat_dim, lon_dim);
lon_var = ncfile->get_var("lon");

lon_var->add_att("units",      "degrees_east");
lon_var->add_att("long_name",  "Longitude");
lon_var->add_att("_FillValue", -9999.f);

r = Config->variable_name();

s = r.sval();

ncfile->add_var((const char *) s, ncFloat, lat_dim, lon_dim);
data_var = ncfile->get_var((const char *) s);

j = Config->grib_code();

data_var->add_att("grib_code",   j);

r = Config->units();

s = r.sval();

data_var->add_att("units",      (const char *) s);

r = Config->long_name();

s = r.sval();

data_var->add_att("long_name",  (const char *) s);

r = Config->level();

s = r.sval();

data_var->add_att("level",      (const char *) s);

data_var->add_att("_FillValue",  -9999.f);

unix_to_mdyhms(issue_time, month, day, year, hour, minute, second);

sprintf(junk, "%04d%02d%02d_%02d%02d%02d", year, month, day, hour, minute, second);

data_var->add_att("init_time",  junk);

data_var->add_att("init_time_ut",  (long) issue_time);


unix_to_mdyhms(valid_time, month, day, year, hour, minute, second);

sprintf(junk, "%04d%02d%02d_%02d%02d%02d", year, month, day, hour, minute, second);

data_var->add_att("valid_time",  junk);

data_var->add_att("valid_time_ut",  (long) valid_time);

data_var->add_att("accum_time",  "1 hour");

   //
   //  global attributes
   //

grid_output(ginfo, ncfile);

   //
   //  fill in lat/lon values
   //

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      ToGrid->xy_to_latlon((double) x, (double) y, lat, lon);

      if ( west_longitude_positive )  lon = -lon;   //  east -> west


      f[0] = (float) lat;

      lat_var->set_cur(y, x);

      lat_var->put(f, 1, 1);


      f[0] = (float) lon;

      lon_var->set_cur(y, x);

      lon_var->put(f, 1, 1);

   }

}

   //
   //  fill in data values
   //

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      iv = get_interpolated_value(x, y);

      if ( iv.ok )  f[0] = (float) (iv.value);
      else          f[0] = (float) fill_value;

      data_var->set_cur(y, x);

      data_var->put(f, 1, 1);

   }

}

   //
   //  done
   //

if ( ncfile )  { delete ncfile;  ncfile = (NcFile *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////



