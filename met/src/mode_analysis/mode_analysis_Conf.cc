

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
   //     on April 3, 2009    7:10 am  MST
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "mode_analysis_Conf.h"
#include "vx_econfig/icodecell_to_result.h"


////////////////////////////////////////////////////////////////////////


static const int Panic = 0;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class mode_analysis_Conf
   //


////////////////////////////////////////////////////////////////////////


mode_analysis_Conf::mode_analysis_Conf()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


mode_analysis_Conf::~mode_analysis_Conf()

{

clear();

}


////////////////////////////////////////////////////////////////////////


mode_analysis_Conf::mode_analysis_Conf(const mode_analysis_Conf &)

{

cerr << "\n\n  mode_analysis_Conf::mode_analysis_Conf(const mode_analysis_Conf &) -> should never be called!\n\n";

exit ( 1 );

//  init_from_scratch();

//  assign(a);

}


////////////////////////////////////////////////////////////////////////


mode_analysis_Conf & mode_analysis_Conf::operator=(const mode_analysis_Conf &)

{

cerr << "\n\n  mode_analysis_Conf::operator=(const mode_analysis_Conf &) -> should never be called!\n\n";

exit ( 1 );

// if ( this == &a )  return ( * this );

// assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void mode_analysis_Conf::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void mode_analysis_Conf::clear()

{

                          _fcst_entry = (const SymbolTableEntry *) 0;

                           _obs_entry = (const SymbolTableEntry *) 0;

                        _single_entry = (const SymbolTableEntry *) 0;

                          _pair_entry = (const SymbolTableEntry *) 0;

                        _simple_entry = (const SymbolTableEntry *) 0;

                       _cluster_entry = (const SymbolTableEntry *) 0;

                       _matched_entry = (const SymbolTableEntry *) 0;

                     _unmatched_entry = (const SymbolTableEntry *) 0;

                         _model_entry = (const SymbolTableEntry *) 0;

                      _fcst_thr_entry = (const SymbolTableEntry *) 0;

                       _obs_thr_entry = (const SymbolTableEntry *) 0;

                      _fcst_var_entry = (const SymbolTableEntry *) 0;

                      _fcst_lev_entry = (const SymbolTableEntry *) 0;

                       _obs_var_entry = (const SymbolTableEntry *) 0;

                       _obs_lev_entry = (const SymbolTableEntry *) 0;

                     _fcst_lead_entry = (const SymbolTableEntry *) 0;

                _fcst_init_hour_entry = (const SymbolTableEntry *) 0;

                    _fcst_accum_entry = (const SymbolTableEntry *) 0;

                      _obs_lead_entry = (const SymbolTableEntry *) 0;

                 _obs_init_hour_entry = (const SymbolTableEntry *) 0;

                     _obs_accum_entry = (const SymbolTableEntry *) 0;

                      _fcst_rad_entry = (const SymbolTableEntry *) 0;

                       _obs_rad_entry = (const SymbolTableEntry *) 0;

                      _area_min_entry = (const SymbolTableEntry *) 0;

                      _area_max_entry = (const SymbolTableEntry *) 0;

               _area_filter_min_entry = (const SymbolTableEntry *) 0;

               _area_filter_max_entry = (const SymbolTableEntry *) 0;

               _area_thresh_min_entry = (const SymbolTableEntry *) 0;

               _area_thresh_max_entry = (const SymbolTableEntry *) 0;

         _intersection_area_min_entry = (const SymbolTableEntry *) 0;

         _intersection_area_max_entry = (const SymbolTableEntry *) 0;

                _union_area_min_entry = (const SymbolTableEntry *) 0;

                _union_area_max_entry = (const SymbolTableEntry *) 0;

            _symmetric_diff_min_entry = (const SymbolTableEntry *) 0;

            _symmetric_diff_max_entry = (const SymbolTableEntry *) 0;

                _fcst_valid_min_entry = (const SymbolTableEntry *) 0;

                _fcst_valid_max_entry = (const SymbolTableEntry *) 0;

                 _obs_valid_min_entry = (const SymbolTableEntry *) 0;

                 _obs_valid_max_entry = (const SymbolTableEntry *) 0;

                 _fcst_init_min_entry = (const SymbolTableEntry *) 0;

                 _fcst_init_max_entry = (const SymbolTableEntry *) 0;

                  _obs_init_min_entry = (const SymbolTableEntry *) 0;

                  _obs_init_max_entry = (const SymbolTableEntry *) 0;

                _centroid_x_min_entry = (const SymbolTableEntry *) 0;

                _centroid_x_max_entry = (const SymbolTableEntry *) 0;

                _centroid_y_min_entry = (const SymbolTableEntry *) 0;

                _centroid_y_max_entry = (const SymbolTableEntry *) 0;

              _centroid_lat_min_entry = (const SymbolTableEntry *) 0;

              _centroid_lat_max_entry = (const SymbolTableEntry *) 0;

              _centroid_lon_min_entry = (const SymbolTableEntry *) 0;

              _centroid_lon_max_entry = (const SymbolTableEntry *) 0;

                  _axis_ang_min_entry = (const SymbolTableEntry *) 0;

                  _axis_ang_max_entry = (const SymbolTableEntry *) 0;

                    _length_min_entry = (const SymbolTableEntry *) 0;

                    _length_max_entry = (const SymbolTableEntry *) 0;

                     _width_min_entry = (const SymbolTableEntry *) 0;

                     _width_max_entry = (const SymbolTableEntry *) 0;

              _aspect_ratio_min_entry = (const SymbolTableEntry *) 0;

              _aspect_ratio_max_entry = (const SymbolTableEntry *) 0;

                 _curvature_min_entry = (const SymbolTableEntry *) 0;

                 _curvature_max_entry = (const SymbolTableEntry *) 0;

               _curvature_x_min_entry = (const SymbolTableEntry *) 0;

               _curvature_x_max_entry = (const SymbolTableEntry *) 0;

               _curvature_y_min_entry = (const SymbolTableEntry *) 0;

               _curvature_y_max_entry = (const SymbolTableEntry *) 0;

                _complexity_min_entry = (const SymbolTableEntry *) 0;

                _complexity_max_entry = (const SymbolTableEntry *) 0;

              _intensity_10_min_entry = (const SymbolTableEntry *) 0;

              _intensity_10_max_entry = (const SymbolTableEntry *) 0;

              _intensity_25_min_entry = (const SymbolTableEntry *) 0;

              _intensity_25_max_entry = (const SymbolTableEntry *) 0;

              _intensity_50_min_entry = (const SymbolTableEntry *) 0;

              _intensity_50_max_entry = (const SymbolTableEntry *) 0;

              _intensity_75_min_entry = (const SymbolTableEntry *) 0;

              _intensity_75_max_entry = (const SymbolTableEntry *) 0;

              _intensity_90_min_entry = (const SymbolTableEntry *) 0;

              _intensity_90_max_entry = (const SymbolTableEntry *) 0;

            _intensity_user_min_entry = (const SymbolTableEntry *) 0;

            _intensity_user_max_entry = (const SymbolTableEntry *) 0;

             _intensity_sum_min_entry = (const SymbolTableEntry *) 0;

             _intensity_sum_max_entry = (const SymbolTableEntry *) 0;

             _centroid_dist_min_entry = (const SymbolTableEntry *) 0;

             _centroid_dist_max_entry = (const SymbolTableEntry *) 0;

             _boundary_dist_min_entry = (const SymbolTableEntry *) 0;

             _boundary_dist_max_entry = (const SymbolTableEntry *) 0;

          _convex_hull_dist_min_entry = (const SymbolTableEntry *) 0;

          _convex_hull_dist_max_entry = (const SymbolTableEntry *) 0;

                _angle_diff_min_entry = (const SymbolTableEntry *) 0;

                _angle_diff_max_entry = (const SymbolTableEntry *) 0;

                _area_ratio_min_entry = (const SymbolTableEntry *) 0;

                _area_ratio_max_entry = (const SymbolTableEntry *) 0;

    _intersection_over_area_min_entry = (const SymbolTableEntry *) 0;

    _intersection_over_area_max_entry = (const SymbolTableEntry *) 0;

          _complexity_ratio_min_entry = (const SymbolTableEntry *) 0;

          _complexity_ratio_max_entry = (const SymbolTableEntry *) 0;

_percentile_intensity_ratio_min_entry = (const SymbolTableEntry *) 0;

_percentile_intensity_ratio_max_entry = (const SymbolTableEntry *) 0;

                  _interest_min_entry = (const SymbolTableEntry *) 0;

                  _interest_max_entry = (const SymbolTableEntry *) 0;

                       _version_entry = (const SymbolTableEntry *) 0;


_m.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void mode_analysis_Conf::read(const char * _config_filename)

{

const SymbolTableEntry * _e = (const SymbolTableEntry *) 0;

   //
   //  read the config file into the machine
   //

_m.read(_config_filename);

   //
   //  lookup the entries in the symbol table
   //

_e = _m.find("fcst");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst\"\n\n";

   exit ( 1 );

}

_fcst_entry = _e;


_e = _m.find("obs");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs\"\n\n";

   exit ( 1 );

}

_obs_entry = _e;


_e = _m.find("single");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"single\"\n\n";

   exit ( 1 );

}

