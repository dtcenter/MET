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

#include "vx_nc_util.h"
#include "vx_tc_util.h"
#include "vx_util.h"
#include "vx_log.h"

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
static void normalize_stats();
static void write_stats();
static void clean_up();
static void process_track_file(const ConcatString&,
    TrackInfoArray&);
static bool is_keeper(const ATCFLineBase*);
static void filter_tracks(TrackInfoArray&);
static void read_nc_tracks(NcFile*);

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

    normalize_stats();

    write_stats();

    clean_up();

    return(0);
}

////////////////////////////////////////////////////////////////////////

void usage() {

    cout << "\n*** Model Evaluation Tools (MET" << met_version
         << ") ***\n\n"
         << "Usage: " << program_name << "\n"
         << "\t-data file_1 ... file_n | data_file_list\n"
         << "\t-config file\n"
         << "\t-out file\n"
         << "\t[-log file]\n"
         << "\t[-v level]\n\n"

         << "\twhere\t\"-data file_1 ... file_n | data_file_list\" "
         << "is the NetCDF output of TC-RMW to be processed or an "
         << "ASCII file containing a list of files (required).\n"

         << "\t\t\"-config file\" is the RMWAnalysisConfig to be used "
         << "(required).\n"

         << "\t\t\"-out file\" is the NetCDF output file to be written "
         << "(required).\n"

         << "\t\t\"-log file\" outputs log messages to the specified "
         << "file (optional).\n"

         << "\t\t\"-v level\" overrides the default level of logging ("
         << mlog.verbosity_level() << ") (optional).\n\n" << flush;

    exit(1);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {

    CommandLine cline;
    ConcatString default_config_file;

    // Default output directory
    out_dir = replace_path(default_out_dir);

    // Print usage statement for no arguments
    if(argc <= 1) usage();

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
    out_file = a[0];
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
    nc_in = open_ncfile(data_files[0].c_str());
    mlog << Debug(1) << "Reading dimensions from"
         << data_files[0] << "\n";

    // Get dimension sizes
    get_dim(nc_in, "range", n_range, true);
    range_dim = get_nc_dim(nc_in, "range");

    get_dim(nc_in, "azimuth", n_azimuth, true);
    azimuth_dim = get_nc_dim(nc_in, "azimuth");

    if (get_dim(nc_in, "height", n_level)) {
        mlog << Debug(1) << "Found height vertical dimension.\n";
        level_dim = get_nc_dim(nc_in, "height");
        level_name = "height";
    } else if (get_dim(nc_in, "pressure", n_level)) {
        mlog << Debug(1) << "Found pressure vertical dimension.\n";
        level_dim = get_nc_dim(nc_in, "pressure");
        level_name = "pressure";
    } else {
        mlog << Warning << "No vertical dimension found.\n";
    }

    mlog << Debug(2)
         << "(n_range, n_azimuth, n_level) = ("
         << n_range << ", " << n_azimuth << ", " << n_level << ")\n";

    // Get dimension coordinates
    vector<size_t> start;
    vector<size_t> count;
    start.push_back(0);

    ConcatString s;

    NcVar range_var = get_nc_var(nc_in, "range");
    count.clear();
    count.push_back(n_range);
    range_coord.resize(n_range);
    range_var.getVar(start, count, range_coord.data());

    NcVar azimuth_var = get_nc_var(nc_in, "azimuth");
    count.clear();
    count.push_back(n_azimuth);
    azimuth_coord.resize(n_azimuth);
    azimuth_var.getVar(start, count, azimuth_coord.data());
    get_att_value_string(&azimuth_var, "units", s);
    azimuth_units = s.string();

    NcVar level_var = get_nc_var(nc_in, level_name.c_str());
    count.clear();
    count.push_back(n_level);
    level_coord.resize(n_level);
    level_var.getVar(start, count, level_coord.data());
    get_att_value_string(&level_var, "units", s);
    level_units = s.string();

    // Read variable information
    for(int i_var = 0; i_var < conf_info.get_n_data(); i_var++) {
        data_names.push_back(conf_info.data_info[i_var]->name().string());
        NcVar var = get_nc_var(
            nc_in, conf_info.data_info[i_var]->name().c_str());
        int n_dim = get_dim_count(&var) - 1;
        data_n_dims.push_back(n_dim);
        ConcatString s;
        get_att_value_string(&var, "long_name", s);
        data_long_names.push_back(s.string());
        get_att_value_string(&var, "units", s);
        data_units.push_back(s.string());
    }

    // Initialize statistical data cube lists
    for(int i_var = 0; i_var < data_names.size(); i_var++) {

        // Size data cubes
        DataCube* data_count_2d = new DataCube();
        DataCube* data_count_3d = new DataCube();
        DataCube* data_mean_2d = new DataCube();
        DataCube* data_mean_3d = new DataCube();
        DataCube* data_stdev_2d = new DataCube();
        DataCube* data_stdev_3d = new DataCube();
        DataCube* data_max_2d = new DataCube();
        DataCube* data_max_3d = new DataCube();
        DataCube* data_min_2d = new DataCube();
        DataCube* data_min_3d = new DataCube();

        data_count_2d->set_size(n_range, n_azimuth, 1);
        data_count_3d->set_size(n_range, n_azimuth, n_level);
        data_mean_2d->set_size(n_range, n_azimuth, 1);
        data_mean_3d->set_size(n_range, n_azimuth, n_level);
        data_stdev_2d->set_size(n_range, n_azimuth, 1);
        data_stdev_3d->set_size(n_range, n_azimuth, n_level);
        data_max_2d->set_size(n_range, n_azimuth, 1);
        data_max_3d->set_size(n_range, n_azimuth, n_level);
        data_min_2d->set_size(n_range, n_azimuth, 1);
        data_min_3d->set_size(n_range, n_azimuth, n_level);

        data_count_2d->set_constant(0);
        data_count_3d->set_constant(0);
        data_mean_2d->set_constant(0);
        data_mean_3d->set_constant(0);
        data_stdev_2d->set_constant(0);
        data_stdev_3d->set_constant(0);
        data_max_2d->set_constant(-1.0e6);
        data_max_3d->set_constant(-1.0e6);
        data_min_2d->set_constant(1.0e6);
        data_min_3d->set_constant(1.0e6);

        if (data_n_dims[i_var] == 2) {
            data_counts.push_back(data_count_2d);
            data_means.push_back(data_mean_2d);
            data_stdevs.push_back(data_stdev_2d);
            data_mins.push_back(data_min_2d);
            data_maxs.push_back(data_max_2d);
        }
        if (data_n_dims[i_var] == 3) {
            data_counts.push_back(data_count_3d);
            data_means.push_back(data_mean_3d);
            data_stdevs.push_back(data_stdev_3d);
            data_mins.push_back(data_min_3d);
            data_maxs.push_back(data_max_3d);
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
    data_2d_sq.set_size(n_range, n_azimuth, 1);
    data_3d.set_size(n_range, n_azimuth, n_level);
    data_3d_sq.set_size(n_range, n_azimuth, n_level);

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
        nc_in = open_ncfile(data_files[i_file].c_str());

        get_dim(nc_in, "track_point", n_track_point, true);
        track_point_dim = get_nc_dim(nc_in, "track_point");
        mlog << Debug(1) << "Number of track points "
             << n_track_point << "\n";

        // Read track information
        read_nc_tracks(nc_in);

        // Filter tracks
        filter_tracks(adeck_tracks);

        if (adeck_tracks.n_tracks() > 0) {

            for(int i_var = 0; i_var < data_names.size(); i_var++) {
                NcVar var = get_nc_var(nc_in, data_names[i_var].c_str());
                mlog << Debug(2) << "Processing "
                     << data_names[i_var] << "\n";

                for(int i_track = 0; i_track < n_track_point; i_track++) {
                    if (data_n_dims[i_var] == 2) {
                        start_2d[2] = (size_t) i_track;
                        mlog << Debug(4) << data_names[i_var] << i_track << "\n";
                        var.getVar(start_2d, count_2d, data_2d.data());

                        // Update partial sums
                        data_2d_sq = data_2d;
                        data_2d_sq.square();
                        data_counts[i_var]->increment();
                        mlog << Debug(4) << i_track << " "
                             << data_counts[i_var]->data()[0] << "\n";
                        mlog << Debug(4) << i_track << " "
                             << data_2d.data()[0] << "\n";
                        data_means[i_var]->add_assign(data_2d);
                        mlog << Debug(4) << i_track << " "
                             << data_means[i_var]->data()[0] << "\n";
                        data_stdevs[i_var]->add_assign(data_2d_sq);
                        data_mins[i_var]->min_assign(data_2d);
                        mlog << Debug(4) << i_track << " "
                             << data_mins[i_var]->data()[0] << "\n";
                        data_maxs[i_var]->max_assign(data_2d);
                        mlog << Debug(4) << i_track << " "
                             << data_maxs[i_var]->data()[0] << "\n";
                    }
                    if (data_n_dims[i_var] == 3) {
                        mlog << Debug(4) << data_names[i_var] << i_track << "\n";
                        start_3d[3] = (size_t) i_track;
                        var.getVar(start_3d, count_3d, data_3d.data());

                        // Update partial sums
                        data_3d_sq = data_3d;
                        data_3d_sq.square();
                        data_counts[i_var]->increment();
                        data_means[i_var]->add_assign(data_3d);
                        data_stdevs[i_var]->add_assign(data_3d_sq);
                        data_mins[i_var]->min_assign(data_3d);
                        data_maxs[i_var]->max_assign(data_3d);
                    }
                } // end loop over track points
            } // end loop over variables
        } // end if have tracks
    } // end loop over files
}

////////////////////////////////////////////////////////////////////////

void normalize_stats() {

    for(int i_var = 0; i_var < data_names.size(); i_var++) {

        // Normalize
        data_means[i_var]->divide_assign(*data_counts[i_var]);
        data_stdevs[i_var]->divide_assign(*data_counts[i_var]);

        // Compute standard deviation
        DataCube data_mean_sq = *data_means[i_var];
        data_mean_sq.square();
        data_stdevs[i_var]->subtract_assign(data_mean_sq);
        data_stdevs[i_var]->square_root();
    }
}

////////////////////////////////////////////////////////////////////////

void write_stats() {

    // Create output file
    nc_out = open_ncfile(out_file.c_str(), true);

    mlog << Debug(1) << "Writing output file: " << out_file << "\n";

    // Define dimensions
    range_dim = add_dim(nc_out, "range", n_range);
    azimuth_dim = add_dim(nc_out, "azimuth", n_azimuth);
    level_dim = add_dim(nc_out, level_name, n_level);

    vector<NcDim> dims_2d;
    dims_2d.push_back(range_dim);
    dims_2d.push_back(azimuth_dim);

    vector<NcDim> dims_3d;
    dims_3d.push_back(range_dim);
    dims_3d.push_back(azimuth_dim);
    dims_3d.push_back(level_dim);

    NcVar range_var = nc_out->addVar("range", ncDouble, range_dim);
    NcVar azimuth_var = nc_out->addVar("azimuth", ncDouble, azimuth_dim);
    NcVar level_var = nc_out->addVar(level_name, ncDouble, level_dim);

    vector<size_t> offset;
    vector<size_t> count_range;
    vector<size_t> count_azimuth;
    vector<size_t> count_level;
    offset.push_back(0);
    count_range.push_back(n_range);
    count_azimuth.push_back(n_azimuth);
    count_level.push_back(n_level);

    for (int r = 0; r < n_range; r++) {
        range_coord[r] = r;
    }

    range_var.putVar(offset, count_range, range_coord.data());
    add_att(&range_var, "units", "RMW");

    azimuth_var.putVar(offset, count_azimuth, azimuth_coord.data());
    add_att(&azimuth_var, "units", azimuth_units);

    level_var.putVar(offset, count_level, level_coord.data());
    add_att(&level_var, "units", level_units);

    vector<size_t> offset_2d;
    vector<size_t> count_2d;
    vector<size_t> offset_3d;
    vector<size_t> count_3d;

    offset_2d.push_back(0);
    offset_2d.push_back(0);
    count_2d.push_back(n_range);
    count_2d.push_back(n_azimuth);

    offset_3d.push_back(0);
    offset_3d.push_back(0);
    offset_3d.push_back(0);
    count_3d.push_back(n_range);
    count_3d.push_back(n_azimuth);
    count_3d.push_back(n_level);

    for(int i_var = 0; i_var < data_names.size(); i_var++) {
        if (data_n_dims[i_var] == 2) {
            NcVar var_mean = nc_out->addVar(
                data_names[i_var] + "_mean",
                ncDouble, dims_2d);
            add_att(&var_mean, "long_name",
                data_long_names[i_var] + " Mean");
            add_att(&var_mean, "units", data_units[i_var]);
            var_mean.putVar(offset_2d, count_2d,
                data_means[i_var]->data());

            NcVar var_stdev = nc_out->addVar(
                data_names[i_var] + "_stdev",
                ncDouble, dims_2d);
            add_att(&var_stdev, "long_name",
                data_long_names[i_var] + " Standard Deviation");
            add_att(&var_stdev, "units", data_units[i_var]);
            var_stdev.putVar(offset_2d, count_2d,
                data_stdevs[i_var]->data());

            NcVar var_min = nc_out->addVar(
                data_names[i_var] + "_min",
                ncDouble, dims_2d);
            add_att(&var_min, "long_name",
                data_long_names[i_var] + " Minimum");
            add_att(&var_min, "units", data_units[i_var]);
            var_min.putVar(offset_2d, count_2d,
                data_mins[i_var]->data());

            NcVar var_max = nc_out->addVar(
                data_names[i_var] + "_max",
                ncDouble, dims_2d);
            add_att(&var_max, "long_name",
                data_long_names[i_var] + " Maximum");
            add_att(&var_max, "units", data_units[i_var]);
            var_max.putVar(offset_2d, count_2d,
                data_maxs[i_var]->data());
        }

        if (data_n_dims[i_var] == 3) {
            NcVar var_mean = nc_out->addVar(
                data_names[i_var] + "_mean",
                ncDouble, dims_3d);
            add_att(&var_mean, "long_name",
                data_long_names[i_var] + " Mean");
            add_att(&var_mean, "units", data_units[i_var]);
            var_mean.putVar(offset_3d, count_3d,
                data_means[i_var]->data());

            NcVar var_stdev = nc_out->addVar(
                data_names[i_var] + "_stdev",
                ncDouble, dims_3d);
            add_att(&var_stdev, "long_name",
                data_long_names[i_var] + " Standard Deviation");
            add_att(&var_stdev, "units", data_units[i_var]);
            var_stdev.putVar(offset_3d, count_3d,
                data_stdevs[i_var]->data());

            NcVar var_min = nc_out->addVar(
                data_names[i_var] + "_min",
                ncDouble, dims_3d);
            add_att(&var_min, "long_name",
                data_long_names[i_var] + " Minimum");
            add_att(&var_min, "units", data_units[i_var]);
            var_min.putVar(offset_3d, count_3d,
                data_mins[i_var]->data());

            NcVar var_max = nc_out->addVar(
                data_names[i_var] + "_max",
                ncDouble, dims_3d);
            add_att(&var_max, "long_name",
                data_long_names[i_var] + " Maximum");
            add_att(&var_max, "units", data_units[i_var]);
            var_max.putVar(offset_3d, count_3d,
                data_maxs[i_var]->data());
        }
    }

    nc_out->close();
}

////////////////////////////////////////////////////////////////////////

void clean_up() {

    // Delete allocated memory
    int i;
    for(i=0; i<data_counts.size(); i++) delete data_counts[i];
    for(i=0; i<data_means.size();  i++) delete data_means[i];
    for(i=0; i<data_stdevs.size(); i++) delete data_stdevs[i];
    for(i=0; i<data_mins.size();   i++) delete data_mins[i];
    for(i=0; i<data_maxs.size();   i++) delete data_maxs[i];
}

////////////////////////////////////////////////////////////////////////

void process_track_file(const ConcatString& filename,
                        TrackInfoArray& tracks) {

    int cur_read, cur_add;
    LineDataFile f;
    ConcatString cs;
    ATCFTrackLine line;

    // Initialize
    tracks.clear();

    // Open the file
    if(!f.open(filename.c_str())) {
        mlog << Error
             << "\nprocess_track_file() -> "
             << "unable to open file \""
             << filename << "\"\n\n";
        exit(1);
    }

    // Initialize counts
    cur_read = cur_add = 0;

    // Read each line in the file
    while(f >> line) {
        mlog << Debug(3) << line << "\n";

        // Increment the line counts
        cur_read++;

        if(!is_keeper(&line)) continue;

        // Attempt to add the current line to the TrackInfoArray
        if(tracks.add(line, false, false)) {
            cur_add++;
        }
    } // End while loop over lines

    // Close the file
    f.close();

    // Dump out the track information
    mlog << Debug(3)
         << "Identified " << tracks.n_tracks() << " track(s).\n";

    remove(adeck_source.c_str());

    return;
}

////////////////////////////////////////////////////////////////////////

bool is_keeper(const ATCFLineBase* line) {
    bool keep = true;

    // Check model
    if(conf_info.Model.n_elements() > 1 &&
        !conf_info.Model.has(line->technique()))
        keep = false;

    // Check storm id
    else if(conf_info.StormId.n_elements() > 1 &&
            !has_storm_id(conf_info.StormId, line->basin(),
                          line->cyclone_number(), line->warning_time()))
        keep = false;

    // Check basin
    else if(conf_info.Basin.n_elements() > 1 &&
            !conf_info.Basin.has(line->basin()))
        keep = false;

    // Check cyclone
    else if(conf_info.Cyclone.n_elements() > 1 &&
            !conf_info.Cyclone.has(line->cyclone_number()))
        keep = false;

    // Initialization time window
    else if((conf_info.InitBeg > 1 &&
             conf_info.InitBeg > line->warning_time()) ||
            (conf_info.InitEnd > 1 &&
             conf_info.InitEnd < line->warning_time()))
        keep = false;

    return keep;
}

////////////////////////////////////////////////////////////////////////

void filter_tracks(TrackInfoArray& tracks) {

    TrackInfoArray t = tracks;

    // Initialize
    tracks.clear();

    // Loop over tracks
    for(int i = 0; i < t.n_tracks(); i++) {
        // Valid time window
        if((conf_info.ValidBeg > 0 &&
            conf_info.ValidBeg > t[i].valid_min()) ||
           (conf_info.ValidEnd > 0 &&
            conf_info.ValidEnd < t[i].valid_max())) {
             mlog << Debug(4)
                  << "Discarding track " << i+1
                  << " for falling outside the "
                  << "valid time window: "
                  << unix_to_yyyymmdd_hhmmss(t[i].valid_min()) << " to "
                  << unix_to_yyyymmdd_hhmmss(t[i].valid_max()) << "\n";
            continue;
        }

        // Retain track
        tracks.add(t[i]);
    }
}

////////////////////////////////////////////////////////////////////////

void read_nc_tracks(NcFile* nc_in) {

    mlog << Debug(3) << adeck_source << "\n";

    ofstream f;
    f.open(adeck_source.c_str());

    adeck_tracks.clear();

    NcDim track_line_dim;
    get_dim(nc_in, "track_line", n_track_line, true);

    mlog << Debug(3) << "Number of track lines "
         << n_track_line << "\n";

    NcVar track_lines_var = get_nc_var(nc_in, "TrackLines");

    vector<size_t> counts;
    vector<size_t> offsets;

    for(int i = 0; i < n_track_line; i++) {
        offsets.clear();
        offsets.push_back(i);
        counts.clear();
        counts.push_back(1);

        char* track_line_str;
        track_lines_var.getVar(offsets, counts, &track_line_str);
        ConcatString track_line(track_line_str);
        mlog << Debug(3) << track_line << "\n";

        f << track_line << "\n";
    }
    f.close();

    adeck_tracks.clear();
    process_track_file(adeck_source, adeck_tracks);
}

////////////////////////////////////////////////////////////////////////
