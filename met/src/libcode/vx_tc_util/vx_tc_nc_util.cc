// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include "vx_tc_nc_util.h"

////////////////////////////////////////////////////////////////////////

void write_tc_tracks(NcFile* nc_out,
    const NcDim& track_point_dim,
    const TrackInfoArray& tracks) {

    TrackInfo track = tracks[0];
    StringArray track_lines = track.track_lines();

    NcDim track_line_dim = add_dim(nc_out, "track_line", track_lines.n());

    NcVar track_lines_var = nc_out->addVar(
        "TrackLines", ncString, track_line_dim);

    NcVar track_lat_var = nc_out->addVar(
        "Lat", ncDouble, track_point_dim);
    add_att(&track_lat_var, "long_name", "Track Point Latitude");
    add_att(&track_lat_var, "units", "degrees_east");
    add_att(&track_lat_var, "standard_name", "latitude_track");
    NcVar track_lon_var = nc_out->addVar(
        "Lon", ncDouble, track_point_dim);
    add_att(&track_lon_var, "long_name", "Track Point Longitude");
    add_att(&track_lon_var, "units", "degrees_north");
    add_att(&track_lon_var, "standard_name", "longitude_track");
    NcVar track_mrd_var = nc_out->addVar(
        "RMW", ncDouble, track_point_dim);
    add_att(&track_mrd_var, "long_name", "Radius of Maximum Winds");
    add_att(&track_mrd_var, "units", "nautical_miles");
    add_att(&track_mrd_var, "standard_name", "radius_max_wind");

    double* track_lat_data = new double[track.n_points()];
    double* track_lon_data = new double[track.n_points()];
    double* track_mrd_data = new double[track.n_points()];

    for(int i = 0; i < track.n_points(); i++) {
        mlog << Debug(4) << track[i].serialize() << "\n";
        track_lat_data[i] = track[i].lat();
        track_lon_data[i] = track[i].lon();
        track_mrd_data[i] = track[i].mrd();
    }

    vector<size_t> offsets;
    vector<size_t> counts;

    mlog << Debug(2) << "Writing " << track_lines.n() << " track lines.\n";

    for(int i = 0; i < track_lines.n(); i++) {
        offsets.clear();
        offsets.push_back(i);
        counts.clear();
        counts.push_back(1);
        string line = track_lines[i];
        mlog << Debug(3) << line << "\n";
        const char* str = line.c_str();
        track_lines_var.putVar(offsets, counts, &str);
    }

    offsets.clear();
    offsets.push_back(0);

    counts.clear();
    counts.push_back(track.n_points());

    track_lat_var.putVar(offsets, counts, track_lat_data);
    track_lon_var.putVar(offsets, counts, track_lon_data);
    track_mrd_var.putVar(offsets, counts, track_mrd_data);

    delete[] track_lat_data;
    delete[] track_lon_data;
    delete[] track_mrd_data;
}

////////////////////////////////////////////////////////////////////////

set<string> get_pressure_level_strings(
    map<string, vector<string> > variable_levels) {

    set<string> pressure_level_strings;

    for (map<string, vector<string> >::iterator i = variable_levels.begin();
        i != variable_levels.end(); ++i) {
        vector<string> levels = variable_levels[i->first];
        for (int j = 0; j < levels.size(); j++) {
            string label = levels[j].substr(0, 1);
            if (label == "P") {
                pressure_level_strings.insert(levels[j]);
            }
        }
    }

    return pressure_level_strings;
}

////////////////////////////////////////////////////////////////////////

set<double> get_pressure_levels(
    map<string, vector<string> > variable_levels) {

    set<double> pressure_levels;

    for (map<string, vector<string> >::iterator i = variable_levels.begin();
        i != variable_levels.end(); ++i) {
        vector<string> levels = variable_levels[i->first];
        for (int j = 0; j < levels.size(); j++) {
            string label = levels[j].substr(0, 1);
            double level = atof(levels[j].substr(1).c_str());
            if (label == "P") {
                pressure_levels.insert(level);
            }
        }
    }

    return pressure_levels;
}

////////////////////////////////////////////////////////////////////////

set<double> get_pressure_levels(
    set<string> pressure_level_strings) {

    set<double> pressure_levels;

    for (set<string>::iterator i = pressure_level_strings.begin();
        i != pressure_level_strings.end(); ++i) {

        string level_str = *i;
        double level = atof(level_str.substr(1).c_str());
        pressure_levels.insert(level);
    }

    return pressure_levels;
}

////////////////////////////////////////////////////////////////////////

