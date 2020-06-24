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

#include "mtd_nc_output.h"

#include "write_netcdf.h"


////////////////////////////////////////////////////////////////////////


static NcDim  nx_dim ;
static NcDim  ny_dim ;
static NcDim  nt_dim ;


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

NcFile out(output_filename, NcFile::replace);

if ( IS_INVALID_NC(out) )  {

   mlog << Error << "\n\n  do_mtd_nc_output() -> trouble opening output file \""
        << output_filename << "\"\n\n";

   exit ( 1 );

}

const bool have_pairs =    (fcst_obj.n_objects() != 0)
                        && ( obs_obj.n_objects() != 0);

   //
   //  dimensions
   //

nx_dim = add_dim(&out,  nx_dim_name, fcst_raw.nx());
ny_dim = add_dim(&out,  ny_dim_name, fcst_raw.ny());
nt_dim = add_dim(&out,  nt_dim_name, fcst_raw.nt());

   //
   //  global attributes
   //

write_netcdf_global(&out, output_filename, "MTD",
                    config.model.c_str(), config.obtype.c_str(), config.desc.c_str());

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

NcFile out(output_filename, NcFile::replace);

if ( IS_INVALID_NC(out) )  {

   mlog << Error << "\n\n  do_mtd_nc_output[single]() -> trouble opening output file \""
        << output_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  dimensions
   //

nx_dim = add_dim(&out,  nx_dim_name, raw.nx());
ny_dim = add_dim(&out,  ny_dim_name, raw.ny());
nt_dim = add_dim(&out,  nt_dim_name, raw.nt());

   //
   //  global attributes
   //

write_netcdf_global(&out, output_filename, "MTD",
                    config.model.c_str(), config.obtype.c_str(), config.desc.c_str());

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

NcVar lat_var = add_var(&out, lat_name, ncFloat, ny_dim, nx_dim);
NcVar lon_var = add_var(&out, lon_name, ncFloat, ny_dim, nx_dim);

add_att(&lat_var, "long_name", "Latitude");
add_att(&lon_var, "long_name", "Longitude");

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

long offsets[2] = {0,0};
long lengths[2] = {ny, nx};
put_nc_data(&lat_var, lat_data, lengths, offsets);
put_nc_data(&lon_var, lon_data, lengths, offsets);

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

NcVar var = add_var(&out, name, ncFloat, nt_dim, ny_dim, nx_dim);

if ( is_fcst )  s = "Forecast Raw Data";
else            s = "Observed Raw Data";

add_att(&var, "long_name", s.text());

add_att(&var, "_FillValue", bad_data_float);

long offsets[3] = {0,0,0};
long lengths[3] = {nt,ny, nx};
put_nc_data(&var, raw.data(), lengths, offsets);

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

NcVar var = add_var(&out, name, ncInt, nt_dim, ny_dim, nx_dim);

if ( is_fcst )  s = "Forecast Object ID";
else            s = "Observed Object ID";

add_att(&var, "long_name", s.text());

add_att(&var, "_FillValue", bad_data_int);

long offsets[3] = {0,0,0};
long lengths[3] = {nt,ny, nx};
put_nc_data(&var, id.data(), lengths, offsets);


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

NcVar var = add_var(&out, name, ncInt, nt_dim, ny_dim, nx_dim);

if ( is_fcst )  s = "Forecast Cluster ID";
else            s = "Observed Cluster ID";

add_att(&var, "long_name", s.text());

add_att(&var, "_FillValue", bad_data_int);

   //
   //  create mapping array
   //

const int n_objects = ( is_fcst ? (e.n_fcst_simples()) : (e.n_obs_simples()) );

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

long offsets[3] = {0,0,0};
long lengths[3] = {nt,ny, nx};
put_nc_data(&var, out_data, lengths, offsets);

   //
   //  done
   //

if ( remap )  { delete [] remap;  remap = 0; }

if ( out_data )  { delete [] out_data;  out_data = 0; }

return;

}


////////////////////////////////////////////////////////////////////////
