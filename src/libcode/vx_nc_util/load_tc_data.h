// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __LOAD_TC_DATA_H__
#define  __LOAD_TC_DATA_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_log.h"
#include "vx_grid.h"
#include "vx_data2d.h"

////////////////////////////////////////////////////////////////////////

extern void load_tc_dland(const ConcatString &, Grid &, DataPlane &);

extern void load_tc_basin(const ConcatString &, Grid &, DataPlane &,
                          StringArray &);

////////////////////////////////////////////////////////////////////////

#endif   /*  __LOAD_TC_DATA_H__  */

////////////////////////////////////////////////////////////////////////
