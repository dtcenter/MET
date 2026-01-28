// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   grid_diag.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    10/01/19  Fillmore        New
//   001    07/28/20  Halley Gotway   Updates for #1391.
//   002    03/04/21  Halley Gotway   Bugfix #1694.
//   003    08/20/21  Halley Gotway   Bugfix #1886 for integer overflow.
//   004    07/06/22  Howard Soh      METplus-Internal #19 Rename main to met_main
//   005    10/03/22  Prestopnik      MET #2227 Remove using namespace std and netCDF from header files
//   006    10/26/22  Linden          MET #2232 Refine the Grid-Diag output variable names when specifying two input data sources
//   007    01/07/26  Halley Gotway   MET #3171 Multiple masks and information theory
//
////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <dirent.h>
#include <fstream>
#include <limits.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <netcdf>

#include "main.h"
#include "grid_diag.h"
#include "series_data.h"
#include "series_pdf.h"

#include "vx_statistics.h"
#include "vx_nc_util.h"
#include "vx_regrid.h"
#include "vx_log.h"

#ifdef WITH_PROFILER
#include "ctrack.hpp"
#endif

using namespace std;
using namespace netCDF;

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void setup_diag_info(void);
static void process_series(void);
static void process_hist1d(const vector<DataPlane> &);
static void process_hist2d(const vector<DataPlane> &);
static void process_info_theory(void);
static void setup_nc_file(void);
static ConcatString get_nc_var_str(const VarInfo *, int);
static void write_nc_var_int(const char *, const char *, int);
static void add_var_att_local(NcVar *, const char *, const ConcatString &);
static void write_hist_bins(void);
static void write_hist1d(void);
static void write_hist2d(void);
static void write_info_theory(void);
static void clean_up(void);

static Met2dDataFile *get_mtddf(const StringArray &, const int);

static void usage(int exit_code=1);
static void set_data_files(const StringArray &);
static void set_out_file(const StringArray &);
static void set_config_file(const StringArray &);
static void set_compress(const StringArray &);

////////////////////////////////////////////////////////////////////////

int met_main(int argc, char *argv[]) {

   // Process the command line arguments
   process_command_line(argc, argv);

   // Setup diagnostic info
   setup_diag_info();

   // Process series
   process_series();

   // Setup netcdf output
   setup_nc_file();

   // Write histogram bins
   write_hist_bins();

   // Write 1D variable histograms
   if(conf_info.nc_info.do_hist1d) write_hist1d();

   // Write 2D joint variable histograms
   if(conf_info.nc_info.do_hist2d) write_hist2d();

   // Write information theory output
   if(conf_info.nc_info.do_info_theory) write_info_theory();

   // Write benchmarking metrics
   #ifdef WITH_PROFILER
   ctrack::result_print();
   #endif 

   // Close files and deallocate memory
   clean_up();

   return 0;
}

////////////////////////////////////////////////////////////////////////

