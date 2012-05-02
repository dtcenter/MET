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

#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

//
// Enumeration for output_flag configuration parameter
//

enum STATOutputType {
   STATOutputType_None, // Do not output this line type
   STATOutputType_Stat, // Write output to the .stat file
   STATOutputType_Both  // Write output to .stat and .txt files
};

////////////////////////////////////////////////////////////////////////

//
// Enumeration for field type configuration parameters
//

enum FieldType {
   FieldType_None, // Default
   FieldType_Fcst, // Apply to forecast field
   FieldType_Obs,  // Apply to observation field
   FieldType_Both  // Apply to both forecast and observation field
};

////////////////////////////////////////////////////////////////////////

//
// Enumeration for all the possible STAT line types
//

enum STATLineType {
   stat_sl1l2,
   stat_sal1l2,
   stat_vl1l2,
   stat_val1l2,
   stat_fho,
   stat_ctc,
   stat_cts,
   stat_mctc,
   stat_mcts,
   stat_cnt,
   stat_pct,
   stat_pstd,
   stat_pjc,
   stat_prc,
   stat_mpr,
   stat_nbrctc,
   stat_nbrcts,
   stat_nbrcnt,
   stat_isc,
   stat_wdir,
   stat_rhist,
   stat_orank,
   no_stat_line_type
};

////////////////////////////////////////////////////////////////////////

//
// Corresponding line type strings
//

static const char stat_sl1l2_str[]  = "SL1L2";
static const char stat_sal1l2_str[] = "SAL1L2";
static const char stat_vl1l2_str[]  = "VL1L2";
static const char stat_val1l2_str[] = "VAL1L2";
static const char stat_fho_str[]    = "FHO";
static const char stat_ctc_str[]    = "CTC";
static const char stat_cts_str[]    = "CTS";
static const char stat_mctc_str[]   = "MCTC";
static const char stat_mcts_str[]   = "MCTS";
static const char stat_cnt_str[]    = "CNT";
static const char stat_pct_str[]    = "PCT";
static const char stat_pstd_str[]   = "PSTD";
static const char stat_pjc_str[]    = "PJC";
static const char stat_prc_str[]    = "PRC";
static const char stat_mpr_str[]    = "MPR";
static const char stat_nbrctc_str[] = "NBRCTC";
static const char stat_nbrcts_str[] = "NBRCTS";
static const char stat_nbrcnt_str[] = "NBRCNT";
static const char stat_isc_str[]    = "ISC";
static const char stat_wdir_str[]   = "WDIR";
static const char stat_rhist_str[]  = "RHIST";
static const char stat_orank_str[]  = "ORANK";
static const char stat_na_str[]     = "NA";

////////////////////////////////////////////////////////////////////////

//
// Enumeration for bootstrapping interval configuration parameter
//

enum BootIntervalType {
   BootIntervalType_None,      // Default 
   BootIntervalType_BCA,       // Bias-Corrected and adjusted method
   BootIntervalType_Percentile // Percentile method
};

//
// Struct to store bootstrapping information
//

struct BootInfo {
   BootIntervalType interval; // Bootstrap interval type
   double           rep_prop; // Proportion of sample for replicates
   int              n_rep;    // Number of replicates
   ConcatString     rng;      // GSL random number generator
   ConcatString     seed;     // RNG seed value
};


////////////////////////////////////////////////////////////////////////

//
// Struct to store interpolation information
//

struct InterpInfo {
   FieldType   field;      // How to apply interpolation options
   double      vld_thresh; // Valid data interpolation threshold
   int         n_interp;   // Number of interpolation types   
   StringArray method;     // Interpolation methods
   IntArray    width;      // Interpolation widths
};

////////////////////////////////////////////////////////////////////////

//
// Struct to store neighborhood information
//

struct NbrhdInfo {
   double      vld_thresh; // Valid data neighborhood threshold
   IntArray    width;      // Neighborhood widths
   ThreshArray cov_ta;     // Fractional coverage thresholds
};

