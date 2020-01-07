// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <cmath>

#include "mode_conf_info.h"
#include "configobjecttype_to_string.h"

#include "vx_data2d_factory.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class ModeConfInfo
//
////////////////////////////////////////////////////////////////////////

ModeConfInfo::ModeConfInfo()

{

   init_from_scratch();

}

////////////////////////////////////////////////////////////////////////

ModeConfInfo::~ModeConfInfo()

{

   clear();
}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::init_from_scratch()

{

   // Initialize pointers
   fcst_info = (VarInfo *) 0;
   obs_info  = (VarInfo *) 0;

   clear();

   return;

}

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::clear()

{

   // Initialize values

   model.clear();
   desc.clear();
   obtype.clear();

   mask_missing_flag = FieldType_None;

   grid_res = bad_data_double;

   fcst_conv_radius_array.clear();
    obs_conv_radius_array.clear();

   fcst_conv_radius = bad_data_int;
   obs_conv_radius  = bad_data_int;

   fcst_conv_thresh_array.clear();
   obs_conv_thresh_array.clear();

   fcst_merge_thresh_array.clear();
   obs_merge_thresh_array.clear();

   fcst_conv_thresh.clear();
   obs_conv_thresh.clear();

   fcst_vld_thresh = bad_data_double;
   obs_vld_thresh = bad_data_double;

   fcst_filter_attr_map.clear();
   obs_filter_attr_map.clear();

   fcst_merge_flag = MergeType_None;
   obs_merge_flag = MergeType_None;

   match_flag = MatchType_None;

   max_centroid_dist = bad_data_double;

   mask_grid_name.clear();
   mask_grid_flag = FieldType_None;

   mask_poly_name.clear();
   mask_poly_flag = FieldType_None;

   centroid_dist_wt    = bad_data_double;
   boundary_dist_wt    = bad_data_double;
   convex_hull_dist_wt = bad_data_double;
   angle_diff_wt       = bad_data_double;
   aspect_diff_wt      = bad_data_double;
   area_ratio_wt       = bad_data_double;
   int_area_ratio_wt   = bad_data_double;
   curvature_ratio_wt  = bad_data_double;
   complexity_ratio_wt = bad_data_double;
   inten_perc_ratio_wt = bad_data_double;


   inten_perc_value = bad_data_int;

   centroid_dist_if    = (PiecewiseLinear *) 0;
   boundary_dist_if    = (PiecewiseLinear *) 0;
   convex_hull_dist_if = (PiecewiseLinear *) 0;
   angle_diff_if       = (PiecewiseLinear *) 0;
   aspect_diff_if      = (PiecewiseLinear *) 0;
   area_ratio_if       = (PiecewiseLinear *) 0;
   int_area_ratio_if   = (PiecewiseLinear *) 0;
   curvature_ratio_if  = (PiecewiseLinear *) 0;
   complexity_ratio_if = (PiecewiseLinear *) 0;
   inten_perc_ratio_if = (PiecewiseLinear *) 0;

   total_interest_thresh = bad_data_double;

   print_interest_thresh = bad_data_double;

   met_data_dir.clear();

   plot_valid_flag = false;
   plot_gcarc_flag = false;
   ps_plot_flag    = false;

   nc_info.set_all_true();

   ct_stats_flag   = false;

   shift_right = 0;

   output_prefix.clear();

   version.clear();

   quilt = false;

   // Deallocate memory
   if(fcst_info) { delete fcst_info; fcst_info = (VarInfo *) 0; }
   if(obs_info)  { delete obs_info;  obs_info  = (VarInfo *) 0; }

   return;

}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::read_config(const char *default_file_name, const char *user_file_name)

{

   // Read the config file constants
   conf.read(replace_path(config_const_filename).c_str());
   conf.read(replace_path(config_map_data_filename).c_str());

   // Read the default config file
   conf.read(default_file_name);

   // Read the user-specified config file
   conf.read(user_file_name);

   nc_info.set_compress_level(conf.nc_compression());

   return;

}

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::process_config(GrdFileType ftype, GrdFileType otype)

