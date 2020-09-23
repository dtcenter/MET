// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

#include <netcdf>
using namespace netCDF;

#include "point_stat_conf_info.h"

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

static const char * program_name = "point_stat";

// Default configuration file name
static const char * default_config_filename =
   "MET_BASE/config/PointStatConfig_default";

// Observation header length
static const int hdr_arr_len = 3;

// Observation values length
static const int obs_arr_len = 5;

// Header columns
static const char **txt_columns[n_txt] = {
   fho_columns,    ctc_columns,    cts_columns,
   mctc_columns,   mcts_columns,   cnt_columns,
   sl1l2_columns,  sal1l2_columns, vl1l2_columns,
   val1l2_columns, pct_columns,    pstd_columns,
   pjc_columns,    prc_columns,    ecnt_columns,
   rps_columns,    eclv_columns,   mpr_columns,
   vcnt_columns
};

// Length of header columns
static const int n_txt_columns[n_txt] = {
   n_fho_columns,    n_ctc_columns,    n_cts_columns,
   n_mctc_columns,   n_mcts_columns,   n_cnt_columns,
   n_sl1l2_columns,  n_sal1l2_columns, n_vl1l2_columns,
   n_val1l2_columns, n_pct_columns,    n_pstd_columns,
   n_pjc_columns,    n_prc_columns,    n_ecnt_columns,
   n_rps_columns,    n_eclv_columns,   n_mpr_columns,
   n_vcnt_columns
};

// Text file abbreviations
static const char *txt_file_abbr[n_txt] = {
   "fho",    "ctc",    "cts",
   "mctc",   "mcts",   "cnt",
   "sl1l2",  "sal1l2", "vl1l2",
   "val1l2", "pct",    "pstd",
   "pjc",    "prc",    "ecnt",
   "rps",    "eclv",   "mpr",
   "vcnt"
};

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

// Input files
static ConcatString fcst_file;
static StringArray  obs_file;

// Input Config file
static ConcatString      config_file;
static PointStatConfInfo conf_info;

// Optional arguments
static unixtime     obs_valid_beg_ut = (unixtime) 0;
static unixtime     obs_valid_end_ut = (unixtime) 0;
static ConcatString out_dir;

////////////////////////////////////////////////////////////////////////
//
// Variables for Output Files
//
////////////////////////////////////////////////////////////////////////

// Timing information
static unixtime fcst_valid_ut = (unixtime) 0;
static int      fcst_lead_sec = bad_data_int;

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
static bool is_first_pass = true;

// Data file factory and input files
static Met2dDataFileFactory mtddf_factory;
static Met2dDataFile *fcst_mtddf = (Met2dDataFile *) 0;

// Pointer to the random number generator to be used
static gsl_rng *rng_ptr = (gsl_rng *) 0;

// Strings to be output in the STAT and optional text files
static StatHdrColumns shc;

////////////////////////////////////////////////////////////////////////

#endif   //  __POINT_STAT_H__

////////////////////////////////////////////////////////////////////////
