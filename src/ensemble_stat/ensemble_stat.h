// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   ensemble_stat.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    11/11/08  Halley Gotway   New
//
////////////////////////////////////////////////////////////////////////

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

#include "ensemble_stat_conf_info.h"

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

static const char * program_name = "ensemble_stat";

// Text file abbreviations
static const char *txt_file_abbr[n_txt] = {
   "rhist", "orank"
};

// Header columns
static const char **txt_columns[n_txt] = {
   rhist_columns, orank_columns
};

// Length of header columns
static const int n_txt_columns[n_txt] = {
   n_rhist_columns, n_orank_columns
};

// Point observation header length
static const int hdr_arr_len = 3;

// Point observation values length
static const int obs_arr_len = 5;

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

// Input Forecast files
static int          n_fcst;
static int          n_rank;
static StringArray  fcst_file_list;
static GribFile     fcst_gb_file;
static NcFile      *fcst_nc_file = (NcFile *) 0;
static FileType     fcst_ftype   = NoFileType;

// Input Observation files
static StringArray  grid_obs_list;
static int          grid_obs_flag = 0;

static StringArray  point_obs_list;
static int          point_obs_flag = 0;

// Input Config file
static EnsembleStatConfInfo conf_info;
static ConcatString         config_file;
static ConcatString         out_file;

// Optional arguments
static ConcatString out_dir;
static int          verbosity = 2;

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

// Strings to be output in the STAT and optional text files
static StatHdrColumns shc;

// Arrays for storing point observations
static NumArray na_lat, na_lon, na_lvl, na_elv;
static NumArray na_obs, na_obs_rank;

// Arrays to store running sums and counts
static NumArray na_count, na_min, na_max, na_sum, na_sum_sq;
static NumArray *na_thresh_count = (NumArray *) 0;

// JHG, don't need the following:
static gsl_rng *rng_ptr = (gsl_rng *) 0;

////////////////////////////////////////////////////////////////////////

#endif   //  __GRID_STAT_H__

////////////////////////////////////////////////////////////////////////
