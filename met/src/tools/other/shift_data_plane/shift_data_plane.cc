// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////
//
//   Filename:   shift_data_plane.cc
//
//   Description:
//      This tool reads a single 2-dimensional field from a gridded
//      input file, translates all data values by a user-specified
//      amount, and writes the shifted output data in NetCDF format.
//      This shift is defined by specifying an old lat/lon location
//      followed by its new location.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    11-12-14  Halley Gotway  New
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

#include "GridTemplate.h"

#include "vx_log.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_nc_util.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

// Variables for command line arguments
static double FrLat = bad_data_double;
static double FrLon = bad_data_double;
static double ToLat = bad_data_double;
static double ToLon = bad_data_double;
static ConcatString InputFilename;
static ConcatString OutputFilename;
static ConcatString FieldString;
static InterpMthd Method = InterpMthd_DW_Mean;
static int Width = 2;
static GridTemplateFactory::GridTemplates Shape = GridTemplateFactory::GridTemplate_Square;
static int compress_level = -1;

// Static global variables
static ConcatString shift_cs;

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_data_file();
static void write_netcdf(const DataPlane &dp, const Grid &grid,
                         const VarInfo *vinfo, const GrdFileType& ftype);
static void usage();
static void set_from(const StringArray &);
static void set_to(const StringArray &);
static void set_method(const StringArray &);
static void set_shape(const StringArray &);
static void set_width(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);
static void set_compress(const StringArray &);

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

   // Check for zero arguments
   if(argc == 1) usage();

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Allow for negative numbers on the command line
   // which may be used for the -from and -to options.
   cline.allow_numbers();

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_from,      "-from",   2);
   cline.add(set_to,        "-to",     2);
   cline.add(set_method,    "-method", 1);
   cline.add(set_width,     "-width",  1);
   cline.add(set_shape, 	  "-shape",  1);
   cline.add(set_logfile,   "-log",    1);
   cline.add(set_verbosity, "-v",      1);
   cline.add(set_compress,  "-compress",  1);

   // Parse the command line
   cline.parse();

   // Check for error. There should be three arguments left:
   // the input filename, the output filename, and config string.
   if(cline.n() != 3) usage();

   // Store the filenames and config string.
   InputFilename  = cline[0];
   OutputFilename = cline[1];
   FieldString    = cline[2];

   // Check definition of the shift
   if(is_bad_data(FrLat) || is_bad_data(FrLon) ||
      is_bad_data(ToLat) || is_bad_data(ToLon)) {
      mlog << Error << "\nprocess_command_line() -> "
           << "Missing the -from and/or -to options!\n\n";
      usage();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_data_file() {
   DataPlane dp_in, dp_shift;
   Grid grid;
   GrdFileType ftype;
   double fr_x, fr_y, to_x, to_y, dx, dy, v;
   int x, y;

   // Parse the config string
   MetConfig config;
   config.read(replace_path(config_const_filename).c_str());
   config.read_string(FieldString.c_str());

   // Note: The command line argument MUST processed before this
   if (compress_level < 0) compress_level = config.nc_compression();

   // Get the gridded file type from config string, if present
   ftype = parse_conf_file_type(&config);

   // Read the input data file
   Met2dDataFileFactory m_factory;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;

   mlog << Debug(1)  << "Reading input file: " << InputFilename << "\n";
   mtddf = m_factory.new_met_2d_data_file(InputFilename.c_str(), ftype);

   if(!mtddf) {
      mlog << Error << "\nprocess_data_file() -> "
           << "\"" << InputFilename << "\" not a valid data file\n\n";
      exit(1);
   }

   // Read data from the input file
   VarInfoFactory v_factory;
   VarInfo *vinfo;
   vinfo = v_factory.new_var_info(mtddf->file_type());

   if(!vinfo) {
      mlog << Error << "\nprocess_data_file() -> "
           << "unable to determine file type of \"" << InputFilename
           << "\"\n\n";
      exit (1);
   }

   // Populate the VarInfo object using config
   vinfo->set_dict(config);

   // Open the input file
   if(!mtddf->open(InputFilename.c_str())) {
      mlog << Error << "\nprocess_data_file() -> can't open file \""
           << InputFilename << "\"\n\n";
      exit(1);
   }

   // Get the data plane from the file for this VarInfo object
   if(!mtddf->data_plane(*vinfo, dp_in)) {
      mlog << Error << "\nprocess_data_file() -> trouble getting field \""
           << FieldString << "\" from file \"" << InputFilename << "\"\n\n";
      exit(1);
   }

   // List the range of data values
   if(mlog.verbosity_level() >= 2) {
      double dmin, dmax;

      // Get the range of data values
      dp_in.data_range(dmin, dmax);

      mlog << Debug(2)
           << "Range of data for \"" << FieldString << "\" is "
           << dmin << " to " << dmax << ".\n";
   }

   // Compute the shift, converting from degrees east to west
   grid = mtddf->grid();
   grid.latlon_to_xy(FrLat, -1.0*FrLon, fr_x, fr_y);
   grid.latlon_to_xy(ToLat, -1.0*ToLon, to_x, to_y);

   // Check for bad data
   if(is_bad_data(fr_x) || is_bad_data(fr_y) ||
      is_bad_data(to_x) || is_bad_data(to_y)) {
      mlog << Error << "\nprocess_data_file() -> "
           << "problem defining the grid x/y shift from ("
           << fr_x << ", " << fr_y << ") to ("
           << to_x << ", " << to_y << ").\n\n";
      exit(1);
   }

   // Compute the shift
   dx = to_x - fr_x;
   dy = to_y - fr_y;

   shift_cs << cs_erase << "Shifting from lat/lon ("
            << FrLat << ", " << FrLon << ") to lat/lon ("
            << ToLat << ", " << ToLon << ") is grid x/y shift ("
            << dx << ", " << dy << ")";

   mlog << Debug(2) << shift_cs << "\n";

   // Shift the data

   dp_shift = dp_in;
   for(x=0; x<dp_shift.nx(); x++) {
      for(y=0; y<dp_shift.ny(); y++) {
         v = compute_horz_interp(dp_in, x - dx, y - dy, bad_data_double,
                                 Method, Width, Shape, 1.0);
         dp_shift.set(v, x, y);
      } // end for y
   } // end for x

   // Write the shifted data
   write_netcdf(dp_shift, grid, vinfo, mtddf->file_type());

   // Clean up
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }
   if(vinfo) { delete vinfo; vinfo = (VarInfo *)       0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_netcdf(const DataPlane &dp, const Grid &grid,
                  const VarInfo *vinfo, const GrdFileType &ftype) {
   ConcatString cs;

   // Create a new NetCDF file and open it
   NcFile *f_out = open_ncfile(OutputFilename.c_str(), true);

   if(IS_INVALID_NC_P(f_out)) {
      mlog << Error << "\nwrite_netcdf() -> "
           << "trouble opening output NetCDF file \""
           << OutputFilename << "\"\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(f_out, OutputFilename.c_str(), program_name.c_str());

   // Add the run command
   add_att(f_out, "RunCommand", shift_cs);

   // Add the projection information
   write_netcdf_proj(f_out, grid);

   // Define Dimensions
   NcDim lat_dim = add_dim(f_out, "lat", (long) grid.ny());
   NcDim lon_dim = add_dim(f_out, "lon", (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(f_out, &lat_dim, &lon_dim, grid);

   // Define output variable and attributes
   cs << cs_erase << vinfo->name_attr();
   if(vinfo->level().type() != LevelType_Accum &&
      ftype != FileType_NcMet &&
      ftype != FileType_General_Netcdf &&
      ftype != FileType_NcPinterp &&
      ftype != FileType_NcCF) {
      cs << "_" << vinfo->level_attr();
   }

   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = 0;

   NcVar data_var = add_var(f_out, (string)cs, ncFloat, lat_dim, lon_dim, deflate_level);
   add_att(&data_var, "name", (string)cs);
   add_att(&data_var, "long_name", (string)vinfo->long_name_attr());
   add_att(&data_var, "level", (string)vinfo->level_attr());
   add_att(&data_var, "units", (string)vinfo->units_attr());
   add_att(&data_var, "_FillValue", bad_data_float);
   write_netcdf_var_times(&data_var, dp);
   add_att(&data_var, "smoothing_method", (string)interpmthd_to_string(Method));
   add_att(&data_var, "smoothing_neighborhood", Width*Width);

   GridTemplateFactory gtf;
   add_att(&data_var, "smoothing_shape", gtf.enum2String(Shape));

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
   if(data)  {                 delete [] data; data  = (float *)  0; }
   if(f_out) {
      delete f_out;   f_out = (NcFile *) 0;
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
        << "\toutput_filename\n"
        << "\tfield_string\n"
        << "\t-from lat lon\n"
        << "\t-to   lat lon\n"
        << "\t[-method type]\n"
        << "\t[-width n]\n"
	      << "\t[-shape SHAPE]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"input_filename\" is the name of a "
        << "gridded data file to be plotted (required).\n"

        << "\t\t\"output_filename\" is the name of the output "
        << "NetCDF file to be written (required).\n"

        << "\t\t\"field_string\" defines the data to be shifted "
        << "from the input file (required).\n"

        << "\t\t\"-from lat lon\" specifies the starting latitude "
        << "(degrees north) and longitude (degrees east) to "
        << "define the shift.\n"

        << "\t\t\"-to lat lon\" specifies the ending latitude "
        << "(degrees north) and longitude (degrees east) to "
        << "define the shift.\n"

        << "\t\t\"-method type\" overrides the default interpolation "
        << "method (DW_MEAN) (optional).\n"

        << "\t\t\"-width\" overrides the default interpolation width (2) "
        << "(optional).\n"

        << "\t\t\"-shape\" overrides the default interpolation shape (SQUARE) "
        << "(optional).\n"



        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t\"-compress level\" overrides the compression level of NetCDF variable (optional).\n\n" << flush;

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_from(const StringArray &a) {
   FrLat = atof(a[0].c_str());
   FrLon = atof(a[1].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_to(const StringArray &a) {
   ToLat = atof(a[0].c_str());
   ToLon = atof(a[1].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_method(const StringArray &a) {
   Method = string_to_interpmthd(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_width(const StringArray &a) {
   Width = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_shape(const StringArray &a) {
	GridTemplateFactory gtf;
	Shape = gtf.string2Enum(a[0]);
}


////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray &a) {
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray &a) {
   mlog.set_verbosity_level(atoi(a[0].c_str()));
}

////////////////////////////////////////////////////////////////////////

void set_compress(const StringArray & a) {
   compress_level = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////
