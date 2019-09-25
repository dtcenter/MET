// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

static const char * GOES_global_attr_names[] = {
      "naming_authority",
      "project",
      "production_site",
      "production_environment",
      "spatial_resolution",
      "orbital_slot",
      "platform_ID",
      "instrument_type",
      "scene_id",
      "instrument_ID",
      "dataset_name",
      "iso_series_metadata_id",
      "title",
      "keywords",
      "keywords_vocabulary",
      "processing_level",
      "date_created",
      "cdm_data_type",
      "time_coverage_start",
      "time_coverage_end",
      "timeline_id",
      "id"
};


// Variables for command line arguments
static ConcatString InputFilename;
static ConcatString OutputFilename;
static StringArray FieldSA;
static RegridInfo RGInfo;
static StringArray VarNameSA;
static int compress_level = -1;
static bool opt_override_method = false;
static bool opt_override_width = false;

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
static void set_name(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);
static void set_compress(const StringArray &);

////////////////////////////////////////////////////////////////////////
// for GOES 16
//
static const int factor_float_to_int = 1000000;
static const char *key_geostationary_data = "MET_GEOSTATIONARY_DATA";
static const char *dim_name_lat = "lat";
static const char *dim_name_lon = "lon";
static const char *var_name_lat = "latitude";
static const char *var_name_lon = "longitude";

static IntArray qc_flags;

static unixtime find_valid_time(multimap<string,NcVar> mapVar);
static void get_grid_mapping(Grid &fr_grid, Grid to_grid, IntArray *cellMapping,
                             ConcatString geostationary_file);
static int  get_lat_count(NcFile *);
static int  get_lon_count(NcFile *);
static ConcatString make_geostationary_filename(Grid fr_grid, Grid to_grid, bool grid_map=true);
static IntArray *read_grid_mapping(const char *grid_map_file);
static void regrid_goes_variable(NcFile *nc_in, Met2dDataFile *fr_mtddf,
      VarInfo *vinfo, DataPlane &fr_dp, DataPlane &to_dp,
      Grid fr_grid, Grid to_grid, IntArray *cellMapping);
static void save_geostationary_data(const ConcatString geostationary_file,
      const float *latitudes, const float *longitudes,
      const GoesImagerData grid_data);
static void set_goes_interpolate_option();
static void set_qc_flags(const StringArray &);
static void write_grid_mapping(const char *grid_map_file,
      IntArray *cellMapping, Grid fr_grid, Grid to_grid);

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
   RGInfo.gaussian_dx     = default_gaussian_dx;
   RGInfo.gaussian_radius = default_gaussian_radius;
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
   cline.add(set_name,       "-name",       1);
   cline.add(set_logfile,    "-log",        1);
   cline.add(set_verbosity,  "-v",          1);
   cline.add(set_compress,   "-compress",   1);
   cline.add(set_qc_flags,   "-qc",         1);

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

   ConcatString att_val;
   if (get_global_att(InputFilename.c_str(), (string)"scene_id", att_val)) {
      if ( att_val == "Full Disk" ||  att_val =="CONUS"
          || att_val == "Mesoscale" )
      set_goes_interpolate_option();
   }
