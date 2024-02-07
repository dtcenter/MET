// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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

#include "vx_log.h"
#include "vx_data2d.h"
#include "vx_nc_util.h"
#include "vx_tc_util.h"
#include "tcrmw_grid.h"

////////////////////////////////////////////////////////////////////////

extern void write_tc_track_lines(netCDF::NcFile*,
    const TrackInfo&);

extern void write_tc_track_lat_lon(netCDF::NcFile*,
    const netCDF::NcDim&, const TrackInfo&);

extern void write_tc_track_point(netCDF::NcFile*,
    const netCDF::NcDim&, const TrackPoint&);

extern void write_tc_rmw(netCDF::NcFile*,
    const netCDF::NcDim&, const TrackInfo&);

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

extern void def_tc_pressure(netCDF::NcFile*,
    const netCDF::NcDim&, std::set<double>);

extern void def_tc_range_azimuth(netCDF::NcFile*,
    const netCDF::NcDim&, const netCDF::NcDim&, const TcrmwGrid&, double);

extern void def_tc_init_time(netCDF::NcFile*,
    netCDF::NcVar&, netCDF::NcVar&);

extern void def_tc_valid_time(netCDF::NcFile*,
    const netCDF::NcDim&, netCDF::NcVar&, netCDF::NcVar&);

extern void def_tc_lead_time(netCDF::NcFile*,
    const netCDF::NcDim&, netCDF::NcVar&, netCDF::NcVar&);

extern void def_tc_lat_lon(netCDF::NcFile*,
    const netCDF::NcDim&, const netCDF::NcDim&, const netCDF::NcDim&,
    netCDF::NcVar&, netCDF::NcVar&);

extern void def_tc_variables(netCDF::NcFile*,
    std::map<std::string, std::vector<std::string> >,
    std::map<std::string, std::string>, std::map<std::string, std::string>,
    const netCDF::NcDim&, const netCDF::NcDim&, const netCDF::NcDim&, const netCDF::NcDim&,
    std::map<std::string, netCDF::NcVar>&);

extern void def_tc_data(netCDF::NcFile*,
    const netCDF::NcDim&, const netCDF::NcDim&, const netCDF::NcDim&,
    netCDF::NcVar&, VarInfo*);

extern void def_tc_data_3d(netCDF::NcFile*,
    const netCDF::NcDim&, const netCDF::NcDim&, const netCDF::NcDim&, const netCDF::NcDim&,
    netCDF::NcVar&, VarInfo*);

extern void def_tc_azi_mean_data(netCDF::NcFile*,
    const netCDF::NcDim&, const netCDF::NcDim&,
    netCDF::NcVar&, VarInfo*);

extern void write_tc_init_time(netCDF::NcFile*,
    const netCDF::NcVar&, const netCDF::NcVar&,
    const unixtime&);

extern void write_tc_valid_time(netCDF::NcFile*,
    const int&,
    const netCDF::NcVar&, const netCDF::NcVar&,
    const unixtime&);

extern void write_tc_lead_time(netCDF::NcFile*,
    const int&,
    const netCDF::NcVar&, const netCDF::NcVar&,
    const int&);

extern void write_tc_data(netCDF::NcFile*, const TcrmwGrid&,
    const int&, const netCDF::NcVar&, const double*);

extern void write_tc_data_rev(netCDF::NcFile*, const TcrmwGrid&,
    const int&, const netCDF::NcVar&, const double*);

extern void write_tc_azi_mean_data(netCDF::NcFile*, const TcrmwGrid&,
    const int&, const netCDF::NcVar&, const double*);

extern void write_tc_pressure_level_data(netCDF::NcFile*, const TcrmwGrid&,
    std::map<std::string, int>, const std::string&,
    const int&, const netCDF::NcVar&, const double*);

extern void write_tc_pressure_level_data(netCDF::NcFile*, const TcrmwGrid&,
    const int&, const int&, const netCDF::NcVar&, const double*);

////////////////////////////////////////////////////////////////////////

#endif  //  __VX_TC_NC_UTIL_H__

////////////////////////////////////////////////////////////////////////
