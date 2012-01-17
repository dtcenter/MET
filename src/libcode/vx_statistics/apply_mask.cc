// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_log.h"

///////////////////////////////////////////////////////////////////////////////

void process_poly_mask(const ConcatString &, const Grid &,
                       DataPlane &, ConcatString&);

///////////////////////////////////////////////////////////////////////////////

void parse_grid_mask(const ConcatString &mask_grid_str, const Grid &grid,
                     DataPlane &mask_dp, ConcatString &mask_name) {
   Grid mask_grid;

   // Initialize the DataPlane masking object by turning all points on
   mask_dp.set_size(grid.nx(), grid.ny());
   mask_dp.set_constant(mask_on_value);

   // Store the name of the masking grid
   mask_name = mask_grid_str;

   //
   // Check to make sure that we're not using the full domain
   //
   if(strcmp(full_domain_str, mask_grid_str) != 0) {

      //
      // Search for the grid name in the predefined grids
      //
      if(!find_grid_by_name(mask_name, mask_grid)) {
         mlog << Error << "\n\n  parse_grid_mask() -> "
              << "the mask_grid requested \"" << mask_grid_str
              << "\" is not defined.\n\n";
         exit(1);
      }

      apply_grid_mask(grid, mask_grid, mask_dp);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// The mask_poly_str contains masking regions defined one of 3 ways:
//
// (1) An ASCII file containing a lat/lon polygon.
// (2) The NetCDF output of the gen_poly_mask tool.
// (3) A 2D data file, followed by a description of the field to be used,
//     and optionally, a threshold to be applied to the field.
//
///////////////////////////////////////////////////////////////////////////////

void parse_poly_mask(const ConcatString &mask_poly_str, const Grid &grid,
                     DataPlane &mask_dp, ConcatString &mask_name) {
   char mask_poly_path[PATH_MAX], file_name[PATH_MAX];
   char magic_str[PATH_MAX], thresh_str[PATH_MAX];
   SingleThresh st;
   char *ptr;

   // 2D Data file
   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;

   // VarInfo object
   VarInfoFactory info_factory;
   VarInfo *info = (VarInfo *) 0;

   // Replace any instances of MET_BASE with it's expanded value
   replace_string(met_base_str, MET_BASE, mask_poly_str, mask_poly_path);

   // Store the masking file name
   if((ptr = strtok(mask_poly_path, " ")) == NULL) {
      mlog << Error << "\n\n  parse_poly_mask() -> "
           << "must supply a file name to be used.\n\n";
      exit(1);
   }
   else {
      strcpy(file_name, ptr);
   }

   // Store the magic string, if present, and NA otherwise.
   if((ptr = strtok(NULL, " ")) != NULL) strcpy(magic_str, ptr);
   else                                  strcpy(magic_str, na_str);

   // Store the threshold string, if present, and default threshold otherwise.
   if((ptr = strtok(NULL, " ")) != NULL) strcpy(thresh_str, ptr);
   else                                  strcpy(thresh_str, default_mask_thresh);

   // Attempt to open the data file
   mtddf = mtddf_factory.new_met_2d_data_file(file_name);

   // If data file pointer is NULL, assume a lat/lon polyline file
   if(!mtddf) {
      process_poly_mask(file_name, grid, mask_dp, mask_name);
   }
   // Otherwise, process the input data file
   else {

      // Check that the masking grid matches the input grid
      if(!(mtddf->grid() == grid)) {
         mlog << Error << "\n\n  parse_poly_mask() -> "
              << "the masking grid does not match the input grid.\n\n";
         exit(1);
      }

      // Create a new VarInfo object
      info = info_factory.new_var_info(mtddf->file_type());
      info->set_magic(magic_str);

      // Extract the data plane from the input file
      mtddf->data_plane(*info, mask_dp);

      // Parse the threshold information
      st.set(thresh_str);

      // Apply threshold to the data plane
      mask_dp.threshold(st);

      // Store the mask name
      mask_name = info->name();
   }

   // Clean up
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }
   if(info)  { delete info;  info  = (VarInfo *)       0; }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void process_poly_mask(const ConcatString &file_name, const Grid &grid,
                       DataPlane &mask_dp, ConcatString &mask_name) {
   Polyline mask_poly;

   // Initialize the DataPlane masking object by turning all points on
   mask_dp.set_size(grid.nx(), grid.ny());
   mask_dp.set_constant(mask_on_value);

   // Initialize the masking polyline
   mask_poly.clear();

   // Parse out the polyline from the file
   parse_latlon_poly_file(file_name, mask_poly);

   // Store the name of the masking polyline
   mask_name = mask_poly.name;

   // Apply the polyline mask to the DataPlane object
   apply_poly_mask_latlon(mask_poly, grid, mask_dp);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void parse_sid_mask(const char *mask_sid_file, StringArray &mask_sid) {
   ifstream in;
   char tmp_file[PATH_MAX], sid_str[PATH_MAX];

   // Replace any instances of MET_BASE with it's expanded value
   replace_string(met_base_str, MET_BASE, mask_sid_file, tmp_file);

   // Check for an empty length string
   if(strlen(tmp_file) == 0) return;

   // Open the mask station id file specified
   in.open(tmp_file);

   if(!in) {
      mlog << Error << "\n\n  parse_sid_mask() -> "
           << "Can't open the mask station ID file specified ("
           << tmp_file << ").\n\n";
      exit(1);
   }

   // Read in and store the masking station ID names
   while(in >> sid_str) mask_sid.add(sid_str);

   // Close the input file
   in.close();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void apply_grid_mask(const Grid &grid, const Grid &mask_grid, DataPlane &dp) {
   int x, y;
   double lat, lon, mask_x, mask_y;
   int in_count, out_count;

   //
   // If the masking grid's dimensions are zero, there's nothing to be done
   //
   if(mask_grid.nx() == 0 || mask_grid.ny() == 0) {
      mlog << Warning << "\n\n  void apply_grid_mask() -> "
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

///////////////////////////////////////////////////////////////////////////////

void apply_poly_mask_xy(const Polyline &poly, DataPlane &dp) {
   int x, y;
   int in_count, out_count;

   //
   // If the polyline contains less than 3 points, no masking region is
   // defined and there is nothing to do
   //
   if(poly.n_points < 3) {
      mlog << Warning << "\n\n  void apply_poly_mask_xy() -> "
           << "no polyline masking applied since the masking polyline "
           << "contains fewer than 3 points.\n\n";
      return;
   }

   //
   // Mask out any grid points not inside the masking polygon
   //
   in_count = out_count = 0;
   for(x=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         if(!(poly.is_inside(x, y))) {
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

void apply_poly_mask_latlon(const Polyline &poly, const Grid &grid, DataPlane &dp) {
   int x, y;
   double lat, lon;
   int in_count, out_count;

   //
   // If the polyline contains less than 3 points, no masking region is
   // defined and there is nothing to do
   //
   if(poly.n_points < 3) {
      mlog << Warning << "\n\n  void apply_poly_mask_latlon() -> "
           << "no polyline masking applied since the masking polyline "
           << "contains fewer than 3 points.\n\n";
      return;
   }

   //
   // Mask out any grid points who's corresponding lat/lon coordinates are
   // not inside the masking lat/lon polygon.
   //
   in_count = out_count = 0;
   for(x=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         grid.xy_to_latlon(x, y, lat, lon);

         if(!(poly.is_inside(lon, lat))) {
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