cout << "  DEBUG HS gaussian_radius: " << RGInfo.gaussian_radius << ", gaussian_dx: " << RGInfo.gaussian_dx << " width: " << RGInfo.width << "\n";
   RGInfo.validate();

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
   int global_attr_count;
   bool opt_all_attrs = false;
   NcFile *nc_in = (NcFile *)0;
   IntArray *cellMapping = (IntArray *)0;
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
   if(ftype == FileType_Python_Numpy ||
      ftype == FileType_Python_Xarray) {
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

   bool is_geostationary = ( fr_grid.name() == "geostationary" );

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
   ConcatString geostationary_file(tmp_dir);
   ConcatString grid_map_file(tmp_dir);
   if (is_geostationary) {
      ConcatString env_coord_name;
      grid_map_file.add("/");
      grid_map_file.add(make_geostationary_filename(fr_grid, to_grid));
      geostationary_file.add("/");
      geostationary_file.add(make_geostationary_filename(fr_grid, to_grid, false));
      if (file_exists(grid_map_file.c_str())) {
         run_cs << " with " << grid_map_file;
      }
      else if (get_env(key_geostationary_data, env_coord_name) &&
               env_coord_name.nonempty() &&
               file_exists(env_coord_name.c_str())) {
         run_cs << " with " << env_coord_name;
      }
      else if (file_exists(geostationary_file.c_str())) {
         run_cs << " with " << geostationary_file;
      }
   }

   // Open the output file
   open_nc(to_grid, run_cs);

   if (is_geostationary) {
      if ((RGInfo.method != InterpMthd_Min)
            && (RGInfo.method != InterpMthd_Max)
            && (RGInfo.method != InterpMthd_Median)
            && (RGInfo.method != InterpMthd_UW_Mean)) {
         mlog << Error << "\n" << method_name << "The Interpolation method \""
              << interpmthd_to_string(RGInfo.method)
              << "\" is not supported for GOES 16.\n\n";
         exit(1);
      }

      nc_in = open_ncfile(InputFilename.c_str());

      multimap<string,NcVar> mapVar = GET_NC_VARS_P(nc_in);

      valid_time = find_valid_time(mapVar);
      to_dp.set_init(valid_time);
      to_dp.set_valid(valid_time);
      to_dp.set_size(to_grid.nx(), to_grid.ny());
      global_attr_count =  sizeof(GOES_global_attr_names)/sizeof(*GOES_global_attr_names);
      if (file_exists(grid_map_file.text())) {
         cellMapping = read_grid_mapping(grid_map_file.text());
      }
      else {
         cellMapping = new IntArray[to_grid.nx() * to_grid.ny()];
         get_grid_mapping(fr_grid, to_grid, cellMapping, geostationary_file);
         write_grid_mapping(grid_map_file.text(), cellMapping,
               fr_grid, to_grid);
      }
   }

   // Loop through the requested fields
   for(int i=0; i<FieldSA.n_elements(); i++) {

      // Initialize
      vinfo->clear();

      // Populate the VarInfo object using the config string
      config.read_string(FieldSA[i].c_str());
      vinfo->set_dict(config);

      if (is_geostationary) {
         to_dp.erase();
         to_dp.set_init(valid_time);
         to_dp.set_valid(valid_time);
         regrid_goes_variable(nc_in, fr_mtddf, vinfo, fr_dp, to_dp,
               fr_grid, to_grid, cellMapping);
      }
      else {
         // Get the data plane from the file for this VarInfo object
         if(!fr_mtddf->data_plane(*vinfo, fr_dp)) {
            mlog << Error << "\nprocess_data_file() -> trouble getting field \""
                 << FieldSA[i] << "\" from file \"" << InputFilename << "\"\n\n";
            exit(1);
         }

         // Regrid the data plane
         to_dp = met_regrid(fr_dp, fr_grid, to_grid, RGInfo);
      }

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
      if(VarNameSA.n_elements() == 0) {
         vname << cs_erase << vinfo->name();
         if(vinfo->level().type() != LevelType_Accum &&
            ftype != FileType_NcMet &&
            ftype != FileType_General_Netcdf &&
            ftype != FileType_NcPinterp &&
            ftype != FileType_NcCF) {
            vname << "_" << vinfo->level_name();
         }
      }
      else {
         vname = VarNameSA[i];
      }

      // Write the regridded data
      write_nc(to_dp, to_grid, vinfo, vname.c_str());

      if (is_geostationary) {
         NcVar to_var = get_nc_var(nc_out, vname.c_str());
         NcVar var_data = get_nc_var(nc_in, vinfo->name().c_str());
         for (int idx=0; idx<global_attr_count; idx++) {
	   copy_nc_att(nc_in, &to_var, (string)GOES_global_attr_names[idx]);
         }
         copy_nc_atts(&var_data, &to_var, opt_all_attrs);
      }

   } // end for i

   if (is_geostationary && nc_in != NULL) {
      multimap<string,NcVar> mapVar = GET_NC_VARS_P(nc_in);
      for (multimap<string,NcVar>::iterator itVar = mapVar.begin();
            itVar != mapVar.end(); ++itVar) {
         if ((*itVar).first == "t"
               || string::npos != (*itVar).first.find("time")) {
            NcVar from_var = (*itVar).second;
            copy_nc_var(nc_out, &from_var);
         }
      }
      //copy_nc_atts(_nc_in, nc_out, opt_all_attrs);
   }

   // Close the output file
   close_nc();

   // nc_in->close();
   delete nc_in;  nc_in = 0;

   delete [] cellMapping;   cellMapping = 0;

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
   add_att(&data_var, "long_name", (string)vinfo->long_name());
   add_att(&data_var, "level", (string)vinfo->level_name());
   add_att(&data_var, "units", (string)vinfo->units());
   add_att(&data_var, "_FillValue", bad_data_float);
   write_netcdf_var_times(&data_var, dp);

   write_nc_data(dp, grid, &data_var);

   return;
}

////////////////////////////////////////////////////////////////////////
// GOES related modules
//

