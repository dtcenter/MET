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
#include "vx_regrid.h"

#include "vx_tc_util.h"
#include "vx_nc_util.h"
#include "vx_tc_nc_util.h"
#include "vx_util.h"
#include "vx_log.h"

#include "met_file.h"

////////////////////////////////////////////////////////////////////////

static void usage();
static void process_command_line(int, char**);
static void process_decks();
static void process_adecks(TrackInfoArray&);
static void process_bdecks(TrackInfoArray&);
static void process_edecks(TrackInfoArray&);
static void get_atcf_files(const StringArray&,
    const StringArray&, StringArray&, StringArray&);
static void process_track_files(const StringArray&,
    const StringArray&, TrackInfoArray&, bool, bool);
static void set_adeck(const StringArray&);
static void set_bdeck(const StringArray&);
static void set_edeck(const StringArray&);
static void set_atcf_source(const StringArray&,
    StringArray&, StringArray&);
static void set_fcst_files(const StringArray&);
static void set_config(const StringArray&);
static void set_out(const StringArray&);
static void set_logfile(const StringArray&);
static void set_verbosity(const StringArray&);
static StringArray parse_file_list(const StringArray&,
    const GrdFileType);
static void get_series_data(int, VarInfo*, DataPlane&);
static void get_series_entry(int, VarInfo*,
    const StringArray&, const GrdFileType,
    StringArray&, DataPlane&);
static bool read_single_entry(VarInfo*, const ConcatString&,
    const GrdFileType, DataPlane&, Grid&);
static void setup_grid();
static void setup_nc_file();
static void compute_grids(const TrackInfoArray&);
static void write_nc(const ConcatString&,
    const DataPlane&, FieldType);
static bool read_data_plane(VarInfo*, DataPlane&, Met2dDataFile*,
    const ConcatString&);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

    // Set handler for memory allocation error
    set_new_handler(oom); // out of memory

    // Process command line arguments
    process_command_line(argc, argv);

    // Setup grid
    setup_grid();

    // Setup NetCDF output
    setup_nc_file();

    // Process deck files
    process_decks();

    return(0);
}

////////////////////////////////////////////////////////////////////////

void usage() {

    cout << "\n*** Model Evaluation Tools (MET" << met_version
         << ") ***\n\n"
         << "Usage: " << program_name << "\n"
         << "\t-fcst file_1 ... file_n | fcst_file_list\n"
         << "\t-adeck file\n"
         << "\t-config file\n"
         << "\t[-bdeck file]\n"
         << "\t[-ddeck file]\n"
         << "\t[-log file]\n"
         << "\t[-out file]\n"
         << "\t[-v level]\n\n" << flush;

    exit(1);
}

////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {

    CommandLine cline;
    ConcatString default_config_file;

    // Default output base
    out_dir = "./";

    // Parse command line into tokens
    cline.set(argc, argv);

    // Set usage function
    cline.set_usage(usage);

    // Add function calls for arguments
    cline.add(set_fcst_files, "-fcst",  -1);
    cline.add(set_adeck,      "-adeck", -1);
    cline.add(set_bdeck,      "-bdeck", -1);
    cline.add(set_edeck,      "-edeck", -1);
    cline.add(set_config,     "-config", 1);
    cline.add(set_out,        "-out",    1);
    cline.add(set_logfile,    "-log",    1);
    cline.add(set_verbosity,  "-v",      1);

    // Parse command line
    cline.parse();

    // Check number of arguments
    // if(cline.n() != 1) usage();

    // fcst_file = cline[0];

    // Create default config file name
    default_config_file = replace_path(default_config_filename);

    // List config files
    mlog << Debug(1)
         << "Config File Default: " << default_config_file << "\n"
         << "Config File User: " << config_file << "\n";

    // Read config files
    conf_info.read_config(default_config_file.c_str(),
                          config_file.c_str());


    // Get forecast file type from config
    GrdFileType ftype
        = parse_conf_file_type(conf_info.Conf.lookup_dictionary(
        conf_key_fcst));

    // Process the configuration
    conf_info.process_config(ftype);

    // Read forecast file
    // if(!(fcst_mtddf
    //     = mtddf_factory.new_met_2d_data_file(
    //         fcst_file.c_str(), ftype))) {

    //     mlog << Error << "\nTrouble reading forecast file \""
    //          << fcst_file << "\"\n\n";
    //     exit(1);
    // }

    return;
}

////////////////////////////////////////////////////////////////////////

void process_decks() {
    // Process ADECK files
    TrackInfoArray adeck_tracks;
    process_adecks(adeck_tracks);

    compute_grids(adeck_tracks);

    // nc_out->close();
}

////////////////////////////////////////////////////////////////////////

