

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __GET_PINTERP_GRID_H__
#define  __GET_PINTERP_GRID_H__


////////////////////////////////////////////////////////////////////////


#include <netcdf>
using namespace netCDF;

#include "vx_grid.h"


////////////////////////////////////////////////////////////////////////


extern bool get_pinterp_grid(const char * pinterp_filename, Grid &);

extern bool get_pinterp_grid(NcFile &, Grid &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __GET_PINTERP_GRID_H__  */


////////////////////////////////////////////////////////////////////////


