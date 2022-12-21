// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   tc_diag.cc
//
//   Description:
//
//   Mod#   Date      Name          Description
//   ----   ----      ----          -----------
//   000    09/27/22  Halley Gotway New
//
////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <dirent.h>
#include <fstream>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "main.h"
#include "tc_diag.h"

#include "series_data.h"

#include "vx_grid.h"
#include "vx_regrid.h"
#include "vx_tc_util.h"
#include "vx_nc_util.h"
#include "vx_tc_nc_util.h"
#include "vx_data2d_nc_met.h"
#include "vx_util.h"
#include "vx_log.h"
#include "vx_math.h"

#include "met_file.h"

////////////////////////////////////////////////////////////////////////

static void usage();
static void process_command_line(int, char**);static void set_file_type(const StringArray &);

static void process_diagnostics();
static void process_tracks(TrackInfoArray&);
static void get_atcf_files(const StringArray&, const StringArray&, StringArray&, StringArray&);
static void process_track_files(const StringArray&, const StringArray&, TrackInfoArray&);
static bool is_keeper(const ATCFLineBase *);
static void set_deck(const StringArray&);
static void set_atcf_source(const StringArray&, StringArray&, StringArray&);
static void set_data_files(const StringArray&);
static void set_config(const StringArray&);
static void set_outdir(const StringArray&);
static void setup_grid();
static void setup_nc_file();
static void build_outfile_name(const ConcatString&, const char*, ConcatString&);
static void compute_lat_lon(TcrmwGrid&, double*, double*);
static void process_fields(const TrackInfoArray&);

////////////////////////////////////////////////////////////////////////

// JHG see: https://github.com/dtcenter/MET/issues/2168#issuecomment-1347477566

int met_main(int argc, char *argv[]) {

   // Process command line arguments
   process_command_line(argc, argv);

   // Setup grid
   setup_grid();

   // Setup NetCDF output
   setup_nc_file();

   // Process gridded and track data
   process_diagnostics();

   return(0);
}

////////////////////////////////////////////////////////////////////////

const string get_tool_name() {
   return "tc_diag";
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"
        << "Usage: " << program_name << "\n"
        << "\t-data file_1 ... file_n | data_file_list\n"
        << "\t-deck file\n"
        << "\t-config file\n"
        << "\t[-outdir path]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-data file_1 ... file_n | data_file_list\" "
        << "specifies the gridded data files or an ASCII file "
        << "containing a list of files to be used (required).\n"

        << "\t\t\"-deck source\" is the ATCF format data source "
        << "(required).\n"

        << "\t\t\"-config file\" is a TCDiagConfig file to be used "
        << "(required).\n"

        << "\t\t\"-outdir path\" overrides the default output directory "
        << "(" << out_dir << ") (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n" << flush;

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;
   ConcatString default_config_file;

   // Default output directory
   out_dir = replace_path(default_out_dir);

   // Default output prefix
   out_prefix = replace_path(default_out_prefix);

   // Print usage statement for no arguments
   if(argc <= 1) usage();

   // Parse command line into tokens
   cline.set(argc, argv);

   // Set usage function
   cline.set_usage(usage);

   // Add function calls for arguments
   cline.add(set_data_files, "-data",   -1);
   cline.add(set_deck,       "-deck",   -1);
   cline.add(set_config,     "-config",  1);
   cline.add(set_outdir,     "-outdir",  1);

   // Parse command line
   cline.parse();

   // Check for required arguments
   if(data_files.n()  == 0 ||
      deck_source.n() == 0 ||
      config_file.empty()) {
      mlog << Error << "\nThe \"-data\", \"-deck\", and \"-config\" "
           << "command line arguments are required!\n\n";
      usage();
   }

   // Create default config file name
   default_config_file = replace_path(default_config_filename);

   // List config files
   mlog << Debug(1)
        << "Config File Default: " << default_config_file << "\n"
        << "Config File User: " << config_file << "\n";

   // Parse the data file list
   data_files = parse_file_list(data_files);

   // Read config files
   conf_info.read_config(default_config_file.c_str(), config_file.c_str());

   // Get data file type from input files
   set_file_type(data_files);

   // Process the configuration
   conf_info.process_config(ftype);

   return;
}

