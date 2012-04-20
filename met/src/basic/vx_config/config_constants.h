// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

#ifndef __CONFIG_CONSTANTS_H__
#define __CONFIG_CONSTANTS_H__

////////////////////////////////////////////////////////////////////////

// Enumeration for output_flag configuration parameter
enum StatOutputType {
   StatOutputType_None = 0, // Do not output this line type
   StatOutputType_Stat = 1, // Write output to the .stat file
   StatOutputType_Both = 2  // Write output to .stat and .txt files
};

////////////////////////////////////////////////////////////////////////

// Structure to store all possible output_flag entries
struct OutputFlag {
   StatOutputType fho, ctc, cts;          // Categorical lines
   StatOutputType mctc, mcts;             // Multi-category lines
   StatOutputType cnt, sl1l2, sal1l2;     // Continuous lines
   StatOutputType vl1l2, val1l2;          // Vector lines
   StatOutputType pct, pstd, pjc, prc;    // Probabilistic lines
   StatOutputType mpr;                    // Raw matched pairs
   StatOutputType nbrctc, nbrcts, nbrcnt; // Neighborhood lines
   StatOutputType isc;                    // Intensity-scale line
   StatOutputType rhist, orank;           // Ensemble lines
};

////////////////////////////////////////////////////////////////////////

static const char config_const_filename[] =
   "MET_BASE/data/config/ConfigConstants";

//
// Constants for parameter names used in configuartion files
//

static const char conf_version[]     = "version";
static const char conf_model[]       = "model";
static const char conf_output_flag[] = "output_flag";

//
// Constants for parameter values used in configuartion files
//

// File types
static const char conf_grib1[]       = "GRIB1";
static const char conf_grib2[]       = "GRIB2";
static const char conf_netcdf_met[]  = "NETCDF_MET";
static const char conf_netcdf_pint[] = "NETCDF_PINT";

// Interpolation methods
static const char conf_min[]     = "MIN";
static const char conf_max[]     = "MAX";
static const char conf_median[]  = "MEDIAN";
static const char conf_uw_mean[] = "UW_MEAN";
static const char conf_dw_mean[] = "DW_MEAN";
static const char conf_ls_fit[]  = "LS_FIT";
static const char conf_bilin[]   = "BILIN";

// Interpolation types
static const char conf_fcst[] = "FCST";
static const char conf_obs[]  = "OBS";
static const char conf_both[] = "BOTH";

// Output flag values
static const char conf_none[] = "NONE";
static const char conf_stat[] = "STAT";

// Bootstrapping interval type
static const char conf_pctile[] = "PCTILE";
static const char conf_bca[]    = "BCA";

////////////////////////////////////////////////////////////////////////

#endif   //  __CONFIG_CONSTANTS_H__

////////////////////////////////////////////////////////////////////////
