// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
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

#include "get_nccf_grid.h"

#include "vx_log.h"


///////////////////////////////////////////////////////////////////////////////

static const int grid_debug_level = 4;

///////////////////////////////////////////////////////////////////////////////


//static void get_att(NcFile *, NcAtt * & att, const char * name);
//static bool has_att(NcFile *, const char * name);

static void read_netcdf_grid_v3       (NcFile *, Grid &);
static void read_netcdf_grid_v2       (NcFile *, Grid &);

static void get_latlon_data_v3        (NcFile *, LatLonData &);
static void get_lambert_data_v3       (NcFile *, LambertData &);
static void get_stereographic_data_v3 (NcFile *, StereographicData &);
static void get_mercator_data_v3      (NcFile *, MercatorData &);

static void get_latlon_data_v2        (NcFile *, LatLonData &);
static void get_lambert_data_v2       (NcFile *, LambertData &);
static void get_stereographic_data_v2 (NcFile *, StereographicData &);
static void get_mercator_data_v2      (NcFile *, MercatorData &);


///////////////////////////////////////////////////////////////////////////////


void read_netcdf_grid(NcFile * f_in, Grid & gr)
{
   bool v3_flag = false;

   // Check for the MET version global attribute
cout << " DEBUG_HS get_nccf_grid.cc read_netcdf_grid\n";

   if (has_att(f_in, "MET_version"))
     v3_flag = true;

   // Parse the projection information based on the version

cout << " DEBUG_HS get_nccf_grid.cc read_netcdf_gridv3? " << v3_flag << "\n";
   if (v3_flag)
     read_netcdf_grid_v3(f_in, gr);
   else
     read_netcdf_grid_v2(f_in, gr);

   return;
}


///////////////////////////////////////////////////////////////////////////////


void read_netcdf_grid_v3(NcFile * f_in, Grid & gr)
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

   proj_att = get_nc_att(f_in, "Projection");

      //
      // Parse out the grid specification depending on the projection type
      // The following 4 Projection types are supported:
      //    - Lat/Lon
      //    - Mercator
      //    - Lambert Conformal
      //    - Polar Stereographic
      //

   if ( strcmp(proj_att->as_string(0), latlon_proj_type) == 0 )  {

      get_latlon_data_v3(f_in, ll_data);

      gr.set(ll_data);

   } else if ( strcmp(proj_att->as_string(0), mercator_proj_type) == 0 )  {

      get_mercator_data_v3(f_in, mc_data);

      gr.set(mc_data);

   } else if ( strcmp(proj_att->as_string(0), lambert_proj_type) == 0 )  {

      get_lambert_data_v3(f_in, lc_data);

      gr.set(lc_data);

   } else if ( strcmp(proj_att->as_string(0), stereographic_proj_type) == 0 )  {

      get_stereographic_data_v3(f_in, st_data);

      gr.set(st_data);

   } else {   // Unsupported projection type

      ConcatString junk;
      junk << proj_att->as_string(0);
     
      mlog << Error << "\nread_netcdf_grid_v3() -> "
           << "Projection type " << junk
           << " not currently supported.\n\n";

      exit(1);

   }

   //
   //  done
   //

return;

}

///////////////////////////////////////////////////////////////////////////////


void read_netcdf_grid_v2(NcFile * f_in, Grid & gr)

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

cout << " ---DEBUG_HS get_nccf_grid.cc read_netcdf_grid_v2\n";       
cout << " ---DEBUG_HS get_nccf_grid.cc read_netcdf_grid_v2\n";       
cout << " ---DEBUG_HS get_nccf_grid.cc read_netcdf_grid_v2\n";       
   proj_att = get_nc_att(f_in, "Projection");
