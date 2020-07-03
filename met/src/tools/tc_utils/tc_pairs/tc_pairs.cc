// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   tc_pairs.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    03/14/12  Halley Gotway   New
//   001    05/20/14  Halley Gotway   Refine consensus to ensure the
//                                    presence of required members at
//                                    each lead time.
//   002    07/15/14  Halley Gotway   Simplify Interp12 logic.
//   003    07/15/14  Halley Gotway   Fix bug in init_beg/end logic.
//   004    02/10/16  Halley Gotway   Prior to calling acerr_ rescale
//                    longitudes from [-180, 180] to [0, 360] for tracks
//                    crossing the international date line.
//   005    02/10/16  Halley Gotway   Add support for analysis tracks.
//   006    06/01/16  Halley Gotway   Apply interp12 logic to tracks
//                    with ATCF id's ending in '3'.
//   007    06/01/16  Halley Gotway   Add support for EDecks.
//   008    09/29/16  Halley Gotway   Add DESC output column.
//   009    03/09/17  Halley Gotway   Define BEST track time step.
//   010    03/02/17  Win             MET-667 Add support for tracks
//                    that contain all required lead times.
//   011    07/27/18  Halley Gotway   Support masks defined by
//                    the gen_vx_mask tool.
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

#include "tc_pairs.h"

#include "vx_nc_util.h"
#include "vx_tc_util.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_log.h"

#include "met_file.h"

////////////////////////////////////////////////////////////////////////

static const int mxp  = 1000;
static const int nvtx = 10;

extern "C" {
   void acerr_(float [mxp], float [mxp], float [mxp], float [mxp],
           int *, float [mxp], float [mxp], int *);
   void oclip_(char [2], int *, int *, int *, int *,
           float *,  float *,  float *, float *, float *,
           float *,  float *,  float *, float *, float *,
           float [nvtx], float [nvtx], float [nvtx]);
   void oclip5_(char [2], int *, int *, int *, int *,
           float *,  float *,  float *, float *, float *,
           float *,  float *,  float *, float *, float *,
           float [nvtx], float [nvtx], float [nvtx]);
   void oclipd5_(char [2], int *, int *, int *, int *,
           float *,  float *,  float *, float *, float *,
           float *,  float *,  float *, float *, float *,
           float [nvtx], float [nvtx], float [nvtx]);
   void oclipd5_test_(char [2], int *, int *, int *, int *,
           float *,  float *,  float *, float *, float *,
           float *,  float *,  float *, float *, float *,
           float [nvtx], float [nvtx], float [nvtx]);
   void btclip_(char [2], int *, int *, int *, int *,
           float [mxp],  float [mxp],  float [mxp],
           float [mxp],  float [mxp],  float [mxp],
           float [nvtx], float [nvtx], float [nvtx]);
   void btclip5_(char [2], int *, int *, int *, int *, int *,
           float [mxp],  float [mxp],  float [mxp],
           float [mxp],  float [mxp],  float [mxp],
           float [nvtx], float [nvtx], float [nvtx]);
   void btclipd5_(char [2], int *, int *, int *, int *, int *,
           float [mxp],  float [mxp],  float [mxp],
           float [mxp],  float [mxp],  float [mxp],
           float [nvtx], float [nvtx], float [nvtx]);
   void btclipa_(char [2], int *, int *, int *, int *,
           float [mxp],  float [mxp],  float [mxp],
           float [mxp],  float [mxp],  float [mxp],
           float [nvtx], float [nvtx], float [nvtx]);
}

////////////////////////////////////////////////////////////////////////

static void   process_command_line (int, char **);
static void   process_decks        ();
static void   process_bdecks       (TrackInfoArray &);
static void   process_adecks       (const TrackInfoArray &);
static void   process_edecks       (const TrackInfoArray &);
static void   get_atcf_files       (const StringArray &,
                                    const StringArray &,
                                    StringArray &, StringArray &);
static void   process_track_files  (const StringArray &,
                                    const StringArray &,
                                    TrackInfoArray &, bool, bool);
static void   process_prob_files   (const StringArray &,
                                    const StringArray &,
                                    ProbInfoArray &);
static bool   is_keeper            (const ATCFLineBase *);
static void   filter_tracks        (TrackInfoArray &);
static void   filter_probs         (ProbInfoArray &);
static bool   check_masks          (const MaskPoly &, const Grid &,
                                    const MaskPlane &,
                                    double lat, double lon);
static void   derive_interp12      (TrackInfoArray &);
static int    derive_consensus     (TrackInfoArray &);
static int    derive_lag           (TrackInfoArray &);
static int    derive_baseline      (TrackInfoArray &, const TrackInfoArray &);
static void   derive_baseline_model(const ConcatString &,
                                    const TrackInfo &, int,
                                    TrackInfoArray &);
static void   process_match        (const TrackInfo &, const TrackInfo &,
                                    TrackPairInfoArray &);
static double compute_dland        (double, double);
static void   compute_track_err    (const TrackInfo &, const TrackInfo &,
                                    TimeArray &, NumArray &, NumArray &,
                                    NumArray &, NumArray &, NumArray &);
