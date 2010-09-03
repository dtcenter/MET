// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_READ_NETCDF_H__
#define  __VX_READ_NETCDF_H__

////////////////////////////////////////////////////////////////////////

#include "netcdf.hh"

#include "vx_met_util/constants.h"
#include "vx_data_grids/grid.h"
#include "vx_wrfdata/vx_wrfdata.h"
#include "vx_math/vx_math.h"

////////////////////////////////////////////////////////////////////////

extern void read_netcdf_grid(NcFile *, Grid &, int);

extern int  has_variable(NcFile *, const char *);

////////////////////////////////////////////////////////////////////////

#endif   //  __VX_READ_NETCDF_H__

////////////////////////////////////////////////////////////////////////
