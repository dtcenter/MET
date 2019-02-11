// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MET_VX_REGRID_H__
#define  __MET_VX_REGRID_H__


////////////////////////////////////////////////////////////////////////


#include "vx_grid.h"
#include "data_plane.h"
#include "config_constants.h"


////////////////////////////////////////////////////////////////////////


extern DataPlane met_regrid (const DataPlane & in, const Grid & from_grid, const Grid & to_grid, const RegridInfo & info);


////////////////////////////////////////////////////////////////////////


extern DataPlane met_regrid_generic       (const DataPlane & in, const Grid & from_grid, const Grid & to_grid, const RegridInfo & info);
extern DataPlane met_regrid_budget        (const DataPlane & in, const Grid & from_grid, const Grid & to_grid, const RegridInfo & info);
extern DataPlane met_regrid_area_weighted (const DataPlane & in, const Grid & from_grid, const Grid & to_grid, const RegridInfo & info);
extern DataPlane met_regrid_force         (const DataPlane & in, const Grid & from_grid, const Grid & to_grid, const RegridInfo & info);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_VX_REGRID_H__  */


////////////////////////////////////////////////////////////////////////


