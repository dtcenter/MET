// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#ifndef  __DATA_2D_UTILS_H__
#define  __DATA_2D_UTILS_H__

////////////////////////////////////////////////////////////////////////

#include "vx_grid.h"
#include "data_plane.h"
#include "var_info.h"

////////////////////////////////////////////////////////////////////////

extern bool derive_wdir(const DataPlane &u2d, const DataPlane &v2d,
                        DataPlane &wdir);

extern bool derive_wind(const DataPlane &u2d, const DataPlane &v2d,
                        DataPlane &wind);

extern void rotate_wdir_grid_to_earth(const DataPlane &wdir2d,
                                      const Grid &,
                                      DataPlane &wdir2d_rot);

extern bool rotate_uv_grid_to_earth(const DataPlane &u2d, const DataPlane &v2d,
                                    const Grid &,
                                    DataPlane &u2d_rot, DataPlane &v2d_rot);

extern void set_attrs(const VarInfo *info, DataPlane &dp);

////////////////////////////////////////////////////////////////////////

#endif   /*  __DATA_2D_UTILS_H__  */

////////////////////////////////////////////////////////////////////////
