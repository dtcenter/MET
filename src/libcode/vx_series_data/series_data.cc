// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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

bool get_series_entry(int i_series, VarInfo* data_info,
        const StringArray& search_files, const GrdFileType type,
        DataPlane& dp, Grid& grid,
        bool error_out, bool print_warning,
        bool search_all_files) {
   int i;
   bool found;

   mlog << Debug(3)
        << "Processing series entry " << i_series + 1 << ": "
        << data_info->magic_time_str() << "\n";

   // Initialize
   dp.clear();

   // Search for data, beginning with the i_series index
   for(i=0,found=false; i<search_files.n(); i++) {

      int i_cur = (i_series + i) % search_files.n();

      found = read_single_entry(data_info, search_files[i_cur],
                                type, dp, grid, print_warning);

      // Break out of the loop if data was found or not searching all files
      if(found || !search_all_files) break;

   } // end for i

   // Error out if not found and specified
   if(!found && error_out) {
      mlog << Error << "\nget_series_entry() -> "
           << "Could not find data for " << data_info->magic_time_str()
           << " in file list:\n" << write_css(search_files) << "\n\n";
      exit(1);
   }

   return(found);
}

////////////////////////////////////////////////////////////////////////

bool read_single_entry(VarInfo* info, const ConcatString& filename,
        const GrdFileType type, DataPlane& dp, Grid& grid,
        bool print_warning) {

   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile* mtddf = (Met2dDataFile*) 0;

   // Check that file exists
   if(!file_exists(filename.c_str())) {
      if(print_warning) {
         mlog << Warning << "\nread_single_entry() -> "
              << "File does not exist: " << filename << "\n\n";
      }
      return(false);
   }

   // Open data file
   mtddf = mtddf_factory.new_met_2d_data_file(filename.c_str(), type);

   // Attempt to read gridded data
   // TODO: Should we pass print_warning as an option to data_plane?
   bool found = mtddf->data_plane(*info, dp);

   // Store grid
   if(found) grid = mtddf->grid();

   // Cleanup
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }

   return(found);
}

////////////////////////////////////////////////////////////////////////
