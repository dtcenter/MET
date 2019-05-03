// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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

#ifndef _VX_PLOT_UTIL_H_
#define _VX_PLOT_UTIL_H_

////////////////////////////////////////////////////////////////////////

#include "vx_color.h"
#include "vx_ps.h"
#include "vx_pxm.h"
#include "nav.h"
#include "grid.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////////////

static const int max_region_points = 3000;

static const char usa_county_data[]    = "map/county_data";
static const char usa_state_data[]     = "map/usa_data";
static const char world_outline_data[] = "map/group3";

////////////////////////////////////////////////////////////////////////////////

typedef enum {satellite, lambert, mercator} Projection;

////////////////////////////////////////////////////////////////////////////////

struct MapRegion
{
   int number;
   int n_points;

   int a;
   int b;
   int c;

   double lat_max;
   double lat_min;

   double lon_max;
   double lon_min;

   double lat[max_region_points];
   double lon[max_region_points];
};

////////////////////////////////////////////////////////////////////////

struct CountyRegion {

   int n_points;

   int a;
   int b;

   double lat_max;
   double lat_min;

   double lon_max;
   double lon_min;

   double lat[max_region_points];
   double lon[max_region_points];
};

////////////////////////////////////////////////////////////////////////////////

extern int operator>>(ifstream &, MapRegion &);

extern int operator>>(ifstream &, CountyRegion &);

extern void draw_states(const Grid &, const BoundingBox &, PSfile &,
                        const BoundingBox &, const BoundingBox &, Color);
extern void draw_states(const Grid &, const BoundingBox &, PSfile &,
                        const BoundingBox &, const BoundingBox &, Color,
                        const char *);
extern void draw_states(const Grid &, PSfile &,
                        const BoundingBox &, const BoundingBox &, Color);

extern void draw_state_region(const Grid &, const BoundingBox &, PSfile &,
                              const BoundingBox &, const MapRegion &);
extern void draw_state_region(const Grid &, PSfile &,
                              const BoundingBox &, const MapRegion &);

extern void draw_counties(const Grid &, const BoundingBox &, PSfile &,
                          const BoundingBox &, const BoundingBox &, Color);
extern void draw_counties(const Grid &, const BoundingBox &, PSfile &,
                          const BoundingBox &, const BoundingBox &, Color,
                          const char *);
extern void draw_counties(const Grid &, PSfile &,
                          const BoundingBox &, const BoundingBox &, Color);

extern void draw_county_region(const Grid &, const BoundingBox &, PSfile &,
                               const BoundingBox &, const CountyRegion &);
extern void draw_county_region(const Grid &, PSfile &,
                               const BoundingBox &, const CountyRegion &);

extern void draw_world(const Grid &, const BoundingBox &, PSfile &,
                       const BoundingBox &, const BoundingBox &, Color);
extern void draw_world(const Grid &, const BoundingBox &, PSfile &,
                       const BoundingBox &, const BoundingBox &, Color,
                       const char *);
extern void draw_world(const Grid &, PSfile &,
                       const BoundingBox &, const BoundingBox &, Color);

extern void draw_world_region(const Grid &, const BoundingBox &, PSfile &,
                              const BoundingBox &, const MapRegion &);
extern void draw_world_region(const Grid &, PSfile &,
                              const BoundingBox &, const MapRegion &);

extern void draw_grid(const Grid &, const BoundingBox &, int, PSfile &,
                      const BoundingBox &, Color);
extern void draw_grid(const Grid &, int, PSfile &,
                      const BoundingBox &, Color);

extern void gc_arcto(const Grid &, const BoundingBox &, PSfile &, double, double,
                     double, double, double, const BoundingBox &);
extern void gc_arcto(const Grid &, PSfile &, double, double,
                     double, double, double, const BoundingBox &);

extern void gridxy_to_pagexy(const Grid &, const BoundingBox &, double, double,
                             double &, double &, const BoundingBox &);
extern void gridxy_to_pagexy(const Grid &, double, double,
                             double &, double &, const BoundingBox &);

extern void latlon_to_pagexy(const Grid &, const BoundingBox &, double, double,
                             double &, double &, const BoundingBox &);
extern void latlon_to_pagexy(const Grid &, double, double,
                             double &, double &, const BoundingBox &);

////////////////////////////////////////////////////////////////////////////////

#endif  //  _VX_PLOT_UTIL_H_

////////////////////////////////////////////////////////////////////////////////
