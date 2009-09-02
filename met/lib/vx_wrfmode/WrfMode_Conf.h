

////////////////////////////////////////////////////////////////////////


#ifndef  __WRFMODE_CONF_H__
#define  __WRFMODE_CONF_H__


////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated
   //
   //     Do not edit by hand
   //
   //
   //     Created from config file "WrfModeConfig_default"
   //
   //     on March 26, 2009    10:44 am  MST
   //


////////////////////////////////////////////////////////////////////////


#include "vx_econfig/machine.h"
#include "vx_econfig/result.h"


////////////////////////////////////////////////////////////////////////


class WrfMode_Conf {

   private:

      void init_from_scratch();

      // void assign(const WrfMode_Conf &);


      WrfMode_Conf(const WrfMode_Conf &);
      WrfMode_Conf & operator=(const WrfMode_Conf &);

         //
         //  symbol table entries for variables (not allocated)
         //


      const SymbolTableEntry * _model_entry;

      const SymbolTableEntry * _grid_res_entry;

      const SymbolTableEntry * _fcst_field_entry;

      const SymbolTableEntry * _obs_field_entry;

      const SymbolTableEntry * _mask_missing_flag_entry;

      const SymbolTableEntry * _mask_grid_entry;

      const SymbolTableEntry * _mask_grid_flag_entry;

      const SymbolTableEntry * _mask_poly_entry;

      const SymbolTableEntry * _mask_poly_flag_entry;

      const SymbolTableEntry * _fcst_raw_thresh_entry;

      const SymbolTableEntry * _obs_raw_thresh_entry;

      const SymbolTableEntry * _fcst_conv_radius_entry;

      const SymbolTableEntry * _obs_conv_radius_entry;

      const SymbolTableEntry * _bad_data_thresh_entry;

      const SymbolTableEntry * _fcst_conv_thresh_entry;

      const SymbolTableEntry * _obs_conv_thresh_entry;

      const SymbolTableEntry * _fcst_area_thresh_entry;

      const SymbolTableEntry * _obs_area_thresh_entry;

      const SymbolTableEntry * _fcst_inten_perc_entry;

      const SymbolTableEntry * _fcst_inten_perc_thresh_entry;

      const SymbolTableEntry * _obs_inten_perc_entry;

      const SymbolTableEntry * _obs_inten_perc_thresh_entry;

      const SymbolTableEntry * _fcst_merge_thresh_entry;

      const SymbolTableEntry * _obs_merge_thresh_entry;

      const SymbolTableEntry * _fcst_merge_flag_entry;

      const SymbolTableEntry * _obs_merge_flag_entry;

      const SymbolTableEntry * _match_flag_entry;

      const SymbolTableEntry * _max_centroid_dist_entry;

      const SymbolTableEntry * _centroid_dist_weight_entry;

      const SymbolTableEntry * _boundary_dist_weight_entry;

      const SymbolTableEntry * _convex_hull_dist_weight_entry;

      const SymbolTableEntry * _angle_diff_weight_entry;

      const SymbolTableEntry * _area_ratio_weight_entry;

      const SymbolTableEntry * _int_area_ratio_weight_entry;

      const SymbolTableEntry * _complexity_ratio_weight_entry;

      const SymbolTableEntry * _intensity_percentile_entry;

      const SymbolTableEntry * _intensity_ratio_weight_entry;

      const SymbolTableEntry * _centroid_dist_if_entry;

      const SymbolTableEntry * _boundary_dist_if_entry;

      const SymbolTableEntry * _convex_hull_dist_if_entry;

      const SymbolTableEntry * _angle_diff_if_entry;

      const SymbolTableEntry * _corner_entry;

      const SymbolTableEntry * _ratio_if_entry;

      const SymbolTableEntry * _area_ratio_if_entry;

      const SymbolTableEntry * _int_area_ratio_if_entry;

      const SymbolTableEntry * _complexity_ratio_if_entry;

      const SymbolTableEntry * _intensity_ratio_if_entry;

      const SymbolTableEntry * _aspect_ratio_conf_entry;

