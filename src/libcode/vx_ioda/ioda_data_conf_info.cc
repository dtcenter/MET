// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <cmath>

#include "ioda_data_conf_info.h"

#include "vx_log.h"

using namespace std;


////////////////////////////////////////////////////////////////////////
//
//  Code for class IODADataConfInfo
//
////////////////////////////////////////////////////////////////////////

IODADataConfInfo::IODADataConfInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

IODADataConfInfo::~IODADataConfInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void IODADataConfInfo::init_from_scratch() {
   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void IODADataConfInfo::clear() {

   // Initialize values
   metadata_map.clear();
   missing_thresh.clear();
   obs_name_map.clear();
   obs_to_qc_map.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void IODADataConfInfo::read_data_config(const char *default_file_name,
                                        const char *user_file_name) {

   // Read the config file constants
   conf.read(replace_path(config_const_filename).c_str());

   conf.read(replace_path(DEF_DATA_CONFIG_NAME).c_str());

   // Read the default config file
   conf.read(default_file_name);

   // Read the user-specified config file
   if (user_file_name && strcmp(user_file_name, "")) conf.read(user_file_name);

   return;
}

////////////////////////////////////////////////////////////////////////

void IODADataConfInfo::process_data_config() {
   static const char *method_name = "IODADataConfInfo::process_data_config() -> ";

   // Initialize
   clear();

   // Conf: missing_thresh
   missing_thresh = conf.lookup_thresh_array(conf_key_missing_thresh, false);

   // Conf: obs_name_map
   obs_name_map = parse_conf_obs_name_map(&conf);
   metadata_map = parse_conf_metadata_map(&conf);
   obs_to_qc_map = parse_conf_obs_to_qc_map(&conf);

   mlog << Debug(5) << method_name
        << "obs_name_map: " << obs_name_map.size()
        << ", metadata_map: " << metadata_map.size()
        << ", obs_to_qc_map: " << obs_to_qc_map.size() << "\n";
   return;
}

////////////////////////////////////////////////////////////////////////
