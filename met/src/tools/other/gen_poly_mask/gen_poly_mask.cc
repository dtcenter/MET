// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   gen_poly_mask.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    10/23/08  Halley Gotway   New
//   001    07/27/10  Halley Gotway  Add lat/lon variables to NetCDF.
//   002    08/04/11  Halley Gotway  Enhance to allow the masking area
//                    to be specified as a lat/lon polyline file or
//                    gridded data file.  Also, add -name option.
//   003    10/13/11  Holmes          Added use of command line class to
//                                    parse the command line arguments.
//   004    12/20/11  Bullock         Ported to new repository
//   005    11/12/12  Halley Gotway   Fix bug masking grids.
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
#include <unistd.h>

#include "netcdf.hh"
#include "grib_classes.h"

#include "vx_log.h"
#include "data2d_factory.h"
#include "mask_poly.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "write_netcdf.h"


////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

////////////////////////////////////////////////////////////////////////
//
// Variables for command line arguments
//
////////////////////////////////////////////////////////////////////////

// Input data file, polyline file, and output NetCDF file
static ConcatString data_filename;
static ConcatString mask_filename;
static ConcatString out_filename;
static ConcatString mask_name;

// Masking polyline
static MaskPoly poly_mask;

// Grid on which the data field resides
static Grid grid, grid_mask;

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_data_file();
static void process_mask_file();
static void write_netcdf();
static void usage();
static void set_mask_name(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

program_name = get_short_name(argv[0]);

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the data file and extract the grid information
   process_data_file();

   // Process the mask file
   process_mask_file();

   // Write out the mask file to NetCDF
   write_netcdf();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv)

