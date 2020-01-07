// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   tc_dland.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    03/19/12  Halley Gotway   New
//   001    07/25/14  Halley Gotway   Add -land option and update how
//                                    distances are computed.
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

#include "grib_classes.h"

#include "vx_log.h"
#include "data2d_factory.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "nc_utils.h"
#include "write_netcdf.h"
#include "tc_poly.h"

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

// North-West Hemisphere up to 60N
static const LatLonData NWHemTenthData =
   { "NWHemTenthDegree", 0.0, -180.0, 0.1, 0.1, 601, 1801 };

// Default location of data file
static const char *default_land_data_files [] = {
   "MET_BASE/tc_data/aland.dat",
   "MET_BASE/tc_data/wland.dat",
   "MET_BASE/tc_data/shland.dat"
};
static const int n_default_land_data_files =
   sizeof(default_land_data_files)/sizeof(*default_land_data_files);

////////////////////////////////////////////////////////////////////////
//    
// Variables for command line arguments
//
////////////////////////////////////////////////////////////////////////

static ConcatString out_filename;
static LatLonData GridData = NWHemTenthData;
static bool latlon_flag = true;
static StringArray land_data_files;
static TCPolyArray land_array;
static int compress_level = -1;

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_land_data();
static void process_distances();
static void usage();
static void set_grid(const StringArray &);
static void set_noll(const StringArray &);
static void set_land(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);
static void set_compress(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   program_name = get_short_name(argv[0]);

   // Set handler to be called for memory allocation error
   set_new_handler(oom);
   
   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the land data files
   process_land_data();
   
   // Process the MODE file
   process_distances();
   
   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;

   //
   // Initialize land data files
   //
   for(int i=0; i<n_default_land_data_files; i++) {
      land_data_files.add(default_land_data_files[i]);
   }
   
   //
   // Check for too few arguments
   //
   if(argc < 1) usage();

   //
   // Parse the command line into tokens
   //
   cline.set(argc, argv);

   //
   // Set the usage function
   //
   cline.set_usage(usage);
   
   //
   // Check if the -land option was used.
   //
   if(cline.has_option("-land")) land_data_files.clear();

   //
   // Add the options function calls
   //
   cline.add(set_grid,      "-grid", 6);
   cline.add(set_noll,      "-noll", 0);
   cline.add(set_land,      "-land", 1);
   cline.add(set_logfile,   "-log",  1);
   cline.add(set_verbosity, "-v",    1);
   cline.add(set_compress,  "-compress",  1);
   
   cline.allow_numbers();

   //
   // Parse the command line
   //
   cline.parse();

   //
   // Check for error. There should be two arguments left.
   //
   if(cline.n() != 1) usage();

   // Store the required arguments
   out_filename  = cline[0];

   return;
}

////////////////////////////////////////////////////////////////////////

void process_land_data() {
   
   for(int i=0; i<land_data_files.n_elements(); i++) {
      mlog << Debug(2)
           << "Reading TC land data file: "
           << replace_path(land_data_files[i]) << "\n";
      land_array.add_file(replace_path(land_data_files[i]).c_str());
   }

   land_array.set_check_dist();
   
   return;
}

////////////////////////////////////////////////////////////////////////