cout << " === DEBUG_HS get_nccf_grid.cc read_netcdf_grid_v2\n";       

      //
      // Parse out the grid specification depending on the projection type
      // The following 4 Projection types are supported:
      //    - Lat/Lon
      //    - Mercator
      //    - Lambert Conformal
      //    - Polar Stereographic
      //

   if ( strcmp(proj_att->as_string(0), latlon_proj_type) == 0 )  {

      get_latlon_data_v2(f_in, ll_data);

      gr.set(ll_data);

   } else if ( strcmp(proj_att->as_string(0), mercator_proj_type) == 0 )  {

      get_mercator_data_v2(f_in, mc_data);

      gr.set(mc_data);

   } else if ( strcmp(proj_att->as_string(0), lambert_proj_type) == 0 )  {

      get_lambert_data_v2(f_in, lc_data);

      gr.set(lc_data);

   } else if ( strcmp(proj_att->as_string(0), stereographic_proj_type) == 0 )  {

      get_stereographic_data_v2(f_in, st_data);

      gr.set(st_data);

   } else {   // Unsupported projection type

      ConcatString junk;
      junk << proj_att->as_string(0);
     
      mlog << Error << "\nread_netcdf_grid_v2() -> "
           << "Projection type " << junk
           << " not currently supported.\n\n";

      exit(1);

   }

   //
   //  done
   //

return;

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

//void get_att(NcFile * ncfile, NcAtt * & att, const char * name)
//
//{
//
//att = ncfile->get_att(name);
//
//if ( !att )  {
//
//   mlog << Error << "\nget_att() -> \"" << name << "\" attribute not found.\n\n";
//
//   exit ( 1 );
//
//}
//
//return;
//
//}


///////////////////////////////////////////////////////////////////////////////


//bool has_att(NcFile * ncfile, const char * att_name)
//
//{
//
//int i, n;
//bool status = false;
//NcAtt *att = (NcAtt *) 0;
//
//n = ncfile->num_atts();
//
//for ( i=0; i<n; i++ )  {
//
//   att = ncfile->get_att(i);
//
//   if ( !att )  {
//
//      mlog << Error << "\nhas_att() -> "
//           << "can't read attribute number " << i << ".\n\n";
//
//      exit ( 1 );
//
//   }
//
//   if ( strcmp(att->name(), att_name) == 0 )  {
//
//      status = true;
//      break;
//   }
//}
//
//return(status);
//
//}


///////////////////////////////////////////////////////////////////////////////


void get_latlon_data_v3(NcFile * ncfile, LatLonData & data)

