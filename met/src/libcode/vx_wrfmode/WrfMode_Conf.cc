

////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated
   //
   //     Do not edit by hand
   //
   //
   //     Created by econfig_codegen from config file "/d1/johnhg/MET/MET_development/svn-met-dev.cgd.ucar.edu/trunk/met/config_templates/WrfModeConfig_default"
   //
   //     on March 8, 2011    11:55 am  MST
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "WrfMode_Conf.h"
#include "icodecell_to_result.h"


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

clear();

const SymbolTableEntry * _e = (const SymbolTableEntry *) 0;

   //
   //  read the config file into the machine
   //

_m.read(_config_filename);

   //
   //  lookup the entries in the symbol table
   //

_e = _m.find("model");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"model\"\n\n";

   exit ( 1 );

}

_model_entry = _e;


_e = _m.find("grid_res");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"grid_res\"\n\n";

   exit ( 1 );

}

_grid_res_entry = _e;


_e = _m.find("fcst_field");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_field\"\n\n";

   exit ( 1 );

}

_fcst_field_entry = _e;


_e = _m.find("obs_field");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_field\"\n\n";

   exit ( 1 );

}

_obs_field_entry = _e;


_e = _m.find("mask_missing_flag");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_missing_flag\"\n\n";

   exit ( 1 );

}

_mask_missing_flag_entry = _e;


_e = _m.find("mask_grid");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_grid\"\n\n";

   exit ( 1 );

}

_mask_grid_entry = _e;


_e = _m.find("mask_grid_flag");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_grid_flag\"\n\n";

   exit ( 1 );

}

_mask_grid_flag_entry = _e;


_e = _m.find("mask_poly");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_poly\"\n\n";

   exit ( 1 );

}

_mask_poly_entry = _e;


_e = _m.find("mask_poly_flag");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_poly_flag\"\n\n";

   exit ( 1 );

}

_mask_poly_flag_entry = _e;


_e = _m.find("fcst_raw_thresh");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_raw_thresh\"\n\n";

   exit ( 1 );

}

_fcst_raw_thresh_entry = _e;


_e = _m.find("obs_raw_thresh");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_raw_thresh\"\n\n";

   exit ( 1 );

}

_obs_raw_thresh_entry = _e;


_e = _m.find("fcst_conv_radius");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_conv_radius\"\n\n";

   exit ( 1 );

}

_fcst_conv_radius_entry = _e;


_e = _m.find("obs_conv_radius");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_conv_radius\"\n\n";

   exit ( 1 );

}

_obs_conv_radius_entry = _e;


_e = _m.find("bad_data_thresh");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"bad_data_thresh\"\n\n";

   exit ( 1 );

}

_bad_data_thresh_entry = _e;


_e = _m.find("fcst_conv_thresh");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_conv_thresh\"\n\n";

   exit ( 1 );

}

_fcst_conv_thresh_entry = _e;


_e = _m.find("obs_conv_thresh");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_conv_thresh\"\n\n";

   exit ( 1 );

}

_obs_conv_thresh_entry = _e;


_e = _m.find("fcst_area_thresh");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_area_thresh\"\n\n";

   exit ( 1 );

}

_fcst_area_thresh_entry = _e;


_e = _m.find("obs_area_thresh");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_area_thresh\"\n\n";

   exit ( 1 );

}

_obs_area_thresh_entry = _e;


_e = _m.find("fcst_inten_perc");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_inten_perc\"\n\n";

   exit ( 1 );

}

_fcst_inten_perc_entry = _e;


_e = _m.find("fcst_inten_perc_thresh");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_inten_perc_thresh\"\n\n";

   exit ( 1 );

}

_fcst_inten_perc_thresh_entry = _e;


_e = _m.find("obs_inten_perc");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_inten_perc\"\n\n";

   exit ( 1 );

}

_obs_inten_perc_entry = _e;


_e = _m.find("obs_inten_perc_thresh");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_inten_perc_thresh\"\n\n";

   exit ( 1 );

}

_obs_inten_perc_thresh_entry = _e;


_e = _m.find("fcst_merge_thresh");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_merge_thresh\"\n\n";

   exit ( 1 );

}

_fcst_merge_thresh_entry = _e;


_e = _m.find("obs_merge_thresh");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_merge_thresh\"\n\n";

   exit ( 1 );

}

_obs_merge_thresh_entry = _e;


_e = _m.find("fcst_merge_flag");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_merge_flag\"\n\n";

   exit ( 1 );

}

_fcst_merge_flag_entry = _e;


