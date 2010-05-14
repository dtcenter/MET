

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
   //     on May 14, 2010    2:30 pm  MDT
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "WrfMode_Conf.h"
#include "vx_econfig/icodecell_to_result.h"


////////////////////////////////////////////////////////////////////////


static const int Panic = 0;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class WrfMode_Conf
   //


////////////////////////////////////////////////////////////////////////


WrfMode_Conf::WrfMode_Conf()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


WrfMode_Conf::~WrfMode_Conf()

{

clear();

}


////////////////////////////////////////////////////////////////////////


WrfMode_Conf::WrfMode_Conf(const WrfMode_Conf &)

{

cerr << "\n\n  WrfMode_Conf::WrfMode_Conf(const WrfMode_Conf &) -> should never be called!\n\n";

exit ( 1 );

//  init_from_scratch();

//  assign(a);

}


////////////////////////////////////////////////////////////////////////


WrfMode_Conf & WrfMode_Conf::operator=(const WrfMode_Conf &)

{

cerr << "\n\n  WrfMode_Conf::operator=(const WrfMode_Conf &) -> should never be called!\n\n";

exit ( 1 );

// if ( this == &a )  return ( * this );

// assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void WrfMode_Conf::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void WrfMode_Conf::clear()

{

                  _model_entry = (const SymbolTableEntry *) 0;

               _grid_res_entry = (const SymbolTableEntry *) 0;

             _fcst_field_entry = (const SymbolTableEntry *) 0;

              _obs_field_entry = (const SymbolTableEntry *) 0;

      _mask_missing_flag_entry = (const SymbolTableEntry *) 0;

              _mask_grid_entry = (const SymbolTableEntry *) 0;

         _mask_grid_flag_entry = (const SymbolTableEntry *) 0;

              _mask_poly_entry = (const SymbolTableEntry *) 0;

         _mask_poly_flag_entry = (const SymbolTableEntry *) 0;

        _fcst_raw_thresh_entry = (const SymbolTableEntry *) 0;

         _obs_raw_thresh_entry = (const SymbolTableEntry *) 0;

       _fcst_conv_radius_entry = (const SymbolTableEntry *) 0;

        _obs_conv_radius_entry = (const SymbolTableEntry *) 0;

        _bad_data_thresh_entry = (const SymbolTableEntry *) 0;

       _fcst_conv_thresh_entry = (const SymbolTableEntry *) 0;

        _obs_conv_thresh_entry = (const SymbolTableEntry *) 0;

       _fcst_area_thresh_entry = (const SymbolTableEntry *) 0;

        _obs_area_thresh_entry = (const SymbolTableEntry *) 0;

        _fcst_inten_perc_entry = (const SymbolTableEntry *) 0;

 _fcst_inten_perc_thresh_entry = (const SymbolTableEntry *) 0;

         _obs_inten_perc_entry = (const SymbolTableEntry *) 0;

  _obs_inten_perc_thresh_entry = (const SymbolTableEntry *) 0;

      _fcst_merge_thresh_entry = (const SymbolTableEntry *) 0;

       _obs_merge_thresh_entry = (const SymbolTableEntry *) 0;

        _fcst_merge_flag_entry = (const SymbolTableEntry *) 0;

         _obs_merge_flag_entry = (const SymbolTableEntry *) 0;

             _match_flag_entry = (const SymbolTableEntry *) 0;

      _max_centroid_dist_entry = (const SymbolTableEntry *) 0;

   _centroid_dist_weight_entry = (const SymbolTableEntry *) 0;

   _boundary_dist_weight_entry = (const SymbolTableEntry *) 0;

_convex_hull_dist_weight_entry = (const SymbolTableEntry *) 0;

      _angle_diff_weight_entry = (const SymbolTableEntry *) 0;

      _area_ratio_weight_entry = (const SymbolTableEntry *) 0;

  _int_area_ratio_weight_entry = (const SymbolTableEntry *) 0;

_complexity_ratio_weight_entry = (const SymbolTableEntry *) 0;

   _intensity_percentile_entry = (const SymbolTableEntry *) 0;

 _intensity_ratio_weight_entry = (const SymbolTableEntry *) 0;

       _centroid_dist_if_entry = (const SymbolTableEntry *) 0;

       _boundary_dist_if_entry = (const SymbolTableEntry *) 0;

    _convex_hull_dist_if_entry = (const SymbolTableEntry *) 0;

          _angle_diff_if_entry = (const SymbolTableEntry *) 0;

                 _corner_entry = (const SymbolTableEntry *) 0;

               _ratio_if_entry = (const SymbolTableEntry *) 0;

          _area_ratio_if_entry = (const SymbolTableEntry *) 0;

      _int_area_ratio_if_entry = (const SymbolTableEntry *) 0;

    _complexity_ratio_if_entry = (const SymbolTableEntry *) 0;

     _intensity_ratio_if_entry = (const SymbolTableEntry *) 0;

      _aspect_ratio_conf_entry = (const SymbolTableEntry *) 0;

        _area_ratio_conf_entry = (const SymbolTableEntry *) 0;

  _total_interest_thresh_entry = (const SymbolTableEntry *) 0;

  _print_interest_thresh_entry = (const SymbolTableEntry *) 0;

           _met_data_dir_entry = (const SymbolTableEntry *) 0;

   _fcst_raw_color_table_entry = (const SymbolTableEntry *) 0;

    _obs_raw_color_table_entry = (const SymbolTableEntry *) 0;

      _fcst_raw_plot_min_entry = (const SymbolTableEntry *) 0;

      _fcst_raw_plot_max_entry = (const SymbolTableEntry *) 0;

       _obs_raw_plot_min_entry = (const SymbolTableEntry *) 0;

       _obs_raw_plot_max_entry = (const SymbolTableEntry *) 0;

          _stride_length_entry = (const SymbolTableEntry *) 0;

       _mode_color_table_entry = (const SymbolTableEntry *) 0;

       _zero_border_size_entry = (const SymbolTableEntry *) 0;

        _plot_valid_flag_entry = (const SymbolTableEntry *) 0;

        _plot_gcarc_flag_entry = (const SymbolTableEntry *) 0;

               _grib_ptv_entry = (const SymbolTableEntry *) 0;

          _output_prefix_entry = (const SymbolTableEntry *) 0;

                _version_entry = (const SymbolTableEntry *) 0;


_m.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void WrfMode_Conf::read(const char * _config_filename)

{

const SymbolTableEntry * _e = (const SymbolTableEntry *) 0;

   //
   //  read the config file into the machine
   //

_m.read(_config_filename);

   //
   //  lookup the entries in the symbol table
   //

_e = _m.find("model");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"model\"\n\n";

   exit ( 1 );

}

_model_entry = _e;


_e = _m.find("grid_res");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"grid_res\"\n\n";

   exit ( 1 );

}

_grid_res_entry = _e;


_e = _m.find("fcst_field");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_field\"\n\n";

   exit ( 1 );

}

