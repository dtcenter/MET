// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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

extern void write_tc_tracks(NcFile*,
    const NcDim&, const TrackInfoArray&);

extern std::set<std::string> get_pressure_level_strings(
    std::map<std::string, std::vector<std::string> >);

extern std::set<double> get_pressure_levels(
    std::map<std::string, std::vector<std::string> >);

extern std::set<double> get_pressure_levels(
    std::set<std::string>);

extern std::map<double, int> get_pressure_level_indices(
    std::set<double>);

extern std::map<std::string, int> get_pressure_level_indices(
    std::set<std::string>, std::set<double>);

extern void def_tc_pressure(NcFile*,
    const NcDim&, std::set<double>);

extern void def_tc_range_azimuth(NcFile*,
    const NcDim&, const NcDim&, const TcrmwGrid&, double);

extern void def_tc_lat_lon_time(NcFile*,
    const NcDim&, const NcDim&, const NcDim&,
    NcVar&, NcVar&, NcVar&);

extern void def_tc_variables(NcFile*,
    std::map<std::string, std::vector<std::string> >,
    std::map<std::string, std::string>, std::map<std::string, std::string>,
    const NcDim&, const NcDim&, const NcDim&, const NcDim&,
    std::map<std::string, NcVar>&);

extern void def_tc_data(NcFile*,
    const NcDim&, const NcDim&, const NcDim&,
    NcVar&, VarInfo*);

extern void def_tc_data_3d(NcFile*,
    const NcDim&, const NcDim&, const NcDim&, const NcDim&,
    NcVar&, VarInfo*);

extern void def_tc_azi_mean_data(NcFile*,
    const NcDim&, const NcDim&,
    NcVar&, VarInfo*);

extern void write_tc_valid_time(NcFile*,
    const int&, const NcVar&, const long&);

extern void write_tc_data(NcFile*, const TcrmwGrid&,
    const int&, const NcVar&, const double*);

extern void write_tc_data_rev(NcFile*, const TcrmwGrid&,
    const int&, const NcVar&, const double*);

extern void write_tc_azi_mean_data(NcFile*, const TcrmwGrid&,
    const int&, const NcVar&, const double*);

extern void write_tc_pressure_level_data(NcFile*, const TcrmwGrid&,
    std::map<std::string, int>, const std::string&,
    const int&, const NcVar&, const double*);

////////////////////////////////////////////////////////////////////////

#endif  //  __VX_TC_NC_UTIL_H__

////////////////////////////////////////////////////////////////////////
