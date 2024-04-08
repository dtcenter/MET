// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
//   Mod#  Date      Name       Description
//   ----  ----      ----       -----------
//   000   04/18/19  Fillmore   New
//   001   09/28/22  Prestopnik MET #2227 Remove namespace std and netCDF from header files
//
////////////////////////////////////////////////////////////////////////

#ifndef  __TC_RMW_H__
#define  __TC_RMW_H__

////////////////////////////////////////////////////////////////////////


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

#include "tc_rmw_conf_info.h"
#include "tc_rmw_wind_converter.h"

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
static TCRMW_WindConverter wind_converter;


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
static netCDF::NcFile*      nc_out = (netCDF::NcFile*) nullptr;
static netCDF::NcDim        range_dim;
static netCDF::NcDim        azimuth_dim;
static netCDF::NcDim        pressure_dim;
static netCDF::NcDim        track_point_dim;
static netCDF::NcVar        init_time_str_var;
static netCDF::NcVar        init_time_ut_var;
static netCDF::NcVar        valid_time_str_var;
static netCDF::NcVar        valid_time_ut_var;
static netCDF::NcVar        lead_time_str_var;
static netCDF::NcVar        lead_time_sec_var;
static netCDF::NcVar        lat_arr_var;
static netCDF::NcVar        lon_arr_var;
static netCDF::NcVar        data_var;
static netCDF::NcVar        wind_r_var;
static netCDF::NcVar        wind_a_var;

static std::vector<netCDF::NcVar> data_vars;
static std::vector<netCDF::NcVar> azi_mean_data_vars;

static std::map<std::string, netCDF::NcVar> data_3d_vars;

// List of output NetCDF variable names
static StringArray nc_var_sa;

static std::map<std::string, std::vector<std::string> > variable_levels;
static std::map<std::string, std::string> variable_long_names;
static std::map<std::string, std::string> variable_units;

static std::set<std::string> pressure_level_strings;
static std::set<double> pressure_levels;
static std::map<std::string, int> pressure_level_indices;

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

////////////////////////////////////////////////////////////////////////

#endif  //  __TC_RMW_H__

////////////////////////////////////////////////////////////////////////
