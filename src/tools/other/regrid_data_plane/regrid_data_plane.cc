// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////
//
//   Filename:   regrid_data_plane.cc
//
//   Description:
//      This tool reads 2-dimensional fields from a gridded input file,
//      regrids them to the user-specified output grid, and writes the
//      regridded output data in NetCDF format.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    01-29-15  Halley Gotway  New
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
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

//#include "netcdf.hh"

#include "vx_log.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_nc_util.h"
#include "vx_grid.h"
#include "vx_regrid.h"
#include "vx_util.h"
#include "vx_statistics.h"

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

// Constants
static const InterpMthd DefaultInterpMthd = InterpMthd_Nearest;
static const int        DefaultInterpWdth = 1;
static const double     DefaultVldThresh  = 0.5;

// Variables for command line arguments
static ConcatString InputFilename;
static ConcatString OutputFilename;
static StringArray FieldSA;
static RegridInfo RGInfo;
static ConcatString VarName;

// Output NetCDF file
static NcFile *nc_out  = (NcFile *) 0;
static NcDim  lat_dim ;
static NcDim  lon_dim ;

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_data_file();
static void open_nc(const Grid &grid, const ConcatString run_cs);
static void write_nc(const DataPlane &dp, const Grid &grid,
                     const VarInfo *vinfo, const GrdFileType& ftype);
