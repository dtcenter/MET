// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
//   001    10/21/20  Halley Gotway   Fix for MET #1465
//   002    12/15/20  Halley Gotway   Matching logic for MET #1448
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

#include "vx_data2d_factory.h"
#include "vx_statistics.h"
#include "vx_tc_util.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_log.h"

#include "met_file.h"
#include "nav.h"

////////////////////////////////////////////////////////////////////////

static void   process_command_line (int, char **);
static void   process_genesis      ();
static void   get_atcf_files       (const StringArray &,
                                    const StringArray &,
                                    const char *,
                                    StringArray &, StringArray &);
static void   get_fcst_genesis     (const StringArray &,
                                    const StringArray &,
                                    GenesisInfoArray &);
static void   get_best_oper_tracks (const StringArray &,
                                    const StringArray &,
                                    TrackInfoArray &, TrackInfoArray &);

static void   do_genesis_ctc       (int, const ConcatString &,
                                    const GenesisInfoArray &,
                                    const TrackInfoArray &,
                                    const TrackInfoArray &,
                                    GenCTCInfo &);
static bool   find_genesis_match   (const GenesisInfo &,
                                    const TrackInfoArray &,
                                    const TrackInfoArray & ,
                                    double, GenesisInfo &);

static void   setup_txt_files      (int);
static void   setup_table          (AsciiTable &);
static void   write_cts            (int, GenCTCInfo &);
static void   finish_txt_files     ();

static void   usage                ();
static void   set_source           (const StringArray &, const char *,
                                    StringArray &, StringArray &);
static void   set_genesis          (const StringArray &);
static void   set_track            (const StringArray &);
static void   set_config           (const StringArray &);
static void   set_out              (const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Identify and process genesis events and write output
   process_genesis();

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
   cline.add(set_genesis, "-genesis", -1);
   cline.add(set_track,   "-track",   -1);
   cline.add(set_config,  "-config",   1);
   cline.add(set_out,     "-out",      1);

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
   GenesisInfoArray fcst_ga, new_ga;
   TrackInfoArray oper_ta, best_ta;
   map<ConcatString,GenesisInfoArray> fcst_ga_map;
   map<ConcatString,GenesisInfoArray>::iterator it;
   ConcatString atcf_id, cs;
   GenCTCInfo cur_info;

   // Get the list of genesis track files
   get_atcf_files(genesis_source, genesis_model_suffix, atcf_gen_reg_exp,
                  genesis_files,  genesis_files_model_suffix);

   mlog << Debug(2)
        << "Processing " << genesis_files.n()
        << " forecast genesis track files.\n";
   get_fcst_genesis(genesis_files, genesis_files_model_suffix,
                    fcst_ga);

   // Get the list of verifing track files
   get_atcf_files(track_source, track_model_suffix, atcf_reg_exp,
                  track_files,  track_files_model_suffix);

   mlog << Debug(2)
        << "Processing " << track_files.n()
        << " verifying track files.\n";
   get_best_oper_tracks(track_files, track_files_model_suffix,
                        best_ta, oper_ta);

   // Setup output files based on the number of techniques present
   setup_txt_files(fcst_ga.n_technique());

   // Process each verification filter
   for(i=0; i<conf_info.n_vx(); i++) {

      // Initialize
      fcst_ga_map.clear();

      // Subset the forecast genesis events
      for(j=0; j<fcst_ga.n(); j++) {

         // Check filters
         if(!conf_info.VxOpt[i].is_keeper(fcst_ga[j])) continue;

         // Store the current forecast ATCF ID
         atcf_id = fcst_ga[j].technique();

         // Check specified forecast models
         if( conf_info.VxOpt[i].Model.n() > 0 &&
            !conf_info.VxOpt[i].Model.has(atcf_id)) continue;

         // Add a new map entry, if necessary
         if(fcst_ga_map.count(atcf_id) == 0) {
            new_ga.clear();
            fcst_ga_map[atcf_id] = new_ga;
         }

         // Store the current genesis event
         fcst_ga_map[atcf_id].add(fcst_ga[j]);

      } // end j

      // Process the genesis events for each forecast model.
      // Compare them to all unfiltered BEST and operational tracks.
      for(j=0,it=fcst_ga_map.begin(); it!=fcst_ga_map.end(); it++,j++) {
         mlog << Debug(2)
              << "[Filter " << i+1 << " (" << conf_info.VxOpt[i].Desc
              << ") " << ": Model " << j+1 << "] " << "For " << it->first
              << " model, comparing " << it->second.n()
              << " genesis forecasts to " << best_ta.n() << " "
              << conf_info.BestEventInfo.Technique << " and "
              << oper_ta.n() << " " << conf_info.OperTechnique
              << " tracks.\n";
         do_genesis_ctc(i, it->first, it->second,
                        best_ta, oper_ta, cur_info);

         // Write output for the current model
         cur_info.model = it->first;
         write_cts(i, cur_info);

      } // end for j

   } // end for i n_vx

   // Finish output files
   finish_txt_files();

   return;
}

