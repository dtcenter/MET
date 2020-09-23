// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <cmath>

#include "grid_diag_conf_info.h"

#include "vx_data2d_factory.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class GridDiagConfInfo
//
////////////////////////////////////////////////////////////////////////

GridDiagConfInfo::GridDiagConfInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

GridDiagConfInfo::~GridDiagConfInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void GridDiagConfInfo::init_from_scratch() {

   // Initialize pointers
   data_info = (VarInfo **) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GridDiagConfInfo::clear() {

   // Initialize values
   desc.clear();
   mask_grid_file.clear();
   mask_grid_name.clear();
   mask_poly_file.clear();
   mask_poly_name.clear();
   mask_area.clear();
   version.clear();

   // Clear data_info
   if(data_info) {
      for(int i=0; i<n_data; i++)
        if(data_info[i]) {
           delete data_info[i];
           data_info[i] = (VarInfo *) 0;
        }
      delete data_info;
      data_info = (VarInfo **) 0;
   }

   // Reset counts
   n_data = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void GridDiagConfInfo::read_config(const char *default_file_name,
                                   const char *user_file_name) {

   // Read the config file constants
   conf.read(replace_path(config_const_filename).c_str());

   // Read the default config file
   conf.read(default_file_name);

   // Read the user config file
   conf.read(user_file_name);

   return;
}

////////////////////////////////////////////////////////////////////////

void GridDiagConfInfo::set_n_data() {
   Dictionary *dict = (Dictionary *) 0;

   // Conf: data.field
   dict = conf.lookup_array(conf_key_data_field);

   // Determine the number of fields (name/level) to be processed
   n_data = parse_conf_n_vx(dict);

   // Check for empty data
   if(n_data == 0) {
      mlog << Error << "\nGridDiagConfInfo::set_n_data() -> "
          << "the \"data.field\" array can't be empty!\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

void GridDiagConfInfo::process_config(vector<GrdFileType> file_types) {
   ConcatString s;
   StringArray sa;
   VarInfoFactory info_factory;
   Dictionary *dict = (Dictionary *) 0;
   Dictionary i_dict;
   GrdFileType file_type;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Conf: version
   version = parse_conf_version(&conf);

   // Conf: desc
   desc = parse_conf_string(&conf, conf_key_desc);

   // Conf: data.field
   dict = conf.lookup_array(conf_key_data_field);

   // Allocate space based on the number of verification tasks
   data_info = new VarInfo * [n_data];

   // Initialize pointers
   for(int i=0; i<n_data; i++) data_info[i] = (VarInfo *) 0;

   // Parse the data field information
   for(int i=0; i<n_data; i++) {

      // Determine the file type
      file_type = (file_types.size() > 1 ?
                   file_types[i] : file_types[0]);

      // Allocate new VarInfo objects
      data_info[i] = info_factory.new_var_info(file_type);

      // Get the current dictionaries
      i_dict = parse_conf_i_vx_dict(dict, i);

      // Set the current dictionaries
      data_info[i]->set_dict(i_dict);

      // Dump the contents of the current VarInfo
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed data field number " << i+1 << ":\n";
         data_info[i]->dump(cout);
      }

      // Make sure that n_bin and range have been specified
      if(data_info[i]->range().n() != 2 ||
         data_info[i]->n_bins()    <= 0) {
         mlog << Error << "\nGridDiagConfInfo::process_config() -> "
              << "each \"data.field\" entry must include an entry for "
              << "the number of bins (" << conf_key_n_bins
              << ") and the range of the data (" << conf_key_range_flag
              << ").\n\n";
         exit(1);
      }

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void GridDiagConfInfo::process_masks(const Grid &grid) {
   MaskPlane mask_grid, mask_poly;
   ConcatString name;

   mlog << Debug(2)
        << "Processing masking regions.\n";

   // Initialize the mask to all points on
   mask_area.set_size(grid.nx(), grid.ny(), true);

   // Conf: mask.grid
   mask_grid_file = conf.lookup_string(conf_key_mask_grid);

   // Conf: mask.poly
   mask_poly_file = conf.lookup_string(conf_key_mask_poly);

   // Parse the masking grid
   if(mask_grid_file.length() > 0) {
      mlog << Debug(3)
           << "Processing grid mask: " << mask_grid_file << "\n";
      parse_grid_mask(mask_grid_file, grid, mask_grid, mask_grid_name);
      apply_mask(mask_area, mask_grid);
   }

   // Parse the masking polyline
   if(mask_poly_file.length() > 0) {
      mlog << Debug(3)
           << "Processing poly mask: " << mask_poly_file << "\n";
      parse_poly_mask(mask_poly_file, grid, mask_poly, mask_poly_name);
      apply_mask(mask_area, mask_poly);
   }

   // Report the size of the mask
   mlog << Debug(3)
        << "Including " << mask_area.count() << " of the " << grid.nxy()
        << " grid points in the analysis.\n";

   return;
}

////////////////////////////////////////////////////////////////////////
