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

    // Initialize pointers
    data_info = (VarInfo**) 0;

    clear();

    return;
}

////////////////////////////////////////////////////////////////////////

void GridDiagConfInfo::clear() {

    Version.clear();

    // Clear data_info
    if(data_info) {
        for(int i = 0; i < n_data; i++) {
            if(data_info[i]) {
                data_info[i] = (VarInfo*) 0;
            }
        }
        delete data_info;
        data_info = (VarInfo**) 0;
    }

    // Reset field count
    n_data = 0;

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

void GridDiagConfInfo::process_config(GrdFileType ftype) {

    VarInfoFactory info_factory;
    Dictionary *fdict = (Dictionary *) 0;

    // Conf: Version
    Version = Conf.lookup_string(conf_key_version);
    check_met_version(Version.c_str());

    // Conf: data.field
    fdict = Conf.lookup_array(conf_key_data_field);

    // Determine number of fields (name/level)
    n_data = parse_conf_n_vx(fdict);

    mlog << Debug(2) << "n_data:" << n_data << "\n";

    // Check for empty data settings
    if(n_data == 0) {
        mlog << Error << "\nGridDiagConfInfo::process_config() -> "
             << "data may not be empty.\n\n";
        exit(1);
    }

    // Allocate space based on number of fields
    data_info = new VarInfo*[n_data];

    // Initialize pointers
    for(int i = 0; i < n_data; i++) {
        data_info[i] = (VarInfo*) 0;
    }

    // Parse data field information
    for(int i = 0; i < n_data; i++) {

        // Allocate new VarInfo objects
        data_info[i] = info_factory.new_var_info(ftype);

        // Get current dictionary
        Dictionary i_fdict = parse_conf_i_vx_dict(fdict, i);

        // Set current dictionary
        data_info[i]->set_dict(i_fdict);

        mlog << Debug(2) << data_info[i]->magic_str() << "\n";

        // Dump contents of current VarInfo
        if(mlog.verbosity_level() >=5) {
            mlog << Debug(5) << "Parsed data field "
            << i + 1 << ":\n";
            data_info[i]->dump(cout);
        }
    }

    return;
}

////////////////////////////////////////////////////////////////////////
