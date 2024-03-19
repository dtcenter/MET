// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <cmath>
#include <algorithm>

#include "mode_conf_info.h"
#include "configobjecttype_to_string.h"

#include "vx_data2d_factory.h"
#include "vx_log.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

static const char default_multivar_name[] = "Super";

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

ModeConfInfo & ModeConfInfo::operator=(const ModeConfInfo &s)
{
   if(this == &s) return *this;

   clear();
   assign(s);

   return *this;
}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::init_from_scratch()

{

   N_fields_f = 0;
   N_fields_o = 0;

   Field_Index_f = 0;
   Field_Index_o = 0;

   fcst_array = 0;
    obs_array = 0;

   fcst_array = 0;
    obs_array = 0;

   clear();

   return;

}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::assign( const ModeConfInfo &m)
{
   Field_Index_f = m.Field_Index_f;
   Field_Index_o = m.Field_Index_o;
   N_fields_f = m.N_fields_f;
   N_fields_o = m.N_fields_o;
   fcst_multivar_logic = m.fcst_multivar_logic;
   obs_multivar_logic = m.obs_multivar_logic;
   fcst_multivar_compare_index = m.fcst_multivar_compare_index;
   obs_multivar_compare_index = m.obs_multivar_compare_index;
   fcst_multivar_name = m.fcst_multivar_name;
   fcst_multivar_level = m.fcst_multivar_level;
   obs_multivar_name = m.obs_multivar_name;
   obs_multivar_level = m.obs_multivar_level;
   data_type = m.data_type;        
   conf = m.conf;
   centroid_dist_wt = m.centroid_dist_wt;
   boundary_dist_wt = m.boundary_dist_wt;
   convex_hull_dist_wt = m.convex_hull_dist_wt;
   angle_diff_wt = m.angle_diff_wt;
   aspect_diff_wt = m.aspect_diff_wt;
   area_ratio_wt = m.area_ratio_wt;
   int_area_ratio_wt = m.int_area_ratio_wt;
   curvature_ratio_wt = m.curvature_ratio_wt;
   complexity_ratio_wt = m.complexity_ratio_wt;
   inten_perc_ratio_wt = m.inten_perc_ratio_wt;

   fcst_array = 0;
   Fcst = 0;
   if (N_fields_f > 0) {
      fcst_array = new Mode_Field_Info[N_fields_f];
      for (int i=0; i<N_fields_f; ++i)
      {
         fcst_array[i].clone(m.fcst_array[i]);
      }
      Fcst = fcst_array + Field_Index_f;
   }
   obs_array = 0;
   Obs = 0;
   if (N_fields_o > 0) {
      obs_array = new Mode_Field_Info[N_fields_o];
      for (int i=0; i<N_fields_o; ++i)
      {
         obs_array[i].clone(m.obs_array[i]);
      }
      Obs = obs_array + Field_Index_o;
   }

   // need to be recomputed, maybe, so do so just in case
   Dictionary * dict      = (Dictionary *) nullptr;
   dict = conf.lookup_dictionary(conf_key_interest_function);
   centroid_dist_if    = parse_interest_function(dict, conf_key_centroid_dist);
   boundary_dist_if    = parse_interest_function(dict, conf_key_boundary_dist);
   convex_hull_dist_if = parse_interest_function(dict, conf_key_convex_hull_dist);
   angle_diff_if       = parse_interest_function(dict, conf_key_angle_diff);
   aspect_diff_if      = parse_interest_function(dict, conf_key_aspect_diff);
   area_ratio_if       = parse_interest_function(dict, conf_key_area_ratio);
   int_area_ratio_if   = parse_interest_function(dict, conf_key_int_area_ratio);
   curvature_ratio_if  = parse_interest_function(dict, conf_key_curvature_ratio);
   complexity_ratio_if = parse_interest_function(dict, conf_key_complexity_ratio);
   inten_perc_ratio_if = parse_interest_function(dict, conf_key_inten_perc_ratio);



   total_interest_thresh = m.total_interest_thresh;
   print_interest_thresh = m.print_interest_thresh;
   max_centroid_dist = m.max_centroid_dist;
   quilt = m.quilt;
   plot_valid_flag = m.plot_valid_flag;
   plot_gcarc_flag = m.plot_gcarc_flag;
   ps_plot_flag = m.ps_plot_flag;
   ct_stats_flag = m.ct_stats_flag;
   mask_missing_flag = m.mask_missing_flag;
   match_flag = m.match_flag;
   mask_grid_flag = m.mask_grid_flag;
   mask_poly_flag = m.mask_poly_flag;
   inten_perc_value = m.inten_perc_value;
   grid_res = m.grid_res;
   model = m.model;
   desc = m.desc;
   obtype = m.obtype;
   mask_grid_name = m.mask_grid_name;
   mask_poly_name = m.mask_poly_name;
   met_data_dir = m.met_data_dir;
   object_pi = m.object_pi;
   nc_info = m.nc_info;
   shift_right = m.shift_right;
   output_prefix = m.output_prefix;
   version = m.version;
}

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::clear()

