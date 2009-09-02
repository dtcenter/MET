// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_APPLY_MASK_H__
#define  __VX_APPLY_MASK_H__

////////////////////////////////////////////////////////////////////////

#include "vx_data_grids/grid.h"
#include "vx_wrfdata/vx_wrfdata.h"

////////////////////////////////////////////////////////////////////////

extern void parse_grid_mask(const char *, const Grid &, WrfData &,
                            char *&);
extern void parse_grid_mask(const char *, Grid &, WrfData &);

extern void parse_poly_mask(const char *, const Grid &, WrfData &,
                            char *&);
extern void parse_poly_mask(const char *, Grid &, WrfData &);

extern void parse_sid_mask(const char *, StringArray &);

extern void apply_grid_mask(const Grid *, const Grid *, WrfData &, int);

extern void apply_poly_mask_xy(const Polyline *, WrfData &, int);

extern void apply_poly_mask_latlon(const Polyline *, const Grid *,
                                   WrfData &, int);

////////////////////////////////////////////////////////////////////////

#endif   //  __VX_APPLY_MASK_H__

////////////////////////////////////////////////////////////////////////
