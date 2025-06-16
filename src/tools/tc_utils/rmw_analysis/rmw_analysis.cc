// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <dirent.h>
#include <fstream>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <netcdf>

#include "main.h"
#include "rmw_analysis.h"

#include "vx_statistics.h"
#include "vx_nc_util.h"
#include "vx_tc_util.h"
#include "vx_util.h"
#include "vx_log.h"

using namespace std;
using namespace netCDF;

////////////////////////////////////////////////////////////////////////

static void usage();
static void process_command_line(int, char**);
static void set_data_files(const StringArray&);
static void set_config(const StringArray&);
static void set_out(const StringArray&);
static void setup();
static void process_data_files();
static void normalize_stats();
static void write_stats();
static bool is_keeper_track(const TrackInfo &);
static bool is_keeper_point(const TrackPoint &);
static TrackInfo read_nc_track();
static TrackInfo parse_track_file(const ConcatString&);

////////////////////////////////////////////////////////////////////////

int met_main(int argc, char *argv[]) {

   // Process command line arguments
   process_command_line(argc, argv);

   // Set up
   setup();

   // Process data files
   process_data_files();

   // Compute mean and standard deviation
   normalize_stats();

   // Write output
   write_stats();

   return 0;
}

////////////////////////////////////////////////////////////////////////

const string get_tool_name() {
   return "rmw_analysis";
}

////////////////////////////////////////////////////////////////////////

static void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
       << ") ***\n\n"
       << "Usage: " << program_name << "\n"
       << "\t-data file_1 ... file_n | file_list\n"
       << "\t-config file\n"
       << "\t-out file\n"
       << "\t[-log file]\n"
       << "\t[-v level]\n\n"

       << "\twhere\t\"-data file_1 ... file_n | file_list\" "
       << "is a list of the NetCDF TC-RMW output files to be processed "
       << "or an ASCII file containing a list of file names (required).\n"

       << "\t\t\"-config file\" is the RMWAnalysisConfig to be used "
       << "(required).\n"

       << "\t\t\"-out file\" is the NetCDF output file to be written "
       << "(required).\n"

       << "\t\t\"-log file\" outputs log messages to the specified "
       << "file (optional).\n"

       << "\t\t\"-v level\" overrides the default level of logging ("
       << mlog.verbosity_level() << ") (optional).\n\n" << flush;

   exit(1);
}

////////////////////////////////////////////////////////////////////////

static void process_command_line(int argc, char **argv) {

   CommandLine cline;
   ConcatString default_config_file;

   // Default output directory
   out_dir = replace_path(default_out_dir);

   // Print usage statement for no arguments
   if(argc <= 1) usage();

   // Parse command line into tokens
   cline.set(argc, argv);

   // Set usage function
   cline.set_usage(usage);

   cline.add(set_data_files, "-data",   -1);
   cline.add(set_config,     "-config", -1);
   cline.add(set_out,        "-out",     1);

   // Parse command line
   cline.parse();

   // Create default config file name
   default_config_file = replace_path(default_config_filename);

   // List config files
   mlog << Debug(1)
       << "Config File Default: " << default_config_file << "\n"
       << "Config File User: " << config_file << "\n";

   // Read config files
   conf_info.read_config(default_config_file.c_str(),
                      config_file.c_str());

   // Process the configuration
   conf_info.process_config();

   return;
}

////////////////////////////////////////////////////////////////////////

static void set_data_files(const StringArray& a) {
   data_files.add(a);
}

////////////////////////////////////////////////////////////////////////