void check_lat_lon(int data_size, float  *latitudes, float  *longitudes) {
   int cnt_printed = 0;
   int cnt_missing_lat = 0;
   int cnt_missing_lon = 0;
   int cnt_bad_lat = 0;
   int cnt_bad_lon = 0;
   float min_lat=90.0;
   float max_lat=-90.0;
   float min_lon=360.0;
   float max_lon=-360.0;
   for (int idx=0; idx<data_size; idx++) {
      if (cnt_printed < 10 && latitudes[idx] > MISSING_LATLON
            && longitudes[idx] > MISSING_LATLON) {
         mlog << Debug(7) << "  index: " << idx <<  " lat: " << latitudes[idx]
                          << ", lon: " << longitudes[idx] << "\n";
         cnt_printed++;
      }
      if (latitudes[idx] <= MISSING_LATLON) cnt_missing_lat++;
      else if (latitudes[idx] < -90 || latitudes[idx] > 90)  cnt_bad_lat++;
      else {
         if (min_lat > latitudes[idx]) min_lat = latitudes[idx];
         if (max_lat < latitudes[idx]) max_lat = latitudes[idx];
      }
      if (longitudes[idx] <= MISSING_LATLON) cnt_missing_lon++;
      else if (longitudes[idx] < -180 || longitudes[idx] > 180) cnt_bad_lon++;
      else {
         if (min_lon > longitudes[idx]) min_lon = longitudes[idx];
         if (max_lon < longitudes[idx]) max_lon = longitudes[idx];
      }
   }
   mlog << Debug(7) << "\n Count: missing - lat: " << cnt_missing_lat
        << ", lon: " << cnt_missing_lon << "\n"
        << "        invalid - lat: " << cnt_bad_lat << ", lon: " << cnt_bad_lon << "\n"
        << "     LAT min: " << min_lat << ", max: " << max_lat << "\n"
        << "    LONG min: " << min_lon << ", max: " << max_lon << "\n";
}

////////////////////////////////////////////////////////////////////////

unixtime find_valid_time(multimap<string,NcVar> mapVar) {
   NcVar from_var;
   unixtime valid_time;
   ConcatString time_unit, tmp_time_unit;
   double time_values [100];
   static const char *method_name = "find_valid_time() ";

   valid_time = 0;
   time_values[0] = 0;
   for (multimap<string,NcVar>::iterator itVar = mapVar.begin();
         itVar != mapVar.end(); ++itVar) {
      if ((*itVar).first == "t" || (*itVar).first == "time") {
         from_var = (*itVar).second;
         get_nc_data(&from_var, time_values);
	 get_nc_att(&from_var, (string)"units", time_unit);
         valid_time = get_reference_unixtime(time_unit);
         valid_time += (unixtime)nint(time_values[0]);
         mlog << Debug(2) << method_name << "valid time: " << time_values[0]
              << " (" << (int)time_values[0] << ") " << time_unit << " ==> "
              << unix_to_yyyymmdd_hhmmss(valid_time) << "\n";
         break;
      }
   }

   if (valid_time == 0) {
      mlog << Error << "\n" << method_name << "-> "
           << "trouble finding time variable from \""
           << InputFilename << "\"\n\n";
      exit(1);
   }

   return valid_time;
}

////////////////////////////////////////////////////////////////////////

