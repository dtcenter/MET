// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

#include <netcdf>
using namespace netCDF;

#include "wavelet_stat_conf_info.h"

#include "vx_data2d_factory.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_stat_out.h"
#include "vx_gsl_prob.h"
#include "vx_ps.h"
#include "vx_color.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

static const char * program_name = "wavelet_stat";

// Default configuration file name
static const char * default_config_filename =
   "MET_BASE/config/WaveletStatConfig_default";

static const char * default_out_dir = ".";

// Text file abbreviations
static const char *isc_file_abbr = "isc";

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
static ConcatString out_dir;

////////////////////////////////////////////////////////////////////////
//
// Variables for Output Files
//
////////////////////////////////////////////////////////////////////////

// Output NetCDF file
static ConcatString out_nc_file;
static NcFile       *nc_out    = (NcFile *) 0;
static NcDim        x_dim     ;
static NcDim        y_dim     ;
static NcDim        scale_dim ;
static NcDim        tile_dim  ;
static NcVar        fcst_var  ;
static NcVar        obs_var   ;
static NcVar        diff_var  ;

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
static ofstream    *isc_out = (ofstream *) 0;
static AsciiTable   isc_at;
static int          i_isc_row;

////////////////////////////////////////////////////////////////////////
//
// Plotting Info
//
////////////////////////////////////////////////////////////////////////

static ColorTable fcst_ct, obs_ct, wvlt_ct;
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

static Box  xy_bb;

static const Box full_pane_bb(
   h_margin,
   page_width-h_margin,
   v_margin,
   page_height-4.0*v_margin
);

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
static bool is_first_pass = true;

// Data file factory and input files
static Met2dDataFileFactory mtddf_factory;
static Met2dDataFile *fcst_mtddf = (Met2dDataFile *) 0;
static Met2dDataFile *obs_mtddf  = (Met2dDataFile *) 0;

// Strings to be output in the STAT and optional text files
static StatHdrColumns shc;

////////////////////////////////////////////////////////////////////////

#endif   //  __WAVELET_STAT_H__

////////////////////////////////////////////////////////////////////////
