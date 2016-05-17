// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __WRITE_NETCDF_H__
#define  __WRITE_NETCDF_H__

////////////////////////////////////////////////////////////////////////

#include "netcdf.hh"

#include "vx_grid.h"
// #include "vx_wrfdata.h"

////////////////////////////////////////////////////////////////////////

extern void write_netcdf_global    (NcFile *, const char *, const char *,
                                    const char *model_name = (const char *) 0,
                                    const char *obtype     = (const char *) 0);
extern void write_netcdf_proj      (NcFile *, const Grid &);
extern void write_netcdf_latlon    (NcFile *, NcDim *, NcDim *, const Grid &);
extern void write_netcdf_grid_wgt  (NcFile *, NcDim *, NcDim *, const DataPlane &); 
extern void write_netcdf_var_times (NcVar *, const DataPlane &);
extern void write_netcdf_var_times (NcVar *, const unixtime, const unixtime, const int);

////////////////////////////////////////////////////////////////////////

#endif   //  __WRITE_NETCDF_H__

////////////////////////////////////////////////////////////////////////