{

   // Initialize values

   model.clear();
   desc.clear();
   obtype.clear();

   mask_missing_flag = FieldType::None;

   grid_res = bad_data_double;

   Field_Index_f = 0;
   Field_Index_o = 0;

   fcst_multivar_logic.clear();
   obs_multivar_logic.clear();

   match_flag = MatchType::None;

   max_centroid_dist = bad_data_double;

   mask_grid_name.clear();
   mask_grid_flag = FieldType::None;

   mask_poly_name.clear();
   mask_poly_flag = FieldType::None;

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

   centroid_dist_if    = (PiecewiseLinear *) nullptr;
   boundary_dist_if    = (PiecewiseLinear *) nullptr;
   convex_hull_dist_if = (PiecewiseLinear *) nullptr;
   angle_diff_if       = (PiecewiseLinear *) nullptr;
   aspect_diff_if      = (PiecewiseLinear *) nullptr;
   area_ratio_if       = (PiecewiseLinear *) nullptr;
   int_area_ratio_if   = (PiecewiseLinear *) nullptr;
   curvature_ratio_if  = (PiecewiseLinear *) nullptr;
   complexity_ratio_if = (PiecewiseLinear *) nullptr;
   inten_perc_ratio_if = (PiecewiseLinear *) nullptr;

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
   if ( fcst_array )  { delete [] fcst_array;  fcst_array = 0; }
   if (  obs_array )  { delete []  obs_array;   obs_array = 0; }

   Fcst = 0;
    Obs = 0;

   // so traditional mode will have the right value
   data_type = ModeDataType_Traditional; 

   N_fields_f = 0;
   N_fields_o = 0;

   return;

}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::read_config(const char * default_file_name, const char * user_file_name)

