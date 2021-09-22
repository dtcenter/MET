// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2021
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   gen_ens_prod.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    09/10/21  Halley Gotway   Initial version (MET #1904).
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
#include <limits.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "gen_ens_prod.h"

#include "vx_nc_util.h"
#include "vx_data2d_nc_met.h"
#include "vx_regrid.h"
#include "vx_log.h"

#include "nc_obs_util.h"
#include "nc_point_obs_in.h"

////////////////////////////////////////////////////////////////////////

static void process_command_line  (int, char **);
static void process_grid          (const Grid &);
static void process_n_vld         ();
static void process_ensemble      ();
static bool get_data_plane        (const char *, GrdFileType, VarInfo *,
                                   DataPlane &, bool do_regrid);
static bool get_data_plane_array  (const char *, GrdFileType, VarInfo *,
                                   DataPlaneArray &, bool do_regrid);

static void clear_counts();
static void track_counts(int, const DataPlane &);

static void setup_nc_file   (unixtime, const char *);
static void setup_txt_files ();
static void setup_table     (AsciiTable &);

static void write_ens_nc(int, DataPlane &);
static void write_ens_var_float(int, float *, DataPlane &,
                                const char *, const char *);
static void write_ens_var_int(int, int *, DataPlane &,
                              const char *, const char *);

static void write_orank_nc(PairDataEnsemble &, DataPlane &, int, int, int);
static void write_orank_var_float(int, int, int, float *, DataPlane &,
                                  const char *, const char *);
static void write_orank_var_int(int, int, int, int *, DataPlane &,
                                const char *, const char *);

static void add_var_att_local(VarInfo *, NcVar *, bool is_int, DataPlane &,
                              const char *, const char *);

static void finish_txt_files();
static void clean_up();

static void usage();

static void set_ens_files  (const StringArray &);
static void set_out_file   (const StringArray &);
static void set_config_file(const StringArray &);
static void set_ctrl_file  (const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Check for valid ensemble data
   // JHG, this is dumb. This logic should be incorporated into process_ensemble().
   process_n_vld();

   // Process the ensemble fields
   process_ensemble();

   // Close the text files and deallocate memory
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   int i;
   CommandLine cline;
   ConcatString default_config_file;
   Met2dDataFile *ens_mtddf = (Met2dDataFile *) 0;

   //
   // Check for zero arguments
   //
   if(argc == 1) usage();

   //
   // Parse the command line into tokens
   //
   cline.set(argc, argv);

   //
   // Set the usage function
   //
   cline.set_usage(usage);

   //
   // Add the options function calls
   //
   cline.add(set_ens_files,   "-ens",   -1);
   cline.add(set_out_file,    "-out",    1);
   cline.add(set_config_file, "-config", 1);
   cline.add(set_ctrl_file,   "-ctrl",   1);

   //
   // Parse the command line
   //
   cline.parse();

   // Check for error, there should be zero arguments left
   if(cline.n() != 0) usage();

   // Check that the required arguments have been set
   if(ens_files.n() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the ensemble file list must be set using the "
           << "\"-ens\" option.\n\n";
      exit(1);
   }
   if(config_file.length() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the output file must be set using the "
           << "\"-out\" option.\n\n";
      exit(1);
   }
   if(config_file.length() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the configuration file must be set using the "
           << "\"-config\" option.\n\n";
      exit(1);
   }

   // Create the default config file name
   default_config_file = replace_path(default_config_filename);

   // List the config files
   mlog << Debug(1)
        << "Default Config File: " << default_config_file << "\n"
        << "User Config File: "    << config_file << "\n";

   // Read the config files
   conf_info.read_config(default_config_file, config_file);

   // Get the ensemble file type from config, if present
   etype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_ens));

   // Read the first input ensemble file
   if(!(ens_mtddf = mtddf_factory.new_met_2d_data_file(ens_files[0].c_str(), etype))) {
      mlog << Error << "\nprocess_command_line() -> "
           << "trouble reading ensemble file \"" << ens_files[0] << "\"\n\n";
      exit(1);
   }

   // Store the input ensemble file type
   etype = ens_mtddf->file_type();

   // Process the configuration
   conf_info.process_config(etype);

   // Allocate arrays to store threshold counts
   thresh_count_na       = new NumArray   [conf_info.get_max_n_cat_ta()];
   thresh_nbrhd_count_na = new NumArray * [conf_info.get_max_n_cat_ta()];

   for(i=0; i<conf_info.get_max_n_cat_ta(); i++) {
      thresh_nbrhd_count_na[i] = new NumArray [conf_info.get_n_nbrhd()];
   }

   // List the input ensemble files
   mlog << Debug(1) << "Ensemble Files["
        << ens_files.n() << "]:\n";
   for(i=0; i<ens_files.n(); i++) {
      mlog << "   " << ens_files[i]  << "\n";
   }

   // Check for missing non-python ensemble files
   for(i=0; i<ens_files.n(); i++) {
      if(!file_exists(ens_files[i].c_str()) &&
         !is_python_grdfiletype(etype)) {
         mlog << Warning << "\nprocess_command_line() -> "
              << "can't open input ensemble file: "
              << ens_files[i] << "\n\n";
         ens_file_vld.add(0);
      }
      else {
         ens_file_vld.add(1);
      }
   }

   // Deallocate memory for data files
   if(ens_mtddf) { delete ens_mtddf; ens_mtddf = (Met2dDataFile *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_grid(const Grid &fcst_grid) {
   Grid obs_grid;

   // Parse regridding logic
   RegridInfo ri;
   if(conf_info.get_n_ens_var() > 0) {
      ri = conf_info.ens_info[0]->regrid();
   }
   else {
      mlog << Error << "\nprocess_grid() -> "
           << "at least one ensemble field must be specified!\n\n";
      exit(1);
   }

   // Determine the verification grid
   grid = parse_vx_grid(ri, &fcst_grid, &fcst_grid);
   nxy  = grid.nx() * grid.ny();

   return;
}

////////////////////////////////////////////////////////////////////////

void process_n_vld() {
   int i, j, n_vld;
   DataPlane dp;
   DataPlaneArray dpa;

   // Initialize
   n_ens_vld.clear();

   // Loop through the ensemble fields to be processed
   for(i=0; i<conf_info.get_n_ens_var(); i++) {

      // Loop through the ensemble files
      for(j=0, n_vld=0; j<ens_files.n(); j++) {

         // Check for valid file
         if(!ens_file_vld[j]) continue;

         // Check for valid data
         if(!get_data_plane(ens_files[j].c_str(), etype,
                            conf_info.ens_info[i], dp, false)) {
            mlog << Warning << "\nprocess_n_vld() -> "
                 << "ensemble field \""
                 << conf_info.ens_info[i]->magic_str()
                 << "\" not found in file \"" << ens_files[j]
                 << "\"\n\n";
         }
         else {

            // Increment the valid counter
            n_vld++;
         }
      } // end for j

      // Check for enough valid data
      if((double) n_vld/n_ens < conf_info.vld_ens_thresh) {
         mlog << Error << "\nprocess_n_vld() -> "
              << n_ens - n_vld << " missing ensemble fields exceeds the "
              << "maximum allowable specified by \"ens.ens_thresh\" "
              << "in the configuration file.\n\n";
         exit(1);
      }

      // Store the valid data count
      n_ens_vld.add(n_vld);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

bool get_data_plane(const char *infile, GrdFileType ftype,
                    VarInfo *info, DataPlane &dp, bool do_regrid) {
   bool found;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;

   // Read the current ensemble file
   if(!(mtddf = mtddf_factory.new_met_2d_data_file(infile, ftype))) {
      mlog << Error << "\nget_data_plane() -> "
           << "trouble reading file \"" << infile << "\"\n\n";
      exit(1);
   }

   // Read the gridded data field
   if((found = mtddf->data_plane(*info, dp))) {

      // Setup the verification grid, if necessary
      if(nxy == 0) process_grid(mtddf->grid());

      // Regrid, if requested and necessary
      if(do_regrid && !(mtddf->grid() == grid)) {
         mlog << Debug(1)
              << "Regridding field \"" << info->magic_str()
              << "\" to the verification grid.\n";
         dp = met_regrid(dp, mtddf->grid(), grid, info->regrid());
      }

      // Store the valid time, if not already set
      if(ens_valid_ut == (unixtime) 0) {
         ens_valid_ut = dp.valid();
      }
      // Check to make sure that the valid time doesn't change
      else if(ens_valid_ut != dp.valid()) {
         mlog << Warning << "\nget_data_plane() -> "
              << "The valid time has changed, "
              << unix_to_yyyymmdd_hhmmss(ens_valid_ut)
              << " != " << unix_to_yyyymmdd_hhmmss(dp.valid())
              << " in \"" << infile << "\"\n\n";
      }

   } // end if found

   // Deallocate the data file pointer, if necessary
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }

   return(found);
}

////////////////////////////////////////////////////////////////////////

bool get_data_plane_array(const char *infile, GrdFileType ftype,
                          VarInfo *info, DataPlaneArray &dpa,
                          bool do_regrid) {
   int n, i;
   bool found;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;

   // Read the current ensemble file
   if(!(mtddf = mtddf_factory.new_met_2d_data_file(infile, ftype))) {
      mlog << Error << "\nget_data_plane_array() -> "
           << "trouble reading file \"" << infile << "\"\n\n";
      exit(1);
   }

   // Read the gridded data field
   n = mtddf->data_plane_array(*info, dpa);

   // Check for at least one field
   if((found = (n > 0))) {

      // Setup the verification grid, if necessary
      if(nxy == 0) process_grid(mtddf->grid());

      // Regrid, if requested and necessary
      if(do_regrid && !(mtddf->grid() == grid)) {
         mlog << Debug(1)
              << "Regridding " << dpa.n_planes()
              << " field(s) \"" << info->magic_str()
              << "\" to the verification grid.\n";

         // Loop through the forecast fields
         for(i=0; i<dpa.n_planes(); i++) {
            dpa[i] = met_regrid(dpa[i], mtddf->grid(), grid,
                                info->regrid());
         }
      }

      // Store the valid time, if not already set
      if(ens_valid_ut == (unixtime) 0) {
         ens_valid_ut = dpa[0].valid();
      }
      // Check to make sure that the valid time doesn't change
      else if(ens_valid_ut != dpa[0].valid()) {
         mlog << Warning << "\nget_data_plane_array() -> "
              << "The valid time has changed, "
              << unix_to_yyyymmdd_hhmmss(ens_valid_ut)
              << " != " << unix_to_yyyymmdd_hhmmss(dpa[0].valid())
              << " in \"" << infile << "\"\n\n";
      }

   } // end if found

   // Deallocate the data file pointer, if necessary
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }

   return(found);
}

////////////////////////////////////////////////////////////////////////

void process_ensemble() {
   int i, j;
   bool reset;
   DataPlane ens_dp;
   unixtime max_init_ut = bad_data_ll;

   // Loop through each of the ensemble fields to be processed
   for(i=0; i<conf_info.get_n_ens_var(); i++) {

      mlog << Debug(2) << "\n" << sep_str << "\n\n"
           << "Processing ensemble field: "
           << conf_info.ens_info[i]->magic_str() << "\n";

      // Loop through each of the input forecast files
      for(j=0, reset=true; j<ens_files.n(); j++) {

         // Skip bad data files
         if(!ens_file_vld[j]) continue;

         // Read the current field
         if(!get_data_plane(ens_files[j].c_str(), etype,
                            conf_info.ens_info[i], ens_dp, true)) continue;

         // Create a NetCDF file to store the ensemble output
         if(nc_out == (NcFile *) 0) {
            setup_nc_file(ens_dp.valid(), "_ens.nc");
         }

         // Reset the running sums and counts
         if(reset) {
            clear_counts();
            reset = false;
         }

         // Apply current data to the running sums and counts
         track_counts(i, ens_dp);

         // Keep track of the maximum initialization time
         if(is_bad_data(max_init_ut) || ens_dp.init() > max_init_ut) {
            max_init_ut = ens_dp.init();
         }

      } // end for j

      // Write out the ensemble information to a NetCDF file
      ens_dp.set_init(max_init_ut);
      write_ens_nc(i, ens_dp);

   } // end for i

   // Close the output NetCDF file
   if(nc_out) {
      delete nc_out;
      nc_out = (NcFile *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void clear_counts() {
   int i, j, k;

   // Allocate memory in one big chunk based on grid size, if needed
   count_na.extend(nxy);
   min_na.extend(nxy);
   max_na.extend(nxy);
   sum_na.extend(nxy);
   sum_sq_na.extend(nxy);
   for(i=0; i<conf_info.get_max_n_cat_ta(); i++) {
      thresh_count_na[i].extend(nxy);
      for(j=0; j<conf_info.get_n_nbrhd(); j++) {
         thresh_nbrhd_count_na[i][j].extend(nxy);
      }
   }

   // Erase existing values
   count_na.erase();
   min_na.erase();
   max_na.erase();
   sum_na.erase();
   sum_sq_na.erase();
   for(i=0; i<conf_info.get_max_n_cat_ta(); i++) {
      thresh_count_na[i].erase();
      for(j=0; j<conf_info.get_n_nbrhd(); j++) {
         thresh_nbrhd_count_na[i][j].erase();
      }
   }

   // Initialize arrays
   for(i=0; i<nxy; i++) {
      count_na.add(0);
      min_na.add(bad_data_double);
      max_na.add(bad_data_double);
      sum_na.add(0.0);
      sum_sq_na.add(0.0);
      for(j=0; j<conf_info.get_max_n_cat_ta(); j++) {
         thresh_count_na[j].add(0);
         for(k=0; k<conf_info.get_n_nbrhd(); k++) {
            thresh_nbrhd_count_na[j][k].add(0);
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void track_counts(int i_vx, const DataPlane &dp) {
   int i, j, k;
   double v;

   // Pointers to data buffers for faster access
   const double *Data = dp.data();
   double *CountBuf   = count_na.buf();
   double *MinBuf     = min_na.buf();
   double *MaxBuf     = max_na.buf();
   double *SumBuf     = sum_na.buf();
   double *SumSqBuf   = sum_sq_na.buf();

   // Ensemble thresholds
   const int Nthresh = conf_info.ens_cat_ta[i_vx].n();
   SingleThresh *ThreshBuf = conf_info.ens_cat_ta[i_vx].buf();

   // Increment counts for each grid point
   for(i=0; i<nxy; i++) {

      // Get current value
      v = *Data++;

      // Skip the bad data value
      if(is_bad_data(v)) continue;

      // Otherwise, update counts and sums
      else {

         CountBuf[i] += 1;

         if(v <= MinBuf[i] || is_bad_data(MinBuf[i])) MinBuf[i] = v;
         if(v >= MaxBuf[i] || is_bad_data(MaxBuf[i])) MaxBuf[i] = v;

         SumBuf[i]   += v;
         SumSqBuf[i] += v*v;

         for(j=0; j<Nthresh; j++) {
            if(ThreshBuf[j].check(v)) thresh_count_na[j].inc(i, 1);
         }
      } // end else

   } // end for i

   // Increment NMEP count anywhere fractional coverage > 0
   if(conf_info.nc_info[i_vx].do_nmep) {
      DataPlane frac_dp;

      // Loop over thresholds
      for(i=0; i<Nthresh; i++) {

         // Loop over neighborhood sizes
         for(j=0; j<conf_info.get_n_nbrhd(); j++) {

            // Compute fractional coverage
            fractional_coverage(dp, frac_dp,
               conf_info.nbrhd_prob.width[j],
               conf_info.nbrhd_prob.shape,
               ThreshBuf[i], conf_info.nbrhd_prob.vld_thresh);

            // Increment counts
            const double *Frac = frac_dp.data();
            for(k=0; k<nxy; k++) {
               if(Frac[k] > 0) thresh_nbrhd_count_na[i][j].inc(k, 1);
            } // end for k

         } // end for j
      } // end for i
   } // end if do_nmep

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_nc_file(unixtime valid_ut, const char *suffix) {

   // Create a new NetCDF file and open it
   nc_out = open_ncfile(out_file.c_str(), true);

   if(IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_file << "\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, out_file.text(), program_name,
                       conf_info.model.c_str());

   // Add the projection information
   write_netcdf_proj(nc_out, grid);

   // Define Dimensions
   lat_dim = add_dim(nc_out, "lat", (long) grid.ny());
   lon_dim = add_dim(nc_out, "lon", (long) grid.nx());

   // Add the lat/lon variables
   if(conf_info.nc_info[0].do_latlon) {
      write_netcdf_latlon(nc_out, &lat_dim, &lon_dim, grid);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ens_nc(int i_ens, DataPlane &dp) {
   int i, j, k, l;
   double t, v;
   char type_str[max_str_len];
   DataPlane prob_dp, smooth_dp;

   // Arrays for storing ensemble data
   float *ens_mean  = (float *) 0;
   float *ens_stdev = (float *) 0;
   float *ens_minus = (float *) 0;
   float *ens_plus  = (float *) 0;
   float *ens_min   = (float *) 0;
   float *ens_max   = (float *) 0;
   float *ens_range = (float *) 0;
   int   *ens_vld   = (int   *) 0;
   float *ens_prob  = (float *) 0;

   // Allocate memory for storing ensemble data
   ens_mean  = new float [grid.nx()*grid.ny()];
   ens_stdev = new float [grid.nx()*grid.ny()];
   ens_minus = new float [grid.nx()*grid.ny()];
   ens_plus  = new float [grid.nx()*grid.ny()];
   ens_min   = new float [grid.nx()*grid.ny()];
   ens_max   = new float [grid.nx()*grid.ny()];
   ens_range = new float [grid.nx()*grid.ny()];
   ens_vld   = new int   [grid.nx()*grid.ny()];
   ens_prob  = new float [grid.nx()*grid.ny()];

   // Store the threshold for the ratio of valid data points
   t = conf_info.vld_data_thresh;

   // Store the data
   for(i=0; i<count_na.n(); i++) {

      // Valid data count
      ens_vld[i] = nint(count_na[i]);

      // Check for too much missing data
      if((double) (count_na[i]/n_ens_vld[i_ens]) < t) {
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
         ens_mean[i]  = (float) (sum_na[i]/count_na[i]);
         ens_stdev[i] = (float) compute_stdev(sum_na[i], sum_sq_na[i], ens_vld[i]);
         ens_minus[i] = (float) ens_mean[i] - ens_stdev[i];
         ens_plus[i]  = (float) ens_mean[i] + ens_stdev[i];
         ens_min[i]   = (float) min_na[i];
         ens_max[i]   = (float) max_na[i];
         v = max_na[i] - min_na[i];
         if(is_eq(v, 0.0)) v = 0;
         ens_range[i] = (float) v;
      }
   } // end for i

   // Add the ensemble mean, if requested
   if(conf_info.nc_info[i_ens].do_mean) {
      write_ens_var_float(i_ens, ens_mean, dp,
                          "ENS_MEAN",
                          "Ensemble Mean");
   }

   // Add the ensemble standard deviation, if requested
   if(conf_info.nc_info[i_ens].do_stdev) {
      write_ens_var_float(i_ens, ens_stdev, dp,
                          "ENS_STDEV",
                          "Ensemble Standard Deviation");
   }

   // Add the ensemble mean minus one standard deviation, if requested
   if(conf_info.nc_info[i_ens].do_minus) {
      write_ens_var_float(i_ens, ens_minus, dp,
                          "ENS_MINUS",
                          "Ensemble Mean Minus 1 Standard Deviation");
   }

   // Add the ensemble mean plus one standard deviation, if requested
   if(conf_info.nc_info[i_ens].do_plus) {
      write_ens_var_float(i_ens, ens_plus, dp,
                          "ENS_PLUS",
                          "Ensemble Mean Plus 1 Standard Deviation");
   }

   // Add the ensemble minimum value, if requested
   if(conf_info.nc_info[i_ens].do_min) {
      write_ens_var_float(i_ens, ens_min, dp,
                          "ENS_MIN",
                          "Ensemble Minimum");
   }

   // Add the ensemble maximum value, if requested
   if(conf_info.nc_info[i_ens].do_max) {
      write_ens_var_float(i_ens, ens_max, dp,
                          "ENS_MAX",
                          "Ensemble Maximum");
   }

   // Add the ensemble range, if requested
   if(conf_info.nc_info[i_ens].do_range) {
      write_ens_var_float(i_ens, ens_range, dp,
                          "ENS_RANGE",
                          "Ensemble Range");
   }

   // Add the ensemble valid data count, if requested
   if(conf_info.nc_info[i_ens].do_vld) {
      write_ens_var_int(i_ens, ens_vld, dp,
                        "ENS_VLD",
                        "Ensemble Valid Data Count");
   }

   // Add the ensemble relative frequencies and neighborhood probabilities, if requested
   if(conf_info.nc_info[i_ens].do_freq || conf_info.nc_info[i_ens].do_nep) {

      prob_dp.set_size(dp.nx(), dp.ny());

      // Loop through each threshold
      for(i=0; i<conf_info.ens_cat_ta[i_ens].n(); i++) {

         // Initialize
         prob_dp.erase();

         // Output variable name
         snprintf(type_str, sizeof(type_str), "ENS_FREQ_%s",
                  conf_info.ens_cat_ta[i_ens][i].get_abbr_str().contents().c_str());

         // Compute the ensemble relative frequency
         for(j=0; j<count_na.n(); j++) {

            // Check for too much missing data
            if((double) (count_na[j]/n_ens_vld[i_ens]) < t) {
               ens_prob[j] = bad_data_float;
            }
            else {
               ens_prob[j] = (float) (thresh_count_na[i][j]/count_na[j]);
            }

            // Also store value in a DataPlane object
            prob_dp.buf()[j] = ens_prob[j];

         } // end for j

         // Write the ensemble relative frequency
         if(conf_info.nc_info[i_ens].do_freq) {
            write_ens_var_float(i_ens, ens_prob, dp,
                                type_str,
                                "Ensemble Relative Frequency");
         }

         // Write the neighborhood ensemble probability
         if(conf_info.nc_info[i_ens].do_nep) {

            GaussianInfo info;
            // Compute the mean neighborhood probability
            for(j=0; j<conf_info.get_n_nbrhd(); j++) {

               smooth_dp = smooth_field(prob_dp, InterpMthd_UW_Mean,
                              conf_info.nbrhd_prob.width[j],
                              conf_info.nbrhd_prob.shape,
                              conf_info.nbrhd_prob.vld_thresh, info);

               for(k=0; k<count_na.n(); k++) {
                  ens_prob[k] = (float) smooth_dp.buf()[k];
               }

               // Output variable name
               snprintf(type_str, sizeof(type_str), "ENS_NEP_%s_%s%i",
                        conf_info.ens_cat_ta[i_ens][i].get_abbr_str().contents().c_str(),
                        interpmthd_to_string(InterpMthd_Nbrhd).c_str(),
                        conf_info.nbrhd_prob.width[j]*conf_info.nbrhd_prob.width[j]);

               write_ens_var_float(i_ens, ens_prob, dp,
                                   type_str,
                                   "Neighborhood Ensemble Probability");

            } // end for j
         } // end if do_nep
      } // end for i
   } // end if

   // Add the neighborhood maximum ensemble probabilities, if requested
   if(conf_info.nc_info[i_ens].do_nmep) {

      prob_dp.set_size(dp.nx(), dp.ny());

      // Loop through each threshold
      for(i=0; i<conf_info.ens_cat_ta[i_ens].n(); i++) {

         // Loop through each neigbhorhood size
         for(j=0; j<conf_info.get_n_nbrhd(); j++) {

            // Initialize
            prob_dp.erase();

            // Compute the neighborhood maximum ensemble probability
            for(k=0; k<count_na.n(); k++) {

               // Check for too much missing data
               if((double) (count_na[k]/n_ens_vld[i_ens]) < t) {
                  prob_dp.buf()[k] = bad_data_double;
               }
               else {
                  prob_dp.buf()[k] = (double) (thresh_nbrhd_count_na[i][j][k]/count_na[k]);
               }
            } // end for k

            // Apply requested NMEP smoothers
            for(k=0; k<conf_info.nmep_smooth.n_interp; k++) {

               smooth_dp = smooth_field(prob_dp,
                              string_to_interpmthd(conf_info.nmep_smooth.method[k].c_str()),
                              conf_info.nmep_smooth.width[k],
                              conf_info.nmep_smooth.shape,
                              conf_info.nmep_smooth.vld_thresh,
                              conf_info.nmep_smooth.gaussian);

               for(l=0; l<count_na.n(); l++) {
                  ens_prob[l] = (float) smooth_dp.buf()[l];
               }

               // Output variable name
               snprintf(type_str, sizeof(type_str), "ENS_NMEP_%s_%s%i_%s%i",
                        conf_info.ens_cat_ta[i_ens][i].get_abbr_str().contents().c_str(),
                        interpmthd_to_string(InterpMthd_Nbrhd).c_str(),
                        conf_info.nbrhd_prob.width[j]*conf_info.nbrhd_prob.width[j],
                        conf_info.nmep_smooth.method[k].c_str(),
                        conf_info.nmep_smooth.width[k]*conf_info.nmep_smooth.width[k]);

               write_ens_var_float(i_ens, ens_prob, dp,
                                   type_str,
                                   "Neighborhood Maximum Ensemble Probability");

            } // end for k
         } // end for j
      } // end for i
   } // end if do_nep

   // Deallocate and clean up
   if(ens_mean)  { delete [] ens_mean;  ens_mean  = (float *) 0; }
   if(ens_stdev) { delete [] ens_stdev; ens_stdev = (float *) 0; }
   if(ens_minus) { delete [] ens_minus; ens_minus = (float *) 0; }
   if(ens_plus)  { delete [] ens_plus;  ens_plus  = (float *) 0; }
   if(ens_min)   { delete [] ens_min;   ens_min   = (float *) 0; }
   if(ens_max)   { delete [] ens_max;   ens_max   = (float *) 0; }
   if(ens_range) { delete [] ens_range; ens_range = (float *) 0; }
   if(ens_vld)   { delete [] ens_vld;   ens_vld   = (int   *) 0; }
   if(ens_prob)  { delete [] ens_prob;  ens_prob  = (float *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ens_var_float(int i_ens, float *ens_data, DataPlane &dp,
                         const char *type_str,
                         const char *long_name_str) {
   NcVar ens_var;
   ConcatString ens_var_name, var_str, name_str, cs;

   // Append nc_pairs_var_str config file entry
   cs = conf_info.ens_var_str[i_ens];
   if(cs.length() > 0) var_str << "_" << cs;

   // Construct the variable name
   ens_var_name << cs_erase
                << conf_info.ens_info[i_ens]->name_attr() << "_"
                << conf_info.ens_info[i_ens]->level_attr()
                << var_str << "_" << type_str;

   // Skip variable names that have already been written
   if(nc_ens_var_sa.has(ens_var_name)) return;

   // Otherwise, add to the list of previously defined variables
   nc_ens_var_sa.add(ens_var_name);

   ens_var = add_var(nc_out, (string)ens_var_name, ncFloat, lat_dim, lon_dim);

   //
   // Construct the variable name attribute
   // For the ensemble mean, just use the variable name.
   // For all other fields, append the field type.
   //
   if(strcmp(type_str, "ENS_MEAN") == 0) {
      name_str << cs_erase
               << conf_info.ens_info[i_ens]->name_attr();
   }
   else {
      name_str << cs_erase
               << conf_info.ens_info[i_ens]->name_attr() << "_"
               << type_str;
   }

   // Add the variable attributes
   add_var_att_local(conf_info.ens_info[i_ens], &ens_var, false, dp,
                     name_str.c_str(), long_name_str);

   // Write the data
   if(!put_nc_data_with_dims(&ens_var, &ens_data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_ens_var_float() -> "
           << "error in ens_var->put for the " << ens_var_name
           << " field.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ens_var_int(int i_ens, int *ens_data, DataPlane &dp,
                       const char *type_str,
                       const char *long_name_str) {
   NcVar ens_var;
   ConcatString ens_var_name, var_str, name_str, cs;

   // Append nc_pairs_var_str config file entry
   cs = conf_info.ens_var_str[i_ens];
   if(cs.length() > 0) var_str << "_" << cs;

   // Construct the variable name
   ens_var_name << cs_erase
                << conf_info.ens_info[i_ens]->name_attr() << "_"
                << conf_info.ens_info[i_ens]->level_attr()
                << var_str << "_" << type_str;

   // Skip variable names that have already been written
   if(nc_ens_var_sa.has(ens_var_name)) return;

   // Otherwise, add to the list of previously defined variables
   nc_ens_var_sa.add(ens_var_name);

   int deflate_level = conf_info.get_compression_level();
   ens_var = add_var(nc_out, (string)ens_var_name, ncInt, lat_dim, lon_dim, deflate_level);

   // Construct the variable name attribute
   name_str << cs_erase
            << conf_info.ens_info[i_ens]->name_attr() << "_"
            << type_str;

   // Add the variable attributes
   add_var_att_local(conf_info.ens_info[i_ens], &ens_var, true, dp,
                     name_str.c_str(), long_name_str);

   // Write the data
   if(!put_nc_data_with_dims(&ens_var, &ens_data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_ens_var_int() -> "
           << "error in ens_var->put for the " << ens_var_name
           << " field.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void add_var_att_local(VarInfo *info, NcVar *nc_var, bool is_int, DataPlane &dp,
                       const char *name_str, const char *long_name_str) {
   ConcatString att_str;

   // Construct the long name
   att_str << cs_erase
           << info->name_attr() << " at "
           << info->level_attr() << " "
           << long_name_str;

   // Add variable attributes
   add_att(nc_var, "name", name_str);
   add_att(nc_var, "long_name", (string)att_str);
   add_att(nc_var, "level", (string)info->level_attr());
   add_att(nc_var, "units", (string)info->units_attr());

   if(is_int) add_att(nc_var, "_FillValue", bad_data_int);
   else       add_att(nc_var, "_FillValue", bad_data_float);

   // Write out times
   write_netcdf_var_times(nc_var, dp);

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {
   int i, j;

   mlog << Debug(2) << "\n" << sep_str << "\n\n";

   // List the output NetCDF file
   mlog << Debug(1) << "Output file: " << out_file << "\n";

   // Deallocate threshold count arrays
   if(thresh_count_na) {
      for(i=0; i<conf_info.get_max_n_cat_ta(); i++) {
         thresh_count_na[i].clear();
      }
      delete [] thresh_count_na;
      thresh_count_na = (NumArray *) 0;
   }
   if(thresh_nbrhd_count_na) {
      for(i=0; i<conf_info.get_max_n_cat_ta(); i++) {
         for(j=0; j<conf_info.get_n_nbrhd(); j++) {
            thresh_nbrhd_count_na[i][j].clear();
         }
         delete [] thresh_nbrhd_count_na[i];
         thresh_nbrhd_count_na[i] = (NumArray *) 0;
      }
      delete [] thresh_nbrhd_count_na;
      thresh_nbrhd_count_na = (NumArray **) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\t-ens file_1 ... file_n | ens_file_list\n"
        << "\t-out file\n"
        << "\t-config file\n"
        << "\t[-ctrl file]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-ens file_1 ... file_n\" are the gridded ensemble "
        << "data files to be used (required).\n"

        << "\t\t\"ens_file_list\" is an ASCII file containing a list "
        << "of ensemble member file names (required).\n"

        << "\t\t\"-out file\" is the NetCDF output file for the derived "
        << "ensemble products (required).\n"

        << "\t\t\"-config file\" is a GenEnsProdConfig file "
        << "containing the desired configuration settings (required).\n"

        << "\t\t\"-ctrl file\" is the gridded ensemble control data file "
        << "included in mean but excluded from the spread (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n";

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_ens_files(const StringArray & a) {
   ens_files.add(parse_file_list(a));
}

////////////////////////////////////////////////////////////////////////

void set_out_file(const StringArray & a) {
   out_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_config_file(const StringArray & a) {
   config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_ctrl_file(const StringArray & a) {
   ctrl_file = a[0];
}

////////////////////////////////////////////////////////////////////////