////////////////////////////////////////////////////////////////////////

void set_file_type(const StringArray &file_list) {
   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;
   int i;

   // Get data file type from config
   GrdFileType conf_ftype = parse_conf_file_type(conf_info.Conf.lookup_dictionary(conf_key_data));

   // Find the first file that actually exists
   for(i=0; i<file_list.n(); i++) {
      if(file_exists(file_list[i].c_str()) || is_python_grdfiletype(conf_ftype)) break;
   }

   // Check for no valid files
   if(i == file_list.n()) {
      mlog << Error << "\nTrouble reading input data files.\n\n";
      exit(1);
   }

   // Read first valid file
   if(!(mtddf = mtddf_factory.new_met_2d_data_file(file_list[i].c_str(), conf_ftype))) {
       mlog << Error << "\nTrouble reading data file \""
            << file_list[i] << "\"\n\n";
       exit(1);
   }

   // Store the actual file type
   ftype = mtddf->file_type();

   // Clean up
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_diagnostics() {

   // Process the track data
   TrackInfoArray tracks;
   process_tracks(tracks);

   // Process the gridded data
   process_fields(tracks);

   // List the output file
   mlog << Debug(1)
        << "Writing output file: " << out_file << "\n";
}

////////////////////////////////////////////////////////////////////////

void process_tracks(TrackInfoArray& tracks) {
   StringArray files, files_model_suffix;

   // Initialize
   tracks.clear();

   // Get list of track files
   get_atcf_files(deck_source, deck_model_suffix,
                  files, files_model_suffix);

   mlog << Debug(2)
        << "Processing " << files.n() << " track data file(s).\n";

   process_track_files(files, files_model_suffix, tracks);

   write_tc_tracks(nc_out, track_point_dim, tracks);

   return;
}

////////////////////////////////////////////////////////////////////////
// Automated Tropical Cyclone Forecasting System
// https://www.nrlmry.navy.mil/atcf_web/docs/ATCF-FAQ.html

void get_atcf_files(const StringArray& source,
                    const StringArray& model_suffix,
                    StringArray& files,
                    StringArray& files_model_suffix) {

   StringArray cur_source, cur_files;

   if(source.n() != model_suffix.n()) {
      mlog << Error << "\nget_atcf_files() -> "
           << "the source and suffix arrays must be equal length!\n\n";
      exit(1);
   }

   // Initialize
   files.clear();
   files_model_suffix.clear();

   // Build list of files from all sources
   for(int i = 0; i < source.n(); i++) {
      cur_source.clear();
      cur_source.add(source[i]);
      cur_files = get_filenames(cur_source, NULL, atcf_suffix);

      for(int j = 0; j < cur_files.n(); j++) {
         files.add(cur_files[j]);
         files_model_suffix.add(model_suffix[i]);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_track_files(const StringArray& files,
                         const StringArray& model_suffix,
                         TrackInfoArray& tracks) {
   int i, cur_read, cur_add, tot_read, tot_add;
   LineDataFile f;
   ConcatString cs;
   ATCFTrackLine line;

   // Initialize
   tracks.clear();

   // Initialize counts
   tot_read = tot_add = 0;

   // Process input ATCF files
   for(i=0; i<files.n(); i++) {

       mlog << Debug(1) << "Reading track file: " << files[i] << "\n";

       // Open current file
       if(!f.open(files[i].c_str())) {
          mlog << Error << "\nprocess_track_files() -> "
              << "unable to open file \"" << files[i] << "\"\n\n";
          exit(1);
       }

       // Initialize counts
       cur_read = cur_add = 0;

       // Read each line
       while(f >> line) {

          // Increment line counts
          cur_read++;
          tot_read++;

          // Add model suffix, if specified
          if(model_suffix[i].length() > 0) {
             cs << cs_erase << line.technique() << model_suffix[i];
             line.set_technique(cs);
          }

          // Check the keep status
          if(!is_keeper(&line)) continue;

          // Attempt to add current line to TrackInfoArray
          if(tracks.add(line, true, false)) {
             cur_add++;
             tot_add++;
          }
       }

       // Dump out current number of lines
       mlog << Debug(4)
           << "[File " << i + 1 << " of " << files.n()
           << "] Used " << cur_add << " of " << cur_read
           << " lines read from file \n\"" << files[i] << "\"\n";

       // Close current file
       f.close();

   } // End loop over files

   // Check the number of tracks: exit for 0 and warning for > 1
   if(tracks.n() == 0) {
       mlog << Error << "\nprocess_track_files() -> "
           << "no tracks retained! Adjust the configuration file "
           << "filtering options to select a single track.\n\n";
       exit(1);
   }
   // Issue warning if more than one track found
   else if(tracks.n() > 1) {
       mlog << Warning << "\nprocess_track_files() -> "
           << "multiple tracks found (" << tracks.n()
           << ")! Using the first one. Adjust the configuration file "
           << "filtering options to select a single track.\n\n";
       TrackInfo first_track = tracks[i];
       tracks.clear();
       tracks.add(first_track);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Check if the ATCFLineBase should be kept.
//
////////////////////////////////////////////////////////////////////////

bool is_keeper(const ATCFLineBase * line) {
   bool keep = true;
   ConcatString cs;

   // Check model
   if(conf_info.Model.nonempty() &&
     conf_info.Model != line->technique()) {
     cs << "model " << line->technique() << " != " << conf_info.Model;
     keep = false;
   }

   // Check storm id
   else if(conf_info.StormId.nonempty() &&
         conf_info.StormId != line->storm_id()) {
     cs << "storm_id " << line->storm_id() << " != " << conf_info.StormId;
     keep = false;
   }

   // Check basin
   else if(conf_info.Basin.nonempty() &&
         conf_info.Basin != line->basin()) {
     cs << "basin " << line->basin() << " != " << conf_info.Basin;
     keep = false;
   }

   // Check cyclone
   else if(conf_info.Cyclone.nonempty() &&
         conf_info.Cyclone != line->cyclone_number()) {
     cs << "cyclone " << line->cyclone_number() << " != " << conf_info.Cyclone;
     keep = false;
   }

   // Check initialization time
   else if(conf_info.InitInc != (unixtime) 0 &&
         conf_info.InitInc != line->warning_time()) {
     cs << "init_inc " << unix_to_yyyymmddhh(line->warning_time())
       << " != " << unix_to_yyyymmdd_hhmmss(conf_info.InitInc);
     keep = false;
   }

   // Check valid time
   else if((conf_info.ValidBeg > 0 &&
          conf_info.ValidBeg > line->valid())   ||
         (conf_info.ValidEnd > 0 &&
          conf_info.ValidEnd < line->valid())   ||
         (conf_info.ValidInc.n() > 0 &&
         !conf_info.ValidInc.has(line->valid())) ||
         (conf_info.ValidExc.n() > 0 &&
          conf_info.ValidExc.has(line->valid()))) {
      cs << "valid_time " << unix_to_yyyymmddhh(line->valid());
      keep = false;
   }

   // Check valid hour
   else if(conf_info.ValidHour.n() > 0 &&
         !conf_info.ValidHour.has(line->valid_hour())) {
      cs << "valid_hour " << line->valid_hour();
      keep = false;
   }

   // Check lead time
   else if(conf_info.LeadTime.n() > 0 &&
        !conf_info.LeadTime.has(line->lead())){
     cs << "lead_time " << sec_to_hhmmss(line->lead());
     keep = false;
   }

   if(!keep) {
     mlog << Debug(4) << "Skipping track line for " << cs << ":\n"
         << line->get_line() << "\n";
   }

   // Return the keep status
   return(keep);
}

////////////////////////////////////////////////////////////////////////

void set_deck(const StringArray& a) {
   set_atcf_source(a, deck_source, deck_model_suffix);
}

////////////////////////////////////////////////////////////////////////

void set_atcf_source(const StringArray& a,
                     StringArray& source,
                     StringArray& model_suffix) {
   StringArray sa;
   ConcatString cs, suffix;

   // Check for optional suffix sub-argument
   for(int i = 0; i < a.n(); i++) {
      if(a[i] == "suffix") {
         cs = a[i];
         sa = cs.split("=");
         if(sa.n() != 2) {
            mlog << Error << "\nset_atcf_source() -> "
                 << "the model suffix must be specified as "
                 << "\"suffix=string\".\n\n";
         }
         else {
            suffix = sa[1];
         }
      }
   }

   // Parse remaining sources
   for(int i = 0; i < a.n(); i++) {
      if( a[i] == "suffix" ) continue;
      source.add(a[i]);
      model_suffix.add(suffix);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void set_data_files(const StringArray& a) {
   data_files = a;
}

////////////////////////////////////////////////////////////////////////

void set_config(const StringArray& a) {
   config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_outdir(const StringArray& a) {
   out_dir = a[0];
}

////////////////////////////////////////////////////////////////////////

void setup_grid() {

   grid_data.name = "TCDiag";
   grid_data.range_n = conf_info.n_range;
   grid_data.azimuth_n = conf_info.n_azimuth;
   grid_data.range_max_km = conf_info.max_range_km;

   tcrmw_grid.set_from_data(grid_data);
   grid.set(grid_data);

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_nc_file() {
   VarInfo* data_info = (VarInfo*) 0;

   // Create NetCDF file
   nc_out = open_ncfile(out_file.c_str(), true);

   if(IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_file << "\n\n";
      exit(1);
   }

   mlog << Debug(4)
        << "Range = " << tcrmw_grid.range_n()
        << ", Azimuth = " << tcrmw_grid.azimuth_n() << "\n";

   // Define dimensions
   range_dim = add_dim(nc_out, "range", (long) tcrmw_grid.range_n());
   azimuth_dim = add_dim(nc_out, "azimuth", (long) tcrmw_grid.azimuth_n());
   track_point_dim = add_dim(nc_out, "track_point", NC_UNLIMITED);

   // Define range and azimuth dimensions
   def_tc_range_azimuth(nc_out, range_dim, azimuth_dim, tcrmw_grid,
      conf_info.rmw_scale);

   // Define latitude and longitude arrays
   def_tc_lat_lon_time(nc_out, range_dim, azimuth_dim,
      track_point_dim, lat_arr_var, lon_arr_var, valid_time_var);

   // Find all variable levels, long names, and units
   for(int i_var = 0; i_var < conf_info.get_n_data(); i_var++) {
      // Get VarInfo
      data_info = conf_info.data_info[i_var];
      mlog << Debug(4) << "Processing field: " << data_info->magic_str() << "\n";
	   string fname = data_info->name_attr();
      variable_levels[fname].push_back(data_info->level_attr());
      variable_long_names[fname] = data_info->long_name_attr();
      variable_units[fname] = data_info->units_attr();
   }

   // Define pressure levels
   pressure_level_strings = get_pressure_level_strings(variable_levels);
   pressure_levels = get_pressure_levels(pressure_level_strings);
   pressure_level_indices = get_pressure_level_indices(pressure_level_strings, pressure_levels);
   pressure_dim = add_dim(nc_out, "pressure", pressure_levels.size());
   def_tc_pressure(nc_out, pressure_dim, pressure_levels);

   def_tc_variables(nc_out,
       variable_levels, variable_long_names, variable_units,
       range_dim, azimuth_dim, pressure_dim, track_point_dim,
       data_3d_vars);

   return;
}

////////////////////////////////////////////////////////////////////////

void build_outfile_name(unixtime valid_ut, int lead_sec,
                        const char *suffix, ConcatString &str) {

   //
   // Create output file name
   //

   // Append the output directory and program name
   str << cs_erase << out_dir << "/" << program_name;

   // Append the output prefix, if defined
   if(conf_info.output_prefix.nonempty())
      str << "_" << conf_info.output_prefix;

   // Append the timing information
   str << "_"
       << sec_to_hhmmss(lead_sec) << "L_"
       << unix_to_yyyymmdd_hhmmss(valid_ut) << "V";

   // Append the suffix
   str << suffix;

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_lat_lon(TcrmwGrid& tcrmw_grid,
        double *lat_arr, double *lon_arr) {

   // Compute lat and lon coordinate arrays
   for(int ir = 0; ir < tcrmw_grid.range_n(); ir++) {
      for(int ia = 0; ia < tcrmw_grid.azimuth_n(); ia++) {
         double lat, lon;
         int i = ir * tcrmw_grid.azimuth_n() + ia;
         tcrmw_grid.range_azi_to_latlon(
            ir * tcrmw_grid.range_delta_km(),
            ia * tcrmw_grid.azimuth_delta_deg(),
            lat, lon);
         lat_arr[i] = lat;
         lon_arr[i] = - lon;
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_fields(const TrackInfoArray& tracks) {
   VarInfo * data_info = (VarInfo *) 0;
   DataPlane data_dp;

   // Define latitude and longitude arrays
   int nra = tcrmw_grid.range_n() * tcrmw_grid.azimuth_n();
   lat_arr = new double[nra];
   lon_arr = new double[nra];

   // Take only first track
   TrackInfo track = tracks[0];

   mlog << Debug(2) << "Processing 1 track consisting of "
        << track.n_points() << " points.\n";

   mlog << Debug(3) << track.serialize() << "\n";

   // Loop over track points
   for(int i_point=0; i_point<track.n_points(); i_point++) {

      TrackPoint point = track[i_point];
      unixtime valid_time = point.valid();
      long valid_yyyymmddhh = unix_to_long_yyyymmddhh(valid_time);

      mlog << Debug(3) << "[" << i_point+1 << " of "
           << track.n_points()  << "] Processing track point valid at "
           << unix_to_yyyymmdd_hhmmss(valid_time)
           << " with center (lat, lon) = (" << point.lat() << ", "
           << point.lon() << ").\n";

      // Set grid center
      grid_data.lat_center = point.lat();
      grid_data.lon_center = -1.0*point.lon(); // internal sign change

      // RMW is same as mrd()
      grid_data.range_max_km = conf_info.rmw_scale * point.mrd() *
                               tc_km_per_nautical_miles * conf_info.n_range;
      tcrmw_grid.clear();
      tcrmw_grid.set_from_data(grid_data);
      grid.clear();
      grid.set(grid_data);

      // Compute lat and lon coordinate arrays
      compute_lat_lon(tcrmw_grid, lat_arr, lon_arr);

      // Write coordinate arrays
      write_tc_data(nc_out, tcrmw_grid, i_point, lat_arr_var, lat_arr);
      write_tc_data(nc_out, tcrmw_grid, i_point, lon_arr_var, lon_arr);

      // Write valid time
      write_tc_valid_time(nc_out, i_point, valid_time_var, valid_yyyymmddhh);

      for(int i_var=0; i_var<conf_info.get_n_data(); i_var++) {

         // Update the variable info with the valid time of the track point
         data_info = conf_info.data_info[i_var];

	      string sname = data_info->name_attr().string();
	      string slevel = data_info->level_attr().string();

	      data_info->set_valid(valid_time);

         // Find data for this track point
         get_series_entry(i_point, data_info, data_files, ftype, data_dp,
                          latlon_arr);

         // Check data range
         double data_min, data_max;
         data_dp.data_range(data_min, data_max);
         mlog << Debug(4) << "data_min:" << data_min << "\n";
         mlog << Debug(4) << "data_max:" << data_max << "\n";

         // Regrid data
         data_dp = met_regrid(data_dp, latlon_arr, grid, data_info->regrid());
	      data_dp.data_range(data_min, data_max);
         mlog << Debug(4) << "data_min:" << data_min << "\n";
         mlog << Debug(4) << "data_max:" << data_max << "\n";

         // Write data
         if(variable_levels[data_info->name_attr()].size() > 1) {
            write_tc_pressure_level_data(nc_out, tcrmw_grid,
               pressure_level_indices, data_info->level_attr(),
               i_point, data_3d_vars[data_info->name_attr()], data_dp.data());
         }
         else {
            write_tc_data_rev(nc_out, tcrmw_grid, i_point,
               data_3d_vars[data_info->name_attr()], data_dp.data());
         }
      } // end for i_var
   } // end for i_point

   // Clean up
   if(lat_arr) { delete[] lat_arr; lat_arr = (double *) 0; }
   if(lon_arr) { delete[] lon_arr; lon_arr = (double *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////
