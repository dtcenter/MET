// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "rmw_analysis.h"
#include "rmw_analysis_utils.h"

// #include "vx_grid.h"
// #include "vx_regrid.h"
// #include "vx_tc_util.h"
#include "vx_nc_util.h"
#include "vx_util.h"
#include "vx_log.h"

// #include "met_file.h"

////////////////////////////////////////////////////////////////////////

static void usage();
static void process_command_line(int, char**);
static void set_data_files(const StringArray&);
static void set_config(const StringArray&);
static void set_out(const StringArray&);
static void set_logfile(const StringArray&);
static void set_verbosity(const StringArray&);
static void setup();
static void process_files();

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

    // Set handler for memory allocation error
    set_new_handler(oom); // out of memory

    // Process command line arguments
    process_command_line(argc, argv);

    // Set up
    setup();

    // Process files
    process_files();

    return(0);
}

////////////////////////////////////////////////////////////////////////

void usage() {

    cout << "\n*** Model Evaluation Tools (MET" << met_version
         << ") ***\n\n"
         << "Usage: " << program_name << "\n"
         << "\t[-out file]\n"
         << "\t[-log file]\n"
         << "\t[-v level]\n\n" << flush;

    exit(1);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {

    CommandLine cline;
    ConcatString default_config_file;

    // Default output directory
    out_dir = replace_path(default_out_dir);

    // Parse command line into tokens
    cline.set(argc, argv);

    // Set usage function
    cline.set_usage(usage);

    cline.add(set_data_files, "-data",   -1);
    cline.add(set_config,     "-config", -1);
    cline.add(set_out,        "-out",     1);
    cline.add(set_logfile,    "-log",     1);
    cline.add(set_verbosity,  "-v",       1);

    // Parse command line
    cline.parse();

    // Create default config file name
    default_config_file = replace_path(default_config_filename);

    // List config files
    mlog << Debug(1)
         << "Config File Default: " << default_config_file << "\n"
         << "Config File User: " << config_file << "\n";

    // Read config files
    conf_info.read_config(default_config_file.c_str(),
                          config_file.c_str());

    // Process the configuration
    conf_info.process_config();

    return;
}

////////////////////////////////////////////////////////////////////////

void set_data_files(const StringArray& a) {
    data_files = a;
}

////////////////////////////////////////////////////////////////////////

void set_config(const StringArray& a) {
    config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_out(const StringArray& a) {
    out_dir = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray& a) {
    ConcatString filename = a[0];
    mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray& a) {
    mlog.set_verbosity_level(atoi(a[0].c_str()));
}

////////////////////////////////////////////////////////////////////////

void setup() {

    // Open first data file
    nc_out = open_ncfile(data_files[0].c_str());
    mlog << Debug(1) << "Reading dimensions from"
         << data_files[0] << "\n";

    // Get dimension sizes
    get_dim(nc_out, "range", n_range, true);
    range_dim = get_nc_dim(nc_out, "range");

    get_dim(nc_out, "azimuth", n_azimuth, true);
    azimuth_dim = get_nc_dim(nc_out, "azimuth");

    if (get_dim(nc_out, "height", n_level)) {
        mlog << Debug(1) << "Found height vertical dimension.\n";
        level_dim = get_nc_dim(nc_out, "height");
        level_name = "height";
    } else if (get_dim(nc_out, "pressure", n_level)) {
        mlog << Debug(1) << "Found pressure vertical dimension.\n";
        level_dim = get_nc_dim(nc_out, "pressure");
        level_name = "pressure";
    } else {
        mlog << Error << "No vertical dimension found.\n";
        exit(1);
    }

    mlog << Debug(2)
         << "(n_range, n_azimuth, n_level) = ("
         << n_range << ", " << n_azimuth << ", " << n_level << ")\n"; 

    // Read variable information
    for(int i_var = 0; i_var < conf_info.get_n_data(); i_var++) {
        data_names.push_back(conf_info.data_info[i_var]->name().string());
        NcVar var = get_nc_var(
            nc_out, conf_info.data_info[i_var]->name().c_str());
        int n_dim = get_dim_count(&var);
        data_n_dims.push_back(n_dim);
        ConcatString s;
        get_att_value_string(&var, "long_name", s);
        data_long_names.push_back(s.string());
        get_att_value_string(&var, "units", s);
        data_units.push_back(s.string());
    }
}

////////////////////////////////////////////////////////////////////////

void process_files() {

}

////////////////////////////////////////////////////////////////////////
