// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __TC_GEN_H__
#define  __TC_GEN_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

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

#include "tc_gen_conf_info.h"

#include "vx_tc_util.h"
#include "vx_grid.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

// Program name
static const char * program_name = "tc_gen";

// ATCF genesis file name regular expression 
static const char * atcf_gen_reg_exp = "atcf_gen";

// ATCF file name regular expression 
static const char * atcf_reg_exp = ".dat$";

// Default configuration file name
static const char * default_config_filename =
   "MET_BASE/config/TCGenConfig_default";

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

// Input files
static StringArray   genesis_source, genesis_model_suffix;
static StringArray   track_source, track_model_suffix;
static ConcatString  config_file;
static TCGenConfInfo conf_info;

// Optional arguments
static ConcatString out_base;

////////////////////////////////////////////////////////////////////////
//
// Miscellaneous Variables
//
////////////////////////////////////////////////////////////////////////

static StringArray out_files;

////////////////////////////////////////////////////////////////////////

#endif   //  __TC_GEN_H__

////////////////////////////////////////////////////////////////////////
