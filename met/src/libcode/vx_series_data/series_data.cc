// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "series_data.h"

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
        list = parse_ascii_file_list(a[0].c_str());
    }

    return list;

}

////////////////////////////////////////////////////////////////////////
