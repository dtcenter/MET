// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <sys/types.h>
#include <dirent.h>

#include "config_util.h"

#include "vx_math.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

ConcatString parse_conf_version(Dictionary *dict) {
   ConcatString s;

   if(!dict) {
      mlog << Error << "\nparse_conf_version() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   s = dict->lookup_string(conf_key_version);

   if(dict->last_lookup_status()) {
      check_met_version(s);
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString parse_conf_model(Dictionary *dict) {
   ConcatString s;

   if(!dict) {
      mlog << Error << "\nparse_conf_model() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }
   
   s = dict->lookup_string(conf_key_model);

   if(dict->last_lookup_status()) {
   
      // Check that it's non-empty and contains no whitespace
      if(s.empty() || check_reg_exp(ws_reg_exp, s) == true) {
         mlog << Error << "\nparse_conf_model() -> "
              << "The model name (\"" << s << "\") must be non-empty and "
              << "contain no embedded whitespace.\n\n";
         exit(1);
      }
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

GrdFileType parse_conf_file_type(Dictionary *dict) {
   GrdFileType t = FileType_None;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_file_type() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }
   
   // Get the integer flag value for the current entry
   v = dict->lookup_int(conf_key_file_type, false);

   if(dict->last_lookup_status()) {
      // Convert integer to enumerated GrdFileType
           if(v == conf_const.lookup_int(conf_val_grib1))       t = FileType_Gb1;
      else if(v == conf_const.lookup_int(conf_val_grib2))       t = FileType_Gb2;
      else if(v == conf_const.lookup_int(conf_val_netcdf_met))  t = FileType_NcMet;
      else if(v == conf_const.lookup_int(conf_val_netcdf_pint)) t = FileType_NcPinterp;
      else {
         mlog << Error << "\nparse_conf_file_type() -> "
              << "Unexpected config file value of " << v << " for \""
              << conf_key_file_type << "\".\n\n";
         exit(1);
      }
   } 

   return(t);
}

////////////////////////////////////////////////////////////////////////

map<STATLineType,STATOutputType> parse_conf_output_flag(Dictionary *dict) {
   Dictionary *out_dict = (Dictionary *) 0;
   map<STATLineType,STATOutputType> output_map;
   STATLineType line_type;
   STATOutputType output_type;
   int i, v;

   if(!dict) {
      mlog << Error << "\nparse_conf_output_flag() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }
   
   // Get the output flag dictionary
   out_dict = dict->lookup_dictionary(conf_key_output_flag);
   
   // Loop over the output flag dictionary entries
   for(i=0; i<out_dict->n_entries(); i++) {

      // Get the line type for the current entry
      line_type = string_to_statlinetype((*out_dict)[i]->name());
     
      // Get the integer flag value for the current entry
      v = out_dict->lookup_int((*out_dict)[i]->name());

      // Convert integer to enumerated STATOutputType
           if(v == conf_const.lookup_int(conf_val_none)) output_type = STATOutputType_None;
      else if(v == conf_const.lookup_int(conf_val_stat)) output_type = STATOutputType_Stat;
      else if(v == conf_const.lookup_int(conf_val_both)) output_type = STATOutputType_Both;
      else {
         mlog << Error << "\nparse_conf_output_flag() -> "
              << "Unexpected config file value of " << v << " for \""
              << conf_key_output_flag << "." << (*out_dict)[i]->name()
              << "\".\n\n";
         exit(1);
      }

      // Store entry line type and corresponding output type
      output_map[line_type] = output_type;
   }
   
   return(output_map);
}

////////////////////////////////////////////////////////////////////////

map<STATLineType,StringArray> parse_conf_output_stats(Dictionary *dict) {
   Dictionary *out_dict = (Dictionary *) 0;
   map<STATLineType,StringArray> output_map;
   STATLineType line_type;
   StringArray sa;
   int i;

   if(!dict) {
      mlog << Error << "\nparse_conf_output_stats() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Get the output flag dictionary
   out_dict = dict->lookup_dictionary(conf_key_output_stats);

   // Loop over the output flag dictionary entries
   for(i=0; i<out_dict->n_entries(); i++) {

      // Get the line type for the current entry
      line_type = string_to_statlinetype((*out_dict)[i]->name());

      // Get the StringArray value for the current entry
      sa = out_dict->lookup_string_array((*out_dict)[i]->name());

      // Set ignore case to true
      sa.set_ignore_case(true);

      // Store entry line type and corresponding list of statistics
      output_map[line_type].add(sa);
   }

   return(output_map);
}

////////////////////////////////////////////////////////////////////////
//
// Compute the number of verification tasks specified in the current
// dictionary array.
//
////////////////////////////////////////////////////////////////////////

int parse_conf_n_vx(Dictionary *dict) {
   int i, total;

   if(!dict) {
      mlog << Error << "\nparse_conf_n_vx() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }
   
   // Check that this dictionary is an array
   if(!dict->is_array()) {
      mlog << Error << "\nparse_conf_n_vx() -> "
           << "This function must be passed a Dictionary array.\n\n";
      exit(1);
   }

   // Loop over the fields to be verified
   for(i=0,total=0; i<dict->n_entries(); i++) {

      // Increment count by the length of the level array
      total += (*dict)[i]->dict_value()->lookup_string_array(conf_key_level).n_elements();
   }

   return(total);
}

////////////////////////////////////////////////////////////////////////
//
// Retrieve the dictionary for the i-th verification task.
//
////////////////////////////////////////////////////////////////////////

Dictionary parse_conf_i_vx_dict(Dictionary *dict, int index) {
   Dictionary i_dict;
   DictionaryEntry entry;
   StringArray lvl;
   int i, total;

   if(!dict) {
      mlog << Error << "\nparse_conf_i_vx_dict() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }
   
   // Check that this dictionary is an array
   if(!dict->is_array()) {
      mlog << Error << "\nparse_conf_i_vx_dict() -> "
           << "This function must be passed a Dictionary array.\n\n";
      exit(1);
   }
   
   // Loop over the fields to be verified
   for(i=0,total=0; i<dict->n_entries(); i++) {

      // Increment count by the length of the level array
      lvl    = (*dict)[i]->dict_value()->lookup_string_array(conf_key_level);
      total += lvl.n_elements();

      // Check if we're in the correct entry
      if(total > index) {

         // Copy the current entry's dictionary
         i_dict = *((*dict)[i]->dict_value());

         // Set up the new entry, taking only a single level value
         entry.set_string(conf_key_level, lvl[index-(total-lvl.n_elements())]);

         // Store the new entry
         i_dict.store(entry);

         break;
      }
   } // end for i

   return(i_dict);
}

////////////////////////////////////////////////////////////////////////

StringArray parse_conf_message_type(Dictionary *dict) {
   StringArray sa;
   int i;

   if(!dict) {
      mlog << Error << "\nparse_conf_message_type() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }
   
   sa = dict->lookup_string_array(conf_key_message_type);

   // Check that at least one message type is provided
   if(sa.n_elements() == 0) {
      mlog << Error << "\nparse_conf_message_type() -> "
           << "At least one message type must be provided.\n\n";
      exit(1);
   }

   return(sa);
}

////////////////////////////////////////////////////////////////////////

StringArray parse_conf_obs_qty(Dictionary *dict) {
   StringArray sa;

   if(!dict) {
      mlog << Error << "\nparse_conf_obs_qty() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   sa = dict->lookup_string_array(conf_key_obs_qty);

   return(sa);
}

////////////////////////////////////////////////////////////////////////

NumArray parse_conf_ci_alpha(Dictionary *dict) {
   NumArray na;
   int i;

   if(!dict) {
      mlog << Error << "\nparse_conf_ci_alpha() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }
   
   na = dict->lookup_num_array(conf_key_ci_alpha);

   // Check that at least one alpha value is provided
   if(na.n_elements() == 0) {
      mlog << Error << "\nparse_conf_ci_alpha() -> "
           << "At least one confidence interval alpha value must be "
           << "specified.\n\n";
      exit(1);
   }

   // Check that the values for alpha are between 0 and 1
   for(i=0; i<na.n_elements(); i++) {
      if(na[i] <= 0.0 || na[i] >= 1.0) {
         mlog << Error << "\nparse_conf_ci_alpha() -> "
              << "All confidence interval alpha values ("
              << na[i] << ") must be greater than 0 "
              << "and less than 1.\n\n";
         exit(1);
      }
   }

   return(na);
}

////////////////////////////////////////////////////////////////////////

TimeSummaryInfo parse_conf_time_summary(Dictionary *dict) {
  if (!dict)
  {
    mlog << Error << "\nparse_conf_time_summary() -> "
	 << "empty dictionary!\n\n";
    exit(1);
  }
   
  TimeSummaryInfo info;

  // Conf: time_summary.flag
  info.flag = dict->lookup_bool(conf_key_time_summary_flag);

  // Conf: time_summary.beg
  info.beg = timestring_to_sec(dict->lookup_string(conf_key_time_summary_beg));

  // Conf: time_summary.end
  info.end = timestring_to_sec(dict->lookup_string(conf_key_time_summary_end));

  // Conf: time_summary.step
  info.step = dict->lookup_int(conf_key_time_summary_step);
  if (info.step <= 0) {
    mlog << Error << "\nparse_conf_time_summary() -> "
	 << "The \"" << conf_key_time_summary_step
	 << "\" parameter (" << info.step
	 << ") must be greater than 0!\n\n";
    exit(1);
  }
  
  // Conf: time_summary.width
  info.width = dict->lookup_int(conf_key_time_summary_width);
  if (info.width <= 0) {
    mlog << Error << "\nparse_conf_time_summary() -> "
	 << "The \"" << conf_key_time_summary_width
	 << "\" parameter (" << info.width
	 << ") must be greater than 0!\n\n";
    exit(1);
  }
  
  // Conf: time_summary.grib_code
  info.grib_code = dict->lookup_int_array(conf_key_time_summary_grib_code);
  
  // Conf: time_summary.type
  info.type = dict->lookup_string_array(conf_key_time_summary_type);

  return(info);
}

////////////////////////////////////////////////////////////////////////

BootInfo parse_conf_boot(Dictionary *dict) {
   BootInfo info;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_boot() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }
   
   // Conf: boot.interval
   v = dict->lookup_int(conf_key_boot_interval);

   // Convert integer to enumerated BootIntervalType
        if(v == conf_const.lookup_int(conf_val_bca))    info.interval = BootIntervalType_BCA;
   else if(v == conf_const.lookup_int(conf_val_pctile)) info.interval = BootIntervalType_Percentile;
   else {
      mlog << Error << "\nparse_conf_boot() -> "
           << "Unexpected config file value of " << v << " for \""
           << conf_key_boot_interval << "\".\n\n";
      exit(1);
   }

   // Conf: boot.rep_prop
   info.rep_prop = dict->lookup_double(conf_key_boot_rep_prop);

   // Check that it is between 0 and 1
   if(info.rep_prop <= 0.0 || info.rep_prop > 1.0) {
      mlog << Error << "\nparse_conf_boot() -> "
           << "The \"" << conf_key_boot_rep_prop
           << "\" parameter (" << info.rep_prop
           << ")must be between 0 and 1!\n\n";
      exit(1);
   }

   // Conf: boot.n_rep
   info.n_rep = dict->lookup_int(conf_key_boot_n_rep);

   // Check n_rep >= 0
   if(info.n_rep < 0) {
      mlog << Error << "\nparse_conf_boot() -> "
           << "The number of bootstrap resamples in the \""
           << conf_key_boot_n_rep << "\" parameter (" << info.n_rep
           << ") must be >= 0.\n\n";
      exit(1);
   }

   // Conf: boot_rng
   info.rng = dict->lookup_string(conf_key_boot_rng);

   // Conf: boot_seed
   info.seed = dict->lookup_string(conf_key_boot_seed);

   return(info);
}

////////////////////////////////////////////////////////////////////////

InterpInfo parse_conf_interp(Dictionary *dict) {
   Dictionary *interp_dict = (Dictionary *) 0;
   Dictionary *type_dict = (Dictionary *) 0;
   InterpInfo info;
   NumArray mthd_na, wdth_na;
   ConcatString method;
   int i, j, k, v, width, n_entries;
   bool is_correct_type = false;

   if(!dict) {
      mlog << Error << "\nparse_conf_interp() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: interp
   interp_dict = dict->lookup_dictionary(conf_key_interp);

   // Conf: field - may be missing
   v = interp_dict->lookup_int(conf_key_field, false);

   // If found, interpret value
   if(interp_dict->last_lookup_status()) info.field = int_to_fieldtype(v);

   // Conf: vld_thresh
   info.vld_thresh = interp_dict->lookup_double(conf_key_vld_thresh);

   // Check that the interpolation threshold is between 0 and 1.
   if(info.vld_thresh < 0.0 || info.vld_thresh > 1.0) {
      mlog << Error << "\nparse_conf_interp() -> "
           << "The \"" << conf_key_interp << "." << conf_key_vld_thresh
           << "\" parameter (" << info.vld_thresh
           << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   // Conf: type
   const DictionaryEntry * type_entry = interp_dict->lookup(conf_key_type);

   if(type_entry) is_correct_type = (type_entry->type() == ArrayType ||
                                     type_entry->type() == DictionaryType);

   // Check that type is a dictionary or array of dictionaries
   if(!type_entry || !is_correct_type) {
      mlog << Error << "\nparse_conf_interp() -> "
           << "Lookup failed for name \"" << conf_key_type << "\"\n\n";
      exit(1);
   }

   if(type_entry->type() == ArrayType) {
      type_dict = type_entry->array_value();
      n_entries = type_dict->n_entries();
   }
   else {
      type_dict = type_entry->dict_value();
      n_entries = 1;
   }

   // Loop over the interpolation type dictionary entries
   for(i=0, info.n_interp=0; i<n_entries; i++) {

      // Get the methods and widths for the current entry
      if(type_entry->type() == ArrayType) {
         mthd_na = (*type_dict)[i]->dict_value()->lookup_num_array(conf_key_method);
         wdth_na = (*type_dict)[i]->dict_value()->lookup_num_array(conf_key_width);
      }
      else {
         mthd_na = type_dict->lookup_num_array(conf_key_method);
         wdth_na = type_dict->lookup_num_array(conf_key_width);
      }

      // Loop over the methods
      for(j=0; j<mthd_na.n_elements(); j++) {

         // Store interpolation method as a string
              if(mthd_na[j] == conf_const.lookup_int(interpmthd_min_str))     method = interpmthd_min_str;
         else if(mthd_na[j] == conf_const.lookup_int(interpmthd_max_str))     method = interpmthd_max_str;
         else if(mthd_na[j] == conf_const.lookup_int(interpmthd_median_str))  method = interpmthd_median_str;
         else if(mthd_na[j] == conf_const.lookup_int(interpmthd_uw_mean_str)) method = interpmthd_uw_mean_str;
         else if(mthd_na[j] == conf_const.lookup_int(interpmthd_dw_mean_str)) method = interpmthd_dw_mean_str;
         else if(mthd_na[j] == conf_const.lookup_int(interpmthd_ls_fit_str))  method = interpmthd_ls_fit_str;
         else if(mthd_na[j] == conf_const.lookup_int(interpmthd_bilin_str))   method = interpmthd_bilin_str;
         else {
            mlog << Error << "\nparse_conf_interval() -> "
                 << "Unexpected config file value of " << mthd_na[j]
                 << " for \"" << conf_key_method << "\".\n\n";
            exit(1);
         }

         // Loop over the widths
         for(k=0; k<wdth_na.n_elements(); k++) {

            // Store the current width
            width = nint(wdth_na[k]);

            // Check for the nearest neighbor special case
            if(width == 1 && strcmp(method, interpmthd_uw_mean_str) != 0) {
               mlog << Warning << "\nparse_conf_interp() -> "
                    << "For neareast neighbor interpolation method, "
                    << "resetting method from \"" << method << "\" to \""
                    << interpmthd_uw_mean_str
                    << "\" since the interpolation width is 1.\n\n";
               method = interpmthd_uw_mean_str;
            }

            // Check for the bilinear interpolation special case
            if(width != 2 && strcmp(method, interpmthd_bilin_str) == 0) {
               mlog << Warning << "\nparse_conf_interp() -> "
                    << "For bilinear interpolation method, resetting "
                    << "width from \"" << width << "\" to \"2\".\n\n";
               width = 2;
            }

            // Add the current entries
            info.n_interp += 1;
            info.method.add(method);
            info.width.add(width);

         } // end for k
      } // end for j
   } // end for i

   return(info);
}

////////////////////////////////////////////////////////////////////////

NbrhdInfo parse_conf_nbrhd(Dictionary *dict) {
   Dictionary *nbrhd_dict = (Dictionary *) 0;
   NbrhdInfo info;
   int i;

   if(!dict) {
      mlog << Error << "\nparse_conf_nbrhd() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }
   
   // Conf: nbrhd
   nbrhd_dict = dict->lookup_dictionary(conf_key_nbrhd);

   // Conf: vld_thresh
   info.vld_thresh = nbrhd_dict->lookup_double(conf_key_vld_thresh);

   // Check that the interpolation threshold is between 0 and 1.
   if(info.vld_thresh < 0.0 || info.vld_thresh > 1.0) {
      mlog << Error << "\nparse_conf_nbrhd() -> "
           << "The \"" << conf_key_nbrhd << "." << conf_key_vld_thresh
           << "\" parameter (" << info.vld_thresh
           << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   // Conf: width
   info.width = nbrhd_dict->lookup_num_array(conf_key_width);

   // Check that at least one neighborhood width is provided
   if(info.width.n_elements() == 0) {
      mlog << Error << "\nparse_conf_nbrhd() -> "
           << "At least one neighborhood width must be provided.\n\n";
      exit(1);
   }

   // Check for valid widths
   for(i=0; i<info.width.n_elements(); i++) {

      if(info.width[i] < 1 || info.width[i]%2 == 0) {
         mlog << Error << "\nparse_conf_nbrhd() -> "
              << "The neighborhood widths must be odd values greater "
              << "than or equal to 1 (" << info.width[i] << ").\n\n";
         exit(1);
      }
   }
   
   // Conf: cov_thresh
   info.cov_ta = nbrhd_dict->lookup_thresh_array(conf_key_cov_thresh);

   // Check that at least one neighborhood coverage threshold is provided
   if(info.cov_ta.n_elements() == 0) {
      mlog << Error << "\nparse_conf_nbrhd() -> "
           << "At least one neighborhood coverage threshold must be provided.\n\n";
      exit(1);
   }

   // Check for valid coverage thresholds
   for(i=0; i<info.cov_ta.n_elements(); i++) {

      if(info.cov_ta[i].thresh < 0.0 || info.cov_ta[i].thresh > 1.0) {
         mlog << Error << "\nparse_conf_nbrhd() -> "
              << "The neighborhood coverage threshold value must be set "
              << "between 0 and 1.\n\n";
         exit(1);
      }
   }
   
   return(info);
}

////////////////////////////////////////////////////////////////////////

DuplicateType parse_conf_duplicate_flag(Dictionary *dict) {
   DuplicateType t;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_duplicate_flag() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Get the integer flag value for the current entry
   v = dict->lookup_int(conf_key_duplicate_flag);
      
   // Convert integer to enumerated DuplicateType
        if(v == conf_const.lookup_int(conf_val_none))   t = DuplicateType_None;
   else if(v == conf_const.lookup_int(conf_val_unique)) t = DuplicateType_Unique;
   else if(v == conf_const.lookup_int(conf_val_single)) t = DuplicateType_Single;
   else {
      mlog << Error << "\nparse_conf_duplicate_flag() -> "
           << "Unexpected config file value of " << v << " for \""
           << conf_key_duplicate_flag << "\".\n\n";
      exit(1);
   }

   return(t);
}

////////////////////////////////////////////////////////////////////////

ConcatString parse_conf_tmp_dir(Dictionary *dict) {
   ConcatString s;

   if(!dict) {
      mlog << Error << "\nparse_conf_tmp_dir() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }
   
   // Read the temporary directory
   s = dict->lookup_string(conf_key_tmp_dir);

   // Make sure that it exists
   if(opendir(s) == NULL ) {
      mlog << Error << "\nparse_conf_tmp_dir() -> "
           << "Cannot access the \"" << conf_key_tmp_dir << "\" directory: "
           << s << "\n\n";
      exit(1);
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

GridDecompType parse_conf_grid_decomp_flag(Dictionary *dict) {
   GridDecompType t;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_grid_decomp_flag() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }
   
   // Get the integer flag value for the current entry
   v = dict->lookup_int(conf_key_grid_decomp_flag);

   // Convert integer to enumerated GridDecompType
        if(v == conf_const.lookup_int(conf_val_none)) t = GridDecompType_None;
   else if(v == conf_const.lookup_int(conf_val_auto)) t = GridDecompType_Auto;
   else if(v == conf_const.lookup_int(conf_val_tile)) t = GridDecompType_Tile;
   else if(v == conf_const.lookup_int(conf_val_pad))  t = GridDecompType_Pad;
   else {
      mlog << Error << "\nparse_conf_grid_decomp_flag() -> "
           << "Unexpected config file value of " << v << " for \""
           << conf_key_grid_decomp_flag << "\".\n\n";
      exit(1);
   }

   return(t);
}

////////////////////////////////////////////////////////////////////////

WaveletType parse_conf_wavelet_type(Dictionary *dict) {
   WaveletType t;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_wavelet_type() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }
   
   // Get the integer flag value for the current entry
   v = dict->lookup_int(conf_key_wavelet_type);

   // Convert integer to enumerated WaveletType
        if(v == conf_const.lookup_int(conf_val_none))         t = WaveletType_None;
   else if(v == conf_const.lookup_int(conf_val_haar))         t = WaveletType_Haar;
   else if(v == conf_const.lookup_int(conf_val_haar_cntr))    t = WaveletType_Haar_Cntr;
   else if(v == conf_const.lookup_int(conf_val_daub))         t = WaveletType_Daub;
   else if(v == conf_const.lookup_int(conf_val_daub_cntr))    t = WaveletType_Daub_Cntr;
   else if(v == conf_const.lookup_int(conf_val_bspline))      t = WaveletType_BSpline;
   else if(v == conf_const.lookup_int(conf_val_bspline_cntr)) t = WaveletType_BSpline_Cntr;
   else {
      mlog << Error << "\nparse_conf_wavelet_type() -> "
           << "Unexpected config file value of " << v << " for \""
           << conf_key_wavelet_type << "\".\n\n";
      exit(1);
   }

   return(t);
}

////////////////////////////////////////////////////////////////////////

PlotInfo parse_conf_plot_info(Dictionary *dict) {
   PlotInfo info;

   if(!dict) {
      mlog << Error << "\nparse_conf_plot_info() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }
   
   // Get the color table
   info.color_table = dict->lookup_string(conf_key_color_table);

   // Get the minimum plot value, 0 if not present
   info.plot_min = dict->lookup_double(conf_key_plot_min, false);
   if(is_bad_data(info.plot_min)) info.plot_min = 0.0;
   
   // Get the maximum plot value, 0 if not present
   info.plot_max = dict->lookup_double(conf_key_plot_max, false);
   if(is_bad_data(info.plot_max)) info.plot_max = 0.0;

   // Get the colorbar spacing, 1 if not present
   info.colorbar_spacing = dict->lookup_int(conf_key_colorbar_spacing, false);
   if(is_bad_data(info.colorbar_spacing)) info.colorbar_spacing = 1;

   // Check that the colorbar spacing is set >= 1
   if(info.colorbar_spacing < 1) {
      mlog << Error << "\nparse_conf_plot_info() -> "
           << "the colorbar_spacing (" << info.colorbar_spacing
           << ") must be set >= 1.\n\n";
      exit(1);
   }   

   return(info);
}

////////////////////////////////////////////////////////////////////////

void parse_conf_range_int(Dictionary *dict, int &beg, int &end) {

   if(!dict) {
      mlog << Error << "\nparse_conf_range_int() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Lookup the integer values
   beg = dict->lookup_int(conf_key_beg);
   end = dict->lookup_int(conf_key_end);

   // Check the range
   if(beg > end) {
      mlog << Error << "\nparse_conf_range_int() -> "
           << "the ending value (" << end
           << ") must be >= the beginning value (" << beg << ").\n\n";
      exit(1);
   }
   
   return;
}

////////////////////////////////////////////////////////////////////////

void parse_conf_range_double(Dictionary *dict, double &beg, double &end) {

   if(!dict) {
      mlog << Error << "\nnparse_conf_range_double -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Lookup the double values
   beg = dict->lookup_double(conf_key_beg);
   end = dict->lookup_double(conf_key_end);

   // Check the range
   if(beg > end) {
      mlog << Error << "\nparse_conf_range_double() -> "
           << "the ending value (" << end
           << ") must be >= the beginning value (" << beg << ").\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void check_prob_thresh(const ThreshArray &ta) {
   int i, n;

   n = ta.n_elements();

   // Check for at least 3 thresholds beginning with 0 and ending with 1.
   if(n < 3 ||
      !is_eq(ta[0].thresh,   0.0) ||
      !is_eq(ta[n-1].thresh, 1.0)) {

      mlog << Error << "\ncheck_prob_thresh() -> "
           << "When verifying a probability field, you must "
           << "select at least 3 thresholds beginning with 0.0 "
           << "and ending with 1.0.\n\n";
      exit(1);
   }

   for(i=0; i<n; i++) {

      // Check that all threshold types are greater than or equal to
      if(ta[i].type != thresh_ge) {
         mlog << Error << "\ncheck_prob_thresh() -> "
              << "When verifying a probability field, all "
              << "thresholds must be set as equal to, "
              << "using \"ge\" or \">=\".\n\n";
         exit(1);
      }

      // Check that all thresholds are in [0, 1].
      if(ta[i].thresh < 0.0 ||
         ta[i].thresh > 1.0) {

         mlog << Error << "\ncheck_prob_thresh() -> "
              << "When verifying a probability field, all "
              << "thresholds must be between 0 and 1.\n\n";
         exit(1);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

const char * statlinetype_to_string(const STATLineType t) {
   const char *s = (const char *) 0;

   switch(t) {
      case(stat_sl1l2):        s = stat_sl1l2_str;   break;
      case(stat_sal1l2):       s = stat_sal1l2_str;  break;
      case(stat_vl1l2):        s = stat_vl1l2_str;   break;
      case(stat_val1l2):       s = stat_val1l2_str;  break;
      case(stat_fho):          s = stat_fho_str;     break;
      case(stat_ctc):          s = stat_ctc_str;     break;
      case(stat_cts):          s = stat_cts_str;     break;
      case(stat_mctc):         s = stat_mctc_str;    break;
      case(stat_mcts):         s = stat_mcts_str;    break;
      case(stat_cnt):          s = stat_cnt_str;     break;
      case(stat_pct):          s = stat_pct_str;     break;
      case(stat_pstd):         s = stat_pstd_str;    break;
      case(stat_pjc):          s = stat_pjc_str;     break;
      case(stat_prc):          s = stat_prc_str;     break;
      case(stat_mpr):          s = stat_mpr_str;     break;
      case(stat_nbrctc):       s = stat_nbrctc_str;  break;
      case(stat_nbrcts):       s = stat_nbrcts_str;  break;
      case(stat_nbrcnt):       s = stat_nbrcnt_str;  break;
      case(stat_isc):          s = stat_isc_str;     break;
      case(stat_wdir):         s = stat_wdir_str;    break;
      case(stat_rhist):        s = stat_rhist_str;   break;
      case(stat_orank):        s = stat_orank_str;   break;
      case(stat_ssvar):        s = stat_ssvar_str;   break;
      case(no_stat_line_type): s = stat_na_str;      break;
      default:                 s = (const char *) 0; break;
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

void statlinetype_to_string(const STATLineType t, char *out) {

   strcpy(out, statlinetype_to_string(t));

   return;
}

////////////////////////////////////////////////////////////////////////

STATLineType string_to_statlinetype(const char *s) {
   STATLineType t;

        if(strcasecmp(s, stat_sl1l2_str)  == 0) t = stat_sl1l2;
   else if(strcasecmp(s, stat_sal1l2_str) == 0) t = stat_sal1l2;
   else if(strcasecmp(s, stat_vl1l2_str)  == 0) t = stat_vl1l2;
   else if(strcasecmp(s, stat_val1l2_str) == 0) t = stat_val1l2;
   else if(strcasecmp(s, stat_fho_str)    == 0) t = stat_fho;
   else if(strcasecmp(s, stat_ctc_str)    == 0) t = stat_ctc;
   else if(strcasecmp(s, stat_cts_str)    == 0) t = stat_cts;
   else if(strcasecmp(s, stat_mctc_str)   == 0) t = stat_mctc;
   else if(strcasecmp(s, stat_mcts_str)   == 0) t = stat_mcts;
   else if(strcasecmp(s, stat_cnt_str)    == 0) t = stat_cnt;
   else if(strcasecmp(s, stat_pct_str)    == 0) t = stat_pct;
   else if(strcasecmp(s, stat_pstd_str)   == 0) t = stat_pstd;
   else if(strcasecmp(s, stat_pjc_str)    == 0) t = stat_pjc;
   else if(strcasecmp(s, stat_prc_str)    == 0) t = stat_prc;
   else if(strcasecmp(s, stat_mpr_str)    == 0) t = stat_mpr;
   else if(strcasecmp(s, stat_nbrctc_str) == 0) t = stat_nbrctc;
   else if(strcasecmp(s, stat_nbrcts_str) == 0) t = stat_nbrcts;
   else if(strcasecmp(s, stat_nbrcnt_str) == 0) t = stat_nbrcnt;
   else if(strcasecmp(s, stat_isc_str)    == 0) t = stat_isc;
   else if(strcasecmp(s, stat_wdir_str)   == 0) t = stat_wdir;
   else if(strcasecmp(s, stat_rhist_str)  == 0) t = stat_rhist;
   else if(strcasecmp(s, stat_orank_str)  == 0) t = stat_orank;
   else if(strcasecmp(s, stat_ssvar_str)  == 0) t = stat_ssvar;
   else                                         t = no_stat_line_type;

   return(t);
}

////////////////////////////////////////////////////////////////////////

FieldType int_to_fieldtype(int v) {
   FieldType t;

   // Convert integer to enumerated FieldType
        if(v == conf_const.lookup_int(conf_val_none)) t = FieldType_None;
   else if(v == conf_const.lookup_int(conf_val_both)) t = FieldType_Both;
   else if(v == conf_const.lookup_int(conf_val_fcst)) t = FieldType_Fcst;
   else if(v == conf_const.lookup_int(conf_val_obs))  t = FieldType_Obs;
   else {
      mlog << Error << "\nint_to_fieldtype() -> "
           << "Unexpected value of " << v << ".\n\n";
      exit(1);
   }

   return(t);
}

////////////////////////////////////////////////////////////////////////

ConcatString fieldtype_to_string(FieldType type) {
   ConcatString s;

   // Convert enumerated FieldType to string
   switch(type) {
      case(FieldType_None): s = conf_val_none; break;
      case(FieldType_Both): s = conf_val_both; break;
      case(FieldType_Fcst): s = conf_val_fcst; break;
      case(FieldType_Obs):  s = conf_val_obs; break;
      default:
         mlog << Error << "\nfieldtype_to_string() -> "
              << "Unexpected FieldType value of " << type << ".\n\n";
         exit(1);
         break;
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

MergeType int_to_mergetype(int v) {
   MergeType t;

   // Convert integer to enumerated MergeType
        if(v == conf_const.lookup_int(conf_val_none))   t = MergeType_None;
   else if(v == conf_const.lookup_int(conf_val_both))   t = MergeType_Both;
   else if(v == conf_const.lookup_int(conf_val_thresh)) t = MergeType_Thresh;
   else if(v == conf_const.lookup_int(conf_val_engine)) t = MergeType_Engine;
   else {
      mlog << Error << "\nint_to_mergetype() -> "
           << "Unexpected value of " << v << ".\n\n";
      exit(1);
   }

   return(t);
}

////////////////////////////////////////////////////////////////////////

ConcatString mergetype_to_string(MergeType type) {
   ConcatString s;

   // Convert enumerated MergeType to string
   switch(type) {
      case(MergeType_None):   s = conf_val_none;   break;
      case(MergeType_Both):   s = conf_val_both;   break;
      case(MergeType_Thresh): s = conf_val_thresh; break;
      case(MergeType_Engine): s = conf_val_engine; break;
      default:
         mlog << Error << "\nmergetype_to_string() -> "
              << "Unexpected MergeType value of " << type << ".\n\n";
         exit(1);
         break;
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

MatchType int_to_matchtype(int v) {
   MatchType t;

   // Convert integer to enumerated MatchType
        if(v == conf_const.lookup_int(conf_val_none))       t = MatchType_None;
   else if(v == conf_const.lookup_int(conf_val_merge_both)) t = MatchType_MergeBoth;
   else if(v == conf_const.lookup_int(conf_val_merge_fcst)) t = MatchType_MergeFcst;
   else if(v == conf_const.lookup_int(conf_val_no_merge))   t = MatchType_NoMerge;
   else {
      mlog << Error << "\nint_to_matchtype() -> "
           << "Unexpected value of " << v << ".\n\n";
      exit(1);
   }

   return(t);
}

////////////////////////////////////////////////////////////////////////

ConcatString matchtype_to_string(MatchType type) {
   ConcatString s;

   // Convert enumerated MatchType to string
   switch(type) {
      case(MatchType_None):      s = conf_val_none;       break;
      case(MatchType_MergeBoth): s = conf_val_merge_both; break;
      case(MatchType_MergeFcst): s = conf_val_merge_fcst; break;
      case(MatchType_NoMerge):   s = conf_val_no_merge;   break;
      default:
         mlog << Error << "\nmatchtype_to_string() -> "
              << "Unexpected MatchType value of " << type << ".\n\n";
         exit(1);
         break;
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString griddecomptype_to_string(GridDecompType type) {
   ConcatString s;

   // Convert enumerated GridDecompType to string
   switch(type) {
      case(GridDecompType_None): s = conf_val_none; break;
      case(GridDecompType_Auto): s = conf_val_auto; break;
      case(GridDecompType_Tile): s = conf_val_tile; break;
      case(GridDecompType_Pad):  s = conf_val_pad; break;
      default:
         mlog << Error << "\ngriddecomptype_to_string() -> "
              << "Unexpected GridDecompType value of " << type << ".\n\n";
         exit(1);
         break;
   }
   
   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString wavelettype_to_string(WaveletType type) {
   ConcatString s;

   // Convert enumerated WaveletType to string
   switch(type) {
      case(WaveletType_None):          s = conf_val_none; break;
      case(WaveletType_Haar):          s = conf_val_haar; break;
      case(WaveletType_Haar_Cntr):     s = conf_val_haar_cntr; break;
      case(WaveletType_Daub):          s = conf_val_daub; break;
      case(WaveletType_Daub_Cntr):     s = conf_val_daub_cntr; break;
      case(WaveletType_BSpline):       s = conf_val_bspline; break;
      case(WaveletType_BSpline_Cntr):  s = conf_val_bspline_cntr; break;
      default:
         mlog << Error << "\nwavlettype_to_string() -> "
              << "Unexpected WaveletType value of " << type << ".\n\n";
         exit(1);
         break;
   }
   
   return(s);
}

////////////////////////////////////////////////////////////////////////
