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
#include <cstdio>
#include <cmath>

#include "mtd_config_info.h"
#include "3d_att.h"
#include "configobjecttype_to_string.h"

#include "vx_data2d_factory.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////


static const int default_inten_perc_value = 99;

static const int default_min_volume = 2000;


////////////////////////////////////////////////////////////////////////
//
//  Code for class MtdConfigInfo
//
////////////////////////////////////////////////////////////////////////

MtdConfigInfo::MtdConfigInfo()

{

   init_from_scratch();

}

////////////////////////////////////////////////////////////////////////

MtdConfigInfo::~MtdConfigInfo()

{

   clear();
}

////////////////////////////////////////////////////////////////////////

void MtdConfigInfo::init_from_scratch()

{

   // Initialize pointers
   fcst_info = (VarInfo *) 0;
   obs_info  = (VarInfo *) 0;

   clear();

   return;

}

////////////////////////////////////////////////////////////////////////


void MtdConfigInfo::clear()

{

   // Initialize values

   model.clear();
   desc.clear();
   obtype.clear();

   do_2d_att_ascii = true;
   do_3d_att_ascii = true;

   mask_missing_flag = FieldType_None;

   fcst_conv_radius = bad_data_int;
   obs_conv_radius = bad_data_int;

   fcst_conv_time_beg = bad_data_int;
   obs_conv_time_beg = bad_data_int;

   fcst_conv_thresh.clear();
   obs_conv_thresh.clear();

   fcst_conv_time_end = bad_data_int;
   obs_conv_time_end = bad_data_int;

   fcst_vld_thresh = bad_data_double;
   obs_vld_thresh = bad_data_double;

   fcst_merge_thresh.clear();
   obs_merge_thresh.clear();

   fcst_merge_flag = MergeType_None;
   obs_merge_flag = MergeType_None;

   match_flag = MatchType_None;

   max_centroid_dist = bad_data_double;

   mask_grid_name.clear();
   mask_grid_flag = FieldType_None;

   mask_poly_name.clear();
   mask_poly_flag = FieldType_None;

   space_centroid_dist_wt = bad_data_double;
   time_centroid_delta_wt = bad_data_double;
   speed_delta_wt         = bad_data_double;
   direction_diff_wt      = bad_data_double;
   volume_ratio_wt        = bad_data_double;
   axis_angle_diff_wt     = bad_data_double;
   start_time_delta_wt    = bad_data_double;
   end_time_delta_wt      = bad_data_double;

   space_centroid_dist_if = (PiecewiseLinear *) 0;
   time_centroid_delta_if = (PiecewiseLinear *) 0;
   speed_delta_if         = (PiecewiseLinear *) 0;
   direction_diff_if      = (PiecewiseLinear *) 0;
   volume_ratio_if        = (PiecewiseLinear *) 0;
   axis_angle_diff_if     = (PiecewiseLinear *) 0;
   start_time_delta_if    = (PiecewiseLinear *) 0;
   end_time_delta_if      = (PiecewiseLinear *) 0;

   total_interest_thresh = bad_data_double;

   print_interest_thresh = bad_data_double;

   zero_border_size = bad_data_int;

   nc_info.set_all_true();

   ct_stats_flag   = false;

   inten_perc_value = default_inten_perc_value;

   min_volume = default_min_volume;

   output_prefix.clear();

   version.clear();

   delta_t_seconds = 0;

   // Deallocate memory
   if(fcst_info) { delete fcst_info; fcst_info = (VarInfo *) 0; }
   if(obs_info)  { delete obs_info;  obs_info  = (VarInfo *) 0; }

   return;

}


////////////////////////////////////////////////////////////////////////


void MtdConfigInfo::read_config(const char *default_file_name, const char *user_file_name)

