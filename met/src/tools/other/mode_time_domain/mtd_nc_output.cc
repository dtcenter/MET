// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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

#include "mtd_nc_output.h"

#include "write_netcdf.h"


////////////////////////////////////////////////////////////////////////


static NcDim * nx_dim = 0;
static NcDim * ny_dim = 0;
static NcDim * nt_dim = 0;


////////////////////////////////////////////////////////////////////////


static void do_latlon     (NcFile & out, const Grid &);
static void do_raw        (NcFile & out, const MtdFloatFile & raw, const bool is_fcst);
static void do_object_id  (NcFile & out, const MtdIntFile   &  id, const bool is_fcst);
static void do_cluster_id (NcFile & out, const MtdIntFile   &  id, const bool is_fcst, const MM_Engine &);


////////////////////////////////////////////////////////////////////////


void do_mtd_nc_output(const MtdNcOutInfo &  nc_info, const MM_Engine    & engine,
                      const MtdFloatFile & fcst_raw, const MtdFloatFile & obs_raw,
                      const MtdIntFile   & fcst_obj, const MtdIntFile   & obs_obj,
                      const MtdConfigInfo & config,
                      const char * output_filename)

{

NcFile out(output_filename, NcFile::Replace);

if ( ! out.is_valid() )  {

   mlog << Error << "\n\n  do_mtd_nc_output() -> trouble opening output file \""
        << output_filename << "\"\n\n";

   exit ( 1 );

}

const bool have_pairs =    (fcst_obj.n_objects() != 0)
                        && ( obs_obj.n_objects() != 0);

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

write_netcdf_global(&out, output_filename, "MTD", config.model, config.obtype);

write_nc_grid(out, fcst_raw.grid());

   //
   //  variables
   //

if ( nc_info.do_latlon )  {

   do_latlon(out, fcst_raw.grid());

}


if ( nc_info.do_raw )  {

   do_raw(out, fcst_raw, true);
   do_raw(out,  obs_raw, false);

}

if ( nc_info.do_object_id )  {

   do_object_id (out, fcst_obj, true);
   do_object_id (out,  obs_obj, false);

}


if ( have_pairs && (nc_info.do_cluster_id) )  {

   do_cluster_id (out, fcst_obj, true,  engine);
   do_cluster_id (out,  obs_obj, false, engine);

}



   //
   //  done
   //

out.close();

return;

}


////////////////////////////////////////////////////////////////////////

   //
   //  for single fields
   //

void do_mtd_nc_output(const MtdNcOutInfo &  nc_info,
                      const MtdFloatFile & raw,
                      const MtdIntFile   & obj,
                      const MtdConfigInfo & config,
                      const char * output_filename)

