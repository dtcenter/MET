// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   grid_stat.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    11/11/08  Halley Gotway   New
//   001    05/03/10  Halley Gotway   Add fcst/obs/diff variable name
//                    arrays to keep track of NetCDF variables. 
//
////////////////////////////////////////////////////////////////////////

#ifndef  __GRID_STAT_H__
#define  __GRID_STAT_H__

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
#include "vx_grib_classes/grib_classes.h"

#include "grid_stat_conf_info.h"

#include "vx_wrfdata/vx_wrfdata.h"
#include "vx_met_util/vx_met_util.h"
#include "vx_data_grids/grid.h"
#include "vx_util/vx_util.h"
#include "vx_cal/vx_cal.h"
#include "vx_math/vx_math.h"
#include "vx_contable/vx_contable.h"
#include "vx_gsl_prob/vx_gsl_prob.h"
#include "vx_econfig/result.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

static const char * program_name = "grid_stat";

// Text file abbreviations
static const char *txt_file_abbr[n_txt] = {
   "fho",    "ctc",    "cts",
   "mctc",   "mcts",   "cnt",
   "sl1l2",  "vl1l2",  "pct",
   "pstd",   "pjc",    "prc",
   "nbrctc", "nbrcts", "nbrcnt"
};

// Header columns
static const char **txt_columns[n_txt] = {
   fho_columns,    ctc_columns,    cts_columns,
   mctc_columns,   mcts_columns,   cnt_columns,
   sl1l2_columns,  vl1l2_columns,  pct_columns,
   pstd_columns,   pjc_columns,    prc_columns,
   nbrctc_columns, nbrcts_columns, nbrcnt_columns
};

// Length of header columns
static const int n_txt_columns[n_txt] = {
   n_fho_columns,    n_ctc_columns,    n_cts_columns,
   n_mctc_columns,   n_mcts_columns,   n_cnt_columns,
   n_sl1l2_columns,  n_vl1l2_columns,  n_pct_columns,
   n_pstd_columns,   n_pjc_columns,    n_prc_columns,
   n_nbrctc_columns, n_nbrcts_columns, n_nbrcnt_columns
};

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

// Input files
static ConcatString fcst_file;
static ConcatString obs_file;

// Input Config file
static ConcatString     config_file;
static GridStatConfInfo conf_info;

// Optional arguments
static unixtime     fcst_valid_ut = (unixtime) 0;
static int          fcst_lead_sec = bad_data_int;
static unixtime     obs_valid_ut  = (unixtime) 0;
static int          obs_lead_sec  = bad_data_int;
static ConcatString out_dir;
static int          verbosity     = 2;

////////////////////////////////////////////////////////////////////////
//
// Variables for Output Files
//
////////////////////////////////////////////////////////////////////////

// Output NetCDF file
static ConcatString out_nc_file;
static NcFile      *nc_out  = (NcFile *) 0;
static NcDim       *lat_dim = (NcDim *)  0;
static NcDim       *lon_dim = (NcDim *)  0;

// List of output NetCDF variable names
static StringArray fcst_var_sa;
static StringArray obs_var_sa;
static StringArray diff_var_sa;

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

// Pointer to the random number generator to be used
static gsl_rng *rng_ptr = (gsl_rng *) 0;

// Strings to be output in the STAT and optional text files
static StatHdrColumns shc;

////////////////////////////////////////////////////////////////////////

#endif   //  __GRID_STAT_H__

////////////////////////////////////////////////////////////////////////
