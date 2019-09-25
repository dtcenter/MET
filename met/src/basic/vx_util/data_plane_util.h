// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////
//
//   Filename:   data_plane_util.h
//
//   Description:
//      Contains utility functions that operate on data planes.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    12-01-03  Halley Gotway
//
////////////////////////////////////////////////////////////////////////

#ifndef  __DATA_PLANE_UTIL__
#define  __DATA_PLANE_UTIL__

////////////////////////////////////////////////////////////////////////

#include "data_plane.h"
#include "interp_mthd.h"
#include "num_array.h"

#include "GridTemplate.h"

////////////////////////////////////////////////////////////////////////
//
// Utility functions operating on a DataPlane
//
////////////////////////////////////////////////////////////////////////

extern void rescale_probability(DataPlane &);

extern void smooth_field(const DataPlane &dp, DataPlane &smooth_dp,
               InterpMthd mthd, int width,
               const GridTemplateFactory::GridTemplates shape,
               double t, const double gaussian_radius, const double gaussian_dx);

extern DataPlane smooth_field(const DataPlane &dp,
                    InterpMthd mthd, int width,
                    const GridTemplateFactory::GridTemplates shape,
                    double t, const double gaussian_radius, const double gaussian_dx);

extern void fractional_coverage(const DataPlane &dp, DataPlane &frac_dp,
               int width, const GridTemplateFactory::GridTemplates shape,
               SingleThresh t, double vld_t);

extern void fractional_coverage_square(const DataPlane &dp, DataPlane &frac_dp,
               int width, SingleThresh t, double vld_t);

extern void apply_mask(const DataPlane &, const MaskPlane &, NumArray &);
extern void apply_mask(DataPlane &, const MaskPlane &);
extern void apply_mask(MaskPlane &, const MaskPlane &);

extern void mask_bad_data(DataPlane &, const DataPlane &,
                          double v = bad_data_double);
extern void mask_bad_data(MaskPlane &, const DataPlane &);

extern DataPlane subtract(const DataPlane &, const DataPlane &);

extern DataPlane normal_cdf(const DataPlane &, const DataPlane &,
                            const DataPlane &);

extern DataPlane normal_cdf_inv(const double, const DataPlane &,
                                const DataPlane &);

extern DataPlane gradient(const DataPlane &, int dim, int delta);

////////////////////////////////////////////////////////////////////////

#endif   //  __DATA_PLANE_UTIL__

////////////////////////////////////////////////////////////////////////
