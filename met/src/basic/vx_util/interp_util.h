// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
#include "GridTemplate.h"
#include "config_gaussian.h"

///////////////////////////////////////////////////////////////////////////////

// Exponent used in distance weighted mean calculations
static const int dw_mean_pow = 2;

////////////////////////////////////////////////////////////////////////

//
// Struct to store information about the surface
//

struct SurfaceInfo {
   MaskPlane   * land_ptr;     // Pointer to land/sea mask (not allocated)
   DataPlane   * topo_ptr;     // Pointer to model topography (not allocated)
   SingleThresh  topo_use_obs_thresh;
   SingleThresh  topo_interp_fcst_thresh;

   SurfaceInfo();

   void clear();
};

///////////////////////////////////////////////////////////////////////////////
//
// Utility functions for horizontal interpolation on a DataPlane
//
///////////////////////////////////////////////////////////////////////////////

// Round x_dbl and y_dbl to nearest integer
extern NumArray interp_points  (const DataPlane &dp, const GridTemplate &gt, double x_dbl, double y_dbl);

// GridTemplate version takes center x/y
extern NumArray interp_points  (const DataPlane &dp, const GridTemplate &gt, int x, int y);
extern double   interp_min     (const DataPlane &dp, const GridTemplate &gt, int x, int y, double t, const MaskPlane *mp = 0);
extern double   interp_max     (const DataPlane &dp, const GridTemplate &gt, int x, int y, double t, const MaskPlane *mp = 0);
extern double   interp_median  (const DataPlane &dp, const GridTemplate &gt, int x, int y, double t, const MaskPlane *mp = 0);
extern double   interp_uw_mean (const DataPlane &dp, const GridTemplate &gt, int x, int y, double t, const MaskPlane *mp = 0);

// Non-GridTemplate version takes lower-left corner x/y
extern double   interp_min_ll     (const DataPlane &dp, int x_ll, int y_ll, int w, double t);
extern double   interp_max_ll     (const DataPlane &dp, int x_ll, int y_ll, int w, double t);
extern double   interp_median_ll  (const DataPlane &dp, int x_ll, int y_ll, int w, double t);
extern double   interp_uw_mean_ll (const DataPlane &dp, int x_ll, int y_ll, int w, double t);

// GridTemplate version takes center x/y
extern double   interp_dw_mean   (const DataPlane &, const GridTemplate &gt, double obs_x, double obs_y, int i_pow, double t, const MaskPlane *mp = 0);
extern double   interp_ls_fit    (const DataPlane &, const GridTemplate &gt, double obs_x, double obs_y, double t, const MaskPlane *mp = 0);
extern void     interp_gaussian_dp(DataPlane &, const GaussianInfo &, double t);
extern double   interp_gaussian  (const DataPlane &, const DataPlane &, double obs_x, double obs_y, int max_r, double t);

extern double   interp_geog_match(const DataPlane &, const GridTemplate &gt, double obs_x, double obs_y, double obs_v, const MaskPlane *mp = 0);

extern double   interp_nbrhd   (const DataPlane &, const GridTemplate &gt, int x, int y, double t, const SingleThresh *,
                                double cmn, double csd, const MaskPlane *mp = 0);
extern double   interp_bilin   (const DataPlane &, double obs_x, double obs_y, const MaskPlane *mp = 0);
extern double   interp_xy      (const DataPlane &, int x, int y, const MaskPlane *mp = 0);

extern double   interp_best    (const DataPlane &dp, const GridTemplate &gt, int x, int y, double obs_v, double t, const MaskPlane *mp = 0);

extern void     get_xy_ll      (double x, double y, int w, int h, int &x_ll, int &y_ll);

///////////////////////////////////////////////////////////////////////////////
//
// Utility functions for horizontal and vertical interpolation
//
///////////////////////////////////////////////////////////////////////////////

extern double compute_sfc_interp(const DataPlane &dp,
                                 double obs_x,   double obs_y,
                                 double obs_elv, double obs_v,
                                 const InterpMthd mthd, const int width,
                                 const GridTemplateFactory::GridTemplates shape,
                                 double interp_thresh, const SurfaceInfo &sfc_info,
                                 bool is_land_obs);

extern MaskPlane compute_sfc_mask(const GridTemplate &gt, int x, int y,
                                  const SurfaceInfo &sfc_info,
                                  bool is_land_obs, double obs_elv);

extern double compute_horz_interp(const DataPlane &dp,
                                  double obs_x, double obs_y, double obs_v,
                                  const InterpMthd mthd, const int width,
                                  const GridTemplateFactory::GridTemplates shape,
                                  double interp_thresh, const SingleThresh *cat_thresh = 0);

extern double compute_horz_interp(const DataPlane &dp,
                                  double obs_x, double obs_y,
                                  double obs_v, double cmn, double csd,
                                  const InterpMthd mthd, const int width,
                                  const GridTemplateFactory::GridTemplates shape,
                                  double interp_thresh, const SingleThresh *cat_thresh = 0);

extern double compute_vert_pinterp(double, double, double, double, double);
extern double compute_vert_zinterp(double, double, double, double, double);

///////////////////////////////////////////////////////////////////////////////
//
// Interpolate two fields in time
//
///////////////////////////////////////////////////////////////////////////////

extern DataPlane valid_time_interp(const DataPlane &, const DataPlane &,
                                   const unixtime, InterpMthd);

///////////////////////////////////////////////////////////////////////////////

#endif   //  __INTERP_UTIL_H__

///////////////////////////////////////////////////////////////////////////////
