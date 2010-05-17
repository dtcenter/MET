

////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_ANALYSIS_CONF_H__
#define  __MODE_ANALYSIS_CONF_H__


////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated
   //
   //     Do not edit by hand
   //
   //
   //     Created from config file "MODEAnalysisConfig_default"
   //
   //     on May 17, 2010    9:19 am  MDT
   //


////////////////////////////////////////////////////////////////////////


#include "vx_econfig/machine.h"
#include "vx_econfig/result.h"


////////////////////////////////////////////////////////////////////////


class mode_analysis_Conf {

   private:

      void init_from_scratch();

      // void assign(const mode_analysis_Conf &);


      mode_analysis_Conf(const mode_analysis_Conf &);
      mode_analysis_Conf & operator=(const mode_analysis_Conf &);

         //
         //  symbol table entries for variables (not allocated)
         //


      const SymbolTableEntry * _fcst_entry;

      const SymbolTableEntry * _obs_entry;

      const SymbolTableEntry * _single_entry;

      const SymbolTableEntry * _pair_entry;

      const SymbolTableEntry * _simple_entry;

      const SymbolTableEntry * _cluster_entry;

      const SymbolTableEntry * _matched_entry;

      const SymbolTableEntry * _unmatched_entry;

      const SymbolTableEntry * _model_entry;

      const SymbolTableEntry * _fcst_thr_entry;

      const SymbolTableEntry * _obs_thr_entry;

      const SymbolTableEntry * _fcst_var_entry;

      const SymbolTableEntry * _fcst_lev_entry;

      const SymbolTableEntry * _obs_var_entry;

      const SymbolTableEntry * _obs_lev_entry;

      const SymbolTableEntry * _fcst_lead_entry;

      const SymbolTableEntry * _fcst_init_hour_entry;

      const SymbolTableEntry * _fcst_accum_entry;

      const SymbolTableEntry * _obs_lead_entry;

      const SymbolTableEntry * _obs_init_hour_entry;

      const SymbolTableEntry * _obs_accum_entry;

      const SymbolTableEntry * _fcst_rad_entry;

      const SymbolTableEntry * _obs_rad_entry;

      const SymbolTableEntry * _area_min_entry;

      const SymbolTableEntry * _area_max_entry;

      const SymbolTableEntry * _area_filter_min_entry;

      const SymbolTableEntry * _area_filter_max_entry;

      const SymbolTableEntry * _area_thresh_min_entry;

      const SymbolTableEntry * _area_thresh_max_entry;

      const SymbolTableEntry * _intersection_area_min_entry;

      const SymbolTableEntry * _intersection_area_max_entry;

      const SymbolTableEntry * _union_area_min_entry;

      const SymbolTableEntry * _union_area_max_entry;

      const SymbolTableEntry * _symmetric_diff_min_entry;

      const SymbolTableEntry * _symmetric_diff_max_entry;

      const SymbolTableEntry * _fcst_valid_min_entry;

      const SymbolTableEntry * _fcst_valid_max_entry;

      const SymbolTableEntry * _obs_valid_min_entry;

      const SymbolTableEntry * _obs_valid_max_entry;

      const SymbolTableEntry * _fcst_init_min_entry;

      const SymbolTableEntry * _fcst_init_max_entry;

      const SymbolTableEntry * _obs_init_min_entry;

      const SymbolTableEntry * _obs_init_max_entry;

      const SymbolTableEntry * _centroid_x_min_entry;

      const SymbolTableEntry * _centroid_x_max_entry;

      const SymbolTableEntry * _centroid_y_min_entry;

      const SymbolTableEntry * _centroid_y_max_entry;

      const SymbolTableEntry * _centroid_lat_min_entry;

      const SymbolTableEntry * _centroid_lat_max_entry;

      const SymbolTableEntry * _centroid_lon_min_entry;

      const SymbolTableEntry * _centroid_lon_max_entry;

      const SymbolTableEntry * _axis_ang_min_entry;

      const SymbolTableEntry * _axis_ang_max_entry;

      const SymbolTableEntry * _length_min_entry;

      const SymbolTableEntry * _length_max_entry;

      const SymbolTableEntry * _width_min_entry;

      const SymbolTableEntry * _width_max_entry;

      const SymbolTableEntry * _aspect_ratio_min_entry;

      const SymbolTableEntry * _aspect_ratio_max_entry;

      const SymbolTableEntry * _curvature_min_entry;

      const SymbolTableEntry * _curvature_max_entry;

      const SymbolTableEntry * _curvature_x_min_entry;

      const SymbolTableEntry * _curvature_x_max_entry;

      const SymbolTableEntry * _curvature_y_min_entry;

      const SymbolTableEntry * _curvature_y_max_entry;

      const SymbolTableEntry * _complexity_min_entry;

      const SymbolTableEntry * _complexity_max_entry;

      const SymbolTableEntry * _intensity_10_min_entry;

      const SymbolTableEntry * _intensity_10_max_entry;

      const SymbolTableEntry * _intensity_25_min_entry;

      const SymbolTableEntry * _intensity_25_max_entry;

      const SymbolTableEntry * _intensity_50_min_entry;

      const SymbolTableEntry * _intensity_50_max_entry;

      const SymbolTableEntry * _intensity_75_min_entry;

      const SymbolTableEntry * _intensity_75_max_entry;

      const SymbolTableEntry * _intensity_90_min_entry;

      const SymbolTableEntry * _intensity_90_max_entry;

      const SymbolTableEntry * _intensity_user_min_entry;

      const SymbolTableEntry * _intensity_user_max_entry;

      const SymbolTableEntry * _intensity_sum_min_entry;

      const SymbolTableEntry * _intensity_sum_max_entry;