{

int j, n;
VarInfoFactory info_factory;
Dictionary *fcst_dict = (Dictionary *) 0;
Dictionary *obs_dict  = (Dictionary *) 0;
Dictionary *dict      = (Dictionary *) 0;
PlotInfo plot_info;

      // Dump the contents of the config file

   if(mlog.verbosity_level() >= 5) conf.dump(cout);

      // Initialize

   clear();

      // Conf: version

   version = parse_conf_version(&conf);

      // Conf: model

   model = parse_conf_string(&conf, conf_key_model);

      // Conf: desc

   desc = parse_conf_string(&conf, conf_key_desc);

      // Conf: obtype

   obtype = parse_conf_string(&conf, conf_key_obtype);

      //  grid_res

   grid_res = conf.lookup_double(conf_key_grid_res);

      //  quilt

   quilt = conf.lookup_bool(conf_key_quilt);

      // Conf: fcst and obs

   fcst_dict = conf.lookup_dictionary(conf_key_fcst);
   obs_dict  = conf.lookup_dictionary(conf_key_obs);

      // Allocate new VarInfo objects

   fcst_info = info_factory.new_var_info(ftype);
   obs_info  = info_factory.new_var_info(otype);

      // Set the dictionaries

   fcst_info->set_dict(*(fcst_dict->lookup_dictionary(conf_key_field)));
   obs_info->set_dict(*(obs_dict->lookup_dictionary(conf_key_field)));

      // Dump the contents of the VarInfo objects

   if(mlog.verbosity_level() >= 5)  {
      mlog << Debug(5)
           << "Parsed forecast field:\n";
      fcst_info->dump(cout);
      mlog << Debug(5)
           << "Parsed observation field:\n";
      obs_info->dump(cout);
   }

      // No support for wind direction

   if(fcst_info->is_wind_direction() || obs_info->is_wind_direction())  {
      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "the wind direction field may not be verified "
           << "using MODE.\n\n";
      exit(1);
   }

      // Conf: fcst.raw_thresh and obs.raw_thresh are deprecated

   if ( fcst_dict->lookup(conf_key_raw_thresh) ||
         obs_dict->lookup(conf_key_raw_thresh) )  {
      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "the \"" << conf_key_raw_thresh << "\" entry is deprecated in MET "
           << met_version << "!  Use \"" << conf_key_censor_thresh << "\" and \""
           << conf_key_censor_val << "\" instead.\n\n";
      exit(1);
   }

      // Conf: fcst.conv_radius and obs.conv_radius

   fcst_conv_radius_array = fcst_dict->lookup_int_array(conf_key_conv_radius);
    obs_conv_radius_array =  obs_dict->lookup_int_array(conf_key_conv_radius);

   if ( fcst_conv_radius_array.n_elements() != obs_conv_radius_array.n_elements() )  {

      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "fcst and obs convolution radius arrays need to be the same size\n\n";

      exit ( 1 );

   }

      // Check that fcst_conv_radius and obs_conv_radius are non-negative

   n = fcst_conv_radius_array.n_elements();   //  same as obs_conv_radius_array.n_elements()

   for (j=0; j<n; ++j)  {

      if(fcst_conv_radius_array[j] < 0 || obs_conv_radius_array[j] < 0) {

         mlog << Error << "\nModeConfInfo::process_config() -> "
              << "fcst_conv_radius (" << fcst_conv_radius_array[j]
              << ") and obs_conv_radius (" << obs_conv_radius_array[j]
              << ") must be non-negative\n\n";

         exit(1);

      }

   }

   if ( fcst_conv_radius_array.n_elements() == 1 )  fcst_conv_radius = fcst_conv_radius_array[0];
   if (  obs_conv_radius_array.n_elements() == 1 )   obs_conv_radius =  obs_conv_radius_array[0];

      // Conf: fcst.conv_thresh and obs.conv_thresh

   fcst_conv_thresh_array = fcst_dict->lookup_thresh_array(conf_key_conv_thresh);
    obs_conv_thresh_array =  obs_dict->lookup_thresh_array(conf_key_conv_thresh);

   if ( fcst_conv_thresh_array.n_elements() != obs_conv_thresh_array.n_elements() )  {

      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "fcst and obs convolution threshold arrays need to be the same size\n\n";

      exit ( 1 );

   }

   if ( fcst_conv_thresh_array.n_elements() == 1 )  fcst_conv_thresh = fcst_conv_thresh_array[0];
   if (  obs_conv_thresh_array.n_elements() == 1 )   obs_conv_thresh =  obs_conv_thresh_array[0];

      // Conf: fcst.vld_thresh and obs.vld_thresh

   fcst_vld_thresh = fcst_dict->lookup_double(conf_key_vld_thresh);
   obs_vld_thresh  = obs_dict->lookup_double(conf_key_vld_thresh);

      // Conf: fcst.filter_attr_name and fcst.filter_attr_thresh
      //        obs.filter_attr_name and  obs.filter_attr_thresh

   fcst_filter_attr_map = parse_conf_filter_attr_map(fcst_dict);
    obs_filter_attr_map = parse_conf_filter_attr_map(obs_dict);

      // Conf: fcst.merge_flag and obs.merge_flag

   fcst_merge_flag = int_to_mergetype(fcst_dict->lookup_int(conf_key_merge_flag));
   obs_merge_flag  = int_to_mergetype(obs_dict->lookup_int(conf_key_merge_flag));

      // Conf: fcst.merge_thresh and obs.merge_thresh

   fcst_merge_thresh_array = fcst_dict->lookup_thresh_array(conf_key_merge_thresh);
    obs_merge_thresh_array =  obs_dict->lookup_thresh_array(conf_key_merge_thresh);

   if ( need_fcst_merge_thresh() && (fcst_merge_thresh_array.n_elements() != fcst_conv_thresh_array.n_elements()) )  {

      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "fcst conv thresh and fcst merge thresh arrays need to be the same size\n\n";

      exit ( 1 );

   }


   if ( need_obs_merge_thresh() && (obs_merge_thresh_array.n_elements() != obs_conv_thresh_array.n_elements()) )  {

      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "obs conv thresh and obs merge thresh arrays need to be the same size\n\n";

      exit ( 1 );

   }

   if ( fcst_merge_thresh_array.n_elements() == 1 )  fcst_merge_thresh = fcst_merge_thresh_array[0];
   if (  obs_merge_thresh_array.n_elements() == 1 )   obs_merge_thresh =  obs_merge_thresh_array[0];

      // Conf: mask_missing_flag

   mask_missing_flag = int_to_fieldtype(conf.lookup_int(conf_key_mask_missing_flag));

      // Conf: match_flag

   match_flag = int_to_matchtype(conf.lookup_int(conf_key_match_flag));

      // Check that match_flag is set between 0 and 3

   if(match_flag == MatchType_None &&
      (fcst_merge_flag != MergeType_None || obs_merge_flag  != MergeType_None) ) {
      mlog << Warning << "\nModeConfInfo::process_config() -> "
           << "When matching is disabled (match_flag = "
           << matchtype_to_string(match_flag)
           << ") but merging is requested (fcst_merge_flag = "
           << mergetype_to_string(fcst_merge_flag)
           << ", obs_merge_flag = "
           << mergetype_to_string(obs_merge_flag)
           << ") any merging information will be discarded.\n\n";
   }

      // Conf: max_centroid_dist

   max_centroid_dist = conf.lookup_double(conf_key_max_centroid_dist);

      // Check that max_centroid_dist is > 0

   if(max_centroid_dist <= 0) {
      mlog << Warning << "\nModeConfInfo::process_config() -> "
           << "max_centroid_dist (" << max_centroid_dist
           << ") should be set > 0\n\n";
   }

      // Conf: mask.grid

   mask_grid_name = conf.lookup_string(conf_key_mask_grid);
   mask_grid_flag = int_to_fieldtype(conf.lookup_int(conf_key_mask_grid_flag));

      // Conf: mask.poly

   mask_poly_name = conf.lookup_string(conf_key_mask_poly);
   mask_poly_flag = int_to_fieldtype(conf.lookup_int(conf_key_mask_poly_flag));

      // Conf: weight

   dict = conf.lookup_dictionary(conf_key_weight);

      // Parse the weights

   centroid_dist_wt    = dict->lookup_double(conf_key_centroid_dist);
   boundary_dist_wt    = dict->lookup_double(conf_key_boundary_dist);
   convex_hull_dist_wt = dict->lookup_double(conf_key_convex_hull_dist);
   angle_diff_wt       = dict->lookup_double(conf_key_angle_diff);
   aspect_diff_wt      = dict->lookup_double(conf_key_aspect_diff);
   area_ratio_wt       = dict->lookup_double(conf_key_area_ratio);
   int_area_ratio_wt   = dict->lookup_double(conf_key_int_area_ratio);
   curvature_ratio_wt  = dict->lookup_double(conf_key_curvature_ratio);
   complexity_ratio_wt = dict->lookup_double(conf_key_complexity_ratio);
   inten_perc_ratio_wt = dict->lookup_double(conf_key_inten_perc_ratio);
   inten_perc_value    = dict->lookup_int(conf_key_inten_perc_value);

      // Check that intensity_percentile >= 0 and <= 102

   if(inten_perc_value < 0 || inten_perc_value > 102) {
      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "inten_perc_value (" << inten_perc_value
           << ") must be set between 0 and 102.\n\n";
      exit(1);
   }

      // Check that the fuzzy engine weights are non-negative

   if(centroid_dist_wt    < 0 || boundary_dist_wt    < 0 ||
      convex_hull_dist_wt < 0 || angle_diff_wt       < 0 ||
      aspect_diff_wt      < 0 || area_ratio_wt       < 0 ||
      int_area_ratio_wt   < 0 || curvature_ratio_wt  < 0 ||
      complexity_ratio_wt < 0 || inten_perc_ratio_wt < 0) {
      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "All of the fuzzy engine weights must be >= 0.\n\n";
      exit(1);
   }

      // Check that the sum of the weights is non-zero for matching

   if(match_flag != MatchType_None &&
      is_eq(centroid_dist_wt    + boundary_dist_wt   +
            convex_hull_dist_wt + angle_diff_wt      +
            aspect_diff_wt      + area_ratio_wt      +
            int_area_ratio_wt   + curvature_ratio_wt +
            complexity_ratio_wt + inten_perc_ratio_wt, 0.0)) {
      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "When matching is requested, the sum of the fuzzy engine "
           << "weights cannot be 0\n\n";
      exit(1);
   }

      // Conf: interest_function

   dict = conf.lookup_dictionary(conf_key_interest_function);

      // Parse the interest functions

   centroid_dist_if    = dict->lookup_pwl(conf_key_centroid_dist);
   boundary_dist_if    = dict->lookup_pwl(conf_key_boundary_dist);
   convex_hull_dist_if = dict->lookup_pwl(conf_key_convex_hull_dist);
   angle_diff_if       = dict->lookup_pwl(conf_key_angle_diff);
   aspect_diff_if      = dict->lookup_pwl(conf_key_aspect_diff);
   area_ratio_if       = dict->lookup_pwl(conf_key_area_ratio);
   int_area_ratio_if   = dict->lookup_pwl(conf_key_int_area_ratio);
   curvature_ratio_if  = dict->lookup_pwl(conf_key_curvature_ratio);
   complexity_ratio_if = dict->lookup_pwl(conf_key_complexity_ratio);
   inten_perc_ratio_if = dict->lookup_pwl(conf_key_inten_perc_ratio);

      // Conf: total_interest_thresh

   total_interest_thresh = conf.lookup_double(conf_key_total_interest_thresh);

      // Check that total_interest_thresh is between 0 and 1.

   if(total_interest_thresh < 0 || total_interest_thresh > 1) {
      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "total_interest_thresh (" << total_interest_thresh
           << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

      // Conf: print_interest_thresh

   print_interest_thresh = conf.lookup_double(conf_key_print_interest_thresh);

      // Check that print_interest_thresh is between 0 and 1.

   if(print_interest_thresh < 0 || print_interest_thresh > 1) {
      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "print_interest_thresh (" << print_interest_thresh
           << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

      // Conf: met_data_dir

   met_data_dir = replace_path(conf.lookup_string(conf_key_met_data_dir).c_str());

      // Conf: fcst_raw_plot

   fcst_raw_pi = parse_conf_plot_info(conf.lookup_dictionary(conf_key_fcst_raw_plot));

      // Conf: obs_raw_plot

   obs_raw_pi = parse_conf_plot_info(conf.lookup_dictionary(conf_key_obs_raw_plot));

      // Conf: object_plot

   object_pi = parse_conf_plot_info(conf.lookup_dictionary(conf_key_object_plot));

      // Conf: plot_valid_flag

   plot_valid_flag = conf.lookup_bool(conf_key_plot_valid_flag);

      // Conf: plot_gcarc_flag

   plot_gcarc_flag = conf.lookup_bool(conf_key_plot_gcarc_flag);

      // Conf: ps_plot_flag

   ps_plot_flag = conf.lookup_bool(conf_key_ps_plot_flag);

      // Conf: nc_pairs_flag

   parse_nc_info();

      // Conf: ct_stats_flag

   ct_stats_flag = conf.lookup_bool(conf_key_ct_stats_flag);

      // Conf: output_prefix

   output_prefix = conf.lookup_string(conf_key_output_prefix);

      // Conf: shift_right

   shift_right = fcst_dict->lookup_int(conf_key_shift_right);

   return;

}


////////////////////////////////////////////////////////////////////////


void ModeConfInfo::set_perc_thresh(const DataPlane &f_dp,
                                   const DataPlane &o_dp)

{

   //
   // Compute percentiles for forecast and observation thresholds.
   //
   if( !fcst_conv_thresh_array.need_perc()  &&
       !obs_conv_thresh_array.need_perc()   &&
       !fcst_merge_thresh_array.need_perc() &&
       !obs_merge_thresh_array.need_perc() )  return;

   //
   // Sort the input arrays
   //
   NumArray fsort, osort;
   int nxy = f_dp.nx() * f_dp.ny();

   fsort.extend(nxy);
   osort.extend(nxy);

   for(int i=0; i<nxy; i++) {
      if(!is_bad_data(f_dp.data()[i])) fsort.add(f_dp.data()[i]);
      if(!is_bad_data(o_dp.data()[i])) osort.add(o_dp.data()[i]);
   }

   fsort.sort_array();
   osort.sort_array();

   //
   // Compute percentiles
   //
    fcst_conv_thresh_array.set_perc(&fsort, &osort, (NumArray *) 0,
                                    &fcst_conv_thresh_array,
                                     &obs_conv_thresh_array);
     obs_conv_thresh_array.set_perc(&fsort, &osort, (NumArray *) 0,
                                    &fcst_conv_thresh_array,
                                     &obs_conv_thresh_array);
   fcst_merge_thresh_array.set_perc(&fsort, &osort, (NumArray *) 0,
                                    &fcst_merge_thresh_array,
                                     &obs_merge_thresh_array);
    obs_merge_thresh_array.set_perc(&fsort, &osort, (NumArray *) 0,
                                    &fcst_merge_thresh_array,
                                     &obs_merge_thresh_array);

   return;

}


////////////////////////////////////////////////////////////////////////


void ModeConfInfo::parse_nc_info()

{

const DictionaryEntry * e = (const DictionaryEntry *) 0;

e = conf.lookup(conf_key_nc_pairs_flag);

if ( !e )  {

   mlog << Error << "\nModeConfInfo::parse_nc_info() -> "
        << "lookup failed for key \"" << conf_key_nc_pairs_flag
        << "\"\n\n";

   exit ( 1 );

}

const ConfigObjectType type = e->type();

if ( type == BooleanType )  {

   bool value = e->b_value();

   if ( ! value )  nc_info.set_all_false();

   return;

}

   //
   //  it should be a dictionary
   //

if ( type != DictionaryType )  {

   mlog << Error << "\nModeConfInfo::parse_nc_info() -> "
        << "bad type (" << configobjecttype_to_string(type)
        << ") for key \"" << conf_key_nc_pairs_flag << "\"\n\n";

   exit ( 1 );

}

   //
   //  parse the various entries
   //

Dictionary * d = e->dict_value();

nc_info.do_latlon     = d->lookup_bool(conf_key_latlon_flag);
nc_info.do_raw        = d->lookup_bool(conf_key_raw_flag);
nc_info.do_object_raw = d->lookup_bool(conf_key_object_raw_flag);
nc_info.do_object_id  = d->lookup_bool(conf_key_object_id_flag);
nc_info.do_cluster_id = d->lookup_bool(conf_key_cluster_id_flag);
nc_info.do_polylines  = d->lookup_bool(conf_key_polylines_flag);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModeConfInfo::set_conv_radius_by_index(int k)

{

   //
   //  we already know that the fcst and obs radius arrays have
   //    the same number of elements
   //

if ( (k < 0) || (k >= fcst_conv_radius_array.n_elements()) )  {

   mlog << Error << "\nModeConfInfo::set_conv_radius_by_index(int) -> "
        << "range check error\n\n";

   exit ( 1 );

}

fcst_conv_radius = fcst_conv_radius_array[k];
 obs_conv_radius =  obs_conv_radius_array[k];

return;

}


////////////////////////////////////////////////////////////////////////


void ModeConfInfo::set_conv_thresh_by_index(int k)

{

   //
   //  we already know that the fcst and obs threshold arrays have
   //    the same number of elements
   //

if ( (k < 0) || (k >= fcst_conv_thresh_array.n_elements()) )  {

   mlog << Error << "\nModeConfInfo::set_conv_thresh_by_index(int) -> "
        << "range check error\n\n";

   exit ( 1 );

}

fcst_conv_thresh = fcst_conv_thresh_array[k];
 obs_conv_thresh =  obs_conv_thresh_array[k];

return;

}


////////////////////////////////////////////////////////////////////////


void ModeConfInfo::set_fcst_merge_thresh_by_index(int k)

{

if ( (k < 0) || (k >= fcst_merge_thresh_array.n_elements()) )  {

   mlog << Error << "\nModeConfInfo::set_fcst_merge_thresh_by_index(int) -> "
        << "range check error\n\n";

   exit ( 1 );

}

fcst_merge_thresh = fcst_merge_thresh_array[k];

return;

}


////////////////////////////////////////////////////////////////////////


void ModeConfInfo::set_obs_merge_thresh_by_index(int k)

{

if ( (k < 0) || (k >= obs_merge_thresh_array.n_elements()) )  {

   mlog << Error << "\nModeConfInfo::set_obs_merge_thresh_by_index(int) -> "
        << "range check error\n\n";

   exit ( 1 );

}

 obs_merge_thresh =  obs_merge_thresh_array[k];

return;

}


////////////////////////////////////////////////////////////////////////


int ModeConfInfo::n_runs() const

{

const int nr = n_conv_radii();
const int nt = n_conv_threshs();

return ( nr*nt );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for struct ModeNcOutInfo
   //


////////////////////////////////////////////////////////////////////////


ModeNcOutInfo::ModeNcOutInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void ModeNcOutInfo::clear()

{

set_all_true();

return;

}


////////////////////////////////////////////////////////////////////////


bool ModeNcOutInfo::all_false() const

{

bool status = do_latlon || do_raw || do_object_raw || do_object_id || do_cluster_id || do_polylines;

return ( !status );

}


////////////////////////////////////////////////////////////////////////


void ModeNcOutInfo::set_all_false()

{

do_latlon     = false;
do_raw        = false;
do_object_raw = false;
do_object_id  = false;
do_cluster_id = false;
do_polylines  = false;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeNcOutInfo::set_all_true()

{

do_latlon     = true;
do_raw        = true;
do_object_raw = true;
do_object_id  = true;
do_cluster_id = true;
do_polylines  = true;

return;

}


////////////////////////////////////////////////////////////////////////