static void process_command_line(int argc, char **argv) {
   CommandLine cline;
   ConcatString default_config_file;
   Grid data_grid;

   // Check for zero arguments
   if(argc == 1) usage();

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_data_files,  "-data",    -1);
   cline.add(set_config_file, "-config",   1);
   cline.add(set_out_file,    "-out",      1);
   cline.add(set_compress,    "-compress", 1);

   // Parse the command line
   cline.parse();

   // Check for error, there should be zero arguments left
   if(cline.n() != 0) usage();

   // Check that the required arguments have been set
   if(data_files.empty()) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the data file list must be set using the "
           << "\"-data\" option.\n\n";
      exit(1);
   }
   if(config_file.empty()) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the configuration file must be set using the "
           << "\"-config\" option.\n\n";
      exit(1);
   }
   if(out_file.empty()) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the output NetCDF file must be set using the "
           << "\"-out\" option.\n\n";
      exit(1);
   }

   // Create the default config file name
   default_config_file = replace_path(default_config_filename);

   // List the config files
   mlog << Debug(1)
       << "Default Config File: " << default_config_file << "\n"
       << "User Config File: "    << config_file << "\n";

   // Read the config files
   conf_info.read_config(default_config_file.c_str(),
                         config_file.c_str());

   // Determine the number of data fields
   conf_info.set_n_data();

   // Multiple -data options must match the number of fields
   if(data_files.size() > 1 &&
      data_files.size() != conf_info.get_n_data()) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the number of \"-data\" options ("
           << data_files.size() << ") does not match the number of "
           << "data fields requested (" << conf_info.get_n_data()
           << ")!\n\n";
      exit(1);
   }

   // Process the input data file lists
   for(int i=0; i < data_files.size(); i++) {

      // Parse the data file list
      data_files[i] = parse_file_list(data_files[i]);

      // Set the series length
      if(is_bad_data(n_series)) n_series = data_files[i].n();

      // Make sure n_series does not change
      else if(data_files[i].n() != n_series) {
         mlog << Error << "\nprocess_command_line() -> "
              << "when the \"-data\" option is used multiple times "
              << "the series length must remain constant ("
              << n_series << " != " << data_files[i].n() << ")!\n\n";
         exit(1);
      }

      // Get mtddf
      data_mtddf = get_mtddf(data_files[i], i);

      // Store the input data file types
      file_types.emplace_back(data_mtddf->file_type());

      // Store the grid
      data_grid = data_mtddf->grid();

      // Deallocate memory for data files
      if(data_mtddf) {
         delete data_mtddf;
         data_mtddf = (Met2dDataFile *) nullptr;
      }

   } // end for i

   // Process the configuration
   conf_info.process_config(file_types);

   // Determine the verification grid
   grid = parse_vx_grid(conf_info.data_info[0]->regrid(),
                        &data_grid, &data_grid);

   // The regrid.to_grid option cannot be set to FCST or OBS
   if(conf_info.data_info[0]->regrid().field == FieldType::Fcst ||
      conf_info.data_info[0]->regrid().field == FieldType::Obs) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the \"regrid.to_grid\" configuration option cannot be set to "
           << "FCST or OBS!\nSpecify a named grid, grid specification string, "
           << "or the path to a gridded data file instead.\n\n";
      exit(1);
   }

   // Process masking regions
   conf_info.process_masks(grid);
}

////////////////////////////////////////////////////////////////////////

string get_tool_name() {
   return "grid_diag";
}

////////////////////////////////////////////////////////////////////////

static void setup_diag_info(void) {
   #ifdef WITH_PROFILER
   CTRACK;
   #endif

   // Resize based on the number of variables and masks
   diag_info.resize(conf_info.get_n_data());
   for(auto &info : diag_info) info.resize(conf_info.get_n_mask());

   // Loop over variables
   for(int i_var=0; i_var < conf_info.get_n_data(); i_var++) {

      // Find bin ranges
      const VarInfo *i_data = conf_info.data_info[i_var];
      NumArray range(i_data->range());
      int i_n_bins = i_data->n_bins();
      double var_min = range[0];
      double var_max = range[1];
      double bin_delta = (var_max - var_min) / i_n_bins;

      // Compute bin values
      vector<double> bin_min(i_n_bins);
      vector<double> bin_max(i_n_bins);
      vector<double> bin_mid(i_n_bins);
      for(int i_bin=0; i_bin < i_n_bins; i_bin++) {
         bin_min[i_bin] = var_min + bin_delta * i_bin;
         bin_max[i_bin] = var_min + bin_delta * (i_bin + 1);
         bin_mid[i_bin] = var_min + bin_delta * (i_bin + 0.5);
      }

      // 1D histogram
      mlog << Debug(2)
           << "Initializing " << i_data->magic_str_attr()
           << " histogram with " << i_n_bins << " bins from "
           << var_min << " to " << var_max << ".\n";
      vector<long long> hist1d;
      init_pdf(i_n_bins, hist1d);

      // Keep track of unique output variable names
      if(nc_var_sa.has(i_data->magic_str_attr())) unique_variable_names = false;
      nc_var_sa.add(i_data->magic_str_attr());

      // 2D histograms
      map<int, vector<long long> > hist2d; 
      for(int j_var=i_var+1; j_var < conf_info.get_n_data(); j_var++) {

         const VarInfo *j_data = conf_info.data_info[j_var];

         int j_n_bins = j_data->n_bins();

         mlog << Debug(2)
              << "Initializing " << i_data->magic_str_attr() << "_"
              << j_data->magic_str_attr() << " joint histogram with "
              << i_n_bins << " x " << j_n_bins << " bins.\n";

         hist2d[j_var] = vector<long long>();
         init_joint_pdf(i_n_bins, j_n_bins, hist2d[j_var]);
      }

      // Initialize diagnostic info for each mask
      for(int i_mask=0; i_mask < conf_info.get_n_mask(); i_mask++) {
         diag_info[i_var][i_mask].bin_min   = bin_min;
         diag_info[i_var][i_mask].bin_max   = bin_max;
         diag_info[i_var][i_mask].bin_mid   = bin_mid;
         diag_info[i_var][i_mask].bin_delta = bin_delta;
         diag_info[i_var][i_mask].var_min   = bad_data_double;
         diag_info[i_var][i_mask].var_max   = bad_data_double;
         diag_info[i_var][i_mask].hist1d    = hist1d;
         diag_info[i_var][i_mask].hist2d    = hist2d;
         diag_info[i_var][i_mask].entropy   = 0;
      } // end for i_mask

   } // end for i_var
}

