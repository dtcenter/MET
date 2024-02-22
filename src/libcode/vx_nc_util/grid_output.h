// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __NETCDF_GRID_OUTPUT_H__
#define  __NETCDF_GRID_OUTPUT_H__


////////////////////////////////////////////////////////////////////////


#include <netcdf>

#include "vx_grid.h"


////////////////////////////////////////////////////////////////////////


extern void grid_output(const GridInfo &, netCDF::NcFile *, netCDF::NcDim &, netCDF::NcDim &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __NETCDF_GRID_OUTPUT_H__  */


////////////////////////////////////////////////////////////////////////

