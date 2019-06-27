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
#include "vx_data2d.h"
#include "vx_nc_util.h"
#include "vx_tc_util.h"
#include "tcrmw_grid.h"

////////////////////////////////////////////////////////////////////////

extern void write_tc_tracks(const ConcatString&,
    const TrackInfoArray&);

extern void def_tc_range_azimuth(NcFile*,
    const NcDim&, const NcDim&, const TcrmwGrid&);

extern void def_tc_lat_lon_time(NcFile*,
    const NcDim&, const NcDim&, const NcDim&,
    NcVar&, NcVar&, NcVar&);

extern void def_tc_data(NcFile*,
    const NcDim&, const NcDim&, const NcDim&,
    NcVar&, VarInfo*);

extern void write_tc_valid_time(NcFile*,
    const int&, const NcVar&, const long&);

extern void write_tc_data(NcFile*, const TcrmwGrid&,
    const int&, const NcVar&, double*);

////////////////////////////////////////////////////////////////////////

#endif  //  __VX_TC_NC_UTIL_H__

////////////////////////////////////////////////////////////////////////