_single_entry = _e;


_e = _m.find("pair");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"pair\"\n\n";

   exit ( 1 );

}

_pair_entry = _e;


_e = _m.find("simple");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"simple\"\n\n";

   exit ( 1 );

}

_simple_entry = _e;


_e = _m.find("cluster");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"cluster\"\n\n";

   exit ( 1 );

}

_cluster_entry = _e;


_e = _m.find("matched");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"matched\"\n\n";

   exit ( 1 );

}

_matched_entry = _e;


_e = _m.find("unmatched");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"unmatched\"\n\n";

   exit ( 1 );

}

_unmatched_entry = _e;


_e = _m.find("model");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"model\"\n\n";

   exit ( 1 );

}

_model_entry = _e;


_e = _m.find("fcst_thr");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_thr\"\n\n";

   exit ( 1 );

}

_fcst_thr_entry = _e;


_e = _m.find("obs_thr");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_thr\"\n\n";

   exit ( 1 );

}

_obs_thr_entry = _e;


_e = _m.find("fcst_var");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_var\"\n\n";

   exit ( 1 );

}

_fcst_var_entry = _e;


_e = _m.find("fcst_lev");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_lev\"\n\n";

   exit ( 1 );

}

_fcst_lev_entry = _e;


_e = _m.find("obs_var");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_var\"\n\n";

   exit ( 1 );

}

