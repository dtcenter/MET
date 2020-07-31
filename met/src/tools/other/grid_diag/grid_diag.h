// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
//
////////////////////////////////////////////////////////////////////////

#ifndef  __GRID_DIAG_H__
#define  __GRID_DIAG_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

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
using namespace netCDF;

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
static vector <StringArray> data_files;
static vector <GrdFileType> file_types;
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
static NcFile *nc_out = (NcFile *) 0;
vector<NcDim> data_var_dims;
vector<NcVar> hist_vars;
vector<NcVar> joint_hist_vars;

////////////////////////////////////////////////////////////////////////
//
// Miscellaneous Variables
//
////////////////////////////////////////////////////////////////////////

// Grid variables
static Grid grid;

// Data file factory and input files
static Met2dDataFileFactory mtddf_factory;
static Met2dDataFile *data_mtddf = (Met2dDataFile *) 0;

// Variable min/max values
vector<double> var_mins;
vector<double> var_maxs;

// Variable histogram map
map<ConcatString, vector<int> > histograms;
map<ConcatString, vector<int> > joint_histograms;
map<ConcatString, vector<double> > bin_mins;
map<ConcatString, vector<double> > bin_maxs;
map<ConcatString, vector<double> > bin_mids;
map<ConcatString, double> bin_deltas;

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
