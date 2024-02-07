// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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

////////////////////////////////////////////////////////////////////////

static bool read_single_entry(VarInfo*, const ConcatString&, const GrdFileType,
                              DataPlane&, Grid&);

static bool read_all_entries(vector<VarInfo*>&, const ConcatString&, const GrdFileType,
                             vector<DataPlane>&, Grid&);

////////////////////////////////////////////////////////////////////////

bool get_series_entry(int i_series, VarInfo* data_info,
        const StringArray& search_files, const GrdFileType type,
        DataPlane& dp, Grid& grid,
        bool error_out, bool print_warning) {
   int i;
   bool found;

   // Save the log print warning state
   bool save_print_warning_state = mlog.print_warning();

   // Suppress log warnings, if requested
   if(!print_warning) mlog.set_print_warning(false);

   mlog << Debug(3)
        << "Processing series entry " << i_series + 1 << ": "
        << data_info->magic_time_str() << "\n";

   // Initialize
   dp.clear();

   // Search for data, beginning with the i_series index
   for(i=0,found=false; i<search_files.n(); i++) {

      int i_cur = (i_series + i) % search_files.n();

      found = read_single_entry(data_info, search_files[i_cur],
                                type, dp, grid);

      // Break out of the loop if data was found
      if(found) break;

   } // end for i

   // Error out if not found and specified
   if(!found && error_out) {
      mlog << Error << "\nget_series_entry() -> "
           << "Could not find data for " << data_info->magic_time_str()
           << " in file list:\n" << write_css(search_files) << "\n\n";
      exit(1);
   }

   // Restore warnings to their original state
   mlog.set_print_warning(save_print_warning_state);

   return(found);
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

bool get_series_entries(int i_series, vector<VarInfo*> &vi_list,
        const StringArray &search_files, const GrdFileType type,
        vector<DataPlane> &dp_list, Grid &grid,
        bool error_out, bool print_warning) {
   int i;
   bool found;

   // Check for at least one field requested
   if(vi_list.size() <= 0) return(false);

   // Save the log print warning state
   bool save_print_warning_state = mlog.print_warning();

   // Suppress log warnings, if requested
   if(!print_warning) mlog.set_print_warning(false);

   mlog << Debug(3)
        << "Processing series entry " << i_series + 1 << ": "
        << vi_list[0]->magic_time_str() << "\n";

   // Initialize
   dp_list.clear();

   // Search for data, beginning with the i_series index
   for(i=0,found=false; i<search_files.n(); i++) {

      int i_cur = (i_series + i) % search_files.n();

      found = read_all_entries(vi_list, search_files[i_cur],
                               type, dp_list, grid);

      // Break out of the loop if data was found
      if(found) break;

   } // end for i

   // Error out if not found and specified
   if(!found && error_out) {
      mlog << Error << "\nget_series_entries() -> "
           << "Could not find data for " << vi_list[0]->magic_time_str()
           << " in file list:\n" << write_css(search_files) << "\n\n";
      exit(1);
   }

   // Restore warnings to their original state
   mlog.set_print_warning(save_print_warning_state);

   return(found);
}

////////////////////////////////////////////////////////////////////////

bool read_all_entries(vector<VarInfo*> &vi_list, const ConcatString &filename,
        const GrdFileType type, vector<DataPlane> &dp_list, Grid &grid) {

   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile* mtddf = (Met2dDataFile*) 0;

   // Check that file exists
   if(!file_exists(filename.c_str())) {
      mlog << Warning << "\nread_all_entries() -> "
           << "File does not exist: " << filename << "\n\n";
      return(false);
   }

   // Open data file
   mtddf = mtddf_factory.new_met_2d_data_file(filename.c_str(), type);

   // Attempt to read gridded data
   int n_valid = mtddf->data_planes(vi_list, dp_list);

   // Store grid
   if(n_valid > 0) grid = mtddf->grid();

   // Cleanup
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }

   return(n_valid > 0);
}

////////////////////////////////////////////////////////////////////////
