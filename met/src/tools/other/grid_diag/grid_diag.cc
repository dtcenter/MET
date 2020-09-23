// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

#include "grid_diag.h"
#include "series_data.h"
#include "series_pdf.h"

#include "vx_statistics.h"
#include "vx_nc_util.h"
#include "vx_regrid.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_series(void);

static void setup_histograms(void);
static void setup_joint_histograms(void);
static void setup_nc_file(void);
static void write_nc_var_int(const char *, const char *, int);
static void add_var_att_local(NcVar *, const char *, const ConcatString);
static void write_histograms(void);
static void write_joint_histograms(void);
static void clean_up();

static Met2dDataFile *get_mtddf(const StringArray &, const int);

static void usage();
static void set_data_files(const StringArray &);
static void set_out_file(const StringArray &);
static void set_config_file(const StringArray &);
static void set_log_file(const StringArray &);
static void set_verbosity(const StringArray &);
static void set_compress(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Setup variable histograms
   setup_histograms();

   // Setup joint variable histograms
   setup_joint_histograms();

   // Process series
   process_series();

   // Setup netcdf output
   setup_nc_file();

   // Write variable histograms
   write_histograms();

   // Write joint variable histograms
   write_joint_histograms();

   // Close files and deallocate memory
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
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
   cline.add(set_log_file,    "-log",      1);
   cline.add(set_verbosity,   "-v",        1);
   cline.add(set_compress,    "-compress", 1);

   // Parse the command line
   cline.parse();

   // Check for error, there should be zero arguments left
   if(cline.n() != 0) usage();

   // Check that the required arguments have been set
   if(data_files.size() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the data file list must be set using the "
           << "\"-data\" option.\n\n";
      exit(1);
   }
   if(config_file.length() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the configuration file must be set using the "
           << "\"-config\" option.\n\n";
      exit(1);
   }
   if(out_file.length() == 0) {
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
   for(int i=0; i<data_files.size(); i++) {

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
      file_types.push_back(data_mtddf->file_type());

      // Store the grid
      data_grid = data_mtddf->grid();

      // Deallocate memory for data files
      if(data_mtddf) {
         delete data_mtddf;
         data_mtddf = (Met2dDataFile *) 0;
      }

   } // end for i

   // Process the configuration
   conf_info.process_config(file_types);

   // Determine the verification grid
   grid = parse_vx_grid(conf_info.data_info[0]->regrid(),
                        &data_grid, &data_grid);

   // Process masking regions
   conf_info.process_masks(grid);
}

////////////////////////////////////////////////////////////////////////

void process_series(void) {
   DataPlane data_dp[conf_info.get_n_data()];
   double min, max;
   StringArray *cur_files;
   GrdFileType *cur_ftype;
   Grid cur_grid;

   // List the lengths of the series options
   mlog << Debug(1)
       << "Processing " << conf_info.get_n_data() << " data fields"
       << " from " << n_series << " input file(s).\n";

   // Loop over the input files
   for(int i_series=0; i_series<n_series; i_series++) {

      // List the lengths of the series options
      mlog << Debug(2)
           << "Processing series entry " << i_series+1 << " of "
           << n_series << ".\n";

      // Process the 1d histograms
      for(int i_var=0; i_var<conf_info.get_n_data(); i_var++) {

        VarInfo *data_info = conf_info.data_info[i_var];

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
             << "Reading field " << data_info->magic_str()
             << " data from file: " << (*cur_files)[i_series]
             << "\n";

        get_series_entry(i_series, data_info, *cur_files, *cur_ftype,
                         data_dp[i_var], cur_grid);

        // Regrid, if necessary
        if(!(cur_grid == grid)) {
           mlog << Debug(2)
                << "Regridding field " << data_info->magic_str()
                << " to the verification grid.\n";
           data_dp[i_var] = met_regrid(data_dp[i_var],
                                       cur_grid, grid,
                                       data_info->regrid());
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

        // Apply the mask before updating the data ranges
        apply_mask(data_dp[i_var], conf_info.mask_area);

        // Update the range of the data values
        data_dp[i_var].data_range(min, max);
        if(is_bad_data(var_mins[i_var]) || min < var_mins[i_var]) {
           var_mins[i_var] = min;
        }
        if(is_bad_data(var_maxs[i_var]) || max < var_maxs[i_var]) {
           var_maxs[i_var] = max;
        }

        // Update partial sums
        update_pdf(bin_mins[data_info->magic_str()][0],
                   bin_deltas[data_info->magic_str()],
                   histograms[data_info->magic_str()],
                   data_dp[i_var], conf_info.mask_area);
     } // end for i_var

     // Process the 2d joint histograms
     for(int i_var=0; i_var<conf_info.get_n_data(); i_var++) {

        VarInfo *data_info = conf_info.data_info[i_var];

        for(int j_var=i_var+1; j_var<conf_info.get_n_data(); j_var++) {

           VarInfo *joint_info = conf_info.data_info[j_var];

           ConcatString joint_str = data_info->magic_str();
           joint_str.add("_");
           joint_str.add(joint_info->magic_str());

           // Update joint partial sums
           update_joint_pdf(data_info->n_bins(),
                            joint_info->n_bins(),
                            bin_mins[data_info->magic_str()][0],
                            bin_mins[joint_info->magic_str()][0],
                            bin_deltas[data_info->magic_str()],
                            bin_deltas[joint_info->magic_str()],
                            joint_histograms[joint_str],
                            data_dp[i_var], data_dp[j_var],
                            conf_info.mask_area);
       } // end for j_var
     } // end for i_var
   } // end for i_series

   // Report the ranges of values for each field
   for(int i_var=0; i_var<conf_info.get_n_data(); i_var++) {

      VarInfo *data_info = conf_info.data_info[i_var];

      mlog << Debug(2)
           << "Processed " << data_info->magic_str()
           << " data with range (" << var_mins[i_var] << ", "
           << var_maxs[i_var] << ") into bins with range ("
           << data_info->range()[0] << ", "
           << data_info->range()[1] << ").\n";

      if(var_mins[i_var] < data_info->range()[0] ||
         var_maxs[i_var] > data_info->range()[1]) {
         mlog << Warning << "\nprocess_series() -> "
              << "the range of the " << data_info->magic_str()
              << " data (" << var_mins[i_var] << ", " << var_maxs[i_var]
              << ") falls outside the configuration file range ("
              << data_info->range()[0] << ", "
              << data_info->range()[1] << ")!\n\n";
      }
   } // end for i_var

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_histograms(void) {

   for(int i_var=0; i_var<conf_info.get_n_data(); i_var++) {

      VarInfo *data_info = conf_info.data_info[i_var];

      // Initialize variable min and max values
      var_mins.push_back(bad_data_double);
      var_maxs.push_back(bad_data_double);

      // Find bin ranges
      NumArray range = data_info->range();
      int n_bins = data_info->n_bins();
      double min = range[0];
      double max = range[1];
      double delta = (max - min) / n_bins;

      // Compute bin values
      vector<double> bin_min;
      vector<double> bin_max;
      vector<double> bin_mid;
      bin_min.clear();
      bin_max.clear();
      bin_mid.clear();
      for(int k=0; k<n_bins; k++) {
         bin_min.push_back(min + delta * k);
         bin_max.push_back(min + delta * (k + 1));
         bin_mid.push_back(min + delta * (k + 0.5));
      }

      bin_mins[data_info->magic_str()] = bin_min;
      bin_maxs[data_info->magic_str()] = bin_max;
      bin_mids[data_info->magic_str()] = bin_mid;
      bin_deltas[data_info->magic_str()] = delta;

      // Initialize histograms
      mlog << Debug(2)
           << "Initializing " << data_info->magic_str()
           << " histogram with " << n_bins << " bins from "
           << min << " to " << max << ".\n";
      histograms[data_info->magic_str()] = vector<int>();
      init_pdf(n_bins, histograms[data_info->magic_str()]);
   } // for i_var
}

////////////////////////////////////////////////////////////////////////

void setup_joint_histograms(void) {

   for(int i_var=0; i_var<conf_info.get_n_data(); i_var++) {

      VarInfo *data_info = conf_info.data_info[i_var];
      int n_bins = data_info->n_bins();

      for(int j_var=i_var+1; j_var<conf_info.get_n_data(); j_var++) {

         VarInfo *joint_info = conf_info.data_info[j_var];
         int n_joint_bins = joint_info->n_bins();

         ConcatString joint_str = data_info->magic_str();
         joint_str.add("_");
         joint_str.add(joint_info->magic_str());
         mlog << Debug(2)
              << "Initializing " << joint_str << " joint histogram with "
              << n_bins << " x " << n_joint_bins << " bins.\n";
         joint_histograms[joint_str] = vector<int>();

         init_joint_pdf(n_bins, n_joint_bins,
                        joint_histograms[joint_str]);
      } // end  for j_var
   } // end for i_var
}

////////////////////////////////////////////////////////////////////////

void setup_nc_file(void) {
   int n;
   ConcatString cs;

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
                       NULL, NULL, conf_info.desc.c_str());
   add_att(nc_out, "mask_grid", (conf_info.mask_grid_name.nonempty() ?
                                (string)conf_info.mask_grid_name :
                                na_str));
   add_att(nc_out, "mask_poly", (conf_info.mask_poly_name.nonempty() ?
                                (string)conf_info.mask_poly_name :
                                na_str));

   // Add time range information to the global attributes
   add_att(nc_out, "init_beg",  (string)unix_to_yyyymmdd_hhmmss(init_beg));
   add_att(nc_out, "init_end",  (string)unix_to_yyyymmdd_hhmmss(init_end));
   add_att(nc_out, "valid_beg", (string)unix_to_yyyymmdd_hhmmss(valid_beg));
   add_att(nc_out, "valid_end", (string)unix_to_yyyymmdd_hhmmss(valid_end));
   add_att(nc_out, "lead_beg",  (string)sec_to_hhmmss(lead_beg));
   add_att(nc_out, "lead_end",  (string)sec_to_hhmmss(lead_end));

   // Write the grid size, mask size, and series length
   write_nc_var_int("grid_size", "number of grid points", grid.nxy());
   write_nc_var_int("mask_size", "number of mask points", conf_info.mask_area.count());
   write_nc_var_int("n_series", "length of series", n_series);

   // Compression level
   int deflate_level = compress_level;
   if(deflate_level < 0) deflate_level = conf_info.conf.nc_compression();

   for(int i_var=0; i_var < conf_info.get_n_data(); i_var++) {

      VarInfo *data_info = conf_info.data_info[i_var];

      // Set variable NetCDF name
      ConcatString var_name = data_info->name_attr();
      var_name.add("_");
      var_name.add(data_info->level_attr());

      // Define histogram dimensions
      NcDim var_dim = add_dim(nc_out, var_name,
                              (long) data_info->n_bins());
      data_var_dims.push_back(var_dim);

      // Define histogram bins
      ConcatString var_min_name = var_name;
      ConcatString var_max_name = var_name;
      ConcatString var_mid_name = var_name;
      var_min_name.add("_min");
      var_max_name.add("_max");
      var_mid_name.add("_mid");
      NcVar var_min = add_var(nc_out, var_min_name, ncFloat, var_dim,
                              deflate_level);
      NcVar var_max = add_var(nc_out, var_max_name, ncFloat, var_dim,
                              deflate_level);
      NcVar var_mid = add_var(nc_out, var_mid_name, ncFloat, var_dim,
                              deflate_level);

      // Add variable attributes
      cs << cs_erase << "Minimum value of " << var_name << " bin";
      add_var_att_local(&var_min, "long_name", cs);
      add_var_att_local(&var_min, "units", data_info->units_attr());

      cs << cs_erase << "Maximum value of " << var_name << " bin";
      add_var_att_local(&var_max, "long_name", cs);
      add_var_att_local(&var_max, "units", data_info->units_attr());

      cs << cs_erase << "Midpoint value of " << var_name << " bin";
      add_var_att_local(&var_mid, "long_name", cs);
      add_var_att_local(&var_mid, "units", data_info->units_attr());

      // Write bin values
      var_min.putVar(bin_mins[data_info->magic_str()].data());
      var_max.putVar(bin_maxs[data_info->magic_str()].data());
      var_mid.putVar(bin_mids[data_info->magic_str()].data());
   }

   // Define histograms
   for(int i_var=0; i_var < conf_info.get_n_data(); i_var++) {

      VarInfo *data_info = conf_info.data_info[i_var];

      // Set variable NetCDF name
      ConcatString var_name = data_info->name_attr();
      var_name.add("_");
      var_name.add(data_info->level_attr());

      ConcatString hist_name("hist_");
      hist_name.add(var_name);
      NcDim var_dim = data_var_dims[i_var];
      NcVar hist_var = add_var(nc_out, hist_name, ncInt, var_dim,
                               deflate_level);
      hist_vars.push_back(hist_var);

      // Add variable attributes
      cs << cs_erase << "Histogram of " << var_name << " values";
      add_var_att_local(&hist_var, "long_name", cs);
   }

   // Define joint histograms
   for(int i_var=0; i_var < conf_info.get_n_data(); i_var++) {

      VarInfo *data_info = conf_info.data_info[i_var];

      for(int j_var=i_var+1; j_var<conf_info.get_n_data(); j_var++) {

         VarInfo *joint_info = conf_info.data_info[j_var];

         ConcatString hist_name("hist_");
         hist_name.add(data_info->name_attr());
         hist_name.add("_");
         hist_name.add(data_info->level_attr());
         hist_name.add("_");
         hist_name.add(joint_info->name_attr());
         hist_name.add("_");
         hist_name.add(joint_info->level_attr());

         NcDim var_dim = data_var_dims[i_var];
         NcDim joint_dim = data_var_dims[j_var];
         vector<NcDim> dims;
         dims.clear();
         dims.push_back(var_dim);
         dims.push_back(joint_dim);

         NcVar hist_var = add_var(nc_out, hist_name, ncInt, dims,
                                  deflate_level);
         joint_hist_vars.push_back(hist_var);

         // Add variable attributes
         cs << cs_erase
            << "Joint histogram of " << hist_name << " values";
         add_var_att_local(&hist_var, "long_name", cs);
      } // end  for j_var
   } // end for i_var
}

////////////////////////////////////////////////////////////////////////

void write_nc_var_int(const char *var_name, const char *long_name,
                      int n) {

   // Add the variable
   NcVar var = add_var(nc_out, var_name, ncInt);
   add_att(&var, "long_name", long_name);

   if(!put_nc_data(&var, &n)) {
      mlog << Error << "\nwrite_nc_int() -> "
           << "error writing the \"" << long_name << "\" variable.\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

void add_var_att_local(NcVar *var, const char *att_name,
                       const ConcatString att_value) {
   if(att_value.nonempty()) add_att(var, att_name, att_value.c_str());
   else                     add_att(var, att_name, na_str);
}

////////////////////////////////////////////////////////////////////////

void write_histograms(void) {

   for(int i_var=0; i_var < conf_info.get_n_data(); i_var++) {

      VarInfo *data_info = conf_info.data_info[i_var];
      NcVar hist_var = hist_vars[i_var];

      int *hist = histograms[data_info->magic_str()].data();

      hist_var.putVar(hist);
   }
}

////////////////////////////////////////////////////////////////////////

void write_joint_histograms(void) {
   vector<size_t> offsets;
   vector<size_t> counts;

   int i_hist=0;
   for(int i_var=0; i_var<conf_info.get_n_data(); i_var++) {

      VarInfo *data_info = conf_info.data_info[i_var];

      for(int j_var=i_var+1; j_var<conf_info.get_n_data(); j_var++) {

         VarInfo *joint_info = conf_info.data_info[j_var];

         ConcatString joint_str = data_info->magic_str();
         joint_str.add("_");
         joint_str.add(joint_info->magic_str());

         int *hist = joint_histograms[joint_str].data();

         offsets.clear();
         counts.clear();
         offsets.push_back(0);
         offsets.push_back(0);
         counts.push_back(data_info->n_bins());
         counts.push_back(joint_info->n_bins());

         NcVar hist_var = joint_hist_vars[i_hist];
         hist_var.putVar(offsets, counts, hist);

         i_hist++;
      } // end  for j_var
   } // end for i_var
}

////////////////////////////////////////////////////////////////////////

Met2dDataFile *get_mtddf(const StringArray &file_list,
                       const int i_field) {
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;
   Dictionary *dict = (Dictionary *) 0;
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
   for(i=0; i<file_list.n(); i++) {
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

   return(mtddf);
}

////////////////////////////////////////////////////////////////////////

void clean_up() {

   // Close the output NetCDF file
   if(nc_out) {

      // List the NetCDF file after it is finished
      mlog << Debug(1) << "Output file: " << out_file << "\n";

      delete nc_out;
      nc_out = (NcFile *) 0;
    }

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: "<< program_name<< "\n"
        << "\t-data  file_1 ... file_n | data_file_list\n"
        << "\t-out file\n"
        << "\t-config file\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"-data file_1 ... file_n\" are the gridded "
        << "data files to be used (required).\n"

        << "\t\t\"-data data_file_list\" is an ASCII file containing "
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

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_data_files(const StringArray & a) {
   data_files.push_back(a);
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

void set_log_file(const StringArray & a) {
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a) {
   mlog.set_verbosity_level(atoi(a[0].c_str()));
}

////////////////////////////////////////////////////////////////////////

void set_compress(const StringArray & a) {
   compress_level = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////
