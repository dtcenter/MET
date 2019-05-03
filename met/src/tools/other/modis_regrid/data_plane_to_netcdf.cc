// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;

#include <cstdio>
#include <ctime>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "data_plane_to_netcdf.h"
#include "vx_log.h"             // mlog
#include "vx_cal.h"             // bad_data_float
#include "concat_string.h"      // max_str_len
#include "nc_utils.h"
#include "write_netcdf.h"       // write_netcdf functions
#include "var_info_factory.h"   // VarInfoFactory


extern int get_compress();

////////////////////////////////////////////////////////////////////////


void write_grid_to_netcdf(const DataPlane & plane, const Grid & grid,
      const char * out_filename, const char * var_name,
      const char * var_long_name, const char * var_units)
{

  // Initialization
  VarInfoFactory var_fac;
  VarInfo* var = (VarInfo *) 0; // allocated, need to delete
  
  //  create a VarInfo object
  var = var_fac.new_var_info((string)"FileType_NcMet");
  if( !var )
  {
    mlog << Debug(4) << "write_grid_to_netcdf() - can't switch on file type \"FileType_NcMet\" and instantiate the appropriate class\n";
    exit(1);
  }
  
  // Set up a NetCDF object
  var->set_name(var_name);
  var->set_long_name(var_long_name);
  var->set_units(var_units);

  write_grid_to_netcdf(plane, grid, out_filename, *var);

  if(var) { delete var; var   = (VarInfo *) 0; }

}


////////////////////////////////////////////////////////////////////////


void write_grid_to_netcdf(const DataPlane & plane, const Grid & grid, const char * out_filename, const VarInfo & var_info) 
{
  // Initialization
  NcFile *f_out   = (NcFile *) 0;
  NcDim  lat_dim  ;
  NcDim  lon_dim  ;
  NcVar  f_var    ;
  
  
  // Create a new NetCDF file and open it
  f_out = open_ncfile(out_filename, true);
  
  if(IS_INVALID_NC_P(f_out)) 
  {
    mlog << Error << "\nwrite_netcdf() -> "
	 << "trouble opening output file " << out_filename
	 << "\n\n";
    delete f_out;  f_out = (NcFile *) 0;
    
    exit(1);
  }
  
  // Add global attributes
  const char * program_name = "data_plane_to_netcdf";
  write_netcdf_global(f_out, out_filename, program_name);
  
  // Add the projection information
  write_netcdf_proj(f_out, grid);
  
  // Define Dimensions
  lat_dim = add_dim(f_out, "lat", (long) grid.ny());
  lon_dim = add_dim(f_out, "lon", (long) grid.nx());
  
  // Add the lat/lon variables
  write_netcdf_latlon(f_out, &lat_dim, &lon_dim, grid);
  
  int deflate_level = get_compress();
  //if (deflate_level < 0) deflate_level = 0;
  
  // Define variable
  f_var = add_var(f_out, (string)var_info.name(), ncFloat, lat_dim, lon_dim, deflate_level);
  
  // Add variable attributes
  add_att(&f_var, "name", (string)var_info.name());
  add_att(&f_var, "units", (string)var_info.units());
  add_att(&f_var, "long_name", (string)var_info.long_name());
  add_att(&f_var, "_FillValue", bad_data_float);
  
  // Write out the times
  write_netcdf_var_times(&f_var, plane);
  
  // Write the data
  if (!put_nc_data_with_dims(&f_var, plane.data(), plane.ny(), plane.nx())) 
  {
    mlog << Error << "\nwrite_netcdf() -> "
	 << "error with f_var->put()\n\n";
    exit(1);
  }
  
  // Close and clean up
  delete f_out;
  f_out = (NcFile *) 0;
  
  return;
}
