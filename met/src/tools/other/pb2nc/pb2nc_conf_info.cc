// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

#include "pb2nc_conf_info.h"

#include "vx_data2d_factory.h"
#include "apply_mask.h"
#include "grib_strings.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class PB2NCConfInfo
//
////////////////////////////////////////////////////////////////////////

PB2NCConfInfo::PB2NCConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PB2NCConfInfo::~PB2NCConfInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void PB2NCConfInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PB2NCConfInfo::clear() {

   // Initialize values
   message_type.clear();
   station_id.clear();
   beg_ds = end_ds = bad_data_int;
   grid_mask.clear();
   area_mask.clear();
   poly_mask.clear();
   beg_elev = end_elev = bad_data_double;
   pb_report_type.clear();
   in_report_type.clear();
   instrument_type.clear();
   beg_level = end_level = bad_data_double;
   level_category.clear();
   obs_bufr_var.clear();
   quality_mark_thresh = bad_data_int;
   event_stack_flag = false;
   tmp_dir.clear();
   version.clear();
   obs_bufr_map.clear();
   message_type_map.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PB2NCConfInfo::read_config(const char *default_file_name,
                                const char *user_file_name) {

   // Read the config file constants
   conf.read(replace_path(config_const_filename).c_str());

   // Read the default config file
   conf.read(default_file_name);

   // Read the user-specified config file
   conf.read(user_file_name);

   return;
}

////////////////////////////////////////////////////////////////////////

void PB2NCConfInfo::process_config() {
   int i;
   ConcatString s, mask_name;
   StringArray sa;
   StringArray * sid_list = 0;
   Dictionary *dict = (Dictionary *) 0;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Initialize
   clear();

   // Conf: version
   version = parse_conf_version(&conf);

   // Conf: message_type
   message_type = conf.lookup_string_array(conf_key_message_type);

   // Conf: message_type_group_map
   map<ConcatString,StringArray> group_map;
   group_map = parse_conf_message_type_group_map(&conf);

   // Expand the values for any message type group names
   for(i=0; i<message_type.n_elements(); i++) {
      if(group_map.count(message_type[i]) > 0) {
         message_type.add(group_map[message_type[i]]);
      }
   }

   // Parse surface_message_types from message_type_group_map
   if(group_map.count((string)surface_msg_typ_group_str) == 0) {
      mlog << Error << "\nPB2NCConfInfo::process_config() -> "
           << "\"" << conf_key_message_type_group_map
           << "\" must contain an entry for \""
           << surface_msg_typ_group_str << "\".\n\n";
      exit(1);
   }
   else {
     surface_message_types = group_map[(string)surface_msg_typ_group_str];
   }

   // Conf: station_id
   sa = conf.lookup_string_array(conf_key_station_id);
   sid_list = new StringArray [sa.n_elements()];
   for(i=0; i<sa.n_elements(); i++) {
      parse_sid_mask(replace_path(sa[i]), sid_list[i], mask_name);
      station_id.add(sid_list[i]);
   }

   // Conf: beg_ds and end_ds
   dict = conf.lookup_dictionary(conf_key_obs_window);
   parse_conf_range_int(dict, beg_ds, end_ds);

   // Conf: grid_mask
   s = conf.lookup_string(conf_key_mask_grid);
   parse_grid_mask(s, grid_mask);

   // Conf: poly_mask
   s = conf.lookup_string(conf_key_mask_poly);
   parse_poly_mask(s, poly_mask, grid_mask, area_mask, mask_name);

   // Conf: beg_elev and end_elev
   dict = conf.lookup_dictionary(conf_key_elevation_range);
   parse_conf_range_double(dict, beg_elev, end_elev);

   // Conf: pb_report_type
   pb_report_type = conf.lookup_num_array(conf_key_pb_report_type);

   // Check the values
   for(i=0; i<pb_report_type.n_elements(); i++) {
      if(pb_report_type[i] < 100 || pb_report_type[i] > 600) {
         mlog << Warning << "\nPB2NCConfInfo::process_config() -> "
              << "the \"" << conf_key_pb_report_type
              << "\" entries (" << pb_report_type[i]
              << ") should be set between 100 and 600.\n\n";
      }
   }

   // Conf: in_report_type
   in_report_type = conf.lookup_num_array(conf_key_in_report_type);

   // Conf: instrument_type
   instrument_type = conf.lookup_num_array(conf_key_instrument_type);

   // Conf: beg_level and end_level
   dict = conf.lookup_dictionary(conf_key_level_range);
   parse_conf_range_double(dict, beg_level, end_level);

   // Conf: level_category
   level_category = conf.lookup_num_array(conf_key_level_category);

   // Check the values
   for(i=0; i<level_category.n_elements(); i++) {
      if(level_category[i] < 0 || level_category[i] > 7) {
         mlog << Warning << "\nPB2NCConfInfo::process_config() -> "
              << "the \"" << conf_key_level_category
              << "\" entries (" << level_category[i]
              << ") should be set between 0 and 7.\n\n";
      }
   }

   // Conf: time_summary
   timeSummaryInfo = parse_conf_time_summary(&conf);

   // Conf: obs_bufr_var
   sa = conf.lookup_string_array(conf_key_obs_bufr_var, false);
   for(i=0; i<sa.n_elements(); i++) obs_bufr_var.add(sa[i]);

   // Conf: quality_mark_thresh
   quality_mark_thresh = conf.lookup_int(conf_key_quality_mark_thresh);

   // Check the value
   if(quality_mark_thresh < 0 || quality_mark_thresh > 15) {
      mlog << Warning << "\nPB2NCConfInfo::process_config() -> "
           << "the \"" << conf_key_quality_mark_thresh
           << "\" entry (" << quality_mark_thresh
           << ") should be set between 0 and 15.\n\n";
   }

   // Conf: event_stack_flag
   event_stack_flag = conf.lookup_bool(conf_key_event_stack_flag);

   // Conf: tmp_dir
   tmp_dir = parse_conf_tmp_dir(&conf);

   obs_bufr_map = parse_conf_obs_bufr_map(&conf);
   message_type_map = parse_conf_message_type_map(&conf);

   if ( sid_list )  { delete [] sid_list;   sid_list = (StringArray *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////