_obs_var_entry = _e;


_e = _m.find("obs_lev");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_lev\"\n\n";

   exit ( 1 );

}

_obs_lev_entry = _e;


_e = _m.find("fcst_lead");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_lead\"\n\n";

   exit ( 1 );

}

_fcst_lead_entry = _e;


_e = _m.find("fcst_init_hour");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_init_hour\"\n\n";

   exit ( 1 );

}

_fcst_init_hour_entry = _e;


_e = _m.find("fcst_accum");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_accum\"\n\n";

   exit ( 1 );

}

_fcst_accum_entry = _e;


_e = _m.find("obs_lead");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_lead\"\n\n";

   exit ( 1 );

}

_obs_lead_entry = _e;


_e = _m.find("obs_init_hour");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_init_hour\"\n\n";

   exit ( 1 );

}

_obs_init_hour_entry = _e;


_e = _m.find("obs_accum");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_accum\"\n\n";

   exit ( 1 );

}

_obs_accum_entry = _e;


_e = _m.find("fcst_rad");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_rad\"\n\n";

   exit ( 1 );

}

_fcst_rad_entry = _e;


_e = _m.find("obs_rad");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_rad\"\n\n";

   exit ( 1 );

}

_obs_rad_entry = _e;


_e = _m.find("area_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"area_min\"\n\n";

   exit ( 1 );

}

_area_min_entry = _e;


_e = _m.find("area_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"area_max\"\n\n";

   exit ( 1 );

}

_area_max_entry = _e;


_e = _m.find("area_filter_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"area_filter_min\"\n\n";

   exit ( 1 );

}

_area_filter_min_entry = _e;


_e = _m.find("area_filter_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"area_filter_max\"\n\n";

   exit ( 1 );

}

_area_filter_max_entry = _e;


_e = _m.find("area_thresh_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"area_thresh_min\"\n\n";

   exit ( 1 );

}

_area_thresh_min_entry = _e;


_e = _m.find("area_thresh_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"area_thresh_max\"\n\n";

   exit ( 1 );

}

_area_thresh_max_entry = _e;


_e = _m.find("intersection_area_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intersection_area_min\"\n\n";

   exit ( 1 );

}

_intersection_area_min_entry = _e;


_e = _m.find("intersection_area_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intersection_area_max\"\n\n";

   exit ( 1 );

}

_intersection_area_max_entry = _e;


