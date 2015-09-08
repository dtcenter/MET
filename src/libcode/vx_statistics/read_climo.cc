// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "read_climo.h"

////////////////////////////////////////////////////////////////////////

void read_climo_data_plane(Dictionary *d, int i_vx,
                           unixtime vld_ut, const Grid &vx_grid,
                           DataPlane &cmn_dp, DataPlane &csd_dp) {
   DataPlaneArray cmn_dpa, csd_dpa;

   // Check for bad pointer
   if(!d) return;
   
   // Read array of climatology fields
   read_climo_data_plane_array(d, i_vx, vld_ut, vx_grid,
                               cmn_dpa, csd_dpa); 

   // Check for multiple matches
   if(cmn_dpa.n_planes() > 1) {
      mlog << Warning << "\nread_climo_data_plane() -> "
           << "Found " << cmn_dpa.n_planes() << " matching climatology "
           << "mean fields.  Using the first match.\n\n";
   }

   // Store the climatology mean
   if(cmn_dpa.n_planes() > 0) cmn_dp = cmn_dpa[0];
   else                       cmn_dp.clear();

   // Check for multiple matches
   if(csd_dpa.n_planes() > 1) {
      mlog << Warning << "\nread_climo_data_plane() -> "
           << "Found " << csd_dpa.n_planes() << " matching climatology "
           << "standard deviation fields.  Using the first match.\n\n";
   }

   // Store the climatology standard deviation
   if(csd_dpa.n_planes() > 0) csd_dp = csd_dpa[0];
   else                       csd_dp.clear();
   
   return;
}

////////////////////////////////////////////////////////////////////////

void read_climo_data_plane_array(Dictionary *d, int i_vx,
                                 unixtime vld_ut, const Grid &vx_grid,
                                 DataPlaneArray &cmn_dpa,
                                 DataPlaneArray &csd_dpa) {
   StringArray mn_files, sd_files;
   Dictionary i_dict;

   // Check for bad pointer
   if(!d) return;

   // Get the specified field entry
   i_dict = parse_conf_i_vx_dict(d, i_vx);

   // JHG need to work here to actually read the climo data.
   // For now, just return a field of bad data
   DataPlane dp;
   dp.set_size(vx_grid.nx(), vx_grid.ny());
   cmn_dpa.add(dp, bad_data_double, bad_data_double);
   csd_dpa.add(dp, bad_data_double, bad_data_double);
   
   return;
}

////////////////////////////////////////////////////////////////////////
