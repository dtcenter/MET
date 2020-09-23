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
static void   process_genesis_pair (int, const ConcatString &,
                                    const GenesisInfoArray &,
                                    const GenesisInfoArray &,
                                    const GenesisInfoArray &,
                                    GenCTCInfo &);

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
   ConcatString atcf_id, cs;

   GenCTCInfo cur_info;

   // Get the list of genesis track files
   get_atcf_files(genesis_source, genesis_model_suffix, atcf_gen_reg_exp,
                  genesis_files,  genesis_files_model_suffix);

   mlog << Debug(2)
        << "Processing " << genesis_files.n()
        << " forecast genesis track files.\n";
   process_track_files(genesis_files, genesis_files_model_suffix,
                       fcst_ga, false);

   // Get the list of verifing track files
   get_atcf_files(track_source, track_model_suffix, atcf_reg_exp,
                  track_files,  track_files_model_suffix);

   mlog << Debug(2)
        << "Processing " << track_files.n()
        << " verifying track files.\n";
   process_track_files(track_files, track_files_model_suffix,
                       anly_ga, true);

   // Setup output files based on the number of techniques present
   setup_txt_files(fcst_ga.n_technique());

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

         // Check filters
         if(conf_info.VxOpt[i].is_keeper(anly_ga[j])) {

            if(anly_ga[j].technique() == conf_info.BestEventInfo.Technique) {
                best_ga.add(anly_ga[j]);
            }
            else if(anly_ga[j].technique() == conf_info.OperEventInfo.Technique) {
                oper_ga.add(anly_ga[j]);
            }
         }
      } // end for j

      // Loop through and process the genesis event pairs
      for(j=0,it=fcst_ga_map.begin(); it!=fcst_ga_map.end(); it++,j++) {
         mlog << Debug(2)
              << "[Filter " << i+1 << " (" << conf_info.VxOpt[i].Desc
              << ") " << ": Model " << j+1 << "] " << "For " << it->first
              << " model, comparing " << it->second.n()
              << " genesis forecasts to " << best_ga.n() << " "
              << conf_info.BestEventInfo.Technique << " and "
              << oper_ga.n() << " " << conf_info.OperEventInfo.Technique
              << " genesis events.\n";
         process_genesis_pair(i, it->first, it->second,
                              best_ga, oper_ga, cur_info);

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

void process_genesis_pair(int i_vx, const ConcatString &model,
                          const GenesisInfoArray &fga,
                          const GenesisInfoArray &bga,
                          const GenesisInfoArray &oga,
                          GenCTCInfo &info) {
   int i, j, i_match;
   unixtime ut, init_beg, init_end;
   map<ConcatString,GenCTCInfo> info_map;
   vector<const GenesisInfo *> fga_match;
   const GenesisInfo *g_ptr;
   bool match;

   // Initialize
   info.clear();

   // Loop over the model genesis events to find HITS and FALSE ALARMS
   for(i=0; i<fga.n(); i++) {

      // Initialize
      g_ptr = 0;

      // Keep track of timing info
      info.add_fcst_valid(fga[i].valid_min(), fga[i].valid_max());

      // Find BEST track match
      i_match = bga.find_match(fga[i],
                   conf_info.VxOpt[i_vx].GenesisRadius,
                   conf_info.VxOpt[i_vx].GenesisSecBeg,
                   conf_info.VxOpt[i_vx].GenesisSecEnd);
      if(!is_bad_data(i_match)) g_ptr = &bga[i_match];

      // Otherwise, find operational track match
      if(is_bad_data(i_match)) {
         i_match = oga.find_match(fga[i],
                      conf_info.VxOpt[i_vx].GenesisRadius,
                      conf_info.VxOpt[i_vx].GenesisSecBeg,
                      conf_info.VxOpt[i_vx].GenesisSecEnd);
         if(!is_bad_data(i_match)) g_ptr = &oga[i_match];
      }

      // Store the match pointer
      fga_match.push_back(g_ptr);

      // Print log messages about matches
      if(g_ptr) {
          mlog << Debug(4) << fga[i].technique() << " "
               << unix_to_yyyymmdd_hhmmss(fga[i].init())
               << " initialization, "
               << fga[i].lead_time()/sec_per_hour << " lead, "
               << unix_to_yyyymmdd_hhmmss(fga[i].genesis_time())
               << " genesis at ("
               << fga[i].lat() << ", " << fga[i].lon()
               << ") is a HIT for " << g_ptr->technique() << " "
               << unix_to_yyyymmdd_hhmmss(g_ptr->genesis_time())
               << " genesis at (" << g_ptr->lat() << ", "
               << g_ptr->lon() << ").\n";

          // Increment the HIT count
          info.cts_info.cts.inc_fy_oy();
      }
      else {
          mlog << Debug(4) << fga[i].technique() << " "
               << unix_to_yyyymmdd_hhmmss(fga[i].init())
               << " initialization, "
               << fga[i].lead_time()/sec_per_hour << " lead, "
               << unix_to_yyyymmdd_hhmmss(fga[i].genesis_time())
               << " genesis at (" << fga[i].lat() << ", "
               << fga[i].lon() << ") is a FALSE ALARM.\n";

          // Increment the FALSE ALARM count
          info.cts_info.cts.inc_fy_on();
      }
   } // end for i fga

   // Loop over the BEST track genesis events to find MISSES
   for(i=0; i<bga.n(); i++) {

      // Keep track of timing info
      info.add_obs_valid(bga[i].valid_min(), bga[i].valid_max());

      // Define opportunities to forecast this genesis event
      init_beg = bga[i].genesis_time() - conf_info.LeadSecEnd;
      init_end = bga[i].genesis_time() - conf_info.LeadSecBeg;

      // Loop over the model opportunities
      for(ut=init_beg; ut<=init_end; ut+=conf_info.InitFreqSec) {

         // Search for forecast genesis events matching this BEST track
         // genesis event for each possible combination of initialization
         // and lead time.

         for(j=0, match=false; j<fga.n(); j++) {
             if(fga_match[j] == &bga[i] && fga[j].init() == ut) {
                mlog << Debug(4) << bga[i].technique() << " "
                     << unix_to_yyyymmdd_hhmmss(bga[i].genesis_time())
                     << " genesis at (" << bga[i].lat() << ", "
                     << bga[i].lon() << ") for "
                     << unix_to_yyyymmdd_hhmmss(ut)
                     << " initialization "
                     << (bga[i].genesis_time() - ut)/sec_per_hour
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
            mlog << Debug(4) << bga[i].technique() << " "
                 << unix_to_yyyymmdd_hhmmss(bga[i].genesis_time())
                 << " genesis at (" << bga[i].lat() << ", "
                 << bga[i].lon() << ") for "
                 << unix_to_yyyymmdd_hhmmss(ut) << " initialization "
                 << (bga[i].genesis_time() - ut)/sec_per_hour
                 << " lead is a MISS.\n";
            info.cts_info.cts.inc_fn_oy();
         }
      } // end for ut
   } // end for i bga

   mlog << Debug(3) << "For " << model
        << " model, contingency table hits = "
        << info.cts_info.cts.fy_oy() << ", false alarms = "
        << info.cts_info.cts.fy_on() << ", and misses = "
        << info.cts_info.cts.fn_oy() << ".\n";

   return;
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

void process_track_files(const StringArray &files,
                         const StringArray &model_suffix,
                         GenesisInfoArray &genesis, bool is_anly) {
   int i, j, k;
   int n_lines, tot_lines, tot_tracks, n_genesis;
   ConcatString suffix, atcf_ids;
   StringArray best_tech, oper_tech;
   LineDataFile f;
   ATCFTrackLine line;
   TrackInfoArray tracks;
   GenesisEventInfo *event_info = (GenesisEventInfo *) 0;
   bool keep;

   // Analysis ATCF ID's
   if(is_anly) {
      atcf_ids << conf_info.BestEventInfo.Technique;
      if(conf_info.OperEventInfo.Technique.nonempty()) {
         atcf_ids << " or " << conf_info.OperEventInfo.Technique << " analysis";
      }
   }
   // Forecast ATCF ID's
   else {
      atcf_ids << "forecast";
   }

   // Initialize
   genesis.clear();
   tot_lines = tot_tracks = n_genesis = 0;

   // Set metadata pointers
   best_tech.add(conf_info.BestEventInfo.Technique);
   line.set_best_technique(&best_tech);
   oper_tech.add(conf_info.OperEventInfo.Technique);
   line.set_oper_technique(&oper_tech);

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

      // Set metadata pointer
      suffix = model_suffix[i];
      line.set_tech_suffix(&suffix);

      // Read each input line. Track lines may be interleaved
      // within a file but cannot span multiple files.
      while(f >> line) {

         // Increment the line counts
         n_lines++;

         // Only process the specified BEST and operational track
         if(is_anly && !line.is_best_track() && !line.is_oper_track()) {
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
            if(tracks[j].duration() < conf_info.MinDur*sec_per_hour) {
               continue;
            }
         }

         // Select the genesis event filtering criteria
         if(tracks[j].is_best_track())      event_info = &conf_info.BestEventInfo;
         else if(tracks[j].is_oper_track()) event_info = &conf_info.OperEventInfo;
         else                               event_info = &conf_info.FcstEventInfo;

         // Check the genesis event criteria for each track point
         keep = false;
         for(k=0; k<tracks[j].n_points(); k++) {
            if(event_info->is_keeper(tracks[j][k])) {
               keep = true;
               break;
            }
         } // end for k

         if(!keep) continue;

         // Store the genesis event
         genesis.add(tracks[j], k);

      } // end for j

      // Dump out the current number of lines
      mlog << Debug(4)
           << "[File " << i+1 << " of " << files.n() << "] Found "
           << genesis.n() - n_genesis << " " << atcf_ids
           << " genesis events, from " << tracks.n()
           << " tracks, from " << n_lines
           << " input lines, from file \"" << files[i] << "\".\n";
      n_genesis = genesis.n();

   } // end for i

   // Dump out the total number of lines
   mlog << Debug(3)
        << "Found a total of " << genesis.n() << " " << atcf_ids
        << " genesis events, from " << tot_tracks << " tracks, from "
        << tot_lines << " input lines, from " << files.n()
        << " input files.\n";

   // Compute the distance to land
   for(i=0; i<genesis.n(); i++) {
      genesis.set_dland(i, conf_info.compute_dland(     genesis[i].lat(),
                                                   -1.0*genesis[i].lon()));
   }

   // Dump out the track information
   mlog << Debug(2)
        << "Identified " << genesis.n() << " "
        << (is_anly ? "analysis" : "forecast")
        << " genesis events.\n";

   // Dump out very verbose output
   if(mlog.verbosity_level() >= 6) {
      mlog << Debug(6)
           << genesis.serialize_r() << "\n";
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
      write_fho_row(shc, info.cts_info,
                    conf_info.OutputMap[stat_fho],
                    stat_at, i_stat_row,
                    txt_at[i_fho], i_txt_row[i_fho]);
   }

   // Write out CTC
   if(conf_info.OutputMap[stat_ctc] != STATOutputType_None) {
      write_ctc_row(shc, info.cts_info,
                    conf_info.OutputMap[stat_ctc],
                    stat_at, i_stat_row,
                    txt_at[i_ctc], i_txt_row[i_ctc]);
   }

   // Write out CTS
   if(conf_info.OutputMap[stat_cts] != STATOutputType_None) {

      // Compute the statistics
      info.cts_info.allocate_n_alpha(1);
      info.cts_info.alpha[0] = conf_info.CIAlpha;
      info.cts_info.compute_stats();
      info.cts_info.compute_ci();

      write_cts_row(shc, info.cts_info,
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
