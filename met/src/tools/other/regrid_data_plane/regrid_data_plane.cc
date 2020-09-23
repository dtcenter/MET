// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
//   001    03-23-17  Halley Gotway  Change -name to an array.
//   002    06-25-17  Howard Soh     Support GOES-16
//   003    09-24-17  Howard Soh     Support Gaussian filtering
//   004    01-28-20  Howard Soh     Moved GOES-16/17 to point2grib
//   005    04-09-20  Halley Gotway  Add convert and censor options.
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

#include "vx_log.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_nc_util.h"
#include "vx_grid.h"
#include "vx_regrid.h"
#include "vx_util.h"
#include "vx_statistics.h"

#include "GridTemplate.h"

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

// Constants
static const InterpMthd DefaultInterpMthd = InterpMthd_Nearest;
static const int        DefaultInterpWdth = 1;
static const double     DefaultVldThresh  = 0.5;

static const float      MISSING_LATLON = -999.0;

// Variables for command line arguments
static ConcatString InputFilename;
static ConcatString OutputFilename;
static StringArray FieldSA;
static RegridInfo RGInfo;
static StringArray VarNameSA;
static int compress_level = -1;

// Output NetCDF file
static NcFile *nc_out  = (NcFile *) 0;
static NcDim  lat_dim ;
static NcDim  lon_dim ;

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_data_file();
static void open_nc(const Grid &grid, const ConcatString run_cs);
static void write_nc(const DataPlane &dp, const Grid &grid,
                     const VarInfo *vinfo, const char *vname);
static void close_nc();
static void usage();
static void set_field(const StringArray &);
static void set_method(const StringArray &);
static void set_shape(const StringArray &);
static void set_gaussian_dx(const StringArray &);
static void set_gaussian_radius(const StringArray &);
static void set_width(const StringArray &);
static void set_vld_thresh(const StringArray &);
static void set_convert(const StringArray &);
static void set_censor(const StringArray &);
static void set_name(const StringArray &);
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

   // Set default regridding options
   RGInfo.enable     = true;
   RGInfo.field      = FieldType_None;
   RGInfo.method     = DefaultInterpMthd;
   RGInfo.width      = DefaultInterpWdth;
   RGInfo.gaussian.dx     = default_gaussian_dx;
   RGInfo.gaussian.radius = default_gaussian_radius;
   RGInfo.gaussian.trunc_factor = default_trunc_factor;
   RGInfo.vld_thresh = DefaultVldThresh;
   RGInfo.shape      = GridTemplateFactory::GridTemplate_Square;

   // Check for zero arguments
   if(argc == 1) usage();

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_field,      "-field",      1);
   cline.add(set_method,     "-method",     1);
   cline.add(set_shape,      "-shape",      1);
   cline.add(set_width,      "-width",      1);
   cline.add(set_gaussian_radius, "-gaussian_radius", 1);
   cline.add(set_gaussian_dx,     "-gaussian_dx",      1);
   cline.add(set_vld_thresh, "-vld_thresh", 1);
   cline.add(set_convert,    "-convert",    1);
   cline.add(set_censor,     "-censor",     2);
   cline.add(set_name,       "-name",       1);
   cline.add(set_logfile,    "-log",        1);
   cline.add(set_verbosity,  "-v",          1);
   cline.add(set_compress,   "-compress",   1);

   cline.allow_numbers();

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

   // Check that the number of output names and fields match
   if(VarNameSA.n_elements() > 0 &&
      VarNameSA.n_elements() != FieldSA.n_elements()) {
      mlog << Error << "\nprocess_command_line() -> "
           << "When the -name option is used, the number of entries ("
           << VarNameSA.n_elements() << ") must match the number of "
           << "-field entries (" << FieldSA.n_elements() << ")!\n\n";
      usage();
   }

   RGInfo.validate();
   if (RGInfo.method == InterpMthd_Gaussian || RGInfo.method == InterpMthd_MaxGauss)
      RGInfo.gaussian.compute();

   return;
}

////////////////////////////////////////////////////////////////////////

