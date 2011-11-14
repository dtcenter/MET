// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   ensemble_stat.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    02/01/10  Halley Gotway   New
//   001    09/09/11  Halley Gotway   Call set_grid after reading
//                    gridded observation files.
//   002    10/13/11  Holmes          Added use of command line class to
//                                    parse the command line arguments.
//   003    10/18/11  Halley Gotway   When the user requests verification
//                    in the config file, check that obs files have been
//                    specified on the command line.
//   004    11/14/11  Holmes          Added code to enable reading of
//                                    multiple config files.
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

#include "netcdf.hh"
#include "grib_classes.h"

#include "vx_wrfdata.h"
#include "grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_contable.h"
#include "vx_gsl_prob.h"
#include "result.h"
#include "read_gridded_data.h"
#include "write_netcdf.h"

#include "ensemble_stat.h"

////////////////////////////////////////////////////////////////////////

static void process_command_line  (int, char **);
static void process_ensemble      ();
static void process_vx            ();

static void process_point_vx      ();
static void process_point_obs     (int);
static int  process_point_ens     (int);
static void process_point_scores  ();

static void process_grid_vx       ();
static void process_grid_scores   (WrfData *&, WrfData &, WrfData &,
                                   EnsPairData &);

static void parse_ens_file_list(const char *);
static void set_grid(const Grid &);
static void clear_counts(const WrfData &, int);
static void track_counts(const WrfData &, int);

static void setup_nc_file       (unixtime, int, const char *);
static void setup_txt_files     ();
static void setup_table         (AsciiTable &);

static void build_outfile_name(unixtime, const char *,
                               ConcatString &);

static void write_ens_nc(int, WrfData &);
static void write_ens_var_float(int, float *, const char *, WrfData &, const char *);
static void write_ens_var_int(int, int *, const char *, WrfData &, const char *);

static void write_orank_nc(EnsPairData &, WrfData &, int, int, int);
static void write_orank_var_float(int, int, int, float *, const char *, WrfData &, const char *);
static void write_orank_var_int(int, int, int, int *, const char *, WrfData &, const char *);

static void add_var_att(int, NcVar *, int, WrfData &, const char *);

static void finish_txt_files();
static void clean_up();

static void usage();

