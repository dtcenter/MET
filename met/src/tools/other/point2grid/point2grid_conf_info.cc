// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include "point2grid_conf_info.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class PointToGridConfInfo
//
////////////////////////////////////////////////////////////////////////

PointToGridConfInfo::PointToGridConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PointToGridConfInfo::~PointToGridConfInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void PointToGridConfInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PointToGridConfInfo::clear() {

   // Initialize values
   message_type.clear();
   beg_ds = end_ds = bad_data_int;
   quality_mark_thresh = bad_data_int;
   version.clear();
   valid_time = 0;
   //def_var_name_map.clear();
   var_name_map.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PointToGridConfInfo::read_config(const char *default_file_name,
                                      const char *user_file_name) {

   // Read the config file constants
   conf.read(replace_path(config_const_filename).c_str());

   // Read the default config file
   conf.read(default_file_name);

   // Read the user-specified config file
   if (strcmp(user_file_name, default_file_name)) {
      def_var_name_map.clear();
      def_var_name_map = parse_conf_key_value_map(
                            &conf, conf_key_var_name_map);
      conf.read(user_file_name);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PointToGridConfInfo::process_config() {
   ConcatString s;
   StringArray sa;
   Dictionary *dict = (Dictionary *) 0;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Initialize
   clear();

   // Conf: version
   version = parse_conf_version(&conf);

   // Conf: message_type
   message_type = conf.lookup_string_array(conf_key_message_type);

   // Set valid time, if present
   s = conf.lookup_string(conf_key_valid_time, false);
   if (!s.empty()) valid_time = timestring_to_unix(s.c_str());

   // Conf: beg_ds and end_ds
   dict = conf.lookup_dictionary(conf_key_obs_window);
   parse_conf_range_int(dict, beg_ds, end_ds);

   // Conf: var_name_map
   var_name_map = parse_conf_key_value_map(&conf, conf_key_var_name_map);

   // Conf: quality_mark_thresh
   quality_mark_thresh = conf.lookup_int(conf_key_quality_mark_thresh);

   // Check the value
   if(quality_mark_thresh < 0 || quality_mark_thresh > 15) {
      mlog << Warning << "\nPointToGridConfInfo::process_config() -> "
           << "the \"" << conf_key_quality_mark_thresh
           << "\" entry (" << quality_mark_thresh
           << ") should be set between 0 and 15.\n\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString PointToGridConfInfo::get_var_name(const ConcatString var_name) {
   ConcatString out_var;
   ConcatString t_name;
   
   t_name = var_name_map[var_name];
   if (t_name.empty()) t_name = def_var_name_map[var_name];
   if (t_name.empty()) {
      ConcatString tmp_key, tmp_value;
      tmp_key = "grib_code_";
      tmp_key << atoi(var_name.c_str());
      tmp_value = var_name_map[tmp_key];
      if (tmp_value.empty()) tmp_value = def_var_name_map[tmp_key];
      if (!tmp_value.empty()) t_name = tmp_value;
   }
   out_var = t_name.empty() ? var_name : t_name;
   return out_var;
}

////////////////////////////////////////////////////////////////////////