_e = _m.find("obs_merge_flag");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_merge_flag\"\n\n";

   exit ( 1 );

}

_obs_merge_flag_entry = _e;


_e = _m.find("match_flag");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"match_flag\"\n\n";

   exit ( 1 );

}

_match_flag_entry = _e;


_e = _m.find("max_centroid_dist");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"max_centroid_dist\"\n\n";

   exit ( 1 );

}

_max_centroid_dist_entry = _e;


_e = _m.find("centroid_dist_weight");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"centroid_dist_weight\"\n\n";

   exit ( 1 );

}

_centroid_dist_weight_entry = _e;


_e = _m.find("boundary_dist_weight");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"boundary_dist_weight\"\n\n";

   exit ( 1 );

}

_boundary_dist_weight_entry = _e;


_e = _m.find("convex_hull_dist_weight");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"convex_hull_dist_weight\"\n\n";

   exit ( 1 );

}

_convex_hull_dist_weight_entry = _e;


_e = _m.find("angle_diff_weight");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"angle_diff_weight\"\n\n";

   exit ( 1 );

}

_angle_diff_weight_entry = _e;


_e = _m.find("area_ratio_weight");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"area_ratio_weight\"\n\n";

   exit ( 1 );

}

_area_ratio_weight_entry = _e;


_e = _m.find("int_area_ratio_weight");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"int_area_ratio_weight\"\n\n";

   exit ( 1 );

}

_int_area_ratio_weight_entry = _e;


_e = _m.find("complexity_ratio_weight");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"complexity_ratio_weight\"\n\n";

   exit ( 1 );

}

_complexity_ratio_weight_entry = _e;


_e = _m.find("intensity_percentile");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_percentile\"\n\n";

   exit ( 1 );

}

_intensity_percentile_entry = _e;


_e = _m.find("intensity_ratio_weight");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_ratio_weight\"\n\n";

   exit ( 1 );

}

_intensity_ratio_weight_entry = _e;


_e = _m.find("centroid_dist_if");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"centroid_dist_if\"\n\n";

   exit ( 1 );

}

_centroid_dist_if_entry = _e;


_e = _m.find("boundary_dist_if");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"boundary_dist_if\"\n\n";

   exit ( 1 );

}

_boundary_dist_if_entry = _e;


_e = _m.find("convex_hull_dist_if");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"convex_hull_dist_if\"\n\n";

   exit ( 1 );

}

_convex_hull_dist_if_entry = _e;


_e = _m.find("angle_diff_if");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"angle_diff_if\"\n\n";

   exit ( 1 );

}

_angle_diff_if_entry = _e;


_e = _m.find("corner");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"corner\"\n\n";

   exit ( 1 );

}

_corner_entry = _e;


_e = _m.find("ratio_if");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"ratio_if\"\n\n";

   exit ( 1 );

}

_ratio_if_entry = _e;


_e = _m.find("area_ratio_if");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"area_ratio_if\"\n\n";

   exit ( 1 );

}

_area_ratio_if_entry = _e;


_e = _m.find("int_area_ratio_if");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"int_area_ratio_if\"\n\n";

   exit ( 1 );

}

_int_area_ratio_if_entry = _e;


_e = _m.find("complexity_ratio_if");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"complexity_ratio_if\"\n\n";

   exit ( 1 );

}

_complexity_ratio_if_entry = _e;


_e = _m.find("intensity_ratio_if");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_ratio_if\"\n\n";

   exit ( 1 );

}

_intensity_ratio_if_entry = _e;


_e = _m.find("aspect_ratio_conf");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"aspect_ratio_conf\"\n\n";

   exit ( 1 );

}

_aspect_ratio_conf_entry = _e;


_e = _m.find("area_ratio_conf");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"area_ratio_conf\"\n\n";

   exit ( 1 );

}

_area_ratio_conf_entry = _e;


_e = _m.find("total_interest_thresh");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"total_interest_thresh\"\n\n";

   exit ( 1 );

}

_total_interest_thresh_entry = _e;


_e = _m.find("print_interest_thresh");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"print_interest_thresh\"\n\n";

   exit ( 1 );

}

_print_interest_thresh_entry = _e;


_e = _m.find("met_data_dir");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"met_data_dir\"\n\n";

   exit ( 1 );

}

_met_data_dir_entry = _e;


_e = _m.find("fcst_raw_color_table");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_raw_color_table\"\n\n";

   exit ( 1 );

}

_fcst_raw_color_table_entry = _e;


_e = _m.find("obs_raw_color_table");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_raw_color_table\"\n\n";

   exit ( 1 );

}