      const SymbolTableEntry * _centroid_dist_min_entry;

      const SymbolTableEntry * _centroid_dist_max_entry;

      const SymbolTableEntry * _boundary_dist_min_entry;

      const SymbolTableEntry * _boundary_dist_max_entry;

      const SymbolTableEntry * _convex_hull_dist_min_entry;

      const SymbolTableEntry * _convex_hull_dist_max_entry;

      const SymbolTableEntry * _angle_diff_min_entry;

      const SymbolTableEntry * _angle_diff_max_entry;

      const SymbolTableEntry * _area_ratio_min_entry;

      const SymbolTableEntry * _area_ratio_max_entry;

      const SymbolTableEntry * _intersection_over_area_min_entry;

      const SymbolTableEntry * _intersection_over_area_max_entry;

      const SymbolTableEntry * _complexity_ratio_min_entry;

      const SymbolTableEntry * _complexity_ratio_max_entry;

      const SymbolTableEntry * _percentile_intensity_ratio_min_entry;

      const SymbolTableEntry * _percentile_intensity_ratio_max_entry;

      const SymbolTableEntry * _interest_min_entry;

      const SymbolTableEntry * _interest_max_entry;

      const SymbolTableEntry * _version_entry;



         //
         //  the machine that "runs" the config file
         //


      Machine _m;


   public:

      mode_analysis_Conf();
     ~mode_analysis_Conf();

      void clear();

      void read(const char * config_filename);

         //
         //  Symbol Access
         //

      Result fcst();


      Result obs();


      Result single();


      Result pair();


      Result simple();


      Result cluster();


      Result matched();


      Result unmatched();


      Result model(int);   //  1-dimensional array, indices from 0 to 0

      int n_model_elements();


      Result fcst_thr(int);   //  1-dimensional array, indices from 0 to 0

      int n_fcst_thr_elements();


      Result obs_thr(int);   //  1-dimensional array, indices from 0 to 0

      int n_obs_thr_elements();


      Result fcst_var(int);   //  1-dimensional array, indices from 0 to 0

      int n_fcst_var_elements();


      Result fcst_lev(int);   //  1-dimensional array, indices from 0 to 0

      int n_fcst_lev_elements();


      Result obs_var(int);   //  1-dimensional array, indices from 0 to 0

      int n_obs_var_elements();


      Result obs_lev(int);   //  1-dimensional array, indices from 0 to 0

      int n_obs_lev_elements();


      Result fcst_lead(int);   //  1-dimensional array, indices from 0 to 0

      int n_fcst_lead_elements();


      Result fcst_init_hour(int);   //  1-dimensional array, indices from 0 to 0

      int n_fcst_init_hour_elements();


      Result fcst_accum(int);   //  1-dimensional array, indices from 0 to 0

      int n_fcst_accum_elements();


      Result obs_lead(int);   //  1-dimensional array, indices from 0 to 0

      int n_obs_lead_elements();


      Result obs_init_hour(int);   //  1-dimensional array, indices from 0 to 0

      int n_obs_init_hour_elements();


      Result obs_accum(int);   //  1-dimensional array, indices from 0 to 0

      int n_obs_accum_elements();


      Result fcst_rad(int);   //  1-dimensional array, indices from 0 to 0

      int n_fcst_rad_elements();


      Result obs_rad(int);   //  1-dimensional array, indices from 0 to 0

      int n_obs_rad_elements();


      Result area_min();


      Result area_max();


      Result area_filter_min();


      Result area_filter_max();


      Result area_thresh_min();


      Result area_thresh_max();


      Result intersection_area_min();


      Result intersection_area_max();


      Result union_area_min();


      Result union_area_max();


      Result symmetric_diff_min();


      Result symmetric_diff_max();


      Result fcst_valid_min();


      Result fcst_valid_max();


      Result obs_valid_min();


      Result obs_valid_max();


      Result fcst_init_min();


      Result fcst_init_max();


      Result obs_init_min();


      Result obs_init_max();


      Result centroid_x_min();


      Result centroid_x_max();


      Result centroid_y_min();


      Result centroid_y_max();


      Result centroid_lat_min();


      Result centroid_lat_max();


      Result centroid_lon_min();


      Result centroid_lon_max();


      Result axis_ang_min();


      Result axis_ang_max();


      Result length_min();


      Result length_max();


      Result width_min();


      Result width_max();


      Result aspect_ratio_min();


      Result aspect_ratio_max();


      Result curvature_min();


      Result curvature_max();


      Result curvature_x_min();


      Result curvature_x_max();


      Result curvature_y_min();


      Result curvature_y_max();


      Result complexity_min();


      Result complexity_max();


      Result intensity_10_min();


      Result intensity_10_max();


      Result intensity_25_min();


      Result intensity_25_max();


      Result intensity_50_min();


      Result intensity_50_max();


      Result intensity_75_min();


      Result intensity_75_max();


      Result intensity_90_min();


      Result intensity_90_max();


      Result intensity_user_min();


      Result intensity_user_max();


      Result intensity_sum_min();


      Result intensity_sum_max();


      Result centroid_dist_min();


      Result centroid_dist_max();


      Result boundary_dist_min();


      Result boundary_dist_max();


      Result convex_hull_dist_min();


      Result convex_hull_dist_max();


      Result angle_diff_min();


      Result angle_diff_max();


      Result area_ratio_min();


      Result area_ratio_max();


      Result intersection_over_area_min();


      Result intersection_over_area_max();


      Result complexity_ratio_min();


      Result complexity_ratio_max();


      Result percentile_intensity_ratio_min();


      Result percentile_intensity_ratio_max();


      Result interest_min();


      Result interest_max();


      Result version();

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_ANALYSIS_CONF_H__  */


////////////////////////////////////////////////////////////////////////


