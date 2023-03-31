// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   tc_diag.h
//
//   Description:
//
//   Mod#  Date      Name          Description
//   ----  ----      ----          -----------
//   000   09/27/22  Halley Gotway New
//
////////////////////////////////////////////////////////////////////////

#ifndef  __TC_DIAG_H__
#define  __TC_DIAG_H__

////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <map>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <netcdf>

#include "tc_diag_conf_info.h"

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
static const char* program_name = "tc_diag";

// ATCF file suffix
static const char* atcf_suffix = ".dat";

// Default configuration file name
static const char* default_config_filename =
   "MET_BASE/config/TCDiagConfig_default";

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
static std::map<std::string,StringArray> data_files_map;
static StringArray    deck_source, deck_model_suffix;
static ConcatString   config_file;
static TCDiagConfInfo conf_info;
static GrdFileType    file_type = FileType_None;

// Optional arguments
static ConcatString out_dir;
static ConcatString out_prefix;

////////////////////////////////////////////////////////////////////////
//
// Variables for Output Files
//
////////////////////////////////////////////////////////////////////////

// Output NetCDF file
static ConcatString nc_out_file;
static NcFile*      nc_out = (NcFile*) 0;
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

static Grid input_grid;

// Grid coordinate arrays
static double *lat_arr = (double *) 0;
static double *lon_arr = (double *) 0;

////////////////////////////////////////////////////////////////////////

#endif  //  __TC_DIAG_H__

////////////////////////////////////////////////////////////////////////
