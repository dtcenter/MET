// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __SERIES_PDF_H__
#define  __SERIES_PDF_H__

////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <vector>

#include <netcdf>

#include "vx_util.h"
#include "vx_log.h"
#include "vx_data2d.h"
#include "vx_data2d_factory.h"
#include "vx_grid.h"

////////////////////////////////////////////////////////////////////////

void init_pdf(
    int n,
    std::vector<long long>& pdf);

////////////////////////////////////////////////////////////////////////

void init_pdf(
    double min,
    double max,
    double delta,
    std::vector<long long>& pdf);

////////////////////////////////////////////////////////////////////////

void init_joint_pdf(
    int n_A,
    int n_B,
    std::vector<long long>& pdf);

////////////////////////////////////////////////////////////////////////

void update_pdf(
    double min,
    double delta,
    std::vector<long long>& pdf,
    const DataPlane&,
    const MaskPlane&);

////////////////////////////////////////////////////////////////////////

void update_joint_pdf(
    int n_A,
    int n_B,
    double min_A,
    double min_B,
    double delta_A,
    double delta_B,
    std::vector<long long>& pdf,
    const DataPlane&,
    const DataPlane&,
    const MaskPlane&);

////////////////////////////////////////////////////////////////////////

void print_pdf(
    double min,
    double delta,
    const std::vector<long long>& pdf);

////////////////////////////////////////////////////////////////////////

void write_nc_pdf(
    netCDF::NcFile* nc_out,
    const VarInfo& info,
    double min,
    double delta,
    const std::vector<long long>& pdf);

////////////////////////////////////////////////////////////////////////

#endif  // __SERIES_PDF_H__

////////////////////////////////////////////////////////////////////////
