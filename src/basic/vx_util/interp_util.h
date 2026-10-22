// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
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
//   001    09-07-21  Halley Gotway   Add wrap_lon.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef  __INTERP_UTIL_H__
#define  __INTERP_UTIL_H__

///////////////////////////////////////////////////////////////////////////////

#include "data_plane.h"
#include "field_type.h"
#include "interp_mthd.h"
#include "GridTemplate.h"
#include "config_gaussian.h"

#include <vector>

///////////////////////////////////////////////////////////////////////////////

// Exponent used in distance weighted mean calculations
static const int dw_mean_pow = 2;

////////////////////////////////////////////////////////////////////////

//
// Struct to store information about the surface
//

struct SurfaceInfo {

   // Pointer to land/sea mask (not allocated)
   bool          land_flag;
   MaskPlane *   land_ptr;

   // Pointer to model topography (not allocated)
   bool          topo_flag;
   DataPlane *   topo_ptr;

   // Topography interpolation options
   InterpMthd    topo_interp_mthd;
   int           topo_interp_wdth;
   GridTemplateFactory::GridTemplates topo_interp_shape;
   double        topo_interp_vld_thresh;

   // Filter observations based on topography difference
   SingleThresh  topo_mask_use_obs_thresh;

   // Filter model points based on topography difference
   SingleThresh  topo_mask_interp_fcst_thresh;

   // Lapse rate correction options for surface temperatures
   FieldType     lapse_rate_correction_apply_to;
   double        lapse_rate_correction_value;

   // MSL/AGL conversion options for heights
   FieldType     msl_agl_conversion_apply_to;
   FieldType     msl_agl_conversion_apply_from;
   SingleThresh  msl_agl_conversion_thresh;
   bool          msl_agl_conversion_msl_to_agl;

   SurfaceInfo();

   void clear();

   bool need_land() const;
   bool need_topo() const;
};

////////////////////////////////////////////////////////////////////////

//
// Neighboring faces of an unstructured grid (UGRID), sorted by distance
// from the closest to the farthest. The x index into the DataPlane is the
// face index and y is always 0.
//

struct UGridNeighbors {

   std::vector<int>    index;    // Face index
   std::vector<double> dist_km;  // Distance from the target point (km), empty if not needed
   std::vector<double> x_km;     // Local east offset from the target point (km)
   std::vector<double> y_km;     // Local north offset from the target point (km)

   // Area (km^2) of every face, indexed by the face index (not allocated),
   // nullptr if unknown
   const std::vector<double> *face_area = nullptr;

   void clear();
   int  n() const;
};

inline void UGridNeighbors::clear() {
   index.clear();
   dist_km.clear();
   x_km.clear();
   y_km.clear();
   face_area = nullptr;
}

inline int UGridNeighbors::n() const { return static_cast<int>(index.size()); }

//
// Closest faces of every face for smoothing the unstructured grid. Only
// the face indices are stored, n_max per face sorted by distance, with -1
// after the last face within the maximum distance. The first n faces of
// a row are the n closest faces, so the table serves any n <= n_max.
//

struct UGridNeighborTable {

   int              n_face = 0;
   int              n_max  = 0;
   std::vector<int> index;       // [n_face * n_max]

   // Area (km^2) of every face (not allocated), nullptr if unknown
   const std::vector<double> *face_area = nullptr;

   void clear();
   void get(int face, int n, UGridNeighbors &nbrs) const;
};

inline void UGridNeighborTable::clear() {
   n_face = 0;
   n_max = 0;
   std::vector<int>().swap(index);   // release the memory
   face_area = nullptr;
}

//
// Point information for the unstructured grid interpolation: the
// observation value (BEST) and the categorical threshold and
// climatology (NBRHD).
//