{

   // Read the config file constants
   conf.read(replace_path(config_const_filename).c_str());
   conf.read(replace_path(config_map_data_filename).c_str());

   // Read the default config file
   conf.read(default_file_name);

   // Read the user-specified config file
   conf.read(user_file_name);

   return;

}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::process_config_traditional(GrdFileType ftype, GrdFileType otype)
{
   process_config_except_fields();
   process_config_field (ftype, otype, ModeDataType_Traditional, 0);
}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::process_config_except_fields()
{
   Dictionary * dict      = (Dictionary *) nullptr;

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

      // Conf: mask_missing_flag

   mask_missing_flag = int_to_fieldtype(conf.lookup_int(conf_key_mask_missing_flag));

      // Conf: match_flag

   match_flag = int_to_matchtype(conf.lookup_int(conf_key_match_flag));

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

   if(match_flag != MatchType::None &&
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

   centroid_dist_if    = parse_interest_function(dict, conf_key_centroid_dist);
   boundary_dist_if    = parse_interest_function(dict, conf_key_boundary_dist);
   convex_hull_dist_if = parse_interest_function(dict, conf_key_convex_hull_dist);
   angle_diff_if       = parse_interest_function(dict, conf_key_angle_diff);
   aspect_diff_if      = parse_interest_function(dict, conf_key_aspect_diff);
   area_ratio_if       = parse_interest_function(dict, conf_key_area_ratio);
   int_area_ratio_if   = parse_interest_function(dict, conf_key_int_area_ratio);
   curvature_ratio_if  = parse_interest_function(dict, conf_key_curvature_ratio);
   complexity_ratio_if = parse_interest_function(dict, conf_key_complexity_ratio);
   inten_perc_ratio_if = parse_interest_function(dict, conf_key_inten_perc_ratio);

      // Conf: total_interest_thresh

   total_interest_thresh = conf.lookup_double(conf_key_total_interest_thresh);

      // Check that total_interest_thresh is between 0 and 1.

   if(total_interest_thresh < 0 || total_interest_thresh > 1) {
      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "\"total_interest_thresh\" (" << total_interest_thresh
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

   get_multivar_programs();

   nc_info.set_compress_level(conf.nc_compression());

}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::process_config_field(GrdFileType ftype, GrdFileType otype,
                                        ModeDataType dt, int field_index)

{
   set_data_type(dt);

   if (data_type == ModeDataType_MvMode_Fcst) {

      process_config_fcst(ftype, field_index);

   } else if (data_type == ModeDataType_MvMode_Obs) {

      process_config_obs(otype, field_index);

   } else {

      process_config_both(ftype, otype, field_index);
   }
}

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::process_config_both(GrdFileType ftype, GrdFileType otype,
                                       int field_index)
{
   int j, k, n;


   Dictionary * fcst_dict = (Dictionary *) nullptr;
   Dictionary * obs_dict  = (Dictionary *) nullptr;

      // Conf: fcst and obs

   fcst_dict = conf.lookup_dictionary(conf_key_fcst);
   obs_dict  = conf.lookup_dictionary(conf_key_obs);

      // Conf: shift_right

   shift_right = fcst_dict->lookup_int(conf_key_shift_right);


   if (field_index == 0) {
      read_fields_0 (fcst_array, fcst_dict, ftype, 'F');
      read_fields_1 (fcst_array, fcst_dict, ftype, 'F', 0);
      read_fields_0 (obs_array, obs_dict, otype, 'O');
      read_fields_1 (obs_array, obs_dict, otype, 'O', 0);
      fcst_array->raw_pi = parse_conf_plot_info(conf.lookup_dictionary(conf_key_fcst_raw_plot));
      obs_array->raw_pi = parse_conf_plot_info(conf.lookup_dictionary(conf_key_obs_raw_plot));

   } else {
      read_fields_1 (fcst_array, fcst_dict, ftype, 'F', field_index);
      read_fields_1 (obs_array, obs_dict, otype, 'O', field_index);
   }      

   Fcst = fcst_array + field_index;
   Obs =  obs_array + field_index;

      // Dump the contents of the VarInfo objects

   if(mlog.verbosity_level() >= 5)  {

      mlog << Debug(5)
           << "Parsed forecast field:\n";
      fcst_array[field_index].var_info->dump(cout);
      mlog << Debug(5)
           << "Parsed observation field:\n";
      obs_array[field_index].var_info->dump(cout);
   }

   evaluate_fcst_settings(field_index);
   evaluate_obs_settings(field_index);

   if (data_type == ModeDataType_Traditional) {

      if ( fcst_array[field_index].conv_radius_array.n_elements() !=
           obs_array[field_index].conv_radius_array.n_elements() )  {
         mlog << Error << "\nModeConfInfo::process_config_both() -> "
              << "fcst and obs convolution radius arrays need to be the same size for traditional mode\n\n";
         exit ( 1 );
      }
      if ( fcst_array[field_index].conv_thresh_array.n_elements() !=
           obs_array[field_index].conv_thresh_array.n_elements() )  {

         mlog << Error << "\nModeConfInfo::process_config_both() -> "
              << "fcst and obs convolution threshold arrays need to be the same size\n\n";
         exit ( 1 );
      }
   }

   return;

}

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::process_config_fcst(GrdFileType ftype, int field_index)
{
   int j, k, n;

   Dictionary * fcst_dict = (Dictionary *) nullptr;

   fcst_dict = conf.lookup_dictionary(conf_key_fcst);
   shift_right = fcst_dict->lookup_int(conf_key_shift_right);

   if (field_index == 0) {
      read_fields_0 (fcst_array, fcst_dict, ftype, 'F');
      read_fields_1 (fcst_array, fcst_dict, ftype, 'F', 0);
      fcst_array->raw_pi = parse_conf_plot_info(conf.lookup_dictionary(conf_key_fcst_raw_plot));
   } else {
      read_fields_1 (fcst_array, fcst_dict, ftype, 'F', field_index);
   }
   
   Fcst = fcst_array + field_index;


      // Dump the contents of the VarInfo object

   if(mlog.verbosity_level() >= 5)  {
      mlog << Debug(5)
           << "Parsed forecast field:\n";
      fcst_array[field_index].var_info->dump(cout);
   }

   evaluate_fcst_settings(field_index);

   return;

}

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::process_config_obs(GrdFileType otype, int field_index)
{
   int j, k, n;

   Dictionary * obs_dict  = (Dictionary *) nullptr;

   obs_dict  = conf.lookup_dictionary(conf_key_obs);
   shift_right = obs_dict->lookup_int(conf_key_shift_right);

   if (field_index == 0) {
      read_fields_0 (obs_array, obs_dict, otype, 'O');
      read_fields_1 (obs_array, obs_dict, otype, 'O', 0);
      obs_array->raw_pi = parse_conf_plot_info(conf.lookup_dictionary(conf_key_obs_raw_plot));
   } else {
      read_fields_1 (obs_array, obs_dict, otype, 'O', field_index);
   }

   Obs =  obs_array + field_index;

      // Dump the contents of the VarInfo object

   if(mlog.verbosity_level() >= 5)  {
      mlog << Debug(5)
           << "Parsed observation field:\n";
      obs_array[field_index].var_info->dump(cout);
   }

   evaluate_obs_settings(field_index);

   return;

}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::config_set_all_percentile_thresholds(const std::vector<ModeInputData> &fdata,
                                                        const std::vector<ModeInputData> &odata)
{

   // for each forecast input, it's either not a percentile threshold,
   // is a simple (single input) percentile, is a frequency bias percentile threshold, or is a
   // climatology percentile (we don't have climatology).
   //
   // As a complication we have both conv_thresh and merge_thresh, so what if they are different?
   // right now I simply go with frequency bias as highest priority

   // indices of forecast and obs inputs that have frequency bias percentile thresholding
   vector<int> fcst_freq, obs_freq;

   // indices (common to forecast and obs) that require both inputs (fcst and obs)
   // which is either frequency bias, or sample obs with a forecast input, or sample fcst
   // with an obs input
   vector<int> indices_with_both;

   for (int j=0; j<N_fields_f; ++j)
   {
      PercThreshType ptype = perctype(fcst_array[j]);
      switch (ptype)
      {
      case perc_thresh_sample_fcst:
         // do this now... this might be done twice depending on obs settings that need both?
         data_type = ModeDataType_MvMode_Fcst;
         set_field_index(j);
         set_perc_thresh(fdata[j]._dataPlane);
         break;
      case perc_thresh_sample_obs:
         // currently expect matching index in obs and fcst, so check to make sure not both
         if (j >= N_fields_o) {
            mlog << Error << "\nModeConfInfo::config_set_all_percentile_thresholds\n"
                 << " SOP Thresholding on fcst index " << j+1
                 << " out of range of obs " << N_fields_o + 1 << "\n\n";
            exit ( 1 );
         }
         // deal with this later
         indices_with_both.push_back(j);
         break;
      case perc_thresh_freq_bias:
         // currently expect matching index in obs and fcst, so check to make sure not both
         if (j >= N_fields_o) {
            mlog << Error << "\nModeConfInfo::config_set_all_percentile_thresholds\n"
                 << " FBIAS Thresholding on fcst index " << j+1
                 << " out of range of obs " << N_fields_o + 1 << "\n\n";
            exit ( 1 );
         }
         fcst_freq.push_back(j);
         indices_with_both.push_back(j);
         break;
      default:
         break;
      }
   }
   for (int j=0; j<N_fields_o; ++j)
   {
      PercThreshType ptype = perctype(obs_array[j]);
      switch (ptype)
      {
      case perc_thresh_sample_obs:
         // do this now... this might be done twice depending on obs settings that need both?
         data_type = ModeDataType_MvMode_Obs;
         set_field_index(j);
         set_perc_thresh(odata[j]._dataPlane);
         break;
      case perc_thresh_sample_fcst:
         // currently expect matching index in obs and fcst, so check to make sure not both
         if (j >= N_fields_f) {
            mlog << Error << "\nModeConfInfo::config_set_all_percentile_thresholds\n"
                 << " SFP Thresholding on obs index " << j+1
                 << " out of range of fcst " << N_fields_f + 1 << "\n\n";
            exit ( 1 );
         }
         // deal with this later
         if (find(indices_with_both.begin(), indices_with_both.end(), j) == indices_with_both.end()) {
            indices_with_both.push_back(j);
         }
         break;
      case perc_thresh_freq_bias:
         // currently expect matching index in obs and fcst, so check to make sure not both
         if (j >= N_fields_f) {
            mlog << Error << "\nModeConfInfo::config_set_all_percentile_thresholds\n"
                 << " FBIAS Thresholding on obs index " << j+1
                 << " out of range of fcst " << N_fields_f + 1 << "\n\n";
            exit ( 1 );
         }
         // deal with this later
         obs_freq.push_back(j);
         if (find(indices_with_both.begin(), indices_with_both.end(), j) == indices_with_both.end()) {
            indices_with_both.push_back(j);
         }
         break;
      default:
         break;
      }
   }

   // make sure no overlap in fcst/obs frequencies, as that's a no no
   for (size_t i=0; i<fcst_freq.size(); ++i)
   {
      int findex = fcst_freq[i];
      if (find(obs_freq.begin(), obs_freq.end(), findex) != obs_freq.end())
      {
         mlog << Error << "\nModeConfInfo::config_set_all_percentile_thresholds\n"
              << " FBIAS Thresholding in both fcst and obs at index " << findex+1
              << " not allowed\n\n";
         exit(1);
      }
   }
   for (size_t i=0; i<obs_freq.size(); ++i)
   {
      int findex = obs_freq[i];
      if (find(fcst_freq.begin(), fcst_freq.end(), findex) != fcst_freq.end())
      {
         mlog << Error << "\nModeConfInfo::config_set_all_percentile_thresholds\n"
              << " FBIAS Thresholding in both fcst and obs at index " << findex+1
              << " not allowed\n\n";
         exit(1);
      }
   }

   // now do all the indices with both, which come from FBIAS and SOP on forecasts
   // and SFP on obs  
   for (size_t i=0; i<indices_with_both.size(); ++i) {
      int index = indices_with_both[i];
      data_type = ModeDataType_MvMode_Both;
      set_field_index(index, index);
      set_perc_thresh(fdata[index]._dataPlane, odata[index]._dataPlane);
   }
}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::evaluate_fcst_settings(int j)
{
   // No support for wind direction
   if(fcst_array[j].var_info->is_wind_direction()) {
      mlog << Error << "\nModeConfInfo::evaluate_fcst_settings(" << j << ") -> "
           << "the wind direction field may not be verified "
           << "using MODE.\n\n";
      exit(1);
   }

   // Check that fcst_conv_radius values are non-negative
   int n = fcst_array[j].conv_radius_array.n_elements();

   for (int k=0; k<n; ++k)  {

      if(fcst_array[j].conv_radius_array[k] < 0) {

         mlog << Error << "\nModeConfInfo::evaluate_fcst_settings(" << j << ") -> "
              << "fcst_conv_radius (" << fcst_array[j].conv_radius_array[k]
              << ") must be non-negative\n\n";

         exit(1);

      }

   }   //  for k

   if ( fcst_array[j].conv_radius_array.n_elements() == 1 )  fcst_array[j].conv_radius = fcst_array[j].conv_radius_array[0];
   if ( fcst_array[j].conv_thresh_array.n_elements() == 1 )  fcst_array[j].conv_thresh = fcst_array[j].conv_thresh_array[0];
   if ( fcst_array[j].need_merge_thresh() && (fcst_array[j].merge_thresh_array.n_elements() != fcst_array[j].conv_thresh_array.n_elements()) )  {

      mlog << Error << "\nModeConfInfo::evaluate_fcst_settings(" << j << ") -> "
           << "fcst conv thresh and fcst merge thresh arrays need to be the same size\n\n";

      exit ( 1 );
   }
      
   if ( fcst_array[j].merge_thresh_array.n_elements() == 1 )  fcst_array[j].merge_thresh = fcst_array[j].merge_thresh_array[0];

   if(match_flag == MatchType::None && fcst_array[j].merge_flag != MergeType::None) { 
      mlog << Warning << "\nModeConfInfo::evaluate_fcst_settings(" << j << ") -> "
           << "When matching is disabled (match_flag = "
           << matchtype_to_string(match_flag)
           << ") but merging is requested (fcst_merge_flag = "
           << mergetype_to_string(fcst_array[j].merge_flag)
           << ") any merging information will be discarded.\n\n";
   }
}
   

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::evaluate_obs_settings(int j)
{      
   // No support for wind direction
   if(obs_array[j].var_info->is_wind_direction())  {
      mlog << Error << "\nModeConfInfo::evaluate_obs_settings(" << j << ") -> "
           << "the wind direction field may not be verified "
           << "using MODE.\n\n";
      exit(1);
   }

   // Check that obs_conv_radius values are non-negative
   int n = obs_array[j].conv_radius_array.n_elements(); 
   for (int k=0; k<n; ++k)  {

      if(obs_array[j].conv_radius_array[k] < 0) {

         mlog << Error << "\nModeConfInfo::evaluate_obs_settings(" << j << ") -> "
              << "obs_conv_radius (" << obs_array[j].conv_radius_array[k]
              << ") must be non-negative\n\n";

         exit(1);
      }
   }   //  for k

   if (  obs_array[j].conv_radius_array.n_elements() == 1 )   obs_array[j].conv_radius =  obs_array[j].conv_radius_array[0];
   if (  obs_array[j].conv_thresh_array.n_elements() == 1 )   obs_array[j].conv_thresh =  obs_array[j].conv_thresh_array[0];

   if ( obs_array[j].need_merge_thresh() && (obs_array[j].merge_thresh_array.n_elements() != obs_array[j].conv_thresh_array.n_elements()) )  {

      mlog << Error << "\nModeConfInfo::evaluate_obs_settings(" << j << ") -> "
           << "obs conv thresh and obs merge thresh arrays need to be the same size\n\n";

      exit ( 1 );

   }

   if (  obs_array[j].merge_thresh_array.n_elements() == 1 )   obs_array[j].merge_thresh =  obs_array[j].merge_thresh_array[0];

   // Check that match_flag is set between 0 and 3
   if(match_flag == MatchType::None && obs_array[j].merge_flag  != MergeType::None) {
      mlog << Warning << "\nModeConfInfo::evaluate_obs_settings(" << j << ") -> "
           << "When matching is disabled (match_flag = "
           << matchtype_to_string(match_flag)
           << ") but merging is requested (obs_merge_flag = "
           << mergetype_to_string(obs_array[j].merge_flag)
           << ") any merging information will be discarded.\n\n";
   }
}

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::read_fields_0 (Mode_Field_Info * & info_array, Dictionary * dict, GrdFileType type, char _fo)

{

const DictionaryEntry * ee = dict->lookup(conf_key_field);

if ( !ee )  {

   mlog << "\nModeConfInfo::read_fields () -> \"field\" entry not found in dictionary!\n\n";

   exit ( 1 );

}

const Dictionary * field = ee->dict();

const int N = ( (field->is_array()) ? (field->n_entries()) : 1 );

info_array = new Mode_Field_Info [N];

if ( field->is_array() ) {

   int j, k;
   const DictionaryEntry * e = 0;
   const Dictionary & D = *field;

   if (data_type == ModeDataType_Traditional) {
      // traditional mode, extra test
      if ( (N_fields_f > 0) && (N != N_fields_f) )  {
         mlog << Error
              << "\nModeConfInfo::read_fields() -> fcst and obs dictionaries have different number of entries in traditional mode"
              << ", not allowed\n\n";
         exit ( 1 );

      }
   }
   
   if (_fo == 'F')  {
      N_fields_f = N;
   } else {
      N_fields_o = N;
   }
   for (j=0; j<N; ++j)  {

      e = D[j];

      if ( (e->type() != DictionaryType) && (e->type() != ArrayType) )  {

         mlog << Error
              << "\nModeConfInfo::read_fields() -> field entry # " << (j + 1) << " is not a dictionary!\n\n";

         exit ( 1 );

      }
   }

   return;

}   //  if is array


   //
   //  nope, traditional mode
   //

N_fields_f = N;
N_fields_o = N;

   //
   //  done
   //

return;

}

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::read_fields_1 (Mode_Field_Info * & info_array, Dictionary * dict, GrdFileType type, char _fo,
                                  int field_index)

{

 const DictionaryEntry * ee = dict->lookup(conf_key_field);
 if ( !ee )  {

   mlog << "\nModeConfInfo::read_fields () -> \"field\" entry not found in dictionary!\n\n";

   exit ( 1 );

}

const Dictionary * field = ee->dict();

const int N = ( (field->is_array()) ? (field->n_entries()) : 1 );

if ( field->is_array() ) {

   const DictionaryEntry * e = 0;
   const Dictionary & D = *field;

    e = D[field_index];
    if ( (e->type() != DictionaryType) && (e->type() != ArrayType) )  {
       mlog << Error
            << "\nModeConfInfo::read_fields() -> field entry # " << field_index+1 << " is not a dictionary!\n\n";
       exit ( 1 );

    }

   info_array[field_index].set(true, field_index, e->dict_value(), &conf, type, _fo, false);

   return;

}   //  if is array

   //
   //  nope, traditional mode
   //

 if (field_index != 0) {
       mlog << Error
            << "\nModeConfInfo::read_fields() -> traditional mode but looking for a field array index " << field_index
            << "\n\n";
       exit ( 1 );
 }
    

info_array[0].set(false, 0, dict, &conf, type, _fo);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


PiecewiseLinear * ModeConfInfo::parse_interest_function(Dictionary * dict, const char * conf_key_if)

{

   //
   //  lookup piecewise linear interest function
   //

PiecewiseLinear * pwl_if = dict->lookup_pwl(conf_key_if);

   //
   //  range check the points
   //

for (int j=0; j<pwl_if->n_points(); ++j)  {

   if ( pwl_if->y(j) < 0 || pwl_if->y(j) > 1 )  {

      mlog << Error << "\nModeConfInfo::parse_interest_function() -> "
           << "all \"" << conf_key_if << "\" interest function points ("
           << pwl_if->x(j) << ", " << pwl_if->y(j)
           << ") must be in the range of 0 and 1.\n\n";

      exit(1);

   }

}   //  for j

return pwl_if;

}


////////////////////////////////////////////////////////////////////////


void ModeConfInfo::set_field_index(int k)

{
   if (data_type == ModeDataType_MvMode_Obs) {
      if ( (k < 0) || (k >= N_fields_o) ) {
         mlog << Error
              << "\nModeConfInfo::set_field_index(int) -> range check error\n\n";
         exit ( 1 );
      }
      Field_Index_o = k;
      Field_Index_f = -1;
      Obs =  obs_array + k;
   }
   else if (data_type == ModeDataType_MvMode_Fcst) {
      if ( (k < 0) || (k >= N_fields_f) ) {
         mlog << Error
              << "\nModeConfInfo::set_field_index(int) -> range check error\n\n";
         exit ( 1 );
      }
      Field_Index_f = k;
      Field_Index_o = -1;
      Fcst =  fcst_array + k;
   } else {
      // set obs and fcst index values the same
      set_field_index(k, k);
   }      
   return;
}


////////////////////////////////////////////////////////////////////////


void ModeConfInfo::set_field_index(int f_index, int o_index)

{
if ( (f_index < 0) || (f_index >= N_fields_f) || (o_index < 0) ||
     (o_index >= N_fields_o))  {

   mlog << Error
        << "\nModeConfInfo::set_field_index(int) -> range check error\n\n";

   exit ( 1 );

}

Field_Index_f = f_index;
Field_Index_o = o_index;

if (data_type != ModeDataType_MvMode_Fcst) {
   Obs =  obs_array + o_index;
}
if (data_type != ModeDataType_MvMode_Obs) {
   Fcst = fcst_array + f_index;
}
return;

}


////////////////////////////////////////////////////////////////////////


void ModeConfInfo::set_perc_thresh(const DataPlane &f_dp,
                                   const DataPlane &o_dp)


{

   //
   // Compute percentiles for forecast and observation thresholds.
   //
   if( !(Fcst->conv_thresh_array.need_perc()  ) &&
       !(Obs->conv_thresh_array.need_perc()   ) &&
       !(Fcst->merge_thresh_array.need_perc() ) &&
       !(Obs->merge_thresh_array.need_perc()  ) )  return;

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
    Fcst->conv_thresh_array.set_perc(&fsort, &osort, (NumArray *) 0,
                                     &(Fcst->conv_thresh_array),
                                      &(Obs->conv_thresh_array));

     Obs->conv_thresh_array.set_perc(&fsort, &osort, (NumArray *) 0,
                                     &(Fcst->conv_thresh_array),
                                      &(Obs->conv_thresh_array));

   Fcst->merge_thresh_array.set_perc(&fsort, &osort, (NumArray *) 0,
                                     &(Fcst->merge_thresh_array),
                                      &(Obs->merge_thresh_array));

    Obs->merge_thresh_array.set_perc(&fsort, &osort, (NumArray *) 0,
                                     &(Fcst->merge_thresh_array),
                                      &(Obs->merge_thresh_array));

   return;
}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::set_perc_thresh(const DataPlane &dp)
{
   //
   // Compute percentiles for forecast or observation thresholds.
   //
   Mode_Field_Info *F;
   
   if (data_type == ModeDataType_MvMode_Obs) {
      F = Obs;
   } else if (data_type == ModeDataType_MvMode_Fcst) {
      F = Fcst;
   } else {
      mlog << Warning 
           << "\nModeConfInfo::set_perc_thresh() -> "
           << "Logic error wrong method called expect obs only or fcst only, got "
           << sprintModeDataType(data_type) << "\n'n";
      return;
   }
   if( !(F->conv_thresh_array.need_perc()  ) &&
       !(F->merge_thresh_array.need_perc() ) ) {
      return;
   }

   //
   // Sort the input array
   //
   NumArray sort;
   int nxy = dp.nx() * dp.ny();

   sort.extend(nxy);

   for(int i=0; i<nxy; i++) {
      if(!is_bad_data(dp.data()[i])) sort.add(dp.data()[i]);
   }

   sort.sort_array();

   //
   // Compute percentiles by hacking in the same input as if its two
   //
   F->conv_thresh_array.set_perc(&sort, &sort, (NumArray *) 0,
                                 &(F->conv_thresh_array),
                                 &(F->conv_thresh_array));
   F->merge_thresh_array.set_perc(&sort, &sort, (NumArray *) 0,
                                  &(F->merge_thresh_array),
                                  &(F->merge_thresh_array));
   return;

}

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::parse_nc_info()

{

const DictionaryEntry * e = (const DictionaryEntry *) nullptr;

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
   if (data_type != ModeDataType_MvMode_Obs) {
      if ( (k < 0) || (k >= Fcst->conv_radius_array.n_elements()) )  {
         mlog << Error 
              << "\nModeConfInfo::set_conv_radius_by_index(int) -> "
              << "range check error\n\n";
         exit ( 1 );
      }
      Fcst->conv_radius =  Fcst->conv_radius_array[k];
   }
   if (data_type != ModeDataType_MvMode_Fcst) {
      if ( (k < 0) || (k >= Obs->conv_radius_array.n_elements()) )  {
         
         mlog << Error 
              << "\nModeConfInfo::set_conv_radius_by_index(int) -> "
              << "range check error\n\n";
         exit ( 1 );
      }
      Obs->conv_radius = Obs->conv_radius_array[k];
   }
   return;
}


////////////////////////////////////////////////////////////////////////


void ModeConfInfo::set_conv_thresh(SingleThresh s)
{
   if (data_type != ModeDataType_MvMode_Obs) {
      Fcst->conv_thresh = s;
   }
   if (data_type != ModeDataType_MvMode_Fcst) {
      Obs->conv_thresh =  s;
   }
}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::set_conv_radius(int r)
{
   if (data_type != ModeDataType_MvMode_Obs) {
      Fcst->conv_radius = r;
   }
   if (data_type != ModeDataType_MvMode_Fcst) {
      Obs->conv_radius = r;
   }
}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::set_conv_thresh_by_index(int k)

{
   if (data_type != ModeDataType_MvMode_Fcst) {
      if ( (k < 0) || (k >= Obs->conv_thresh_array.n_elements()) )  {
         mlog << Error 
              << "\nModeConfInfo::set_conv_thresh_by_index(int) -> "
              << "range check error\n\n";
         exit ( 1 );
      }
      Obs->conv_thresh =  Obs->conv_thresh_array[k];
   }
   if (data_type != ModeDataType_MvMode_Obs) {
      if ( (k < 0) || (k >= Fcst->conv_thresh_array.n_elements()) )  {
         
         mlog << Error 
              << "\nModeConfInfo::set_conv_thresh_by_index(int) -> "
              << "range check error\n\n";
         exit ( 1 );
      }
      Fcst->conv_thresh = Fcst->conv_thresh_array[k];
   }
   return;
}


////////////////////////////////////////////////////////////////////////

void ModeConfInfo::set_merge_thresh_by_index(int k)
{
   if (data_type != ModeDataType_MvMode_Fcst) {
      if ( Obs->need_merge_thresh  () )  set_obs_merge_thresh_by_index  (k);
   }
   if (data_type != ModeDataType_MvMode_Obs) {
         if ( Fcst->need_merge_thresh () )  set_fcst_merge_thresh_by_index (k);
   }
}   

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::set_fcst_merge_thresh_by_index(int k)

{
   if (data_type == ModeDataType_MvMode_Obs)
   {
      mlog << Error 
           << "\nModeConfInfo::set_fcst_merge_thresh_by_index(int) -> "
           << "software is set for obs data not fcst\n\n";
      exit (1);
   }
   Fcst->set_merge_thresh_by_index(k);
   return;

}




////////////////////////////////////////////////////////////////////////


void ModeConfInfo::set_conv_thresh_by_merge_index(int k)
{
   if (data_type != ModeDataType_MvMode_Fcst) {
      if (Obs->need_merge_thresh()) set_obs_conv_thresh_by_merge_index (k);
   }
   if (data_type != ModeDataType_MvMode_Obs) {
      if (Fcst->need_merge_thresh()) set_fcst_conv_thresh_by_merge_index (k);
   }
}

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::set_fcst_conv_thresh_by_merge_index(int k)

{
   if (data_type == ModeDataType_MvMode_Obs)
   {
      mlog << Error 
           << "\nModeConfInfo::set_fcst_conv_thresh_by_merge_index(int) -> "
           << "software is set for obs data not fcst\n\n";
      exit (1);
   }
   Fcst->set_conv_thresh_by_merge_index(k);

   return;

}


////////////////////////////////////////////////////////////////////////


void ModeConfInfo::set_obs_conv_thresh_by_merge_index(int k)

{
   if (data_type == ModeDataType_MvMode_Fcst)
   {
      mlog << Error 
           << "\nModeConfInfo::set_obs_conv_thresh_by_merge_index(int) -> "
           << "software is set for fcst data not obs\n\n";
      exit (1);
   }
   Obs->set_conv_thresh_by_merge_index(k);
   return;

}


////////////////////////////////////////////////////////////////////////

void ModeConfInfo::set_fcst_merge_flag(MergeType t)
{
   if (data_type == ModeDataType_MvMode_Obs)
   {
      mlog << Error 
           << "\nModeConfInfo::set_fcst_merge_flag(int) -> "
           << "software is set for obs data not fcst\n\n";
      exit (1);
   }
   Fcst->merge_flag = t;
}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::set_obs_merge_flag(MergeType t)
{
   if (data_type == ModeDataType_MvMode_Fcst)
   {
      mlog << Error 
           << "\nModeConfInfo::set_obs_merge_flag(int) -> "
           << "software is set for fcst data not obs\n\n";
      exit (1);
   }
   Obs->merge_flag = t;
}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::set_fcst_merge_thresh(SingleThresh s)
{
   if (data_type == ModeDataType_MvMode_Obs)
   {
      mlog << Error 
           << "\nModeConfInfo::set_fcst_merge_thresh(int) -> "
           << "software is set for obs data not fcst\n\n";
      exit (1);
   }
   Fcst->merge_thresh = s;
}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::set_obs_merge_thresh(SingleThresh s)
{
   if (data_type == ModeDataType_MvMode_Fcst)
   {
      mlog << Error 
           << "\nModeConfInfo::set_obs_merge_thresh(int) -> "
           << "software is set for fcst data not obs\n\n";
      exit (1);
   }
   Obs->merge_thresh = s;
}

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::set_obs_merge_thresh_by_index(int k)

{
   if (data_type == ModeDataType_MvMode_Fcst)
   {
      mlog << Error 
           << "\nModeConfInfo::set_obs_merge_thresh_by_index(int) -> "
           << "software is set for fcst data not obs\n\n";
      exit (1);
   }

   Obs->set_merge_thresh_by_index(k);
   return;
}

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::set_data_type(ModeDataType type)
{
   data_type = type;
}

////////////////////////////////////////////////////////////////////////

GrdFileType ModeConfInfo::file_type_for_field(bool isFcst, int field_index)
{
   // look at the dictionary for the obs or forecast at index, with
   // parents
   
   Dictionary * dict = (Dictionary *) nullptr;
   if (isFcst) {
      dict = conf.lookup_dictionary(conf_key_fcst);
   } else {
      dict = conf.lookup_dictionary(conf_key_obs);
   }

   const DictionaryEntry *e = dict->lookup(conf_key_field);
   if ( !e )  {
      mlog << "\nModeConfInfo::read_fields () -> \"field\" entry not found in dictionary!\n\n";
   exit ( 1 );
   }
   
   const Dictionary * field = e->dict();
   const int N = ( (field->is_array()) ? (field->n_entries()) : 1 );
   if (field_index < 0 || field_index >= N) {
      mlog << Error 
           << "\nModeConfInfo::file_type_for_field() -> "
           << "index " << field_index+1 << " out of range 0 to " << N
           << "\n\n";
      exit (1);
   }
   e = (*field)[field_index];
   if ( (e->type() != DictionaryType) && (e->type() != ArrayType) )  {
      mlog << Error
           << "\nModeConfInfo::file_type_for_field() -> "
           << "field entry # " << field_index+1 << " is not a dictionary!\n\n";
      exit ( 1 );
   }
   Dictionary *dictf = e->dict_value();
   return parse_conf_file_type(dictf);
}

////////////////////////////////////////////////////////////////////////


int ModeConfInfo::n_runs() const

{

const int nr = n_conv_radii();
const int nt = n_conv_threshs();

return ( nr*nt );

}


////////////////////////////////////////////////////////////////////////


bool ModeConfInfo::is_multivar() 

{

const DictionaryEntry * e = conf.lookup("fcst");

if ( e->type() != DictionaryType )  {

   mlog << Error
        << "\nModeConfInfo::is_multivar() const -> bad object type for entry \"fcst\"\n\n";

   exit ( 1 );

}


const DictionaryEntry * e2 = e->dict()->lookup("field");
bool status = false;

switch ( e2->type() )  {

   case ArrayType:        status = true;   break;
   case DictionaryType:   status = false;  break;

   default:
      mlog << Error
           << "\nModeConfInfo::is_multivar() const -> bad object type for entry \"fcst.field\"\n\n";
      exit ( 1 );

}


return status;

}


////////////////////////////////////////////////////////////////////////


void ModeConfInfo::get_multivar_programs()

{

Dictionary * dict = (Dictionary *) nullptr;

fcst_multivar_logic.clear();
obs_multivar_logic.clear();
fcst_multivar_compare_index.clear();
obs_multivar_compare_index.clear();
fcst_multivar_name = default_multivar_name;
obs_multivar_name = default_multivar_name;
fcst_multivar_level = na_str;
obs_multivar_level = na_str;

// this has to be outside the fcst and obs dictionaries
if ( conf.lookup(conf_key_fcst_multivar_compare_index)) fcst_multivar_compare_index = conf.lookup_int_array(conf_key_fcst_multivar_compare_index);

// this has to be outside the fcst and obs dictionaries
if ( conf.lookup(conf_key_obs_multivar_compare_index)) obs_multivar_compare_index = conf.lookup_int_array(conf_key_obs_multivar_compare_index);


dict = conf.lookup_dictionary(conf_key_fcst);

if ( dict->lookup(conf_key_multivar_logic) ) fcst_multivar_logic = dict->lookup_string(conf_key_multivar_logic);
if ( dict->lookup(conf_key_multivar_name)  ) fcst_multivar_name  = dict->lookup_string(conf_key_multivar_name);
if ( dict->lookup(conf_key_multivar_level) ) fcst_multivar_level = dict->lookup_string(conf_key_multivar_level);

dict = conf.lookup_dictionary(conf_key_obs);

if ( dict->lookup(conf_key_multivar_logic) ) obs_multivar_logic = dict->lookup_string(conf_key_multivar_logic);
if ( dict->lookup(conf_key_multivar_name)  ) obs_multivar_name  = dict->lookup_string(conf_key_multivar_name);
if ( dict->lookup(conf_key_multivar_level) ) obs_multivar_level = dict->lookup_string(conf_key_multivar_level);


return;
}

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::check_multivar_not_implemented()
{
   if (!is_multivar()) {
      return;
   }

   bool status = false;
   if (quilt) {
      mlog << Error
           << "\nModeConfInfo::check_multivar_not_implemented():\n"
           << "  quilting not yet implemented for multivar mode\n\n";
      status = true;
   }
   
   for (int i=0; i<N_fields_f; ++i) {
      if (data_type != ModeDataType_MvMode_Obs) {
         if (fcst_array[i].merge_flag == MergeType::Both || fcst_array[i].merge_flag == MergeType::Engine)
         {
            mlog << Error
                 << "\nModeConfInfo::check_multivar_not_implemented():\n"
                 << "  merge_flag ENGINE or BOTH not implemented for multivariate mode\n\n";
            status = true;
         }
         if (fcst_array[i].conv_thresh_array.n() > 1 || fcst_array[i].merge_thresh_array.n() > 1) {
            mlog << Error
                 << "\nModeConfInfo::check_multivar_not_implemented():\n"
                 << "  more than one conv_thresh or merge_thresh per input is not allowed in multivariate mode\n\n";
            status = true;
         }
      }
   }
   for (int i=0; i<N_fields_o; ++i) {
      if (data_type != ModeDataType_MvMode_Fcst) {
         if (obs_array[i].merge_flag == MergeType::Both || obs_array[i].merge_flag == MergeType::Engine) {
            mlog << Error
                 << "\nModeConfInfo::check_multivar_not_implemented():\n"
                 << "  merge_flag ENGINE or BOTH not implemented for multivariate mode\n\n";
            status = true;
            break;
         }
         if (obs_array[i].conv_thresh_array.n() > 1 || obs_array[i].merge_thresh_array.n() > 1) {
            mlog << Error
                 << "\nModeConfInfo::check_multivar_not_implemented():\n"
                 << "  more than one conv_thresh or merge_thresh per input is not allowed in multivariate mode\n\n";
            status = true;
         }
      }
   }

   if (status) {
      mlog << Error
           << "\nModeConfInfo::check_multivar_not_implemented:\n"
           << "  Some features not yet implemented in multivar mode\n\n";
      exit ( 1 );
   }
}


////////////////////////////////////////////////////////////////////////

PercThreshType ModeConfInfo::perctype(const Mode_Field_Info &f)  const
{
   PercThreshType pm=no_perc_thresh_type;;
   PercThreshType pc=no_perc_thresh_type;;
   if  (f.merge_thresh_array.n() > 0) {
      pm = f.merge_thresh_array[0].get_ptype();
   }
   if  (f.conv_thresh_array.n() > 0) {
      pc = f.conv_thresh_array[0].get_ptype();
   }
   if (pm == perc_thresh_sample_climo || pc == perc_thresh_sample_climo) {
      mlog << Error << "\nModeConfInfo::perctype()\n"
           << "  Thresholding with 'SCP' in an input not implemented for multivariate mode\n\n";
      exit ( 1 );
   }
   if (pm == perc_thresh_climo_dist || pc == perc_thresh_climo_dist) {
      mlog << Error << "\nModeConfInfo::perctype()\n"
              << "  Thresholding with 'CDP' in an input not implemented for multivariate mode\n\n";
      exit ( 1 );
   }
   if (pm == perc_thresh_freq_bias ||
       pc == perc_thresh_freq_bias) {
      return perc_thresh_freq_bias;
   }
   // put in tests if they are not the same and
   // both are sample settings, so the right one
   // gets returned (pass in fcst/obs boolean)
   if (pm == perc_thresh_sample_obs ||
       pc == perc_thresh_sample_obs) {
      return perc_thresh_sample_obs;
   }
   if (pm == perc_thresh_sample_fcst ||
       pc == perc_thresh_sample_fcst) {
      return perc_thresh_sample_fcst;
   }
   return no_perc_thresh_type;
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

return !status;

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
