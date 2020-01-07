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
#include <cmath>

#include "ascii2nc_conf_info.h"

#include "vx_log.h"


////////////////////////////////////////////////////////////////////////
//
//  Code for class Ascii2NcConfInfo
//
////////////////////////////////////////////////////////////////////////

Ascii2NcConfInfo::Ascii2NcConfInfo()
{
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

Ascii2NcConfInfo::~Ascii2NcConfInfo()
{
   clear();
}

////////////////////////////////////////////////////////////////////////

void Ascii2NcConfInfo::init_from_scratch()
{
   clear();
}

////////////////////////////////////////////////////////////////////////

void Ascii2NcConfInfo::clear()
{
   _version.clear();
   _messageTypeMap.clear();
}

////////////////////////////////////////////////////////////////////////

void Ascii2NcConfInfo::read_config(const string &default_filename,
				   const string &user_filename)
{
  // Read the config file constants

  _conf.read(replace_path(config_const_filename).c_str());
  
  // Read the default config file

  _conf.read(replace_path(default_filename.c_str()).c_str());

  // Read the user config file

  _conf.read(user_filename.c_str());

  // Process the configuration file

  process_config();

  return;
}

////////////////////////////////////////////////////////////////////////

void Ascii2NcConfInfo::process_config()
{
  
  _version = parse_conf_version(&_conf);
  check_met_version(_version.c_str());

  _timeSummaryInfo = parse_conf_time_summary(&_conf);
  _messageTypeMap  = parse_conf_message_type_map(&_conf);
  
   return;
}

////////////////////////////////////////////////////////////////////////
