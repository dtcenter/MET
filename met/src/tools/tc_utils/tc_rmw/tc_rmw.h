// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   tc_rmw.h
//
//   Description:
//
//   Mod#  Date      Name      Description
//   ----  ----      ----      -----------
//   000   04/18/19  Fillmore  New
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

// Default output prefix
static const char* default_out_prefix = "";

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

// Input files
static StringArray   data_files;
static StringArray   deck_source, deck_model_suffix;
static ConcatString  config_file;
static TCRMWConfInfo conf_info;
static GrdFileType   ftype;

// Optional arguments
static ConcatString out_dir;
static ConcatString out_prefix;

////////////////////////////////////////////////////////////////////////
//
// Variables for Output Files
//
////////////////////////////////////////////////////////////////////////

// Output NetCDF file
static ConcatString out_file;
static NcFile*      nc_out = (NcFile*) 0;
static NcDim        range_dim;
static NcDim        azimuth_dim;
static NcDim        pressure_dim;
static NcDim        track_point_dim;
static NcVar        lat_arr_var;
static NcVar        lon_arr_var;
static NcVar        valid_time_var;
static NcVar        data_var;
static NcVar        wind_r_var;
static NcVar        wind_a_var;

static vector<NcVar> data_vars;
static vector<NcVar> azi_mean_data_vars;

static map<string, NcVar> data_3d_vars;

// List of output NetCDF variable names
static StringArray nc_var_sa;

static map<string, vector<string> > variable_levels;
static map<string, string> variable_long_names;
static map<string, string> variable_units;

static set<string> pressure_level_strings;
static set<double> pressure_levels;
static map<string, int> pressure_level_indices;

////////////////////////////////////////////////////////////////////////
//
// Miscellaneous Variables
//
////////////////////////////////////////////////////////////////////////

static DataPlane dp;
static Grid      latlon_arr;
static TcrmwData grid_data;
static TcrmwGrid tcrmw_grid;
static Grid      grid;

// Grid coordinate arrays
static double* lat_arr;
static double* lon_arr;

// Wind arrays
static double* wind_r_arr;
static double* wind_a_arr;

////////////////////////////////////////////////////////////////////////

#endif  //  __TC_RMW_H__

////////////////////////////////////////////////////////////////////////
