// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_TC_NC_UTIL_H__
#define  __VX_TC_NC_UTIL_H__

////////////////////////////////////////////////////////////////////////

#include <netcdf>
using namespace netCDF;

#include "vx_log.h"
#include "vx_nc_util.h"
#include "vx_tc_util.h"
#include "tcrmw_grid.h"

////////////////////////////////////////////////////////////////////////

extern void write_nc_tracks(const ConcatString&,
    const TrackInfoArray&);

extern void write_nc_range_azimuth(NcFile*,
    const NcDim&, const NcDim&, const TcrmwGrid&);

////////////////////////////////////////////////////////////////////////

#endif  //  __VX_TC_NC_UTIL_H__

////////////////////////////////////////////////////////////////////////
