// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __GET_MET_GRID_H__
#define  __GET_MET_GRID_H__

////////////////////////////////////////////////////////////////////////

#include <netcdf>

#include "vx_grid.h"

////////////////////////////////////////////////////////////////////////

extern void read_netcdf_grid(netCDF::NcFile *, Grid &);

extern int  has_variable(netCDF::NcFile *, const char *);

////////////////////////////////////////////////////////////////////////

#endif   //  __GET_MET_GRID_H__

////////////////////////////////////////////////////////////////////////
