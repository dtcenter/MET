// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   gen_vx_mask.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    12/09/14  Halley Gotway   New
//   001    05/25/16  Halley Gotway   Allow -value to be set negative.
//   002    06/01/16  Halley Gotway   Pass -input_field timing
//                    timing information through to the output.
//   003    06/02/16  Halley Gotway   Add box masking type.
//   004    11/15/16  Halley Gotway   Add solar masking types.
//   005    04/08/17  Halley Gotway   Add lat/lon masking types.
//   006    07/09/18  Bullock         Add shapefile masking type.
//   007    04/08/19  Halley Gotway   Add percentile thresholds.
//   008    04/06/20  Halley Gotway   Generalize input_grid option.
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "gen_vx_mask.h"

#include "grib_classes.h"

#include "vx_log.h"
#include "nav.h"
#include "vx_math.h"
#include "vx_nc_util.h"
#include "data2d_nc_met.h"
#include "solar.h"
#include "shp_file.h"
#include "grid_closed_poly.h"

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   static DataPlane dp_data, dp_mask, dp_out;

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the input grid
   process_input_grid(dp_data);

   // Process the mask file
   process_mask_file(dp_mask);

   // Apply combination logic if the current mask is binary
   if(mask_type == MaskType_Poly   ||
      mask_type == MaskType_Shape  ||
      mask_type == MaskType_Box    ||
      mask_type == MaskType_Grid   ||
      thresh.get_type() != thresh_na) {
      dp_out = combine(dp_data, dp_mask, set_logic);
   }
   // Otherwise, pass through the distance or raw values
   else {
      dp_out = dp_mask;
   }

   // Write out the mask file to NetCDF
   write_netcdf(dp_out);

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;

   // Check for zero arguments
   if(argc == 1) usage();

   // Initialize the configuration object
   config.read(replace_path(config_const_filename).c_str());

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_type,         "-type",         1);
   cline.add(set_input_field,  "-input_field",  1);
   cline.add(set_mask_field,   "-mask_field",   1);
   cline.add(set_complement,   "-complement",   0);
   cline.add(set_union,        "-union",        0);
   cline.add(set_intersection, "-intersection", 0);
   cline.add(set_symdiff,      "-symdiff",      0);
   cline.add(set_thresh,       "-thresh",       1);
   cline.add(set_height,       "-height",       1);
   cline.add(set_width,        "-width",        1);
   cline.add(set_value,        "-value",        1);
   cline.add(set_name,         "-name",         1);
   cline.add(set_logfile,      "-log",          1);
   cline.add(set_verbosity,    "-v",            1);
   cline.add(set_compress,     "-compress",     1);
   cline.add(set_shapeno,      "-shapeno",      1);

   cline.allow_numbers();

   // Parse the command line
   cline.parse();

   // There should be two arguments left, the input/output file names
   if(cline.n() != 3) usage();

   // Store the arguments
   input_gridname = cline[0];
   mask_filename  = cline[1];
   out_filename   = cline[2];

   // List the input files
   mlog << Debug(1)
        << "Input Grid:\t\t" << input_gridname << "\n"
        << "Mask File:\t\t"  << mask_filename  << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void process_input_grid(DataPlane &dp) {
   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile *mtddf_ptr = (Met2dDataFile *) 0;
   GrdFileType ftype = FileType_None;

   // Get the gridded file type from the data config string, if present
   if(input_field_str.length() > 0) {
      config.read_string(input_field_str.c_str());
      ftype = parse_conf_file_type(&config);
   }

   // Parse info.name as a white-space separated string
   StringArray sa;
   sa.parse_wsss(input_gridname);

   // Search for a named grid
   if(sa.n() == 1 && find_grid_by_name(sa[0].c_str(), grid)) {
      mlog << Debug(3)
           << "Use the grid named \"" << input_gridname << "\".\n";
   }
   // Parse grid definition
   else if(sa.n() > 1 && parse_grid_def(sa, grid)) {
      mlog << Debug(3)
           << "Use the grid defined by string \"" << input_gridname
           << "\".\n";
   }
   // Extract the grid from a gridded data file
   else {

      mlog << Debug(3)
           << "Use the grid defined by file \"" << input_gridname
           << "\".\n";

      // Attempt to open the data file
      mtddf_ptr = mtddf_factory.new_met_2d_data_file(input_gridname.c_str(), ftype);
      if(!mtddf_ptr) {
         mlog << Error << "\nprocess_input_grid() -> "
              << "can't open input file \"" << input_gridname << "\"\n\n";
         exit(1);
      }

      // Read the input data plane, if requested
      if(input_field_str.length() > 0) {
         get_data_plane(mtddf_ptr, input_field_str.c_str(), dp);
      }
      // Check for the output of a previous call to this tool
      else if(get_gen_vx_mask_data(mtddf_ptr, dp)) {
      }

      // Extract the grid
      grid = mtddf_ptr->grid();
   }

   // If not yet set, fill the input data plane with zeros
    if(dp.is_empty()) {
      dp.set_size(grid.nx(), grid.ny());
      dp.set_constant(0.0);
   }

   mlog << Debug(2)
        << "Parsed Input Grid:\t" << grid.name()
        << " (" << grid.nx() << " x " << grid.ny() << ")\n";

   // Clean up
   if(mtddf_ptr) { delete mtddf_ptr; mtddf_ptr = (Met2dDataFile *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_mask_file(DataPlane &dp) {
   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile *mtddf_ptr = (Met2dDataFile *) 0;
   GrdFileType ftype = FileType_None;

   // Initialize
   solar_ut = (unixtime) 0;

   // Process the mask file as a lat/lon polyline file
   if(mask_type == MaskType_Poly   ||
      mask_type == MaskType_Box    ||
      mask_type == MaskType_Circle ||
      mask_type == MaskType_Track) {

      poly_mask.clear();
      poly_mask.load(mask_filename.c_str());

      mlog << Debug(2)
           << "Parsed Lat/Lon Mask:\t" << poly_mask.name()
           << " containing " << poly_mask.n_points() << " points\n";
   }

   // Process the mask from a shapefile
   else if(mask_type == MaskType_Shape) {

      get_shapefile_outline(shape);

      mlog << Debug(2)
           << "Parsed Shape Mask:\t" << mask_filename
           << " containing " << shape.n_points << " points\n";
   }

   // For solar masking, check for a date/time string
   else if(is_solar_masktype(mask_type) &&
           is_datestring(mask_filename.c_str())) {

      solar_ut = timestring_to_unix(mask_filename.c_str());

      mlog << Debug(2)
           << "Solar Time String:\t"
           << unix_to_yyyymmdd_hhmmss(solar_ut) << "\n";
   }

   // Nothing to do for Lat/Lon masking types
   else if(mask_type == MaskType_Lat ||
           mask_type == MaskType_Lon) {
   }

   // Otherwise, process the mask file as a gridded data file
   else {

      // Get the gridded file type from the mask config string, if present
      if(mask_field_str.length() > 0) {
         config.read_string(mask_field_str.c_str());
         ftype = parse_conf_file_type(&config);
      }

      mtddf_ptr = mtddf_factory.new_met_2d_data_file(mask_filename.c_str(), ftype);
      if(!mtddf_ptr) {
         mlog << Error << "\nprocess_mask_file() -> "
              << "can't open gridded mask data file \"" << mask_filename << "\"\n\n";
         exit(1);
      }

      // Read mask_field, if specified
      if(mask_field_str.length() > 0) {
         get_data_plane(mtddf_ptr, mask_field_str.c_str(), dp);
      }

      // Extract the grid
      grid_mask = mtddf_ptr->grid();

      mlog << Debug(2)
           << "Parsed Mask Grid:\t" << grid_mask.name()
           << " (" << grid_mask.nx() << " x " << grid_mask.ny() << ")\n";

      // Check for matching grids
      if(mask_type == MaskType_Data && grid != grid_mask) {
         mlog << Error << "\nprocess_mask_file() -> "
              << "The input grid and mask grid must be identical for "
              << "\"data\" masking.\n"
              << "Data Grid -> " << grid.serialize() << "\n"
              << "Mask Grid -> " << grid_mask.serialize() << "\n\n";
         exit(1);
      }
   }

   // For solar masking, parse the valid time from gridded data
   if(is_solar_masktype(mask_type) && solar_ut == (unixtime) 0) {

      if(mask_field_str.length() == 0) {
         mlog << Error << "\nprocess_mask_file() -> "
              << "use \"-mask_field\" to specify the data whose valid "
              << "time should be used for \"solar_alt\" and "
              << "\"solar_azi\" masking.\n\n";
         exit(1);
      }
      solar_ut = dp.valid();
      dp.clear();

      mlog << Debug(2) << "Solar File Time:\t"
           << unix_to_yyyymmdd_hhmmss(solar_ut) << "\n";
   }

   // Check that mask_field has been set for data masking
   if(mask_type == MaskType_Data && mask_field_str.length() == 0) {
      mlog << Error << "\nprocess_mask_file() -> "
           << "use \"-mask_field\" to specify the field for "
           << "\"data\" masking.\n\n";
      exit(1);
   }

   // Initialize the masking field, if needed
   if(dp.is_empty()) dp.set_size(grid.nx(), grid.ny());

   // Construct the mask
   switch(mask_type) {

      case MaskType_Poly:
         apply_poly_mask(dp);
         break;

      case MaskType_Box:
         apply_box_mask(dp);
         break;

      case MaskType_Circle:
         apply_circle_mask(dp);
         break;

      case MaskType_Track:
         apply_track_mask(dp);
         break;

      case MaskType_Grid:
         apply_grid_mask(dp);
         break;

      case MaskType_Data:
         apply_data_mask(dp);
         break;

      case MaskType_Solar_Alt:
      case MaskType_Solar_Azi:
         apply_solar_mask(dp);
         break;

      case MaskType_Lat:
      case MaskType_Lon:
         apply_lat_lon_mask(dp);
         break;

      case MaskType_Shape:
         apply_shape_mask(dp);
         break;

      default:
         mlog << Error << "\nprocess_mask_file() -> "
              << "Unxpected MaskType value (" << mask_type << ")\n\n";
         exit(1);
         break;
   }

   // Clean up
   if(mtddf_ptr) { delete mtddf_ptr; mtddf_ptr = (Met2dDataFile *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void get_data_plane(Met2dDataFile *mtddf_ptr,
                    const char *config_str, DataPlane &dp) {
   VarInfoFactory vi_factory;
   VarInfo *vi_ptr = (VarInfo *) 0;
   double dmin, dmax;

   // Parse the config string
   config.read_string(config_str);

   // Allocate new VarInfo object
   vi_ptr = vi_factory.new_var_info(mtddf_ptr->file_type());
   if(!vi_ptr) {
      mlog << Error << "\nget_data_plane() -> "
           << "can't allocate new VarInfo pointer.\n\n";
      exit(1);
   }

   // Read config into the VarInfo object
   vi_ptr->set_dict(config);

   // Get data plane from the file for this VarInfo object
   if(!mtddf_ptr->data_plane(*vi_ptr, dp)) {
      mlog << Error << "\nget_data_plane() -> "
           << "trouble reading field \"" << config_str
           << "\" from file \"" << mtddf_ptr->filename() << "\"\n\n";
      exit(1);
   }

   // Dump the range of data values read
   dp.data_range(dmin, dmax);
   mlog << Debug(3)
        << "Read field \"" << vi_ptr->magic_str() << "\" from \""
        << mtddf_ptr->filename() << "\" with data ranging from "
        << dmin << " to " << dmax << ".\n";

   // Clean up
   if(vi_ptr) { delete vi_ptr; vi_ptr = (VarInfo *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

bool get_gen_vx_mask_data(Met2dDataFile *mtddf_ptr, DataPlane &dp) {
   bool status = false;
   ConcatString tool, config_str;
   int i;

   // Must be MET NetCDF format
   if(mtddf_ptr->file_type() != FileType_NcMet) return(status);

   // Cast pointer of correct type
   MetNcMetDataFile *mnmdf_ptr = (MetNcMetDataFile *) mtddf_ptr;

   // Check for the MET_tool global attribute
   if(!get_global_att(mnmdf_ptr->MetNc->Nc, (string)"MET_tool", tool)) return(status);

   // Check for gen_vx_mask output
   if(tool != program_name) return(status);

   // Loop through the NetCDF variables
   for(i=0; i<mnmdf_ptr->MetNc->Nvars; i++) {

      // Skip the lat/lon variables
      if(mnmdf_ptr->MetNc->Var[i].name == "lat" ||
         mnmdf_ptr->MetNc->Var[i].name == "lon") continue;

      // Read the first non-lat/lon variable
      config_str << "'name=\"" << mnmdf_ptr->MetNc->Var[i].name << "\"; level=\"(*,*)\";'";
      get_data_plane(mtddf_ptr, config_str.c_str(), dp);
      status = true;
      break;
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

void get_shapefile_outline(ShpPolyRecord & cur_shape) {
   const char * const shape_filename = mask_filename.c_str();
   ShpFile f;
   ShpPolyRecord & pr = cur_shape;

   // Open shapefile
   if(!(f.open(shape_filename))) {
      mlog << Error << "\nget_shapefile_outline() -> "
           << "unable to open shape file \"" << shape_filename
           << "\"\n\n";
      exit(1);
   }

   // Make sure it's a polygon file, and not some other type
   if(f.shape_type() != shape_type_polygon) {
      mlog << Error << "\nget_shapefile_outline() -> "
           << "shape file \"" << shape_filename
           << "\" is not a polygon file\n\n";
      exit(1);
   }

   // Skip through un-needed records
   for(int i=0; i<shape_number; i++) {
      if(f.at_eof()) break;
      f >> pr;
   }

   if(f.at_eof()) {
      mlog << Error << "\nget_shapefile_outline() -> "
           << "hit eof before reading specified record\n\n";
      exit(1);
   }

   // Get the target record
   f >> pr;
   pr.toggle_longitudes();

   return;
}

////////////////////////////////////////////////////////////////////////

void apply_poly_mask(DataPlane & dp) {
   int x, y, n_in;
   bool inside;
   double lat, lon;

   n_in = 0;

   // Check each grid point being inside the polyline
   for(x=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         // Lat/Lon value for the current grid point
         grid.xy_to_latlon(x, y, lat, lon);

         // Check current grid point inside polyline
         inside = poly_mask.latlon_is_inside(lat, lon);   //  returns bool

         // Check the complement
         if(complement) inside = !inside;

         // Increment count
         if ( inside )  ++n_in;

         // Store the current mask value
         dp.set( (inside ? 1.0 : 0.0), x, y);

      } // end for y
   } // end for x

   if(complement) {
      mlog << Debug(3)
           << "Applying complement of polyline mask.\n";
   }

   // List number of points inside the mask
   mlog << Debug(3)
        << "Polyline Masking:\t" << n_in << " of " << grid.nx() * grid.ny()
        << " points inside\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void apply_box_mask(DataPlane &dp) {
   int i, x_ll, y_ll, x, y, n_in;
   double cen_x, cen_y;
   bool inside;

   // Process the height and width
   if(is_bad_data(height) && is_bad_data(width)) {
      mlog << Error << "\napply_box_mask() -> "
           << "the \"-height\" and/or \"-width\" options must be "
           << "specified in grid units for box masking.\n\n";
      exit(1);
   }
   else if(is_bad_data(height) && !is_bad_data(width)) {
      height = width;
   }
   else if(!is_bad_data(height) && is_bad_data(width)) {
      width = height;
   }

   // Process each lat/lon point
   for(i=0; i<poly_mask.n_points(); i++) {

      // Convert box lat/lon to grid x/y
      grid.latlon_to_xy(poly_mask.lat(i), poly_mask.lon(i), cen_x, cen_y);

      // Get the lower-left x, y
      get_xy_ll(cen_x, cen_y, width, height, x_ll, y_ll);

      // Set mask value for points within the box
      for(x=x_ll; x<x_ll+width; x++) {
         if(x < 0 || x >= dp.nx()) continue;

         for(y=y_ll; y<y_ll+height; y++) {
            if(y < 0 || y >= dp.ny()) continue;

            // Set the mask
            dp.set(1, x, y);

         } // end for y
      } // end for x
   } // end for i

   // Loop through the field, handle the complement, and count up points
   for(x=0, n_in=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         inside = (dp(x, y) == 1);

         // Check the complement
         if(complement) inside = !inside;

         // Increment count
         n_in += inside;

         // Store the current mask value
         dp.set(inside, x, y);
      }
   }

   if(complement) {
      mlog << Debug(3)
           << "Applying complement of box mask.\n";
   }

   mlog << Debug(3)
        << "Box Masking (" << height << " x " << width << "):\t"
        << n_in << " of " << grid.nx() * grid.ny()
        << " points inside\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void apply_circle_mask(DataPlane &dp) {
   int x, y, i, n_in;
   double lat, lon, dist, v;
   bool check;

   // Check for no threshold
   if(thresh.get_type() == thresh_na) {
      mlog << Warning
           << "\napply_circle_mask() -> since \"-thresh\" was not used "
           << "to specify a threshold in kilometers for circle masking, "
           << "the minimum distance to the points will be written.\n\n";
   }

   // For each grid point, compute mimumum distance to polyline points
   for(x=0,n_in=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         // Lat/Lon value for the current grid point
         grid.xy_to_latlon(x, y, lat, lon);
         lon -= 360.0*floor((lon + 180.0)/360.0);

         // Find the minimum distance to a polyline point
         for(i=0, dist=1.0E10; i<poly_mask.n_points(); i++) {
            dist = min(dist, gc_dist(lat, lon, poly_mask.lat(i), poly_mask.lon(i)));
         }

         // Apply threshold, if specified
         if(thresh.get_type() != thresh_na) {
            check = thresh.check(dist);

            // Check the complement
            if(complement) check = !check;

            // Increment count
            n_in += check;

            v = (check ? 1.0 : 0.0);
         }
         else {
            v = dist;
         }

         // Store the result
         dp.set(v, x, y);
      } // end for y
   } // end for x

   if(thresh.get_type() != thresh_na && complement) {
      mlog << Debug(3)
           << "Applying complement of circle mask.\n";
   }

   // List the number of points inside the mask
   if(thresh.get_type() != thresh_na) {
      mlog << Debug(3)
           << "Circle Masking:\t" << n_in << " of " << grid.nx() * grid.ny()
           << " points inside\n";
   }
   // Otherwise, list the min/max distances computed
   else {
      double dmin, dmax;
      dp.data_range(dmin, dmax);
      mlog << Debug(3)
           << "Circle Masking:\tDistances ranging from "
           << dmin << " km to " << dmax << " km\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void apply_track_mask(DataPlane &dp) {
   int x, y, i, n_in;
   double lat, lon, dist, v;
   bool check;

   // Check for no threshold
   if(thresh.get_type() == thresh_na) {
      mlog << Warning
           << "\napply_track_mask() -> since \"-thresh\" was not used "
           << "to specify a threshold for track masking, the minimum "
           << "distance to the track will be written.\n\n";
   }

   // For each grid point, compute mimumum distance to track
   for(x=0,n_in=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         // Lat/Lon value for the current grid point
         grid.xy_to_latlon(x, y, lat, lon);
         lon -= 360.0*floor((lon + 180.0)/360.0);

         // Find the minimum distance to the track
         for(i=1, dist=1.0E10; i<poly_mask.n_points(); i++) {
            dist = min(dist,
                       gc_dist_to_line(poly_mask.lat(i-1), poly_mask.lon(i-1),
                                       poly_mask.lat(i),   poly_mask.lon(i),
                                       lat, lon));
         }

         // Apply threshold, if specified
         if(thresh.get_type() != thresh_na) {
            check = thresh.check(dist);

            // Check the complement
            if(complement) check = !check;

            // Increment count
            n_in += check;

            v = (check ? 1.0 : 0.0);
         }
         else {
            v = dist;
         }

         // Store the result
         dp.set(v, x, y);
      } // end for y
   } // end for x

   if(thresh.get_type() != thresh_na && complement) {
      mlog << Debug(3)
           << "Applying complement of track mask.\n";
   }

   // List the number of points inside the mask
   if(thresh.get_type() != thresh_na) {
      mlog << Debug(3)
           << "Track Masking:\t\t" << n_in << " of " << grid.nx() * grid.ny()
           << " points inside\n";
   }
   // Otherwise, list the min/max distances computed
   else {
      double dmin, dmax;
      dp.data_range(dmin, dmax);
      mlog << Debug(3)
           << "Track Masking:\t\tDistances ranging from "
           << dmin << " km to " << dmax << " km\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void apply_grid_mask(DataPlane &dp) {
   int x, y, n_in;
   bool inside;
   double lat, lon, mask_x, mask_y;

   // Check each grid point being inside the masking grid
   for(x=0,n_in=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         // Lat/Lon value for the current grid point
         grid.xy_to_latlon(x, y, lat, lon);
         lon -= 360.0*floor((lon + 180.0)/360.0);

         // Convert Lat/Lon to masking grid x/y
         grid_mask.latlon_to_xy(lat, lon, mask_x, mask_y);

         // Check for point falling within the masking grid
         inside = (mask_x >= 0 && mask_x < grid_mask.nx() &&
                   mask_y >= 0 && mask_y < grid_mask.ny());

         // Apply the complement
         if(complement) inside = !inside;

         // Increment count
         n_in += inside;

         // Store the current mask value
         dp.set(inside, x, y);
      } // end for y
   } // end for x

   if(complement) {
      mlog << Debug(3)
           << "Applying complement of grid mask.\n";
   }

   mlog << Debug(3)
        << "Grid Masking:\t\t" << n_in << " of " << grid.nx() * grid.ny()
        << " points inside\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void apply_data_mask(DataPlane &dp) {
   int x, y, n_in;
   bool check;

   // Nothing to do without a threshold
   if(thresh.get_type() == thresh_na) {
      double dmin, dmax;
      dp.data_range(dmin, dmax);
      mlog << Debug(3)
           << "Data Masking:\t\tValues ranging from "
           << dmin << " km to " << dmax << " km\n";
      mlog << Warning
           << "\napply_data_mask() -> since \"-thresh\" was not used "
           << "to specify a threshold for data masking, the raw data "
           << "values will be written.\n\n";
      return;
   }

   // Process percentile thresholds
   if(thresh.need_perc()) {
      NumArray d;
      int nxy = dp.nx()*dp.ny();
      d.extend(nxy);
      for(int i=0; i<nxy; i++) {
         if(!is_bad_data(dp.data()[i])) d.add(dp.data()[i]);
      }
      thresh.set_perc(&d, &d, &d);
   }

   // For each grid point, apply the data threshold
   for(x=0,n_in=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         // Apply the threshold
         check = thresh.check(dp(x,y));

         // Check the complement
         if(complement) check = !check;

         // Increment count
         n_in += check;

         // Store the result
         dp.set(check ? 1.0 : 0.0, x, y);
      } // end for y
   } // end for x

   // List the number of points inside the mask
   mlog << Debug(3)
        << "Data Masking:\t\t" << n_in << " of " << grid.nx() * grid.ny()
        << " points inside\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void apply_solar_mask(DataPlane &dp) {
   int x, y, n_in;
   double lat, lon, alt, azi, v;
   bool check;

   // Check for no threshold
   if(thresh.get_type() == thresh_na) {
      mlog << Warning
           << "\napply_solar_mask() -> since \"-thresh\" was not used "
           << "the raw " << masktype_to_string(mask_type)
           << " values will be written.\n\n";
   }

   // Compute solar value for each grid point Lat/Lon
   for(x=0,n_in=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         // Lat/Lon value for the current grid point
         grid.xy_to_latlon(x, y, lat, lon);
         lon -= 360.0*floor((lon + 180.0)/360.0);

         // Compute the solar altitude and azimuth
         solar_altaz(solar_ut, lat, lon, alt, azi);
         v = (mask_type == MaskType_Solar_Alt ? alt : azi);

         // Apply threshold, if specified
         if(thresh.get_type() != thresh_na) {
            check = thresh.check(v);

            // Check the complement
            if(complement) check = !check;

            // Increment count
            n_in += check;

            v = (check ? 1.0 : 0.0);
         }

         // Store the current solar value
         dp.set(v, x, y);

      } // end for y
   } // end for x

   if(thresh.get_type() != thresh_na && complement) {
      mlog << Debug(3)
           << "Applying complement of the "
           << masktype_to_string(mask_type) << " mask.\n";
   }

   const char *mask_str = (mask_type == MaskType_Solar_Alt ?
                           "Altitude" : "Azimuth");

   // List the number of points inside the mask
   if(thresh.get_type() != thresh_na) {
      mlog << Debug(3)
           << "Solar " << mask_str << " Masking:\t\t"
           << n_in << " of " << grid.nx() * grid.ny()
           << " points inside\n";
   }
   // Otherwise, list the min/max distances computed
   else {
      double dmin, dmax;
      dp.data_range(dmin, dmax);
      mlog << Debug(3)
           << "Solar " << mask_str << " Masking:\t\t"
           << "Values ranging from " << dmin << " to " << dmax << "\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void apply_lat_lon_mask(DataPlane &dp) {
   int x, y, n_in;
   double lat, lon, v;
   bool check;

   // Check for no threshold
   if(thresh.get_type() == thresh_na) {
      mlog << Warning
           << "\napply_lat_lon_mask() -> since \"-thresh\" was not used "
           << "the raw " << masktype_to_string(mask_type)
           << " values will be written.\n\n";
   }

   // Compute Lat/Lon value for each grid point
   for(x=0,n_in=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         // Lat/Lon value for the current grid point
         grid.xy_to_latlon(x, y, lat, lon);
         v = (mask_type == MaskType_Lat ? lat :
              rescale_deg(-1.0*lon, -180.0, 180.0));

         // Apply threshold, if specified
         if(thresh.get_type() != thresh_na) {
            check = thresh.check(v);

            // Check the complement
            if(complement) check = !check;

            // Increment count
            n_in += check;

            v = (check ? 1.0 : 0.0);
         }

         // Store the current solar value
         dp.set(v, x, y);

      } // end for y
   } // end for x

   if(thresh.get_type() != thresh_na && complement) {
      mlog << Debug(3)
           << "Applying complement of the "
           << masktype_to_string(mask_type) << " mask.\n";
   }

   const char *mask_str = (mask_type == MaskType_Lat ?
                           "Latitude" : "Longitude");

   // List the number of points inside the mask
   if(thresh.get_type() != thresh_na) {
      mlog << Debug(3)
           << mask_str << " Masking:\t\t" << n_in << " of "
           << grid.nx() * grid.ny()<< " points inside\n";
   }
   // Otherwise, list the min/max distances computed
   else {
      double dmin, dmax;
      dp.data_range(dmin, dmax);
      mlog << Debug(3)
           << mask_str << " Masking:\t\tValues ranging from "
           << dmin << " to " << dmax << "\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void apply_shape_mask(DataPlane & dp) {
   int x, y, n_in;
   int j, k, n;
   int start, stop;
   double dx, dy, lat, lon;
   bool status = false;
   GridClosedPoly p;
   GridClosedPolyArray a;

   // Load up array
   for(j=0; j<(shape.n_parts); j++) {

      p.clear();

      start = shape.start_index(j);
      stop  = shape.stop_index(j);

      n = stop - start + 1;

      for(k=0; k<n; ++k) {

         lat = shape.lat(start + k);
         lon = shape.lon(start + k);

         lon = -lon;   //  west is positive for us

         grid.latlon_to_xy(lat, lon, dx, dy);

         x = nint(dx);
         y = nint(dy);

         p.add_point(x, y);
      } // for k

      a.add(p);

   } // for j

   // Check grid points
   for(x=0, n_in=0; x<(grid.nx()); x++) {
      for (y=0; y<(grid.ny()); y++) {

         status = a.is_inside(x, y);

         // Check the complement
         if(complement) status = !status;

         if(status) n_in++;

         dp.set( (status ? 1.0 : 0.0 ), x, y);
      } // for y
   } // for x

   if(complement) {
      mlog << Debug(3)
           << "Applying complement of the shapefile mask.\n";
   }

   // List number of points inside the mask
   mlog << Debug(3)
        << "Shape Masking:\t\t" << n_in << " of " << grid.nx() * grid.ny()
        << " points inside\n";

   return;
}


////////////////////////////////////////////////////////////////////////

DataPlane combine(const DataPlane &dp_data, const DataPlane &dp_mask,
                  SetLogic logic) {
   int x, y, n_in;
   bool v_data, v_mask;
   double v;
   DataPlane dp;

   // Check that the input data planes have the same dimension
   if(dp_data.nx() != dp_mask.nx() || dp_data.ny() != dp_mask.ny()) {
      mlog << Error << "\ncombine() -> "
           << "dimensions of input data planes do not match: ("
           << dp_data.nx() << ", " << dp_data.ny() << ") != ("
           << dp_mask.nx() << ", " << dp_mask.ny() << ")\n\n";
      exit(1);
   }

   // Pass timing information through
   dp.set_init (dp_data.init());
   dp.set_valid(dp_data.valid());
   dp.set_lead (dp_data.lead());
   dp.set_accum(dp_data.accum());

   // Set the output data plane size
   dp.set_size(grid.nx(), grid.ny());

   // Process each point
   for(x=0,n_in=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         // Get the two input data values
         v_data = !is_eq(dp_data(x, y), 0.0);
         v_mask = !is_eq(dp_mask(x, y), 0.0);

         switch(logic) {

            case SetLogic_Union:
               if(v_data || v_mask) v = mask_val;
               else                 v = 0.0;
               break;

            case SetLogic_Intersection:
               if(v_data && v_mask) v = mask_val;
               else                 v = 0.0;
               break;

            case SetLogic_SymDiff:
               if((v_data && !v_mask) || (!v_data && v_mask)) v = mask_val;
               else                                           v = 0.0;
               break;

            // Default behavior is to apply the mask value or pass through
            // the data value
            default:
               if(v_mask) v = mask_val;
               else       v = dp_data(x, y);
               break;
         }

         // Increment count
         n_in += !is_eq(v, 0.0);

         // Store the result
         dp.set(v, x, y);

      } // end for y
   } // end for x

   // List the number of points inside the mask
   if(logic != SetLogic_None) {
      mlog << Debug(3)
           << "Mask " << setlogic_to_string(logic) << ": " << n_in
           << " of " << grid.nx() * grid.ny() << " points inside\n";
   }

   return(dp);
}

////////////////////////////////////////////////////////////////////////

void write_netcdf(const DataPlane &dp) {
   int n, x, y;
   ConcatString cs;

   float *mask_data = (float *)  0;
   NcFile *f_out    = (NcFile *) 0;
   NcDim lat_dim;
   NcDim lon_dim;
   NcVar mask_var;

   // Create a new NetCDF file and open it.
   f_out = open_ncfile(out_filename.c_str(), true);

   if(IS_INVALID_NC_P(f_out)) {
      mlog << Error << "\nwrite_netcdf() -> "
           << "trouble opening output file " << out_filename
           << "\n\n";
      delete f_out;
      f_out = (NcFile *) 0;
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(f_out, out_filename.c_str(), program_name);

   // Add the projection information
   write_netcdf_proj(f_out, grid);

   // Define Dimensions
   lat_dim = add_dim(f_out, "lat", (long) grid.ny());
   lon_dim = add_dim(f_out, "lon", (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(f_out, &lat_dim, &lon_dim, grid);

   // Set the mask_name, if not already set
   if(mask_name.length() == 0) {
      if(mask_type == MaskType_Poly   ||
         mask_type == MaskType_Circle ||
         mask_type == MaskType_Track) {
         mask_name = poly_mask.name();
      }
      else {
         mask_name << masktype_to_string(mask_type) << "_mask";
      }
   }

   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = config.nc_compression();

   // Define Variables
   mask_var = add_var(f_out, string(mask_name), ncFloat, lat_dim, lon_dim, deflate_level);
   cs << cs_erase << mask_name << " masking region";
   add_att(&mask_var, "long_name", string(cs));
   add_att(&mask_var, "_FillValue", bad_data_float);
   cs << cs_erase << masktype_to_string(mask_type);
   if(thresh.get_type() != thresh_na) cs << thresh.get_str();
   add_att(&mask_var, "mask_type", string(cs));

   // Write the solar time
   if(is_solar_masktype(mask_type)) {
      ConcatString time_str;
      unix_to_yyyymmdd_hhmmss(solar_ut, time_str);
      add_att(&mask_var, "solar_time", time_str.text());
   }

   // Write out the times
   if(dp.init() != (unixtime) 0 || dp.valid() != (unixtime) 0) {
      write_netcdf_var_times(&mask_var, dp);
   }

   // Allocate memory to store the mask values for each grid point
   mask_data = new float [grid.nx()*grid.ny()];

   // Loop through each grid point
   for(x=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {
         n = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);
         mask_data[n] = dp(x, y);
      } // end for y
   } // end for x

   if(!put_nc_data_with_dims(&mask_var, &mask_data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_netcdf() -> "
           << "error with mask_var->put\n\n";
      // Delete allocated memory
      if(mask_data) { delete[] mask_data; mask_data = (float *) 0; }
      exit(1);
   }

   // Delete allocated memory
   if(mask_data) { delete[] mask_data; mask_data = (float *) 0; }

   delete f_out;
   f_out = (NcFile *) 0;

   mlog << Debug(1)
        << "Output File:\t\t" << out_filename << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

bool is_solar_masktype(MaskType t) {
   return(t == MaskType_Solar_Alt || t == MaskType_Solar_Azi);
}

////////////////////////////////////////////////////////////////////////

MaskType string_to_masktype(const char *s) {
   MaskType t = MaskType_None;

        if(strcasecmp(s, "poly")      == 0) t = MaskType_Poly;
   else if(strcasecmp(s, "box")       == 0) t = MaskType_Box;
   else if(strcasecmp(s, "circle")    == 0) t = MaskType_Circle;
   else if(strcasecmp(s, "track")     == 0) t = MaskType_Track;
   else if(strcasecmp(s, "grid")      == 0) t = MaskType_Grid;
   else if(strcasecmp(s, "data")      == 0) t = MaskType_Data;
   else if(strcasecmp(s, "solar_alt") == 0) t = MaskType_Solar_Alt;
   else if(strcasecmp(s, "solar_azi") == 0) t = MaskType_Solar_Azi;
   else if(strcasecmp(s, "lat")       == 0) t = MaskType_Lat;
   else if(strcasecmp(s, "lon")       == 0) t = MaskType_Lon;
   else if(strcasecmp(s, "shape")     == 0) t = MaskType_Shape;
   else {
      mlog << Error << "\nstring_to_masktype() -> "
           << "unsupported masking type \"" << s << "\"\n\n";
      exit(1);
   }

   return(t);
}

////////////////////////////////////////////////////////////////////////

const char * masktype_to_string(const MaskType t) {
   const char *s = (const char *) 0;

   switch(t) {
      case MaskType_Poly:      s = "poly";           break;
      case MaskType_Box:       s = "box";            break;
      case MaskType_Circle:    s = "circle";         break;
      case MaskType_Track:     s = "track";          break;
      case MaskType_Grid:      s = "grid";           break;
      case MaskType_Data:      s = "data";           break;
      case MaskType_Solar_Alt: s = "solar_alt";      break;
      case MaskType_Solar_Azi: s = "solar_azi";      break;
      case MaskType_Lat:       s = "lat";            break;
      case MaskType_Lon:       s = "lon";            break;
      case MaskType_Shape:     s = "shape";          break;
      case MaskType_None:      s = na_str;           break;
      default:                 s = (const char *) 0; break;
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tinput_grid\n"
        << "\tmask_file\n"
        << "\tout_file\n"
        << "\t[-type string]\n"
        << "\t[-input_field string]\n"
        << "\t[-mask_field string]\n"
        << "\t[-complement]\n"
        << "\t[-union | -intersection | -symdiff]\n"
        << "\t[-thresh string]\n"
        << "\t[-height n]\n"
        << "\t[-width n]\n"
        << "\t[-shapeno n]\n"
        << "\t[-value n]\n"
        << "\t[-name string]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"input_grid\" is a named grid, the path to a "
        << "gridded data file, or an explicit grid specification "
        << "string (required).\n"
        << "\t\t   If set to a " << program_name << " output file, "
        << "automatically read mask data as the \"input_field\".\n"

        << "\t\t\"mask_file\" defines the masking information "
        << "(required).\n"
        << "\t\t   For \"poly\", \"box\", \"circle\", and \"track\" "
        << "masking, specify an ASCII Lat/Lon file.\n"
        << "\t\t   For \"grid\" masking, specify a named grid, the "
        << "path to a gridded data file, or an explicit grid "
        << "specification.\n"
        << "\t\t   For \"data\" masking specify a gridded data file.\n"
        << "\t\t   For \"solar_alt\" and \"solar_azi\" masking, "
        << "specify a gridded data file or a timestring in "
        << "YYYYMMDD[_HH[MMSS]] format.\n"
        << "\t\t   For \"lat\" and \"lon\" masking, no \"mask_file\" "
        << "needed, simply repeat \"input_grid\".\n"
        << "\t\t   For \"shape\" masking, specify a shapefile "
        << "(suffix \".shp\").\n"

        << "\t\t\"out_file\" is the output NetCDF mask file to be "
        << "written (required).\n"

        << "\t\t\"-type string\" overrides the default masking type ("
        << masktype_to_string(default_mask_type) << ") (optional)\n"
        << "\t\t   \"poly\", \"box\", \"circle\", \"track\", \"grid\", "
        << "\"data\", \"solar_alt\", \"solar_azi\", \"lat\", \"lon\" "
        << "or \"shape\"\n"

        << "\t\t\"-input_field string\" reads existing mask data from "
        << "the \"input_grid\" gridded data file (optional).\n"

        << "\t\t\"-mask_field string\" (optional).\n"
        << "\t\t   For \"data\" masking, define the field from "
        << "\"mask_file\" to be used.\n"

        << "\t\t\"-complement\" computes the complement of the current "
        << "mask (optional).\n"

        << "\t\t\"-union | -intersection | -symdiff\" specify how "
        << "to combine the \"input_field\" data with the current mask "
        << "(optional).\n"


        << "\t\t\"-thresh string\" defines the threshold to be applied "
        << "(optional).\n"
        << "\t\t   For \"circle\" and \"track\" masking, threshold the "
        << "distance (km).\n"
        << "\t\t   For \"data\" masking, threshold the values of "
        << "\"mask_field\".\n"
        << "\t\t   For \"solar_alt\" and \"solar_azi\" masking, "
        << "threshold the computed solar values.\n"
        << "\t\t   For \"lat\" and \"lon\" masking, threshold the "
        << "latitude and longitude values.\n"

        << "\t\t\"-height n\" and \"-width n\" (optional).\n"
        << "\t\t   For \"box\" masking, specify these dimensions in grid "
        << "units.\n"

        << "\t\t\"-shapeno n\" (optional).\n"
        << "\t\t   For \"shape\" masking, specify the shape number "
        << "(0-based) to be used.\n"

        << "\t\t\"-value n\" overrides the default output mask data "
        << "value (" << default_mask_val << ") (optional).\n"

        << "\t\t\"-name string\" specifies the output variable name "
        << "for the mask (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t\"-compress level\" overrides the compression level of "
        << "NetCDF variable (" << config.nc_compression()
        << ") (optional).\n\n"

        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_type(const StringArray & a) {
   if(type_is_set) {
      mlog << Error << "\n" << program_name << " -> "
           << "the -type command line option can only be used once!\n"
           << "To apply multiple masks, run this tool multiple times "
           << "using the output of one run as the input to the next."
           << "\n\n"; 
      exit(1);
   }
   mask_type = string_to_masktype(a[0].c_str());
   type_is_set = true;
}

////////////////////////////////////////////////////////////////////////

void set_input_field(const StringArray & a) {
   input_field_str = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_mask_field(const StringArray & a) {
   mask_field_str = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_complement(const StringArray & a) {
   complement = true;
}

////////////////////////////////////////////////////////////////////////

void set_union(const StringArray & a) {
   set_logic = SetLogic_Union;
}

////////////////////////////////////////////////////////////////////////

void set_intersection(const StringArray & a) {
   set_logic = SetLogic_Intersection;
}

////////////////////////////////////////////////////////////////////////

void set_symdiff(const StringArray & a) {
   set_logic = SetLogic_SymDiff;
}

////////////////////////////////////////////////////////////////////////

void set_thresh(const StringArray & a) {
   thresh.set(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_height(const StringArray & a) {
   height = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_width(const StringArray & a) {
   width = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_value(const StringArray & a) {
   mask_val = atof(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_name(const StringArray & a) {
   mask_name = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a) {
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a) {
   mlog.set_verbosity_level(atoi(a[0].c_str()));
}

////////////////////////////////////////////////////////////////////////

void set_compress(const StringArray & a) {
   compress_level = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_shapeno(const StringArray & a) {

   shape_number = atoi(a[0].c_str());

   if(shape_number < 0) {
      mlog << Error << "\n" << program_name << " -> "
           << "bad shapeno ... " << shape_number << "\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
