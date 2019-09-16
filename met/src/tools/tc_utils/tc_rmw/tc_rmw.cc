// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

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

#include "tcrmw_grid.h"
#include "series_data.h"

#include "vx_grid.h"
#include "vx_regrid.h"
#include "vx_tc_util.h"
#include "vx_nc_util.h"
#include "vx_tc_nc_util.h"
#include "vx_data2d_nc_met.h"
#include "vx_util.h"
#include "vx_log.h"

#include "met_file.h"

////////////////////////////////////////////////////////////////////////

static void usage();
static void process_command_line(int, char**);
static void process_decks();
static void process_adecks(TrackInfoArray&);
static void process_bdecks(TrackInfoArray&);
static void get_atcf_files(const StringArray&,
    const StringArray&, StringArray&, StringArray&);
static void process_track_files(const StringArray&,
    const StringArray&, TrackInfoArray&, bool, bool);
static void set_adeck(const StringArray&);
static void set_bdeck(const StringArray&);
static void set_atcf_source(const StringArray&,
    StringArray&, StringArray&);
static void set_data_files(const StringArray&);
static void set_config(const StringArray&);
static void set_out_dir(const StringArray&);
static void set_out_prefix(const StringArray&);
static void set_logfile(const StringArray&);
static void set_verbosity(const StringArray&);
static void setup_grid();
static void setup_nc_file();
static void build_outfile_name(const ConcatString&,
    const char*, ConcatString&);
static void compute_lat_lon(TcrmwGrid&,
    double*, double*);
static void wind_ne_to_ra(TcrmwGrid&,
    DataPlane&, DataPlane&,
    double*, double*, double*, double*);
static void process_fields(const TrackInfoArray&);

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
         << "\t-data file_1 ... file_n | data_file_list\n"
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

void process_command_line(int argc, char **argv) {

    CommandLine cline;
    ConcatString default_config_file;

    // Default output directory
    out_dir = replace_path(default_out_dir);

    // Default output prefix
    out_prefix = replace_path(default_out_prefix);

    // Parse command line into tokens
    cline.set(argc, argv);

    // Set usage function
    cline.set_usage(usage);

    // Add function calls for arguments
    cline.add(set_data_files, "-data",  -1);
    cline.add(set_adeck,      "-adeck", -1);
    cline.add(set_bdeck,      "-bdeck", -1);
    cline.add(set_config,     "-config", 1);
    cline.add(set_out_dir,    "-out",    1);
    cline.add(set_out_prefix, "-prefix", -1);
    cline.add(set_logfile,    "-log",    1);
    cline.add(set_verbosity,  "-v",      1);

    // Parse command line
    cline.parse();

    // Check number of arguments
    // if(cline.n() != 1) usage();

    // data_file = cline[0];

    // Create default config file name
    default_config_file = replace_path(default_config_filename);

    // List config files
    mlog << Debug(1)
         << "Config File Default: " << default_config_file << "\n"
         << "Config File User: " << config_file << "\n";

    // Read config files
    conf_info.read_config(default_config_file.c_str(),
                          config_file.c_str());


    // Get data file type from config
    ftype = parse_conf_file_type(conf_info.Conf.lookup_dictionary(
        conf_key_data));

    // Process the configuration
    conf_info.process_config(ftype);

    // Search for files
    data_files
        = parse_file_list(data_files, ftype);

    return;
}

////////////////////////////////////////////////////////////////////////

void process_decks() {
    // Process ADECK files
    TrackInfoArray adeck_tracks;
    process_adecks(adeck_tracks);

    process_fields(adeck_tracks);

    // nc_out->close();
}

////////////////////////////////////////////////////////////////////////

void process_adecks(TrackInfoArray& adeck_tracks) {
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

    // ConcatString adeck_track_file("adeck.nc");
    // write_tc_tracks(adeck_track_file, adeck_tracks);
}

////////////////////////////////////////////////////////////////////////

