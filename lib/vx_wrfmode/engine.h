// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#ifndef  __VX_WRFMODE_ENGINE_H__
#define  __VX_WRFMODE_ENGINE_H__

///////////////////////////////////////////////////////////////////////////////

#include "vx_wrfmode/interest.h"
#include "vx_wrfmode/set.h"
#include "vx_wrfmode/WrfMode_Conf.h"

#include "vx_wrfdata/vx_wrfdata.h"
#include "vx_math/vx_math.h"
#include "vx_color/vx_color.h"
#include "vx_data_grids/grid.h"
#include "vx_met_util/vx_met_util.h"

///////////////////////////////////////////////////////////////////////////////
//
// Constants used to format the output text files
//
///////////////////////////////////////////////////////////////////////////////

static const char hms_fmt     [] = "%.2i%.2i%.2i";
static const char ymd_hms_fmt [] = "%.4i%.2i%.2i_%.2i%.2i%.2i";

///////////////////////////////////////////////////////////////////////////////

//
// R, G, B values to be used for unmatched objects in postscript output
//
static const Color unmatched_color(0, 0, 255);

//
// Standard names for raw field parameters
//
static const char grid_res_name               [] = "grid_res";
static const char fcst_grib_code_name         [] = "fcst_grib_code";
static const char obs_grib_code_name          [] = "obs_grib_code";
static const char mask_missing_flag_name      [] = "mask_missing_flag";
static const char mask_grid_name              [] = "mask_grid";
static const char mask_grid_flag_name         [] = "mask_grid_flag";
static const char mask_poly_name              [] = "mask_poly";
static const char mask_poly_flag_name         [] = "mask_poly_flag";

//
// Standard names for object definition parameters
//
static const char fcst_raw_thresh_name        [] = "fcst_raw_thresh";
static const char obs_raw_thresh_name         [] = "obs_raw_thresh";
static const char fcst_conv_radius_name       [] = "fcst_conv_radius";
static const char obs_conv_radius_name        [] = "obs_conv_radius";
static const char bad_data_thresh_name        [] = "bad_data_thresh";
static const char fcst_conv_thresh_name       [] = "fcst_conv_thresh";
static const char obs_conv_thresh_name        [] = "obs_conv_thresh";
static const char fcst_area_thresh_name       [] = "fcst_area_thresh";
static const char obs_area_thresh_name        [] = "obs_area_thresh";
static const char fcst_inten_perc_name        [] = "fcst_inten_perc";
static const char fcst_inten_perc_thresh_name [] = "fcst_inten_perc_thresh";
static const char obs_inten_perc_name         [] = "obs_inten_perc";
static const char obs_inten_perc_thresh_name  [] = "obs_inten_perc_thresh";
static const char fcst_merge_thresh_name      [] = "fcst_merge_thresh";
static const char obs_merge_thresh_name       [] = "obs_merge_thresh";

//
// Standard names for matching/merging flags
//
static const char fcst_merge_flag_name        [] = "fcst_merge_flag";
static const char obs_merge_flag_name         [] = "obs_merge_flag";
static const char match_flag_name             [] = "match_flag";
static const char max_centroid_dist_name      [] = "max_centroid_dist";

//
// Standard names for fuzzy engine weights
//
static const char centroid_dist_weight_name   [] = "centroid_dist_weight";
static const char boundary_dist_weight_name   [] = "boundary_dist_weight";
static const char convex_hull_dist_weight_name[] = "convex_hull_dist_weight";
static const char angle_diff_weight_name      [] = "angle_diff_weight";
static const char area_ratio_weight_name      [] = "area_ratio_weight";
static const char int_area_ratio_weight_name  [] = "int_area_ratio_weight";
static const char complexity_ratio_weight_name[] = "complexity_ratio_weight";
static const char intensity_percentile_name   [] = "intensity_percentile";
static const char intensity_ratio_weight_name [] = "intensity_ratio_weight";

