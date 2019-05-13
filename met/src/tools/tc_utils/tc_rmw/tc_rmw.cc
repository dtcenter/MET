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

#include "tc_rmw.h"

#include "vx_grid.h"
#include "tcrmw_grid.h"

#include "vx_tc_util.h"
#include "vx_util.h"
#include "vx_log.h"

#include "met_file.h"

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_decks();
static void process_bdecks(TrackInfoArray&);
static void process_adecks(const TrackInfoArray&);
static void process_edecks(const TrackInfoArray&);
static void get_atcf_files(const StringArray&,
                           const StringArray&,
                           StringArray&, StringArray&);
static void process_track_files(const StringArray&,
                                const StringArray&,
                                TrackInfoArray&, bool, bool);
static void compute_grids();
static void set_bdeck(const StringArray&);
static void set_adeck(const StringArray&);
static void set_edeck(const StringArray&);
static void set_atcf_source(const StringArray&,
                            StringArray&, StringArray&);
static void set_config(const StringArray&);
static void set_out(const StringArray&);
static void set_logfile(const StringArray&);
static void set_verbosity(const StringArray&);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

    // Process the command line arguments
    process_command_line(argc, argv);

    // Process deck files
    process_decks();

    // List the output files
    for(int i =0 ; i < out_files.n_elements(); i++) {
        mlog << Debug(1)
             << "Output file: " << out_files[i] << "\n";
    }

    return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {

    CommandLine cline;
    ConcatString default_config_file;

    // Default output file
    out_base = "./tc_rmw";

    // Parse command line into tokens
    cline.set(argc, argv);

    // Set usage function

    // Add function calls for arguments
    cline.add(set_config,    "-config", 1);
    cline.add(set_out,       "-out",    1);
    cline.add(set_logfile,   "-log",    1);
    cline.add(set_verbosity, "-v",      1);

    // Parse command line
    cline.parse();

    // Create efault config file name
    default_config_file = replace_path(default_config_filename);

    // List config files
    mlog << Debug(1)
         << "Config File Default: " << default_config_file << "\n"
         << "Config File User: " << config_file << "\n";

    // Read config files
    conf_info.read_config(default_config_file.c_str(),
                          config_file.c_str());

    return;
}

////////////////////////////////////////////////////////////////////////

void process_decks() {
    TrackInfoArray bdeck_tracks;

    // Process BDECK files
    process_bdecks(bdeck_tracks);
}

////////////////////////////////////////////////////////////////////////

static void process_bdecks(TrackInfoArray&) {

}

////////////////////////////////////////////////////////////////////////

static void process_adecks(const TrackInfoArray&) {

}

////////////////////////////////////////////////////////////////////////

static void process_edecks(const TrackInfoArray&) {

}

////////////////////////////////////////////////////////////////////////
// Automated Tropical Cyclone Forecasting System
// https://www.nrlmry.navy.mil/atcf_web/docs/ATCF-FAQ.html

void get_atcf_files(const StringArray& source,
                    const StringArray& model_suffix,
                    StringArray& files,
                    StringArray& files_model_suffix) {

    StringArray cur_source, cur_files;

    if(source.n_elements() != model_suffix.n_elements()) {
        mlog << Error
             << "\nget_atcf_files() -> "
             << "the source and suffix arrays must be equal length!\n\n";
        exit(1);
    }

    // Initialize
    files.clear();
    files_model_suffix.clear();

    // Build list of files from all sources
    for(int i = 0; i < source.n_elements(); i++) {
        cur_source.clear();
        cur_source.add(source[i]);
        cur_files = get_filenames(cur_source, NULL, atcf_suffix);

        for (int j = 0; j < cur_files.n_elements(); j++) {
            files.add(cur_files[j]);
            files_model_suffix.add(model_suffix[i]);
        }
    }

    return;
}

////////////////////////////////////////////////////////////////////////

static void process_track_files(const StringArray& files,
                                const StringArray& model_suffix,
                                TrackInfoArray& tracks,
                                bool check_keep,
                                bool check_anly) {

}

////////////////////////////////////////////////////////////////////////

void set_bdeck(const StringArray& b) {
    set_atcf_source(b, bdeck_source, bdeck_model_suffix);
}

////////////////////////////////////////////////////////////////////////

void set_adeck(const StringArray& a) {
    set_atcf_source(a, adeck_source, adeck_model_suffix);
}

////////////////////////////////////////////////////////////////////////

void set_edeck(const StringArray& e) {
    set_atcf_source(e, edeck_source, edeck_model_suffix);
}

////////////////////////////////////////////////////////////////////////

void set_atcf_source(const StringArray& a,
                     StringArray& source,
                     StringArray& model_suffix) {
    StringArray sa;
    ConcatString cs, suffix;

    // Check for optional suffix sub-argument
    for(int i = 0; i < a.n_elements(); i++) {
        if( a[i] == "suffix" ) {
            cs = a[i];
            sa = cs.split("=");
            if(sa.n_elements() != 2) {
                mlog << Error
                     << "\nset_atcf_source() -> "
                     << "the model suffix must be specified as "
                     << "\"suffix=string\".\n\n";
            }
            else {
                suffix = sa[1];
            }
        }
    }

    // Parse the remaining sources
    for(int i = 0; i < a.n_elements(); i++) {
        if( a[i] == "suffix" ) continue;
        source.add(a[i]);
        model_suffix.add(suffix);
    }
}

////////////////////////////////////////////////////////////////////////

void set_config(const StringArray& a) {
    config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_out(const StringArray& a) {
    out_base = a[0];
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

static void compute_grids() {

    TcrmwGrid grid;
}
