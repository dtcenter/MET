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
#include "vx_grib_classes/grib_classes.h"

#include "vx_wrfdata/vx_wrfdata.h"
#include "vx_met_util/vx_met_util.h"
#include "vx_data_grids/grid.h"
#include "vx_util/vx_util.h"
#include "vx_cal/vx_cal.h"
#include "vx_math/vx_math.h"
#include "vx_contable/vx_contable.h"
#include "vx_gsl_prob/vx_gsl_prob.h"
#include "vx_econfig/result.h"

#include "ensemble_stat.h"

////////////////////////////////////////////////////////////////////////

static void process_command_line (int, char **);
static void process_ensemble     ();
static void process_ranks        ();
static void process_scores       ();

static void process_point_obs();
static void process_grid_obs();

static void parse_fcst_file_list(const char *);
static void read_field(const char *, const GCInfo &, int &, WrfData &, Grid &, int);
static void clear_counts(const WrfData &, int);
static void track_counts(const WrfData &, int);

static void setup_first_ens_pass(const WrfData &, const Grid &);
static void setup_nc_file       (unixtime, int);

static void setup_first_vx_pass (const WrfData &);
static void setup_txt_files     (unixtime, int);
static void setup_table         (AsciiTable &);

static void build_outfile_name(unixtime, int, const char *,
                               ConcatString &);

static void write_ens_nc(int, WrfData &);
static void write_ens_var_float(int, float *, const char *, WrfData &);
static void write_ens_var_int(int, int *, const char *, WrfData &);
static void write_nc(const WrfData &, const WrfData &,
                     int, InterpMthd, int);

static void finish_txt_files();

static void clean_up();

static void usage(int, char **);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the ensemble
   process_ensemble();

   // Process the verification
   //process_verification();

   // Compute the scores and write them out
   //process_scores();

   // Close the text files and deallocate memory
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   int i;

   // JHG, be sure to add -valid_beg and -valid_end command line args

   // Set default output directory
   out_dir << MET_BASE << "/out/ensemble_stat";

   if(argc < 4) {
      usage(argc, argv);
      exit(1);
   }

   // If the next argument is a number, it should be followed by a list
   // of files for each ensemble members
   if(is_number(argv[1]) == 1) {

      n_fcst = atoi(argv[1]);

      if(n_fcst <= 0) {
         cerr << "\n\nERROR: process_command_line() -> "
              << "the number of ensemble member files must be >= 1 ("
              << n_fcst << ")\n\n" << flush;
         exit(1);
      }

      // Add each of the ensemble members to the list of files
      for(i=2; i<2+n_fcst; i++) {
         fcst_file_list.add(argv[i]);
      }
   }
   // Otherwise, the next argument is a file containing a list of
   // ensemble member file names
   else {
      i=1;
      parse_fcst_file_list(argv[i]);
      i++;
   }

   // Define the number of ranks as one more than the number of forecasts
   n_rank = n_fcst + 1;

   // Store the config file name
   config_file = argv[i];

   // Store the output file name
   out_file = argv[i+1];

   // Parse command line arguments
   for(i=0; i<argc; i++) {

      if(strcmp(argv[i], "-grid_obs") == 0) {
         grid_obs_list.add(argv[i+1]);
         grid_obs_flag = 1;
         i++;
      }
      else if(strcmp(argv[i], "-point_obs") == 0) {
         point_obs_list.add(argv[i+1]);
         point_obs_flag = 1;
         i++;
      }
      else if(strcmp(argv[i], "-outdir") == 0) {
         out_dir = argv[i+1];
         i++;
      }
      else if(strcmp(argv[i], "-v") == 0) {
         verbosity = atoi(argv[i+1]);
         i++;
      }
      else if(argv[i][0] == '-') {
         cerr << "\n\nERROR: process_command_line() -> "
              << "unrecognized command line switch: "
              << argv[i] << "\n\n" << flush;
         exit(1);
      }
   }

   // Read the config file
   conf_info.read_config(config_file);

   // Set the model name
   shc.set_model(conf_info.conf.model().sval());

   // Allocate arrays to store threshold counts
   na_thresh_count = new NumArray [conf_info.get_max_n_thresh()];

   // List the input files
   if(verbosity > 0) {

      cout << "Configuration File:\n"
           << "   " << config_file << "\n" << flush;

      cout << "Ensemble Files["
           << fcst_file_list.n_elements() << "]:\n" << flush;
      for(i=0; i<fcst_file_list.n_elements(); i++) {
         cout << "   " << fcst_file_list[i]  << "\n" << flush;
      }

      if(grid_obs_list.n_elements() > 0) {
         cout << "Gridded Observation Files["
              << grid_obs_list.n_elements() << "]:\n" << flush;
         for(i=0; i<grid_obs_list.n_elements(); i++) {
            cout << "   " << grid_obs_list[i] << "\n" << flush;
         }
      }

      if(point_obs_list.n_elements() > 0) {
         cout << "Point Observation Files["
              << point_obs_list.n_elements() << "]:\n" << flush;
         for(i=0; i<point_obs_list.n_elements(); i++) {
            cout << "   " << point_obs_list[i] << "\n" << flush;
         }
      }

      cout << "Output NetCDF File:\n"
           << "   " << out_file << "\n" << flush;
   }

// JHG, should I require an output file name, or just construct one?

   return;
}

////////////////////////////////////////////////////////////////////////

