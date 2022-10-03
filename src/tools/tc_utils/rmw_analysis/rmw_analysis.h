// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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
//    Mod#  Date      Name       Description
//    ----  ----      ----       -----------
//    000   08/19/19  Fillmore   New
//    001   09/28/22  Prestopnik MET #2227 Remove namespace std and netCDF from header files
//
////////////////////////////////////////////////////////////////////////

#ifndef  __RMW_ANALYSIS_H__
#define  __RMW_ANALYSIS_H__

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
static netCDF::NcFile* nc_in = (netCDF::NcFile*) 0;
static netCDF::NcFile* nc_out = (netCDF::NcFile*) 0;

// Grid dimension information
static netCDF::NcDim range_dim;
static netCDF::NcDim azimuth_dim;
static netCDF::NcDim level_dim;
static netCDF::NcDim track_point_dim;

static std::string range_name;
static std::string range_units;
static std::string azimuth_name;
static std::string azimuth_units;
static std::string level_name;
static std::string level_units;

static netCDF::NcVar valid_time_var;

// Grid data
static int n_range;
static int n_azimuth;
static int n_level;
static int n_track_point;
static int n_track_line;

std::vector<double> range_coord;
std::vector<double> azimuth_coord;
std::vector<double> level_coord;

std::vector<unixtime> track_valid_time;
std::vector<double> track_lat;
std::vector<double> track_lon;
std::vector<double> track_rmw;

// Variable information
static std::vector<std::string> data_names;
static std::vector<int>    data_n_dims;
static std::vector<std::string> data_long_names;
static std::vector<std::string> data_units;

// Variable data
static std::vector<DataCube*> data_counts;
static std::vector<DataCube*> data_means;
static std::vector<DataCube*> data_stdevs;
static std::vector<DataCube*> data_mins;
static std::vector<DataCube*> data_maxs;

// Track information
static TrackInfoArray adeck_tracks;
static ConcatString adeck_source = "adeck.tmp";

#endif  //  __RMW_ANALYSIS_H__

////////////////////////////////////////////////////////////////////////
