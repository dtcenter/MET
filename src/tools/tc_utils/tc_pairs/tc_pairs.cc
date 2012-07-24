// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "tc_pairs.h"

#include "vx_tc_util.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_log.h"

#include "met_file.h"

////////////////////////////////////////////////////////////////////////

static const int mxp = 100;

extern "C" {
   void acerr_(float [mxp], float [mxp], float [mxp], float [mxp], int *,
               float [mxp], float [mxp], int *);
}

////////////////////////////////////////////////////////////////////////

static void   process_command_line(int, char **);
static void   process_tracks      ();
static void   process_track_files (const StringArray &, TrackInfoArray &);
static void   filter_tracks       (TrackInfoArray &);
static void   merge_interp12      (TrackInfoArray &);
static void   derive_consensus    (TrackInfoArray &);
static void   process_match       (const TrackInfo &, const TrackInfo &,
                                   TrackPairInfoArray &);
static double compute_dland       (double, double);
static void   compute_acerr       (const TrackInfo &, const TrackInfo &,
                                   TimeArray &, NumArray &, NumArray &,
                                   NumArray &);
static void   load_dland          ();
static void   write_output        (const TrackPairInfoArray &);
static void   setup_table         (AsciiTable &);
static void   clean_up            ();
static void   usage               ();
static void   set_adeck           (const StringArray &);
static void   set_bdeck           (const StringArray &);
static void   set_config          (const StringArray &);
static void   set_out             (const StringArray &);
static void   set_logfile         (const StringArray &);
static void   set_verbosity       (const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the tropical cyclone tracks and write output
   process_tracks();

   // Close the text files and deallocate memory
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;
   ConcatString default_config_file;
   int i;

   // Default output file
   out_base = "./out_tcmpr";

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add function calls for the arguments
   cline.add(set_adeck,     "-adeck", -1);
   cline.add(set_bdeck,     "-bdeck", -1);
   cline.add(set_config,    "-config", 1);
   cline.add(set_out,       "-out",    1);
   cline.add(set_logfile,   "-log",    1);
   cline.add(set_verbosity, "-v",      1);

   // Parse the command line
   cline.parse();

   // Set the output file name
   out_file.clear();
   out_file << out_base << tc_stat_file_ext;
   
   // Check for the minimum number of arguments
   if(adeck_source.n_elements() == 0 ||
      bdeck_source.n_elements() == 0 ||
      config_file.length()      == 0) {
      mlog << Error
           << "\nprocess_command_line(int argc, char **argv) -> "
           << "You must specify at least one source of ADECK data, "
           << "BDECK data, and the config file using the \"-adeck\", "
           << "\"-bdeck\", and \"-config\" command line options.\n\n";
      usage();
   }

   // List the input track files
   for(i=0; i<adeck_source.n_elements(); i++)
      mlog << Debug(1)
           << "[Source " << i+1 << "] ADECK Source: " << adeck_source[i] << "\n";
   for(i=0; i<bdeck_source.n_elements(); i++)
      mlog << Debug(1)
           << "[Source " << i+1 << "] BDECK Source: " << bdeck_source[i] << "\n";

   // Create the default config file name
   default_config_file = replace_path(default_config_filename);

   // List the config files
   mlog << Debug(1)
        << "Config File Default: " << default_config_file << "\n"
        << "Config File User: " << config_file << "\n";
        
   // Read the config files
   conf_info.read_config(default_config_file, config_file);

   // Load the distance to land data file
   if(dland_dp.nx() == 0 || dland_dp.ny() == 0) load_dland();
   
   return;
}

////////////////////////////////////////////////////////////////////////

