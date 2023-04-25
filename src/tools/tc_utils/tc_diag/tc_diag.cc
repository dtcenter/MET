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

#ifdef _OPENMP
  #include "omp.h"
#endif

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
static void get_atcf_files(const StringArray&,
                           const StringArray&,
                           StringArray&,
                           StringArray&);
static void process_track_files(const StringArray&,
                                const StringArray&,
                                TrackInfoArray&);
static void process_track_points(const TrackInfoArray &);
static void process_fields(const TrackInfoArray &,
                           unixtime, int,
                           const string &,
                           const DomainInfo &);
static bool is_keeper(const ATCFLineBase *);

static void set_deck(const StringArray&);
static void set_atcf_source(const StringArray&,
                            StringArray&,
                            StringArray&);
static void set_data(const StringArray&);
static void set_config(const StringArray&);
static void set_outdir(const StringArray&);

static void setup_out_files(const TrackInfoArray &);
static ConcatString get_out_key(const TrackInfo &);
static ConcatString get_tmp_key(const TrackInfo &,
                                const TrackPoint &,
                                const string &);

static ConcatString build_tmp_file_name(const TrackInfo *,
                                        const TrackPoint *,
                                        const string &);
static ConcatString build_out_file_name(const TrackInfo *,
                                        const char *);
static void close_out_files();

static void compute_lat_lon(TcrmwGrid&, double*, double*);

////////////////////////////////////////////////////////////////////////