_fcst_field_entry = _e;


_e = _m.find("obs_field");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_field\"\n\n";

   exit ( 1 );

}

_obs_field_entry = _e;


_e = _m.find("mask_missing_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_missing_flag\"\n\n";

   exit ( 1 );

}

_mask_missing_flag_entry = _e;


_e = _m.find("mask_grid");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_grid\"\n\n";

   exit ( 1 );

}

_mask_grid_entry = _e;


_e = _m.find("mask_grid_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_grid_flag\"\n\n";

   exit ( 1 );

}

_mask_grid_flag_entry = _e;


_e = _m.find("mask_poly");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_poly\"\n\n";

   exit ( 1 );

}

_mask_poly_entry = _e;


_e = _m.find("mask_poly_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_poly_flag\"\n\n";

   exit ( 1 );

}

_mask_poly_flag_entry = _e;


_e = _m.find("fcst_raw_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_raw_thresh\"\n\n";

   exit ( 1 );

}

_fcst_raw_thresh_entry = _e;


_e = _m.find("obs_raw_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_raw_thresh\"\n\n";

   exit ( 1 );

}

_obs_raw_thresh_entry = _e;


_e = _m.find("fcst_conv_radius");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_conv_radius\"\n\n";

   exit ( 1 );

}

_fcst_conv_radius_entry = _e;


_e = _m.find("obs_conv_radius");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_conv_radius\"\n\n";

   exit ( 1 );

}

_obs_conv_radius_entry = _e;


_e = _m.find("bad_data_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"bad_data_thresh\"\n\n";

   exit ( 1 );

}

_bad_data_thresh_entry = _e;


_e = _m.find("fcst_conv_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_conv_thresh\"\n\n";

   exit ( 1 );

}

_fcst_conv_thresh_entry = _e;


_e = _m.find("obs_conv_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_conv_thresh\"\n\n";

   exit ( 1 );

}

