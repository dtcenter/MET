// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
using namespace std;

#include <netcdf>
using namespace netCDF;

#include "vx_util.h"
#include "vx_log.h"
#include "vx_data2d.h"
#include "vx_data2d_factory.h"
#include "vx_grid.h"

////////////////////////////////////////////////////////////////////////

void init_pdf(
    int n,
    vector<int>& pdf);

////////////////////////////////////////////////////////////////////////

void init_pdf(
    double min,
    double max,
    double delta,
    vector<int>& pdf);

////////////////////////////////////////////////////////////////////////

void init_joint_pdf(
    int n_A,
    int n_B,
    vector<int>& pdf);

////////////////////////////////////////////////////////////////////////

void update_pdf(
    double min,
    double delta,
    vector<int>& pdf,
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
    vector<int>& pdf,
    const DataPlane&,
    const DataPlane&,
    const MaskPlane&);

////////////////////////////////////////////////////////////////////////

void print_pdf(
    double min,
    double delta,
    const vector<int>& pdf);

////////////////////////////////////////////////////////////////////////

void write_nc_pdf(
    NcFile* nc_out,
    const VarInfo& info,
    double min,
    double delta,
    const vector<int>& pdf);

////////////////////////////////////////////////////////////////////////

#endif  // __SERIES_PDF_H__

////////////////////////////////////////////////////////////////////////