{

   // Read the config file constants

conf.read(replace_path(config_const_filename).c_str());
conf.read(replace_path(config_map_data_filename).c_str());

   // Read the default config file

conf.read(default_file_name);

   // Read the user-specified config file

if ( user_file_name )  conf.read(user_file_name);

   //  check the fuzzy-engine weights
   //     calculation of total interest assumes these tests

   //
   //  Update: these tests are not really needed since the InterestCalculator
   //          class checks this
   //

/*
bool all_zero = true;

if ( is_eq(space_centroid_dist_wt, 0.0) )  space_centroid_dist_wt = 0.0;
else                                       all_zero = false;

if ( is_eq(time_centroid_delta_wt, 0.0) )  time_centroid_delta_wt = 0.0;
else                                       all_zero = false;

if ( is_eq(speed_delta_wt, 0.0) )          speed_delta_wt = 0.0;
else                                       all_zero = false;

if ( is_eq(direction_diff_wt, 0.0) )       direction_diff_wt = 0.0;
else                                       all_zero = false;

if ( is_eq(volume_ratio_wt, 0.0) )         volume_ratio_wt = 0.0;
else                                       all_zero = false;

if ( is_eq(axis_angle_diff_wt, 0.0) )      axis_angle_diff_wt = 0.0;
else                                       all_zero = false;

if ( is_eq(start_time_delta_wt, 0.0) )     start_time_delta_wt = 0.0;
else                                       all_zero = false;

if ( is_eq(end_time_delta_wt, 0.0) )       end_time_delta_wt = 0.0;
else                                       all_zero = false;


if ( all_zero )  {

   mlog << Error << "\n\n  MtdConfigInfo::read_config() -> all the fuzzy engine weights are zero!\n\n";

   exit ( 1 );

}
*/

   //
   //  done
   //

return;

}

////////////////////////////////////////////////////////////////////////


void MtdConfigInfo::process_config(GrdFileType ftype, GrdFileType otype)

