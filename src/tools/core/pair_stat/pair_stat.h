// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   pair_stat.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    11/07/24  Halley Gotway   MET #3006 New
//
////////////////////////////////////////////////////////////////////////

#ifndef  __PAIR_STAT_H__
#define  __PAIR_STAT_H__

////////////////////////////////////////////////////////////////////////

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

#include "pair_stat_conf_info.h"

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

static const char * program_name = "pair_stat";

// Default configuration file name
static const char * default_config_filename =
   "MET_BASE/config/PairStatConfig_default";
static const char * ioda_data_config_filename =
   "MET_BASE/config/IODADataConfig_default";

// Header columns
static const char * const * txt_columns[n_txt] = {
   fho_columns,    ctc_columns,       cts_columns,
   mctc_columns,   mcts_columns,
   cnt_columns,    sl1l2_columns,     sal1l2_columns,
   vcnt_columns,   vl1l2_columns,     val1l2_columns,
   pct_columns,    pstd_columns,      pjc_columns,
   prc_columns,    eclv_columns,
   mpr_columns,    seeps_mpr_columns, seeps_columns
};

// Length of header columns
static const int n_txt_columns[n_txt] = {
   n_fho_columns,    n_ctc_columns,       n_cts_columns,
   n_mctc_columns,   n_mcts_columns,
   n_cnt_columns,    n_sl1l2_columns,     n_sal1l2_columns,
   n_vcnt_columns,   n_vl1l2_columns,     n_val1l2_columns,
   n_pct_columns,    n_pstd_columns,      n_pjc_columns,
   n_prc_columns,    n_eclv_columns,
   n_mpr_columns,    n_seeps_mpr_columns, n_seeps_columns
};

// Text file abbreviations
static const char * const txt_file_abbr[n_txt] = {
   "fho",    "ctc",       "cts",
   "mctc",   "mcts",
   "cnt",    "sl1l2",     "sal1l2",
   "vcnt",   "vl1l2",     "val1l2",
   "pct",    "pstd",      "pjc",
   "prc",    "eclv",
   "mpr",    "seeps_mpr", "seeps"
};

///////////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
///////////////////////////////////////////////////////////////////////////////

// Input files
static StringArray pairs_files;
static PairsFormat pairs_format;

// Input Config file
static ConcatString     config_file;
static PairStatConfInfo conf_info;

// Optional arguments
static ConcatString out_base;

///////////////////////////////////////////////////////////////////////////////
//
// Variables for Output Files
//
///////////////////////////////////////////////////////////////////////////////

// Timing information
static unixtime fcst_valid_ut = (unixtime) 0;
static int      fcst_lead_sec = bad_data_int;

// Output STAT file
static ConcatString     stat_file;
static std::ofstream    *stat_out = (std::ofstream *) nullptr;
static AsciiTable       stat_at;
static int              i_stat_row;

// Optional ASCII output files
static ConcatString     txt_file[n_txt];
static std::ofstream    *txt_out[n_txt];
static AsciiTable       txt_at[n_txt];
static int              i_txt_row[n_txt];

///////////////////////////////////////////////////////////////////////////////
//
// Miscellaneous Variables
//
///////////////////////////////////////////////////////////////////////////////

// Data file factory and input files
static Met2dDataFile *fcst_mtddf = nullptr;

// Pointer to the random number generator to be used
static gsl_rng *rng_ptr = nullptr;

// Strings to be output in the STAT and optional text files
static StatHdrColumns shc;

///////////////////////////////////////////////////////////////////////////////

#endif   //  __PAIR_STAT_H__

///////////////////////////////////////////////////////////////////////////////
