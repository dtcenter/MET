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

using namespace std;

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
using namespace netCDF;

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

static void process_tracks(TrackInfoArray&);
static void get_atcf_files(const StringArray&, const StringArray&, StringArray&, StringArray&);
static void process_track_files(const StringArray&, const StringArray&, TrackInfoArray&);
static void process_track_points(const TrackInfoArray &);
static void process_fields(const TrackPoint &);
static bool is_keeper(const ATCFLineBase *);

static void set_deck(const StringArray&);
static void set_atcf_source(const StringArray&, StringArray&, StringArray&);
static void set_data(const StringArray&);
static void set_config(const StringArray&);
static void set_outdir(const StringArray&);

static void setup_out_files(const TrackInfoArray &);
static void setup_nc_file(const TrackInfo &, const char *, NcFile *);
static ConcatString build_tmpfile_name(const TrackInfo &, const char *);
static ConcatString build_outfile_name(const TrackInfo &, const char *);
static void write_out_files();

static void compute_lat_lon(TcrmwGrid&, double*, double*);

////////////////////////////////////////////////////////////////////////

// JHG see: https://github.com/dtcenter/MET/issues/2168#issuecomment-1347477566
// JHG tcrmw output dimensions should be changed:
// FROM:	double TMP(range, azimuth, pressure, track_point) ;
// TO:  	double TMP(track_point, pressure, range, azimuth) ; the last 2 are the "gridded dimensions"
// JHG, parallelize the processing of VALID TIMES. For each one, process all the data for all the track points
// Propose that we have TC-Diag write:
// - 1 ASCII CIRA diag file per track (unless that's turned off)
// - 1 NetCDF diag file per track (unless that's turned off)
// - 1 Gridded NetCDF file per track with the raw and/or cyl coordinate data used to compute diagnostics (unless that's turned off) - Should we combine with above?
//
// JHG note that "regrid" appears in the TC-Diag and TC-RMW config files
// I think it currently only applies to the raw data, meaning we can regrid
// the raw data PRIOR TO the cyl coordinate transformation. Wondering if we
// need options to support the cyl coordinate transformation?

// Storm motion computation (from 4/7/23):
//   - Include the full track in each temporary NetCDF file
//   - For each track point, write the vmax and mslp as a single value for that time.
//   - Write the full array of (lat, lon) points for the entire track for simplicity.

