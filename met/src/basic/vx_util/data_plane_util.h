// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
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
/*
extern void smooth_field(const DataPlane &, DataPlane &,
                         InterpMthd mthd, int wdth, double t);
*/
extern void smooth_field(const DataPlane &dp, DataPlane &smooth_dp,
                         InterpMthd mthd, int width, const GridTemplateFactory::GridTemplates shape, double t);
/*
extern void fractional_coverage(const DataPlane &, DataPlane &,
                                int, SingleThresh, double);
*/
extern void fractional_coverage(const DataPlane &dp, DataPlane &frac_dp,
                                int width, const GridTemplateFactory::GridTemplates shape, SingleThresh t, double vld_t);
	
extern void apply_mask(const DataPlane &, const DataPlane &, NumArray &);

extern void apply_mask(DataPlane &, const DataPlane &);

extern void mask_bad_data(DataPlane &, const DataPlane &, double v = bad_data_double);

////////////////////////////////////////////////////////////////////////

#endif   //  __DATA_PLANE_UTIL__

////////////////////////////////////////////////////////////////////////