map<double, int> get_pressure_level_indices(
    set<double> pressure_levels) {

    map<double, int> pressure_level_indices;

    int k = pressure_levels.size() - 1;

    for (set<double>::iterator i = pressure_levels.begin();
        i != pressure_levels.end(); ++i) {

        double level = *i;
        pressure_level_indices[level] = k; 
        k--;
    }

    return pressure_level_indices;
}

////////////////////////////////////////////////////////////////////////

map<string, int> get_pressure_level_indices(
    set<string> pressure_level_strings, set<double> pressure_levels) {

    map<string, int> pressure_level_indices;

    map<double, int> indices_from_levels
        = get_pressure_level_indices(pressure_levels);

    for (set<string>::iterator i = pressure_level_strings.begin();
        i != pressure_level_strings.end(); ++i) {

        string level_str = *i;
        double level = atof(level_str.substr(1).c_str());
        pressure_level_indices[level_str] = indices_from_levels[level];
    }

    return pressure_level_indices;
}

////////////////////////////////////////////////////////////////////////

void def_tc_pressure(NcFile* nc_out,
    const NcDim& pressure_dim, set<double> pressure_levels) {

    NcVar pressure_var;

    double* pressure_data = new double[pressure_levels.size()];

    // Define variable
    pressure_var = nc_out->addVar("pressure", ncDouble, pressure_dim);

    // Set attributes
    add_att(&pressure_var, "long_name", "pressure");
    add_att(&pressure_var, "units", "millibars");
    add_att(&pressure_var, "standard_name", "pressure");
    add_att(&pressure_var, "_FillValue", bad_data_double);

    // Extract pressure coordinates
    int k = pressure_levels.size() - 1;
    for (set<double>::iterator i = pressure_levels.begin();
        i != pressure_levels.end(); ++i) {
        pressure_data[k] = *i;
        k--;
    }

    put_nc_data(&pressure_var, &pressure_data[0]);

    // Cleanup
    if(pressure_data) { delete [] pressure_data; pressure_data = (double *) 0; }

    return;
}

////////////////////////////////////////////////////////////////////////

void def_tc_range_azimuth(NcFile* nc_out,
    const NcDim& range_dim, const NcDim& azimuth_dim,
    const TcrmwGrid& grid, double rmw_scale) {

    NcVar range_var;
    NcVar azimuth_var;

    double* range_data = new double[grid.range_n()];
    double* azimuth_data = new double[grid.azimuth_n()];

    // Define variables
    range_var = nc_out->addVar("range", ncDouble, range_dim);
    azimuth_var = nc_out->addVar("azimuth", ncDouble, azimuth_dim);

    // Set attributes
    add_att(&range_var, "long_name", "range");
    add_att(&range_var, "units", "fraction of RMW");
    add_att(&range_var, "standard_name", "range");
    add_att(&range_var, "_FillValue", bad_data_double);

    add_att(&azimuth_var, "long_name", "azimuth");
    add_att(&azimuth_var, "units", "degrees_clockwise_from_north");
    add_att(&azimuth_var, "standard_name", "azimuth");
    add_att(&azimuth_var, "_FillValue", bad_data_double);

    // Compute grid coordinates
    for (int i = 0; i < grid.range_n(); i++) {
        range_data[i] = i * rmw_scale;
    }
    for (int j = 0; j < grid.azimuth_n(); j++) {
        azimuth_data[j] = j * grid.azimuth_delta_deg();
    }

    // Write coordinates
    put_nc_data(&range_var, &range_data[0]);
    put_nc_data(&azimuth_var, &azimuth_data[0]);

    // Cleanup
    if(range_data)   { delete [] range_data;   range_data   = (double *) 0; }
    if(azimuth_data) { delete [] azimuth_data; azimuth_data = (double *) 0; }

    return;
}

////////////////////////////////////////////////////////////////////////

void def_tc_lat_lon_time(NcFile* nc_out,
    const NcDim& range_dim, const NcDim& azimuth_dim,
    const NcDim& track_point_dim,
    NcVar& lat_var, NcVar& lon_var, NcVar& valid_time_var) {

    vector<NcDim> dims;
    dims.push_back(range_dim);
    dims.push_back(azimuth_dim);
    dims.push_back(track_point_dim);

    lat_var = nc_out->addVar("lat", ncDouble, dims);
    lon_var = nc_out->addVar("lon", ncDouble, dims);
    valid_time_var = nc_out->addVar("valid_time", ncUint64,
        track_point_dim);

    // Set attributes
    add_att(&lat_var, "long_name", "latitude");
    add_att(&lat_var, "units", "degrees_north");
    add_att(&lat_var, "standard_name", "latitude");

    add_att(&lon_var, "long_name", "longitude");
    add_att(&lon_var, "units", "degrees_east");
    add_att(&lon_var, "standard_name", "longitude");

    add_att(&valid_time_var, "long_name", "valid_time");
    add_att(&valid_time_var, "units", "yyyymmddhh");
    add_att(&valid_time_var, "standard_name", "valid_time");
}