static void close_nc();
static void usage();
static void set_field(const StringArray &);
static void set_method(const StringArray &);
static void set_width(const StringArray &);
static void set_vld_thresh(const StringArray &);
static void set_name(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Store the program name
   program_name = get_short_name(argv[0]);

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the input data file
   process_data_file();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;

   // Set default regridding options
   RGInfo.enable     = true;
   RGInfo.field      = FieldType_None;
   RGInfo.method     = DefaultInterpMthd;
   RGInfo.width      = DefaultInterpWdth;
   RGInfo.vld_thresh = DefaultVldThresh;

   // Check for zero arguments
   if(argc == 1) usage();

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_field,      "-field",      1);
   cline.add(set_method,     "-method",     1);
   cline.add(set_width,      "-width",      1);
   cline.add(set_vld_thresh, "-vld_thresh", 1);
   cline.add(set_name,       "-name",       1);
   cline.add(set_logfile,    "-log",        1);
   cline.add(set_verbosity,  "-v",          1);

   // Parse the command line
   cline.parse();

   // Check for error. There should be three arguments left:
   // - input filename
   // - destination grid,
   // - output filename
   if(cline.n() != 3) usage();

   // Store the filenames and config string.
   InputFilename  = cline[0];
   RGInfo.name    = cline[1];
   OutputFilename = cline[2];

   // Check for at least one configuration string
   if(FieldSA.n_elements() < 1) {
      mlog << Error << "\nprocess_command_line() -> "
           << "The -field option must be used at least once!\n\n";
      usage();
   }

   // Check the nearest neighbor special case
   if(RGInfo.method != InterpMthd_Nearest && RGInfo.width == 1) {
      mlog << Warning << "\nprocess_command_line() -> "
           << "Resetting the regridding method from \""
           << interpmthd_to_string(RGInfo.method) << "\" to \""
           << interpmthd_nearest_str
           << "\" since the regridding width is 1.\n\n";
      RGInfo.method = InterpMthd_Nearest;
   }

   // Check the bilinear and budget special cases
   if((RGInfo.method == InterpMthd_Bilin ||
       RGInfo.method == InterpMthd_Budget) &&
      RGInfo.width != 2) {
      mlog << Warning << "\nprocess_command_line() -> "
           << "Resetting the regridding width from "
           << RGInfo.width << " to 2 for regridding method \""
           << interpmthd_to_string(RGInfo.method) << "\".\n\n";
      RGInfo.width = 2;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_data_file() {
   DataPlane fr_dp, to_dp;
   Grid fr_grid, to_grid;
   GrdFileType ftype;
   double dmin, dmax;
   ConcatString run_cs;

   // Initialize configuration object
   MetConfig config;
   config.read(replace_path(config_const_filename));
   config.read_string(FieldSA[0]);

   // Get the gridded file type from config string, if present
   ftype = parse_conf_file_type(&config);

   // Read the input data file
   Met2dDataFileFactory m_factory;
   Met2dDataFile *fr_mtddf = (Met2dDataFile *) 0;

   // Determine the "from" grid
   mlog << Debug(1)  << "Reading data file: " << InputFilename << "\n";
   fr_mtddf = m_factory.new_met_2d_data_file(InputFilename, ftype);

   if(!fr_mtddf) {
      mlog << Error << "\nprocess_data_file() -> "
           << "\"" << InputFilename << "\" not a valid data file\n\n";
      exit(1);
   }
   fr_grid = fr_mtddf->grid();
   mlog << Debug(2) << "Input grid: " << fr_grid.serialize() << "\n";

   // Determine the "to" grid
   to_grid = parse_vx_grid(RGInfo, &fr_grid, &fr_grid);
   mlog << Debug(2) << "Output grid: " << to_grid.serialize() << "\n";

   mlog << Debug(2) << "Interpolation options: "
        << "method = " << interpmthd_to_string(RGInfo.method)
        << ", width = " << RGInfo.width
        << ", vld_thresh = " << RGInfo.vld_thresh << "\n";

   // Build the run command string
   run_cs << "Regrid from " << fr_grid.serialize() << " to " << to_grid.serialize();

   // Setup the VarInfo request object
   VarInfoFactory v_factory;
   VarInfo *vinfo;
   vinfo = v_factory.new_var_info(fr_mtddf->file_type());

   if(!vinfo) {
      mlog << Error << "\nprocess_data_file() -> "
           << "unable to determine file type of \"" << InputFilename
           << "\"\n\n";
      exit(1);
   }

   // Open the output file
   open_nc(to_grid, run_cs);

   // Loop through the requested fields
   for(int i=0; i<FieldSA.n_elements(); i++) {

      // Initialize
      vinfo->clear();

      // Populate the VarInfo object using the config string
      config.read_string(FieldSA[i]);
      vinfo->set_dict(config);

      // Get the data plane from the file for this VarInfo object
      if(!fr_mtddf->data_plane(*vinfo, fr_dp)) {
         mlog << Error << "\nprocess_data_file() -> trouble getting field \""
              << FieldSA[i] << "\" from file \"" << InputFilename << "\"\n\n";
         exit(1);
      }

      // Regrid the data plane
      to_dp = met_regrid(fr_dp, fr_grid, to_grid, RGInfo);

      // List range of data values
      if(mlog.verbosity_level() >= 2) {
         fr_dp.data_range(dmin, dmax);
         mlog << Debug(2)
              << "Range of input data (" << FieldSA[i] << ") is "
              << dmin << " to " << dmax << ".\n";
         to_dp.data_range(dmin, dmax);
         mlog << Debug(2)
              << "Range of regridded data (" << FieldSA[i] << ") is "
              << dmin << " to " << dmax << ".\n";
      }

      // Write the regridded data
      write_nc(to_dp, to_grid, vinfo, fr_mtddf->file_type());

   } // end for i

   // Close the output file
   close_nc();

   // Clean up
   if(fr_mtddf) { delete fr_mtddf; fr_mtddf = (Met2dDataFile *) 0; }
   if(vinfo)    { delete vinfo;    vinfo    = (VarInfo *)       0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void open_nc(const Grid &grid, ConcatString run_cs) {

   // Create output file
   nc_out = open_ncfile(OutputFilename, true);

   if(IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nopen_nc() -> "
           << "trouble opening output NetCDF file \""
           << OutputFilename << "\"\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, OutputFilename, program_name);

   // Add the run command
   add_att(nc_out, "RunCommand", run_cs);

   // Add the projection information
   write_netcdf_proj(nc_out, grid);

   // Define Dimensions
   lat_dim = add_dim(nc_out, "lat", (long) grid.ny());
   lon_dim = add_dim(nc_out, "lon", (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(nc_out, &lat_dim, &lon_dim, grid);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nc(const DataPlane &dp, const Grid &grid,
              const VarInfo *vinfo, const GrdFileType &ftype) {
   ConcatString nc_var_name;

   // Define output variable name, if not already set
   if(VarName.length() == 0) {
      nc_var_name << cs_erase << vinfo->name();
      if(vinfo->level().type() != LevelType_Accum &&
            ftype != FileType_NcMet &&
            ftype != FileType_General_Netcdf &&
            ftype != FileType_NcPinterp &&
            ftype != FileType_NcCF) {
         nc_var_name << "_" << vinfo->level_name();
      }
   }
   else {
      nc_var_name = VarName;
   }

   NcVar data_var = add_var(nc_out, (string)nc_var_name, ncFloat, lat_dim, lon_dim);
   add_att(&data_var, "name", (string)nc_var_name);
   add_att(&data_var, "long_name", (string)vinfo->long_name());
   add_att(&data_var, "level", (string)vinfo->level_name());
   add_att(&data_var, "units", (string)vinfo->units());
   add_att(&data_var, "_FillValue", bad_data_float);
   write_netcdf_var_times(&data_var, dp);

   // Allocate memory to store data values for each grid point
   float *data = new float [grid.nx()*grid.ny()];

   // Store the data
   for(int x=0; x<grid.nx(); x++) {
      for(int y=0; y<grid.ny(); y++) {
         int n = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);
         data[n] = (float) dp(x, y);
      } // end for y
   } // end for x

   // Write out the data
   if(!put_nc_data_with_dims(&data_var, &data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_nc() -> "
           << "error writing data to the output file.\n\n";
      exit(1);
   }

   // Clean up
   if(data) { delete [] data;  data = (float *)  0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void close_nc() {

   // Clean up
   if(nc_out) {
      delete nc_out; nc_out = (NcFile *) 0; 
   }

   // List the output file
   mlog << Debug(1)
        << "Writing output file: " << OutputFilename << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tinput_filename\n"
        << "\tto_grid\n"
        << "\toutput_filename\n"
        << "\t-field string\n"
        << "\t[-method type]\n"
        << "\t[-width n]\n"
        << "\t[-vld_thresh n]\n"
        << "\t[-name str]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"input_filename\" is the gridded data file to be "
        << "read (required).\n"

        << "\t\t\"to_grid\" defines the output grid as a named grid, the "
        << "path to a gridded data file, or an explicit grid "
        << "specification string (required).\n"

        << "\t\t\"output_filename\" is the output NetCDF file to be "
        << "written (required).\n"

        << "\t\t\"-field string\" may be used multiple times to define "
        << "the data to be regridded (required).\n"

        << "\t\t\"-method type\" overrides the default regridding "
        << "method (" << interpmthd_to_string(RGInfo.method)
        << ") (optional).\n"

        << "\t\t\"-width n\" overrides the default regridding "
        << "width (" << RGInfo.width << ") (optional).\n"

        << "\t\t\"-vld_thresh n\" overrides the default required "
        << "ratio of valid data for regridding (" << RGInfo.vld_thresh
        << ") (optional).\n"

        << "\t\t\"-name str\" specifies the output variable name "
        << "(optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n" << flush;

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_field(const StringArray &a) {
   FieldSA.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_method(const StringArray &a) {
   RGInfo.method = string_to_interpmthd(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_width(const StringArray &a) {
   RGInfo.width = atoi(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_vld_thresh(const StringArray &a) {
   RGInfo.vld_thresh = atof(a[0]);
   if(RGInfo.vld_thresh > 1 || RGInfo.vld_thresh < 0) {
      mlog << Error << "\nset_vld_thresh() -> "
           << "-vld_thresh may only be set between 0 and 1: "
           << RGInfo.vld_thresh << "\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

void set_name(const StringArray & a) {
   VarName = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray &a) {
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray &a) {
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////