////////////////////////////////////////////////////////////////////////
//
// For each genesis forecast, find the matching BEST tracks genesis
// event that is the closest in space and time, where:
// (1) The difference between the BEST and forecast track genesis falls
//     within the configurable genesis_window.
// (2) The distance between the BEST and forecast genesis locations is
//     less than the configurable genesis_radius.
// (3) For multiple BEST track matches, choose the one closest in space.
//
// If there are no BEST track matches, apply to same logic with the
// operational baseline genesis events.
//
// Any genesis forecast with a BEST or operational match is a HIT.
//
// Any genesis forecast with no BEST or operational match is a FALSE
// ALARM.
//
// For each BEST track genesis event, define the timing information for
// the model's opportunities to forecast that genesis event using the
// configurable init_freq and lead_window options.  Any case for which
// no match exists is a MISS.
//
////////////////////////////////////////////////////////////////////////

void do_genesis_ctc(int i_vx, const ConcatString &model,
                    const GenesisInfoArray &fga,
                    const TrackInfoArray   &bta,
                    const TrackInfoArray   &ota,
                    GenCTCInfo &info) {
   int i, j, dsec;
   double dist;
   unixtime ut, init_beg, init_end;
   map<ConcatString,GenCTCInfo> info_map;
   GenesisInfo best_gi;
   vector<GenesisInfo> fga_match;
   ConcatString desc;

   // Initialize
   info.clear();

   // Loop over the model genesis events.
   // Determine HITS and FALSE ALARMS.
   for(i=0; i<fga.n(); i++) {

      desc << cs_erase
           << fga[i].technique() << " "
           << unix_to_yyyymmdd_hhmmss(fga[i].init())
           << " initialization, "
           << fga[i].lead_time()/sec_per_hour << " lead, "
           << unix_to_yyyymmdd_hhmmss(fga[i].genesis_time())
           << " genesis forecast at ("
           << fga[i].lat() << ", " << fga[i].lon() << ")";
      
      // Keep track of timing info
      info.add_fcst_valid(fga[i].valid_min(), fga[i].valid_max());

      // Search for a BEST track match
      if(find_genesis_match(fga[i], bta, ota,
                            conf_info.VxOpt[i_vx].GenesisRadius,
                            best_gi)) {

         // Store the match
         fga_match.push_back(best_gi);

         // Discard the case if the forecast initialization time
         // is at or after the BEST track genesis time.
         if(fga[i].init() >= best_gi.genesis_time()) {
            mlog << Debug(4) << "Discard " << desc
                 << " since the model initialization time is at or "
                 << "after the matching BEST track "
                 << unix_to_yyyymmdd_hhmmss(best_gi.genesis_time())
                 << " genesis time.\n";
            continue;
         }
         // Check for a HIT
         else {

            // Compute time and space offsets
            dsec = best_gi.genesis_time() - fga[i].genesis_time();
            dist = gc_dist(best_gi.lat(), best_gi.lon(),
                           fga[i].lat(),  fga[i].lon());

            // Technique 1:
            // HIT if forecast genesis time and location
            // are within the temporal and spatial windows.
            if(dsec >= conf_info.VxOpt[i_vx].GenesisSecBeg &&
               dsec <= conf_info.VxOpt[i_vx].GenesisSecEnd &&
               dist <= conf_info.VxOpt[i_vx].GenesisRadius) {
               mlog << Debug(4) << desc
                    << " is a technique 1 HIT for "
                    << best_gi.technique() << " "
                    << unix_to_yyyymmdd_hhmmss(best_gi.genesis_time())
                    << " genesis at (" << best_gi.lat() << ", "
                    << best_gi.lon() << ").\n";

               // Increment the HIT count
               info.cts_tech1.cts.inc_fy_oy();
            }
            else {
               mlog << Debug(4) << desc
                    << " is a technique 1 FALSE ALARM for "
                    << best_gi.technique() << " "
                    << unix_to_yyyymmdd_hhmmss(best_gi.genesis_time())
                    << " genesis at (" << best_gi.lat() << ", "
                    << best_gi.lon() << ").\n";

               // Increment the FALSE ALARM count
               info.cts_tech1.cts.inc_fy_on();
            }
            
            // Technique 2:
            // HIT if forecast init time is close enough to
            // the BEST genesis time.
            
            // Compute time offset
            dsec = best_gi.genesis_time() - fga[i].init();
            if(dsec <= conf_info.VxOpt[i_vx].GenesisInitDSec) {
               mlog << Debug(4) << desc
                    << " is a technique 2 HIT for "
                    << best_gi.technique() << " "
                    << unix_to_yyyymmdd_hhmmss(best_gi.genesis_time())
                    << " genesis at (" << best_gi.lat() << ", "
                    << best_gi.lon() << ").\n";

               // Increment the HIT count
               info.cts_tech2.cts.inc_fy_oy();
            }
            else {
               mlog << Debug(4) << desc
                    << " is a technique 2 FALSE ALARM for "
                    << best_gi.technique() << " "
                    << unix_to_yyyymmdd_hhmmss(best_gi.genesis_time())
                    << " genesis at (" << best_gi.lat() << ", "
                    << best_gi.lon() << ").\n";

               // Increment the FALSE ALARM count
               info.cts_tech2.cts.inc_fy_on();
            }
         }
      }
      // No BEST track match is a FALSE ALARM
      else {
         mlog << Debug(4) << desc
              << " is a technique 1 and 2 FALSE ALARM.\n";

         // Increment the FALSE ALARM count for both techniques
         info.cts_tech1.cts.inc_fy_on();
         info.cts_tech2.cts.inc_fy_on();
      }
   } // end for i fga
/* JHG do this too!
   // Loop over the BEST track genesis events to find MISSES
   for(i=0; i<bta.n(); i++) {

      // Keep track of timing info
      info.add_obs_valid(bta[i].valid_min(), bta[i].valid_max());

      // Define opportunities to forecast this genesis event
      init_beg = bta[i].genesis_time() - conf_info.LeadSecEnd;
      init_end = bta[i].genesis_time() - conf_info.LeadSecBeg;

      // Loop over the model opportunities
      for(ut=init_beg; ut<=init_end; ut+=conf_info.InitFreqSec) {

         // Search for forecast genesis events matching this BEST track
         // genesis event for each possible combination of initialization
         // and lead time.

         for(j=0, match=false; j<fga.n(); j++) {
             if(fga_match[j] == &bta[i] && fga[j].init() == ut) {
                mlog << Debug(4) << bga[i].technique() << " "
                     << unix_to_yyyymmdd_hhmmss(bga[i].genesis_time())
                     << " genesis at (" << bga[i].lat() << ", "
                     << bga[i].lon() << ") for "
                     << unix_to_yyyymmdd_hhmmss(ut)
                     << " initialization, "
                     << (bta[i].genesis_time() - ut)/sec_per_hour
                     << " lead matches " << fga[j].technique() << " "
                     << unix_to_yyyymmdd_hhmmss(fga[j].genesis_time())
                     << " genesis at (" << fga[j].lat() << ", "
                     << fga[j].lon() << ").\n";
                 match = true;
                 break;
             }
         }

         // Increment the MISS count
         if(!match) {
            mlog << Debug(4) << bta[i].technique() << " "
                 << unix_to_yyyymmdd_hhmmss(bta[i].genesis_time())
                 << " genesis at (" << bta[i].lat() << ", "
                 << bta[i].lon() << ") for "
                 << unix_to_yyyymmdd_hhmmss(ut) << " initialization, "
                 << (bta[i].genesis_time() - ut)/sec_per_hour
                 << " lead is a MISS.\n";
            info.cts_tech1.cts.inc_fn_oy();
         }
      } // end for ut
   } // end for i bga
*/
   mlog << Debug(3) << "For " << model
        << " model, technique 1 contingency table hits = "
        << info.cts_tech1.cts.fy_oy() << ", false alarms = "
        << info.cts_tech1.cts.fy_on() << ", and misses = "
        << info.cts_tech1.cts.fn_oy() << ".\n";

   mlog << Debug(3) << "For " << model
        << " model, technique 2 contingency table hits = "
        << info.cts_tech2.cts.fy_oy() << ", false alarms = "
        << info.cts_tech2.cts.fy_on() << ", and misses = "
        << info.cts_tech2.cts.fn_oy() << ".\n";

   return;
}

