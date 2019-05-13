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

#include "tc_rmw_conf_info.h"

#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class TCRMWConfInfo
//
////////////////////////////////////////////////////////////////////////

TCRMWConfInfo::TCRMWConfInfo() {

    init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCRMWConfInfo::~TCRMWConfInfo() {

    clear();
}

////////////////////////////////////////////////////////////////////////

void TCRMWConfInfo::init_from_scratch() {

    clear();

    return;
}

////////////////////////////////////////////////////////////////////////

void TCRMWConfInfo::clear() {

    Desc.clear();
    Model.clear();
    StormId.clear();
    Basin.clear();
    Cyclone.clear();
    StormName.clear();
    InitBeg = InitEnd = (unixtime) 0;
    InitHour.clear();
    LeadReq.clear();
    ValidBeg = ValidEnd = (unixtime) 0;
    Track = TrackType_None;
    Version.clear();

    return;
}

////////////////////////////////////////////////////////////////////////

void TCRMWConfInfo::read_config(const char* default_file_name,
                                const char* user_file_name) {

    // Read config file constants
    Conf.read(replace_path(config_const_filename).c_str());

    // Read default config file
    Conf.read(default_file_name);

    // Read user-specified config file
    Conf.read(user_file_name);

    // Process configuration file
    process_config();

    return;
}

////////////////////////////////////////////////////////////////////////

void TCRMWConfInfo::process_config() {
    int i, j;
    StringArray sa;
    Dictionary *dict = (Dictionary *) 0;

    // Conf: Version
    Version = Conf.lookup_string(conf_key_version);
    check_met_version(Version.c_str());

    // Conf: Desc
    Desc = parse_conf_string(&Conf, conf_key_desc);

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

    // Conf: InitInc
    sa = Conf.lookup_string_array(conf_key_init_inc);
    for(i=0; i<sa.n_elements(); i++)
        InitInc.add(timestring_to_unix(sa[i].c_str()));

    // Conf: InitExc
    sa = Conf.lookup_string_array(conf_key_init_exc);
    for(i=0; i<sa.n_elements(); i++)
        InitExc.add(timestring_to_unix(sa[i].c_str()));

    // Conf: InitHour
    sa = Conf.lookup_string_array(conf_key_init_hour);
    for(i=0; i<sa.n_elements(); i++)
        InitHour.add(timestring_to_sec(sa[i].c_str()));

    // Conf: ValidBeg, ValidEnd
    ValidBeg = Conf.lookup_unixtime(conf_key_valid_beg);
    ValidEnd = Conf.lookup_unixtime(conf_key_valid_end);

    // Conf: LeadReq
    sa = Conf.lookup_string_array(conf_key_lead_req);
    for(i=0; i<sa.n_elements(); i++){
        LeadReq.add(timestring_to_sec(sa[i].c_str()));
    }

    // Conf: Track
    // Track = int_to_tracktype(Conf.lookup_int(conf_key_track));

    return;
}

////////////////////////////////////////////////////////////////////////
