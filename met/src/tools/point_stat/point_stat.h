// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   point_stat.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    11/11/08  Halley Gotway   New
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

#ifndef  __POINT_STAT_H__
#define  __POINT_STAT_H__

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

#include "netcdf.hh"

#include "point_stat_conf_info.h"

#include "vx_wrfdata.h"
#include "vx_met_util.h"
#include "vx_grib_classes.h"
#include "vx_gdata.h"
#include "grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_statistics.h"
#include "vx_stat_out.h"
#include "vx_gsl_prob.h"
#include "result.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

static const char * program_name = "point_stat";

// Default configuration file name
static const char * default_config_filename =
   "MET_BASE/data/config/PointStatConfig_default";

// Text file abbreviations
static const char *txt_file_abbr[n_txt] = {
   "fho",    "ctc",    "cts",
   "mctc",   "mcts",   "cnt",
   "sl1l2",  "sal1l2", "vl1l2",
   "val1l2", "pct",    "pstd",
   "pjc",    "prc",    "mpr"
};

// Header columns
static const char **txt_columns[n_txt] = {
   fho_columns,    ctc_columns,    cts_columns,
   mctc_columns,   mcts_columns,   cnt_columns,
   sl1l2_columns,  sal1l2_columns, vl1l2_columns,
   val1l2_columns, pct_columns,    pstd_columns,
   pjc_columns,    prc_columns,    mpr_columns
};

// Length of header columns
static const int n_txt_columns[n_txt] = {
   n_fho_columns,    n_ctc_columns,    n_cts_columns,
   n_mctc_columns,   n_mcts_columns,   n_cnt_columns,
   n_sl1l2_columns,  n_sal1l2_columns, n_vl1l2_columns,
   n_val1l2_columns, n_pct_columns,    n_pstd_columns,
   n_pjc_columns,    n_prc_columns,    n_mpr_columns
};

// Observation header length
static const int hdr_arr_len = 3;

// Observation values length
static const int obs_arr_len = 5;

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

// Input files
static ConcatString fcst_file;
static ConcatString climo_file;
static StringArray  obs_file;

// Input Config file
static ConcatString      config_file;
static PointStatConfInfo conf_info;

// Optional arguments
static unixtime     fcst_valid_ut    = (unixtime) 0;
static int          fcst_lead_sec    = bad_data_int;
static unixtime     obs_valid_beg_ut = (unixtime) 0;
static unixtime     obs_valid_end_ut = (unixtime) 0;
static ConcatString out_dir;
static int          verbosity        = 2;
static int          climo_flag       = 0;

////////////////////////////////////////////////////////////////////////
//
// Variables for Output Files
//
////////////////////////////////////////////////////////////////////////

// Output STAT file
static ConcatString stat_file;
static ofstream    *stat_out = (ofstream *)  0;
static AsciiTable   stat_at;
static int          i_stat_row;

// Optional ASCII output files
static ConcatString txt_file[n_txt];
static ofstream    *txt_out[n_txt];
static AsciiTable   txt_at[n_txt];
static int          i_txt_row[n_txt];

////////////////////////////////////////////////////////////////////////
//
// Miscellaneous Variables
//
////////////////////////////////////////////////////////////////////////

// Grid variables
static Grid grid;

// Array of WrfData objects for storing the forecast and climotological
// fields to be verified.  Sized as fcst_wd[n_vx][n_fcst] and
// climo_wd[n_vx][n_climo]
static WrfData **fcst_wd  = (WrfData **) 0;
static WrfData **climo_wd = (WrfData **) 0;

// Pointer to the random number generator to be used
static gsl_rng *rng_ptr = (gsl_rng *) 0;

// Strings to be output in the STAT and optional text files
static StatHdrColumns shc;

////////////////////////////////////////////////////////////////////////

#endif   //  __POINT_STAT_H__

////////////////////////////////////////////////////////////////////////