      const SymbolTableEntry * _area_ratio_conf_entry;

      const SymbolTableEntry * _total_interest_thresh_entry;

      const SymbolTableEntry * _print_interest_thresh_entry;

      const SymbolTableEntry * _met_data_dir_entry;

      const SymbolTableEntry * _fcst_raw_color_table_entry;

      const SymbolTableEntry * _obs_raw_color_table_entry;

      const SymbolTableEntry * _fcst_raw_plot_min_entry;

      const SymbolTableEntry * _fcst_raw_plot_max_entry;

      const SymbolTableEntry * _obs_raw_plot_min_entry;

      const SymbolTableEntry * _obs_raw_plot_max_entry;

      const SymbolTableEntry * _stride_length_entry;

      const SymbolTableEntry * _mode_color_table_entry;

      const SymbolTableEntry * _zero_border_size_entry;

      const SymbolTableEntry * _plot_valid_flag_entry;

      const SymbolTableEntry * _plot_gcarc_flag_entry;

      const SymbolTableEntry * _grib_ptv_entry;

      const SymbolTableEntry * _output_prefix_entry;

      const SymbolTableEntry * _version_entry;



         //
         //  the machine that "runs" the config file
         //


      Machine _m;


   public:

      WrfMode_Conf();
     ~WrfMode_Conf();

      void clear();

      void read(const char * config_filename);

         //
         //  Symbol Access
         //

      Result model();


      Result grid_res();


      Result fcst_field();


      Result obs_field();


      Result mask_missing_flag();


      Result mask_grid();


      Result mask_grid_flag();


      Result mask_poly();


      Result mask_poly_flag();


      Result fcst_raw_thresh();


      Result obs_raw_thresh();


      Result fcst_conv_radius();


      Result obs_conv_radius();


      Result bad_data_thresh();


      Result fcst_conv_thresh();


      Result obs_conv_thresh();


      Result fcst_area_thresh();


      Result obs_area_thresh();


      Result fcst_inten_perc();


      Result fcst_inten_perc_thresh();


      Result obs_inten_perc();


      Result obs_inten_perc_thresh();


      Result fcst_merge_thresh();


      Result obs_merge_thresh();


      Result fcst_merge_flag();


      Result obs_merge_flag();


      Result match_flag();


      Result max_centroid_dist();


      Result centroid_dist_weight();


      Result boundary_dist_weight();


      Result convex_hull_dist_weight();


      Result angle_diff_weight();


      Result area_ratio_weight();


      Result int_area_ratio_weight();


      Result complexity_ratio_weight();


      Result intensity_percentile();


      Result intensity_ratio_weight();


      Result centroid_dist_if(const Result &);   //  pwl function


      Result boundary_dist_if(const Result &);   //  pwl function


      Result convex_hull_dist_if(const Result &);   //  pwl function


      Result angle_diff_if(const Result &);   //  pwl function


      Result corner();


      Result ratio_if(const Result &);   //  pwl function


      Result area_ratio_if(const Result &);   //  function of 1 variable


      Result int_area_ratio_if(const Result &);   //  pwl function


      Result complexity_ratio_if(const Result &);   //  function of 1 variable


      Result intensity_ratio_if(const Result &);   //  function of 1 variable


      Result aspect_ratio_conf(const Result &);   //  function of 1 variable


      Result area_ratio_conf(const Result &);   //  function of 1 variable


      Result total_interest_thresh();


      Result print_interest_thresh();


      Result met_data_dir();


      Result fcst_raw_color_table();


      Result obs_raw_color_table();


      Result fcst_raw_plot_min();


      Result fcst_raw_plot_max();


      Result obs_raw_plot_min();


      Result obs_raw_plot_max();


      Result stride_length();


      Result mode_color_table();


      Result zero_border_size();


      Result plot_valid_flag();


      Result plot_gcarc_flag();


      Result grib_ptv();


      Result output_prefix();


      Result version();

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __WRFMODE_CONF_H__  */


////////////////////////////////////////////////////////////////////////


