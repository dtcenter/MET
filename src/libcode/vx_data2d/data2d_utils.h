// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
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

extern bool build_grid_by_grid_string(
               const char *attr_grid, Grid &grid,
               const char *caller_name=nullptr,
               bool do_warning=true);

extern bool build_grid_by_grid_string(
               const ConcatString &attr_grid, Grid &grid,
               const char *caller_name=nullptr,
               bool do_warning=true);

extern bool derive_wind_speed(
               const DataPlane &uwnd, const DataPlane &vwnd,
               DataPlane &wspd);

extern bool derive_wind_direction(
               const DataPlane &uwnd, const DataPlane &vwnd,
               DataPlane &wdir);

extern bool derive_u_wind(
               const DataPlane &wspd, const DataPlane &wdir,
               DataPlane &uwnd);

extern bool derive_v_wind(
               const DataPlane &wspd, const DataPlane &wdir,
               DataPlane &vwnd);

extern void rotate_wind_direction_grid_to_earth(
               const DataPlane &wdir2d,
               const Grid &,
               DataPlane &wdir2d_rot);

extern bool rotate_uv_grid_to_earth(
               const DataPlane &u2d, const DataPlane &v2d,
               const Grid &,
               DataPlane &u2d_rot, DataPlane &v2d_rot);

extern void set_attrs(const VarInfo *info, DataPlane &dp);

////////////////////////////////////////////////////////////////////////

#endif   /*  __DATA_2D_UTILS_H__  */

////////////////////////////////////////////////////////////////////////
