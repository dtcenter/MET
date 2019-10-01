// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//    Filename:    grid_diag.h
//
//    Description:
//
//    Mod#  Date      Name      Description
//    ----  ----      ----      -----------
//    000   08/19/19  Fillmore  New
//
////////////////////////////////////////////////////////////////////////

#ifndef  __GRID_DIAG_H__
#define  __GRID_DIAG_H__

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

#include "grid_diag_conf_info.h"

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
static const char* program_name = "grid_diag";

// Default configuration file name
static const char* default_config_filename =
    "MET_BASE/config/GridDiagConfig_default";

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
static StringArray      data_files, found_data_files;
static ConcatString     config_file;
static ConcatString     mask_file;
static GridDiagConfInfo conf_info;
static GrdFileType      ftype;

// Optional arguments
static ConcatString out_dir;
static ConcatString out_prefix;

// Data plane, mask plane, and grid
static DataPlane  dp;
static MaskPlane  mp;
static Grid       grid;
static Grid       latlon_arr;

// Data file factory and input files
static Met2dDataFileFactory mtddf_factory;
static Met2dDataFile* data_mtddf = (Met2dDataFile*) 0;

#endif  //  __GRID_DIAG_H__

////////////////////////////////////////////////////////////////////////