{

NcAtt * att = (NcAtt *) 0;

   // Store the grid name
data.name = latlon_proj_type;

   // Latitude of the bottom left corner
att = get_nc_att(ncfile, , "lat_ll");
data.lat_ll = atof(att->as_string(0));

   // Longitude of the bottom left corner
att = get_nc_att(ncfile, "lon_ll");
data.lon_ll = atof(att->as_string(0));
data.lon_ll *= -1.0;

   // Latitude increment
att = get_nc_att(ncfile, "delta_lat");
data.delta_lat = atof(att->as_string(0));

   // Longitude increment
att = get_nc_att(ncfile, "delta_lon");
data.delta_lon = atof(att->as_string(0));

   // Number of points in the Latitude (y) direction
att = get_nc_att(ncfile, "Nlat");
data.Nlat = atoi(att->as_string(0));

   // Number of points in the Longitudinal (x) direction
att = get_nc_att(ncfile, "Nlon");
data.Nlon = atoi(att->as_string(0));

mlog << Debug(grid_debug_level)
     << "Latitude/Longitude Grid Data:\n"
     << "lat_ll = " << data.lat_ll << "\n"
     << "lon_ll = " << data.lon_ll << "\n"
     << "delta_lat = " << data.delta_lat << "\n"
     << "delta_lon = " << data.delta_lon << "\n"
     << "Nlat = " << data.Nlat << "\n"
     << "Nlon = " << data.Nlon << "\n";

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_lambert_data_v3(NcFile * ncfile, LambertData & data)

{

NcAtt * att = (NcAtt *) 0;

   // Store the grid name
data.name = lambert_proj_type;

   // First scale latitude
att = get_nc_att(ncfile, "scale_lat_1");
data.scale_lat_1 = atof(att->as_string(0));

   // Second scale latitude
att = get_nc_att(ncfile, "scale_lat_2");
data.scale_lat_2 = atof(att->as_string(0));

   // Latitude pin
att = get_nc_att(ncfile, "lat_pin");
data.lat_pin = atof(att->as_string(0));

   // Longitude pin
att = get_nc_att(ncfile, "lon_pin");
data.lon_pin = atof(att->as_string(0));
data.lon_pin *= -1.0;

   // X pin
att = get_nc_att(ncfile, "x_pin");
data.x_pin = atof(att->as_string(0));

   // Y pin
att = get_nc_att(ncfile, "y_pin");
data.y_pin = atof(att->as_string(0));

   // Orientation longitude
att = get_nc_att(ncfile, "lon_orient");
data.lon_orient = atof(att->as_string(0));
data.lon_orient *= -1.0;

   // Grid spacing in km
att = get_nc_att(ncfile, "d_km");
data.d_km = atof(att->as_string(0));

   // Radius of the earth
att = get_nc_att(ncfile, "r_km");
data.r_km = atof(att->as_string(0));

   // Number of points in the x-direction
att = get_nc_att(ncfile, "nx");
data.nx = atoi(att->as_string(0));

   // Number of points in the y-direction
att = get_nc_att(ncfile, "ny");
data.ny = atoi(att->as_string(0));

mlog << Debug(grid_debug_level)
     << "Lambert Conformal Grid Data:\n"
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
     << "ny = " << data.ny << "\n";

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_stereographic_data_v3(NcFile * ncfile, StereographicData & data)

{

NcAtt * att = (NcAtt *) 0;
const char * c = (const char *) 0;

   // Store the grid name
data.name = stereographic_proj_type;

   // Hemisphere
att = get_nc_att(ncfile, "hemisphere");
c = att->as_string(0);
data.hemisphere = *c;

   // Scale latitude
att = get_nc_att(ncfile, "scale_lat");
data.scale_lat = atof(att->as_string(0));

   // Latitude pin
att = get_nc_att(ncfile, "lat_pin");
data.lat_pin = atof(att->as_string(0));

   // Longitude pin
att = get_nc_att(ncfile, "lon_pin");
data.lon_pin = atof(att->as_string(0));
data.lon_pin *= -1.0;

   // X pin
att = get_nc_att(ncfile, "x_pin");
data.x_pin = atof(att->as_string(0));

   // Y pin
att = get_nc_att(ncfile, "y_pin");
data.y_pin = atof(att->as_string(0));

   // Orientation longitude
att = get_nc_att(ncfile, "lon_orient");
data.lon_orient = atof(att->as_string(0));
data.lon_orient *= -1.0;

   // Grid spacing in km
att = get_nc_att(ncfile, "d_km");
data.d_km = atof(att->as_string(0));

   // Radius of the earth
att = get_nc_att(ncfile, "r_km");
data.r_km = atof(att->as_string(0));

   // Number of points in the x-direction
att = get_nc_att(ncfile, "nx");
data.nx = atoi(att->as_string(0));

   // Number of points in the y-direction
att = get_nc_att(ncfile, "ny");
data.ny = atoi(att->as_string(0));

mlog << Debug(grid_debug_level)
     << "Stereographic Grid Data:\n"
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
     << "ny = " << data.ny << "\n";

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_mercator_data_v3(NcFile * ncfile, MercatorData & data)

{

NcAtt * att = (NcAtt *) 0;

   // Store the grid name
data.name = mercator_proj_type;

   // Latitude of the bottom left corner
att = get_nc_att(ncfile, "lat_ll");
data.lat_ll = atof(att->as_string(0));

   // Longitude of the bottom left corner
att = get_nc_att(ncfile, "lon_ll");
data.lon_ll = atof(att->as_string(0));
data.lon_ll *= -1.0;

   // Latitude of the bottom left corner
att = get_nc_att(ncfile, "lat_ur");
data.lat_ur = atof(att->as_string(0));

   // Longitude of the bottom left corner
att = get_nc_att(ncfile, "lon_ur");
data.lon_ur = atof(att->as_string(0));
data.lon_ur *= -1.0;

   // Number of points in the Latitudinal (y) direction
att = get_nc_att(ncfile,, "ny");
data.ny = atoi(att->as_string(0));

   // Number of points in the Longitudinal (x) direction
att = get_nc_att(ncfile,, "nx");
data.nx = atoi(att->as_string(0));

mlog << Debug(grid_debug_level)
     << "Mercator Data:\n"
     << "lat_ll = " << data.lat_ll << "\n"
     << "lon_ll = " << data.lon_ll << "\n"
     << "lat_ur = " << data.lat_ur << "\n"
     << "lon_ur = " << data.lon_ur << "\n"
     << "ny = " << data.ny << "\n"
     << "nx = " << data.nx << "\n";

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_latlon_data_v2(NcFile * ncfile, LatLonData & data)

{

NcAtt * att = (NcAtt *) 0;

   // Store the grid name
data.name = latlon_proj_type;

   // Latitude of the bottom left corner
att = get_nc_att(ncfile,, "lat_ll_deg");
data.lat_ll = atof(att->as_string(0));

   // Longitude of the bottom left corner
att = get_nc_att(ncfile,, "lon_ll_deg");
data.lon_ll = atof(att->as_string(0));
data.lon_ll *= -1.0;

   // Latitude increment
att = get_nc_att(ncfile,, "delta_lat_deg");
data.delta_lat = atof(att->as_string(0));

   // Longitude increment
att = get_nc_att(ncfile,, "delta_lon_deg");
data.delta_lon = atof(att->as_string(0));

   // Number of points in the Latitude (y) direction
att = get_nc_att(ncfile,, "Nlat");
data.Nlat = atoi(att->as_string(0));

   // Number of points in the Longitudinal (x) direction
att = get_nc_att(ncfile,, "Nlon");
data.Nlon = atoi(att->as_string(0));

mlog << Debug(grid_debug_level)
     << "Latitude/Longitude Grid Data:\n"
     << "lat_ll = " << data.lat_ll << "\n"
     << "lon_ll = " << data.lon_ll << "\n"
     << "delta_lat = " << data.delta_lat << "\n"
     << "delta_lon = " << data.delta_lon << "\n"
     << "Nlat = " << data.Nlat << "\n"
     << "Nlon = " << data.Nlon << "\n";

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_lambert_data_v2(NcFile * ncfile, LambertData & data)

{

NcAtt * att = (NcAtt *) 0;

   // Store the grid name
data.name = lambert_proj_type;

   // First scale latitude
att = get_nc_att(ncfile,, "p1_deg");
data.scale_lat_1 = atof(att->as_string(0));

   // Second scale latitude
att = get_nc_att(ncfile,, "p2_deg");
data.scale_lat_2 = atof(att->as_string(0));

   // Latitude pin
att = get_nc_att(ncfile,, "p0_deg");
data.lat_pin = atof(att->as_string(0));

   // Longitude pin
att = get_nc_att(ncfile,, "l0_deg");
data.lon_pin = atof(att->as_string(0));
data.lon_pin *= -1.0;

   // X pin
data.x_pin = 0.0;

   // Y pin
data.y_pin = 0.0;

   // Orientation longitude
att = get_nc_att(ncfile,, "lcen_deg");
data.lon_orient = atof(att->as_string(0));
data.lon_orient *= -1.0;

   // Grid spacing in km
att = get_nc_att(ncfile,, "d_km");
data.d_km = atof(att->as_string(0));

   // Radius of the earth
att = get_nc_att(ncfile,, "r_km");
data.r_km = atof(att->as_string(0));

   // Number of points in the x-direction
att = get_nc_att(ncfile,, "nx");
data.nx = atoi(att->as_string(0));

   // Number of points in the y-direction
att = get_nc_att(ncfile,, "ny");
data.ny = atoi(att->as_string(0));

mlog << Debug(grid_debug_level)
     << "Lambert Conformal Grid Data:\n"
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
     << "ny = " << data.ny << "\n";

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_stereographic_data_v2(NcFile * ncfile, StereographicData & data)

{

NcAtt * att = (NcAtt *) 0;

   // Store the grid name
data.name = stereographic_proj_type;

   // Hemisphere, assume northern
data.hemisphere = 'N';

   // Scale latitude
att = get_nc_att(ncfile,, "p1_deg");
data.scale_lat = atof(att->as_string(0));

   // Latitude pin
att = get_nc_att(ncfile,, "p0_deg");
data.lat_pin = atof(att->as_string(0));

   // Longitude pin
att = get_nc_att(ncfile,, "l0_deg");
data.lon_pin = atof(att->as_string(0));
data.lon_pin *= -1.0;

   // X pin
data.x_pin = 0.0;

   // Y pin
data.y_pin = 0.0;

   // Orientation longitude
att = get_nc_att(ncfile,, "lcen_deg");
data.lon_orient = atof(att->as_string(0));
data.lon_orient *= -1.0;

   // Grid spacing in km
att = get_nc_att(ncfile,, "d_km");
data.d_km = atof(att->as_string(0));

   // Radius of the earth
att = get_nc_att(ncfile,, "r_km");
data.r_km = atof(att->as_string(0));

   // Number of points in the x-direction
att = get_nc_att(ncfile,, "nx");
data.nx = atoi(att->as_string(0));

   // Number of points in the y-direction
att = get_nc_att(ncfile,, "ny");
data.ny = atoi(att->as_string(0));

mlog << Debug(grid_debug_level)
     << "Stereographic Grid Data:\n"
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
     << "ny = " << data.ny << "\n";

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void get_mercator_data_v2(NcFile * ncfile, MercatorData & data)

{

NcAtt * att = (NcAtt *) 0;

   // Store the grid name
data.name = mercator_proj_type;

   // Latitude of the bottom left corner
att = get_nc_att(ncfile,, "lat_ll_deg");
data.lat_ll = atof(att->as_string(0));

   // Longitude of the bottom left corner
att = get_nc_att(ncfile,, "lon_ll_deg");
data.lon_ll = atof(att->as_string(0));
data.lon_ll *= -1.0;

   // Latitude of the bottom left corner
att = get_nc_att(ncfile,, "lat_ur_deg");
data.lat_ur = atof(att->as_string(0));

   // Longitude of the bottom left corner
att = get_nc_att(ncfile,, "lon_ur_deg");
data.lon_ur = atof(att->as_string(0));
data.lon_ur *= -1.0;

   // Number of points in the Latitudinal (y) direction
att = get_nc_att(ncfile,, "Nlat");
data.ny = atoi(att->as_string(0));

   // Number of points in the Longitudinal (x) direction
att = get_nc_att(ncfile,, "Nlon");
data.nx = atoi(att->as_string(0));

mlog << Debug(grid_debug_level)
     << "Mercator Data:\n"
     << "lat_ll = " << data.lat_ll << "\n"
     << "lon_ll = " << data.lon_ll << "\n"
     << "lat_ur = " << data.lat_ur << "\n"
     << "lon_ur = " << data.lon_ur << "\n"
     << "ny = " << data.ny << "\n"
     << "nx = " << data.nx << "\n";

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////