int met_main(int argc, char *argv[]) {

   // Process command line arguments
   process_command_line(argc, argv);

   // Process the track data
   TrackInfoArray tracks;
   process_tracks(tracks);

   // Setup output files for each track
   setup_out_files(tracks);

   // Process the gridded data
   process_track_points(tracks);

   // Write output files
   write_out_files();


/* JHG work here

   // JHG, not yet ready to write the tracks to the output
   // if(nc_out) write_tc_tracks(nc_out, track_point_dim, tracks);

   // Setup NetCDF output
   if(!conf_info.nc_diag_info.all_false()) setup_nc_file(tracks[0]);

   // List the output file
   if(nc_out) {
      mlog << Debug(1) << "Writing output file: "
           << nc_out_file << "\n";
   }
*/

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
        << "\t-data domain [ file_1 ... file_n | data_file_list ]\n"
        << "\t-deck file\n"
        << "\t-config file\n"
        << "\t[-outdir path]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-data domain [ file_1 ... file_n | data_file_list ]\"\n"

        << "\t\t\tSpecifies a domain name followed by the gridded data files\n"
        << "\t\t\tor an ASCII file containing a list of files to be used.\n"
        << "\t\t\tSpecify \"-data\" once for each \"domain\" data source (required).\n"

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
   StringArray data_files;

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
   cline.add(set_data,   "-data",   -1);
   cline.add(set_deck,   "-deck",   -1);
   cline.add(set_config, "-config",  1);
   cline.add(set_outdir, "-outdir",  1);

   // Parse command line
   cline.parse();

   // Check for required arguments
   if(data_files_map.size() == 0 ||
      deck_source.n()       == 0 ||
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

   // Parse the data file lists
   map<string,StringArray>::iterator it;
   for(it = data_files_map.begin(); it != data_files_map.end(); it++) {
      data_files = parse_file_list(it->second);
      data_files_map[it->first] = data_files;
   }

   // Read config files
   conf_info.read_config(default_config_file.c_str(), config_file.c_str());

   // Get data file type from input files
   set_file_type(data_files);

   // Process the configuration
   conf_info.process_config(file_type, data_files_map);

   return;
}

////////////////////////////////////////////////////////////////////////

void set_file_type(const StringArray &file_list) {
   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;
   int i;

   // Get data file type from config
   GrdFileType conf_file_type =
      parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_data));

   // Find the first file that actually exists
   for(i=0; i<file_list.n(); i++) {
      if(file_exists(file_list[i].c_str()) ||
         is_python_grdfiletype(conf_file_type)) break;
   }

   // Check for no valid files
   if(i == file_list.n()) {
      mlog << Error << "\nTrouble reading input data files.\n\n";
      exit(1);
   }

   // Read first valid file
   if(!(mtddf = mtddf_factory.new_met_2d_data_file(file_list[i].c_str(), conf_file_type))) {
       mlog << Error << "\nTrouble reading data file \""
            << file_list[i] << "\"\n\n";
       exit(1);
   }

   // Store the actual file type
   file_type = mtddf->file_type();

   // Clean up
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_tracks(TrackInfoArray& tracks) {
   StringArray files, files_model_suffix;
   TimeArray init_ta;
   int i;

   // Get list of track files
   get_atcf_files(deck_source, deck_model_suffix,
                  files, files_model_suffix);

   mlog << Debug(2) << "Processing " << files.n()
        << " track data file(s).\n";

   process_track_files(files, files_model_suffix, tracks);

   // Get list of unique track initialization times
   for(i=0; i<tracks.n(); i++) {
      if(!init_ta.has(tracks[i].init())) init_ta.add(tracks[i].init());
   }

   // Check for a single track initialization time
   if(init_ta.n() > 1) {
      mlog << Error << "\nprocess_tracks() -> "
           << "set the \"init_inc\" config option to select one of the "
           << init_ta.n() << " track initialization times between "
           << unix_to_yyyymmdd_hhmmss(init_ta.min()) << " and "
           << unix_to_yyyymmdd_hhmmss(init_ta.max()) << ".\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Automated Tropical cyclone Forecasting System
// https://www.nrlmry.navy.mil/atcf_web/docs/ATCF-FAQ.html
//
////////////////////////////////////////////////////////////////////////

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

   // Check for no matching tracks
   if(tracks.n() == 0) {
       mlog << Error << "\nprocess_track_files() -> "
           << "no tracks retained! Adjust the config file "
           << "filtering options to select a single track.\n\n";
       exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Check if the ATCFLineBase should be kept
//
////////////////////////////////////////////////////////////////////////

bool is_keeper(const ATCFLineBase * line) {
   bool keep = true;
   ConcatString cs;

   // Check model
   if(conf_info.model.n() > 0 &&
      !conf_info.model.has(line->technique())) {
     cs << "model " << line->technique() << " not in " << write_css(conf_info.model);
     keep = false;
   }

   // Check storm id
   else if(conf_info.storm_id.nonempty() &&
         conf_info.storm_id != line->storm_id()) {
     cs << "storm_id " << line->storm_id() << " != " << conf_info.storm_id;
     keep = false;
   }

   // Check basin
   else if(conf_info.basin.nonempty() &&
           conf_info.basin != line->basin()) {
     cs << "basin " << line->basin() << " != " << conf_info.basin;
     keep = false;
   }

   // Check cyclone
   else if(conf_info.cyclone.nonempty() &&
         conf_info.cyclone != line->cyclone_number()) {
     cs << "cyclone " << line->cyclone_number() << " != " << conf_info.cyclone;
     keep = false;
   }

   // Check initialization time
   else if(conf_info.init_inc != (unixtime) 0 &&
         conf_info.init_inc != line->warning_time()) {
     cs << "init_inc " << unix_to_yyyymmddhh(line->warning_time())
       << " != " << unix_to_yyyymmdd_hhmmss(conf_info.init_inc);
     keep = false;
   }

   // Check valid time
   else if((conf_info.valid_beg > 0 &&
          conf_info.valid_beg > line->valid())   ||
         (conf_info.valid_end > 0 &&
          conf_info.valid_end < line->valid())   ||
         (conf_info.valid_inc.n() > 0 &&
         !conf_info.valid_inc.has(line->valid())) ||
         (conf_info.valid_exc.n() > 0 &&
          conf_info.valid_exc.has(line->valid()))) {
      cs << "valid_time " << unix_to_yyyymmddhh(line->valid());
      keep = false;
   }

   // Check valid hour
   else if(conf_info.valid_hour.n() > 0 &&
         !conf_info.valid_hour.has(line->valid_hour())) {
      cs << "valid_hour " << line->valid_hour();
      keep = false;
   }

   // Check lead time
   else if(conf_info.lead_time.n() > 0 &&
        !conf_info.lead_time.has(line->lead())){
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

void set_data(const StringArray& a) {

   // Check for enough arguments
   if(a.n() < 2) {
      mlog << Error << "\nset_data() -> "
           << "each \"-data\" command line option must specify a domain name "
           << "followed by the corresponding data files.\n\n";
      usage();
   }

   // First argument is the domain name
   string domain = a[0];

   // Remaining arguments are the data files
   StringArray sa;
   for(int i=1; i<a.n(); i++) sa.add(a[i]);

   // Update the data file map
   if(data_files_map.count(domain) == 0) data_files_map[domain] = sa;
   else                                  data_files_map[domain].add(sa);

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

void setup_out_files(const TrackInfoArray &tracks) {
   OutFileInfo info;

   // Write output files separately for each track
   for(int i=0; i<tracks.n(); i++) {

      // Initialize
      info.clear();

      // Store pointer to the TrackInfo
      info.track = tracks[i];

      // NetCDF cylindrical coordinates
      if(conf_info.nc_cyl_coord_flag) {
         info.nc_cyl_coord_file = build_outfile_name(info.track, "_cyl.nc");
         setup_nc_file(info.track, info.nc_cyl_coord_file.c_str(), info.nc_cyl_coord_out);
      }

      // NetCDF diagnostics output
      if(conf_info.nc_diag_flag) {
         info.nc_diag_file = build_outfile_name(info.track, "_diag.nc");
         setup_nc_file(info.track, info.nc_diag_file.c_str(), info.nc_diag_out);
      }

      // CIRA diagnostics output
      if(conf_info.cira_diag_flag) {
         info.cira_diag_file = build_outfile_name(info.track, "_diag.txt");
         info.cira_diag_out = new ofstream;
         info.cira_diag_out->open(info.cira_diag_file);

         if(!(*info.cira_diag_out)) {
            mlog << Error << "\nsetup_out_files()-> "
                 << "can't open the output file \"" << info.cira_diag_file
                 << "\" for writing!\n\n";
            exit(1);
         }

         // Fixed width
         info.cira_diag_out->setf(ios::fixed);

         // List the output file
         mlog << Debug(1) << "Writing output file: " << info.cira_diag_file << "\n";

      }

      // Store out file info for each track
      out_info.push_back(info);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_nc_file(const TrackInfo &track, const char *out_file, NcFile *nc_out) {

   // Open the output NetCDF file
   nc_out = open_ncfile(out_file, true);

   if(IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_file << "\n\n";
      exit(1);
   }

   // List the output file
   mlog << Debug(1) << "Writing output file: " << out_file << "\n";

   // Track point dimension
   NcDim track_point_dim = add_dim(nc_out, "track_point", track.n_points());

   // Write the track
   write_tc_track(nc_out, track_point_dim, track);

   return;
}

/* JHG work here to write (lat, lon) of each track point
   What about also writing all the ATCF lines?

   VarInfo* var_info = (VarInfo*) 0;

   // Build the output file name
   nc_out_file = build_outfile_name(track, out_dir, fcst_hour, domain, ".nc");

   // Create NetCDF file
   nc_out = open_ncfile(nc_out_file.c_str(), true);

   if(IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << nc_out_file << "\n\n";
      exit(1);
   }

   // Track point dimension
   track_point_dim = add_dim(nc_out, "track_point", NC_UNLIMITED);

   // Loop over the domain definitions
   map<std::string,TCDiagDomainInfo>::iterator it;
   for(it  = conf_info.domain_info_map.begin();
       it != conf_info.domain_info_map.end();
       it++) {

      TCDiagDomainInfo *di = &(it->second);

      mlog << Debug(4) << "Writing cylindrical coordinates grid for domain \""
           << it->first << "\" with range = " << di->data.range_n
           << " and azimuth = " << di->data.azimuth_n << ".\n";

      // Define dimension names
      ConcatString rng_cs("range");
      ConcatString azi_cs("azimuth");
      if(conf_info.domain_info_map.size() > 1) {
         rng_cs << "_" << it->first;
         azi_cs << "_" << it->first;
      }

      // Define dimensions
      di->range_dim   = add_dim(nc_out, rng_cs.c_str(), (long) di->data.range_n);
      di->azimuth_dim = add_dim(nc_out, azi_cs.c_str(), (long) di->data.azimuth_n);
   }

   return;
}
*/
/* JHG, gotta do this stuff later
   // Define range and azimuth dimensions
   def_tc_range_azimuth(nc_out, range_dim, azimuth_dim, tcrmw_grid,
      conf_info.rmw_scale);

   // Define latitude and longitude arrays
   def_tc_lat_lon_time(nc_out, range_dim, azimuth_dim,
      track_point_dim, lat_arr_var, lon_arr_var, valid_time_var);

   // Find all variable levels, long names, and units
   for(int i_var=0; i_var<conf_info.get_n_data(); i_var++) {

      // Get VarInfo
      var_info = conf_info.data_opt[i_var].var_info;
      mlog << Debug(4) << "Processing field: " << var_info->magic_str() << "\n";
      string fname = var_info->name_attr();
      variable_levels[fname].push_back(var_info->level_attr());
      variable_long_names[fname] = var_info->long_name_attr();
      variable_units[fname] = var_info->units_attr();
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
*/

////////////////////////////////////////////////////////////////////////

ConcatString build_tmpfile_name(const TrackInfo &track,
                                const TrackPoint &point,
                                const char *domain) {
   ConcatString cs;

   // Build the temp file name with the program name,
   // track/timing information, and domain name

   cs << conf_info.tmp_dir
      << "/tmp_" << program_name
      << "_"     << track.storm_id()
      << "_"     << track.technique()
      << "_"     << unix_to_yyyymmddhh(track.init())
      << "_f"    << point.lead() / sec_per_hour
      << "_"     << domain;

   return(make_temp_file_name(cs.text(), ".nc"));
}

////////////////////////////////////////////////////////////////////////

ConcatString build_outfile_name(const TrackInfo &track,
                                const char *suffix) {
   ConcatString cs;

   // Build the output file name
   cs << out_dir << "/" << program_name;

   // Append the output prefix, if defined
   if(conf_info.output_prefix.nonempty())
      cs << "_" << conf_info.output_prefix;

   // Append the storm ID, model, and initialization time
   cs << "_" << track.storm_id()
      << "_" << track.technique()
      << "_" << unix_to_yyyymmddhh(track.init());

   // Append the suffix
   cs << suffix;

   return(cs);
}

////////////////////////////////////////////////////////////////////////

void write_out_files() {

   for(int i=0; i<out_info.size(); i++) out_info[i].clear();

   return;
}


////////////////////////////////////////////////////////////////////////

void compute_lat_lon(TcrmwGrid& tcrmw_grid,
                     double *lat_arr, double *lon_arr) {

   // Compute lat and lon coordinate arrays
   for(int ir=0; ir<tcrmw_grid.range_n(); ir++) {
      for(int ia=0; ia<tcrmw_grid.azimuth_n(); ia++) {
         double lat, lon;
         int i = ir * tcrmw_grid.azimuth_n() + ia;
         tcrmw_grid.range_azi_to_latlon(
            ir * tcrmw_grid.range_delta_km(),
            ia * tcrmw_grid.azimuth_delta_deg(),
            lat, lon);
         lat_arr[i] =   lat;
         lon_arr[i] = - lon; // degrees east to west
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_track_points(const TrackInfoArray& tracks) {
   int i, j, n_pts;

   // Build list of unique valid times
   TimeArray valid_ta;
   for(i=0,n_pts=0; i<tracks.n(); i++) {
      n_pts += tracks[i].n_points();
      for(j=0; j<tracks[i].n_points(); j++) {
         if(!valid_ta.has(tracks[i][j].valid())) valid_ta.add(tracks[i][j].valid());
      }
   }

   // Sort the valid times
   valid_ta.sort_array();

   mlog << Debug(2) << "Processing " << tracks.n() << " tracks consisting of "
        << n_pts << " points over " << valid_ta.n() << " valid times.\n";

   // Parallel: Loop over the unique valid times
   for(i=0; i<valid_ta.n(); i++) {

      mlog << Debug(3) << "Processing track points for "
           << unix_to_yyyymmdd_hhmmss(valid_ta[i]) << ".\n";

      // Read ALL the gridded data for this valid time for ALL domains
      // JHG process_fields(valid_ta[i], tracks);
      // store it as a map of domain to DataPlaneArray?

      // Parallel: Process the current valid time for all the tracks
      for(j=0; j<tracks.n(); j++) {

         // Parallel: Loop over domain info, do coordinate transformation, and run diagnostic scripts



         // JHG work here
         // compute_diagnostics(...);
         mlog << Debug(3) << "JHG track " << j << " of " << tracks.n() << "\n";

      } // end for j
   } // end for i
/*

Need a data structure to map TrackInfo objects to data sources:

struct TrackDataInfo
{
   TrackInfo *track_ptr;
   vector<string> domains;
   vector<StringArray *> data_files;
};

// Array of structs that
vector<TrackDataInfo> track_data_info;

John Halley Gotway, 2 min
I've been thinking more about this and realize that the logic gets tricky pretty quickly. For 5 active storms, we'd have 5 nests and 5 tracks to process. But there's currently no logic that'd connect "domain = d01" with a particular storm_id... so no obvious way of connecting the model data to the track to which it corresponds.

John Halley Gotway, Now
I could either require that the domain name be set to the storm id (or something that connects it to the track data)... or I could do it automatically, making sure that the track point being processed actually falls INSIDE the nest. Only drawback is whether it's conceivable for the track points of 2 storms to exist inside the same nest.
*/

/* JHG
   // Loop over track points
   for(int i=0; i<track.n_points(); i++) {

      mlog << Debug(3) << "[" << i+1 << " of "
           << track.n_points()  << "] Processing track point valid at "
           << unix_to_yyyymmdd_hhmmss(track[i].valid())
           << " with center (lat, lon) = (" << track[i].lat() << ", "
           << track[i].lon() << ").\n";

      // Process the fields for this track point
      process_fields(track[i]);

   } // end for i

   // JHG here's where we'd make calls to python scripts to compute diagnostics!
*/
   return;
}

////////////////////////////////////////////////////////////////////////

void process_fields(const TrackPoint& point) {
   DataPlane data_dp;
/* JHG this has moved up
   // Loop over the fields to be processed
   for(int i_var=0; i_var<conf_info.domain_info_map.size(); i_var++) {

      // Update the variable info with the valid time of the track point
      VarInfo *var_info = conf_info.data_opt[i_var].var_info;

      // Store pointer to the grid info
      TCRMWGridInfo *gi = conf_info.data_opt[i_var].grid_info;

      // Define latitude and longitude arrays
      int nra = gi->data.range_n * gi->data.azimuth_n;
      double *lat_arr = new double[nra];
      double *lon_arr = new double[nra];

      // Set grid center
      gi->data.lat_center =      point.lat();
      gi->data.lon_center = -1.0*point.lon(); // degrees east to west

      // RMW is same as mrd()
      gi->data.range_max_km = gi->rmw_scale * point.mrd() *
                              tc_km_per_nautical_miles * gi->data.range_n;

      // Instantiate the grid
      TcrmwGrid tcrmw_grid(gi->data);

      // Compute lat and lon coordinate arrays
      compute_lat_lon(tcrmw_grid, lat_arr, lon_arr);

      // JHG, write to nc?

      // Clean up
      if(lat_arr) { delete[] lat_arr; lat_arr = (double *) 0; }
      if(lon_arr) { delete[] lon_arr; lon_arr = (double *) 0; }

   } // end for i_var
*/
   return;
}
/* JHG keep working here
      //
      //





      // Write NetCDF output
      if(nc_out) {

         // Write coordinate arrays
         write_tc_data(nc_out, tcrmw_grid, i_point, lat_arr_var, lat_arr);
         write_tc_data(nc_out, tcrmw_grid, i_point, lon_arr_var, lon_arr);

         // Write valid time
         write_tc_valid_time(nc_out, i_point, valid_time_var, valid_yyyymmddhh);
      }

      for(int i_var=0; i_var<conf_info.get_n_data(); i_var++) {

         // Update the variable info with the valid time of the track point
         var_info = conf_info.data_opt[i_var].var_info;

         var_info->set_valid(valid_time);

         // Find data for this track point
         get_series_entry(i_point, var_info, data_files, file_type,
                          data_dp, input_grid);

         // Check data range
         double data_min, data_max;
         data_dp.data_range(data_min, data_max);
         mlog << Debug(4) << "data_min:" << data_min << "\n";
         mlog << Debug(4) << "data_max:" << data_max << "\n";

         // Regrid data
         data_dp = met_regrid(data_dp, input_grid, output_grid, var_info->regrid());
         data_dp.data_range(data_min, data_max);
         mlog << Debug(4) << "data_min:" << data_min << "\n";
         mlog << Debug(4) << "data_max:" << data_max << "\n";

         // Write NetCDF output
         if(nc_out) {

            if(variable_levels[var_info->name_attr()].size() > 1) {
               write_tc_pressure_level_data(nc_out, tcrmw_grid,
                  pressure_level_indices, var_info->level_attr(),
                  i_point, data_3d_vars[var_info->name_attr()], data_dp.data());
            }
            else {
               write_tc_data_rev(nc_out, tcrmw_grid, i_point,
                  data_3d_vars[var_info->name_attr()], data_dp.data());
            }
         }
      } // end for i_var


*/

////////////////////////////////////////////////////////////////////////
//
//  Code for class OutFileInfo
//
////////////////////////////////////////////////////////////////////////

OutFileInfo::OutFileInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

OutFileInfo::~OutFileInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void OutFileInfo::init_from_scratch() {

   // Initialize output file stream pointers
   nc_cyl_coord_out = (NcFile *) 0;
   nc_diag_out      = (NcFile *) 0;
   cira_diag_out    = (ofstream *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void OutFileInfo::clear() {

   track.clear();

   // Write NetCDF cylindrical coordinates file
   if(nc_cyl_coord_out) {

      // Close the output file
      nc_cyl_coord_out->close();
      delete nc_cyl_coord_out;
      nc_cyl_coord_out = (NcFile *) 0;
   }
   nc_cyl_coord_file.clear();

   // Write NetCDF diagnostics file
   if(nc_diag_out) {

      // Close the output file
      nc_diag_out->close();
      delete nc_diag_out;
      nc_diag_out = (NcFile *) 0;

   }
   nc_diag_file.clear();

   // Write CIRA diagnostics files
   if(cira_diag_out) {

      // Write the output
      // JHG segfault *cira_diag_out << cira_diag_at;

      // Close the output file
      cira_diag_out->close();
      delete cira_diag_out;
      cira_diag_out = (ofstream *) 0;
   }
   cira_diag_file.clear();
   cira_diag_at.clear();

   return;
}

////////////////////////////////////////////////////////////////////////