void get_grid_mapping(Grid &fr_grid, Grid to_grid, IntArray *cellMapping,
                      ConcatString geostationary_file) {
   static const char *method_name = "get_grid_mapping() ";
   DataPlane from_dp, to_dp;
   ConcatString cur_coord_name;

   int to_lat_count = to_grid.ny();
   int to_lon_count = to_grid.nx();
   int from_lat_count = fr_grid.ny();;
   int from_lon_count = fr_grid.nx();

   bool has_coord_input = false;
   ConcatString tmp_coord_name;   

   if (get_env(key_geostationary_data, tmp_coord_name) &&
       tmp_coord_name.nonempty() &&
       file_exists(tmp_coord_name.c_str())) {
      has_coord_input = true;
      cur_coord_name = tmp_coord_name;
   }
   else {
      if (file_exists(geostationary_file.c_str())) {
         has_coord_input = true;
         cur_coord_name = geostationary_file;
      }
   }

   // Override the from nx & ny from NetCDF if exists
   NcFile *coord_nc_in = (NcFile *)0;
   if (has_coord_input) {
      mlog << Debug(2)  << method_name << " Reading coord file: " << cur_coord_name << "\n";
      coord_nc_in = open_ncfile(cur_coord_name.c_str());
      if (!IS_INVALID_NC_P(coord_nc_in)) {
         from_lat_count = get_lat_count(coord_nc_in);
         from_lon_count = get_lon_count(coord_nc_in);
      }
   }
   int data_size  = from_lat_count * from_lon_count;
   mlog << Debug(4) << method_name << " data_size (ny*nx): " << data_size
        << " = " << from_lat_count << " * " << from_lon_count << "\n"
        << "                    target grid (nx,ny)="
        << to_lon_count << "," << to_lat_count << "\n";

   from_dp.set_size(from_lon_count, from_lat_count);
   to_dp.set_size(to_lon_count, to_lat_count);

   if (data_size > 0) {
      double x, y;
      float  lat, lon;
      int    idx_x, idx_y;
      int    coord_offset, to_offset;
      int    count_in_grid;
      float  *latitudes  = (float *)NULL;
      float  *longitudes = (float *)NULL;
      bool latlon_allocated = false;
      float  *latitudes_buf  = (float *)NULL;
      float  *longitudes_buf = (float *)NULL;
      int buff_size = data_size*sizeof(float);
      GoesImagerData grid_data;
      grid_data.reset();

      if (has_coord_input) {
         latitudes_buf  = new float[data_size];
         longitudes_buf = new float[data_size];

         latitudes = latitudes_buf;
         longitudes = longitudes_buf;
         memset(latitudes,  0, buff_size);
         memset(longitudes, 0, buff_size);

         if (!IS_INVALID_NC_P(coord_nc_in)) {
            NcVar var_lat = get_nc_var(coord_nc_in, var_name_lat);
            NcVar var_lon = get_nc_var(coord_nc_in, var_name_lon);
            if (!IS_INVALID_NC(var_lat) && !IS_INVALID_NC(var_lon)) {
               get_nc_data(&var_lat, latitudes);
               get_nc_data(&var_lon, longitudes);
            }
         }
         else {
            FILE *pFile = met_fopen ( cur_coord_name.c_str(), "rb" );
            (void) fread (latitudes,sizeof(latitudes[0]),data_size,pFile);
            (void) fread (longitudes,sizeof(longitudes[0]),data_size,pFile);
            fclose (pFile);

            bool compare_binary_and_computation = false;
            if (compare_binary_and_computation && fr_grid.info().gi) {
               grid_data.copy(fr_grid.info().gi);
               grid_data.compute_lat_lon();
               grid_data.test();

               int lat_matching_count = 0;
               int lat_mis_matching_count = 0;
               int lon_matching_count = 0;
               int lon_mis_matching_count = 0;
               float *tmp_lats  = grid_data.lat_values;
               float *tmp_lons  = grid_data.lon_values;

               for (int idx=0; idx<data_size; idx++) {
                   if ((latitudes[idx] > MISSING_LATLON) && (tmp_lats[idx] > MISSING_LATLON)) {
                      if (!is_eq(latitudes[idx], tmp_lats[idx], loose_tol)) {
                         lat_mis_matching_count++;
                         mlog << Warning << method_name << "diff lat at " << idx
                              << "  binary-computing: " << latitudes[idx] << " - "
                              << tmp_lats[idx] << " = " << (latitudes[idx]-tmp_lats[idx]) << "\n";
                      }
                      else lat_matching_count++;
                   }
                   else lat_matching_count++;
                   if ((longitudes[idx] > MISSING_LATLON) && (tmp_lons[idx] > MISSING_LATLON)) {
                      if (!is_eq(longitudes[idx], tmp_lons[idx], loose_tol)) {
                         lon_mis_matching_count++;
                         mlog << Warning << method_name << "diff lon at " << idx
                              << "  binary-computing: " << longitudes[idx] << " - "
                              << tmp_lons[idx] << " = " << (longitudes[idx]-tmp_lons[idx]) << "\n";
                      }
                      else lon_matching_count++;
                   }
                   else lon_matching_count++;
               }
               if ((lon_mis_matching_count > 0) || (lat_mis_matching_count > 0)) {
                  mlog << Warning << method_name << "mis-matching: lon = " << lon_mis_matching_count
                       << "  lat =  " << lat_mis_matching_count << "   matched:  lon = "
                       << lon_matching_count << "  lat =  " << lat_matching_count << "\n";
               }
            }
         }
      }
      else {
         if (fr_grid.info().gi) {
            grid_data.copy(fr_grid.info().gi);
            grid_data.compute_lat_lon();
            latitudes = grid_data.lat_values;
            longitudes = grid_data.lon_values;
            if (!file_exists(geostationary_file.c_str())) {
               save_geostationary_data(geostationary_file,
                     latitudes, longitudes, grid_data);
            }
         }
      }
      if (latitudes && longitudes) {
         check_lat_lon(data_size, latitudes, longitudes);

         count_in_grid = 0;

         //Following the logic at DataPlane::two_to_one(int x, int y) n = y*Nx + x;
         for (int xIdx=0; xIdx<from_lat_count; xIdx++) {
            for (int yIdx=0; yIdx<from_lon_count; yIdx++) {
               coord_offset = from_dp.two_to_one(yIdx, xIdx);
               lat = latitudes[coord_offset];
               lon = longitudes[coord_offset];
               to_grid.latlon_to_xy(lat, -1.0*lon, x, y);
               idx_x = nint(x);
               idx_y = nint(y);

               if (0 <= idx_x && idx_x < to_lon_count && 0 <= idx_y && idx_y < to_lat_count) {
                  to_offset = to_dp.two_to_one(idx_x, idx_y);
                  cellMapping[to_offset].add(coord_offset);
                  count_in_grid++;
               }
            }
         }
         mlog << Debug(3) << method_name << " within grid: " << count_in_grid
              << " out of " << data_size << " (" << count_in_grid*100/data_size << "%)\n";
      }
      else {
         if (0 == latitudes)
            mlog << Error << method_name << " Fail to get latitude\n";
         if (0 == longitudes)
            mlog << Error << method_name << " Fail to get longitudes\n";
      }

      if (latitudes_buf)  { delete [] latitudes_buf;   latitudes_buf  = NULL; }
      if (longitudes_buf) { delete [] longitudes_buf;  longitudes_buf = NULL; }

      grid_data.release();

   }   //  if data_size > 0


   if(coord_nc_in) {
      delete coord_nc_in;  coord_nc_in = 0;
   }
}