_obs_conv_thresh_entry = _e;


_e = _m.find("fcst_area_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_area_thresh\"\n\n";

   exit ( 1 );

}

_fcst_area_thresh_entry = _e;


_e = _m.find("obs_area_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_area_thresh\"\n\n";

   exit ( 1 );

}

_obs_area_thresh_entry = _e;


_e = _m.find("fcst_inten_perc");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_inten_perc\"\n\n";

   exit ( 1 );

}

_fcst_inten_perc_entry = _e;


_e = _m.find("fcst_inten_perc_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_inten_perc_thresh\"\n\n";

   exit ( 1 );

}

_fcst_inten_perc_thresh_entry = _e;


_e = _m.find("obs_inten_perc");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_inten_perc\"\n\n";

   exit ( 1 );

}

_obs_inten_perc_entry = _e;


_e = _m.find("obs_inten_perc_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_inten_perc_thresh\"\n\n";

   exit ( 1 );

}

_obs_inten_perc_thresh_entry = _e;


_e = _m.find("fcst_merge_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_merge_thresh\"\n\n";

   exit ( 1 );

}

_fcst_merge_thresh_entry = _e;


_e = _m.find("obs_merge_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_merge_thresh\"\n\n";

   exit ( 1 );

}

_obs_merge_thresh_entry = _e;


_e = _m.find("fcst_merge_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_merge_flag\"\n\n";

   exit ( 1 );

}

_fcst_merge_flag_entry = _e;


_e = _m.find("obs_merge_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_merge_flag\"\n\n";

   exit ( 1 );

}

_obs_merge_flag_entry = _e;


_e = _m.find("match_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"match_flag\"\n\n";

   exit ( 1 );

}

_match_flag_entry = _e;


_e = _m.find("max_centroid_dist");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"max_centroid_dist\"\n\n";

   exit ( 1 );

}

_max_centroid_dist_entry = _e;


_e = _m.find("centroid_dist_weight");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"centroid_dist_weight\"\n\n";

   exit ( 1 );

}

_centroid_dist_weight_entry = _e;


_e = _m.find("boundary_dist_weight");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"boundary_dist_weight\"\n\n";

   exit ( 1 );

}

_boundary_dist_weight_entry = _e;


_e = _m.find("convex_hull_dist_weight");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"convex_hull_dist_weight\"\n\n";

   exit ( 1 );

}

_convex_hull_dist_weight_entry = _e;


_e = _m.find("angle_diff_weight");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"angle_diff_weight\"\n\n";

   exit ( 1 );

}

_angle_diff_weight_entry = _e;


_e = _m.find("area_ratio_weight");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"area_ratio_weight\"\n\n";

   exit ( 1 );

}

_area_ratio_weight_entry = _e;


_e = _m.find("int_area_ratio_weight");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"int_area_ratio_weight\"\n\n";

   exit ( 1 );

}

_int_area_ratio_weight_entry = _e;


_e = _m.find("complexity_ratio_weight");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"complexity_ratio_weight\"\n\n";

   exit ( 1 );

}

_complexity_ratio_weight_entry = _e;


_e = _m.find("intensity_percentile");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_percentile\"\n\n";

   exit ( 1 );

}

_intensity_percentile_entry = _e;


_e = _m.find("intensity_ratio_weight");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_ratio_weight\"\n\n";

   exit ( 1 );

}

_intensity_ratio_weight_entry = _e;


_e = _m.find("centroid_dist_if");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"centroid_dist_if\"\n\n";

   exit ( 1 );

}

_centroid_dist_if_entry = _e;


_e = _m.find("boundary_dist_if");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"boundary_dist_if\"\n\n";

   exit ( 1 );

}

_boundary_dist_if_entry = _e;


_e = _m.find("convex_hull_dist_if");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"convex_hull_dist_if\"\n\n";

   exit ( 1 );

}

_convex_hull_dist_if_entry = _e;


_e = _m.find("angle_diff_if");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"angle_diff_if\"\n\n";

   exit ( 1 );

}

_angle_diff_if_entry = _e;


_e = _m.find("corner");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"corner\"\n\n";

   exit ( 1 );

}

_corner_entry = _e;


_e = _m.find("ratio_if");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"ratio_if\"\n\n";

   exit ( 1 );

}

_ratio_if_entry = _e;


_e = _m.find("area_ratio_if");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"area_ratio_if\"\n\n";

   exit ( 1 );

}

_area_ratio_if_entry = _e;