////////////////////////////////////////////////////////////////////////

static void process_series(void) {
   vector<DataPlane> data_dp(conf_info.get_n_data());
   const StringArray *cur_files;
   const GrdFileType *cur_ftype;
   Grid cur_grid;

   // List the lengths of the series options
   mlog << Debug(1)
       << "Processing " << conf_info.get_n_data() << " data fields"
       << " from " << n_series << " input file(s).\n";

   // Loop over the input files
   for(int i_series=0; i_series < n_series; i_series++) {

      // List the lengths of the series options
      mlog << Debug(2)
           << "Processing series entry " << i_series+1 << " of "
           << n_series << ".\n";

      // Read the input data for this series entry
      for(int i_var=0; i_var < conf_info.get_n_data(); i_var++) {

         VarInfo *i_data = conf_info.data_info[i_var];

         // Check for separate data files for each field
         if(data_files.size() > 1) {
            cur_files = &data_files[i_var];
            cur_ftype = &file_types[i_var];
         }
         else {
            cur_files = &data_files[0];
            cur_ftype = &file_types[0];
         }

         mlog << Debug(2)
              << "Reading field " << i_data->magic_str_attr()
              << " data from file: " << (*cur_files)[i_series]
              << "\n";

         get_series_entry(i_series, i_data, *cur_files, *cur_ftype,
                          data_dp[i_var], cur_grid);

         // Regrid, if necessary
         if(!(cur_grid == grid)) {
            mlog << Debug(2)
                 << "Regridding field " << i_data->magic_str_attr()
                 << " to the verification grid using "
                 << i_data->regrid().get_str() << ".\n";
            data_dp[i_var] = met_regrid(data_dp[i_var],
                                        cur_grid, grid,
                                        i_data->regrid());
         }

         // Initialize time ranges
         if(i_series == 0 && i_var == 0) {
            init_beg  = init_end  = data_dp[i_var].init();
            valid_beg = valid_end = data_dp[i_var].valid();
            lead_beg  = lead_end  = data_dp[i_var].lead();
         }
         // Update time ranges
         else {
            if(data_dp[i_var].init() < init_beg) {
               init_beg  = data_dp[i_var].init();
            }
            if(data_dp[i_var].init() > init_end) {
               init_end  = data_dp[i_var].init();
            }
            if(data_dp[i_var].valid() < valid_beg) {
               valid_beg = data_dp[i_var].valid();
            }
            if(data_dp[i_var].valid() > valid_end) {
               valid_end = data_dp[i_var].valid();
            }
            if(data_dp[i_var].lead() < lead_beg) {
               lead_beg  = data_dp[i_var].lead();
            }
            if(data_dp[i_var].lead() > lead_end) {
               lead_end  = data_dp[i_var].lead();
            }
         }
      } // end for i_var

      // Process the 1D histograms
      process_hist1d(data_dp);

      // Process the 2D histograms
      process_hist2d(data_dp);

   } // end for i_series

   // Process information theory
   if(conf_info.nc_info.do_info_theory) process_info_theory();

}
      
////////////////////////////////////////////////////////////////////////