////////////////////////////////////////////////////////////////////////

int get_lat_count(NcFile *_nc) {
   int lat_count = 0;
   NcDim dim_lat = get_nc_dim(_nc, dim_name_lat);
   if(IS_INVALID_NC(dim_lat)) dim_lat = get_nc_dim(_nc, "y");
   if(!IS_INVALID_NC(dim_lat)) lat_count= get_dim_size(&dim_lat);
   return lat_count;
}

int get_lon_count(NcFile *_nc) {
   int lon_count = 0;
   NcDim dim_lon = get_nc_dim(_nc, dim_name_lon);
   if(IS_INVALID_NC(dim_lon)) dim_lon = get_nc_dim(_nc, "x");
   if(!IS_INVALID_NC(dim_lon)) lon_count= get_dim_size(&dim_lon);
   return lon_count;
}

////////////////////////////////////////////////////////////////////////

static ConcatString make_geostationary_filename(
      Grid fr_grid, Grid to_grid, bool grid_map) {
   ConcatString geo_data_filename;
   GridInfo info = fr_grid.info();

   if (info.gi) {
      size_t offset;
      string scene_id = info.gi->scene_id;
      offset = scene_id.find(' ');
      if (offset != string::npos) {
         geo_data_filename << scene_id.substr(0, offset).c_str() << "_";
      }
      else {
         geo_data_filename << scene_id.c_str() << "_";
      }
   }
   geo_data_filename << fr_grid.nx() << "_" << fr_grid.ny();
   if (info.gi) {
      geo_data_filename << "_" << int(info.gi->dx_rad * factor_float_to_int)
            << "_" << int(info.gi->dy_rad * factor_float_to_int)
            << "_" << int(info.gi->x_image_bounds[0] * factor_float_to_int)
            << "_" << int(info.gi->y_image_bounds[0] * factor_float_to_int);
   }
   if (grid_map) {
      geo_data_filename << "_to_" << to_grid.name() << ".grid_map";
   }
   else {
      geo_data_filename << ".nc";
   }
   return geo_data_filename;
}

////////////////////////////////////////////////////////////////////////

IntArray *read_grid_mapping(const char *grid_map_file) {
   static const char *method_name = "read_grid_mapping() ";

   //if ( ! file_exists(filename) )  {
   int nx, ny, map_size;
   IntArray *cellMapping = 0;

   nx = ny = map_size = 0;
   string line;
   ifstream map_file (grid_map_file);
   if (map_file.is_open()) {
      bool map_data;
      int to_offset, coord_offset, map_count;
      StringArray str_arr;
      StringArray cell_index_arr;

      map_count = 0;
      map_data = false;
      while ( getline (map_file, line) ) {
         if (0 == line.compare("[mapping]")) {
            map_data = true;
            continue;
         }
         if (0 == line.compare("[metadata]")) {
            map_data = false;
            continue;
         }

         str_arr.clear();
         str_arr.parse_delim(line.c_str(), "=");
         if (str_arr.n_elements() == 2) {
            if (map_data) {
               cell_index_arr.clear();
               to_offset = atoi(str_arr[0].c_str());
               if ((to_offset >= 0) && (to_offset < map_size)) {
                  cell_index_arr.parse_delim(str_arr[1], ",");
                  if (0 < cell_index_arr.n_elements()) map_count++;
                  for (int idx=0; idx<cell_index_arr.n_elements(); idx++) {
                     coord_offset = atoi(cell_index_arr[idx].c_str());
                     if (coord_offset >= 0) {
                        cellMapping[to_offset].add(coord_offset);
                     }
                  }
               }
               else {
                  mlog << Warning << method_name << "The offset "
                       << to_offset  << "is out of range\n";
               }
            }
            else if ( strcasecmp(str_arr[0].c_str(), "nx") == 0 )  {
 	       nx = atoi(str_arr[1].c_str());
               if ((nx > 0) && (ny > 0)) {
                  if (cellMapping == 0) {
                     map_size = nx * ny;
                     cellMapping = new IntArray[map_size];
                  }
                  else {
                     mlog << Warning << method_name << "Ignored " << line << "\n";
                  }
               }
            }
            else if ( strcasecmp(str_arr[0].c_str(), "ny") == 0 )  {
	       ny = atoi(str_arr[1].c_str());
               if ((nx > 0) && (ny > 0)) {
                  if (cellMapping == 0) {
                     map_size = nx * ny;
                     cellMapping = new IntArray[map_size];
                  }
                  else {
                     mlog << Warning << method_name << "Ignored " << line << "\n";
                  }
               }
            }
            else {
               mlog << Debug(7) << method_name << "Not using " << line << "\n";
            }
         }
      }
      map_file.close();
      mlog << Debug(7) << method_name << "Read " << map_count
           << " mappings (out of " << map_size << ")\n";
   }
   else {
      mlog << Error << method_name << "Unable to open file" << grid_map_file << "\n";
   }
   return cellMapping;
}

