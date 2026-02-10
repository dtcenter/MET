// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
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
static netCDF::NcFile *nc_out = nullptr;
netCDF::NcDim mask_dim;
std::vector<netCDF::NcDim> data_var_dims;
int deflate_level;

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

// Input files
static Met2dDataFile *data_mtddf = nullptr;

// Struct to store diagnostic info for each field and masking region
struct DiagInfo {

   // Histogram bins
   std::vector<double> bin_min;
   std::vector<double> bin_max;
   std::vector<double> bin_mid;
   double bin_delta;

   // Input data info
   double var_min;
   double var_max;

   // Single and joint histograms 
   std::vector<long long> hist1d;
   std::map<int, std::vector<long long> > hist2d;

   // Information theory
   double entropy;
   std::map<int, double> joint_entropy;
   std::map<int, double> mutual_information;
};

// DiagInfo objects [n_data][n_mask]
std::vector<std::vector<DiagInfo> > diag_info; 

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
