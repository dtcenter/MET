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

void write_nc_tracks(const ConcatString& track_nc_file,
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

    // nc_out->close();
}

////////////////////////////////////////////////////////////////////////

void write_nc_range_azimuth(NcFile* nc_out,
    const NcDim& range_dim, const NcDim& azimuth_dim,
    const TcrmwGrid& grid) {

    NcVar range_var;
    NcVar azimuth_var;

    float* range_data = new float[grid.range_n()];
    float* azimuth_data = new float[grid.azimuth_n()];

    // Define variables
    range_var = nc_out->addVar("range", ncFloat, range_dim);
    azimuth_var = nc_out->addVar("azimuth", ncFloat, azimuth_dim);

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