//
// Standard names for interest functions
//
static const char centroid_dist_if_name       [] = "centroid_dist_if";
static const char boundary_dist_if_name       [] = "boundary_dist_if";
static const char convex_hull_dist_if_name    [] = "convex_hull_dist_if";
static const char angle_diff_if_name          [] = "angle_diff_if";
static const char ratio_if_name               [] = "ratio_if";
static const char area_ratio_if_name          [] = "area_ratio_if";
static const char int_area_ratio_if_name      [] = "int_area_ratio_if";
static const char complexity_ratio_if_name    [] = "complexity_ratio_if";
static const char intensity_ratio_if_name     [] = "intensity_ratio_if";

//
// Standard names for confidence functions
//
static const char aspect_ratio_conf_name      [] = "aspect_ratio_conf";
static const char area_ratio_conf_name        [] = "area_ratio_conf";

//
// Standard names for interest thresholds
//
static const char total_interest_thresh_name  [] = "total_interest_thresh";
static const char print_interest_thresh_name  [] = "print_interest_thresh";

//
// Standard names for misc items
//
static const char zero_border_size_name       [] = "zero_border_size";
static const char raw_color_table_name        [] = "raw_color_table";
static const char mode_color_table_name       [] = "mode_color_table";
static const char ncep_defaults_name          [] = "ncep_defaults";

///////////////////////////////////////////////////////////////////////////////

static const int max_singles = 500;

///////////////////////////////////////////////////////////////////////////////

struct InterestInfo {
   int    fcst_number;
   int    obs_number;
   int    pair_number;
   double interest_value;
};

///////////////////////////////////////////////////////////////////////////////

typedef double (*ConfidenceFunc) (double);

///////////////////////////////////////////////////////////////////////////////

class Engine {

   private:

      Engine(const Engine &);
      Engine & operator=(const Engine &);

      void init_from_scratch();

   public:

      Engine();
     ~Engine();

      //
      // Non-const functions
      //

      void clear_features();
      void clear_colors();

      void set(const char *fcst_filename, const char * obs_filename);
      void set(const WrfData &fcst_wd, const WrfData &obs_wd);
      void set_fcst(const char *fcst_filename);
      void set_fcst(const WrfData &fcst_wd);
      void set_obs(const char *obs_filename);
      void set_obs(const WrfData &obs_wd);

      int two_to_one(int, int) const;

      void do_fcst_filter();
      void do_obs_filter();

      void do_fcst_convolution();
      void do_obs_convolution();

      void do_fcst_thresholding();
      void do_obs_thresholding();

      void do_fcst_splitting();
      void do_obs_splitting();

      void do_fcst_merging();
      void do_obs_merging();

      void do_fcst_merging(const char *);
      void do_obs_merging(const char *);

      void do_matching();

      //
      // Perform no matching, but still define the single features
      // for each simple object
      //

      void do_no_match();

      //
      // Perform merging and matching in one step based on the
      // interest threshold
      //

      void do_match_merge();

      //
      // Perform merging of the forecast and observation fields
      // based on a lower convolution threshold
      //

      void do_fcst_merge_thresh();
      void do_obs_merge_thresh();

      //
      // Perform merging of the forecast and observation fields
      // using a Fuzzy Engine
      //

      void do_fcst_merge_engine(const char *);
      void do_obs_merge_engine(const char *);

      //
      // Perform match between the forecast and observation field
      // allowing no merging in the observation field
      //

      void do_match_fcst_merge();

      //
      // Perform match between the forecast and observation field
      // allowing no merging in either field
      //

      void do_match_only();

      void do_fcst_clus_splitting();
      void do_obs_clus_splitting();

      void do_cluster_features();

      //
      // Object holding default configuration values
      //

      WrfMode_Conf wconf;

      void process_engine_config();

      //
      // SingleThresh variables to store the thesholds
      // specified in the config file
      //
      SingleThresh fcst_raw_thresh;
      SingleThresh obs_raw_thresh;

