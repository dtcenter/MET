

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __GET_WRF_GRID_H__
#define  __GET_WRF_GRID_H__


////////////////////////////////////////////////////////////////////////


#include <netcdf>

#include "vx_grid.h"


////////////////////////////////////////////////////////////////////////


extern bool get_wrf_grid(const char * wrf_filename, Grid & grid);

extern bool get_wrf_grid(netCDF::NcFile & nc, Grid & grid);


////////////////////////////////////////////////////////////////////////


#endif   /*  __GET_WRF_GRID_H__  */


////////////////////////////////////////////////////////////////////////


