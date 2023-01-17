// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __WRITE_NETCDF_H__
#define  __WRITE_NETCDF_H__

////////////////////////////////////////////////////////////////////////

#include <netcdf>

#include "vx_grid.h"
#include "vx_config.h"
#include "nc_utils.h"
#include "observation.h"

////////////////////////////////////////////////////////////////////////

static const float FILL_VALUE = -9999.f;

////////////////////////////////////////////////////////////////////////

extern void write_netcdf_global     (netCDF::NcFile *, const char *, const char *,
                                     const char *model_name = (const char *) 0,
                                     const char *obtype     = (const char *) 0,
                                     const char *desc       = (const char *) 0);
extern void write_netcdf_proj       (netCDF::NcFile *, const Grid &, netCDF::NcDim &, netCDF::NcDim &);
extern void write_netcdf_latlon     (netCDF::NcFile *, netCDF::NcDim *, netCDF::NcDim *, const Grid &);
extern void write_netcdf_grid_weight(netCDF::NcFile *, netCDF::NcDim *, netCDF::NcDim *, const GridWeightType, const DataPlane &); 
extern void write_netcdf_var_times  (netCDF::NcVar *, const DataPlane &);
extern void write_netcdf_var_times  (netCDF::NcVar *, const unixtime, const unixtime, const int);

      

#endif   //  __WRITE_NETCDF_H__

////////////////////////////////////////////////////////////////////////
