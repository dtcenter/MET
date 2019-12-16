// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

#include "vx_statistics.h"
#include "vx_nc_util.h"
#include "vx_regrid.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static void process_command_line_config(int, char **);

static void process_command_line(int, char **);

static Met2dDataFile *get_mtddf(const StringArray &, const GrdFileType);

static void get_series_data(int, VarInfo *, VarInfo *,
                            DataPlane &, DataPlane &);
static void get_series_entry(int, VarInfo *, const StringArray &,
                             const GrdFileType, StringArray &,
                             DataPlane &);
static bool read_single_entry(VarInfo *, const ConcatString &,
                              const GrdFileType, DataPlane &, Grid &);

static void setup_nc_file(const VarInfo *, const VarInfo *);
static void add_nc_var(const ConcatString &, const ConcatString &,
                       const ConcatString &, const ConcatString &,
                       const ConcatString &, double);
static void put_nc_val(int, const ConcatString &, float);

static void set_range(const unixtime &, unixtime &, unixtime &);
static void set_range(const int &, int &, int &);

static void clean_up();

static void usage();
static void set_fcst_files(const StringArray &);
static void set_obs_files(const StringArray &);
static void set_data_files(const StringArray &);
static void set_both_files(const StringArray &);
static void set_out_file(const StringArray &);
static void set_config_file(const StringArray &);
static void set_log_file(const StringArray &);
static void set_verbosity(const StringArray &);
static void set_compress(const StringArray &);