void process_data_file() {
   DataPlane fr_dp, to_dp;
   Grid fr_grid, to_grid;
   GrdFileType ftype;
   double dmin, dmax;
   ConcatString run_cs, vname;
   //Variables for GOES
   unixtime valid_time = 0;
   bool opt_all_attrs = false;
   NcFile *nc_in = (NcFile *)0;
   static const char *method_name = "process_data_file() ";

   // Initialize configuration object
   MetConfig config;
   config.read(replace_path(config_const_filename).c_str());
   config.read_string(FieldSA[0].c_str());

   // Note: The command line argument MUST processed before this
   if (compress_level < 0) compress_level = config.nc_compression();

   // Get the gridded file type from config string, if present
   ftype = parse_conf_file_type(&config);

   // Read the input data file
   Met2dDataFileFactory m_factory;
   Met2dDataFile *fr_mtddf = (Met2dDataFile *) 0;

   // Determine the "from" grid
   mlog << Debug(1)  << "Reading data file: " << InputFilename << "\n";
   fr_mtddf = m_factory.new_met_2d_data_file(InputFilename.c_str(), ftype);

   if(!fr_mtddf) {
      mlog << Error << "\nprocess_data_file() -> "
           << "\"" << InputFilename << "\" not a valid data file\n\n";
      exit(1);
   }

   // Store the input data file types
   ftype = fr_mtddf->file_type();

   // Setup the VarInfo request object
   VarInfoFactory v_factory;
   VarInfo *vinfo;
   vinfo = v_factory.new_var_info(ftype);

   if(!vinfo) {
      mlog << Error << "\nprocess_data_file() -> "
           << "unable to determine file type of \"" << InputFilename
           << "\"\n\n";
      exit(1);
   }

   // For python types read the first field to set the grid
   if(is_python_grdfiletype(ftype)) {
      config.read_string(FieldSA[0].c_str());
      vinfo->set_dict(config);
      if(!fr_mtddf->data_plane(*vinfo, fr_dp)) {
         mlog << Error << "\nTrouble reading data from file \""
              << InputFilename << "\"\n\n";
         exit(1);
      }
   }

   fr_grid = fr_mtddf->grid();
   mlog << Debug(2) << "Input grid: " << fr_grid.serialize() << "\n";

   // Determine the "to" grid
   to_grid = parse_vx_grid(RGInfo, &fr_grid, &fr_grid);
   mlog << Debug(2) << "Output grid: " << to_grid.serialize() << "\n";

   GridTemplateFactory gtf;
   mlog << Debug(2) << "Interpolation options: "
        << "method = " << interpmthd_to_string(RGInfo.method)
        << ", width = " << RGInfo.width
        << ", shape = " << gtf.enum2String(RGInfo.shape)
        << ", vld_thresh = " << RGInfo.vld_thresh << "\n";

   // Build the run command string
   run_cs << "Regrid from " << fr_grid.serialize() << " to " << to_grid.serialize();

   ConcatString tmp_dir = config.get_tmp_dir();

   // Open the output file
   open_nc(to_grid, run_cs);

   // Loop through the requested fields
   for(int i=0; i<FieldSA.n_elements(); i++) {

      // Initialize
      vinfo->clear();

      // Populate the VarInfo object using the config string
      config.read_string(FieldSA[i].c_str());
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

      // Select output variable name
      if(VarNameSA.n() == 0) {
         vname << cs_erase << vinfo->name_attr();
         if(vinfo->level().type() != LevelType_Accum &&
            ftype != FileType_NcMet &&
            ftype != FileType_General_Netcdf &&
            ftype != FileType_NcPinterp &&
            ftype != FileType_NcCF) {
            vname << "_" << vinfo->level_attr();
         }
      }
      else {
         vname = VarNameSA[i];
      }

      // Write the regridded data
      write_nc(to_dp, to_grid, vinfo, vname.c_str());

   } // end for i

   // Close the output file
   close_nc();

   delete nc_in;  nc_in  = 0;

   // Clean up
   if(fr_mtddf) { delete fr_mtddf; fr_mtddf = (Met2dDataFile *) 0; }
   if(vinfo)    { delete vinfo;    vinfo    = (VarInfo *)       0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void open_nc(const Grid &grid, ConcatString run_cs) {

   // Create output file
   nc_out = open_ncfile(OutputFilename.c_str(), true);

   if(IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nopen_nc() -> "
           << "trouble opening output NetCDF file \""
           << OutputFilename << "\"\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, OutputFilename.c_str(), program_name.c_str());

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

void write_nc_data(const DataPlane &dp, const Grid &grid, NcVar *data_var) {

   // Allocate memory to store data values for each grid point
   float *data = new float [grid.nx()*grid.ny()];

   // Store the data
   int grid_nx = grid.nx();
   int grid_ny = grid.ny();
   for(int x=0; x<grid_nx; x++) {
      for(int y=0; y<grid_ny; y++) {
         int n = DefaultTO.two_to_one(grid_nx, grid_ny, x, y);
         data[n] = (float) dp(x, y);
      } // end for y
   } // end for x

   // Write out the data
   if(!put_nc_data_with_dims(data_var, &data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_nc_data() -> "
           << "error writing data to the output file.\n\n";
      exit(1);
   }

   // Clean up
   if(data) { delete [] data;  data = (float *)  0; }

   return;
}

void write_nc(const DataPlane &dp, const Grid &grid,
              const VarInfo *vinfo, const char *vname) {

   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = 0;

   NcVar data_var = add_var(nc_out, (string)vname, ncFloat,
                            lat_dim, lon_dim, deflate_level);
   add_att(&data_var, "name", (string)vname);
   add_att(&data_var, "long_name", (string)vinfo->long_name_attr());
   add_att(&data_var, "level", (string)vinfo->level_attr());
   add_att(&data_var, "units", (string)vinfo->units_attr());
   add_att(&data_var, "_FillValue", bad_data_float);
   write_netcdf_var_times(&data_var, dp);

   write_nc_data(dp, grid, &data_var);

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

   GridTemplateFactory gtf;
   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tinput_filename\n"
        << "\tto_grid\n"
        << "\toutput_filename\n"
        << "\t-field string\n"
        << "\t[-method type]\n"
        << "\t[-width n]\n"
        << "\t[-gaussian_dx n]\n"
        << "\t[-gaussian_radius n]\n"
        << "\t[-shape type]\n"
        << "\t[-vld_thresh n]\n"
        << "\t[-convert string]\n"
        << "\t[-censor thresh value]\n"
        << "\t[-name list]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"input_filename\" is the gridded data file to be "
        << "read (required).\n"

        << "\t\t\"to_grid\" defines the output grid as a named grid, "
        << "the path to a gridded data file, or an explicit grid "
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
        << "\t\t\tThe width should be the ratio of dx between "
        << "from_grid and to_grid for MAXGAUSS.\n"
        << "\t\t\tFor example, width=" << nint(RGInfo.gaussian.dx / 3)
        << " if the from_grid is 3 km and to_grid is "
        << RGInfo.gaussian.dx << "km.\n"

        << "\t\t\"-gaussian_dx n\" overrides the default a delta "
        << "distance for Gaussian smoothing (" << RGInfo.gaussian.dx
        << ") (optional).\n"

        << "\t\t\"-gaussian_radius n\" overrides the default radius of "
        << "influence for Gaussian smoothing ("
        << RGInfo.gaussian.radius << ") (optional).\n"

        << "\t\t\"-shape type\" overrides the default interpolation "
        << "shape (" << gtf.enum2String(RGInfo.shape) << ") (optional).\n"

        << "\t\t\"-vld_thresh n\" overrides the default required "
        << "ratio of valid data for regridding (" << RGInfo.vld_thresh
        << ") (optional).\n"

        << "\t\t\"-convert string\" specifies a conversion for the "
        << "regridded output (optional).\n"
        << "\t\t\tFor example, -convert 'convert(x) = x - 273.15;'\n"

        << "\t\t\"-censor thresh value\" specifies censoring logic for "
        << "the regridded output as a threshold string and replacement "
        << "value (optional).\n"

        << "\t\t\"-name list\" specifies a comma-separated list of "
        << "output variable names for each field specified (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t\"-compress level\" overrides the compression level of "
        << "NetCDF variable (optional).\n\n" << flush;

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_field(const StringArray &a) {
   FieldSA.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_method(const StringArray &a) {
   RGInfo.method = string_to_interpmthd(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_gaussian_dx(const StringArray &a) {
   RGInfo.gaussian.dx = atof(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////
void set_width(const StringArray &a) {
   RGInfo.width = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_gaussian_radius(const StringArray &a) {
   RGInfo.gaussian.radius = atof(a[0].c_str());
}


////////////////////////////////////////////////////////////////////////

void set_shape(const StringArray &a) {
   GridTemplateFactory gtf;
   RGInfo.shape = gtf.string2Enum(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_vld_thresh(const StringArray &a) {
   RGInfo.vld_thresh = atof(a[0].c_str());
   if(RGInfo.vld_thresh > 1 || RGInfo.vld_thresh < 0) {
      mlog << Error << "\nset_vld_thresh() -> "
           << "-vld_thresh may only be set between 0 and 1: "
           << RGInfo.vld_thresh << "\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

void set_convert(const StringArray &a) {

   // Can only be used once
   if(RGInfo.convert_fx.is_set()) {
      mlog << Error << "\nset_convert() -> "
           << "-convert_x may only be used once!\n\n";
      exit(1);
   }

   MetConfig config;
   config.read_string(a[0].c_str());

   RGInfo.convert_fx.set(config.lookup(conf_key_convert));
}

////////////////////////////////////////////////////////////////////////

void set_censor(const StringArray &a) {
   RGInfo.censor_thresh.add(a[0].c_str());
   RGInfo.censor_val.add(atof(a[1].c_str()));
}

////////////////////////////////////////////////////////////////////////

void set_name(const StringArray & a) {
   VarNameSA.add_css(a[0]);
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
