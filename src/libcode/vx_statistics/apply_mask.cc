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
#include <iostream>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <regex.h>

#include "apply_mask.h"

#include "vx_data2d.h"
#include "vx_data2d_factory.h"
#include "vx_regrid.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static void process_poly_mask(const ConcatString &, const Grid &,
                              DataPlane &, ConcatString&);

////////////////////////////////////////////////////////////////////////

Grid parse_vx_grid(const RegridInfo info, const Grid *fgrid, const Grid *ogrid) {
   Grid vx_grid;

   // Check if regridding is disabled
   if(!info.enable) {

      // Check for a grid mismatch
      if(!(*fgrid == *ogrid)) {
         mlog << Error << "\nparse_vx_grid() -> "
              << "The forecast and observation grids do not match:\n"
              << fgrid->serialize() << " !=\n" << ogrid->serialize()
              << "\nSpecify regridding logic in the config file \"regrid\" section.\n\n";
         exit(1);
      }
      else {
         mlog << Debug(3)
              << "Use the matching forecast and observation grids.\n";
         vx_grid = *fgrid;
      }
   }

   // Otherwise, process regridding logic
   else {

      // Parse info.name as a white-space separated string
      StringArray sa;
      sa.parse_wsss(info.name);

      // Verify on the forecast grid
      if(info.field == FieldType_Fcst) {
         mlog << Debug(3)
              << "Use the forecast grid.\n";
         vx_grid = *fgrid;
      }
      // Verify on the observation grid
      else if(info.field == FieldType_Obs) {
         mlog << Debug(3)
              << "Use the observation grid.\n";
         vx_grid = *ogrid;
      }
      // Search for a named grid
      else if(sa.n_elements() == 1 && find_grid_by_name(info.name.c_str(), vx_grid)) {
         mlog << Debug(3)
              << "Use the grid named \"" << info.name << "\".\n";
      }
      // Parse grid definition
      else if(sa.n_elements() > 1 && parse_grid_def(sa, vx_grid)) {
         mlog << Debug(3)
              << "Use the grid defined by string \"" << info.name << "\".\n";
      }
      // Extract the grid from a gridded data file
      else {

         mlog << Debug(3)
              << "Use the grid defined by file \"" << info.name << "\".\n";

         Met2dDataFileFactory mtddf_factory;
         Met2dDataFile *mtddf = (Met2dDataFile *) 0;

         // Attempt to open the data file
         if(!(mtddf = mtddf_factory.new_met_2d_data_file(info.name.c_str()))) {
            mlog << Error << "\nparse_vx_grid() -> "
                 << "can't open file \"" << info.name << "\"\n\n";
            exit(1);
         }
         vx_grid = mtddf->grid();
         delete mtddf;
      }
   }

   mlog << Debug(3)
        << "Grid Definition: " << vx_grid.serialize() << "\n";

   return(vx_grid);
}

////////////////////////////////////////////////////////////////////////

