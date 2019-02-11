// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

////////////////////////////////////////////////////////////////////////

#include "vx_plot_util.h"

////////////////////////////////////////////////////////////////////////////////

extern void data_plane_plot(const ConcatString &, const ConcatString &, const Grid &,
                            const ConcatString &, const ColorTable &, MetConfig *,
                            const DataPlane &);

////////////////////////////////////////////////////////////////////////////////

#endif  //  __DATA_PLANE_PLOT_H__

////////////////////////////////////////////////////////////////////////////////
