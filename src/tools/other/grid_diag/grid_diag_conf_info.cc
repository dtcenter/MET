// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

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

using namespace std;

////////////////////////////////////////////////////////////////////////
//
//  Code for struct GridDiagNcOutInfo
//
////////////////////////////////////////////////////////////////////////

GridDiagNcOutInfo::GridDiagNcOutInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void GridDiagNcOutInfo::clear() {

   set_all_true();

   return;
}

////////////////////////////////////////////////////////////////////////

bool GridDiagNcOutInfo::all_false() const {

   bool status = do_hist1d || do_hist2d || do_info_theory;

   return !status;
}

////////////////////////////////////////////////////////////////////////

void GridDiagNcOutInfo::set_all_false() {

   do_hist1d      = false;
   do_hist2d      = false;
   do_info_theory = false;

   return;
}

////////////////////////////////////////////////////////////////////////

void GridDiagNcOutInfo::set_all_true() {

   do_hist1d      = true;
   do_hist2d      = true;
   do_info_theory = true;

   return;
}

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

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GridDiagConfInfo::clear() {

   // Initialize values
   desc.clear();
   mask_name.clear();
   mask_map.clear();
   version.clear();

   // Clear data_info
   for(auto &info : data_info) {
      if(info) { delete info; info = nullptr; }
   }
   data_info.clear();
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

   // Conf: data.field
   auto dict = conf.lookup_array(conf_key_data_field);

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
   Dictionary i_dict;
   GrdFileType file_type;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Conf: version
   version = parse_conf_version(&conf);

   // Conf: desc
   desc = parse_conf_string(&conf, conf_key_desc);

   // Conf: data.field
   Dictionary *dict = conf.lookup_array(conf_key_data_field);

   // Allocate space based on the number of verification tasks
   data_info.resize(n_data, nullptr);

   // Parse the data field information
   for(int i=0; i<n_data; i++) {

      // Determine the file type
      file_type = (file_types.size() > 1 ?
                   file_types[i] : file_types[0]);

      // Allocate new VarInfo objects
      data_info[i] = VarInfoFactory::new_var_info(file_type);

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

   // Conf: output_flag
   parse_output_flag();

   return;
}

////////////////////////////////////////////////////////////////////////

void GridDiagConfInfo::parse_output_flag() {

   // Lookup the output_flag dictionary 
   auto e = conf.lookup(conf_key_output_flag);

   if(!e) {
      mlog << Error << "\nGridDiagConfInfo::parse_output_flag() -> "
           << "lookup failed for key \"" << conf_key_output_flag
           << "\"\n\n";
      exit(1);
   }

   // Process as boolean
   if(e->type() == BooleanType) {
      if(e->b_value()) nc_info.set_all_true();
      else             nc_info.set_all_false();
      return;
   }

   // Otherwise, it should be a dictionary
   if(e->type() != DictionaryType) {
      mlog << Error << "\nGridDiagConfInfo::parse_output_flag() -> "
           << "bad type (" << configobjecttype_to_string(e->type())
           << ") for key \"" << conf_key_output_flag << "\"\n\n";
      exit(1);
   }

   // Parse the various entries
   auto d = e->dict_value();

   nc_info.do_hist1d      = d->lookup_bool(conf_key_hist1d_flag);
   nc_info.do_hist2d      = d->lookup_bool(conf_key_hist2d_flag);
   nc_info.do_info_theory = d->lookup_bool(conf_key_info_theory_flag);

   return;
}

////////////////////////////////////////////////////////////////////////

void GridDiagConfInfo::process_masks(const Grid &grid) {
   MaskPlane mp;
   ConcatString name;

   mlog << Debug(2)
        << "Processing masking regions.\n";

   // Conf: mask.grid
   StringArray mask_grid_sa(conf.lookup_string_array(conf_key_mask_grid));

   // Conf: mask.poly
   StringArray mask_poly_sa(conf.lookup_string_array(conf_key_mask_poly));

   // Check for all masking regions being empty 
   if(mask_grid_sa.all_empty() && mask_poly_sa.all_empty()) {
      mlog << Debug(3)
           << "Adding the \"" << full_domain_str << "\" domain since "
           << "no grid or polyline masking regions were specified.\n";
      mask_grid_sa.add(full_domain_str);
   }

   // Parse the masking grids
   for(int i=0; i<mask_grid_sa.n(); i++) {
      if(mask_grid_sa[i].empty()) continue;
      parse_grid_mask(mask_grid_sa[i], grid, mp, name);
      mask_name.add(name);
      mask_map[name] = mp;
      mlog << Debug(3)
           << "Processing grid mask \"" << mask_grid_sa[i]
           << "\" which includes " << mp.count() << " of the "
           << grid.nxy() << " grid points.\n";
   }

   // Parse the masking polyline
   for(int i=0; i<mask_poly_sa.n(); i++) {
      if(mask_poly_sa[i].empty()) continue;
      parse_poly_mask(mask_poly_sa[i], grid, mp, name);
      mask_name.add(name);
      mask_map[name] = mp;
      mlog << Debug(3)
           << "Processing poly mask \"" << mask_poly_sa[i]
           << "\" which includes " << mp.count() << " of the "
           << grid.nxy() << " grid points.\n";
   }

   // Report the number of masks
   mlog << Debug(3)
        << "Applying " << (int) mask_map.size() << " masking regions.\n";

   return;
}

////////////////////////////////////////////////////////////////////////
