// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __MTD_CONFIG_H__
#define  __MTD_CONFIG_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_config.h"
#include "vx_data2d.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////


struct MtdNcOutInfo {

   bool do_latlon;

   bool do_raw;

   // bool do_object_raw;

   bool do_object_id;

   bool do_cluster_id;

   // bool do_polylines;

      ///////////////

   MtdNcOutInfo();

   void clear();   //  sets everything to "true"

   bool all_false() const;

   void set_all_false();
   void set_all_true();

   void dump(ostream &) const;

};


////////////////////////////////////////////////////////////////////////


class MtdConfigInfo {

   private:

      void init_from_scratch();

   public:

      MtdConfigInfo();
     ~MtdConfigInfo();

      void clear();

      void read_config    (const char * default_filename, const char * user_filename);

      void process_config (GrdFileType ftype, GrdFileType otype);

      void parse_nc_info  ();
      void parse_txt_info  ();

         // Store data parsed from the MODE configuration object

      MetConfig conf;                          // MODE configuration object

      ConcatString     model;                  // Model name
      ConcatString     desc;                   // Description
      ConcatString     obtype;                 // Observation type

      VarInfo *        fcst_info;              // allocated
      VarInfo *        obs_info;               // allocated

      int              fcst_conv_radius;       // Convolution radius in grid squares
      int              obs_conv_radius;

      SingleThresh     fcst_conv_thresh;       // Convolution threshold to define objects
      SingleThresh     obs_conv_thresh;

      double           fcst_vld_thresh;        // Minimum ratio of valid data points in the convolution area
      double           obs_vld_thresh;

      SingleThresh     fcst_area_thresh;       // Discard objects whose area doesn't meet threshold
      SingleThresh     obs_area_thresh;

      int              fcst_inten_perc_value;  // Intensity percentile of interest
      int              obs_inten_perc_value;

      SingleThresh     fcst_inten_perc_thresh; // Discard objects whose percentile intensity doesn't meet threshold
      SingleThresh     obs_inten_perc_thresh;

      SingleThresh     fcst_merge_thresh;      // Lower convolution threshold used for double merging method
      SingleThresh     obs_merge_thresh;

      MergeType        fcst_merge_flag;        // Define which merging methods should be employed
      MergeType        obs_merge_flag;

      FieldType        mask_missing_flag;      // Mask missing data between fcst and obs

      MatchType        match_flag;             // Define which matching methods should be employed

      double           max_centroid_dist;      // Only compare objects whose centroids are close enough (in grid squares)

      ConcatString     mask_grid_name;         // Path for masking grid area
      FieldType        mask_grid_flag;         // Define which fields should be masked out

      ConcatString     mask_poly_name;         // Path for masking poly area
      FieldType        mask_poly_flag;         // Define which fields should be masked out

         //  Weights used as input to the fuzzy engine

      double           space_centroid_dist_wt;
      double           time_centroid_delta_wt;
      double           speed_delta_wt;
      double           direction_diff_wt;
      double           volume_ratio_wt;
      double           axis_angle_diff_wt;
      double           start_time_delta_wt;
      double           end_time_delta_wt;

         // Interest functions used as input to the fuzzy engine
         //   not allocated

      PiecewiseLinear * space_centroid_dist_if;
      PiecewiseLinear * time_centroid_delta_if;
      PiecewiseLinear * speed_delta_if;
      PiecewiseLinear * direction_diff_if;
      PiecewiseLinear * volume_ratio_if;
      PiecewiseLinear * axis_angle_diff_if;
      PiecewiseLinear * start_time_delta_if;
      PiecewiseLinear * end_time_delta_if;


      double           total_interest_thresh;  // Total interest threshold

      double           print_interest_thresh;  // Only write output for pairs with >= this interest

      int              zero_border_size;       // Zero out edge rows and columns for object definition

      // bool             nc_pairs_flag;          // output NetCDF file
      MtdNcOutInfo      nc_info;
      bool             ct_stats_flag;          // Flag for the output contingency table statistics file

      bool             do_2d_att_ascii;
      bool             do_3d_att_ascii;

      ConcatString     output_prefix;          // String to customize output file name
      ConcatString     version;                // Config file version

      int              min_volume;             //  throw away 3D objects with volumes smaller than this

         //
         //  delta_t_seconds:  this is not from the config file
         //

      int              delta_t_seconds;

         //
         //  write the header columns to a table
         //

      void write_header_cols(AsciiTable &, const int row) const;

         //
         //  calculate total interest
         //

};

////////////////////////////////////////////////////////////////////////

#endif  /*  __MTD_CONFIG_H__  */

////////////////////////////////////////////////////////////////////////