////////////////////////////////////////////////////////////////////////

bool find_genesis_match(const GenesisInfo    &fcst_gi,
                        const TrackInfoArray &bta,
                        const TrackInfoArray &ota,
                        const double rad,
                        GenesisInfo &best_gi) {
   int i, j;
   int i_best = bad_data_int;
   int i_oper = bad_data_int;

   // Search the BEST track points for a match
   for(i=0, i_best=bad_data_int;
       i<bta.n() && is_bad_data(i_best);
       i++) {
      for(j=0; j<bta[i].n_points(); j++) {
         if(fcst_gi.is_match(bta[i][j], rad)) {
            i_best = i;
            mlog << Debug(4) << fcst_gi.technique() << " "
                 << unix_to_yyyymmdd_hhmmss(fcst_gi.init())
                 << " initialization, "
                 << fcst_gi.lead_time()/sec_per_hour << " lead, "
                 << unix_to_yyyymmdd_hhmmss(fcst_gi.genesis_time())
                 << " genesis at (" << fcst_gi.lat() << ", " << fcst_gi.lon()
                 << ") matches BEST track storm id "
                 << bta[i].storm_id() << ".\n";
            break;
         }
      }
   } // end for bta

   // If no BEST track match was found, search the operational tracks
   if(is_bad_data(i_best)) {

      for(i=0, i_oper=bad_data_int;
          i<ota.n() && is_bad_data(i_oper);
          i++) {
         for(j=0; j<ota[i].n_points(); j++) {
            if(fcst_gi.is_match(ota[i][j], rad)) {
               i_oper = i;
               mlog << Debug(4) << fcst_gi.technique() << " "
                    << unix_to_yyyymmdd_hhmmss(fcst_gi.init())
                    << " initialization, "
                    << fcst_gi.lead_time()/sec_per_hour << " lead, "
                    << unix_to_yyyymmdd_hhmmss(fcst_gi.genesis_time())
                    << " genesis at (" << fcst_gi.lat() << ", " << fcst_gi.lon()
                    << ") matches operational " << ota[i].technique()
                    << " track storm id " << ota[i].storm_id() << ".\n";
               break;
            }
         }
      } // end for ota

      // Find BEST track for this operational track
      if(!is_bad_data(i_oper)) {
         for(i=0; i<bta.n(); i++) {
            if(bta[i].storm_id() == ota[i_oper].storm_id()) {
               i_best = i;
               break;
            }
         }
      }
   }

   // Deteremine the BEST track genesis
   if(!is_bad_data(i_best)) {

      // Find the BEST track genesis point
      if(!best_gi.set(bta[i_best], (&conf_info.BestEventInfo))) {
         mlog << Debug(4)
              << "No genesis point found for BEST track storm id "
              << bta[i].storm_id() << ".\n";
         i_best = bad_data_int;
      }
   }
   // No matching BEST track found
   else {
      mlog << Debug(4) << fcst_gi.technique() << " "
           << unix_to_yyyymmdd_hhmmss(fcst_gi.init())
           << " initialization, "
           << fcst_gi.lead_time()/sec_per_hour << " lead, "
           << unix_to_yyyymmdd_hhmmss(fcst_gi.genesis_time())
           << " genesis at (" << fcst_gi.lat() << ", " << fcst_gi.lon()
           << ") has no match in the BEST or operational tracks.\n";
   }

   return(i_best);
}

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

