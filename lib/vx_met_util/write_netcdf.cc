// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_met_util/write_netcdf.h"

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_proj(NcFile *f_out, const Grid &grid) {
   char attribute_str[max_str_len];
   GridData gdata;

   //
   // Retrieve the data structure used for this grid
   //
   grid.grid_data(gdata);

   //
   // Write the name of projection
   //
   f_out->add_att("Projection", grid.name());

   //
   // Add Lat/Lon projection information
   //
   if(grid.proj_type() == PlateCarreeProj) {

      sprintf(attribute_str, "%f degrees_north", gdata.pc_data.lat_ll_deg);
      f_out->add_att("lat_ll_deg", attribute_str);

      sprintf(attribute_str, "%f degrees_east", -1.0*gdata.pc_data.lon_ll_deg);
      f_out->add_att("lon_ll_deg", attribute_str);

      sprintf(attribute_str, "%f degrees", gdata.pc_data.delta_lat_deg);
      f_out->add_att("delta_lat_deg", attribute_str);

      sprintf(attribute_str, "%f degrees", gdata.pc_data.delta_lon_deg);
      f_out->add_att("delta_lon_deg", attribute_str);

      sprintf(attribute_str, "%i grid_points", gdata.pc_data.Nlat);
      f_out->add_att("Nlat", attribute_str);

      sprintf(attribute_str, "%i grid_points", gdata.pc_data.Nlon);
      f_out->add_att("Nlon", attribute_str);

   }

   //
   // Add Mercator projection information
   //
   else if(grid.proj_type() == MercatorProj) {

      sprintf(attribute_str, "%f degrees_north", gdata.mc_data.lat_ll_deg);
      f_out->add_att("lat_ll_deg", attribute_str);
      sprintf(attribute_str, "%f degrees_east", -1.0*gdata.mc_data.lon_ll_deg);
      f_out->add_att("lon_ll_deg", attribute_str);
      sprintf(attribute_str, "%f degrees_north", gdata.mc_data.lat_ur_deg);
      f_out->add_att("lat_ur_deg", attribute_str);
      sprintf(attribute_str, "%f degrees_east", -1.0*gdata.mc_data.lon_ur_deg);
      f_out->add_att("lon_ur_deg", attribute_str);
      sprintf(attribute_str, "%i grid_points", gdata.mc_data.ny);
      f_out->add_att("Nlat", attribute_str);
      sprintf(attribute_str, "%i grid_points", gdata.mc_data.nx);
      f_out->add_att("Nlon", attribute_str);
   }
   //
   // Add Lambert Conformal projection information
   //
   else if(grid.proj_type() == LambertProj) {

      sprintf(attribute_str, "%f degrees_north", gdata.lc_data.p1_deg);
      f_out->add_att("p1_deg", attribute_str);
      sprintf(attribute_str, "%f degrees_north", gdata.lc_data.p2_deg);
      f_out->add_att("p2_deg", attribute_str);
      sprintf(attribute_str, "%f degrees_north", gdata.lc_data.p0_deg);
      f_out->add_att("p0_deg", attribute_str);
      sprintf(attribute_str, "%f degrees_east", -1.0*gdata.lc_data.l0_deg);
      f_out->add_att("l0_deg", attribute_str);
      sprintf(attribute_str, "%f degrees_east", -1.0*gdata.lc_data.lcen_deg);
      f_out->add_att("lcen_deg", attribute_str);
      sprintf(attribute_str, "%f km", gdata.lc_data.d_km);
      f_out->add_att("d_km", attribute_str);
      sprintf(attribute_str, "%f km", gdata.lc_data.r_km);
      f_out->add_att("r_km", attribute_str);
      sprintf(attribute_str, "%i grid_points", gdata.lc_data.nx);
      f_out->add_att("nx", attribute_str);
      sprintf(attribute_str, "%i grid_points", gdata.lc_data.ny);
      f_out->add_att("ny", attribute_str);
   }
   //
   // Add Polar Stereographic projection information
   //
   else if(grid.proj_type() == StereographicProj) {

      sprintf(attribute_str, "%f degrees_north", gdata.st_data.p1_deg);
      f_out->add_att("p1_deg", attribute_str);
      sprintf(attribute_str, "%f degrees_north", gdata.st_data.p0_deg);
      f_out->add_att("p0_deg", attribute_str);
      sprintf(attribute_str, "%f degrees_east", -1.0*gdata.st_data.l0_deg);
      f_out->add_att("l0_deg", attribute_str);
      sprintf(attribute_str, "%f degrees_east", -1.0*gdata.st_data.lcen_deg);
      f_out->add_att("lcen_deg", attribute_str);
      sprintf(attribute_str, "%f km", gdata.st_data.d_km);
      f_out->add_att("d_km", attribute_str);
      sprintf(attribute_str, "%f km", gdata.st_data.r_km);
      f_out->add_att("r_km", attribute_str);
      sprintf(attribute_str, "%i grid_points", gdata.st_data.nx);
      f_out->add_att("nx", attribute_str);
      sprintf(attribute_str, "%i grid_points", gdata.st_data.ny);
      f_out->add_att("ny", attribute_str);
   }
   //
   // Unsupported projection type
   //
   else {
      cerr << "\n\nERROR: read_netcdf_grid() -> "
           << "Projection type " << grid.name()
           << " not currently supported.\n\n"
           << flush;
      exit(1);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