void process_tracks() {
   StringArray adeck_files, bdeck_files, suffix_list;
   TrackLine line;
   TrackInfoArray adeck_tracks, bdeck_tracks;
   TrackPairInfoArray pairs;
   ifstream in;
   int i, j, n_match;

   // Search for files ending in .dat
   suffix_list.add(atcf_suffix);

   // Retrieve the ADECK and BDECK file lists
   adeck_files = get_filenames(adeck_source, suffix_list);
   bdeck_files = get_filenames(bdeck_source, suffix_list);

   // Process the ADECK track files
   mlog << Debug(2)
        << "Processing " << adeck_files.n_elements() << " ADECK file(s).\n";
   process_track_files(adeck_files, adeck_tracks);

   // Process the BDECK track files
   mlog << Debug(2)
        << "Processing " << bdeck_files.n_elements() << " BDECK file(s).\n";
   process_track_files(bdeck_files, bdeck_tracks);

   // Merge 6-hourly TrackPoints into 12-hourly interpolated Tracks
   if(conf_info.Interp12) {
      mlog << Debug(2)
           << "Merging 6-hour TrackPoints into 12-hour interpolated tracks.\n";
      merge_interp12(adeck_tracks);
   }

   // Filter the ADECK tracks using the config file information
   mlog << Debug(2)
        << "Filtering ADECK tracks based on config file settings.\n";
   filter_tracks(adeck_tracks);
   
   // Derive consensus forecasts from the ADECK tracks
   mlog << Debug(2)
        << "Deriving " << conf_info.NCon << " ADECK consensus tracks(s).\n";
   derive_consensus(adeck_tracks);

   mlog << Debug(2)
        << "Matching " << adeck_tracks.n_tracks() << " ADECK tracks to "
        << bdeck_tracks.n_tracks() << " BDECK tracks.\n";
   
   // Loop through the ADECK tracks and find a matching BDECK track
   for(i=0; i<adeck_tracks.n_tracks(); i++) {

      for(j=0,n_match=0; j<bdeck_tracks.n_tracks(); j++) {

         // Check if the BDECK track matches the current ADECK track
         if(adeck_tracks[i].is_match(bdeck_tracks[j])) {
            n_match++;
            mlog << Debug(4)
                 << "[Track " << i+1 << "] ADECK track " << i+1
                 << " matches BDECK track " << j+1 << ":\n"
                 << "    ADECK: " << adeck_tracks[i].serialize() << "\n"
                 << "    BDECK: " << bdeck_tracks[j].serialize() << "\n";

            // Process the matching tracks
            process_match(adeck_tracks[i], bdeck_tracks[j], pairs);
         }
      } // end for j

      // Check for no matching BDECK track
      if(n_match == 0) {
         mlog << Warning
              << "\nADECK track " << i+1 << " matches 0 BDECK tracks:\n"
              << "   ADECK: " << adeck_tracks[i].serialize() << "\n\n";
      }
      else {
         mlog << Debug(3)
              << "[Track " << i+1 << "] ADECK track " << i+1 << " matches "
              << n_match << " BDECK track(s).\n";
      }
   } // end for i

   // Dump out very verbose output
   if(mlog.verbosity_level() >= 5) {
      mlog << Debug(5)
           << pairs.serialize_r() << "\n";
   }

   // Write the output file
   write_output(pairs);
   
   return;
}

////////////////////////////////////////////////////////////////////////

