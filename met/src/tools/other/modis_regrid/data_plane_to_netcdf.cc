// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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
#include "write_netcdf.h"       // write_netcdf functions
#include "var_info_factory.h"   // VarInfoFactory
#include "netcdf.hh"


////////////////////////////////////////////////////////////////////////


void write_grid_to_netcdf(const DataPlane & plane, const Grid & grid, const char * out_filename, const char * var_name, const char * var_long_name, const char * var_units)
{

  // Initialization
  VarInfoFactory var_fac;
  VarInfo* var = (VarInfo *) 0; // allocated, need to delete
  
  //  create a VarInfo object
  var = var_fac.new_var_info("FileType_NcMet");
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

  if(var)   
  { delete var; var   = (VarInfo *) 0; }

}


////////////////////////////////////////////////////////////////////////


void write_grid_to_netcdf(const DataPlane & plane, const Grid & grid, const char * out_filename, const VarInfo & var_info) 
{
  // Initialization
  NcFile *f_out   = (NcFile *) 0;
  NcDim  *lat_dim = (NcDim *)  0;
  NcDim  *lon_dim = (NcDim *)  0;
  NcVar  *f_var = (NcVar *)  0;
  
  
  // Create a new NetCDF file and open it
  f_out = new NcFile(out_filename, NcFile::Replace);
  
  if(!f_out->is_valid()) 
  {
    mlog << Error << "\nwrite_netcdf() -> "
	 << "trouble opening output file " << out_filename
	 << "\n\n";
    f_out->close();
    delete f_out;  f_out = (NcFile *) 0;
    
    exit(1);
  }
  
  // Add global attributes
  const char * program_name = "data_plane_to_netcdf";
  write_netcdf_global(f_out, out_filename, program_name);
  
  // Add the projection information
  write_netcdf_proj(f_out, grid);
  
  // Define Dimensions
  lat_dim = f_out->add_dim("lat", (long) grid.ny());
  lon_dim = f_out->add_dim("lon", (long) grid.nx());
  
  // Add the lat/lon variables
  write_netcdf_latlon(f_out, lat_dim, lon_dim, grid);
  
  // Define variable
  f_var = f_out->add_var(var_info.name(), ncFloat, lat_dim, lon_dim);
  
  // Add variable attributes
  f_var->add_att("name", var_info.name());
  f_var->add_att("units", var_info.units());
  f_var->add_att("long_name", var_info.long_name());
  f_var->add_att("_FillValue", bad_data_float);
  
  // Write out the times
  write_netcdf_var_times(f_var, plane);
  
  // Write the data
  if (!f_var->put(plane.data(), plane.ny(), plane.nx())) 
  {
    mlog << Error << "\nwrite_netcdf() -> "
	 << "error with f_var->put()\n\n";
    exit(1);
  }
  
  // Close and clean up
  f_out->close();
  delete f_out;
  f_out = (NcFile *) 0;
  
  return;
}
