// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   wavelet_stat.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    11/11/08  Halley Gotway   New
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

#ifndef  __WAVELET_STAT_H__
#define  __WAVELET_STAT_H__

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

#include "wavelet_stat_conf_info.h"

#include "vx_wrfdata.h"
#include "vx_met_util.h"
#include "grid.h"
#include "vx_gdata.h"
#include "vx_grib_classes.h"
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

static const char * default_config_filename = "MET_BASE/data/config/WaveletStatConfig_default";

static const char * program_name = "wavelet_stat";

static const char * default_out_dir = "MET_BASE/out/wavelet_stat";

// Text file abbreviations
static const char *isc_file_abbr = "isc";

static const char * grid_decomp_str[] = {
   "Auto", "User-Defined", "Padding"
};

static const char * mask_missing_str[] = {
   "Off", "Fcst", "Obs", "Fcst/Obs"
};

static const char * wavelet_str[] = {
   "Haar",       "Centered-Haar",
   "Daubechies", "Centered-Daubechies",
   "Bspline",    "Centered-Bspline"
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
static ConcatString        config_file;
static WaveletStatConfInfo conf_info;

// Optional arguments
static unixtime     fcst_valid_ut = (unixtime) 0;
static int          fcst_lead_sec = bad_data_int;
static unixtime     obs_valid_ut  = (unixtime) 0;
static int          obs_lead_sec  = bad_data_int;
static ConcatString out_dir;
static ConcatString met_data_dir;
static int          verbosity     = 2;

////////////////////////////////////////////////////////////////////////
//
// Variables for Output Files
//
////////////////////////////////////////////////////////////////////////

// Output file flags
static int nc_flag = 1;
static int ps_flag = 1;

// Output NetCDF file
static ConcatString out_nc_file;
static NcFile       *nc_out    = (NcFile *) 0;
static NcDim        *x_dim     = (NcDim *)  0;
static NcDim        *y_dim     = (NcDim *)  0;
static NcDim        *scale_dim = (NcDim *)  0;
static NcDim        *tile_dim  = (NcDim *)  0;

static NcVar        *fcst_var  = (NcVar *)  0;
static NcVar        *obs_var   = (NcVar *)  0;
static NcVar        *diff_var  = (NcVar *)  0;

// Output PostScript file
static ConcatString out_ps_file;
static PSfile       *ps_out = (PSfile *) 0;

// Output STAT file
static ConcatString stat_file;
static ofstream    *stat_out = (ofstream *)  0;
static AsciiTable   stat_at;
static int          i_stat_row;

// Optional ISC output file
static ConcatString isc_file;
static ofstream    *isc_out;
static AsciiTable   isc_at;
static int          i_isc_row;

////////////////////////////////////////////////////////////////////////
//
// Plotting Info
//
////////////////////////////////////////////////////////////////////////

static ColorTable fcst_ct, obs_ct, wvlt_ct;
static const Color c_map(25, 25, 25);
static const Color c_hull(0, 0, 0);
static const Color c_bndy(0, 0, 255);
static const int n_color_bars = 20;
static const double l_width = 0.50;
static const double l_width_thick = 1.00;

static double raw_plot_min, raw_plot_max;

static int n_page;

static const double page_width = 8.5*72.0;
static const double page_height = 11.0*72.0;

static const double h_margin = 20.0;
static const double v_margin = 20.0;
static const double clrbar_width = h_margin;

static const double plot_text_sep = 15.0;

static BoundingBox  xy_bb;
static BoundingBox  ll_bb;

static const BoundingBox full_pane_bb = {
   h_margin, v_margin,
   page_width-h_margin, page_height-4.0*v_margin,
   page_width-2.0*h_margin, page_height-5.0*v_margin
};

static const double h_tab_cen = page_width/2.0;
static const double v_tab_cen = page_height/2.0;

static double sm_plot_height, lg_plot_height;

static double h_tab_1, h_tab_2, h_tab_3;
static double v_tab_1, v_tab_2, v_tab_3;

static Color c_fcst_fill(150, 150, 150);
static Color c_obs_fill(150, 150, 150);
static Color c_wvlt_fill(150, 150, 150);

////////////////////////////////////////////////////////////////////////
//
// Miscellaneous Variables
//
////////////////////////////////////////////////////////////////////////

// Grid variables
static Grid grid;

// Strings to be output in the STAT and optional text files
static StatHdrColumns shc;

////////////////////////////////////////////////////////////////////////

#endif   //  __WAVELET_STAT_H__

////////////////////////////////////////////////////////////////////////