void process_track_files(const StringArray &files,
                         TrackInfoArray &tracks) {
   int i, cur_read, cur_add, tot_read, tot_add;
   ifstream in;
   TrackLine line;

   // Initialize
   tracks.clear();

   // Initialize counts
   tot_read = tot_add = 0;

   // Process each of the ADECK ATCF files
   for(i=0; i<files.n_elements(); i++) {

      // Open the current file
      in.open(files[i]);
      if(!in) {
         mlog << Error
              << "\nprocess_track_files() -> "
              << "unable to open file \"" << files[i] << "\"\n\n";
         exit(1);
      }

      // Initialize counts
      cur_read = cur_add = 0;

      // Read each line in the file
      while(in >> line) {

         // Increment the line counts
         cur_read++;
         tot_read++;
         
         // Add the current line to the TrackInfoArray
         if(tracks.add(line, conf_info.CheckDup)) {
            cur_add++;
            tot_add++;
         }
      }

      // Dump out the current number of lines
      mlog << Debug(4)
           << "[File " << i+1 << "] Used " << cur_add << " of " << cur_read
           << " lines read from file \"" << files[i] << "\"\n";

      // Close the current file
      in.close();

   } // end for i

   // Dump out the total number of lines
   mlog << Debug(3)
        << "Used " << tot_add << " of " << tot_read
        << " lines read from " << files.n_elements() << " file(s).\n";

   // Dump out the track information
   mlog << Debug(3)
        << "Identified " << tracks.n_tracks() << " track(s).\n";

   // Dump out the count of points for each track
   for(i=0; i<tracks.n_tracks(); i++) {
      mlog << Debug(4)
           << "[Track " << i+1 << "] " << tracks[i].serialize() << "\n";
   }

   // Dump out very verbose output
   if(mlog.verbosity_level() >= 5) {
      mlog << Debug(5)
           << tracks.serialize_r() << "\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void filter_tracks(TrackInfoArray &tracks) {
   int i, j;
   int m, d, y, h, mm, s;
   int n_mod, n_bas, n_cyc, n_name, n_init, n_vld, n_hh, n_maski, n_maskv;
   bool status;
   TrackInfoArray t = tracks;
   
   // Initialize
   tracks.clear();
   n_bas = n_cyc = n_mod = n_init = n_vld = n_hh = 0;
   n_maski = n_maskv = 0;
   
   // Loop through the tracks and determine which should be retained
   for(i=0; i<t.n_tracks(); i++) {

      // Check model
      if(conf_info.Model.n_elements() > 0 &&
         !conf_info.Model.has(t[i].technique())) {
         mlog << Debug(4)
              << "Discarding track " << i+1 << " for model mismatch.\n";
         n_mod++;
         continue;
      }
     
      // Check basin
      if(conf_info.Basin.n_elements() > 0 &&
         !conf_info.Basin.has(t[i].basin())) {
         mlog << Debug(4)
              << "Discarding track " << i+1 << " for basin mismatch.\n";
         n_bas++;
         continue;
      }

      // Check cyclone
      if(conf_info.Cyclone.n_elements() > 0 &&
         !conf_info.Cyclone.has(t[i].cyclone())) {
         mlog << Debug(4)
              << "Discarding track " << i+1 << " for cyclone mismatch.\n";
         n_cyc++;
         continue;
      }

      // Check storm name
      if(conf_info.StormName.n_elements() > 0 &&
         !conf_info.StormName.has(t[i].storm_name())) {
         mlog << Debug(4)
              << "Discarding track " << i+1 << " for storm name mismatch.\n";
         n_name++;
         continue;
      }

      // Initialization time window
      if((conf_info.InitBeg > 0 &&
          conf_info.InitBeg > t[i].init()) ||
         (conf_info.InitEnd > 0 &&
          conf_info.InitEnd < t[i].init())) {
         mlog << Debug(4)
              << "Discarding track " << i+1 << " for falling outside of the "
              << "initialization time window.\n";
         n_init++;
         continue;
      }

      // Valid time window
      if((conf_info.ValidBeg > 0 &&
          conf_info.ValidBeg > t[i].valid_min()) ||
         (conf_info.ValidEnd > 0 &&
          conf_info.ValidEnd < t[i].valid_max())) {
         mlog << Debug(4)
              << "Discarding track " << i+1 << " for falling outside of the "
              << "valid time window.\n";
         n_vld++;
         continue;
      }

      // Initialization hour
      unix_to_mdyhms(t[i].init(), m, d, y, h, mm, s);
      if(conf_info.InitHH.n_elements() > 0 &&
         !conf_info.InitHH.has(hms_to_sec(h, mm, s))) {
         mlog << Debug(4)
              << "Discarding track " << i+1 << " for initialization hour "
              << "mismatch.\n";
         n_hh++;
         continue;
      }

      // Initialization location mask
      if(conf_info.InitMask.n_points() > 0 &&
         !conf_info.InitMask.latlon_is_inside_dege(t[i][0].lat(),
                                                   t[i][0].lon())) {
         mlog << Debug(4)
              << "Discarding track " << i+1 << " for falling outside the "
              << "initialization polyline.\n";
         n_maski++;
         continue;
      }

      // Valid location mask
      if(conf_info.ValidMask.n_points() > 0) {

         // Loop over all the points in the current track
         for(j=0,status=true; j<t[i].n_points(); j++) {

            // In the TrackPoint falls outside of the polyline break out
            if(!conf_info.ValidMask.latlon_is_inside_dege(t[i][j].lat(),
                                                          t[i][j].lon())) {
               status = false;
               break;
            }
         } // end for j

         if(!status) {
            mlog << Debug(4)
                 << "Discarding track " << i+1 << " for falling outside the "
                 << "valid polyline.\n";
            n_maskv++;
            continue;
         }
      }

      // If we've made it here, retain this track
      tracks.add(t[i]);
   }

   // Print summary filtering info
   mlog << Debug(3)
        << "Total tracks read       = " << t.n_tracks()      << "\n"
        << "Total tracks kept       = " << tracks.n_tracks() << "\n"
        << "Rejected for model      = " << n_mod             << "\n"
        << "Rejected for basin      = " << n_bas             << "\n"
        << "Rejected for cyclone    = " << n_cyc             << "\n"
        << "Rejected for storm name = " << n_name             << "\n"
        << "Rejected for init time  = " << n_init            << "\n"
        << "Rejected for init hour  = " << n_hh              << "\n"
        << "Rejected for valid time = " << n_vld             << "\n"
        << "Rejected for init mask  = " << n_maski           << "\n"
        << "Rejected for valid mask = " << n_maskv           << "\n";
   
   return;
}

////////////////////////////////////////////////////////////////////////

void merge_interp12(TrackInfoArray &tracks) {
   int i, j;
   ConcatString model;
   TrackInfo merge_track;

   // Loop through the tracks looking for 12-hour interpolated models
   for(i=0; i<tracks.n_tracks(); i++) {

      // Continue if this is not an interpolated model
      if(!tracks[i].is_interp()) continue;

      // Continue if the valid increment is not 12-hours
      if(tracks[i].valid_inc() != 12 * sec_per_hour) continue;

      // Build the model name to be found
      model = tracks[i].technique();
      model.chomp('I');
      model << '2';

      // Loop through the tracks looking for a matching 6-hour model
      for(j=0; j<tracks.n_tracks(); j++) {

         // Match the model name, basin, cyclone, and init time
         if(tracks[j].technique() == model &&
            tracks[j].basin()     == tracks[i].basin() &&
            tracks[j].cyclone()   == tracks[i].cyclone() &&
            tracks[j].init()      == tracks[i].init()) break;
      }

      // Check for no match found
      if(j == tracks.n_tracks()) {
         mlog << Warning
              << "\n[Track " << i+1 << "] Found no 6-hour track to merge "
              << "into 12-hour interpolated track " << i+1 << ":\n"
              << "    12-hour: " << tracks[i].serialize() << "\n\n";
         continue;
      }

      // Merge the 6-hour TrackPoints into the interpolated track
      merge_track = tracks[i];
      merge_track.merge_points(tracks[j]);

      mlog << Debug(4)
           << "[Track " << i+1 << "] Merging 6-hour track " << j+1
           << " into 12-hour interpolated track " << i+1 << ":\n"
           << "     6-hour: " << tracks[j].serialize() << "\n"
           << "    12-hour: " << tracks[i].serialize() << "\n"
           << "     Merged: " << merge_track.serialize() << "\n";
        
      // Store the merged track
      tracks.set(i, merge_track);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void derive_consensus(TrackInfoArray &tracks) {
   int i, j, k, l;
   ConcatString cur_case;
   StringArray case_list, case_cmp;
   TrackInfoArray con_tracks;
   TrackInfo con;
   const char *sep = " ";

   // If no consensus models are defined, nothing to do
   if(conf_info.NCon == 0) return;

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
        << "Buliding consensus track(s) for " << case_list.n_elements()
        << " cases.\n";

   // Loop through the cases and process each consensus model
   for(i=0; i<case_list.n_elements(); i++) {

      // Break the case back out into Basin, Cyclone, InitTime
      cur_case = case_list[i];
      case_cmp = cur_case.split(sep);
     
      // Loop through the consensus models
      for(j=0; j<conf_info.NCon; j++) {

         // Initialize
         con_tracks.clear();
         con.clear();

         // Loop through the consensus members
         for(k=0; k<conf_info.ConMembers[j].n_elements(); k++) {

            // Loop through the tracks looking for a match
            for(l=0; l<tracks.n_tracks(); l++) {

               // If the consenus member was found for this case,
               // add it to the TrackInfoArray object
               if(case_cmp[0] == tracks[l].basin()                         &&
                  case_cmp[1] == tracks[l].cyclone()                       &&
                  case_cmp[2] == unix_to_yyyymmdd_hhmmss(tracks[l].init()) &&
                  conf_info.ConMembers[j][k] == tracks[l].technique()) {
                  con_tracks.add(tracks[l]);
                  break;
               }
            } // end for l
         } // end for k

         // Check that the required number of tracks were found
         if(con_tracks.n_tracks() < conf_info.ConMinReq[j]) {
            mlog << Debug(4)
                 << "[Case " << i+1 << "] For case \"" << case_list[i]
                 << "\" skipping consensus model \"" << conf_info.ConModel[j]
                 << "\" since the minimum number of required members were not found ("
                 << con_tracks.n_tracks() << " < "
                 << conf_info.ConMinReq[j] << ").\n";
            continue;
         }

         // Derive the consensus model from the TrackInfoArray
         con = consensus(con_tracks, conf_info.ConModel[j], conf_info.ConMinReq[j]);

         mlog << Debug(4)
              << "[Case " << i+1 << "] For case \"" << case_list[i]
              << "\" adding consensus model \"" << conf_info.ConModel[j]
              << "\" since the minimum number of required members were found ("
              << con_tracks.n_tracks() << " >= "
              << conf_info.ConMinReq[j] << "):\n"
              << "    " << con.serialize() << "\n";

         mlog << Debug(5)
              << "Adding consensus track:\n" << con.serialize_r(1) << "\n";
         
         // Add the consensus model
         tracks.add(con);

      } // end for j

   } // end for i


   return;
}

////////////////////////////////////////////////////////////////////////

void process_match(const TrackInfo &adeck, const TrackInfo &bdeck,
                   TrackPairInfoArray &p) {
   int i, i_adeck, i_bdeck, i_err;
   TimeArray valid_list, valid_err;
   NumArray tk_err, altk_err, crtk_err;
   double adeck_dland, bdeck_dland, e_tk, e_altk, e_crtk;

   TrackPairInfo pair;
   const TrackPoint *adeck_point = (TrackPoint *) 0;
   const TrackPoint *bdeck_point = (TrackPoint *) 0;
   TrackPoint empty_point;

   // Initialize TrackPairInfo with the current tracks
   pair.initialize(adeck, bdeck);
   
   // Compute the track errors
   compute_acerr(adeck, bdeck, valid_err, tk_err, altk_err, crtk_err);

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
      e_tk = e_altk = e_crtk = bad_data_double;
      
      // Get the errors if ADECK and BDECK are both valid
      if(i_err >= 0 && i_adeck >= 0 && i_bdeck >= 0) {
         e_tk   = tk_err[i_err];
         e_altk = altk_err[i_err];
         e_crtk = crtk_err[i_err];
      }

      mlog << Debug(5)
           << "[Time " << i+1 << "] Valid time "
           << unix_to_yyyymmdd_hhmmss(valid_list[i])
           << ", ADECK: index = " << i_adeck << ", dland = " << adeck_dland
           << ", BDECK: index = " << i_bdeck << ", dland = " << bdeck_dland
           << ", ERROR: track = " << e_tk << ", along = " << e_altk
           << ", cross = " << e_crtk << "\n";

      // Add this info to the TrackPairInfoArray
      pair.add(*adeck_point, *bdeck_point, adeck_dland, bdeck_dland, e_tk, e_altk, e_crtk);
      
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

void compute_acerr(const TrackInfo &adeck, const TrackInfo &bdeck,
                   TimeArray &vld_err, NumArray &tk_err,
                   NumArray &altk_err, NumArray &crtk_err) {
   int i, i_adeck, i_bdeck, status;
   unixtime ut, ut_min, ut_max;
   int ut_inc, n_ut;
   float alat[mxp], alon[mxp], blat[mxp], blon[mxp];
   float crtk[mxp], altk[mxp];

   // Initialize
   vld_err.clear();
   tk_err.clear();
   altk_err.clear();
   crtk_err.clear();

   // Get the valid time range
   ut_min = min(adeck.valid_min(), bdeck.valid_min());
   ut_max = max(adeck.valid_max(), bdeck.valid_max());

   // Use the BDECK spacing to determine the valid increment
   ut_inc = bdeck.valid_inc();
   
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
   
   // Loop through the valid times
   for(i=0; i<n_ut; i++) {
     
      // Add the current valid time
      ut = ut_min+(i*ut_inc);
      vld_err.add(ut);

      // Initialize to bad data
      altk_err.add(bad_data_double);
      crtk_err.add(bad_data_double);
      tk_err.add(bad_data_double);

      // Get the indices for this time
      i_adeck = adeck.valid_index(ut);
      i_bdeck = bdeck.valid_index(ut);

      // Populate array arguments
      if(i_adeck >= 0) alat[i] = adeck[i_adeck].lat();
      else             alat[i] = bad_data_float;
      if(i_adeck >= 0) alon[i] = adeck[i_adeck].lon();
      else             alon[i] = bad_data_float;
      if(i_bdeck >= 0) blat[i] = bdeck[i_bdeck].lat();
      else             blat[i] = bad_data_float;
      if(i_bdeck >= 0) blon[i] = bdeck[i_bdeck].lon();
      else             blon[i] = bad_data_float;
   }

   // Call the ACERR subroutine
   acerr_(blat, blon, alat, alon, &n_ut, crtk, altk, &status);

   // Store the track errors, converting from km to nautical miles
   if(status == 0) {
      for(i=0; i<n_ut; i++) {

         // Along-track error
         if(!is_bad_data(altk[i]))
            altk_err.set(i, tc_nautical_miles_per_km * altk[i]);

         // Cross-track error
         if(!is_bad_data(crtk[i]))
            crtk_err.set(i, tc_nautical_miles_per_km * crtk[i]);

         // Track error
         if(!is_bad_data(altk_err[i]) &&
            !is_bad_data(crtk_err[i]))
            tk_err.set(i, sqrt(altk_err[i]*altk_err[i] +
                               crtk_err[i]*crtk_err[i]));
      }
   }
   
   return;
}

////////////////////////////////////////////////////////////////////////

void load_dland() {
   ConcatString file_name;
   LongArray dim;
   int i;

   // Get the path for the distance to land file
   file_name = replace_path(conf_info.DLandFile);
   
   mlog << Debug(3)
        << "Distance to land file: " << file_name << "\n";
  
   // Open the NetCDF output of the tc_dland tool
   MetNcFile MetNc;
   if(!MetNc.open(file_name)) {
      mlog << Error
           << "\nload_dland() -> "
           << "problem reading file \"" << file_name << "\"\n\n";
      exit(1);
   }

   // Find the first non-lat/lon variable
   for(i=0; i<MetNc.Nvars; i++) {
      if(strcmp(MetNc.Var[i].name, nc_met_lat_var_name) != 0 &&
         strcmp(MetNc.Var[i].name, nc_met_lon_var_name) != 0)
         break;
   }

   // Check range
   if(i == MetNc.Nvars) {
      mlog << Error
           << "\nload_dland() -> "
           << "can't find non-lat/lon variable in file \""
           << file_name << "\"\n\n";
      exit(1);
   }

   // Store the grid
   dland_grid = MetNc.grid;

   // Set the dimension to (*,*)
   dim.add(vx_data2d_star);
   dim.add(vx_data2d_star);

   // Read the data
   if(!MetNc.data(MetNc.Var[i].var, dim, dland_dp)) {
      mlog << Error
           << "\nload_dland() -> "
           << "can't read data from file \""
           << file_name << "\"\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_output(const TrackPairInfoArray &p) {
   int i_row, i;
   TcHdrColumns tchc;   

   // Create the output file
   open_tc_txt_file(out, out_file);

   // Initialize the output AsciiTable
   out_at.set_size(p.n_points() + 1, n_tc_header_cols + n_tc_mpr_cols);
   setup_table(out_at);

   // Write the header row
   write_tc_mpr_header_row(1, out_at, 0, 0);

   // Initialize the row index to 1 to account for the header
   i_row = 1;

   // Store masking regions in the header
   if(conf_info.InitMask.n_points() > 0)  tchc.set_init_mask(conf_info.InitMask.name());
   else                                   tchc.set_init_mask(na_str);
   if(conf_info.ValidMask.n_points() > 0) tchc.set_valid_mask(conf_info.ValidMask.name());
   else                                   tchc.set_valid_mask(na_str);
   
   // Loop through the TrackPairInfo objects
   for(i=0; i<p.n_pairs(); i++) {

      // More header columns
      tchc.set_adeck_model(p[i].adeck().technique());
      tchc.set_bdeck_model(p[i].bdeck().technique());
      tchc.set_basin(p[i].bdeck().basin());
      tchc.set_cyclone(p[i].bdeck().cyclone());
      tchc.set_storm_name(p[i].bdeck().storm_name());
     
     // Write the current TrackPairInfo object
     write_tc_mpr_row(tchc, p[i], out_at, i_row);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_table(AsciiTable &at) {

   // Left-justify all columns
   at.set_table_just(LeftJust);

   // Set the precision
   at.set_precision(default_precision);

   // Set the bad data value
   at.set_bad_data_value(bad_data_double);

   // Set the bad data string
   at.set_bad_data_str(na_str);

   // Don't write out trailing blank rows
   at.set_delete_trailing_blank_rows(1);

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {
  
   // Write the AsciiTable contents and close the output file
   if(out != (ofstream *) 0) {
      *out << out_at;
      
      // List the file being closed
      mlog << Debug(1)
           << "Output file: " << out_file << "\n";

      // Close the output file
      out->close();
      delete out;
      out = (ofstream *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\t-adeck source\n"
        << "\t-bdeck source\n"
        << "\t-config file\n"
        << "\t[-out base]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-adeck source\" is used one or more times to "
        << "specify a file or top-level directory containing ATCF "
        << "model output \"" << atcf_suffix
        << "\" data to process (required).\n"

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
        << mlog.verbosity_level() << ") (optional).\n\n";

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_adeck(const StringArray & a) {
   for(int i=0; i<a.n_elements(); i++) adeck_source.add(a[i]);
}

////////////////////////////////////////////////////////////////////////

void set_bdeck(const StringArray & a) {
   for(int i=0; i<a.n_elements(); i++) bdeck_source.add(a[i]);
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
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////