_e = _m.find("int_area_ratio_if");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"int_area_ratio_if\"\n\n";

   exit ( 1 );

}

_int_area_ratio_if_entry = _e;


_e = _m.find("complexity_ratio_if");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"complexity_ratio_if\"\n\n";

   exit ( 1 );

}

_complexity_ratio_if_entry = _e;


_e = _m.find("intensity_ratio_if");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_ratio_if\"\n\n";

   exit ( 1 );

}

_intensity_ratio_if_entry = _e;


_e = _m.find("aspect_ratio_conf");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"aspect_ratio_conf\"\n\n";

   exit ( 1 );

}

_aspect_ratio_conf_entry = _e;


_e = _m.find("area_ratio_conf");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"area_ratio_conf\"\n\n";

   exit ( 1 );

}

_area_ratio_conf_entry = _e;


_e = _m.find("total_interest_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"total_interest_thresh\"\n\n";

   exit ( 1 );

}

_total_interest_thresh_entry = _e;


_e = _m.find("print_interest_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"print_interest_thresh\"\n\n";

   exit ( 1 );

}

_print_interest_thresh_entry = _e;


_e = _m.find("met_data_dir");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"met_data_dir\"\n\n";

   exit ( 1 );

}

_met_data_dir_entry = _e;


_e = _m.find("fcst_raw_color_table");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_raw_color_table\"\n\n";

   exit ( 1 );

}

_fcst_raw_color_table_entry = _e;


_e = _m.find("obs_raw_color_table");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_raw_color_table\"\n\n";

   exit ( 1 );

}

_obs_raw_color_table_entry = _e;


_e = _m.find("fcst_raw_plot_min");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_raw_plot_min\"\n\n";

   exit ( 1 );

}

_fcst_raw_plot_min_entry = _e;


_e = _m.find("fcst_raw_plot_max");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_raw_plot_max\"\n\n";

   exit ( 1 );

}

_fcst_raw_plot_max_entry = _e;


_e = _m.find("obs_raw_plot_min");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_raw_plot_min\"\n\n";

   exit ( 1 );

}

_obs_raw_plot_min_entry = _e;


_e = _m.find("obs_raw_plot_max");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_raw_plot_max\"\n\n";

   exit ( 1 );

}

_obs_raw_plot_max_entry = _e;


_e = _m.find("stride_length");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"stride_length\"\n\n";

   exit ( 1 );

}

_stride_length_entry = _e;


_e = _m.find("mode_color_table");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"mode_color_table\"\n\n";

   exit ( 1 );

}

_mode_color_table_entry = _e;


_e = _m.find("zero_border_size");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"zero_border_size\"\n\n";

   exit ( 1 );

}

_zero_border_size_entry = _e;


_e = _m.find("plot_valid_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"plot_valid_flag\"\n\n";

   exit ( 1 );

}

_plot_valid_flag_entry = _e;


_e = _m.find("plot_gcarc_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"plot_gcarc_flag\"\n\n";

   exit ( 1 );

}

_plot_gcarc_flag_entry = _e;


_e = _m.find("grib_ptv");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"grib_ptv\"\n\n";

   exit ( 1 );

}

_grib_ptv_entry = _e;


_e = _m.find("output_prefix");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"output_prefix\"\n\n";

   exit ( 1 );

}

_output_prefix_entry = _e;


_e = _m.find("version");

if ( !_e && Panic )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"version\"\n\n";

   exit ( 1 );

}

_version_entry = _e;



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::model()