      SingleThresh fcst_conv_thresh;
      SingleThresh obs_conv_thresh;

      SingleThresh fcst_area_thresh;
      SingleThresh obs_area_thresh;

      SingleThresh fcst_inten_perc_thresh;
      SingleThresh obs_inten_perc_thresh;

      SingleThresh fcst_merge_thresh;
      SingleThresh obs_merge_thresh;

      //
      // Data
      //

      char fcst_var_str[max_str_len];
      char obs_var_str[max_str_len];
      char fcst_lvl_str[max_str_len];
      char obs_lvl_str[max_str_len];
      char fcst_unit_str[max_str_len];
      char obs_unit_str[max_str_len];

      int need_fcst_filter;
      int need_obs_filter;

      int need_fcst_conv;
      int need_obs_conv;

      int need_fcst_thresh;
      int need_obs_thresh;

      int need_fcst_split;
      int need_obs_split;

      int need_fcst_merge;
      int need_obs_merge;

      int need_match;

      int need_fcst_clus_split;
      int need_obs_clus_split;

      WrfData * fcst_raw;
      WrfData * fcst_filter;
      WrfData * fcst_thresh;
      WrfData * fcst_conv;
      WrfData * fcst_mask;
      WrfData * fcst_split;
      WrfData * fcst_clus_split;

      WrfData * obs_raw;
      WrfData * obs_filter;
      WrfData * obs_thresh;
      WrfData * obs_conv;
      WrfData * obs_mask;
      WrfData * obs_split;
      WrfData * obs_clus_split;

      Engine * fcst_engine;
      Engine * obs_engine;

      int n_fcst;
      int n_obs;
      int n_clus;

      SingleFeature * fcst_single;
      SingleFeature * obs_single;
      PairFeature   * pair;

      SingleFeature * fcst_clus;
      SingleFeature * obs_clus;
      PairFeature   * pair_clus;

      Color fcst_color [max_singles];
      Color obs_color  [max_singles];
      ColorTable ctable;

      InterestInfo info[max_singles*max_singles];
      InterestInfo info_clus[max_singles];

      int get_info_index(int) const;

      int get_matched_fcst(int)   const;
      int get_unmatched_fcst(int) const;

      int get_matched_obs(int)    const;
      int get_unmatched_obs(int)  const;

      SetCollection collection;
};

///////////////////////////////////////////////////////////////////////////////

extern double total_interest(WrfMode_Conf &, int, const PairFeature &);
extern double total_interest_print(WrfMode_Conf &, int, const PairFeature &, ostream *);
extern double interest_percentile(Engine &, const double, const int);

extern void write_engine_stats(Engine &, const Grid &, AsciiTable &);
extern void write_header(Engine &, AsciiTable &, const int);
extern void write_header_columns(Engine &, AsciiTable & , const int);
extern void write_fcst_single(Engine &, const int, const Grid &, AsciiTable &, const int);
extern void write_obs_single(Engine &, const int, const Grid &, AsciiTable &, const int);
extern void write_pair(Engine &, const int, const int, AsciiTable &, int &);
extern void write_fcst_cluster(Engine &, const int, const Grid &, AsciiTable &, const int);
extern void write_obs_cluster(Engine &, const int, const Grid &, AsciiTable &, const int);
extern void write_cluster_pair(Engine &, const int, AsciiTable &, const int);

extern void calc_fcst_clus_ch_mask(const Engine &, WrfData &);
extern void calc_obs_clus_ch_mask(const Engine &, WrfData &);

extern void calc_fcst_cluster_mask(const Engine &, WrfData &, const int);
extern void calc_obs_cluster_mask(const Engine &, WrfData &, const int);

extern void parse_thresh_info(const char *, SingleThresh &);

///////////////////////////////////////////////////////////////////////////////

#endif   // __VX_WRFMODE_ENGINE_H__

///////////////////////////////////////////////////////////////////////////////
