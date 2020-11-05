// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

#include "vx_data2d_factory.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_stat_out.h"
#include "vx_gsl_prob.h"
#include "nc_utils.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

static const char  *program_name = "plot_point_obs";

static const Color  c_map(25, 25, 25);
static const double l_width = 0.5;
static const double default_dotsize = 1.0;

static const double margin_size = 36.0;

static const bool use_flate = true;

static int   obs_hid_block[DEF_NC_BUFFER_SIZE];
static int   obs_vid_block[DEF_NC_BUFFER_SIZE];
static float obs_lvl_block[DEF_NC_BUFFER_SIZE];
static float obs_hgt_block[DEF_NC_BUFFER_SIZE];
static float obs_val_block[DEF_NC_BUFFER_SIZE];
static float obs_arr_block[DEF_NC_BUFFER_SIZE][OBS_ARRAY_LEN];

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

static Grid         grid("G004");
static Box          grid_bb;
static StringArray  ityp;
static IntArray     ivar;
static StringArray  svar;
static StringArray  var_list;
static ConcatString data_plane_filename;
static double       dotsize = default_dotsize;

////////////////////////////////////////////////////////////////////////

#endif   //  __PLOT_POINT_OBS_H__

////////////////////////////////////////////////////////////////////////