_obs_raw_color_table_entry = _e;


_e = _m.find("fcst_raw_plot_min");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_raw_plot_min\"\n\n";

   exit ( 1 );

}

_fcst_raw_plot_min_entry = _e;


_e = _m.find("fcst_raw_plot_max");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_raw_plot_max\"\n\n";

   exit ( 1 );

}

_fcst_raw_plot_max_entry = _e;


_e = _m.find("obs_raw_plot_min");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_raw_plot_min\"\n\n";

   exit ( 1 );

}

_obs_raw_plot_min_entry = _e;


_e = _m.find("obs_raw_plot_max");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_raw_plot_max\"\n\n";

   exit ( 1 );

}

_obs_raw_plot_max_entry = _e;


_e = _m.find("stride_length");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"stride_length\"\n\n";

   exit ( 1 );

}

_stride_length_entry = _e;


_e = _m.find("mode_color_table");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"mode_color_table\"\n\n";

   exit ( 1 );

}

_mode_color_table_entry = _e;


_e = _m.find("zero_border_size");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"zero_border_size\"\n\n";

   exit ( 1 );

}

_zero_border_size_entry = _e;


_e = _m.find("plot_valid_flag");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"plot_valid_flag\"\n\n";

   exit ( 1 );

}

_plot_valid_flag_entry = _e;


_e = _m.find("plot_gcarc_flag");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"plot_gcarc_flag\"\n\n";

   exit ( 1 );

}

_plot_gcarc_flag_entry = _e;


_e = _m.find("grib_ptv");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"grib_ptv\"\n\n";

   exit ( 1 );

}

_grib_ptv_entry = _e;


_e = _m.find("output_prefix");

if ( !_e )  {

   cerr << "\n\n  WrfMode_Conf::read(const char *) -> can't get symbol table entry for variable \"output_prefix\"\n\n";

   exit ( 1 );

}

_output_prefix_entry = _e;


_e = _m.find("version");