_e = _m.find("union_area_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"union_area_min\"\n\n";

   exit ( 1 );

}

_union_area_min_entry = _e;


_e = _m.find("union_area_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"union_area_max\"\n\n";

   exit ( 1 );

}

_union_area_max_entry = _e;


_e = _m.find("symmetric_diff_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"symmetric_diff_min\"\n\n";

   exit ( 1 );

}

_symmetric_diff_min_entry = _e;


_e = _m.find("symmetric_diff_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"symmetric_diff_max\"\n\n";

   exit ( 1 );

}

_symmetric_diff_max_entry = _e;


_e = _m.find("fcst_valid_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_valid_min\"\n\n";

   exit ( 1 );

}

_fcst_valid_min_entry = _e;


_e = _m.find("fcst_valid_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_valid_max\"\n\n";

   exit ( 1 );

}

_fcst_valid_max_entry = _e;


_e = _m.find("obs_valid_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_valid_min\"\n\n";

   exit ( 1 );

}

_obs_valid_min_entry = _e;


_e = _m.find("obs_valid_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_valid_max\"\n\n";

   exit ( 1 );

}

_obs_valid_max_entry = _e;


_e = _m.find("fcst_init_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_init_min\"\n\n";

   exit ( 1 );

}

_fcst_init_min_entry = _e;


_e = _m.find("fcst_init_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_init_max\"\n\n";

   exit ( 1 );

}

_fcst_init_max_entry = _e;


_e = _m.find("obs_init_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_init_min\"\n\n";

   exit ( 1 );

}

_obs_init_min_entry = _e;


_e = _m.find("obs_init_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_init_max\"\n\n";

   exit ( 1 );

}

_obs_init_max_entry = _e;


_e = _m.find("centroid_x_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"centroid_x_min\"\n\n";

   exit ( 1 );

}

_centroid_x_min_entry = _e;


_e = _m.find("centroid_x_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"centroid_x_max\"\n\n";

   exit ( 1 );

}

_centroid_x_max_entry = _e;


_e = _m.find("centroid_y_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"centroid_y_min\"\n\n";

   exit ( 1 );

}

_centroid_y_min_entry = _e;


_e = _m.find("centroid_y_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"centroid_y_max\"\n\n";

   exit ( 1 );

}

_centroid_y_max_entry = _e;


_e = _m.find("centroid_lat_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"centroid_lat_min\"\n\n";

   exit ( 1 );

}

_centroid_lat_min_entry = _e;


_e = _m.find("centroid_lat_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"centroid_lat_max\"\n\n";

   exit ( 1 );

}

_centroid_lat_max_entry = _e;


_e = _m.find("centroid_lon_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"centroid_lon_min\"\n\n";

   exit ( 1 );

}

_centroid_lon_min_entry = _e;


_e = _m.find("centroid_lon_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"centroid_lon_max\"\n\n";

   exit ( 1 );

}

_centroid_lon_max_entry = _e;


_e = _m.find("axis_ang_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"axis_ang_min\"\n\n";

   exit ( 1 );

}

_axis_ang_min_entry = _e;


_e = _m.find("axis_ang_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"axis_ang_max\"\n\n";

   exit ( 1 );

}

_axis_ang_max_entry = _e;


_e = _m.find("length_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"length_min\"\n\n";

   exit ( 1 );

}

_length_min_entry = _e;


_e = _m.find("length_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"length_max\"\n\n";

   exit ( 1 );

}

_length_max_entry = _e;


_e = _m.find("width_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"width_min\"\n\n";

   exit ( 1 );

}

_width_min_entry = _e;


_e = _m.find("width_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"width_max\"\n\n";

   exit ( 1 );

}

_width_max_entry = _e;


_e = _m.find("aspect_ratio_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"aspect_ratio_min\"\n\n";

   exit ( 1 );

}

_aspect_ratio_min_entry = _e;


_e = _m.find("aspect_ratio_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"aspect_ratio_max\"\n\n";

   exit ( 1 );

}

_aspect_ratio_max_entry = _e;


_e = _m.find("curvature_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"curvature_min\"\n\n";

   exit ( 1 );

}

_curvature_min_entry = _e;


_e = _m.find("curvature_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"curvature_max\"\n\n";

   exit ( 1 );

}

_curvature_max_entry = _e;


_e = _m.find("curvature_x_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"curvature_x_min\"\n\n";

   exit ( 1 );

}

