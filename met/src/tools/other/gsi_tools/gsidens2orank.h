// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   gsidens2orank.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    07/09/15  Halley Gotway   New
//
////////////////////////////////////////////////////////////////////////

#ifndef  __GSIDENS2ORANK_H__
#define  __GSIDENS2ORANK_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <vector>

#include "vx_util.h"
#include "vx_log.h"

#include "gsi_util.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

static const char *program_name = "gsidens2orank";
static const int   rec_pad_length = 4;

////////////////////////////////////////////////////////////////////////

static const char *conv_extra_columns [] = {
   "N_USE",    // number of ensemble members in which obs was used
   "PREP_USE", // read_prepbufr usage
   "SETUP_QC"  // setup qc
};

static const int n_conv_extra_cols =
   sizeof(conv_extra_columns)/sizeof(*conv_extra_columns);

////////////////////////////////////////////////////////////////////////

static const char * rad_extra_columns [] = {
   "N_USE",     // number of ensemble members in which obs was used
   "CHAN_USE",  // channel used
   "SCAN_POS",  // sensor scan position
   "SAT_ZNTH",  // satellite zenith angle (degrees)
   "SAT_AZMTH", // satellite azimuth angle (degrees)
   "SUN_ZNTH",  // solar zenith angle (degrees)
   "SUN_AZMTH", // solar azimuth angle (degrees)
   "SUN_GLNT",  // sun glint angle (degrees)
   "FRAC_WTR",  // fractional coverage by water
   "FRAC_LND",  // fractional coverage by land
   "FRAC_ICE",  // fractional coverage by ice
   "FRAC_SNW",  // fractional coverage by snow
   "SFC_TWTR",  // surface temperature over water (K)
   "SFC_TLND",  // surface temperature over land (K)
   "SFC_TICE",  // surface temperature over ice (K)
   "SFC_TSNW",  // surface temperature over snow (K)
   "TSOIL",     // soil temperature (K)
   "SOILM",     // soil moisture
   "LAND_TYPE", // surface land type
   "FRAC_VEG",  // vegetation fraction
   "SNW_DPTH",  // snow depth
   "TFND",      // foundation temperature: Tr
   "TWARM",     // diurnal warming: d(Tw) at depth zob
   "TCOOL",     // sub-layer cooling: d(Tc) at depth zob
   "TZFND"      // d(Tz)/d(Tr)
};

static const int n_rad_extra_cols =
   sizeof(rad_extra_columns)/sizeof(*rad_extra_columns);

////////////////////////////////////////////////////////////////////////

static const char *retr_extra_columns [] = {
   "SST_FG",   // SST first guess used for SST retrieval -- replaces SFC_TWTR
   "SST_NCEP", // NCEP SST analysis at t                 -- replaces SFC_TLND
   "SST_PHY",  // Physical SST retrieval                 -- replaces SFC_TICE
   "SST_NAVY", // Navy SST retrieval                     -- replaces SFC_TSNW
   "D_TA",     // d(ta) corresponding to sstph           -- replaces TSOIL
   "D_QA",     // d(qa) corresponding to sstph           -- replaces SOILM
   "DATA_TYPE" // data type                              -- replaces LAND_TYPE
};

static const int n_retr_extra_cols =
   sizeof(retr_extra_columns)/sizeof(*retr_extra_columns);
static const int retr_extra_begin = 12;

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

static bool           swap_endian = false;
static ConcatString   output_filename;
static ConcatString   ens_mean_filename;
static NumArray       channel;
static StringArray    hdr_name;
static StringArray    hdr_value;
static StatHdrColumns shc;
static int            n_ens;
static bool           conv_flag, retr_flag;

// Pointer to the random number generator to be used
static gsl_rng     *rng_ptr          = (gsl_rng *) 0;
static const char  *default_rng_name = "mt19937";
static const char  *default_rng_seed = "";
static ConcatString rng_name;
static ConcatString rng_seed;

////////////////////////////////////////////////////////////////////////

// Store conventional and radiance information
static StringArray      obs_key;
static map<int,StringArray> obs_key_map;
static map<int,int>     obs_index_map;
static vector<ConvData> conv_data;
static vector<RadData>  rad_data;
static PairDataEnsemble ens_pd;

////////////////////////////////////////////////////////////////////////

#endif   /*  __GSIDENS2ORANK_H__  */

////////////////////////////////////////////////////////////////////////
