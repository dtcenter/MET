// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <regex.h>

#include "vx_met_util/vx_met_util.h"

///////////////////////////////////////////////////////////////////////////////

void process_grib_mask(const char *, const char *, const char *,
                       Grid &, WrfData &, char *&);
void process_netcdf_mask(const char *, const char *, const char *,
                         Grid &, WrfData &, char *&);
void process_poly_mask(const char *, const Grid &, WrfData &, char *&);

///////////////////////////////////////////////////////////////////////////////

void parse_grid_mask(const char *mask_grid_str, const Grid &grid,
                     WrfData &mask_wd, char *&mask_name) {
   int x, y;
   Grid mask_grid;

   // Initialize the WrfData masking object
   mask_wd.set_size(grid.nx(), grid.ny());
   for(x=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {
         mask_wd.put_xy_int(wrfdata_int_data_max, x, y);
      }
   }

   // Store the name of the masking grid
   mask_name = new char [strlen(mask_grid_str)+1];
   strcpy(mask_name, mask_grid_str);

   //
   // Check to make sure that we're not using the full domain
   //
   if(strcmp(full_domain_str, mask_grid_str) != 0) {

      //
      // Search for the grid name in the predefined grids
      //
      if(!find_grid_by_name(mask_name, mask_grid)) {
         cerr << "\n\nERROR: parse_grid_mask() -> "
              << "the mask_grid requested \"" << mask_grid_str
              << "\" is not defined.\n\n" << flush;
         exit(1);
      }

      apply_grid_mask(&grid, &mask_grid, mask_wd, 0);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void parse_grid_mask(const char *mask_grid_str,
                     Grid &mask_grid, WrfData &mask_wd) {
   int x, y;

   //
   // Search the predefined grids
   //
   if(!find_grid_by_name(mask_grid_str, mask_grid)) {
      cerr << "\n\nERROR: parse_grid_mask() -> "
           << "the mask_grid requested \"" << mask_grid_str
           << "\" is not defined.\n\n" << flush;
      exit(1);
   }

   //
   // Setup the WrfData masking object
   //
   mask_wd.set_size(mask_grid.nx(), mask_grid.ny());
   for(x=0; x<mask_grid.nx(); x++) {
      for(y=0; y<mask_grid.ny(); y++) {
         mask_wd.put_xy_int(wrfdata_int_data_max, x, y);
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// The mask_poly_str contains masking regions defined one of 3 ways:
//
// (1) An ASCII file containing a lat/lon polygon.
// (2) The NetCDF output of the gen_poly_mask tool.
// (3) A NetCDF data file, followed by the name of the NetCDF variable
//     to be used, and optionally, a threshold to be applied to the field.
// (4) A GRIB data file, followed by a description of the field
//     to be used, and optionally, a threshold to be applied to the field.
//
///////////////////////////////////////////////////////////////////////////////

void parse_poly_mask(const char *mask_poly_str, const Grid &grid,
                     WrfData &mask_wd, char *&mask_name) {
   char mask_poly_tmp[max_str_len];
   char file_name[max_str_len];
   char field_name[max_str_len];
   char thresh_str[max_str_len];
   char *ptr;
   FileType ftype;
   Grid mask_grid;

   // Replace any instances of MET_BASE with it's expanded value
   replace_string(met_base_str, MET_BASE, mask_poly_str, mask_poly_tmp);

   // Initialize the field name and threshold string
   strcpy(field_name, "");
   strcpy(thresh_str, "");

   // Tokenizing the string
   if((ptr = strtok(mask_poly_tmp, " ")) == NULL) {
      cerr << "\n\nERROR: parse_poly_mask() -> "
           << "must supply a file name to be used.\n\n" << flush;
      exit(1);
   }
   else {
      // Copy the file name
      strcpy(file_name, ptr);

      // Get the field name if present
      if((ptr = strtok(NULL, " ")) != NULL) strcpy(field_name, ptr);

      // Get the threshold string if present
      if((ptr = strtok(NULL, " ")) != NULL) strcpy(thresh_str, ptr);
   }

   // Allocate space for the mask name
   mask_name = new char [max_str_len];

   // Get the file type
   ftype = get_file_type(file_name);

   // Switch on the file type
   switch(ftype) {

      // Process a GRIB file
      case(GbFileType):

         process_grib_mask(file_name, field_name, thresh_str,
                           mask_grid, mask_wd, mask_name);
         break;

      // Process a NetCDF file
      case(NcFileType):

         process_netcdf_mask(file_name, field_name, thresh_str,
                             mask_grid, mask_wd, mask_name);

         break;

      // Process the mask file as a lat/lon polyline file
      default:

         mask_grid = grid;
         process_poly_mask(file_name, grid, mask_wd, mask_name);

         break;

   } // end switch

   // Check that the dimensions of the masking region match the dimensions
   // of the grid
   if(mask_grid.nx() != grid.nx() ||
      mask_grid.ny() != grid.ny()) {
      cerr << "\n\nERROR: parse_poly_mask() -> "
           << "the dimensions of the masking region ("
           << mask_grid.nx() << ", " << mask_grid.ny()
           << ") must match the dimensions of the data ("
           << grid.nx() << ", " << grid.ny() << ").\n\n" << flush;
      exit(1);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void parse_poly_mask(const char *mask_poly_str,
                     Grid &mask_grid, WrfData &mask_wd) {
   char mask_poly_tmp[max_str_len];
   char file_name[max_str_len];
   char field_name[max_str_len];
   char thresh_str[max_str_len];
   char *ptr;
   FileType ftype;

   char * mask_name = new char [max_str_len];

   // Replace any instances of MET_BASE with it's expanded value
   replace_string(met_base_str, MET_BASE, mask_poly_str, mask_poly_tmp);

   // Initialize the field name and threshold string
   strcpy(field_name, "");
   strcpy(thresh_str, "");

   // Tokenizing the string
   if((ptr = strtok(mask_poly_tmp, " ")) == NULL) {
      cerr << "\n\nERROR: parse_poly_mask() -> "
           << "must supply a file name to be used.\n\n" << flush;
      exit(1);
   }
   else {
      // Copy the file name
      strcpy(file_name, ptr);

      // Get the field name if present
      if((ptr = strtok(NULL, " ")) != NULL) strcpy(field_name, ptr);

      // Get the threshold string if present
      if((ptr = strtok(NULL, " ")) != NULL) strcpy(thresh_str, ptr);
   }

   // Get the file type
   ftype = get_file_type(file_name);

   // Switch on the file type
   switch(ftype) {

      // Process a GRIB file
      case(GbFileType):

         process_grib_mask(file_name, field_name, thresh_str,
                           mask_grid, mask_wd, mask_name);
         break;

      // Process a NetCDF file
      case(NcFileType):

         process_netcdf_mask(file_name, field_name, thresh_str,
                             mask_grid, mask_wd, mask_name);

         break;

      // Process the mask file as a lat/lon polyline file
      default:

         if(mask_grid.nx() == 0 || mask_grid.ny() == 0) {
            cerr << "\n\nERROR -> parse_poly_mask() -> "
                 << "When applying a masking lat/lon polyline, you must "
                 << "specify the grid to which it should be applied.\n\n"
                 << flush;
            exit(1);
         }
         process_poly_mask(file_name, mask_grid, mask_wd, mask_name);

         break;

   } // end switch

   if(mask_name) { delete mask_name; mask_name = (char *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void process_grib_mask(const char *file_name, const char *field_name,
                       const char *thresh_str,
                       Grid &mask_grid, WrfData &mask_wd, char *&mask_name) {
   SingleThresh st;
   GribFile gb_file;
   GribRecord rec;
   GCInfo gci;
   char tmp_str[max_str_len];
   int status;
   int v = 0;

   // Parse the threshold info
   if(strlen(thresh_str) > 0) st.set(thresh_str);

   // Check that a field name has been specified
   if(strlen(field_name) == 0) {
      cerr << "\n\nERROR: parse_grib_mask() -> "
           << "when using a GRIB file to define a masking region, "
           << "a field name must be specified.\n\n" << flush;
      exit(1);
   }

   // Open the GRIB file
   if(!(gb_file.open(file_name))) {
      cerr << "\n\nERROR: parse_grib_mask() -> "
           << "can't open GRIB file: "
           << file_name << "\n\n" << flush;
      exit(1);
   }

   // Parse the field name into a GCInfo object
   gci.set_gcinfo(field_name, 1);

   // Read the GRIB record
   status = get_grib_record(gb_file, rec,
                            gci, mask_wd, mask_grid, v);

   if(status != 0) {
      cerr << "\n\nERROR: parse_grib_mask() -> "
           << "can't find " << field_name << " field in the GRIB file: "
           << file_name << "\n\n" << flush;
      exit(1);
   }

   // If specified, apply the threshold
   if(strlen(thresh_str) > 0) mask_wd.threshold_double(st);

   // Construct the name of the masking region but first remove any '/'
   // characters from the field name
   replace_string("/", "_", field_name, tmp_str);

   if(strlen(thresh_str) > 0) sprintf(mask_name, "%s_%s", tmp_str, thresh_str);
   else                       strcpy(mask_name, tmp_str);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void process_netcdf_mask(const char *file_name, const char *field_name,
                         const char *thresh_str,
                         Grid &mask_grid, WrfData &mask_wd, char *&mask_name) {
   int i, n_vars, x, y, nx, ny;
   double v, v_min, v_max;
   char var_name[max_str_len];
   SingleThresh st;
   NcFile *nc_file;
   NcVar  *mask_var;
   NcDim  *xdim, *ydim;

   int    *int_data = (int *)    0;
   double *dbl_data = (double *) 0;
   float  *flt_data = (float *)  0;

   // Parse the threshold info
   if(strlen(thresh_str) > 0) st.set(thresh_str);

   // Open the NetCDF file
   nc_file = new NcFile(file_name);
   if(!nc_file || !(nc_file->is_valid())) {
      cerr << "\n\nERROR: parse_netcdf_mask() -> "
           << "can't open NetCDF file: "
           << file_name << "\n\n" << flush;
      exit(1);
   }

   // Parse out the Grid definition
   read_netcdf_grid(nc_file, mask_grid, 0);

   // If no variable name provided, find the first 2-dimensional variable
   if(strlen(field_name) == 0) {

      // Search for the first 2-dimensional variable
      n_vars = nc_file->num_vars();
      for(i=0; i<n_vars; i++) {

         // Retrieve ith variable
         mask_var = nc_file->get_var(i);
         if(!mask_var || !mask_var->is_valid()) {
            cerr << "\n\nERROR: parse_netcdf_mask() -> "
                 << "can't read the " << i << "th variable.\n\n"
                 << flush;
            exit(1);
         }

         // Check the number of dimensions
         if(mask_var->num_dims() == 2) break;
      } // end for

      // Check whether a suitable variable was found
      if(i==n_vars) {
         cerr << "\n\nERROR: parse_netcdf_mask() -> "
              << "can't find a 2-dimensional masking variable.\n\n"
              << flush;
         exit(1);
      }
   }
   // Otherwise, use the specified variable name
   else {

      // Access the masking variable in the NetCDF file
      mask_var = nc_file->get_var(field_name);
      if(!mask_var || !mask_var->is_valid()) {
         cerr << "\n\nERROR: parse_netcdf_mask() -> "
              << "\"" << field_name << "\" variable not found.\n\n"
              << flush;
         exit(1);
      }

      // Check that this variable has 2 dimensions
      if(mask_var->num_dims() != 2) {
         cerr << "\n\nERROR: parse_netcdf_mask() -> "
              << "the masking NetCDF variable \"" << field_name
              << "\" must have 2 dimensions.\n\n" << flush;
         exit(1);
      }
   }

   // Store the variable name
   strcpy(var_name, mask_var->name());

   // Retreive the 2 dimensions, ordered (y, x)
   ydim = mask_var->get_dim(0);
   xdim = mask_var->get_dim(1);

   // Check the dimensions
   if(!xdim || !ydim) {
      cerr << "\n\nERROR: parse_netcdf_mask() -> "
           << "trouble retreiving the NetCDF dimensions from variable \""
           << var_name << "\".\n\n" << flush;
      exit(1);
   }
   nx = (int) xdim->size();
   ny = (int) ydim->size();

   // Set up the size of the WrfData object
   mask_wd.set_size(nx, ny);

   // Allocate space for the mask data depending on the type of variable
   // and read the data
   switch(mask_var->type()) {

      case(ncInt):

         int_data = new int [nx*ny];

         if(!mask_var->get(&int_data[0], ny, nx)) {
            cerr << "\n\nERROR: parse_netcdf_mask() -> "
                 << "error with the mask_var->get integer.\n\n" << flush;
            exit(1);
         }

         // Read through the data to find the min/max values
         v_min =  1.0e30;
         v_max = -1.0e30;

         for(x=0; x<nx; x++) {
            for(y=0; y<ny; y++) {
               v = (double) int_data[mask_wd.two_to_one(x, y)];
               if(!is_bad_data(v) && v > v_max) v_max = v;
               if(!is_bad_data(v) && v < v_min) v_min = v;
            }
         }

         // Set up the wrfdata object
         mask_wd.set_b(v_min);
         mask_wd.set_m((double) (v_max-v_min)/wrfdata_int_data_max);

         // Parse the mask data into the WrfData object
         for(x=0; x<nx; x++) {
            for(y=0; y<ny; y++) {
               v = (double) int_data[mask_wd.two_to_one(x, y)];
               mask_wd.put_xy_double(v, x, y);
            }
         }

         break;

      case(ncFloat):

         flt_data = new float [nx*ny];

         if(!mask_var->get(&flt_data[0], ny, nx)) {
            cerr << "\n\nERROR: parse_netcdf_mask() -> "
                 << "error with the mask_var->get float.\n\n" << flush;
            exit(1);
         }

         // Read through the data to find the min/max values
         v_min =  1.0e30;
         v_max = -1.0e30;

         for(x=0; x<nx; x++) {
            for(y=0; y<ny; y++) {
               v = (double) flt_data[mask_wd.two_to_one(x, y)];
               if(!is_bad_data(v) && v > v_max) v_max = v;
               if(!is_bad_data(v) && v < v_min) v_min = v;
            }
         }

         // Set up the wrfdata object
         mask_wd.set_b(v_min);
         mask_wd.set_m((double) (v_max-v_min)/wrfdata_int_data_max);

         // Parse the mask data into the WrfData object
         for(x=0; x<nx; x++) {
            for(y=0; y<ny; y++) {
               v = (double) flt_data[mask_wd.two_to_one(x, y)];
               mask_wd.put_xy_double(v, x, y);
            }
         }

         break;

      case(ncDouble):

         dbl_data = new double [nx*ny];

         if(!mask_var->get(&dbl_data[0], ny, nx)) {
            cerr << "\n\nERROR: parse_netcdf_mask() -> "
                 << "error with the mask_var->get double.\n\n" << flush;
            exit(1);
         }

         // Read through the data to find the min/max values
         v_min =  1.0e30;
         v_max = -1.0e30;

         for(x=0; x<nx; x++) {
            for(y=0; y<ny; y++) {
               v = (double) dbl_data[mask_wd.two_to_one(x, y)];
               if(!is_bad_data(v) && v > v_max) v_max = v;
               if(!is_bad_data(v) && v < v_min) v_min = v;
            }
         }

         // Set up the wrfdata object
         mask_wd.set_b(v_min);
         mask_wd.set_m((double) (v_max-v_min)/wrfdata_int_data_max);

         // Parse the mask data into the WrfData object
         for(x=0; x<nx; x++) {
            for(y=0; y<ny; y++) {
               v = (double) dbl_data[mask_wd.two_to_one(x, y)];
               mask_wd.put_xy_double(v, x, y);
            }
         }

         break;

      default:
         cerr << "\n\nERROR: parse_netcdf_mask() -> "
              << "unexpected variable type \""
              << mask_var->type() << "\".\n\n" << flush;
         exit(1);
         break;
   } // end switch

   // Delete allocated memory
   if(int_data) { delete int_data; int_data = (int *)    0; }
   if(flt_data) { delete flt_data; flt_data = (float *)  0; }
   if(dbl_data) { delete dbl_data; dbl_data = (double *) 0; }

   // If specified, apply the threshold
   if(strlen(thresh_str) > 0) mask_wd.threshold_double(st);

   // Construct the name of the masking region
   if(strlen(thresh_str) > 0)
      sprintf(mask_name, "%s_%s", mask_var->name(), thresh_str);
   else
      strcpy(mask_name, mask_var->name());

   return;
}

///////////////////////////////////////////////////////////////////////////////

void process_poly_mask(const char *file_name, const Grid &grid,
                       WrfData &mask_wd, char *&mask_name) {
   int x, y;
   Polyline mask_poly;

   // Initialize the WrfData masking object
   mask_wd.set_size(grid.nx(), grid.ny());
   for(x=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {
         mask_wd.put_xy_int(wrfdata_int_data_max, x, y);
      }
   }

   // Initialize the masking polyline
   mask_poly.clear();

   // Parse out the polyline from the file
   parse_latlon_poly_file(file_name, mask_poly);

   // Store the name of the masking polyline
   strcpy(mask_name, mask_poly.name);

   // Apply the polyline mask to the WrfData object
   apply_poly_mask_latlon(&mask_poly, &grid, mask_wd, 0);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void parse_sid_mask(const char *mask_sid_file, StringArray &mask_sid) {
   ifstream in;
   char tmp_file[PATH_MAX], sid_str[max_str_len];

   // Replace any instances of MET_BASE with it's expanded value
   replace_string(met_base_str, MET_BASE, mask_sid_file, tmp_file);

   // Check for an empty length string
   if(strlen(tmp_file) == 0) return;

   // Open the mask station id file specified
   in.open(tmp_file);

   if(!in) {
      cerr << "\n\nERROR: parse_sid_mask() -> "
           << "Can't open the mask station ID file specified ("
           << tmp_file << ").\n\n" << flush;
      exit(1);
   }

   // Read in and store the masking station ID names
   while(in >> sid_str) mask_sid.add(sid_str);

   // Close the input file
   in.close();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void apply_grid_mask(const Grid *grid, const Grid *mask_grid, WrfData &wd, int v_int) {
   int x, y;
   double lat, lon, mask_x, mask_y;
   int in_count, out_count;

   //
   // If the grid pointer is NULL or the grid's dimensions are zero,
   // no masking region is defined and there is nothing to do
   //
   if(!mask_grid || mask_grid->nx() == 0 || mask_grid->ny() == 0) {
      cout << "***WARNING*** void apply_grid_mask() -> "
           << "no grid masking applied since the masking grid "
           << "specified is empty\n"
           << flush;

      return;
   }

   //
   // For each point of the original grid, convert it to (lat, lon) and then
   // to an (x, y) point on the mask grid. If the (x, y) is on within the
   // bounds of the mask grid, retain it's value.  Otherwise, replace it
   // with the value of v_int
   //
   in_count = out_count = 0;
   for(x=0; x<wd.get_nx(); x++) {
      for(y=0; y<wd.get_ny(); y++) {

         grid->xy_to_latlon(x, y, lat, lon);
         mask_grid->latlon_to_xy(lat, lon, mask_x, mask_y);

         if(mask_x < 0 || mask_x >= mask_grid->nx() ||
            mask_y < 0 || mask_y >= mask_grid->ny()) {
            out_count++;
            wd.put_xy_int(v_int, x, y);
         }
         else {
            in_count++;
         }
      } // for y
   } // for x

   return;
}

///////////////////////////////////////////////////////////////////////////////

void apply_poly_mask_xy(const Polyline *p, WrfData &wd, int v_int) {
   int x, y;
   int in_count, out_count;

   //
   // If the polyline pointer is NULL or the polyline contains less than 3
   // points, no masking region is defined and there is nothing to do
   //
   if(!p || p->n_points < 3) {
      cout << "***WARNING*** void apply_poly_mask_xy() -> "
           << "no polyline masking applied since the masking polyline "
           << "contains fewer than 3 points\n"
           << flush;

      return;
   }

   //
   // Replace any grid points not inside the masking polygon with the value
   // of v_int
   //
   in_count = out_count = 0;
   for(x=0; x<wd.get_nx(); x++) {
      for(y=0; y<wd.get_ny(); y++) {

         if(!(p->is_inside(x, y))) {
            out_count++;
            wd.put_xy_int(v_int, x, y);
         }
         else {
            in_count++;
         }
      } // for y
   } // for x

   return;
}

///////////////////////////////////////////////////////////////////////////////

void apply_poly_mask_latlon(const Polyline *p, const Grid *grid, WrfData &wd, int v_int) {
   int x, y;
   double lat, lon;
   int in_count, out_count;

   //
   // If the polyline pointer is NULL or the polyline contains less than 3
   // points, no masking region is defined and there is nothing to do
   //
   if(!p || p->n_points < 3) {
      cout << "***WARNING*** void apply_poly_mask_latlon() -> "
           << "no polyline masking applied since the masking polyline "
           << "contains fewer than 3 points\n"
           << flush;

      return;
   }

   //
   // Replace any grid points who's corresponding lat/lon coordinates are
   // not inside the masking lat/lon polygon with the value of v_int
   //
   in_count = out_count = 0;
   for(x=0; x<wd.get_nx(); x++) {
      for(y=0; y<wd.get_ny(); y++) {

         grid->xy_to_latlon(x, y, lat, lon);

         if(!(p->is_inside(lon, lat))) {
            out_count++;
            wd.put_xy_int(v_int, x, y);
         }
         else {
            in_count++;
         }
      } // for y
   } // for x

   return;
}

///////////////////////////////////////////////////////////////////////////////