{

CommandLine cline;

   //
   // check for zero arguments
   //
   if (argc == 1)
      usage();

   //
   // parse the command line into tokens
   //
   cline.set(argc, argv);

   //
   // set the usage function
   //
   cline.set_usage(usage);

   //
   // add the options function calls
   //
   cline.add(set_mask_name, "-name", 1);
   cline.add(set_logfile,   "-log",  1);
   cline.add(set_verbosity, "-v",    1);

   //
   // parse the command line
   //
   cline.parse();

   //
   // Check for error. There should be three arguments left; the data
   // filename, the mask filename, and the netCDF filename.
   //
   if (cline.n() != 3)
      usage();

   // Store the arguments
   data_filename = cline[0];
   mask_filename = cline[1];
   out_filename  = cline[2];

   // List the input files
   mlog << Debug(1)
        << "Input Data File:\t" << data_filename << "\n"
        << "Input Mask File:\t" << mask_filename << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void process_data_file()

{

Met2dDataFileFactory factory;
Met2dDataFile * datafile = (Met2dDataFile *) 0;

datafile = factory.new_met_2d_data_file(data_filename);

if ( !datafile )  {

   mlog << Error << "\nprocess_data_file() -> "
        << "can't open data file \"" << data_filename << "\"\n\n";

   exit ( 1 );

}

grid = datafile->grid();

delete datafile;  datafile = (Met2dDataFile *) 0;

mlog << Debug(2)
     << "Parsed Input Grid:\t" << grid.name()
     << " (" << grid.nx() << " x " << grid.ny() << ")\n";


   //
   //  done
   //

return;
}

////////////////////////////////////////////////////////////////////////

void process_mask_file()

{

   //
   //  first, assume the mask file is a gridded data file
   //

Met2dDataFileFactory factory;
Met2dDataFile * datafile = (Met2dDataFile *) 0;

datafile = factory.new_met_2d_data_file(mask_filename);

if ( datafile )  {

   grid_mask = datafile->grid();

   delete datafile;  datafile = (Met2dDataFile *) 0;

   mlog << Debug(2)
        << "Parsed Masking Grid:\t" << mask_name
        << " (" << grid_mask.nx() << " x " << grid_mask.ny()
        << ")\n";

}   //  if datafile

   //
   //  ok, assume it's an ascii polyline file
   //

else {

   // Parse out the polyline from the file
   poly_mask.clear();

   poly_mask.load(mask_filename);

   // If not already specified, retrieve the mask name

   if(mask_name.length() == 0) mask_name = poly_mask.name();

   mlog << Debug(2)
        << "Parsed Polyline:\t" << mask_name
        << " containing " << poly_mask.n_points() << " points\n";

}

   //
   //  done
   //

return;

}

////////////////////////////////////////////////////////////////////////

void write_netcdf()

{

   int n, x, y, xx, yy, in_count, out_count;
   double lat, lon, x_dbl, y_dbl;
   char var_str[max_str_len];

   int *mask_data = (int *)   0;

   NcFile *f_out   = (NcFile *) 0;
   NcDim *lat_dim  = (NcDim *) 0;
   NcDim *lon_dim  = (NcDim *) 0;
   NcVar *mask_var = (NcVar *) 0;

   // Create a new NetCDF file and open it.
   f_out = new NcFile(out_filename, NcFile::Replace);

   if(!f_out->is_valid()) {
      mlog << Error << "\nwrite_netcdf() -> "
           << "trouble opening output file " << out_filename
           << "\n\n";
      f_out->close();
      delete f_out;
      f_out = (NcFile *) 0;

      exit(1);
   }

   // Add global attributes
   write_netcdf_global(f_out, out_filename, program_name);

   // Add the projection information
   write_netcdf_proj(f_out, grid);

   // Define Dimensions
   lat_dim = f_out->add_dim("lat", (long) grid.ny());
   lon_dim = f_out->add_dim("lon", (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(f_out, lat_dim, lon_dim, grid);

   // Check that mask_name is set
   if(!mask_name) {
      mlog << Error << "\nwrite_netcdf() -> "
           << "the masking variable name must be set using the "
           << "\"-name\" command line argument.\n\n";
      exit(1);
   }

   // Define Variables
   mask_var = f_out->add_var(mask_name, ncInt, lat_dim, lon_dim);
   sprintf(var_str, "%s masking region", mask_name.text());
   mask_var->add_att("long_name", var_str);

   // Allocate memory to store the mask values for each grid point
   mask_data = new int [grid.nx()*grid.ny()];

   // Loop through and compute the Lat/Lon and mask value for each grid point
   in_count = out_count = 0;
   for(x=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         // Lat/Lon values for each grid point
         grid.xy_to_latlon(x, y, lat, lon);
         lon -= 360.0*floor((lon + 180.0)/360.0);
         n = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);

         // Grid masking
         if(grid_mask.nx() > 0 && grid_mask.ny() > 0) {

            // Convert the lat/lon to x/y
            grid_mask.latlon_to_xy(lat, lon, x_dbl, y_dbl);
            xx = nint(x_dbl);
            yy = nint(y_dbl);

            // Check if the lat/lon is off the grid
            if(xx < 0 || xx >= grid_mask.nx() ||
               yy < 0 || yy >= grid_mask.ny()) {
               mask_data[n] = 0;
               out_count++;
            }
            else {
               mask_data[n] = 1;
               in_count++;
            }
         }
         // Polyline masking
         else {

            //Convert the longitude from degrees_east to degrees_west.
            if(poly_mask.latlon_is_inside(lat, (-1.0*lon))) {
               mask_data[n] = 1;
               in_count++;
            }
            else {
               mask_data[n] = 0;
               out_count++;
            }
         }

      } // end for y
   } // end for x

   if(!mask_var->put(&mask_data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_netcdf() -> "
           << "error with mask_var->put\n\n";
      exit(1);
   }

   // Delete allocated memory
   if(mask_data) { delete mask_data; mask_data = (int *) 0; }

   f_out->close();
   delete f_out;
   f_out = (NcFile *) 0;

   mlog << Debug(2)
        << "Points Inside Mask:\t" << in_count << " of "
        << grid.nx()*grid.ny() << "\n";

   // List the output file
   mlog << Debug(1)
        << "Output NetCDF File:\t" << out_filename << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void usage()

{

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tdata_file\n"
        << "\tmask_file\n"
        << "\tnetcdf_file\n"
        << "\t[-name str]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"data_file\" is a gridded file in either GRIB "
        << "or NetCDF format from which the grid information is to be "
        << "retrieved (required).\n"

        << "\t\t\"mask_file\" is an ASCII Lat/Lon polyline file "
        << "or gridded data file defining the masking region "
        << "(required).\n"

        << "\t\t\"netcdf_file\" indicates the name of the output NetCDF "
        << "mask file to be written (required).\n"

        << "\t\t\"-name str\" specifies the name to be used for the mask"
        << " (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n"

        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_mask_name(const StringArray & a)
{
   mask_name = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a)
{
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a)
{
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////

