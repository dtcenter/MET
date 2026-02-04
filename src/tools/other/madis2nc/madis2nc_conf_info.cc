// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
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

#include "madis2nc_conf_info.h"

#include "vx_log.h"

using namespace std;


////////////////////////////////////////////////////////////////////////
//
//  Code for class Madis2NcConfInfo
//
////////////////////////////////////////////////////////////////////////

Madis2NcConfInfo::Madis2NcConfInfo()
{
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

Madis2NcConfInfo::~Madis2NcConfInfo()
{
   clear();
}

////////////////////////////////////////////////////////////////////////

void Madis2NcConfInfo::init_from_scratch()
{
   clear();
}

////////////////////////////////////////////////////////////////////////

void Madis2NcConfInfo::clear()
{
   _version.clear();
   grib_name_map.clear();
   grib_unit_map.clear();
}

////////////////////////////////////////////////////////////////////////

ConcatString Madis2NcConfInfo::get_grib_var_name(const int grib_code) {
   ConcatString grib_var_name = grib_name_map[grib_code];
   return grib_var_name;
}

////////////////////////////////////////////////////////////////////////

ConcatString Madis2NcConfInfo::get_grib_var_unit(const int grib_code) {
   ConcatString grib_var_name = grib_unit_map[grib_code];
   return grib_var_name;
}

////////////////////////////////////////////////////////////////////////

void Madis2NcConfInfo::read_config(const string &default_filename,
                                   const string &user_filename)
{
  // Read the config file constants

  _conf.read(replace_path(config_const_filename).c_str());
  
  // Read the default config file

  _conf.read(replace_path(default_filename).c_str());

  // Read the user config file

  _conf.read(user_filename.c_str());

  // Process the configuration file

  process_config();

  return;
}

////////////////////////////////////////////////////////////////////////

void Madis2NcConfInfo::process_config()
{

  _version = parse_conf_version(&_conf);
  check_met_version(_version.c_str());

  // Conf: grib_var_map
  for (const auto& pair : parse_conf_key_values_map(&_conf, conf_key_grib_var_map)) {
    int grib_code = atoi(pair.first.c_str());
    grib_name_map[grib_code] = pair.second[0];
    grib_unit_map[grib_code] = pair.second[1];
  }

  _timeSummaryInfo = parse_conf_time_summary(&_conf);
  
   return;
}

////////////////////////////////////////////////////////////////////////