static void set_config(const StringArray& a) {
   config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

static void set_out(const StringArray& a) {
   out_file = a[0];
}

////////////////////////////////////////////////////////////////////////

static void setup() {
   const char *method_name = "setup() -> ";

   // Open first data file
   mlog << Debug(1) << "Reading dimensions: "
        << data_files[0] << "\n";
   nc_in = open_ncfile(data_files[0].c_str());
   if(!nc_in) {
      mlog << Error << "\n" << method_name
          << "unable to open data file \""
          << data_files[0] << "\"\n\n";
      exit(1);
   }

   // Get dimension sizes
   get_dim(nc_in, "range", n_range, true);
   range_dim = get_nc_dim(nc_in, "range");

   get_dim(nc_in, "azimuth", n_azimuth, true);
   azimuth_dim = get_nc_dim(nc_in, "azimuth");

   if(get_dim(nc_in, "height", n_level)) {
      mlog << Debug(3) << "Found height vertical dimension.\n";
      level_dim = get_nc_dim(nc_in, "height");
      level_name = "height";
   } else if(get_dim(nc_in, "pressure", n_level)) {
      mlog << Debug(3) << "Found pressure vertical dimension.\n";
      level_dim = get_nc_dim(nc_in, "pressure");
      level_name = "pressure";
   } else {
      mlog << Warning << "No vertical dimension found.\n";
   }

   mlog << Debug(2)
       << "Range/Azimuth dimensions (n_level, n_range, n_azimuth) = ("
       << n_level << ", " << n_range << ", " << n_azimuth << ")\n";

   // Get dimension coordinates
   vector<size_t> start;
   vector<size_t> count;
   start.emplace_back(0);

   ConcatString s;

   NcVar range_var = get_nc_var(nc_in, "range");
   count.clear();
   count.emplace_back(n_range);
   range_coord.resize(n_range);
   range_var.getVar(start, count, range_coord.data());

   NcVar azimuth_var = get_nc_var(nc_in, "azimuth");
   count.clear();
   count.emplace_back(n_azimuth);
   azimuth_coord.resize(n_azimuth);
   azimuth_var.getVar(start, count, azimuth_coord.data());
   get_att_value_string(&azimuth_var, "units", s);
   azimuth_units = s.string();

   NcVar level_var = get_nc_var(nc_in, level_name.c_str());
   count.clear();
   count.emplace_back(n_level);
   level_coord.resize(n_level);
   level_var.getVar(start, count, level_coord.data());
   get_att_value_string(&level_var, "units", s);
   level_units = s.string();

   // Read variable information
   for(int i_var = 0; i_var < conf_info.get_n_data(); i_var++) {
      data_names.emplace_back(conf_info.data_info[i_var]->name().string());
      NcVar var = get_nc_var(
         nc_in, conf_info.data_info[i_var]->name().c_str());
      int n_dim = get_dim_count(&var) - 1;
      data_n_dims.emplace_back(n_dim);
      ConcatString s;
      get_att_value_string(&var, "long_name", s);
      data_long_names.emplace_back(s.string());
      get_att_value_string(&var, "units", s);
      data_units.emplace_back(s.string());
   }

   // 2D data cubes
   DataCube zero_2d;
   DataCube max_2d;
   DataCube min_2d;

   zero_2d.set_size(1, n_range, n_azimuth);
   max_2d.set_size(1, n_range, n_azimuth);
   min_2d.set_size(1, n_range, n_azimuth);

   zero_2d.set_constant(0);
   max_2d.set_constant(-1.0e6);
   min_2d.set_constant(1.0e6);

   // 3D data cubes
   DataCube zero_3d;
   DataCube max_3d;
   DataCube min_3d;

   zero_3d.set_size(n_level, n_range, n_azimuth);
   max_3d.set_size(n_level, n_range, n_azimuth);
   min_3d.set_size(n_level, n_range, n_azimuth);

   zero_3d.set_constant(0);
   max_3d.set_constant(-1.0e6);
   min_3d.set_constant(1.0e6);

   // Initialize statistical data cube lists
   for(int i_var = 0; i_var < data_names.size(); i_var++) {

      if(data_n_dims[i_var] == 2) {
         data_counts.emplace_back(zero_2d);
         data_means.emplace_back(zero_2d);
         data_stdevs.emplace_back(zero_2d);
         data_mins.emplace_back(min_2d);
         data_maxs.emplace_back(max_2d);
      }
      if(data_n_dims[i_var] == 3) {
         data_counts.emplace_back(zero_3d);
         data_means.emplace_back(zero_3d);
         data_stdevs.emplace_back(zero_3d);
         data_mins.emplace_back(min_3d);
         data_maxs.emplace_back(max_3d);
      }
   } // end for i_var
}

////////////////////////////////////////////////////////////////////////

static void process_data_files() {
   const char *method_name = "process_data_files() -> ";

   // Size data cubes
   DataCube data_2d;
   DataCube data_2d_sq;
   DataCube data_3d;
   DataCube data_3d_sq;

   data_2d.set_size(1, n_range, n_azimuth);
   data_2d_sq.set_size(1, n_range, n_azimuth);
   data_3d.set_size(n_level, n_range, n_azimuth);
   data_3d_sq.set_size(n_level, n_range, n_azimuth);

   // Set up array track point slices
   vector<size_t> start_2d;
   vector<size_t> count_2d;
   vector<size_t> start_3d;
   vector<size_t> count_3d;
   start_2d.emplace_back(0);
   start_2d.emplace_back(0);
   start_2d.emplace_back(0);
   count_2d.emplace_back(1);
   count_2d.emplace_back(n_range);
   count_2d.emplace_back(n_azimuth);
   start_3d.emplace_back(0);
   start_3d.emplace_back(0);
   start_3d.emplace_back(0);
   start_3d.emplace_back(0);
   count_3d.emplace_back(1);
   count_3d.emplace_back(n_level);
   count_3d.emplace_back(n_range);
   count_3d.emplace_back(n_azimuth);

   // Loop over the input files
   for(int i_file = 0; i_file < data_files.n(); i_file++) {

      // Open current data file
      nc_in = open_ncfile(data_files[i_file].c_str());
      if(!nc_in) {
         mlog << Error << "\n" << method_name
              << "unable to open data file \""
              << data_files[i_file] << "\"\n\n";
         exit(1);
      }

      // Get the track point dimension
      get_dim(nc_in, "track_point", n_track_point, true);

      // Read track information
      TrackInfo cur_track(read_nc_track());

      // The number of track points should match
      if(n_track_point != cur_track.n_points()) {
         mlog << Error << "\n" << method_name
              << "the track dimension (" << n_track_point
              << ") does not match the number of track points ("
              << cur_track.n_points() << ") in file \"" << data_files[i_file]
              << "\"!\n\n";
         exit(1);
      }

      // Determine which track points to use 
      vector<bool> keep_track_point(n_track_point, false);
      int n_keep = 0;

      // Check the entire track
      if(is_keeper_track(cur_track)) {

         // Check each track point
         for(int i_point = 0; i_point < n_track_point; i_point++) {

            if(is_keeper_point(cur_track[i_point])) {

               // Update the keep status
               keep_track_point[i_point] = true;
               n_keep++;

               // Store track point locations
               track_lat.add(cur_track[i_point].lat());
               track_lon.add(cur_track[i_point].lon());
            }
         } // end for i_point
      }

      mlog << Debug(2) << "Processing data for " << n_keep << " of "
           << n_track_point << " track points in: " << data_files[i_file]
           << "\n";

      // Loop over variables to be processed 
      for(int i_var = 0; i_var < data_names.size(); i_var++) {
         NcVar var = get_nc_var(nc_in, data_names[i_var].c_str());

         mlog << Debug(3) << "Processing field: "
              << data_names[i_var] << "\n";

         // Loop over track points
         for(int i_point = 0; i_point < n_track_point; i_point++) {

            // Skip track points filered out
            if(!keep_track_point[i_point]) continue;

            if(data_n_dims[i_var] == 2) {
               mlog << Debug(4) << "Processing 2D " << data_names[i_var]
                    << " for track point " << i_point + 1 << ".\n";
               start_2d[0] = (size_t) i_point;
               var.getVar(start_2d, count_2d, data_2d.data());

               // Update partial sums
               data_2d_sq = data_2d;
               data_2d_sq.square();
               data_counts[i_var].increment();
               data_means[i_var].add_assign(data_2d);
               data_stdevs[i_var].add_assign(data_2d_sq);
               data_mins[i_var].min_assign(data_2d);
               data_maxs[i_var].max_assign(data_2d);
            }
	    else if(data_n_dims[i_var] == 3) {
               mlog << Debug(4) << "Processing 3D " << data_names[i_var]
                    << " for track point " << i_point + 1 << ".\n";
               start_3d[0] = (size_t) i_point;
               var.getVar(start_3d, count_3d, data_3d.data());

               // Update partial sums
               data_3d_sq = data_3d;
               data_3d_sq.square();
               data_counts[i_var].increment();
               data_means[i_var].add_assign(data_3d);
               data_stdevs[i_var].add_assign(data_3d_sq);
               data_mins[i_var].min_assign(data_3d);
               data_maxs[i_var].max_assign(data_3d);
            }
         } // end for i_point
      } // end for i_var
   } // end for i_file

   return;
}

////////////////////////////////////////////////////////////////////////

static void normalize_stats() {

   for(int i_var = 0; i_var < data_names.size(); i_var++) {

      // Normalize
      data_means[i_var].divide_assign(data_counts[i_var]);
      data_stdevs[i_var].divide_assign(data_counts[i_var]);

      // Compute standard deviation
      DataCube data_mean_sq = data_means[i_var];
      data_mean_sq.square();
      data_stdevs[i_var].subtract_assign(data_mean_sq);
      data_stdevs[i_var].square_root();
   } // end for i_var
}

////////////////////////////////////////////////////////////////////////

static void write_stats() {
   const char *method_name = "write_stats() -> ";

   // Check for no track points processed
   if(track_lat.n() == 0) {

      mlog << Warning << "\n" << method_name
           << "No track points retained!\n\n";

      // Reset min and max to bad data
      for(int i_var = 0; i_var < data_names.size(); i_var++) {
         data_mins[i_var].set_constant(bad_data_double);
         data_maxs[i_var].set_constant(bad_data_double);
      }
   }

   // Create output file
   nc_out = open_ncfile(out_file.c_str(), true);

   mlog << Debug(1) << "Writing output file: " << out_file << "\n";

   // Add global attributes
   write_netcdf_global(nc_out, out_file.c_str(), program_name);

   // Define dimensions
   range_dim = add_dim(nc_out, "range", n_range);
   azimuth_dim = add_dim(nc_out, "azimuth", n_azimuth);
   level_dim = add_dim(nc_out, level_name, n_level);

   vector<NcDim> dims_2d;
   dims_2d.emplace_back(range_dim);
   dims_2d.emplace_back(azimuth_dim);

   vector<NcDim> dims_3d;
   dims_3d.emplace_back(level_dim);
   dims_3d.emplace_back(range_dim);
   dims_3d.emplace_back(azimuth_dim);

   // Define variables
   NcVar level_var = nc_out->addVar(level_name, ncDouble, level_dim);
   NcVar range_var = nc_out->addVar("range", ncDouble, range_dim);
   NcVar azimuth_var = nc_out->addVar("azimuth", ncDouble, azimuth_dim);

   vector<size_t> offset;
   vector<size_t> count_range;
   vector<size_t> count_azimuth;
   vector<size_t> count_level;
   offset.emplace_back(0);
   count_level.emplace_back(n_level);
   count_range.emplace_back(n_range);
   count_azimuth.emplace_back(n_azimuth);

   for(int r = 0; r < n_range; r++) range_coord[r] = r;

   range_var.putVar(offset, count_range, range_coord.data());
   add_att(&range_var, "units", "RMW");

   azimuth_var.putVar(offset, count_azimuth, azimuth_coord.data());
   add_att(&azimuth_var, "units", azimuth_units);

   level_var.putVar(offset, count_level, level_coord.data());
   add_att(&level_var, "units", level_units);

   vector<size_t> offset_2d;
   vector<size_t> count_2d;
   vector<size_t> offset_3d;
   vector<size_t> count_3d;

   offset_2d.emplace_back(0);
   offset_2d.emplace_back(0);
   count_2d.emplace_back(n_range);
   count_2d.emplace_back(n_azimuth);

   offset_3d.emplace_back(0);
   offset_3d.emplace_back(0);
   offset_3d.emplace_back(0);
   count_3d.emplace_back(n_level);
   count_3d.emplace_back(n_range);
   count_3d.emplace_back(n_azimuth);

   for(int i_var = 0; i_var < data_names.size(); i_var++) {
      if(data_n_dims[i_var] == 2) {
         NcVar var_mean = nc_out->addVar(
            data_names[i_var] + "_mean",
            ncDouble, dims_2d);
         add_att(&var_mean, "long_name",
            data_long_names[i_var] + " Mean");
         add_att(&var_mean, "units", data_units[i_var]);
         add_att(&var_mean, "_FillValue", bad_data_double);
         var_mean.putVar(offset_2d, count_2d,
            data_means[i_var].data());

         NcVar var_stdev = nc_out->addVar(
            data_names[i_var] + "_stdev",
            ncDouble, dims_2d);
         add_att(&var_stdev, "long_name",
            data_long_names[i_var] + " Standard Deviation");
         add_att(&var_stdev, "units", data_units[i_var]);
         add_att(&var_stdev, "_FillValue", bad_data_double);
         var_stdev.putVar(offset_2d, count_2d,
            data_stdevs[i_var].data());

         NcVar var_min = nc_out->addVar(
            data_names[i_var] + "_min",
            ncDouble, dims_2d);
         add_att(&var_min, "long_name",
            data_long_names[i_var] + " Minimum");
         add_att(&var_min, "units", data_units[i_var]);
         add_att(&var_min, "_FillValue", bad_data_double);
         var_min.putVar(offset_2d, count_2d,
            data_mins[i_var].data());

         NcVar var_max = nc_out->addVar(
            data_names[i_var] + "_max",
            ncDouble, dims_2d);
         add_att(&var_max, "long_name",
            data_long_names[i_var] + " Maximum");
         add_att(&var_max, "units", data_units[i_var]);
         add_att(&var_max, "_FillValue", bad_data_double);
         var_max.putVar(offset_2d, count_2d,
            data_maxs[i_var].data());
      }

      else if(data_n_dims[i_var] == 3) {
         NcVar var_mean = nc_out->addVar(
            data_names[i_var] + "_mean",
            ncDouble, dims_3d);
         add_att(&var_mean, "long_name",
            data_long_names[i_var] + " Mean");
         add_att(&var_mean, "units", data_units[i_var]);
         add_att(&var_mean, "_FillValue", bad_data_double);
         var_mean.putVar(offset_3d, count_3d,
            data_means[i_var].data());

         NcVar var_stdev = nc_out->addVar(
            data_names[i_var] + "_stdev",
            ncDouble, dims_3d);
         add_att(&var_stdev, "long_name",
            data_long_names[i_var] + " Standard Deviation");
         add_att(&var_stdev, "units", data_units[i_var]);
         add_att(&var_stdev, "_FillValue", bad_data_double);
         var_stdev.putVar(offset_3d, count_3d,
            data_stdevs[i_var].data());

         NcVar var_min = nc_out->addVar(
            data_names[i_var] + "_min",
            ncDouble, dims_3d);
         add_att(&var_min, "long_name",
            data_long_names[i_var] + " Minimum");
         add_att(&var_min, "units", data_units[i_var]);
         add_att(&var_min, "_FillValue", bad_data_double);
         var_min.putVar(offset_3d, count_3d,
            data_mins[i_var].data());

         NcVar var_max = nc_out->addVar(
            data_names[i_var] + "_max",
            ncDouble, dims_3d);
         add_att(&var_max, "long_name",
            data_long_names[i_var] + " Maximum");
         add_att(&var_max, "units", data_units[i_var]);
         add_att(&var_max, "_FillValue", bad_data_double);
         var_max.putVar(offset_3d, count_3d,
            data_maxs[i_var].data());
      }
   } // end for i_var

   // Add the number of track points
   NcVar npoints_var = nc_out->addVar("TrackPoint_count", ncInt);
   add_att(&npoints_var, "long_name", "Number of Track Points");
   int n_points = track_lat.n();
   npoints_var.putVar(&n_points);

   // Add the average track point latitude
   NcVar lat_var = nc_out->addVar("TrackLat_mean", ncDouble);
   add_att(&lat_var, "long_name", "Track Point Latitude Mean");
   add_att(&lat_var, "units", "degrees_north");
   add_att(&lat_var, "standard_name", "latitude_track");
   add_att(&lat_var, "_FillValue", bad_data_double);
   double lat_mean = track_lat.mean();
   lat_var.putVar(&lat_mean);

   // Add the average track point latitude
   NcVar lon_var = nc_out->addVar("TrackLon_mean", ncDouble);
   add_att(&lon_var, "long_name", "Track Point Longitude Mean");
   add_att(&lon_var, "units", "degrees_east");
   add_att(&lon_var, "standard_name", "longitude_track");
   add_att(&lon_var, "_FillValue", bad_data_double);
   double lon_mean = track_lon.mean();
   lon_var.putVar(&lon_mean);

   nc_out->close();
}

////////////////////////////////////////////////////////////////////////
//
// Check if the track should be kept by checking items that remain
// constant across all track points:
//    model, storm id, basin, cyclone, storm name, and init time
//
////////////////////////////////////////////////////////////////////////

static bool is_keeper_track(const TrackInfo &t) {
   bool keep = true;

   // Conf: Model
   if(conf_info.Model.n() > 0 &&
      !conf_info.Model.has(t.technique()))
      keep = false;

   // Conf: Storm ID
   else if(conf_info.StormId.n() > 0 &&
           !conf_info.StormId.has(t.storm_id()))
      keep = false;

   // Conf: Basin
   else if(conf_info.Basin.n() > 0 &&
           !conf_info.Basin.has(t.basin()))
      keep = false;

   // Conf: Cyclone
   else if(conf_info.Cyclone.n() > 0 &&
           !conf_info.Cyclone.has(t.cyclone()))
      keep = false;

   // Conf: Storm Name
   else if(conf_info.StormName.n() > 0 &&
           !conf_info.StormName.has(t.storm_name()))
      keep = false;

   // Conf: Initialization time
   else if((conf_info.InitBeg > 0 &&
            conf_info.InitBeg > t.init()) ||
           (conf_info.InitEnd > 0 &&
            conf_info.InitEnd < t.init()) ||
           (conf_info.InitInc.n() > 0 &&
            !conf_info.InitInc.has(t.init())) ||
           (conf_info.InitExc.n() > 0 &&
            conf_info.InitExc.has(t.init())))
      keep = false;

   // Conf: Initialization hour
   else if(conf_info.InitHour.n() > 0 &&
           !conf_info.InitHour.has(t.init_hour()))
      keep = false;

   // Conf: InitMask and InitThreshMap
   else if(conf_info.InitMaskName.nonempty() ||
           !conf_info.InitThreshMap.empty()) {

      // Get the initialization index
      int i_init = t.lead_index(0);

      // Check for bad data
      if(i_init < 0 || i_init > t.n_points()) {
         keep = false;
      }
      else {

         // Conf: InitMask
         if(!check_masks(conf_info.InitPolyMask,
                         conf_info.InitGridMask,
                         conf_info.InitAreaMask,
                         t[i_init].lat(), t[i_init].lon()))
            keep = false;

         // Conf: InitThreshMap
         for(const auto &m : conf_info.InitThreshMap) {

            // Get the value
            double val = t[i_init].get_atcf_val(m.first);

            // Check the threshold
            if(!m.second.check_dbl(val)) {
               keep = false;
               break;
            }
         } 
      }
   }

   // Return the keep status
   return keep;
}

////////////////////////////////////////////////////////////////////////
//
// Check if the point should be kept by checking items that can change
// across track points:
//    model, storm id, basin, cyclone, and initialization time
//
////////////////////////////////////////////////////////////////////////

static bool is_keeper_point(const TrackPoint &p) {
   bool keep = true;

   // Conf: Lead time
   if(conf_info.LeadTime.n() > 0 &&
      !conf_info.LeadTime.has(p.lead()))
      keep = false;

   // Conf: Valid time
   if((conf_info.ValidBeg > 0 &&
       conf_info.ValidBeg > p.valid()) ||
      (conf_info.ValidEnd > 0 &&
       conf_info.ValidEnd < p.valid()) ||
      (conf_info.ValidInc.n() > 0 &&
       !conf_info.ValidInc.has(p.valid())) ||
      (conf_info.ValidExc.n() > 0 &&
       conf_info.ValidExc.has(p.valid())))
      keep = false;

   // Conf: Valid hour
   else if(conf_info.ValidHour.n() > 0 &&
           !conf_info.ValidHour.has(p.valid_hour()))
      keep = false;

   // Conf: ValidMask
   if(!check_masks(conf_info.ValidPolyMask,
                   conf_info.ValidGridMask,
                   conf_info.ValidAreaMask,
                   p.lat(), p.lon()))
      keep = false;

   // Conf: Category (e.g. CycloneLevel) 
   else if(conf_info.Category.n() > 0 &&
           !conf_info.Category.has(cyclonelevel_to_string(p.level())))
      keep = false;

   // Conf: ColumnThreshMap
   for(const auto &m : conf_info.ColumnThreshMap) {

      // Get the value
      double val = p.get_atcf_val(m.first);

      // Check the threshold
      if(!m.second.check_dbl(val)) {
         keep = false;
         break;
      }
   }

   // Return the keep status
   return keep;
}

////////////////////////////////////////////////////////////////////////

static TrackInfo read_nc_track() {

   mlog << Debug(3) << "Temporary track file: "
        << adeck_source << "\n";

   ofstream f;
   f.open(adeck_source.c_str());

   NcDim track_line_dim;
   get_dim(nc_in, "track_line", n_track_line, true);

   mlog << Debug(3) << "Reading " << n_track_line << " track lines.\n";

   NcVar track_lines_var = get_nc_var(nc_in, "TrackLines");

   vector<size_t> counts;
   vector<size_t> offsets;

   for(int i = 0; i < n_track_line; i++) {
      offsets.clear();
      offsets.emplace_back(i);
      counts.clear();
      counts.emplace_back(1);

      char* track_line_str;
      track_lines_var.getVar(offsets, counts, &track_line_str);
      ConcatString track_line(track_line_str);

      mlog << Debug(4) << "[Line " << i+1 << " of " << n_track_line << "] "
           << track_line << "\n";

      f << track_line << "\n";
   }
   f.close();

   return parse_track_file(adeck_source);
}

////////////////////////////////////////////////////////////////////////

static TrackInfo parse_track_file(const ConcatString& filename) {
   const char *method_name = "process_track_file() -> ";

   // Open the file
   LineDataFile f;
   if(!f.open(filename.c_str())) {
      mlog << Error << "\n" << method_name
          << "unable to open track file \""
          << filename << "\"\n\n";
      exit(1);
   }

   // Parse the track lines
   ATCFTrackLine line;
   TrackInfoArray tracks;
   while(f >> line) tracks.add(line, false, false);

   // Close the file
   f.close();

   // Remove the temporary file
   remove(filename.c_str());

   // Should be exactly one track
   if(tracks.n() != 1) {
       mlog << Error << "\n" << method_name
            << "expected exactly one track but found "
            << tracks.n() << "!\n\n";
       exit(1);
   }

   return(tracks[0]);
}

////////////////////////////////////////////////////////////////////////