////////////////////////////////////////////////////////////////////////

void regrid_goes_variable(NcFile *nc_in, Met2dDataFile *fr_mtddf,
      VarInfo *vinfo, DataPlane &fr_dp, DataPlane &to_dp,
      Grid fr_grid, Grid to_grid, IntArray *cellMapping) {

   bool has_qc_var = false;
   int to_lat_count = to_grid.ny();
   int to_lon_count = to_grid.nx();
   int from_lat_count = fr_grid.ny();
   int from_lon_count = fr_grid.nx();
   int from_data_size = from_lat_count * from_lon_count;
   ConcatString qc_var_name;
   ncbyte qc_value;
   ncbyte  *qc_data = new ncbyte[from_data_size];
   float *from_data = new float[from_data_size];
   static const char *method_name = "regrid_goes_variable() ";

   // -99 is arbitrary number as invalid QC value
   memset(qc_data, -99, from_data_size*sizeof(ncbyte));

   NcVar var_qc;
   NcVar var_data = get_nc_var(nc_in, vinfo->name().c_str());
   if (IS_INVALID_NC(var_data)) {
      mlog << Error << "The variable \"" << vinfo->name() << "\" does not exist\n";
      exit(1);
   }

   //AOD:ancillary_variables = "DQF" ; byte DQF(y, x) ;
   if (get_att_value_string(&var_data, (string)"ancillary_variables", qc_var_name)) {
      var_qc = get_nc_var(nc_in, qc_var_name.c_str());
      get_nc_data(&var_qc, qc_data);
      has_qc_var = true;
      mlog << Debug(3) << method_name << "found QC var: " << qc_var_name << ".\n";
   }

   get_nc_data(&var_data, (float *)from_data);

   fr_dp.set_size(from_lon_count, from_lat_count);
   for (int xIdx=0; xIdx<from_lon_count; xIdx++) {
      for (int yIdx=0; yIdx<from_lat_count; yIdx++) {
         int offset = fr_dp.two_to_one(xIdx,yIdx);
         fr_dp.set(from_data[offset],xIdx,yIdx);
      }
   }

   int offset = 0;
   int valid_count = 0;
   int censored_count = 0;
   int missing_count = 0;
   int non_missing_count = 0;
   int qc_filtered_count = 0;
   // int global_attr_count;
   float data_value;
   float from_min_value =  10e10;
   float from_max_value = -10e10;
   float qc_min_value =  10e10;
   float qc_max_value = -10e10;
   IntArray cellArray;
   NumArray dataArray;
   bool has_qc_flags = (qc_flags.n_elements() > 0);

   missing_count = non_missing_count = 0;
   to_dp.set_constant(bad_data_double);

   for (int xIdx=0; xIdx<to_lon_count; xIdx++) {
      for (int yIdx=0; yIdx<to_lat_count; yIdx++) {
         offset = to_dp.two_to_one(xIdx,yIdx);
         cellArray = cellMapping[offset];
         if (0 < cellArray.n_elements()) {
            valid_count = 0;
            dataArray.clear();
            for (int dIdx=0; dIdx<cellArray.n_elements(); dIdx++) {
               data_value = from_data[cellArray[dIdx]];
               if (is_eq(data_value, bad_data_float)) {
                  missing_count++;
                  continue;
               }

               non_missing_count++;
               if(mlog.verbosity_level() >= 4) {
                  if (from_min_value > data_value) from_min_value = data_value;
                  if (from_max_value < data_value) from_max_value = data_value;
               }

               //Filter by QC flag
               qc_value = qc_data[cellArray[dIdx]];
               if (!has_qc_var || !has_qc_flags || qc_flags.has(qc_value)) {
                  for(int i=0; i<vinfo->censor_thresh().n_elements(); i++) {
                     // Break out after the first match.
                     if(vinfo->censor_thresh()[i].check(data_value)) {
                        data_value = vinfo->censor_val()[i];
                        censored_count++;
                        break;
                     }
                  }
                  dataArray.add(data_value);
                  if (mlog.verbosity_level() >= 4) {
                     if (qc_min_value > qc_value) qc_min_value = qc_value;
                     if (qc_max_value < qc_value) qc_max_value = qc_value;
                  }
               }
               else {
                  qc_filtered_count++;
               }
               valid_count++;
            }
            if (0 < dataArray.n_elements()) {
               int data_count = dataArray.n_elements();
               float to_value;
               if      (RGInfo.method == InterpMthd_Min) to_value = dataArray.min();
               else if (RGInfo.method == InterpMthd_Max) to_value = dataArray.max();
               else if (RGInfo.method == InterpMthd_Median) {
                  cellArray.sort_increasing();
                  to_value = dataArray[data_count/2];
                  if (0 == data_count % 2)
                     to_value = (to_value + dataArray[(data_count/2)+1])/2;
               }
               else to_value = dataArray.sum() / data_count;

               to_dp.set(to_value, xIdx, yIdx);
               mlog << Debug(9) << method_name
                    <<   "max: " << dataArray.max()
                    << ", min: " << dataArray.min()
                    << ", mean: " << dataArray.sum()/data_count
                    << " from " << data_count << " data values.\n";
            }
         }
         else {}
      }
   }

   delete [] qc_data;   qc_data = 0;
   delete [] from_data; from_data = 0;

   mlog << Debug(4) << method_name << " Count: missing: "
        << missing_count << ", non_missing: " << non_missing_count
        << ", value range: [" << from_min_value << " - " << from_max_value
        << "] QCed: Filtered: " << qc_filtered_count
        << " [" << qc_min_value << " - " << qc_max_value << "]\n";
}

