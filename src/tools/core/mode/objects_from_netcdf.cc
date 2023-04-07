// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <netcdf>

#include "objects_from_netcdf.h"


////////////////////////////////////////////////////////////////////////


using namespace netCDF;


////////////////////////////////////////////////////////////////////////


   //
   //  these don't seem to be collected in any header file
   //
   //    that I could find
   //


static const string lat_dim_name = "lat";
static const string lon_dim_name = "lon";

static const string fcst_simple_id_var_name = "fcst_obj_id";
static const string  obs_simple_id_var_name =  "obs_obj_id";

static const string fcst_cluster_id_var_name = "fcst_clus_id";
static const string  obs_cluster_id_var_name =  "obs_clus_id";


////////////////////////////////////////////////////////////////////////


static void populate_bool_plane(const int * buf, const int nx, const int ny, BoolPlane & bp_out);


void  objects_from_arrays(const char *netcf_filename, bool do_clusters,
			  int *fcst_objects, int *obs_objects, int nx, int ny,
			  BoolPlane & fcst_out, 
			  BoolPlane & obs_out)
{
  populate_bool_plane(fcst_objects, nx, ny, fcst_out);
  populate_bool_plane(obs_objects, nx, ny, obs_out);
}  


////////////////////////////////////////////////////////////////////////


void objects_from_netcdf(const char * netcdf_filename, 
                         bool do_clusters,     //  do we look at cluster objects or simple objects?
                         BoolPlane & fcst_out, 
                         BoolPlane & obs_out)

{

int * buf = 0;
const string * fcst_var_name = 0;
const string *  obs_var_name = 0;

if ( do_clusters )  {

   fcst_var_name = &fcst_cluster_id_var_name;
    obs_var_name = &obs_cluster_id_var_name;

} else {

   fcst_var_name = &fcst_simple_id_var_name;
    obs_var_name = &obs_simple_id_var_name;

}


   //
   //  open the netcdf file
   //

NcFile nc((std::string) netcdf_filename, NcFile::read);

   //
   //  grab the lat/lon dimensions
   //

NcDim lat_dim, lon_dim;

lat_dim = nc.getDim(lat_dim_name);
lon_dim = nc.getDim(lon_dim_name);

   //
   //  use the lat/lon dimensions to set the size of the bool planes
   //

const int n_lat = (int) lat_dim.getSize();
const int n_lon = (int) lon_dim.getSize();
const int nx    = n_lon;
const int ny    = n_lat;

buf = new int [nx*ny];

   //
   //  grab the netcdf variables
   //

NcVar f_var, o_var;

f_var = nc.getVar(*fcst_var_name);
o_var = nc.getVar( *obs_var_name);

   //
   //  populate the bool planes
   //

f_var.getVar(buf);

populate_bool_plane(buf, nx, ny, fcst_out);

o_var.getVar(buf);

populate_bool_plane(buf, nx, ny, obs_out);

   //
   //  done
   //

if ( buf )  { delete [] buf;  buf = 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void populate_bool_plane(const int * buf, const int nx, const int ny, BoolPlane & bp_out)

{

int x, y, n, k;
bool tf;

bp_out.set_size(nx, ny);

for (x=0; x<nx; ++x)  {

   for (y=0; y<ny; ++y)  {

      n = y*nx + x;

      k = buf[n];

      tf = ( k > 0 );

      bp_out.put(tf, x, y);

   }   //  for y

}   //  for x




return;

}


////////////////////////////////////////////////////////////////////////