_curvature_x_min_entry = _e;


_e = _m.find("curvature_x_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"curvature_x_max\"\n\n";

   exit ( 1 );

}

_curvature_x_max_entry = _e;


_e = _m.find("curvature_y_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"curvature_y_min\"\n\n";

   exit ( 1 );

}

_curvature_y_min_entry = _e;


_e = _m.find("curvature_y_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"curvature_y_max\"\n\n";

   exit ( 1 );

}

_curvature_y_max_entry = _e;


_e = _m.find("complexity_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"complexity_min\"\n\n";

   exit ( 1 );

}

_complexity_min_entry = _e;


_e = _m.find("complexity_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"complexity_max\"\n\n";

   exit ( 1 );

}

_complexity_max_entry = _e;


_e = _m.find("intensity_10_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_10_min\"\n\n";

   exit ( 1 );

}

_intensity_10_min_entry = _e;


_e = _m.find("intensity_10_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_10_max\"\n\n";

   exit ( 1 );

}

_intensity_10_max_entry = _e;


_e = _m.find("intensity_25_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_25_min\"\n\n";

   exit ( 1 );

}

_intensity_25_min_entry = _e;


_e = _m.find("intensity_25_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_25_max\"\n\n";

   exit ( 1 );

}

_intensity_25_max_entry = _e;


_e = _m.find("intensity_50_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_50_min\"\n\n";

   exit ( 1 );

}

_intensity_50_min_entry = _e;


_e = _m.find("intensity_50_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_50_max\"\n\n";

   exit ( 1 );

}

_intensity_50_max_entry = _e;


_e = _m.find("intensity_75_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_75_min\"\n\n";

   exit ( 1 );

}

_intensity_75_min_entry = _e;


_e = _m.find("intensity_75_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_75_max\"\n\n";

   exit ( 1 );

}

_intensity_75_max_entry = _e;


_e = _m.find("intensity_90_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_90_min\"\n\n";

   exit ( 1 );

}

_intensity_90_min_entry = _e;


_e = _m.find("intensity_90_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_90_max\"\n\n";

   exit ( 1 );

}

_intensity_90_max_entry = _e;


_e = _m.find("intensity_user_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_user_min\"\n\n";

   exit ( 1 );

}

_intensity_user_min_entry = _e;


_e = _m.find("intensity_user_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_user_max\"\n\n";

   exit ( 1 );

}

_intensity_user_max_entry = _e;


_e = _m.find("intensity_sum_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_sum_min\"\n\n";

   exit ( 1 );

}

_intensity_sum_min_entry = _e;


_e = _m.find("intensity_sum_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intensity_sum_max\"\n\n";

   exit ( 1 );

}

_intensity_sum_max_entry = _e;


_e = _m.find("centroid_dist_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"centroid_dist_min\"\n\n";

   exit ( 1 );

}

_centroid_dist_min_entry = _e;


_e = _m.find("centroid_dist_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"centroid_dist_max\"\n\n";

   exit ( 1 );

}

_centroid_dist_max_entry = _e;


_e = _m.find("boundary_dist_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"boundary_dist_min\"\n\n";

   exit ( 1 );

}

_boundary_dist_min_entry = _e;


_e = _m.find("boundary_dist_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"boundary_dist_max\"\n\n";

   exit ( 1 );

}

_boundary_dist_max_entry = _e;


_e = _m.find("convex_hull_dist_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"convex_hull_dist_min\"\n\n";

   exit ( 1 );

}

_convex_hull_dist_min_entry = _e;


_e = _m.find("convex_hull_dist_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"convex_hull_dist_max\"\n\n";

   exit ( 1 );

}

_convex_hull_dist_max_entry = _e;


_e = _m.find("angle_diff_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"angle_diff_min\"\n\n";

   exit ( 1 );

}

_angle_diff_min_entry = _e;


_e = _m.find("angle_diff_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"angle_diff_max\"\n\n";

   exit ( 1 );

}

_angle_diff_max_entry = _e;


_e = _m.find("area_ratio_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"area_ratio_min\"\n\n";

   exit ( 1 );

}

_area_ratio_min_entry = _e;


_e = _m.find("area_ratio_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"area_ratio_max\"\n\n";

   exit ( 1 );

}

_area_ratio_max_entry = _e;


_e = _m.find("intersection_over_area_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intersection_over_area_min\"\n\n";

   exit ( 1 );

}

