// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __NETCDF_GRIDS_H__
#define  __NETCDF_GRIDS_H__


////////////////////////////////////////////////////////////////////////


#include "nc_utils.h"
#include "vx_grid.h"


////////////////////////////////////////////////////////////////////////


extern bool  read_nc_grid(NcFile &, Grid &);

extern bool write_nc_grid(NcFile &, const Grid &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __NETCDF_GRIDS_H__  */


////////////////////////////////////////////////////////////////////////


