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
   ConcatString color_table;      // Color table file
   double       plot_min;         // Minimum plot value
   double       plot_max;         // Maximum plot value
   int          colorbar_spacing; // Spacing of colors in the colorbar
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
// Enumeration for MODE merging options
//

enum MergeType {
   MergeType_None,   // No additional merging
   MergeType_Both,   // Double-threshold and fuzzy engine
   MergeType_Thresh, // Double-threshold only
   MergeType_Engine  // Fuzzy engine only
};

////////////////////////////////////////////////////////////////////////

//
// Enumeration for MODE matching options
//

enum MatchType {
   MatchType_None,      // No matching
   MatchType_MergeBoth, // Match with merging in both fcst and obs
   MatchType_MergeFcst, // Match with merging in fcst only
   MatchType_NoMerge    // Match with no additional merging
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
static const char conf_key_obs_window[]        = "obs_window";
static const char conf_key_beg[]               = "beg";
static const char conf_key_end[]               = "end";
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
static const char conf_key_ps_plot_flag[]      = "ps_plot_flag";
static const char conf_key_nc_pairs_flag[]     = "nc_pairs_flag";
static const char conf_key_duplicate_flag[]    = "duplicate_flag";
static const char conf_key_rank_corr_flag[]    = "rank_corr_flag";
static const char conf_key_tmp_dir[]           = "tmp_dir";
static const char conf_key_output_prefix[]     = "output_prefix";
static const char conf_key_met_data_dir[]      = "met_data_dir";
static const char conf_key_fcst_raw_plot[]     = "fcst_raw_plot";
static const char conf_key_obs_raw_plot[]      = "obs_raw_plot";
static const char conf_key_color_table[]       = "color_table";
static const char conf_key_plot_min[]          = "plot_min";
static const char conf_key_plot_max[]          = "plot_max";
static const char conf_key_colorbar_spacing[]  = "colorbar_spacing";

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
static const char conf_key_wvlt_plot[]         = "wvlt_plot";

//
// Ensemble-Stat specific parameter key names
//

static const char conf_key_ens[]                     = "ens";
static const char conf_key_ens_field[]               = "ens.field";
static const char conf_key_ens_ens_thresh[]          = "ens.ens_thresh";
static const char conf_key_ens_vld_thresh[]          = "ens.vld_thresh";
static const char conf_key_rng_type[]                = "rng.type";
static const char conf_key_rng_seed[]                = "rng.seed";
static const char conf_key_ensemble_flag[]           = "ensemble_flag";
static const char * conf_key_ensemble_flag_entries[] = {
   "mean",  "stdev", "minus",     "plus",      "min",
   "max",   "range", "vld_count", "frequency", "rank"
};
static const int n_conf_key_ensemble_flag_entries =
                    sizeof(conf_key_ensemble_flag_entries)/
                    sizeof(*conf_key_ensemble_flag_entries);

//
// STAT-Analysis specific parameter key names
//

static const char conf_key_fcst_lead[]         = "fcst_lead";
static const char conf_key_obs_lead[]          = "obs_lead";
static const char conf_key_fcst_valid_beg[]    = "fcst_valid_beg";
static const char conf_key_fcst_valid_end[]    = "fcst_valid_end";
static const char conf_key_obs_valid_beg[]     = "obs_valid_beg";
static const char conf_key_obs_valid_end[]     = "obs_valid_end";
static const char conf_key_fcst_init_beg[]     = "fcst_init_beg";
static const char conf_key_fcst_init_end[]     = "fcst_init_end";
static const char conf_key_obs_init_beg[]      = "obs_init_beg";
static const char conf_key_obs_init_end[]      = "obs_init_end";
static const char conf_key_fcst_init_hour[]    = "fcst_init_hour";
static const char conf_key_obs_init_hour[]     = "obs_init_hour";
static const char conf_key_fcst_var[]          = "fcst_var";
static const char conf_key_obs_var[]           = "obs_var";
static const char conf_key_fcst_lev[]          = "fcst_lev";
static const char conf_key_obs_lev[]           = "obs_lev";
static const char conf_key_obtype[]            = "obtype";
static const char conf_key_vx_mask[]           = "vx_mask";
static const char conf_key_interp_mthd[]       = "interp_mthd";
static const char conf_key_interp_pnts[]       = "interp_pnts";
static const char conf_key_fcst_thresh[]       = "fcst_thresh";
static const char conf_key_obs_thresh[]        = "obs_thresh";
static const char conf_key_alpha[]             = "alpha";
static const char conf_key_line_type[]         = "line_type";
static const char conf_key_column[]            = "column";
static const char conf_key_out_alpha[]         = "out_alpha";
static const char conf_key_vif_flag[]          = "vif_flag";
static const char conf_key_jobs[]              = "jobs";

//
// MODE specific parameter key names
//

static const char conf_key_raw_thresh[]            = "raw_thresh";
static const char conf_key_conv_radius[]           = "conv_radius";
static const char conf_key_conv_thresh[]           = "conv_thresh";
static const char conf_key_area_thresh[]           = "area_thresh";
static const char conf_key_inten_perc_value[]      = "inten_perc_value";
static const char conf_key_inten_perc_thresh[]     = "inten_perc_thresh";
static const char conf_key_merge_thresh[]          = "merge_thresh";
static const char conf_key_merge_flag[]            = "merge_flag";
static const char conf_key_match_flag[]            = "match_flag";
static const char conf_key_max_centroid_dist[]     = "max_centroid_dist";
static const char conf_key_weight[]                = "weight";
static const char conf_key_interest_function[]     = "interest_function";
static const char conf_key_centroid_dist[]         = "centroid_dist";
static const char conf_key_boundary_dist[]         = "boundary_dist";
static const char conf_key_convex_hull_dist[]      = "convex_hull_dist";
static const char conf_key_angle_diff[]            = "angle_diff";
static const char conf_key_area_ratio[]            = "area_ratio";
static const char conf_key_int_area_ratio[]        = "int_area_ratio";
static const char conf_key_complexity_ratio[]      = "complexity_ratio";
static const char conf_key_inten_perc_ratio[]      = "inten_perc_ratio";
static const char conf_key_object_plot[]           = "object_plot";
static const char conf_key_intensity_percentile[]  = "intensity_percentile";
static const char conf_key_total_interest_thresh[] = "total_interest_thresh";
static const char conf_key_print_interest_thresh[] = "print_interest_thresh";
static const char conf_key_zero_border_size[]      = "zero_border_size";
static const char conf_key_plot_valid_flag[]       = "plot_valid_flag";
static const char conf_key_plot_gcarc_flag[]       = "plot_gcarc_flag";
static const char conf_key_ct_stats_flag[]         = "ct_stats_flag";

//
// PB2NC specific parameter key names
//

static const char conf_key_station_id[]            = "station_id";
static const char conf_key_elevation_range[]       = "elevation_range";
static const char conf_key_pb_report_type[]        = "pb_report_type";
static const char conf_key_in_report_type[]        = "in_report_type";
static const char conf_key_instrument_type[]       = "instrument_type";
static const char conf_key_level_range[]           = "level_range";
static const char conf_key_level_category[]        = "level_category";
static const char conf_key_obs_grib_code[]         = "obs_grib_code";
static const char conf_key_quality_mark_thresh[]   = "quality_mark_thresh";
static const char conf_key_event_stack_flag[]      = "event_stack_flag";

//
// MODE-Analysis specific parameter key names
//

static const char conf_key_single[]                         = "single";
static const char conf_key_pair[]                           = "pair";
static const char conf_key_simple[]                         = "simple";
static const char conf_key_cluster[]                        = "cluster";
static const char conf_key_matched[]                        = "matched";
static const char conf_key_unmatched[]                      = "unmatched";
static const char conf_key_fcst_thr[]                       = "fcst_thr";
static const char conf_key_obs_thr[]                        = "obs_thr";
static const char conf_key_fcst_accum[]                     = "fcst_accum";
static const char conf_key_obs_accum[]                      = "obs_accum";
static const char conf_key_fcst_rad[]                       = "fcst_rad";
static const char conf_key_obs_rad[]                        = "obs_rad";
static const char conf_key_area_min[]                       = "area_min";
static const char conf_key_area_max[]                       = "area_max";
static const char conf_key_area_filter_min[]                = "area_filter_min";
static const char conf_key_area_filter_max[]                = "area_filter_max";
static const char conf_key_area_thresh_min[]                = "area_thresh_min";
static const char conf_key_area_thresh_max[]                = "area_thresh_max";
static const char conf_key_intersection_area_min[]          = "intersection_area_min";
static const char conf_key_intersection_area_max[]          = "intersection_area_max";
static const char conf_key_union_area_min[]                 = "union_area_min";
static const char conf_key_union_area_max[]                 = "union_area_max";
static const char conf_key_symmetric_diff_min[]             = "symmetric_diff_min";
static const char conf_key_symmetric_diff_max[]             = "symmetric_diff_max";
static const char conf_key_fcst_valid_min[]                 = "fcst_valid_min";
static const char conf_key_fcst_valid_max[]                 = "fcst_valid_max";
static const char conf_key_obs_valid_min[]                  = "obs_valid_min";
static const char conf_key_obs_valid_max[]                  = "obs_valid_max";
static const char conf_key_fcst_init_min[]                  = "fcst_init_min";
static const char conf_key_fcst_init_max[]                  = "fcst_init_max";
static const char conf_key_obs_init_min[]                   = "obs_init_min";
static const char conf_key_obs_init_max[]                   = "obs_init_max";
static const char conf_key_centroid_x_min[]                 = "centroid_x_min";
static const char conf_key_centroid_x_max[]                 = "centroid_x_max";
static const char conf_key_centroid_y_min[]                 = "centroid_y_min";
static const char conf_key_centroid_y_max[]                 = "centroid_y_max";
static const char conf_key_centroid_lat_min[]               = "centroid_lat_min";
static const char conf_key_centroid_lat_max[]               = "centroid_lat_max";
static const char conf_key_centroid_lon_min[]               = "centroid_lon_min";
static const char conf_key_centroid_lon_max[]               = "centroid_lon_max";
static const char conf_key_axis_ang_min[]                   = "axis_ang_min";
static const char conf_key_axis_ang_max[]                   = "axis_ang_max";
static const char conf_key_length_min[]                     = "length_min";
static const char conf_key_length_max[]                     = "length_max";
static const char conf_key_width_min[]                      = "width_min";
static const char conf_key_width_max[]                      = "width_max";
static const char conf_key_aspect_ratio_min[]               = "aspect_ratio_min";
static const char conf_key_aspect_ratio_max[]               = "aspect_ratio_max";
static const char conf_key_curvature_min[]                  = "curvature_min";
static const char conf_key_curvature_max[]                  = "curvature_max";
static const char conf_key_curvature_x_min[]                = "curvature_x_min";
static const char conf_key_curvature_x_max[]                = "curvature_x_max";
static const char conf_key_curvature_y_min[]                = "curvature_y_min";
static const char conf_key_curvature_y_max[]                = "curvature_y_max";
static const char conf_key_complexity_min[]                 = "complexity_min";
static const char conf_key_complexity_max[]                 = "complexity_max";
static const char conf_key_intensity_10_min[]               = "intensity_10_min";
static const char conf_key_intensity_10_max[]               = "intensity_10_max";
static const char conf_key_intensity_25_min[]               = "intensity_25_min";
static const char conf_key_intensity_25_max[]               = "intensity_25_max";
static const char conf_key_intensity_50_min[]               = "intensity_50_min";
static const char conf_key_intensity_50_max[]               = "intensity_50_max";
static const char conf_key_intensity_75_min[]               = "intensity_75_min";
static const char conf_key_intensity_75_max[]               = "intensity_75_max";
static const char conf_key_intensity_90_min[]               = "intensity_90_min";
static const char conf_key_intensity_90_max[]               = "intensity_90_max";
static const char conf_key_intensity_user_min[]             = "intensity_user_min";
static const char conf_key_intensity_user_max[]             = "intensity_user_max";
static const char conf_key_intensity_sum_min[]              = "intensity_sum_min";
static const char conf_key_intensity_sum_max[]              = "intensity_sum_max";
static const char conf_key_centroid_dist_min[]              = "centroid_dist_min";
static const char conf_key_centroid_dist_max[]              = "centroid_dist_max";
static const char conf_key_boundary_dist_min[]              = "boundary_dist_min";
static const char conf_key_boundary_dist_max[]              = "boundary_dist_max";
static const char conf_key_convex_hull_dist_min[]           = "convex_hull_dist_min";
static const char conf_key_convex_hull_dist_max[]           = "convex_hull_dist_max";
static const char conf_key_angle_diff_min[]                 = "angle_diff_min";
static const char conf_key_angle_diff_max[]                 = "angle_diff_max";
static const char conf_key_area_ratio_min[]                 = "area_ratio_min";
static const char conf_key_area_ratio_max[]                 = "area_ratio_max";
static const char conf_key_intersection_over_area_min[]     = "intersection_over_area_min";
static const char conf_key_intersection_over_area_max[]     = "intersection_over_area_max";
static const char conf_key_complexity_ratio_min[]           = "complexity_ratio_min";
static const char conf_key_complexity_ratio_max[]           = "complexity_ratio_max";
static const char conf_key_percentile_intensity_ratio_min[] = "percentile_intensity_ratio_min";
static const char conf_key_percentile_intensity_ratio_max[] = "percentile_intensity_ratio_max";
static const char conf_key_interest_min[]                   = "interest_min";
static const char conf_key_interest_max[]                   = "interest_max";

//
// WWMCA specific parameter key names
//

static const char conf_key_to_grid[]       = "to_grid";
static const char conf_key_max_minutes[]   = "max_minutes";
static const char conf_key_variable_name[] = "variable_name";
static const char conf_key_units[]         = "units";
static const char conf_key_long_name[]     = "long_name";

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

//
// MODE specific parameter value names
//

// Merging options
static const char conf_val_thresh[] = "THRESH";
static const char conf_val_engine[] = "ENGINE";

// Matching options
static const char conf_val_merge_both[] = "MERGE_BOTH";
static const char conf_val_merge_fcst[] = "MERGE_FCST";
static const char conf_val_no_merge[]   = "NO_MERGE";

////////////////////////////////////////////////////////////////////////

#endif   //  __CONFIG_CONSTANTS_H__

////////////////////////////////////////////////////////////////////////