void process_bdecks(TrackInfoArray& bdeck_tracks) {
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

void process_track_files(const StringArray& files,
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
            if(tracks.add(line, false, check_anly)) {
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

void set_data_files(const StringArray& a) {
    data_files = a;
}

////////////////////////////////////////////////////////////////////////

void set_config(const StringArray& a) {
    config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_out_dir(const StringArray& a) {
    out_dir = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_out_prefix(const StringArray& a) {
    out_prefix = a[0];
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

void setup_grid() {

    grid_data.name = "TCRMW";
    grid_data.range_n = conf_info.n_range;
    grid_data.azimuth_n = conf_info.n_azimuth;
    grid_data.range_max_km = conf_info.max_range_km;

    tcrmw_grid.set_from_data(grid_data);
    grid.set(grid_data);
}

////////////////////////////////////////////////////////////////////////

void setup_nc_file() {
    VarInfo* data_info = (VarInfo*) 0;

    // Create output NetCDF file name
    // out_nc_file.add("tc_rmw_out.nc");
    build_outfile_name(out_prefix, "out.nc", out_nc_file);

    mlog << Debug(1) << out_nc_file << "\n";

    // Create NetCDF file
    nc_out = open_ncfile(out_nc_file.c_str(), true);

    if(IS_INVALID_NC_P(nc_out)) {
        mlog << Error << "\nsetup_nc_file() -> "
             << "trouble opening output NetCDF file "
             << out_nc_file << "\n\n";
        exit(1);
    }

    mlog << Debug(4) << tcrmw_grid.range_n() << "\n";
    mlog << Debug(4) << tcrmw_grid.azimuth_n() << "\n";

    // Define dimensions
    range_dim = add_dim(nc_out, "range", (long) tcrmw_grid.range_n());
    azimuth_dim = add_dim(nc_out, "azimuth", (long) tcrmw_grid.azimuth_n());
    track_point_dim = add_dim(nc_out, "track_point", NC_UNLIMITED);

    // Define range and azimuth dimensions
    def_tc_range_azimuth(nc_out, range_dim, azimuth_dim, tcrmw_grid);

    // Define latitude and longitude arrays
    def_tc_lat_lon_time(nc_out, range_dim, azimuth_dim,
        track_point_dim, lat_arr_var, lon_arr_var, valid_time_var);

    // Find all variable levels, long names, and units
    for(int i_var = 0; i_var < conf_info.get_n_data(); i_var++) {
        // Get VarInfo
        data_info = conf_info.data_info[i_var];
        mlog << Debug(4) << "data_info->name():"
             << data_info->name().c_str() << "\n";
        mlog << Debug(4) << "data_info->level_name():"
             << data_info->level_name().c_str() << "\n";
        variable_levels[data_info->name()].push_back(
            data_info->level_name());
        variable_long_names[data_info->name()]
            = data_info->long_name();
        variable_units[data_info->name()]
            = data_info->units();
    }

    // Define pressure levels
    pressure_levels = get_pressure_levels(variable_levels);
    pressure_dim = add_dim(nc_out, "pressure", pressure_levels.size());
    def_tc_pressure(nc_out, pressure_dim, pressure_levels);

    // Define variables
    for(int i_var = 0; i_var < conf_info.get_n_data(); i_var++) {
        // Get VarInfo
        data_info = conf_info.data_info[i_var];
        def_tc_data(nc_out, range_dim, azimuth_dim,
            track_point_dim, data_var, data_info);
        data_vars.push_back(data_var);
        // def_tc_azi_mean_data(nc_out, range_dim,
        //     track_point_dim, data_var, data_info);
        // azi_mean_data_vars.push_back(data_var);
    }

    def_tc_variables(nc_out,
        variable_levels, variable_long_names, variable_units,
        range_dim, azimuth_dim, pressure_dim, track_point_dim,
        data_3d_vars);
}

////////////////////////////////////////////////////////////////////////

void build_outfile_name(
    const ConcatString& prefix, const char* suffix, ConcatString& str) {

    // Append output directory and program name
    str << cs_erase << out_dir << "/" << program_name;

    // Append prefix
    str << prefix << "_";

    // Append suffix
    str << suffix;

    return;
}

////////////////////////////////////////////////////////////////////////

void compute_lat_lon(TcrmwGrid& tcrmw_grid,
    double* lat_arr, double* lon_arr) {

    // Compute lat and lon coordinate arrays
    for(int ir = 0; ir < tcrmw_grid.range_n(); ir++) {
        for(int ia = 0; ia < tcrmw_grid.azimuth_n(); ia++) {
            double lat, lon;
            int i = ir * tcrmw_grid.azimuth_n() + ia;
            tcrmw_grid.range_azi_to_latlon(
                ir * tcrmw_grid.range_delta_km(),
                ia * tcrmw_grid.azimuth_delta_deg(),
                    lat, lon);
            lat_arr[i] = lat;
            lon_arr[i] = - lon;
        }
    }
}

////////////////////////////////////////////////////////////////////////

void wind_ne_to_ra(TcrmwGrid& tcrmw_grid,
    DataPlane& u_dp, DataPlane& v_dp,
    double* lat_arr, double* lon_arr,
    double* wind_r_arr, double* wind_a_arr) {

    // Transform (u, v) to (radial, azimuthal)
    for(int ir = 0; ir < tcrmw_grid.range_n(); ir++) {
        for(int ia = 0; ia < tcrmw_grid.azimuth_n(); ia++) {
            int i = ir * tcrmw_grid.azimuth_n() + ia;
            double lat = lat_arr[i];
            double lon = - lon_arr[i];
            double u = u_dp.data()[i];
            double v = v_dp.data()[i];
            double wind_r;
            double wind_a;
            tcrmw_grid.wind_ne_to_ra(
                lat, lon, u, v, wind_r, wind_a);
            // tcrmw_grid.wind_ne_to_ra_conventional(
            //     lat, lon, u, v, wind_r, wind_a);
            wind_r_arr[i] = wind_r;
            wind_a_arr[i] = wind_a;
        }
    }
}

////////////////////////////////////////////////////////////////////////

void process_fields(const TrackInfoArray& tracks) {

    VarInfo *data_info = (VarInfo *) 0;
    DataPlane data_dp;

    // Define latitude and longitude arrays
    lat_arr = new double[
        tcrmw_grid.range_n() * tcrmw_grid.azimuth_n()];
    lon_arr = new double[
        tcrmw_grid.range_n() * tcrmw_grid.azimuth_n()];

    // Assume single track for now
    TrackInfo track = tracks[0];

    // Loop over track points
    for(int i_point = 0; i_point < track.n_points(); i_point++) {

        TrackPoint point = track[i_point];
        unixtime valid_time = point.valid();
        long valid_yyyymmddhh = unix_to_long_yyyymmddhh(valid_time);
        mlog << Debug(4)
             << "(" << point.lat() << ", " << point.lon() << ")\n";

        // Set grid center
        grid_data.lat_center = point.lat();
        grid_data.lon_center = - point.lon(); // internal sign change
        // RMW is same as mrd()
        grid_data.range_max_km = conf_info.rmw_scale *
            point.mrd() * nm_to_km * conf_info.n_range;
        tcrmw_grid.clear();
        tcrmw_grid.set_from_data(grid_data);
        grid.clear();
        grid.set(grid_data);

        // Compute lat and lon coordinate arrays
        compute_lat_lon(tcrmw_grid, lat_arr, lon_arr);

        // Write coordinate arrays
        write_tc_data(nc_out, tcrmw_grid, i_point, lat_arr_var, lat_arr);
        write_tc_data(nc_out, tcrmw_grid, i_point, lon_arr_var, lon_arr);

        // Write valid time
        write_tc_valid_time(nc_out, i_point,
            valid_time_var, valid_yyyymmddhh);

        for(int i_var = 0; i_var < conf_info.get_n_data(); i_var++) {
            // Get variable info
            data_info = conf_info.data_info[i_var];
            get_series_entry(i_point, data_info,
                data_files, ftype, found_data_files,
                data_dp, latlon_arr);
            // Check data range
            double data_min, data_max;
            data_dp.data_range(data_min, data_max);
            mlog << Debug(4) << "data_min:" << data_min << "\n";
            mlog << Debug(4) << "data_max:" << data_max << "\n";
            // Regrid data
            data_dp = met_regrid(data_dp,
                latlon_arr, grid, data_info->regrid());
            data_dp.data_range(data_min, data_max);
            mlog << Debug(4) << "data_min:" << data_min << "\n";
            mlog << Debug(4) << "data_max:" << data_max << "\n";
            // Write data
            write_tc_data_rev_range(nc_out, tcrmw_grid, i_point,
                data_vars[i_var], data_dp.data());
            // write_tc_azi_mean_data(nc_out, tcrmw_grid, i_point,
            //     azi_mean_data_vars[i_var], data_dp.data());
        }
    } // Close loop over track points

    delete[] lat_arr;
    delete[] lon_arr;
}

////////////////////////////////////////////////////////////////////////
