// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_met_util/read_netcdf.h"

///////////////////////////////////////////////////////////////////////////////


static void get_att(NcFile *, NcAtt * & att, const char * name);
static bool has_att(NcFile *, const char * name);

static void read_netcdf_grid_v3       (NcFile *, Grid &,              int verbosity);
static void read_netcdf_grid_v2       (NcFile *, Grid &,              int verbosity);

static void get_latlon_data_v3        (NcFile *, LatLonData &,        int verbosity);
static void get_lambert_data_v3       (NcFile *, LambertData &,       int verbosity);
static void get_stereographic_data_v3 (NcFile *, StereographicData &, int verbosity);
static void get_mercator_data_v3      (NcFile *, MercatorData &,      int verbosity);

static void get_latlon_data_v2        (NcFile *, LatLonData &,        int verbosity);
static void get_lambert_data_v2       (NcFile *, LambertData &,       int verbosity);
static void get_stereographic_data_v2 (NcFile *, StereographicData &, int verbosity);
static void get_mercator_data_v2      (NcFile *, MercatorData &,      int verbosity);


///////////////////////////////////////////////////////////////////////////////


void read_netcdf_grid(NcFile * f_in, Grid & gr, int verbosity) {
   NcAtt * proj_att = (NcAtt *) 0;
   bool v3_flag = false;

   // Check for the MET version global attribute
   if(has_att(f_in, "MET_version")) {

      // Get the version number from the NetCDF file
      get_att(f_in, proj_att, "MET_version");

      // Check to see if it matches the current version
      if(strcmp(proj_att->as_string(0), met_version) == 0) v3_flag = true;
   }

   // Parse the projection information based on the version
   if(v3_flag) read_netcdf_grid_v3(f_in, gr, verbosity);
   else        read_netcdf_grid_v2(f_in, gr, verbosity);

   return;
}


///////////////////////////////////////////////////////////////////////////////


void read_netcdf_grid_v3(NcFile * f_in, Grid & gr, int verbosity)

{

NcAtt * proj_att = (NcAtt *) 0;

   // Structures to store projection info
   LambertData       lc_data;
   StereographicData st_data;
   LatLonData        ll_data;
   MercatorData      mc_data;

       //
       // Parse the grid specification out of the global attributes
       //

   get_att(f_in, proj_att, "Projection");

      //
      // Parse out the grid specification depending on the projection type
      // The following 4 Projection types are supported:
      //    - Lat/Lon
      //    - Mercator
      //    - Lambert Conformal
      //    - Polar Stereographic
      //

   if ( strcmp(proj_att->as_string(0), latlon_proj_type) == 0 )  {

      get_latlon_data_v3(f_in, ll_data, verbosity);

      gr.set(ll_data);

   } else if ( strcmp(proj_att->as_string(0), mercator_proj_type) == 0 )  {

      get_mercator_data_v3(f_in, mc_data, verbosity);

      gr.set(mc_data);

   } else if ( strcmp(proj_att->as_string(0), lambert_proj_type) == 0 )  {

      get_lambert_data_v3(f_in, lc_data, verbosity);

      gr.set(lc_data);

   } else if ( strcmp(proj_att->as_string(0), stereographic_proj_type) == 0 )  {

      get_stereographic_data_v3(f_in, st_data, verbosity);

      gr.set(st_data);

   } else {   // Unsupported projection type

      cerr << "\n\nERROR: read_netcdf_grid_v3() -> "
           << "Projection type " << proj_att->as_string(0)
           << " not currently supported.\n\n"
           << flush;

      exit(1);

   }

   //
   //  done
   //

return;

}

///////////////////////////////////////////////////////////////////////////////


void read_netcdf_grid_v2(NcFile * f_in, Grid & gr, int verbosity)

