// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//    Filename:    rmw_analysis.h
//
//    Description:
//
//    Mod#  Date      Name      Description
//    ----  ----      ----      -----------
//    000   08/19/19  Fillmore  New
//
////////////////////////////////////////////////////////////////////////

#ifndef  __RMW_ANALYSIS_H__
#define  __RMW_ANALYSIS_H__

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

#include "rmw_analysis_conf_info.h"

#include "vx_util.h"
#include "vx_tc_util.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

// Program name
static const char* program_name = "rmw_analysis";

// ATCF file suffix
static const char* atcf_suffix = ".dat";

// Default configuration file name
static const char* default_config_filename =
    "MET_BASE/config/RMWAnalysisConfig_default";

// Default output directory
static const char* default_out_dir = ".";

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

// Input files
static StringArray         data_files;
static ConcatString        config_file;
static RMWAnalysisConfInfo conf_info;

// Optional arguments
static ConcatString out_dir;

// Output file
ConcatString out_file;

// NetCDF file information
static NcFile* nc_in = (NcFile*) 0;
static NcFile* nc_out = (NcFile*) 0;

// Grid dimension information
static NcDim range_dim;
static NcDim azimuth_dim;
static NcDim level_dim;
static NcDim track_point_dim;

static string range_name;
static string range_units;
static string azimuth_name;
static string azimuth_units;
static string level_name;
static string level_units;

static NcVar valid_time_var;

// Grid data
static int n_range;
static int n_azimuth;
static int n_level;
static int n_track_point;
static int n_track_line;

vector<double> range_coord;
vector<double> azimuth_coord;
vector<double> level_coord;

vector<unixtime> track_valid_time;
vector<double> track_lat;
vector<double> track_lon;
vector<double> track_rmw;

// Variable information
static vector<string> data_names;
static vector<int>    data_n_dims;
static vector<string> data_long_names;
static vector<string> data_units;

// Variable data
static vector<DataCube*> data_counts;
static vector<DataCube*> data_means;
static vector<DataCube*> data_stdevs;
static vector<DataCube*> data_mins;
static vector<DataCube*> data_maxs;

// Track information
static TrackInfoArray adeck_tracks;
static ConcatString adeck_source = "adeck.tmp";

#endif  //  __RMW_ANALYSIS_H__

////////////////////////////////////////////////////////////////////////
