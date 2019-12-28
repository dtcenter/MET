// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
   fcst_info = (VarInfo **)    0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GridDiagConfInfo::clear() {
   int i;

   // Initialize values
   model.clear();
   mask_grid_file.clear();
   mask_grid_name.clear();
   mask_poly_file.clear();
   mask_poly_name.clear();
   mask_area.clear();
   version.clear();

   // Clear fcst_info
   if(fcst_info) {
      for(i=0; i<n_fcst; i++)
         if(fcst_info[i]) { delete fcst_info[i]; fcst_info[i] = (VarInfo *) 0; }
      delete fcst_info; fcst_info = (VarInfo **) 0;
   }

   // Reset counts
   n_fcst = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void GridDiagConfInfo::read_config(const char *default_file_name,
                                   const char *user_file_name) {

   // Read the config file constants
   conf.read(replace_path(config_const_filename).c_str());

   // Read the default config file
   conf.read(default_file_name);

   // Read the user-specified config file
   conf.read(user_file_name);

   return;
}

////////////////////////////////////////////////////////////////////////

void GridDiagConfInfo::process_config(GrdFileType ftype,
                                            GrdFileType otype) {
   int i, n;
   ConcatString s;
   StringArray sa;
   VarInfoFactory info_factory;
   Dictionary *fdict = (Dictionary *) 0;
   Dictionary i_fdict;
   BootInfo boot_info;
   map<STATLineType,StringArray>::iterator it;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Initialize
   clear();

   // Conf: version
   version = parse_conf_version(&conf);

   // Conf: model
   model = parse_conf_string(&conf, conf_key_model);

   // Conf: fcst.field
   fdict = conf.lookup_array(conf_key_fcst_field);

   // Determine the number of fields (name/level) to be verified
   n_fcst = parse_conf_n_vx(fdict);

   // Check for empty fcst
   if(n_fcst == 0) {
      mlog << Error << "\nGridDiagConfInfo::process_config() -> "
           << "the \"fcst\" settings may not be empty.\n\n";
      exit(1);
   }

   // Allocate space based on the number of verification tasks
   fcst_info = new VarInfo * [n_fcst];

   // Initialize pointers
   for(i=0; i<n_fcst; i++) fcst_info[i] = (VarInfo *) 0;

   // Parse the fcst field information
   for(i=0; i<n_fcst; i++) {

      // Allocate new VarInfo objects
      fcst_info[i] = info_factory.new_var_info(ftype);

      // Get the current dictionaries
      i_fdict = parse_conf_i_vx_dict(fdict, i);

      // Set the current dictionaries
      fcst_info[i]->set_dict(i_fdict);

      // Dump the contents of the current VarInfo
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed forecast field number " << i+1 << ":\n";
         fcst_info[i]->dump(cout);
      }

      // No support for wind direction
      if(fcst_info[i]->is_wind_direction()) {
         mlog << Error << "\nGridDiagConfInfo::process_config() -> "
              << "the wind direction field may not be verified "
              << "using grid_diag.\n\n";
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

   // Parse out the masking grid
   if(mask_grid_file.length() > 0) {
      mlog << Debug(3)
           << "Processing grid mask: " << mask_grid_file << "\n";
      parse_grid_mask(mask_grid_file, grid, mask_grid, mask_grid_name);
      apply_mask(mask_area, mask_grid);
   }

   // Parse out the masking polyline
   if(mask_poly_file.length() > 0) {
      mlog << Debug(3)
           << "Processing poly mask: " << mask_poly_file << "\n";
      parse_poly_mask(mask_poly_file, grid, mask_poly, mask_poly_name);
      apply_mask(mask_area, mask_poly);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
