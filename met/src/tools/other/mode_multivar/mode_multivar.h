// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//    Filename:    mode_multivar.h
//
//    Description:
//
//    Mod#  Date      Name      Description
//    ----  ----      ----      -----------
//    000   08/19/19  Fillmore  New
//
////////////////////////////////////////////////////////////////////////

#ifndef  __MODE_MULTIVAR_H__
#define  __MODE_MULTIVAR_H__

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

#include "mode_multivar_conf_info.h"

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
static const char* program_name = "mode_multivar";

// Default configuration file name
static const char* default_config_filename =
    "MET_BASE/config/MODEMultivarConfig_default";

// Default output directory
static const char* default_out_dir = ".";

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

// Input files
static ConcatString         config_file;
static ModeMultivarConfInfo conf_info;

// Optional arguments
static ConcatString out_dir;

#endif  //  __MODE_MULTIVAR_H__

////////////////////////////////////////////////////////////////////////


