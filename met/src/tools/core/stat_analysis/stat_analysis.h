// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   stat_analysis.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    11/11/08  Halley Gotway   New
//   001    05/03/12  Halley Gotway   Switch to using vx_config library.
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

#ifndef  __STAT_ANALYSIS_H__
#define  __STAT_ANALYSIS_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cmath>
#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "vx_config.h"
#include "vx_analysis_util.h"
#include "vx_util.h"
#include "vx_statistics.h"
#include "vx_stat_out.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

static const char * program_name = "stat_analysis";

static const char * default_config_filename =
   "MET_BASE/config/STATAnalysisConfig_default";

static const char * go_index_config_file =
   "MET_BASE/config/STATAnalysisConfig_GO_Index";
   
////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

// Search directories and the corresponding files set with -lookin
static StringArray search_dirs;
static StringArray files;

// Output file set with -out
static ConcatString out_file;
static ofstream   *sa_out   = (ofstream *) 0;

// Config file set with -config
static ConcatString config_file;

// Job command which may be set on the command line
static ConcatString command_line_job_options;

////////////////////////////////////////////////////////////////////////
//
// Variables for Output Files
//
////////////////////////////////////////////////////////////////////////

// Default temporary path and file
static ConcatString tmp_dir;
static ConcatString tmp_file;
static ConcatString tmp_path;

// Output file stream for the temporary file
static ofstream tmp_out;

// STAT-Analysis configuration object
static MetConfig conf;

// STAT-Analysis default job
static STATAnalysisJob default_job;

////////////////////////////////////////////////////////////////////////

#endif   //  __STAT_ANALYSIS_H__

////////////////////////////////////////////////////////////////////////