{

NcAtt * proj_att = (NcAtt *) 0;

   // Structures to store projection info
   LambertData       lc_data;
   StereographicData st_data;
   LatLonData        ll_data;
   MercatorData      mc_data;

       //
       // Parse the grid specification out of the global attributes
       //

   get_att(f_in, proj_att, "Projection");

      //
      // Parse out the grid specification depending on the projection type
      // The following 4 Projection types are supported:
      //    - Lat/Lon
      //    - Mercator
      //    - Lambert Conformal
      //    - Polar Stereographic
      //

   if ( strcmp(proj_att->as_string(0), latlon_proj_type) == 0 )  {

      get_latlon_data_v2(f_in, ll_data, verbosity);

      gr.set(ll_data);

   } else if ( strcmp(proj_att->as_string(0), mercator_proj_type) == 0 )  {

      get_mercator_data_v2(f_in, mc_data, verbosity);

      gr.set(mc_data);

   } else if ( strcmp(proj_att->as_string(0), lambert_proj_type) == 0 )  {

      get_lambert_data_v2(f_in, lc_data, verbosity);

      gr.set(lc_data);

   } else if ( strcmp(proj_att->as_string(0), stereographic_proj_type) == 0 )  {

      get_stereographic_data_v2(f_in, st_data, verbosity);

      gr.set(st_data);

   } else {   // Unsupported projection type

      cerr << "\n\nERROR: read_netcdf_grid_v2() -> "
           << "Projection type " << proj_att->as_string(0)
           << " not currently supported.\n\n"
           << flush;

      exit(1);

   }

   //
   //  done
   //

return;

}

///////////////////////////////////////////////////////////////////////////////

