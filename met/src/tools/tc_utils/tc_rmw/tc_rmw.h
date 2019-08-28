// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//    Filename:    tc_rmw.h
//
//    Description:
//
//    Mod#  Date      Name      Description
//    ----  ----      ----      -----------
//    000   04/18/19  Fillmore  New
//
////////////////////////////////////////////////////////////////////////

#ifndef  __TC_RMW_H__
#define  __TC_RMW_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <netcdf>
using namespace netCDF;

#include "tc_rmw_conf_info.h"

#include "vx_data2d_factory.h"
#include "vx_tc_util.h"
#include "vx_grid.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

// Program name
static const char* program_name = "tc_rmw";

// ATCF file suffix
static const char* atcf_suffix = ".dat";

// Default configuration file name
static const char* default_config_filename =
    "MET_BASE/config/TCRMWConfig_default";

// Default output directory
static const char* default_out_dir = ".";

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

// Input files
static StringArray    data_files, found_data_files;
static StringArray    adeck_source, adeck_model_suffix;
static StringArray    bdeck_source, bdeck_model_suffix;
static ConcatString   config_file;
static TCRMWConfInfo  conf_info;
static GrdFileType    ftype;

// Optional arguments
static ConcatString out_dir;

////////////////////////////////////////////////////////////////////////
//
// Variables for Output Files
//
////////////////////////////////////////////////////////////////////////

// Output NetCDF file
static ConcatString out_nc_file;
static NcFile*      nc_out = (NcFile*) 0;
static NcDim        range_dim;
static NcDim        azimuth_dim;
static NcDim        track_point_dim;
static NcVar        lat_arr_var;
static NcVar        lon_arr_var;
static NcVar        valid_time_var;
static NcVar        data_var;
static NcVar        wind_r_var;
static NcVar        wind_a_var;

static vector<NcVar> data_vars;
static vector<NcVar> azi_mean_data_vars;

// List of output NetCDF variable names
static StringArray nc_var_sa;

static map<string, vector<string> > variable_levels;

////////////////////////////////////////////////////////////////////////
//
// Miscellaneous Variables
//
////////////////////////////////////////////////////////////////////////

static StringArray  out_files;
static DataPlane    dp;
static Grid         latlon_arr;
static TcrmwData    grid_data;
static TcrmwGrid    tcrmw_grid;
static Grid         grid;
static ConcatString wwarn_file;

// Data file factory and input files
static Met2dDataFileFactory mtddf_factory;
static Met2dDataFile* data_mtddf = (Met2dDataFile*) 0;

// Grid coordinate arrays
static double* lat_arr;
static double* lon_arr;

// Wind arrays
static double* wind_r_arr;
static double* wind_a_arr;

////////////////////////////////////////////////////////////////////////

#endif  //  __TC_RMW_H__

////////////////////////////////////////////////////////////////////////