static void set_grid_obs(const StringArray &);
static void set_point_obs(const StringArray &);
static void set_ens_valid(const StringArray &);
static void set_ens_lead(const StringArray &);
static void set_obs_valid_beg(const StringArray &);
static void set_obs_valid_end(const StringArray &);
static void set_obs_lead(const StringArray &);
static void set_outdir(const StringArray &);
static void set_verbosity(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the ensemble
   process_ensemble();

   // Only perform verification if requested
   if(vx_flag) process_vx();

   // Close the text files and deallocate memory
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   int i;
   struct stat results;
   char tmp_str[max_str_len], tmp2_str[max_str_len];
   FileType ftype, otype;
   CommandLine cline;
   char default_conf_file[PATH_MAX];


   // Set default output directory
   out_dir << MET_BASE << "/out/ensemble_stat";

   //
   // check for zero arguments
   //
   if (argc == 1)
      usage();

   //
   // parse the command line into tokens
   //
   cline.set(argc, argv);

   //
   // set the usage function
   //
   cline.set_usage(usage);

   //
   // add the options function calls
   //
   cline.add(set_grid_obs, "-grid_obs", 1);
   cline.add(set_point_obs, "-point_obs", 1);
   cline.add(set_ens_valid, "-ens_valid", 1);
   cline.add(set_ens_lead, "-ens_lead", 1);
   cline.add(set_obs_valid_beg, "-obs_valid_beg", 1);
   cline.add(set_obs_valid_end, "-obs_valid_end", 1);
   cline.add(set_obs_lead, "-obs_lead", 1);
   cline.add(set_outdir, "-outdir", 1);
   cline.add(set_verbosity, "-v", 1);

   //
   // parse the command line
   //
   cline.parse();

   //
   // Check for error. There should be either a number followed by that
   // number of filenames or a single filename and a config filename.
   //
   if (cline.n() < 2)
      usage();

   if (cline.n() == 2)
   {
      //
      // it should be a filename and then a config filename
      //
      if (is_integer(cline[0]) == 0)
         parse_ens_file_list(cline[0]);
      else
         usage();

   }
   else
   {
      //
      // More than two arguments. Check that the first is an integer
      // followed by that number of filenames and a config filename.
      //
      if (is_integer(cline[0]) == 1)
      {
         n_ens = atoi(cline[0]);

         if (n_ens <= 0)
         {
            cerr << "\n\nERROR: process_command_line() -> "
                 << "the number of ensemble member files must be >= 1 ("
                 << n_ens << ")\n\n" << flush;
            exit(1);
         }

         if ((cline.n() - 2) == n_ens)
         {
            //
            // Add each of the ensemble members to the list of files.
            //
            for(i = 1; i <= n_ens; i++)
               ens_file_list.add(cline[i]);

         }
         else
            usage();

      }

   }

   //
   // Store the config file name
   //
   config_file = cline[cline.n() - 1];

   // Determine the input forecast file type
   ftype = get_file_type(ens_file_list[0]);

   // Determine the input observation file type
   if( (grid_obs_file_list.n_elements() +
        point_obs_file_list.n_elements()) == 0)
                          otype = NoFileType;
   else if(grid_obs_flag) otype = get_file_type(grid_obs_file_list[0]);
   else                   otype = GbFileType;

   // Check that the end_ut >= beg_ut
   if(obs_valid_beg_ut != (unixtime) 0 &&
      obs_valid_end_ut != (unixtime) 0 &&
      obs_valid_beg_ut > obs_valid_end_ut) {

      unix_to_yyyymmdd_hhmmss(obs_valid_end_ut, tmp_str);
      unix_to_yyyymmdd_hhmmss(obs_valid_beg_ut, tmp2_str);

      cerr << "\n\nERROR: process_command_line() -> "
           << "the ending time (" << tmp_str
           << ") must be greater than the beginning time ("
           << tmp2_str << ").\n\n" << flush;
      exit(1);
   }

   // Read the default config file first and then read the user's
   replace_string(met_base_str, MET_BASE, default_config_filename, default_conf_file);
   if (verbosity > 0)
      cout << "\n\n  Reading default config file \"" << default_conf_file << "\"\n\n" << flush;
   conf_info.read_config(default_conf_file, ftype, otype);
   if (verbosity > 0)
      cout << "\n\n  Reading user config file \"" << config_file << "\"\n\n" << flush;
   conf_info.read_config(config_file, ftype, otype);

   // Set the model name
   shc.set_model(conf_info.conf.model().sval());

   // Allocate arrays to store threshold counts
   na_thresh_count = new NumArray [conf_info.get_max_n_thresh()];

   // Set the random number generator and seed value to be used when
   // computing bootstrap confidence intervals
   rng_set(rng_ptr, conf_info.conf.rng_type().sval(),
           conf_info.conf.rng_seed().sval());

   // List the input files
   if(verbosity > 0) {

      cout << "Configuration File:\n"
           << "   " << config_file << "\n" << flush;

      cout << "Ensemble Files["
           << ens_file_list.n_elements() << "]:\n" << flush;
      for(i=0; i<ens_file_list.n_elements(); i++) {
         cout << "   " << ens_file_list[i]  << "\n" << flush;
      }

      if(grid_obs_file_list.n_elements() > 0) {
         cout << "Gridded Observation Files["
              << grid_obs_file_list.n_elements() << "]:\n" << flush;
         for(i=0; i<grid_obs_file_list.n_elements(); i++) {
            cout << "   " << grid_obs_file_list[i] << "\n" << flush;
         }
      }

      if(point_obs_file_list.n_elements() > 0) {
         cout << "Point Observation Files["
              << point_obs_file_list.n_elements() << "]:\n" << flush;
         for(i=0; i<point_obs_file_list.n_elements(); i++) {
            cout << "   " << point_obs_file_list[i] << "\n" << flush;
         }
      }
   }

   // Check for missing ensemble files
   for(i=0; i<ens_file_list.n_elements(); i++) {

      if(stat(ens_file_list[i], &results) != 0) {
         cout << "\n\n***WARNING***: process_command_line() -> "
              << "can't open input ensemble file: "
              << ens_file_list[i] << "\n\n" << flush;
         ens_file_vld.add(0);
      }
      else {
         ens_file_vld.add(1);
      }
   }

   // Count up the number of valid forecast files
   n_ens_vld = ens_file_vld.sum();

   // Set n_rank as one more than the number of valid forecasts
   n_rank = n_ens_vld + 1;

   // Set flag to indicate whether verification is to be performed
   if((point_obs_flag || grid_obs_flag) &&
      (conf_info.get_n_vx() > 0) &&
      (conf_info.conf.output_flag(i_orank).ival() > 0 ||
       conf_info.conf.output_flag(i_rhist).ival() > 0)) vx_flag = 1;
   else                                                 vx_flag = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void process_ensemble() {
   int i, j, n_miss, n_vld;
   bool status;
   double t;
   Grid data_grid;
   WrfData ens_wd;
   char tmp_str[max_str_len], tmp2_str[max_str_len];

   // Loop through each of the ensemble fields to be processed
   for(i=0; i<conf_info.get_n_ens_var(); i++) {

      if(verbosity > 1) {

         cout << "\n" << sep_str << "\n\n"
              << "Processing ensemble field: "
              << conf_info.ens_gci[i].info_str
              << "\n" << flush;
      }

      // Loop through each of the input forecast files
      for(j=0, n_miss=0, n_vld=0; j<ens_file_list.n_elements(); j++) {

         // If the current forecast file is valid, read the field
         if(ens_file_vld[j]) {

            // Read the gridded data from the input forecast file
            status = read_field(ens_file_list[j], conf_info.ens_gci[i],
                                ens_valid_search_ut, ens_lead_search_sec,
                                ens_wd, data_grid, verbosity);

            // Track the missing and valid counts
            if(!status) {

               cout << "\n\n***WARNING***: process_ensemble() -> "
                    << "Cannot find \"" << conf_info.ens_gci[i].info_str
                    << "\" field in ensemble file: " << ens_file_list[j]
                    << "\n\n" << flush;

               n_miss++;
               continue;
            }
            else {
               n_vld++;
            }

            // Set the grid
            set_grid(data_grid);

            // Store the ensemble valid time, if not already set
            if(ens_valid_ut == (unixtime) 0) {
               ens_valid_ut = ens_wd.get_valid_time();
            }
            // Check to make sure that the valid time doesn't change
            else {

               if(ens_valid_ut != ens_wd.get_valid_time()) {

                  unix_to_yyyymmdd_hhmmss(ens_valid_ut, tmp_str);
                  unix_to_yyyymmdd_hhmmss(ens_wd.get_valid_time(), tmp2_str);

                  cout << "\n\n***WARNING***: process_ensemble() -> "
                       << "The valid time has changed, "
                       << tmp_str << " != " << tmp2_str << ": "
                       << ens_file_list[j] << "\n\n" << flush;
               }
            }

            // Store the lead time
            ens_lead_na.add(ens_wd.get_lead_time());
         }
         else {

            cout << "\n\n***WARNING***: process_ensemble() -> "
                 << "Cannot read ensemble file: "
                 << ens_file_list[j] << "\n\n" << flush;

            n_miss++;
         }

         // Check if the number of missing fields exceeds the threshold
         t = 1.0 - conf_info.conf.vld_ens_thresh().dval();
         if((double) n_miss/n_ens > t) {
            cerr << "\n\nERROR: process_ensemble_files() -> "
                 << n_miss << " missing fields exceeds the maximum "
                 << "allowable specified by \"vld_ens_thresh\" "
                 << "in the configuration file.\n\n" << flush;
            exit(1);
         }

         // Continue if the current field is missing
         if(!status) continue;

         // Create a NetCDF file to store the ensemble output
         if(nc_out == (NcFile *) 0)
            setup_nc_file(ens_wd.get_valid_time(), ens_wd.get_lead_time(),
                          "_ens.nc");

         // Reset the running sums and counts
         if(n_vld==1) clear_counts(ens_wd, i);

         // Apply current data to the running sums and counts
         track_counts(ens_wd, i);

      } // end for j

      // Check if the number of missing fields exceeds the threshold
      t = 1.0 - conf_info.conf.vld_ens_thresh().dval();
      if((double) n_miss/n_ens > t) {
         cerr << "\n\nERROR: process_ensemble_files() -> "
              << n_miss << " missing fields exceeds the maximum "
              << "allowable specified by \"vld_ens_thresh\" "
              << "in the configuration file.\n\n" << flush;
         exit(1);
      }

      // Write out the ensemble information to a NetCDF file
      write_ens_nc(i, ens_wd);
   }

   // Close the output NetCDF file
   if(nc_out) {
      nc_out->close();
      delete nc_out;
      nc_out = (NcFile *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_vx() {

   if(conf_info.get_n_vx() > 0) {

      if(point_obs_file_list.n_elements() == 0 &&
         grid_obs_file_list.n_elements()  == 0) {
         cerr << "\n\nERROR: process_vx() -> "
              << " when \"fcst_field\" is non-empty, you must use "
              << "\"-point_obs\" and/or \"-grid_obs\" to specify the "
              << "verifying observations.\n\n" << flush;
         exit(1);
      }

      // Process masks Grids and Polylines in the config file
      conf_info.process_masks(grid);

      // Setup the GCPairData objects
      conf_info.set_gc_pd();

      // Process the point observations
      if(point_obs_flag) process_point_vx();

      // Process the gridded observations
      if(grid_obs_flag) process_grid_vx();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_point_vx() {
   int i, j, n_miss, miss_flag;
   double t;

   // Set the matching time window.  If obs_valid_beg_ut and obs_valid_end_ut
   // were not set on the command line, use beg_ds and end_ds.
   if(obs_valid_beg_ut == (unixtime) 0) {
      obs_valid_beg_ut = ens_valid_ut + conf_info.conf.beg_ds().ival();
   }
   if(obs_valid_end_ut == (unixtime) 0) {
      obs_valid_end_ut = ens_valid_ut + conf_info.conf.end_ds().ival();
   }

   // Store the valid time window
   for(i=0; i<conf_info.get_n_vx(); i++) {
      conf_info.gc_pd[i].set_beg_ut(obs_valid_beg_ut);
      conf_info.gc_pd[i].set_end_ut(obs_valid_end_ut);
   }

   // Process each point observation NetCDF file
   for(i=0; i<point_obs_file_list.n_elements(); i++) {
      process_point_obs(i);
   }

   // Set the size to store the ensemble data
   for(i=0; i<conf_info.get_n_vx(); i++) {
      conf_info.gc_pd[i].set_ens_size();
   }

   // Process each ensemble file
   for(i=0, n_miss=0; i<ens_file_list.n_elements(); i++) {

      // If the current forecast file is valid, process it
      if(ens_file_vld[i]) miss_flag = process_point_ens(i);
      else                miss_flag = 1;

      // Increment the missing count
      n_miss += miss_flag;

      // Check if the number of missing fields exceeds the threshold
      t = 1.0 - conf_info.conf.vld_ens_thresh().dval();
      if((double) n_miss/n_ens > t) {
         cerr << "\n\nERROR: process_point_vx() -> "
              << n_miss << " missing fields exceeds the maximum "
              << "allowable specified in the configuration file.\n\n"
              << flush;
         exit(1);
      }

      // If a file was missing, add bad data for that ensemble member
      if(miss_flag) {
         for(j=0; j<conf_info.get_n_vx(); j++) conf_info.gc_pd[j].add_miss();
      }
   } // end for i

   // Compute the scores and write them out
   process_point_scores();

   return;
}

////////////////////////////////////////////////////////////////////////

void process_point_obs(int i_nc) {
   int i_obs, j;
   float obs_arr[obs_arr_len], hdr_arr[hdr_arr_len];
   char hdr_typ_str[max_str_len];
   char hdr_sid_str[max_str_len];
   char hdr_vld_str[max_str_len];
   unixtime hdr_ut;
   NcFile *obs_in;

   if(verbosity > 1) {
      cout << "\n" << sep_str << "\n\n"
           << "Processing point observation file: "
           << point_obs_file_list[i_nc] << "\n" << flush;
   }

   // Open the observation file as a NetCDF file.
   obs_in = new NcFile(point_obs_file_list[i_nc]);

   if(!obs_in->is_valid()) {
      obs_in->close();
      delete obs_in;
      obs_in = (NcFile *) 0;

      cout << "\n\n***WARNING***: process_point_obs() -> "
           << "can't open observation netCDF file: "
           << point_obs_file_list[i_nc] << "\n\n" << flush;
      return;
   }

   // Define dimensions
   NcDim *strl_dim    = (NcDim *) 0; // Maximum string length
   NcDim *obs_dim     = (NcDim *) 0; // Number of observations
   NcDim *hdr_dim     = (NcDim *) 0; // Number of PrepBufr messages

   // Define variables
   NcVar *obs_arr_var = (NcVar *) 0;
   NcVar *hdr_typ_var = (NcVar *) 0;
   NcVar *hdr_sid_var = (NcVar *) 0;
   NcVar *hdr_vld_var = (NcVar *) 0;
   NcVar *hdr_arr_var = (NcVar *) 0;

   // Read the dimensions
   strl_dim = obs_in->get_dim("mxstr");
   obs_dim  = obs_in->get_dim("nobs");
   hdr_dim  = obs_in->get_dim("nhdr");

   if(!strl_dim || !strl_dim->is_valid() ||
      !obs_dim  || !obs_dim->is_valid()  ||
      !hdr_dim  || !hdr_dim->is_valid()) {
      cerr << "\n\nERROR: process_point_obs() -> "
           << "can't read \"mxstr\", \"nobs\" or \"nmsg\" "
           << "dimensions from netCDF file: "
           << point_obs_file_list[i_nc] << "\n\n" << flush;
      exit(1);
   }

   // Read the variables
   obs_arr_var = obs_in->get_var("obs_arr");
   hdr_typ_var = obs_in->get_var("hdr_typ");
   hdr_sid_var = obs_in->get_var("hdr_sid");
   hdr_vld_var = obs_in->get_var("hdr_vld");
   hdr_arr_var = obs_in->get_var("hdr_arr");

   if(!obs_arr_var || !obs_arr_var->is_valid() ||
      !hdr_typ_var || !hdr_typ_var->is_valid() ||
      !hdr_sid_var || !hdr_sid_var->is_valid() ||
      !hdr_vld_var || !hdr_vld_var->is_valid() ||
      !hdr_arr_var || !hdr_arr_var->is_valid()) {
      cerr << "\n\nERROR: process_point_obs() -> "
           << "can't read \"obs_arr\", \"hdr_typ\", \"hdr_sid\", "
           << "\"hdr_vld\", or \"hdr_arr\" variables from netCDF file: "
           << point_obs_file_list[i_nc] << "\n\n" << flush;
      exit(1);
   }

   if(verbosity > 1) {
      cout << "Searching " << obs_dim->size()
           << " observations from " << hdr_dim->size()
           << " header messages.\n" << flush;
   }

   // Process each observation in the file
   for(i_obs=0; i_obs<obs_dim->size(); i_obs++) {

      // Read the current observation message
      if(!obs_arr_var->set_cur((long) i_obs)
      || !obs_arr_var->get(obs_arr, 1, obs_arr_len)) {
         cerr << "\n\nERROR: process_point_obs() -> "
              << "can't read the record for observation "
              << "number " << i_obs << "\n\n" << flush;
         exit(1);
      }

      // Read the corresponding header array for this observation
      if(!hdr_arr_var->set_cur((long) (obs_arr[0])) ||
         !hdr_arr_var->get(hdr_arr, 1, hdr_arr_len)) {
         cerr << "\n\nERROR: process_point_obs() -> "
              << "can't read the header array record for header "
              << "number " << obs_arr[0] << "\n\n" << flush;
         exit(1);
      }

      // Read the corresponding header type for this observation
      if(!hdr_typ_var->set_cur((long) (obs_arr[0])) ||
         !hdr_typ_var->get(hdr_typ_str, 1, strl_dim->size())) {
         cerr << "\n\nERROR: process_point_obs() -> "
              << "can't read the message type record for header "
              << "number " << obs_arr[0] << "\n\n" << flush;
         exit(1);
      }

      // Read the corresponding header Station ID for this observation
      if(!hdr_sid_var->set_cur((long) (obs_arr[0])) ||
         !hdr_sid_var->get(hdr_sid_str, 1, strl_dim->size())) {
         cerr << "\n\nERROR: process_point_obs() -> "
              << "can't read the station ID record for header "
              << "number " << obs_arr[0] << "\n\n" << flush;
         exit(1);
      }

      // Read the corresponding valid time for this observation
      if(!hdr_vld_var->set_cur((long) (obs_arr[0])) ||
         !hdr_vld_var->get(hdr_vld_str, 1, strl_dim->size())) {
         cerr << "\n\nERROR: process_point_obs() -> "
              << "can't read the valid time for header "
              << "number " << obs_arr[0] << "\n\n" << flush;
         exit(1);
      }

      // Convert string to a unixtime
      hdr_ut = timestring_to_unix(hdr_vld_str);

      // Check each conf_info.gc_pd object to see if this observation
      // should be added
      for(j=0; j<conf_info.get_n_vx(); j++) {

         // Attempt to add the observation to the conf_info.gc_pd object
         conf_info.gc_pd[j].add_obs(hdr_arr, hdr_typ_str, hdr_sid_str,
                                    hdr_ut, obs_arr, grid);
      }
   } // end for i_obs

   // Deallocate and clean up
   obs_in->close();
   delete obs_in;
   obs_in = (NcFile *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////

int process_point_ens(int i_ens) {
   int i, j, n_fcst_rec;
   NumArray fcst_lvl_na;
   Grid data_grid;
   WrfData *fcst_wd  = (WrfData *) 0;

   if(verbosity > 1) {
      cout << "\n" << sep_str << "\n\n"
           << "Processing ensemble file: "
           << ens_file_list[i_ens] << "\n" << flush;
   }

   // Loop through each of the fields to be verified and extract
   // the forecast fields for verification
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Read the gridded data from the input forecast file
      n_fcst_rec = read_field_levels(ens_file_list[i_ens], conf_info.gc_pd[i].fcst_gci,
                                     ens_valid_search_ut, ens_lead_search_sec,
                                     fcst_wd, fcst_lvl_na, data_grid, verbosity);

      // Check for zero fields
      if(n_fcst_rec == 0) {
         cout << "\n\n***WARNING***: process_point_ens() -> "
              << "no records matching "
              << conf_info.gc_pd[i].fcst_gci.info_str
              << " found in file: "
              << ens_file_list[i_ens] << "\n" << flush;
         return(1);
      }

      // Set the grid
      set_grid(data_grid);

      // Dump out the number of levels found
      if(verbosity > 1) {
         cout << "For "
              << conf_info.gc_pd[i].fcst_gci.info_str
              << " found " << n_fcst_rec
              << " forecast levels.\n" << flush;
      }

      // Set the number of pointers to the raw forecast field
      conf_info.gc_pd[i].set_n_fcst(n_fcst_rec);

      // Set the forecast fields
      for(j=0; j<n_fcst_rec; j++) {

         // Store information for the raw forecast fields
         conf_info.gc_pd[i].set_fcst_lvl(j, fcst_lvl_na[j]);
         conf_info.gc_pd[i].set_fcst_wd_ptr(j, &fcst_wd[j]);
      } // end for j

      // Compute forecast values for this ensemble member
      conf_info.gc_pd[i].add_ens();

      // Delete allocated WrfData objects
      if(fcst_wd) { delete [] fcst_wd; fcst_wd = (WrfData *) 0; }

   } // end for i

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_point_scores() {
   EnsPairData *pd_ptr;
   int i, j, k, l;

   if(verbosity > 1) {
      cout << "\n" << sep_str << "\n\n"
           << flush;
   }

   // Create output text files as requested in the config file
   setup_txt_files();

   // Store the forecast lead time
   shc.set_fcst_lead_sec(ens_lead_na.mode());

   // Store the forecast valid time
   shc.set_fcst_valid_beg(ens_valid_ut);
   shc.set_fcst_valid_end(ens_valid_ut);

   // Loop through the Ensemble pair data objects, compute the scores
   // requested, and write the output.
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Store the forecast variable name
      shc.set_fcst_var(conf_info.gc_pd[i].fcst_gci.abbr_str.text());

      // Set the forecast level name
      shc.set_fcst_lev(conf_info.gc_pd[i].fcst_gci.lvl_str.text());

      // Store the observation variable name
      shc.set_obs_var(conf_info.gc_pd[i].obs_gci.abbr_str.text());

      // Set the observation level name
      shc.set_obs_lev(conf_info.gc_pd[i].obs_gci.lvl_str.text());

      // Set the observation lead time
      shc.set_obs_lead_sec(0);

      // Set the observation valid time
      shc.set_obs_valid_beg(conf_info.gc_pd[i].beg_ut);
      shc.set_obs_valid_end(conf_info.gc_pd[i].end_ut);

      // Loop through the message types
      for(j=0; j<conf_info.get_n_msg_typ(); j++) {

         // Store the message type
         shc.set_msg_typ(conf_info.msg_typ[j]);

         // Loop through the verification masking regions
         for(k=0; k<conf_info.get_n_mask(); k++) {

            // Store the verification masking region
            shc.set_mask(conf_info.mask_name[k]);

            // Loop through the interpolation methods
            for(l=0; l<conf_info.get_n_interp(); l++) {

               // Store the interpolation method and width being applied
               shc.set_interp_mthd(conf_info.interp_mthd[l]);
               shc.set_interp_wdth(conf_info.interp_wdth[l]);

               pd_ptr = &conf_info.gc_pd[i].pd[j][k][l];

               // Compute the ranks for the observations
               pd_ptr->compute_rank(n_ens_vld, rng_ptr);

               if(verbosity > 1) {

                  cout << "Processing point verification "
                       << conf_info.gc_pd[i].fcst_gci.info_str
                       << " versus "
                       << conf_info.gc_pd[i].obs_gci.info_str
                       << ", for observation type " << pd_ptr->msg_typ
                       << ", over region " << pd_ptr->mask_name
                       << ", for interpolation method "
                       << shc.get_interp_mthd_str() << "("
                       << shc.get_interp_pnts_str()
                       << "), using " << pd_ptr->n_pair << " pairs.\n"
                       << flush;
               }

               // Continue if there are no points
               if(pd_ptr->n_pair == 0) continue;

               // Compute ensemble statistics
               pd_ptr->compute_rhist(n_ens_vld);
               pd_ptr->compute_stats(n_ens_vld);

               // Write out the ORANK lines
               if(conf_info.conf.output_flag(i_orank).ival()) {

                  write_orank_row(shc, pd_ptr,
                     conf_info.conf.output_flag(i_orank).ival(),
                     stat_at, i_stat_row,
                     txt_at[i_orank], i_txt_row[i_orank]);

                  // Reset the observation valid time
                  shc.set_obs_valid_beg(conf_info.gc_pd[i].beg_ut);
                  shc.set_obs_valid_end(conf_info.gc_pd[i].end_ut);
               }

               // Compute RHIST scores
               if(conf_info.conf.output_flag(i_rhist).ival()) {

                  write_rhist_row(shc, pd_ptr,
                     conf_info.conf.output_flag(i_rhist).ival(),
                     stat_at, i_stat_row,
                     txt_at[i_rhist], i_txt_row[i_rhist]);
               }

            } // end for l
         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void process_grid_vx() {
   int i, j, k, n_miss;
   bool status;
   double t;
   char tmp_str[max_str_len];
   WrfData *fcst_wd, *fcst_wd_smooth;
   WrfData  obs_wd,   obs_wd_smooth;
   unixtime valid_ut;
   Grid data_grid;
   EnsPairData pd;

   if(verbosity > 1) {
      cout << "\n" << sep_str << "\n\n"
           << flush;
   }

   // Create output text files as requested in the config file
   setup_txt_files();

   // Allocate space to store the forecast fields
   fcst_wd        = new WrfData [n_ens];
   fcst_wd_smooth = new WrfData [n_ens];

   // Loop through each of the fields to be verified
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Set the forecast lead time
      shc.set_fcst_lead_sec(ens_lead_na.mode());

      // Set the forecast valid time
      shc.set_fcst_valid_beg(ens_valid_ut);
      shc.set_fcst_valid_end(ens_valid_ut);

      // Set the forecast variable name
      shc.set_fcst_var(conf_info.gc_pd[i].fcst_gci.abbr_str.text());

      // Set the forecast level name
      shc.set_fcst_lev(conf_info.gc_pd[i].fcst_gci.lvl_str.text());

      // Set the observation variable name
      shc.set_obs_var(conf_info.gc_pd[i].obs_gci.abbr_str.text());

      // Set the observation level name
      shc.set_obs_lev(conf_info.gc_pd[i].obs_gci.lvl_str.text());

      // Set the observation type
      if(conf_info.gc_pd[i].fcst_gci.code == apcp_grib_code &&
         conf_info.gc_pd[i].obs_gci.code  == apcp_grib_code) {
         shc.set_msg_typ("MC_PCP");
      }
      else {
         shc.set_msg_typ("ANALYS");
      }

      // Loop through each of the input ensemble files
      for(j=0, n_miss=0; j<ens_file_list.n_elements(); j++) {

         // Initialize
         fcst_wd[j].clear();

         // If the current ensemble file is valid, read the field
         if(ens_file_vld[j]) {

            // Read the gridded data from the input forecast file
            status = read_field(ens_file_list[j], conf_info.gc_pd[i].fcst_gci,
                                ens_valid_search_ut, ens_lead_search_sec,
                                fcst_wd[j], data_grid, verbosity);

            // Count the number of missing files
            if(!status) {
               n_miss++;
               continue;
            }

            // Set the grid
            set_grid(data_grid);
         }
         else {
            n_miss++;
         }

         // Check if the number of missing fields exceeds the threshold
         t = 1.0 - conf_info.conf.vld_ens_thresh().dval();
         if((double) n_miss/n_ens > t) {
            cerr << "\n\nERROR: process_grid_vx() -> "
                 << n_miss << " missing fields exceeds the maximum "
                 << "allowable specified in the configuration file.\n\n"
                 << flush;
            exit(1);
         }
      } // end for j

      // Create a NetCDF file to store the verification matched pairs
      if(nc_out == (NcFile *) 0)
         setup_nc_file(fcst_wd[j].get_valid_time(), fcst_wd[j].get_lead_time(),
                       "_orank.nc");

      // Read the observation file
      for(j=0, n_miss=0; j<grid_obs_file_list.n_elements(); j++) {

         // Get the valid time to be extracted
         if(obs_valid_beg_ut == obs_valid_end_ut) {
            valid_ut = obs_valid_beg_ut;
         }
         else {
            valid_ut = (unixtime) 0;
         }

         // Read the gridded data from the input observation file
         status = read_field(grid_obs_file_list[j], conf_info.gc_pd[i].obs_gci,
                             valid_ut, obs_lead_sec,
                             obs_wd, data_grid, verbosity);

         // If found, break out of the loop
         if(!status) n_miss++;
         else {

            // Set the grid
            set_grid(data_grid);

            break;
         }

      }

      // Check if the observation field was found
      if(n_miss == grid_obs_file_list.n_elements()) {
         cerr << "\n\nERROR: process_grid_vx() -> "
              << conf_info.gc_pd[i].obs_gci.info_str
              << " not found in observation files.\n"
              << flush;
         exit(1);
      }

      // Set the observation lead time
      shc.set_obs_lead_sec(obs_wd.get_lead_time());

      // Set the observation valid time
      shc.set_obs_valid_beg(obs_wd.get_valid_time());
      shc.set_obs_valid_end(obs_wd.get_valid_time());

      // Loop through and apply each of the smoothing operations
      for(j=0; j<conf_info.get_n_interp(); j++) {

         // Check for allowable smoothing operation
         if(conf_info.interp_mthd[j] == im_dw_mean ||
            conf_info.interp_mthd[j] == im_ls_fit  ||
            conf_info.interp_mthd[j] == im_bilin   ||
            conf_info.interp_mthd[j] == im_nbrhd) {

            interpmthd_to_string(conf_info.interp_mthd[j], tmp_str);
            cout << "\n\n***WARNING***: process_grid_vx() -> "
                 << tmp_str << " smoothing option not supported for "
                 << "gridded observations.\n\n" << flush;
            continue;
         }

         // Set the interpolation method and width
         shc.set_interp_mthd(conf_info.interp_mthd[j]);
         shc.set_interp_wdth(conf_info.interp_wdth[j]);

         // If requested in the config file, smooth the forecast fields
         for(k=0; k<ens_file_list.n_elements(); k++) {
            if(conf_info.conf.interp_flag().ival() == 1 ||
               conf_info.conf.interp_flag().ival() == 3) {
               fcst_wd_smooth[k] = smooth_field(fcst_wd[k],
                                      conf_info.interp_mthd[j],
                                      conf_info.interp_wdth[j],
                                      conf_info.conf.interp_thresh().dval());
            }
            // Do not smooth the forecast field
            else {
               fcst_wd_smooth[k] = fcst_wd[k];
            }
         } // end for k

         // If requested in the config file, smooth the observation field
         if(conf_info.conf.interp_flag().ival() == 2 ||
            conf_info.conf.interp_flag().ival() == 3) {
            obs_wd_smooth  = smooth_field(obs_wd,
                                conf_info.interp_mthd[j],
                                conf_info.interp_wdth[j],
                                conf_info.conf.interp_thresh().dval());
         }
         // Do not smooth the observation field
         else {
            obs_wd_smooth = obs_wd;
         }

         // Loop through the masks to be applied
         for(k=0; k<conf_info.get_n_mask(); k++) {

            // Set the mask name
            shc.set_mask(conf_info.mask_name[k]);

            // Initialize
            pd.clear();

            // Apply the current mask to the fields and compute the pairs
            process_grid_scores(fcst_wd_smooth, obs_wd_smooth,
                                conf_info.mask_wd[k], pd);

            if(verbosity > 1) {

               cout << "Processing gridded verification "
                    << conf_info.gc_pd[i].fcst_gci.info_str
                    << " versus "
                    << conf_info.gc_pd[i].obs_gci.info_str
                    << ", for observation type " << shc.get_msg_typ()
                    << ", over region " << shc.get_mask()
                    << ", for interpolation method "
                    << shc.get_interp_mthd_str() << "("
                    << shc.get_interp_pnts_str()
                    << "), using " << pd.n_pair << " pairs.\n"
                    << flush;
            }

            // Continue if there are no points
            if(pd.n_pair == 0) continue;

            // Compute ensemble statistics
            pd.compute_rhist(n_ens_vld);
            pd.compute_stats(n_ens_vld);

            // Compute RHIST scores
            if(conf_info.conf.output_flag(i_rhist).ival()) {

               write_rhist_row(shc, &pd,
                  conf_info.conf.output_flag(i_rhist).ival(),
                  stat_at, i_stat_row,
                  txt_at[i_rhist], i_txt_row[i_rhist]);
            }

            // Write out the smoothed forecast, observation, and difference
            if(conf_info.conf.output_flag(i_nc_orank).ival())
               write_orank_nc(pd, obs_wd_smooth, i, j, k);

         } // end for k
      } // end for j
   } // end for i

   // Delete allocated WrfData objects
   if(fcst_wd)        { delete [] fcst_wd;        fcst_wd        = (WrfData *) 0; }
   if(fcst_wd_smooth) { delete [] fcst_wd_smooth; fcst_wd_smooth = (WrfData *) 0; }

   // Close the output NetCDF file
   if(nc_out) {
      nc_out->close();
      delete nc_out;
      nc_out = (NcFile *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_grid_scores(WrfData *&fcst_wd, WrfData &obs_wd,
                         WrfData &mask_wd, EnsPairData &pd) {
   int i, j, x, y;
   double v;

   // Loop through the observation field
   for(x=0; x<obs_wd.get_nx(); x++) {
      for(y=0; y<obs_wd.get_ny(); y++) {

         // Skip any grid points containing bad data values or where the
         // verification masking region is turned off
         if(obs_wd.is_bad_xy(x, y) || !mask_wd.s_is_on(x, y) ) continue;

         // Add the observation point
         pd.add_obs(x, y, obs_wd.get_xy_double(x, y));

      } // end for y
   } // end for x

   // Allocate space for the ensemble data
   pd.set_size();

   // Loop through the observation points
   for(i=0; i<pd.n_pair; i++) {

      x = nint(pd.x_na[i]);
      y = nint(pd.y_na[i]);

      // Loop through each of the ensemble members
      for(j=0; j<n_ens; j++) {

         // Look for missing data
         if(fcst_wd[j].get_nx() == 0 || fcst_wd[j].get_ny() == 0) {
            v = bad_data_double;
         }
         // Get the ensemble value
         else {
            v = fcst_wd[j].get_xy_double(x, y);
         }

         // Store the ensemble value
         pd.add_ens(i, v);
      } // end for j
   } // end for i

   // Compute the ranks and valid counts
   if(pd.n_pair > 0) pd.compute_rank(n_ens_vld, rng_ptr);

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_ens_file_list(const char *fcst_file) {
   char tmp_str[PATH_MAX];
   ifstream f_in;
   FileType type;

   // Check the input file type
   type = get_file_type(fcst_file);
   if(type != NoFileType) {
      cerr << "\n\nERROR: parse_ens_file_list() -> "
           << "the input ensemble file list \"" << fcst_file
           << "\" is not correct." << "\n\n" << flush;
      exit(1);
   }

   // Open the ensemble file list
   f_in.open(fcst_file);
   if(!f_in) {
      cerr << "\n\nERROR: parse_ens_file_list() -> "
           << "can't open input ensemble file list \"" << fcst_file
           << "\" for reading\n\n" << flush;
      exit(1);
   }

   // Read each ensemble member listed in the file
   while(f_in >> tmp_str) {
      ens_file_list.add(tmp_str);
      n_ens++;
   }

   // Close the input file
   f_in.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void set_grid(const Grid &gr) {

   // Set the grid, if not already set
   if(grid.nx() == 0 && grid.ny() == 0) {
      grid = gr;
   }
   // Check to make sure that the grid doesn't change
   else {
      if(!(grid == gr)) {
         cerr << "\n\nERROR: set_field() -> "
              << "All data files must be on the same grid!\n\n"
              << flush;
         exit(1);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void clear_counts(const WrfData &wd, int i_gc) {
   int i, j;

   // Clear arrays
   na_count.clear();
   na_min.clear();
   na_max.clear();
   na_sum.clear();
   na_sum_sq.clear();
   for(i=0; i<conf_info.get_max_n_thresh(); i++) {
      na_thresh_count[i].clear();
   }

   // Initialize arrays
   for(i=0; i<wd.get_nx()*wd.get_ny(); i++) {
      na_count.add(0);
      na_min.add(bad_data_double);
      na_max.add(bad_data_double);
      na_sum.add(0.0);
      na_sum_sq.add(0.0);
      for(j=0; j<conf_info.get_max_n_thresh(); j++) {
         na_thresh_count[j].add(0);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void track_counts(const WrfData &wd, int i_gc) {
   int x, y, n, i;
   double v;

   for(x=0; x<wd.get_nx(); x++) {
      for(y=0; y<wd.get_ny(); y++) {

         n = wd.two_to_one(x, y);
         v = wd.get_xy_double(x, y);

         // Skip the bad data value
         if(is_bad_data(v)) continue;

         // Otherwise, add to counts and sums
         else {
            na_count.set(n, na_count[n] + 1);

            if(v <= na_min[n] || is_bad_data(na_min[n])) na_min.set(n, v);
            if(v >= na_max[n] || is_bad_data(na_max[n])) na_max.set(n, v);

            na_sum.set(n, na_sum[n] + v);
            na_sum_sq.set(n, na_sum_sq[n] + v*v);

            for(i=0; i<conf_info.ens_ta[i_gc].n_elements(); i++) {
               if(conf_info.ens_ta[i_gc][i].check(v))
                  na_thresh_count[i].set(n, na_thresh_count[i][n] + 1);
            }
         }

      } // end for y
   } // end for x

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_nc_file(unixtime valid_ut, int lead_sec, const char *suffix) {
   ConcatString out_nc_file;

   // Create output NetCDF file name
   build_outfile_name(ens_valid_ut, suffix, out_nc_file);

   // Create a new NetCDF file and open it
   nc_out = new NcFile(out_nc_file, NcFile::Replace);

   if(!nc_out->is_valid()) {
      cerr << "\n\nERROR: setup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_nc_file << "\n\n" << flush;
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, out_nc_file.text(), program_name);

   // Add the projection information
   write_netcdf_proj(nc_out, grid);

   // Define Dimensions
   lat_dim = nc_out->add_dim("lat",   (long) grid.ny());
   lon_dim = nc_out->add_dim("lon",   (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(nc_out, lat_dim, lon_dim, grid);

   // Append to the list of output files
   out_nc_file_list.add(out_nc_file);

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_txt_files() {
   int  i, max_col;
   ConcatString tmp_str;

   // Check to see if the text files have already been set up
   if(stat_at.nrows() > 0 || stat_at.ncols() > 0) return;

   // Create output file names for the stat file and optional text files
   build_outfile_name(ens_valid_ut, "", tmp_str);

   /////////////////////////////////////////////////////////////////////
   //
   // Setup the output STAT file
   //
   /////////////////////////////////////////////////////////////////////

   // Get the maximum number of data columns
   max_col = max(get_n_orank_columns(n_ens+1),
                 get_n_rhist_columns(n_ens))
             + n_header_columns;

   // Initialize file stream
   stat_out = (ofstream *) 0;

   // Build the file name
   stat_file << tmp_str << stat_file_ext;

   // Create the output STAT file
   open_txt_file(stat_out, stat_file, verbosity);

   // Setup the STAT AsciiTable
   stat_at.set_size(conf_info.n_stat_row() + 1, max_col);
   setup_table(stat_at);

   // Write the text header row
   write_header_row((const char **) 0, 0, 1, stat_at, 0, 0);

   // Initialize the row index to 1 to account for the header
   i_stat_row = 1;

   /////////////////////////////////////////////////////////////////////
   //
   // Setup each of the optional output text files
   //
   /////////////////////////////////////////////////////////////////////

   // Loop through output file type
   for(i=0; i<n_txt; i++) {

      // Only set it up if requested in the config file
      if(conf_info.conf.output_flag(i).ival() >= flag_txt_out) {

         // Only create ORANK file when using point observations
         if(i == i_orank && !point_obs_flag) continue;

         // Initialize file stream
         txt_out[i] = (ofstream *) 0;

         // Build the file name
         txt_file[i] << tmp_str << "_" << txt_file_abbr[i]
                     << txt_file_ext;

         // Create the output text file
         open_txt_file(txt_out[i], txt_file[i], verbosity);

         // Get the maximum number of columns for this line type
         switch(i) {

            case(i_rhist):
               max_col = get_n_rhist_columns(n_rank) + n_header_columns + 1;
               break;

            case(i_orank):
               max_col = get_n_orank_columns(n_ens) + n_header_columns + 1;
               break;

            default:
               max_col = n_txt_columns[i]  + n_header_columns + 1;
               break;
         } // end switch

         // Setup the text AsciiTable
         txt_at[i].set_size(conf_info.n_txt_row(i) + 1, max_col);
         setup_table(txt_at[i]);

         // Write the text header row
         switch(i) {

            case(i_rhist):
               write_rhist_header_row(1, n_rank, txt_at[i], 0, 0);
               break;

            case(i_orank):
               write_orank_header_row(1, n_ens, txt_at[i], 0, 0);
               break;

            default:
               write_header_row(txt_columns[i], n_txt_columns[i], 1,
                  txt_at[i], 0, 0);
               break;
         } // end switch

         // Initialize the row index to 1 to account for the header
         i_txt_row[i] = 1;
      }
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

void build_outfile_name(unixtime ut, const char *suffix, ConcatString &str) {
   int mon, day, yr, hr, min, sec;
   char tmp_str[max_str_len];

   //
   // Create output file name
   //

   // Initialize
   str.clear();

   // Append the output directory and program name
   str << out_dir.text() << "/" << program_name;

   // Append the output prefix, if defined
   if(strlen(conf_info.conf.output_prefix().sval()) > 0)
      str << "_" << conf_info.conf.output_prefix().sval();

   // Append the timing information
   unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
   sprintf(tmp_str, "%.4i%.2i%.2i_%.2i%.2i%.2iV",
           yr, mon, day, hr, min, sec);
   str << "_" << tmp_str;

   // Append the suffix
   str << suffix;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ens_nc(int i_gc, WrfData &wd) {
   int i, j;
   double t, v;
   char thresh_str[max_str_len], var_str[max_str_len];

   // Arrays for storing ensemble data
   float *ens_mean  = (float *) 0;
   float *ens_stdev = (float *) 0;
   float *ens_minus = (float *) 0;
   float *ens_plus  = (float *) 0;
   float *ens_min   = (float *) 0;
   float *ens_max   = (float *) 0;
   float *ens_range = (float *) 0;
   int   *ens_vld   = (int   *) 0;
   float *ens_freq  = (float *) 0;

   // Allocate memory for storing ensemble data
   ens_mean  = new float [grid.nx()*grid.ny()];
   ens_stdev = new float [grid.nx()*grid.ny()];
   ens_minus = new float [grid.nx()*grid.ny()];
   ens_plus  = new float [grid.nx()*grid.ny()];
   ens_min   = new float [grid.nx()*grid.ny()];
   ens_max   = new float [grid.nx()*grid.ny()];
   ens_range = new float [grid.nx()*grid.ny()];
   ens_vld   = new int   [grid.nx()*grid.ny()];
   ens_freq  = new float [grid.nx()*grid.ny()];

   // Store the threshold for the ratio of valid data points
   t = conf_info.conf.vld_data_thresh().dval();

   // Store the data
   for(i=0; i<na_count.n_elements(); i++) {

      // Valid data count
      ens_vld[i] = nint(na_count[i]);

      // Check for too much missing data
      if((double) (na_count[i]/n_ens) < t) {
         ens_mean[i]  = bad_data_float;
         ens_stdev[i] = bad_data_float;
         ens_minus[i] = bad_data_float;
         ens_plus[i]  = bad_data_float;
         ens_min[i]   = bad_data_float;
         ens_max[i]   = bad_data_float;
         ens_range[i] = bad_data_float;
      }
      else {
         // Compute ensemble summary
         ens_mean[i]  = (float) na_sum[i]/na_count[i];
         ens_stdev[i] = (float) compute_stdev(na_sum[i], na_sum_sq[i], ens_vld[i]);
         ens_minus[i] = (float) ens_mean[i] - ens_stdev[i];
         ens_plus[i]  = (float) ens_mean[i] + ens_stdev[i];
         ens_min[i]   = (float) na_min[i];
         ens_max[i]   = (float) na_max[i];
         v = na_max[i] - na_min[i];
         if(is_eq(v, 0.0)) v = 0;
         ens_range[i] = (float) v;
      }
   } // end for i

   // Add the ensemble mean if requested
   if(conf_info.conf.output_flag(i_nc_mean).ival()) {
      write_ens_var_float(i_gc, ens_mean, "ENS_MEAN", wd,
                          "Ensemble Mean");
   }

   // Add the ensemble standard deviation if requested
   if(conf_info.conf.output_flag(i_nc_stdev).ival()) {
      write_ens_var_float(i_gc, ens_stdev, "ENS_STDEV", wd,
                          "Ensemble Standard Deviation");
   }

   // Add the ensemble mean minus one standard deviation if requested
   if(conf_info.conf.output_flag(i_nc_minus).ival()) {
      write_ens_var_float(i_gc, ens_minus, "ENS_MINUS", wd,
                          "Ensemble Mean Minus 1 Standard Deviation");
   }

   // Add the ensemble mean plus one standard deviation if requested
   if(conf_info.conf.output_flag(i_nc_plus).ival()) {
      write_ens_var_float(i_gc, ens_plus, "ENS_PLUS", wd,
                          "Ensemble Mean Plus 1 Standard Deviation");
   }

   // Add the ensemble minimum value if requested
   if(conf_info.conf.output_flag(i_nc_min).ival()) {
      write_ens_var_float(i_gc, ens_min, "ENS_MIN", wd,
                          "Ensemble Minimum");
   }

   // Add the ensemble maximum value if requested
   if(conf_info.conf.output_flag(i_nc_max).ival()) {
      write_ens_var_float(i_gc, ens_max, "ENS_MAX", wd,
                          "Ensemble Maximum");
   }

   // Add the ensemble range if requested
   if(conf_info.conf.output_flag(i_nc_range).ival()) {
      write_ens_var_float(i_gc, ens_range, "ENS_RANGE", wd,
                          "Ensemble Range");
   }

   // Add the ensemble valid data count if requested
   if(conf_info.conf.output_flag(i_nc_vld).ival()) {
      write_ens_var_int(i_gc, ens_vld, "ENS_VLD", wd,
                        "Ensemble Valid Data Count");
   }

   // Add the ensemble relative frequencies if requested
   if(conf_info.conf.output_flag(i_nc_freq).ival()) {

      // Loop through each threshold
      for(i=0; i<conf_info.ens_ta[i_gc].n_elements(); i++) {

         conf_info.ens_ta[i_gc][i].get_abbr_str(thresh_str);
         sprintf(var_str, "ENS_FREQ_%s", thresh_str);

         // Store the data
         for(j=0; j<na_count.n_elements(); j++) {

            // Check for too much missing data
            if((double) (na_count[j]/n_ens) < t) {
               ens_freq[j] = bad_data_float;
            }
            else {
               ens_freq[j] = (float) (na_thresh_count[i][j]/na_count[j]);
            }

         } // end for j

         // Write the ensemble relative frequency
         write_ens_var_float(i_gc, ens_freq, var_str, wd,
                             "Ensemble Relative Frequency");

      } // end for i
   } // end if

   // Deallocate and clean up
   if(ens_mean)  { delete [] ens_mean;  ens_mean  = (float *) 0; }
   if(ens_stdev) { delete [] ens_stdev; ens_stdev = (float *) 0; }
   if(ens_minus) { delete [] ens_minus; ens_minus = (float *) 0; }
   if(ens_plus)  { delete [] ens_plus;  ens_plus  = (float *) 0; }
   if(ens_min)   { delete [] ens_min;   ens_min   = (float *) 0; }
   if(ens_max)   { delete [] ens_max;   ens_max   = (float *) 0; }
   if(ens_range) { delete [] ens_range; ens_range = (float *) 0; }
   if(ens_vld)   { delete [] ens_vld;   ens_vld   = (int   *) 0; }
   if(ens_freq)  { delete [] ens_freq;  ens_freq  = (float *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ens_var_float(int i_gc, float *ens_data, const char *var_str,
                         WrfData &wd, const char *long_name_str) {
   NcVar *ens_var;
   char ens_var_name[max_str_len];

   // Construct the variable name
   sprintf(ens_var_name, "%s_%s_%s",
           conf_info.ens_gci[i_gc].abbr_str.text(),
           conf_info.ens_gci[i_gc].lvl_str.text(), var_str);
   ens_var = nc_out->add_var(ens_var_name, ncFloat, lat_dim, lon_dim);

   // Add the variable attributes
   add_var_att(i_gc, ens_var, 0, wd, long_name_str);

   // Write the data
   if(!ens_var->put(&ens_data[0], grid.ny(), grid.nx())) {
      cerr << "\n\nERROR: write_ens_var_float() -> "
           << "error in ens_var->put for the " << ens_var_name
           << " field.\n\n" << flush;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ens_var_int(int i_gc, int *ens_data, const char *var_str,
                       WrfData &wd, const char *long_name_str) {
   NcVar *ens_var;
   char ens_var_name[max_str_len];

   // Construct the variable name
   sprintf(ens_var_name, "%s_%s_%s",
           conf_info.ens_gci[i_gc].abbr_str.text(),
           conf_info.ens_gci[i_gc].lvl_str.text(), var_str);
   ens_var = nc_out->add_var(ens_var_name, ncInt, lat_dim, lon_dim);

   // Add the variable attributes
   add_var_att(i_gc, ens_var, 1, wd, long_name_str);

   // Write the data
   if(!ens_var->put(&ens_data[0], grid.ny(), grid.nx())) {
      cerr << "\n\nERROR: write_ens_var_int() -> "
           << "error in ens_var->put for the " << ens_var_name
           << " field.\n\n" << flush;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_orank_nc(EnsPairData &pd, WrfData &wd,
                    int i_gc, int i_interp, int i_mask) {
   int i, n;

   // Arrays for storing observation rank data
   float *obs_v    = (float *) 0;
   int   *obs_rank = (int *)   0;
   float *obs_pit  = (float *) 0;
   int   *ens_vld  = (int *)   0;

   // Allocate memory for storing ensemble data
   obs_v    = new float [grid.nx()*grid.ny()];
   obs_rank = new int   [grid.nx()*grid.ny()];
   obs_pit  = new float [grid.nx()*grid.ny()];
   ens_vld  = new int   [grid.nx()*grid.ny()];

   // Initialize
   for(i=0; i<grid.nx()*grid.ny(); i++) {
      obs_v[i]    = bad_data_float;
      obs_rank[i] = bad_data_int;
      obs_pit[i]  = bad_data_float;
      ens_vld[i]  = bad_data_int;
   }

   // Loop over all the pairs
   for(i=0; i<pd.n_pair; i++) {

      n = two_to_one(grid.nx(), grid.ny(),
                     nint(pd.x_na[i]), nint(pd.y_na[i]));

      // Store the observation value, rank, and number of valid ensembles
      obs_v[n]    = pd.o_na[i];
      obs_rank[n] = nint(pd.r_na[i]);
      obs_pit[n]  = pd.pit_na[i];
      ens_vld[n]  = nint(pd.v_na[i]);

   } // end for i

   // Add the observation values
   write_orank_var_float(i_gc, i_interp, i_mask, obs_v, "OBS", wd,
                         "Observation Value");

   // Add the observation ranks
   write_orank_var_int(i_gc, i_interp, i_mask, obs_rank, "OBS_RANK", wd,
                       "Observation Rank");

   // Add the probability integral transforms
   write_orank_var_float(i_gc, i_interp, i_mask, obs_pit, "OBS_PIT", wd,
                         "Probability Integral Transform");

   // Add the number of valid ensemble members
   write_orank_var_int(i_gc, i_interp, i_mask, ens_vld, "ENS_VLD", wd,
                       "Ensemble Valid Data Count");

   // Deallocate and clean up
   if(obs_v)    { delete [] obs_v;    obs_v    = (float *) 0; }
   if(obs_rank) { delete [] obs_rank; obs_rank = (int   *) 0; }
   if(obs_pit)  { delete [] obs_pit;  obs_pit  = (float *) 0; }
   if(ens_vld)  { delete [] ens_vld;  ens_vld  = (int   *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_orank_var_float(int i_gc, int i_interp, int i_mask,
                           float *data, const char *var_str,
                           WrfData &wd, const char *long_name_str) {
   NcVar *nc_var;
   int wdth;
   char var_name[max_str_len], mthd_str[max_str_len];

   // Get the interpolation method string
   interpmthd_to_string(conf_info.interp_mthd[i_interp], mthd_str);
   wdth = conf_info.interp_wdth[i_interp];

   // Check for smoothing
   if((conf_info.interp_wdth[i_interp] > 1) &&
      (conf_info.conf.interp_flag().ival() == 2 ||
       conf_info.conf.interp_flag().ival() == 3)) {
      sprintf(var_name, "%s_%s_%s_%s_%s_%i",
              var_str,
              conf_info.gc_pd[i_gc].obs_gci.abbr_str.text(),
              conf_info.gc_pd[i_gc].obs_gci.lvl_str.text(),
              conf_info.mask_name[i_mask],
              mthd_str, wdth*wdth);
   }
   // Field was not smoothed
   else {
      sprintf(var_name, "%s_%s_%s_%s",
              var_str,
              conf_info.gc_pd[i_gc].obs_gci.abbr_str.text(),
              conf_info.gc_pd[i_gc].obs_gci.lvl_str.text(),
              conf_info.mask_name[i_mask]);
   }

   // Define the variable
   nc_var = nc_out->add_var(var_name, ncFloat, lat_dim, lon_dim);

   // Add the variable attributes
   add_var_att(i_gc, nc_var, 0, wd, long_name_str);

   // Write the data
   if(!nc_var->put(&data[0], grid.ny(), grid.nx())) {
      cerr << "\n\nERROR: write_orank_var_float() -> "
           << "error in nc_var->put for the " << var_name
           << " field.\n\n" << flush;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_orank_var_int(int i_gc, int i_interp, int i_mask,
                         int *data, const char *var_str,
                         WrfData &wd, const char *long_name_str) {
   NcVar *nc_var;
   int wdth;
   char var_name[max_str_len], mthd_str[max_str_len];

   // Get the interpolation method string
   interpmthd_to_string(conf_info.interp_mthd[i_interp], mthd_str);
   wdth = conf_info.interp_wdth[i_interp];

   // Check for smoothing
   if((conf_info.interp_wdth[i_interp] > 1) &&
      (conf_info.conf.interp_flag().ival() == 2 ||
       conf_info.conf.interp_flag().ival() == 3)) {
      sprintf(var_name, "%s_%s_%s_%s_%s_%i",
              var_str,
              conf_info.gc_pd[i_gc].obs_gci.abbr_str.text(),
              conf_info.gc_pd[i_gc].obs_gci.lvl_str.text(),
              conf_info.mask_name[i_mask],
              mthd_str, wdth*wdth);
   }
   // Field was not smoothed
   else {
      sprintf(var_name, "%s_%s_%s_%s",
              var_str,
              conf_info.gc_pd[i_gc].obs_gci.abbr_str.text(),
              conf_info.gc_pd[i_gc].obs_gci.lvl_str.text(),
              conf_info.mask_name[i_mask]);
   }

   // Define the variable
   nc_var = nc_out->add_var(var_name, ncInt, lat_dim, lon_dim);

   // Add the variable attributes
   add_var_att(i_gc, nc_var, 1, wd, long_name_str);

   // Write the data
   if(!nc_var->put(&data[0], grid.ny(), grid.nx())) {
      cerr << "\n\nERROR: write_orank_var_int() -> "
           << "error in nc_var->put for the " << var_name
           << " field.\n\n" << flush;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void add_var_att(int i_gc, NcVar *ens_var, int flag,
                 WrfData &wd, const char *long_name_str) {
   char tmp_str[max_str_len];

   // Construct the long name
   sprintf(tmp_str, "%s at %s %s",
      conf_info.ens_gci[i_gc].abbr_str.text(),
      conf_info.ens_gci[i_gc].lvl_str.text(),
      long_name_str);

   // Add variable attributes
   ens_var->add_att("units", conf_info.ens_gci[i_gc].units_str.text());
   ens_var->add_att("long_name", tmp_str);
   ens_var->add_att("level", conf_info.ens_gci[i_gc].lvl_str.text());

   if(flag) ens_var->add_att("_FillValue", bad_data_int);
   else     ens_var->add_att("_FillValue", bad_data_float);

   // Write out times
   write_netcdf_var_times(ens_var, wd);

   return;
}

////////////////////////////////////////////////////////////////////////

void finish_txt_files() {
   int i;

   // Write out the contents of the STAT AsciiTable and
   // close the STAT output files
   if(stat_out) {
      *stat_out << stat_at;
      close_txt_file(stat_out, stat_file, verbosity);
   }

   // Finish up each of the optional text files
   for(i=0; i<n_txt; i++) {

      // Only write the table if requested in the config file
      if(conf_info.conf.output_flag(i).ival() >= flag_txt_out) {

         // Write the AsciiTable to a file
         if(txt_out[i]) {
            *txt_out[i] << txt_at[i];
            close_txt_file(txt_out[i], txt_file[i], verbosity);
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {
   int i;

   if(verbosity > 1) {
      cout << "\n" << sep_str << "\n\n"
           << flush;
   }

   // Close the output text files that were open for writing
   if(vx_flag) finish_txt_files();

   // List the output NetCDF files
   for(i=0; i<out_nc_file_list.n_elements(); i++) {
      cout << "Output file: " << out_nc_file_list[i] << "\n" << flush;
   }

   // Deallocate threshold count arrays
   if(na_thresh_count) {
      for(i=0; i<conf_info.get_max_n_thresh(); i++) {
         na_thresh_count[i].clear();
      }
      delete [] na_thresh_count;
      na_thresh_count = (NumArray *) 0;
   }

   // Deallocate memory for the random number generator
   rng_free(rng_ptr);

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tn_ens ens_file_1 ... ens_file_n | ens_file_list\n"
        << "\tconfig_file\n"
        << "\t[-grid_obs file]\n"
        << "\t[-point_obs file]\n"
        << "\t[-ens_valid time]\n"
        << "\t[-ens_lead time]\n"
        << "\t[-obs_valid_beg time]\n"
        << "\t[-obs_valid_end time]\n"
        << "\t[-obs_lead time]\n"
        << "\t[-outdir path]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"n_ens ens_file_1 ... ens_file_n\" is the number "
        << "of ensemble members followed by a list of ensemble member "
        << "file names (required).\n"

        << "\t\t\"ens_file_list\" is an ASCII file containing a list "
        << "of ensemble member file names (required).\n"

        << "\t\t\"config_file\" is an EnsembleStatConfig file "
        << "containing the desired configuration settings (required).\n"

        << "\t\t\"-grid_obs file\" specifies a gridded observation file. "
        << "May be used multiple times (optional).\n"

        << "\t\t\"-point_obs file\" specifies a NetCDF point observation file. "
        << "May be used multiple times (optional).\n"

        << "\t\t\"-ens_valid time\" in YYYYMMDD[_HH[MMSS]] format "
        << "sets the ensemble valid time to be used (optional).\n"

        << "\t\t\"-ens_lead time\" in HH[MMSS] format sets "
        << "the ensemble lead time to be used (optional).\n"

        << "\t\t\"-obs_valid_beg time\" in YYYYMMDD[_HH[MMSS]] sets the "
        << "beginning of the matching time window (optional).\n"

        << "\t\t\"-obs_valid_end time\" in YYYYMMDD[_HH[MMSS]] sets the "
        << "end of the matching time window (optional).\n"

        << "\t\t\"-obs_lead time\" in HH[MMSS] format sets "
        << "the observation lead time to be used (optional).\n"

        << "\t\t\"-outdir path\" overrides the default output directory "
        << "(" << out_dir << ") (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << verbosity << ") (optional).\n"

        << "\n\tNOTE: The ensemble members and gridded observations "
        << "must be on the same grid.\n\n" << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_grid_obs(const StringArray & a)
{
   grid_obs_file_list.add(a[0]);
   grid_obs_flag = 1;
}

////////////////////////////////////////////////////////////////////////

void set_point_obs(const StringArray & a)
{
   point_obs_file_list.add(a[0]);
   point_obs_flag = 1;
}

////////////////////////////////////////////////////////////////////////

void set_ens_valid(const StringArray & a)
{
   ens_valid_search_ut = timestring_to_unix(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_ens_lead(const StringArray & a)
{
   ens_lead_search_sec = timestring_to_sec(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_obs_valid_beg(const StringArray & a)
{
   obs_valid_beg_ut = timestring_to_unix(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_obs_valid_end(const StringArray & a)
{
   obs_valid_end_ut = timestring_to_unix(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_obs_lead(const StringArray & a)
{
   obs_lead_sec = timestring_to_sec(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_outdir(const StringArray & a)
{
   out_dir = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a)
{
   verbosity = atoi(a[0]);
}

////////////////////////////////////////////////////////////////////////
