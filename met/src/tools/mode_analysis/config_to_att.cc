// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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
//
////////////////////////////////////////////////////////////////////////

using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>


#include "config_to_att.h"


////////////////////////////////////////////////////////////////////////


void config_to_att(mode_analysis_Conf & config, ModeAttributes & atts)

{

int k, n, s;
unixtime t;
Result r;


atts.clear();


   //
   //  toggle members
   //

r = config.fcst();
if ( r.type() != no_result_type )  {
   if ( r.ival() )  atts.set_fcst();
}

r = config.obs();
if ( r.type() != no_result_type )  {
   if ( r.ival() )  atts.set_obs();
}

r = config.single();
if ( r.type() != no_result_type )  {
   if ( r.ival() )  atts.set_single();
}

r = config.pair();
if ( r.type() != no_result_type )  {
   if ( r.ival() )  atts.set_pair();
}

r = config.simple();
if ( r.type() != no_result_type )  {
   if ( r.ival() )  atts.set_simple();
}

r = config.cluster();
if ( r.type() != no_result_type )  {
   if ( r.ival() )  atts.set_cluster();
}

r = config.matched();
if ( r.type() != no_result_type )  {
   if ( r.ival() )  atts.set_matched();
}

r = config.unmatched();
if ( r.type() != no_result_type )  {
   if ( r.ival() )  atts.set_unmatched();
}


   //
   //  string array members
   //

n = config.n_model_elements();
if ( n > 0 )  {
   for (k=0; k<n; ++k)  {
      r = config.model(k);
      atts.model.add(r.sval());
   }
}

n = config.n_fcst_thr_elements();
if ( n > 0 )  {
   for (k=0; k<n; ++k)  {
      r = config.fcst_thr(k);
      atts.fcst_thr.add(r.sval());
   }
}

n = config.n_obs_thr_elements();
if ( n > 0 )  {
   for (k=0; k<n; ++k)  {
      r = config.obs_thr(k);
      atts.obs_thr.add(r.sval());
   }
}

n = config.n_fcst_var_elements();
if ( n > 0 )  {
   for (k=0; k<n; ++k)  {
      r = config.fcst_var(k);
      atts.fcst_var.add(r.sval());
   }
}

n = config.n_fcst_lev_elements();
if ( n > 0 )  {
   for (k=0; k<n; ++k)  {
      r = config.fcst_lev(k);
      atts.fcst_lev.add(r.sval());
   }
}

n = config.n_obs_var_elements();
if ( n > 0 )  {
   for (k=0; k<n; ++k)  {
      r = config.obs_var(k);
      atts.obs_var.add(r.sval());
   }
}

n = config.n_obs_lev_elements();
if ( n > 0 )  {
   for (k=0; k<n; ++k)  {
      r = config.obs_lev(k);
      atts.obs_lev.add(r.sval());
   }
}


   //
   //  int array members
   //

n = config.n_fcst_lead_elements();
if ( n > 0 )  {
   for (k=0; k<n; ++k)  {
      r = config.fcst_lead(k);
      s = timestring_to_sec(r.sval());
      atts.fcst_lead.add(s);
   }
}

n = config.n_fcst_init_hour_elements();
if ( n > 0 )  {
   for (k=0; k<n; ++k)  {
      r = config.fcst_init_hour(k);
      s = timestring_to_sec(r.sval());
      atts.fcst_init_hour.add(s);
   }
}

n = config.n_fcst_accum_elements();
if ( n > 0 )  {
   for (k=0; k<n; ++k)  {
      r = config.fcst_accum(k);
      s = timestring_to_sec(r.sval());
      atts.fcst_accum.add(s);
   }
}

n = config.n_obs_lead_elements();
if ( n > 0 )  {
   for (k=0; k<n; ++k)  {
      r = config.obs_lead(k);
      s = timestring_to_sec(r.sval());
      atts.obs_lead.add(s);
   }
}

n = config.n_obs_init_hour_elements();
if ( n > 0 )  {
   for (k=0; k<n; ++k)  {
      r = config.obs_init_hour(k);
      s = timestring_to_sec(r.sval());
      atts.obs_init_hour.add(s);
   }
}

n = config.n_obs_accum_elements();
if ( n > 0 )  {
   for (k=0; k<n; ++k)  {
      r = config.obs_accum(k);
      s = timestring_to_sec(r.sval());
      atts.obs_accum.add(s);
   }
}

n = config.n_fcst_rad_elements();
if ( n > 0 )  {
   for (k=0; k<n; ++k)  {
      r = config.fcst_rad(k);
      atts.fcst_rad.add(r.ival());
   }
}

n = config.n_obs_rad_elements();
if ( n > 0 )  {
   for (k=0; k<n; ++k)  {
      r = config.obs_rad(k);
      atts.obs_rad.add(r.ival());
   }
}


   //
   //  int maxmin members
   //

r = config.area_min();
if ( r.type() != no_result_type )  {
   atts.set_area_min(r.ival());
}

r = config.area_max();
if ( r.type() != no_result_type )  {
   atts.set_area_max(r.ival());
}

r = config.area_filter_min();
if ( r.type() != no_result_type )  {
   atts.set_area_filter_min(r.ival());
}

r = config.area_filter_max();
if ( r.type() != no_result_type )  {
   atts.set_area_filter_max(r.ival());
}

r = config.area_thresh_min();
if ( r.type() != no_result_type )  {
   atts.set_area_thresh_min(r.ival());
}

r = config.area_thresh_max();
if ( r.type() != no_result_type )  {
   atts.set_area_thresh_max(r.ival());
}

r = config.intersection_area_min();
if ( r.type() != no_result_type )  {
   atts.set_intersection_area_min(r.ival());
}

r = config.intersection_area_max();
if ( r.type() != no_result_type )  {
   atts.set_intersection_area_max(r.ival());
}

r = config.union_area_min();
if ( r.type() != no_result_type )  {
   atts.set_union_area_min(r.ival());
}

r = config.union_area_max();
if ( r.type() != no_result_type )  {
   atts.set_union_area_max(r.ival());
}

r = config.symmetric_diff_min();
if ( r.type() != no_result_type )  {
   atts.set_symmetric_diff_min(r.ival());
}

r = config.symmetric_diff_max();
if ( r.type() != no_result_type )  {
   atts.set_symmetric_diff_max(r.ival());
}


   //
   //  unixtime maxmin members
   //

r = config.fcst_valid_min();
if ( (r.type() != no_result_type) && (strlen(r.sval()) > 0 ) )  {
   t = timestring_to_unix(r.sval());
   atts.set_fcst_valid_min(t);
}

r = config.fcst_valid_max();
if ( (r.type() != no_result_type) && (strlen(r.sval()) > 0 ) )  {
   t = timestring_to_unix(r.sval());
   atts.set_fcst_valid_max(t);
}

r = config.obs_valid_min();
if ( (r.type() != no_result_type) && (strlen(r.sval()) > 0 ) )  {
   t = timestring_to_unix(r.sval());
   atts.set_obs_valid_min(t);
}

r = config.obs_valid_max();
if ( (r.type() != no_result_type) && (strlen(r.sval()) > 0 ) )  {
   t = timestring_to_unix(r.sval());
   atts.set_obs_valid_max(t);
}

r = config.fcst_init_min();
if ( (r.type() != no_result_type) && (strlen(r.sval()) > 0 ) )  {
   t = timestring_to_unix(r.sval());
   atts.set_fcst_init_min(t);
}

r = config.fcst_init_max();
if ( (r.type() != no_result_type) && (strlen(r.sval()) > 0 ) )  {
   t = timestring_to_unix(r.sval());
   atts.set_fcst_init_max(t);
}

r = config.obs_init_min();
if ( (r.type() != no_result_type) && (strlen(r.sval()) > 0 ) )  {
   t = timestring_to_unix(r.sval());
   atts.set_obs_init_min(t);
}

r = config.obs_init_max();
if ( (r.type() != no_result_type) && (strlen(r.sval()) > 0 ) )  {
   t = timestring_to_unix(r.sval());
   atts.set_obs_init_max(t);
}


   //
   //  double maxmin members
   //

r = config.centroid_x_min();
if ( r.type() != no_result_type )  {
   atts.set_centroid_x_min(r.dval());
}

r = config.centroid_x_max();
if ( r.type() != no_result_type )  {
   atts.set_centroid_x_max(r.dval());
}

r = config.centroid_y_min();
if ( r.type() != no_result_type )  {
   atts.set_centroid_y_min(r.dval());
}

r = config.centroid_y_max();
if ( r.type() != no_result_type )  {
   atts.set_centroid_y_max(r.dval());
}

r = config.centroid_lat_min();
if ( r.type() != no_result_type )  {
   atts.set_centroid_lat_min(r.dval());
}

r = config.centroid_lat_max();
if ( r.type() != no_result_type )  {
   atts.set_centroid_lat_max(r.dval());
}

r = config.centroid_lon_min();
if ( r.type() != no_result_type )  {
   atts.set_centroid_lon_min(r.dval());
}

r = config.centroid_lon_max();
if ( r.type() != no_result_type )  {
   atts.set_centroid_lon_max(r.dval());
}

r = config.axis_ang_min();
if ( r.type() != no_result_type )  {
   atts.set_axis_ang_min(r.dval());
}

r = config.axis_ang_max();
if ( r.type() != no_result_type )  {
   atts.set_axis_ang_max(r.dval());
}

r = config.length_min();
if ( r.type() != no_result_type )  {
   atts.set_length_min(r.dval());
}

r = config.length_max();
if ( r.type() != no_result_type )  {
   atts.set_length_max(r.dval());
}

r = config.width_min();
if ( r.type() != no_result_type )  {
   atts.set_width_min(r.dval());
}

r = config.width_max();
if ( r.type() != no_result_type )  {
   atts.set_width_max(r.dval());
}

r = config.aspect_ratio_min();
if ( r.type() != no_result_type )  {
   atts.set_aspect_ratio_min(r.dval());
}

r = config.aspect_ratio_max();
if ( r.type() != no_result_type )  {
   atts.set_aspect_ratio_max(r.dval());
}

r = config.curvature_min();
if ( r.type() != no_result_type )  {
   atts.set_curvature_min(r.dval());
}

r = config.curvature_max();
if ( r.type() != no_result_type )  {
   atts.set_curvature_max(r.dval());
}

r = config.curvature_x_min();
if ( r.type() != no_result_type )  {
   atts.set_curvature_x_min(r.dval());
}

r = config.curvature_x_max();
if ( r.type() != no_result_type )  {
   atts.set_curvature_x_max(r.dval());
}

r = config.curvature_y_min();
if ( r.type() != no_result_type )  {
   atts.set_curvature_y_min(r.dval());
}

r = config.curvature_y_max();
if ( r.type() != no_result_type )  {
   atts.set_curvature_y_max(r.dval());
}

r = config.complexity_min();
if ( r.type() != no_result_type )  {
   atts.set_complexity_min(r.dval());
}

r = config.complexity_max();
if ( r.type() != no_result_type )  {
   atts.set_complexity_max(r.dval());
}

r = config.intensity_10_min();
if ( r.type() != no_result_type )  {
   atts.set_intensity_10_min(r.dval());
}

r = config.intensity_10_max();
if ( r.type() != no_result_type )  {
   atts.set_intensity_10_max(r.dval());
}

r = config.intensity_25_min();
if ( r.type() != no_result_type )  {
   atts.set_intensity_25_min(r.dval());
}

r = config.intensity_25_max();
if ( r.type() != no_result_type )  {
   atts.set_intensity_25_max(r.dval());
}

r = config.intensity_50_min();
if ( r.type() != no_result_type )  {
   atts.set_intensity_50_min(r.dval());
}

r = config.intensity_50_max();
if ( r.type() != no_result_type )  {
   atts.set_intensity_50_max(r.dval());
}

r = config.intensity_75_min();
if ( r.type() != no_result_type )  {
   atts.set_intensity_75_min(r.dval());
}

r = config.intensity_75_max();
if ( r.type() != no_result_type )  {
   atts.set_intensity_75_max(r.dval());
}

r = config.intensity_90_min();
if ( r.type() != no_result_type )  {
   atts.set_intensity_90_min(r.dval());
}

r = config.intensity_90_max();
if ( r.type() != no_result_type )  {
   atts.set_intensity_90_max(r.dval());
}

r = config.intensity_user_min();
if ( r.type() != no_result_type )  {
   atts.set_intensity_user_min(r.dval());
}

r = config.intensity_user_max();
if ( r.type() != no_result_type )  {
   atts.set_intensity_user_max(r.dval());
}

r = config.intensity_sum_min();
if ( r.type() != no_result_type )  {
   atts.set_intensity_sum_min(r.dval());
}

r = config.intensity_sum_max();
if ( r.type() != no_result_type )  {
   atts.set_intensity_sum_max(r.dval());
}

r = config.centroid_dist_min();
if ( r.type() != no_result_type )  {
   atts.set_centroid_dist_min(r.dval());
}

r = config.centroid_dist_max();
if ( r.type() != no_result_type )  {
   atts.set_centroid_dist_max(r.dval());
}

r = config.boundary_dist_min();
if ( r.type() != no_result_type )  {
   atts.set_boundary_dist_min(r.dval());
}

r = config.boundary_dist_max();
if ( r.type() != no_result_type )  {
   atts.set_boundary_dist_max(r.dval());
}

r = config.convex_hull_dist_min();
if ( r.type() != no_result_type )  {
   atts.set_convex_hull_dist_min(r.dval());
}

r = config.convex_hull_dist_max();
if ( r.type() != no_result_type )  {
   atts.set_convex_hull_dist_max(r.dval());
}

r = config.angle_diff_min();
if ( r.type() != no_result_type )  {
   atts.set_angle_diff_min(r.dval());
}

r = config.angle_diff_max();
if ( r.type() != no_result_type )  {
   atts.set_angle_diff_max(r.dval());
}

r = config.area_ratio_min();
if ( r.type() != no_result_type )  {
   atts.set_area_ratio_min(r.dval());
}

r = config.area_ratio_max();
if ( r.type() != no_result_type )  {
   atts.set_area_ratio_max(r.dval());
}

r = config.intersection_over_area_min();
if ( r.type() != no_result_type )  {
   atts.set_intersection_over_area_min(r.dval());
}

r = config.intersection_over_area_max();
if ( r.type() != no_result_type )  {
   atts.set_intersection_over_area_max(r.dval());
}

r = config.complexity_ratio_min();
if ( r.type() != no_result_type )  {
   atts.set_complexity_ratio_min(r.dval());
}

r = config.complexity_ratio_max();
if ( r.type() != no_result_type )  {
   atts.set_complexity_ratio_max(r.dval());
}

r = config.percentile_intensity_ratio_min();
if ( r.type() != no_result_type )  {
   atts.set_percentile_intensity_ratio_min(r.dval());
}

r = config.percentile_intensity_ratio_max();
if ( r.type() != no_result_type )  {
   atts.set_percentile_intensity_ratio_max(r.dval());
}

r = config.interest_min();
if ( r.type() != no_result_type )  {
   atts.set_interest_min(r.dval());
}

r = config.interest_max();
if ( r.type() != no_result_type )  {
   atts.set_interest_max(r.dval());
}



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


