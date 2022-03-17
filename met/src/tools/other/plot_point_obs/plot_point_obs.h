// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   plot_point_obs.h
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    11/05/20  Halley Gotway  New
//
////////////////////////////////////////////////////////////////////////

#ifndef  __PLOT_POINT_OBS_H__
#define  __PLOT_POINT_OBS_H__

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

#include "plot_point_obs_conf_info.h"

#include "vx_util.h"
#include "vx_stat_out.h"
#include "vx_gsl_prob.h"
#include "nc_utils.h"
#ifdef WITH_PYTHON
#include "pointdata_python.h"
#endif

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

static const char *program_name = "plot_point_obs";

static const char *DEFAULT_CONFIG_FILENAME =
   "MET_BASE/config/PlotPointObsConfig_default";

static const double line_width = 0.5;

static const int num_colorbar_vals = 300;

static const double one_inch = 72;

static const int num_ticks = 9;

static const bool use_flate = true;

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

static StringArray  nc_file;
static ConcatString ps_file;
static StringArray  ityp;
static IntArray     ivar;
static StringArray  svar;
static StringArray  var_list;
static StringArray  qty_list;

static ConcatString         config_filename;
static PlotPointObsConfInfo conf_info;
static ConcatString         plot_grid_string;
static ConcatString         title_string;

static double dotsize = bad_data_double;

static bool added_colorbar = false;

////////////////////////////////////////////////////////////////////////

#endif   //  __PLOT_POINT_OBS_H__

////////////////////////////////////////////////////////////////////////