////////////////////////////////////////////////////////////////////////

//
// Struct to store plotting information
//

struct PlotInfo {
   ConcatString color_table; // Color table file
   double       plot_min;    // Minimum plot value
   double       plot_max;    // Maximum plot value
};

////////////////////////////////////////////////////////////////////////

//
// Enumeration for duplicate_flag configuration parameter
//

enum DuplicateType {
   DuplicateType_None,   // Apply no logic for duplicate point obs
   DuplicateType_Unique, // Filter our duplicate observation values
   DuplicateType_Single  // Keep only a single observation per station
};

////////////////////////////////////////////////////////////////////////

//
// Enumeration for grid_decomp_flag configuration parameter
//

enum GridDecompType {
   GridDecompType_None, // Default
   GridDecompType_Auto, // Automatic tiling
   GridDecompType_Tile, // User-specified tile definitions
   GridDecompType_Pad   // Pad out to next largest tile
};

////////////////////////////////////////////////////////////////////////

//
// Enumeration for wavelet.type configuration parameter
//

enum WaveletType {
   WaveletType_None,        // Default
   WaveletType_Haar,        // Haar wavelet
   WaveletType_Haar_Cntr,   // Centered Haar wavelet
   WaveletType_Daub,        // Daubechies wavelet
   WaveletType_Daub_Cntr,   // Centered Daubechies wavelet
   WaveletType_BSpline,     // BSpline wavelet
   WaveletType_BSpline_Cntr // Centered BSpline wavelet
};

////////////////////////////////////////////////////////////////////////
//
// Constants used in configuartion files
//
////////////////////////////////////////////////////////////////////////

static const char config_const_filename[] = "MET_BASE/data/config/ConfigConstants";

//
// Parameter key names common to multiple tools
//

static const char conf_key_version[]           = "version";
static const char conf_key_model[]             = "model";
static const char conf_key_output_flag[]       = "output_flag";
static const char conf_key_fcst[]              = "fcst";
static const char conf_key_obs[]               = "obs";
static const char conf_key_fcst_field[]        = "fcst.field";
static const char conf_key_obs_field[]         = "obs.field";
static const char conf_key_file_type[]         = "file_type";
static const char conf_key_init_time[]         = "init_time";  // YYYYMMDD[_HH[MMSS]]
static const char conf_key_valid_time[]        = "valid_time"; // YYYYMMDD[_HH[MMSS]]
static const char conf_key_lead_time[]         = "lead_time";  // HH[MMSS]
static const char conf_key_name[]              = "name";
static const char conf_key_GRIB1_ptv[]         = "GRIB1_ptv";
static const char conf_key_GRIB1_rec[]         = "GRIB1_rec";
static const char conf_key_GRIB2_disc[]        = "GRIB2_disc";
static const char conf_key_GRIB2_parm_cat[]    = "GRIB2_parm_cat";
static const char conf_key_GRIB2_parm[]        = "GRIB2_parm";
static const char conf_key_level[]             = "level";
static const char conf_key_GRIB_lvl_typ[]      = "GRIB_lvl_typ";
static const char conf_key_GRIB_lvl_val1[]     = "GRIB_lvl_val1";
static const char conf_key_GRIB_lvl_val2[]     = "GRIB_lvl_val2";
static const char conf_key_message_type[]      = "message_type";
static const char conf_key_cat_thresh[]        = "cat_thresh";
static const char conf_key_prob[]              = "prob";
static const char conf_key_thresh_lo[]         = "thresh_lo";
static const char conf_key_thresh_hi[]         = "thresh_hi";
static const char conf_key_fcst_wind_thresh[]  = "fcst.wind_thresh";
static const char conf_key_obs_wind_thresh[]   = "obs.wind_thresh";
static const char conf_key_mask_grid[]         = "mask.grid";
static const char conf_key_mask_poly[]         = "mask.poly";
static const char conf_key_mask_sid[]          = "mask.sid";
static const char conf_key_ci_alpha[]          = "ci_alpha";
static const char conf_key_boot_interval[]     = "boot.interval";
static const char conf_key_boot_rep_prop[]     = "boot.rep_prop";
static const char conf_key_boot_n_rep[]        = "boot.n_rep";
static const char conf_key_boot_rng[]          = "boot.rng";
static const char conf_key_boot_seed[]         = "boot.seed";
static const char conf_key_interp[]            = "interp";
static const char conf_key_field[]             = "field";
static const char conf_key_vld_thresh[]        = "vld_thresh";
static const char conf_key_type[]              = "type";
static const char conf_key_method[]            = "method";
static const char conf_key_width[]             = "width";
static const char conf_key_nbrhd[]             = "nbrhd";
static const char conf_key_cov_thresh[]        = "cov_thresh";
static const char conf_key_nc_pairs_flag[]     = "nc_pairs_flag";
static const char conf_key_duplicate_flag[]    = "duplicate_flag";
static const char conf_key_rank_corr_flag[]    = "rank_corr_flag";
static const char conf_key_tmp_dir[]           = "tmp_dir";
static const char conf_key_output_prefix[]     = "output_prefix";
static const char conf_key_met_data_dir[]      = "met_data_dir";
static const char conf_key_color_table[]       = "color_table";
static const char conf_key_plot_min[]          = "plot_min";
static const char conf_key_plot_max[]          = "plot_max";

