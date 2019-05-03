

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __VX_GRID_FIND_GRID_BY_NAME_H__
#define  __VX_GRID_FIND_GRID_BY_NAME_H__


////////////////////////////////////////////////////////////////////////


#include "vx_data_grids/grid.h"


////////////////////////////////////////////////////////////////////////


extern bool find_grid_by_name(const char *, Grid &);

extern bool find_grid_by_name(const char *, GridInfo &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_GRID_FIND_GRID_BY_NAME_H__  */


////////////////////////////////////////////////////////////////////////




