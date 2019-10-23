// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

#include "rmw_analysis_conf_info.h"

#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class RMWAnalysisConfInfo
//
////////////////////////////////////////////////////////////////////////

RMWAnalysisConfInfo::RMWAnalysisConfInfo() {

    init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

RMWAnalysisConfInfo::~RMWAnalysisConfInfo() {

    clear();
}

////////////////////////////////////////////////////////////////////////

void RMWAnalysisConfInfo::init_from_scratch() {

    // Initialize pointers
    data_info = (VarInfo**) 0;

    clear();

    return;
}

////////////////////////////////////////////////////////////////////////

void RMWAnalysisConfInfo::clear() {

    Version.clear();

    return;
}

////////////////////////////////////////////////////////////////////////

void RMWAnalysisConfInfo::read_config(const char* default_file_name,
                                const char* user_file_name) {

    // Read config file constants
    Conf.read(replace_path(config_const_filename).c_str());

    // Read default config file
    Conf.read(default_file_name);

    // Read user-specified config file
    Conf.read(user_file_name);

    return;
}

////////////////////////////////////////////////////////////////////////

void RMWAnalysisConfInfo::process_config() {

    // Conf: Version
    Version = Conf.lookup_string(conf_key_version);
    check_met_version(Version.c_str());

    return;
}

////////////////////////////////////////////////////////////////////////
