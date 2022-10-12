// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   gen_ens_prod.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    09/10/21  Halley Gotway   MET #1904 Initial version.
//   001    09/29/22  Prestopnik      MET #2227 Remove namespace std and netCDF from header files
//
////////////////////////////////////////////////////////////////////////

#ifndef  __GEN_ENS_PROD_H__
#define  __GEN_ENS_PROD_H__

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

#include <netcdf>

#include "gen_ens_prod_conf_info.h"

#include "vx_data2d_factory.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_stat_out.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

static const char * program_name = "gen_ens_prod";

// Default configuration file name
static const char * default_config_filename =
   "MET_BASE/config/GenEnsProdConfig_default";

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

static StringArray        ens_files;
static IntArray           ens_file_vld;
static GrdFileType        etype = FileType_None;
static int                n_ens_files;
static GenEnsProdConfInfo conf_info;
static ConcatString       config_file;
static ConcatString       out_file;
static ConcatString       ctrl_file;
static unixtime           ens_valid_ut = (unixtime) 0;

////////////////////////////////////////////////////////////////////////
//
// Variables for Output Files
//
////////////////////////////////////////////////////////////////////////

// Output NetCDF file
static netCDF::NcFile *nc_out  = (netCDF::NcFile *) 0;
static netCDF::NcDim  lat_dim;
static netCDF::NcDim  lon_dim;

// List of output NetCDF variable names
static StringArray nc_ens_var_sa;

////////////////////////////////////////////////////////////////////////
//
// Miscellaneous Variables
//
////////////////////////////////////////////////////////////////////////

// Grid variables
static Grid grid;
static int nxy = 0;

// Data file factory and input files
static Met2dDataFileFactory mtddf_factory;

// Arrays to store running sums and counts
static NumArray cnt_na, min_na, max_na, sum_na;
static NumArray stdev_cnt_na, stdev_sum_na, stdev_ssq_na;
static NumArray *thresh_cnt_na = (NumArray *) 0; // [n_thresh]
static NumArray **thresh_nbrhd_cnt_na = (NumArray **) 0; // [n_thresh][n_nbrhd]

////////////////////////////////////////////////////////////////////////

#endif   //  __GEN_ENS_PROD_H__

////////////////////////////////////////////////////////////////////////
