// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "series_data.h"

////////////////////////////////////////////////////////////////////////

StringArray parse_file_list(const StringArray& a,
    const GrdFileType type) {

    Met2dDataFileFactory mtddf_factory;
    Met2dDataFile *mtddf = (Met2dDataFile*) 0;
    StringArray list;

    // Check for empty list
    if(a.n_elements() == 0) {
        mlog << Error << "\nparse_file_list() -> "
             << "empty list!\n\n";
        exit(1);
    }

    // Attempt read of first file
    mtddf = mtddf_factory.new_met_2d_data_file(
        a[0].c_str(), type);

    // If successful store list
    if(mtddf) {
        list.add(a);
        delete mtddf;
        mtddf = (Met2dDataFile*) 0;
    } else { // Otherwise read a list of files
        if(a.n_elements() != 1) {
            mlog << Error << "\nparse_file_list() -> "
                 << "more than one file list!\n\n";
            exit(1);
        }
        list = parse_ascii_file_list(a[0].c_str());
    }

    if(mlog.verbosity_level() >= 3) {
        for(int i = 0; i < list.n_elements(); i++) {
            mlog << Debug(3) << list[i] << "\n";
        }
    }
    return list;
}

////////////////////////////////////////////////////////////////////////

void get_series_data(int i_series, VarInfo* data_info, DataPlane& dp) {

}

////////////////////////////////////////////////////////////////////////

void get_series_entry(int i_series, VarInfo* data_info,
    const StringArray& search_files, const GrdFileType type,
    StringArray& found_files, DataPlane& dp, Grid& grid) {

    mlog << Debug(2)
         << "Processing series entry " << i_series + 1 << ": "
         << data_info->name() << ": " << data_info->magic_str() << "\n";

    ConcatString filename;
    filename = search_files[i_series];

    read_single_entry(data_info, filename, type, dp, grid);
}

////////////////////////////////////////////////////////////////////////

bool read_single_entry(VarInfo* info, const ConcatString& filename,
    const GrdFileType type, DataPlane& dp, Grid& grid) {

    Met2dDataFileFactory mtddf_factory;
    Met2dDataFile* mtddf = (Met2dDataFile*) 0;

    // Check that file exists
    if(!file_exists(filename.c_str())) {
        mlog << Warning << "\nseries_data:read_single_entry() - >"
             << "File does not exist: " << filename << "\n\n";
        return(false);
    }

    // Open data file
    mtddf = mtddf_factory.new_met_2d_data_file(filename.c_str(), type);

    // Attempt to read gridded data
    bool found = mtddf->data_plane(*info, dp);

    // Store grid
    if(found) {
        grid = mtddf->grid();
    }

    return found;
}

////////////////////////////////////////////////////////////////////////
