// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __MODE_CONF_INFO_H__
#define  __MODE_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_config.h"
#include "vx_data2d.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////


struct ModeNcOutInfo {

   bool do_latlon;

   bool do_raw;

   bool do_object_raw;

   bool do_object_id;

   bool do_cluster_id;

   bool do_polylines;

   int compress_level;

   ///////////////

   ModeNcOutInfo();

   void clear();   //  sets everything to "true"

   bool all_false() const;

   void set_all_false();
   void set_all_true();
   void set_compress_level(int compression_level) {compress_level = compression_level;} ;

};


////////////////////////////////////////////////////////////////////////


class ModeConfInfo {

   private:

      void init_from_scratch();

   public:

      ModeConfInfo();
     ~ModeConfInfo();

      void clear();

      void read_config    (const char * default_filename, const char * user_filename);

      void process_config (GrdFileType ftype, GrdFileType otype);

      void set_perc_thresh(const DataPlane &, const DataPlane &);

      void parse_nc_info  ();

      void set_conv_radius_by_index  (int);
      void set_conv_thresh_by_index (int);

      void set_fcst_merge_thresh_by_index (int);
      void set_obs_merge_thresh_by_index  (int);

      int n_conv_threshs  () const;
      int n_conv_radii    () const;

      int n_fcst_merge_threshs () const;
      int n_obs_merge_threshs  () const;

      int n_runs() const;   //  # threshs times # radii

      int get_compression_level();

      // Store data parsed from the MODE configuration object

      MetConfig conf;                          // MODE configuration object

      ConcatString     model;                  // Model name
      ConcatString     desc;                   // Description
      ConcatString     obtype;                 // Observation type

      double           grid_res;

      VarInfo *        fcst_info;              // allocated
      VarInfo *         obs_info;              // allocated

      bool             quilt;                  // default: false

      IntArray         fcst_conv_radius_array; // List of convolution radii in grid squares
      IntArray          obs_conv_radius_array;

      int              fcst_conv_radius;       // Convolution radius in grid squares
      int               obs_conv_radius;

      ThreshArray      fcst_conv_thresh_array; // List of conv thresholds to use
      ThreshArray       obs_conv_thresh_array;

      SingleThresh     fcst_conv_thresh;       // Convolution threshold to define objects
      SingleThresh      obs_conv_thresh;

      double           fcst_vld_thresh;        // Minimum ratio of valid data points in the convolution area
      double            obs_vld_thresh;

      map<ConcatString,ThreshArray> fcst_filter_attr_map; // Discard objects that don't meet these attribute thresholds
      map<ConcatString,ThreshArray>  obs_filter_attr_map;

      ThreshArray      fcst_merge_thresh_array; // Lower convolution threshold used for double merging method
      ThreshArray       obs_merge_thresh_array;

      SingleThresh      fcst_merge_thresh;      // Lower convolution threshold used for double merging method
      SingleThresh       obs_merge_thresh;

      MergeType        fcst_merge_flag;        // Define which merging methods should be employed
      MergeType         obs_merge_flag;

      FieldType        mask_missing_flag;      // Mask missing data between fcst and obs

      MatchType        match_flag;             // Define which matching methods should be employed

      double           max_centroid_dist;      // Only compare objects whose centroids are close enough (in grid squares)

      ConcatString     mask_grid_name;         // Path for masking grid area
      FieldType        mask_grid_flag;         // Define which fields should be masked out

      ConcatString     mask_poly_name;         // Path for masking poly area
      FieldType        mask_poly_flag;         // Define which fields should be masked out

      double           centroid_dist_wt;       // Weights used as input to the fuzzy engine
      double           boundary_dist_wt;
      double           convex_hull_dist_wt;
      double           angle_diff_wt;
      double           aspect_diff_wt;
      double           area_ratio_wt;
      double           int_area_ratio_wt;
      double           curvature_ratio_wt;
      double           complexity_ratio_wt;
      double           inten_perc_ratio_wt;

      int              inten_perc_value;       // Intensity percentile used for the intensity percentile ratio

      PiecewiseLinear * centroid_dist_if;      // Interest functions used as input to the fuzzy engine
      PiecewiseLinear * boundary_dist_if;      // not allocated
      PiecewiseLinear * convex_hull_dist_if;
      PiecewiseLinear * angle_diff_if;
      PiecewiseLinear * aspect_diff_if;
      PiecewiseLinear * area_ratio_if;
      PiecewiseLinear * int_area_ratio_if;
      PiecewiseLinear * curvature_ratio_if;
      PiecewiseLinear * complexity_ratio_if;
      PiecewiseLinear * inten_perc_ratio_if;

      double           total_interest_thresh;  // Total interest threshold defining significance

      double           print_interest_thresh;  // Only write output for pairs with this interest

      ConcatString     met_data_dir;           // MET data directory

      PlotInfo         fcst_raw_pi;            // Raw forecast plotting info
      PlotInfo         obs_raw_pi;             // Raw observation plotting info
      PlotInfo         object_pi;              // Object plotting info

      bool             plot_valid_flag;        // Zoom up plot to the sub-region of valid data
      bool             plot_gcarc_flag;        // Plot lines as great-circle arcs
      bool             ps_plot_flag;           // Flag for the output PostScript image file
      // bool             nc_pairs_flag;          // output NetCDF file
      ModeNcOutInfo    nc_info;
      bool             ct_stats_flag;          // Flag for the output contingency table statistics file

      int              shift_right;            //  shift amount for global grids

      ConcatString     output_prefix;          // String to customize output file name
      ConcatString     version;                // Config file version

      bool             need_fcst_merge_thresh () const;   //  mergetype is both or thresh
      bool             need_obs_merge_thresh  () const;   //  mergetype is both or thresh

};


////////////////////////////////////////////////////////////////////////


inline int ModeConfInfo::n_conv_radii() const { return ( fcst_conv_radius_array.n_elements() ); }   //  should be the same as
                                                                                                    //  obs_conv_radius_array.n_elements()


inline int ModeConfInfo::n_conv_threshs() const { return ( fcst_conv_thresh_array.n_elements() ); }   //  should be the same as
                                                                                                      //  obs_conv_thresh_array.n_elements()


inline int ModeConfInfo::n_fcst_merge_threshs () const { return ( fcst_merge_thresh_array.n_elements() ); }
inline int ModeConfInfo::n_obs_merge_threshs  () const { return (  obs_merge_thresh_array.n_elements() ); }


inline bool ModeConfInfo::need_fcst_merge_thresh () const { return ( (fcst_merge_flag == MergeType_Both) || (fcst_merge_flag == MergeType_Thresh) ); }
inline bool ModeConfInfo::need_obs_merge_thresh  () const { return ( ( obs_merge_flag == MergeType_Both) || ( obs_merge_flag == MergeType_Thresh) ); }

inline int ModeConfInfo::get_compression_level() { return conf.nc_compression(); }

////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_CONF_INFO_H__  */


////////////////////////////////////////////////////////////////////////