void read_netcdf(NcFile *f_in, const char *var_name, char *lvl_name,
                 WrfData &wd, Grid &gr, int verbosity) {
   int status;

   status = read_netcdf_status(f_in, var_name, lvl_name, wd, gr, verbosity);

   if(status != 0) {
      cerr << "\n\nERROR: read_netcdf() -> "
           << "\"" << var_name << "\" variable at \""
           << lvl_name << "\" level not found.\n\n" << flush;
      exit(1);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

int read_netcdf_status(NcFile *f_in, const char *var_name, char *lvl_name,
                       WrfData &wd, Grid &gr, int verbosity) {
   double v, v_min, v_max;
   int i, x, y, accum;
   unixtime init_ut, valid_ut;

   float *pcp_data = (float *) 0;

   NcVar *pcp_var  = (NcVar *) 0;
   NcAtt *lvl_att  = (NcAtt *) 0;
   NcAtt *time_att = (NcAtt *) 0;

   //
   // Parse the grid specification out of the global attributes
   //
   read_netcdf_grid(f_in, gr, verbosity);

   //
   // Check whether the file contains the requested variable
   //
   if(!has_variable(f_in, var_name)) {
      cout << "\n\nWARNING: read_netcdf_status() -> "
           << "\"" << var_name << "\" variable not found.\n\n"
           << flush;
      return(1);
   }

   //
   // Access the precip variable in the NetCDF file
   //
   pcp_var = f_in->get_var(var_name);
   if(!pcp_var || !pcp_var->is_valid()) {
      cout << "\n\nWARNING: read_netcdf_status() -> "
           << "Trouble reading \"" << var_name << "\" variable.\n\n"
           << flush;
      return(1);
   }

   //
   // Extract the level name from the precip variable
   //
   lvl_att = pcp_var->get_att("level");
   if(!lvl_att) {
      cout << "\n\nWARNING: read_netcdf_status() -> "
           << "\"level\" attribute not found for the \""
           << var_name << "\" variable.\n\n" << flush;
      return(1);
   }
   strcpy(lvl_name, lvl_att->as_string(0));

   //
   // Extract the valid, lead, and accumulation time attributes from
   // the precip variable
   //
   time_att = pcp_var->get_att("init_time_ut");
   if(!time_att) {
      cout << "\n\nWARNING: read_netcdf_status() -> "
           << "\"init_time_ut\" attribute not found for the \""
           << var_name << "\" variable.\n\n" << flush;
      return(1);
   }
   init_ut = time_att->as_long(0);

   time_att = pcp_var->get_att("valid_time_ut");
   if(!time_att) {
      cout << "\n\nWARNING: read_netcdf_status() -> "
           << "\"valid_time_ut\" attribute not found for the \""
           << var_name << "\" variable.\n\n" << flush;
      return(1);
   }
   valid_ut = time_att->as_long(0);

   //
   // Get the accumulation time.
   // Search for the accum_time_sec variable attribute. If not found,
   // read the accum_time attribute string and interpret its value as hours.
   //
   for(i=0, accum=-1; i<pcp_var->num_atts(); i++) {

      time_att = pcp_var->get_att(i);
      if(strcmp(time_att->name(), "accum_time_sec") == 0) {
         accum = time_att->as_int(0);
         break;
      }
   }

   if(accum == -1) {

      time_att = pcp_var->get_att("accum_time");
      if(!time_att) {
         cout << "\n\nWARNING: read_netcdf_status() -> "
              << "\"accum_time\" attribute not found for the \""
              << var_name << "\" variable.\n\n" << flush;
         return(1);
      }
      accum = timestring_to_sec(time_att->as_string(0));
   }

   //
   // Allocate space for the precip data
   //
   pcp_data = new float [gr.nx()*gr.ny()];

   if( !pcp_var->get(&pcp_data[0], gr.ny(), gr.nx()) ) {
      cout << "\n\nWARNING: read_netcdf_status() -> "
           << "error with the pcp_var->get\n\n" << flush;
      return(1);
   }

   //
   // Set up the size of the wrfdata object
   //
   wd.set_size(gr.nx(), gr.ny());

   //
   // Read through the precip data to find the min/max values
   //
   v_min = 1.0e30;
   v_max = -1.0e30;

   for(x=0; x<gr.nx(); x++) {
      for(y=0; y<gr.ny(); y++) {
         v = (double) pcp_data[wd.two_to_one(x, y)];
         if(!is_bad_data(v) && v > v_max) v_max = v;
         if(!is_bad_data(v) && v < v_min) v_min = v;
      }
   }

   if(verbosity > 2) {
      cout << "NetCDF " << var_name << " Variable (min, max) = ("
           << v_min << ", " << v_max << ")\n" << flush;
   }

   //
   // Set up the wrfdata object
   //
   wd.set_valid_time(valid_ut);
   wd.set_lead_time( (int) (valid_ut - init_ut));
   wd.set_accum_time(accum);
   wd.set_b(v_min);
   wd.set_m( (double) (v_max-v_min)/wrfdata_int_data_max);

   //
   // Parse the precip data into the wrfdata object
   //
   for(x=0; x<gr.nx(); x++) {
      for(y=0; y<gr.ny(); y++) {
         v = (double) pcp_data[wd.two_to_one(x, y)];
         wd.put_xy_int(wd.double_to_int(v), x, y);
      }
   }

   if(pcp_data) { delete pcp_data; pcp_data = (float *) 0; }

   return(0);
}

///////////////////////////////////////////////////////////////////////////////

int has_variable(NcFile *f_in, const char *var_name) {
   int n_var, i, found;
   NcVar *nc_var = (NcVar *) 0;

   //
   // Initialize to not found
   //
   found = 0;

   //
   // Retrieve the number of variables
   //
   n_var = f_in->num_vars();

   for(i=0; i<n_var; i++) {

     //
     // Retreive the next variable
     //
     nc_var = f_in->get_var(i);

     //
     // Check if this is the variable name requested
     //
     if(strcmp(nc_var->name(), var_name) == 0) found = 1;
   }

   return(found);
}

///////////////////////////////////////////////////////////////////////////////


void get_att(NcFile * ncfile, NcAtt * & att, const char * name)

{

att = ncfile->get_att(name);

if ( !att )  {

   cerr << "\n\nERROR: get_att() -> \"" << name << "\" attribute not found.\n\n" << flush;

   exit ( 1 );

}

return;

}


///////////////////////////////////////////////////////////////////////////////


bool has_att(NcFile * ncfile, const char * att_name)

{

int i, n;
bool status = false;
NcAtt *att = (NcAtt *) 0;

n = ncfile->num_atts();

for ( i=0; i<n; i++ )  {

   att = ncfile->get_att(i);

   if ( !att )  {

      cerr << "\n\nERROR: has_att() -> can't read attribute number " << i << ".\n\n" << flush;

      exit ( 1 );

   }

   if ( strcmp(att->name(), att_name) == 0 )  {

      status = true;
      break;
   }
}

return(status);

}


///////////////////////////////////////////////////////////////////////////////


void get_latlon_data_v3(NcFile * ncfile, LatLonData & data, int verbosity)

{

NcAtt * att = (NcAtt *) 0;

if ( verbosity > 3 )  {

   cout << "It's a Lat/Lon (PlateCarree or Equidistant Cylindrical) grid...\n" << flush;

}

   // Store the grid name
data.name = latlon_proj_type;

   // Latitude of the bottom left corner
get_att(ncfile, att, "lat_ll");
data.lat_ll = atof(att->as_string(0));

   // Longitude of the bottom left corner
get_att(ncfile, att, "lon_ll");
data.lon_ll = atof(att->as_string(0));
data.lon_ll *= -1.0;

   // Latitude increment
get_att(ncfile, att, "delta_lat");
data.delta_lat = atof(att->as_string(0));

   // Longitude increment
get_att(ncfile, att, "delta_lon");
data.delta_lon = atof(att->as_string(0));

   // Number of points in the Latitude (y) direction
get_att(ncfile, att, "Nlat");
data.Nlat = atoi(att->as_string(0));

   // Number of points in the Longitudinal (x) direction
get_att(ncfile, att, "Nlon");
data.Nlon = atoi(att->as_string(0));



if ( verbosity > 3 )  {

   cout << "Latitude/Longitude Grid Data:\n"
        << "lat_ll = " << data.lat_ll << "\n"
        << "lon_ll = " << data.lon_ll << "\n"
        << "delta_lat = " << data.delta_lat << "\n"
        << "delta_lon = " << data.delta_lon << "\n"
        << "Nlat = " << data.Nlat << "\n"
        << "Nlon = " << data.Nlon << "\n" << flush;

}

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_lambert_data_v3(NcFile * ncfile, LambertData & data, int verbosity)

{

NcAtt * att = (NcAtt *) 0;

if ( verbosity > 3 )  {

   cout << "It's a Lambert Conformal grid...\n" << flush;

}

   // Store the grid name
data.name = lambert_proj_type;

   // First scale latitude
get_att(ncfile, att, "scale_lat_1");
data.scale_lat_1 = atof(att->as_string(0));

   // Second scale latitude
get_att(ncfile, att, "scale_lat_2");
data.scale_lat_2 = atof(att->as_string(0));

   // Latitude pin
get_att(ncfile, att, "lat_pin");
data.lat_pin = atof(att->as_string(0));

   // Longitude pin
get_att(ncfile, att, "lon_pin");
data.lon_pin = atof(att->as_string(0));
data.lon_pin *= -1.0;

   // X pin
get_att(ncfile, att, "x_pin");
data.x_pin = atof(att->as_string(0));

   // Y pin
get_att(ncfile, att, "y_pin");
data.y_pin = atof(att->as_string(0));

   // Orientation longitude
get_att(ncfile, att, "lon_orient");
data.lon_orient = atof(att->as_string(0));
data.lon_orient *= -1.0;

   // Grid spacing in km
get_att(ncfile, att, "d_km");
data.d_km = atof(att->as_string(0));

   // Radius of the earth
get_att(ncfile, att, "r_km");
data.r_km = atof(att->as_string(0));

   // Number of points in the x-direction
get_att(ncfile, att, "nx");
data.nx = atoi(att->as_string(0));

   // Number of points in the y-direction
get_att(ncfile, att, "ny");
data.ny = atoi(att->as_string(0));



if(verbosity > 3) {

   cout << "Lambert Conformal Grid Data:\n"
        << "scale_lat_1 = " << data.scale_lat_1 << "\n"
        << "scale_lat_2 = " << data.scale_lat_2 << "\n"
        << "lat_pin = " << data.lat_pin << "\n"
        << "lon_pin = " << data.lon_pin << "\n"
        << "x_pin = " << data.x_pin << "\n"
        << "y_pin = " << data.y_pin << "\n"
        << "lon_orient = " << data.lon_orient << "\n"
        << "d_km = " << data.d_km << "\n"
        << "r_km = " << data.r_km << "\n"
        << "nx = " << data.nx << "\n"
        << "ny = " << data.ny << "\n" << flush;

}

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_stereographic_data_v3(NcFile * ncfile, StereographicData & data, int verbosity)

{

NcAtt * att = (NcAtt *) 0;
const char * c = (const char *) 0;


if ( verbosity > 3 )  {

   cout << "It's a Polar Stereographic grid...\n" << flush;

}

   // Store the grid name
data.name = stereographic_proj_type;


   // Hemisphere
get_att(ncfile, att, "hemisphere");
c = att->as_string(0);
data.hemisphere = *c;

   // Scale latitude
get_att(ncfile, att, "scale_lat");
data.scale_lat = atof(att->as_string(0));

   // Latitude pin
get_att(ncfile, att, "lat_pin");
data.lat_pin = atof(att->as_string(0));

   // Longitude pin
get_att(ncfile, att, "lon_pin");
data.lon_pin = atof(att->as_string(0));
data.lon_pin *= -1.0;

   // X pin
get_att(ncfile, att, "x_pin");
data.x_pin = atof(att->as_string(0));

   // Y pin
get_att(ncfile, att, "y_pin");
data.y_pin = atof(att->as_string(0));

   // Orientation longitude
get_att(ncfile, att, "lon_orient");
data.lon_orient = atof(att->as_string(0));
data.lon_orient *= -1.0;

   // Grid spacing in km
get_att(ncfile, att, "d_km");
data.d_km = atof(att->as_string(0));

   // Radius of the earth
get_att(ncfile, att, "r_km");
data.r_km = atof(att->as_string(0));

   // Number of points in the x-direction
get_att(ncfile, att, "nx");
data.nx = atoi(att->as_string(0));

   // Number of points in the y-direction
get_att(ncfile, att, "ny");
data.ny = atoi(att->as_string(0));



if ( verbosity > 3 )  {

   cout << "Stereographic Grid Data:\n"
        << "hemisphere = " << data.hemisphere << "\n"
        << "scale_lat = " << data.scale_lat << "\n"
        << "lat_pin = " << data.lat_pin << "\n"
        << "lon_pin = " << data.lon_pin << "\n"
        << "x_pin = " << data.x_pin << "\n"
        << "y_pin = " << data.y_pin << "\n"
        << "lon_orient = " << data.lon_orient << "\n"
        << "d_km = " << data.d_km << "\n"
        << "r_km = " << data.r_km << "\n"
        << "nx = " << data.nx << "\n"
        << "ny = " << data.ny << "\n" << flush;

}

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_mercator_data_v3(NcFile * ncfile, MercatorData & data, int verbosity)

{

NcAtt * att = (NcAtt *) 0;

if ( verbosity > 3 )  {

   cout << "It's a Mercator grid...\n" << flush;

}

   // Store the grid name
data.name = mercator_proj_type;

   // Latitude of the bottom left corner
get_att(ncfile, att, "lat_ll");
data.lat_ll = atof(att->as_string(0));

   // Longitude of the bottom left corner
get_att(ncfile, att, "lon_ll");
data.lon_ll = atof(att->as_string(0));
data.lon_ll *= -1.0;

   // Latitude of the bottom left corner
get_att(ncfile, att, "lat_ur");
data.lat_ur = atof(att->as_string(0));

   // Longitude of the bottom left corner
get_att(ncfile, att, "lon_ur");
data.lon_ur = atof(att->as_string(0));
data.lon_ur *= -1.0;

   // Number of points in the Latitudinal (y) direction
get_att(ncfile, att, "ny");
data.ny = atoi(att->as_string(0));

   // Number of points in the Longitudinal (x) direction
get_att(ncfile, att, "nx");
data.nx = atoi(att->as_string(0));


if ( verbosity > 3 )  {

   cout << "Mercator Data:\n"
        << "lat_ll = " << data.lat_ll << "\n"
        << "lon_ll = " << data.lon_ll << "\n"
        << "lat_ur = " << data.lat_ur << "\n"
        << "lon_ur = " << data.lon_ur << "\n"
        << "ny = " << data.ny << "\n"
        << "nx = " << data.nx << "\n" << flush;

}

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_latlon_data_v2(NcFile * ncfile, LatLonData & data, int verbosity)

{

NcAtt * att = (NcAtt *) 0;

if ( verbosity > 3 )  {

   cout << "It's a Lat/Lon (PlateCarree or Equidistant Cylindrical) grid...\n" << flush;

}

   // Store the grid name
data.name = latlon_proj_type;

   // Latitude of the bottom left corner
get_att(ncfile, att, "lat_ll_deg");
data.lat_ll = atof(att->as_string(0));

   // Longitude of the bottom left corner
get_att(ncfile, att, "lon_ll_deg");
data.lon_ll = atof(att->as_string(0));
data.lon_ll *= -1.0;

   // Latitude increment
get_att(ncfile, att, "delta_lat_deg");
data.delta_lat = atof(att->as_string(0));

   // Longitude increment
get_att(ncfile, att, "delta_lon_deg");
data.delta_lon = atof(att->as_string(0));

   // Number of points in the Latitude (y) direction
get_att(ncfile, att, "Nlat");
data.Nlat = atoi(att->as_string(0));

   // Number of points in the Longitudinal (x) direction
get_att(ncfile, att, "Nlon");
data.Nlon = atoi(att->as_string(0));



if ( verbosity > 3 )  {

   cout << "Latitude/Longitude Grid Data:\n"
        << "lat_ll = " << data.lat_ll << "\n"
        << "lon_ll = " << data.lon_ll << "\n"
        << "delta_lat = " << data.delta_lat << "\n"
        << "delta_lon = " << data.delta_lon << "\n"
        << "Nlat = " << data.Nlat << "\n"
        << "Nlon = " << data.Nlon << "\n" << flush;

}

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_lambert_data_v2(NcFile * ncfile, LambertData & data, int verbosity)

{

NcAtt * att = (NcAtt *) 0;

if ( verbosity > 3 )  {

   cout << "It's a Lambert Conformal grid...\n" << flush;

}

   // Store the grid name
data.name = lambert_proj_type;

   // First scale latitude
get_att(ncfile, att, "p1_deg");
data.scale_lat_1 = atof(att->as_string(0));

   // Second scale latitude
get_att(ncfile, att, "p2_deg");
data.scale_lat_2 = atof(att->as_string(0));

   // Latitude pin
get_att(ncfile, att, "p0_deg");
data.lat_pin = atof(att->as_string(0));

   // Longitude pin
get_att(ncfile, att, "l0_deg");
data.lon_pin = atof(att->as_string(0));
data.lon_pin *= -1.0;

   // X pin
data.x_pin = 0.0;

   // Y pin
data.y_pin = 0.0;

   // Orientation longitude
get_att(ncfile, att, "lcen_deg");
data.lon_orient = atof(att->as_string(0));
data.lon_orient *= -1.0;

   // Grid spacing in km
get_att(ncfile, att, "d_km");
data.d_km = atof(att->as_string(0));

   // Radius of the earth
get_att(ncfile, att, "r_km");
data.r_km = atof(att->as_string(0));

   // Number of points in the x-direction
get_att(ncfile, att, "nx");
data.nx = atoi(att->as_string(0));

   // Number of points in the y-direction
get_att(ncfile, att, "ny");
data.ny = atoi(att->as_string(0));



if(verbosity > 3) {

   cout << "Lambert Conformal Grid Data:\n"
        << "scale_lat_1 = " << data.scale_lat_1 << "\n"
        << "scale_lat_2 = " << data.scale_lat_2 << "\n"
        << "lat_pin = " << data.lat_pin << "\n"
        << "lon_pin = " << data.lon_pin << "\n"
        << "x_pin = " << data.x_pin << "\n"
        << "y_pin = " << data.y_pin << "\n"
        << "lon_orient = " << data.lon_orient << "\n"
        << "d_km = " << data.d_km << "\n"
        << "r_km = " << data.r_km << "\n"
        << "nx = " << data.nx << "\n"
        << "ny = " << data.ny << "\n" << flush;

}

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_stereographic_data_v2(NcFile * ncfile, StereographicData & data, int verbosity)

{

NcAtt * att = (NcAtt *) 0;


if ( verbosity > 3 )  {

   cout << "It's a Polar Stereographic grid...\n" << flush;

}

   // Store the grid name
data.name = stereographic_proj_type;

   // Hemisphere, assume northern
data.hemisphere = 'N';

   // Scale latitude
get_att(ncfile, att, "p1_deg");
data.scale_lat = atof(att->as_string(0));

   // Latitude pin
get_att(ncfile, att, "p0_deg");
data.lat_pin = atof(att->as_string(0));

   // Longitude pin
get_att(ncfile, att, "l0_deg");
data.lon_pin = atof(att->as_string(0));
data.lon_pin *= -1.0;

   // X pin
data.x_pin = 0.0;

   // Y pin
data.y_pin = 0.0;

   // Orientation longitude
get_att(ncfile, att, "lcen_deg");
data.lon_orient = atof(att->as_string(0));
data.lon_orient *= -1.0;

   // Grid spacing in km
get_att(ncfile, att, "d_km");
data.d_km = atof(att->as_string(0));

   // Radius of the earth
get_att(ncfile, att, "r_km");
data.r_km = atof(att->as_string(0));

   // Number of points in the x-direction
get_att(ncfile, att, "nx");
data.nx = atoi(att->as_string(0));

   // Number of points in the y-direction
get_att(ncfile, att, "ny");
data.ny = atoi(att->as_string(0));



if ( verbosity > 3 )  {

   cout << "Stereographic Grid Data:\n"
        << "hemisphere = " << data.hemisphere << "\n"
        << "scale_lat = " << data.scale_lat << "\n"
        << "lat_pin = " << data.lat_pin << "\n"
        << "lon_pin = " << data.lon_pin << "\n"
        << "x_pin = " << data.x_pin << "\n"
        << "y_pin = " << data.y_pin << "\n"
        << "lon_orient = " << data.lon_orient << "\n"
        << "d_km = " << data.d_km << "\n"
        << "r_km = " << data.r_km << "\n"
        << "nx = " << data.nx << "\n"
        << "ny = " << data.ny << "\n" << flush;

}

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_mercator_data_v2(NcFile * ncfile, MercatorData & data, int verbosity)

{

NcAtt * att = (NcAtt *) 0;

if ( verbosity > 3 )  {

   cout << "It's a Mercator grid...\n" << flush;

}

   // Store the grid name
data.name = mercator_proj_type;

   // Latitude of the bottom left corner
get_att(ncfile, att, "lat_ll_deg");
data.lat_ll = atof(att->as_string(0));

   // Longitude of the bottom left corner
get_att(ncfile, att, "lon_ll_deg");
data.lon_ll = atof(att->as_string(0));
data.lon_ll *= -1.0;

   // Latitude of the bottom left corner
get_att(ncfile, att, "lat_ur_deg");
data.lat_ur = atof(att->as_string(0));

   // Longitude of the bottom left corner
get_att(ncfile, att, "lon_ur_deg");
data.lon_ur = atof(att->as_string(0));
data.lon_ur *= -1.0;

   // Number of points in the Latitudinal (y) direction
get_att(ncfile, att, "Nlat");
data.ny = atoi(att->as_string(0));

   // Number of points in the Longitudinal (x) direction
get_att(ncfile, att, "Nlon");
data.nx = atoi(att->as_string(0));


if ( verbosity > 3 )  {

   cout << "Mercator Data:\n"
        << "lat_ll = " << data.lat_ll << "\n"
        << "lon_ll = " << data.lon_ll << "\n"
        << "lat_ur = " << data.lat_ur << "\n"
        << "lon_ur = " << data.lon_ur << "\n"
        << "ny = " << data.ny << "\n"
        << "nx = " << data.nx << "\n" << flush;

}

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////
