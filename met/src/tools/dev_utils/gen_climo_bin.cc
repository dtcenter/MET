// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:    gen_climo_bin.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
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

#include "vx_util.h"
#include "vx_nc_util.h"
#include "vx_log.h"
#include "data_plane.h"
#include "vx_data2d.h"
#include "vx_data2d_factory.h"

////////////////////////////////////////////////////////////////////////
//
// Variables for command line arguments
//
////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

static Grid grid;
static ConcatString out_file;
static NcFile       *nc_out = (NcFile *) 0;
static NcDim        lat_dim, lon_dim, cdf_dim;
static NcVar        cdf_x_var, cdf_y_var;

static ConcatString bin_file, mean_file, stdev_file;
static ConcatString field_str, var_name;

// n_bin is the number of climatological bins.
// The number of climo values computed is actually n_bin - 1.
// For example, 9 climo values defines 10 bins.
static int n_bin = 10;

static const int deflate_level = -1;

////////////////////////////////////////////////////////////////////////

static void process_binary();
static void process_mean_stdev();
static void get_field(const char *, const char *, DataPlane &);

static void setup_nc_file();
static void write_nc_bin(const DataPlane &, int, double);

static void set_out_file(const StringArray & a);
static void set_n_bin(const StringArray & a);
static void set_bin_file(const StringArray & a);
static void set_mean_file(const StringArray & a);
static void set_stdev_file(const StringArray & a);
static void set_field_str(const StringArray & a);
static void set_var_name(const StringArray & a);
static void set_verbosity(const StringArray & a);
static void my_memcpy(void * to, unsigned char * & from, int n_bytes);

