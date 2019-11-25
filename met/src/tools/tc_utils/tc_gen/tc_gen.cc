// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   tc_gen.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    05/17/19  Halley Gotway   New
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
#include <map>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "tc_gen.h"

#include "vx_statistics.h"
#include "vx_tc_util.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_log.h"

#include "met_file.h"

////////////////////////////////////////////////////////////////////////

static void   process_command_line (int, char **);
static void   process_genesis      ();
static void   get_atcf_files       (const StringArray &,
                                    const StringArray &,
                                    const char *,
                                    StringArray &, StringArray &);
static void   process_track_files  (const StringArray &,
                                    const StringArray &,
                                    GenesisInfoArray &, bool);
static void   process_genesis_pair (const ConcatString &,
                                    const GenesisInfoArray &,
                                    const GenesisInfoArray &,
                                    const GenesisInfoArray &);
static void   setup_table          (AsciiTable &);
static void   usage                ();
static void   set_genesis          (const StringArray &);
static void   set_track            (const StringArray &);
static void   set_config           (const StringArray &);
static void   set_out              (const StringArray &);
static void   set_logfile          (const StringArray &);
static void   set_verbosity        (const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Identify and process genesis events and write output
   process_genesis();

   // List the output files
   for(int i=0; i<out_files.n(); i++) {
      mlog << Debug(1)
           << "Output file: " << out_files[i] << "\n";
   }

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;
   ConcatString default_config_file;
   int i;

   // Default output file
   out_base = "./tc_gen";

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add function calls for the arguments
   cline.add(set_genesis,   "-genesis", -1);
   cline.add(set_track,     "-track",   -1);
   cline.add(set_config,    "-config",   1);
   cline.add(set_out,       "-out",      1);
   cline.add(set_logfile,   "-log",      1);
   cline.add(set_verbosity, "-v",        1);

   // Parse the command line
   cline.parse();

   // Add empty suffix strings, as needed
   for(i=genesis_model_suffix.n(); i<genesis_source.n(); i++) {
      genesis_model_suffix.add("");
   }
   for(i=track_model_suffix.n(); i<track_source.n(); i++) {
      track_model_suffix.add("");
   }

   // Check for the minimum number of arguments
   if(genesis_source.n() == 0 ||
      track_source.n()   == 0 ||
      config_file.length()        == 0) {
      mlog << Error
           << "\nprocess_command_line(int argc, char **argv) -> "
           << "the \"-genesis\", \"-track\", and \"-config\" command "
           << "line options are required\n\n";
      usage();
   }

   // List the input genesis track files
   for(i=0; i<genesis_source.n(); i++) {
      mlog << Debug(1)
           << "[Source " << i+1 << " of " << genesis_source.n()
           << "] Genesis Source: " << genesis_source[i]
           << ", Model Suffix: " << genesis_model_suffix[i] << "\n";
   }

   // List the input track track files
   for(i=0; i<track_source.n(); i++) {
      mlog << Debug(1)
           << "[Source " << i+1 << " of " << track_source.n()
           << "] Track Source: " << track_source[i]
           << ", Model Suffix: " << track_model_suffix[i] << "\n";
   }

   // Create the default config file name
   default_config_file = replace_path(default_config_filename);

   // List the config files
   mlog << Debug(1)
        << "Config File Default: " << default_config_file << "\n"
        << "Config File User: " << config_file << "\n";

   // Read the config files
   conf_info.read_config(default_config_file.c_str(),
                         config_file.c_str());

   return;
}

////////////////////////////////////////////////////////////////////////

