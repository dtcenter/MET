// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_READ_GRIDDED_DATA_H__
#define  __VX_READ_GRIDDED_DATA_H__

////////////////////////////////////////////////////////////////////////

#include "vx_met_util/constants.h"
#include "vx_met_util/file_type.h"
#include "vx_met_util/pair_data.h"
#include "vx_data_grids/grid.h"
#include "vx_wrfdata/vx_wrfdata.h"
#include "vx_math/vx_math.h"

////////////////////////////////////////////////////////////////////////

extern int read_field(const char *, GCInfo &, unixtime, int,
                      WrfData &, Grid &, int);

extern int read_field_levels(const char *, GCInfo &, unixtime, int,
                             WrfData *&, NumArray &, Grid &, int);

////////////////////////////////////////////////////////////////////////

#endif   //  __VX_READ_GRIDDED_DATA_H__

////////////////////////////////////////////////////////////////////////