_intersection_over_area_min_entry = _e;


_e = _m.find("intersection_over_area_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"intersection_over_area_max\"\n\n";

   exit ( 1 );

}

_intersection_over_area_max_entry = _e;


_e = _m.find("complexity_ratio_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"complexity_ratio_min\"\n\n";

   exit ( 1 );

}

_complexity_ratio_min_entry = _e;


_e = _m.find("complexity_ratio_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"complexity_ratio_max\"\n\n";

   exit ( 1 );

}

_complexity_ratio_max_entry = _e;


_e = _m.find("percentile_intensity_ratio_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"percentile_intensity_ratio_min\"\n\n";

   exit ( 1 );

}

_percentile_intensity_ratio_min_entry = _e;


_e = _m.find("percentile_intensity_ratio_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"percentile_intensity_ratio_max\"\n\n";

   exit ( 1 );

}

_percentile_intensity_ratio_max_entry = _e;


_e = _m.find("interest_min");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"interest_min\"\n\n";

   exit ( 1 );

}

_interest_min_entry = _e;


_e = _m.find("interest_max");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"interest_max\"\n\n";

   exit ( 1 );

}

_interest_max_entry = _e;


_e = _m.find("version");

if ( !_e && Panic )  {

   cerr << "\n\n  mode_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"version\"\n\n";

   exit ( 1 );

}

_version_entry = _e;



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::fcst()