static void process_hist1d(const vector<DataPlane> &data_dp) {

   // Update the 1D histogram counts
   for(int i_var=0; i_var < conf_info.get_n_data(); i_var++) {

      const VarInfo *i_data = conf_info.data_info[i_var];

      // Loop over the masks
      for(int i_mask=0; i_mask < conf_info.get_n_mask(); i_mask++) {

         DiagInfo *i_diag = &diag_info[i_var][i_mask];

         // Apply the mask before updating the data ranges
         DataPlane dp(data_dp[i_var]);
         apply_mask(dp, conf_info.mask_mp[i_mask]);
         double min;
         double max;
         dp.data_range(min, max);
         if(is_bad_data(i_diag->var_min) || min < i_diag->var_min) {
            i_diag->var_min = min;
         }
         if(is_bad_data(i_diag->var_max) || max > i_diag->var_max) {
            i_diag->var_max = max;
         }

         // Update 1D histogram counts
         update_pdf(i_diag->bin_min[0],
                    i_diag->bin_delta,
                    i_diag->hist1d,
                    data_dp[i_var],
                    conf_info.mask_mp[i_mask]);

         mlog << Debug(2)
              << "Processed " << i_data->magic_str_attr()
              << " data over region " << conf_info.mask_name[i_mask]
              << " with range (" << i_diag->var_min << ", "
              << i_diag->var_max << ") into bins with range ("
              << i_data->range()[0] << ", "
              << i_data->range()[1] << ").\n";

         // Compare input data and bin ranges 
         if(i_diag->var_min < i_data->range()[0] ||
            i_diag->var_max > i_data->range()[1]) {
            mlog << Warning << "\nprocess_hist1d() -> "
                 << "the range of the " << i_data->magic_str_attr()
                 << " data over region " << conf_info.mask_name[i_mask]
                 << " (" << i_diag->var_min << ", " << i_diag->var_max
                 << ") falls outside the configuration file range ("
                 << i_data->range()[0] << ", "
                 << i_data->range()[1] << ")!\n\n";
         }
      } // end for i_mask
   } // end for i_var
}

////////////////////////////////////////////////////////////////////////

static void process_hist2d(const vector<DataPlane> &data_dp) {

   // Process the 2D joint histograms
   for(int i_var=0; i_var < conf_info.get_n_data(); i_var++) {

      const VarInfo *i_data = conf_info.data_info[i_var];

      for(int j_var=i_var+1; j_var < conf_info.get_n_data(); j_var++) {

         const VarInfo *j_data = conf_info.data_info[j_var];

         for(int i_mask=0; i_mask < conf_info.get_n_mask(); i_mask++) {

            DiagInfo *i_diag = &diag_info[i_var][i_mask];
            DiagInfo *j_diag = &diag_info[j_var][i_mask];

            // Update 2D histogram counts
            update_joint_pdf(i_data->n_bins(),
                             j_data->n_bins(),
                             i_diag->bin_min[0],
                             j_diag->bin_min[0],
                             i_diag->bin_delta,
                             j_diag->bin_delta,
                             i_diag->hist2d[j_var],
                             data_dp[i_var], data_dp[j_var],
                             conf_info.mask_mp[i_mask]);
         } // end for i_mask
      } // end for j_var
   } // end for i_var
}

////////////////////////////////////////////////////////////////////////

