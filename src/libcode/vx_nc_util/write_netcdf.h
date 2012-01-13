// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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

extern void write_netcdf_global    (NcFile *, const char *, const char *);
extern void write_netcdf_proj      (NcFile *, const Grid &);
extern void write_netcdf_latlon    (NcFile *, NcDim *, NcDim *, const Grid &);
extern void write_netcdf_var_times (NcVar *, const DataPlane &);
extern void write_netcdf_var_times (NcVar *, const unixtime, const unixtime, const int);

////////////////////////////////////////////////////////////////////////

#endif   //  __WRITE_NETCDF_H__

////////////////////////////////////////////////////////////////////////
