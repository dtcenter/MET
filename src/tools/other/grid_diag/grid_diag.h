// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   grid_diag.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    10/01/19  Fillmore        New
//   001    09/29/22  Prestopnik      MET #2227 Remove namespace std and netCDF from header files
//
////////////////////////////////////////////////////////////////////////

#ifndef  __GRID_DIAG_H__
#define  __GRID_DIAG_H__

////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <map>
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

#include "grid_diag_conf_info.h"

#include "vx_data2d_factory.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_stat_out.h"
#include "vx_gsl_prob.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

static const char * program_name = "grid_diag";

// Default configuration file name
static const char * default_config_filename =
    "MET_BASE/config/GridDiagConfig_default";

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

// Input files
static std::vector <StringArray> data_files;
static std::vector <GrdFileType> file_types;
static int compress_level = -1;

// Output file
static ConcatString out_file;

// Input Config file
static ConcatString config_file;
static GridDiagConfInfo conf_info;

////////////////////////////////////////////////////////////////////////
//
// Variables for Output Files
//
////////////////////////////////////////////////////////////////////////

// Output NetCDF file
static netCDF::NcFile *nc_out = (netCDF::NcFile *) nullptr;
std::vector<netCDF::NcDim> data_var_dims;
std::vector<netCDF::NcVar> hist_vars;
std::vector<netCDF::NcVar> joint_hist_vars;

static bool multiple_data_sources = false;
static bool unique_variable_names = true;

// List of output NetCDF variable names
static StringArray nc_var_sa;

////////////////////////////////////////////////////////////////////////
//
// Miscellaneous Variables
//
////////////////////////////////////////////////////////////////////////

// Grid variables
static Grid grid;

// Data file factory and input files
static Met2dDataFileFactory mtddf_factory;
static Met2dDataFile *data_mtddf = (Met2dDataFile *) nullptr;

// Variable min/max values
std::vector<double> var_mins;
std::vector<double> var_maxs;

// Variable histogram map
std::map<ConcatString, std::vector<long long> > histograms;
std::map<ConcatString, std::vector<long long> > joint_histograms;
std::map<ConcatString, std::vector<double> > bin_mins;
std::map<ConcatString, std::vector<double> > bin_maxs;
std::map<ConcatString, std::vector<double> > bin_mids;
std::map<ConcatString, double> bin_deltas;

// Series length
static int n_series = bad_data_int;

// Range of timing values encountered in the data
static unixtime init_beg  = (unixtime) 0;
static unixtime init_end  = (unixtime) 0;
static unixtime valid_beg = (unixtime) 0;
static unixtime valid_end = (unixtime) 0;
static int      lead_beg  = bad_data_int;
static int      lead_end  = bad_data_int;


#endif  //  __GRID_DIAG_H__

////////////////////////////////////////////////////////////////////////