{

Result _temp_result;

if ( !_model_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_model_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::grid_res()

{

Result _temp_result;

if ( !_grid_res_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_grid_res_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_field()

{

Result _temp_result;

if ( !_fcst_field_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_field_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_field()

{

Result _temp_result;

if ( !_obs_field_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_field_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::mask_missing_flag()

{

Result _temp_result;

if ( !_mask_missing_flag_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_mask_missing_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::mask_grid()

{

Result _temp_result;

if ( !_mask_grid_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_mask_grid_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::mask_grid_flag()

{

Result _temp_result;

if ( !_mask_grid_flag_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_mask_grid_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::mask_poly()

{

Result _temp_result;

if ( !_mask_poly_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_mask_poly_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::mask_poly_flag()

{

Result _temp_result;

if ( !_mask_poly_flag_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_mask_poly_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_raw_thresh()

{

Result _temp_result;

if ( !_fcst_raw_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_raw_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_raw_thresh()

{

Result _temp_result;

if ( !_obs_raw_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_raw_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_conv_radius()

{

Result _temp_result;

if ( !_fcst_conv_radius_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_conv_radius_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_conv_radius()

{

Result _temp_result;

if ( !_obs_conv_radius_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_conv_radius_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::bad_data_thresh()

{

Result _temp_result;

if ( !_bad_data_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_bad_data_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_conv_thresh()

{

Result _temp_result;

if ( !_fcst_conv_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_conv_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_conv_thresh()

{

Result _temp_result;

if ( !_obs_conv_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_conv_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_area_thresh()

{

Result _temp_result;

if ( !_fcst_area_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_area_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_area_thresh()

{

Result _temp_result;

if ( !_obs_area_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_area_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_inten_perc()

{

Result _temp_result;

if ( !_fcst_inten_perc_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_inten_perc_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_inten_perc_thresh()

{

Result _temp_result;

if ( !_fcst_inten_perc_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_inten_perc_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_inten_perc()

{

Result _temp_result;

if ( !_obs_inten_perc_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_inten_perc_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_inten_perc_thresh()

{

Result _temp_result;

if ( !_obs_inten_perc_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_inten_perc_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_merge_thresh()

{

Result _temp_result;

if ( !_fcst_merge_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_merge_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_merge_thresh()

{

Result _temp_result;

if ( !_obs_merge_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_merge_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_merge_flag()

{

Result _temp_result;

if ( !_fcst_merge_flag_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_merge_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_merge_flag()

{

Result _temp_result;

if ( !_obs_merge_flag_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_merge_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::match_flag()

{

Result _temp_result;

if ( !_match_flag_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_match_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::max_centroid_dist()

{

Result _temp_result;

if ( !_max_centroid_dist_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_max_centroid_dist_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::centroid_dist_weight()

{

Result _temp_result;

if ( !_centroid_dist_weight_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_centroid_dist_weight_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::boundary_dist_weight()

{

Result _temp_result;

if ( !_boundary_dist_weight_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_boundary_dist_weight_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::convex_hull_dist_weight()

{

Result _temp_result;

if ( !_convex_hull_dist_weight_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_convex_hull_dist_weight_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::angle_diff_weight()

{

Result _temp_result;

if ( !_angle_diff_weight_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_angle_diff_weight_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::area_ratio_weight()

{

Result _temp_result;

if ( !_area_ratio_weight_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_area_ratio_weight_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::int_area_ratio_weight()

{

Result _temp_result;

if ( !_int_area_ratio_weight_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_int_area_ratio_weight_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::complexity_ratio_weight()

{

Result _temp_result;

if ( !_complexity_ratio_weight_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_complexity_ratio_weight_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::intensity_percentile()

{

Result _temp_result;

if ( !_intensity_percentile_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intensity_percentile_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::intensity_ratio_weight()

{

Result _temp_result;

if ( !_intensity_ratio_weight_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intensity_ratio_weight_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::centroid_dist_if(const Result & _r)

{

Result _temp_result;

if ( !_centroid_dist_if_entry && !Panic )  return ( _temp_result );

double _x, _y;
const PiecewiseLinear & _pwl_f = *(_centroid_dist_if_entry->pl);


_x = _r.dval();

_y = _pwl_f(_x);

_temp_result.set_double(_y);

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::boundary_dist_if(const Result & _r)

{

Result _temp_result;

if ( !_boundary_dist_if_entry && !Panic )  return ( _temp_result );

double _x, _y;
const PiecewiseLinear & _pwl_f = *(_boundary_dist_if_entry->pl);


_x = _r.dval();

_y = _pwl_f(_x);

_temp_result.set_double(_y);

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::convex_hull_dist_if(const Result & _r)

{

Result _temp_result;

if ( !_convex_hull_dist_if_entry && !Panic )  return ( _temp_result );

double _x, _y;
const PiecewiseLinear & _pwl_f = *(_convex_hull_dist_if_entry->pl);


_x = _r.dval();

_y = _pwl_f(_x);

_temp_result.set_double(_y);

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::angle_diff_if(const Result & _r)

{

Result _temp_result;

if ( !_angle_diff_if_entry && !Panic )  return ( _temp_result );

double _x, _y;
const PiecewiseLinear & _pwl_f = *(_angle_diff_if_entry->pl);


_x = _r.dval();

_y = _pwl_f(_x);

_temp_result.set_double(_y);

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::corner()

{

Result _temp_result;

if ( !_corner_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_corner_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::ratio_if(const Result & _r)

{

Result _temp_result;

if ( !_ratio_if_entry && !Panic )  return ( _temp_result );

double _x, _y;
const PiecewiseLinear & _pwl_f = *(_ratio_if_entry->pl);


_x = _r.dval();

_y = _pwl_f(_x);

_temp_result.set_double(_y);

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::area_ratio_if(const Result & _r_0)

{

Result _temp_result;

if ( !_area_ratio_if_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;


   //
   //  push the arguments onto the stack
   //

result_to_icodecell(_r_0, _cell);

_m.push(_cell);

   //
   //  run the program for the function
   //

_m.run( *_area_ratio_if_entry );

   //
   //  get the function's return value
   //

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::int_area_ratio_if(const Result & _r)

{

Result _temp_result;

if ( !_int_area_ratio_if_entry && !Panic )  return ( _temp_result );

double _x, _y;
const PiecewiseLinear & _pwl_f = *(_int_area_ratio_if_entry->pl);


_x = _r.dval();

_y = _pwl_f(_x);

_temp_result.set_double(_y);

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::complexity_ratio_if(const Result & _r_0)

{

Result _temp_result;

if ( !_complexity_ratio_if_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;


   //
   //  push the arguments onto the stack
   //

result_to_icodecell(_r_0, _cell);

_m.push(_cell);

   //
   //  run the program for the function
   //

_m.run( *_complexity_ratio_if_entry );

   //
   //  get the function's return value
   //

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::intensity_ratio_if(const Result & _r_0)

{

Result _temp_result;

if ( !_intensity_ratio_if_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;


   //
   //  push the arguments onto the stack
   //

result_to_icodecell(_r_0, _cell);

_m.push(_cell);

   //
   //  run the program for the function
   //

_m.run( *_intensity_ratio_if_entry );

   //
   //  get the function's return value
   //

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::aspect_ratio_conf(const Result & _r_0)

{

Result _temp_result;

if ( !_aspect_ratio_conf_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;


   //
   //  push the arguments onto the stack
   //

result_to_icodecell(_r_0, _cell);

_m.push(_cell);

   //
   //  run the program for the function
   //

_m.run( *_aspect_ratio_conf_entry );

   //
   //  get the function's return value
   //

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::area_ratio_conf(const Result & _r_0)

{

Result _temp_result;

if ( !_area_ratio_conf_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;


   //
   //  push the arguments onto the stack
   //

result_to_icodecell(_r_0, _cell);

_m.push(_cell);

   //
   //  run the program for the function
   //

_m.run( *_area_ratio_conf_entry );

   //
   //  get the function's return value
   //

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::total_interest_thresh()

{

Result _temp_result;

if ( !_total_interest_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_total_interest_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::print_interest_thresh()

{

Result _temp_result;

if ( !_print_interest_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_print_interest_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::met_data_dir()

{

Result _temp_result;

if ( !_met_data_dir_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_met_data_dir_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_raw_color_table()

{

Result _temp_result;

if ( !_fcst_raw_color_table_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_raw_color_table_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_raw_color_table()

{

Result _temp_result;

if ( !_obs_raw_color_table_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_raw_color_table_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_raw_plot_min()

{

Result _temp_result;

if ( !_fcst_raw_plot_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_raw_plot_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_raw_plot_max()

{

Result _temp_result;

if ( !_fcst_raw_plot_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_raw_plot_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_raw_plot_min()

{

Result _temp_result;

if ( !_obs_raw_plot_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_raw_plot_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_raw_plot_max()

{

Result _temp_result;

if ( !_obs_raw_plot_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_raw_plot_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::stride_length()

{

Result _temp_result;

if ( !_stride_length_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_stride_length_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::mode_color_table()

{

Result _temp_result;

if ( !_mode_color_table_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_mode_color_table_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::zero_border_size()

{

Result _temp_result;

if ( !_zero_border_size_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_zero_border_size_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::plot_valid_flag()

{

Result _temp_result;

if ( !_plot_valid_flag_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_plot_valid_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::plot_gcarc_flag()

{

Result _temp_result;

if ( !_plot_gcarc_flag_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_plot_gcarc_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::grib_ptv()

{

Result _temp_result;

if ( !_grib_ptv_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_grib_ptv_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::output_prefix()

{

Result _temp_result;

if ( !_output_prefix_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_output_prefix_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::version()

{

Result _temp_result;

if ( !_version_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_version_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


