#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <cmath>

#include "grid_diag_conf_info.h"

#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class GridDiagConfInfo
//
////////////////////////////////////////////////////////////////////////

GridDiagConfInfo::GridDiagConfInfo() {

    init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

GridDiagConfInfo::~GridDiagConfInfo() {

    clear();
}

////////////////////////////////////////////////////////////////////////

void GridDiagConfInfo::init_from_scratch() {

    clear();

    return;
}

////////////////////////////////////////////////////////////////////////

void GridDiagConfInfo::clear() {

    Version.clear();

    return;
}

////////////////////////////////////////////////////////////////////////

void GridDiagConfInfo::read_config(const char* default_file_name,
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

void GridDiagConfInfo::process_config() {

    // Conf: Version
    Version = Conf.lookup_string(conf_key_version);
    check_met_version(Version.c_str());

    return;
}

////////////////////////////////////////////////////////////////////////
