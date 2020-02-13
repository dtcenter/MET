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
        mlog << Warning << "No vertical dimension found.\n";
    }

    mlog << Debug(2)
         << "(n_range, n_azimuth, n_level) = ("
         << n_range << ", " << n_azimuth << ", " << n_level << ")\n"; 

    // Read variable information
    for(int i_var = 0; i_var < conf_info.get_n_data(); i_var++) {
        data_names.push_back(conf_info.data_info[i_var]->name().string());
        NcVar var = get_nc_var(
            nc_out, conf_info.data_info[i_var]->name().c_str());
        int n_dim = get_dim_count(&var) - 1;
        data_n_dims.push_back(n_dim);
        ConcatString s;
        get_att_value_string(&var, "long_name", s);
        data_long_names.push_back(s.string());
        get_att_value_string(&var, "units", s);
        data_units.push_back(s.string());
    }

    // Size data cubes
    DataCube data_2d;
    DataCube data_3d;

    data_2d.set_size(n_range, n_azimuth, 1);
    data_3d.set_size(n_range, n_azimuth, n_level);

    data_2d.set_constant(0);
    data_3d.set_constant(0);

    // Initialize statistical data cube lists
    for(int i_var = 0; i_var < data_names.size(); i_var++) {
        if (data_n_dims[i_var] == 2) {
            data_counts.push_back(data_2d);
            data_means.push_back(data_2d);
            data_stdevs.push_back(data_2d);
            data_mins.push_back(data_2d);
            data_maxs.push_back(data_2d);
        }
        if (data_n_dims[i_var] == 3) {
            data_counts.push_back(data_3d);
            data_means.push_back(data_3d);
            data_stdevs.push_back(data_3d);
            data_mins.push_back(data_3d);
            data_maxs.push_back(data_3d);
        }
    }
}

////////////////////////////////////////////////////////////////////////

void process_files() {

    // Size data cubes
    DataCube data_2d;
    DataCube data_2d_sq;
    DataCube data_3d;
    DataCube data_3d_sq;

    data_2d.set_size(n_range, n_azimuth, 1);
    data_3d.set_size(n_range, n_azimuth, n_level);

    // Set up array track point slices
    vector<size_t> start_2d;
    vector<size_t> count_2d;
    vector<size_t> start_3d;
    vector<size_t> count_3d;
    start_2d.push_back(0);
    start_2d.push_back(0);
    start_2d.push_back(0);
    count_2d.push_back(n_range);
    count_2d.push_back(n_azimuth);
    count_2d.push_back(1);
    start_3d.push_back(0);
    start_3d.push_back(0);
    start_3d.push_back(0);
    start_3d.push_back(0);
    count_3d.push_back(n_range);
    count_3d.push_back(n_azimuth);
    count_3d.push_back(n_level);
    count_3d.push_back(1);

    for(int i_file = 0; i_file < data_files.n_elements(); i_file++) {
        mlog << Debug(1) << "Processing "
             << data_files[i_file] << "\n";
        nc_out = open_ncfile(data_files[i_file].c_str());

        get_dim(nc_out, "track_point", n_track_point, true);
        track_point_dim = get_nc_dim(nc_out, "track_point");
        mlog << Debug(1) << "Number of track points "
             << n_track_point << "\n";

        for(int i_var = 0; i_var < data_names.size(); i_var++) {
            NcVar var = get_nc_var(nc_out, data_names[i_var].c_str());
            mlog << Debug(2) << "Processing "
                 << data_names[i_var] << "\n";

            for(int i_track = 0; i_track < n_track_point; i_track++) {
                if (data_n_dims[i_var] == 2) {
                    start_2d[2] = i_track;
                    mlog << Debug(4) << data_names[i_var] << i_track << "\n";
                    var.getVar(start_2d, count_2d, data_2d.data());

                    // Update partial sums
                    data_2d_sq = data_2d;
                    data_2d_sq.square();
                    data_counts[i_var].increment();
                    data_means[i_var].add_assign(data_2d);
                    data_stdevs[i_var].add_assign(data_2d_sq);
                    data_mins[i_var].min_assign(data_2d);
                    data_maxs[i_var].max_assign(data_2d);
                }
                if (data_n_dims[i_var] == 3) {
                    mlog << Debug(4) << data_names[i_var] << i_track << "\n";
                    start_3d[3] = i_track;
                    var.getVar(start_3d, count_3d, data_3d.data());

                    // Update partial sums
                    data_3d_sq = data_3d;
                    data_3d_sq.square();
                    data_counts[i_var].increment();
                    data_means[i_var].add_assign(data_3d);
                    data_stdevs[i_var].add_assign(data_3d_sq);
                    data_mins[i_var].min_assign(data_3d);
                    data_maxs[i_var].max_assign(data_3d);
                }
            } // end loop over track points
        } // end loop over variables
    } // end loop over files
}

////////////////////////////////////////////////////////////////////////