////////////////////////////////////////////////////////////////////////

void write_tc_valid_time(NcFile* nc_out,
    const int& i_point, const NcVar& var,
    const long& valid_time) {

    vector<size_t> offsets;
    vector<size_t> counts;

    offsets.clear();
    offsets.push_back(i_point);

    counts.clear();
    counts.push_back(1);

    var.putVar(offsets, counts, &valid_time);
}

////////////////////////////////////////////////////////////////////////

void def_tc_variables(NcFile* nc_out,
    map<string, vector<string> > variable_levels,
    map<string, string> variable_long_names,
    map<string, string> variable_units,
    const NcDim& range_dim, const NcDim& azimuth_dim,
    const NcDim& pressure_dim, const NcDim& track_point_dim,
    map<string, NcVar>& data_vars) {

    vector<NcDim> dims;
    dims.push_back(range_dim);
    dims.push_back(azimuth_dim);
    dims.push_back(track_point_dim);

    vector<NcDim> dims_3d;
    dims_3d.push_back(range_dim);
    dims_3d.push_back(azimuth_dim);
    dims_3d.push_back(pressure_dim);
    dims_3d.push_back(track_point_dim);

    for (map<string, vector<string> >::iterator i = variable_levels.begin();
        i != variable_levels.end(); ++i) {

        NcVar data_var;
        string var_name = i->first;
        vector<string> levels = variable_levels[i->first];
        string long_name = variable_long_names[i->first];
        string units = variable_units[i->first];

        if (levels.size() > 1) {
            data_var = nc_out->addVar(
                var_name, ncDouble, dims_3d);
            add_att(&data_var, "long_name", long_name);
            add_att(&data_var, "units", units);
            add_att(&data_var, "_FillValue", bad_data_double);
        } else {
            data_var = nc_out->addVar(
                var_name, ncDouble, dims);
            add_att(&data_var, "long_name", long_name);
            add_att(&data_var, "units", units);
            add_att(&data_var, "_FillValue", bad_data_double);
        }
        data_vars[var_name] = data_var;
    }
}

////////////////////////////////////////////////////////////////////////

void def_tc_data(NcFile* nc_out,
    const NcDim& range_dim, const NcDim& azimuth_dim,
    const NcDim& track_point_dim,
    NcVar& data_var, VarInfo* data_info) {

    vector<NcDim> dims;
    dims.push_back(range_dim);
    dims.push_back(azimuth_dim);
    dims.push_back(track_point_dim);

    ConcatString var_name = data_info->name_attr();
    var_name.add("_");
    var_name.add(data_info->level_attr());

    data_var = nc_out->addVar(
        var_name, ncDouble, dims);

    // Set attributes
    add_att(&data_var, "long_name", data_info->long_name_attr());
    add_att(&data_var, "units", data_info->units_attr());
    add_att(&data_var, "_FillValue", bad_data_double);
}

////////////////////////////////////////////////////////////////////////

void def_tc_data_3d(NcFile* nc_out,
    const NcDim& range_dim, const NcDim& azimuth_dim,
    const NcDim& pressure_dim, const NcDim& track_point_dim,
    NcVar& data_var, VarInfo* data_info) {

    vector<NcDim> dims;
    dims.push_back(range_dim);
    dims.push_back(azimuth_dim);
    dims.push_back(pressure_dim);
    dims.push_back(track_point_dim);

    data_var = nc_out->addVar(
        data_info->name_attr(), ncDouble, dims);

    // Set attributes
    add_att(&data_var, "long_name", data_info->long_name_attr());
    add_att(&data_var, "units", data_info->units_attr());
    add_att(&data_var, "_FillValue", bad_data_double);
}

////////////////////////////////////////////////////////////////////////

void def_tc_azi_mean_data(NcFile* nc_out,
    const NcDim& range_dim,
    const NcDim& track_point_dim,
    NcVar& data_var, VarInfo* data_info) {

    vector<NcDim> dims;
    dims.push_back(range_dim);
    dims.push_back(track_point_dim);

    ConcatString var_name = data_info->name_attr();
    var_name.add("_");
    var_name.add(data_info->level_attr());
    var_name.add("_azi_mean");

    data_var = nc_out->addVar(var_name, ncDouble, dims);

    // Set attributes
    add_att(&data_var, "long_name", data_info->long_name_attr());
    add_att(&data_var, "units", data_info->units_attr());
    add_att(&data_var, "_FillValue", bad_data_double);
}

////////////////////////////////////////////////////////////////////////