if ( !_e )  {

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


void WrfMode_Conf::st_dump(ostream & _out, int _depth) const

{

_m.st_dump(_out, _depth);

return;

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::model()

{

if ( !_model_entry )  {

   cerr << "\n\n   WrfMode_Conf::model() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_model_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::grid_res()

{

if ( !_grid_res_entry )  {

   cerr << "\n\n   WrfMode_Conf::grid_res() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_grid_res_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_field()

{

if ( !_fcst_field_entry )  {

   cerr << "\n\n   WrfMode_Conf::fcst_field() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_fcst_field_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_field()

{

if ( !_obs_field_entry )  {

   cerr << "\n\n   WrfMode_Conf::obs_field() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_obs_field_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::mask_missing_flag()

{

if ( !_mask_missing_flag_entry )  {

   cerr << "\n\n   WrfMode_Conf::mask_missing_flag() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_mask_missing_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::mask_grid()

{

if ( !_mask_grid_entry )  {

   cerr << "\n\n   WrfMode_Conf::mask_grid() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_mask_grid_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::mask_grid_flag()

{

if ( !_mask_grid_flag_entry )  {

   cerr << "\n\n   WrfMode_Conf::mask_grid_flag() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_mask_grid_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::mask_poly()

{

if ( !_mask_poly_entry )  {

   cerr << "\n\n   WrfMode_Conf::mask_poly() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_mask_poly_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::mask_poly_flag()

{

if ( !_mask_poly_flag_entry )  {

   cerr << "\n\n   WrfMode_Conf::mask_poly_flag() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_mask_poly_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_raw_thresh()

{

if ( !_fcst_raw_thresh_entry )  {

   cerr << "\n\n   WrfMode_Conf::fcst_raw_thresh() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_fcst_raw_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_raw_thresh()

{

if ( !_obs_raw_thresh_entry )  {

   cerr << "\n\n   WrfMode_Conf::obs_raw_thresh() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_obs_raw_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_conv_radius()

{

if ( !_fcst_conv_radius_entry )  {

   cerr << "\n\n   WrfMode_Conf::fcst_conv_radius() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_fcst_conv_radius_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_conv_radius()

{

if ( !_obs_conv_radius_entry )  {

   cerr << "\n\n   WrfMode_Conf::obs_conv_radius() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_obs_conv_radius_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::bad_data_thresh()

{

if ( !_bad_data_thresh_entry )  {

   cerr << "\n\n   WrfMode_Conf::bad_data_thresh() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_bad_data_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_conv_thresh()

{

if ( !_fcst_conv_thresh_entry )  {

   cerr << "\n\n   WrfMode_Conf::fcst_conv_thresh() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_fcst_conv_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_conv_thresh()

{

if ( !_obs_conv_thresh_entry )  {

   cerr << "\n\n   WrfMode_Conf::obs_conv_thresh() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_obs_conv_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_area_thresh()

{

if ( !_fcst_area_thresh_entry )  {

   cerr << "\n\n   WrfMode_Conf::fcst_area_thresh() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_fcst_area_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_area_thresh()

{

if ( !_obs_area_thresh_entry )  {

   cerr << "\n\n   WrfMode_Conf::obs_area_thresh() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_obs_area_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_inten_perc()

{

if ( !_fcst_inten_perc_entry )  {

   cerr << "\n\n   WrfMode_Conf::fcst_inten_perc() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_fcst_inten_perc_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_inten_perc_thresh()

{

if ( !_fcst_inten_perc_thresh_entry )  {

   cerr << "\n\n   WrfMode_Conf::fcst_inten_perc_thresh() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_fcst_inten_perc_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_inten_perc()

{

if ( !_obs_inten_perc_entry )  {

   cerr << "\n\n   WrfMode_Conf::obs_inten_perc() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_obs_inten_perc_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_inten_perc_thresh()

{

if ( !_obs_inten_perc_thresh_entry )  {

   cerr << "\n\n   WrfMode_Conf::obs_inten_perc_thresh() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_obs_inten_perc_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_merge_thresh()

{

if ( !_fcst_merge_thresh_entry )  {

   cerr << "\n\n   WrfMode_Conf::fcst_merge_thresh() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_fcst_merge_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_merge_thresh()

{

if ( !_obs_merge_thresh_entry )  {

   cerr << "\n\n   WrfMode_Conf::obs_merge_thresh() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_obs_merge_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_merge_flag()

{

if ( !_fcst_merge_flag_entry )  {

   cerr << "\n\n   WrfMode_Conf::fcst_merge_flag() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_fcst_merge_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_merge_flag()

{

if ( !_obs_merge_flag_entry )  {

   cerr << "\n\n   WrfMode_Conf::obs_merge_flag() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_obs_merge_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::match_flag()

{

if ( !_match_flag_entry )  {

   cerr << "\n\n   WrfMode_Conf::match_flag() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_match_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::max_centroid_dist()

{

if ( !_max_centroid_dist_entry )  {

   cerr << "\n\n   WrfMode_Conf::max_centroid_dist() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_max_centroid_dist_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::centroid_dist_weight()

{

if ( !_centroid_dist_weight_entry )  {

   cerr << "\n\n   WrfMode_Conf::centroid_dist_weight() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_centroid_dist_weight_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::boundary_dist_weight()

{

if ( !_boundary_dist_weight_entry )  {

   cerr << "\n\n   WrfMode_Conf::boundary_dist_weight() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_boundary_dist_weight_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::convex_hull_dist_weight()

{

if ( !_convex_hull_dist_weight_entry )  {

   cerr << "\n\n   WrfMode_Conf::convex_hull_dist_weight() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_convex_hull_dist_weight_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::angle_diff_weight()

{

if ( !_angle_diff_weight_entry )  {

   cerr << "\n\n   WrfMode_Conf::angle_diff_weight() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_angle_diff_weight_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::area_ratio_weight()

{

if ( !_area_ratio_weight_entry )  {

   cerr << "\n\n   WrfMode_Conf::area_ratio_weight() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_area_ratio_weight_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::int_area_ratio_weight()

{

if ( !_int_area_ratio_weight_entry )  {

   cerr << "\n\n   WrfMode_Conf::int_area_ratio_weight() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_int_area_ratio_weight_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::complexity_ratio_weight()

{

if ( !_complexity_ratio_weight_entry )  {

   cerr << "\n\n   WrfMode_Conf::complexity_ratio_weight() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_complexity_ratio_weight_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::intensity_percentile()

{

if ( !_intensity_percentile_entry )  {

   cerr << "\n\n   WrfMode_Conf::intensity_percentile() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_intensity_percentile_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::intensity_ratio_weight()

{

if ( !_intensity_ratio_weight_entry )  {

   cerr << "\n\n   WrfMode_Conf::intensity_ratio_weight() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_intensity_ratio_weight_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::centroid_dist_if(const Result & _r)

{

if ( !_centroid_dist_if_entry )  {

   cerr << "\n\n   WrfMode_Conf::centroid_dist_if() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
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

if ( !_boundary_dist_if_entry )  {

   cerr << "\n\n   WrfMode_Conf::boundary_dist_if() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
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

if ( !_convex_hull_dist_if_entry )  {

   cerr << "\n\n   WrfMode_Conf::convex_hull_dist_if() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
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

if ( !_angle_diff_if_entry )  {

   cerr << "\n\n   WrfMode_Conf::angle_diff_if() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
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

if ( !_corner_entry )  {

   cerr << "\n\n   WrfMode_Conf::corner() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_corner_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::ratio_if(const Result & _r)

{

if ( !_ratio_if_entry )  {

   cerr << "\n\n   WrfMode_Conf::ratio_if() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
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

if ( !_area_ratio_if_entry )  {

   cerr << "\n\n   WrfMode_Conf::area_ratio_if() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
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

if ( !_int_area_ratio_if_entry )  {

   cerr << "\n\n   WrfMode_Conf::int_area_ratio_if() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
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

if ( !_complexity_ratio_if_entry )  {

   cerr << "\n\n   WrfMode_Conf::complexity_ratio_if() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
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

if ( !_intensity_ratio_if_entry )  {

   cerr << "\n\n   WrfMode_Conf::intensity_ratio_if() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
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

if ( !_aspect_ratio_conf_entry )  {

   cerr << "\n\n   WrfMode_Conf::aspect_ratio_conf() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
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

if ( !_area_ratio_conf_entry )  {

   cerr << "\n\n   WrfMode_Conf::area_ratio_conf() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
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

if ( !_total_interest_thresh_entry )  {

   cerr << "\n\n   WrfMode_Conf::total_interest_thresh() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_total_interest_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::print_interest_thresh()

{

if ( !_print_interest_thresh_entry )  {

   cerr << "\n\n   WrfMode_Conf::print_interest_thresh() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_print_interest_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::met_data_dir()

{

if ( !_met_data_dir_entry )  {

   cerr << "\n\n   WrfMode_Conf::met_data_dir() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_met_data_dir_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_raw_color_table()

{

if ( !_fcst_raw_color_table_entry )  {

   cerr << "\n\n   WrfMode_Conf::fcst_raw_color_table() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_fcst_raw_color_table_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_raw_color_table()

{

if ( !_obs_raw_color_table_entry )  {

   cerr << "\n\n   WrfMode_Conf::obs_raw_color_table() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_obs_raw_color_table_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_raw_plot_min()

{

if ( !_fcst_raw_plot_min_entry )  {

   cerr << "\n\n   WrfMode_Conf::fcst_raw_plot_min() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_fcst_raw_plot_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::fcst_raw_plot_max()

{

if ( !_fcst_raw_plot_max_entry )  {

   cerr << "\n\n   WrfMode_Conf::fcst_raw_plot_max() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_fcst_raw_plot_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_raw_plot_min()

{

if ( !_obs_raw_plot_min_entry )  {

   cerr << "\n\n   WrfMode_Conf::obs_raw_plot_min() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_obs_raw_plot_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::obs_raw_plot_max()

{

if ( !_obs_raw_plot_max_entry )  {

   cerr << "\n\n   WrfMode_Conf::obs_raw_plot_max() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_obs_raw_plot_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::stride_length()

{

if ( !_stride_length_entry )  {

   cerr << "\n\n   WrfMode_Conf::stride_length() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_stride_length_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::mode_color_table()

{

if ( !_mode_color_table_entry )  {

   cerr << "\n\n   WrfMode_Conf::mode_color_table() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_mode_color_table_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::zero_border_size()

{

if ( !_zero_border_size_entry )  {

   cerr << "\n\n   WrfMode_Conf::zero_border_size() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_zero_border_size_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::plot_valid_flag()

{

if ( !_plot_valid_flag_entry )  {

   cerr << "\n\n   WrfMode_Conf::plot_valid_flag() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_plot_valid_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::plot_gcarc_flag()

{

if ( !_plot_gcarc_flag_entry )  {

   cerr << "\n\n   WrfMode_Conf::plot_gcarc_flag() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_plot_gcarc_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::grib_ptv()

{

if ( !_grib_ptv_entry )  {

   cerr << "\n\n   WrfMode_Conf::grib_ptv() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_grib_ptv_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::output_prefix()

{

if ( !_output_prefix_entry )  {

   cerr << "\n\n   WrfMode_Conf::output_prefix() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_output_prefix_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WrfMode_Conf::version()

{

if ( !_version_entry )  {

   cerr << "\n\n   WrfMode_Conf::version() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_version_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