void process_genesis() {
   int i, j;
   StringArray genesis_files, genesis_files_model_suffix;
   StringArray track_files, track_files_model_suffix;
   GenesisInfoArray fcst_ga, anly_ga, cur_ga, best_ga, oper_ga;
   map<ConcatString,GenesisInfoArray> fcst_ga_map;
   map<ConcatString,GenesisInfoArray>::iterator it;
   ConcatString atcf_id;

   // Get the list of genesis track files
   get_atcf_files(genesis_source, genesis_model_suffix, atcf_gen_reg_exp,
                  genesis_files,  genesis_files_model_suffix);

   mlog << Debug(2)
        << "Processing " << genesis_files.n() << " genesis track files.\n";
   process_track_files(genesis_files, genesis_files_model_suffix,
                       fcst_ga, false);

   // Get the list of verifing track files
   get_atcf_files(track_source, track_model_suffix, atcf_reg_exp,
                  track_files,  track_files_model_suffix);

   mlog << Debug(2)
        << "Processing " << track_files.n() << " verifying track files.\n";
   process_track_files(track_files, track_files_model_suffix,
                       anly_ga, true);

   // Process each verification filter
   for(i=0; i<conf_info.n_vx(); i++) {

      // Initialize
      fcst_ga_map.clear();

      // Subset the forecast genesis events
      for(j=0; j<fcst_ga.n(); j++) {

         // Check filters
         if(conf_info.VxOpt[i].is_keeper(fcst_ga[j])) {

            // Store the current forecast ATCF ID
            atcf_id = fcst_ga[j].technique();

            // Check specified forecast models
            if(conf_info.VxOpt[i].Model.n() == 0 ||
               conf_info.VxOpt[i].Model.has(atcf_id)) {

               // Add a new map entry, if necessary
               if(fcst_ga_map.count(atcf_id) == 0) {
                  cur_ga.clear();
                  fcst_ga_map[atcf_id] = cur_ga;
               }

               // Store the current object
               fcst_ga_map[atcf_id].add(fcst_ga[j]);
            }
         }
      } // end j

      // Subset the BEST and operational genesis events
      best_ga.clear();
      oper_ga.clear();
      for(j=0; j<anly_ga.n(); j++) {

         // TODO: Consider making the is_keeper logic different for analysis tracks

         // Check filters
         if(conf_info.VxOpt[i].is_keeper(anly_ga[j])) {

            if(anly_ga[j].technique() == conf_info.BestEventInfo.Technique) {
                best_ga.add(anly_ga[j]);
            }
            else if(anly_ga[j].technique() == conf_info.OperEventInfo.Technique) {
                oper_ga.add(anly_ga[j]);
            }
         }
      } // end j

      // Loop through and process the genesis event pairs
      for(j=0,it=fcst_ga_map.begin(); it!=fcst_ga_map.end(); it++,j++) {
         mlog << Debug(2)
              << "[Filter " << i+1 << ", Model " << j+1 << "] "
              << "For " << it->first << ", comparing " << it->second.n()
              << " genesis forecasts to " << best_ga.n() << " "
              << conf_info.BestEventInfo.Technique << " and "
              << oper_ga.n() << " " << conf_info.OperEventInfo.Technique
              << " genesis events.\n";
         process_genesis_pair(it->first, it->second, best_ga, oper_ga);
      }

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Logic for counting hits, misses, and false alarms for TC genesis
// events:
//
// (1) For each genesis forecast find all BEST tracks genesis events
//     where:
//     (a) The timestamp falls within the search window centered on the
//         forecast genesis time.
//     (b) The location falls within the search radius centered on the
//         forecast genesis location.
//     (c) For multiple BEST track matches, choose the one closest in
//         space to the forecast genesis event.
//
// If the initialization time is within the window defined by the
// BEST track genesis time [t - lead_window.beg, t - lead_window.end]
// then we have a "hit."
//
// If the initialization time is > (t - lead_window.end) hours before
// the best-track genesis time, then we have a false alarm.
//
// If the initialization time is after the best-track genesis time, we
// discard that forecast.
//
// If there are no matches in the best-tracks, conduct the same steps as
// above, but instead check for operational baseline matches.
//
////////////////////////////////////////////////////////////////////////

void process_genesis_pair(const ConcatString &model,
                          const GenesisInfoArray &fga,
                          const GenesisInfoArray &bga,
                          const GenesisInfoArray &oga) {
   int i, j;
   unixtime ut, init_beg, init_end;
   map<ConcatString,CTSInfo> cts_info_map;

   // Loop over the best track genesis events to define the cases
   for(i=0; i<bga.n(); i++) {

      init_beg = bga[i].genesis_time() - conf_info.LeadSecBeg;
      init_end = bga[i].genesis_time() - conf_info.LeadSecBeg;

      // Loop over

      // DEFINE CASES as...
      // For each BEST track genesis event, define the init times that
      // the model could have predicted the genesis




   // JHG work here
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// TODO: This logic isn't sufficient for finding files with "atcf_gen"
//       embedded in the middle of the filename.
//
////////////////////////////////////////////////////////////////////////

void get_atcf_files(const StringArray &source,
                    const StringArray &model_suffix,
                    const char *reg_exp,
                    StringArray &files,
                    StringArray &files_model_suffix) {
   StringArray cur_source, cur_files;
   int i, j;

   // Initialize
   files.clear();
   files_model_suffix.clear();

   // Build list of files and corresponding model suffix list
   for(i=0; i<source.n(); i++) {
      cur_source.clear();
      cur_source.add(source[i]);
      cur_files = get_filenames(cur_source, NULL, reg_exp);

      for(j=0; j<cur_files.n(); j++) {
         files.add(cur_files[j]);
         files_model_suffix.add(model_suffix[i]);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_track_files(const StringArray &files,
                         const StringArray &model_suffix,
                         GenesisInfoArray &genesis, bool is_anly) {
   int i, j, k;
   int n_lines, tot_lines, tot_tracks;
   ConcatString cs;
   LineDataFile f;
   ATCFTrackLine line;
   TrackInfoArray tracks;
   bool keep;

   // Initialize
   genesis.clear();
   tot_lines = tot_tracks = 0;

   // Process each of the input ATCF files
   for(i=0; i<files.n(); i++) {

      // Open the current file
      if(!f.open(files[i].c_str())) {
         mlog << Error
              << "\nprocess_track_files() -> "
              << "unable to open file \"" << files[i] << "\"\n\n";
         exit(1);
      }

      // Initialize
      tracks.clear();
      n_lines = 0;

      // Read each input line. Track lines may be interleaved
      // within a file but cannot span multiple files.
      while(f >> line) {

         // Increment the line counts
         n_lines++;

         // Add model suffix, if specified
         if(model_suffix[i].length() > 0) {
            cs << cs_erase << line.technique() << model_suffix[i];
            line.set_technique(cs);
         }

         // Only process the specified BEST and operational track
         if(is_anly &&
            line.technique() != conf_info.BestEventInfo.Technique &&
            line.technique() != conf_info.OperEventInfo.Technique) {
            continue;
         }

         // Add the current line checking for analysis tracks
         tracks.add(line, false, true);
      }

      // Increment counts
      tot_lines  += n_lines;
      tot_tracks += tracks.n();

      // Close the current file
      f.close();

      // Search the tracks for genesis events
      for(j=0; j<tracks.n(); j++) {

         // Check for empty tracks
         if(tracks[j].n_points() == 0) {
            continue;
         }

         // Check the model genesis event criteria
         if(!is_anly) {

            // Check the lead time window
            if(tracks[j][0].lead() < conf_info.LeadSecBeg ||
               tracks[j][0].lead() > conf_info.LeadSecEnd) {
               continue;
            }

            // Check the minimum duration
            if(tracks[j].duration() < conf_info.MinDur) {
               continue;
            }

            // Check the genesis event criteria for each track point
            keep = false;
            for(k=0; k<tracks[j].n_points(); k++) {
               if(conf_info.FcstEventInfo.is_keeper(tracks[j][k])) {
                  keep = true;
                  break;
               }
            } // end for k

            if(!keep) continue;
         }

         // Store the genesis event
         genesis.add(tracks[j]);

      } // end for j

      // Dump out the current number of lines
      mlog << Debug(4)
           << "[File " << i+1 << " of " << files.n()
           << "] Found " << genesis.n() << " genesis events, from "
           << tracks.n() << " tracks, from " << n_lines
           << " input lines, from file \"" << files[i] << "\".\n";

   } // end for i

   // Dump out the total number of lines
   mlog << Debug(3)
        << "Found a total of " << genesis.n()
        << " genesis events, from " << tot_tracks << " tracks, from "
        << tot_lines << " input lines, from " << files.n()
        << " input files.\n";

   // Compute the distance to land
   for(i=0; i<genesis.n(); i++) {
      genesis.set_dland(i, conf_info.compute_dland(genesis[i].lat(),
                                                   genesis[i].lon()));
   }

   // Dump out the track information
   mlog << Debug(2)
        << "Identified " << genesis.n() << " genesis events.\n";

   // Dump out very verbose output
   if(mlog.verbosity_level() >= 5) {
      mlog << Debug(5)
           << genesis.serialize_r() << "\n";
   }
   // Dump out track info
   else {
      for(i=0; i<genesis.n(); i++) {
         mlog << Debug(4)
              << "[Genesis " << i+1 << " of " << genesis.n()
              << "] " << genesis[i].serialize() << "\n";
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_table(AsciiTable &at) {

   // Justify the TC-STAT AsciiTable objects
   justify_tc_stat_cols(at);

   // Set the precision
   at.set_precision(conf_info.Conf.output_precision());

   // Set the bad data value
   at.set_bad_data_value(bad_data_double);

   // Set the bad data string
   at.set_bad_data_str(na_str);

   // Don't write out trailing blank rows
   at.set_delete_trailing_blank_rows(1);

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\t-genesis path\n"
        << "\t-track path\n"
        << "\t-config file\n"
        << "\t[-out base]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-genesis path\" specifies an ATCF genesis file "
        << "or top-level directory with files matching the regular "
        << "expression \"" << atcf_gen_reg_exp << "\" (required).\n"

        << "\t\t\"-track path\" specifies an ATCF track file or "
        << "top-level directory with files matching the regular "
        << "expression \"" << atcf_reg_exp << "\" for the verifying "
        << "BEST and operational tracks (required).\n"

        << "\t\t\"-config file\" is used once to specify the "
        << "TCGenConfig file containing the desired configuration "
        << "settings (required).\n"

        << "\t\t\"-out base\" overrides the default output file base "
        << "(" << out_base << ") (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n";

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_genesis(const StringArray & a) {
   int i;
   StringArray sa;
   ConcatString cs;

   // Check for optional suffix sub-argument
   for(int i=0; i<a.n(); i++) {

      cs = a[i];
      if(cs.startswith("suffix")) {
         sa = cs.split("=");
         if(sa.n() != 2) {
            mlog << Error << "\nset_genesis() -> "
                 << "the model suffix must be specified as "
                 << "\"suffix=string\".\n\n";
         }
         else {
            genesis_model_suffix.add(sa[1]);
         }
      }
      else {
         genesis_source.add(a[i]);
      }
   }

   // Check for consistent usage
   if(genesis_model_suffix.n() > 0 &&
      genesis_model_suffix.n() != genesis_source.n()) {
      mlog << Error << "\nset_genesis() -> "
           << "the number of \"suffix=string\" options must match the "
           << "number of -genesis options.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void set_track(const StringArray & a) {
   int i;
   StringArray sa;
   ConcatString cs;

   // Check for optional suffix sub-argument
   for(int i=0; i<a.n(); i++) {

      cs = a[i];
      if(cs.startswith("suffix")) {
         sa = cs.split("=");
         if(sa.n() != 2) {
            mlog << Error << "\nset_track() -> "
                 << "the model suffix must be specified as "
                 << "\"suffix=string\".\n\n";
         }
         else {
            track_model_suffix.add(sa[1]);
         }
      }
      else {
         track_source.add(a[i]);
      }
   }

   // Check for consistent usage
   if(track_model_suffix.n() > 0 &&
      track_model_suffix.n() != track_source.n()) {
      mlog << Error << "\nset_track() -> "
           << "the number of \"suffix=string\" options must match the "
           << "number of -track options.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void set_config(const StringArray & a) {
   config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_out(const StringArray & a) {
   out_base = a[0];
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
