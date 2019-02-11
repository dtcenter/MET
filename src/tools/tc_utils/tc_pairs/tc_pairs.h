// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   tc_pairs.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    03/14/12  Halley Gotway   New
//   001    03/09/17  Halley Gotway   Define BEST track time step.
//
////////////////////////////////////////////////////////////////////////

#ifndef  __TC_PAIRS_H__
#define  __TC_PAIRS_H__

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

#include "tc_pairs_conf_info.h"

#include "vx_tc_util.h"
#include "vx_grid.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

// Program name
static const char * program_name = "tc_pairs";

// ATCF file suffix
static const char * atcf_suffix = ".dat";

// Default configuration file name
static const char * default_config_filename =
   "MET_BASE/config/TCPairsConfig_default";

// BEST track time step (6 hours)
static const int best_track_time_step = 21600;

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

// Input files
static StringArray     adeck_source, adeck_model_suffix;
static StringArray     edeck_source, edeck_model_suffix;
static StringArray     bdeck_source, bdeck_model_suffix;
static ConcatString    config_file;
static TCPairsConfInfo conf_info;

// Optional arguments
static ConcatString out_base;

////////////////////////////////////////////////////////////////////////
//
// Miscellaneous Variables
//
////////////////////////////////////////////////////////////////////////

static StringArray  out_files;
static DataPlane    dland_dp;
static Grid         dland_grid;
static ConcatString wwarn_file;

////////////////////////////////////////////////////////////////////////

#endif   //  __TC_PAIRS_H__

////////////////////////////////////////////////////////////////////////