void parse_grid_weight(const Grid &grid, const GridWeightType t,
                       DataPlane &wgt_dp) {
   int x, y;
   double w, lat, lon;

   // Initialize
   wgt_dp.clear();
   wgt_dp.set_size(grid.nx(), grid.ny());

   // Compute weight for each grid point
   for(x=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         if(t == GridWeightType_Cos_Lat) {
            grid.xy_to_latlon(x, y, lat, lon);
            w = cosd(lat);
         }
         else if(t == GridWeightType_Area) {
            w = grid.calc_area(x, y);
         }
         else {
            w = default_grid_weight;
         }

         // Store the current weight
         wgt_dp.set(w, x, y);

      } // end for y
   } // end for x

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_grid_mask(const ConcatString &mask_grid_str, const Grid &grid,
                     DataPlane &mask_dp, ConcatString &mask_name) {

   // Check for empty input string
   if(mask_grid_str.empty()) return;

   Grid mask_grid;

   mlog << Debug(4) << "parse_grid_mask() -> "
        << "parsing grid mask \"" << mask_grid_str << "\"\n";

   // Initialize the DataPlane masking object by turning all points on
   mask_dp.set_size(grid.nx(), grid.ny());
   mask_dp.set_constant(mask_on_value);

   // Store the name of the masking grid
   mask_name = mask_grid_str;

   //
   // Check to make sure that we're not using the full domain
   //
   if( full_domain_str != mask_grid_str) {

      //
      // Search for the grid name in the predefined grids
      //
      if(!find_grid_by_name(mask_name.c_str(), mask_grid)) {
         mlog << Error << "\nparse_grid_mask() -> "
              << "the mask_grid requested \"" << mask_grid_str
              << "\" is not defined.\n\n";
         exit(1);
      }

      apply_grid_mask(grid, mask_grid, mask_dp);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_grid_mask(const ConcatString &mask_grid_str, const Grid &grid,
                     MaskPlane &mask, ConcatString &mask_name) {

   // Check for empty input string
   if(mask_grid_str.empty()) return;

   DataPlane dp;

   parse_grid_mask(mask_grid_str, grid, dp, mask_name);

   mask = dp.mask_plane();

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_grid_mask(const ConcatString &mask_grid_str, Grid &grid) {

   // Check for empty input string
   if(mask_grid_str.empty()) return;

   Met2dDataFileFactory factory;
   Met2dDataFile * datafile = (Met2dDataFile *) 0;

   // First, try to find the grid by name.
   if(!find_grid_by_name(mask_grid_str.c_str(), grid)) {

      // If that doesn't work, try to open a data file.
      datafile = factory.new_met_2d_data_file(replace_path(mask_grid_str.c_str()).c_str());

      if(!datafile) {
        mlog << Error << "\nparse_grid_mask() -> "
             << "can't open data file \"" << mask_grid_str << "\"\n\n";
        exit(1);
      }

      // Store the data file's grid
      grid = datafile->grid();

      delete datafile; datafile = (Met2dDataFile *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// The mask_poly_str contains masking regions defined one of 3 ways:
//
// (1) An ASCII file containing a lat/lon polygon.
// (2) The NetCDF output of the gen_vx_mask tool.
// (3) A 2D data file, followed by a description of the field to be used,
//     and optionally, a threshold to be applied to the field.
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// Parse the specified mask and regrid to the input grid, if needed.
//
////////////////////////////////////////////////////////////////////////

void parse_poly_mask(const ConcatString &mask_poly_str, const Grid &grid,
                     DataPlane &mask_dp, ConcatString &mask_name) {

   // Check for empty input string
   if(mask_poly_str.empty()) return;

   ConcatString file_name, config_str;
   StringArray tokens;
   Grid mask_grid;

   mlog << Debug(4) << "parse_poly_mask() -> "
        << "parsing poly mask \"" << mask_poly_str << "\"\n";

   // Tokenize the input string
   tokens = mask_poly_str.split(poly_str_delim);
   file_name = replace_path(tokens[0].c_str());
   file_name.ws_strip();
   if(tokens.n_elements() > 1) config_str = tokens[1];

   // If not a 2D data file, process as a lat/lon polyline file
   if(!is_2d_data_file(file_name, config_str)) {
      process_poly_mask(file_name, grid, mask_dp, mask_name);
   }
   // Otherwise, process as a 2d data file
   else {

      parse_poly_2d_data_mask(mask_poly_str, mask_grid, mask_dp, mask_name);

      // Regrid, if necessary, using nearest neighbor.
      if(!(mask_grid == grid)) {

         RegridInfo ri;
         ri.enable = true;
         ri.method = InterpMthd_Nearest;
         ri.width  = 1;
         ri.shape  = GridTemplateFactory::GridTemplate_Square;

         mlog << Debug(2)
              << "Regridding mask grid to the verification grid using nearest "
              << "neighbor interpolation:\n"
              << mask_grid.serialize() << "!=\n" << grid.serialize() << "\n";

         mask_dp = met_regrid(mask_dp, mask_grid, grid, ri);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_poly_mask(const ConcatString &mask_poly_str, const Grid &grid,
                     MaskPlane &mask, ConcatString &mask_name) {

   // Check for empty input string
   if(mask_poly_str.empty()) return;

   DataPlane dp;

   parse_poly_mask(mask_poly_str, grid, dp, mask_name);

   mask = dp.mask_plane();

   return;
}

////////////////////////////////////////////////////////////////////////

void process_poly_mask(const ConcatString &file_name, const Grid &grid,
                       DataPlane &mask_dp, ConcatString &mask_name) {
   MaskPoly poly_mask;

   mlog << Debug(4) << "parse_poly_mask() -> "
        << "parsing poly mask file \"" << file_name << "\"\n";

   // Initialize the DataPlane masking object by turning all points on
   mask_dp.set_size(grid.nx(), grid.ny());
   mask_dp.set_constant(mask_on_value);

   // Initialize the masking polyline
   poly_mask.clear();

   // Parse out the polyline from the file
   poly_mask.load(file_name.c_str());

   // Store the name of the masking polyline
   mask_name = poly_mask.name();

   // Apply the polyline mask to the DataPlane object
   apply_poly_mask_latlon(poly_mask, grid, mask_dp);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Parse the poly mask information.  If it's a gridded data file,
// return the masking grid, mask plane, and the name of the mask.
// If not, return the polyline object.
//
////////////////////////////////////////////////////////////////////////

void parse_poly_mask(const ConcatString &mask_poly_str,
                     MaskPoly &mask_poly, Grid &mask_grid,
                     MaskPlane &mask_plane, ConcatString &mask_name) {

   // Check for empty input string
   if(mask_poly_str.empty()) return;

   ConcatString file_name, config_str;
   DataPlane mask_dp;
   StringArray tokens;

   mlog << Debug(4) << "parse_poly_mask() -> "
        << "parsing poly mask \"" << mask_poly_str << "\"\n";

   // Tokenize the input string
   tokens = mask_poly_str.split(poly_str_delim);
   file_name = replace_path(tokens[0].c_str());
   file_name.ws_strip();
   if(tokens.n_elements() > 1) config_str = tokens[1];

   // If not a 2D data file, process as a lat/lon polyline file
   if(!is_2d_data_file(file_name, config_str)) {
      mask_poly.load(file_name.c_str());
   }
   // Otherwise, process as a 2d data file
   else {
      parse_poly_2d_data_mask(mask_poly_str, mask_grid, mask_dp, mask_name);
      mask_plane = mask_dp.mask_plane();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_poly_2d_data_mask(const ConcatString &mask_poly_str,
                             Grid &mask_grid, DataPlane &mask_dp,
                             ConcatString &mask_name) {

   // Check for empty input string
   if(mask_poly_str.empty()) return;

   ConcatString file_name, thresh_str;
   StringArray tokens;
   SingleThresh st;
   bool append_level, append_thresh;

   // Tokenize the input string
   tokens = mask_poly_str.split(poly_str_delim);
   file_name = replace_path(tokens[0].c_str());
   file_name.ws_strip();

   // Initialize config object
   MetConfig config;
   config.read(replace_path(config_const_filename).c_str());

   // Parse the dictionary string
   if(tokens.n_elements() > 1) {
      append_level = true;
      config.read_string(tokens[1].c_str());
   }
   else {
      append_level = false;
      config.read_string(default_mask_dict);
   }

   // Parse the requested file type
   GrdFileType type = parse_conf_file_type(&config);

   // 2D Data file
   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;

   // VarInfo object
   VarInfoFactory info_factory;
   VarInfo *info = (VarInfo *) 0;

   // Open the data file
   mtddf = mtddf_factory.new_met_2d_data_file(file_name.c_str(), type);

   // If data file pointer is NULL, assume a lat/lon polyline file
   if(!mtddf) {
      mlog << Error << "\nparse_poly_2d_data_mask() -> "
           << "cannot read file \"" << file_name << "\"!\n\n";
      exit(1);
   }

   // Store the masking grid
   mask_grid = mtddf->grid();

   // Create a new VarInfo object
   info = info_factory.new_var_info(mtddf->file_type());

   // Set up the VarInfo object
   info->set_dict(config);

   // Extract the data plane from the input file
   mtddf->data_plane(*info, mask_dp);

   // Parse the threshold string
   if(tokens.n_elements() > 2) {
      append_thresh = true;
      thresh_str = tokens[2];
   }
   else {
      append_thresh = false;
      thresh_str = default_mask_thresh;
   }
   thresh_str.ws_strip();

   // Apply threshold to the data plane and convert to MaskPlane
   st.set(thresh_str.c_str());
   mask_dp.threshold(st);

   // Store the mask name
   mask_name = info->name();

   // Append level info
   if(append_level) mask_name << "_" << info->level_name();

   // Append threshold info
   if(append_thresh) mask_name << st.get_str();

   // Clean up
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }
   if(info)  { delete info;  info  = (VarInfo *)       0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void apply_grid_mask(const Grid &grid, const Grid &mask_grid, DataPlane &dp) {
   int x, y;
   double lat, lon, mask_x, mask_y;
   int in_count, out_count;

   //
   // If the masking grid's dimensions are zero, there's nothing to be done
   //
   if(mask_grid.nx() == 0 || mask_grid.ny() == 0) {
      mlog << Warning << "\nvoid apply_grid_mask() -> "
           << "no grid masking applied since the masking grid "
           << "specified is empty.\n\n";
      return;
   }

   //
   // For each point of the original grid, convert it to (lat, lon) and then
   // to an (x, y) point on the mask grid. If the (x, y) is within the
   // bounds of the mask grid, retain it's value.  Otherwise, mask it out.
   //
   in_count = out_count = 0;
   for(x=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         grid.xy_to_latlon(x, y, lat, lon);
         mask_grid.latlon_to_xy(lat, lon, mask_x, mask_y);

         if(mask_x < 0 || mask_x >= mask_grid.nx() ||
            mask_y < 0 || mask_y >= mask_grid.ny()) {
            out_count++;
            dp.set(mask_off_value, x, y);
         }
         else {
            in_count++;
         }
      } // for y
   } // for x

   return;
}

////////////////////////////////////////////////////////////////////////

void apply_poly_mask_latlon(const MaskPoly &poly, const Grid &grid, DataPlane &dp) {
   int x, y;
   double lat, lon;
   int in_count, out_count;

   //
   // If the polyline contains less than 3 points, no masking region is
   // defined and there is nothing to do
   //
   if(poly.n_points() < 3) {
      mlog << Warning << "\nvoid apply_poly_mask_latlon() -> "
           << "no polyline masking applied since the masking polyline "
           << "contains fewer than 3 points.\n\n";
      return;
   }

   //
   // Mask out any grid points whose corresponding lat/lon coordinates are
   // not inside the masking lat/lon polygon.
   //
   in_count = out_count = 0;
   for(x=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         grid.xy_to_latlon(x, y, lat, lon);

         if(!(poly.latlon_is_inside(lat, lon))) {
            out_count++;
            dp.set(mask_off_value, x, y);
         }
         else {
            in_count++;
         }
      } // for y
   } // for x

   return;
}


///////////////////////////////////////////////////////////////////////////////

DataPlane parse_geog_data(Dictionary *dict, const Grid &vx_grid,
                          const char *fcst_file) {
   DataPlane dp;
   StringArray geog_files;
   RegridInfo regrid_info;
   GrdFileType ftype;
   Dictionary *field_dict = (Dictionary *) 0;
   bool found = false;
   int i;

   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;

   VarInfoFactory info_factory;
   VarInfo *info = (VarInfo *) 0;

   if(!dict) {
      mlog << Error << "\nparse_geog_data() -> "
           << "null pointer!\n\n";
         exit(1);
   }

   // Parse the file names and append the forecast file
   geog_files = dict->lookup_string_array(conf_key_file_name, false);
   geog_files.add(fcst_file);

   // Parse the field
   if(!(field_dict = dict->lookup_dictionary(conf_key_field))) {
      mlog << Error << "\nparse_geog_data() -> "
           << "error parsing \"" << conf_key_field << "\" dictionary.\n\n";
           exit(1);
   }

   // Check for specified file type
   ftype = parse_conf_file_type(field_dict);

   // Search each input file for a match
   for(i=0; !found && i<geog_files.n(); i++) {

      // Allocate memory for data file
      if(!(mtddf = mtddf_factory.new_met_2d_data_file(geog_files[i].c_str(), ftype))) {
         mlog << Error << "\nparse_geog_data() -> "
              << "Trouble reading geography mask file \""
              << geog_files[i] << "\"\n\n";
         exit(1);
      }

      // Parse the variable name and level
      info = info_factory.new_var_info(mtddf->file_type());
      info->set_dict(*field_dict);

      // Read the data
      found = mtddf->data_plane(*info, dp);

      if(found) {

         mlog << Debug(3)
              << "Geography mask data " << info->magic_str()
              << " found in \"" << geog_files[i] << "\".\n\n";

         // Regrid, if needed
         if(!(mtddf->grid() == vx_grid)) {
            regrid_info = parse_conf_regrid(dict);
            mlog << Debug(2)
                 << "Regridding geography mask data " << info->magic_str()
                 << " to the verification grid.\n";
            dp = met_regrid(dp, mtddf->grid(), vx_grid, regrid_info);
         }
      }
      else {
         mlog << Debug(3)
              << "Geography mask data " << info->magic_str()
              << " not found in \"" << geog_files[i] << "\".\n\n";
      }

      // Deallocate memory
      if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }
      if(info)  { delete info;  info  = (VarInfo       *) 0; }
   }

   if(!found) {
      mlog << Error << "\nparse_geog_data() -> "
           << "Geography mask data not found!\n\n";
      exit(1);
   }

   return(dp);
}

////////////////////////////////////////////////////////////////////////
