// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

void get_series_entry(int i_series, VarInfo* data_info,
        const StringArray& search_files, const GrdFileType type,
        DataPlane& dp, Grid& grid) {

   mlog << Debug(3)
        << "Processing series entry " << i_series + 1 << ": "
        << data_info->magic_str() << "\n";

   ConcatString filename;
   filename = search_files[i_series];

   // Initialize
   dp.clear();

   // Error out if requested data is not found in the i-th file
   if(!read_single_entry(data_info, filename, type, dp, grid)) {
      mlog << Error << "\nget_series_entry() -> "
           << "Could not find data for " << data_info->magic_str()
           << " in file: " << filename << "\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool read_single_entry(VarInfo* info, const ConcatString& filename,
        const GrdFileType type, DataPlane& dp, Grid& grid) {

   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile* mtddf = (Met2dDataFile*) 0;

   // Check that file exists
   if(!file_exists(filename.c_str())) {
       mlog << Warning << "\nread_single_entry() -> "
            << "File does not exist: " << filename << "\n\n";
       return(false);
   }

   // Open data file
   mtddf = mtddf_factory.new_met_2d_data_file(filename.c_str(), type);

   // Attempt to read gridded data
   bool found = mtddf->data_plane(*info, dp);

   // Store grid
   if(found) grid = mtddf->grid();

   // Cleanup
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }

   return(found);
}

////////////////////////////////////////////////////////////////////////
