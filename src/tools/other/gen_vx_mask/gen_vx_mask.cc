// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
//   009    06/01/21  Seth Linden     Change -type from optional to required.
//   010    08/30/21  Halley Gotway   MET #1891 Fix input and mask fields.
//   011    12/13/21  Halley Gotway   MET #1993 Fix -type grid.
//   012    05/05/22  Halley Gotway   MET #2152 Add -type poly_xy.
//   013    07/06/22  Howard Soh      METplus-Internal #19 Rename main to met_main
//   014    09/28/22  Prestopnik      MET #2227 Remove namespace std and netCDF from header files
//   015    05/03/23  Halley Gotway   MET #1060 Support multiple shapes
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <dirent.h>
#include <fstream>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <netcdf>
using namespace netCDF;

#include "main.h"
#include "gen_vx_mask.h"

#include "grib_classes.h"

#include "vx_log.h"
#include "nav.h"
#include "vx_math.h"
#include "vx_nc_util.h"
#include "data2d_nc_met.h"
#include "solar.h"
#include "dbf_file.h"
#include "shp_file.h"
#include "grid_closed_poly.h"

////////////////////////////////////////////////////////////////////////

int met_main(int argc, char *argv[]) {
   static DataPlane dp_data, dp_mask, dp_out;

   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the input grid
   process_input_grid(dp_data);

   // Process the mask file
   process_mask_file(dp_mask);

   // Apply combination logic if the current mask is binary
   if(mask_type == MaskType_Poly    ||
      mask_type == MaskType_Poly_XY ||
      mask_type == MaskType_Shape   ||
      mask_type == MaskType_Box     ||
      mask_type == MaskType_Grid    ||
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

const string get_tool_name() {
   return "gen_vx_mask";
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;

   // Check for zero arguments
   if(argc == 1) usage();

   // Initialize the configuration object
   global_config.read(replace_path(config_const_filename).c_str());

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
   cline.add(set_compress,     "-compress",     1);
   cline.add(set_shapeno,      "-shapeno",      1);
   cline.add(set_shape_str,    "-shape_str",    2);

   cline.allow_numbers();

   // Parse the command line
   cline.parse();

   // There should be two arguments left, the input/output file names
   if(cline.n() != 3) usage();

   // Store the arguments
   input_gridname = cline[0];
   mask_filename  = cline[1];
   out_filename   = cline[2];

   // Check for the mask type (from -type string)
   if(mask_type == MaskType_None) {
     mlog << Error << "\n" << program_name << " -> "
          << "the -type command line requirement must be set to a specific masking type!\n"
          << "\t\t   \"poly\", \"box\", \"circle\", \"track\", \"grid\", "
          << "\"data\", \"solar_alt\", \"solar_azi\", \"lat\", \"lon\" "
          << "or \"shape\"" << "\n\n";
     exit(1);
   }
   
   // List the input files
   mlog << Debug(1)
        << "Input Grid:\t\t" << input_gridname << "\n"
        << "Mask File:\t\t"  << mask_filename  << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void process_input_grid(DataPlane &dp) {

   // Parse the input grid as a white-space separated string
   StringArray sa;
   sa.parse_wsss(input_gridname);

   // Search for a named grid
   if(sa.n() == 1 && find_grid_by_name(sa[0].c_str(), grid)) {
      mlog << Debug(3)
           << "Use input grid named \"" << input_gridname << "\".\n";
   }
   // Parse grid definition
   else if(sa.n() > 1 && parse_grid_def(sa, grid)) {
      mlog << Debug(3)
           << "Use input grid defined by string \"" << input_gridname
           << "\".\n";
   }
   // Extract the grid from a gridded data file
   else {

      mlog << Debug(3)
           << "Use input grid defined by file \"" << input_gridname
           << "\".\n";

      // Read the input grid and data plane, if requested
      get_data_plane(input_gridname, input_field_str, true, dp, grid);
   }

   // If not yet set, fill the input data plane with zeros
   if(dp.is_empty()) {
      dp.set_size(grid.nx(), grid.ny());
      dp.set_constant(0.0);
   }

   mlog << Debug(2)
        << "Parsed Input Grid:\t" << grid.name()
        << " (" << grid.nx() << " x " << grid.ny() << ")\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void process_mask_file(DataPlane &dp) {

   // Initialize
   solar_ut = (unixtime) 0;

   // Process the mask file as a lat/lon polyline file
   if(mask_type == MaskType_Poly    ||
      mask_type == MaskType_Poly_XY ||
      mask_type == MaskType_Box     ||
      mask_type == MaskType_Circle  ||
      mask_type == MaskType_Track) {

      poly_mask.clear();
      poly_mask.load(mask_filename.c_str());

      mlog << Debug(2)
           << "Parsed Lat/Lon Mask:\t" << poly_mask.name()
           << " containing " << poly_mask.n_points() << " points\n";
   }

   // Process the mask from a shapefile
   else if(mask_type == MaskType_Shape) {

      // If -shape_str was specified, find the matching records
      if(shape_str_map.size() > 0) get_shapefile_strings();

      // Get the records specified by -shapeno and -shape_str
      get_shapefile_records();

      int n_pts;
      vector<ShpPolyRecord>::const_iterator it;
      for(it  = shape_recs.begin(), n_pts=0;
          it != shape_recs.end(); ++it) {
         n_pts += it->n_points;
      }

      mlog << Debug(2)
           << "Parsed Shape Mask:\t" << mask_filename
           << " using " << shape_recs.size()
           << " shape(s) containing a total of "
           << n_pts << " points\n";
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

   // Otherwise, process the mask file as a named grid, grid specification
   // string or gridded data file
   else {

      // For the grid mask type, support named grids and grid
      // specification strings
      if(mask_type == MaskType_Grid) {

         // Parse the mask file as a white-space separated string
         StringArray sa;
         sa.parse_wsss(mask_filename);

         // Search for a named grid
         if(sa.n() == 1 && find_grid_by_name(sa[0].c_str(), grid_mask)) {
            mlog << Debug(3)
                 << "Use mask grid named \"" << mask_filename << "\".\n";
         }
         // Parse grid definition
         else if(sa.n() > 1 && parse_grid_def(sa, grid_mask)) {
            mlog << Debug(3)
                 << "Use mask grid defined by string \"" << mask_filename
                 << "\".\n";
         }
      }

      // Parse as a gridded data file if not already set
      if(grid_mask.nxy() == 0) {

         // Extract the grid from a gridded data file
         mlog << Debug(3)
              << "Use mask grid defined by file \"" << mask_filename
              << "\".\n";

         // Read the mask grid and data plane, if requested
         get_data_plane(mask_filename, mask_field_str,
                        mask_type == MaskType_Data,
                        dp, grid_mask);
      }

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

      case MaskType_Poly_XY:
         apply_poly_xy_mask(dp);
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
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void get_data_plane(const ConcatString &file_name,
                    const ConcatString &config_str,
                    bool read_gen_vx_mask_output,
                    DataPlane &dp, Grid &dp_grid) {
   ConcatString local_cs = config_str;
   GrdFileType ftype = FileType_None;

   // Initialize to the global configuration
   MetConfig local_config = global_config;

   // Parse non-empty config strings
   if(local_cs.length() > 0) {
      local_config.read_string(local_cs.c_str());
      ftype = parse_conf_file_type(&local_config);
   }

   // Attempt to open the data file
   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile *mtddf_ptr = (Met2dDataFile *) 0;
   mtddf_ptr = mtddf_factory.new_met_2d_data_file(file_name.c_str(), ftype);
   if(!mtddf_ptr) {
      mlog << Error << "\nget_data_plane() -> "
           << "can't open input file \"" << file_name << "\"\n\n";
      exit(1);
   }

   // Read gen_vx_mask output from a previous run
   if(read_gen_vx_mask_output &&
      local_cs.length()      == 0 &&
      mtddf_ptr->file_type() == FileType_NcMet) {
      if(get_gen_vx_mask_config_str((MetNcMetDataFile *) mtddf_ptr, local_cs)) {
         local_config.read_string(local_cs.c_str());
      }
   }

   // Read data plane, if requested
   if(local_cs.length() > 0) {

      // Allocate new VarInfo object
      VarInfoFactory vi_factory;
      VarInfo *vi_ptr = (VarInfo *) 0;
      vi_ptr = vi_factory.new_var_info(mtddf_ptr->file_type());
      if(!vi_ptr) {
         mlog << Error << "\nget_data_plane() -> "
              << "can't allocate new VarInfo pointer.\n\n";
         exit(1);
      }

      // Read config into the VarInfo object
      vi_ptr->set_dict(local_config);

      // Get data plane from the file for this VarInfo object
      if(!mtddf_ptr->data_plane(*vi_ptr, dp)) {
         mlog << Error << "\nget_data_plane() -> "
              << "trouble reading field \"" << local_cs
              << "\" from file \"" << mtddf_ptr->filename() << "\"\n\n";
         exit(1);
      }

      // Dump the range of data values read
      double dmin, dmax;
      dp.data_range(dmin, dmax);
      mlog << Debug(3)
           << "Read field \"" << vi_ptr->magic_str() << "\" from \""
           << mtddf_ptr->filename() << "\" with data ranging from "
           << dmin << " to " << dmax << ".\n";

      // Clean up
      if(vi_ptr) { delete vi_ptr; vi_ptr = (VarInfo *) 0; }

   } // end if

   // Extract the grid
   dp_grid = mtddf_ptr->grid();

   // Clean up
   if(mtddf_ptr) { delete mtddf_ptr; mtddf_ptr = (Met2dDataFile *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

bool get_gen_vx_mask_config_str(MetNcMetDataFile *mnmdf_ptr,
                                ConcatString &config_str) {
   bool status = false;
   ConcatString tool;
   int i;

   // Check for null pointer
   if(!mnmdf_ptr) return(status);

   // Check for the MET_tool global attribute
   if(!get_global_att(mnmdf_ptr->MetNc->Nc, (string) "MET_tool", tool)) return(status);

   // Check for gen_vx_mask output
   if(tool != program_name) return(status);

   // Loop through the NetCDF variables
   for(i=0; i<mnmdf_ptr->MetNc->Nvars; i++) {

      // Skip the lat/lon variables
      if(mnmdf_ptr->MetNc->Var[i].name == "lat" ||
         mnmdf_ptr->MetNc->Var[i].name == "lon") continue;

      // Read the first non-lat/lon variable
      config_str << cs_erase
                 << "'name=\"" << mnmdf_ptr->MetNc->Var[i].name
                 << "\"; level=\"(*,*)\";'";
      status = true;
      break;
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

void get_shapefile_strings() {
   DbfFile f;
   StringArray rec_names;
   StringArray rec_values;

   // Get the corresponding dbf file name
   ConcatString dbf_filename = mask_filename;
   dbf_filename.replace(".shp", ".dbf");

   mlog << Debug(3) << "Shape dBASE file:\t"
        << dbf_filename << "\n";

   // Open the database file
   if(!(f.open(dbf_filename.c_str()))) {
      mlog << Error << "\nget_shapefile_strings() -> "
           << "unable to open database file \"" << dbf_filename
           << "\"\n\n";
      exit(1);
   }

   // Get the subrecord names, ignoring case
   rec_names = f.subrecord_names();
   rec_names.set_ignore_case(true);

   // Print the subrecord names
   mlog << Debug(4) << "Filtering shapes using "
        << shape_str_map.size() << " of the " << rec_names.n()
        << " available attributes (" << write_css(rec_names)
        << ").\n";

   // Check that the attributes requested actually exist
   map<string,StringArray>::const_iterator it;
   for(it  = shape_str_map.begin();
       it != shape_str_map.end(); it++) {

      if(!rec_names.has(it->first)) {
         mlog << Warning << "\nget_shapefile_strings() -> "
              << "the \"-shape_str\" name \"" << it->first
              << "\" is not in the list of " << rec_names.n()
              << " shapefile attributes and will be ignored:\n"
              << write_css(rec_names) << "\n\n";
      }
   }

   // Check each record
   for(int i=0; i<f.header()->n_records; i++) {

      // Get the values for the current record
      rec_values = f.subrecord_values(i);

      // Add matching records to the list
      if(is_shape_str_match(i, rec_names, rec_values)) {
         mlog << Debug(4) << "Shape number " << i
              << " is a shapefile match.\n";
         if(!shape_numbers.has(i)) shape_numbers.add(i);
      }
   }

   // Close the database file
   f.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void get_shapefile_records() {
   const char * const shape_filename = mask_filename.c_str();
   ShpFile f;
   ShpPolyRecord pr;

   // Check the number of requested shapes
   if(shape_numbers.n() == 0) {
      mlog << Error << "\nget_shapefile_records() -> "
           << "at least one shape number must be specified using "
           << "the \"-shapeno\" or \"-shape_str\" command line options!\n\n";
      exit(1);
   }

   // Open shapefile
   if(!(f.open(shape_filename))) {
      mlog << Error << "\nget_shapefile_records() -> "
           << "unable to open shape file \"" << shape_filename
           << "\"\n\n";
      exit(1);
   }

   // Make sure it's a polygon file, and not some other type
   if(f.shape_type() != shape_type_polygon) {
      mlog << Error << "\nget_shapefile_records() -> "
           << "shape file \"" << shape_filename
           << "\" is not a polygon file\n\n";
      exit(1);
   }

   // Get the maximum shape number
   int max_shape_number = nint(shape_numbers.max());

   // Loop through and store the specified records
   for(int i=0; i<=max_shape_number; i++) {

      // Check for end of file
      if(f.at_eof()) break;

      // Read the current record
      f >> pr;

      // Store requested records
      if(shape_numbers.has(i)) {
         mlog << Debug(3)
              << "Using shape number " << i << " with "
              << pr.n_points << " points.\n";
         pr.toggle_longitudes();
         shape_recs.push_back(pr);
      }
   }

   // Check for end of file
   if(f.at_eof()) {
      mlog << Error << "\nget_shapefile_records() -> "
           << "hit eof before reading all specified record(s)\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool is_shape_str_match(const int i_shape, const StringArray &names, const StringArray &values) {
   bool match = true;
   int i_match;

   // Check each map entry
   map<string,StringArray>::const_iterator it;
   for(it  = shape_str_map.begin();
       it != shape_str_map.end(); it++) {

      // Ignore names that do not exist in the shapefile
      if(!names.has(it->first, i_match)) continue;

      // The corresponding value must match
      if(!it->second.has(values[i_match].c_str())) {
         mlog << Debug(4) << "Shape number " << i_shape << " \""
              << it->first << "\" value (" << values[i_match]
              << ") does not match (" << write_css(it->second)
              << ")\n";
         match = false;
         break;
      }
      else {
         mlog << Debug(3) << "Shape number " << i_shape << " \""
              << it->first << "\" value (" << values[i_match]
              << ") matches (" << write_css(it->second)
              << ")\n";
      }

   }

   return(match);
}

////////////////////////////////////////////////////////////////////////

void apply_poly_mask(DataPlane & dp) {
   int x, y, n_in;
   bool inside;
   double lat, lon;

   // Check the Lat/Lon of each grid point being inside the polyline
   for(x=0,n_in=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         // Lat/Lon value for the current grid point
         grid.xy_to_latlon(x, y, lat, lon);

         // Check current grid point inside polyline
         inside = poly_mask.latlon_is_inside(lat, lon);   //  returns bool

         // Check the complement
         if(complement) inside = !inside;

         // Increment count
         if(inside) n_in++;

         // Store the current mask value
         dp.set((inside ? 1.0 : 0.0), x, y);

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

void apply_poly_xy_mask(DataPlane & dp) {
   int i, x, y, n_in;
   bool inside;
   double x_dbl, y_dbl;
   GridClosedPoly poly_xy;

   // Convert MaskPoly Lat/Lon coordinates to Grid X/Y
   for(i=0; i<poly_mask.n_points(); i++) {
      grid.latlon_to_xy(poly_mask.lat(i), poly_mask.lon(i), x_dbl, y_dbl);
      poly_xy.add_point(x_dbl, y_dbl);
   }

   // Check the X/Y of each grid point being inside the polyline
   for(x=0,n_in=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         // Check current grid point inside polyline
         inside = (poly_xy.is_inside((double) x, (double) y) != 0);

         // Check the complement
         if(complement) inside = !inside;

         // Increment count
         if(inside) n_in++;

         // Store the current mask value
         dp.set((inside ? 1.0 : 0.0), x, y);

      } // end for y
   } // end for x

   if(complement) {
      mlog << Debug(3)
           << "Applying complement of polyline XY mask.\n";
   }

   // List number of points inside the mask
   mlog << Debug(3)
        << "Polyline XY Masking:\t" << n_in << " of " << grid.nx() * grid.ny()
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
   for(x=0,n_in=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         inside = (dp(x, y) == 1);

         // Check the complement
         if(complement) inside = !inside;

         // Increment count
         n_in += inside;

         // Store the current mask value
         dp.set(inside, x, y);

      } // end for y
   } // end for x

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
   bool status = false;

   // Load the shapes
   GridClosedPolyArray poly;
   vector<GridClosedPolyArray> poly_list;
   vector<ShpPolyRecord>::const_iterator rec_it;
   for(rec_it  = shape_recs.begin();
       rec_it != shape_recs.end(); ++rec_it) {
      poly.set(*rec_it, grid);
      poly_list.push_back(poly);
   }

   // Check grid points
   for(x=0,n_in=0; x<(grid.nx()); x++) {
      for(y=0; y<(grid.ny()); y++) {

         vector<GridClosedPolyArray>::const_iterator poly_it;
         for(poly_it  = poly_list.begin();
             poly_it != poly_list.end(); ++poly_it) {

            // Check if point is inside
            status = poly_it->is_inside(x, y);

            // Break after the first match
            if(status) break;
         }

         // Check the complement
         if(complement) status = !status;

         // Increment count
         if(status) n_in++;

         // Store the current mask value
         dp.set((status ? 1.0 : 0.0 ), x, y);

      } // end for y
   } // end for x

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
           << "Mask " << setlogic_to_string(logic)
	   << (logic == SetLogic_Intersection ? ":\t" : ":\t\t")
           << n_in << " of " << grid.nx() * grid.ny()
	   << " points inside\n";
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
   write_netcdf_proj(f_out, grid, lat_dim, lon_dim);

   // Add the lat/lon variables
   write_netcdf_latlon(f_out, &lat_dim, &lon_dim, grid);

   // Set the mask_name, if not already set
   if(mask_name.length() == 0) {
      if(mask_type == MaskType_Poly    ||
         mask_type == MaskType_Poly_XY ||
         mask_type == MaskType_Circle  ||
         mask_type == MaskType_Track) {
         mask_name = poly_mask.name();
      }
      else {
         mask_name << masktype_to_string(mask_type) << "_mask";
      }
   }

   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = global_config.nc_compression();

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
   else if(strcasecmp(s, "poly_xy")   == 0) t = MaskType_Poly_XY;
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
      case MaskType_Poly_XY:   s = "poly_xy";        break;
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
        << "\t-type string\n"
        << "\t[-input_field string]\n"
        << "\t[-mask_field string]\n"
        << "\t[-complement]\n"
        << "\t[-union | -intersection | -symdiff]\n"
        << "\t[-thresh string]\n"
        << "\t[-height n]\n"
        << "\t[-width n]\n"
        << "\t[-shapeno n]\n"
        << "\t[-shape_str name string]\n"
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
        << "\t\t   For \"poly\", \"poly_xy\", \"box\", \"circle\", "
        << "and \"track\" masking, specify an ASCII Lat/Lon file.\n"
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

        << "\t\t\"-type string\" specify the masking type "
        << "(required).\n"
        << "\t\t   \"poly\", \"poly_xy\", \"box\", \"circle\", \"track\", "
        << "\"grid\", \"data\", \"solar_alt\", \"solar_azi\", \"lat\", "
        << "\"lon\" or \"shape\"\n"
        
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
        << "\t\t   For \"shape\" masking, specify the integer shape "
        << "number(s) (0-based) to be used as a comma-separated list.\n"

        << "\t\t\"-shape_str name string\" (optional).\n"
        << "\t\t   For \"shape\" masking, specify the shape(s) to be used "
        << "as a named attribute followed by a comma-separated list of "
        << "matching strings. If used multiple times, only shapes matching "
        << "all named attributes will be used.\n"

        << "\t\t\"-value n\" overrides the default output mask data "
        << "value (" << default_mask_val << ") (optional).\n"

        << "\t\t\"-name string\" specifies the output variable name "
        << "for the mask (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t\"-compress level\" overrides the compression level of "
        << "NetCDF variable (" << global_config.nc_compression()
        << ") (optional).\n\n"

        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_type(const StringArray & a) {
   if(type_is_set) {
      mlog << Error << "\n" << program_name << " -> "
           << "the -type command line requirement can only be used once!\n"
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

void set_compress(const StringArray & a) {
   compress_level = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_shapeno(const StringArray & a) {
   NumArray cur_na;

   cur_na.add_css(a[0].c_str());

   // Check and add each unique shape number
   for(int i=0; i<cur_na.n(); i++) {
      if(cur_na[i] < 0) {
         mlog << Error << "\n" << program_name << " -> "
              << "bad shapeno ... " << cur_na[i] << "\n\n";
         exit(1);
      }
      else if(!shape_numbers.has(cur_na[i])) {
         shape_numbers.add(cur_na[i]);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void set_shape_str(const StringArray & a) {
   StringArray sa;

   // Comma-separated list of matching strings, ignoring case
   sa.parse_css(a[1]);
   sa.set_ignore_case(true);

   // Append options to existing entry
   if(shape_str_map.count(a[0]) > 0) {
      shape_str_map[a[0]].add(sa);
   }
   // Or add a new entry
   else {
      shape_str_map[a[0]] = sa;
   }

   return;
}

////////////////////////////////////////////////////////////////////////
