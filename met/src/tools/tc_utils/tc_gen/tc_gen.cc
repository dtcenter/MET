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

#include "vx_tc_util.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_log.h"

#include "met_file.h"

////////////////////////////////////////////////////////////////////////

static void   process_command_line (int, char **);
static void   process_tracks       ();
static void   get_atcf_files       (const StringArray &,
                                    const StringArray &,
                                    StringArray &, StringArray &);
static void   process_track_files  (const StringArray &,
                                    const StringArray &,
                                    GenesisInfoArray &);
static bool   check_masks          (const MaskPoly &, const Grid &,
                                    const MaskPlane &,
                                    double lat, double lon);
static void   process_match        (const TrackInfo &, const TrackInfo &,
                                    TrackPairInfoArray &);
static void   process_watch_warn   (TrackPairInfoArray &);
static void   write_tracks         (const TrackPairInfoArray &);
static void   write_prob_rirw      (const ProbRIRWPairInfoArray &);
static void   setup_table          (AsciiTable &);
static void   usage                ();
static void   set_lookin           (const StringArray &);
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
   process_tracks();

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
   out_base = "./tc_gen";

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add function calls for the arguments
   cline.add(set_lookin,    "-lookin", -1);
   cline.add(set_config,    "-config",  1);
   cline.add(set_out,       "-out",     1);
   cline.add(set_logfile,   "-log",     1);
   cline.add(set_verbosity, "-v",       1);

   // Parse the command line
   cline.parse();

   // Check for the minimum number of arguments
   if(atcf_source.n_elements() == 0 ||
      config_file.length()     == 0) {
      mlog << Error
           << "\nprocess_command_line(int argc, char **argv) -> "
           << "the \"-lookin\" and \"-config\" command line options are "
           << "required\n\n";
      usage();
   }

   // List the input track files
   for(i=0; i<atcf_source.n_elements(); i++) {
      mlog << Debug(1)
           << "[Source " << i+1 << " of " << atcf_source.n_elements()
           << "] ATCF Source: " << atcf_source[i] << ", Model Suffix: "
           << atcf_model_suffix[i] << "\n";
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

void process_tracks() {
   TrackInfoArray tracks;
   StringArray atcf_files, atcf_files_model_suffix;
   GenesisInfoArray ga, amodel_ga, bmodel_ga;
   map<ConcatString,GenesisInfoArray> amodel_ga_map;

   // Initialize
   tracks.clear();

   // Get the list of track files
   get_atcf_files(atcf_source, atcf_model_suffix,
                  atcf_files,  atcf_files_model_suffix);

   mlog << Debug(2)
        << "Processing " << atcf_files.n() << " ATCF files.\n";
   process_track_files(atcf_files, atcf_files_model_suffix, ga);

   // Loop through the filters and subset the genesis events
   for(int i=0; i<conf_info.n_vx(); i++) {

      // Initialize
      bmodel_ga.clear();
      amodel_ga_map.clear();

      // Loop through and subset the genesis events
      for(int j=0; j<ga.n(); j++) {

         // Check filters
         if(conf_info.VxOpt[i].is_keeper(ga[j])) {

            // Store the current model name
            ConcatString cs = ga[j].technique();

            // Check requested bmodel
            if(cs == conf_info.VxOpt[i].BModel) {
               bmodel_ga.add(ga[i]);
            }
            // Check requested amodels
            else if(conf_info.VxOpt[i].AModel.n() == 0 ||
                    conf_info.VxOpt[i].AModel.has(cs)) {

               // Add a new map entry, if necessary
               if(amodel_ga_map.count(cs) == 0) {
                  amodel_ga.clear();
                  amodel_ga_map[cs] = amodel_ga;
               }

               // Store the current object
               amodel_ga_map[cs].add(ga[j]);
            }
         }
      } // end j

      // JHG, do more work here and process the pairs!

   }

   return;
}

////////////////////////////////////////////////////////////////////////

void get_atcf_files(const StringArray &source,
                    const StringArray &model_suffix,
                    StringArray &files,
                    StringArray &files_model_suffix) {
   StringArray cur_source, cur_files;
   int i, j;

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
                         GenesisInfoArray &genesis) {
   int i, n_lines, n_tracks, tot_lines, tot_tracks;
   LineDataFile f;
   ConcatString cs;
   ATCFTrackLine line;
   TrackInfo cur_track;

   // Initialize counts
   tot_lines = tot_tracks = 0;

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
      n_lines = n_tracks = 0;

      // Read each line in the file
      while(f >> line) {

         // Increment the line counts
         n_lines++;

         // Add model suffix, if specified
         if(model_suffix[i].length() > 0) {
            cs << cs_erase << line.technique() << model_suffix[i];
            line.set_technique(cs);
         }

         // Check for BEST track technqiue
         if(conf_info.BestTechnique.has(line.technique())) {
            line.set_best_track(true);
         }

         // Attempt to add the current line to the current track
         if(!cur_track.add(line)) {

            // This track is complete, add it to GenInfoArray.
            genesis.add(cur_track, conf_info.FHrStart);
            n_tracks++;

            // Clear the current track and start a new one.
            cur_track.clear();
            cur_track.add(line);
         }
      }

      // Add the last track
      if(cur_track.n_points() > 0) {
         genesis.add(cur_track, conf_info.FHrStart);
         n_tracks++;
      }

      // Dump out the current number of lines
      mlog << Debug(4)
           << "[File " << i+1 << " of " << files.n_elements()
           << "] Parsed " << n_tracks << " tracks from " << n_lines
           << " lines read from file \"" << files[i] << "\"\n";

      // Close the current file
      f.close();

      // Increment counts
      tot_lines  += n_lines;
      tot_tracks += n_tracks;

   } // end for i

   // Dump out the total number of lines
   mlog << Debug(3)
        << "Parsed " << tot_tracks << " tracks from " << tot_lines
        << " lines read from " << files.n_elements() << " files.\n";

   // Compute the distance to land
   for(i=0; i<genesis.n(); i++) {
      genesis.set_dland(i, conf_info.compute_dland(
                              genesis[i].lat(), genesis[i].lon()));
   }

   // Dump out the track information
   mlog << Debug(3)
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

void process_match(const TrackInfo &adeck, const TrackInfo &bdeck,
                   TrackPairInfoArray &p) {
/*
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
*/
   return;
}

////////////////////////////////////////////////////////////////////////

void process_watch_warn(TrackPairInfoArray &p) {
/*
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
*/
   return;
}

////////////////////////////////////////////////////////////////////////

void write_tracks(const TrackPairInfoArray &p) {
/*
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
*/
   return;
}

////////////////////////////////////////////////////////////////////////

void write_prob_rirw(const ProbRIRWPairInfoArray &p) {
/*
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
*/
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
        << "\t-lookin path\n"
        << "\t-config file\n"
        << "\t[-out base]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-lookin path\" specifies an ATCF file or "
        << "top-level directory containing ATCF files ending in \""
        << atcf_suffix << "\" to process (required).\n"

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

void set_lookin(const StringArray & a) {
   int i;
   StringArray sa;
   ConcatString cs;

   // Check for optional suffix sub-argument
   for(int i=0; i<a.n(); i++) {
      if(a[i] == "suffix") {
         cs = a[i];
         sa = cs.split("=");
         if(sa.n_elements() != 2) {
            mlog << Error << "\nset_lookin() -> "
                 << "the model suffix must be specified as "
                 << "\"suffix=string\".\n\n";
         }
         else {
            atcf_model_suffix.add(sa[1]);
         }
      }
      else {
         atcf_source.add(a[i]);
      }
   }

   // Check for consistent usage
   if(atcf_model_suffix.n() > 0 &&
      atcf_model_suffix.n() != atcf_source.n()) {
      mlog << Error << "\nset_lookin() -> "
           << "the number of \"suffix=string\" options must match the "
           << "number of -lookin options.\n\n";
      exit(1);
   }

   // Add empty suffix strings
   for(i=atcf_model_suffix.n(); i<atcf_source.n(); i++) {
      atcf_model_suffix.add("");
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
