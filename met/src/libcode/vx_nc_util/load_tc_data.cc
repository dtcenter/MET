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

#include "vx_data2d_nc_met.h"
#include "load_dland.h"

////////////////////////////////////////////////////////////////////////

void load_dland(const ConcatString &dland_file, Grid &grid,
                DataPlane &dp) {
   ConcatString file_name;
   LongArray dim;
   int i;

   // Get the path for the distance to land file
   file_name = replace_path(dland_file);

   mlog << Debug(1)
        << "Distance to land file: " << file_name << "\n";

   // Check for no file provided
   if(file_name.empty()) return;

   // Open the NetCDF output of the tc_dland tool
   MetNcFile MetNc;
   if(!MetNc.open(file_name.c_str())) {
      mlog << Error
           << "\nload_dland() -> "
           << "problem reading file \"" << file_name << "\"\n\n";
      exit(1);
   }

   // Find the first non-lat/lon variable
   for(i=0; i<MetNc.Nvars; i++) {
      if(strcmp(MetNc.Var[i].name.c_str(), nc_met_lat_var_name) != 0 &&
         strcmp(MetNc.Var[i].name.c_str(), nc_met_lon_var_name) != 0)
         break;
   }

   // Check range
   if(i == MetNc.Nvars) {
      mlog << Error
           << "\nload_dland() -> "
           << "can't find non-lat/lon variable in file \""
           << file_name << "\"\n\n";
      exit(1);
   }

   // Store the grid
   grid = MetNc.grid;

   // Set the dimension to (*,*)
   dim.add(vx_data2d_star);
   dim.add(vx_data2d_star);

   // Read the data
   if(!MetNc.data(MetNc.Var[i].var, dim, dp)) {
      mlog << Error
           << "\nload_dland() -> "
           << "can't read data from file \""
           << file_name << "\"\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