static void usage();

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   program_name = get_short_name(argv[0]);

   // Parse the command line
   CommandLine cline;
   cline.set(argc, argv);
   cline.set_usage(usage);
   cline.add(set_out_file,   "-out",   1);
   cline.add(set_n_bin,      "-n_bin", 1);
   cline.add(set_bin_file,   "-bin",   1);
   cline.add(set_mean_file,  "-mean",  1);
   cline.add(set_stdev_file, "-stdev", 1);
   cline.add(set_field_str,  "-field", 1);
   cline.add(set_var_name,   "-name",  1);
   cline.add(set_verbosity,  "-v",     1);

   cline.parse();

   // Check required arguments
   if(out_file.empty()) {
      mlog << Error << "\nThe \"-out\" command line option is required.\n\n";
      usage();
   }
   if(bin_file.empty() &&
      (mean_file.empty() || stdev_file.empty() || field_str.empty())) {
      mlog << Error << "\nEither \"-bin\" or \"-mean\", \"-stdev\", "
           << "and \"-field\" command line options are required.\n\n";
      usage();
   }

   // Process the binary or mean/stdev input files
   if(!bin_file.empty()) process_binary();
   else                  process_mean_stdev();

   // List the NetCDF file after it is finished
   mlog << Debug(1)
        << "Finished writing output file: " << out_file << "\n";
   delete nc_out;
   nc_out = (NcFile *) 0;

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_binary() {
   DataPlane dp;
   DataPlaneArray dpa;
   StringArray grid_spec;
   int i, j, x, y, nxy, fd;
   double cdf_y;
   const int buf_size = 512;
   const int n_read_bin = 101;
   unsigned char buf[buf_size];
   float v;

   // Each grid point contains 101 values but skip the first and last.
   // 99 points defines 100 bins.

   // Read 2.5 degree global climatology data in binary format
   // latlon: lat  90.000000 to -90.000000 by 2.500000  nxny 10512
   //         long 0.000000 to -2.500000 by 2.500000, (144 x 73)
   grid_spec.parse_wsss("latlon 144 73 -90 0 2.5 2.5");
   parse_grid_def(grid_spec, grid);

   // Initialize to bad data
   dp.set_size(grid.nx(), grid.ny());
   dp.set_constant(bad_data_double);
   for(i=0; i<(n_bin-1); i++) dpa.add(dp, i+1, i+1);

   // Open the input file
   if((fd = ::open(bin_file.c_str(), O_RDONLY)) < 0) {
      mlog << Error << "\nprocess_binary() -> "
           << "trouble opening input binary file "
           << bin_file << "\n\n";
      exit(1);
   }

   // Set the output variable name, if necessary
   if(var_name.empty()) {
      StringArray sa = bin_file.split("/.");
      var_name << sa[(sa.n_elements() - 2)]
               << "_CLIMO_BINS";
   }

   // Setup the output file
   setup_nc_file();

   // Read Nx*Ny records
   nxy = grid.nx()*grid.ny();
   mlog << "Processing " << nxy << " grid points.\n";

   // Loop over grid points
   for(i=0; i<nxy; i++) {
      one_to_two_0_0_0(grid.nx(), grid.ny(), i, x, y);
      read_fortran_binary(fd, buf, buf_size, 4, true);

      // Loop over climo bins, skipping the first and last points
      for(j=0; j<(n_bin-1); j++) {
         int byte_offset = (j+1)*((n_read_bin - 1)/(n_bin))*4;
         unsigned char * b = buf + byte_offset;
         my_memcpy(&v, b, 4);
         shuffle_4(&v);
         dpa.set(v, j, x, y);
      }
   }

   // Close the input file
   if(fd >= 0) {
      ::close(fd);
      fd = -1;
   }

   // Write data for each climo bin
   for(i=0; i<(n_bin-1); i++) {
      cdf_y  = (double) (i+1)/n_bin;
      write_nc_bin(dpa[i], i, cdf_y);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_mean_stdev() {
   double cdf_y;
   DataPlane mn_dp, sd_dp, cur_dp;

   // Read the climo mean and standard deviation fields
   get_field(mean_file.c_str(), field_str.c_str(), mn_dp);
   get_field(stdev_file.c_str(), field_str.c_str(), sd_dp);

   // Setup the output file
   setup_nc_file();

   // Loop over each of the climo bins
   for(int i=0; i<n_bin-1; i++) {
      cdf_y  = (double) (i+1)/n_bin;
      cur_dp = normal_cdf_inv(cdf_y, mn_dp, sd_dp);
      write_nc_bin(cur_dp, i, cdf_y);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_nc_file() {

   // Create a new NetCDF file and open it
   nc_out = open_ncfile(out_file.c_str(), true);

   if(!nc_out || IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_file << "\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, out_file.text(), program_name.c_str(), "Climatology", "NA");

   // Add the projection information
   write_netcdf_proj(nc_out, grid);

   // Add dimensions
   lat_dim = add_dim(nc_out, "lat", (long) grid.ny());
   lon_dim = add_dim(nc_out, "lon", (long) grid.nx());
   write_netcdf_latlon(nc_out, &lat_dim, &lon_dim, grid);

   // The number of CDF values is one less than the number of bins
   cdf_dim = add_dim(nc_out, "cdf", (long) n_bin-1);

   // Add CDF variables
   cdf_y_var = add_var(nc_out, "CDF_Value", ncFloat,
                       cdf_dim, deflate_level);
   cdf_x_var = add_var(nc_out, (string) var_name, ncFloat,
                       cdf_dim, lat_dim, lon_dim, deflate_level);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nc_bin(const DataPlane &dp, int i_cdf, double cdf_y) {
   int x, y, n;
   double dmin, dmax;

   // Allocate memory
   n = grid.nx() * grid.ny();
   float * data = new float [n];

   dp.data_range(dmin, dmax);
   mlog << Debug(2)
        << "[" << i_cdf+1 << " of " << n_bin-1
        << "] Range of CDF X-values for CDF Y-value = "
        << cdf_y << ": " << dmin << " to " << dmax << "\n";

   // Store the data
   for(x=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {
         n = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);
         data[n] = dp(x, y);
      } // end for y
   } // end for x

   // Set up the lenghts and offsets
   long lengths[3];
   long offsets[3];

   offsets[0] = i_cdf;
   offsets[1] = 0;
   offsets[2] = 0;

   lengths[0] = 1;
   lengths[1] = grid.ny();
   lengths[2] = grid.nx();

   // Write out the current CDF Y-values
   if(!put_nc_data(&cdf_y_var, cdf_y, i_cdf)) {
      mlog << Error << "\nwrite_nc_bin() -> "
           << "error writing NetCDF variable name \"CDF_Value\" for the "
           << i_cdf << "-th CDF Y-value ( " << cdf_y << ").\n\n";
      exit(1);
   }

   // Write out the gridded field of CDF X-values
   if(!put_nc_data(&cdf_x_var, &data[0], lengths, offsets)) {
      mlog << Error << "\nwrite_nc_bin() -> "
           << "error writing NetCDF variable name \""
           << var_name << "\" for the " << i_cdf
           << "-th CDF Y-value ( " << cdf_y << ").\n\n";
      exit(1);
   }

   // Deallocate and clean up
   if(data) { delete [] data; data = (float *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void get_field(const char *file, const char *config_str, DataPlane &dp) {
   GrdFileType ftype;
   Met2dDataFile * mtddf_ptr = (Met2dDataFile * ) 0;
   Met2dDataFileFactory m_factory;
   VarInfo * vi_ptr = (VarInfo * ) 0;
   VarInfoFactory v_factory;
   double dmin, dmax;

   // Parse the config string
   MetConfig config;
   config.read(replace_path(config_const_filename).c_str());
   config.read_string(config_str);

   // Get the gridded file type from config string, if present
   ftype = parse_conf_file_type(&config);

   // Instantiate Met2dDataFile object from the factory
   mlog << Debug(1)  << "Opening data file: " << file << "\n";
   mtddf_ptr = m_factory.new_met_2d_data_file(file, ftype);
   if(!mtddf_ptr) {
      mlog << Error << "\n" << program_name
           << " -> file \"" << file << "\" not a valid data file\n\n";
      exit(1);
   }

   // Make sure the grid definition has not changed
   if(grid.nx() > 0 && grid.ny() > 0) {

      // Check for a grid mismatch
      if(!(grid == mtddf_ptr->grid())) {
         mlog << Error << "\nget_field() -> "
              << "The grids do not match:\n" << grid.serialize()
              << " !=\n" << mtddf_ptr->grid().serialize() << "\n\n";
         exit(1);
      }
   }
   // Otherwise, store the grid
   else {
      grid = mtddf_ptr->grid();
   }

   // Instantiate VarInfo object from the factory
   vi_ptr = v_factory.new_var_info(mtddf_ptr->file_type());
   if(!vi_ptr) {
      mlog << Error << "\n" << program_name
           << " -> unable to determine filetype of \"" << file
           << "\"\n\n";
      exit(1);
   }

   // Populate the VarInfo object
   vi_ptr->set_dict(config);

   // Open the data file
   if(!mtddf_ptr->open(file)) {
      mlog << Error << "\n" << program_name
           << " -> can't open file \"" << file << "\"\n\n";
      exit(1);
   }

   // Read the data
   if(!mtddf_ptr->data_plane(*vi_ptr, dp)) {
      mlog << Error << "\n" << program_name
           << " -> trouble getting field \"" << config_str
           << "\" from file \"" << file << "\"\n\n";
      exit (1);
   }

   // Dump the range of data values read
   dp.data_range(dmin, dmax);
   mlog << Debug(2)
        << "Read field \"" << vi_ptr->magic_str() << "\" from \""
        << mtddf_ptr->filename() << "\" with data ranging from "
        << dmin << " to " << dmax << ".\n";

   // Set the output variable name, if necessary
   if(var_name.empty()) {
      var_name << vi_ptr->name() << "_"
               << vi_ptr->level_name()
               << "_CLIMO_BINS";
   }

   // Clean up
   if(mtddf_ptr) { delete mtddf_ptr; mtddf_ptr = (Met2dDataFile * ) 0; }
   if(vi_ptr)    { delete vi_ptr;    vi_ptr    =        (VarInfo *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void my_memcpy(void * to, unsigned char * & from, int n_bytes) {
   memcpy(to, from, n_bytes);
   from += n_bytes;
   return;
}

////////////////////////////////////////////////////////////////////////

void set_out_file  (const StringArray & a) { out_file   = a[0];       return; }
void set_n_bin     (const StringArray & a) { n_bin      = atoi(a[0].c_str()); return; }
void set_bin_file  (const StringArray & a) { bin_file   = a[0];       return; }
void set_mean_file (const StringArray & a) { mean_file  = a[0];       return; }
void set_stdev_file(const StringArray & a) { stdev_file = a[0];       return; }
void set_field_str (const StringArray & a) { field_str  = a[0];       return; }
void set_var_name  (const StringArray & a) { var_name   = a[0];       return; }
void set_verbosity (const StringArray & a) {
                          mlog.set_verbosity_level(atoi(a[0].c_str()));       return; }

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"

        << "\t-out out_file\n"
        << "\t[-n_bin n]\n"
        << "\t[-bin   path]\n"
        << "\t[-mean  path]\n"
        << "\t[-stdev path]\n"
        << "\t[-field string]\n"
        << "\t[-name  string]\n"
        << "\t[-v     level]\n\n"

        << "\twhere\t\"-out path\" is the output NetCDF file (required).\n"
        << "\t\t\"-n_bin n\" overrides the default number of equal-area climatology bins ("
        << n_bin << ") (optional).\n"
        << "\t\t\"-bin path\" is the input binary binned climatology file (optional).\n"
        << "\t\t\"-mean path\" is the input climatology mean file (optional).\n"
        << "\t\t\"-stdev path\" is the input climatology standard deviation file (optional).\n"
        << "\t\t\"-field string\" defines the mean and standard deviation field to be processed (optional).\n"
        << "\t\t\"-name string\" is the output NetCDF variable name (optional).\n"
        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n"

        << "\tNOTE: Either \"-bin\" or \"-mean\", \"-stdev\", and \"-field\" must be specified.\n\n"
        << flush;

   exit(1);
}

////////////////////////////////////////////////////////////////////////