////////////////////////////////////////////////////////////////////////

static void save_geostationary_data(const ConcatString geostationary_file,
      const float *latitudes, const float *longitudes,
      const GoesImagerData grid_data) {
   bool has_error = false;
   int deflate_level = 0;
   static const char *method_name = "save_geostationary_data() ";

   NcFile *nc_file = open_ncfile(geostationary_file.text(), true);
   NcDim xdim = add_dim(nc_file, dim_name_lon, grid_data.nx);
   NcDim ydim = add_dim(nc_file, dim_name_lat, grid_data.ny);

   NcVar lat_var = add_var(nc_file, var_name_lat, ncFloat, ydim, xdim, deflate_level);
   NcVar lon_var = add_var(nc_file, var_name_lon, ncFloat, ydim, xdim, deflate_level);

   if (!IS_INVALID_NC(lat_var)) {
      if (grid_data.dy_rad >= 0) {
         add_att(&lat_var, "standard_name", var_name_lat);
         add_att(&lat_var, "units","degrees_north");
      }
      else {
         add_att(&lat_var, "long_name", var_name_lat);
         add_att(&lat_var, "units","degrees_south");
      }
      add_att(&lat_var, "dy_rad", grid_data.dy_rad);
      if(!put_nc_data((NcVar *)&lat_var, latitudes)) {
         has_error = true;
         mlog << Error << "Can not save latitudes\n";
      }
   }
   if (!IS_INVALID_NC(lon_var)) {
      if (grid_data.dy_rad >= 0) {
         add_att(&lon_var, "standard_name", var_name_lon);
         add_att(&lon_var, "units","degrees_east");
      }
      else {
         add_att(&lon_var, "long_name", var_name_lon);
         add_att(&lon_var, "units","degrees_west");
      }
      add_att(&lon_var, "dx_rad", grid_data.dx_rad);
      if(!put_nc_data((NcVar *)&lon_var, longitudes)) {
         has_error = true;
         mlog << Error << "Can not save longitudes\n";
      }
   }

   add_att(nc_file, "Conventions", "CF-1.6");

   if (has_error) {
      remove(geostationary_file.c_str());
      mlog << Warning << "The geostationary data file ("
           << geostationary_file << ") was not saved!\n";
   }
   else {
     mlog << Debug(3) << method_name << "The geostationary data file ("
          << geostationary_file << ") was saved\n";
   }

   delete nc_file;  nc_file = 0;

}

////////////////////////////////////////////////////////////////////////

void set_goes_interpolate_option() {
   if (!opt_override_width && RGInfo.width == DefaultInterpWdth) {
      RGInfo.width = 2;
   }
   if (!opt_override_method && RGInfo.method == DefaultInterpMthd) {
      RGInfo.method = InterpMthd_UW_Mean;
   }
}

////////////////////////////////////////////////////////////////////////