{

NcFile out(output_filename, NcFile::Replace);

if ( ! out.is_valid() )  {

   mlog << Error << "\n\n  do_mtd_nc_output[single]() -> trouble opening output file \""
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


out.add_dim(nx_dim_name, raw.nx());
out.add_dim(ny_dim_name, raw.ny());
out.add_dim(nt_dim_name, raw.nt());

nx_dim = out.get_dim(nx_dim_name);
ny_dim = out.get_dim(ny_dim_name);
nt_dim = out.get_dim(nt_dim_name);

   //
   //  global attributes
   //

write_netcdf_global(&out, output_filename, "MTD", config.model, config.obtype);

write_nc_grid(out, raw.grid());

   //
   //  variables
   //

if ( nc_info.do_latlon )  {

   do_latlon(out, raw.grid());

}


if ( nc_info.do_raw )  {

   do_raw(out, raw, true);

}

if ( nc_info.do_object_id )  {

   do_object_id (out, obj, true);

}



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


out.add_var(lat_name, ncFloat, ny_dim, nx_dim);
out.add_var(lon_name, ncFloat, ny_dim, nx_dim);

NcVar * lat_var = out.get_var(lat_name);
NcVar * lon_var = out.get_var(lon_name);

lat_var->add_att("long_name", "Latitude");
lon_var->add_att("long_name", "Longitude");

float * lat_data = new float [nx*ny];
float * lon_data = new float [nx*ny];


Lat = lat_data;
Lon = lon_data;

for (y=0; y<ny; ++y)  {

   for (x=0; x<nx; ++x)  {

      grid.xy_to_latlon((double) x, (double) y, lat, lon);

      lon = -lon;

      *Lat++ = (float) lat;
      *Lon++ = (float) lon;

   }

}

lat_var->set_cur(0, 0);

lat_var->put(lat_data, ny, nx);


lon_var->set_cur(0, 0);

lon_var->put(lon_data, ny, nx);



   //
   //  done
   //

if ( lat_data )  { delete [] lat_data;  lat_data = 0; }
if ( lon_data )  { delete [] lon_data;  lon_data = 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void do_raw(NcFile & out, const MtdFloatFile & raw, const bool is_fcst)

{

const int nx = raw.nx();
const int ny = raw.ny();
const int nt = raw.nt();
ConcatString s;

const char * const name = ( is_fcst ? fcst_raw_name : obs_raw_name );

out.add_var(name, ncFloat, nt_dim, ny_dim, nx_dim);

NcVar * var = out.get_var(name);

if ( is_fcst )  s = "Forecast Raw Data";
else            s = "Observed Raw Data";

var->add_att("long_name", s.text());

var->add_att("_FillValue", bad_data_float);

var->set_cur(0, 0, 0);

var->put(raw.data(), nt, ny, nx);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_object_id (NcFile & out, const MtdIntFile & id, const bool is_fcst)

{

const int nx = id.nx();
const int ny = id.ny();
const int nt = id.nt();
ConcatString s;


const char * const name = ( is_fcst ? fcst_obj_id_name : obs_obj_id_name );

out.add_var(name, ncInt, nt_dim, ny_dim, nx_dim);

NcVar * var = out.get_var(name);

if ( is_fcst )  s = "Forecast Object ID";
else            s = "Observed Object ID";

var->add_att("long_name", s.text());

var->add_att("_FillValue", bad_data_int);

var->set_cur(0, 0, 0);

var->put(id.data(), nt, ny, nx);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_cluster_id (NcFile & out, const MtdIntFile & id, const bool is_fcst, const MM_Engine & e)

{

int j, k;
const int nx = id.nx();
const int ny = id.ny();
const int nt = id.nt();
const int * ip = id.data();
int * out_data = 0;
int * op = 0;
int * remap = 0;
ConcatString s;

const int n3 = nx*ny*nt;


out_data = new int [n3];

const char * const name = ( is_fcst ? fcst_clus_id_name : obs_clus_id_name );

out.add_var(name, ncInt, nt_dim, ny_dim, nx_dim);

NcVar * var = out.get_var(name);

if ( is_fcst )  s = "Forecast Cluster ID";
else            s = "Observed Cluster ID";

var->add_att("long_name", s.text());

var->add_att("_FillValue", bad_data_int);

   //
   //  create mapping array
   //

// const int n_clusters = e.n_composites();
const int n_objects = ( is_fcst ? (e.n_fcst_simples()) : (e.n_obs_simples()) );

// mlog << Debug(5) << "\n\n  " << n_clusters << " clusters\n\n";

remap = new int [n_objects + 1];

remap[0] = 0;

for (j=1; j<=n_objects; ++j)  {

   if ( is_fcst )  k = e.map_fcst_id_to_composite (j - 1);
   else            k = e.map_obs_id_to_composite  (j - 1);

   remap[j] = 1 + k;

   mlog << Debug(5) << "remap " << j << " = " << remap[j] << '\n';

}

op = out_data;

for (j=0; j<n3; ++j)  {

   *op++ = remap[*ip++];

}


var->set_cur(0, 0, 0);

var->put(out_data, nt, ny, nx);

   //
   //  done
   //

if ( remap )  { delete [] remap;  remap = 0; }

if ( out_data )  { delete [] out_data;  out_data = 0; }

return;

}


////////////////////////////////////////////////////////////////////////


