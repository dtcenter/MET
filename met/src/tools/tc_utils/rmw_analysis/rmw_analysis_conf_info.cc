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

#include "apply_mask.h"
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
    Model.clear();
    StormId.clear();
    Basin.clear();
    Cyclone.clear();
    StormName.clear();
    InitBeg = InitEnd = (unixtime) 0;
    ValidBeg = ValidEnd = (unixtime) 0;
    InitMaskName.clear();
    InitPolyMask.clear();
    InitGridMask.clear();
    InitAreaMask.clear();
    ValidMaskName.clear();
    ValidPolyMask.clear();
    ValidGridMask.clear();
    ValidAreaMask.clear();

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

    ConcatString poly_file;

    // Conf: Version
    Version = Conf.lookup_string(conf_key_version);
    check_met_version(Version.c_str());

    // Conf: Model
    Model = Conf.lookup_string_array(conf_key_model);

    // Conf: StormId
    StormId = Conf.lookup_string_array(conf_key_storm_id);

    // Conf: Basin
    Basin = Conf.lookup_string_array(conf_key_basin);

    // Conf: Cyclone
    Cyclone = Conf.lookup_string_array(conf_key_cyclone);

    // Conf: StormName
    StormName = Conf.lookup_string_array(conf_key_storm_name);

    // Conf: InitBeg, InitEnd
    InitBeg = Conf.lookup_unixtime(conf_key_init_beg);
    InitEnd = Conf.lookup_unixtime(conf_key_init_end);

    // Conf: ValidBeg, ValidEnd
    ValidBeg = Conf.lookup_unixtime(conf_key_valid_beg);
    ValidEnd = Conf.lookup_unixtime(conf_key_valid_end);

    // Conf: InitMask
    if(nonempty(Conf.lookup_string(conf_key_init_mask).c_str())) {
        poly_file = replace_path(Conf.lookup_string(conf_key_init_mask));
        mlog << Debug(2)
             << "Init Points Masking File: " << poly_file << "\n";
        parse_poly_mask(poly_file, InitPolyMask, InitGridMask,
                        InitAreaMask, InitMaskName);
    }

    // Conf: ValidMask
    if(nonempty(Conf.lookup_string(conf_key_valid_mask).c_str())) {
        poly_file = replace_path(Conf.lookup_string(conf_key_valid_mask));
        mlog << Debug(2)
             << "Valid Point Masking File: " << poly_file << "\n";
        parse_poly_mask(poly_file, ValidPolyMask, ValidGridMask,
                        ValidAreaMask, ValidMaskName);
    }

    return;
}

////////////////////////////////////////////////////////////////////////
