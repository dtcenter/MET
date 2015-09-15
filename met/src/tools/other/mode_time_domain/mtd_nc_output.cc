

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "mtd_nc_output.h"


////////////////////////////////////////////////////////////////////////


static NcDim * nx_dim = 0;
static NcDim * ny_dim = 0;
static NcDim * nt_dim = 0;


////////////////////////////////////////////////////////////////////////


static void do_latlon(NcFile & out, const Grid &);


////////////////////////////////////////////////////////////////////////


void do_mtd_nc_output(const MtdNcOutInfo & nc_info, const MM_Engine &, 
                      const MtdFloatFile & fcst_raw, const MtdFloatFile & obs_raw,
                      const MtdIntFile   & fcst_obj, const MtdIntFile   & obs_obj,
                      const char * output_filename)

{

NcFile out(output_filename, NcFile::Replace);

if ( ! out.is_valid() )  {

   cerr << "\n\n  do_mtd_nc_output() -> trouble opening output file \""
        << output_filename << "\"\n\n";

   exit ( 1 );

}

// nc_info.dump(cout);

nx_dim = 0;
ny_dim = 0;
nt_dim = 0;

   //
   //  dimensions
   //


out.add_dim(nx_dim_name, fcst_raw.nx());
out.add_dim(ny_dim_name, fcst_raw.ny());
out.add_dim(nt_dim_name, fcst_raw.nt());

nx_dim = out.get_dim(nx_dim_name);
ny_dim = out.get_dim(ny_dim_name);
nt_dim = out.get_dim(nt_dim_name);

   //
   //  global attributes
   //

   //
   //  variables
   //

if ( nc_info.do_latlon )  do_latlon(out, fcst_raw.grid());




   //
   //  done
   //

out.close();

return;

}


////////////////////////////////////////////////////////////////////////


void do_latlon(NcFile & out, const Grid & grid)

{

int x, y;
double lat, lon;
float * Lat = 0;
float * Lon = 0;
const int nx = grid.nx();
const int ny = grid.ny();

out.add_var(lat_name, ncFloat, nx_dim, ny_dim);
out.add_var(lon_name, ncFloat, nx_dim, ny_dim);

NcVar * lat_var = out.get_var(lat_name);
NcVar * lon_var = out.get_var(lon_name);

float * lat_data = new float [nx*ny];
float * lon_data = new float [nx*ny];


Lat = lat_data;
Lon = lon_data;

for (y=0; y<ny; ++y)  {

   for (x=0; x<nx; ++x)  {

      grid.latlon_to_xy((double) x, (double) y, lat, lon);

      *Lat++ = (float) lat;
      *Lon++ = (float) lon;

   }

}

lat_var->set_cur(0, 0);

lat_var->put(lat_data, nx, ny);


lon_var->set_cur(0, 0);

lon_var->put(lon_data, nx, ny);



   //
   //  done
   //

if ( lat_data )  { delete [] lat_data;  lat_data = 0; }
if ( lon_data )  { delete [] lon_data;  lon_data = 0; }

return;

}


////////////////////////////////////////////////////////////////////////