{

Result _temp_result;

if ( !_fcst_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::obs()

{

Result _temp_result;

if ( !_obs_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::single()

{

Result _temp_result;

if ( !_single_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_single_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::pair()

{

Result _temp_result;

if ( !_pair_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_pair_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::simple()

{

Result _temp_result;

if ( !_simple_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_simple_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::cluster()

{

Result _temp_result;

if ( !_cluster_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_cluster_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::matched()

{

Result _temp_result;

if ( !_matched_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_matched_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::unmatched()

{

Result _temp_result;

if ( !_unmatched_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_unmatched_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::model(int _i0)

{

Result _temp_result;

if ( !_model_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _model_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int mode_analysis_Conf::n_model_elements()

{

if ( !_model_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _model_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::fcst_thr(int _i0)

{

Result _temp_result;

if ( !_fcst_thr_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _fcst_thr_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int mode_analysis_Conf::n_fcst_thr_elements()

{

if ( !_fcst_thr_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _fcst_thr_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::obs_thr(int _i0)

{

Result _temp_result;

if ( !_obs_thr_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _obs_thr_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int mode_analysis_Conf::n_obs_thr_elements()

{

if ( !_obs_thr_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _obs_thr_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::fcst_var(int _i0)

{

Result _temp_result;

if ( !_fcst_var_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _fcst_var_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int mode_analysis_Conf::n_fcst_var_elements()

{

if ( !_fcst_var_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _fcst_var_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::fcst_lev(int _i0)

{

Result _temp_result;

if ( !_fcst_lev_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _fcst_lev_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int mode_analysis_Conf::n_fcst_lev_elements()

{

if ( !_fcst_lev_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _fcst_lev_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::obs_var(int _i0)

{

Result _temp_result;

if ( !_obs_var_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _obs_var_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int mode_analysis_Conf::n_obs_var_elements()

{

if ( !_obs_var_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _obs_var_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::obs_lev(int _i0)

{

Result _temp_result;

if ( !_obs_lev_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _obs_lev_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int mode_analysis_Conf::n_obs_lev_elements()

{

if ( !_obs_lev_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _obs_lev_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::fcst_lead(int _i0)

{

Result _temp_result;

if ( !_fcst_lead_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _fcst_lead_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int mode_analysis_Conf::n_fcst_lead_elements()

{

if ( !_fcst_lead_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _fcst_lead_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::fcst_init_hour(int _i0)

{

Result _temp_result;

if ( !_fcst_init_hour_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _fcst_init_hour_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int mode_analysis_Conf::n_fcst_init_hour_elements()

{

if ( !_fcst_init_hour_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _fcst_init_hour_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::fcst_accum(int _i0)

{

Result _temp_result;

if ( !_fcst_accum_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _fcst_accum_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int mode_analysis_Conf::n_fcst_accum_elements()

{

if ( !_fcst_accum_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _fcst_accum_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::obs_lead(int _i0)

{

Result _temp_result;

if ( !_obs_lead_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _obs_lead_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int mode_analysis_Conf::n_obs_lead_elements()

{

if ( !_obs_lead_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _obs_lead_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::obs_init_hour(int _i0)

{

Result _temp_result;

if ( !_obs_init_hour_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _obs_init_hour_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int mode_analysis_Conf::n_obs_init_hour_elements()

{

if ( !_obs_init_hour_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _obs_init_hour_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::obs_accum(int _i0)

{

Result _temp_result;

if ( !_obs_accum_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _obs_accum_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int mode_analysis_Conf::n_obs_accum_elements()

{

if ( !_obs_accum_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _obs_accum_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::fcst_rad(int _i0)

{

Result _temp_result;

if ( !_fcst_rad_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _fcst_rad_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int mode_analysis_Conf::n_fcst_rad_elements()

{

if ( !_fcst_rad_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _fcst_rad_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::obs_rad(int _i0)

{

Result _temp_result;

if ( !_obs_rad_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _obs_rad_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int mode_analysis_Conf::n_obs_rad_elements()

{

if ( !_obs_rad_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _obs_rad_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::area_min()

{

Result _temp_result;

if ( !_area_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_area_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::area_max()

{

Result _temp_result;

if ( !_area_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_area_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::area_filter_min()

{

Result _temp_result;

if ( !_area_filter_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_area_filter_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::area_filter_max()

{

Result _temp_result;

if ( !_area_filter_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_area_filter_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::area_thresh_min()

{

Result _temp_result;

if ( !_area_thresh_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_area_thresh_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::area_thresh_max()

{

Result _temp_result;

if ( !_area_thresh_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_area_thresh_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intersection_area_min()

{

Result _temp_result;

if ( !_intersection_area_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intersection_area_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intersection_area_max()

{

Result _temp_result;

if ( !_intersection_area_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intersection_area_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::union_area_min()

{

Result _temp_result;

if ( !_union_area_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_union_area_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::union_area_max()

{

Result _temp_result;

if ( !_union_area_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_union_area_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::symmetric_diff_min()

{

Result _temp_result;

if ( !_symmetric_diff_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_symmetric_diff_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::symmetric_diff_max()

{

Result _temp_result;

if ( !_symmetric_diff_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_symmetric_diff_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::fcst_valid_min()

{

Result _temp_result;

if ( !_fcst_valid_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_valid_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::fcst_valid_max()

{

Result _temp_result;

if ( !_fcst_valid_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_valid_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::obs_valid_min()

{

Result _temp_result;

if ( !_obs_valid_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_valid_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::obs_valid_max()

{

Result _temp_result;

if ( !_obs_valid_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_valid_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::fcst_init_min()

{

Result _temp_result;

if ( !_fcst_init_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_init_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::fcst_init_max()

{

Result _temp_result;

if ( !_fcst_init_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_init_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::obs_init_min()

{

Result _temp_result;

if ( !_obs_init_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_init_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::obs_init_max()

{

Result _temp_result;

if ( !_obs_init_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_init_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::centroid_x_min()

{

Result _temp_result;

if ( !_centroid_x_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_centroid_x_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::centroid_x_max()

{

Result _temp_result;

if ( !_centroid_x_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_centroid_x_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::centroid_y_min()

{

Result _temp_result;

if ( !_centroid_y_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_centroid_y_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::centroid_y_max()

{

Result _temp_result;

if ( !_centroid_y_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_centroid_y_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::centroid_lat_min()

{

Result _temp_result;

if ( !_centroid_lat_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_centroid_lat_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::centroid_lat_max()

{

Result _temp_result;

if ( !_centroid_lat_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_centroid_lat_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::centroid_lon_min()

{

Result _temp_result;

if ( !_centroid_lon_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_centroid_lon_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::centroid_lon_max()

{

Result _temp_result;

if ( !_centroid_lon_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_centroid_lon_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::axis_ang_min()

{

Result _temp_result;

if ( !_axis_ang_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_axis_ang_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::axis_ang_max()

{

Result _temp_result;

if ( !_axis_ang_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_axis_ang_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::length_min()

{

Result _temp_result;

if ( !_length_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_length_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::length_max()

{

Result _temp_result;

if ( !_length_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_length_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::width_min()

{

Result _temp_result;

if ( !_width_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_width_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::width_max()

{

Result _temp_result;

if ( !_width_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_width_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::aspect_ratio_min()

{

Result _temp_result;

if ( !_aspect_ratio_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_aspect_ratio_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::aspect_ratio_max()

{

Result _temp_result;

if ( !_aspect_ratio_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_aspect_ratio_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::curvature_min()

{

Result _temp_result;

if ( !_curvature_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_curvature_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::curvature_max()

{

Result _temp_result;

if ( !_curvature_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_curvature_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::curvature_x_min()

{

Result _temp_result;

if ( !_curvature_x_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_curvature_x_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::curvature_x_max()

{

Result _temp_result;

if ( !_curvature_x_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_curvature_x_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::curvature_y_min()

{

Result _temp_result;

if ( !_curvature_y_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_curvature_y_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::curvature_y_max()

{

Result _temp_result;

if ( !_curvature_y_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_curvature_y_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::complexity_min()

{

Result _temp_result;

if ( !_complexity_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_complexity_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::complexity_max()

{

Result _temp_result;

if ( !_complexity_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_complexity_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intensity_10_min()

{

Result _temp_result;

if ( !_intensity_10_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intensity_10_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intensity_10_max()

{

Result _temp_result;

if ( !_intensity_10_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intensity_10_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intensity_25_min()

{

Result _temp_result;

if ( !_intensity_25_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intensity_25_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intensity_25_max()

{

Result _temp_result;

if ( !_intensity_25_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intensity_25_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intensity_50_min()

{

Result _temp_result;

if ( !_intensity_50_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intensity_50_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intensity_50_max()

{

Result _temp_result;

if ( !_intensity_50_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intensity_50_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intensity_75_min()

{

Result _temp_result;

if ( !_intensity_75_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intensity_75_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intensity_75_max()

{

Result _temp_result;

if ( !_intensity_75_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intensity_75_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intensity_90_min()

{

Result _temp_result;

if ( !_intensity_90_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intensity_90_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intensity_90_max()

{

Result _temp_result;

if ( !_intensity_90_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intensity_90_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intensity_user_min()

{

Result _temp_result;

if ( !_intensity_user_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intensity_user_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intensity_user_max()

{

Result _temp_result;

if ( !_intensity_user_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intensity_user_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intensity_sum_min()

{

Result _temp_result;

if ( !_intensity_sum_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intensity_sum_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intensity_sum_max()

{

Result _temp_result;

if ( !_intensity_sum_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intensity_sum_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::centroid_dist_min()

{

Result _temp_result;

if ( !_centroid_dist_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_centroid_dist_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::centroid_dist_max()

{

Result _temp_result;

if ( !_centroid_dist_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_centroid_dist_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::boundary_dist_min()

{

Result _temp_result;

if ( !_boundary_dist_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_boundary_dist_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::boundary_dist_max()

{

Result _temp_result;

if ( !_boundary_dist_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_boundary_dist_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::convex_hull_dist_min()

{

Result _temp_result;

if ( !_convex_hull_dist_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_convex_hull_dist_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::convex_hull_dist_max()

{

Result _temp_result;

if ( !_convex_hull_dist_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_convex_hull_dist_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::angle_diff_min()

{

Result _temp_result;

if ( !_angle_diff_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_angle_diff_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::angle_diff_max()

{

Result _temp_result;

if ( !_angle_diff_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_angle_diff_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::area_ratio_min()

{

Result _temp_result;

if ( !_area_ratio_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_area_ratio_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::area_ratio_max()

{

Result _temp_result;

if ( !_area_ratio_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_area_ratio_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intersection_over_area_min()

{

Result _temp_result;

if ( !_intersection_over_area_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intersection_over_area_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::intersection_over_area_max()

{

Result _temp_result;

if ( !_intersection_over_area_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_intersection_over_area_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::complexity_ratio_min()

{

Result _temp_result;

if ( !_complexity_ratio_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_complexity_ratio_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::complexity_ratio_max()

{

Result _temp_result;

if ( !_complexity_ratio_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_complexity_ratio_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::percentile_intensity_ratio_min()

{

Result _temp_result;

if ( !_percentile_intensity_ratio_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_percentile_intensity_ratio_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::percentile_intensity_ratio_max()

{

Result _temp_result;

if ( !_percentile_intensity_ratio_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_percentile_intensity_ratio_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::interest_min()

{

Result _temp_result;

if ( !_interest_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_interest_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::interest_max()

{

Result _temp_result;

if ( !_interest_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_interest_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result mode_analysis_Conf::version()

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