static void   process_watch_warn   (TrackPairInfoArray &);
static void   write_tracks         (const TrackPairInfoArray &);
static void   write_prob_rirw      (const ProbRIRWPairInfoArray &);
static void   setup_table          (AsciiTable &);
static void   usage                ();
static void   set_adeck            (const StringArray &);
static void   set_edeck            (const StringArray &);
static void   set_bdeck            (const StringArray &);
static void   set_atcf_source      (const StringArray &,
                                    StringArray &, StringArray &);
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

   // Process the ATCF deck files and write output
   process_decks();

   // List the output files
   for(int i=0; i<out_files.n_elements(); i++) {
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
   out_base = "./tc_pairs";

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add function calls for the arguments
   cline.add(set_adeck,     "-adeck", -1);
   cline.add(set_edeck,     "-edeck", -1);
   cline.add(set_bdeck,     "-bdeck", -1);
   cline.add(set_config,    "-config", 1);
   cline.add(set_out,       "-out",    1);
   cline.add(set_logfile,   "-log",    1);
   cline.add(set_verbosity, "-v",      1);

   // Parse the command line
   cline.parse();

   // Check for the minimum number of arguments
   if((adeck_source.n_elements() == 0 &&
       edeck_source.n_elements() == 0) ||
      bdeck_source.n_elements()  == 0  ||
      config_file.length()       == 0) {
      mlog << Error
           << "\nprocess_command_line(int argc, char **argv) -> "
           << "You must specify at least one source of ADECK or EDECK "
           << "data, BDECK data, and the config file using the "
           << "\"-adeck\", \"-edeck\", \"-bdeck\", and \"-config\" "
           << "command line options.\n\n";
      usage();
   }

   // List the input ADECK track files
   for(i=0; i<adeck_source.n_elements(); i++) {
      mlog << Debug(1)
           << "[Source " << i+1 << " of " << adeck_source.n_elements()
           << "] ADECK Source: " << adeck_source[i] << ", Model Suffix: "
           << adeck_model_suffix[i] << "\n";
   }

   // List the input EDECK track files
   for(i=0; i<edeck_source.n_elements(); i++) {
      mlog << Debug(1)
           << "[Source " << i+1 << " of " << edeck_source.n_elements()
           << "] EDECK Source: " << edeck_source[i] << ", Model Suffix: "
           << edeck_model_suffix[i] << "\n";
   }

   // List the input BDECK track files
   for(i=0; i<bdeck_source.n_elements(); i++) {
      mlog << Debug(1)
           << "[Source " << i+1 << " of " << bdeck_source.n_elements()
           << "] BDECK Source: " << bdeck_source[i] << ", Model Suffix: "
           << bdeck_model_suffix[i] << "\n";
   }

   // Create the default config file name
   default_config_file = replace_path(default_config_filename);

   // List the config files
   mlog << Debug(1)
        << "Config File Default: " << default_config_file << "\n"
        << "Config File User: " << config_file << "\n";

   // Read the config files
   conf_info.read_config(default_config_file.c_str(), config_file.c_str());

   // Load the distance to land data file
   if(dland_dp.is_empty()) {
      load_dland(conf_info.DLandFile, dland_grid, dland_dp);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_decks() {
   TrackInfoArray bdeck_tracks;

   // Process BDECK files
   process_bdecks(bdeck_tracks);

   // Process ADECK files
   if(adeck_source.n_elements() > 0) {
      process_adecks(bdeck_tracks);
   }

   // Process EDECK files
   if(edeck_source.n_elements() > 0) {
      process_edecks(bdeck_tracks);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_bdecks(TrackInfoArray &bdeck_tracks) {
   StringArray files, files_model_suffix;

   // Initialize
   bdeck_tracks.clear();

   // Get the list of track files
   get_atcf_files(bdeck_source, bdeck_model_suffix,
                  files, files_model_suffix);

   mlog << Debug(2)
        << "Processing " << files.n_elements() << " BDECK file(s).\n";
   process_track_files(files, files_model_suffix, bdeck_tracks, false,
                       (conf_info.AnlyTrack == TrackType_BDeck ||
                        conf_info.AnlyTrack == TrackType_Both));
   mlog << Debug(2)
        << "Found " << bdeck_tracks.n_tracks() << " BDECK track(s).\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void process_adecks(const TrackInfoArray &bdeck_tracks) {
   StringArray files, files_model_suffix;
   TrackInfoArray adeck_tracks;
   TrackPairInfoArray pairs;
   int i, j, n_match;

   // Get the list of track files
   get_atcf_files(adeck_source, adeck_model_suffix,
                  files, files_model_suffix);

   mlog << Debug(2)
        << "Processing " << files.n_elements() << " ADECK file(s).\n";
   process_track_files(files, files_model_suffix, adeck_tracks, true,
                       (conf_info.AnlyTrack == TrackType_ADeck ||
                        conf_info.AnlyTrack == TrackType_Both));

   //
   // Derive new track types
   //

   // Handle 12-hourly interpolated models
   if(conf_info.Interp12) {
      mlog << Debug(2)
           << "Deriving 12-hour interpolated ADECK tracks.\n";
      derive_interp12(adeck_tracks);
   }

   // Derive consensus forecasts from the ADECK tracks
   mlog << Debug(2)
        << "Deriving " << conf_info.NConsensus
        << " ADECK consensus model(s).\n";
   i = derive_consensus(adeck_tracks);
   mlog << Debug(2)
        << "Added " << i << " ADECK consensus tracks(s).\n";

   // Derive lag forecasts from the ADECK tracks
   mlog << Debug(2)
        << "Deriving " << conf_info.LagTime.n_elements()
        << " ADECK lag model(s).\n";
   i = derive_lag(adeck_tracks);
   mlog << Debug(2)
        << "Added " << i << " ADECK lag tracks(s).\n";

   // Derive CLIPER/SHIFOR forecasts from the ADECK/BDECK tracks
   mlog << Debug(2)
        << "Deriving " << conf_info.OperBaseline.n_elements() +
                          conf_info.BestBaseline.n_elements()
        << " CLIPER/SHIFOR baseline model(s).\n";
   i = derive_baseline(adeck_tracks, bdeck_tracks);
   mlog << Debug(2)
        << "Added " << i << " CLIPER/SHIFOR baseline track(s).\n";

   // Filter the ADECK tracks using the config file information
   mlog << Debug(2)
        << "Filtering " << adeck_tracks.n_tracks()
        << " ADECK tracks based on config file settings.\n";
   filter_tracks(adeck_tracks);

   //
   // Loop through the ADECK tracks and find a matching BDECK track
   //

   mlog << Debug(2)
        << "Matching " << adeck_tracks.n_tracks() << " ADECK tracks to "
        << bdeck_tracks.n_tracks() << " BDECK tracks.\n";

   for(i=0; i<adeck_tracks.n_tracks(); i++) {

      for(j=0,n_match=0; j<bdeck_tracks.n_tracks(); j++) {

         // Check if the BDECK track matches the current ADECK track
         if(adeck_tracks[i].is_match(bdeck_tracks[j])) {
            n_match++;
            mlog << Debug(4)
                 << "[Track " << i+1 << "] ADECK track " << i+1
                 << " matches BDECK track " << j+1 << ":\n"
                 << "    ADeck: " << adeck_tracks[i].serialize() << "\n"
                 << "    BDeck: " << bdeck_tracks[j].serialize() << "\n";

            // Process the matching tracks
            process_match(adeck_tracks[i], bdeck_tracks[j], pairs);
         }
      } // end for j

      // Dump the number of matching tracks
      mlog << Debug(3)
           << "[Track " << i+1 << "] ADECK track " << i+1 << " matches "
           << n_match << " BDECK track(s).\n";

   } // end for i

   // Add the watch/warning information to the matched track pairs
   process_watch_warn(pairs);

   // Dump out very verbose output
   if(mlog.verbosity_level() >= 5) {
      mlog << Debug(5)
           << pairs.serialize_r() << "\n";
   }

   // Write out the track pairs
   write_tracks(pairs);

   return;
}

////////////////////////////////////////////////////////////////////////

void process_edecks(const TrackInfoArray &bdeck_tracks) {
   StringArray files, files_model_suffix;
   ProbInfoArray edeck_probs;
   ProbRIRWPairInfo cur_ri;
   ProbRIRWPairInfoArray prob_rirw_pairs;
   int n_match, i, j;

   // Get the list of ATCF files
   get_atcf_files(edeck_source, edeck_model_suffix,
                  files, files_model_suffix);

   mlog << Debug(2)
        << "Processing " << files.n_elements()
        << " EDECK file(s).\n";
   process_prob_files(files, files_model_suffix, edeck_probs);

   // Filter the EDECK tracks using the config file information
   mlog << Debug(2)
        << "Filtering " << edeck_probs.n_probs()
        << " probabilities based on config file settings.\n";
   filter_probs(edeck_probs);

   //
   // Match BDECK tracks to EDECK probabilities
   //

   mlog << Debug(2)
        << "Matching " << edeck_probs.n_probs()
        << " EDECK probabilities to "
        << bdeck_tracks.n_tracks() << " BDECK tracks.\n";

   for(i=0; i<edeck_probs.n_probs(); i++) {

      for(j=0,n_match=0; j<bdeck_tracks.n_tracks(); j++) {

         // Check if the BDECK track matches the current EDECK
         if(edeck_probs[i]->is_match(bdeck_tracks[j])) {

            // Attempt to store the pair
            if(!cur_ri.set(edeck_probs.prob_rirw(i), bdeck_tracks[j])) continue;

            mlog << Debug(4)
                 << "[Prob " << i+1 << "] EDECK probability " << i+1
                 << " matches BDECK track " << j+1 << ":\n"
                 << "    EDeck: " << edeck_probs[i]->serialize() << "\n"
                 << "    BDeck: " << bdeck_tracks[j].serialize() << "\n";

            // Compute the distances to land
            cur_ri.set_adland(compute_dland(cur_ri.prob_rirw().lat(), -1.0*cur_ri.prob_rirw().lon()));
            cur_ri.set_bdland(compute_dland(cur_ri.blat(),            -1.0*cur_ri.blon()));

            // Store the current pair
            prob_rirw_pairs.add(cur_ri);

            // Increment the match counter
            n_match++;
         }
      } // end for j

      // Dump the number of matching tracks
      mlog << Debug(3)
           << "[Prob " << i+1 << "] EDECK probability " << i+1
           << " matches " << n_match << " BDECK track(s).\n";

   } // end for i

   // Dump out very verbose output
   if(mlog.verbosity_level() >= 5) {
      mlog << Debug(5)
           << prob_rirw_pairs.serialize_r() << "\n";
   }

   // Write out the ProbRIRW pairs
   if(prob_rirw_pairs.n_pairs() > 0) write_prob_rirw(prob_rirw_pairs);

   return;
}

////////////////////////////////////////////////////////////////////////

void get_atcf_files(const StringArray &source,
                    const StringArray &model_suffix,
                    StringArray &files,
                    StringArray &files_model_suffix) {
   StringArray cur_source, cur_files;
   int i, j;

   if(source.n_elements() != model_suffix.n_elements()) {
      mlog << Error
           << "\nget_atcf_files() -> "
           << "the source and suffix arrays must be equal length!\n\n";
      exit(1);
   }

   // Initialize
   files.clear();
   files_model_suffix.clear();

   // Build list of files and corresponding model suffix list
   for(i=0; i<source.n_elements(); i++) {
      cur_source.clear();
      cur_source.add(source[i]);
      cur_files = get_filenames(cur_source, NULL, atcf_suffix);

      for(j=0; j<cur_files.n_elements(); j++) {
         files.add(cur_files[j]);
         files_model_suffix.add(model_suffix[i]);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_track_files(const StringArray &files,
                         const StringArray &model_suffix,
                         TrackInfoArray &tracks, bool check_keep,
                         bool check_anly) {
   int i, cur_read, cur_add, tot_read, tot_add;
   LineDataFile f;
   ConcatString suffix;
   ATCFTrackLine line;

   // Initialize
   tracks.clear();

   // Initialize counts
   tot_read = tot_add = 0;

   // Set metadata pointers
   line.set_basin_map(&conf_info.BasinMap);
   line.set_best_technique(&conf_info.BestTechnique);
   line.set_oper_technique(&conf_info.OperTechnique);

   // Process each of the input ATCF files
   for(i=0; i<files.n_elements(); i++) {

      // Open the current file
      if(!f.open(files[i].c_str())) {
         mlog << Error
              << "\nprocess_track_files() -> "
              << "unable to open file \"" << files[i] << "\"\n\n";
         exit(1);
      }

      // Initialize counts
      cur_read = cur_add = 0;

      // Set metadata pointer
      suffix = model_suffix[i];
      line.set_tech_suffix(&suffix);

      // Read each line in the file
      while(f >> line) {

         // Increment the line counts
         cur_read++;
         tot_read++;

         // Check the keep status if requested
         if(check_keep && !is_keeper(&line)) continue;

         // Attempt to add the current line to the TrackInfoArray
         if(tracks.add(line, conf_info.CheckDup, check_anly)) {
            cur_add++;
            tot_add++;
         }
      }

      // Dump out the current number of lines
      mlog << Debug(4)
           << "[File " << i+1 << " of " << files.n_elements()
           << "] Used " << cur_add << " of " << cur_read
           << " lines read from file \"" << files[i] << "\"\n";

      // Close the current file
      f.close();

   } // end for i

   // Dump out the total number of lines
   mlog << Debug(3)
        << "Used " << tot_add << " of " << tot_read
        << " lines read from " << files.n_elements() << " file(s).\n";

   // Dump out the track information
   mlog << Debug(3)
        << "Identified " << tracks.n_tracks() << " track(s).\n";

   // Dump out very verbose output
   if(mlog.verbosity_level() >= 5) {
      mlog << Debug(5)
           << tracks.serialize_r() << "\n";
   }
   // Dump out track info
   else {
      for(i=0; i<tracks.n_tracks(); i++) {
         mlog << Debug(4)
              << "[Track " << i+1 << " of " << tracks.n_tracks()
              << "] " << tracks[i].serialize() << "\n";
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_prob_files(const StringArray &files,
                        const StringArray &model_suffix,
                        ProbInfoArray &probs) {
   int i, cur_read, cur_add, tot_read, tot_add;
   LineDataFile f;
   ConcatString suffix;
   ATCFProbLine line;

   // Initialize
   probs.clear();

   // Initialize counts
   tot_read = tot_add = 0;

   // Set metadata pointers
   line.set_basin_map(&conf_info.BasinMap);
   line.set_best_technique(&conf_info.BestTechnique);
   line.set_oper_technique(&conf_info.OperTechnique);

   // Process each of the input ATCF files
   for(i=0; i<files.n_elements(); i++) {

      // Open the current file
      if(!f.open(files[i].c_str())) {
         mlog << Error
              << "\nprocess_prob_files() -> "
              << "unable to open file \"" << files[i] << "\"\n\n";
         exit(1);
      }

      // Initialize counts
      cur_read = cur_add = 0;

      // Set metadata pointer
      suffix = model_suffix[i];
      line.set_tech_suffix(&suffix);

      // Read each line in the file
      while(f >> line) {

         // Increment the line counts
         cur_read++;
         tot_read++;

         // Check the keep status
         if(!is_keeper(&line)) continue;

         // Attempt to add the current line to ProbInfoArray
         if(probs.add(line, conf_info.CheckDup)) {
            cur_add++;
            tot_add++;
         }
      }

      // Dump out the current number of lines
      mlog << Debug(4)
           << "[File " << i+1 << " of " << files.n_elements()
           << "] Used " << cur_add << " of " << cur_read
           << " lines read from file \"" << files[i] << "\"\n";

      // Close the current file
      f.close();

   } // end for i

   // Dump out the total number of lines
   mlog << Debug(3)
        << "Used " << tot_add << " of " << tot_read
        << " lines read from " << files.n_elements() << " file(s).\n";

   // Dump out the track information
   mlog << Debug(3)
        << "Identified " << probs.n_probs() << " probabilities.\n";

   // Dump out very verbose output
   if(mlog.verbosity_level() >= 5) {
      mlog << Debug(5)
           << probs.serialize_r() << "\n";
   }
   // Dump out track info
   else {
      for(i=0; i<probs.n_probs(); i++) {
         mlog << Debug(4)
              << "[Prob " << i+1 << " of " << probs.n_probs()
              << "] " << probs[i]->serialize() << "\n";
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Check if the ATCFLineBase should be kept.  Only check those columns
// that remain constant across the entire track:
//    model, storm id, basin, cyclone, and init time
//
////////////////////////////////////////////////////////////////////////

bool is_keeper(const ATCFLineBase * line) {
   bool keep = true;
   int m, d, y, h, mm, s;

   // Decompose warning time
   unix_to_mdyhms(line->warning_time(), m, d, y, h, mm, s);

   // Check model
   if(conf_info.Model.n_elements() > 0 &&
      !conf_info.Model.has(line->technique()))
      keep = false;

   // Check storm id
   else if(conf_info.StormId.n_elements() > 0 &&
           !has_storm_id(conf_info.StormId, line->basin(),
                         line->cyclone_number(), line->warning_time()))
      keep = false;

   // Check basin
   else if(conf_info.Basin.n_elements() > 0 &&
           !conf_info.Basin.has(line->basin()))
      keep = false;

   // Check cyclone
   else if(conf_info.Cyclone.n_elements() > 0 &&
           !conf_info.Cyclone.has(line->cyclone_number()))
      keep = false;

   // Initialization time window
   else if((conf_info.InitBeg > 0 &&
            conf_info.InitBeg > line->warning_time()) ||
           (conf_info.InitEnd > 0 &&
            conf_info.InitEnd < line->warning_time()) ||
           (conf_info.InitInc.n_elements() > 0 &&
            !conf_info.InitInc.has(line->warning_time())) ||
           (conf_info.InitExc.n_elements() > 0 &&
            conf_info.InitExc.has(line->warning_time())))
      keep = false;

   // Initialization hour
   else if(conf_info.InitHour.n_elements() > 0 &&
           !conf_info.InitHour.has(hms_to_sec(h, mm, s)))
      keep = false;

   // Return the keep status
   return(keep);
}

////////////////////////////////////////////////////////////////////////

void filter_tracks(TrackInfoArray &tracks) {
   int i, j;
   int n_name, n_vld, n_mask_init, n_mask_vld, n_req_lead;
   bool status;
   TrackInfoArray t = tracks;

   // Initialize
   tracks.clear();
   n_name = n_vld = n_mask_init = n_mask_vld = n_req_lead = 0;

   // Loop through the tracks and determine which should be retained
   // The is_keeper() function has already filtered by model, storm id,
   // basin, cyclone, initialization time, and initialization hour.
   for(i=0; i<t.n_tracks(); i++) {

      // Check storm name
      if(conf_info.StormName.n_elements() > 0 &&
         !conf_info.StormName.has(t[i].storm_name())) {
         mlog << Debug(4)
              << "Discarding track " << i+1 << " for storm name mismatch: "
              << t[i].storm_name() << "\n";
         n_name++;
         continue;
      }

      // Valid time window
      if((conf_info.ValidBeg > 0 &&
          conf_info.ValidBeg > t[i].valid_min()) ||
         (conf_info.ValidEnd > 0 &&
          conf_info.ValidEnd < t[i].valid_max())) {
         mlog << Debug(4)
              << "Discarding track " << i+1 << " for falling outside the "
              << "valid time window: "
              <<  unix_to_yyyymmdd_hhmmss(t[i].valid_min()) << " to "
              <<  unix_to_yyyymmdd_hhmmss(t[i].valid_max()) << "\n";
         n_vld++;
         continue;
      }

      // MET-667 Incorporate support for required lead times.
      // These are used in determining whether to keep or discard a track; keep a track
      // if all the required lead times are present.  If no required lead times are
      // specified in the config file, then ignore checking and proceed as usual.
      if(conf_info.LeadReq.n_elements() > 0) {

         // Loop over the required lead times
         for(j=0, status=true; j<conf_info.LeadReq.n_elements(); j++) {

            // If required lead time is missing, break out
            if(t[i].lead_index(conf_info.LeadReq[j]) == -1) {
               status = false;
               break;
            }
         }

         // For bad status, discard this track and increment counter
         if(!status) {
            mlog << Debug(4)
                 << "Discarding track " << i+1
                 << " for not containing all required lead times. \n";
            n_req_lead++;
            continue;
         }
      }

      // Initialization location mask
      if(conf_info.InitMaskName.nonempty() &&
         !check_masks(conf_info.InitPolyMask,
                      conf_info.InitGridMask,
                      conf_info.InitAreaMask,
                      t[i][0].lat(), t[i][0].lon())) {
         mlog << Debug(4)
              << "Discarding track " << i+1 << " for falling outside the "
              << "initialization masking region: ("
              << t[i][0].lat() << ", " << t[i][0].lon() << ")\n";
         n_mask_init++;
         continue;
      }

      // Valid location mask
      if(conf_info.ValidMaskName.nonempty()) {

         // Loop over all the points in the current track
         for(j=0, status=true; j<t[i].n_points(); j++) {

            // If the TrackPoint falls outside of the polyline break out
            if(!check_masks(conf_info.ValidPolyMask,
                            conf_info.ValidGridMask,
                            conf_info.ValidAreaMask,
                            t[i][j].lat(), t[i][j].lon())) {
               status = false;
               break;
            }
         } // end for j

         if(!status) {
            mlog << Debug(4)
                 << "Discarding track " << i+1 << " for falling outside the "
                 << "valid masking region: "
                 << t[i][j].lat() << ", " << t[i][j].lon() << ")\n";
            n_mask_vld++;
            continue;
         }
      }

      // If we've made it here, retain this track
      tracks.add(t[i]);
   }

   // Print summary filtering info
   mlog << Debug(3)
        << "Total tracks read                = " << t.n_tracks()      << "\n"
        << "Total tracks kept                = " << tracks.n_tracks() << "\n"
        << "Rejected for storm name          = " << n_name            << "\n"
        << "Rejected for valid time          = " << n_vld             << "\n"
        << "Rejected for required lead times = " << n_req_lead        << "\n"
        << "Rejected for init mask           = " << n_mask_init       << "\n"
        << "Rejected for valid mask          = " << n_mask_vld        << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void filter_probs(ProbInfoArray &probs) {
   int i;
   int n_vld, n_mask_init, n_mask_vld;
   ProbInfoArray p = probs;

   // Initialize
   probs.clear();
   n_vld = n_mask_init = n_mask_vld = 0;

   // Loop through the pairs and determine which should be retained
   // The is_keeper() function has already filtered by model, storm id,
   // basin, cyclone, initialization time, and initialization hour.
   for(i=0; i<p.n_probs(); i++) {

      // Valid time window
      if((conf_info.ValidBeg > 0 &&
          conf_info.ValidBeg > p[i]->valid()) ||
         (conf_info.ValidEnd > 0 &&
          conf_info.ValidEnd < p[i]->valid())) {
         mlog << Debug(4)
              << "Discarding probability " << i+1 << " for falling "
              << "outside the valid time window: "
              <<  unix_to_yyyymmdd_hhmmss(p[i]->valid()) << "\n";
         n_vld++;
         continue;
      }

      // Initialization location mask
      if(conf_info.InitMaskName.nonempty()&&
         !check_masks(conf_info.InitPolyMask,
                      conf_info.InitGridMask,
                      conf_info.InitAreaMask,
                      p[i]->lat(), p[i]->lon())) {
         mlog << Debug(4)
              << "Discarding probability " << i+1 << " for falling "
              << "outside the initialization masking region: ("
              << p[i]->lat() << ", " << p[i]->lon() << ")\n";
         n_mask_init++;
         continue;
      }

      // Valid location mask
      if(conf_info.ValidMaskName.nonempty() &&
         !check_masks(conf_info.ValidPolyMask,
                      conf_info.ValidGridMask,
                      conf_info.ValidAreaMask,
                      p[i]->lat(), p[i]->lon())) {
         mlog << Debug(4)
              << "Discarding probability " << i+1 << " for falling "
              << "outside the valid polyline: ("
              << p[i]->lat() << ", " << p[i]->lon() << ")\n";
         n_mask_vld++;
         continue;
      }

      // If we've made it here, retain this probability
      if(p[i]->type() == ATCFLineType_ProbRIRW) probs.add(p.prob_rirw(i));
   }

   // Print summary filtering info
   mlog << Debug(3)
        << "Total probabilities read = " << p.n_probs()     << "\n"
        << "Total probabilities kept = " << probs.n_probs() << "\n"
        << "Rejected for valid time  = " << n_vld           << "\n"
        << "Rejected for init mask   = " << n_mask_init     << "\n"
        << "Rejected for valid mask  = " << n_mask_vld      << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

bool check_masks(const MaskPoly &mask_poly, const Grid &mask_grid,
                 const MaskPlane &mask_area, double lat, double lon) {
   double grid_x, grid_y;

   //
   // Check polyline masking.
   //
   if(mask_poly.n_points() > 0) {
      if(!mask_poly.latlon_is_inside_dege(lat, lon)) {
         return false;
      }
   }

   //
   // Check grid masking.
   //
   if(mask_grid.nx() > 0 || mask_grid.ny() > 0) {
      mask_grid.latlon_to_xy(lat, -1.0*lon, grid_x, grid_y);
      if(grid_x < 0 || grid_x >= mask_grid.nx() ||
         grid_y < 0 || grid_y >= mask_grid.ny()) {
         return false;
      }

      //
      // Check area mask.
      //
      if(mask_area.nx() > 0 || mask_area.ny() > 0) {
         if(!mask_area.s_is_on(nint(grid_x), nint(grid_y))) {
            return false;
         }
      }
   }

   return true;
}

////////////////////////////////////////////////////////////////////////
//
// Apply the following logic to derive 12-hour interpolated tracks:
//
// - The interp12 parameter is set to NONE, FILL, or REPLACE.
// - Loop through the TrackInfoArray looking for models ending in '2'
//   or '3'.
// - For each track ending in '2', search for a corresponding one
//   ending in 'I'.
// - If not found, copy the '2' track, rename it 'I', and add it to
//   the track list.
// - If found and interp12 is set to REPLACE, copy the '2' track,
//   rename it 'I', and replace the existing 'I' track.
// - For each track ending in '3', search for a corresponding one ending
//   in '2'.
// - If not found, apply the same logic listed above.
//
////////////////////////////////////////////////////////////////////////

void derive_interp12(TrackInfoArray &tracks) {
   int i, j, n_add, n_replace;
   ConcatString track_case, amodel;
   StringArray track_case_list;
   TrackInfo interp_track;
   const char *sep = ":";
   char c;

   // If Interp12 logic set to NONE, Nothing to do.
   if(conf_info.Interp12 == Interp12Type_None) return;

   // Loop through the track array and store case information
   for(i=0; i<tracks.n_tracks(); i++) {

      // Build track case information as AMODEL:STORMID:INIT
      track_case << cs_erase
                 << tracks[i].technique() << sep
                 << tracks[i].storm_id() << sep
                 << (tracks[i].init() > 0 ?
                     unix_to_yyyymmdd_hhmmss(tracks[i].init()).text() : na_str);
      track_case_list.add(track_case);
   }

   // Loop through the track array and apply the interp12 logic
   for(i=0, n_add=0, n_replace=0; i<tracks.n_tracks(); i++) {

      // Skip AMODEL names not ending in '2' or '3'
     c = tracks[i].technique()[tracks[i].technique().length() - 1];
      if(c != '2' && c != '3') continue;

      // Search for corresponding track with AMODEL name ending in '2'
      if(c == '3') {
         amodel = tracks[i].technique();
         amodel.chomp('3');
         amodel << '2';
         track_case << cs_erase
                    << amodel << sep
                    << tracks[i].storm_id() << sep
                    << (tracks[i].init() > 0 ?
                       unix_to_yyyymmdd_hhmmss(tracks[i].init()).text() : na_str);
         if(track_case_list.has(track_case)) continue;
      }

      // Swap the '2' or '3' for an 'I' in the AMODEL
      amodel = tracks[i].technique();
      amodel.chomp(c);
      amodel << 'I';

      // Create a copy the '2' or '3' track and rename it to 'I'.
      interp_track = tracks[i];
      interp_track.set_technique(amodel.c_str());

      // Search for corresponding track with AMODEL name ending in 'I'
      track_case << cs_erase
                 << amodel << sep
                 << tracks[i].storm_id() << sep
                 << (tracks[i].init() > 0 ?
                    unix_to_yyyymmdd_hhmmss(tracks[i].init()).text() : na_str);

      // If the 'I' track doesn't exist, add the new interp track.
      if(!track_case_list.has(track_case, j)) {

         mlog << Debug(3)
              << "Adding new track for Interp12 case \"" << track_case
              << "\" by renaming the " << tracks[i].technique() << " track.\n";

         tracks.add(interp_track);
         n_add++;
      }
      // If the 'I' track does exist, check logic, and replace it.
      else {

         // Check logic before replacing existing 'I' track with new one.
         if(conf_info.Interp12 == Interp12Type_Replace) {

            mlog << Debug(3)
                 << "Replacing existing track for Interp12 case \"" << track_case
                 << "\" with the " << tracks[i].technique() << " track.\n";

            tracks.set(j, interp_track);
            n_replace++;
         }
      }
   }

   mlog << Debug(2)
        << "Finished adding " << n_add << " and replacing " << n_replace
        << " Interp12 track(s).\n";

   return;
}

////////////////////////////////////////////////////////////////////////

int derive_consensus(TrackInfoArray &tracks) {
   int i, j, k, l;
   ConcatString cur_case;
   StringArray case_list, case_cmp, req_list;
   TrackInfoArray con_tracks;
   TrackInfo new_track;
   bool found, skip;
   const char *sep = " ";
   int n_add = 0;

   // If no consensus models are defined, nothing to do
   if(conf_info.NConsensus == 0) return(0);

   // Loop through the tracks to build a list of cases
   for(i=0; i<tracks.n_tracks(); i++) {

      // Case defined as: Basin, Cyclone, InitTime
      cur_case.erase();
      cur_case << tracks[i].basin() << sep
               << tracks[i].cyclone() << sep
               << unix_to_yyyymmdd_hhmmss(tracks[i].init());

      // Add this case
      if(!case_list.has(cur_case)) case_list.add(cur_case);

   } // end for i

   mlog << Debug(3)
        << "Building consensus track(s) for " << case_list.n_elements()
        << " cases.\n";

   // Loop through the cases and process each consensus model
   for(i=0; i<case_list.n_elements(); i++) {

      // Break the case back out into Basin, Cyclone, InitTime
      cur_case = case_list[i];
      case_cmp = cur_case.split(sep);

      // Loop through the consensus models
      for(j=0; j<conf_info.NConsensus; j++) {

         // Initialize
         con_tracks.clear();
         new_track.clear();
         req_list.clear();

         // Loop through the consensus members
         for(k=0, skip=false;
             k<conf_info.Consensus[j].Members.n_elements(); k++) {

            // Add required members to the list
            if(conf_info.Consensus[j].Required[k]) {
               req_list.add(conf_info.Consensus[j].Members[k]);
            }

            // Loop through the tracks looking for a match
            for(l=0, found=false; l<tracks.n_tracks(); l++) {

               // If the consenus member was found for this case,
               // add it to the TrackInfoArray object
               if(tracks[l].basin()     == case_cmp[0]                       &&
                  tracks[l].cyclone()   == case_cmp[1]                       &&
                  tracks[l].technique() == conf_info.Consensus[j].Members[k] &&
                  tracks[l].init()      == yyyymmdd_hhmmss_to_unix(case_cmp[2].c_str())) {
                  con_tracks.add(tracks[l]);
                  found = true;
                  mlog << Debug(5)
                       << "[Case " << i+1 << "] For case \""
                       << case_list[i] << "\" member \""
                       << conf_info.Consensus[j].Members[k]
                       <<  "\" was found.\n";
                  break;
               }
            } // end for l

            // Check if the model was not found
            if(!found) {
               mlog << Debug(5)
                    << "[Case " << i+1 << "] For case \""
                    << case_list[i] << "\" member \""
                    << conf_info.Consensus[j].Members[k]
                    <<  "\" was not found.\n";

               // Check if it was a required model
               if(conf_info.Consensus[j].Required[k]) {
                  mlog << Debug(4)
                       << "[Case " << i+1 << "] For case \"" << case_list[i]
                       << "\" skipping consensus model \""
                       << conf_info.Consensus[j].Name
                       << "\" since required member \""
                       << conf_info.Consensus[j].Members[k]
                       <<  "\" was not found.\n";
                     skip = true;
                     break;
               }
            }
         } // end for k

         // If a required member was missing, continue to the next case
         if(skip) continue;

         // Check that the required number of tracks were found
         if(con_tracks.n_tracks() < conf_info.Consensus[j].MinReq) {
            mlog << Debug(4)
                 << "[Case " << i+1 << "] For case \"" << case_list[i]
                 << "\" skipping consensus model \"" << conf_info.Consensus[j].Name
                 << "\" since the minimum number of required members were not found ("
                 << con_tracks.n_tracks() << " < "
                 << conf_info.Consensus[j].MinReq << ").\n";
            continue;
         }

         // Derive the consensus model from the TrackInfoArray
         new_track = consensus(con_tracks, conf_info.Consensus[j].Name,
                               conf_info.Consensus[j].MinReq, req_list);

         if(mlog.verbosity_level() >= 5) {
            mlog << Debug(5)
                 << "Adding consensus track:\n"
                 << new_track.serialize_r(1) << "\n";
         }
         else {
            mlog << Debug(4)
                 << "Adding consensus track:\n"
                 << new_track.serialize() << "\n";
         }

         // Add the consensus model
         tracks.add(new_track);
         n_add++;

      } // end for j

   } // end for i

   return(n_add);
}

////////////////////////////////////////////////////////////////////////

int derive_lag(TrackInfoArray &tracks) {
   int i, j, k, s, n_tracks;
   TrackInfo new_track;
   TrackPoint new_point;
   ConcatString lag_model;
   int n_add = 0;

   // If no time lags are requested, nothing to do
   if(conf_info.LagTime.n_elements() == 0) return(0);

   // Store the input number of tracks to process
   n_tracks = tracks.n_tracks();

   // Loop through the time lags to be applied
   for(i=0; i<conf_info.LagTime.n_elements(); i++) {

      mlog << Debug(3)
           << "Building time-lagged track(s) for lag time \""
           << sec_to_hhmmss(nint(conf_info.LagTime[i])) << "\"\n";

      // Store current lag time
      s = nint(conf_info.LagTime[i]);

      // Loop through the tracks
      for(j=0; j<n_tracks; j++) {

         // Make a copy of the current track
         new_track = tracks[j];

         // Adjust the TrackInfo model name
         lag_model << cs_erase << new_track.technique()
                   << "_LAG_" << sec_to_timestring(s);
         new_track.set_technique(lag_model.c_str());

         // Adjust the TrackInfo times
         new_track.set_init(new_track.init() + s);
         new_track.set_valid_min(new_track.valid_min() + s);
         new_track.set_valid_max(new_track.valid_max() + s);

         // Loop over the track points
         for(k=0; k<new_track.n_points(); k++) {

            // Make a copy of the current track point
            new_point = new_track[k];

            // Adjust the TrackPoint times
            new_point.set_valid(new_point.valid() + s);

            // Store the current time-lagged track point
            new_track.set_point(k, new_point);

         } // end for k

         if(mlog.verbosity_level() >= 5) {
            mlog << Debug(5)
                 << "Adding time-lagged track:\n"
                 << new_track.serialize_r(1) << "\n";
         }
         else {
            mlog << Debug(4)
                 << "Adding time-lagged track:\n"
                 << new_track.serialize() << "\n";
         }

         // Store the current time-lagged track
         tracks.add(new_track);
         n_add++;

      } // end for j

   } // end for i

   return(n_add);
}

////////////////////////////////////////////////////////////////////////

int derive_baseline(TrackInfoArray &atracks, const TrackInfoArray &btracks) {
   int i, j, k;
   ConcatString cur_case;
   StringArray case_list;
   int n_add = 0;

   // If no baseline models are requested, nothing to do
   if(conf_info.OperBaseline.n_elements() == 0 &&
      conf_info.BestBaseline.n_elements() == 0) return(0);

   mlog << Debug(3)
        << "Building CLIPER/SHIFOR operational baseline forecasts using "
        << conf_info.OperBaseline.n_elements() << " method(s).\n";

   // Loop over the ADECK tracks
   for(i=0; i<atracks.n_tracks(); i++) {

      // Define the current case as stormid and initialization time
      cur_case << cs_erase
               << atracks[i].storm_id() << ":"
               << atracks[i].init();

      // Build a unique list of cases
      if(!case_list.has(cur_case)) case_list.add(cur_case);

      // Only derive baselines from the CARQ model
      if(!atracks[i].is_oper_track()) continue;

      // Loop over the operational baseline methods
      for(j=0; j<conf_info.OperBaseline.n_elements(); j++) {

         // Derive the baseline model
         derive_baseline_model(conf_info.OperBaseline[j],
                               atracks[i], 0, atracks);
         n_add++;
      } // end for j
   } // end for i

   mlog << Debug(3)
        << "Building CLIPER/SHIFOR BEST track baseline forecasts using "
        << conf_info.BestBaseline.n_elements() << " method(s) for "
        << case_list.n_elements() << " cases.\n";

   // Loop over the BDECK tracks
   for(i=0; i<btracks.n_tracks(); i++) {

      // Only derive baselines from the BEST tracks
      if(!btracks[i].is_best_track()) continue;

      // Derive baseline model for each track point
      for(j=0; j<btracks[i].n_points(); j++) {

         // Define the current case as stormid and current valid time
         cur_case << cs_erase
                  << btracks[i].storm_id() << ":"
                  << btracks[i][j].valid();

         // Check if the current case is in the list
         if(!case_list.has(cur_case)) continue;

         // Loop over the BEST track baseline methods
         for(k=0; k<conf_info.BestBaseline.n_elements(); k++) {

            // Derive the baseline model
            derive_baseline_model(conf_info.BestBaseline[k],
                                  btracks[i], j, atracks);
            n_add++;
         } // end for k
      } // end for j
   } // end for i

   return(n_add);
}

////////////////////////////////////////////////////////////////////////

void derive_baseline_model(const ConcatString &model,
                           const TrackInfo &ti, int i_start,
                           TrackInfoArray &atracks) {
   int i, ntp;
   int mon, day, yr, hr, minute, sec, lead_sec;
   unixtime ut;
   char basin[2];
   float tp_mon[mxp], tp_day[mxp], tp_hr[mxp];
   float tp_lat[mxp], tp_lon[mxp], tp_vmax[mxp];
   float tp_dir[2], tp_spd[2];
   float bl_lat[nvtx], bl_lon[nvtx], bl_vmax[nvtx];
   TrackInfo  new_track;
   TrackPoint new_point;

   // Check bounds
   if(i_start < 0 || i_start >= ti.n_points()) {
       mlog << Error
            << "\nderive_baseline_model() -> "
            << "range check error for i_start = " << i_start << "\n\n";
       exit(1);
   }

   // Populate input variables for operational baselines
   if(model[0] == 'O') {

      // Loop over the track points and populate date/location arrays
      for(i=i_start, ntp=0; i<ti.n_points(); i++) {

         // Store lead time = 0 in index 0
         if(ti[i].lead() == 0) {
            tp_lat[0]  = (float) ti[i].lat();
            tp_lon[0]  = (float) ti[i].lon();
            tp_vmax[0] = (float) ti[i].v_max();
            tp_dir[0]  = (float) ti[i].direction();
            tp_spd[0]  = (float) ti[i].speed();
         }

         // Store lead time = -12 in index 1
         else if(ti[i].lead() == (-12*sec_per_hour)) {
            tp_lat[1]  = (float) ti[i].lat();
            tp_lon[1]  = (float) ti[i].lon();
            tp_vmax[1] = (float) ti[i].v_max();
            tp_dir[1]  = (float) ti[i].direction();
            tp_spd[1]  = (float) ti[i].speed();
         }

      } // end for i
   }

   // Populate input variables for BEST track baselines
   if(model[0] == 'B') {

      // Loop over the track points and populate date/location arrays
      for(i=i_start, ntp=0; i<ti.n_points(); i++) {

         // Check if the current time is a 6-hour interval
         if((ti[i].valid()%(6*sec_per_hour)) != 0) continue;

         // Process the valid time
         unix_to_mdyhms(ti[i].valid(),
                        mon, day, yr, hr, minute, sec);

         // Store the valid time, location, and max wind
         tp_mon[ntp]  = (float) mon;
         tp_day[ntp]  = (float) day;
         tp_hr[ntp]   = (float) hr;
         tp_lat[ntp]  = (float) ti[i].lat();
         tp_lon[ntp]  = (float) ti[i].lon();
         tp_vmax[ntp] = (float) ti[i].v_max();

         // Increment the track point counter
         ntp++;

      } // end for i
   }

   // Store the basin name
   strncpy(basin, ti.basin().c_str(), 2);

   // Store the valid time of the starting point
   unix_to_mdyhms(ti[i_start].valid(),
                  mon, day, yr, hr, minute, sec);

   // Call appropriate subroutine to derive baseline model
   if(model == "OCLP") {
      oclip_(basin, &yr, &mon, &day, &hr,
         &tp_lat[0], &tp_lon[0], &tp_vmax[0], &tp_dir[0], &tp_spd[0],
         &tp_lat[1], &tp_lon[1], &tp_vmax[1], &tp_dir[1], &tp_spd[1],
         bl_lat,     bl_lon,     bl_vmax);
   }
   else if(model == "OCS5") {
      oclip5_(basin, &yr, &mon, &day, &hr,
         &tp_lat[0], &tp_lon[0], &tp_vmax[0], &tp_dir[0], &tp_spd[0],
         &tp_lat[1], &tp_lon[1], &tp_vmax[1], &tp_dir[1], &tp_spd[1],
         bl_lat,     bl_lon,     bl_vmax);
   }
   else if(model == "OCD5") {
      oclipd5_(basin, &yr, &mon, &day, &hr,
         &tp_lat[0], &tp_lon[0], &tp_vmax[0], &tp_dir[0], &tp_spd[0],
         &tp_lat[1], &tp_lon[1], &tp_vmax[1], &tp_dir[1], &tp_spd[1],
         bl_lat,     bl_lon,     bl_vmax);
   }
   else if(model == "OCDT") {
      oclipd5_test_(basin, &yr, &mon, &day, &hr,
         &tp_lat[0], &tp_lon[0], &tp_vmax[0], &tp_dir[0], &tp_spd[0],
         &tp_lat[1], &tp_lon[1], &tp_vmax[1], &tp_dir[1], &tp_spd[1],
         bl_lat,     bl_lon,     bl_vmax);
   }
   else if(model == "BCLP") {
      btclip_(basin, &mon, &day, &hr, &ntp,
         tp_mon,   tp_day,   tp_hr,
         tp_lat,   tp_lon,   tp_vmax,
         bl_lat,   bl_lon,   bl_vmax);
   }
   else if(model == "BCS5") {
      btclip5_(basin, &yr, &mon, &day, &hr, &ntp,
         tp_mon,   tp_day,   tp_hr,
         tp_lat,   tp_lon,   tp_vmax,
         bl_lat,   bl_lon,   bl_vmax);
   }
   else if(model == "BCD5") {
      btclipd5_(basin, &yr, &mon, &day, &hr, &ntp,
         tp_mon,   tp_day,   tp_hr,
         tp_lat,   tp_lon,   tp_vmax,
         bl_lat,   bl_lon,   bl_vmax);
   }
   else if(model == "BCLA") {
      btclipa_(basin, &mon, &day, &hr, &ntp,
         tp_mon,   tp_day,   tp_hr,
         tp_lat,   tp_lon,   tp_vmax,
         bl_lat,   bl_lon,   bl_vmax);
   }
   else {
       mlog << Error
            << "\nderive_baseline_model() -> "
            << "unsupported baseline model type \"" << model
            << "\".\n\n";
       exit(1);
   }

   // Populate the CLIPER/SHIFOR track info
   new_track.set_init(ti[i_start].valid());
   new_track.set_basin(ti.basin().c_str());
   new_track.set_cyclone(ti.cyclone().c_str());
   new_track.set_storm_id();
   new_track.set_storm_name(ti.storm_name().c_str());
   new_track.set_technique(model.c_str());
   new_track.set_technique_number(ti.technique_number());

   // Store the initial track point
   new_point.set_valid(ti[i_start].valid());
   new_point.set_lead(0);
   new_point.set_lat(tp_lat[0]);
   new_point.set_lon(tp_lon[0]);
   if(!is_bad_data(tp_vmax[0])) new_point.set_v_max(nint(tp_vmax[0]));
   new_track.add(new_point);

   // Loop over the remaining track points
   for(i=0; i<nvtx; i++) {

      // Check for bad data
      if(is_bad_data(bl_lat[i]) || is_bad_data(bl_lon[i]) ||
         bl_lat[i] < -90        || bl_lon[i] < -180       ||
         bl_lat[i] >  90        || bl_lat[i] >  180) break;

         // Initialize
         new_point.clear();

         // Compute the lead time for the current point
         lead_sec = 12*sec_per_hour*(i+1);

         // Compute the current valid time
         ut = ti[i_start].valid() + lead_sec;

         // Setup the current track point
         new_point.set_valid(ut);
         new_point.set_lead(lead_sec);
         new_point.set_lat(bl_lat[i]);
         new_point.set_lon(bl_lon[i]);
         if(!is_bad_data(bl_vmax[i])) new_point.set_v_max(nint(bl_vmax[i]));

         // Add the current CLIPER/SHIFOR track point
         new_track.add(new_point);

   } // end for j

   if(mlog.verbosity_level() >= 5) {
      mlog << Debug(5)
           << "Adding CLIPER/SHIFOR track:\n"
           << new_track.serialize_r(1) << "\n";
   }
   else {
      mlog << Debug(4)
           << "Adding CLIPER/SHIFOR track:\n"
           << new_track.serialize() << "\n";
   }

   // Add the CLIPER/SHIFOR track to the ADECKS
   atracks.add(new_track);

   return;
}

////////////////////////////////////////////////////////////////////////

void process_match(const TrackInfo &adeck, const TrackInfo &bdeck,
                   TrackPairInfoArray &p) {
   int i, i_adeck, i_bdeck, i_err;
   TimeArray valid_list, valid_err;
   NumArray tk_err, x_err, y_err, altk_err, crtk_err;
   double adeck_dland, bdeck_dland, e_tk, e_x, e_y, e_altk, e_crtk;

   TrackPairInfo pair;
   const TrackPoint *adeck_point = (TrackPoint *) 0;
   const TrackPoint *bdeck_point = (TrackPoint *) 0;
   TrackPoint empty_point;

   // Initialize TrackPairInfo with the current tracks
   pair.initialize(adeck, bdeck);

   // Compute the track errors
   compute_track_err(adeck, bdeck, valid_err, tk_err,
                     x_err, y_err, altk_err, crtk_err);

   // Loop through the ADECK TrackPoints to build a list of valid times
   for(i=0; i<adeck.n_points(); i++) {
      if(!valid_list.has(adeck[i].valid()))
         valid_list.add(adeck[i].valid());
   }

   // Loop through the BDECK TrackPoints to build a list of valid times
   for(i=0; i<bdeck.n_points(); i++) {
      if(!valid_list.has(bdeck[i].valid()))
         valid_list.add(bdeck[i].valid());
   }

   // Sort the valid times
   valid_list.sort_array();

   mlog << Debug(4)
        << "Processing " << valid_list.n_elements()
        << " unique valid times: "
        << unix_to_yyyymmdd_hhmmss(valid_list.min()) << " to "
        << unix_to_yyyymmdd_hhmmss(valid_list.max()) << "\n";

   // Loop through the valid times
   for(i=0; i<valid_list.n_elements(); i++) {

      // Get the indices for this time
      i_adeck = adeck.valid_index(valid_list[i]);
      i_bdeck = bdeck.valid_index(valid_list[i]);

      // Check for TrackPoints in both the ADECK and BDECK tracks
      if(conf_info.MatchPoints &&
         (i_adeck < 0 || i_bdeck < 0)) continue;

      // Initialize distances and pointers
      adeck_dland = bdeck_dland = bad_data_double;
      adeck_point = bdeck_point = &empty_point;

      // Get the ADECK distance to land for the current TrackPoint
      if(i_adeck >= 0) {
         adeck_dland = compute_dland(adeck[i_adeck].lat(), -1.0*adeck[i_adeck].lon());
         adeck_point = &adeck[i_adeck];
      }

      // Get the BDECK distance to land for the current TrackPoint
      if(i_bdeck >= 0) {
         bdeck_dland = compute_dland(bdeck[i_bdeck].lat(), -1.0*bdeck[i_bdeck].lon());
         bdeck_point = &bdeck[i_bdeck];
      }

      // Find the track errors for this time
      i_err = valid_err.index(valid_list[i]);

      // Initialize errors
      e_tk = e_x = e_y = e_altk = e_crtk = bad_data_double;

      // Get the errors if ADECK and BDECK are both valid
      if(i_err >= 0 && i_adeck >= 0 && i_bdeck >= 0) {
         e_tk   = tk_err[i_err];
         e_x    = x_err[i_err];
         e_y    = y_err[i_err];
         e_altk = altk_err[i_err];
         e_crtk = crtk_err[i_err];
      }

      mlog << Debug(5)
           << "[Time " << i+1 << "] Valid time "
           << unix_to_yyyymmdd_hhmmss(valid_list[i])
           << ", ADECK: index = " << i_adeck << ", dland = " << adeck_dland
           << ", BDECK: index = " << i_bdeck << ", dland = " << bdeck_dland
           << ", ERROR: track = " << e_tk << ", x = " << e_x << ", y = " << e_y
           << ", along = " << e_altk << ", cross = " << e_crtk << "\n";

      // Add this info to the TrackPairInfoArray
      pair.add(*adeck_point, *bdeck_point, adeck_dland, bdeck_dland,
               e_tk, e_x, e_y, e_altk, e_crtk);

   } // end for i

   // Add the current TrackPairInfo object
   p.add(pair);

   return;
}

////////////////////////////////////////////////////////////////////////

double compute_dland(double lat, double lon) {
   double x_dbl, y_dbl, dist;
   int x, y;

   // Convert lat,lon to x,y
   dland_grid.latlon_to_xy(lat, lon, x_dbl, y_dbl);

   // Round to nearest int
   x = nint(x_dbl);
   y = nint(y_dbl);

   // Check range
   if(x < 0 || x >= dland_grid.nx() ||
      y < 0 || y >= dland_grid.ny())   dist = bad_data_double;
   else                                dist = dland_dp.get(x, y);

   return(dist);
}

////////////////////////////////////////////////////////////////////////

void compute_track_err(const TrackInfo &adeck, const TrackInfo &bdeck,
                       TimeArray &vld_err, NumArray &tk_err,
                       NumArray &x_err,    NumArray &y_err,
                       NumArray &altk_err, NumArray &crtk_err) {
   int i, i_adeck, i_bdeck, status;
   unixtime ut, ut_min, ut_max;
   int ut_inc, n_ut;
   float alat[mxp], alon[mxp], blat[mxp], blon[mxp];
   float crtk[mxp], altk[mxp];
   double x, y, tk, lon_min, lon_max;

   // Initialize
   vld_err.clear();
   tk_err.clear();
   x_err.clear();
   y_err.clear();
   altk_err.clear();
   crtk_err.clear();

   // Get the valid time range
   ut_min = min(adeck.valid_min(), bdeck.valid_min());
   ut_max = max(adeck.valid_max(), bdeck.valid_max());

   // Determine the valid increment
   // For BEST tracks, use a constant time step
   // For non-BEST tracks, select the most common BDECK time step
   if(bdeck.is_best_track()) ut_inc = best_track_time_step;
   else                      ut_inc = bdeck.valid_inc();

   // Round the valid times to the nearest valid increment
   if(ut_min%ut_inc != 0) ut_min = (ut_min/ut_inc + 1)*ut_inc;
   if(ut_max%ut_inc != 0) ut_max = (ut_max/ut_inc)*ut_inc;

   // Compute the number of valid times
   n_ut = (ut_max-ut_min)/ut_inc + 1;

   mlog << Debug(3)
        << "Computing track errors for " << n_ut << " vaild times: "
        << unix_to_yyyymmdd_hhmmss(ut_min)
        << " to " << unix_to_yyyymmdd_hhmmss(ut_max)
        << " by " << sec_to_hhmmss(ut_inc) << " increment.\n";

   // Check for too many track points
   if(n_ut > mxp) {
      mlog << Error
           << "\ncompute_track_err() -> "
           << "exceeded the maximum number of allowable track points ("
           << n_ut << " > " << mxp << ")\n\n";
      exit(1);
   }

   // Loop through the valid times
   for(i=0, lon_min=180, lon_max=-180; i<n_ut; i++) {

      // Add the current valid time
      ut = ut_min+(i*ut_inc);
      vld_err.add(ut);

      // Initialize to bad data
      tk_err.add(bad_data_double);
      x_err.add(bad_data_double);
      y_err.add(bad_data_double);
      altk_err.add(bad_data_double);
      crtk_err.add(bad_data_double);

      // Get the indices for this time
      i_adeck = adeck.valid_index(ut);
      i_bdeck = bdeck.valid_index(ut);

      // Populate array arguments
      alat[i] = (i_adeck >= 0 ? adeck[i_adeck].lat() : bad_data_float);
      alon[i] = (i_adeck >= 0 ? adeck[i_adeck].lon() : bad_data_float);
      blat[i] = (i_bdeck >= 0 ? bdeck[i_bdeck].lat() : bad_data_float);
      blon[i] = (i_bdeck >= 0 ? bdeck[i_bdeck].lon() : bad_data_float);

      // Check for good data
      if(is_bad_data(alat[i]) || is_bad_data(blat[i]) ||
         is_bad_data(alon[i]) || is_bad_data(blon[i])) continue;

      // Keep track of min/max longitudes
      lon_min = (min(alon[i], blon[i]) < lon_min ? min(alon[i], blon[i]) : lon_min);
      lon_max = (max(alon[i], blon[i]) > lon_max ? max(alon[i], blon[i]) : lon_max);

      // Compute and store track errors
      latlon_to_xytk_err(alat[i], alon[i], blat[i], blon[i], x, y, tk);
      x_err.set(i, x);
      y_err.set(i, y);
      tk_err.set(i, tk);
   }

   // If the range of longitudes is large, rescale from [-180, 180] to [0, 360]
   // prior to computing along and cross-track errors.
   if((lon_max - lon_min) >= 180) {
      for(i=0; i<n_ut; i++) {
         alon[i] = rescale_deg(alon[i], 0, 360);
         blon[i] = rescale_deg(blon[i], 0, 360);
      }
   }

   // Call the ACERR subroutine for along and cross track errors
   acerr_(blat, blon, alat, alon, &n_ut, crtk, altk, &status);

   // Store the along/cross track errors in nm
   if(status == 0) {
      for(i=0; i<n_ut; i++) {

         // Along-track error
         if(!is_bad_data(altk[i]))
            altk_err.set(i, tc_nautical_miles_per_km*altk[i]);

         // Cross-track error
         if(!is_bad_data(crtk[i]))
            crtk_err.set(i, tc_nautical_miles_per_km*crtk[i]);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_watch_warn(TrackPairInfoArray &p) {
   ConcatString file_name;
   LineDataFile f_in;
   DataLine dl;
   ConcatString ww_sid;
   unixtime ww_ut;
   WatchWarnType ww_type;
   int i, n_ww;

   // Get the path for the watch/warning file
   file_name = replace_path(conf_info.WatchWarnFile);

   // Check for non-empty file name
   if(file_name.empty()) {
      mlog << Debug(1)
           << "No watch/warning file specified.\n";
      return;
   }

   mlog << Debug(1)
        << "Watch/Warning file: " << file_name << "\n";

   // Open the watch/warning ASCII file
   if(!f_in.open(file_name.c_str())) {
      mlog << Error << "\nprocess_watch_warn() -> "
           << "can't open input watch/warning ASCII file \""
           << file_name << "\" for reading\n\n";
      exit(1);
   }

   // Read the file line by line
   while(f_in >> dl) {

      // Parse the watch/warning time
      ww_ut = mdyhms_to_unix(atoi(dl[0]), atoi(dl[1]),
                             atoi(dl[2]), atoi(dl[3]), 0, 0);

      // Parse the number of watch/warning messages
      n_ww = atoi(dl[5]);

      // Parse the storm id
      ww_sid = dl[6];
      ww_sid.ws_strip();

      // Determine the maximum severity watch/warning in effect
      for(i=0, ww_type=NoWatchWarnType; i<n_ww; i++) {

         // Read the next line
         f_in >> dl;

         // Compute maximum severity watch/warning
         ww_type = ww_max(ww_type, int_to_watchwarntype(atoi(dl[0])));
      }

      // Add the current watch/warning information to the tracks.
      p.add_watch_warn(ww_sid, ww_type, ww_ut + conf_info.WatchWarnOffset);
   }

   // Close the input file
   f_in.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void write_tracks(const TrackPairInfoArray &p) {
   int i_row, i;
   TcHdrColumns tchc;
   ConcatString out_file;
   AsciiTable out_at;
   ofstream *out = (ofstream *) 0;

   // Set the track pair output file name
   out_file << out_base << tc_stat_file_ext;
   out_files.add(out_file);

   // Create the output file
   open_tc_txt_file(out, out_file.c_str());

   // Initialize the output AsciiTable
   out_at.set_size(p.n_points() + 1, n_tc_header_cols + n_tc_mpr_cols);
   setup_table(out_at);

   // Write the header row
   write_tc_mpr_header_row(1, out_at, 0, 0);

   // Initialize the row index to 1 to account for the header
   i_row = 1;

   // Store user-defined description
   tchc.set_desc(conf_info.Desc);

   // Store masking regions in the header
   if(conf_info.InitMaskName.nonempty())  tchc.set_init_mask(conf_info.InitMaskName);
   else                                   tchc.set_init_mask(na_string);
   if(conf_info.ValidMaskName.nonempty()) tchc.set_valid_mask(conf_info.ValidMaskName);
   else                                   tchc.set_valid_mask(na_string);

   // Loop through the TrackPairInfo objects
   for(i=0; i<p.n_pairs(); i++) {

      // More header columns
      tchc.set_adeck_model(p[i].adeck().technique());
      tchc.set_bdeck_model(p[i].bdeck().technique());
      tchc.set_storm_id(p[i].bdeck().storm_id());
      tchc.set_basin(p[i].bdeck().basin());
      tchc.set_cyclone(p[i].bdeck().cyclone());
      tchc.set_storm_name(p[i].bdeck().storm_name());

      // Write the current TrackPairInfo object
      write_tc_mpr_row(tchc, p[i], out_at, i_row);
   }

   // Write the AsciiTable contents and clean up
   if(out != (ofstream *) 0) {
      *out << out_at;
      out->close();
      delete out;
      out = (ofstream *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_prob_rirw(const ProbRIRWPairInfoArray &p) {
   int i_row, i, n, n_row, max_n;
   TcHdrColumns tchc;
   ConcatString out_file;
   AsciiTable out_at;
   ofstream *out = (ofstream *) 0;

   // Set the track pair output file name
   out_file << out_base << "_PROBRIRW" << tc_stat_file_ext;
   out_files.add(out_file);

   // Create the output file
   open_tc_txt_file(out, out_file.c_str());

   // Determine the number of output rows and max number of probs
   max_n = p[0].prob_rirw().n_prob();
   for(i=0,n_row=1; i<p.n_pairs(); i++) {

      // Current number of probabilities
      n = p[i].prob_rirw().n_prob();

      // Keep track of maximum probabilities
      if(n > max_n) max_n = n;

      // Number of output lines:
      // One line for each pair
      // Plus one more for the maximum
      n_row++;
      if(n > 1) n_row++;
   }

   // Initialize the output AsciiTable
   out_at.set_size(n_row, n_tc_header_cols + get_n_prob_rirw_cols(max_n));
   setup_table(out_at);

   // Write the header row
   write_prob_rirw_header_row(1, max_n, out_at, 0, 0);

   // Initialize the row index to 1 to account for the header
   i_row = 1;

   // Store user-defined description
   tchc.set_desc(conf_info.Desc);

   // Store masking regions in the header
   if(conf_info.InitMaskName.nonempty())  tchc.set_init_mask(conf_info.InitMaskName);
   else                                   tchc.set_init_mask(na_string);
   if(conf_info.ValidMaskName.nonempty()) tchc.set_valid_mask(conf_info.ValidMaskName);
   else                                   tchc.set_valid_mask(na_string);

   // Loop through the ProbRIRWPairInfo objects
   for(i=0; i<p.n_pairs(); i++) {

      // More header columns
      tchc.set_adeck_model(p[i].prob_rirw().technique());
      tchc.set_bdeck_model(p[i].bmodel());
      tchc.set_storm_id(p[i].prob_rirw().storm_id());
      tchc.set_basin(p[i].prob_rirw().basin());
      tchc.set_cyclone(p[i].prob_rirw().cyclone());
      tchc.set_storm_name(p[i].bdeck()->storm_name());

      // Write the current TrackPairInfo object
      write_prob_rirw_row(tchc, p[i], out_at, i_row);
   }

   // Write the AsciiTable contents and clean up
   if(out != (ofstream *) 0) {
      *out << out_at;
      out->close();
      delete out;
      out = (ofstream *) 0;
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
        << "\t-adeck source and/or -edeck source\n"
        << "\t-bdeck source\n"
        << "\t-config file\n"
        << "\t[-out base]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-adeck source\" is used one or more times to "
        << "specify a file or top-level directory containing ATCF "
        << "model output \"" << atcf_suffix
        << "\" data to process (required if no -edeck).\n"

        << "\t\t\"-edeck source\" is used one or more times to "
        << "specify a file or top-level directory containing ATCF "
        << "ensemble model output \"" << atcf_suffix
        << "\" data to process (required if no -adeck).\n"

        << "\t\t\"-bdeck source\" is used one or more times to "
        << "specify a file or top-level directory containing ATCF "
        << "best track \"" << atcf_suffix
        << "\" data to process (required).\n"

        << "\t\t\"-config file\" is used once to specify the "
        << "TCPairsConfig file containing the desired configuration "
        << "settings (required).\n"

        << "\t\t\"-out base\" overrides the default output file base "
        << "(" << out_base << ") (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n"

        << "\tNote: The \"-adeck\", \"-edeck\", and \"-bdeck\" options "
        << "may include \"suffix=string\" to modify the model names "
        << "from that source.\n\n";

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_adeck(const StringArray & a) {
   set_atcf_source(a, adeck_source, adeck_model_suffix);
}

////////////////////////////////////////////////////////////////////////

void set_edeck(const StringArray & a) {
   set_atcf_source(a, edeck_source, edeck_model_suffix);
}

////////////////////////////////////////////////////////////////////////

void set_bdeck(const StringArray & a) {
   set_atcf_source(a, bdeck_source, bdeck_model_suffix);
}

////////////////////////////////////////////////////////////////////////

void set_atcf_source(const StringArray & a,
                     StringArray &source, StringArray &model_suffix) {
   int i;
   StringArray sa;
   ConcatString cs, suffix;

   // Check for optional suffix sub-argument
   for(i=0; i<a.n_elements(); i++) {
      cs = a[i];
      if(cs.startswith("suffix")) {
         sa = cs.split("=");
         if(sa.n_elements() != 2) {
            mlog << Error
                 << "\nset_atcf_source() -> "
                 << "the model suffix must be specified as "
                 << "\"suffix=string\".\n\n";
            usage();
         }
         else {
            suffix = sa[1];
         }
      }
   }

   // Parse the remaining sources
   for(i=0; i<a.n_elements(); i++) {
      cs = a[i];
      if(cs.startswith("suffix")) continue;
      source.add(a[i]);
      model_suffix.add(suffix);
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
