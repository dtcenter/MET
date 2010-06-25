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

void read_netcdf_grid(NcFile *f_in, Grid &gr, int verbosity) {
   NcAtt *proj_att = (NcAtt *) 0;

   // Structures to store projection info
   LambertData       lc_data;
   StereographicData st_data;
   LatLonData        pc_data;
   MercatorData      mc_data;

   //
   // Parse the grid specification out of the global attributes
   //
   proj_att = f_in->get_att("Projection");
   if(!proj_att) {
      cerr << "\n\nERROR: read_netcdf_grid() -> "
           << "\"Projection\" attribute not found.\n\n"
           << flush;
      exit(1);
   }

   //
   // Parse out the grid specification depending on the projection type
   // The following 4 Projection types are supported:
   //    - Lat/Lon
   //    - Mercator
   //    - Lambert Conformal
   //    - Polar Stereographic
   //

   //
   // Latitude/Longitude Projections Grid
   // Also called Equidistant Cylindrical or Plate Carree Projection Grid
   //
   if(strcmp(proj_att->as_string(0), proj_type[0]) == 0) {

      if(verbosity > 3) {
         cout << "It's a Lat/Lon (PlateCarree or Equidistant Cylindrical) grid...\n"
              << flush;
      }

      // Store the grid name
      pc_data.name = proj_type[0];

      // Latitude of the bottom left corner
      proj_att = f_in->get_att("lat_ll_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"lat_ll_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      pc_data.lat_ll_deg = atof(proj_att->as_string(0));

      // Longitude of the bottom left corner
      proj_att = f_in->get_att("lon_ll_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"lon_ll_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      pc_data.lon_ll_deg = -1.0*atof(proj_att->as_string(0));

      // Latitudinal increment
      proj_att = f_in->get_att("delta_lat_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"delta_lat_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      pc_data.delta_lat_deg = atof(proj_att->as_string(0));

      // Longitudinal increment
      proj_att = f_in->get_att("delta_lon_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"delta_lon_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      pc_data.delta_lon_deg = atof(proj_att->as_string(0));

      // Number of points in the Latitudinal (y) direction
      proj_att = f_in->get_att("Nlat");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"Nlat\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      pc_data.Nlat = atoi(proj_att->as_string(0));

      // Number of points in the Longitudinal (x) direction
      proj_att = f_in->get_att("Nlon");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"Nlon\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      pc_data.Nlon = atoi(proj_att->as_string(0));

      Grid pcp_grid(pc_data);
      gr = pcp_grid;

      if(verbosity > 3) {
         cout << "Latitude/Longitude Data:\n"
              << "lat_ll_deg = " << pc_data.lat_ll_deg << "\n"
              << "lon_ll_deg = " << pc_data.lon_ll_deg << "\n"
              << "delta_lat_deg = " << pc_data.delta_lat_deg << "\n"
              << "delta_lon_deg = " << pc_data.delta_lon_deg << "\n"
              << "Nlat = " << pc_data.Nlat << "\n"
              << "Nlon = " << pc_data.Nlon << "\n" << flush;
      }
   }
   //
   // Mercator Projection
   //
   else if(strcmp(proj_att->as_string(0), proj_type[1]) == 0) {

      if(verbosity > 3) {
         cout << "It's a Mercator grid...\n" << flush;
      }

      // Store the grid name
      mc_data.name = proj_type[0];

      // Latitude of the bottom left corner
      proj_att = f_in->get_att("lat_ll_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"lat_ll_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      mc_data.lat_ll_deg = atof(proj_att->as_string(0));

      // Longitude of the bottom left corner
      proj_att = f_in->get_att("lon_ll_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"lon_ll_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      mc_data.lon_ll_deg = -1.0*atof(proj_att->as_string(0));

      // Latitude of the bottom left corner
      proj_att = f_in->get_att("lat_ur_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"lat_ur_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      mc_data.lat_ur_deg = atof(proj_att->as_string(0));

      // Longitude of the bottom left corner
      proj_att = f_in->get_att("lon_ur_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"lon_ur_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      mc_data.lon_ur_deg = -1.0*atof(proj_att->as_string(0));

      // Number of points in the Latitudinal (y) direction
      proj_att = f_in->get_att("Nlat");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"Nlat\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      mc_data.ny = atoi(proj_att->as_string(0));

      // Number of points in the Longitudinal (x) direction
      proj_att = f_in->get_att("Nlon");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"Nlon\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      mc_data.nx = atoi(proj_att->as_string(0));

      Grid pcp_grid(mc_data);
      gr = pcp_grid;

      if(verbosity > 3) {
         cout << "Mercator Data:\n"
              << "lat_ll_deg = " << mc_data.lat_ll_deg << "\n"
              << "lon_ll_deg = " << mc_data.lon_ll_deg << "\n"
              << "lat_ur_deg = " << mc_data.lat_ur_deg << "\n"
              << "lon_ur_deg = " << mc_data.lon_ur_deg << "\n"
              << "ny = " << mc_data.ny << "\n"
              << "nx = " << mc_data.nx << "\n" << flush;
      }
   }
   //
   // Lambert Conformal Projection
   //
   else if(strcmp(proj_att->as_string(0), proj_type[2]) == 0) {

      if(verbosity > 3) {
         cout << "It's a Lambert Conformal grid...\n" << flush;
      }

      // Store the grid name
      lc_data.name = proj_type[2];

      // First scale latitude
      proj_att = f_in->get_att("p1_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"p1_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      lc_data.scale_lat_1 = atof(proj_att->as_string(0));

      // Second scale latitude
      proj_att = f_in->get_att("p2_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"p2_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      lc_data.scale_lat_2 = atof(proj_att->as_string(0));

      // Latitude of first point
      proj_att = f_in->get_att("p0_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"p0_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      lc_data.lat_pin = atof(proj_att->as_string(0));

      // Longitude of first point
      proj_att = f_in->get_att("l0_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"l0_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      lc_data.lon_pin = -1.0*atof(proj_att->as_string(0));

      //  pin this point to the lower_left corner of the grid
      lc_data.x_pin = 0.0;
      lc_data.y_pin = 0.0;

      // Center longitude
      proj_att = f_in->get_att("lcen_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"lcen_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      lc_data.lcen = -1.0*atof(proj_att->as_string(0));

      // Grid spacing in km
      proj_att = f_in->get_att("d_km");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"d_km\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      lc_data.d_km = atof(proj_att->as_string(0));

      // Radius of the earth
      proj_att = f_in->get_att("r_km");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"r_km\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      lc_data.r_km = atof(proj_att->as_string(0));

      // Number of points in the x-direction
      proj_att = f_in->get_att("nx");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"nx\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      lc_data.nx = atoi(proj_att->as_string(0));

      // Number of points in the y-direction
      proj_att = f_in->get_att("ny");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"ny\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      lc_data.ny = atoi(proj_att->as_string(0));

      Grid pcp_grid(lc_data);
      gr = pcp_grid;

      if(verbosity > 3) {
         cout << "Lambert Conformal Data:\n"
              << "scale_lat_1 = " << lc_data.scale_lat_1 << "\n"
              << "scale_lat_2 = " << lc_data.scale_lat_2 << "\n"
              << "lat_pin = " << lc_data.lat_pin << "\n"
              << "lon_pin = " << lc_data.lon_pin << "\n"
              << "x_pin = " << lc_data.x_pin << "\n"
              << "y_pin = " << lc_data.y_pin << "\n"
              << "lcen = " << lc_data.lcen << "\n"
              << "d_km = " << lc_data.d_km << "\n"
              << "r_km = " << lc_data.r_km << "\n"
              << "nx = " << lc_data.nx << "\n"
              << "ny = " << lc_data.ny << "\n" << flush;
      }
   }
   //
   // Polar Stereographic Projection
   //
   else if(strcmp(proj_att->as_string(0), proj_type[3]) == 0) {
     if(verbosity > 3) {
         cout << "It's a Polar Stereographic grid...\n" << flush;
      }

      // Store the grid name
      st_data.name = proj_type[3];

      // Scale latitude
      proj_att = f_in->get_att("p1_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"p1_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      st_data.scale_lat = atof(proj_att->as_string(0));

      // Latitude of first point
      proj_att = f_in->get_att("p0_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"p0_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      st_data.lat_pin = atof(proj_att->as_string(0));

      // Longitude of first point
      proj_att = f_in->get_att("l0_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"l0_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      st_data.lon_pin = -1.0*atof(proj_att->as_string(0));

      //  pin this point to the lower_left corner of the grid
      st_data.x_pin = 0.0;
      st_data.y_pin = 0.0;

      // Center longitude
      proj_att = f_in->get_att("lcen_deg");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"lcen_deg\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      st_data.lcen = -1.0*atof(proj_att->as_string(0));

      // Grid spacing in km
      proj_att = f_in->get_att("d_km");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"d_km\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      st_data.d_km = atof(proj_att->as_string(0));

      // Radius of the earth
      proj_att = f_in->get_att("r_km");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"r_km\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      st_data.r_km = atof(proj_att->as_string(0));

      // Number of points in the x-direction
      proj_att = f_in->get_att("nx");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"nx\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      st_data.nx = atoi(proj_att->as_string(0));

      // Number of points in the y-direction
      proj_att = f_in->get_att("ny");
      if(!proj_att) {
         cerr << "\n\nERROR: read_netcdf_grid() -> "
              << "\"ny\" attribute not found.\n\n"
              << flush;
         exit(1);
      }
      st_data.ny = atoi(proj_att->as_string(0));

      Grid pcp_grid(st_data);
      gr = pcp_grid;

      if(verbosity > 3) {
         cout << "Stereographic Data:\n"
              << "scale_lat = " << st_data.scale_lat << "\n"
              << "lat_pin = " << st_data.lat_pin << "\n"
              << "lon_pin = " << st_data.lon_pin << "\n"
              << "x_pin = " << st_data.x_pin << "\n"
              << "y_pin = " << st_data.y_pin << "\n"
              << "lcen = " << st_data.lcen << "\n"
              << "d_km = " << st_data.d_km << "\n"
              << "r_km = " << st_data.r_km << "\n"
              << "nx = " << st_data.nx << "\n"
              << "ny = " << st_data.ny << "\n" << flush;
      }
   }
   //
   // Unsupported projection type
   //
   else {
      cerr << "\n\nERROR: read_netcdf_grid() -> "
           << "Projection type " << proj_att->as_string(0)
           << " not currently supported.\n\n"
           << flush;
      exit(1);
   }

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
      accum = sec_per_hour*atoi(time_att->as_string(0));
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