void process_distances() {
   int n, x, y, c, npts, nlog, imin;
   double lat, lon;
   float *dland = (float *) 0;

   // Instantiate the grid
   Grid grid(GridData);
   
   // NetCDF variables
   NcFile *f_out     = (NcFile *) 0;
   NcDim  lat_dim    ;
   NcDim  lon_dim    ;
   NcVar  dland_var ;

   // Create a new NetCDF file and open it
   f_out = open_ncfile(out_filename.c_str(), true);

   if(IS_INVALID_NC_P(f_out)) {
      mlog << Error << "\nprocess_distances() -> "
           << "trouble opening output file " << out_filename
           << "\n\n";
      delete f_out;
      f_out = (NcFile *) 0;
      exit(1);
   }

   // Add global attributes
   mlog << Debug(3) << "Writing NetCDF global attributes.\n";
   write_netcdf_global(f_out, out_filename.c_str(), program_name.c_str());

   // Add the projection information
   mlog << Debug(3) << "Writing NetCDF map projection.\n";
   write_netcdf_proj(f_out, grid);

   // Define Dimensions
   lat_dim = add_dim(f_out, "lat", (long) grid.ny());
   lon_dim = add_dim(f_out, "lon", (long) grid.nx());
   
   // Add the lat/lon variables
   if(latlon_flag) {
      mlog << Debug(3) << "Writing NetCDF lat/lon variables.\n";
      write_netcdf_latlon(f_out, &lat_dim, &lon_dim, grid);
   }

   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = 0;
   
   // Define Variables
   dland_var = add_var(f_out, (string)"dland", ncFloat, lat_dim, lon_dim, deflate_level);
   add_att(&dland_var, "long_name", "distance to land");
   add_att(&dland_var, "units", "nm");
   add_att(&dland_var, "_FillValue", bad_data_float);

   // Allocate memory to store the data values for each grid point
   dland = new float [grid.nx()*grid.ny()];

   // Dump out grid info
   mlog << Debug(2)
        << "Computing distances for " << grid.nx() * grid.ny()
        << " points in grid (" << grid.serialize() << ")...\n";

   // Determine how often to update the status
   npts = grid.nx()*grid.ny();
   nlog = npts/20;

   // Loop over the grid and compute the distance to land for each point   
   for(x=0,c=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         if(++c % nlog == 0 && mlog.verbosity_level() == 3) {
            cout << nint((double) c/npts*100.0) << "% " << flush;
         }

         // Call two_to_one
         n = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);

         // Convert x,y to lat,lon
         grid.xy_to_latlon(x, y, lat, lon);
         lon = rescale_deg(lon, -180.0, 180.0);

         // Compute distance to land
         dland[n] = land_array.min_dist(lat, lon, imin);

         // Convert to nuatical miles
         if(!is_bad_data(dland[n])) dland[n] *= nautical_miles_per_km;

         mlog << Debug(4)
              << "Lat = " << lat << ", Lon = " << lon
              << ", Dist from \"" << land_array[imin].name()
              << "\" = " << dland[n] << " nm\n";

      } // end for y
   } // end for x

   if(mlog.verbosity_level() == 3) cout << "\n" << flush;

   // Write the computed distances to the output file
   mlog << Debug(3) << "Writing distance to land variable.\n";
   if(!put_nc_data_with_dims(&dland_var, &dland[0], grid.ny(), grid.nx())) {
      if(dland) { delete dland; dland = (float *) 0; }
      mlog << Error << "\nprocess_distances() -> "
           << "error with dland_var->put\n\n";
      exit(1);
   }

   // Delete allocated memory
   if(dland) { delete [] dland;  dland = (float *) 0; }

   // Close the output NetCDF file
   delete f_out;
   f_out = (NcFile *) 0;

   // List the output file
   mlog << Debug(1)
        << "Output NetCDF File:\t" << out_filename << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tout_file\n"
        << "\t[-grid spec]\n"
        << "\t[-noll]\n"
        << "\t[-land file]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"out_file\" is the NetCDF output file containing "
        << "the computed distances to land (required).\n"

        << "\t\t\"-grid spec\" overrides the default 1/10th degree "
        << "NW Hemisphere grid (optional).\n"
        << "\t\t   spec = lat_ll lon_ll delta_lat delta_lon n_lat n_lon\n"

        << "\t\t\"-land file\" overwrites the default land data files "
        << "(optional).\n"

        << "\t\t\"-noll\" skips writing the lat/lon variables in the "
        << "output NetCDF file to reduce the file size (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"
        
        << "\t\t\"-compress level\" specifies the compression level of "
        << "output NetCDF variable (optional).\n\n"

        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_grid(const StringArray & a) {
   ConcatString filename;

   GridData.name      = "user_defined";
   GridData.lat_ll    = atof(a[0].c_str());
   GridData.lon_ll    = -1.0*atof(a[1].c_str()); // switch from deg east to west
   GridData.delta_lat = atof(a[2].c_str());
   GridData.delta_lon = atof(a[3].c_str());
   GridData.Nlat      = atoi(a[4].c_str());
   GridData.Nlon      = atoi(a[5].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_noll(const StringArray & a) {
   latlon_flag = false;
}

////////////////////////////////////////////////////////////////////////

void set_land(const StringArray & a) {
   land_data_files.add(a[0]);
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