static void process_info_theory() {

   // Compute Shannon entropy for the 1D histograms
   for(int i_var=0; i_var < conf_info.get_n_data(); i_var++) {

      // Loop over the masks
      for(int i_mask=0; i_mask < conf_info.get_n_mask(); i_mask++) {

         DiagInfo *i_diag = &diag_info[i_var][i_mask];

         // Initialize
         i_diag->entropy = 0.0;

         // Sum of histogram 
         long long hist1d_sum = 0;
         for(const auto &x : i_diag->hist1d) hist1d_sum += x;

         // Accumulate entropy for each bin
         for(const auto &x : i_diag->hist1d) {
            auto p_x = (double) x / (double) hist1d_sum;
            if(p_x > 0) i_diag->entropy -= p_x * log2(p_x);
         }

      } // end for i_mask
   } // end for i_var

   // Compute joint entropy and mutual information for the 2D histograms
   for(int i_var=0; i_var < conf_info.get_n_data(); i_var++) {

      const VarInfo *i_data = conf_info.data_info[i_var];

      for(int j_var=i_var+1; j_var < conf_info.get_n_data(); j_var++) {

         const VarInfo *j_data = conf_info.data_info[j_var];

         for(int i_mask=0; i_mask < conf_info.get_n_mask(); i_mask++) {

            DiagInfo *i_diag = &diag_info[i_var][i_mask];

            // Initialize
            i_diag->joint_entropy[j_var] = 0.0;
            i_diag->mutual_information[j_var] = 0.0;

            // 2D histogram sums 
            long long hist2d_ij_sum = 0;
            vector<long long> hist2d_i_sum(i_data->n_bins(), 0);
            vector<long long> hist2d_j_sum(j_data->n_bins(), 0);

            for(int i=0; i<i_data->n_bins(); i++) {
               for(int j=0; j<j_data->n_bins(); j++) {

		  int n = i * j_data->n_bins() + j;

                  // Increment sums
                  hist2d_ij_sum   += i_diag->hist2d[j_var][n];
                  hist2d_i_sum[i] += i_diag->hist2d[j_var][n];
                  hist2d_j_sum[j] += i_diag->hist2d[j_var][n];

               } // end for j
            } // end for i

            // Compute probabilities and acccumulate mutual information
            for(int i=0; i<i_data->n_bins(); i++) {

               auto p_i = (double) hist2d_i_sum[i] / (double) hist2d_ij_sum;

               for(int j=0; j<j_data->n_bins(); j++) {

                  auto p_j = (double) hist2d_j_sum[j] / (double) hist2d_ij_sum;

		  int n = i * j_data->n_bins() + j;

                  auto p_ij = (double) i_diag->hist2d[j_var][n] / (double) hist2d_ij_sum;

                  // Accumulate joint entropy mutual information terms
                  if(p_ij > 0) {
                     i_diag->joint_entropy[j_var] -=
                        p_ij * log2(p_ij);
                     i_diag->mutual_information[j_var] +=
                        p_ij * log2(p_ij/(p_i*p_j));
                  }

               } // end for j
            } // end for i
         } // end for i_mask
      } // end for j_var
   } // end for i_var
}

////////////////////////////////////////////////////////////////////////

static ConcatString get_nc_var_str(const VarInfo *info, int index) {
   ConcatString cs;

   if(!info) return cs;

   // Append the NetCDF name and level
   cs << info->name_attr() << "_" << info->level_attr();

   if(multiple_data_sources && !unique_variable_names) {
      cs << "_VAR" << index;
   }

   return cs;
}

////////////////////////////////////////////////////////////////////////

static void setup_nc_file(void) {

   // Create NetCDF file
   nc_out = open_ncfile(out_file.c_str(), true);

   if(IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_file << "\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, out_file.c_str(), program_name,
                       nullptr, nullptr, conf_info.desc.c_str());

   // Add time range information to the global attributes
   add_att(nc_out, "init_beg",  (string)unix_to_yyyymmdd_hhmmss(init_beg));
   add_att(nc_out, "init_end",  (string)unix_to_yyyymmdd_hhmmss(init_end));
   add_att(nc_out, "valid_beg", (string)unix_to_yyyymmdd_hhmmss(valid_beg));
   add_att(nc_out, "valid_end", (string)unix_to_yyyymmdd_hhmmss(valid_end));
   add_att(nc_out, "lead_beg",  (string)sec_to_hhmmss(lead_beg));
   add_att(nc_out, "lead_end",  (string)sec_to_hhmmss(lead_end));

   // Write the grid size and series length
   write_nc_var_int("grid_size", "number of grid points", grid.nxy());
   write_nc_var_int("n_series", "length of series", n_series);

   // Compression level
   deflate_level = compress_level;
   if(deflate_level < 0) deflate_level = conf_info.conf.nc_compression();

   // Create the mask dimension
   mask_dim = add_dim(nc_out, "mask",
                      (long) conf_info.get_n_mask());

   // Create the mask name variable
   NcVar mask_name_var = add_var(nc_out, "mask_name", ncString, mask_dim, deflate_level);
   add_att(&mask_name_var, "long_name", "Name of masking region");

   // Create the mask size variable
   NcVar mask_size_var = add_var(nc_out, "mask_size", ncInt64, mask_dim, deflate_level);
   add_att(&mask_size_var, "long_name", "Number of mask points");

   // Write the mask names and sizes
   vector<size_t> offsets(1);
   vector<size_t> counts(1);
   for(int i_mask=0; i_mask < conf_info.get_n_mask(); i_mask++) {
      offsets[0] = i_mask;
      counts[0] = 1;
      string mask_name(conf_info.mask_name[i_mask]);
      mask_name_var.putVar(offsets, counts, &mask_name);
      int mask_size = conf_info.mask_mp[i_mask].count();
      mask_size_var.putVar(offsets, counts, &mask_size);
   }
}

