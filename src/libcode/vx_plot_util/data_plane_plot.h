// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   data_plane_plot.h
//
//   Description:
//
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    12-28-11  Holmes
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DATA_PLANE_PLOT_H__
#define __DATA_PLANE_PLOT_H__

////////////////////////////////////////////////////////////////////////////////

#include "vx_plot_util.h"

////////////////////////////////////////////////////////////////////////////////

extern void data_plane_plot(const ConcatString &, const ConcatString &,
                            const Grid &, const ConcatString &,
                            const ColorTable &, MetConfig *, const DataPlane &);

extern void create_image(Ppm &, const Grid &, const DataPlane &,
                         const ColorTable &);

extern void fill_colorbar_image(Ppm &, const ColorTable &);

extern void draw_border(PSfile &, const Box &, double);

////////////////////////////////////////////////////////////////////////////////

#endif  //  __DATA_PLANE_PLOT_H__

////////////////////////////////////////////////////////////////////////////////
