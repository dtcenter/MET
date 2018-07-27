// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __NC_TOOLS_H__
#define  __NC_TOOLS_H__

#include <netcdf>
using namespace netCDF;
#include "nc_utils.h"


////////////////////////////////////////////////////////////////////////

extern void write_obs_var_names(NetcdfObsVars &, StringArray &);
extern void write_obs_var_units(NetcdfObsVars &, StringArray &);
extern void write_obs_var_descriptions(NetcdfObsVars &, StringArray &);

#endif   /*  __NC_TOOLS_H__  */


////////////////////////////////////////////////////////////////////////