{

   VarInfoFactory info_factory;
   Dictionary *fcst_dict = (Dictionary *) 0;
   Dictionary *obs_dict  = (Dictionary *) 0;
   Dictionary *dict      = (Dictionary *) 0;
   PlotInfo plot_info;
   bool status = false;
   double sum;

      // Dump the contents of the config file

   // if(mlog.verbosity_level() >= 5) conf.dump(cout);

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
//
//       if(mlog.verbosity_level() >= 5) {
//       mlog << Debug(5)
//            << "Parsed forecast field:\n";
//       fcst_info->dump(cout);
//       mlog << Debug(5)
//            << "Parsed observation field:\n";
//       obs_info->dump(cout);
//    }
//


      // No support for wind direction

   if(fcst_info->is_wind_direction() || obs_info->is_wind_direction()) {
      mlog << Error << "\nMtdConfigInfo::process_config() -> "
           << "the wind direction field may not be verified "
           << "using MODE.\n\n";
      exit(1);
   }

      // Conf: fcst.conv_radius and obs.conv_radius

   fcst_conv_radius = nint(fcst_dict->lookup_double(conf_key_conv_radius));
   obs_conv_radius  = nint(obs_dict->lookup_double(conf_key_conv_radius));

      // Check that fcst_conv_radius and obs_conv_radius are non-negative

   if(fcst_conv_radius < 0 || obs_conv_radius < 0) {
      mlog << Error << "\nMtdConfigInfo::process_config() -> "
           << "fcst_conv_radius (" << fcst_conv_radius
           << ") and obs_conv_radius (" << obs_conv_radius
           << ") must be non-negative\n\n";
      exit(1);
   }

      // Conf: fcst.conv_time_window

   dict = fcst_dict->lookup_dictionary(conf_key_conv_time_window);
   parse_conf_range_int(dict, fcst_conv_time_beg, fcst_conv_time_end);

      // Conf: obs.conv_time_window

   dict = obs_dict->lookup_dictionary(conf_key_conv_time_window);
   parse_conf_range_int(dict, obs_conv_time_beg, obs_conv_time_end);

      // Conf: fcst.conv_thresh and obs.conv_thresh

   fcst_conv_thresh = fcst_dict->lookup_thresh(conf_key_conv_thresh);
   obs_conv_thresh  = obs_dict->lookup_thresh(conf_key_conv_thresh);

      // Conf: fcst.vld_thresh and obs.vld_thresh

   // fcst_vld_thresh = fcst_dict->lookup_double(conf_key_vld_thresh);
   // obs_vld_thresh  = obs_dict->lookup_double(conf_key_vld_thresh);

      // Conf: fcst.merge_thresh and obs.merge_thresh

   // fcst_merge_thresh = fcst_dict->lookup_thresh(conf_key_merge_thresh);
   // obs_merge_thresh  = obs_dict->lookup_thresh(conf_key_merge_thresh);

      // Conf: fcst.merge_flag and obs.merge_flag

   // fcst_merge_flag = int_to_mergetype(fcst_dict->lookup_int(conf_key_merge_flag));
   // obs_merge_flag  = int_to_mergetype(obs_dict->lookup_int(conf_key_merge_flag));

      // Conf: mask_missing_flag

   // mask_missing_flag = int_to_fieldtype(conf.lookup_int(conf_key_mask_missing_flag));

      // Conf: match_flag

   // match_flag = int_to_matchtype(conf.lookup_int(conf_key_match_flag));

      // Check that match_flag is set between 0 and 3
/*
   if(match_flag == MatchType_None &&
      (fcst_merge_flag != MergeType_None || obs_merge_flag  != MergeType_None) ) {
      mlog << Warning << "\nMtdConfigInfo::process_config() -> "
           << "When matching is disabled (match_flag = "
           << matchtype_to_string(match_flag)
           << ") but merging is requested (fcst_merge_flag = "
           << mergetype_to_string(fcst_merge_flag)
           << ", obs_merge_flag = "
           << mergetype_to_string(obs_merge_flag)
           << ") any merging information will be discarded.\n\n";
   }
*/
      // Conf: max_centroid_dist

   // max_centroid_dist = conf.lookup_double(conf_key_max_centroid_dist);

      // Check that max_centroid_dist is > 0
/*
   if(max_centroid_dist <= 0) {
      mlog << Warning << "\nMtdConfigInfo::process_config() -> "
           << "max_centroid_dist (" << max_centroid_dist
           << ") should be set > 0\n\n";
   }

*/
      // Conf: mask.grid

/*
   mask_grid_name = conf.lookup_string(conf_key_mask_grid);
   mask_grid_flag = int_to_fieldtype(conf.lookup_int(conf_key_mask_grid_flag));

      // Conf: mask.poly

   mask_poly_name = conf.lookup_string(conf_key_mask_poly);
   mask_poly_flag = int_to_fieldtype(conf.lookup_int(conf_key_mask_poly_flag));
*/

      // Conf: weight

   dict = conf.lookup_dictionary(conf_key_weight);

      // Parse the weights

   space_centroid_dist_wt = dict->lookup_double(conf_key_space_centroid_dist);
   time_centroid_delta_wt = dict->lookup_double(conf_key_time_centroid_delta);
   speed_delta_wt         = dict->lookup_double(conf_key_speed_delta);
   direction_diff_wt      = dict->lookup_double(conf_key_direction_diff);
   volume_ratio_wt        = dict->lookup_double(conf_key_volume_ratio);
   axis_angle_diff_wt     = dict->lookup_double(conf_key_axis_angle_diff);
   start_time_delta_wt    = dict->lookup_double(conf_key_start_time_delta);
   end_time_delta_wt      = dict->lookup_double(conf_key_end_time_delta);

      // Check that the fuzzy engine weights are non-negative

   status =    (space_centroid_dist_wt >= 0.0)
            && (time_centroid_delta_wt >= 0.0)
            && (speed_delta_wt         >= 0.0)
            && (direction_diff_wt      >= 0.0)
            && (volume_ratio_wt        >= 0.0)
            && (axis_angle_diff_wt     >= 0.0)
            && (start_time_delta_wt    >= 0.0)
            && (end_time_delta_wt      >= 0.0);

   if ( ! status ) {

      mlog << Error << "\nMtdConfigInfo::process_config() -> "
           << "All of the fuzzy engine weights must be >= 0.\n\n";

      exit(1);

   }

      // Check that the sum of the weights is non-zero for matching

   sum =   space_centroid_dist_wt
         + time_centroid_delta_wt
         + speed_delta_wt
         + direction_diff_wt
         + volume_ratio_wt
         + axis_angle_diff_wt
         + start_time_delta_wt
         + end_time_delta_wt;

   if(match_flag != MatchType_None &&
      is_eq( sum, 0.0)) {
      mlog << Error << "\nMtdConfigInfo::process_config() -> "
           << "When matching is requested, the sum of the fuzzy engine "
           << "weights cannot be 0\n\n";
      exit(1);
   }

      // Conf: interest_function

   dict = conf.lookup_dictionary(conf_key_interest_function);

      // Parse the interest functions

   space_centroid_dist_if = dict->lookup_pwl(conf_key_space_centroid_dist);
   time_centroid_delta_if = dict->lookup_pwl(conf_key_time_centroid_delta);
   speed_delta_if         = dict->lookup_pwl(conf_key_speed_delta);
   direction_diff_if      = dict->lookup_pwl(conf_key_direction_diff);
   volume_ratio_if        = dict->lookup_pwl(conf_key_volume_ratio);
   axis_angle_diff_if     = dict->lookup_pwl(conf_key_axis_angle_diff);
   start_time_delta_if    = dict->lookup_pwl(conf_key_start_time_delta);
   end_time_delta_if      = dict->lookup_pwl(conf_key_end_time_delta);

      // Conf: total_interest_thresh

   total_interest_thresh = conf.lookup_double(conf_key_total_interest_thresh);

      // Check that total_interest_thresh is between 0 and 1.

   if(total_interest_thresh < 0 || total_interest_thresh > 1) {
      mlog << Error << "\nMtdConfigInfo::process_config() -> "
           << "total_interest_thresh (" << total_interest_thresh
           << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

      // Conf: print_interest_thresh

   // print_interest_thresh = conf.lookup_double(conf_key_print_interest_thresh);

      // Check that print_interest_thresh is between 0 and 1.
/*
   if(print_interest_thresh < 0 || print_interest_thresh > 1) {
      mlog << Error << "\nMtdConfigInfo::process_config() -> "
           << "print_interest_thresh (" << print_interest_thresh
           << ") must be set between 0 and 1.\n\n";
      exit(1);
   }
*/

      // Conf: nc_pairs_flag

   parse_nc_info();

      //  ascii attribute output flags

   parse_txt_info();

      // Conf: ct_stats_flag

   // ct_stats_flag = conf.lookup_bool(conf_key_ct_stats_flag);

      // Conf: output_prefix

   output_prefix = conf.lookup_string(conf_key_output_prefix);

      // Conf: inten_perc_value

   inten_perc_value = conf.lookup_int(conf_key_inten_perc_value);

      // Check that intensity_percentile >= 0 and <= 100

   if(inten_perc_value < 0 || inten_perc_value > 100) {
      mlog << Error << "\nMtdConfigInfo::process_config() -> "
           << "inten_perc_value (" << inten_perc_value
           << ") must be set between 0 and 100.\n\n";
      exit(1);
   }

      // Conf: min_volume

   min_volume = conf.lookup_int(conf_key_min_volume);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdConfigInfo::parse_nc_info()

{

const DictionaryEntry * e = (const DictionaryEntry *) 0;

e = conf.lookup(conf_key_nc_output);

if ( !e )  {

   mlog << Error
        << "\n\n  MtdConfigInfo::parse_nc_info() -> lookup failed for key \""
        << conf_key_nc_output << "\"\n\n";

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

   mlog << Error
        << "\n\n  MtdConfigInfo::parse_nc_info() -> bad type ("
        << configobjecttype_to_string(type)
        << ") for key \""
        << conf_key_nc_pairs_flag << "\"\n\n";

   exit ( 1 );

}

   //
   //  parse the various entries
   //

Dictionary * d = e->dict_value();

nc_info.do_latlon     = d->lookup_bool(conf_key_latlon_flag);
nc_info.do_raw        = d->lookup_bool(conf_key_raw_flag);
// nc_info.do_object_raw = d->lookup_bool(conf_key_object_raw_flag);
nc_info.do_object_id  = d->lookup_bool(conf_key_object_id_flag);
nc_info.do_cluster_id = d->lookup_bool(conf_key_cluster_id_flag);
// nc_info.do_polylines  = d->lookup_bool(conf_key_do_polylines_flag);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdConfigInfo::parse_txt_info()

{

const DictionaryEntry * e = (const DictionaryEntry *) 0;

const char * key = conf_key_txt_output;


e = conf.lookup(key);

if ( !e )  {

   mlog << Error
        << "\n\n  MtdConfigInfo::parse_txt_info() -> lookup failed for key \""
        << key << "\"\n\n";

   exit ( 1 );

}

   //
   //  it should be a dictionary
   //

const ConfigObjectType type = e->type();

if ( type != DictionaryType )  {

   mlog << Error
        << "\n\n  MtdConfigInfo::parse_txt_info() -> bad type ("
        << configobjecttype_to_string(type)
        << ") for key \""
        << key << "\"\n\n";

   exit ( 1 );

}

   //
   //  parse the various entries
   //

Dictionary * d = e->dict_value();

do_2d_att_ascii = d->lookup_bool(conf_key_do_2d_att_flag);
do_3d_att_ascii = d->lookup_bool(conf_key_do_3d_att_flag);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdConfigInfo::write_header_cols(AsciiTable & table, const int row) const

{

int c = 0;
ConcatString s;


   //  version

table.set_entry(row, c++, (string)met_version);

   //  model

s = check_hdr_str(model);

table.set_entry(row, c++, s.text());

   //  description

s = check_hdr_str(desc);

table.set_entry(row, c++, s.text());

   //  fcst lead

table.set_entry(row, c++, sec_to_hhmmss(fcst_info->lead()));

   //  fcst valid

table.set_entry(row, c++, unix_to_yyyymmdd_hhmmss(fcst_info->valid()));

   //  obs lead

table.set_entry(row, c++, sec_to_hhmmss(obs_info->lead()));

   //  obs valid

table.set_entry(row, c++, unix_to_yyyymmdd_hhmmss(obs_info->valid()));

   //  time delta

table.set_entry(row, c++, sec_to_hhmmss(delta_t_seconds));

   //  fcst time convolution begin

table.set_entry(row, c++, fcst_conv_time_beg);

   //  fcst time convolution end

table.set_entry(row, c++, fcst_conv_time_end);

   //  fcst radius

table.set_entry(row, c++, fcst_conv_radius);

   //  fcst threshold

s = fcst_conv_thresh.get_str();

table.set_entry(row, c++, s.text());

   //  obs time convolution begin

table.set_entry(row, c++, obs_conv_time_beg);

   //  obs time convolution end

table.set_entry(row, c++, obs_conv_time_end);

   //  obs radius

table.set_entry(row, c++, obs_conv_radius);

   //  obs threshold

s = obs_conv_thresh.get_str();

table.set_entry(row, c++, s.text());

   //  fcst var

s = check_hdr_str(fcst_info->name_attr());

table.set_entry(row, c++, s.text());

   //  fcst units

s = check_hdr_str(fcst_info->units_attr(), true);

table.set_entry(row, c++, s.text());

   //  fcst level

s = check_hdr_str(fcst_info->level_attr(), true);

table.set_entry(row, c++, s.text());

   //  obs var

s = check_hdr_str(obs_info->name_attr());

table.set_entry(row, c++, s.text());

   //  obs units

s = check_hdr_str(obs_info->units_attr(), true);

table.set_entry(row, c++, s.text());

   //  obs level

s = check_hdr_str(obs_info->level_attr(), true);

table.set_entry(row, c++, s.text());

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for struct MtdNcOutInfo
   //


////////////////////////////////////////////////////////////////////////


MtdNcOutInfo::MtdNcOutInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void MtdNcOutInfo::clear()

{

set_all_true();

return;

}


////////////////////////////////////////////////////////////////////////


bool MtdNcOutInfo::all_false() const

{

// bool status = do_latlon || do_raw || do_object_raw || do_object_id || do_cluster_id || do_polylines;
   bool status = do_latlon || do_raw ||                  do_object_id || do_cluster_id;

return ( !status );

}


////////////////////////////////////////////////////////////////////////


void MtdNcOutInfo::set_all_false()

{

do_latlon     = false;
do_raw        = false;
// do_object_raw = false;
do_object_id  = false;
do_cluster_id = false;
// do_polylines  = false;

return;

}


////////////////////////////////////////////////////////////////////////


void MtdNcOutInfo::set_all_true()

{

do_latlon     = true;
do_raw        = true;
// do_object_raw = true;
do_object_id  = true;
do_cluster_id = true;
// do_polylines  = true;

return;

}


////////////////////////////////////////////////////////////////////////


void MtdNcOutInfo::dump(ostream & out) const

{

out << "do_latlon     = " << bool_to_string(do_latlon)     << "\n";
out << "do_raw        = " << bool_to_string(do_raw)        << "\n";
out << "do_object_id  = " << bool_to_string(do_object_id)  << "\n";
out << "do_cluster_id = " << bool_to_string(do_cluster_id) << "\n";

return;

}


////////////////////////////////////////////////////////////////////////
