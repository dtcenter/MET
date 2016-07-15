// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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

// MetConfig object containing config value constants
static MetConfig conf_const(replace_path(config_const_filename));

////////////////////////////////////////////////////////////////////////

void RegridInfo::clear() {
   enable = false;
   field = FieldType_None;
   vld_thresh = bad_data_double;
   name.clear();
   method = InterpMthd_None;
   width = bad_data_int;
}

////////////////////////////////////////////////////////////////////////

RegridInfo::RegridInfo() {
   clear();
}

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

ConcatString parse_conf_string(Dictionary *dict, const char *conf_key) {
   ConcatString s;

   if(!dict) {
      mlog << Error << "\nparse_conf_string() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   s = dict->lookup_string(conf_key);

   if(dict->last_lookup_status()) {

      // Check that it's non-empty and contains no whitespace
      if(s.empty() || check_reg_exp(ws_reg_exp, s) == true) {
         mlog << Error << "\nparse_conf_string() -> "
              << "The \"" << conf_key << "\" entry (\"" << s
              << "\") must be non-empty and contain no embedded whitespace.\n\n";
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
      else if(v == conf_const.lookup_int(conf_val_netcdf_nccf)) t = FileType_NcCF;
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
   StringArray lvl;

   if(!dict) return(0);

   // Check that this dictionary is an array
   if(!dict->is_array()) {
      mlog << Error << "\nparse_conf_n_vx() -> "
           << "This function must be passed a Dictionary array.\n\n";
      exit(1);
   }

   // Loop over the fields to be verified
   for(i=0,total=0; i<dict->n_entries(); i++) {

      // Get the level array, which may or may not be defined.
      // If defined, use its length.  If not, use a length of 1.
      lvl = (*dict)[i]->dict_value()->lookup_string_array(conf_key_level, false);

      // Increment count by the length of the level array
      total += (lvl.n_elements() > 0 ? lvl.n_elements() : 1);
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
   int i, total, n_lvl;

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

      // Get the level array, which may or may not be defined.
      // If defined, use its length.  If not, use a length of 1.
      lvl    = (*dict)[i]->dict_value()->lookup_string_array(conf_key_level, false);
      n_lvl  = (lvl.n_elements() > 0 ? lvl.n_elements() : 1);
      total += n_lvl;

      // Check if we're in the correct entry
      if(total > index) {

         // Copy the current entry's dictionary
         i_dict = *((*dict)[i]->dict_value());

         // Set up the new entry, taking only a single level value
         if(lvl.n_elements() > 0) {
            entry.set_string(conf_key_level, lvl[index-(total-n_lvl)]);
            i_dict.store(entry);
         }

         break;
      }
   } // end for i

   return(i_dict);
}

////////////////////////////////////////////////////////////////////////

StringArray parse_conf_message_type(Dictionary *dict) {
   StringArray sa;

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

StringArray parse_conf_sid_exc(Dictionary *dict) {
   StringArray sa, cur, sid_exc_sa;
   ConcatString mask_name;
   int i;

   if(!dict) {
      mlog << Error << "\nparse_conf_sid_exc() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   sa = dict->lookup_string_array(conf_key_sid_exc);

   // Parse station ID's to exclude from each entry
   for(i=0; i<sa.n_elements(); i++) {
      parse_sid_mask(sa[i], cur, mask_name);
      sid_exc_sa.add(cur);
   }

   mlog << Debug(4) << "parse_conf_sid_exc() -> "
        << "Station ID exclusion list contains "
        << sid_exc_sa.n_elements() << " entries.\n";

   return(sid_exc_sa);
}

///////////////////////////////////////////////////////////////////////////////
//
// This function is passed a string containing either a single station ID or
// a file name containing a list of station ID's.  For a single station ID,
// store it in the output StringArray, and store it's value as the mask name.
// For a file name, parse the list of stations into the output StringArray,
// and store the first entry in the file as the mask name.
//
///////////////////////////////////////////////////////////////////////////////

void parse_sid_mask(const ConcatString &mask_sid_str,
                    StringArray &mask_sid, ConcatString &mask_name) {
   ifstream in;
   ConcatString tmp_file;
   char sid_str[PATH_MAX];

   // Initialize
   mask_sid.clear();
   mask_name = na_str;

   // Check for an empty length string
   if(mask_sid_str.empty()) return;

   // Replace any instances of MET_BASE with it's expanded value
   tmp_file = replace_path(mask_sid_str);

   // Process string as a file name, if possible
   if(file_exists(tmp_file)) {

      mlog << Debug(4) << "parse_sid_mask() -> "
           << "parsing station ID masking file \"" << tmp_file << "\"\n";

      // Open the mask station id file specified
      in.open(tmp_file);

      if(!in) {
         mlog << Error << "\nparse_sid_mask() -> "
              << "Can't open the station ID masking file \""
              << tmp_file << "\".\n\n";
         exit(1);
      }

      // Store the first entry as the name of the mask
      in >> sid_str;
      mask_name = sid_str;

      // Store the rest of the entries as masking station ID's
      while(in >> sid_str) mask_sid.add(sid_str);

      // Close the input file
      in.close();

      mlog << Debug(4) << "parse_sid_mask() -> "
           << "parsed " << mask_sid.n_elements() << " station ID's for the \""
           << mask_name << "\" mask from file \"" << tmp_file << "\"\n";
   }
   // Otherwise, process it as a single station ID
   else {

      mlog << Debug(4) << "parse_sid_mask() -> "
           << "storing single station ID mask \"" << mask_sid_str << "\"\n";

      // Check for embedded whitespace or slashes
      if(check_reg_exp(ws_reg_exp, mask_sid_str) ||
         check_reg_exp("[/]", mask_sid_str)) {
         mlog << Error << "\nparse_sid_mask() -> "
              << "masking station ID name can't contain whitespace or slashes \""
              << mask_sid_str << "\".\n\n";
         exit(1);
      }

      // Store the single station ID
      mask_sid.add(mask_sid_str);
      mask_name = mask_sid_str;
   }

   return;
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
   Dictionary *ts_dict = (Dictionary *) 0;
   TimeSummaryInfo info;

   if(!dict) {
      mlog << Error << "\nparse_conf_time_summary() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: time_summary
   ts_dict = dict->lookup_dictionary(conf_key_time_summary);

   // Conf: flag
   info.flag = ts_dict->lookup_bool(conf_key_flag);

   // Conf: beg
   info.beg = timestring_to_sec(ts_dict->lookup_string(conf_key_beg));

   // Conf: end
   info.end = timestring_to_sec(ts_dict->lookup_string(conf_key_end));

   // Conf: step
   info.step = ts_dict->lookup_int(conf_key_step);
   if(info.step <= 0) {
      mlog << Error << "\nparse_conf_time_summary() -> "
           << "The \"" << conf_key_step << "\" parameter (" << info.step
           << ") must be greater than 0!\n\n";
      exit(1);
   }

   // Conf: width
   info.width = ts_dict->lookup_int(conf_key_width);
   if(info.width <= 0) {
     mlog << Error << "\nparse_conf_time_summary() -> "
          << "The \"" << conf_key_width << "\" parameter (" << info.width
          << ") must be greater than 0!\n\n";
     exit(1);
   }

   // Conf: grib_code
   info.grib_code = ts_dict->lookup_int_array(conf_key_grib_code);

   // Conf: type
   info.type = ts_dict->lookup_string_array(conf_key_type);

   // Conf: vld_freq
   info.vld_freq = ts_dict->lookup_int(conf_key_vld_freq);

   // Conf: vld_thresh
   info.vld_thresh = ts_dict->lookup_double(conf_key_vld_thresh);

   // Check that the interpolation threshold is between 0 and 1.
   if(info.vld_thresh < 0.0 || info.vld_thresh > 1.0) {
      mlog << Error << "\nparse_conf_time_summary() -> "
           << "The \"" << conf_key_time_summary << "."
           << conf_key_vld_thresh << "\" parameter (" << info.vld_thresh
           << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   return(info);
}

////////////////////////////////////////////////////////////////////////

map<ConcatString,ConcatString> parse_conf_message_type_map(Dictionary *dict) {
   Dictionary *msg_typ_dict = (Dictionary *) 0;
   map<ConcatString,ConcatString> m;
   ConcatString key, val;
   int i;

   if(!dict) {
      mlog << Error << "\nparse_conf_message_type_map() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: message_type_map
   msg_typ_dict = dict->lookup_array(conf_key_message_type_map);

   // Loop through the array entries
   for(i=0; i<msg_typ_dict->n_entries(); i++) {

      // Lookup the key and value
      key = (*msg_typ_dict)[i]->dict_value()->lookup_string(conf_key_key);
      val = (*msg_typ_dict)[i]->dict_value()->lookup_string(conf_key_val);

      if(m.count(key) >= 1) {
         mlog << Warning << "\nparse_conf_message_type_map() -> "
              << "found multiple entries for key \""
              << key << "\"!\n\n";
      }

      // Add entry to the map
      m.insert(pair<ConcatString, ConcatString>(key, val));
   }

   return(m);
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

RegridInfo parse_conf_regrid(Dictionary *dict) {
   Dictionary *regrid_dict = (Dictionary *) 0;
   RegridInfo info;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_regrid() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: regrid
   regrid_dict = dict->lookup_dictionary(conf_key_regrid);

   // Parse to_grid as an integer
   v = regrid_dict->lookup_int(conf_key_to_grid, false);

   // If integer lookup successful, convert to FieldType.
   if(regrid_dict->last_lookup_status()) {
      info.field  = int_to_fieldtype(v);
      info.enable = (info.field == FieldType_Fcst ||
                     info.field == FieldType_Obs);
   }
   // If integer lookup unsuccessful, parse vx_grid as a string.
   // Do not error out since to_grid isn't specified for climo.regrid.
   else {
      info.name   = regrid_dict->lookup_string(conf_key_to_grid, false);
      info.enable = true;
   }

   // Parse valid data threshold, method, and width
   info.vld_thresh = regrid_dict->lookup_double(conf_key_vld_thresh);
   info.method     = int_to_interpmthd(regrid_dict->lookup_int(conf_key_method));
   info.width      = regrid_dict->lookup_int(conf_key_width);

   // Check the nearest neighbor special case
   if(info.width  == 1 &&
      info.method != InterpMthd_Nearest &&
      info.method != InterpMthd_Force) {
      mlog << Warning << "\nparse_conf_regrid() -> "
           << "Resetting the regridding method from \""
           << interpmthd_to_string(info.method) << "\" to \""
           << interpmthd_nearest_str
           << "\" since the regridding width is 1.\n\n";
      info.method = InterpMthd_Nearest;
   }

   // Check the bilinear and budget special cases
   if((info.method == InterpMthd_Bilin ||
       info.method == InterpMthd_Budget) &&
      info.width != 2) {
      mlog << Warning << "\nparse_conf_regrid() -> "
           << "Resetting the regridding width from "
           << info.width << " to 2 for regridding method \""
           << interpmthd_to_string(info.method) << "\".\n\n";
      info.width = 2;
   }

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
         method = interpmthd_to_string(int_to_interpmthd(mthd_na[j]));

         // Check for budget interpolation
         if(strcmp(method, interpmthd_budget_str) == 0) {
            mlog << Error << "\nparse_conf_interp() -> "
                 << "\"" << interpmthd_budget_str
                 << "\" not valid for interpolating, only regridding.\n\n";
            exit(1);
         }

         // Loop over the widths
         for(k=0; k<wdth_na.n_elements(); k++) {

            // Store the current width
            width = nint(wdth_na[k]);

            // Check for the nearest neighbor special case
            if(width == 1 && strcmp(method, interpmthd_nearest_str) != 0) {
               mlog << Warning << "\nparse_conf_interp() -> "
                    << "For neareast neighbor interpolation method, "
                    << "resetting method from \"" << method << "\" to \""
                    << interpmthd_nearest_str
                    << "\" since the interpolation width is 1.\n\n";
               method = interpmthd_nearest_str;
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

   // Check for at least one interpolation method
   if(info.n_interp == 0) {
      mlog << Error << "\nparse_conf_interp() -> "
           << "Must define at least one interpolation method in the config file.\n\n";
      exit(1);
   }

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

      if(info.cov_ta[i].get_value() < 0.0 ||
         info.cov_ta[i].get_value() > 1.0) {
         mlog << Error << "\nparse_conf_nbrhd() -> "
              << "The neighborhood coverage threshold value must be set "
              << "between 0 and 1.\n\n";
         exit(1);
      }
   }

   return(info);
}

////////////////////////////////////////////////////////////////////////

GridWeightType parse_conf_grid_weight_flag(Dictionary *dict) {
   GridWeightType t;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_grid_weight_flag() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Get the integer flag value for the current entry
   v = dict->lookup_int(conf_key_grid_weight_flag);

   // Convert integer to enumerated GridWeightType
        if(v == conf_const.lookup_int(conf_val_none))    t = GridWeightType_None;
   else if(v == conf_const.lookup_int(conf_val_cos_lat)) t = GridWeightType_Cos_Lat;
   else if(v == conf_const.lookup_int(conf_val_area))    t = GridWeightType_Area;
   else {
      mlog << Error << "\nparse_conf_grid_weight_flag() -> "
           << "Unexpected config file value of " << v << " for \""
           << conf_key_grid_weight_flag << "\".\n\n";
      exit(1);
   }

   return(t);
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

void check_climo_n_vx(Dictionary *dict, const int n_vx) {
   int n;

   // Check for a valid number of climatology mean fields
   n = parse_conf_n_vx(dict->lookup_array(conf_key_climo_mean_field, false));
   if(n != 0 && n != n_vx) {
      mlog << Error << "\ncheck_climo_n_vx() -> "
           << "The number of climatology mean fields in \""
           << conf_key_climo_mean_field
           << "\" must be zero or match the number (" << n_vx
           << ") in \"" << conf_key_fcst_field << "\".\n\n";
      exit(1);
   }

   // Check for a valid number of climatology standard deviation fields
   n = parse_conf_n_vx(dict->lookup_array(conf_key_climo_stdev_field, false));
   if(n != 0 && n != n_vx) {
      mlog << Error << "\ncheck_climo_n_vx() -> "
           << "The number of climatology standard deviation fields in \""
           << conf_key_climo_stdev_field
           << "\" must be zero or match the number ("
           << n_vx << ") in \"" << conf_key_fcst_field << "\".\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

InterpMthd int_to_interpmthd(int i) {
   InterpMthd m;

        if(i == conf_const.lookup_int(interpmthd_min_str))     m = InterpMthd_Min;
   else if(i == conf_const.lookup_int(interpmthd_max_str))     m = InterpMthd_Max;
   else if(i == conf_const.lookup_int(interpmthd_median_str))  m = InterpMthd_Median;
   else if(i == conf_const.lookup_int(interpmthd_uw_mean_str)) m = InterpMthd_UW_Mean;
   else if(i == conf_const.lookup_int(interpmthd_dw_mean_str)) m = InterpMthd_DW_Mean;
   else if(i == conf_const.lookup_int(interpmthd_ls_fit_str))  m = InterpMthd_LS_Fit;
   else if(i == conf_const.lookup_int(interpmthd_bilin_str))   m = InterpMthd_Bilin;
   else if(i == conf_const.lookup_int(interpmthd_nbrhd_str))   m = InterpMthd_Nbrhd;
   else if(i == conf_const.lookup_int(interpmthd_nearest_str)) m = InterpMthd_Nearest;
   else if(i == conf_const.lookup_int(interpmthd_budget_str))  m = InterpMthd_Budget;
   else if(i == conf_const.lookup_int(interpmthd_force_str))   m = InterpMthd_Force;
   else {
      mlog << Error << "\nconf_int_to_interpmthd() -> "
           << "Unexpected value of " << i
           << " for \"" << conf_key_method << "\".\n\n";
      exit(1);
   }

   return(m);
}

////////////////////////////////////////////////////////////////////////

void check_prob_thresh(const ThreshArray &ta) {
   int i, n;

   n = ta.n_elements();

   // Check for at least 3 thresholds beginning with 0 and ending with 1.
   if(n < 3 ||
      !is_eq(ta[0].get_value(),   0.0) ||
      !is_eq(ta[n-1].get_value(), 1.0)) {

      mlog << Error << "\ncheck_prob_thresh() -> "
           << "When verifying a probability field, you must "
           << "select at least 3 thresholds beginning with 0.0 "
           << "and ending with 1.0.\n\n";
      exit(1);
   }

   for(i=0; i<n; i++) {

      // Check that all threshold types are greater than or equal to
      if(ta[i].get_type() != thresh_ge) {
         mlog << Error << "\ncheck_prob_thresh() -> "
              << "When verifying a probability field, all "
              << "thresholds must be set as equal to, "
              << "using \"ge\" or \">=\".\n\n";
         exit(1);
      }

      // Check that all thresholds are in [0, 1].
      if(ta[i].get_value() < 0.0 ||
         ta[i].get_value() > 1.0) {

         mlog << Error << "\ncheck_prob_thresh() -> "
              << "When verifying a probability field, all "
              << "thresholds must be between 0 and 1.\n\n";
         exit(1);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void check_mctc_thresh(const ThreshArray &ta) {
   int i;

   // Check that the threshold values are monotonically increasing
   // and the threshold types are inequalities that remain the same
   for(i=0; i<ta.n_elements()-1; i++) {

      if(ta[i].get_value() >  ta[i+1].get_value() ||
         ta[i].get_type()  != ta[i+1].get_type()  ||
        (ta[i].get_type()  != thresh_lt           &&
         ta[i].get_type()  != thresh_le           &&
         ta[i].get_type()  != thresh_gt           &&
         ta[i].get_type()  != thresh_ge)) {
         mlog << Error << "\ncheck_mctc_thresh() -> "
              << "when verifying using multi-category contingency "
              << "tables, the thresholds must be monotonically "
              << "increasing and be of the same inequality type "
              << "(lt, le, gt, or ge).\n\n";
         exit(1);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool check_fo_thresh(const double f, const SingleThresh &ft,
                     const double o, const SingleThresh &ot,
                     const SetLogic type) {
   bool status = true;
   bool fcheck = ft.check(f);
   bool ocheck = ot.check(o);
   SetLogic t  = type;

   // If either of the thresholds is NA, reset the logic to intersection
   // because an NA threshold is always true.
   if(ft.get_type() == thresh_na || ot.get_type() == thresh_na) {
      t = SetLogic_Intersection;
   }

   switch(t) {
      case(SetLogic_Union):
         if(!fcheck && !ocheck) status = false;
         break;

      case(SetLogic_Intersection):
         if(!fcheck || !ocheck) status = false;
         break;

      case(SetLogic_SymDiff):
         if(fcheck == ocheck) status = false;
         break;

      default:
         mlog << Error << "\ncheck_fo_thresh() -> "
              << "Unexpected SetLogic value of " << type << ".\n\n";
         exit(1);
         break;
   }

   return(status);
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
      case(stat_phist):        s = stat_phist_str;   break;
      case(stat_orank):        s = stat_orank_str;   break;
      case(stat_ssvar):        s = stat_ssvar_str;   break;
      case(stat_header):       s = stat_header_str;  break;
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
   else if(strcasecmp(s, stat_phist_str)  == 0) t = stat_phist;
   else if(strcasecmp(s, stat_orank_str)  == 0) t = stat_orank;
   else if(strcasecmp(s, stat_ssvar_str)  == 0) t = stat_ssvar;
   else if(strcasecmp(s, stat_header_str) == 0) t = stat_header;
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

SetLogic int_to_setlogic(int v) {
   SetLogic t;

   // Convert integer to enumerated SetLogic
        if(v == conf_const.lookup_int(conf_val_none))         t = SetLogic_None;
   else if(v == conf_const.lookup_int(conf_val_union))        t = SetLogic_Union;
   else if(v == conf_const.lookup_int(conf_val_intersection)) t = SetLogic_Intersection;
   else if(v == conf_const.lookup_int(conf_val_symdiff))      t = SetLogic_SymDiff;
   else {
      mlog << Error << "\nint_to_setlogic() -> "
           << "Unexpected value of " << v << ".\n\n";
      exit(1);
   }

   return(t);
}

////////////////////////////////////////////////////////////////////////

SetLogic string_to_setlogic(const char *s) {
   SetLogic t;

   // Convert string to enumerated SetLogic
        if(strcasecmp(s, conf_val_none)                == 0) t = SetLogic_None;

   else if(strcasecmp(s, conf_val_union)               == 0 ||
           strcasecmp(s, setlogic_abbr_union)          == 0 ||
           strcasecmp(s, setlogic_symbol_union)        == 0) t = SetLogic_Union;

   else if(strcasecmp(s, conf_val_intersection)        == 0 ||
           strcasecmp(s, setlogic_abbr_intersection)   == 0 ||
           strcasecmp(s, setlogic_symbol_intersection) == 0) t = SetLogic_Intersection;

   else if(strcasecmp(s, conf_val_symdiff)             == 0 ||
           strcasecmp(s, setlogic_abbr_symdiff)        == 0 ||
           strcasecmp(s, setlogic_symbol_symdiff)      == 0) t = SetLogic_SymDiff;

   else {
      mlog << Error << "\nstring_to_setlogic() -> "
           << "Unexpected SetLogic string \"" << s << "\".\n\n";
      exit(1);
   }

   return(t);
}


////////////////////////////////////////////////////////////////////////

ConcatString setlogic_to_string(SetLogic type) {
   ConcatString s;

   // Convert enumerated SetLogic to string
   switch(type) {
      case(SetLogic_None):         s = conf_val_none;         break;
      case(SetLogic_Union):        s = conf_val_union;        break;
      case(SetLogic_Intersection): s = conf_val_intersection; break;
      case(SetLogic_SymDiff):      s = conf_val_symdiff;      break;
      default:
         mlog << Error << "\nsetlogic_to_string() -> "
              << "Unexpected SetLogic value of " << type << ".\n\n";
         exit(1);
         break;
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString setlogic_to_abbr(SetLogic type) {
   ConcatString s;

   // Convert enumerated SetLogic to an abbreviation
   switch(type) {
      case(SetLogic_None):         s = na_str;                     break;
      case(SetLogic_Union):        s = setlogic_abbr_union;        break;
      case(SetLogic_Intersection): s = setlogic_abbr_intersection; break;
      case(SetLogic_SymDiff):      s = setlogic_abbr_symdiff;      break;
      default:
         mlog << Error << "\nsetlogic_to_abbr() -> "
              << "Unexpected SetLogic value of " << type << ".\n\n";
         exit(1);
         break;
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString setlogic_to_symbol(SetLogic type) {
   ConcatString s;

   // Convert enumerated SetLogic to a symbol
   switch(type) {
      case(SetLogic_None):         s = na_str;                       break;
      case(SetLogic_Union):        s = setlogic_symbol_union;        break;
      case(SetLogic_Intersection): s = setlogic_symbol_intersection; break;
      case(SetLogic_SymDiff):      s = setlogic_symbol_symdiff;      break;
      default:
         mlog << Error << "\nsetlogic_to_symbol() -> "
              << "Unexpected SetLogic value of " << type << ".\n\n";
         exit(1);
         break;
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

SetLogic check_setlogic(SetLogic t1, SetLogic t2) {
   SetLogic t;

   // If not equal, select the non-default logic type
        if(t1 == t2)            t = t1;
   else if(t1 == SetLogic_None) t = t2;
   else if(t2 == SetLogic_None) t = t1;
   // If not equal and both non-default, error out
   else {
      mlog << Error << "\ncheck_setlogic() -> "
           << "The forecast and observed logic must be consistent: "
           << setlogic_to_string(t1) << " != " << setlogic_to_string(t2)
           << "\n\n";
      exit(1);
   }

   return(t);
}

////////////////////////////////////////////////////////////////////////

TrackType int_to_tracktype(int v) {
   TrackType t;

   // Convert integer to enumerated TrackType
        if(v == conf_const.lookup_int(conf_val_none))  t = TrackType_None;
   else if(v == conf_const.lookup_int(conf_val_both))  t = TrackType_Both;
   else if(v == conf_const.lookup_int(conf_val_adeck)) t = TrackType_ADeck;
   else if(v == conf_const.lookup_int(conf_val_bdeck)) t = TrackType_BDeck;
   else {
      mlog << Error << "\nint_to_tracktype() -> "
           << "Unexpected value of " << v << ".\n\n";
      exit(1);
   }

   return(t);
}

////////////////////////////////////////////////////////////////////////

TrackType string_to_tracktype(const char *s) {
   TrackType t;

   // Convert string to enumerated TrackType
        if(strcasecmp(s, conf_val_none)  == 0) t = TrackType_None;
   else if(strcasecmp(s, conf_val_both)  == 0) t = TrackType_Both;
   else if(strcasecmp(s, conf_val_adeck) == 0) t = TrackType_ADeck;
   else if(strcasecmp(s, conf_val_bdeck) == 0) t = TrackType_BDeck;
   else {
      mlog << Error << "\nstring_to_tracktype() -> "
           << "Unexpected TrackType string \"" << s << "\".\n\n";
      exit(1);
   }

   return(t);
}

////////////////////////////////////////////////////////////////////////

ConcatString tracktype_to_string(TrackType type) {
   ConcatString s;

   // Convert enumerated TrackType to string
   switch(type) {
      case(TrackType_None):  s = conf_val_none; break;
      case(TrackType_Both):  s = conf_val_both; break;
      case(TrackType_ADeck): s = conf_val_adeck; break;
      case(TrackType_BDeck): s = conf_val_bdeck; break;
      default:
         mlog << Error << "\ntracktype_to_string() -> "
              << "Unexpected TrackType value of " << type << ".\n\n";
         exit(1);
         break;
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

Interp12Type int_to_interp12type(int v) {
   Interp12Type t;

   // Convert integer to enumerated Interp12Type
        if(v == conf_const.lookup_int(conf_val_none))    t = Interp12Type_None;
   else if(v == conf_const.lookup_int(conf_val_fill))    t = Interp12Type_Fill;
   else if(v == conf_const.lookup_int(conf_val_replace)) t = Interp12Type_Replace;
   else {
      mlog << Error << "\nint_to_interp12type() -> "
           << "Unexpected value of " << v << ".\n\n";
      exit(1);
   }

   return(t);
}

////////////////////////////////////////////////////////////////////////

Interp12Type string_to_interp12type(const char *s) {
   Interp12Type t;

   // Convert string to enumerated Interp12Type
        if(strcasecmp(s, conf_val_none)    == 0) t = Interp12Type_None;
   else if(strcasecmp(s, conf_val_fill)    == 0) t = Interp12Type_Fill;
   else if(strcasecmp(s, conf_val_replace) == 0) t = Interp12Type_Replace;
   else {
      mlog << Error << "\nstring_to_interp12type() -> "
           << "Unexpected Interp12Type string \"" << s << "\".\n\n";
      exit(1);
   }

   return(t);
}

////////////////////////////////////////////////////////////////////////

ConcatString interp12type_to_string(Interp12Type type) {
   ConcatString s;

   // Convert enumerated Interp12Type to string
   switch(type) {
      case(Interp12Type_None):    s = conf_val_none;    break;
      case(Interp12Type_Fill):    s = conf_val_fill;    break;
      case(Interp12Type_Replace): s = conf_val_replace; break;
      default:
         mlog << Error << "\ninterp12type_to_string() -> "
              << "Unexpected Interp12Type value of " << type << ".\n\n";
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