void write_grid_mapping(const char *grid_map_file,
      IntArray *cellMapping, Grid from_grid, Grid to_grid) {
   static const char *method_name = "write_grid_mapping() ";
   int nx = to_grid.nx();
   int ny = to_grid.ny();

   if ( file_exists(grid_map_file) )  {
      mlog << Warning << method_name << grid_map_file << "Exist already\n";
   }
   else {
      ofstream map_file (grid_map_file);
      if (map_file.is_open()) {
         int map_count = 0;
         int map_size = nx * ny;
         GridInfo info = from_grid.info();
         map_file << "[metadata]\n";
         map_file << "nx=" << nx << "\n";
         map_file << "ny=" << ny << "\n";
         map_file << "from_nx=" << from_grid.nx() << "\n";
         map_file << "from_ny=" << from_grid.ny() << "\n";
         if (info.gi) {
            map_file << "from_dx=" << info.gi->dx_rad << "\n";
            map_file << "from_dy=" << info.gi->dy_rad << "\n";
         }
         map_file << "[mapping]\n";
         for (int idx=0; idx<map_size; idx++) {
            int mem_count = cellMapping[idx].n_elements();
            map_file << idx << "=";
            if (mem_count > 0) {
               for (int idx2=0; idx2<mem_count; idx2++) {
                  map_file << cellMapping[idx][idx2];
                  if (idx2 < (mem_count-1)) map_file << ",";
               }
               map_count++;
            }
            map_file << "\n";
         }
         mlog << Debug(5) << method_name << "Write "
              << map_count << " grid mappings (out of " << map_size << ")\n";
      }
      else {
         mlog << Error << method_name << "Unable to open file "
              << grid_map_file << "\n";
      }
      map_file.close();
   }
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
        << "\t[-qc flags]\n"
        << "\t[-method type]\n"
        << "\t[-width n]\n"
        << "\t[-gaussan_dx n]\n"
        << "\t[-gaussan_radius n]\n"
        << "\t[-shape type]\n"
        << "\t[-vld_thresh n]\n"
        << "\t[-name list]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"input_filename\" is the gridded data file to be "
        << "read (required).\n"

        << "\t\t\"to_grid\" defines the output grid as a named grid, the "
        << "path to a gridded data file, or an explicit grid "
        << "specification string (required).\n"

        << "\t\t\"output_filename\" is the output NetCDF file to be "
        << "written (required).\n"

        << "\t\t\"-field string\" may be used multiple times to define "
        << "the data to be regridded (required).\n"

        << "\t\t\"-qc flags\" specifies a comma-separated list of QC flags, for example \"0,1\" (optional).\n"
        << "\t\t\tOnly applied if grid_mapping is set to \"goes_imager_projection\" and the QC variable exists.\n"

        << "\t\t\"-method type\" overrides the default regridding "
        << "method (" << interpmthd_to_string(RGInfo.method)
        << ") (optional).\n"

        << "\t\t\"-width n\" overrides the default regridding "
        << "width (" << RGInfo.width << ") (optional).\n"

        << "\t\t\"-gaussian_dx n\" specifies a delta distance for Gaussian smoothing."
        << " The default is " << RGInfo.gaussian_dx << ". Ignored if not Gaussian method (optional).\n"

        << "\t\t\"-gaussian_radius n\" specifies the radius of influence for Gaussian smoothing."
        << " The default is " << RGInfo.gaussian_radius << "). Ignored if not Gaussian method (optional).\n"

        << "\t\t\"-shape type\" overrides the default interpolation shape ("
        << gtf.enum2String(RGInfo.shape) << ") "
        <<  "(optional).\n"

        << "\t\t\"-vld_thresh n\" overrides the default required "
        << "ratio of valid data for regridding (" << RGInfo.vld_thresh
        << ") (optional).\n"

        << "\t\t\"-name list\" specifies a comma-separated list of "
        << "output variable names for each field specified (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t\"-compress level\" overrides the compression level of NetCDF variable (optional).\n\n" << flush;

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_field(const StringArray &a) {
   FieldSA.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_method(const StringArray &a) {
   RGInfo.method = string_to_interpmthd(a[0].c_str());
   opt_override_method = true;
}

////////////////////////////////////////////////////////////////////////

void set_gaussian_dx(const StringArray &a) {
   RGInfo.gaussian_dx = atof(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////
void set_width(const StringArray &a) {
   RGInfo.width = atoi(a[0].c_str());
   opt_override_width = true;
}

////////////////////////////////////////////////////////////////////////

void set_gaussian_radius(const StringArray &a) {
   RGInfo.gaussian_radius = atof(a[0].c_str());
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

void set_qc_flags(const StringArray & a) {
   int qc_flag;
   StringArray sa;

   sa.parse_css(a[0]);
   for (int idx=0; idx<sa.n_elements(); idx++) {
      qc_flag = atoi(sa[idx].c_str());
      if ( !qc_flags.has(qc_flag) ) qc_flags.add(qc_flag);
   }
}