////////////////////////////////////////////////////////////////////////

static void write_nc_var_int(const char *var_name,
                             const char *long_name, int n) {

   // Add the variable
   NcVar var = add_var(nc_out, var_name, ncInt64);
   add_att(&var, "long_name", long_name);

   if(!put_nc_data(&var, &n)) {
      mlog << Error << "\nwrite_nc_int() -> "
           << "error writing the \"" << long_name << "\" variable.\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

static void add_var_att_local(NcVar *var, const char *att_name,
                              const ConcatString &att_value) {
   if(att_value.nonempty()) add_att(var, att_name, att_value.c_str());
   else                     add_att(var, att_name, na_str);
}

////////////////////////////////////////////////////////////////////////

static void write_hist_bins(void) {

   for(int i_var=0; i_var < conf_info.get_n_data(); i_var++) {

      const VarInfo *i_data = conf_info.data_info[i_var];
      DiagInfo *i_diag = &diag_info[i_var][0];

      // Define NetCDF variable name
      ConcatString var_str(get_nc_var_str(i_data, i_var+1));

      // Define NetCDF dimensions
      NcDim var_dim = add_dim(nc_out, var_str,
                              (long) i_data->n_bins());
      data_var_dims.emplace_back(var_dim);
      
      // Create NetCDF variable
      ConcatString min_var_name(var_str);
      ConcatString max_var_name(var_str);
      min_var_name.add("_min");
      max_var_name.add("_max");
      NcVar var_min = add_var(nc_out, min_var_name, ncFloat,
                              var_dim, deflate_level);
      NcVar var_max = add_var(nc_out, max_var_name, ncFloat,
                              var_dim, deflate_level);

      // Write a coordinate variable using the bin midpoint
      NcVar var_mid = add_var(nc_out, var_str, ncFloat,
                              var_dim, deflate_level);

      // Add variable attributes
      ConcatString cs;
      cs << cs_erase << "Minimum value of " << var_str << " bin";
      add_var_att_local(&var_min, "long_name", cs);
      add_var_att_local(&var_min, "units", i_data->units_attr());

      cs << cs_erase << "Maximum value of " << var_str << " bin";
      add_var_att_local(&var_max, "long_name", cs);
      add_var_att_local(&var_max, "units", i_data->units_attr());

      cs << cs_erase << "Midpoint value of " << var_str << " bin";
      add_var_att_local(&var_mid, "long_name", cs);
      add_var_att_local(&var_mid, "units", i_data->units_attr());

      // Write bin values for the current variable
      var_min.putVar(i_diag->bin_min.data());
      var_max.putVar(i_diag->bin_max.data());
      var_mid.putVar(i_diag->bin_mid.data());

   } // end for i_var
}

////////////////////////////////////////////////////////////////////////

static void write_hist1d(void) {
   vector<size_t> offsets(2);
   vector<size_t> counts(2);

   // Define and write 1D histograms
   for(int i_var=0; i_var < conf_info.get_n_data(); i_var++) {

      const VarInfo *i_data = conf_info.data_info[i_var];

      // Define NetCDF variable name
      ConcatString var_str(get_nc_var_str(i_data, i_var+1));
      ConcatString var_name("hist_");
      var_name << var_str;

      // Create NetCDF variable
      vector<NcDim> dims(2);
      dims[0] = mask_dim;
      dims[1] = data_var_dims[i_var];
      NcVar var = add_var(nc_out, var_name, ncInt64, dims,
                          deflate_level);

      // Add variable attributes
      ConcatString cs;
      cs << "Histogram of " << var_str << " values";
      add_var_att_local(&var, "long_name", cs);

      // Write 1D histogram for each mask
      for(int i_mask=0; i_mask < conf_info.get_n_mask(); i_mask++) {

         const long long *hist = diag_info[i_var][i_mask].hist1d.data();

         offsets[0] = i_mask;
         offsets[1] = 0;
         counts[0]  = 1;
         counts[1]  = i_data->n_bins();

         var.putVar(offsets, counts, hist);

      } // end for i_mask
   } // end for i_var
}

////////////////////////////////////////////////////////////////////////

static void write_hist2d(void) {
   vector<size_t> offsets(3);
   vector<size_t> counts(3);

   // Define and write 2D joint histograms
   for(int i_var=0; i_var < conf_info.get_n_data(); i_var++) {

      const VarInfo *i_data = conf_info.data_info[i_var];

      for(int j_var=i_var+1; j_var < conf_info.get_n_data(); j_var++) {

         const VarInfo *j_data = conf_info.data_info[j_var];

         // Define NetCDF variable name
         ConcatString var_str;
         var_str << get_nc_var_str(i_data, i_var+1) << "_"
                 << get_nc_var_str(j_data, j_var+1);
	 ConcatString var_name("hist_");
         var_name << var_str;

         // Create NetCDF variable
         vector<NcDim> dims(3);
         dims[0] = mask_dim;
         dims[1] = data_var_dims[i_var];
         dims[2] = data_var_dims[j_var];
         NcVar var = add_var(nc_out, var_name, ncInt64, dims,
                             deflate_level);

         // Add variable attributes
         ConcatString cs;
         cs << "Joint histogram of " << var_str << " values";
         add_var_att_local(&var, "long_name", cs);

         // Write 2D histogram for each mask
         for(int i_mask=0; i_mask < conf_info.get_n_mask(); i_mask++) {

            const long long *hist = diag_info[i_var][i_mask].hist2d[j_var].data();

            offsets[0] = i_mask;
            offsets[1] = 0;
            offsets[2] = 0;
            counts[0]  = 1;
            counts[1]  = i_data->n_bins();
            counts[2]  = j_data->n_bins();

            var.putVar(offsets, counts, hist);

         } // end for i_mask
      } // end for j_var
   } // end for i_var
}

////////////////////////////////////////////////////////////////////////

static void write_info_theory(void) {
   ConcatString units_cs("bits");

   // Write entropy for each 1D histogram
   for(int i_var=0; i_var < conf_info.get_n_data(); i_var++) {

      const VarInfo *i_data = conf_info.data_info[i_var];

      // Define NetCDF variable name
      ConcatString var_str(get_nc_var_str(i_data, i_var+1));
      ConcatString var_name("entropy_");
      var_name << var_str;

      // Create NetCDF variable
      NcVar var = add_var(nc_out, var_name, ncFloat,
                          mask_dim, deflate_level);

      // Add variable attributes
      ConcatString cs;
      cs << "Entropy value for " << var_str;
      add_var_att_local(&var, "long_name", cs);
      add_var_att_local(&var, "units", units_cs);

      // Store the data
      vector<double> data(conf_info.get_n_mask());
      for(int i_mask=0; i_mask < conf_info.get_n_mask(); i_mask++) {
         data[i_mask] = diag_info[i_var][i_mask].entropy;
      }

      // Write the data
      var.putVar(data.data());

   } // end for i_var

   // Write joint entropy and mutual information for each 2D joint histogram
   for(int i_var=0; i_var < conf_info.get_n_data(); i_var++) {

      const VarInfo *i_data = conf_info.data_info[i_var];

      for(int j_var=i_var+1; j_var < conf_info.get_n_data(); j_var++) {

         const VarInfo *j_data = conf_info.data_info[j_var];

         ConcatString var_str;
         var_str << get_nc_var_str(i_data, i_var+1) << "_"
                 << get_nc_var_str(j_data, j_var+1);

         // Define NetCDF variable names
	 ConcatString je_var_name("joint_entropy_");
         je_var_name << var_str;
         ConcatString mi_var_name("mutual_information_");
         mi_var_name << var_str;

         // Create NetCDF variables
         NcVar je_var = add_var(nc_out, je_var_name, ncFloat,
                                mask_dim, deflate_level);
         NcVar mi_var = add_var(nc_out, mi_var_name, ncFloat,
                                mask_dim, deflate_level);

         // Add variable attributes
         ConcatString cs;
         cs << "Joint entropy value for " << var_str;
         add_var_att_local(&je_var, "long_name", cs);
         add_var_att_local(&je_var, "units", units_cs);
         cs << cs_erase << "Mutual information value for " << var_str;
         add_var_att_local(&mi_var, "long_name", cs);
         add_var_att_local(&mi_var, "units", units_cs);

         // Store the data
         vector<double> je_data(conf_info.get_n_mask());
         vector<double> mi_data(conf_info.get_n_mask());
         for(int i_mask=0; i_mask < conf_info.get_n_mask(); i_mask++) {
            je_data[i_mask] = diag_info[i_var][i_mask].joint_entropy[j_var];
            mi_data[i_mask] = diag_info[i_var][i_mask].mutual_information[j_var];
         }

         // Write the data
         je_var.putVar(je_data.data());
         mi_var.putVar(mi_data.data());

      } // end for j_var
   } // end for i_var
}

////////////////////////////////////////////////////////////////////////

static Met2dDataFile *get_mtddf(const StringArray &file_list,
                                const int i_field) {
   Met2dDataFile *mtddf = nullptr;
   Dictionary *dict = nullptr;
   Dictionary i_dict;
   GrdFileType file_type;
   int i;

   // Conf: data.field
   dict = conf_info.conf.lookup_array(conf_key_data_field);

   // Get the i-th data.field entry
   i_dict = parse_conf_i_vx_dict(dict, i_field);

   // Look for file_type in the i-th data.field entry
   file_type = parse_conf_file_type(&i_dict);

   // Find the first file that actually exists
   for(i=0; i < file_list.n(); i++) {
      if(file_exists(file_list[i].c_str())) break;
   }

   // Check for no valid files
   if(i == file_list.n()) {
      mlog << Error << "\nget_mtddf() -> "
           << "no valid data files found!\n\n";
      exit(1);
   }

   // Read first valid file
   if(!(mtddf = mtddf_factory.new_met_2d_data_file(
                   file_list[i].c_str(), file_type))) {
      mlog << Error << "\nget_mtddf() -> "
           << "trouble reading data file \""
           << file_list[i] << "\"\n\n";
      exit(1);
   }

   return mtddf;
}

////////////////////////////////////////////////////////////////////////

static void clean_up(void) {

   // Close the output NetCDF file
   if(nc_out) {

      // List the NetCDF file after it is finished
      mlog << Debug(1) << "Output file: " << out_file << "\n";

      delete nc_out;
      nc_out = (NcFile *) nullptr;
    }

   return;
}

////////////////////////////////////////////////////////////////////////

__attribute__((noreturn)) static void usage(int exit_code) {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: "<< program_name<< "\n"
        << "\t-data  file_1 ... file_n | file_list\n"
        << "\t-out file\n"
        << "\t-config file\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"-data file_1 ... file_n\" is a list of gridded "
        << "data files to be used (required).\n"

        << "\t\t\"-data file_list\" is an ASCII file containing "
        << "a list of gridded data files to be used (required).\n"

        << "\t\t\"-out file\" is the NetCDF output file containing "
        << "computed statistics (required).\n"

        << "\t\t\"-config file\" is a GridDiagConfig file "
        << "containing the configuration settings (required).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level()<< ") (optional).\n\n"

        << "\tNOTE: The \"-data\" option can be used once to read all "
        << "fields from each input file or once for each field to be "
        << "processed.\n\n"

        << flush;

   exit(exit_code);
}

////////////////////////////////////////////////////////////////////////

static void set_data_files(const StringArray & a) {
   data_files.emplace_back(a);
   if(!data_files.empty()) multiple_data_sources = true;
}

////////////////////////////////////////////////////////////////////////

static void set_out_file(const StringArray & a) {
   out_file = a[0];
}

////////////////////////////////////////////////////////////////////////

static void set_config_file(const StringArray & a) {
   config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

static void set_compress(const StringArray & a) {
   compress_level = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////