static void process_adecks(TrackInfoArray& adeck_tracks) {
    StringArray files, files_model_suffix;

    // Initialize
    adeck_tracks.clear();

    // Get list of track files
    get_atcf_files(adeck_source, adeck_model_suffix,
                   files, files_model_suffix);

    mlog << Debug(2)
         << "Processing " << files.n_elements() << " ADECK file(s).\n";

    process_track_files(files, files_model_suffix, adeck_tracks,
                        false, false);

    ConcatString adeck_track_file("adeck.nc");

    write_nc_tracks(adeck_track_file, adeck_tracks);
}

////////////////////////////////////////////////////////////////////////

static void process_bdecks(TrackInfoArray& bdeck_tracks) {
    StringArray files, files_model_suffix;

    // Initialize
    bdeck_tracks.clear();

    // Get list of track files
    get_atcf_files(bdeck_source, bdeck_model_suffix,
                   files, files_model_suffix);

    mlog << Debug(2)
         << "Processing " << files.n_elements() << " BDECK file(s).\n";

    process_track_files(files, files_model_suffix, bdeck_tracks,
                        false, false);
}

////////////////////////////////////////////////////////////////////////

static void process_edecks(const TrackInfoArray& edeck_tracks) {

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

        for(int j = 0; j < cur_files.n_elements(); j++) {
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
    int cur_read, cur_add, tot_read, tot_add;

    LineDataFile f;
    ConcatString cs;
    ATCFTrackLine line;

    // Initialize
    tracks.clear();

    // Initialize counts
    tot_read = tot_add = 0;

    // Process input ATCF files
    for(int i = 0; i < files.n_elements(); i++) {

        // Open current file
        if(!f.open(files[i].c_str())) {
            mlog << Error
                 << "\nprocess_track_files() -> "
                 << "unable to open file \"" << files[i] << "\"\n\n";
            exit(1);
        }

        // Initialize counts
        cur_read = cur_add = 0;

        // Read each line
        while(f >> line) {

            // Increment line counts
            cur_read++;
            tot_read++;

            // Add model suffix, if specified
            if(model_suffix[i].length() > 0) {
                cs << cs_erase << line.technique() << model_suffix[i];
                line.set_technique(cs);
            }

            // Attempt to add current line to TrackInfoArray
            if(tracks.add(line, conf_info.CheckDup, check_anly)) {
                cur_add++;
                tot_add++;
            }
        }

        // Dump out current number of lines
        mlog << Debug(4)
             << "[File " << i + 1 << " of " << files.n_elements()
             << "] " << cur_read
             << " lines read from file \n\"" << files[i] << "\"\n";

        // Close current file
        f.close();

    } // end loop over files
}

////////////////////////////////////////////////////////////////////////

void set_adeck(const StringArray& a) {
    set_atcf_source(a, adeck_source, adeck_model_suffix);
}

////////////////////////////////////////////////////////////////////////

void set_bdeck(const StringArray& b) {
    set_atcf_source(b, bdeck_source, bdeck_model_suffix);
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
        if(a[i] == "suffix") {
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

    // Parse remaining sources
    for(int i = 0; i < a.n_elements(); i++) {
        if( a[i] == "suffix" ) continue;
        source.add(a[i]);
        model_suffix.add(suffix);
    }
}

////////////////////////////////////////////////////////////////////////

void set_fcst_files(const StringArray& a) {
    fcst_files = a;
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

StringArray parse_file_list(const StringArray& a,
    const GrdFileType type) {

    Met2dDataFile *mtddf = (Met2dDataFile*) 0;
    StringArray list;

    // Check for empty list
    if(a.n_elements() == 0) {
        mlog << Error << "\nparse_file_list() -> "
             << "empty list!\n\n";
        exit(1);
    }

    // Attempt read of first file
    mtddf = mtddf_factory.new_met_2d_data_file(a[0].c_str(), type);

    // If successful store list
    if(mtddf) {
        list.add(a);
        delete mtddf;
        mtddf = (Met2dDataFile*) 0;
    } else { // Otherwise read a list of files 
        list = parse_ascii_file_list(a[0].c_str());
    }

    return list;
}

////////////////////////////////////////////////////////////////////////

static void get_series_data(int i_series,
    VarInfo* fcst_info, DataPlane& fcst_dp) {

}

////////////////////////////////////////////////////////////////////////

static void get_series_entry(int i_series,
    VarInfo* info, const StringArray& search_files,
    const GrdFileType type, StringArray& found_files,
    DataPlane& dp) {

}

////////////////////////////////////////////////////////////////////////

static bool read_single_entry(VarInfo*, const ConcatString&,
    const GrdFileType, DataPlane&, Grid&) {

    return true;
}

////////////////////////////////////////////////////////////////////////

static void setup_grid() {

    grid_data.name = "TCRMW";
    grid_data.range_n = conf_info.n_range;
    grid_data.azimuth_n = conf_info.n_azimuth;
    grid_data.range_max_km = conf_info.max_range;

    grid.set_from_data(grid_data);
}

////////////////////////////////////////////////////////////////////////

static void compute_grids(const TrackInfoArray& tracks) {

    VarInfo *fcst_info = (VarInfo *) 0;
    DataPlane fcst_dp;

    // these should be DataPlanes
    lat_grid = new double[
        grid.range_n() * grid.azimuth_n()];
    lon_grid = new double[
        grid.range_n() * grid.azimuth_n()];

    // assume single track for now
    TrackInfo track = tracks[0];

    for(int i_point = 0; i_point < track.n_points(); i_point++) {
        TrackPoint point = track[i_point];
        unixtime valid_time = point.valid();
        int year, month, day, hour, minute, second;
        unix_to_mdyhms(
            valid_time,
            month, day, year,
            hour, minute, second);
        long valid_yyyymmddhh = 1000000 * year
            + 10000 * month + 100 * day + hour;
        mlog << Debug(4)
             << "(" << point.lat() << ", " << point.lon() << ")\n";
        grid_data.lat_center = point.lat();
        grid_data.lon_center = - point.lon(); // internal sign change
        grid.clear();
        grid.set_from_data(grid_data);

        for(int i_var = 0; i_var < conf_info.get_n_fcst(); i_var++) {
            // Get VarInfo
            fcst_info = conf_info.fcst_info[i_var];
            // Get data
            get_series_data(i_point, fcst_info, fcst_dp);
        }

        // compute lat and lon coordinate arrays
        // move this to TcrmwGrid class
        for(int ir = 0; ir < grid.range_n(); ir++) {
            for(int ia = 0; ia < grid.azimuth_n(); ia++) {
                double lat, lon;
                grid.range_azi_to_latlon(
                    ir * grid.range_delta_km(),
                    ia * grid.azimuth_delta_deg(),
                    lat, lon);
                lat_grid[ir * grid.azimuth_n() + ia] = lat;
                lon_grid[ir * grid.azimuth_n() + ia] = - lon;
            }
        }
        // write coordinate arrays
        // move this to nc_utils
        vector<size_t> offsets, counts;
        vector<size_t> record_offsets, record_counts;
        offsets.clear();
        record_offsets.clear();
        offsets.push_back(0);
        offsets.push_back(0);
        offsets.push_back(i_point);
        record_offsets.push_back(i_point);
        counts.clear();
        record_counts.clear();
        counts.push_back(grid.range_n());
        counts.push_back(grid.azimuth_n());
        counts.push_back(1);
        record_counts.push_back(1);
        lat_grid_var.putVar(offsets, counts, lat_grid);
        lon_grid_var.putVar(offsets, counts, lon_grid);
        valid_time_var.putVar(
            record_offsets, record_counts,
            &valid_yyyymmddhh);
    }

    delete[] lat_grid;
    delete[] lon_grid;
}

////////////////////////////////////////////////////////////////////////

static void setup_nc_file() {

    out_nc_file.add("tc_rmw_grids.nc");

    mlog << Debug(1) << out_nc_file << "\n";

    // Create NetCDF file
    nc_out = open_ncfile(out_nc_file.c_str(), true);

    if(IS_INVALID_NC_P(nc_out)) {
        mlog << Error << "\nsetup_nc_file() -> "
             << "trouble opening output NetCDF file "
             << out_nc_file << "\n\n";
        exit(1);
    }

    mlog << Debug(4) << grid.range_n() << "\n";
    mlog << Debug(4) << grid.azimuth_n() << "\n";

    // Define dimensions
    range_dim = add_dim(nc_out, "range", (long) grid.range_n());
    azimuth_dim = add_dim(nc_out, "azimuth", (long) grid.azimuth_n());
    track_point_dim = add_dim(nc_out, "track_point", NC_UNLIMITED);

    write_nc_range_azimuth(nc_out, range_dim, azimuth_dim, grid);

    vector<NcDim> dims;
    dims.push_back(range_dim);
    dims.push_back(azimuth_dim);
    dims.push_back(track_point_dim);

    lat_grid_var = add_var(nc_out, "lat", ncDouble, dims);
    lon_grid_var = add_var(nc_out, "lon", ncDouble, dims);
    valid_time_var = add_var(
        nc_out, "valid_time", ncUint64, track_point_dim);
}

////////////////////////////////////////////////////////////////////////

static void write_nc(const ConcatString& field_name,
    const DataPlane& dp, FieldType field_type) {
    ConcatString var_name, var_str;
    NcVar nc_var;

}

////////////////////////////////////////////////////////////////////////

bool read_data_plane(VarInfo* info, DataPlane& dp,
    Met2dDataFile* mtddf, const ConcatString& filename) {

    bool status = mtddf->data_plane(*info, dp);

    if(!status) {
        mlog << Warning << "\nread_data_plane() -> "
             << info->magic_str()
             << "not found in file: " << filename
             << "\n\n";
        return false;
    }

    return(true);
}

////////////////////////////////////////////////////////////////////////