struct UGridPointInfo {
   double              obs_v      = bad_data_double;
   const ClimoPntInfo *cpi        = nullptr;
   const SingleThresh *cat_thresh = nullptr;
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
extern double   interp_min     (const DataPlane &dp, const GridTemplate &gt, int x, int y, double t, const MaskPlane *mp = nullptr);
extern double   interp_max     (const DataPlane &dp, const GridTemplate &gt, int x, int y, double t, const MaskPlane *mp = nullptr);
extern double   interp_median  (const DataPlane &dp, const GridTemplate &gt, int x, int y, double t, const MaskPlane *mp = nullptr);
extern double   interp_uw_mean (const DataPlane &dp, const GridTemplate &gt, int x, int y, double t, const MaskPlane *mp = nullptr);

// Non-GridTemplate version takes lower-left corner x/y
extern double   interp_min_ll     (const DataPlane &dp, int x_ll, int y_ll, int w, double t);
extern double   interp_max_ll     (const DataPlane &dp, int x_ll, int y_ll, int w, double t);
extern double   interp_median_ll  (const DataPlane &dp, int x_ll, int y_ll, int w, double t);
extern double   interp_uw_mean_ll (const DataPlane &dp, int x_ll, int y_ll, int w, double t);

// GridTemplate version takes center x/y
extern double   interp_dw_mean   (const DataPlane &, const GridTemplate &gt, double obs_x, double obs_y, int i_pow, double t, const MaskPlane *mp = nullptr);
extern double   interp_ls_fit    (const DataPlane &, const GridTemplate &gt, double obs_x, double obs_y, double t, const MaskPlane *mp = nullptr);
extern void     interp_gaussian_dp(DataPlane &, const GaussianInfo &, double t);
extern double   interp_gaussian  (const DataPlane &, const DataPlane &, double obs_x, double obs_y, int max_r, double t);

extern double   interp_geog_match(const DataPlane &, const GridTemplate &gt, double obs_x, double obs_y, double obs_v, const MaskPlane *mp = nullptr);

extern double   interp_nbrhd   (const DataPlane &, const GridTemplate &gt, int x, int y, double t, const SingleThresh *,
                                const ClimoPntInfo *, const MaskPlane *mp = nullptr);
extern double   interp_bilin   (const DataPlane &, bool wrap_lon, double obs_x, double obs_y, const MaskPlane *mp = nullptr);
extern double   interp_xy      (const DataPlane &, bool wrap_lon, int x, int y, const MaskPlane *mp = nullptr);

extern double   interp_best    (const DataPlane &dp, const GridTemplate &gt, int x, int y, double obs_v, double t, const MaskPlane *mp = nullptr);

extern void     get_xy_ll      (double x, double y, int w, int h, int &x_ll, int &y_ll);

// Unstructured grid version uses the N closest faces
extern int      ugrid_interp_n_points(const int width, const GridTemplateFactory::GridTemplates shape,
                                      const InterpMthd mthd = InterpMthd::None);
extern NumArray interp_ugrid_points  (const DataPlane &dp, const UGridNeighbors &nbrs, int n_points);
extern double   interp_ugrid         (const DataPlane &dp, const UGridNeighbors &nbrs,
                                      const InterpMthd mthd, int n_points, double t,
                                      const UGridPointInfo &pnt = UGridPointInfo());

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
                                 bool wrap_lon, double interp_thresh,
                                 const SurfaceInfo &sfc_info, bool is_land_obs);

extern MaskPlane compute_sfc_mask(const GridTemplate &gt, int x, int y,
                                  const SurfaceInfo &sfc_info,
                                  bool is_land_obs, double obs_elv);

extern double compute_horz_interp(const DataPlane &dp,
                                  double obs_x, double obs_y, double obs_v,
                                  const InterpMthd mthd, const int width,
                                  const GridTemplateFactory::GridTemplates shape,
                                  bool wrap_lon, double interp_thresh,
                                  const SingleThresh *cat_thresh = nullptr);

extern double compute_horz_interp(const DataPlane &dp,
                                  double obs_x, double obs_y,
                                  double obs_v, const ClimoPntInfo *,
                                  const InterpMthd mthd, const int width,
                                  const GridTemplateFactory::GridTemplates shape,
                                  bool wrap_lon, double interp_thresh,
                                  const SingleThresh *cat_thresh = nullptr);

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
