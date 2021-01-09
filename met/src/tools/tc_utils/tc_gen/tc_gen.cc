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
//   003    12/31/20  Halley Gotway   Add NetCDF output for MET #1430
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
#include "pair_data_genesis.h"

#include "vx_data2d_factory.h"
#include "vx_statistics.h"
#include "vx_tc_util.h"
#include "vx_nc_util.h"
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
static void   process_fcst_tracks  (const StringArray &,
                                    const StringArray &,
                                    GenesisInfoArray &);
static void   process_best_tracks  (const StringArray &,
                                    const StringArray &,
                                    GenesisInfoArray &,
                                    TrackInfoArray &);

static void   get_genesis_pairs    (const TCGenVxOpt &,
                                    const ConcatString &,
                                    const GenesisInfoArray &,
                                    const GenesisInfoArray &,
                                    const TrackInfoArray &,
                                    PairDataGenesis &);

static void   do_genesis_ctc       (const TCGenVxOpt &,
                                    const PairDataGenesis &,
                                    GenCTCInfo &);

static int    find_genesis_match   (const GenesisInfo &,
                                    const GenesisInfoArray &,
                                    const TrackInfoArray &,
                                    double);

static void   setup_txt_files      (int);
static void   setup_table          (AsciiTable &);
static void   setup_nc_file        ();

static void   write_ctc_info       (GenCTCInfo &);
static void   write_nc             (GenCTCInfo &);
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
   if(genesis_source.n()   == 0 ||
      track_source.n()     == 0 ||
      config_file.length() == 0) {
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
   GenesisInfoArray fcst_ga, best_ga, empty_ga;
   TrackInfoArray oper_ta;
   ConcatString model, cs;
   map<ConcatString,GenesisInfoArray> model_ga_map;
   map<ConcatString,GenesisInfoArray>::iterator it;
   PairDataGenesis pairs;
   GenCTCInfo ctc_info;

   // Get the list of genesis track files
   get_atcf_files(genesis_source, genesis_model_suffix, atcf_gen_reg_exp,
                  genesis_files,  genesis_files_model_suffix);

   mlog << Debug(2)
        << "Processing " << genesis_files.n()
        << " forecast genesis track files.\n";
   process_fcst_tracks(genesis_files, genesis_files_model_suffix,
                       fcst_ga);

   // Get the list of verifing track files
   get_atcf_files(track_source, track_model_suffix, atcf_reg_exp,
                  track_files,  track_files_model_suffix);

   mlog << Debug(2)
        << "Processing " << track_files.n()
        << " verifying track files.\n";
   process_best_tracks(track_files, track_files_model_suffix,
                       best_ga, oper_ta);

   // Setup output files based on the number of techniques present
   setup_txt_files(fcst_ga.n_technique());

   // If requested, setup the NetCDF output file
   if(!conf_info.NcInfo.all_false()) setup_nc_file();

   // Process each verification filter
   for(i=0; i<conf_info.n_vx(); i++) {

      // Initialize
      model_ga_map.clear();

      // Subset the forecast genesis events
      for(j=0; j<fcst_ga.n(); j++) {

         // Check filters
         if(!conf_info.VxOpt[i].is_keeper(fcst_ga[j])) continue;

         // Store the current forecast ATCF ID
         model = fcst_ga[j].technique();

         // Check specified forecast models
         if( conf_info.VxOpt[i].Model.n() > 0 &&
            !conf_info.VxOpt[i].Model.has(model)) continue;

         // Add a new map entry, if necessary
         if(model_ga_map.count(model) == 0) {
            empty_ga.clear();
            model_ga_map[model] = empty_ga;
         }

         // Store the current genesis event
         model_ga_map[model].add(fcst_ga[j]);

      } // end j

      // Process the genesis events for each model.
      for(j=0,it=model_ga_map.begin(); it!=model_ga_map.end(); it++,j++) {

         // Initialize
         ctc_info.clear();
         ctc_info.Model = it->first;
         ctc_info.set_vx_opt(&conf_info.VxOpt[i],
                             &conf_info.NcOutGrid);

         mlog << Debug(2)
              << "[Filter " << i+1 << " (" << conf_info.VxOpt[i].Desc
              << ") " << ": Model " << j+1 << "] " << "For " << it->first
              << " model, comparing " << it->second.n()
              << " genesis forecasts to " << best_ga.n() << " "
              << conf_info.BestEventInfo.Technique << " and "
              << oper_ta.n() << " " << conf_info.OperTechnique
              << " tracks.\n";

         // Get the pairs
         get_genesis_pairs(conf_info.VxOpt[i], it->first, it->second,
                           best_ga, oper_ta, pairs);

         // Do the categorical verification
         do_genesis_ctc(conf_info.VxOpt[i], pairs, ctc_info);

         // Write the statistics output
         write_ctc_info(ctc_info);

         // TODO: MET #1597 write a new TC-Gen MPR line type
   
         // Write NetCDF output fields
         if(!conf_info.NcInfo.all_false()) {
            write_nc(ctc_info);
         }
   
      } // end for j

   } // end for i n_vx

   // Finish output files
   finish_txt_files();

   // Close the NetCDF output file
   if(nc_out) {

      // List the NetCDF file after it is finished
      mlog << Debug(1) << "Output file: " << out_nc_file << "\n";

      delete nc_out;
      nc_out = (NcFile *) 0;
   }


   return;
}

