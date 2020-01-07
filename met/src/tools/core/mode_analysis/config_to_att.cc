// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   config_to_att.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    01/01/08  Bullock         New
//   001    10/03/08  Halley Gotway   Add support for:
//                                    AREA_THRESH column,
//                                    apect_ratio min/max options,
//                                    fcst/obs init_time min/max options,
//                                    fcst/obs init_hour options
//   002    05/21/12  Halley Gotway   Add support for:
//                                    fcst/obs valid_hour options
//
////////////////////////////////////////////////////////////////////////

using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>


#include "config_to_att.h"
#include "is_bad_data.h"

////////////////////////////////////////////////////////////////////////


static const bool error_out = false;


////////////////////////////////////////////////////////////////////////


void config_to_att(MetConfig & config, ModeAttributes & atts)

{

int      v_int;
double   v_dbl;
unixtime v_ut;

   //
   //  dump the contents of the config file
   //
if(mlog.verbosity_level() >= 5) config.dump(cout);

   //
   //  initialize
   //

atts.clear();

   //
   //  toggle members
   //

if ( config.lookup_bool(conf_key_fcst, error_out) )
   atts.set_fcst();

if ( config.lookup_bool(conf_key_obs, error_out) )
   atts.set_obs();

if ( config.lookup_bool(conf_key_single, error_out) )
   atts.set_single();

if ( config.lookup_bool(conf_key_pair, error_out) )
   atts.set_pair();

if ( config.lookup_bool(conf_key_simple, error_out) )
   atts.set_simple();

if ( config.lookup_bool(conf_key_cluster, error_out) )
   atts.set_cluster();

if ( config.lookup_bool(conf_key_matched, error_out) )
   atts.set_matched();

if ( config.lookup_bool(conf_key_unmatched, error_out) )
   atts.set_unmatched();

   //
   //  string array members
   //

atts.model.add( config.lookup_string_array(conf_key_model, error_out) );

atts.fcst_thr.add( config.lookup_string_array(conf_key_fcst_thr, error_out) );

atts.obs_thr.add( config.lookup_string_array(conf_key_obs_thr, error_out) );

atts.fcst_var.add( config.lookup_string_array(conf_key_fcst_var, error_out) );

atts.fcst_units.add( config.lookup_string_array(conf_key_fcst_units, error_out) );

atts.fcst_lev.add( config.lookup_string_array(conf_key_fcst_lev, error_out) );

atts.obs_var.add( config.lookup_string_array(conf_key_obs_var, error_out) );

atts.obs_units.add( config.lookup_string_array(conf_key_obs_units, error_out) );

atts.obs_lev.add( config.lookup_string_array(conf_key_obs_lev, error_out) );

   //
   //  int array members
   //

atts.fcst_lead.add( config.lookup_seconds_array(conf_key_fcst_lead, error_out) );

atts.fcst_valid_hour.add( config.lookup_seconds_array(conf_key_fcst_valid_hour, error_out) );

atts.fcst_init_hour.add( config.lookup_seconds_array(conf_key_fcst_init_hour, error_out) );

atts.fcst_accum.add( config.lookup_seconds_array(conf_key_fcst_accum, error_out) );

atts.obs_lead.add( config.lookup_seconds_array(conf_key_obs_lead, error_out) );

atts.obs_valid_hour.add( config.lookup_seconds_array(conf_key_obs_valid_hour, error_out) );

atts.obs_init_hour.add( config.lookup_seconds_array(conf_key_obs_init_hour, error_out) );

atts.obs_accum.add( config.lookup_seconds_array(conf_key_obs_accum, error_out) );

atts.fcst_rad.add( config.lookup_int_array(conf_key_fcst_rad, error_out) );

atts.obs_rad.add( config.lookup_int_array(conf_key_obs_rad, error_out) );

   //
   //  int maxmin members
   //

if ( !is_bad_data(v_int = config.lookup_int(conf_key_area_min, error_out)) )
   atts.set_area_min(v_int);

if ( !is_bad_data(v_int = config.lookup_int(conf_key_area_max, error_out)) )
   atts.set_area_max(v_int);

if ( !is_bad_data(v_int = config.lookup_int(conf_key_area_thresh_min, error_out)) )
   atts.set_area_thresh_min(v_int);

if ( !is_bad_data(v_int = config.lookup_int(conf_key_area_thresh_max, error_out)) )
   atts.set_area_thresh_max(v_int);

if ( !is_bad_data(v_int = config.lookup_int(conf_key_intersection_area_min, error_out)) )
   atts.set_intersection_area_min(v_int);

if ( !is_bad_data(v_int = config.lookup_int(conf_key_intersection_area_max, error_out)) )
   atts.set_intersection_area_max(v_int);

if ( !is_bad_data(v_int = config.lookup_int(conf_key_union_area_min, error_out)) )
   atts.set_union_area_min(v_int);

if ( !is_bad_data(v_int = config.lookup_int(conf_key_union_area_max, error_out)) )
   atts.set_union_area_max(v_int);

if ( !is_bad_data(v_int = config.lookup_int(conf_key_symmetric_diff_min, error_out)) )
   atts.set_symmetric_diff_min(v_int);

if ( !is_bad_data(v_int = config.lookup_int(conf_key_symmetric_diff_max, error_out)) )
   atts.set_symmetric_diff_max(v_int);

   //
   //  unixtime maxmin members
   //

if ( (v_ut = config.lookup_unixtime(conf_key_fcst_valid_min, error_out)) > 0 )
   atts.set_fcst_valid_min(v_ut);

if ( (v_ut = config.lookup_unixtime(conf_key_fcst_valid_max, error_out)) > 0 )
   atts.set_fcst_valid_max(v_ut);

if ( (v_ut = config.lookup_unixtime(conf_key_obs_valid_min, error_out)) > 0 )
   atts.set_obs_valid_min(v_ut);

if ( (v_ut = config.lookup_unixtime(conf_key_obs_valid_max, error_out)) > 0 )
   atts.set_obs_valid_max(v_ut);

if ( (v_ut = config.lookup_unixtime(conf_key_fcst_init_min, error_out)) > 0 )
   atts.set_fcst_init_min(v_ut);

if ( (v_ut = config.lookup_unixtime(conf_key_fcst_init_max, error_out)) > 0 )
   atts.set_fcst_init_max(v_ut);

if ( (v_ut = config.lookup_unixtime(conf_key_obs_init_min, error_out)) > 0 )
   atts.set_obs_init_min(v_ut);

if ( (v_ut = config.lookup_unixtime(conf_key_obs_init_max, error_out)) > 0 )
   atts.set_obs_init_max(v_ut);

   //
   //  double maxmin members
   //

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_centroid_x_min, error_out)) )
   atts.set_centroid_x_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_centroid_x_max, error_out)) )
   atts.set_centroid_x_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_centroid_y_min, error_out)) )
   atts.set_centroid_y_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_centroid_y_max, error_out)) )
   atts.set_centroid_y_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_centroid_lat_min, error_out)) )
   atts.set_centroid_lat_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_centroid_lat_max, error_out)) )
   atts.set_centroid_lat_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_centroid_lon_min, error_out)) )
   atts.set_centroid_lon_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_centroid_lon_max, error_out)) )
   atts.set_centroid_lon_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_axis_ang_min, error_out)) )
   atts.set_axis_ang_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_axis_ang_max, error_out)) )
   atts.set_axis_ang_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_length_min, error_out)) )
   atts.set_length_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_length_max, error_out)) )
   atts.set_length_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_width_min, error_out)) )
   atts.set_width_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_width_max, error_out)) )
   atts.set_width_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_aspect_ratio_min, error_out)) )
   atts.set_aspect_ratio_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_aspect_ratio_max, error_out)) )
   atts.set_aspect_ratio_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_curvature_min, error_out)) )
   atts.set_curvature_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_curvature_max, error_out)) )
   atts.set_curvature_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_curvature_x_min, error_out)) )
   atts.set_curvature_x_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_curvature_x_max, error_out)) )
   atts.set_curvature_x_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_curvature_y_min, error_out)) )
   atts.set_curvature_y_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_curvature_y_max, error_out)) )
   atts.set_curvature_y_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_complexity_min, error_out)) )
   atts.set_complexity_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_complexity_max, error_out)) )
   atts.set_complexity_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_intensity_10_min, error_out)) )
   atts.set_intensity_10_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_intensity_10_max, error_out)) )
   atts.set_intensity_10_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_intensity_25_min, error_out)) )
   atts.set_intensity_25_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_intensity_25_max, error_out)) )
   atts.set_intensity_25_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_intensity_50_min, error_out)) )
   atts.set_intensity_50_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_intensity_50_max, error_out)) )
   atts.set_intensity_50_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_intensity_75_min, error_out)) )
   atts.set_intensity_75_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_intensity_75_max, error_out)) )
   atts.set_intensity_75_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_intensity_90_min, error_out)) )
   atts.set_intensity_90_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_intensity_90_max, error_out)) )
   atts.set_intensity_90_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_intensity_user_min, error_out)) )
   atts.set_intensity_user_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_intensity_user_max, error_out)) )
   atts.set_intensity_user_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_intensity_sum_min, error_out)) )
   atts.set_intensity_sum_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_intensity_sum_max, error_out)) )
   atts.set_intensity_sum_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_centroid_dist_min, error_out)) )
   atts.set_centroid_dist_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_centroid_dist_max, error_out)) )
   atts.set_centroid_dist_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_boundary_dist_min, error_out)) )
   atts.set_boundary_dist_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_boundary_dist_max, error_out)) )
   atts.set_boundary_dist_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_convex_hull_dist_min, error_out)) )
   atts.set_convex_hull_dist_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_convex_hull_dist_max, error_out)) )
   atts.set_convex_hull_dist_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_angle_diff_min, error_out)) )
   atts.set_angle_diff_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_angle_diff_max, error_out)) )
   atts.set_angle_diff_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_aspect_diff_min, error_out)) )
   atts.set_aspect_diff_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_aspect_diff_max, error_out)) )
   atts.set_aspect_diff_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_area_ratio_min, error_out)) )
   atts.set_area_ratio_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_area_ratio_max, error_out)) )
   atts.set_area_ratio_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_intersection_over_area_min, error_out)) )
   atts.set_intersection_over_area_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_intersection_over_area_max, error_out)) )
   atts.set_intersection_over_area_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_curvature_ratio_min, error_out)) )
   atts.set_curvature_ratio_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_curvature_ratio_max, error_out)) )
   atts.set_curvature_ratio_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_complexity_ratio_min, error_out)) )
   atts.set_complexity_ratio_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_complexity_ratio_max, error_out)) )
   atts.set_complexity_ratio_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_percentile_intensity_ratio_min, error_out)) )
   atts.set_percentile_intensity_ratio_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_percentile_intensity_ratio_max, error_out)) )
   atts.set_percentile_intensity_ratio_max(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_interest_min, error_out)) )
   atts.set_interest_min(v_dbl);

if ( !is_bad_data(v_dbl = config.lookup_double(conf_key_interest_max, error_out)) )
   atts.set_interest_max(v_dbl);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