//
// Wavelet-Stat specific parameter key names
//

static const char conf_key_mask_missing_flag[] = "mask_missing_flag";
static const char conf_key_grid_decomp_flag[]  = "grid_decomp_flag";
static const char conf_key_tile_width[]        = "tile.width";
static const char conf_key_tile_location[]     = "tile.location";
static const char conf_key_x_ll[]              = "x_ll";
static const char conf_key_y_ll[]              = "y_ll";
static const char conf_key_wavelet_type[]      = "wavelet.type";
static const char conf_key_wavelet_member[]    = "wavelet.member";
static const char conf_key_ps_plot_flag[]      = "ps_plot_flag";
static const char conf_key_fcst_raw_plot[]     = "fcst_raw_plot";
static const char conf_key_obs_raw_plot[]      = "obs_raw_plot";
static const char conf_key_wvlt_plot[]         = "wvlt_plot";

//
// Parameter value names common to multiple tools
//

// File types
static const char conf_val_grib1[]       = "GRIB1";
static const char conf_val_grib2[]       = "GRIB2";
static const char conf_val_netcdf_met[]  = "NETCDF_MET";
static const char conf_val_netcdf_pint[] = "NETCDF_PINT";

// Output flag values: NONE, BOTH, STAT
static const char conf_val_none[] = "NONE";
static const char conf_val_both[] = "BOTH";
static const char conf_val_stat[] = "STAT";

// Field types: NONE, BOTH, FCST, OBS
static const char conf_val_fcst[] = "FCST";
static const char conf_val_obs[]  = "OBS";

// Bootstrapping interval type
static const char conf_val_pctile[] = "PCTILE";
static const char conf_val_bca[]    = "BCA";

// Duplicate flag values
static const char conf_val_unique[] = "UNIQUE";
static const char conf_val_single[] = "SINGLE";

//
// Wavelet-Stat specific parameter value names
//

// Grid decomposition flag values
static const char conf_val_auto[] = "AUTO";
static const char conf_val_tile[] = "TILE";
static const char conf_val_pad[]  = "PAD";

// Supported wavelet types
static const char conf_val_haar[]         = "HAAR";
static const char conf_val_haar_cntr[]    = "HAAR_CNTR";
static const char conf_val_daub[]         = "DAUB";
static const char conf_val_daub_cntr[]    = "DAUB_CNTR";
static const char conf_val_bspline[]      = "BSPLINE";
static const char conf_val_bspline_cntr[] = "BSPLINE_CNTR";

////////////////////////////////////////////////////////////////////////

#endif   //  __CONFIG_CONSTANTS_H__

////////////////////////////////////////////////////////////////////////
