// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "mode_atts.h"
#include "analysis_utils.h"

#include "vx_math.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ModeAttributes
   //


////////////////////////////////////////////////////////////////////////


ModeAttributes::ModeAttributes()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ModeAttributes::~ModeAttributes()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ModeAttributes::ModeAttributes(const ModeAttributes & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


ModeAttributes & ModeAttributes::operator=(const ModeAttributes & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::init_from_scratch()

{

poly = (MaskPoly *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::clear()

{

   //
   // toggles
   //

is_fcst_toggle_set    = 0;
is_fcst               = 0;

is_single_toggle_set  = 0;
is_single             = 0;

is_simple_toggle_set  = 0;
is_simple             = 0;

is_matched_toggle_set = 0;
is_matched            = 0;


   //
   // string array members
   //

   model.clear();
    desc.clear();
fcst_thr.clear();
 obs_thr.clear();
fcst_var.clear();
fcst_units.clear();
fcst_lev.clear();
 obs_var.clear();
 obs_units.clear();
 obs_lev.clear();

   //
   // int array members
   //

      fcst_lead.clear();
fcst_valid_hour.clear();
 fcst_init_hour.clear();
     fcst_accum.clear();
       obs_lead.clear();
 obs_valid_hour.clear();
  obs_init_hour.clear();
      obs_accum.clear();
       fcst_rad.clear();
        obs_rad.clear();

   //
   // unixtime max/min members
   //

fcst_valid_min_set  = 0;
fcst_valid_min      = (unixtime) 0;

fcst_valid_max_set  = 0;
fcst_valid_max      = (unixtime) 0;

obs_valid_min_set   = 0;
obs_valid_min       = (unixtime) 0;

obs_valid_max_set   = 0;
obs_valid_max       = (unixtime) 0;

fcst_init_min_set   = 0;
fcst_init_min       = (unixtime) 0;

fcst_init_max_set   = 0;
fcst_init_max       = (unixtime) 0;

obs_init_min_set    = 0;
obs_init_min        = (unixtime) 0;

obs_init_max_set    = 0;
obs_init_max        = (unixtime) 0;

   //
   // int max/min members
   //

area_min_set               = 0;
area_min                   = 0;

area_max_set               = 0;
area_max                   = 0;

area_thresh_min_set        = 0;
area_thresh_min            = 0;

area_thresh_max_set        = 0;
area_thresh_max            = 0;

intersection_area_min_set  = 0;
intersection_area_min      = 0;

intersection_area_max_set  = 0;
intersection_area_max      = 0;

union_area_min_set         = 0;
union_area_min             = 0;

union_area_max_set         = 0;
union_area_max             = 0;

symmetric_diff_min_set     = 0;
symmetric_diff_min         = 0;

symmetric_diff_max_set     = 0;
symmetric_diff_max         = 0;


   //
   // double max/min members
   //

centroid_x_min_set                  = 0;
centroid_x_min                      = 0.0;

centroid_x_max_set                  = 0;
centroid_x_max                      = 0.0;

centroid_y_min_set                  = 0;
centroid_y_min                      = 0.0;

centroid_y_max_set                  = 0;
centroid_y_max                      = 0.0;

centroid_lat_min_set                = 0;
centroid_lat_min                    = 0.0;

centroid_lat_max_set                = 0;
centroid_lat_max                    = 0.0;

centroid_lon_min_set                = 0;
centroid_lon_min                    = 0.0;

centroid_lon_max_set                = 0;
centroid_lon_max                    = 0.0;

axis_ang_min_set                    = 0;
axis_ang_min                        = 0.0;

axis_ang_max_set                    = 0;
axis_ang_max                        = 0.0;

length_min_set                      = 0;
length_min                          = 0.0;

length_max_set                      = 0;
length_max                          = 0.0;

width_min_set                       = 0;
width_min                           = 0.0;

width_max_set                       = 0;
width_max                           = 0.0;

aspect_ratio_min_set                = 0;
aspect_ratio_min                    = 0.0;

aspect_ratio_max_set                = 0;
aspect_ratio_max                    = 0.0;

curvature_min_set                   = 0;
curvature_min                       = 0.0;

curvature_max_set                   = 0;
curvature_max                       = 0.0;

curvature_x_min_set                 = 0;
curvature_x_min                     = 0.0;

curvature_x_max_set                 = 0;
curvature_x_max                     = 0.0;

curvature_y_min_set                 = 0;
curvature_y_min                     = 0.0;

curvature_y_max_set                 = 0;
curvature_y_max                     = 0.0;

complexity_min_set                  = 0;
complexity_min                      = 0.0;

complexity_max_set                  = 0;
complexity_max                      = 0.0;

intensity_10_min_set                = 0;
intensity_10_min                    = 0.0;

intensity_10_max_set                = 0;
intensity_10_max                    = 0.0;

intensity_25_min_set                = 0;
intensity_25_min                    = 0.0;

intensity_25_max_set                = 0;
intensity_25_max                    = 0.0;

intensity_50_min_set                = 0;
intensity_50_min                    = 0.0;

intensity_50_max_set                = 0;
intensity_50_max                    = 0.0;

intensity_75_min_set                = 0;
intensity_75_min                    = 0.0;

intensity_75_max_set                = 0;
intensity_75_max                    = 0.0;

intensity_90_min_set                = 0;
intensity_90_min                    = 0.0;

intensity_90_max_set                = 0;
intensity_90_max                    = 0.0;

intensity_user_min_set              = 0;
intensity_user_min                  = 0.0;

intensity_user_max_set              = 0;
intensity_user_max                  = 0.0;

intensity_sum_min_set               = 0;
intensity_sum_min                   = 0.0;

intensity_sum_max_set               = 0;
intensity_sum_max                   = 0.0;

centroid_dist_min_set               = 0;
centroid_dist_min                   = 0.0;

centroid_dist_max_set               = 0;
centroid_dist_max                   = 0.0;

boundary_dist_min_set               = 0;
boundary_dist_min                   = 0.0;

boundary_dist_max_set               = 0;
boundary_dist_max                   = 0.0;

convex_hull_dist_min_set            = 0;
convex_hull_dist_min                = 0.0;

convex_hull_dist_max_set            = 0;
convex_hull_dist_max                = 0.0;

angle_diff_min_set                  = 0;
angle_diff_min                      = 0.0;

angle_diff_max_set                  = 0;
angle_diff_max                      = 0.0;

aspect_diff_min_set                 = 0;
aspect_diff_min                     = 0.0;

aspect_diff_max_set                 = 0;
aspect_diff_max                     = 0.0;

area_ratio_min_set                  = 0;
area_ratio_min                      = 0.0;

area_ratio_max_set                  = 0;
area_ratio_max                      = 0.0;

intersection_over_area_min_set      = 0;
intersection_over_area_min          = 0.0;

intersection_over_area_max_set      = 0;
intersection_over_area_max          = 0.0;

curvature_ratio_min_set             = 0;
curvature_ratio_min                 = 0.0;

curvature_ratio_max_set             = 0;
curvature_ratio_max                 = 0.0;

complexity_ratio_min_set            = 0;
complexity_ratio_min                = 0.0;

complexity_ratio_max_set            = 0;
complexity_ratio_max                = 0.0;

percentile_intensity_ratio_min_set  = 0;
percentile_intensity_ratio_min      = 0.0;

percentile_intensity_ratio_max_set  = 0;
percentile_intensity_ratio_max      = 0.0;

interest_min_set                    = 0;
interest_min                        = 0.0;

interest_max_set                    = 0;
interest_max                        = 0.0;


if ( poly )  { delete poly;  poly =(MaskPoly *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::assign(const ModeAttributes & A)

{

clear();

   //
   //  toggles
   //

   is_fcst_toggle_set = A.is_fcst_toggle_set;
   is_fcst            = A.is_fcst;

 is_single_toggle_set = A.is_single_toggle_set;
 is_single            = A.is_single;

 is_simple_toggle_set = A.is_simple_toggle_set;
 is_simple            = A.is_simple;

is_matched_toggle_set = A.is_matched_toggle_set;
is_matched            = A.is_matched;


   //
   //  string array members
   //

model    = A.model;
desc     = A.desc;
fcst_thr = A.fcst_thr;
obs_thr  = A.obs_thr;
fcst_var = A.fcst_var;
fcst_units = A.fcst_units;
fcst_lev = A.fcst_lev;
obs_var  = A.obs_var;
obs_units  = A.obs_units;
obs_lev  = A.obs_lev;

   //
   //  int array members
   //

fcst_lead       = A.fcst_lead;
fcst_valid_hour = A.fcst_valid_hour;
fcst_init_hour  = A.fcst_init_hour;
fcst_accum      = A.fcst_accum;
obs_lead        = A.obs_lead;
obs_valid_hour  = A.obs_valid_hour;
obs_init_hour   = A.obs_init_hour;
obs_accum       = A.obs_accum;
fcst_rad        = A.fcst_rad;
obs_rad         = A.obs_rad;

   //
   //  unixtime max/min members
   //

fcst_valid_min_set  = A.fcst_valid_min_set;
fcst_valid_min      = A.fcst_valid_min;

fcst_valid_max_set  = A.fcst_valid_max_set;
fcst_valid_max      = A.fcst_valid_max;

obs_valid_min_set   = A.obs_valid_min_set;
obs_valid_min       = A.obs_valid_min;

obs_valid_max_set   = A.obs_valid_max_set;
obs_valid_max       = A.obs_valid_max;

fcst_init_min_set   = A.fcst_init_min_set;
fcst_init_min       = A.fcst_init_min;

fcst_init_max_set   = A.fcst_init_max_set;
fcst_init_max       = A.fcst_init_max;

obs_init_min_set    = A.obs_init_min_set;
obs_init_min        = A.obs_init_min;

obs_init_max_set    = A.obs_init_max_set;
obs_init_max        = A.obs_init_max;


   //
   //  int max/min members
   //

area_min_set               = A.area_min_set;
area_min                   = A.area_min;

area_max_set               = A.area_max_set;
area_max                   = A.area_max;

area_thresh_min_set        = A.area_thresh_min_set;
area_thresh_min            = A.area_thresh_min;

area_thresh_max_set        = A.area_thresh_max_set;
area_thresh_max            = A.area_thresh_max;

intersection_area_min_set  = A.intersection_area_min_set;
intersection_area_min      = A.intersection_area_min;

intersection_area_max_set  = A.intersection_area_max_set;
intersection_area_max      = A.intersection_area_max;

union_area_min_set         = A.union_area_min_set;
union_area_min             = A.union_area_min;

union_area_max_set         = A.union_area_max_set;
union_area_max             = A.union_area_max;

symmetric_diff_min_set     = A.symmetric_diff_min_set;
symmetric_diff_min         = A.symmetric_diff_min;

symmetric_diff_max_set     = A.symmetric_diff_max_set;
symmetric_diff_max         = A.symmetric_diff_max;


   //
   //  double max/min members
   //

centroid_x_min_set                  = A.centroid_x_min_set;
centroid_x_min                      = A.centroid_x_min;

centroid_x_max_set                  = A.centroid_x_max_set;
centroid_x_max                      = A.centroid_x_max;

centroid_y_min_set                  = A.centroid_y_min_set;
centroid_y_min                      = A.centroid_y_min;

centroid_y_max_set                  = A.centroid_y_max_set;
centroid_y_max                      = A.centroid_y_max;

centroid_lat_min_set                = A.centroid_lat_min_set;
centroid_lat_min                    = A.centroid_lat_min;

centroid_lat_max_set                = A.centroid_lat_max_set;
centroid_lat_max                    = A.centroid_lat_max;

centroid_lon_min_set                = A.centroid_lon_min_set;
centroid_lon_min                    = A.centroid_lon_min;

centroid_lon_max_set                = A.centroid_lon_max_set;
centroid_lon_max                    = A.centroid_lon_max;

axis_ang_min_set                    = A.axis_ang_min_set;
axis_ang_min                        = A.axis_ang_min;

axis_ang_max_set                    = A.axis_ang_max_set;
axis_ang_max                        = A.axis_ang_max;

length_min_set                      = A.length_min_set;
length_min                          = A.length_min;

length_max_set                      = A.length_max_set;
length_max                          = A.length_max;

width_min_set                       = A.width_min_set;
width_min                           = A.width_min;

width_max_set                       = A.width_max_set;
width_max                           = A.width_max;

aspect_ratio_min_set                = A.aspect_ratio_min_set;
aspect_ratio_min                    = A.aspect_ratio_min;

aspect_ratio_max_set                = A.aspect_ratio_max_set;
aspect_ratio_max                    = A.aspect_ratio_max;

curvature_min_set                   = A.curvature_min_set;
curvature_min                       = A.curvature_min;

curvature_max_set                   = A.curvature_max_set;
curvature_max                       = A.curvature_max;

curvature_x_min_set                 = A.curvature_x_min_set;
curvature_x_min                     = A.curvature_x_min;

curvature_x_max_set                 = A.curvature_x_max_set;
curvature_x_max                     = A.curvature_x_max;

curvature_y_min_set                 = A.curvature_y_min_set;
curvature_y_min                     = A.curvature_y_min;

curvature_y_max_set                 = A.curvature_y_max_set;
curvature_y_max                     = A.curvature_y_max;

complexity_min_set                  = A.complexity_min_set;
complexity_min                      = A.complexity_min;

complexity_max_set                  = A.complexity_max_set;
complexity_max                      = A.complexity_max;

intensity_10_min_set                = A.intensity_10_min_set;
intensity_10_min                    = A.intensity_10_min;

intensity_10_max_set                = A.intensity_10_max_set;
intensity_10_max                    = A.intensity_10_max;

intensity_25_min_set                = A.intensity_25_min_set;
intensity_25_min                    = A.intensity_25_min;

intensity_25_max_set                = A.intensity_25_max_set;
intensity_25_max                    = A.intensity_25_max;

intensity_50_min_set                = A.intensity_50_min_set;
intensity_50_min                    = A.intensity_50_min;

intensity_50_max_set                = A.intensity_50_max_set;
intensity_50_max                    = A.intensity_50_max;

intensity_75_min_set                = A.intensity_75_min_set;
intensity_75_min                    = A.intensity_75_min;

intensity_75_max_set                = A.intensity_75_max_set;
intensity_75_max                    = A.intensity_75_max;

intensity_90_min_set                = A.intensity_90_min_set;
intensity_90_min                    = A.intensity_90_min;

intensity_90_max_set                = A.intensity_90_max_set;
intensity_90_max                    = A.intensity_90_max;

intensity_user_min_set              = A.intensity_user_min_set;
intensity_user_min                  = A.intensity_user_min;

intensity_user_max_set              = A.intensity_user_max_set;
intensity_user_max                  = A.intensity_user_max;

intensity_sum_min_set               = A.intensity_sum_min_set;
intensity_sum_min                   = A.intensity_sum_min;

intensity_sum_max_set               = A.intensity_sum_max_set;
intensity_sum_max                   = A.intensity_sum_max;

centroid_dist_min_set               = A.centroid_dist_min_set;
centroid_dist_min                   = A.centroid_dist_min;

centroid_dist_max_set               = A.centroid_dist_max_set;
centroid_dist_max                   = A.centroid_dist_max;

boundary_dist_min_set               = A.boundary_dist_min_set;
boundary_dist_min                   = A.boundary_dist_min;

boundary_dist_max_set               = A.boundary_dist_max_set;
boundary_dist_max                   = A.boundary_dist_max;

convex_hull_dist_min_set            = A.convex_hull_dist_min_set;
convex_hull_dist_min                = A.convex_hull_dist_min;

convex_hull_dist_max_set            = A.convex_hull_dist_max_set;
convex_hull_dist_max                = A.convex_hull_dist_max;

angle_diff_min_set                  = A.angle_diff_min_set;
angle_diff_min                      = A.angle_diff_min;

angle_diff_max_set                  = A.angle_diff_max_set;
angle_diff_max                      = A.angle_diff_max;

aspect_diff_min_set                 = A.aspect_diff_min_set;
aspect_diff_min                     = A.aspect_diff_min;

aspect_diff_max_set                 = A.aspect_diff_max_set;
aspect_diff_max                     = A.aspect_diff_max;

area_ratio_min_set                  = A.area_ratio_min_set;
area_ratio_min                      = A.area_ratio_min;

area_ratio_max_set                  = A.area_ratio_max_set;
area_ratio_max                      = A.area_ratio_max;

intersection_over_area_min_set      = A.intersection_over_area_min_set;
intersection_over_area_min          = A.intersection_over_area_min;

intersection_over_area_max_set      = A.intersection_over_area_max_set;
intersection_over_area_max          = A.intersection_over_area_max;

curvature_ratio_min_set             = A.curvature_ratio_min_set;
curvature_ratio_min                 = A.curvature_ratio_min;

curvature_ratio_max_set             = A.curvature_ratio_max_set;
curvature_ratio_max                 = A.curvature_ratio_max;

complexity_ratio_min_set            = A.complexity_ratio_min_set;
complexity_ratio_min                = A.complexity_ratio_min;

complexity_ratio_max_set            = A.complexity_ratio_max_set;
complexity_ratio_max                = A.complexity_ratio_max;

percentile_intensity_ratio_min_set  = A.percentile_intensity_ratio_min_set;
percentile_intensity_ratio_min      = A.percentile_intensity_ratio_min;

percentile_intensity_ratio_max_set  = A.percentile_intensity_ratio_max_set;
percentile_intensity_ratio_max      = A.percentile_intensity_ratio_max;

interest_min_set                    = A.interest_min_set;
interest_min                        = A.interest_min;

interest_max_set                    = A.interest_max_set;
interest_max                        = A.interest_max;


if ( A.poly )  {

   poly = new MaskPoly;

   *poly = *(A.poly);

}

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::dump(ostream & out, int depth) const

{

int j;
int month, day, year, hour, minute, second;
char junk[256];
Indent prefix(depth);


   //
   //  toggles
   //

if ( is_fcst_toggle_set )  {

   if ( is_fcst )   out << prefix << "fcst toggle set\n";
   else             out << prefix << "obs toggle set\n";

}

if ( is_single_toggle_set )  {

   if ( is_single )   out << prefix << "single toggle set\n";
   else               out << prefix << "pair toggle set\n";

}

if ( is_simple_toggle_set )  {

   if ( is_simple )   out << prefix << "simple toggle set\n";
   else               out << prefix << "cluster toggle set\n";

}

if ( is_matched_toggle_set )  {

   if ( is_matched )   out << prefix << "matched toggle set\n";
   else                out << prefix << "unmatched toggle set\n";

}




   //
   //  string array members
   //

if ( model.n_elements() > 0 )  {

   out << prefix << "model = { ";

   for (j=0; j<(model.n_elements()); ++j)  {

      out << "\"" << model[j] << "\"";

      if ( j < (model.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( desc.n_elements() > 0 )  {

   out << prefix << "desc = { ";

   for (j=0; j<(desc.n_elements()); ++j)  {

      out << "\"" << desc[j] << "\"";

      if ( j < (desc.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( fcst_thr.n_elements() > 0 )  {

   out << prefix << "fcst_thr = { ";

   for (j=0; j<(fcst_thr.n_elements()); ++j)  {

      out << "\"" << fcst_thr[j] << "\"";

      if ( j < (fcst_thr.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( obs_thr.n_elements() > 0 )  {

   out << prefix << "obs_thr = { ";

   for (j=0; j<(obs_thr.n_elements()); ++j)  {

      out << "\"" << obs_thr[j] << "\"";

      if ( j < (obs_thr.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( fcst_var.n_elements() > 0 )  {

   out << prefix << "fcst_var = { ";

   for (j=0; j<(fcst_var.n_elements()); ++j)  {

      out << "\"" << fcst_var[j] << "\"";

      if ( j < (fcst_var.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( fcst_units.n_elements() > 0 )  {

   out << prefix << "fcst_units = { ";

   for (j=0; j<(fcst_units.n_elements()); ++j)  {

      out << "\"" << fcst_units[j] << "\"";

      if ( j < (fcst_units.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( fcst_lev.n_elements() > 0 )  {

   out << prefix << "fcst_lev = { ";

   for (j=0; j<(fcst_lev.n_elements()); ++j)  {

      out << "\"" << fcst_lev[j] << "\"";

      if ( j < (fcst_lev.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( obs_var.n_elements() > 0 )  {

   out << prefix << "obs_var = { ";

   for (j=0; j<(obs_var.n_elements()); ++j)  {

      out << "\"" << obs_var[j] << "\"";

      if ( j < (obs_var.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( obs_units.n_elements() > 0 )  {

   out << prefix << "obs_units = { ";

   for (j=0; j<(obs_units.n_elements()); ++j)  {

      out << "\"" << obs_units[j] << "\"";

      if ( j < (obs_units.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( obs_lev.n_elements() > 0 )  {

   out << prefix << "obs_lev = { ";

   for (j=0; j<(obs_lev.n_elements()); ++j)  {

      out << "\"" << obs_lev[j] << "\"";

      if ( j < (obs_lev.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}




   //
   //  int array members
   //

if ( fcst_lead.n_elements() > 0 )  {

   out << prefix << "fcst_lead = { ";

   for (j=0; j<(fcst_lead.n_elements()); ++j)  {

      out << fcst_lead[j];

      if ( j < (fcst_lead.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( fcst_valid_hour.n_elements() > 0 )  {

   out << prefix << "fcst_valid_hour = { ";

   for (j=0; j<(fcst_valid_hour.n_elements()); ++j)  {

      out << fcst_valid_hour[j];

      if ( j < (fcst_valid_hour.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( fcst_init_hour.n_elements() > 0 )  {

   out << prefix << "fcst_init_hour = { ";

   for (j=0; j<(fcst_init_hour.n_elements()); ++j)  {

      out << fcst_init_hour[j];

      if ( j < (fcst_init_hour.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( fcst_accum.n_elements() > 0 )  {

   out << prefix << "fcst_accum = { ";

   for (j=0; j<(fcst_accum.n_elements()); ++j)  {

      out << fcst_accum[j];

      if ( j < (fcst_accum.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( obs_lead.n_elements() > 0 )  {

   out << prefix << "obs_lead = { ";

   for (j=0; j<(obs_lead.n_elements()); ++j)  {

      out << obs_lead[j];

      if ( j < (obs_lead.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( obs_valid_hour.n_elements() > 0 )  {

   out << prefix << "obs_valid_hour = { ";

   for (j=0; j<(obs_valid_hour.n_elements()); ++j)  {

      out << obs_valid_hour[j];

      if ( j < (obs_valid_hour.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( obs_init_hour.n_elements() > 0 )  {

   out << prefix << "obs_init_hour = { ";

   for (j=0; j<(obs_init_hour.n_elements()); ++j)  {

      out << obs_init_hour[j];

      if ( j < (obs_init_hour.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( obs_accum.n_elements() > 0 )  {

   out << prefix << "obs_accum = { ";

   for (j=0; j<(obs_accum.n_elements()); ++j)  {

      out << obs_accum[j];

      if ( j < (obs_accum.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( fcst_rad.n_elements() > 0 )  {

   out << prefix << "fcst_rad = { ";

   for (j=0; j<(fcst_rad.n_elements()); ++j)  {

      out << fcst_rad[j];

      if ( j < (fcst_rad.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}

if ( obs_rad.n_elements() > 0 )  {

   out << prefix << "obs_rad = { ";

   for (j=0; j<(obs_rad.n_elements()); ++j)  {

      out << obs_rad[j];

      if ( j < (obs_rad.n_elements() - 1) )  out << ", ";

   }

   out << " }\n";

}




   //
   //  int max/min members
   //

if ( area_min_set )  out << prefix << "area_min = " << area_min << "\n";
if ( area_max_set )  out << prefix << "area_max = " << area_max << "\n";

if ( area_thresh_min_set )  out << prefix << "area_thresh_min = " << area_thresh_min << "\n";
if ( area_thresh_max_set )  out << prefix << "area_thresh_max = " << area_thresh_max << "\n";

if ( intersection_area_min_set )  out << prefix << "intersection_area_min = " << intersection_area_min << "\n";
if ( intersection_area_max_set )  out << prefix << "intersection_area_max = " << intersection_area_max << "\n";

if ( union_area_min_set )  out << prefix << "union_area_min = " << union_area_min << "\n";
if ( union_area_max_set )  out << prefix << "union_area_max = " << union_area_max << "\n";

if ( symmetric_diff_min_set )  out << prefix << "symmetric_diff_min = " << symmetric_diff_min << "\n";
if ( symmetric_diff_max_set )  out << prefix << "symmetric_diff_max = " << symmetric_diff_max << "\n";




   //
   //  unixtime max/min members
   //

if ( fcst_valid_min_set )  {

   unix_to_mdyhms(fcst_valid_min, month, day, year, hour, minute, second);

   snprintf(junk, sizeof(junk), "%s %2d, %4d   %02d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);

   out << prefix << "fcst_valid_min = " << junk << "\n";

}

if ( fcst_valid_max_set )  {

   unix_to_mdyhms(fcst_valid_max, month, day, year, hour, minute, second);

   snprintf(junk, sizeof(junk), "%s %2d, %4d   %02d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);

   out << prefix << "fcst_valid_max = " << junk << "\n";

}

if ( obs_valid_min_set )  {

   unix_to_mdyhms(obs_valid_min, month, day, year, hour, minute, second);

   snprintf(junk, sizeof(junk), "%s %2d, %4d   %02d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);

   out << prefix << "obs_valid_min = " << junk << "\n";

}

if ( obs_valid_max_set )  {

   unix_to_mdyhms(obs_valid_max, month, day, year, hour, minute, second);

   snprintf(junk, sizeof(junk), "%s %2d, %4d   %02d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);

   out << prefix << "obs_valid_max = " << junk << "\n";

}

if ( fcst_init_min_set )  {

   unix_to_mdyhms(fcst_init_min, month, day, year, hour, minute, second);

   snprintf(junk, sizeof(junk), "%s %2d, %4d   %02d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);

   out << prefix << "fcst_init_min = " << junk << "\n";

}

if ( fcst_init_max_set )  {

   unix_to_mdyhms(fcst_init_max, month, day, year, hour, minute, second);

   snprintf(junk, sizeof(junk), "%s %2d, %4d   %02d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);

   out << prefix << "fcst_init_max = " << junk << "\n";

}

if ( obs_init_min_set )  {

   unix_to_mdyhms(obs_init_min, month, day, year, hour, minute, second);

   snprintf(junk, sizeof(junk), "%s %2d, %4d   %02d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);

   out << prefix << "obs_init_min = " << junk << "\n";

}

if ( obs_init_max_set )  {

   unix_to_mdyhms(obs_init_max, month, day, year, hour, minute, second);

   snprintf(junk, sizeof(junk), "%s %2d, %4d   %02d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);

   out << prefix << "obs_init_max = " << junk << "\n";

}





   //
   //  double max/min members
   //

if ( centroid_x_min_set )  out << prefix << "centroid_x_min = " << centroid_x_min << "\n";
if ( centroid_x_max_set )  out << prefix << "centroid_x_max = " << centroid_x_max << "\n";

if ( centroid_y_min_set )  out << prefix << "centroid_y_min = " << centroid_y_min << "\n";
if ( centroid_y_max_set )  out << prefix << "centroid_y_max = " << centroid_y_max << "\n";

if ( centroid_lat_min_set )  out << prefix << "centroid_lat_min = " << centroid_lat_min << "\n";
if ( centroid_lat_max_set )  out << prefix << "centroid_lat_max = " << centroid_lat_max << "\n";

if ( centroid_lon_min_set )  out << prefix << "centroid_lon_min = " << centroid_lon_min << "\n";
if ( centroid_lon_max_set )  out << prefix << "centroid_lon_max = " << centroid_lon_max << "\n";

if ( axis_ang_min_set )  out << prefix << "axis_ang_min = " << axis_ang_min << "\n";
if ( axis_ang_max_set )  out << prefix << "axis_ang_max = " << axis_ang_max << "\n";

if ( length_min_set )  out << prefix << "length_min = " << length_min << "\n";
if ( length_max_set )  out << prefix << "length_max = " << length_max << "\n";

if ( width_min_set )  out << prefix << "width_min = " << width_min << "\n";
if ( width_max_set )  out << prefix << "width_max = " << width_max << "\n";

if ( aspect_ratio_min_set )  out << prefix << "aspect_ratio_min = " << aspect_ratio_min << "\n";
if ( aspect_ratio_max_set )  out << prefix << "aspect_ratio_max = " << aspect_ratio_max << "\n";

if ( curvature_min_set )  out << prefix << "curvature_min = " << curvature_min << "\n";
if ( curvature_max_set )  out << prefix << "curvature_max = " << curvature_max << "\n";

if ( curvature_x_min_set )  out << prefix << "curvature_x_min = " << curvature_x_min << "\n";
if ( curvature_x_max_set )  out << prefix << "curvature_x_max = " << curvature_x_max << "\n";

if ( curvature_y_min_set )  out << prefix << "curvature_y_min = " << curvature_y_min << "\n";
if ( curvature_y_max_set )  out << prefix << "curvature_y_max = " << curvature_y_max << "\n";

if ( complexity_min_set )  out << prefix << "complexity_min = " << complexity_min << "\n";
if ( complexity_max_set )  out << prefix << "complexity_max = " << complexity_max << "\n";

if ( intensity_10_min_set )  out << prefix << "intensity_10_min = " << intensity_10_min << "\n";
if ( intensity_10_max_set )  out << prefix << "intensity_10_max = " << intensity_10_max << "\n";

if ( intensity_25_min_set )  out << prefix << "intensity_25_min = " << intensity_25_min << "\n";
if ( intensity_25_max_set )  out << prefix << "intensity_25_max = " << intensity_25_max << "\n";

if ( intensity_50_min_set )  out << prefix << "intensity_50_min = " << intensity_50_min << "\n";
if ( intensity_50_max_set )  out << prefix << "intensity_50_max = " << intensity_50_max << "\n";

if ( intensity_75_min_set )  out << prefix << "intensity_75_min = " << intensity_75_min << "\n";
if ( intensity_75_max_set )  out << prefix << "intensity_75_max = " << intensity_75_max << "\n";

if ( intensity_90_min_set )  out << prefix << "intensity_90_min = " << intensity_90_min << "\n";
if ( intensity_90_max_set )  out << prefix << "intensity_90_max = " << intensity_90_max << "\n";

if ( intensity_user_min_set )  out << prefix << "intensity_user_min = " << intensity_user_min << "\n";
if ( intensity_user_max_set )  out << prefix << "intensity_user_max = " << intensity_user_max << "\n";

if ( intensity_sum_min_set )  out << prefix << "intensity_sum_min = " << intensity_sum_min << "\n";
if ( intensity_sum_max_set )  out << prefix << "intensity_sum_max = " << intensity_sum_max << "\n";

if ( centroid_dist_min_set )  out << prefix << "centroid_dist_min = " << centroid_dist_min << "\n";
if ( centroid_dist_max_set )  out << prefix << "centroid_dist_max = " << centroid_dist_max << "\n";

if ( boundary_dist_min_set )  out << prefix << "boundary_dist_min = " << boundary_dist_min << "\n";
if ( boundary_dist_max_set )  out << prefix << "boundary_dist_max = " << boundary_dist_max << "\n";

if ( convex_hull_dist_min_set )  out << prefix << "convex_hull_dist_min = " << convex_hull_dist_min << "\n";
if ( convex_hull_dist_max_set )  out << prefix << "convex_hull_dist_max = " << convex_hull_dist_max << "\n";

if ( angle_diff_min_set )  out << prefix << "angle_diff_min = " << angle_diff_min << "\n";
if ( angle_diff_max_set )  out << prefix << "angle_diff_max = " << angle_diff_max << "\n";

if ( aspect_diff_min_set )  out << prefix << "aspect_diff_min = " << aspect_diff_min << "\n";
if ( aspect_diff_max_set )  out << prefix << "aspect_diff_max = " << aspect_diff_max << "\n";

if ( area_ratio_min_set )  out << prefix << "area_ratio_min = " << area_ratio_min << "\n";
if ( area_ratio_max_set )  out << prefix << "area_ratio_max = " << area_ratio_max << "\n";

if ( intersection_over_area_min_set )  out << prefix << "intersection_over_area_min = " << intersection_over_area_min << "\n";
if ( intersection_over_area_max_set )  out << prefix << "intersection_over_area_max = " << intersection_over_area_max << "\n";

if ( curvature_ratio_min_set )  out << prefix << "curvature_ratio_min = " << curvature_ratio_min << "\n";
if ( curvature_ratio_max_set )  out << prefix << "curvature_ratio_max = " << curvature_ratio_max << "\n";

if ( complexity_ratio_min_set )  out << prefix << "complexity_ratio_min = " << complexity_ratio_min << "\n";
if ( complexity_ratio_max_set )  out << prefix << "complexity_ratio_max = " << complexity_ratio_max << "\n";

if ( percentile_intensity_ratio_min_set )  out << prefix << "percentile_intensity_ratio_min = " << percentile_intensity_ratio_min << "\n";
if ( percentile_intensity_ratio_max_set )  out << prefix << "percentile_intensity_ratio_max = " << percentile_intensity_ratio_max << "\n";

if ( interest_min_set )  out << prefix << "interest_min = " << interest_min << "\n";
if ( interest_max_set )  out << prefix << "interest_max = " << interest_max << "\n";




   //
   //  misc
   //

if ( poly )  {

   out << prefix << "Mask Polyline ...\n";

   poly->dump(out, depth + 1);

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


int ModeAttributes::is_keeper(const ModeLine & L) const

{

int i;
double x;
unixtime t;
const char * c = (const char *) 0;


   //
   //  toggles
   //

if ( is_fcst_toggle_set )  {

   i = L.is_fcst();

   if ( ( is_fcst) && ( !i ) )  return ( 0 );
   if ( (!is_fcst) && (  i ) )  return ( 0 );

}

if ( is_single_toggle_set )  {

   i = L.is_single();

   if ( ( is_single) && ( !i ) )  return ( 0 );
   if ( (!is_single) && (  i ) )  return ( 0 );

}

if ( is_simple_toggle_set )  {

   i = L.is_simple();

   if ( ( is_simple) && ( !i ) )  return ( 0 );
   if ( (!is_simple) && (  i ) )  return ( 0 );

}

if ( is_matched_toggle_set )  {

   i = L.is_matched();

   if ( ( is_matched) && ( !i ) )  return ( 0 );
   if ( (!is_matched) && (  i ) )  return ( 0 );

}

   //
   //  string array members
   //

c = L.model();

if ( (model.n_elements() > 0) && !(model.has(c)) )  return ( 0 );

c = L.desc();

if ( (desc.n_elements() > 0) && !(desc.has(c)) )  return ( 0 );

c = L.fcst_thr();

if ( (fcst_thr.n_elements() > 0) && !(fcst_thr.has(c)) )  return ( 0 );

c = L.obs_thr();

if ( (obs_thr.n_elements() > 0) && !(obs_thr.has(c)) )  return ( 0 );

c = L.fcst_var();

if ( (fcst_var.n_elements() > 0) && !(fcst_var.has(c)) )  return ( 0 );

c = L.fcst_units();

if ( (fcst_units.n_elements() > 0) && !(fcst_units.has(c)) )  return ( 0 );

c = L.fcst_lev();

if ( (fcst_lev.n_elements() > 0) && !(fcst_lev.has(c)) )  return ( 0 );

c = L.obs_var();

if ( (obs_var.n_elements() > 0) && !(obs_var.has(c)) )  return ( 0 );

c = L.obs_units();

if ( (obs_units.n_elements() > 0) && !(obs_units.has(c)) )  return ( 0 );

c = L.obs_lev();

if ( (obs_lev.n_elements() > 0) && !(obs_lev.has(c)) )  return ( 0 );


   //
   //  int array members
   //

i = L.fcst_lead();

if ( (fcst_lead.n_elements() > 0) && !(fcst_lead.has(i)) )  return ( 0 );

i = L.fcst_valid_hour();

if ( (fcst_valid_hour.n_elements() > 0) && !(fcst_valid_hour.has(i)) )  return ( 0 );

i = L.fcst_init_hour();

if ( (fcst_init_hour.n_elements() > 0) && !(fcst_init_hour.has(i)) )  return ( 0 );

i = L.fcst_accum();

if ( (fcst_accum.n_elements() > 0) && !(fcst_accum.has(i)) )  return ( 0 );

i = L.obs_lead();

if ( (obs_lead.n_elements() > 0) && !(obs_lead.has(i)) )  return ( 0 );

i = L.obs_valid_hour();

if ( (obs_valid_hour.n_elements() > 0) && !(obs_valid_hour.has(i)) )  return ( 0 );

i = L.obs_init_hour();

if ( (obs_init_hour.n_elements() > 0) && !(obs_init_hour.has(i)) )  return ( 0 );

i = L.obs_accum();

if ( (obs_accum.n_elements() > 0) && !(obs_accum.has(i)) )  return ( 0 );

i = L.fcst_rad();

if ( (fcst_rad.n_elements() > 0) && !(fcst_rad.has(i)) )  return ( 0 );

i = L.obs_rad();

if ( (obs_rad.n_elements() > 0) && !(obs_rad.has(i)) )  return ( 0 );


   //
   //  unixtime max/min members
   //

if ( fcst_valid_min_set || fcst_valid_max_set )  {

   t = L.fcst_valid();

   if ( fcst_valid_min_set && (t < fcst_valid_min) )   return ( 0 );

   if ( fcst_valid_max_set && (t > fcst_valid_max) )   return ( 0 );

}

if ( obs_valid_min_set || obs_valid_max_set )  {

   t = L.obs_valid();

   if ( obs_valid_min_set && (t < obs_valid_min) )   return ( 0 );

   if ( obs_valid_max_set && (t > obs_valid_max) )   return ( 0 );

}

if ( fcst_init_min_set || fcst_init_max_set )  {

   t = L.fcst_init();

   if ( fcst_init_min_set && (t < fcst_init_min) )   return ( 0 );

   if ( fcst_init_max_set && (t > fcst_init_max) )   return ( 0 );

}

if ( obs_init_min_set || obs_init_max_set )  {

   t = L.obs_init();

   if ( obs_init_min_set && (t < obs_init_min) )   return ( 0 );

   if ( obs_init_max_set && (t > obs_init_max) )   return ( 0 );

}


   //
   //  int max/min members
   //

if ( area_min_set || area_max_set )  {

   i = L.area();

   if ( !is_bad_data( i ) && area_min_set && (i < area_min) )   return ( 0 );

   if ( !is_bad_data( i ) && area_max_set && (i > area_max) )   return ( 0 );

}

if ( area_thresh_min_set || area_thresh_max_set )  {

   i = L.area_thresh();

   if ( !is_bad_data( i ) && area_thresh_min_set && (i < area_thresh_min) )   return ( 0 );

   if ( !is_bad_data( i ) && area_thresh_max_set && (i > area_thresh_max) )   return ( 0 );

}

if ( intersection_area_min_set || intersection_area_max_set )  {

   i = L.intersection_area();

   if ( !is_bad_data( i ) && intersection_area_min_set && (i < intersection_area_min) )   return ( 0 );

   if ( !is_bad_data( i ) && intersection_area_max_set && (i > intersection_area_max) )   return ( 0 );

}

if ( union_area_min_set || union_area_max_set )  {

   i = L.union_area();

   if ( !is_bad_data( i ) && union_area_min_set && (i < union_area_min) )   return ( 0 );

   if ( !is_bad_data( i ) && union_area_max_set && (i > union_area_max) )   return ( 0 );

}

if ( symmetric_diff_min_set || symmetric_diff_max_set )  {

   i = L.symmetric_diff();

   if ( !is_bad_data( i ) && symmetric_diff_min_set && (i < symmetric_diff_min) )   return ( 0 );

   if ( !is_bad_data( i ) && symmetric_diff_max_set && (i > symmetric_diff_max) )   return ( 0 );

}


   //
   //  double max/min members
   //

if ( centroid_x_min_set || centroid_x_max_set )  {

   x = L.centroid_x();

   if ( !is_bad_data( x ) && centroid_x_min_set && (x < centroid_x_min) )   return ( 0 );

   if ( !is_bad_data( x ) && centroid_x_max_set && (x > centroid_x_max) )   return ( 0 );

}

if ( centroid_y_min_set || centroid_y_max_set )  {

   x = L.centroid_y();

   if ( !is_bad_data( x ) && centroid_y_min_set && (x < centroid_y_min) )   return ( 0 );

   if ( !is_bad_data( x ) && centroid_y_max_set && (x > centroid_y_max) )   return ( 0 );

}

if ( centroid_lat_min_set || centroid_lat_max_set )  {

   x = L.centroid_lat();

   if ( !is_bad_data( x ) && centroid_lat_min_set && (x < centroid_lat_min) )   return ( 0 );

   if ( !is_bad_data( x ) && centroid_lat_max_set && (x > centroid_lat_max) )   return ( 0 );

}

if ( centroid_lon_min_set || centroid_lon_max_set )  {

   x = L.centroid_lon();

   if ( !is_bad_data( x ) && centroid_lon_min_set && (x < centroid_lon_min) )   return ( 0 );

   if ( !is_bad_data( x ) && centroid_lon_max_set && (x > centroid_lon_max) )   return ( 0 );

}

if ( axis_ang_min_set || axis_ang_max_set )  {

   x = L.axis_ang();

   if ( !is_bad_data( x ) && axis_ang_min_set && (x < axis_ang_min) )   return ( 0 );

   if ( !is_bad_data( x ) && axis_ang_max_set && (x > axis_ang_max) )   return ( 0 );

}

if ( length_min_set || length_max_set )  {

   x = L.length();

   if ( !is_bad_data( x ) && length_min_set && (x < length_min) )   return ( 0 );

   if ( !is_bad_data( x ) && length_max_set && (x > length_max) )   return ( 0 );

}

if ( width_min_set || width_max_set )  {

   x = L.width();

   if ( !is_bad_data( x ) && width_min_set && (x < width_min) )   return ( 0 );

   if ( !is_bad_data( x ) && width_max_set && (x > width_max) )   return ( 0 );

}

if ( aspect_ratio_min_set || aspect_ratio_max_set )  {

   x = L.aspect_ratio();

   if ( !is_bad_data( x ) && aspect_ratio_min_set && (x < aspect_ratio_min) )   return ( 0 );

   if ( !is_bad_data( x ) && aspect_ratio_max_set && (x > aspect_ratio_max) )   return ( 0 );

}

if ( curvature_min_set || curvature_max_set )  {

   x = L.curvature();

   if ( !is_bad_data( x ) && curvature_min_set && (x < curvature_min) )   return ( 0 );

   if ( !is_bad_data( x ) && curvature_max_set && (x > curvature_max) )   return ( 0 );

}

if ( curvature_x_min_set || curvature_x_max_set )  {

   x = L.curvature_x();

   if ( !is_bad_data( x ) && curvature_x_min_set && (x < curvature_x_min) )   return ( 0 );

   if ( !is_bad_data( x ) && curvature_x_max_set && (x > curvature_x_max) )   return ( 0 );

}

if ( curvature_y_min_set || curvature_y_max_set )  {

   x = L.curvature_y();

   if ( !is_bad_data( x ) && curvature_y_min_set && (x < curvature_y_min) )   return ( 0 );

   if ( !is_bad_data( x ) && curvature_y_max_set && (x > curvature_y_max) )   return ( 0 );

}

if ( complexity_min_set || complexity_max_set )  {

   x = L.complexity();

   if ( !is_bad_data( x ) && complexity_min_set && (x < complexity_min) )   return ( 0 );

   if ( !is_bad_data( x ) && complexity_max_set && (x > complexity_max) )   return ( 0 );

}

if ( intensity_10_min_set || intensity_10_max_set )  {

   x = L.intensity_10();

   if ( !is_bad_data( x ) && intensity_10_min_set && (x < intensity_10_min) )   return ( 0 );

   if ( !is_bad_data( x ) && intensity_10_max_set && (x > intensity_10_max) )   return ( 0 );

}

if ( intensity_25_min_set || intensity_25_max_set )  {

   x = L.intensity_25();

   if ( !is_bad_data( x ) && intensity_25_min_set && (x < intensity_25_min) )   return ( 0 );

   if ( !is_bad_data( x ) && intensity_25_max_set && (x > intensity_25_max) )   return ( 0 );

}

if ( intensity_50_min_set || intensity_50_max_set )  {

   x = L.intensity_50();

   if ( !is_bad_data( x ) && intensity_50_min_set && (x < intensity_50_min) )   return ( 0 );

   if ( !is_bad_data( x ) && intensity_50_max_set && (x > intensity_50_max) )   return ( 0 );

}

if ( intensity_75_min_set || intensity_75_max_set )  {

   x = L.intensity_75();

   if ( !is_bad_data( x ) && intensity_75_min_set && (x < intensity_75_min) )   return ( 0 );

   if ( !is_bad_data( x ) && intensity_75_max_set && (x > intensity_75_max) )   return ( 0 );

}

if ( intensity_90_min_set || intensity_90_max_set )  {

   x = L.intensity_90();

   if ( !is_bad_data( x ) && intensity_90_min_set && (x < intensity_90_min) )   return ( 0 );

   if ( !is_bad_data( x ) && intensity_90_max_set && (x > intensity_90_max) )   return ( 0 );

}

if ( intensity_user_min_set || intensity_user_max_set )  {

   x = L.intensity_user();

   if ( !is_bad_data( x ) && intensity_user_min_set && (x < intensity_user_min) )   return ( 0 );

   if ( !is_bad_data( x ) && intensity_user_max_set && (x > intensity_user_max) )   return ( 0 );

}

if ( intensity_sum_min_set || intensity_sum_max_set )  {

   x = L.intensity_sum();

   if ( !is_bad_data( x ) && intensity_sum_min_set && (x < intensity_sum_min) )   return ( 0 );

   if ( !is_bad_data( x ) && intensity_sum_max_set && (x > intensity_sum_max) )   return ( 0 );

}

if ( centroid_dist_min_set || centroid_dist_max_set )  {

   x = L.centroid_dist();

   if ( !is_bad_data( x ) && centroid_dist_min_set && (x < centroid_dist_min) )   return ( 0 );

   if ( !is_bad_data( x ) && centroid_dist_max_set && (x > centroid_dist_max) )   return ( 0 );

}

if ( boundary_dist_min_set || boundary_dist_max_set )  {

   x = L.boundary_dist();

   if ( !is_bad_data( x ) && boundary_dist_min_set && (x < boundary_dist_min) )   return ( 0 );

   if ( !is_bad_data( x ) && boundary_dist_max_set && (x > boundary_dist_max) )   return ( 0 );

}

if ( convex_hull_dist_min_set || convex_hull_dist_max_set )  {

   x = L.convex_hull_dist();

   if ( !is_bad_data( x ) && convex_hull_dist_min_set && (x < convex_hull_dist_min) )   return ( 0 );

   if ( !is_bad_data( x ) && convex_hull_dist_max_set && (x > convex_hull_dist_max) )   return ( 0 );

}

if ( angle_diff_min_set || angle_diff_max_set )  {

   x = L.angle_diff();

   if ( !is_bad_data( x ) && angle_diff_min_set && (x < angle_diff_min) )   return ( 0 );

   if ( !is_bad_data( x ) && angle_diff_max_set && (x > angle_diff_max) )   return ( 0 );

}

if ( aspect_diff_min_set || aspect_diff_max_set )  {

   x = L.aspect_diff();

   if ( !is_bad_data( x ) && aspect_diff_min_set && (x < aspect_diff_min) )   return ( 0 );

   if ( !is_bad_data( x ) && aspect_diff_max_set && (x > aspect_diff_max) )   return ( 0 );

}

if ( area_ratio_min_set || area_ratio_max_set )  {

   x = L.area_ratio();

   if ( !is_bad_data( x ) && area_ratio_min_set && (x < area_ratio_min) )   return ( 0 );

   if ( !is_bad_data( x ) && area_ratio_max_set && (x > area_ratio_max) )   return ( 0 );

}

if ( intersection_over_area_min_set || intersection_over_area_max_set )  {

   x = L.intersection_over_area();

   if ( !is_bad_data( x ) && intersection_over_area_min_set && (x < intersection_over_area_min) )   return ( 0 );

   if ( !is_bad_data( x ) && intersection_over_area_max_set && (x > intersection_over_area_max) )   return ( 0 );

}

if ( curvature_ratio_min_set || curvature_ratio_max_set )  {

   x = L.curvature_ratio();

   if ( !is_bad_data( x ) && curvature_ratio_min_set && (x < curvature_ratio_min) )   return ( 0 );

   if ( !is_bad_data( x ) && curvature_ratio_max_set && (x > curvature_ratio_max) )   return ( 0 );

}

if ( complexity_ratio_min_set || complexity_ratio_max_set )  {

   x = L.complexity_ratio();

   if ( !is_bad_data( x ) && complexity_ratio_min_set && (x < complexity_ratio_min) )   return ( 0 );

   if ( !is_bad_data( x ) && complexity_ratio_max_set && (x > complexity_ratio_max) )   return ( 0 );

}

if ( percentile_intensity_ratio_min_set || percentile_intensity_ratio_max_set )  {

   x = L.percentile_intensity_ratio();

   if ( !is_bad_data( x ) && percentile_intensity_ratio_min_set && (x < percentile_intensity_ratio_min) )   return ( 0 );

   if ( !is_bad_data( x ) && percentile_intensity_ratio_max_set && (x > percentile_intensity_ratio_max) )   return ( 0 );

}

if ( interest_min_set || interest_max_set )  {

   x = L.interest();

   if ( !is_bad_data( x ) && interest_min_set && (x < interest_min) )   return ( 0 );

   if ( !is_bad_data( x ) && interest_max_set && (x > interest_max) )   return ( 0 );

}


   //
   //  misc
   //

if ( poly )  {

   if ( !(poly->latlon_is_inside_dege(L.centroid_lat(), L.centroid_lon())) )  return ( 0 );

}



   //
   //  ok, already
   //

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::parse_command_line(StringArray & a)

{

int j;
const char * c = (const char *) 0;


j = 0;

while ( j < (a.n_elements()) )  {
  
   c = a[j].c_str();

   if ( c[0] != '-' )  { ++j;  continue; }

      //
      //  toggles
      //

        if ( strcmp(c, "-fcst"     ) == 0 )  { set_fcst     ();  a.shift_down(j, 1); }
   else if ( strcmp(c, "-obs"      ) == 0 )  { set_obs      ();  a.shift_down(j, 1); }
   else if ( strcmp(c, "-single"   ) == 0 )  { set_single   ();  a.shift_down(j, 1); }
   else if ( strcmp(c, "-pair"     ) == 0 )  { set_pair     ();  a.shift_down(j, 1); }
   else if ( strcmp(c, "-simple"   ) == 0 )  { set_simple   ();  a.shift_down(j, 1); }
   else if ( strcmp(c, "-cluster"  ) == 0 )  { set_cluster  ();  a.shift_down(j, 1); }
   else if ( strcmp(c, "-matched"  ) == 0 )  { set_matched  ();  a.shift_down(j, 1); }
   else if ( strcmp(c, "-unmatched") == 0 )  { set_unmatched();  a.shift_down(j, 1); }

      //
      //  string array members
      //

   else if ( strcmp(c, "-model"   ) == 0 )  { add_model   (a[j + 1].c_str());  a.shift_down(j, 2); }
   else if ( strcmp(c, "-desc"    ) == 0 )  { add_desc    (a[j + 1].c_str());  a.shift_down(j, 2); }
   else if ( strcmp(c, "-fcst_thr") == 0 )  { add_fcst_thr(a[j + 1].c_str());  a.shift_down(j, 2); }
   else if ( strcmp(c, "-obs_thr" ) == 0 )  { add_obs_thr (a[j + 1].c_str());  a.shift_down(j, 2); }
   else if ( strcmp(c, "-fcst_var") == 0 )  { add_fcst_var(a[j + 1].c_str());  a.shift_down(j, 2); }
   else if ( strcmp(c, "-fcst_units") == 0 )  { add_fcst_units(a[j + 1].c_str());  a.shift_down(j, 2); }
   else if ( strcmp(c, "-fcst_lev") == 0 )  { add_fcst_lev(a[j + 1].c_str());  a.shift_down(j, 2); }
   else if ( strcmp(c, "-obs_var" ) == 0 )  { add_obs_var (a[j + 1].c_str());  a.shift_down(j, 2); }
   else if ( strcmp(c, "-obs_units" ) == 0 )  { add_obs_units (a[j + 1].c_str());  a.shift_down(j, 2); }
   else if ( strcmp(c, "-obs_lev" ) == 0 )  { add_obs_lev (a[j + 1].c_str());  a.shift_down(j, 2); }

      //
      //  int array members
      //

   else if ( strcmp(c, "-fcst_lead"       ) == 0 )  { add_fcst_lead       (timestring_to_sec(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-fcst_valid_hour" ) == 0 )  { add_fcst_valid_hour (timestring_to_sec(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-fcst_init_hour"  ) == 0 )  { add_fcst_init_hour  (timestring_to_sec(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-fcst_accum"      ) == 0 )  { add_fcst_accum      (timestring_to_sec(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-obs_lead"        ) == 0 )  { add_obs_lead        (timestring_to_sec(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-obs_valid_hour"  ) == 0 )  { add_obs_valid_hour  (timestring_to_sec(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-obs_init_hour"   ) == 0 )  { add_obs_init_hour   (timestring_to_sec(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-obs_accum"       ) == 0 )  { add_obs_accum       (timestring_to_sec(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-fcst_rad"        ) == 0 )  { add_fcst_rad        (atoi(a[j + 1].c_str()));               a.shift_down(j, 2); }
   else if ( strcmp(c, "-obs_rad"         ) == 0 )  { add_obs_rad         (atoi(a[j + 1].c_str()));               a.shift_down(j, 2); }

      //
      //  int max/min members
      //

   else if ( strcmp(c, "-area_min"             ) == 0 )  { set_area_min             (atoi(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-area_max"             ) == 0 )  { set_area_max             (atoi(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-area_thresh_min"      ) == 0 )  { set_area_thresh_min      (atoi(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-area_thresh_max"      ) == 0 )  { set_area_thresh_max      (atoi(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intersection_area_min") == 0 )  { set_intersection_area_min(atoi(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intersection_area_max") == 0 )  { set_intersection_area_max(atoi(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-union_area_min"       ) == 0 )  { set_union_area_min       (atoi(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-union_area_max"       ) == 0 )  { set_union_area_max       (atoi(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-symmetric_diff_min"   ) == 0 )  { set_symmetric_diff_min   (atoi(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-symmetric_diff_max"   ) == 0 )  { set_symmetric_diff_max   (atoi(a[j + 1].c_str()));  a.shift_down(j, 2); }

      //
      //  double max/min members
      //

   else if ( strcmp(c, "-centroid_x_min"                ) == 0 )  { set_centroid_x_min                (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-centroid_x_max"                ) == 0 )  { set_centroid_x_max                (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-centroid_y_min"                ) == 0 )  { set_centroid_y_min                (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-centroid_y_max"                ) == 0 )  { set_centroid_y_max                (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-centroid_lat_min"              ) == 0 )  { set_centroid_lat_min              (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-centroid_lat_max"              ) == 0 )  { set_centroid_lat_max              (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-centroid_lon_min"              ) == 0 )  { set_centroid_lon_min              (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-centroid_lon_max"              ) == 0 )  { set_centroid_lon_max              (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-axis_ang_min"                  ) == 0 )  { set_axis_ang_min                  (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-axis_ang_max"                  ) == 0 )  { set_axis_ang_max                  (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-length_min"                    ) == 0 )  { set_length_min                    (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-length_max"                    ) == 0 )  { set_length_max                    (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-width_min"                     ) == 0 )  { set_width_min                     (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-width_max"                     ) == 0 )  { set_width_max                     (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-aspect_ratio_min"              ) == 0 )  { set_aspect_ratio_min              (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-aspect_ratio_max"              ) == 0 )  { set_aspect_ratio_max              (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-curvature_min"                 ) == 0 )  { set_curvature_min                 (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-curvature_max"                 ) == 0 )  { set_curvature_max                 (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-curvature_x_min"               ) == 0 )  { set_curvature_x_min               (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-curvature_x_max"               ) == 0 )  { set_curvature_x_max               (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-curvature_y_min"               ) == 0 )  { set_curvature_y_min               (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-curvature_y_max"               ) == 0 )  { set_curvature_y_max               (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-complexity_min"                ) == 0 )  { set_complexity_min                (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-complexity_max"                ) == 0 )  { set_complexity_max                (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intensity_10_min"              ) == 0 )  { set_intensity_10_min              (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intensity_10_max"              ) == 0 )  { set_intensity_10_max              (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intensity_25_min"              ) == 0 )  { set_intensity_25_min              (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intensity_25_max"              ) == 0 )  { set_intensity_25_max              (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intensity_50_min"              ) == 0 )  { set_intensity_50_min              (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intensity_50_max"              ) == 0 )  { set_intensity_50_max              (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intensity_75_min"              ) == 0 )  { set_intensity_75_min              (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intensity_75_max"              ) == 0 )  { set_intensity_75_max              (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intensity_90_min"              ) == 0 )  { set_intensity_90_min              (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intensity_90_max"              ) == 0 )  { set_intensity_90_max              (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intensity_user_min"            ) == 0 )  { set_intensity_user_min            (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intensity_user_max"            ) == 0 )  { set_intensity_user_max            (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intensity_sum_min"             ) == 0 )  { set_intensity_sum_min             (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intensity_sum_max"             ) == 0 )  { set_intensity_sum_max             (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-centroid_dist_min"             ) == 0 )  { set_centroid_dist_min             (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-centroid_dist_max"             ) == 0 )  { set_centroid_dist_max             (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-boundary_dist_min"             ) == 0 )  { set_boundary_dist_min             (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-boundary_dist_max"             ) == 0 )  { set_boundary_dist_max             (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-convex_hull_dist_min"          ) == 0 )  { set_convex_hull_dist_min          (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-convex_hull_dist_max"          ) == 0 )  { set_convex_hull_dist_max          (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-angle_diff_min"                ) == 0 )  { set_angle_diff_min                (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-angle_diff_max"                ) == 0 )  { set_angle_diff_max                (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-aspect_diff_min"               ) == 0 )  { set_aspect_diff_min               (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-aspect_diff_max"               ) == 0 )  { set_aspect_diff_max               (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-area_ratio_min"                ) == 0 )  { set_area_ratio_min                (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-area_ratio_max"                ) == 0 )  { set_area_ratio_max                (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intersection_over_area_min"    ) == 0 )  { set_intersection_over_area_min    (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-intersection_over_area_max"    ) == 0 )  { set_intersection_over_area_max    (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-curvature_ratio_min"           ) == 0 )  { set_curvature_ratio_min           (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-curvature_ratio_max"           ) == 0 )  { set_curvature_ratio_max           (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-complexity_ratio_min"          ) == 0 )  { set_complexity_ratio_min          (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-complexity_ratio_max"          ) == 0 )  { set_complexity_ratio_max          (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-percentile_intensity_ratio_min") == 0 )  { set_percentile_intensity_ratio_min(atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-percentile_intensity_ratio_max") == 0 )  { set_percentile_intensity_ratio_max(atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-interest_min"                  ) == 0 )  { set_interest_min                  (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-interest_max"                  ) == 0 )  { set_interest_max                  (atof(a[j + 1].c_str()));  a.shift_down(j, 2); }

      //
      //  unixtime max/min members
      //

   else if ( strcmp(c, "-fcst_valid_min") == 0 )  { set_fcst_valid_min(timestring_to_unix(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-fcst_valid_max") == 0 )  { set_fcst_valid_max(timestring_to_unix(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-obs_valid_min" ) == 0 )  { set_obs_valid_min (timestring_to_unix(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-obs_valid_max" ) == 0 )  { set_obs_valid_max (timestring_to_unix(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-fcst_init_min")  == 0 )  { set_fcst_init_min (timestring_to_unix(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-fcst_init_max")  == 0 )  { set_fcst_init_max (timestring_to_unix(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-obs_init_min" )  == 0 )  { set_obs_init_min  (timestring_to_unix(a[j + 1].c_str()));  a.shift_down(j, 2); }
   else if ( strcmp(c, "-obs_init_max" )  == 0 )  { set_obs_init_max  (timestring_to_unix(a[j + 1].c_str()));  a.shift_down(j, 2); }

      //
      //  misc
      //

   else if ( strcmp(c, "-mask_poly") == 0 )  { set_mask(a[j + 1].c_str());  a.shift_down(j, 2); }


      //
      //  no more options, so just increment the loop variable
      //

   else ++j;

}   //  while



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::augment(const ModeAttributes & m)

{

int j;


   //
   //  toggles  (override)
   //

if ( m.is_fcst_toggle_set )  {
   if ( m.is_fcst )  set_fcst();
   else              set_obs();
}

if ( m.is_single_toggle_set )  {
   if ( m.is_single )  set_single();
   else                set_pair();
}

if ( m.is_simple_toggle_set )  {
   if ( m.is_simple )  set_simple();
   else                set_cluster();
}

if ( m.is_matched_toggle_set )  {
   if ( m.is_matched )  set_matched();
   else                 set_unmatched();
}




   //
   //  string array members  (add)
   //

for (j=0; j<(m.model.n_elements()); ++j)  {

   add_model(m.model[j].c_str());

}

for (j=0; j<(m.desc.n_elements()); ++j)  {

   add_desc(m.desc[j].c_str());

}

for (j=0; j<(m.fcst_thr.n_elements()); ++j)  {

   add_fcst_thr(m.fcst_thr[j].c_str());

}

for (j=0; j<(m.obs_thr.n_elements()); ++j)  {

   add_obs_thr(m.obs_thr[j].c_str());

}

for (j=0; j<(m.fcst_var.n_elements()); ++j)  {

   add_fcst_var(m.fcst_var[j].c_str());

}

for (j=0; j<(m.fcst_units.n_elements()); ++j)  {

   add_fcst_units(m.fcst_units[j].c_str());

}

for (j=0; j<(m.fcst_lev.n_elements()); ++j)  {

   add_fcst_lev(m.fcst_lev[j].c_str());

}

for (j=0; j<(m.obs_var.n_elements()); ++j)  {

   add_obs_var(m.obs_var[j].c_str());

}

for (j=0; j<(m.obs_units.n_elements()); ++j)  {

   add_obs_units(m.obs_units[j].c_str());

}

for (j=0; j<(m.obs_lev.n_elements()); ++j)  {

   add_obs_lev(m.obs_lev[j].c_str());

}




   //
   //  int array members  (add)
   //

for (j=0; j<(m.fcst_lead.n_elements()); ++j)  {

   add_fcst_lead(m.fcst_lead[j]);

}

for (j=0; j<(m.fcst_valid_hour.n_elements()); ++j)  {

   add_fcst_valid_hour(m.fcst_valid_hour[j]);

}

for (j=0; j<(m.fcst_init_hour.n_elements()); ++j)  {

   add_fcst_init_hour(m.fcst_init_hour[j]);

}

for (j=0; j<(m.fcst_accum.n_elements()); ++j)  {

   add_fcst_accum(m.fcst_accum[j]);

}

for (j=0; j<(m.obs_lead.n_elements()); ++j)  {

   add_obs_lead(m.obs_lead[j]);

}

for (j=0; j<(m.obs_valid_hour.n_elements()); ++j)  {

   add_obs_valid_hour(m.obs_valid_hour[j]);

}

for (j=0; j<(m.obs_init_hour.n_elements()); ++j)  {

   add_obs_init_hour(m.obs_init_hour[j]);

}

for (j=0; j<(m.obs_accum.n_elements()); ++j)  {

   add_obs_accum(m.obs_accum[j]);

}

for (j=0; j<(m.fcst_rad.n_elements()); ++j)  {

   add_fcst_rad(m.fcst_rad[j]);

}

for (j=0; j<(m.obs_rad.n_elements()); ++j)  {

   add_obs_rad(m.obs_rad[j]);

}




   //
   //  int max/min members  (override)
   //

if ( m.area_min_set )               set_area_min(m.area_min);
if ( m.area_max_set )               set_area_max(m.area_max);

if ( m.area_thresh_min_set )        set_area_thresh_min(m.area_thresh_min);
if ( m.area_thresh_max_set )        set_area_thresh_max(m.area_thresh_max);

if ( m.intersection_area_min_set )  set_intersection_area_min(m.intersection_area_min);
if ( m.intersection_area_max_set )  set_intersection_area_max(m.intersection_area_max);

if ( m.union_area_min_set )         set_union_area_min(m.union_area_min);
if ( m.union_area_max_set )         set_union_area_max(m.union_area_max);

if ( m.symmetric_diff_min_set )     set_symmetric_diff_min(m.symmetric_diff_min);
if ( m.symmetric_diff_max_set )     set_symmetric_diff_max(m.symmetric_diff_max);




   //
   //  unixtime max/min members  (override)
   //

if ( m.fcst_valid_min_set )  set_fcst_valid_min(m.fcst_valid_min);
if ( m.fcst_valid_max_set )  set_fcst_valid_max(m.fcst_valid_max);

if ( m.obs_valid_min_set )   set_obs_valid_min(m.obs_valid_min);
if ( m.obs_valid_max_set )   set_obs_valid_max(m.obs_valid_max);

if ( m.fcst_init_min_set )   set_fcst_init_min(m.fcst_init_min);
if ( m.fcst_init_max_set )   set_fcst_init_max(m.fcst_init_max);

if ( m.obs_init_min_set )    set_obs_init_min(m.obs_init_min);
if ( m.obs_init_max_set )    set_obs_init_max(m.obs_init_max);




   //
   //  double max/min members  (override)
   //

if ( m.centroid_x_min_set )                  set_centroid_x_min(m.centroid_x_min);
if ( m.centroid_x_max_set )                  set_centroid_x_max(m.centroid_x_max);

if ( m.centroid_y_min_set )                  set_centroid_y_min(m.centroid_y_min);
if ( m.centroid_y_max_set )                  set_centroid_y_max(m.centroid_y_max);

if ( m.centroid_lat_min_set )                set_centroid_lat_min(m.centroid_lat_min);
if ( m.centroid_lat_max_set )                set_centroid_lat_max(m.centroid_lat_max);

if ( m.centroid_lon_min_set )                set_centroid_lon_min(m.centroid_lon_min);
if ( m.centroid_lon_max_set )                set_centroid_lon_max(m.centroid_lon_max);

if ( m.axis_ang_min_set )                    set_axis_ang_min(m.axis_ang_min);
if ( m.axis_ang_max_set )                    set_axis_ang_max(m.axis_ang_max);

if ( m.length_min_set )                      set_length_min(m.length_min);
if ( m.length_max_set )                      set_length_max(m.length_max);

if ( m.width_min_set )                       set_width_min(m.width_min);
if ( m.width_max_set )                       set_width_max(m.width_max);

if ( m.aspect_ratio_min_set )                set_aspect_ratio_min(m.aspect_ratio_min);
if ( m.aspect_ratio_max_set )                set_aspect_ratio_max(m.aspect_ratio_max);

if ( m.curvature_min_set )                   set_curvature_min(m.curvature_min);
if ( m.curvature_max_set )                   set_curvature_max(m.curvature_max);

if ( m.curvature_x_min_set )                 set_curvature_x_min(m.curvature_x_min);
if ( m.curvature_x_max_set )                 set_curvature_x_max(m.curvature_x_max);

if ( m.curvature_y_min_set )                 set_curvature_y_min(m.curvature_y_min);
if ( m.curvature_y_max_set )                 set_curvature_y_max(m.curvature_y_max);

if ( m.complexity_min_set )                  set_complexity_min(m.complexity_min);
if ( m.complexity_max_set )                  set_complexity_max(m.complexity_max);

if ( m.intensity_10_min_set )                set_intensity_10_min(m.intensity_10_min);
if ( m.intensity_10_max_set )                set_intensity_10_max(m.intensity_10_max);

if ( m.intensity_25_min_set )                set_intensity_25_min(m.intensity_25_min);
if ( m.intensity_25_max_set )                set_intensity_25_max(m.intensity_25_max);

if ( m.intensity_50_min_set )                set_intensity_50_min(m.intensity_50_min);
if ( m.intensity_50_max_set )                set_intensity_50_max(m.intensity_50_max);

if ( m.intensity_75_min_set )                set_intensity_75_min(m.intensity_75_min);
if ( m.intensity_75_max_set )                set_intensity_75_max(m.intensity_75_max);

if ( m.intensity_90_min_set )                set_intensity_90_min(m.intensity_90_min);
if ( m.intensity_90_max_set )                set_intensity_90_max(m.intensity_90_max);

if ( m.intensity_user_min_set )              set_intensity_user_min(m.intensity_user_min);
if ( m.intensity_user_max_set )              set_intensity_user_max(m.intensity_user_max);

if ( m.intensity_sum_min_set )               set_intensity_sum_min(m.intensity_sum_min);
if ( m.intensity_sum_max_set )               set_intensity_sum_max(m.intensity_sum_max);

if ( m.centroid_dist_min_set )               set_centroid_dist_min(m.centroid_dist_min);
if ( m.centroid_dist_max_set )               set_centroid_dist_max(m.centroid_dist_max);

if ( m.boundary_dist_min_set )               set_boundary_dist_min(m.boundary_dist_min);
if ( m.boundary_dist_max_set )               set_boundary_dist_max(m.boundary_dist_max);

if ( m.convex_hull_dist_min_set )            set_convex_hull_dist_min(m.convex_hull_dist_min);
if ( m.convex_hull_dist_max_set )            set_convex_hull_dist_max(m.convex_hull_dist_max);

if ( m.angle_diff_min_set )                  set_angle_diff_min(m.angle_diff_min);
if ( m.angle_diff_max_set )                  set_angle_diff_max(m.angle_diff_max);

if ( m.aspect_diff_min_set )                 set_aspect_diff_min(m.aspect_diff_min);
if ( m.aspect_diff_max_set )                 set_aspect_diff_max(m.aspect_diff_max);

if ( m.area_ratio_min_set )                  set_area_ratio_min(m.area_ratio_min);
if ( m.area_ratio_max_set )                  set_area_ratio_max(m.area_ratio_max);

if ( m.intersection_over_area_min_set )      set_intersection_over_area_min(m.intersection_over_area_min);
if ( m.intersection_over_area_max_set )      set_intersection_over_area_max(m.intersection_over_area_max);

if ( m.curvature_ratio_min_set )             set_curvature_ratio_min(m.curvature_ratio_min);
if ( m.curvature_ratio_max_set )             set_curvature_ratio_max(m.curvature_ratio_max);

if ( m.complexity_ratio_min_set )            set_complexity_ratio_min(m.complexity_ratio_min);
if ( m.complexity_ratio_max_set )            set_complexity_ratio_max(m.complexity_ratio_max);

if ( m.percentile_intensity_ratio_min_set )  set_percentile_intensity_ratio_min(m.percentile_intensity_ratio_min);
if ( m.percentile_intensity_ratio_max_set )  set_percentile_intensity_ratio_max(m.percentile_intensity_ratio_max);

if ( m.interest_min_set )                    set_interest_min(m.interest_min);
if ( m.interest_max_set )                    set_interest_max(m.interest_max);





   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_fcst()

{

is_fcst_toggle_set = 1;

is_fcst = 1;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_obs()

{

is_fcst_toggle_set = 1;

is_fcst = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_single()

{

is_single_toggle_set = 1;

is_single = 1;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_pair()

{

is_single_toggle_set = 1;

is_single = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_simple()

{

is_simple_toggle_set = 1;

is_simple = 1;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_cluster()

{

is_simple_toggle_set = 1;

is_simple = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_matched()

{

is_matched_toggle_set = 1;

is_matched = 1;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_unmatched()

{

is_matched_toggle_set = 1;

is_matched = 0;

return;

}


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_model(const char * text)

{

if ( !(model.has(text)) )   model.add(text);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_desc(const char * text)

{

if ( !(desc.has(text)) )   desc.add(text);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_fcst_thr(const char * text)

{

if ( !(fcst_thr.has(text)) )   fcst_thr.add(text);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_obs_thr(const char * text)

{

if ( !(obs_thr.has(text)) )   obs_thr.add(text);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_fcst_var(const char * text)

{

if ( !(fcst_var.has(text)) )   fcst_var.add(text);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_fcst_units(const char * text)

{

if ( !(fcst_units.has(text)) )   fcst_units.add(text);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_fcst_lev(const char * text)

{

if ( !(fcst_lev.has(text)) )   fcst_lev.add(text);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_obs_var(const char * text)

{

if ( !(obs_var.has(text)) )   obs_var.add(text);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_obs_units(const char * text)

{

if ( !(obs_units.has(text)) )   obs_units.add(text);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_obs_lev(const char * text)

{

if ( !(obs_lev.has(text)) )   obs_lev.add(text);

return;

}


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_fcst_lead(int i)

{

if ( !(fcst_lead.has(i)) )   fcst_lead.add(i);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_fcst_valid_hour(int i)

{

if ( !(fcst_valid_hour.has(i)) )   fcst_valid_hour.add(i);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_fcst_init_hour(int i)

{

if ( !(fcst_init_hour.has(i)) )   fcst_init_hour.add(i);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_fcst_accum(int i)

{

if ( !(fcst_accum.has(i)) )   fcst_accum.add(i);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_obs_lead(int i)

{

if ( !(obs_lead.has(i)) )   obs_lead.add(i);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_obs_valid_hour(int i)

{

if ( !(obs_valid_hour.has(i)) )   obs_valid_hour.add(i);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_obs_init_hour(int i)

{

if ( !(obs_init_hour.has(i)) )   obs_init_hour.add(i);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_obs_accum(int i)

{

if ( !(obs_accum.has(i)) )   obs_accum.add(i);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_fcst_rad(int i)

{

if ( !(fcst_rad.has(i)) )   fcst_rad.add(i);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::add_obs_rad(int i)

{

if ( !(obs_rad.has(i)) )   obs_rad.add(i);

return;

}


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_area_min(int i)

{

area_min_set = 1;

area_min = i;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_area_max(int i)

{

area_max_set = 1;

area_max = i;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_area_thresh_min(int i)

{

area_thresh_min_set = 1;

area_thresh_min = i;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_area_thresh_max(int i)

{

area_thresh_max_set = 1;

area_thresh_max = i;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intersection_area_min(int i)

{

intersection_area_min_set = 1;

intersection_area_min = i;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intersection_area_max(int i)

{

intersection_area_max_set = 1;

intersection_area_max = i;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_union_area_min(int i)

{

union_area_min_set = 1;

union_area_min = i;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_union_area_max(int i)

{

union_area_max_set = 1;

union_area_max = i;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_symmetric_diff_min(int i)

{

symmetric_diff_min_set = 1;

symmetric_diff_min = i;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_symmetric_diff_max(int i)

{

symmetric_diff_max_set = 1;

symmetric_diff_max = i;

return;

}


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_centroid_x_min(double x)

{

centroid_x_min_set = 1;

centroid_x_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_centroid_x_max(double x)

{

centroid_x_max_set = 1;

centroid_x_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_centroid_y_min(double x)

{

centroid_y_min_set = 1;

centroid_y_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_centroid_y_max(double x)

{

centroid_y_max_set = 1;

centroid_y_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_centroid_lat_min(double x)

{

centroid_lat_min_set = 1;

centroid_lat_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_centroid_lat_max(double x)

{

centroid_lat_max_set = 1;

centroid_lat_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_centroid_lon_min(double x)

{

centroid_lon_min_set = 1;

centroid_lon_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_centroid_lon_max(double x)

{

centroid_lon_max_set = 1;

centroid_lon_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_axis_ang_min(double x)

{

axis_ang_min_set = 1;

axis_ang_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_axis_ang_max(double x)

{

axis_ang_max_set = 1;

axis_ang_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_length_min(double x)

{

length_min_set = 1;

length_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_length_max(double x)

{

length_max_set = 1;

length_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_width_min(double x)

{

width_min_set = 1;

width_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_width_max(double x)

{

width_max_set = 1;

width_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_aspect_ratio_min(double x)

{

aspect_ratio_min_set = 1;

aspect_ratio_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_aspect_ratio_max(double x)

{

aspect_ratio_max_set = 1;

aspect_ratio_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_curvature_min(double x)

{

curvature_min_set = 1;

curvature_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_curvature_max(double x)

{

curvature_max_set = 1;

curvature_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_curvature_x_min(double x)

{

curvature_x_min_set = 1;

curvature_x_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_curvature_x_max(double x)

{

curvature_x_max_set = 1;

curvature_x_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_curvature_y_min(double x)

{

curvature_y_min_set = 1;

curvature_y_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_curvature_y_max(double x)

{

curvature_y_max_set = 1;

curvature_y_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_complexity_min(double x)

{

complexity_min_set = 1;

complexity_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_complexity_max(double x)

{

complexity_max_set = 1;

complexity_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intensity_10_min(double x)

{

intensity_10_min_set = 1;

intensity_10_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intensity_10_max(double x)

{

intensity_10_max_set = 1;

intensity_10_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intensity_25_min(double x)

{

intensity_25_min_set = 1;

intensity_25_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intensity_25_max(double x)

{

intensity_25_max_set = 1;

intensity_25_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intensity_50_min(double x)

{

intensity_50_min_set = 1;

intensity_50_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intensity_50_max(double x)

{

intensity_50_max_set = 1;

intensity_50_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intensity_75_min(double x)

{

intensity_75_min_set = 1;

intensity_75_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intensity_75_max(double x)

{

intensity_75_max_set = 1;

intensity_75_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intensity_90_min(double x)

{

intensity_90_min_set = 1;

intensity_90_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intensity_90_max(double x)

{

intensity_90_max_set = 1;

intensity_90_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intensity_user_min(double x)

{

intensity_user_min_set = 1;

intensity_user_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intensity_user_max(double x)

{

intensity_user_max_set = 1;

intensity_user_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intensity_sum_min(double x)

{

intensity_sum_min_set = 1;

intensity_sum_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intensity_sum_max(double x)

{

intensity_sum_max_set = 1;

intensity_sum_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_centroid_dist_min(double x)

{

centroid_dist_min_set = 1;

centroid_dist_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_centroid_dist_max(double x)

{

centroid_dist_max_set = 1;

centroid_dist_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_boundary_dist_min(double x)

{

boundary_dist_min_set = 1;

boundary_dist_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_boundary_dist_max(double x)

{

boundary_dist_max_set = 1;

boundary_dist_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_convex_hull_dist_min(double x)

{

convex_hull_dist_min_set = 1;

convex_hull_dist_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_convex_hull_dist_max(double x)

{

convex_hull_dist_max_set = 1;

convex_hull_dist_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_angle_diff_min(double x)

{

angle_diff_min_set = 1;

angle_diff_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_angle_diff_max(double x)

{

angle_diff_max_set = 1;

angle_diff_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_aspect_diff_min(double x)

{

aspect_diff_min_set = 1;

aspect_diff_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_aspect_diff_max(double x)

{

aspect_diff_max_set = 1;

aspect_diff_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_area_ratio_min(double x)

{

area_ratio_min_set = 1;

area_ratio_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_area_ratio_max(double x)

{

area_ratio_max_set = 1;

area_ratio_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intersection_over_area_min(double x)

{

intersection_over_area_min_set = 1;

intersection_over_area_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_intersection_over_area_max(double x)

{

intersection_over_area_max_set = 1;

intersection_over_area_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_curvature_ratio_min(double x)

{

curvature_ratio_min_set = 1;

curvature_ratio_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_curvature_ratio_max(double x)

{

curvature_ratio_max_set = 1;

curvature_ratio_max = x;

return;

}



////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_complexity_ratio_min(double x)

{

complexity_ratio_min_set = 1;

complexity_ratio_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_complexity_ratio_max(double x)

{

complexity_ratio_max_set = 1;

complexity_ratio_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_percentile_intensity_ratio_min(double x)

{

percentile_intensity_ratio_min_set = 1;

percentile_intensity_ratio_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_percentile_intensity_ratio_max(double x)

{

percentile_intensity_ratio_max_set = 1;

percentile_intensity_ratio_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_interest_min(double x)

{

interest_min_set = 1;

interest_min = x;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_interest_max(double x)

{

interest_max_set = 1;

interest_max = x;

return;

}


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_fcst_valid_min(unixtime t)

{

fcst_valid_min_set = 1;

fcst_valid_min = t;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_fcst_valid_max(unixtime t)

{

fcst_valid_max_set = 1;

fcst_valid_max = t;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_obs_valid_min(unixtime t)

{

obs_valid_min_set = 1;

obs_valid_min = t;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_obs_valid_max(unixtime t)

{

obs_valid_max_set = 1;

obs_valid_max = t;

return;

}

////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_fcst_init_min(unixtime t)

{

fcst_init_min_set = 1;

fcst_init_min = t;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_fcst_init_max(unixtime t)

{

fcst_init_max_set = 1;

fcst_init_max = t;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_obs_init_min(unixtime t)

{

obs_init_min_set = 1;

obs_init_min = t;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_obs_init_max(unixtime t)

{

obs_init_max_set = 1;

obs_init_max = t;

return;

}


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////


void ModeAttributes::set_mask(const char * filename)

{

if ( !poly )  poly = new MaskPoly;

poly->load(filename);

return;

}


////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void mode_atts_usage(ostream & s)

{


s << "\n"
  << "toggles\n"
  << "=======\n"
  << "\n";


s << "   -fcst\n"
  << "   -obs\n\n";

s << "   -single\n"
  << "   -pair\n\n";

s << "   -simple\n"
  << "   -cluster\n\n";

s << "   -matched\n"
  << "   -unmatched\n\n";

s << "multiple set string options\n"
  << "===========================\n"
  << "\n";
s << "   -model    value\n";
s << "   -desc     value\n";
s << "   -fcst_thr value\n";
s << "   -obs_thr  value\n";
s << "   -fcst_var value\n";
s << "   -fcst_units value\n";
s << "   -fcst_lev value\n";
s << "   -obs_var  value\n";
s << "   -obs_units  value\n";
s << "   -obs_lev  value\n";

s << "\n"
  << "multiple set integer options\n"
  << "============================\n"
  << "\n";
s << "   -fcst_lead       hh[mmss]\n";
s << "   -fcst_valid_hour hh[mmss]\n";
s << "   -fcst_init_hour  hh[mmss]\n";
s << "   -fcst_accum      hh[mmss]\n";
s << "   -obs_lead        hh[mmss]\n";
s << "   -obs_valid_hour  hh[mmss]\n";
s << "   -obs_init_hour   hh[mmss]\n";
s << "   -obs_accum       hh[mmss]\n";
s << "   -fcst_rad        value\n";
s << "   -obs_rad         value\n";

s << "\n"
  << "integer max/min options\n"
  << "=======================\n"
  << "\n";
s << "   -area_min              value\n";
s << "   -area_max              value\n\n";
s << "   -area_thresh_min       value\n";
s << "   -area_thresh_max       value\n\n";
s << "   -intersection_area_min value\n";
s << "   -intersection_area_max value\n\n";
s << "   -union_area_min        value\n";
s << "   -union_area_max        value\n\n";
s << "   -symmetric_diff_min    value\n";
s << "   -symmetric_diff_max    value\n\n";

s << "\n"
  << "date/time max/min options\n"
  << "=========================\n"
  << "\n";
s << "   -fcst_valid_min yyyymmdd[_hh[mmss]]\n";
s << "   -fcst_valid_max yyyymmdd[_hh[mmss]]\n\n";
s << "   -obs_valid_min  yyyymmdd[_hh[mmss]]\n";
s << "   -obs_valid_max  yyyymmdd[_hh[mmss]]\n\n";
s << "   -fcst_init_min  yyyymmdd[_hh[mmss]]\n";
s << "   -fcst_init_max  yyyymmdd[_hh[mmss]]\n\n";
s << "   -obs_init_min   yyyymmdd[_hh[mmss]]\n";
s << "   -obs_init_max   yyyymmdd[_hh[mmss]]\n\n";

s << "\n"
  << "floating-point max/min options\n"
  << "==============================\n"
  << "\n";
s << "   -centroid_x_min                 value\n";
s << "   -centroid_x_max                 value\n\n";
s << "   -centroid_y_min                 value\n";
s << "   -centroid_y_max                 value\n\n";
s << "   -centroid_lat_min               value\n";
s << "   -centroid_lat_max               value\n\n";
s << "   -centroid_lon_min               value\n";
s << "   -centroid_lon_max               value\n\n";
s << "   -axis_ang_min                   value\n";
s << "   -axis_ang_max                   value\n\n";
s << "   -length_min                     value\n";
s << "   -length_max                     value\n\n";
s << "   -width_min                      value\n";
s << "   -width_max                      value\n\n";
s << "   -aspect_ratio_min               value\n";
s << "   -aspect_ratio_max               value\n\n";
s << "   -curvature_min                  value\n";
s << "   -curvature_max                  value\n\n";
s << "   -curvature_x_min                value\n";
s << "   -curvature_x_max                value\n\n";
s << "   -curvature_y_min                value\n";
s << "   -curvature_y_max                value\n\n";
s << "   -complexity_min                 value\n";
s << "   -complexity_max                 value\n\n";
s << "   -intensity_10_min               value\n";
s << "   -intensity_10_max               value\n\n";
s << "   -intensity_25_min               value\n";
s << "   -intensity_25_max               value\n\n";
s << "   -intensity_50_min               value\n";
s << "   -intensity_50_max               value\n\n";
s << "   -intensity_75_min               value\n";
s << "   -intensity_75_max               value\n\n";
s << "   -intensity_90_min               value\n";
s << "   -intensity_90_max               value\n\n";
s << "   -intensity_user_min             value\n";
s << "   -intensity_user_max             value\n\n";
s << "   -intensity_sum_min              value\n";
s << "   -intensity_sum_max              value\n\n";
s << "   -centroid_dist_min              value\n";
s << "   -centroid_dist_max              value\n\n";
s << "   -boundary_dist_min              value\n";
s << "   -boundary_dist_max              value\n\n";
s << "   -convex_hull_dist_min           value\n";
s << "   -convex_hull_dist_max           value\n\n";
s << "   -angle_diff_min                 value\n";
s << "   -angle_diff_max                 value\n\n";
s << "   -aspect_diff_min                value\n";
s << "   -aspect_diff_max                value\n\n";
s << "   -area_ratio_min                 value\n";
s << "   -area_ratio_max                 value\n\n";
s << "   -intersection_over_area_min     value\n";
s << "   -intersection_over_area_max     value\n\n";
s << "   -curvature_ratio_min            value\n";
s << "   -curvature_ratio_max            value\n\n";
s << "   -complexity_ratio_min           value\n";
s << "   -complexity_ratio_max           value\n\n";
s << "   -percentile_intensity_ratio_min value\n";
s << "   -percentile_intensity_ratio_max value\n\n";
s << "   -interest_min                   value\n";
s << "   -interest_max                   value\n\n";

s << "\n"
  << "miscellaneous options\n"
  << "=====================\n"
  << "\n";
s << "   -mask_poly filename\n\n";


return;

}


////////////////////////////////////////////////////////////////////////


