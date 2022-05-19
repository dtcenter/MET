// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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

#include "ioda2nc_conf_info.h"

#include "vx_data2d_factory.h"
#include "apply_mask.h"
#include "grib_strings.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class IODA2NCConfInfo
//
////////////////////////////////////////////////////////////////////////

IODA2NCConfInfo::IODA2NCConfInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

IODA2NCConfInfo::~IODA2NCConfInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void IODA2NCConfInfo::init_from_scratch() {
   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void IODA2NCConfInfo::clear() {

   // Initialize values
   message_type.clear();
   station_id.clear();
   valid_beg_ut = valid_end_ut = 0;
   beg_ds = end_ds = bad_data_int;
   grid_mask.clear();
   area_mask.clear();
   poly_mask.clear();
   beg_elev = end_elev = bad_data_double;
   beg_level = end_level = bad_data_double;
   quality_mark_thresh = bad_data_int;
   version.clear();
   obs_name_map.clear();
   message_type_map.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void IODA2NCConfInfo::read_config(const char *default_file_name,
                                const char *user_file_name) {

   // Read the config file constants
   conf.read(replace_path(config_const_filename).c_str());

   // Read the default config file
   conf.read(default_file_name);

   // Read the user-specified config file
   if (user_file_name && strcmp(user_file_name, "")) conf.read(user_file_name);

   return;
}

////////////////////////////////////////////////////////////////////////

void IODA2NCConfInfo::process_config() {
   int i;
   ConcatString s, mask_name;
   StringArray sa;
   StringArray * sid_list = 0;
   Dictionary *dict = (Dictionary *) 0;
   static const char *method_name = "IODA2NCConfInfo::process_config() -> ";

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

   // Conf: beg_level and end_level
   dict = conf.lookup_dictionary(conf_key_level_range);
   parse_conf_range_double(dict, beg_level, end_level);

   // Conf: time_summary
   timeSummaryInfo = parse_conf_time_summary(&conf);

   // Conf: quality_mark_thresh
   quality_mark_thresh = conf.lookup_int(conf_key_quality_mark_thresh);

   // Conf: missing_thresh
   missing_thresh = conf.lookup_thresh_array(conf_key_missing_thresh, false);

   // Check the value
   if(quality_mark_thresh < 0 || quality_mark_thresh > 15) {
      mlog << Warning << "\nIODA2NCConfInfo::process_config() -> "
           << "the \"" << conf_key_quality_mark_thresh
           << "\" entry (" << quality_mark_thresh
           << ") should be set between 0 and 15.\n\n";
   }

   // Conf: obs_name_map
   obs_name_map = parse_conf_obs_name_map(&conf);
   message_type_map = parse_conf_message_type_map(&conf);
   metadata_map = parse_conf_metadata_map(&conf);

   if ( sid_list ) delete [] sid_list;

   return;
}

////////////////////////////////////////////////////////////////////////
