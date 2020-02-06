// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    11/11/08  Halley Gotway  New
//   001    06/03/14  Halley Gotway  Add PHIST line type.
//   002    05/10/16  Halley Gotway  Add grid weighting.
//
////////////////////////////////////////////////////////////////////////

#ifndef  __ENSEMBLE_STAT_H__
#define  __ENSEMBLE_STAT_H__

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

#include "ensemble_stat_conf_info.h"

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

static const char * program_name = "ensemble_stat";

// Default configuration file name
static const char * default_config_filename =
   "MET_BASE/config/EnsembleStatConfig_default";

// Text file abbreviations
static const char *txt_file_abbr[n_txt] = {
   "ecnt",  "rps",   "rhist", "phist",
   "orank", "ssvar", "relp"
};

// Header columns
static const char **txt_columns[n_txt] = {
   ecnt_columns,  rps_columns,   rhist_columns,
   phist_columns, orank_columns, ssvar_columns,
   relp_columns
};

// Length of header columns
static const int n_txt_columns[n_txt] = {
   n_ecnt_columns,  n_rps_columns,   n_rhist_columns,
   n_phist_columns, n_orank_columns, n_ssvar_columns,
   n_relp_columns
};

// Maximum number of GRIB records
static const int max_n_rec = 300;

// Point observation header length
static const int hdr_arr_len = 3;

// Point observation header types lengths
static const int hdr_typ_arr_len = 3;

// Point observation values length
static const int obs_arr_len = 5;

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

// Input Ensemble files
static int          n_ens;         // Number of ensemble members
static IntArray     n_ens_vld;     // Number of members with valid data for each ensemble field [n_ens]
static IntArray     n_vx_vld;      // Number of members with valid data for each verification field [n_vx]

static StringArray  ens_file_list;
static IntArray     ens_file_vld;
static GrdFileType  etype = FileType_None;

static bool         ens_mean_flag; // Flag for ensemble mean processing
static ConcatString ens_mean_user; // User-specified ensemble mean data file
static ConcatString ens_mean_file; // Computed ensemble mean output file

// Input Observation files
static StringArray  grid_obs_file_list;
static int          grid_obs_flag = 0;

static StringArray  point_obs_file_list;
static int          point_obs_flag = 0;

static GrdFileType  otype   = FileType_None;
static int          vx_flag = 0;

// Input Config file
static EnsembleStatConfInfo conf_info;
static ConcatString         config_file;
static ConcatString         out_file;

// Optional arguments
static unixtime     ens_valid_ut        = (unixtime) 0;
static NumArray     ens_lead_na;

static unixtime     obs_valid_beg_ut    = (unixtime) 0;
static unixtime     obs_valid_end_ut    = (unixtime) 0;

static ConcatString out_dir;

static int compress_level = -1;

////////////////////////////////////////////////////////////////////////
//
// Variables for Output Files
//
////////////////////////////////////////////////////////////////////////

// Output NetCDF file
static StringArray out_nc_file_list;
static NcFile      *nc_out  = (NcFile *) 0;
static NcDim       lat_dim;
static NcDim       lon_dim;

// List of output NetCDF variable names
static StringArray nc_ens_var_sa;
static StringArray nc_orank_var_sa;

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
static int nxy = 0;

// Weight for each grid point
static DataPlane wgt_dp;

// Data file factory and input files
static Met2dDataFileFactory mtddf_factory;

// Strings to be output in the STAT and optional text files
static StatHdrColumns shc;

// Arrays to store running sums and counts
static NumArray count_na, min_na, max_na, sum_na, sum_sq_na;
static NumArray *thresh_count_na = (NumArray *) 0; // [n_thresh]
static NumArray **thresh_nbrhd_count_na = (NumArray **) 0; // [n_thresh][n_nbrhd]

////////////////////////////////////////////////////////////////////////

#endif   //  __ENSEMBLE_STAT_H__

////////////////////////////////////////////////////////////////////////