////////////////////////////////////////////////////////////////////////
//
// Match the forecast and BEST track genesis events:
// (1) Subset the BEST genesis events based on the current filters.
// (2) For each BEST genesis event, add an unmatched PairDataGenesis
//     entry for each model opportunity to forecast that genesis event.
// (3) Loop over the forecast genesis events. For each, search for a
//     BEST track point valid at that time and within the search radius.
// (4) If found, store the matching BEST track storm id.
// (5) If not found, search the CARQ points for a matching storm id.
// (6) If a storm id was found, update the existing PairDataGenesis
//     entry for that storm id with the forecast genesis event.
// (7) If no match was found, add an unmatched PairDataGenesis
//     entry for that forecast genesis event.
//
////////////////////////////////////////////////////////////////////////

void get_genesis_pairs(const TCGenVxOpt       &vx_opt,
                       const ConcatString     &model,
                       const GenesisInfoArray &fga,
                       const GenesisInfoArray &bga,
                       const TrackInfoArray   &ota,
                       PairDataGenesis        &gpd) {
   int i, i_bga;

   // Initialize
   gpd.clear();
   gpd.set_desc(vx_opt.Desc);
   gpd.set_mask(vx_opt.VxMaskName);
   gpd.set_model(model);

   // Filter the BEST genesis events and define model opportunities
   for(i=0; i<bga.n(); i++) {

      // Check filters
      if(!vx_opt.is_keeper(bga[i])) continue;

      // Add pairs for the forecast opportunities
      gpd.add_best_gen(&bga[i],
                       conf_info.FcstSecBeg, conf_info.FcstSecEnd,
                       conf_info.InitFreqHr*sec_per_hour);
   }

   // Loop over the model genesis events looking for pairs.
   for(i=0; i<fga.n(); i++) {
      
      // Search for a BEST track match
      i_bga = find_genesis_match(fga[i], bga, ota,
                                 vx_opt.GenesisMatchRadius);

      // Add the matched genesis pair
      if(!is_bad_data(i_bga)) {
         gpd.add_gen_pair(&fga[i], &bga[i_bga]);
      }
      // Add the unmatched forecast
      else {
         gpd.add_fcst_gen(&fga[i]);
      }
   } // end for i fga

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Score the PairDataGenesis events, using two methods:
// (1) Loop over the PairDataGenesis entries.
// (2) If the forecast is set but not the BEST track, increment the
//     FALSE ALARM counts.
// (2) If the BEST track is set but not the forecast, increment the
//     MISS counts.
// (3) If both are set, but the model initialization time is at or
//     after the BEST track genesis time, DISCARD that case.
// (4) If both are set and the genesis time and location offsets
//     fall within the configurable windows, increment the
//     dev method HIT counts. Otherwise, increment the dev method
//     FALSE ALARM counts.
// (5) If both are set and the difference between to BEST track genesis
//     time and the forecast initialization time is small enough,
//     increment the ops method HIT counts. Otherwise, increment the
//     ops method FALSE ALARM counts.
//
////////////////////////////////////////////////////////////////////////

void do_genesis_ctc(const TCGenVxOpt      &vx_opt,
                    const PairDataGenesis &pairs,
                    GenCTCInfo            &gci) {
   int i, dsec;
   double dist;
   ConcatString case_cs;

   // Loop over the pairs and score them
   for(i=0; i<pairs.n_pair(); i++) {

      const GenesisInfo *fgi = pairs.fcst_gen(i);
      const GenesisInfo *bgi = pairs.best_gen(i);

      case_cs << cs_erase
              << pairs.model() << " "
              << unix_to_yyyymmdd_hhmmss(pairs.init(i))
              << " initialization, "
              << pairs.lead_time(i)/sec_per_hour << " lead";

      // Count the genesis and track points
      if(fgi) gci.add_fcst_gen(*fgi);
      if(bgi) gci.add_best_gen(*bgi);
      
      // Unmatched forecast genesis (FALSE ALARM)
      if(fgi && !bgi) {

         mlog << Debug(4) << case_cs << ", "
              << unix_to_yyyymmdd_hhmmss(fgi->genesis_time())
              << " forecast genesis at ("
              << fgi->lat() << ", " << fgi->lon()
              << ") is a dev and ops FALSE ALARM.\n";

         // Increment the FALSE ALARM count for both methods 
         gci.inc_dev(true, false, fgi, bgi);
         gci.inc_ops(true, false, fgi, bgi);
      }

      // Unmatched BEST genesis (MISS)
      else if(!fgi && bgi) {

         mlog << Debug(4) << case_cs << ", "
              << unix_to_yyyymmdd_hhmmss(bgi->genesis_time())
              << " BEST track " << bgi->storm_id() << " genesis at ("
              << bgi->lat() << ", " << bgi->lon()
              << ") is a dev and ops MISS.\n";

         // Increment the MISS count for both methods 
         gci.inc_dev(false, true, fgi, bgi);
         gci.inc_ops(false, true, fgi, bgi);
      }
      // Matched genesis pairs (DISCARD, HIT, or FALSE ALARM)
      else {

         case_cs << ", " << unix_to_yyyymmdd_hhmmss(bgi->genesis_time())
                 << " BEST track " << bgi->storm_id() << " genesis at ("
                 << bgi->lat() << ", " << bgi->lon() << ") and "
                 << unix_to_yyyymmdd_hhmmss(fgi->genesis_time())
                 << " forecast hour " << fgi->genesis_fhr()
                 << " genesis at (" << fgi->lat() << ", " << fgi->lon() << ")";
         
         // Discard if the forecast init >= BEST genesis
         if(vx_opt.DiscardFlag &&
            fgi->init() >= bgi->genesis_time()) {
            mlog << Debug(4) << "DISCARD " << case_cs
                 << " since the model initialization time is at or "
                 << "after the matching BEST track "
                 << unix_to_yyyymmdd_hhmmss(bgi->genesis_time())
                 << " genesis time.\n";
         }
         // Check for a HIT
         else {

            // Compute time and space offsets
            dsec = fgi->genesis_time() - bgi->genesis_time();
            dist = gc_dist(bgi->lat(), bgi->lon(),
                           fgi->lat(), fgi->lon());

            ConcatString offset_cs;
            offset_cs << "with a genesis time offset of " << dsec/sec_per_hour
                      << " hours and location offset of " << dist << " km.\n";

            // Dev Method:
            // HIT if forecast genesis time and location
            // are within the temporal and spatial windows.
            if(dsec >= vx_opt.GenesisHitBeg &&
               dsec <= vx_opt.GenesisHitEnd &&
               dist <= vx_opt.GenesisHitRadius) {

               mlog << Debug(4) << case_cs
                    << " is a dev method HIT " << offset_cs;

               // Increment the HIT count
               gci.inc_dev(true, true, fgi, bgi);
            }
            else {
               mlog << Debug(4) << case_cs
                    << " is a dev method FALSE ALARM " << offset_cs;

               // Increment the FALSE ALARM count
               gci.inc_dev(true, false, fgi, bgi);
            }
            
            // Ops Method:
            // HIT if forecast init time is close enough to
            // the BEST genesis time.
            
            // Compute time offset
            dsec = bgi->genesis_time() - fgi->init();

            offset_cs << cs_erase
                      << "with an init vs genesis time offset of "
                      << dsec/sec_per_hour << " hours.\n";

            if(dsec <= vx_opt.GenesisInitDSec) {

               mlog << Debug(4) << case_cs
                    << " is an ops method HIT " << offset_cs;

               // Increment the HIT count
               gci.inc_ops(true, true, fgi, bgi);
            }
            else {
               mlog << Debug(4) << case_cs
                    << " is an ops method FALSE ALARM " << offset_cs;

               // Increment the FALSE ALARM count
               gci.inc_ops(true, false, fgi, bgi);
            }
         }
      }
   } // end for i n_pair

   // If requested, count the unique BEST track hit and miss counts
   gci.inc_best_unique();

   if(vx_opt.DevFlag) {
      mlog << Debug(3) << "For filter ("
           << pairs.desc() << ") " << pairs.model()
           << " model, dev method contingency table hits = "
           << gci.CTSDev.cts.fy_oy() << ", false alarms = "
           << gci.CTSDev.cts.fy_on() << ", and misses = "
           << gci.CTSDev.cts.fn_oy() << ".\n";
   }

   if(vx_opt.OpsFlag) {
      mlog << Debug(3) << "For filter ("
           << pairs.desc() << ") " << pairs.model()
           << " model, ops method contingency table hits = "
           << gci.CTSOps.cts.fy_oy() << ", false alarms = "
           << gci.CTSOps.cts.fy_on() << ", and misses = "
           << gci.CTSOps.cts.fn_oy() << ".\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

int find_genesis_match(const GenesisInfo      &fcst_gi,
                       const GenesisInfoArray &bga,
                       const TrackInfoArray   &ota,
                       const double rad) {
   int i, j;
   int i_best = bad_data_int;
   int i_oper = bad_data_int;

   ConcatString case_cs;
   case_cs << fcst_gi.technique() << " "
           << unix_to_yyyymmdd_hhmmss(fcst_gi.init())
           << " initialization, "
           << fcst_gi.genesis_fhr() << " lead, "
           << unix_to_yyyymmdd_hhmmss(fcst_gi.genesis_time())
           << " forecast genesis at (" << fcst_gi.lat() << ", "
           << fcst_gi.lon() << ")";

   // Search the BEST track points for a match
   for(i=0, i_best=bad_data_int;
       i<bga.n() && is_bad_data(i_best);
       i++) {
      for(j=0; j<bga[i].n_points(); j++) {
         if(fcst_gi.is_match(bga[i][j], rad)) {
            i_best = i;
            mlog << Debug(4) << case_cs
                 << " MATCHES BEST track "
                 << bga[i].storm_id() << ".\n";
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
               mlog << Debug(4) << case_cs
                    << " MATCHES operational " << ota[i].technique()
                    << " track " << ota[i].storm_id() << ".\n";
               break;
            }
         }
      } // end for ota

      // Find BEST track for this operational track
      if(!is_bad_data(i_oper)) {
         for(i=0; i<bga.n(); i++) {
            if(bga[i].storm_id() == ota[i_oper].storm_id()) {
               i_best = i;
               break;
            }
         }
      }
   }

   // Check for no match
   if(is_bad_data(i_best)) {
      mlog << Debug(4) << case_cs
           << " has NO MATCH in the BEST or operational tracks.\n";
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

void process_fcst_tracks(const StringArray &files,
                         const StringArray &model_suffix,
                         GenesisInfoArray  &fcst_ga) {
   int i, j, k;
   int n_lines, tot_lines, tot_tracks, n_genesis;
   ConcatString suffix;
   LineDataFile f;
   ATCFTrackLine line;
   TrackInfoArray fcst_ta;
   GenesisInfo fcst_gi;
   bool keep;

   int valid_freq_sec = conf_info.ValidFreqHr*sec_per_hour;

   // Initialize
   fcst_ga.clear();
   tot_lines = tot_tracks = n_genesis = 0;

   // Process each of the input ATCF files
   for(i=0; i<files.n(); i++) {

      // Open the current file
      if(!f.open(files[i].c_str())) {
         mlog << Error << "\nprocess_fcst_tracks() -> "
              << "unable to open file \"" << files[i] << "\"\n\n";
         exit(1);
      }

      // Initialize
      fcst_ta.clear();
      n_lines = 0;

      // Set metadata pointer
      suffix = model_suffix[i];
      line.set_tech_suffix(&suffix);

      // Process the input track lines
      while(f >> line) {

         // Skip off-hour track points
         if((line.valid_hour() % valid_freq_sec) != 0) continue;
   
         n_lines++;
         fcst_ta.add(line);
      }

      // Increment counts
      tot_lines  += n_lines;
      tot_tracks += fcst_ta.n();

      // Close the current file
      f.close();

      // Search the tracks for genesis events
      for(j=0; j<fcst_ta.n(); j++) {

         // Attempt to define genesis
         if(!fcst_gi.set(fcst_ta[j], conf_info.FcstEventInfo)) {
            continue;
         }

         // Check the forecast lead time window
         if(fcst_gi.genesis_lead() < conf_info.FcstSecBeg ||
            fcst_gi.genesis_lead() > conf_info.FcstSecEnd) {
            mlog << Debug(6) << "Skipping genesis event for forecast hour "
                 << fcst_gi.genesis_fhr() << ".\n";
            continue;
         }

         // Check the forecast track minimum duration
         if(fcst_gi.duration() < conf_info.MinDur*sec_per_hour) {
            mlog << Debug(6) << "Skipping genesis event for track duration of "
                 << fcst_gi.duration()/sec_per_hour << ".\n";
            continue;
         }

         // Compute the distance to land
         fcst_gi.set_dland(conf_info.compute_dland(
                           fcst_gi.lat(), -1.0*fcst_gi.lon()));

         // Store the genesis event
         fcst_ga.add(fcst_gi);

      } // end for j

      // Dump out the current number of lines
      mlog << Debug(4) << "[File " << i+1 << " of " << files.n()
           << "] Found " << fcst_ga.n() - n_genesis
           << " forecast genesis events, from " << fcst_ta.n()
           << " tracks, from " << n_lines << " input lines, from file \""
           << files[i] << "\".\n";
      n_genesis = fcst_ga.n();

   } // end for i

   // Dump out the total number of lines
   mlog << Debug(3) << "Found a total of " << fcst_ga.n()
        << " forecast genesis events, from " << tot_tracks
        << " tracks, from " << tot_lines << " input lines, from "
        << files.n() << " input files.\n";

   // Dump out the number of genesis events
   mlog << Debug(2) << "Found " << fcst_ga.n()
        << " forecast genesis events.\n";

   // Dump out very verbose output
   if(mlog.verbosity_level() > 6) {
      mlog << Debug(6) << fcst_ga.serialize_r() << "\n";
   }
   // Dump out track info
   else {
      for(i=0; i<fcst_ga.n(); i++) {
         mlog << Debug(6) << "[Genesis " << i+1 << " of "
              << fcst_ga.n() << "] " << fcst_ga[i].serialize()
              << "\n";
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_best_tracks(const StringArray &files,
                         const StringArray &model_suffix,
                         GenesisInfoArray  &best_ga,
                         TrackInfoArray    &oper_ta) {
   int i, i_bga, n_lines;
   ConcatString suffix, gen_basin, case_cs, storm_id;
   StringArray best_tech, oper_tech;
   TrackInfoArray best_ta;
   GenesisInfo best_gi;
   LineDataFile f;
   ATCFTrackLine line;

   int valid_freq_sec = conf_info.ValidFreqHr*sec_per_hour;

   // Initialize
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
              << "\nprocess_best_tracks() -> "
              << "unable to open file \"" << files[i] << "\"\n\n";
         exit(1);
      }

      // Set metadata pointer
      suffix = model_suffix[i];
      line.set_tech_suffix(&suffix);

      // Process the input track lines
      while(f >> line) {

         // Skip off-hour track points
         if((line.valid_hour() % valid_freq_sec) != 0) continue;
         
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

   // Search the BEST tracks for genesis events
   for(i=0; i<best_ta.n(); i++) {

      // Attempt to define genesis
      if(!best_gi.set(best_ta[i], conf_info.BestEventInfo)) {
         continue;
      }

      // Check for duplicates
      if(best_ga.has_storm(best_gi, i_bga)) {

         // Determine the basin for this genesis event
         gen_basin = conf_info.compute_basin(best_gi.lat(),
                                             -1.0*best_gi.lon());

         case_cs << cs_erase << "For duplicate "
                 << unix_to_yyyymmdd_hhmmss(best_gi.genesis_time()) << " "
                 << best_gi.technique() << " track genesis at ("
                 << best_gi.lat() << ", " << best_gi.lon() << ") in the "
                 << gen_basin << " basin, ";

         // Keep existing storm id and discard the new one
         if(gen_basin == best_ga[i_bga].basin()) {
            mlog << Debug(3)
                 << case_cs << "keep " << best_ga[i_bga].storm_id()
                 << " and discard " << best_gi.storm_id()
                 << ".\n";
            best_ta.erase_storm_id(best_gi.storm_id());
            oper_ta.erase_storm_id(best_gi.storm_id());
            i--;
            continue;
         }
         // Discard the existing storm id and add the new one
         else if(gen_basin == best_gi.basin()) {
            mlog << Debug(3)
                 << case_cs << "keep " << best_gi.storm_id()
                 << " and discard " << best_ga[i_bga].storm_id()
                 << ".\n";
            best_ga.erase_storm_id(best_ga[i_bga].storm_id());
            best_ta.erase_storm_id(best_ga[i_bga].storm_id());
            oper_ta.erase_storm_id(best_ga[i_bga].storm_id());
            i--;
         }
         else {
            mlog << Warning << "\nprocess_best_tracks() -> "
                 << case_cs << "neither " << best_ga[i_bga].storm_id()
                 << " nor " << best_gi.storm_id()
                 << " matches the basin!\n\n";
            continue;
         }
      }

      // Compute the distance to land
      best_gi.set_dland(conf_info.compute_dland(
                        best_gi.lat(), -1.0*best_gi.lon()));

      // Store the genesis event
      best_ga.add(best_gi);

   } // end for i

   // Dump out the number of genesis events
   mlog << Debug(2) << "Found " << best_ga.n()
        << " BEST genesis events.\n";

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
   n_rows = 1 + 6 * n_model * conf_info.n_vx();
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
         n_rows = 1 + 2 * n_model * conf_info.n_vx();
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

void setup_nc_file() {

   // Build the output NetCDF file name
   out_nc_file << cs_erase << out_base << "_pairs.nc";

   // Create a new NetCDF file and open it
   nc_out = open_ncfile(out_nc_file.c_str(), true);

   if(IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_nc_file << "\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, out_nc_file.c_str(), program_name);

   // Add the projection information
   Grid grid = conf_info.NcOutGrid;
   write_netcdf_proj(nc_out, grid);

   // Define Dimensions
   lat_dim = add_dim(nc_out,"lat", (long) grid.ny());
   lon_dim = add_dim(nc_out,"lon", (long) grid.nx());

   // Add the lat/lon variables
   if(conf_info.NcInfo.do_latlon) {
      write_netcdf_latlon(nc_out, &lat_dim, &lon_dim, grid);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ctc_info(GenCTCInfo &gci) {
   ConcatString dev_name("GENESIS_DEV");
   ConcatString ops_name("GENESIS_OPS");

   // Setup header columns
   shc.set_model(gci.Model.c_str());
   shc.set_desc(gci.VxOpt->Desc.c_str());
   if(gci.VxOpt->Lead.n() == 1) {
      shc.set_fcst_lead_sec(gci.VxOpt->Lead[0]);
   }
   shc.set_fcst_valid_beg(gci.VxOpt->ValidBeg != 0 ?
                          gci.VxOpt->ValidBeg : gci.FcstBeg);
   shc.set_fcst_valid_end(gci.VxOpt->ValidEnd != 0 ?
                          gci.VxOpt->ValidEnd : gci.FcstEnd);
   shc.set_obs_valid_beg(gci.VxOpt->ValidBeg != 0 ?
                         gci.VxOpt->ValidBeg : gci.BestBeg);
   shc.set_obs_valid_end(gci.VxOpt->ValidEnd != 0 ?
                         gci.VxOpt->ValidEnd : gci.BestEnd);
   shc.set_obtype(conf_info.BestEventInfo.Technique.c_str());
   if(!gci.VxOpt->VxMaskName.empty()) {
      shc.set_mask(gci.VxOpt->VxMaskName.c_str());
   }

   // Write out FHO
   if(gci.VxOpt->output_map(stat_fho) != STATOutputType_None) {

      if(gci.VxOpt->DevFlag) {
         shc.set_fcst_var(dev_name);
         shc.set_obs_var (dev_name);
         write_fho_row(shc, gci.CTSDev,
                       gci.VxOpt->OutputMap.at(stat_fho),
                       stat_at, i_stat_row,
                       txt_at[i_fho], i_txt_row[i_fho]);
      }

      if(gci.VxOpt->OpsFlag) {
         shc.set_fcst_var(ops_name);
         shc.set_obs_var (ops_name);
         write_fho_row(shc, gci.CTSOps,
                       gci.VxOpt->OutputMap.at(stat_fho),
                       stat_at, i_stat_row,
                       txt_at[i_fho], i_txt_row[i_fho]);
      }
   }

   // Write out CTC
   if(gci.VxOpt->output_map(stat_ctc) != STATOutputType_None) {

      if(gci.VxOpt->DevFlag) {
         shc.set_fcst_var(dev_name);
         shc.set_obs_var (dev_name);
         write_ctc_row(shc, gci.CTSDev,
                       gci.VxOpt->OutputMap.at(stat_ctc),
                       stat_at, i_stat_row,
                       txt_at[i_ctc], i_txt_row[i_ctc]);
      }

      if(gci.VxOpt->OpsFlag) {
         shc.set_fcst_var(ops_name);
         shc.set_obs_var (ops_name);
         write_ctc_row(shc, gci.CTSOps,
                       gci.VxOpt->OutputMap.at(stat_ctc),
                       stat_at, i_stat_row,
                       txt_at[i_ctc], i_txt_row[i_ctc]);
      } 
   }

   // Write out CTS
   if(gci.VxOpt->output_map(stat_cts) != STATOutputType_None) {

      if(gci.VxOpt->DevFlag) {
         gci.CTSDev.compute_stats();
         gci.CTSDev.compute_ci();

         shc.set_fcst_var(dev_name);
         shc.set_obs_var (dev_name);
         write_cts_row(shc, gci.CTSDev,
                       gci.VxOpt->OutputMap.at(stat_cts),
                       stat_at, i_stat_row,
                       txt_at[i_cts], i_txt_row[i_cts]);
      }

      if(gci.VxOpt->OpsFlag) {
         gci.CTSOps.compute_stats();
         gci.CTSOps.compute_ci();

         shc.set_fcst_var(ops_name);
         shc.set_obs_var (ops_name);
         write_cts_row(shc, gci.CTSOps,
                       gci.VxOpt->OutputMap.at(stat_cts),
                       stat_at, i_stat_row,
                       txt_at[i_cts], i_txt_row[i_cts]);
      } 
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nc(GenCTCInfo &gci) {
   int i;
   ConcatString var_name, long_name;
   unixtime valid_beg, valid_end;

   // Allocate memory
   float *data = (float *) 0;
   int nx = gci.NcOutGrid->nx();
   int ny = gci.NcOutGrid->ny();
   data = new float [nx*ny];

   // Ordered list of output types
   std::vector<string> nc_str {
      fgen_str,      ftrk_str,      bgen_str,      btrk_str,
      fdev_fyoy_str, fdev_fyon_str, bdev_fyoy_str, bdev_fnoy_str,
      fops_fyoy_str, fops_fyon_str, bops_fyoy_str, bops_fnoy_str
   };
   
   // Loop over vector of output types
   for(i=0; i<nc_str.size(); i++) {

      // Continue if no map entry is present
      if(gci.DpMap.count(nc_str[i]) == 0) continue;

      // Setup strings for each output type
      if(nc_str[i] == fgen_str) {
         var_name  << cs_erase << gci.VxOpt->Desc << "_" << gci.Model << "_GENESIS";
         long_name = "Forecast genesis events";
         valid_beg = gci.FcstBeg;
         valid_end = gci.FcstEnd;
      }
      else if(nc_str[i] == ftrk_str) {
         var_name  << cs_erase << gci.VxOpt->Desc << "_" << gci.Model << "_TRACKS";
         long_name = "Forecast track points";
         valid_beg = gci.FcstBeg;
         valid_end = gci.FcstEnd;
      }
      else if(nc_str[i] == bgen_str) {
         var_name  << cs_erase << gci.VxOpt->Desc << "_BEST_GENESIS";
         long_name = "Best track genesis events";
         valid_beg = gci.BestBeg;
         valid_end = gci.BestEnd;
      }
      else if(nc_str[i] == btrk_str) {
         var_name  << cs_erase << gci.VxOpt->Desc << "_BEST_TRACKS";
         long_name = "Best track points";
         valid_beg = gci.BestBeg;
         valid_end = gci.BestEnd;
      }
      else if(nc_str[i] == fdev_fyoy_str) {
         var_name  << cs_erase << gci.VxOpt->Desc << "_" << gci.Model << "_DEV_FY_OY";
         long_name = "Forecast genesis development method hits";
         valid_beg = gci.FcstBeg;
         valid_end = gci.FcstEnd;
      }
      else if(nc_str[i] == fdev_fyon_str) {
         var_name  << cs_erase << gci.VxOpt->Desc << "_" << gci.Model << "_DEV_FY_ON";
         long_name = "Forecast genesis development method false alarms";
         valid_beg = gci.FcstBeg;
         valid_end = gci.FcstEnd;
      }
      else if(nc_str[i] == bdev_fyoy_str) {
         var_name  << cs_erase << gci.VxOpt->Desc << "_" << gci.Model << "_BEST_DEV_FY_OY";
         long_name = "Best track genesis development method hits";
         valid_beg = gci.BestBeg;
         valid_end = gci.BestEnd;
      }
      else if(nc_str[i] == bdev_fnoy_str) {
         var_name  << cs_erase << gci.VxOpt->Desc << "_" << gci.Model << "_BEST_DEV_FN_OY";
         long_name = "Best track genesis development method misses";
         valid_beg = gci.BestBeg;
         valid_end = gci.BestEnd;
      }
      else if(nc_str[i] == fops_fyoy_str) {
         var_name  << cs_erase << gci.VxOpt->Desc << "_" << gci.Model << "_OPS_FY_OY";
         long_name = "Forecast genesis operational method hits";
         valid_beg = gci.FcstBeg;
         valid_end = gci.FcstEnd;
      }
      else if(nc_str[i] == fops_fyon_str) {
         var_name  << cs_erase << gci.VxOpt->Desc << "_" << gci.Model << "_OPS_FY_ON";
         long_name = "Forecast genesis operational method false alarms";
         valid_beg = gci.FcstBeg;
         valid_end = gci.FcstEnd;
      }
      else if(nc_str[i] == bops_fyoy_str) {
         var_name  << cs_erase << gci.VxOpt->Desc << "_" << gci.Model << "_BEST_OPS_FY_OY";
         long_name = "Best track genesis operational method hits";
         valid_beg = gci.BestBeg;
         valid_end = gci.BestEnd;
      }
      else if(nc_str[i] == bops_fnoy_str) {
         var_name  << cs_erase << gci.VxOpt->Desc << "_" << gci.Model << "_BEST_OPS_FN_OY";
         long_name = "Best track genesis operational method misses";
         valid_beg = gci.BestBeg;
         valid_end = gci.BestEnd;
      }

      // Skip variable names that have already been written
      if(nc_var_sa.has(var_name)) continue;

      mlog << Debug(4) << "Writing output variable \""
           << var_name << "\".\n";

      int n, x, y;
      ConcatString cs;
      NcVar nc_var;

      // Otherwise, add to the list of previously defined variables
      nc_var_sa.add(var_name);

      // Define the variable
      nc_var = add_var(nc_out, (string) var_name,
                       ncFloat, lat_dim, lon_dim,
                       conf_info.compression_level());

      // Add variable attributes
      add_att(&nc_var, "name", nc_var.getName());
      add_att(&nc_var, "long_name", long_name);
      add_att(&nc_var, "model", gci.Model);
      add_att(&nc_var, "desc", gci.VxOpt->Desc);
      add_att(&nc_var, "valid_beg", unix_to_yyyymmdd_hhmmss(valid_beg));
      add_att(&nc_var, "valid_end", unix_to_yyyymmdd_hhmmss(valid_end));

      // Reset memory
      memset(data, 0, nx*ny);
             
      // Store the data
      for(x=0; x<nx; x++) {
         for(y=0; y<ny; y++) {
            n = DefaultTO.two_to_one(nx, ny, x, y);
            data[n] = gci.DpMap[(nc_str[i])].get(x, y);
         } // end for y
      } // end for x

      // Write out the data
      if(!put_nc_data_with_dims(&nc_var, &data[0], ny, nx)) {
         mlog << Error << "\nwrite_nc() -> "
              << "error writing NetCDF variable name " << var_name
              << "\n\n";
         exit(1);
      }
   }

   // Deallocate and clean up
   if(data) { delete [] data; data = (float *) 0; }

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
