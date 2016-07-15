// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   interp_util.h
//
//   Description:
//      Contains utility functions for interpolating data to points.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    11-17-08  Halley Gotway
//
///////////////////////////////////////////////////////////////////////////////

#ifndef  __INTERP_UTIL_H__
#define  __INTERP_UTIL_H__

///////////////////////////////////////////////////////////////////////////////

#include "data_plane.h"
#include "interp_mthd.h"

////////////////////////////////////////////////////////////////////////

// Exponent used in distance weighted mean calculations
static const int dw_mean_pow = 2;

////////////////////////////////////////////////////////////////////////
//
// Utility functions for horizontal interpolation on a DataPlane
//
////////////////////////////////////////////////////////////////////////

extern double interp_min     (const DataPlane &, int x_ll, int y_ll, int w, double);
extern double interp_max     (const DataPlane &, int x_ll, int y_ll, int w, double);
extern double interp_median  (const DataPlane &, int x_ll, int y_ll, int w, double);
extern double interp_uw_mean (const DataPlane &, int x_ll, int y_ll, int w, double);

extern void  dp_interp_min     (const DataPlane & fat, DataPlane & out, int w, double);
extern void  dp_interp_max     (const DataPlane & fat, DataPlane & out, int w, double);
extern void  dp_interp_uw_mean (const DataPlane & fat, DataPlane & out, int w, double);

extern double interp_dw_mean (const DataPlane &, int x_ll, int y_ll, int w, double obs_x, double obs_y, int i_pow, double);
extern double interp_ls_fit  (const DataPlane &, int x_ll, int y_ll, int w, double obs_x, double obx_y, double);

extern double interp_bilin   (const DataPlane &, double obs_x, double obs_y);

extern void   get_xy_ll      (double x, double y, int w, int h, int &x_ll, int &y_ll);

////////////////////////////////////////////////////////////////////////
//
// Utility functions for horizontal and vertical interpolation
//
////////////////////////////////////////////////////////////////////////

extern double compute_horz_interp(const DataPlane &, double, double, int, int, double);
extern double compute_vert_pinterp(double, double, double, double, double);
extern double compute_vert_zinterp(double, double, double, double, double);

////////////////////////////////////////////////////////////////////////
//
// Interpolate two fields in time
//
///////////////////////////////////////////////////////////////////////////////

extern DataPlane valid_time_interp(const DataPlane &, const DataPlane &,
                                   const unixtime, InterpMthd);

///////////////////////////////////////////////////////////////////////////////

#endif   //  __INTERP_UTIL_H__

///////////////////////////////////////////////////////////////////////////////
