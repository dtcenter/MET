// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#ifndef  __VX_WRFMODE_ENGINE_H__
#define  __VX_WRFMODE_ENGINE_H__

///////////////////////////////////////////////////////////////////////////////

#include "mode_conf_info.h"

#include "interest.h"
#include "set.h"

#include "vx_shapedata.h"
#include "vx_math.h"
#include "vx_color.h"
#include "vx_grid.h"

#include "crr_array.h"


///////////////////////////////////////////////////////////////////////////////


struct InterestInfo {
   int    fcst_number;
   int    obs_number;
   int    pair_number;
   double interest_value;
};


///////////////////////////////////////////////////////////////////////////////


typedef CRR_Array<Color>         ColorArray;

typedef CRR_Array<InterestInfo>  InterestInfoArray;

typedef CRR_Array<SingleFeature> SingleFeatureArray;

typedef CRR_Array<PairFeature>   PairFeatureArray;


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
static const char aspect_diff_weight_name     [] = "aspect_diff_weight";
static const char area_ratio_weight_name      [] = "area_ratio_weight";
static const char int_area_ratio_weight_name  [] = "int_area_ratio_weight";
static const char curvature_ratio_weight_name [] = "curvature_ratio_weight";
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
static const char aspect_diff_if_name         [] = "aspect_diff_if";
static const char ratio_if_name               [] = "ratio_if";
static const char area_ratio_if_name          [] = "area_ratio_if";
static const char int_area_ratio_if_name      [] = "int_area_ratio_if";
static const char curvature_ratio_if_name     [] = "curvature_ratio_if";
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
static const char raw_color_table_name        [] = "raw_color_table";
static const char mode_color_table_name       [] = "mode_color_table";
static const char ncep_defaults_name          [] = "ncep_defaults";

///////////////////////////////////////////////////////////////////////////////

typedef double (*ConfidenceFunc) (double);

///////////////////////////////////////////////////////////////////////////////


class ModeFuzzyEngine {

   private:

      ModeFuzzyEngine(const ModeFuzzyEngine &);
      ModeFuzzyEngine & operator=(const ModeFuzzyEngine &);

      void init_from_scratch();

   public:

      ModeFuzzyEngine();
     ~ModeFuzzyEngine();

         //
         // Non-const functions
         //

      void clear_features();
      void clear_colors();

      void set_grid(const Grid *);
      void set(const ShapeData &fcst_wd, const ShapeData &obs_wd);

      void set_no_conv(const ShapeData &fcst_wd, const ShapeData &obs_wd);

      void set_fcst (const ShapeData & fcst_wd);
      void set_obs  (const ShapeData &  obs_wd);

      void set_fcst_no_conv (const ShapeData & fcst_wd);
      void set_obs_no_conv  (const ShapeData &  obs_wd);

      int two_to_one(int, int) const;

      void do_fcst_convolution();
      void do_obs_convolution();

      void do_fcst_thresholding();
      void do_obs_thresholding();

      void do_fcst_filtering();
      void do_obs_filtering();

      void do_fcst_splitting();
      void do_obs_splitting();

      void do_fcst_merging();
      void do_obs_merging();

      void do_fcst_merging(const char *default_config,
                           const char *merge_config);
      void do_obs_merging(const char *default_config,
                          const char *merge_config);

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
         // using a Fuzzy ModeFuzzyEngine
         //

      void do_fcst_merge_engine (const char *default_config, const char *merge_config);
      void do_obs_merge_engine  (const char *default_config, const char *merge_config);

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
         // Configuration information
         //

      ModeConfInfo conf_info;

         //
         // Data
         //

      bool need_fcst_conv;
      bool need_obs_conv;

      bool need_fcst_thresh;
      bool need_obs_thresh;

      bool need_fcst_filter;
      bool need_obs_filter;

      bool need_fcst_split;
      bool need_obs_split;

      bool need_fcst_merge;
      bool need_obs_merge;

      bool need_match;

      bool need_fcst_clus_split;
      bool need_obs_clus_split;

      const Grid * grid;             //  not allocated

      ShapeData * fcst_raw;          //  allocated
      ShapeData * fcst_thresh;       //  allocated
      ShapeData * fcst_conv;         //  allocated
      ShapeData * fcst_mask;         //  allocated
      ShapeData * fcst_split;        //  allocated
      ShapeData * fcst_clus_split;   //  allocated

      ShapeData * obs_raw;           //  allocated
      ShapeData * obs_thresh;        //  allocated
      ShapeData * obs_conv;          //  allocated
      ShapeData * obs_mask;          //  allocated
      ShapeData * obs_split;         //  allocated
      ShapeData * obs_clus_split;    //  allocated

      ModeFuzzyEngine * fcst_engine;   //  allocated
      ModeFuzzyEngine * obs_engine;    //  allocated

      int n_fcst;
      int n_obs;
      int n_clus;

      int n_valid;   //  # of data points where both fcst and obs have valid data

      SingleFeatureArray fcst_single;
      SingleFeatureArray obs_single;

      PairFeatureArray pair_single;

      SingleFeatureArray fcst_cluster;
      SingleFeatureArray obs_cluster;

      PairFeatureArray pair_cluster;

      ColorArray fcst_color;
      ColorArray obs_color;

      ColorTable ctable;

      InterestInfoArray info_singles;
      InterestInfoArray info_clus;

      int get_info_index(int) const;

      int get_matched_fcst(int)   const;
      int get_unmatched_fcst(int) const;

      int get_matched_obs(int)    const;
      int get_unmatched_obs(int)  const;

      SetCollection collection;

};

///////////////////////////////////////////////////////////////////////////////

extern double total_interest     (ModeConfInfo &, const PairFeature &, int, int, bool is_single);
extern double interest_percentile(ModeFuzzyEngine &, const double, const int);

extern void write_engine_stats   (ModeFuzzyEngine &, const Grid &, AsciiTable &);
extern void write_header_row     (ModeFuzzyEngine &, AsciiTable &, const int row);   //  row usually zero
extern void write_header_columns (ModeFuzzyEngine &, const Grid &, AsciiTable &, const int row);
extern void write_fcst_single    (ModeFuzzyEngine &, const int, const Grid &, AsciiTable &, const int);
extern void write_obs_single     (ModeFuzzyEngine &, const int, const Grid &, AsciiTable &, const int);
extern void write_pair           (ModeFuzzyEngine &, const Grid &, const int, const int, AsciiTable &, int &);
extern void write_fcst_cluster   (ModeFuzzyEngine &, const int, const Grid &, AsciiTable &, const int);
extern void write_obs_cluster    (ModeFuzzyEngine &, const int, const Grid &, AsciiTable &, const int);
extern void write_cluster_pair   (ModeFuzzyEngine &, const Grid &, const int, AsciiTable &, const int);

// Setup column justification for MODE AsciiTable objects
extern void justify_mode_cols(AsciiTable &);

extern void calc_fcst_clus_ch_mask(const ModeFuzzyEngine &, ShapeData &);
extern void calc_obs_clus_ch_mask(const ModeFuzzyEngine &, ShapeData &);

extern void calc_fcst_cluster_mask(const ModeFuzzyEngine &, ShapeData &, const int);
extern void calc_obs_cluster_mask(const ModeFuzzyEngine &, ShapeData &, const int);

extern void parse_thresh_info(const char *, SingleThresh &);

extern double aspect_ratio_conf(double);


///////////////////////////////////////////////////////////////////////////////

#endif   // __VX_WRFMODE_ENGINE_H__

///////////////////////////////////////////////////////////////////////////////