void get_fcst_genesis(const StringArray &files,
                      const StringArray &model_suffix,
                      GenesisInfoArray  &genesis) {
   int i, j, k;
   int n_lines, tot_lines, tot_tracks, n_genesis;
   ConcatString suffix;
   LineDataFile f;
   ATCFTrackLine line;
   TrackInfoArray tracks;
   GenesisInfo fcst_gi;
   bool keep;

   // Initialize
   genesis.clear();
   tot_lines = tot_tracks = n_genesis = 0;

   // Process each of the input ATCF files
   for(i=0; i<files.n(); i++) {

      // Open the current file
      if(!f.open(files[i].c_str())) {
         mlog << Error << "\nget_fcst_genesis() -> "
              << "unable to open file \"" << files[i] << "\"\n\n";
         exit(1);
      }

      // Initialize
      tracks.clear();
      n_lines = 0;

      // Set metadata pointer
      suffix = model_suffix[i];
      line.set_tech_suffix(&suffix);

      // Process the input track lines
      while(f >> line) {
         n_lines++;
         tracks.add(line);
      }

      // Increment counts
      tot_lines  += n_lines;
      tot_tracks += tracks.n();

      // Close the current file
      f.close();

      // Search the tracks for genesis events
      for(j=0; j<tracks.n(); j++) {

         // Attempt to define genesis
         if(!fcst_gi.set(tracks[j], (&conf_info.FcstEventInfo))) {
            continue;
         }

         // Check the forecast lead time window
         if(fcst_gi.lead_time() < conf_info.LeadSecBeg ||
            fcst_gi.lead_time() > conf_info.LeadSecEnd) {
            mlog << Debug(6) << "Skipping genesis event for lead time "
                 << fcst_gi.lead_time()/sec_per_hour << ".\n";
            continue;
         }

         // Check the forecast track minimum duration
         if(fcst_gi.duration() < conf_info.MinDur*sec_per_hour) {
            mlog << Debug(6) << "Skipping genesis event for track duration of "
                 << fcst_gi.duration()/sec_per_hour << ".\n";
            continue;
         }

         // Store the genesis event
         genesis.add(fcst_gi);

      } // end for j

      // Dump out the current number of lines
      mlog << Debug(4)
           << "[File " << i+1 << " of " << files.n() << "] Found "
           << genesis.n() - n_genesis << " forecast genesis events, from "
           << tracks.n() << " tracks, from " << n_lines
           << " input lines, from file \"" << files[i] << "\".\n";
      n_genesis = genesis.n();

   } // end for i

   // Dump out the total number of lines
   mlog << Debug(3)
        << "Found a total of " << genesis.n()
        << " forecast genesis events, from " << tot_tracks
        << " tracks, from " << tot_lines << " input lines, from "
        << files.n() << " input files.\n";

   // JHG, am I computing dland everywhere needed?

   // Compute the distance to land
   for(i=0; i<genesis.n(); i++) {
      genesis.set_dland(i, conf_info.compute_dland(     genesis[i].lat(),
                                                   -1.0*genesis[i].lon()));
   }

   // Dump out the track information
   mlog << Debug(2) << "Identified " << genesis.n()
        << " forecast genesis events.\n";

   // Dump out very verbose output
   if(mlog.verbosity_level() > 6) {
      mlog << Debug(6) << genesis.serialize_r() << "\n";
   }
   // Dump out track info
   else {
      for(i=0; i<genesis.n(); i++) {
         mlog << Debug(6)
              << "[Genesis " << i+1 << " of " << genesis.n()
              << "] " << genesis[i].serialize() << "\n";
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void get_best_oper_tracks(const StringArray &files,
                          const StringArray &model_suffix,
                          TrackInfoArray &best_ta,
                          TrackInfoArray &oper_ta) {
   int i, n_lines;
   ConcatString suffix;
   StringArray best_tech, oper_tech;
   LineDataFile f;
   ATCFTrackLine line;

   // Initialize
   best_ta.clear();
   oper_ta.clear();
   n_lines = 0;

   // Set metadata pointers
   best_tech.add(conf_info.BestEventInfo.Technique);
   line.set_best_technique(&best_tech);
   oper_tech.add(conf_info.OperTechnique);
   line.set_oper_technique(&oper_tech);

   // Process each of the input ATCF files
   for(i=0; i<files.n(); i++) {

      // Open the current file
      if(!f.open(files[i].c_str())) {
         mlog << Error
              << "\nget_best_oper_tracks() -> "
              << "unable to open file \"" << files[i] << "\"\n\n";
         exit(1);
      }

      // Set metadata pointer
      suffix = model_suffix[i];
      line.set_tech_suffix(&suffix);

      // Process the input track lines
      while(f >> line) {

         // Increment the line counter
         n_lines++;

         // Store all BEST track lines
         if(line.is_best_track()) {
            best_ta.add(line, false, true);
         }
         // Store only 0-hour operational track lines
         else if(line.is_oper_track() && line.lead() == 0) {
            oper_ta.add(line);
         }
         // Skip all other track lines
         else {
            continue;
         }
      }

      // Close the current file
      f.close();

   } // end for i

   // Dump out the total number of lines
   mlog << Debug(3)
        << "Found a total of " << best_ta.n() << " "
        << conf_info.BestEventInfo.Technique
        << " tracks and " << oper_ta.n() << " "
        << conf_info.OperTechnique
        << " operational tracks, from " << n_lines
        << " track lines, from " << files.n()
        << " input files.\n";

   // Dump out very verbose output
   if(mlog.verbosity_level() >= 6) {
      mlog << Debug(6) << "BEST tracks:\n"
           << best_ta.serialize_r() << "\n"
           << "Operational tracks:\n"
           << oper_ta.serialize_r() << "\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Setup the output ASCII files
//
////////////////////////////////////////////////////////////////////////

void setup_txt_files(int n_model) {
   int i, n_rows, n_cols;

   // Initialize file stream
   stat_out = (ofstream *) 0;

   // Build the file name
   stat_file << out_base << stat_file_ext;

   // Create the output STAT file
   open_txt_file(stat_out, stat_file.c_str());

   // Setup the STAT AsciiTable
   n_rows = 1 + 3 * n_model * conf_info.n_vx();
   n_cols = 1 + n_header_columns + n_cts_columns;
   stat_at.set_size(n_rows, n_cols);
   setup_table(stat_at);

   // Write the text header row
   write_header_row((const char **) 0, 0, 1, stat_at, 0, 0);

   // Initialize the row index to 1 to account for the header
   i_stat_row = 1;

   // Setup each of the optional output text files
   for(i=0; i<n_txt; i++) {

      // Only set it up if requested in the config file
      if(conf_info.OutputMap[txt_file_type[i]] == STATOutputType_Both) {

         // Initialize file stream
         txt_out[i] = (ofstream *) 0;

         // Build the file name
         txt_file[i] << out_base << "_" << txt_file_abbr[i]
                     << txt_file_ext;

         // Create the output text file
         open_txt_file(txt_out[i], txt_file[i].c_str());

         // Setup the text AsciiTable
         n_rows = 1 + n_model * conf_info.n_vx();
         n_cols = 1 + n_header_columns + n_txt_columns[i];
         txt_at[i].set_size(n_rows, n_cols);
         setup_table(txt_at[i]);

         // Write header row
         write_header_row(txt_columns[i], n_txt_columns[i], 1,
                          txt_at[i], 0, 0);

         // Initialize the row index to 1 to account for the header
         i_txt_row[i] = 1;
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_table(AsciiTable &at) {

   // Justify the STAT AsciiTable objects
   justify_stat_cols(at);

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

void write_cts(int i_vx, GenCTCInfo &info) {
   ConcatString var_name("GENESIS");

   // Setup header columns
   shc.set_model(info.model.c_str());
   shc.set_desc(conf_info.VxOpt[i_vx].Desc.c_str());
   if(conf_info.VxOpt[i_vx].Lead.n() == 1) {
      shc.set_fcst_lead_sec(conf_info.VxOpt[i_vx].Lead[0]);
   }
   shc.set_fcst_valid_beg(conf_info.VxOpt[i_vx].ValidBeg != 0 ?
                          conf_info.VxOpt[i_vx].ValidBeg : info.fbeg);
   shc.set_fcst_valid_end(conf_info.VxOpt[i_vx].ValidEnd != 0 ?
                          conf_info.VxOpt[i_vx].ValidEnd : info.fend);
   shc.set_obs_valid_beg(conf_info.VxOpt[i_vx].ValidBeg != 0 ?
                         conf_info.VxOpt[i_vx].ValidBeg : info.obeg);
   shc.set_obs_valid_end(conf_info.VxOpt[i_vx].ValidEnd != 0 ?
                         conf_info.VxOpt[i_vx].ValidEnd : info.oend);
   shc.set_fcst_var(var_name);
   shc.set_obs_var(var_name);
   shc.set_obtype(conf_info.BestEventInfo.Technique.c_str());
   if(!conf_info.VxOpt[i_vx].VxMaskName.empty()) {
      shc.set_mask(conf_info.VxOpt[i_vx].VxMaskName.c_str());
   }

   // Write out FHO
   if(conf_info.OutputMap[stat_fho] != STATOutputType_None) {
      write_fho_row(shc, info.cts_tech1,
                    conf_info.OutputMap[stat_fho],
                    stat_at, i_stat_row,
                    txt_at[i_fho], i_txt_row[i_fho]);
   }

   // Write out CTC
   if(conf_info.OutputMap[stat_ctc] != STATOutputType_None) {
      write_ctc_row(shc, info.cts_tech1,
                    conf_info.OutputMap[stat_ctc],
                    stat_at, i_stat_row,
                    txt_at[i_ctc], i_txt_row[i_ctc]);
   }

   // Write out CTS
   if(conf_info.OutputMap[stat_cts] != STATOutputType_None) {

      // Compute the statistics
      info.cts_tech1.allocate_n_alpha(1);
      info.cts_tech1.alpha[0] = conf_info.CIAlpha;
      info.cts_tech1.compute_stats();
      info.cts_tech1.compute_ci();

      write_cts_row(shc, info.cts_tech1,
                    conf_info.OutputMap[stat_cts],
                    stat_at, i_stat_row,
                    txt_at[i_cts], i_txt_row[i_cts]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void finish_txt_files() {
   int i;

   // Write out the contents of the STAT AsciiTable and close the STAT
   // output files
   if(stat_out) {
      *stat_out << stat_at;
      close_txt_file(stat_out, stat_file.c_str());
   }

   // Finish up each of the optional text files
   for(i=0; i<n_txt; i++) {

      // Only write the table if requested in the config file
      if(conf_info.OutputMap[txt_file_type[i]] == STATOutputType_Both) {

         // Write the AsciiTable to a file
         if(txt_out[i]) {
            *txt_out[i] << txt_at[i];
            close_txt_file(txt_out[i], txt_file[i].c_str());
         }
      }
   }

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

        << "\twhere\t\"-genesis path\" is one or more ATCF genesis "
        << "files, an ASCII file list containing them, or a top-level "
        << "directory with files matching the regular expression \""
        << atcf_gen_reg_exp << "\" (required).\n"

        << "\t\t\"-track path\" is one or more ATCF track "
        << "files, an ASCII file list containing them, or a top-level "
        << "directory with files matching the regular expression \""
        << atcf_reg_exp << "\" for the verifying BEST and operational "
        << "tracks (required).\n"

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

void set_source(const StringArray &a, const char *type_str,
                StringArray &source, StringArray &model_suffix) {
   int i, j;
   StringArray sa, tmp_src, tmp_suf;
   ConcatString cs;

   // Check for optional suffix sub-argument
   for(i=0; i<a.n(); i++) {
      cs = a[i];
      if(cs.startswith("suffix")) {
         sa = cs.split("=");
         if(sa.n() != 2) {
            mlog << Error << "\nset_source() -> "
                 << "in -" << type_str
                 << " the model suffix must be specified as "
                 << "\"suffix=string\".\n\n";
            exit(1);
         }
         else {
            tmp_suf.add(sa[1]);
         }
      }
      else {
         tmp_src.add(a[i]);
      }
   } // end for i

   // Check for consistent usage
   if(tmp_suf.n() > 0 && tmp_suf.n() != tmp_src.n()) {
      mlog << Error << "\nset_source() -> "
           << "the number of \"suffix=string\" options must match the "
           << "number of -" << type_str << " options.\n\n";
      exit(1);
   }

   // Process each source element as a file list
   for(i=0; i<tmp_src.n(); i++) {

      sa = parse_ascii_file_list(tmp_src[i].c_str());

      // Add list elements, if present
      if(sa.n() > 0) {
         source.add(sa);
         if(tmp_suf.n() > 0) {
            for(j=0; j<sa.n(); j++) model_suffix.add(tmp_suf[i]);
         }
      }
      // Otherwise, add a single element
      else {
         source.add(tmp_src[i]);
         if(tmp_suf.n() > 0) model_suffix.add(tmp_suf[i]);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void set_genesis(const StringArray & a) {
   set_source(a, "genesis", genesis_source, genesis_model_suffix);
}

////////////////////////////////////////////////////////////////////////

void set_track(const StringArray & a) {
   set_source(a, "track", track_source, track_model_suffix);
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