void process_ensemble() {
   int i, j, n_miss;
   double t;
   WrfData ens_wd;
   Grid ens_grid;

   // Loop through each of the ensemble fields to be processed
   for(i=0; i<conf_info.get_n_ens(); i++) {

      if(verbosity > 1) {

         cout << "\n----------------------------------------\n\n"
              << "Processing ensemble field "
              << conf_info.ens_gci[i].info_str
              << "\n" << flush;
      }

      // Loop through each of the input forecast files
      for(j=0, n_miss=0; j<fcst_file_list.n_elements(); j++) {

         // Read the current field from the current forecast file
         read_field(fcst_file_list[j], conf_info.ens_gci[i],
                    n_miss, ens_wd, ens_grid, verbosity);

         // If this is the first pass, the grid is unset
         if(grid.nx() == 0 && grid.ny() == 0) {

            // Setup the first pass through the data
            setup_first_ens_pass(ens_wd, ens_grid);
         }
         // After the first pass, make sure that the grid doesn't change
         else {
            if(grid != ens_grid) {
               cerr << "\n\nERROR: process_ensemble() -> "
                    << "All data files must be on the same grid: "
                    << fcst_file_list[j] << "\n\n"
                    << flush;
               exit(1);
            }
         }

         // Check if the number of missing fields exceeds the threshold
         t = 1.0 - conf_info.conf.vld_ens_thresh().dval();
         if((double) n_miss/n_fcst > t) {
            cerr << "\n\nERROR: process_ensemble_files() -> "
                 << n_miss << " missing fields exceeds the maximum "
                 << "allowable specified in the configuration file.\n\n"
                 << flush;
            exit(1);
         }

         // Initialize the running sums and counts
         if(j==0) clear_counts(ens_wd, i);

         // Apply the current data field to the running sums and counts
         track_counts(ens_wd, i);

      } // end for j

      // Write out the ensemble information to a NetCDF file
      write_ens_nc(i, ens_wd);
   }

   return;
}
/*
////////////////////////////////////////////////////////////////////////
// JHG:
// - Add ensemble output to NetCDF file
// - Also dump out the counts to the NetCDF file
// - Add option for what % of data points must be valid to be included
// - Add processing of observation files and computation of histograms
// - Ed would like to see median plus other percentiles

void process_fcst_obs_files() {
   int i, j, n_miss;
   double t;
   WrfData fcst_wd;
   Grid fcst_grid;

   // Loop through each of the fields to be verified
   for(i=0; i<conf_info.get_n_vx(); i++) {

      if(verbosity > 1) {

         cout << "\n----------------------------------------\n\n"
              << "Processing "
              << conf_info.fcst_gci[i].info_str
              << "\n" << flush;
      }

      // Loop through each of the input forecast files
      for(j=0, n_miss=0; j<fcst_file_list.n_elements(); j++) {

         // Read the current field from the current forecast file
         read_field(fcst_file_list[j], i, n_miss, fcst_wd, fcst_grid,
                    verbosity);

         // If this is the first pass, the grid is unset

// JHG, the grid is already set at this point so this logic doesn't work
         if(grid.nx() == 0 && grid.ny() == 0) {

            // Setup the first pass through the data
            setup_first_vx_pass(fcst_wd);
         }
         // After the first pass, make sure that the grid doesn't change
         else {
            if(grid != fcst_grid) {
               cerr << "\n\nERROR: process_fcst_obs_files() -> "
                    << "All data files must be on the same grid: "
                    << fcst_file_list[j] << "\n\n"
                    << flush;
               exit(1);
            }
         }

         // Check if the number of missing fields exceeds the threshold
         t = 1.0 - conf_info.conf.vld_ens_thresh().dval();
         if((double) n_miss/n_fcst > t) {
            cerr << "\n\nERROR: process_fcst_obs_files() -> "
                 << n_miss << " missing fields exceeds the maximum "
                 << "allowable specified in the configuration file.\n\n"
                 << flush;
            exit(1);
         }

         // Initialize the running sums and counts.
         if(j==0) clear_counts(fcst_wd, i);

         // Apply the current data field to the running sums and counts
         track_counts(fcst_wd, i);

      } // end for j

      // JHG, write NetCDF data

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void process_point_obs() {
   int i_nc, i_obs, j;
   float obs_arr[obs_arr_len], hdr_arr[hdr_arr_len];
   char hdr_typ_str[max_str_len];
   char hdr_sid_str[max_str_len];
   char hdr_vld_str[max_str_len];
   unixtime hdr_ut;
   NcFile *obs_in;

   for(i_nc=0; i_nc<point_obs_list.n_elements(); i_nc++) {

      // Open the observation file as a NetCDF file.
      // The observation file must be in NetCDF format as the
      // output of the PB2NC or ASCII2NC tool.
      obs_in = new NcFile(point_obs_list[i_nc]);

      if(!obs_in->is_valid()) {
         obs_in->close();
         delete obs_in;
         obs_in = (NcFile *) 0;

         cerr << "\n\nERROR: process_point_obs() -> "
              << "can't open observation netCDF file: "
              << point_obs_list[i_nc] << "\n\n" << flush;
         exit(1);
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
              << point_obs_list[i_nc] << "\n\n" << flush;
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
              << point_obs_list[i_nc] << "\n\n" << flush;
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
   } // end for i

   return;
}

*
*/
////////////////////////////////////////////////////////////////////////