static StringArray parse_file_list(const StringArray &, const GrdFileType);
static void parse_long_names();

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line_config(argc, argv);
   // process_command_line(argc, argv);

   // Close the text files and deallocate memory
   // clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line_config(int argc, char **argv) {
   int i;
   CommandLine cline;
   ConcatString default_config_file;

   // Check for zero arguments
   if(argc == 1) usage();

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_data_files,  "-data",  -1);
   cline.add(set_config_file, "-config", 1);
   cline.add(set_out_file,    "-out",    1);
   cline.add(set_log_file,    "-log",    1);
   cline.add(set_verbosity,   "-v",      1);
   cline.add(set_compress,    "-compress", 1);

   // Parse the command line
   cline.parse();

   // Check for error. There should be zero arguments left.
   if(cline.n() != 0) usage();

   // Check that the required arguments have been set.
   if(data_files.n_elements() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the data file list must be set using the "
           << "\"-data option.\n\n";
      usage();
   }
   if(config_file.length() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the configuration file must be set using the "
           << "\"-config\" option.\n\n";
      usage();
   }
   if(out_file.length() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the output NetCDF file must be set using the "
           << "\"-out\" option.\n\n";
      usage();
   }

   // Create the default config file name
   default_config_file = replace_path(default_config_filename);

   // List the config files
   mlog << Debug(1)
        << "Default Config File: " << default_config_file << "\n"
        << "User Config File: "    << config_file << "\n";

   mlog << Debug(2) << "Read config files.\n";

   // Read the config files
   conf_info.read_config(default_config_file.c_str(), config_file.c_str());

   // Get the data file type from config, if present
   dtype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_data));

   // Parse the data file lists
   data_files = parse_file_list(data_files, dtype);

   // Get mtddf
   data_mtddf = get_mtddf(data_files, dtype);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   int i;
   CommandLine cline;
   ConcatString default_config_file;

   // Check for zero arguments
   if(argc == 1) usage();

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_fcst_files,  "-fcst",  -1);
   cline.add(set_obs_files,   "-obs",   -1);
   cline.add(set_both_files,  "-both",  -1);
   cline.add(set_config_file, "-config", 1);
   cline.add(set_out_file,    "-out",    1);
   cline.add(set_log_file,    "-log",    1);
   cline.add(set_verbosity,   "-v",      1);
   cline.add(set_compress,    "-compress", 1);

   // Parse the command line
   cline.parse();

   // Check for error. There should be zero arguments left.
   if(cline.n() != 0) usage();

   // Check that the required arguments have been set.
   if(fcst_files.n_elements() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the forecast file list must be set using the "
           << "\"-fcst\" or \"-both\" option.\n\n";
      usage();
   }
   if(obs_files.n_elements() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the observation file list must be set using the "
           << "\"-obs\" or \"-both\" option.\n\n";
      usage();
   }
   if(config_file.length() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the configuration file must be set using the "
           << "\"-config\" option.\n\n";
      usage();
   }
   if(out_file.length() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the output NetCDF file must be set using the "
           << "\"-out\" option.\n\n";
      usage();
   }

   // Create the default config file name
   default_config_file = replace_path(default_config_filename);

   // List the config files
   mlog << Debug(1)
        << "Default Config File: " << default_config_file << "\n"
        << "User Config File: "    << config_file << "\n";

   mlog << Debug(2) << "Read config files.\n";

   // Read the config files
   conf_info.read_config(default_config_file.c_str(), config_file.c_str());

   // Get the forecast and observation file types from config, if present
   ftype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_fcst));
   otype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_obs));

   mlog << Debug(2) << "Parse file list.\n";

   // Parse the forecast and observation file lists
   fcst_files = parse_file_list(fcst_files, ftype);
   obs_files  = parse_file_list(obs_files,  otype);

   mlog << Debug(2) << "Get mtddf.\n";

   // Get mtddf
   fcst_mtddf = get_mtddf(fcst_files, ftype);
   obs_mtddf  = get_mtddf(obs_files,  otype);

   // Store the input data file types
   ftype = fcst_mtddf->file_type();
   otype = obs_mtddf->file_type();

   mlog << Debug(2) << "Process configuration.\n";

   // Process the configuration
   conf_info.process_config(ftype, otype);

   // Determine the verification grid
   grid = parse_vx_grid(conf_info.fcst_info[0]->regrid(),
                        &(fcst_mtddf->grid()), &(obs_mtddf->grid()));

   // Process masking regions
   conf_info.process_masks(grid);

   // Set the random number generator and seed value to be used when
   // computing bootstrap confidence intervals
   rng_set(rng_ptr, conf_info.boot_rng.c_str(), conf_info.boot_seed.c_str());

   // List the lengths of the series options
   mlog << Debug(1)
        << "Length of configuration \"fcst.field\" = "
        << conf_info.get_n_fcst() << "\n"
        << "Length of configuration \"obs.field\"  = "
        << conf_info.get_n_obs() << "\n"
        << "Length of forecast file list         = "
        << fcst_files.n_elements() << "\n"
        << "Length of observation file list      = "
        << obs_files.n_elements() << "\n";

   // Determine the length of the series to be analyzed.  Series is
   // defined by the first parameter of length greater than one:
   // - Configuration fcst.field
   // - Configuration obs.field
   // - Forecast file list
   // - Observation file list
   if(conf_info.get_n_fcst() > 1) {
      series_type = SeriesType_Fcst_Conf;
      n_series = conf_info.get_n_fcst();
      mlog << Debug(1)
           << "Series defined by the \"fcst.field\" configuration entry "
           << "of length " << n_series << ".\n";
   }
   else if(conf_info.get_n_obs() > 1) {
      series_type = SeriesType_Obs_Conf;
      n_series = conf_info.get_n_obs();
      mlog << Debug(1)
           << "Series defined by the \"obs.field\" configuration entry "
           << "of length " << n_series << ".\n";
   }
   else if(fcst_files.n_elements() > 1) {
      series_type = SeriesType_Fcst_Files;
      n_series = fcst_files.n_elements();
      mlog << Debug(1)
           << "Series defined by the forecast file list of length "
           << n_series << ".\n";
   }
   else if(obs_files.n_elements() > 1) {
      series_type = SeriesType_Obs_Files;
      n_series = obs_files.n_elements();
      mlog << Debug(1)
           << "Series defined by the observation file list of length "
           << n_series << ".\n";
   }
   else {
      series_type = SeriesType_Fcst_Conf;
      n_series = 1;
      mlog << Debug(1)
           << "The \"fcst.field\" and \"obs.field\" configuration entries "
           << "and the \"-fcst\" and \"-obs\" command line options "
           << "all have length one.\n";
   }

   // If paired, check for consistent settings.
   if(paired) {

      // The number of forecast and observation files must match.
      if(fcst_files.n_elements() != obs_files.n_elements()) {
         mlog << Error << "\nprocess_command_line() -> "
              << "when using the \"-paired\" command line option, the "
              << "number of forecast (" << fcst_files.n_elements()
              << ") and observation (" << obs_files.n_elements()
              << ") files must match.\n\n";
         usage();
      }

      // The number of files must match the series length.
      if(fcst_files.n_elements() != n_series) {
         mlog << Error << "\nprocess_command_line() -> "
              << "when using the \"-paired\" command line option, the "
              << "the file list length (" << fcst_files.n_elements()
              << ") and series length (" << n_series
              << ") must match.\n\n";
         usage();
      }

      // Set the series file names to the input file lists
      for(i=0; i<n_series; i++) {
         found_fcst_files.add(fcst_files[i]);
         found_obs_files.add(obs_files[i]);
      }
   }
   // If not paired, initialize the series file names.
   else {
      for(i=0; i<n_series; i++) {
         found_fcst_files.add("");
         found_obs_files.add("");
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

Met2dDataFile *get_mtddf(const StringArray &file_list, const GrdFileType type) {
   int i;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;

   mlog << "Enter get_mtddf.\n";

   // Find the first file that actually exists
   for(i=0; i<file_list.n_elements(); i++) {
      if(file_exists(file_list[i].c_str())) break;
   }

   // Check for no valid files
   if(i == fcst_files.n_elements()) {
      mlog << Error << "\nTrouble reading forecast files.\n\n";
      exit(1);
   }

   // Read first valid file
   if(!(mtddf = mtddf_factory.new_met_2d_data_file(file_list[i].c_str(), type))) {
      mlog << Error << "\nTrouble reading data file \""
           << file_list[i] << "\"\n\n";
      exit(1);
   }

   return(mtddf);
}

////////////////////////////////////////////////////////////////////////

void get_series_data(int i_series,
                     VarInfo *fcst_info, VarInfo *obs_info,
                     DataPlane &fcst_dp, DataPlane &obs_dp) {

   mlog << Debug(2)
        << "Processing series entry " << i_series + 1 << " of "
        << n_series << ": " << fcst_info->magic_str()
        << " versus " << obs_info->magic_str() << "\n";

   // Switch on the series type
   switch(series_type) {

      case SeriesType_Fcst_Conf:
         get_series_entry(i_series, fcst_info, fcst_files,
                          ftype, found_fcst_files, fcst_dp);
         if(conf_info.get_n_obs() == 1) {
            obs_info->set_valid(fcst_dp.valid());
            mlog << Debug(3)
                 << "Setting the observation valid time search criteria "
                 << "using the forecast valid time of "
                 << unix_to_yyyymmdd_hhmmss(fcst_dp.valid()) << ".\n";
         }
         get_series_entry(i_series, obs_info, obs_files,
                          otype, found_obs_files, obs_dp);
         break;

      case SeriesType_Obs_Conf:
         get_series_entry(i_series, obs_info, obs_files,
                          otype, found_obs_files, obs_dp);
         if(conf_info.get_n_fcst() == 1) {
            fcst_info->set_valid(obs_dp.valid());
            mlog << Debug(3)
                 << "Setting the forecast valid time search criteria "
                 << "using the observation valid time of "
                 << unix_to_yyyymmdd_hhmmss(obs_dp.valid()) << ".\n";
         }
         get_series_entry(i_series, fcst_info, fcst_files,
                          ftype, found_fcst_files, fcst_dp);
         break;

      case SeriesType_Fcst_Files:
         found_fcst_files.set(i_series, fcst_files[i_series]);
         get_series_entry(i_series, fcst_info, fcst_files,
                          ftype, found_fcst_files, fcst_dp);
         if(paired) {
            found_obs_files.set(i_series, obs_files[i_series]);
         }
         else {
            obs_info->set_valid(fcst_dp.valid());
            mlog << Debug(3)
                 << "Setting the observation valid time search criteria "
                 << "using the forecast valid time of "
                 << unix_to_yyyymmdd_hhmmss(fcst_dp.valid()) << ".\n";
         }
         get_series_entry(i_series, obs_info, obs_files,
                          otype, found_obs_files, obs_dp);
         break;

      case SeriesType_Obs_Files:
         found_obs_files.set(i_series, obs_files[i_series]);
         get_series_entry(i_series, obs_info, obs_files,
                          otype, found_obs_files, obs_dp);
         if(paired) {
            found_fcst_files.set(i_series, fcst_files[i_series]);
         }
         else {
            fcst_info->set_valid(obs_dp.valid());
            mlog << Debug(3)
                 << "Setting the forecast valid time search criteria "
                 << "using the observation valid time of "
                 << unix_to_yyyymmdd_hhmmss(obs_dp.valid()) << ".\n";
         }
         get_series_entry(i_series, fcst_info, fcst_files,
                          ftype, found_fcst_files, fcst_dp);
         break;

      default:
         mlog << Error << "\nget_series_data() -> "
              << "unexpected SeriesType value: "
              << series_type << "\n\n";
         exit(1);
         break;
   }

   // Rescale probabilities from [0, 100] to [0, 1]
   if(conf_info.fcst_info[0]->p_flag()) rescale_probability(fcst_dp);
   if(conf_info.obs_info[0]->p_flag())  rescale_probability(obs_dp);

   // Check that non-zero valid times match
   if(fcst_dp.valid() != (unixtime) 0 &&
      obs_dp.valid()  != (unixtime) 0 &&
      fcst_dp.valid() != obs_dp.valid()) {
      mlog << Warning << "\nget_series_data() -> "
           << "Forecast and observation valid times do not match "
           << unix_to_yyyymmdd_hhmmss(fcst_dp.valid()) << " != "
           << unix_to_yyyymmdd_hhmmss(obs_dp.valid()) << " for "
           << fcst_info->magic_str() << " versus "
           << obs_info->magic_str() << ".\n\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void get_series_entry(int i_series, VarInfo *info,
                      const StringArray &search_files,
                      const GrdFileType type,
                      StringArray &found_files, DataPlane &dp) {
   int i, j;
   bool found = false;
   Grid cur_grid;

   // Initialize
   dp.clear();

   // If not already found, search for a matching file
   if(found_files[i_series].length() == 0) {

      // Loop through the file list
      for(i=0; i<search_files.n_elements(); i++) {

         // Start the search with the value of i_series
         j = (i_series + i) % search_files.n_elements();

         mlog << Debug(3)
              << "Searching file " << search_files[j] << "\n";

         // Read gridded data from the current file.
         found = read_single_entry(info, search_files[j], type,
                                   dp, cur_grid);

         // If found, store the file and break out of the loop
         if(found) {
            found_files.set(i_series, search_files[j]);
            break;
         }
      }

      // Check to see if a match was found
      if(!found) {
         mlog << Error << "\nget_series_entry() -> "
              << "Could not find data for " << info->magic_str()
              << " in file list:\n";
         for(i=0; i<search_files.n_elements(); i++)
            mlog << Error << "   " << search_files[i] << "\n";
         mlog << Error << "\n";
         exit(1);
      }
   }

   // If not already done, read the data
   if(dp.nx() == 0 && dp.ny() == 0) {
      found = read_single_entry(info, found_files[i_series], type,
                                dp, cur_grid);
   }

   // Check for a match
   if(found) {
      mlog << Debug(2)
           << "Found data for " << info->magic_str()
           << " in file: " << found_files[i_series] << "\n";

      // Regrid, if necessary
      if(!(cur_grid == grid)) {

         // Check if regridding is disabled
         if(!info->regrid().enable) {
            mlog << Error << "\nget_series_entry() -> "
                 << "The grid of the current series entry does not "
                 << "match the verification grid and regridding is "
                 << "disabled:\n" << cur_grid.serialize()
                 << " !=\n" << grid.serialize()
                 << "\nSpecify regridding logic in the config file "
                 << "\"regrid\" section.\n\n";
            exit(1);
         }

         mlog << Debug(1)
              << "Regridding field " << info->magic_str()
              << " to the verification grid.\n";
         dp = met_regrid(dp, cur_grid, grid,
                         info->regrid());
      }
   }
   // No match here results in a warning.
   else {
      mlog << Warning << "\nget_series_entry() -> "
           << "No match found for " << info->magic_str()
           << " in file: " << found_files[i_series] << "\n\n";

      // Return a field of bad data values
      dp.set_size(grid.nx(), grid.ny());
      dp.set_constant(bad_data_double);
      dp.set_init((unixtime) 0);
      dp.set_valid((unixtime) 0);
      dp.set_lead(bad_data_double);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool read_single_entry(VarInfo *info, const ConcatString &cur_file,
                       const GrdFileType type, DataPlane &dp,
                       Grid &cur_grid) {
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;
   bool found = false;

   // Check that the file exists
   if(!file_exists(cur_file.c_str())) {
      mlog << Warning << "\nread_single_entry() -> "
           << "File does not exist: " << cur_file << "\n\n";
      return(false);
   }

   // Open the data file
   mtddf = mtddf_factory.new_met_2d_data_file(cur_file.c_str(), type);

   // Attempt to read the gridded data from the current file
   found = mtddf->data_plane(*info, dp);

   // Store the current grid
   if(found) cur_grid = mtddf->grid();

   // Close the data file
   delete mtddf; mtddf = (Met2dDataFile *) 0;

   return(found);
}

////////////////////////////////////////////////////////////////////////

void setup_nc_file(const VarInfo *fcst_info, const VarInfo *obs_info) {

   // Create a new NetCDF file and open it
   nc_out = open_ncfile(out_file.c_str(), true);

   if(IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_file << "\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, out_file.c_str(), program_name,
                       conf_info.model.c_str(), conf_info.obtype.c_str(), conf_info.desc.c_str());
   add_att(nc_out, "mask_grid",  (conf_info.mask_grid_name.nonempty() ?
                                  (string)conf_info.mask_grid_name : na_str));
   add_att(nc_out, "mask_poly",  (conf_info.mask_poly_name.nonempty() ?
                                  (string)conf_info.mask_poly_name : na_str));
   add_att(nc_out, "fcst_var",   (string)fcst_info->name());
   add_att(nc_out, "fcst_lev",   (string)fcst_info->level_name());
   add_att(nc_out, "fcst_units", (string)fcst_info->units());
   add_att(nc_out, "obs_var",    (string)obs_info->name());
   add_att(nc_out, "obs_lev",    (string)obs_info->level_name());
   add_att(nc_out, "obs_units",  (string)obs_info->units());

   // Add the projection information
   write_netcdf_proj(nc_out, grid);

   // Define Dimensions
   lat_dim = add_dim(nc_out, "lat", (long) grid.ny());
   lon_dim = add_dim(nc_out, "lon", (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(nc_out, &lat_dim, &lon_dim, grid);

   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = conf_info.get_compression_level();

   // Add the series length variable
   //NcVar * var = nc_out->add_var("n_series", ncInt);
   NcVar var = add_var(nc_out, "n_series", ncInt, deflate_level);
   add_att(&var, "long_name", "length of series");

   if(!put_nc_data(&var, &n_series)) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "error writing the series length variable.\n\n";
            exit(1);
   }

   // Load the long name descriptions for each column name
   parse_long_names();

   return;
}

////////////////////////////////////////////////////////////////////////

void add_nc_var(const ConcatString &var_name,
                const ConcatString &name,
                const ConcatString &long_name,
                const ConcatString &fcst_thresh,
                const ConcatString &obs_thresh,
                double alpha) {
   NcVarData d;

   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = conf_info.get_compression_level();

   // Add a new variable to the NetCDF file
   NcVar var = add_var(nc_out, (string)var_name, ncFloat, lat_dim, lon_dim, deflate_level);
   d.var = new NcVar(var);

   // Add variable attributes
   add_att(d.var, "_FillValue", bad_data_float);
   if(name.length() > 0)        add_att(d.var, "name", (string)name);
   if(long_name.length() > 0)   add_att(d.var, "long_name", (string)long_name);
   if(fcst_thresh.length() > 0) add_att(d.var, "fcst_thresh", (string)fcst_thresh);
   if(obs_thresh.length() > 0)  add_att(d.var, "obs_thresh", (string)obs_thresh);
   if(!is_bad_data(alpha))      add_att(d.var, "alpha", alpha);

   // Store the new NcVarData object in the map
   stat_data[var_name] = d;

   return;
}

////////////////////////////////////////////////////////////////////////

void put_nc_val(int n, const ConcatString &var_name, float v) {
   int x, y;

   // Determine x,y location
   DefaultTO.one_to_two(grid.nx(), grid.ny(), n, x, y);

   // Check for key in the map
   if(stat_data.count(var_name) == 0) {
      mlog << Error << "\nput_nc_val() -> "
           << "variable name \"" << var_name
           << "\" does not exist in the map.\n\n";
      exit(1);
   }

   // Get the NetCDF variable to be written
   NcVar *var = stat_data[var_name].var;

   long offsets[2];
   long lengths[2];
   offsets[0] = y;
   offsets[1] = x;
   lengths[0] = 1;
   lengths[1] = 1;

   // Store the current value
   if(!put_nc_data(var, &v, lengths, offsets)) {
      mlog << Error << "\nput_nc_val() -> "
           << "error writing to variable " << var_name
           << " for point (" << x << ", " << y << ").\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void set_range(const unixtime &t, unixtime &beg, unixtime &end) {

   if(t == (unixtime) 0) return;

   beg = (beg == (unixtime) 0 || t < beg ? t : beg);
   end = (end == (unixtime) 0 || t > end ? t : end);

   return;
}

////////////////////////////////////////////////////////////////////////

void set_range(const int &t, int &beg, int &end) {

   if(is_bad_data(t)) return;

   beg = (is_bad_data(beg) || t < beg ? t : beg);
   end = (is_bad_data(end) || t > end ? t : end);

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {

   // Deallocate NetCDF variable for each map entry
   map<ConcatString, NcVarData>::const_iterator it;
   for(it=stat_data.begin(); it!=stat_data.end(); it++) {
      if(it->second.var) { delete it->second.var; }
   }

   // Close the output NetCDF file
   if(nc_out) {

      // List the NetCDF file after it is finished
      mlog << Debug(1) << "Output file: " << out_file << "\n";

      delete nc_out;
      nc_out = (NcFile *) 0;
   }

   // Deallocate memory for data files
   if(fcst_mtddf) { delete fcst_mtddf; fcst_mtddf = (Met2dDataFile *) 0; }
   if(obs_mtddf)  { delete obs_mtddf;  obs_mtddf  = (Met2dDataFile *) 0; }

   // Deallocate memory for the random number generator
   rng_free(rng_ptr);

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
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
        << "containing the desired configuration settings (required).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t\"-compress level\" overrides the compression level of NetCDF variable ("
        << conf_info.get_compression_level() << ") (optional).\n\n" << flush;

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_fcst_files(const StringArray & a) {
   fcst_files = a;
}

////////////////////////////////////////////////////////////////////////

void set_obs_files(const StringArray & a) {
   obs_files = a;
}

////////////////////////////////////////////////////////////////////////

void set_data_files(const StringArray & a) {
   data_files = a;
}

////////////////////////////////////////////////////////////////////////

void set_both_files(const StringArray & a) {
   set_fcst_files(a);
   set_obs_files(a);
}

////////////////////////////////////////////////////////////////////////

void set_paired(const StringArray & a) {
   paired = true;
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

StringArray parse_file_list(const StringArray & a, const GrdFileType type) {
   int i;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;
   StringArray list;

   mlog << "Enter parse_file_list.\n";

   // Check for empty list
   if(a.n_elements() == 0) {
      mlog << Error << "\nparse_file_list() -> "
           << "empty list!\n\n";
      exit(1);
   }

   mlog << Debug(2) << a[0] << "\n";

   // Attempt to read the first file as a gridded data file
   mtddf = mtddf_factory.new_met_2d_data_file(a[0].c_str(), type);

   mlog << Debug(2) << "Read OK in parse_file_list.\n";

   // If the read was successful, store the list of gridded files.
   // Otherwise, process entries as ASCII files.
   if(mtddf)                            list.add(a);
   else for(i=0; i<a.n_elements(); i++) list = parse_ascii_file_list(a[0].c_str());

   // Cleanup
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }

   return(list);
}

////////////////////////////////////////////////////////////////////////

void parse_long_names() {
   ifstream f_in;
   ConcatString line, key;
   StringArray sa;
   ConcatString file_name = replace_path(stat_long_name_file);

   mlog << Debug(1)
        << "Reading stat column descriptions: " << file_name << "\n";

   // Open the data file
   f_in.open(file_name.c_str());
   if(!f_in) {
      mlog << Error << "\nparse_long_names() -> "
           << "can't open the ASCII file \"" << file_name
           << "\" for reading\n\n";
      exit(1);
   }

   // Read the lines in the file
   while(line.read_line(f_in)) {

      // Parse the line
      sa = line.split("\"");

      // Skip any lines without enough elements
      if(sa.n_elements() < 2) continue;

      // Store the description
      key = sa[0];
      key.ws_strip();
      stat_long_name[key] = sa[1];
   }

   // Close the input file
   f_in.close();

   return;
}

////////////////////////////////////////////////////////////////////////
