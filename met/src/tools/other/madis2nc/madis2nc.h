// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:  madis2nc.h
//
//   Description:
//      Parse MADIS NetCDF files containing surface point observations
//      and reformat them for use by MET.  Initial release provides
//      support for METAR and RAOB MADIS types.  Support for additional
//      MADIS types should be added.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    07-21-11  Halley Gotway  Adapted from contributed code.
//
////////////////////////////////////////////////////////////////////////

#ifndef  __MADIS2NC_H__
#define  __MADIS2NC_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

////////////////////////////////////////////////////////////////////////

#include "vx_log.h"
#include "mask_poly.h"
#include "vx_grid.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

// Enumeration of possible MADIS observation types
enum MadisType {
  madis_none,
  madis_coop,
  madis_HDW,
  madis_HDW1h,
  madis_hydro,
  madis_POES,
  madis_acars,
  madis_acarsProfiles,
  madis_maritime,
  madis_metar,
  madis_mesonet,
  madis_profiler,
  madis_radiometer,
  madis_raob,
  madis_sao,
  madis_satrad,
  madis_snow
};

// Constants
static const char *program_name = "madis2nc";
static const float fill_value   = -9999.f;
static const int   strl_len     = 16; // Length of "YYYYMMDD_HHMMSS"
static const int   hdr_arr_len  = 3;  // Observation header length
static const int   obs_arr_len  = 5;  // Observation values length

////////////////////////////////////////////////////////////////////////
//
// Strings for MADIS types.
//
////////////////////////////////////////////////////////////////////////

static const char *metar_str         = "METAR";
static const char *raob_str          = "RAOB";
static const char *profiler_str      = "PROFILER";
static const char *maritime_str      = "MARITIME";
static const char *mesonet_str       = "MESONET";
static const char *acarsProfiles_str = "ACARSPROFILES";

////////////////////////////////////////////////////////////////////////
//
// Constants for reading MADIS - common for many MADIS files.
//
////////////////////////////////////////////////////////////////////////

static const char *in_recNum_str        = "recNum";
static const char *in_fillValue_str     = "_FillValue";

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

static ConcatString mdfile;
static ConcatString ncfile;
static MadisType    mtype = madis_none;
static StringArray  qc_dd_sa;
static StringArray  lvl_dim_sa;
static int          rec_beg = 0;
static int          rec_end = 0;
static ConcatString argv_str;
static Grid         grid_mask;
static MaskPoly     poly_mask;

// Counters
static int          i_obs    = 0;
static int          rej_fill = 0;
static int          rej_qc   = 0;
static int          rej_grid = 0;
static int          rej_poly = 0;

////////////////////////////////////////////////////////////////////////
//
// Variables for NetCDF file
//
////////////////////////////////////////////////////////////////////////

// Output NetCDF file
NcFile *f_out = (NcFile *) 0;

// Output NetCDF dimensions
static NcDim *strl_dim    = (NcDim *)  0; // Maximum string length
static NcDim *hdr_arr_dim = (NcDim *)  0; // Header array width
static NcDim *obs_arr_dim = (NcDim *)  0; // Observation array width
static NcDim *hdr_dim     = (NcDim *)  0; // Header array length
static NcDim *obs_dim     = (NcDim *)  0; // Observation array length

// Output NetCDF variables
static NcVar *hdr_typ_var = (NcVar *)  0; // Message type
static NcVar *hdr_sid_var = (NcVar *)  0; // Station ID
static NcVar *hdr_vld_var = (NcVar *)  0; // Valid time
static NcVar *hdr_arr_var = (NcVar *)  0; // Header array
static NcVar *obs_qty_var = (NcVar *)  0; // Quality Flag
static NcVar *obs_arr_var = (NcVar *)  0; // Observation array

////////////////////////////////////////////////////////////////////////

#endif   //  __MADIS2NC_H__

////////////////////////////////////////////////////////////////////////
