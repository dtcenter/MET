// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   vx_plot_util.h
//
//   Description:
//
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    11-06-06  Halley Gotway
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __VX_PLOT_UTIL_H__
#define __VX_PLOT_UTIL_H__

////////////////////////////////////////////////////////////////////////

#include "vx_config.h"
#include "vx_color.h"
#include "vx_ps.h"
#include "vx_pxm.h"
#include "vx_render.h"
#include "nav.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_math.h"
#include "map_region.h"

////////////////////////////////////////////////////////////////////////////////

typedef enum {satellite, lambert, mercator} Projection;

////////////////////////////////////////////////////////////////////////////////

extern void draw_map(const Grid &, const Box &, PSfile &, const Box &,
                     MetConfig *);

extern void draw_map_data(const Grid &, const Box &, PSfile &, const Box &,
                          const char *);

extern void draw_region(const Grid &, const Box &, PSfile &, const Box &,
                        const MapRegion &);

extern bool region_overlaps_grid(const Grid &, const Box &,
                                 const MapRegion &);

extern void draw_grid(const Grid &, const Box &, int, PSfile &, const Box &,
                      Color);

extern void gc_arcto(const Grid &, const Box &, PSfile &, double, double,
                     double, double, double, const Box &);

extern void gridxy_to_pagexy(const Grid &, const Box &, double, double,
                             double &, double &, const Box &);

extern void latlon_to_pagexy(const Grid &, const Box &, double, double,
                             double &, double &, const Box &);

////////////////////////////////////////////////////////////////////////////////

#endif  //  __VX_PLOT_UTIL_H__

////////////////////////////////////////////////////////////////////////////////