void read_field(const char *file_name, const GCInfo &gci, int &n_miss,
                WrfData &wd, Grid &grid, int verbosity) {
   FileType ftype = NoFileType;
   GribFile gb_file;
   NcFile  *nc_file = (NcFile *) 0;
   int status;
   char tmp_str[max_str_len];
   GribRecord rec;

   // Switch based on the file type
   ftype = get_file_type(file_name);

   switch(ftype) {

      // GRIB file type
      case(GbFileType):

         // Open the GRIB file
         if(!(gb_file.open(file_name))) {
            cout << "\n\nWARNING: read_field() -> "
                 << "can't open GRIB file: "
                 << file_name << "\n\n" << flush;
            n_miss++;
            return;
         }

         // Retrieve the requested field
         status = get_grib_record(gb_file, rec,
                                  gci, wd, grid, verbosity);

         if(status != 0) {
            cout << "\n\nWARNING: read_field() -> "
                 << "can't retrieve \""
                 << gci.info_str
                 << "\" field from GRIB file: "
                 << file_name << "\n\n" << flush;
            n_miss++;
            return;
         }

         break;

      // NetCDF file type
      case(NcFileType):

         // Open the NetCDF File
         nc_file = new NcFile(file_name);
         if(!nc_file->is_valid()) {
            cout << "\n\nWARNING: read_field() -> "
                 << "can't open NetCDF file: "
                 << file_name << "\n\n" << flush;
            n_miss++;
            return;
         }

         // Retrieve the requested field
         status = read_netcdf_status(nc_file,
                     gci.abbr_str.text(),
                     tmp_str, wd, grid, verbosity);

         if(status != 0) {
            cout << "\n\nWARNING: read_field() -> "
                 << "can't retrieve \""
                 << gci.info_str
                 << "\" field from NetCDF file: "
                 << file_name << "\n\n" << flush;
            n_miss++;
            return;
         }

         break;

      // Unsupported file type
      default:
         cerr << "\n\nERROR: read_field() -> "
              << "unsupported file type: "
              << file_name << "\n\n" << flush;
         exit(1);
         break;
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

void process_scores() {

/* JHG

   int i, j, k, m, n, status;
   int max_scal_t, max_prob_t, wind_t, frac_t;

   WrfData fcst_wd,             obs_wd;
   WrfData fcst_wd_smooth,      obs_wd_smooth;

   GribRecord fcst_r, obs_r;
   Grid fcst_grid, obs_grid;
   char tmp_str[max_str_len];
   char fcst_thresh_str[max_str_len], obs_thresh_str[max_str_len];
   NumArray f_na, o_na;

   // Objects to handle vector winds
   WrfData fcst_ugrd_wd,        obs_ugrd_wd;
   WrfData fcst_ugrd_wd_smooth, obs_ugrd_wd_smooth;
   NumArray f_ugrd_na, o_ugrd_na;

   CNTInfo     cnt_info;
   CTSInfo    *cts_info;
   VL1L2Info  *vl1l2_info;
   NBRCNTInfo  nbrcnt_info;
   NBRCTSInfo *nbrcts_info;
   PCTInfo    *pct_info;

   // Allocate enough space for the CTSInfo objects
   max_scal_t  = conf_info.get_max_n_thresh();
   cts_info    = new CTSInfo [max_scal_t];

   // Allocate enough space for the VL1L2Info objects
   wind_t      = conf_info.get_n_wind_thresh();
   vl1l2_info  = new VL1L2Info [wind_t];

   // Allocate enough space for the NBRCTS objects
   frac_t      = conf_info.frac_ta.n_elements();
   nbrcts_info = new NBRCTSInfo [frac_t];

   // Allocate enough space for the PCTInfo objects
   max_prob_t  = conf_info.get_max_n_prob_obs_thresh();
   pct_info    = new PCTInfo [max_prob_t];

   // Loop through each of the fields to be verified
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Continue based on the forecast file type
      if(fcst_ftype == GbFileType) {

         status = get_grib_record(fcst_gb_file, fcst_r,
                                  conf_info.fcst_gci[i],
                                  fcst_wd, fcst_grid, verbosity);

         if(status != 0) {
            cout << "***WARNING***: process_scores() -> "
                 << conf_info.fcst_gci[i].info_str
                 << " not found in GRIB file: " << fcst_file_list[0]
                 << "\n" << flush;
            continue;
         }
      }
      // fcst_ftype == NcFileType
      else {

         read_netcdf(fcst_nc_file,
                     conf_info.fcst_gci[i].abbr_str.text(),
                     tmp_str,
                     fcst_wd, fcst_grid, verbosity);
      }

      // For probability fields, check to see if they need to be
      // rescaled from [0, 100] to [0, 1]
      if(conf_info.fcst_gci[i].pflag == 1) rescale_probability(fcst_wd);

      // Set the forecast lead time
      shc.set_fcst_lead_sec(fcst_wd.get_lead_time());

      // Set the forecast valid time
      shc.set_fcst_valid_beg(fcst_wd.get_valid_time());
      shc.set_fcst_valid_end(fcst_wd.get_valid_time());

      // Continue based on the forecast file type
      if(obs_ftype == GbFileType) {

         status = get_grib_record(obs_gb_file, obs_r,
                                  conf_info.obs_gci[i],
                                  obs_wd, obs_grid, verbosity);

         if(status != 0) {
            cout << "***WARNING***: process_scores() -> "
                 << conf_info.obs_gci[i].info_str
                 << " not found in GRIB file: " << obs_file
                 << "\n" << flush;
            continue;
         }
      }
      // obs_ftype == NcFileType
      else {

         read_netcdf(obs_nc_file,
                     conf_info.obs_gci[i].abbr_str.text(),
                     tmp_str,
                     obs_wd, obs_grid, verbosity);
      }

      // Set the observation lead time
      shc.set_obs_lead_sec(obs_wd.get_lead_time());

      // Set the observation valid time
      shc.set_obs_valid_beg(obs_wd.get_valid_time());
      shc.set_obs_valid_end(obs_wd.get_valid_time());

      // Check that the grid dimensions match
      if(fcst_grid.nx() != obs_grid.nx() ||
         fcst_grid.ny() != obs_grid.ny()) {
         cerr << "\n\nERROR: process_scores() -> "
              << "Forecast and observation grid dimensions "
              << "do not match (" << fcst_grid.nx() << ", "
              << fcst_grid.ny() << ") != (" << obs_grid.nx()
              << ", " << obs_grid.ny() << ") for field " << i+1
              << ".\n\n" << flush;
         exit(1);
      }

      // Check that the valid times match
      if(fcst_wd.get_valid_time() != obs_wd.get_valid_time()) {
         cout << "***WARNING***: process_scores() -> "
              << "Forecast and observation valid times do not match "
              << fcst_wd.get_valid_time() << " != "
              << obs_wd.get_valid_time() << " for field " << i+1
              << ".\n\n" << flush;
      }

      // Check that the accumulation intervals match when comparing 
      // the same GRIB codes
      if(conf_info.fcst_gci[i].code == conf_info.obs_gci[i].code &&
         fcst_wd.get_accum_time()   != obs_wd.get_accum_time()) {
         cout << "***WARNING***: process_scores() -> "
              << "Forecast and observation accumulation times "
              << "do not match " << fcst_wd.get_accum_time()
              << " != " << obs_wd.get_accum_time() << " for field "
              << i+1 << ".\n\n" << flush;
      }

      // This is the first pass through the loop and grid is unset
      if(grid.nx() == 0 && grid.ny() == 0) {

         // Setup the first pass through the data
         setup_first_pass(fcst_wd, fcst_grid);
      }
      // For multiple verification fields, check to make sure that the
      // grid dimensions don't change
      else {

         if(fcst_grid.nx() != grid.nx() ||
            fcst_grid.ny() != grid.ny() ||
            obs_grid.nx()  != grid.nx() ||
            obs_grid.ny()  != grid.ny()) {
            cerr << "\n\nERROR: process_scores() -> "
                 << "The grid dimensions must remain constant ("
                 << grid.nx() << ", " << grid.ny() << ") != ("
                 << fcst_grid.nx() << ", " << fcst_grid.ny()
                 << ") or ("
                 << obs_grid.nx() << ", " << obs_grid.ny() << ")\n\n"
                 << flush;
            exit(1);
         }
      }

      // Setup strings for the GRIB code
      if(conf_info.fcst_gci[i].code == apcp_grib_code &&
         conf_info.obs_gci[i].code  == apcp_grib_code) {
         shc.set_msg_typ("MC_PCP");
      }
      else {
         shc.set_msg_typ("ANALYS");
      }

      // Store the forecast variable name
      shc.set_fcst_var(conf_info.fcst_gci[i].abbr_str.text());

      // Set the forecast level name
      shc.set_fcst_lev(conf_info.fcst_gci[i].lvl_str.text());

      // Store the observation variable name
      shc.set_obs_var(conf_info.obs_gci[i].abbr_str.text());

      // Set the observation level name
      shc.set_obs_lev(conf_info.obs_gci[i].lvl_str.text());

      if(verbosity > 1) {
         cout << "\n----------------------------------------\n\n"
              << flush;
      }

      // If verifying vector winds, store the UGRD fields
      if(conf_info.fcst_gci[i].code  == ugrd_grib_code &&
         conf_info.obs_gci[i].code   == ugrd_grib_code &&
         conf_info.fcst_gci[i].vflag == 1 &&
         conf_info.obs_gci[i].vflag  == 1) {

         fcst_ugrd_wd = fcst_wd;
         obs_ugrd_wd  = obs_wd;
      }

      // Loop through and apply each of the smoothing operations
      for(j=0; j<conf_info.get_n_interp(); j++) {

         // Store the interpolation method and width being applied
         shc.set_interp_mthd(conf_info.interp_mthd[j]);
         shc.set_interp_wdth(conf_info.interp_wdth[j]);

         // Smooth the forecast and observation fields based on the
         // interp_mthd and interp_wdth
         fcst_wd_smooth = smooth_field(fcst_wd,
                             conf_info.interp_mthd[j],
                             conf_info.interp_wdth[j],
                             conf_info.conf.interp_thresh().dval());
         obs_wd_smooth  = smooth_field(obs_wd,
                             conf_info.interp_mthd[j],
                             conf_info.interp_wdth[j],
                             conf_info.conf.interp_thresh().dval());

         // Loop through the masks to be applied
         for(k=0; k<conf_info.get_n_mask(); k++) {

            // Apply the current mask to the fields
            apply_mask(fcst_wd_smooth, obs_wd_smooth,
                       conf_info.mask_wd[k],
                       f_na, o_na);

            // Continue if no pairs were found
            if(f_na.n_elements() == 0) continue;

            // Set the mask name
            shc.set_mask(conf_info.mask_name[k]);

            if(verbosity > 1) {

               cout << "Processing "
                    << conf_info.fcst_gci[i].info_str
                    << " versus "
                    << conf_info.obs_gci[i].info_str
                    << ", for interpolation method "
                    << shc.get_interp_mthd_str() << "("
                    << shc.get_interp_pnts_str()
                    << "), over region " << shc.get_mask()
                    << ", using " << f_na.n_elements() << " pairs.\n"
                    << flush;
            }

            // Compute CTS scores
            if(conf_info.fcst_gci[i].pflag == 0 &&
               conf_info.fcst_ta[i].n_elements() > 0 &&
               (conf_info.conf.output_flag(i_fho).ival() ||
                conf_info.conf.output_flag(i_ctc).ival() ||
                conf_info.conf.output_flag(i_cts).ival())) {

               // Initialize
               for(m=0; m<max_scal_t; m++) cts_info[m].clear();

               // Compute CTS
               do_cts(cts_info, i, j, k, f_na, o_na);

               // Loop through all of the thresholds
               for(m=0; m<conf_info.fcst_ta[i].n_elements(); m++) {

                  // Write out FHO
                  if(conf_info.conf.output_flag(i_fho).ival() &&
                     cts_info[m].cts.n() > 0) {

                     write_fho_row(shc, cts_info[m],
                        conf_info.conf.output_flag(i_fho).ival(),
                        stat_at, i_stat_row,
                        txt_at[i_fho], i_txt_row[i_fho]);
                  }

                  // Write out CTC
                  if(conf_info.conf.output_flag(i_ctc).ival() &&
                     cts_info[m].cts.n() > 0) {

                     write_ctc_row(shc, cts_info[m],
                        conf_info.conf.output_flag(i_ctc).ival(),
                        stat_at, i_stat_row,
                        txt_at[i_ctc], i_txt_row[i_ctc]);
                  }

                  // Write out CTS
                  if(conf_info.conf.output_flag(i_cts).ival() &&
                     cts_info[m].cts.n() > 0) {

                     write_cts_row(shc, cts_info[m],
                        conf_info.conf.output_flag(i_cts).ival(),
                        stat_at, i_stat_row,
                        txt_at[i_cts], i_txt_row[i_cts]);
                  }
               } // end for m
            } // end Compute CTS

            // Compute CNT scores
            if(conf_info.fcst_gci[i].pflag == 0 &&
               (conf_info.conf.output_flag(i_cnt).ival() ||
                conf_info.conf.output_flag(i_sl1l2).ival())) {

               // Initialize
               cnt_info.clear();

               // Compute CNT
               do_cnt(cnt_info, i, j, k, f_na, o_na);

               // Write out CNT
               if(conf_info.conf.output_flag(i_cnt).ival() &&
                  cnt_info.n > 0) {

                  write_cnt_row(shc, cnt_info,
                     conf_info.conf.output_flag(i_cnt).ival(),
                     stat_at, i_stat_row,
                     txt_at[i_cnt], i_txt_row[i_cnt]);
               }

               // Write out SL1L2 as long as the vflag is not set
               if(conf_info.fcst_gci[i].vflag == 0 &&
                  conf_info.conf.output_flag(i_sl1l2).ival() &&
                  cnt_info.n > 0) {

                  write_sl1l2_row(shc, cnt_info,
                     conf_info.conf.output_flag(i_sl1l2).ival(),
                     stat_at, i_stat_row,
                     txt_at[i_sl1l2], i_txt_row[i_sl1l2]);
               }
            } // end Compute CNT

            // Compute VL1L2 partial sums for UGRD,VGRD
            if(conf_info.fcst_gci[i].pflag == 0 &&
               conf_info.fcst_gci[i].vflag == 1 &&
               conf_info.conf.output_flag(i_vl1l2).ival() &&
               i > 0 &&
               conf_info.fcst_gci[i].code   == vgrd_grib_code &&
               conf_info.fcst_gci[i-1].code == ugrd_grib_code) {

               // Store the forecast variable name
               shc.set_fcst_var(ugrd_vgrd_abbr_str);

               // Store the observation variable name
               shc.set_obs_var(ugrd_vgrd_abbr_str);

               // Initialize
               for(m=0; m<wind_t; m++) vl1l2_info[m].clear();

               // Apply current smoothing method to the UGRD fields
               fcst_ugrd_wd_smooth =
                  smooth_field(fcst_ugrd_wd,
                               conf_info.interp_mthd[j],
                               conf_info.interp_wdth[j],
                               conf_info.conf.interp_thresh().dval());

               obs_ugrd_wd_smooth  =
                  smooth_field(obs_ugrd_wd,
                               conf_info.interp_mthd[j],
                               conf_info.interp_wdth[j],
                               conf_info.conf.interp_thresh().dval());

               // Apply the current mask to the UGRD fields
               apply_mask(fcst_ugrd_wd_smooth, obs_ugrd_wd_smooth,
                          conf_info.mask_wd[k],
                          f_ugrd_na, o_ugrd_na);

               // Compute VL1L2
               do_vl1l2(vl1l2_info, i, j, k,
                        f_na, o_na, f_ugrd_na, o_ugrd_na);

               // Loop through all of the wind speed thresholds
               for(m=0; m<conf_info.fcst_wind_ta.n_elements(); m++) {

                  // Write out VL1L2
                  if(conf_info.conf.output_flag(i_vl1l2).ival() &&
                     vl1l2_info[m].vcount > 0) {

                     write_vl1l2_row(shc, vl1l2_info[m],
                        conf_info.conf.output_flag(i_vl1l2).ival(),
                        stat_at, i_stat_row,
                        txt_at[i_vl1l2], i_txt_row[i_vl1l2]);
                  }
               } // end for m

               // Reset the forecast variable name
               shc.set_fcst_var(conf_info.fcst_gci[i].abbr_str.text());

               // Reset the observation variable name
               shc.set_obs_var(conf_info.obs_gci[i].abbr_str.text());

            } // end Compute VL1L2

            // Compute PCT counts and scores
            if(conf_info.fcst_gci[i].pflag == 1 &&
               (conf_info.conf.output_flag(i_pct).ival()  ||
                conf_info.conf.output_flag(i_pstd).ival() ||
                conf_info.conf.output_flag(i_pjc).ival()  ||
                conf_info.conf.output_flag(i_prc).ival())) {

               // Initialize
               for(m=0; m<max_prob_t; m++) pct_info[m].clear();

               // Compute PCT
               do_pct(pct_info, i, f_na, o_na);

               // Loop through all of the thresholds
               for(m=0; m<max_prob_t; m++) {

                  // Write out PCT
                  if(conf_info.conf.output_flag(i_pct).ival() &&
                     pct_info[m].pct.n() > 0) {

                     write_pct_row(shc, pct_info[m],
                        conf_info.conf.output_flag(i_pct).ival(),
                        stat_at, i_stat_row,
                        txt_at[i_pct], i_txt_row[i_pct]);
                  }

                  // Write out PSTD
                  if(conf_info.conf.output_flag(i_pstd).ival() &&
                     pct_info[m].pct.n() > 0) {

                     write_pstd_row(shc, pct_info[m],
                        conf_info.conf.output_flag(i_pstd).ival(),
                        stat_at, i_stat_row,
                        txt_at[i_pstd], i_txt_row[i_pstd]);
                  }

                  // Write out PJC
                  if(conf_info.conf.output_flag(i_pjc).ival() &&
                     pct_info[m].pct.n() > 0) {

                     write_pjc_row(shc, pct_info[m],
                        conf_info.conf.output_flag(i_pjc).ival(),
                        stat_at, i_stat_row,
                        txt_at[i_pjc], i_txt_row[i_pjc]);
                  }

                  // Write out PRC
                  if(conf_info.conf.output_flag(i_prc).ival() &&
                     pct_info[m].pct.n() > 0) {

                     write_prc_row(shc, pct_info[m],
                        conf_info.conf.output_flag(i_prc).ival(),
                        stat_at, i_stat_row,
                        txt_at[i_prc], i_txt_row[i_prc]);
                  }
               }
            } // end Compute PCT

         } // end for k

         // Write out the smoothed forecast, observation, and difference
         // fields in netCDF format for each GRIB code if requested in
         // the config file
         if(conf_info.conf.output_flag(i_nc).ival())
            write_nc(fcst_wd_smooth, obs_wd_smooth, i,
                     conf_info.interp_mthd[j],
                     conf_info.interp_wdth[j]);

      } // end for j

      // Loop through and apply the Neighborhood methods for each of the
      // neighborhood widths if requested in the config file
      if(conf_info.fcst_gci[i].pflag == 0 &&
         (conf_info.conf.output_flag(i_nbrctc).ival() ||
          conf_info.conf.output_flag(i_nbrcts).ival() ||
          conf_info.conf.output_flag(i_nbrcnt).ival())) {

         // Initialize
         for(j=0; j<frac_t; j++) nbrcts_info[j].clear();

         // Loop through and apply each of the neighborhood widths
         for(j=0; j<conf_info.get_n_nbr_wdth(); j++) {

            // Store the interpolation method and width being applied
            shc.set_interp_mthd(im_nbrhd);
            shc.set_interp_wdth(conf_info.conf.nbr_width(j).ival());

            // Loop through and apply each of the raw threshold values
            for(k=0; k<conf_info.fcst_ta[i].n_elements(); k++) {

               // Compute the fractional coverage based on the
               // threshold
               fcst_wd_smooth = fractional_coverage(fcst_wd,
                                   conf_info.conf.nbr_width(j).ival(),
                                   conf_info.fcst_ta[i][k],
                                   conf_info.conf.nbr_thresh().dval());

               obs_wd_smooth  = fractional_coverage(obs_wd,
                                   conf_info.conf.nbr_width(j).ival(),
                                   conf_info.obs_ta[i][k],
                                   conf_info.conf.nbr_thresh().dval());

               // Loop through the masks to be applied
               for(m=0; m<conf_info.get_n_mask(); m++) {

                  // Apply the current mask to the fields
                  apply_mask(fcst_wd_smooth, obs_wd_smooth,
                             conf_info.mask_wd[m],
                             f_na, o_na);

                  // Continue if no pairs were found
                  if(f_na.n_elements() == 0) continue;

                  // Set the mask name
                  shc.set_mask(conf_info.mask_name[m]);

                  if(verbosity > 1) {

                     conf_info.fcst_ta[i][k].get_str(
                        fcst_thresh_str);
                     conf_info.obs_ta[i][k].get_str(
                        obs_thresh_str);

                     cout << "Processing "
                          << conf_info.fcst_gci[i].info_str
                          << " versus "
                          << conf_info.obs_gci[i].info_str
                          << ", for interpolation method "
                          << shc.get_interp_mthd_str() << "("
                          << shc.get_interp_pnts_str()
                          << "), raw thresholds of "
                          << fcst_thresh_str << " and "
                          << obs_thresh_str
                          << ", over region " << shc.get_mask()
                          << ", using " << f_na.n_elements()
                          << " pairs.\n" << flush;
                  }

                  // Compute NBRCTS scores
                  if(conf_info.conf.output_flag(i_nbrctc).ival() ||
                     conf_info.conf.output_flag(i_nbrcts).ival()) {

                     // Initialize
                     for(n=0; n<conf_info.frac_ta.n_elements(); n++) {
                        nbrcts_info[n].clear();
                     }

                     do_nbrcts(nbrcts_info, i, j, k, m, f_na, o_na);

                     // Loop through all of the thresholds
                     for(n=0; n<conf_info.frac_ta.n_elements(); n++) {

                        // Only write out if n > 0
                        if(nbrcts_info[n].cts_info.cts.n() > 0) {

                           // Write out NBRCTC
                           if(conf_info.conf.output_flag(i_nbrctc).ival()) {

                              write_nbrctc_row(shc, nbrcts_info[n],
                                 conf_info.conf.output_flag(i_nbrctc).ival(),
                                 stat_at, i_stat_row,
                                 txt_at[i_nbrctc], i_txt_row[i_nbrctc]);
                           }

                           // Write out NBRCTS
                           if(conf_info.conf.output_flag(i_nbrcts).ival()) {

                              write_nbrcts_row(shc, nbrcts_info[n],
                                 conf_info.conf.output_flag(i_nbrcts).ival(),
                                 stat_at, i_stat_row,
                                 txt_at[i_nbrcts], i_txt_row[i_nbrcts]);
                           }
                        } // end if
                     } // end for n
                  } // end compute NBRCTS

                  // Compute NBRCNT scores
                  if(conf_info.conf.output_flag(i_nbrcnt).ival()) {

                     // Initialize
                     nbrcnt_info.clear();

                     do_nbrcnt(nbrcnt_info, i, j, k, m, f_na, o_na);

                     // Write out NBRCNT
                     if(conf_info.conf.output_flag(i_nbrcnt).ival() &&
                        nbrcnt_info.cnt_info.n > 0) {

                        write_nbrcnt_row(shc, nbrcnt_info,
                           conf_info.conf.output_flag(i_nbrcnt).ival(),
                           stat_at, i_stat_row,
                           txt_at[i_nbrcnt], i_txt_row[i_nbrcnt]);
                     }
                  } // end compute NBRCNT
               } // end for m
            } // end for k
         } // end for j
      } // end if
   } // end for i

   if(verbosity > 1) {
      cout << "\n----------------------------------------\n\n" << flush;
   }

   // Deallocate memory
   if(cts_info)    { delete [] cts_info;    cts_info    = (CTSInfo *)    0; }
   if(vl1l2_info)  { delete [] vl1l2_info;  vl1l2_info  = (VL1L2Info *)  0; }
   if(nbrcts_info) { delete [] nbrcts_info; nbrcts_info = (NBRCTSInfo *) 0; }
   if(pct_info)    { delete [] pct_info;    pct_info    = (PCTInfo *)    0; }
*/
   return;
}

////////////////////////////////////////////////////////////////////////

void parse_fcst_file_list(const char *fcst_file) {
   char tmp_str[PATH_MAX];
   ifstream f_in;

   // Open the ensemble file list
   f_in.open(fcst_file);
   if(!f_in) {
      cerr << "\n\nparse_fcst_file_list() -> "
           << "can't open input ensemble file list \"" << fcst_file
           << "\" for reading\n\n" << flush;
      exit(1);
   }

   // Read each ensemble member listed in the file
   while(f_in >> tmp_str) {
      fcst_file_list.add(tmp_str);
      n_fcst++;
   }

   // Close the input file
   f_in.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_first_ens_pass(const WrfData &wd, const Grid &ens_grid) {

   // Store the grid to be used through the verification
   grid = ens_grid;

   // Create a NetCDF file to store the ensemble output
   setup_nc_file(wd.get_valid_time(), wd.get_lead_time());

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_first_vx_pass(const WrfData &wd) {

   // Only if fields are to be verified
   if(conf_info.get_n_vx() > 0) {

      // Process masks Grids and Polylines in the config file
      conf_info.process_masks(grid);

      // Create output text files as requested in the config file
      setup_txt_files(wd.get_valid_time(), wd.get_lead_time());

      // Process the point observations
      // JHG process_point_obs();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_txt_files(unixtime valid_ut, int lead_sec) {
   int  i, max_col;
   ConcatString tmp_str;

   // Create output file names for the stat file and optional text files
   build_outfile_name(valid_ut, lead_sec, "", tmp_str);

   /////////////////////////////////////////////////////////////////////
   //
   // Setup the output STAT file
   //
   /////////////////////////////////////////////////////////////////////

   // Get the maximum number of data columns
   max_col = max(n_orank_columns, get_n_rhist_columns(n_fcst))
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

void setup_nc_file(unixtime valid_ut, int lead_sec) {
   int mon, day, yr, hr, min, sec;
   char attribute_str[PATH_MAX], time_str[max_str_len];
   char hostname_str[max_str_len];
   unixtime ut;

   // Create output NetCDF file name
   build_outfile_name(valid_ut, lead_sec, "_pairs.nc", out_nc_file);

   // Create a new NetCDF file and open it
   nc_out = new NcFile(out_nc_file, NcFile::Replace);

   if(!nc_out->is_valid()) {
      cerr << "\n\nERROR: setup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_nc_file << "\n\n" << flush;
      exit(1);
   }

   ut = time(NULL);
   unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
   sprintf(time_str, "%.4i-%.2i-%.2i %.2i:%.2i:%.2i",
           yr, mon, day, hr, min, sec);

   gethostname(hostname_str, max_str_len);

   sprintf(attribute_str, "File %s generated %s UTC on host %s",
           out_nc_file.text(), time_str, hostname_str);
   nc_out->add_att("FileOrigins", attribute_str);

   // Define Dimensions
   lat_dim = nc_out->add_dim("lat",   (long) grid.ny());
   lon_dim = nc_out->add_dim("lon",   (long) grid.nx());

   // Add the projection information
   write_netcdf_proj(nc_out, grid);

   return;
}

////////////////////////////////////////////////////////////////////////

void build_outfile_name(unixtime valid_ut, int lead_sec,
                        const char *suffix, ConcatString &str) {
   int mon, day, yr, hr, min, sec;
   int l_hr, l_min, l_sec;
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
   sec_to_hms(lead_sec, l_hr, l_min, l_sec);
   unix_to_mdyhms(valid_ut, mon, day, yr, hr, min, sec);
   sprintf(tmp_str, "%.2i%.2i%.2iL_%.4i%.2i%.2i_%.2i%.2i%.2iV",
           l_hr, l_min, l_sec, yr, mon, day, hr, min, sec);
   str << "_" << tmp_str;

   // Append the suffix
   str << suffix;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ens_nc(int i_gc, WrfData &wd) {
   int i, j;
   double t;
   NcVar *ens_var;
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
      if((double) (na_count[i]/n_fcst) < t) {
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
         ens_range[i] = (float) na_max[i] - na_min[i];
      }
   } // end for i

   // Add the ensemble mean if requested
   if(conf_info.conf.output_flag(i_nc_mean).ival()) {
      write_ens_var_float(i_gc, ens_mean, "ENS_MEAN", wd);
   }

   // Add the ensemble standard deviation if requested
   if(conf_info.conf.output_flag(i_nc_stdev).ival()) {
      write_ens_var_float(i_gc, ens_stdev, "ENS_STDEV", wd);
   }

   // Add the ensemble mean minus one standard deviation if requested
   if(conf_info.conf.output_flag(i_nc_minus).ival()) {
      write_ens_var_float(i_gc, ens_minus, "ENS_MINUS", wd);
   }

   // Add the ensemble mean plus one standard deviation if requested
   if(conf_info.conf.output_flag(i_nc_plus).ival()) {
      write_ens_var_float(i_gc, ens_plus, "ENS_PLUS", wd);
   }

   // Add the ensemble minimum value if requested
   if(conf_info.conf.output_flag(i_nc_min).ival()) {
      write_ens_var_float(i_gc, ens_min, "ENS_MIN", wd);
   }

   // Add the ensemble maximum value if requested
   if(conf_info.conf.output_flag(i_nc_max).ival()) {
      write_ens_var_float(i_gc, ens_max, "ENS_MAX", wd);
   }

   // Add the ensemble range if requested
   if(conf_info.conf.output_flag(i_nc_range).ival()) {
      write_ens_var_float(i_gc, ens_range, "ENS_RANGE", wd);
   }

   // Add the ensemble valid data count if requested
   if(conf_info.conf.output_flag(i_nc_vld).ival()) {
      write_ens_var_int(i_gc, ens_vld, "ENS_VLD", wd);
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
            if((double) (na_count[j]/n_fcst) < t) {
               ens_freq[j] = bad_data_float;
            }
            else {
               ens_freq[j] = (float) (na_thresh_count[i][j]/na_count[j]);
            }

         } // end for j

         // Write the ensemble relative frequency
         write_ens_var_float(i_gc, ens_freq, var_str, wd);

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

void write_ens_var_float(int i_gc, float *ens_data, const char *var_str, WrfData &wd) {
   NcVar *ens_var;
   char ens_var_name[max_str_len], tmp_str[max_str_len];
   unixtime ut;
   int mon, day, yr, hr, min, sec;

   // Construct the variable name
   sprintf(ens_var_name, "%s_%s_%s",
              conf_info.fcst_gci[i_gc].abbr_str.text(),
              conf_info.fcst_gci[i_gc].lvl_str.text(), var_str);
   ens_var = nc_out->add_var(ens_var_name, ncFloat, lat_dim, lon_dim);

   // Add variable attributes
   get_grib_code_abbr(conf_info.ens_gci[i_gc].code,
                      conf_info.conf.grib_ptv().ival(),
                      tmp_str);
   ens_var->add_att("name", tmp_str);
   get_grib_code_unit(conf_info.ens_gci[i_gc].code,
                      conf_info.conf.grib_ptv().ival(),
                      tmp_str);
   ens_var->add_att("units", tmp_str);
   get_grib_code_name(conf_info.ens_gci[i_gc].code,
                      conf_info.conf.grib_ptv().ival(),
                      tmp_str);
   ens_var->add_att("long_name", tmp_str);
   ens_var->add_att("level", conf_info.fcst_gci[i_gc].lvl_str.text());
   ens_var->add_att("_FillValue", bad_data_float);

   ut = wd.get_valid_time();
   unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
   sprintf(tmp_str, "%.4i-%.2i-%.2i %.2i:%.2i:%.2i",
           yr, mon, day, hr, min, sec);
   ens_var->add_att("valid_time", tmp_str);
   ens_var->add_att("valid_time_ut", (long int) ut);

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

void write_ens_var_int(int i_gc, int *ens_data, const char *var_str, WrfData &wd) {
   NcVar *ens_var;
   char ens_var_name[max_str_len], tmp_str[max_str_len];
   unixtime ut;
   int mon, day, yr, hr, min, sec;

   // Construct the variable name
   sprintf(ens_var_name, "%s_%s_%s",
              conf_info.fcst_gci[i_gc].abbr_str.text(),
              conf_info.fcst_gci[i_gc].lvl_str.text(), var_str);
   ens_var = nc_out->add_var(ens_var_name, ncInt, lat_dim, lon_dim);

   // Add variable attributes
   get_grib_code_abbr(conf_info.ens_gci[i_gc].code,
                      conf_info.conf.grib_ptv().ival(),
                      tmp_str);
   ens_var->add_att("name", tmp_str);
   get_grib_code_unit(conf_info.ens_gci[i_gc].code,
                      conf_info.conf.grib_ptv().ival(),
                      tmp_str);
   ens_var->add_att("units", tmp_str);
   get_grib_code_name(conf_info.ens_gci[i_gc].code,
                      conf_info.conf.grib_ptv().ival(),
                      tmp_str);
   ens_var->add_att("long_name", tmp_str);
   ens_var->add_att("level", conf_info.fcst_gci[i_gc].lvl_str.text());
   ens_var->add_att("_FillValue", bad_data_int);

   ut = wd.get_valid_time();
   unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
   sprintf(tmp_str, "%.4i-%.2i-%.2i %.2i:%.2i:%.2i",
           yr, mon, day, hr, min, sec);
   ens_var->add_att("valid_time", tmp_str);
   ens_var->add_att("valid_time_ut", (long int) ut);

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

void write_nc(const WrfData &fcst_wd, const WrfData &obs_wd,
              int lev, InterpMthd mthd, int wdth) {
   int i, n, x, y, mon, day, yr, hr, min, sec;
   float *fcst_data = (float *) 0;
   float *obs_data  = (float *) 0;
   float *diff_data = (float *) 0;
   double fval, oval;
   unixtime ut;
   NcVar *fcst_var, *obs_var, *diff_var;
   char fcst_var_name[max_str_len], obs_var_name[max_str_len];
   char mthd_str[max_str_len];
   char diff_var_name[max_str_len];
   char time_str[max_str_len];
   char tmp_str[max_str_len];

   // Get the interpolation method string
   interpmthd_to_string(mthd, mthd_str);

   // Allocate memory for the forecast, observation, and difference
   // fields
   fcst_data = new float [grid.nx()*grid.ny()];
   obs_data  = new float [grid.nx()*grid.ny()];
   diff_data = new float [grid.nx()*grid.ny()];

   // Compute the difference field for each of the masking regions
   for(i=0; i<conf_info.get_n_mask(); i++) {

      // If the neighborhood width is greater than 1, name the variables
      // using the smoothing method
      if(wdth > 1) {

         sprintf(fcst_var_name, "FCST_%s_%s_%s_%s_%i",
                 conf_info.fcst_gci[lev].abbr_str.text(),
                 conf_info.fcst_gci[lev].lvl_str.text(),
                 conf_info.mask_name[i],
                 mthd_str, wdth*wdth);
         sprintf(obs_var_name, "OBS_%s_%s_%s_%s_%i",
                 conf_info.obs_gci[lev].abbr_str.text(),
                 conf_info.obs_gci[lev].lvl_str.text(),
                 conf_info.mask_name[i],
                 mthd_str, wdth*wdth);
         sprintf(diff_var_name, "DIFF_%s_%s_%s_%s_%s_%s_%i",
                 conf_info.fcst_gci[lev].abbr_str.text(),
                 conf_info.fcst_gci[lev].lvl_str.text(),
                 conf_info.obs_gci[lev].abbr_str.text(),
                 conf_info.obs_gci[lev].lvl_str.text(),
                 conf_info.mask_name[i],
                 mthd_str, wdth*wdth);
      }
      // If the neighborhood width is equal to 1, no smoothing was done
      else {

         sprintf(fcst_var_name, "FCST_%s_%s_%s",
                 conf_info.fcst_gci[lev].abbr_str.text(),
                 conf_info.fcst_gci[lev].lvl_str.text(),
                 conf_info.mask_name[i]);
         sprintf(obs_var_name, "OBS_%s_%s_%s",
                 conf_info.obs_gci[lev].abbr_str.text(),
                 conf_info.obs_gci[lev].lvl_str.text(),
                 conf_info.mask_name[i]);
         sprintf(diff_var_name, "DIFF_%s_%s_%s_%s_%s",
                 conf_info.fcst_gci[lev].abbr_str.text(),
                 conf_info.fcst_gci[lev].lvl_str.text(),
                 conf_info.obs_gci[lev].abbr_str.text(),
                 conf_info.obs_gci[lev].lvl_str.text(),
                 conf_info.mask_name[i]);
      }

      // Define the forecast and difference variables
      fcst_var = nc_out->add_var(fcst_var_name, ncFloat,
                                 lat_dim, lon_dim);
      obs_var  = nc_out->add_var(obs_var_name,  ncFloat,
                                 lat_dim, lon_dim);
      diff_var = nc_out->add_var(diff_var_name, ncFloat,
                                 lat_dim, lon_dim);

      // Add variable attributes for the forecast field
      fcst_var->add_att("name", shc.get_fcst_var());
      get_grib_code_unit(conf_info.fcst_gci[lev].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      fcst_var->add_att("units", tmp_str);
      get_grib_code_name(conf_info.fcst_gci[lev].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      fcst_var->add_att("long_name", tmp_str);
      fcst_var->add_att("level", shc.get_fcst_lev());

      fcst_var->add_att("_FillValue", bad_data_float);

      ut = fcst_wd.get_valid_time();
      unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
      sprintf(time_str, "%.4i-%.2i-%.2i %.2i:%.2i:%.2i",
              yr, mon, day, hr, min, sec);
      fcst_var->add_att("valid_time", time_str);
      fcst_var->add_att("valid_time_ut", (long int) ut);

      fcst_var->add_att("masking_region", conf_info.mask_name[i]);
      fcst_var->add_att("smoothing_method", mthd_str);
      fcst_var->add_att("smoothing_neighborhood", wdth*wdth);

      // Add variable attributes for the observation field
      obs_var->add_att("name", shc.get_obs_var());
      get_grib_code_unit(conf_info.obs_gci[lev].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      obs_var->add_att("units", tmp_str);
      get_grib_code_name(conf_info.obs_gci[lev].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      obs_var->add_att("long_name", tmp_str);
      obs_var->add_att("level", shc.get_obs_lev());

      obs_var->add_att("_FillValue", bad_data_float);

      ut = obs_wd.get_valid_time();
      unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
      sprintf(time_str, "%.4i-%.2i-%.2i %.2i:%.2i:%.2i",
              yr, mon, day, hr, min, sec);
      obs_var->add_att("valid_time", time_str);
      obs_var->add_att("valid_time_ut", (long int) ut);

      obs_var->add_att("masking_region", conf_info.mask_name[i]);
      obs_var->add_att("smoothing_method", mthd_str);
      obs_var->add_att("smoothing_neighborhood", wdth*wdth);

      // Add variable attributes for the difference field
      sprintf(tmp_str, "%s_minus_%s",
         shc.get_fcst_var(), shc.get_obs_var());
      diff_var->add_att("name", tmp_str);
      get_grib_code_unit(conf_info.fcst_gci[lev].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      diff_var->add_att("fcst_units", tmp_str);
      get_grib_code_name(conf_info.fcst_gci[lev].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      diff_var->add_att("fcst_long_name", tmp_str);
      diff_var->add_att("fcst_level", shc.get_fcst_lev());
      get_grib_code_unit(conf_info.obs_gci[lev].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      diff_var->add_att("obs_units", tmp_str);
      get_grib_code_name(conf_info.obs_gci[lev].code,
                         conf_info.conf.grib_ptv().ival(),
                         tmp_str);
      diff_var->add_att("obs_long_name", tmp_str);
      diff_var->add_att("obs_level", shc.get_obs_lev());

      diff_var->add_att("_FillValue", bad_data_float);

      ut = fcst_wd.get_valid_time();
      unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
      sprintf(time_str, "%.4i-%.2i-%.2i %.2i:%.2i:%.2i",
              yr, mon, day, hr, min, sec);
      diff_var->add_att("fcst_valid_time", time_str);
      fcst_var->add_att("fcst_valid_time_ut", (long int) ut);

      ut = obs_wd.get_valid_time();
      unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
      sprintf(time_str, "%.4i-%.2i-%.2i %.2i:%.2i:%.2i",
              yr, mon, day, hr, min, sec);
      diff_var->add_att("obs_valid_time", time_str);
      fcst_var->add_att("obs_valid_time_ut", (long int) ut);

      diff_var->add_att("masking_region", conf_info.mask_name[i]);
      diff_var->add_att("smoothing_method", mthd_str);
      diff_var->add_att("smoothing_neighborhood", wdth*wdth);

      // Store the forecast, observation, and difference values
      for(x=0; x<grid.nx(); x++) {
         for(y=0; y<grid.ny(); y++) {

            n = two_to_one(grid.nx(), grid.ny(), x, y);

            // Check whether the mask is on or off for this point
            if(!conf_info.mask_wd[i].s_is_on(x, y)) {
               fcst_data[n] = bad_data_float;
               obs_data[n]  = bad_data_float;
               diff_data[n] = bad_data_float;
               continue;
            }

            // Retrieve the forecast and observation values
            fval = fcst_wd.get_xy_double(x, y);
            oval = obs_wd.get_xy_double(x, y);

            // Set the forecast data
            if(is_bad_data(fval)) fcst_data[n] = bad_data_float;
            else                  fcst_data[n] = fval;

            // Set the observation data
            if(is_bad_data(oval)) obs_data[n]  = bad_data_float;
            else                  obs_data[n]  = oval;

            // Set the difference data
            if(is_bad_data(fval) ||
               is_bad_data(oval)) diff_data[n] = bad_data_float;
            else                  diff_data[n] = fval - oval;
         } // end for y
      } // end for x

      // Write out the forecast field
      if(!fcst_var->put(&fcst_data[0], grid.ny(), grid.nx())) {
         cerr << "\n\nERROR: write_nc() -> "
              << "error with the fcst_var->put for fields "
              << shc.get_fcst_var() << " and " << shc.get_obs_var()
              << " and masking region " << conf_info.mask_name[i]
              << "\n\n" << flush;
         exit(1);
      }

      // Write out the observation field
      if(!obs_var->put(&obs_data[0], grid.ny(), grid.nx())) {
         cerr << "\n\nERROR: write_nc() -> "
              << "error with the obs_var->put for fields "
              << shc.get_fcst_var() << " and " << shc.get_obs_var()
              << " and masking region " << conf_info.mask_name[i]
              << "\n\n" << flush;
         exit(1);
      }

      // Write out the difference field
      if(!diff_var->put(&diff_data[0], grid.ny(), grid.nx())) {
         cerr << "\n\nERROR: write_nc() -> "
              << "error with the diff_var->put for fields "
              << shc.get_fcst_var() << " and " << shc.get_obs_var()
              << " and masking region " << conf_info.mask_name[i]
              << "\n\n" << flush;
         exit(1);
      }

   } // end for i

   // Deallocate and clean up
   if(fcst_data) { delete [] fcst_data; fcst_data = (float *) 0; }
   if(obs_data)  { delete [] obs_data;  obs_data  = (float *) 0; }
   if(diff_data) { delete [] diff_data; diff_data = (float *) 0; }

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

   // Close the output text files that were open for writing
   finish_txt_files();

   // Close the output NetCDF file as long as it was opened
   if(nc_out) {

      // List the NetCDF file after it is finished
      if(verbosity > 0) {
         cout << "Output file: "
              << out_nc_file << "\n" << flush;
      }
      nc_out->close();
      delete nc_out;
      nc_out = (NcFile *) 0;
   }
/* JHG
   // Close the forecast file
   if(fcst_ftype == NcFileType) {
      fcst_nc_file->close();
      delete fcst_nc_file;
      fcst_nc_file = (NcFile *) 0;
   }
   else if(fcst_ftype == GbFileType) {
      fcst_gb_file.close();
   }

   // Deallocate threshold count arrays
   if(na_thresh_count) {
      for(i=0; i<conf_info.get_max_n_thresh(); i++) {
         na_thresh_count[i].clear();
      }
      delete [] na_thresh_count;
      na_thresh_count = (NumArray *) 0;
   }

*/
   return;
}

////////////////////////////////////////////////////////////////////////

void usage(int argc, char *argv[]) {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tn_ens ens_file_1 ... ens_file_n | ens_file_list\n"
        << "\tconfig_file\n"
        << "\tout_file\n"
        << "\t[-grid_obs file]\n"
        << "\t[-point_obs file]\n"
        << "\t[-outdir path]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"n_ens\" is the number of ensemble members "
        << "(required).\n"

        << "\t\t\"ens_file_1 ... ens_file_n\" is a list of ensemble "
        << "member file names (required).\n"

        << "\t\t\"ens_file_list\" is an ASCII file containing a list "
        << "of ensemble member file names (required).\n"

        << "\t\t\"config_file\" is an EnsembleStatConfig file "
        << "containing the desired configuration settings (required).\n"

        << "\t\t\"out_file\" is the output NetCDF file name to be "
        << "written (required).\n"

        << "\t\t\"-grid_obs file\" specifies a gridded observation file. "
        << "May be used multiple times (optional).\n"

        << "\t\t\"-point_obs file\" specifies a point observation file. "
        << "May be used multiple times (optional).\n"

        << "\t\t\"-outdir path\" overrides the default output directory "
        << "(" << out_dir << ") (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << verbosity << ") (optional).\n"

        << "\n\tNOTE: The ensemble members and gridded observations "
        << "must be on the same grid.\n\n" << flush;

   return;
}

////////////////////////////////////////////////////////////////////////