void write_tc_data(NcFile* nc_out, const TcrmwGrid& grid,
    const int& i_point, const NcVar& var, const double* data) {

    vector<size_t> offsets;
    vector<size_t> counts;

    offsets.clear();
    offsets.push_back(0);
    offsets.push_back(0);
    offsets.push_back(i_point);

    counts.clear();
    counts.push_back(grid.range_n());
    counts.push_back(grid.azimuth_n());
    counts.push_back(1);

    var.putVar(offsets, counts, data);
}

////////////////////////////////////////////////////////////////////////

void write_tc_data_rev(NcFile* nc_out, const TcrmwGrid& grid,
    const int& i_point, const NcVar& var, const double* data) {

    vector<size_t> offsets;
    vector<size_t> counts;

    double* data_rev;

    offsets.clear();
    offsets.push_back(0);
    offsets.push_back(0);
    offsets.push_back(i_point);

    counts.clear();
    counts.push_back(grid.range_n());
    counts.push_back(grid.azimuth_n());
    counts.push_back(1);

    data_rev = new double[
        grid.range_n() * grid.azimuth_n()];

    for(int ir = 0; ir < grid.range_n(); ir++) {
        for(int ia = 0; ia < grid.azimuth_n(); ia++) {
            int i = ir * grid.azimuth_n() + ia;
            int i_rev = (grid.range_n() - ir - 1) * grid.azimuth_n() + ia;
            data_rev[i_rev] = data[i];
        }
    }

    var.putVar(offsets, counts, data_rev);

    delete[] data_rev;
}

////////////////////////////////////////////////////////////////////////

void write_tc_azi_mean_data(NcFile* nc_out, const TcrmwGrid& grid,
    const int& i_point, const NcVar& var, const double* data) {

    vector<size_t> offsets;
    vector<size_t> counts;

    double* data_rev;
    double* data_azi_mean;

    offsets.clear();
    offsets.push_back(0);
    offsets.push_back(i_point);

    counts.clear();
    counts.push_back(grid.range_n());
    counts.push_back(1);

    data_rev = new double[
        grid.range_n() * grid.azimuth_n()];
    data_azi_mean = new double[grid.range_n()];

    for(int ir = 0; ir < grid.range_n(); ir++) {
        for(int ia = 0; ia < grid.azimuth_n(); ia++) {
            int i = ir * grid.azimuth_n() + ia;
            int i_rev = (grid.range_n() - ir - 1) * grid.azimuth_n() + ia;
            data_rev[i_rev] = data[i];
            data_azi_mean[ir] += data_rev[i];
        }
    }

    for(int ir = 0; ir < grid.range_n(); ir++) {
        data_azi_mean[ir] /= grid.azimuth_n();
    }

    var.putVar(offsets, counts, data_azi_mean);

    delete[] data_rev;
    delete[] data_azi_mean;
}

////////////////////////////////////////////////////////////////////////

extern void write_tc_pressure_level_data(
    NcFile* nc_out, const TcrmwGrid& grid,
    map<string, int> pressure_level_indices, const string& level_str,
    const int& i_point, const NcVar& var, const double* data) {

    vector<size_t> offsets;
    vector<size_t> counts;

    vector<size_t> offsets_3d;
    vector<size_t> counts_3d;

    double* data_rev;

    int i_level = pressure_level_indices[level_str];

    offsets.clear();
    offsets.push_back(0);
    offsets.push_back(0);
    offsets.push_back(i_point);

    offsets_3d.clear();
    offsets_3d.push_back(0);
    offsets_3d.push_back(0);
    offsets_3d.push_back(i_level);
    offsets_3d.push_back(i_point);

    counts.clear();
    counts.push_back(grid.range_n());
    counts.push_back(grid.azimuth_n());
    counts.push_back(1);

    counts_3d.clear();
    counts_3d.push_back(grid.range_n());
    counts_3d.push_back(grid.azimuth_n());
    counts_3d.push_back(1);
    counts_3d.push_back(1);

    data_rev = new double[
        grid.range_n() * grid.azimuth_n()];

    for(int ir = 0; ir < grid.range_n(); ir++) {
        for(int ia = 0; ia < grid.azimuth_n(); ia++) {
            int i = ir * grid.azimuth_n() + ia;
            int i_rev = (grid.range_n() - ir - 1) * grid.azimuth_n() + ia;
            data_rev[i_rev] = data[i];
        }
    }

    // string label = level_str.substr(0, 1);
    // if (label == "P") {
    //     var.putVar(offsets_3d, counts_3d, data_rev);
    // } else {
    //     var.putVar(offsets, counts, data_rev);
    // }
    var.putVar(offsets_3d, counts_3d, data_rev);

    delete[] data_rev;
}

////////////////////////////////////////////////////////////////////////
