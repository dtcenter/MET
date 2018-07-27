// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   summary_util.cc
//
//   Description:
//      Common routines for time summary (into NetCDF).
//

using namespace std;

//#include <algorithm>
#include <iostream>

//#include "vx_math.h"
#include "vx_nc_util.h"

#include "nc_tools.h"


////////////////////////////////////////////////////////////////////////

void write_obs_var_names(NetcdfObsVars &obs_vars, StringArray &obs_names) {
   write_nc_string_array (&obs_vars.obs_var,  obs_names, HEADER_STR_LEN2);
}

////////////////////////////////////////////////////////////////////////

void write_obs_var_units(NetcdfObsVars &obs_vars, StringArray &units) {
   write_nc_string_array (&obs_vars.unit_var,  units, HEADER_STR_LEN2);
}

////////////////////////////////////////////////////////////////////////

void write_obs_var_descriptions(NetcdfObsVars &obs_vars, StringArray &descriptions) {
   write_nc_string_array (&obs_vars.desc_var,  descriptions, HEADER_STR_LEN3);
}