// JHG, things to do as of 4/14/2023:
// - Change order of dimensions
//   from: double TMP_P500(range, azimuth, time) ;
//     to: double TMP_P500(time, pressure, range, azimuth) ;
//   https://github.com/dtcenter/MET/issues/2168#issuecomment-1347477566
// - Parellelize the processing of valid times
// - Add NetCDF variable attributes
// - Write 3D variables to NetCDF output file
// - Write variable names track_lat/track_lon/grid_lat/grid_lon
// - Call the diagnostic scripts on these temp NC files
// - Stitch together the temp files into an output file
// - Do the ACTUAL list of variables from Robert
// - Fix the azimuth computations (DONE BUT NOT COMMITTED)
// - Check if the azimuths are computed the way Robert says they should be
//
// Add back in the "regrid" dictionary to control how regridding to cyl coord is done?
// Storm motion computation (from 4/7/23):
//   - Include the full track in each temporary NetCDF file
//   - For each track point, write the vmax and mslp as a single value for that time.
//   - Write the full array of (lat, lon) points for the entire track for simplicity.
// JHG print a WARNING message if the Diag Track differs from the Tech Id for the data files
//     AND vortex removal has not been requested

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

   // Close the output files
   close_out_files();

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
        << "\t-data tech_ids domain [ file_1 ... file_n | data_file_list ]\n"
        << "\t-deck file\n"
        << "\t-config file\n"
        << "\t[-outdir path]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-data tech_ids domain [ file_1 ... file_n | data_file_list ]\"\n"

        << "\t\t\tSpecifies a comma-separated list of one or more ATCF tech ID's\n"
        << "\t\t\tand a domain name, followed by the gridded data files\n"
        << "\t\t\tor an ASCII file containing a list of files to be used.\n"
        << "\t\t\tSpecify \"-data\" once for each data source (required).\n"

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
   if(data_opt_map.size() == 0 ||
      deck_source.n()     == 0 ||
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

   // Read config files
   conf_info.read_config(default_config_file.c_str(), config_file.c_str());

   // Get data file type from input files
   set_file_type(data_files);

   // Process the configuration
   conf_info.process_config(file_type, data_opt_map);

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
           << unix_to_yyyymmddhh(init_ta.min()) << " and "
           << unix_to_yyyymmddhh(init_ta.max()) << ".\n\n";
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
       << " != " << unix_to_yyyymmddhh(conf_info.init_inc);
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

   // Check for enough arguments (e.g. -data AVNO,AEMN parent gfs_file_list)
   if(a.n() < 3) {
      mlog << Error << "\nset_data() -> "
           << "each \"-data\" command line option must specify a comma-separated list "
           << "of one or more ATCF tech ID's and a domain name, "
           << "followed by the corresponding data files.\n\n";
      usage();
   }

   // First argument is a comma-separated list of tech ID's
   StringArray tech_id;
   tech_id.parse_css(a[0]);

   // Second argument is the domain name
   string domain = a[1];

   // Remaining arguments are the data files
   StringArray sa;
   for(int i=2; i<a.n(); i++) sa.add(a[i]);

   // Update the data file map
   if(data_opt_map.count(domain) == 0) data_opt_map[domain].data_files = sa;
   else                                data_opt_map[domain].data_files.add(sa);

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
   OutFileInfo out_info;

   // Setup output files for each track
   for(int i=0; i<tracks.n(); i++) {

      // Build the map key
      ConcatString key = get_out_key(tracks[i]);

      // Check for duplicates
      if(out_map.count(key) > 0) {
         mlog << Error << "\nsetup_out_files()-> "
              << "found multiple tracks for key \""
              << key << "\"!\n\n";
         exit(1);
      }

      // Add new map entry
      out_map[key] = out_info;

      mlog << Debug(3) << "Preparing output files for "
           << key << " track.\n";

      // Store the track
      out_map[key].trk_ptr = &tracks[i];

      // NetCDF cylindrical coordinates
      if(conf_info.nc_rng_azi_flag) {

         // One output file per track and domain
         map<std::string,DomainInfo>::iterator it;
         for(it  = conf_info.domain_info_map.begin();
             it != conf_info.domain_info_map.end();
             it++) {

            // Build output file name
            ConcatString suffix_cs, file_name;
            suffix_cs << "_cyl_grid_" << it->first << ".nc";
            file_name = build_out_file_name(out_map[key].trk_ptr,
                                            suffix_cs.c_str());

            out_map[key].nc_rng_azi_file_map[it->first] = file_name;
            out_map[key].nc_rng_azi_out_map[it->first] =
               out_map[key].setup_nc_file(file_name);
         }
      }

      // NetCDF diagnostics output
      if(conf_info.nc_diag_flag) {
         out_map[key].nc_diag_file =
            build_out_file_name(out_map[key].trk_ptr, "_diag.nc");
         out_map[key].nc_diag_out =
            out_map[key].setup_nc_file(out_map[key].nc_diag_file);
      }

      // CIRA diagnostics output
      if(conf_info.cira_diag_flag) {
         out_map[key].cira_diag_file =
            build_out_file_name(out_map[key].trk_ptr, "_diag.txt");
         out_map[key].cira_diag_out = new ofstream;
         out_map[key].cira_diag_out->open(out_map[key].cira_diag_file);

         if(!(*out_map[key].cira_diag_out)) {
            mlog << Error << "\nsetup_out_files()-> "
                 << "can't open the output file \""
                 << out_map[key].cira_diag_file
                 << "\" for writing!\n\n";
            exit(1);
         }

         // Fixed width
         out_map[key].cira_diag_out->setf(ios::fixed);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString get_out_key(const TrackInfo &track) {
   ConcatString cs;

   cs << track.storm_id() << "_"
      << track.technique() << "_"
      << unix_to_yyyymmddhh(track.init());

   return(cs);
}

////////////////////////////////////////////////////////////////////////

ConcatString get_tmp_key(const TrackInfo &track,
                         const TrackPoint &point,
                         const string &domain) {
   ConcatString cs;

   cs << track.storm_id() << "_"
      << track.technique() << "_"
      << unix_to_yyyymmddhh(track.init()) << "_f"
      << point.lead() /sec_per_hour << "_"
      << domain;

   return(cs);
}

////////////////////////////////////////////////////////////////////////

ConcatString build_tmp_file_name(const TrackInfo *trk_ptr,
                                 const TrackPoint *pnt_ptr,
                                 const string &domain) {
   ConcatString cs;

   // Build the temp file name with the program name,
   // track/timing information, and domain name

   cs << conf_info.tmp_dir
      << "/tmp_" << program_name << "_"
      << get_tmp_key(*trk_ptr, *pnt_ptr, domain);

   return(make_temp_file_name(cs.text(), ".nc"));
}

////////////////////////////////////////////////////////////////////////

ConcatString build_out_file_name(const TrackInfo *trk_ptr,
                                 const char *suffix) {
   ConcatString cs;

   // Build the output file name
   cs << out_dir << "/" << program_name;

   // Append the output prefix, if defined
   if(conf_info.output_prefix.nonempty()) {
      cs << "_" << conf_info.output_prefix;
   }

   // Append the track information
   cs << "_" << get_out_key(*trk_ptr);

   // Append the suffix
   cs << suffix;

   return(cs);
}

////////////////////////////////////////////////////////////////////////

void close_out_files() {

   // Write output files for each track
   map<std::string,OutFileInfo>::iterator it;
   for(it = out_map.begin(); it != out_map.end(); it++) {
      it->second.clear();
   }

   return;
}


////////////////////////////////////////////////////////////////////////

void compute_lat_lon(TcrmwGrid& ra_grid,
                     double *lat_arr, double *lon_arr) {

   // Compute lat and lon coordinate arrays
   for(int ir=0; ir<ra_grid.range_n(); ir++) {
      for(int ia=0; ia<ra_grid.azimuth_n(); ia++) {
         double lat, lon;
         int i = ir * ra_grid.azimuth_n() + ia;
         ra_grid.range_azi_to_latlon(
            ir * ra_grid.range_delta_km(),
            ia * ra_grid.azimuth_delta_deg(),
            lat, lon);
         lat_arr[i] =   lat;
         lon_arr[i] = - lon; // degrees east to west
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_track_points(const TrackInfoArray& tracks) {
   int i, j, i_pnt, n_pts;
   TmpFileInfo tmp_info;

   // Build list of unique valid times
   TimeArray valid_ta;
   for(i=0,n_pts=0; i<tracks.n(); i++) {
      n_pts += tracks[i].n_points();
      for(j=0; j<tracks[i].n_points(); j++) {
         if(!valid_ta.has(tracks[i][j].valid())) {
            valid_ta.add(tracks[i][j].valid());
         }
      }
   }

   // Sort the valid times
   valid_ta.sort_array();

   mlog << Debug(2) << "Processing " << tracks.n()
        << " tracks consisting of " << n_pts
        << " points with " << valid_ta.n()
        << " unqiue valid times from "
        << unix_to_yyyymmddhh(valid_ta.min())
        << " to " << unix_to_yyyymmddhh(valid_ta.max())
        << ".\n";
/* JHG
#pragma omp parallel default(none)                      \
   shared(valid_ta, mlog, conf_info, tracks, tmp_map)   \
   shared(Error)                                        \
   private(i, j, i_pnt, tmp_info)
   {
*/
   // Parallel: Loop over the unique valid times
   for(i=0; i<valid_ta.n(); i++) {

      // Parallel: Loop over the domains to be processed
      map<string,DomainInfo>::iterator dom_it;
      for(dom_it  = conf_info.domain_info_map.begin();
          dom_it != conf_info.domain_info_map.end();
          dom_it++) {

         mlog << Debug(3) << "Processing the "
              << dom_it->first << " domain.\n";

         // Setup a temp file for this valid time in each track
         for(j=0; j<tracks.n(); j++) {

            // Find the track point for this valid time
            if((i_pnt = tracks[j].valid_index(valid_ta[i])) < 0) continue;

            // Build the map key
            ConcatString tmp_key = get_tmp_key(tracks[j],
                                               tracks[j][i_pnt],
                                               dom_it->first);

            // Check for duplicates
            if(tmp_map.count(tmp_key) > 0) {
               mlog << Error << "\nprocess_track_points()-> "
                    << "found multiple temp file entries for key \""
                    << tmp_key << "\"!\n\n";
               exit(1);
            }

            // Add new map entry
            tmp_map[tmp_key] = tmp_info;

            // Setup a temp file for the current point
            tmp_map[tmp_key].open(&tracks[j],
                                  &tracks[j][i_pnt],
                                  dom_it->second);

         } // end for j

         // Process the gridded data for the current time
         process_fields(tracks, valid_ta[i], i,
                        dom_it->first, dom_it->second);

      } // end for dom_it
   } // end for i
// JHG } // End of omp parallel

   return;
}

////////////////////////////////////////////////////////////////////////

void process_fields(const TrackInfoArray &tracks,
                    unixtime vld_ut, int i_vld,
                    const string &domain,
                    const DomainInfo &di) {
   int i, j, i_pnt;
   DataPlane data_dp;
   Grid grid;
   VarInfoFactory vi_factory;
   VarInfo *vi = (VarInfo *) 0;
   StringArray tmp_key_sa;

   // Loop over the VarInfo fields to be processed
   for(i=0; i<di.var_info_ptr.size(); i++) {

      // Make a local VarInfo copy to store the valid time
      vi = vi_factory.new_copy(di.var_info_ptr[i]);
      vi->set_valid(vld_ut);

      // Find data for this track point
      get_series_entry(i_vld, vi,
                       di.data_files, file_type,
                       data_dp, grid);

      // Do coordinate transformation for each track point
      for(j=0; j<tracks.n(); j++) {

         // Find the track point for this valid time
         if((i_pnt = tracks[j].valid_index(vld_ut)) < 0) continue;

         // Build the map keys
         ConcatString out_key = get_out_key(tracks[j]);
         ConcatString tmp_key = get_tmp_key(tracks[j],
                                            tracks[j][i_pnt],
                                            domain);

         // Store the temp keys
         if(!tmp_key_sa.has(tmp_key)) tmp_key_sa.add(tmp_key);

         // Compute and write the cylindrical coordinate data
         tmp_map[tmp_key].write_nc_data(vi, data_dp, grid);

      } // end for j

      // Deallocate memory
      delete vi;
      vi = (VarInfo *) 0;

   } // end for i

   // Process the temp files
   for(i=0; i<tmp_key_sa.n(); i++) {

      // Close temp file
      tmp_map[tmp_key_sa[i]].close();

      // JHG TODO run python diagnostic scripts here

      // Delete temp file
      tmp_map[tmp_key_sa[i]].clear();
   }

   return;
}

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

   // Initialize track pointer
   trk_ptr = (TrackInfo *) 0;

   // Initialize output file stream pointers
   nc_diag_out   = (NcFile *) 0;
   cira_diag_out = (ofstream *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void OutFileInfo::clear() {

   trk_ptr = (TrackInfo *) 0;

   // Write NetCDF cylindrical coordinates file
   if(nc_rng_azi_out_map.size() > 0) {

// JHG: note that we will NOT want output files for all domains, only the ones that apply

      map<string,NcFile *>::iterator it;
      for(it  = nc_rng_azi_out_map.begin();
          it != nc_rng_azi_out_map.end();
          it++) {

         mlog << Debug(1) << "Writing output file: "
              << nc_rng_azi_file_map[it->first] << "\n";

         // Close the output file
         it->second->close();
         delete it->second;
         it->second = (NcFile *) 0;

         // Clear the file name
         nc_rng_azi_file_map[it->first].clear();
      }

      // Empty the maps
      nc_rng_azi_file_map.clear();
      nc_rng_azi_out_map.clear();
   }

   // Write NetCDF diagnostics file
   if(nc_diag_out) {

      mlog << Debug(1) << "Writing output file: "
           << nc_diag_file << "\n";

      // Close the output file
      nc_diag_out->close();
      delete nc_diag_out;
      nc_diag_out = (NcFile *) 0;

   }
   nc_diag_file.clear();

   // Write CIRA diagnostics files
   if(cira_diag_out) {

      mlog << Debug(1) << "Writing output file: "
           << cira_diag_file << "\n";

      // Write the output
      *cira_diag_out << cira_diag_at;

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

NcFile *OutFileInfo::setup_nc_file(const string &out_file) {

   if(!trk_ptr) return(nullptr);

   // Open the output NetCDF file
   NcFile *nc_out = open_ncfile(out_file.c_str(), true);

   if(IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nOutFileInfo::setup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_file << "\n\n";
      exit(1);
   }

   // Track and valid time dimensions
   NcDim trk_dim =
      add_dim(nc_out, "track_point", trk_ptr->n_points());
   NcDim vld_dim =
      add_dim(nc_out, "time", trk_ptr->n_points());


   // Write the track
   write_tc_track(nc_out, trk_dim, *trk_ptr);

   return(nc_out);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class TmpFileInfo
//
////////////////////////////////////////////////////////////////////////

TmpFileInfo::TmpFileInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TmpFileInfo::~TmpFileInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void TmpFileInfo::init_from_scratch() {

   // Initialize pointers
   trk_ptr = (TrackInfo *) 0;
   pnt_ptr = (TrackPoint *) 0;
   tmp_out = (NcFile *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TmpFileInfo::open(const TrackInfo *t_ptr,
                       const TrackPoint *p_ptr,
                       const DomainInfo &di) {

   // Set pointers
   trk_ptr = t_ptr;
   pnt_ptr = p_ptr;
   domain    = di.domain;

   // Open the temp file
   tmp_file = build_tmp_file_name(trk_ptr, pnt_ptr, domain);

   mlog << Debug(3) << "Creating temp file: " << tmp_file << "\n";

   setup_nc_file(di);

   return;
}

////////////////////////////////////////////////////////////////////////

void TmpFileInfo::close() {

   // Write NetCDF temp file
   if(tmp_out) {

      mlog << Debug(3) << "Writing temp file: "
           << tmp_file << "\n";

      // Close the output file
      tmp_out->close();
      delete tmp_out;
      tmp_out = (NcFile *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void TmpFileInfo::clear() {

   close();

   trk_ptr = (TrackInfo *) 0;
   pnt_ptr = (TrackPoint *) 0;

   grid.clear();
   ra_grid.clear();

   domain.clear();

   // Delete the temp file
   // JHG if(tmp_file.nonempty()) remove_temp_file(tmp_file);

   tmp_file.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TmpFileInfo::setup_nc_file(const DomainInfo &di) {

   // Open the output NetCDF file
   tmp_out = open_ncfile(tmp_file.c_str(), true);

   if(IS_INVALID_NC_P(tmp_out)) {
      mlog << Error << "\nTmpFileInfo::setup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << tmp_file << "\n\n";
      exit(1);
   }

   // Define latitude and longitude arrays
   TcrmwData d = di.data;
   int nra = d.range_n * d.azimuth_n;
   double *lat_arr = new double[nra];
   double *lon_arr = new double[nra];

   // Set grid center
   d.lat_center   =      pnt_ptr->lat();
   d.lon_center   = -1.0*pnt_ptr->lon(); // degrees east to west
   d.range_max_km = di.delta_range_km * d.range_n;

   // Instantiate the grid
   grid.set(d);
   ra_grid.set_from_data(d);

   mlog << Debug(3) << "Defining cylindrical coordinates for (Lat, Lon) = ("
        << pnt_ptr->lat() << ", " << pnt_ptr->lon()
        << "), Range = " << ra_grid.range_n()
        << ", Azimuth = " << ra_grid.azimuth_n() << "\n";

   // Define dimensions
   rng_dim = add_dim(tmp_out, "range", (long) ra_grid.range_n());
   azi_dim = add_dim(tmp_out, "azimuth", (long) ra_grid.azimuth_n());
   trk_dim = add_dim(tmp_out, "track_point", trk_ptr->n_points());
   vld_dim = add_dim(tmp_out, "time", 1);

   // Define range and azimuth dimensions
   def_tc_range_azimuth(tmp_out, rng_dim, azi_dim,
                        ra_grid, 1.0);

   // Define latitude and longitude arrays
   def_tc_lat_lon_time(tmp_out,
                       rng_dim, azi_dim, vld_dim,
                       lat_var, lon_var, vld_var);

   // Write the track
   write_tc_track(tmp_out, trk_dim, *trk_ptr);

   // Compute lat and lon coordinate arrays
   compute_lat_lon(ra_grid, lat_arr, lon_arr);

   // Write coordinate arrays
   write_tc_data(tmp_out, ra_grid, 0, lat_var, lat_arr);
   write_tc_data(tmp_out, ra_grid, 0, lon_var, lon_arr);

   // Write valid time
   write_tc_valid_time(tmp_out, 0, vld_var, pnt_ptr->valid());

   // Clean up
   if(lat_arr) { delete[] lat_arr; lat_arr = (double *) 0; }
   if(lon_arr) { delete[] lon_arr; lon_arr = (double *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void TmpFileInfo::write_nc_data(const VarInfo *vi, const DataPlane &dp_in,
                                const Grid &grid_in) {
   DataPlane dp_out;
   RegridInfo ri;

   // Use default regridding options
   ri.method     = InterpMthd_Nearest;
   ri.width      = 1;
   ri.vld_thresh = 1.0;
   ri.shape      = GridTemplateFactory::GridTemplate_Square;

   // Do the cylindrical coordinate transformation
   dp_out = met_regrid(dp_in, grid_in, grid, ri);

   // Setup dimensions
   vector<NcDim> dims;
   dims.push_back(rng_dim);
   dims.push_back(azi_dim);
   dims.push_back(vld_dim);

   // Create output variable
   ConcatString var_name;
   var_name << vi->name_attr() << "_" << vi->level_attr();
   NcVar cur_var = tmp_out->addVar(var_name, ncDouble, dims);

   // JHG need to add variable attributes

   // Write the data
   write_tc_data_rev(tmp_out, ra_grid, 0, cur_var, dp_out.data());

   return;
}

////////////////////////////////////////////////////////////////////////
