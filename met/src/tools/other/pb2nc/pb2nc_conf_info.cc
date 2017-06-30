// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
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
   poly_mask.clear();
   beg_elev = end_elev = bad_data_double;
   pb_report_type.clear();
   in_report_type.clear();
   instrument_type.clear();
   beg_level = end_level = bad_data_double;
   level_category.clear();
   obs_grib_code.clear();
   bufr_var_name.clear();
   quality_mark_thresh = bad_data_int;
   event_stack_flag = false;
   tmp_dir.clear();
   version.clear();
   obs_var_map.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PB2NCConfInfo::read_config(const char *default_file_name,
                                const char *user_file_name) {

   // Read the config file constants
   conf.read(replace_path(config_const_filename));

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
   StringArray sa, *sid_list;
   Dictionary *dict = (Dictionary *) 0;

   
   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Initialize
   clear();

   // Conf: version
   version = parse_conf_version(&conf);
   
   // Conf: message_type
   message_type = conf.lookup_string_array(conf_key_message_type);

   // Check specific message types
   for(i=0; i<message_type.n_elements(); i++) {
           if(strcmp(message_type[i], anyair_str) == 0) anyair_flag = true;
      else if(strcmp(message_type[i], anysfc_str) == 0) anysfc_flag = true;
      else if(strcmp(message_type[i], onlysf_str) == 0) onlysf_flag = true;
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

   // Parse the masking grid
   if(s.nonempty()) {
      if(!find_grid_by_name(s, grid_mask)) {
         Met2dDataFileFactory factory;
         Met2dDataFile * datafile = (Met2dDataFile *) 0;
         // If that doesn't work, try to open a data file.
         datafile = factory.new_met_2d_data_file(replace_path(s));
         
         if(!datafile) {
            mlog << Error << "\nPB2NCConfInfo::process_config() -> "
                 << "the \"" << conf_key_mask_grid << "\" requested ("
                 << s << ") as named grid or data file is not defined.\n\n";
            exit(1);
         }
         
         // Store the data file's grid
         grid_mask = datafile->grid();
      }
   }

   // Conf: poly_mask
   s = conf.lookup_string(conf_key_mask_poly);

   // Parse the masking polyline
   if(s.nonempty()) poly_mask.load(replace_path(s));

   // Conf: beg_elev and end_elev
   dict = conf.lookup_dictionary(conf_key_elevation_range);
   parse_conf_range_double(dict, beg_elev, end_elev);

   // Conf: pb_report_type
   pb_report_type = conf.lookup_num_array(conf_key_pb_report_type);

   // Check the values
   for(i=0; i<pb_report_type.n_elements(); i++) {
      if(pb_report_type[i] < 100 || pb_report_type[i] > 300) {
         mlog << Error << "\nPB2NCConfInfo::process_config() -> "
              << "the \"" << conf_key_pb_report_type
              << "\" entries (" << pb_report_type[i]
              << ") must be set between 100 and 300.\n\n";
         exit(1);
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
         mlog << Error << "\nPB2NCConfInfo::process_config() -> "
              << "the \"" << conf_key_level_category
              << "\" entries (" << level_category[i]
              << ") must be set between 0 and 7.\n\n";
         exit(1);
      }
   }

   // Conf: obs_grib_code
   sa = conf.lookup_string_array(conf_key_obs_grib_code);

   // Check the values
   if(sa.n_elements() == 0) {
      mlog << Error << "\nPB2NCConfInfo::process_config() -> "
           << "the \"" << conf_key_obs_grib_code
           << "\" entry cannot be empty.\n\n";
      exit(1);
   }
   
   // Convert strings to GRIB codes
   for(i=0; i<sa.n_elements(); i++) obs_grib_code.add(str_to_grib_code(sa[i]));

   // Conf: var_name
   sa = conf.lookup_string_array(conf_key_obs_var, false);
   for(i=0; i<sa.n_elements(); i++) bufr_var_name.add(sa[i]);
   
   // Conf: use_var_id
   bool cnf_use_var_id = conf.lookup_bool(conf_key_use_var_id, false);
   if (cnf_use_var_id) {
      mlog << Error << "\nPB2NCConfInfo::process_config() -> "
           << "the \"" << conf_key_use_var_id
           << "\" entry (" << cnf_use_var_id
           << ") must be supported.\n\n";
      exit(1);
      
   }
   
   // Conf: quality_mark_thresh
   quality_mark_thresh = conf.lookup_int(conf_key_quality_mark_thresh);

   // Check the value
   if(quality_mark_thresh < 0 || quality_mark_thresh > 15) {
      mlog << Error << "\nPB2NCConfInfo::process_config() -> "
           << "the \"" << conf_key_quality_mark_thresh
           << "\" entry (" << quality_mark_thresh
           << ") must be set between 0 and 15.\n\n";
      exit(1);
   }

   // Conf: event_stack_flag
   event_stack_flag = conf.lookup_bool(conf_key_event_stack_flag);

   // Conf: tmp_dir
   tmp_dir = parse_conf_tmp_dir(&conf);

   obs_var_map = parse_conf_obs_var_map(&conf);

   return;
}

////////////////////////////////////////////////////////////////////////
