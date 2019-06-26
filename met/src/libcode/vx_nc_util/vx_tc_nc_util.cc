// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include "vx_tc_nc_util.h"

////////////////////////////////////////////////////////////////////////

void write_tc_tracks(const ConcatString& track_nc_file,
    const TrackInfoArray& tracks) {

    mlog << Debug(2) << "Writing " << track_nc_file << "\n";

    NcFile* nc_out = open_ncfile(track_nc_file.c_str(), true);

    NcDim track_point_dim = add_dim(nc_out, "track_point", NC_UNLIMITED);

    if (IS_INVALID_NC_P(nc_out)) {
        mlog << Error << "\nwrite_nc_tracks() -> "
             << "unable to open NetCDF file " 
             << track_nc_file << "\n\n";
        exit(1);
    }

    NcVar track_lat_var = nc_out->addVar(
        "track_lat", ncFloat, track_point_dim);
    NcVar track_lon_var = nc_out->addVar(
        "track_lon", ncFloat, track_point_dim);

    int offset = 0;

    for(int j = 0; j < tracks.n_tracks(); j++) {

        TrackInfo track = tracks[j];

        mlog << Debug(2) << "Writing track " << j << "\n";

        float* track_lat_data = new float[track.n_points()];
        float* track_lon_data = new float[track.n_points()];

        for(int i = 0; i < track.n_points(); i++) {
            track_lat_data[i] = track[i].lat();
            track_lon_data[i] = track[i].lon();
        }

        put_nc_data(&track_lat_var, track_lat_data,
            track.n_points(), offset);
        put_nc_data(&track_lon_var, track_lon_data,
            track.n_points(), offset);

        delete[] track_lat_data;
        delete[] track_lon_data;

        offset += track.n_points();
    }

    // nc_out->close();
}

////////////////////////////////////////////////////////////////////////

void def_tc_range_azimuth(NcFile* nc_out,
    const NcDim& range_dim, const NcDim& azimuth_dim,
    const TcrmwGrid& grid) {

    NcVar range_var;
    NcVar azimuth_var;

    float* range_data = new float[grid.range_n()];
    float* azimuth_data = new float[grid.azimuth_n()];

    // Define variables
    range_var = nc_out->addVar("range", ncFloat, range_dim);
    azimuth_var = nc_out->addVar("azimuth", ncFloat, azimuth_dim);

    // Set attributes
    add_att(&range_var, "long_name", "range");
    add_att(&range_var, "units", "kilometers");
    add_att(&range_var, "standard_name", "range");
    add_att(&azimuth_var, "long_name", "azimuth");
    add_att(&azimuth_var, "units", "degrees_clockwise_from_north");
    add_att(&azimuth_var, "standard_name", "azimuth");

    // Compute grid coordinates
    for (int i = 0; i < grid.range_n(); i++) {
        range_data[i] = i * grid.range_delta_km();
    }
    for (int j = 0; j < grid.azimuth_n(); j++) {
        azimuth_data[j] = j * grid.azimuth_delta_deg();
    }

    // Write coordinates
    put_nc_data(&range_var, &range_data[0]);
    put_nc_data(&azimuth_var, &azimuth_data[0]);

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

void write_tc_grid(NcFile* nc_out, const TcrmwGrid& grid,
    const int& i_point, const NcVar& var, double* data) {

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

void def_tc_data(NcFile* nc_out,
    const NcDim& range_dim, const NcDim& azimuth_dim,
    const NcDim& track_point_dim,
    NcVar& data_var, VarInfo* data_info) {

    vector<NcDim> dims;
    dims.push_back(range_dim);
    dims.push_back(azimuth_dim);
    dims.push_back(track_point_dim);

    data_var = nc_out->addVar(
        data_info->name(), ncDouble, dims);

    // Set attributes
    add_att(&data_var, "long_name", data_info->long_name());
    add_att(&data_var, "units", data_info->units());
}

////////////////////////////////////////////////////////////////////////

void write_tc_data(NcFile* nc_out,
    const int& i_point, const NcVar& data_var,
    double* data) {

}

////////////////////////////////////////////////////////////////////////
