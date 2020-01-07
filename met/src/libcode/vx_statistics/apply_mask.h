// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __APPLY_MASK_H__
#define  __APPLY_MASK_H__

////////////////////////////////////////////////////////////////////////

#include "mask_poly.h"
#include "vx_shapedata.h"
#include "vx_grid.h"

////////////////////////////////////////////////////////////////////////

static const double mask_on_value         = 1.0;
static const double mask_off_value        = 0.0;
static const char   default_mask_thresh[] = "!=0.0";
static const char   default_mask_dict[]   = "name=\"NA\"; level=\"NA\";";
static const char   poly_str_delim[]      = "{}";

////////////////////////////////////////////////////////////////////////

extern Grid parse_vx_grid(const RegridInfo, const Grid *, const Grid *);

extern void parse_grid_weight(const Grid &, const GridWeightType,
                              DataPlane &);

extern void parse_grid_mask(const ConcatString &, const Grid &,
                            DataPlane &, ConcatString &);
extern void parse_grid_mask(const ConcatString &, const Grid &,
                            MaskPlane &, ConcatString &);
extern void parse_grid_mask(const ConcatString &, Grid &);

extern void parse_poly_mask(const ConcatString &, const Grid &,
                            DataPlane &, ConcatString &);
extern void parse_poly_mask(const ConcatString &, const Grid &,
                            MaskPlane &, ConcatString &);
extern void parse_poly_mask(const ConcatString &, MaskPoly &,
                            Grid &, MaskPlane &, ConcatString &);
extern void parse_poly_2d_data_mask(const ConcatString &, Grid &,
                                    DataPlane &, ConcatString &);

extern void apply_grid_mask(const Grid &, const Grid &,
                            DataPlane &);
extern void apply_poly_mask_latlon(const MaskPoly &, const Grid &,
                                   DataPlane &);

extern DataPlane parse_geog_data(Dictionary *dict, const Grid &grid,
                                 const char *);

////////////////////////////////////////////////////////////////////////

#endif   //  __APPLY_MASK_H__

////////////////////////////////////////////////////////////////////////
