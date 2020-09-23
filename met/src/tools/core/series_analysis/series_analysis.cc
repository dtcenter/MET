// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   series_analysis.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    12/10/12  Halley Gotway  New
//   001    05/27/14  Halley Gotway  Add EIQR and MAD to CNT line type.
//   002    11/14/14  Halley Gotway  Pass the obtype entry from the
//                    from the config file to the output file.
//   003    02/25/15  Halley Gotway  Add automated regridding.
//   004    08/04/15  Halley Gotway  Add conditional continuous
//                    verification.
//   005    09/21/15  Halley Gotway  Add climatology and SAL1L2 output.
//   006    04/20/16  Halley Gotway  Add -paired command line option.
//   007    05/15/17  Prestopnikk P  Add shape for regrid.
//   008    10/06/17  Halley Gotway  Add RMSFA and RMSOA stats.
//   009    10/14/19  Halley Gotway  Add support for climo distribution
//                    percentile thresholds.
//   010    12/11/19  Halley Gotway  Reorganize logic to support the use
//                    of python embedding.
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

#include "series_analysis.h"

#include "vx_statistics.h"
#include "vx_nc_util.h"
#include "vx_regrid.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_grid        (const Grid &, const Grid &);

static Met2dDataFile *get_mtddf(const StringArray &,
                                const GrdFileType);
static bool           file_is_ok(const ConcatString &,
                                 const GrdFileType);

static void get_series_data(int, VarInfo *, VarInfo *,
                            DataPlane &, DataPlane &);
static void get_series_entry(int, VarInfo *, const StringArray &,
                             const GrdFileType, StringArray &,
                             DataPlane &, Grid &);
static bool read_single_entry(VarInfo *, const ConcatString &,
                              const GrdFileType, DataPlane &, Grid &);

static void process_scores();

static void do_cts   (int, const PairDataPoint *);
static void do_mcts  (int, const PairDataPoint *);
static void do_cnt   (int, const PairDataPoint *);
static void do_sl1l2 (int, const PairDataPoint *);
static void do_pct   (int, const PairDataPoint *);

static void store_stat_fho  (int, const ConcatString &, const CTSInfo &);
static void store_stat_ctc  (int, const ConcatString &, const CTSInfo &);
static void store_stat_cts  (int, const ConcatString &, const CTSInfo &);
static void store_stat_mctc (int, const ConcatString &, const MCTSInfo &);
static void store_stat_mcts (int, const ConcatString &, const MCTSInfo &);
static void store_stat_cnt  (int, const ConcatString &, const CNTInfo &);
static void store_stat_sl1l2(int, const ConcatString &, const SL1L2Info &);
static void store_stat_pct  (int, const ConcatString &, const PCTInfo &);
static void store_stat_pstd (int, const ConcatString &, const PCTInfo &);
static void store_stat_pjc  (int, const ConcatString &, const PCTInfo &);
static void store_stat_prc  (int, const ConcatString &, const PCTInfo &);

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
static void set_both_files(const StringArray &);
static void set_paired(const StringArray &);
static void set_out_file(const StringArray &);
static void set_config_file(const StringArray &);
static void set_log_file(const StringArray &);
static void set_verbosity(const StringArray &);
static void set_compress(const StringArray &);

static void parse_long_names();

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Compute the scores and write them out
   process_scores();

   // Close the text files and deallocate memory
   clean_up();

   return(0);
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
   cline.add(set_paired,      "-paired", 0);
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
   if(fcst_files.n() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the forecast file list must be set using the "
           << "\"-fcst\" or \"-both\" option.\n\n";
      usage();
   }
   if(obs_files.n() == 0) {
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

   // Parse the forecast and observation file lists
   fcst_files = parse_file_list(fcst_files);
   obs_files  = parse_file_list(obs_files);

   // Read the config files
   conf_info.read_config(default_config_file.c_str(), config_file.c_str());

   // Get the forecast and observation file types from config, if present
   ftype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_fcst));
   otype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_obs));

   // Get mtddf
   fcst_mtddf = get_mtddf(fcst_files, ftype);
   obs_mtddf  = get_mtddf(obs_files,  otype);

   // Store the input data file types
   ftype = fcst_mtddf->file_type();
   otype = obs_mtddf->file_type();

   // Process the configuration
   conf_info.process_config(ftype, otype);

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
        << fcst_files.n() << "\n"
        << "Length of observation file list      = "
        << obs_files.n() << "\n";

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
   else if(fcst_files.n() > 1) {
      series_type = SeriesType_Fcst_Files;
      n_series = fcst_files.n();
      mlog << Debug(1)
           << "Series defined by the forecast file list of length "
           << n_series << ".\n";
   }
   else if(obs_files.n() > 1) {
      series_type = SeriesType_Obs_Files;
      n_series = obs_files.n();
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
      if(fcst_files.n() != obs_files.n()) {
         mlog << Error << "\nprocess_command_line() -> "
              << "when using the \"-paired\" command line option, the "
              << "number of forecast (" << fcst_files.n()
              << ") and observation (" << obs_files.n()
              << ") files must match.\n\n";
         usage();
      }

      // The number of files must match the series length.
      if(fcst_files.n() != n_series) {
         mlog << Error << "\nprocess_command_line() -> "
              << "when using the \"-paired\" command line option, the "
              << "the file list length (" << fcst_files.n()
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

void process_grid(const Grid &fcst_grid, const Grid &obs_grid) {

   // Determine the verification grid
   grid = parse_vx_grid(conf_info.fcst_info[0]->regrid(),
                        &fcst_grid, &obs_grid);
   nxy  = grid.nx() * grid.ny();

   // Process masking regions
   conf_info.process_masks(grid);

   // Compute the number of reads required
   n_reads = nint(ceil((double) nxy / conf_info.block_size));

   mlog << Debug(2)
        << "Computing statistics using a block size of "
        << conf_info.block_size << ", requiring " << n_reads
        << " pass(es) through the " << grid.nx() << " x "
        << grid.ny() << " grid.\n";

   // Print a warning for too many passes through the data
   if(n_reads > 4) {
      mlog << Warning
           << "\nA block size of " << conf_info.block_size << " for a "
           << grid.nx() << " x " << grid.ny() << " grid requires "
           << n_reads << " passes through the data which will be slow.\n"
           << "Consider increasing \"block_size\" in the configuration "
           << "file based on available memory.\n\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

Met2dDataFile *get_mtddf(const StringArray &file_list,
                         const GrdFileType type) {
   int i;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;

   // Find the first file that actually exists
   for(i=0; i<file_list.n(); i++) {
      if(file_is_ok(file_list[i], type)) break;
   }

   // Check for no valid files
   if(i == file_list.n()) {
      mlog << Error << "\nTrouble reading input data files.\n\n";
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

bool file_is_ok(const ConcatString &file_name, const GrdFileType t) {
   return(file_exists(file_name.c_str()) || is_python_grdfiletype(t));
}

////////////////////////////////////////////////////////////////////////

void get_series_data(int i_series,
                     VarInfo *fcst_info, VarInfo *obs_info,
                     DataPlane &fcst_dp, DataPlane &obs_dp) {
   Grid fcst_grid, obs_grid;

   mlog << Debug(2)
        << "Processing series entry " << i_series + 1 << " of "
        << n_series << ": " << fcst_info->magic_str()
        << " versus " << obs_info->magic_str() << "\n";

   // Switch on the series type
   switch(series_type) {

      case SeriesType_Fcst_Conf:
         get_series_entry(i_series, fcst_info, fcst_files,
                          ftype, found_fcst_files, fcst_dp, fcst_grid);
         if(conf_info.get_n_obs() == 1) {
            obs_info->set_valid(fcst_dp.valid());
            mlog << Debug(3)
                 << "Setting the observation valid time search criteria "
                 << "using the forecast valid time of "
                 << unix_to_yyyymmdd_hhmmss(fcst_dp.valid()) << ".\n";
         }
         get_series_entry(i_series, obs_info, obs_files,
                          otype, found_obs_files, obs_dp, obs_grid);
         break;

      case SeriesType_Obs_Conf:
         get_series_entry(i_series, obs_info, obs_files,
                          otype, found_obs_files, obs_dp, obs_grid);
         if(conf_info.get_n_fcst() == 1) {
            fcst_info->set_valid(obs_dp.valid());
            mlog << Debug(3)
                 << "Setting the forecast valid time search criteria "
                 << "using the observation valid time of "
                 << unix_to_yyyymmdd_hhmmss(obs_dp.valid()) << ".\n";
         }
         get_series_entry(i_series, fcst_info, fcst_files,
                          ftype, found_fcst_files, fcst_dp, fcst_grid);
         break;

      case SeriesType_Fcst_Files:
         found_fcst_files.set(i_series, fcst_files[i_series]);
         get_series_entry(i_series, fcst_info, fcst_files,
                          ftype, found_fcst_files, fcst_dp, fcst_grid);
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
                          otype, found_obs_files, obs_dp, obs_grid);
         break;

      case SeriesType_Obs_Files:
         found_obs_files.set(i_series, obs_files[i_series]);
         get_series_entry(i_series, obs_info, obs_files,
                          otype, found_obs_files, obs_dp, obs_grid);
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
                          ftype, found_fcst_files, fcst_dp, fcst_grid);
         break;

      default:
         mlog << Error << "\nget_series_data() -> "
              << "unexpected SeriesType value: "
              << series_type << "\n\n";
         exit(1);
         break;
   }

   // Setup the verification grid
   if(nxy == 0) process_grid(fcst_grid, obs_grid);

   // Regrid the forecast, if necessary
   if(!(fcst_grid == grid)) {
      if(!fcst_info->regrid().enable) {
         mlog << Error << "\nget_series_data() -> "
              << "The grid of the current series entry does not "
              << "match the verification grid and regridding is "
              << "disabled:\n" << fcst_grid.serialize()
              << " !=\n" << grid.serialize()
              << "\nSpecify regridding logic in the config file "
              << "\"regrid\" section.\n\n";
         exit(1);
      }

      mlog << Debug(1)
           << "Regridding field " << fcst_info->magic_str()
           << " to the verification grid.\n";
      fcst_dp = met_regrid(fcst_dp, fcst_grid, grid,
                           fcst_info->regrid());
   }

   // Regrid the observation, if necessary
   if(!(obs_grid == grid)) {
      if(!obs_info->regrid().enable) {
         mlog << Error << "\nget_series_data() -> "
              << "The grid of the current series entry does not "
              << "match the verification grid and regridding is "
              << "disabled:\n" << obs_grid.serialize()
              << " !=\n" << grid.serialize()
              << "\nSpecify regridding logic in the config file "
              << "\"regrid\" section.\n\n";
         exit(1);
      }

      mlog << Debug(1)
           << "Regridding field " << obs_info->magic_str()
           << " to the verification grid.\n";
      obs_dp = met_regrid(obs_dp, obs_grid, grid,
                          obs_info->regrid());
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
                      StringArray &found_files, DataPlane &dp,
                      Grid &cur_grid) {
   int i, j;
   bool found = false;

   // Initialize
   dp.clear();

   // If not already found, search for a matching file
   if(found_files[i_series].length() == 0) {

      // Loop through the file list
      for(i=0; i<search_files.n(); i++) {

         // Start the search with the value of i_series
         j = (i_series + i) % search_files.n();

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
         for(i=0; i<search_files.n(); i++)
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
   if(!file_is_ok(cur_file, type)) {
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

void process_scores() {
   int i, x, y, i_read, i_series, i_point, i_fcst;
   VarInfo *fcst_info = (VarInfo *) 0;
   VarInfo *obs_info  = (VarInfo *) 0;
   PairDataPoint *pd_ptr = (PairDataPoint *) 0;
   DataPlane fcst_dp, obs_dp;

   // Climatology mean and standard deviation
   DataPlane cmn_dp, csd_dp;
   bool cmn_flag, csd_flag;

   // Number of points skipped due to valid data threshold
   int n_skip_zero = 0;
   int n_skip_pos  = 0;

   // Allocate space to store the pairs for each grid point
   pd_ptr = new PairDataPoint [conf_info.block_size];
   for(i=0; i<conf_info.block_size; i++) pd_ptr[i].extend(n_series);

   // Loop over the data reads
   for(i_read=0; i_read<n_reads; i_read++) {

      // Initialize PairDataPoint objects
      for(i=0; i<conf_info.block_size; i++) pd_ptr[i].erase();

      // Starting grid point
      i_point = i_read*conf_info.block_size;

      // Loop over the series variable
      for(i_series=0; i_series<n_series; i_series++) {

         // Get the index for the forecast and climo VarInfo objects
         i_fcst = (conf_info.get_n_fcst() > 1 ? i_series : 0);

         // Store the current VarInfo objects
         fcst_info = conf_info.fcst_info[i_fcst];
         obs_info  = (conf_info.get_n_obs() > 1 ?
                      conf_info.obs_info[i_series] :
                      conf_info.obs_info[0]);

         // Retrieve the data planes for the current series entry
         get_series_data(i_series, fcst_info, obs_info, fcst_dp, obs_dp);

         if(i_series == 0) {
            mlog << Debug(2)
                 << "Processing data pass number " << i_read + 1 << " of "
                 << n_reads << " for grid points " << i_point + 1 << " to "
                 << min(i_point + conf_info.block_size, nxy) << ".\n";
         }

         // Read climatology data for the current series entry
         cmn_dp = read_climo_data_plane(
                  conf_info.conf.lookup_array(conf_key_climo_mean_field, false),
                  i_fcst, fcst_dp.valid(), grid);
         csd_dp = read_climo_data_plane(
                  conf_info.conf.lookup_array(conf_key_climo_stdev_field, false),
                  i_fcst, fcst_dp.valid(), grid);

         cmn_flag = (cmn_dp.nx() == fcst_dp.nx() && cmn_dp.ny() == fcst_dp.ny());
         csd_flag = (csd_dp.nx() == fcst_dp.nx() && csd_dp.ny() == fcst_dp.ny());

         mlog << Debug(3)
           << "Found " << (cmn_flag ? 0 : 1)
           << " climatology mean and " << (csd_flag == 0 ? 0 : 1)
           << " climatology standard deviation field(s) for forecast "
           << fcst_info->magic_str() << ".\n";

         // Setup the output NetCDF file on the first pass
         if(nc_out == (NcFile *) 0) setup_nc_file(fcst_info, obs_info);

         // Update timing info
         set_range(fcst_dp.init(),  fcst_init_beg,  fcst_init_end);
         set_range(fcst_dp.valid(), fcst_valid_beg, fcst_valid_end);
         set_range(fcst_dp.lead(),  fcst_lead_beg,  fcst_lead_end);
         set_range(obs_dp.init(),   obs_init_beg,   obs_init_end);
         set_range(obs_dp.valid(),  obs_valid_beg,  obs_valid_end);
         set_range(obs_dp.lead(),   obs_lead_beg,   obs_lead_end);

         // Store matched pairs for each grid point
         for(i=0; i<conf_info.block_size && (i_point+i)<nxy; i++) {

            // Convert n to x, y
            DefaultTO.one_to_two(grid.nx(), grid.ny(), i_point+i, x, y);

            // Skip points outside the mask and bad data
            if(!conf_info.mask_area(x, y)              ||
               is_bad_data(fcst_dp(x, y))              ||
               is_bad_data(obs_dp(x,y))                ||
               (cmn_flag && is_bad_data(cmn_dp(x, y))) ||
               (csd_flag && is_bad_data(csd_dp(x, y)))) continue;

            pd_ptr[i].add_grid_pair(fcst_dp(x, y), obs_dp(x, y),
                         (cmn_flag ? cmn_dp(x, y) : bad_data_double),
                         (csd_flag ? csd_dp(x, y) : bad_data_double),
                         default_grid_weight);

         } // end for i

      } // end for i_series

      // Compute statistics for each grid point in the block
      for(i=0; i<conf_info.block_size && (i_point+i)<nxy; i++) {

         // Determine x,y location
         DefaultTO.one_to_two(grid.nx(), grid.ny(), i_point+i, x, y);

         // Check for the required number of matched pairs
         if(pd_ptr[i].f_na.n()/(double) n_series < conf_info.vld_data_thresh) {
            mlog << Debug(4)
                 << "[" << i+1 << " of " << conf_info.block_size
                 << "] Skipping point (" << x << ", " << y << ") with "
                 << pd_ptr[i].f_na.n() << " matched pairs.\n";

            // Keep track of the number of points skipped
            if(pd_ptr[i].f_na.n() == 0) n_skip_zero++;
            else                        n_skip_pos++;

            continue;
         }
         else {
            mlog << Debug(4)
                 << "[" << i+1 << " of " << conf_info.block_size
                 << "] Processing point (" << x << ", " << y << ") with "
                 << pd_ptr[i].n_obs << " matched pairs.\n";
         }

         // Compute contingency table counts and statistics
         if(!conf_info.fcst_info[0]->is_prob() &&
            (conf_info.output_stats[stat_fho].n() +
             conf_info.output_stats[stat_ctc].n() +
             conf_info.output_stats[stat_cts].n()) > 0) {
            do_cts(i_point+i, &pd_ptr[i]);
         }

         // Compute multi-category contingency table counts and statistics
         if(!conf_info.fcst_info[0]->is_prob() &&
            (conf_info.output_stats[stat_mctc].n() +
             conf_info.output_stats[stat_mcts].n()) > 0) {
            do_mcts(i_point+i, &pd_ptr[i]);
         }

         // Compute continuous statistics
         if(!conf_info.fcst_info[0]->is_prob() &&
            conf_info.output_stats[stat_cnt].n() > 0) {
            do_cnt(i_point+i, &pd_ptr[i]);
         }

         // Compute partial sums
         if(!conf_info.fcst_info[0]->is_prob() &&
            (conf_info.output_stats[stat_sl1l2].n()  > 0 ||
             conf_info.output_stats[stat_sal1l2].n() > 0)) {
            do_sl1l2(i_point+i, &pd_ptr[i]);
         }

         // Compute probabilistics counts and statistics
         if(conf_info.fcst_info[0]->is_prob() &&
            (conf_info.output_stats[stat_pct].n() +
             conf_info.output_stats[stat_pstd].n() +
             conf_info.output_stats[stat_pjc].n() +
             conf_info.output_stats[stat_prc].n()) > 0) {
            do_pct(i_point+i, &pd_ptr[i]);
         }
      } // end for i

      // Erase the data
      for(i=0; i<conf_info.block_size; i++) {
         pd_ptr[i].f_na.erase();
         pd_ptr[i].o_na.erase();
         pd_ptr[i].cmn_na.erase();
         pd_ptr[i].csd_na.erase();
      }

   } // end for i_read

   // Add time range information to the global NetCDF attributes
   add_att(nc_out, "fcst_init_beg",  (string)unix_to_yyyymmdd_hhmmss(fcst_init_beg));
   add_att(nc_out, "fcst_init_end",  (string)unix_to_yyyymmdd_hhmmss(fcst_init_end));
   add_att(nc_out, "fcst_valid_beg", (string)unix_to_yyyymmdd_hhmmss(fcst_valid_beg));
   add_att(nc_out, "fcst_valid_end", (string)unix_to_yyyymmdd_hhmmss(fcst_valid_end));
   add_att(nc_out, "fcst_lead_beg",  (string)sec_to_hhmmss(fcst_lead_beg));
   add_att(nc_out, "fcst_lead_end",  (string)sec_to_hhmmss(fcst_lead_end));
   add_att(nc_out, "obs_init_beg",   (string)unix_to_yyyymmdd_hhmmss(obs_init_beg));
   add_att(nc_out, "obs_init_end",   (string)unix_to_yyyymmdd_hhmmss(obs_init_end));
   add_att(nc_out, "obs_valid_beg",  (string)unix_to_yyyymmdd_hhmmss(obs_valid_beg));
   add_att(nc_out, "obs_valid_end",  (string)unix_to_yyyymmdd_hhmmss(obs_valid_end));
   add_att(nc_out, "obs_lead_beg",   (string)sec_to_hhmmss(obs_lead_beg));
   add_att(nc_out, "obs_lead_end",   (string)sec_to_hhmmss(obs_lead_end));

   // Clean up
   if(pd_ptr) { delete [] pd_ptr; pd_ptr = (PairDataPoint *) 0; }

   // Print summary counts
   mlog << Debug(2)
        << "Finished processing statistics for "
        << nxy - n_skip_zero - n_skip_pos << " of " << nxy
        << " grid points.\n"
        << "Skipped " << n_skip_zero << " of " << nxy
        << " points with no valid data.\n"
        << "Skipped " << n_skip_pos << " of " << nxy
        << " points that did not meet the valid data threshold.\n";

   // Print config file suggestions about missing data
   if(n_skip_pos > 0 && conf_info.vld_data_thresh == 1.0) {
      mlog << Debug(2)
           << "Some points skipped due to missing data:\n"
           << "Consider decreasing \"vld_thresh\" in the config file "
           << "to include more points.\n"
           << "Consider requesting \"TOTAL\" from \"output_stats\" "
           << "in the config file to see the valid data counts.\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_cts(int n, const PairDataPoint *pd_ptr) {
   int i, j;

   mlog << Debug(4) << "Computing Categorical Statistics.\n";

   // Allocate objects to store categorical statistics
   int n_cts = conf_info.fcat_ta.n();
   CTSInfo *cts_info = new CTSInfo [n_cts];

   // Setup CTSInfo objects
   for(i=0; i<n_cts; i++) {
      cts_info[i].fthresh = conf_info.fcat_ta[i];
      cts_info[i].othresh = conf_info.ocat_ta[i];

      cts_info[i].allocate_n_alpha(conf_info.ci_alpha.n());
      for(j=0; j<conf_info.ci_alpha.n(); j++) {
         cts_info[i].alpha[j] = conf_info.ci_alpha[j];
      }
   }

   // Compute the counts, stats, normal confidence intervals, and
   // bootstrap confidence intervals
   if(conf_info.boot_interval == BootIntervalType_BCA) {
      compute_cts_stats_ci_bca(rng_ptr, *pd_ptr,
         conf_info.n_boot_rep,
         cts_info, n_cts, true,
         conf_info.rank_corr_flag, conf_info.tmp_dir.c_str());
   }
   else {
      compute_cts_stats_ci_perc(rng_ptr, *pd_ptr,
         conf_info.n_boot_rep, conf_info.boot_rep_prop,
         cts_info, n_cts, true,
         conf_info.rank_corr_flag, conf_info.tmp_dir.c_str());
   }

   // Loop over the categorical thresholds
   for(i=0; i<n_cts; i++) {

      // Add statistic value for each possible FHO column
      for(j=0; j<conf_info.output_stats[stat_fho].n(); j++) {
         store_stat_fho(n, conf_info.output_stats[stat_fho][j],
                        cts_info[i]);
      }

      // Add statistic value for each possible CTC column
      for(j=0; j<conf_info.output_stats[stat_ctc].n(); j++) {
         store_stat_ctc(n, conf_info.output_stats[stat_ctc][j],
                        cts_info[i]);
      }

      // Add statistic value for each possible CTS column
      for(j=0; j<conf_info.output_stats[stat_cts].n(); j++) {
         store_stat_cts(n, conf_info.output_stats[stat_cts][j],
                        cts_info[i]);
      }
   } // end for i

   // Deallocate memory
   if(cts_info) { delete [] cts_info; cts_info = (CTSInfo *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_mcts(int n, const PairDataPoint *pd_ptr) {
   int i;

   mlog << Debug(4) << "Computing Multi-Category Statistics.\n";

   // Object to store multi-category statistics
   MCTSInfo mcts_info;

   // Setup the MCTSInfo object
   mcts_info.cts.set_size(conf_info.fcat_ta.n() + 1);
   mcts_info.set_fthresh(conf_info.fcat_ta);
   mcts_info.set_othresh(conf_info.ocat_ta);

   mcts_info.allocate_n_alpha(conf_info.ci_alpha.n());
   for(i=0; i<conf_info.ci_alpha.n(); i++) {
      mcts_info.alpha[i] = conf_info.ci_alpha[i];
   }

   // Compute the counts, stats, normal confidence intervals, and
   // bootstrap confidence intervals
   if(conf_info.boot_interval == BootIntervalType_BCA) {
      compute_mcts_stats_ci_bca(rng_ptr, *pd_ptr,
         conf_info.n_boot_rep,
         mcts_info, true,
         conf_info.rank_corr_flag, conf_info.tmp_dir.c_str());
   }
   else {
      compute_mcts_stats_ci_perc(rng_ptr, *pd_ptr,
         conf_info.n_boot_rep, conf_info.boot_rep_prop,
         mcts_info, true,
         conf_info.rank_corr_flag, conf_info.tmp_dir.c_str());
   }

   // Add statistic value for each possible MCTC column
   for(i=0; i<conf_info.output_stats[stat_mctc].n(); i++) {
      store_stat_mctc(n, conf_info.output_stats[stat_mctc][i],
                      mcts_info);
   }

   // Add statistic value for each possible MCTS column
   for(i=0; i<conf_info.output_stats[stat_mcts].n(); i++) {
      store_stat_mcts(n, conf_info.output_stats[stat_mcts][i],
                      mcts_info);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_cnt(int n, const PairDataPoint *pd_ptr) {
   int i, j;
   CNTInfo cnt_info;
   PairDataPoint pd;

   mlog << Debug(4) << "Computing Continuous Statistics.\n";

   // Process each filtering threshold
   for(i=0; i<conf_info.fcnt_ta.n(); i++) {

      // Initialize
      cnt_info.clear();

      // Store thresholds
      cnt_info.fthresh = conf_info.fcnt_ta[i];
      cnt_info.othresh = conf_info.ocnt_ta[i];
      cnt_info.logic   = conf_info.cnt_logic;

      // Setup the CNTInfo alpha values
      cnt_info.allocate_n_alpha(conf_info.ci_alpha.n());
      for(j=0; j<conf_info.ci_alpha.n(); j++) {
         cnt_info.alpha[j] = conf_info.ci_alpha[j];
      }

      // Apply continuous filtering thresholds to subset pairs
      pd = subset_pairs(*pd_ptr, cnt_info.fthresh, cnt_info.othresh,
                        cnt_info.logic);

      // Check for no matched pairs to process
      if(pd.n_obs == 0) continue;

      // Compute the stats, normal confidence intervals, and
      // bootstrap confidence intervals
      int precip_flag = (conf_info.fcst_info[0]->is_precipitation() &&
                         conf_info.obs_info[0]->is_precipitation());

      if(conf_info.boot_interval == BootIntervalType_BCA) {
         compute_cnt_stats_ci_bca(rng_ptr, pd,
            precip_flag, conf_info.rank_corr_flag,
            conf_info.n_boot_rep,
            cnt_info, conf_info.tmp_dir.c_str());
      }
      else {
         compute_cnt_stats_ci_perc(rng_ptr, pd,
            precip_flag, conf_info.rank_corr_flag,
            conf_info.n_boot_rep, conf_info.boot_rep_prop,
            cnt_info, conf_info.tmp_dir.c_str());
      }

      // Add statistic value for each possible CNT column
      for(j=0; j<conf_info.output_stats[stat_cnt].n(); j++) {
         store_stat_cnt(n, conf_info.output_stats[stat_cnt][j],
                        cnt_info);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_sl1l2(int n, const PairDataPoint *pd_ptr) {
   int i, j;
   SL1L2Info s_info;

   mlog << Debug(4) << "Computing Scalar Partial Sums.\n";

   // Loop over the continuous thresholds and compute scalar partial sums
   for(i=0; i<conf_info.fcnt_ta.n(); i++) {

      // Store thresholds
      s_info.fthresh = conf_info.fcnt_ta[i];
      s_info.othresh = conf_info.ocnt_ta[i];
      s_info.logic   = conf_info.cnt_logic;

      // Compute partial sums
      s_info.set(*pd_ptr);

      // Add statistic value for each possible SL1L2 column
      for(j=0; j<conf_info.output_stats[stat_sl1l2].n(); j++) {
         store_stat_sl1l2(n, conf_info.output_stats[stat_sl1l2][j], s_info);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_pct(int n, const PairDataPoint *pd_ptr) {
   int i, j;

   mlog << Debug(4) << "Computing Probabilistic Statistics.\n";

   // Object to store probabilistic statistics
   PCTInfo pct_info;

   // Setup the PCTInfo object
   pct_info.fthresh = conf_info.fcat_ta;
   pct_info.allocate_n_alpha(conf_info.ci_alpha.n());

   for(i=0; i<conf_info.ci_alpha.n(); i++) {
      pct_info.alpha[i] = conf_info.ci_alpha[i];
   }

   // Compute PCTInfo for each observation threshold
   for(i=0; i<conf_info.ocat_ta.n(); i++) {

      // Set the current observation threshold
      pct_info.othresh = conf_info.ocat_ta[i];

      // Compute the probabilistic counts and statistics
      compute_pctinfo(*pd_ptr, true, pct_info);

      // Add statistic value for each possible PCT column
      for(j=0; j<conf_info.output_stats[stat_pct].n(); j++) {
         store_stat_pct(n, conf_info.output_stats[stat_pct][j],
                        pct_info);
      }

      // Add statistic value for each possible PSTD column
      for(j=0; j<conf_info.output_stats[stat_pstd].n(); j++) {
         store_stat_pstd(n, conf_info.output_stats[stat_pstd][j],
                         pct_info);
      }

      // Add statistic value for each possible PJC column
      for(j=0; j<conf_info.output_stats[stat_pjc].n(); j++) {
         store_stat_pjc(n, conf_info.output_stats[stat_pjc][j],
                        pct_info);
      }

      // Add statistic value for each possible PRC column
      for(j=0; j<conf_info.output_stats[stat_prc].n(); j++) {
         store_stat_prc(n, conf_info.output_stats[stat_prc][j],
                        pct_info);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_fho(int n, const ConcatString &col,
                    const CTSInfo &cts_info) {
   double v;
   ConcatString lty_stat, var_name;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Get the column value
        if(c == "TOTAL")  { v = (double) cts_info.cts.n(); }
   else if(c == "F_RATE") { v = cts_info.cts.f_rate();     }
   else if(c == "H_RATE") { v = cts_info.cts.h_rate();     }
   else if(c == "O_RATE") { v = cts_info.cts.o_rate();     }
   else {
     mlog << Error << "\nstore_stat_fho() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Construct the NetCDF variable name
   var_name << cs_erase << "series_fho_" << c;

   // Append threshold information
   if(cts_info.fthresh == cts_info.othresh) {
      var_name << "_" << cts_info.fthresh.get_abbr_str();
   }
   else {
      var_name << "_fcst" << cts_info.fthresh.get_abbr_str()
               << "_obs" << cts_info.othresh.get_abbr_str();
   }

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      lty_stat << "FHO_" << c;

      // Add new map entry
      add_nc_var(var_name, c, stat_long_name[lty_stat],
                 cts_info.fthresh.get_str(),
                 cts_info.othresh.get_str(),
                 bad_data_double);
   }

   // Store the statistic value
   put_nc_val(n, var_name, (float) v);

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_ctc(int n, const ConcatString &col,
                    const CTSInfo &cts_info) {
   int v;
   ConcatString lty_stat, var_name;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Get the column value
        if(c == "TOTAL") { v = cts_info.cts.n();     }
   else if(c == "FY_OY") { v = cts_info.cts.fy_oy(); }
   else if(c == "FY_ON") { v = cts_info.cts.fy_on(); }
   else if(c == "FN_OY") { v = cts_info.cts.fn_oy(); }
   else if(c == "FN_ON") { v = cts_info.cts.fn_on(); }
   else {
     mlog << Error << "\nstore_stat_ctc() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Construct the NetCDF variable name
   var_name << cs_erase << "series_ctc_" << c;

   // Append threshold information
   if(cts_info.fthresh == cts_info.othresh) {
      var_name << "_" << cts_info.fthresh.get_abbr_str();
   }
   else {
      var_name << "_fcst" << cts_info.fthresh.get_abbr_str()
               << "_obs" << cts_info.othresh.get_abbr_str();
   }

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      lty_stat << "CTC_" << c;

      // Add new map entry
      add_nc_var(var_name, c, stat_long_name[lty_stat],
                 cts_info.fthresh.get_str(),
                 cts_info.othresh.get_str(),
                 bad_data_double);
   }

   // Store the statistic value
   put_nc_val(n, var_name, (float) v);

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_cts(int n, const ConcatString &col,
                    const CTSInfo &cts_info) {
   int i;
   double v;
   ConcatString lty_stat, var_name;
   int n_ci = 1;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Check for columns with normal or bootstrap confidence limits
   if(strstr(c.c_str(), "_NC") || strstr(c.c_str(), "_BC")) n_ci = cts_info.n_alpha;

   // Loop over the alpha values, if necessary
   for(i=0; i<n_ci; i++) {

      // Get the column value
           if(c == "TOTAL")     { v = (double) cts_info.cts.n(); }
      else if(c == "BASER")     { v = cts_info.baser.v;          }
      else if(c == "BASER_NCL") { v = cts_info.baser.v_ncl[i];   }
      else if(c == "BASER_NCU") { v = cts_info.baser.v_ncu[i];   }
      else if(c == "BASER_BCL") { v = cts_info.baser.v_bcl[i];   }
      else if(c == "BASER_BCU") { v = cts_info.baser.v_bcu[i];   }
      else if(c == "FMEAN")     { v = cts_info.fmean.v;          }
      else if(c == "FMEAN_NCL") { v = cts_info.fmean.v_ncl[i];   }
      else if(c == "FMEAN_NCU") { v = cts_info.fmean.v_ncu[i];   }
      else if(c == "FMEAN_BCL") { v = cts_info.fmean.v_bcl[i];   }
      else if(c == "FMEAN_BCU") { v = cts_info.fmean.v_bcu[i];   }
      else if(c == "ACC")       { v = cts_info.acc.v;            }
      else if(c == "ACC_NCL")   { v = cts_info.acc.v_ncl[i];     }
      else if(c == "ACC_NCU")   { v = cts_info.acc.v_ncu[i];     }
      else if(c == "ACC_BCL")   { v = cts_info.acc.v_bcl[i];     }
      else if(c == "ACC_BCU")   { v = cts_info.acc.v_bcu[i];     }
      else if(c == "FBIAS")     { v = cts_info.fbias.v;          }
      else if(c == "FBIAS_BCL") { v = cts_info.fbias.v_bcl[i];   }
      else if(c == "FBIAS_BCU") { v = cts_info.fbias.v_bcu[i];   }
      else if(c == "PODY")      { v = cts_info.pody.v;           }
      else if(c == "PODY_NCL")  { v = cts_info.pody.v_ncl[i];    }
      else if(c == "PODY_NCU")  { v = cts_info.pody.v_ncu[i];    }
      else if(c == "PODY_BCL")  { v = cts_info.pody.v_bcl[i];    }
      else if(c == "PODY_BCU")  { v = cts_info.pody.v_bcu[i];    }
      else if(c == "PODN")      { v = cts_info.podn.v;           }
      else if(c == "PODN_NCL")  { v = cts_info.podn.v_ncl[i];    }
      else if(c == "PODN_NCU")  { v = cts_info.podn.v_ncu[i];    }
      else if(c == "PODN_BCL")  { v = cts_info.podn.v_bcl[i];    }
      else if(c == "PODN_BCU")  { v = cts_info.podn.v_bcu[i];    }
      else if(c == "POFD")      { v = cts_info.pofd.v;           }
      else if(c == "POFD_NCL")  { v = cts_info.pofd.v_ncl[i];    }
      else if(c == "POFD_NCU")  { v = cts_info.pofd.v_ncu[i];    }
      else if(c == "POFD_BCL")  { v = cts_info.pofd.v_bcl[i];    }
      else if(c == "POFD_BCU")  { v = cts_info.pofd.v_bcu[i];    }
      else if(c == "FAR")       { v = cts_info.far.v;            }
      else if(c == "FAR_NCL")   { v = cts_info.far.v_ncl[i];     }
      else if(c == "FAR_NCU")   { v = cts_info.far.v_ncu[i];     }
      else if(c == "FAR_BCL")   { v = cts_info.far.v_bcl[i];     }
      else if(c == "FAR_BCU")   { v = cts_info.far.v_bcu[i];     }
      else if(c == "CSI")       { v = cts_info.csi.v;            }
      else if(c == "CSI_NCL")   { v = cts_info.csi.v_ncl[i];     }
      else if(c == "CSI_NCU")   { v = cts_info.csi.v_ncu[i];     }
      else if(c == "CSI_BCL")   { v = cts_info.csi.v_bcl[i];     }
      else if(c == "CSI_BCU")   { v = cts_info.csi.v_bcu[i];     }
      else if(c == "GSS")       { v = cts_info.gss.v;            }
      else if(c == "GSS_BCL")   { v = cts_info.gss.v_bcl[i];     }
      else if(c == "GSS_BCU")   { v = cts_info.gss.v_bcu[i];     }
      else if(c == "HK")        { v = cts_info.hk.v;             }
      else if(c == "HK_NCL")    { v = cts_info.hk.v_ncl[i];      }
      else if(c == "HK_NCU")    { v = cts_info.hk.v_ncu[i];      }
      else if(c == "HK_BCL")    { v = cts_info.hk.v_bcl[i];      }
      else if(c == "HK_BCU")    { v = cts_info.hk.v_bcu[i];      }
      else if(c == "HSS")       { v = cts_info.hss.v;            }
      else if(c == "HSS_BCL")   { v = cts_info.hss.v_bcl[i];     }
      else if(c == "HSS_BCU")   { v = cts_info.hss.v_bcu[i];     }
      else if(c == "ODDS")      { v = cts_info.odds.v;           }
      else if(c == "ODDS_NCL")  { v = cts_info.odds.v_ncl[i];    }
      else if(c == "ODDS_NCU")  { v = cts_info.odds.v_ncu[i];    }
      else if(c == "ODDS_BCL")  { v = cts_info.odds.v_bcl[i];    }
      else if(c == "ODDS_BCU")  { v = cts_info.odds.v_bcu[i];    }
      else if(c == "LODDS")     { v = cts_info.lodds.v;          }
      else if(c == "LODDS_NCL") { v = cts_info.lodds.v_ncl[i];   }
      else if(c == "LODDS_NCU") { v = cts_info.lodds.v_ncu[i];   }
      else if(c == "LODDS_BCL") { v = cts_info.lodds.v_bcl[i];   }
      else if(c == "LODDS_BCU") { v = cts_info.lodds.v_bcu[i];   }
      else if(c == "ORSS")      { v = cts_info.orss.v;           }
      else if(c == "ORSS_NCL")  { v = cts_info.orss.v_ncl[i];    }
      else if(c == "ORSS_NCU")  { v = cts_info.orss.v_ncu[i];    }
      else if(c == "ORSS_BCL")  { v = cts_info.orss.v_bcl[i];    }
      else if(c == "ORSS_BCU")  { v = cts_info.orss.v_bcu[i];    }
      else if(c == "EDS")       { v = cts_info.eds.v;            }
      else if(c == "EDS_NCL")   { v = cts_info.eds.v_ncl[i];     }
      else if(c == "EDS_NCU")   { v = cts_info.eds.v_ncu[i];     }
      else if(c == "EDS_BCL")   { v = cts_info.eds.v_bcl[i];     }
      else if(c == "EDS_BCU")   { v = cts_info.eds.v_bcu[i];     }
      else if(c == "SEDS")      { v = cts_info.seds.v;           }
      else if(c == "SEDS_NCL")  { v = cts_info.seds.v_ncl[i];    }
      else if(c == "SEDS_NCU")  { v = cts_info.seds.v_ncu[i];    }
      else if(c == "SEDS_BCL")  { v = cts_info.seds.v_bcl[i];    }
      else if(c == "SEDS_BCU")  { v = cts_info.seds.v_bcu[i];    }
      else if(c == "EDI")       { v = cts_info.edi.v;            }
      else if(c == "EDI_NCL")   { v = cts_info.edi.v_ncl[i];     }
      else if(c == "EDI_NCU")   { v = cts_info.edi.v_ncu[i];     }
      else if(c == "EDI_BCL")   { v = cts_info.edi.v_bcl[i];     }
      else if(c == "EDI_BCU")   { v = cts_info.edi.v_bcu[i];     }
      else if(c == "SEDI")      { v = cts_info.sedi.v;           }
      else if(c == "SEDI_NCL")  { v = cts_info.sedi.v_ncl[i];    }
      else if(c == "SEDI_NCU")  { v = cts_info.sedi.v_ncu[i];    }
      else if(c == "SEDI_BCL")  { v = cts_info.sedi.v_bcl[i];    }
      else if(c == "SEDI_BCU")  { v = cts_info.sedi.v_bcu[i];    }
      else if(c == "BAGSS")     { v = cts_info.bagss.v;          }
      else if(c == "BAGSS_BCL") { v = cts_info.bagss.v_bcl[i];   }
      else if(c == "BAGSS_BCU") { v = cts_info.bagss.v_bcu[i];   }
      else {
        mlog << Error << "\nstore_stat_cts() -> "
             << "unsupported column name requested \"" << c
             << "\"\n\n";
        exit(1);
      }

      // Construct the NetCDF variable name
      var_name << cs_erase << "series_cts_" << c;

      // Append threshold information
      if(cts_info.fthresh == cts_info.othresh) {
         var_name << "_" << cts_info.fthresh.get_abbr_str();
      }
      else {
         var_name << "_fcst" << cts_info.fthresh.get_abbr_str()
                  << "_obs" << cts_info.othresh.get_abbr_str();
      }

      // Append confidence interval alpha value
      if(n_ci > 1) var_name << "_a"  << cts_info.alpha[i];

      // Add map for this variable name
      if(stat_data.count(var_name) == 0) {

         // Build key
         lty_stat << "CTS_" << c;

         // Add new map entry
         add_nc_var(var_name, c, stat_long_name[lty_stat],
                    cts_info.fthresh.get_str(),
                    cts_info.othresh.get_str(),
                    (n_ci > 1 ? cts_info.alpha[i] : bad_data_double));
      }

      // Store the statistic value
      put_nc_val(n, var_name, (float) v);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_mctc(int n, const ConcatString &col,
                     const MCTSInfo &mcts_info) {
   int i, j;
   double v;
   ConcatString lty_stat, var_name;
   StringArray sa;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);
   ConcatString d = c;

   // Get the column value
        if(c == "TOTAL") { v = (double) mcts_info.cts.total(); }
   else if(c == "N_CAT") { v = (double) mcts_info.cts.nrows(); }
   else if(check_reg_exp("F[0-9]*_O[0-9]*", c.c_str())) {

      d = "FI_OJ";

      // Parse column name to retrieve index values
      sa = c.split("_");
      i  = atoi(sa[0].c_str()+1) - 1;
      j  = atoi(sa[1].c_str()+1) - 1;

      // Range check
      if(i < 0 || i >= mcts_info.cts.nrows() ||
         j < 0 || j >= mcts_info.cts.ncols()) {
         mlog << Error << "\nstore_stat_mctc() -> "
              << "range check error for column name requested \"" << c
              << "\"\n\n";
         exit(1);
      }

      // Retrieve the value
      v = (double) mcts_info.cts.entry(i, j);
   }
   else {
     mlog << Error << "\nstore_stat_mctc() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Construct the NetCDF variable name
   var_name << cs_erase << "series_mctc_" << c;

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      lty_stat << "MCTC_" << d;

      // Add new map entry
      add_nc_var(var_name, c, stat_long_name[lty_stat],
                 mcts_info.fthresh.get_str(","),
                 mcts_info.othresh.get_str(","),
                 bad_data_double);
   }

   // Store the statistic value
   put_nc_val(n, var_name, (float) v);

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_mcts(int n, const ConcatString &col,
                     const MCTSInfo &mcts_info) {
   int i;
   double v;
   ConcatString lty_stat, var_name;
   int n_ci = 1;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Check for columns with normal or bootstrap confidence limits
   if(strstr(c.c_str(), "_NC") || strstr(c.c_str(), "_BC")) n_ci = mcts_info.n_alpha;

   // Loop over the alpha values, if necessary
   for(i=0; i<n_ci; i++) {

      // Get the column value
           if(c == "TOTAL")   { v = (double) mcts_info.cts.total(); }
      else if(c == "N_CAT")   { v = (double) mcts_info.cts.nrows(); }
      else if(c == "ACC")     { v = mcts_info.acc.v;                }
      else if(c == "ACC_NCL") { v = mcts_info.acc.v_ncl[i];         }
      else if(c == "ACC_NCU") { v = mcts_info.acc.v_ncu[i];         }
      else if(c == "ACC_BCL") { v = mcts_info.acc.v_bcl[i];         }
      else if(c == "ACC_BCU") { v = mcts_info.acc.v_bcu[i];         }
      else if(c == "HK")      { v = mcts_info.hk.v;                 }
      else if(c == "HK_BCL")  { v = mcts_info.hk.v_bcl[i];          }
      else if(c == "HK_BCU")  { v = mcts_info.hk.v_bcu[i];          }
      else if(c == "HSS")     { v = mcts_info.hss.v;                }
      else if(c == "HSS_BCL") { v = mcts_info.hss.v_bcl[i];         }
      else if(c == "HSS_BCU") { v = mcts_info.hss.v_bcu[i];         }
      else if(c == "GER")     { v = mcts_info.ger.v;                }
      else if(c == "GER_BCL") { v = mcts_info.ger.v_bcl[i];         }
      else if(c == "GER_BCU") { v = mcts_info.ger.v_bcu[i];         }
      else {
        mlog << Error << "\nstore_stat_mcts() -> "
             << "unsupported column name requested \"" << c
             << "\"\n\n";
        exit(1);
      }

      // Construct the NetCDF variable name
      var_name << cs_erase << "series_mcts_" << c;

      // Append confidence interval alpha value
      if(n_ci > 1) var_name << "_a"  << mcts_info.alpha[i];

      // Add map for this variable name
      if(stat_data.count(var_name) == 0) {

         // Build key
         lty_stat << "MCTS_" << c;

         // Add new map entry
         add_nc_var(var_name, c, stat_long_name[lty_stat],
                    mcts_info.fthresh.get_str(","),
                    mcts_info.othresh.get_str(","),
                    (n_ci > 1 ? mcts_info.alpha[i] : bad_data_double));
      }

      // Store the statistic value
      put_nc_val(n, var_name, (float) v);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_cnt(int n, const ConcatString &col,
                    const CNTInfo &cnt_info) {
   int i;
   double v;
   ConcatString lty_stat, var_name;
   int n_ci = 1;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Check for columns with normal or bootstrap confidence limits
   if(strstr(c.c_str(), "_NC") || strstr(c.c_str(), "_BC")) n_ci = cnt_info.n_alpha;

   // Loop over the alpha values, if necessary
   for(i=0; i<n_ci; i++) {

      // Get the column value
           if(c == "TOTAL")                { v = (double) cnt_info.n;                }
      else if(c == "FBAR")                 { v = cnt_info.fbar.v;                    }
      else if(c == "FBAR_NCL")             { v = cnt_info.fbar.v_ncl[i];             }
      else if(c == "FBAR_NCU")             { v = cnt_info.fbar.v_ncu[i];             }
      else if(c == "FBAR_BCL")             { v = cnt_info.fbar.v_bcl[i];             }
      else if(c == "FBAR_BCU")             { v = cnt_info.fbar.v_bcu[i];             }
      else if(c == "FSTDEV")               { v = cnt_info.fstdev.v;                  }
      else if(c == "FSTDEV_NCL")           { v = cnt_info.fstdev.v_ncl[i];           }
      else if(c == "FSTDEV_NCU")           { v = cnt_info.fstdev.v_ncu[i];           }
      else if(c == "FSTDEV_BCL")           { v = cnt_info.fstdev.v_bcl[i];           }
      else if(c == "FSTDEV_BCU")           { v = cnt_info.fstdev.v_bcu[i];           }
      else if(c == "OBAR")                 { v = cnt_info.obar.v;                    }
      else if(c == "OBAR_NCL")             { v = cnt_info.obar.v_ncl[i];             }
      else if(c == "OBAR_NCU")             { v = cnt_info.obar.v_ncu[i];             }
      else if(c == "OBAR_BCL")             { v = cnt_info.obar.v_bcl[i];             }
      else if(c == "OBAR_BCU")             { v = cnt_info.obar.v_bcu[i];             }
      else if(c == "OSTDEV")               { v = cnt_info.ostdev.v;                  }
      else if(c == "OSTDEV_NCL")           { v = cnt_info.ostdev.v_ncl[i];           }
      else if(c == "OSTDEV_NCU")           { v = cnt_info.ostdev.v_ncu[i];           }
      else if(c == "OSTDEV_BCL")           { v = cnt_info.ostdev.v_bcl[i];           }
      else if(c == "OSTDEV_BCU")           { v = cnt_info.ostdev.v_bcu[i];           }
      else if(c == "PR_CORR")              { v = cnt_info.pr_corr.v;                 }
      else if(c == "PR_CORR_NCL")          { v = cnt_info.pr_corr.v_ncl[i];          }
      else if(c == "PR_CORR_NCU")          { v = cnt_info.pr_corr.v_ncu[i];          }
      else if(c == "PR_CORR_BCL")          { v = cnt_info.pr_corr.v_bcl[i];          }
      else if(c == "PR_CORR_BCU")          { v = cnt_info.pr_corr.v_bcu[i];          }
      else if(c == "SP_CORR")              { v = cnt_info.sp_corr.v;                 }
      else if(c == "KT_CORR")              { v = cnt_info.kt_corr.v;                 }
      else if(c == "RANKS")                { v = cnt_info.n_ranks;                   }
      else if(c == "FRANK_TIES")           { v = cnt_info.frank_ties;                }
      else if(c == "ORANK_TIES")           { v = cnt_info.orank_ties;                }
      else if(c == "ME")                   { v = cnt_info.me.v;                      }
      else if(c == "ME_NCL")               { v = cnt_info.me.v_ncl[i];               }
      else if(c == "ME_NCU")               { v = cnt_info.me.v_ncu[i];               }
      else if(c == "ME_BCL")               { v = cnt_info.me.v_bcl[i];               }
      else if(c == "ME_BCU")               { v = cnt_info.me.v_bcu[i];               }
      else if(c == "ESTDEV")               { v = cnt_info.estdev.v;                  }
      else if(c == "ESTDEV_NCL")           { v = cnt_info.estdev.v_ncl[i];           }
      else if(c == "ESTDEV_NCU")           { v = cnt_info.estdev.v_ncu[i];           }
      else if(c == "ESTDEV_BCL")           { v = cnt_info.estdev.v_bcl[i];           }
      else if(c == "ESTDEV_BCU")           { v = cnt_info.estdev.v_bcu[i];           }
      else if(c == "MBIAS")                { v = cnt_info.mbias.v;                   }
      else if(c == "MBIAS_BCL")            { v = cnt_info.mbias.v_bcl[i];            }
      else if(c == "MBIAS_BCU")            { v = cnt_info.mbias.v_bcu[i];            }
      else if(c == "MAE")                  { v = cnt_info.mae.v;                     }
      else if(c == "MAE_BCL")              { v = cnt_info.mae.v_bcl[i];              }
      else if(c == "MAE_BCU")              { v = cnt_info.mae.v_bcu[i];              }
      else if(c == "MSE")                  { v = cnt_info.mse.v;                     }
      else if(c == "MSE_BCL")              { v = cnt_info.mse.v_bcl[i];              }
      else if(c == "MSE_BCU")              { v = cnt_info.mse.v_bcu[i];              }
      else if(c == "BCMSE")                { v = cnt_info.bcmse.v;                   }
      else if(c == "BCMSE_BCL")            { v = cnt_info.bcmse.v_bcl[i];            }
      else if(c == "BCMSE_BCU")            { v = cnt_info.bcmse.v_bcu[i];            }
      else if(c == "RMSE")                 { v = cnt_info.rmse.v;                    }
      else if(c == "RMSE_BCL")             { v = cnt_info.rmse.v_bcl[i];             }
      else if(c == "RMSE_BCU")             { v = cnt_info.rmse.v_bcu[i];             }
      else if(c == "E10")                  { v = cnt_info.e10.v;                     }
      else if(c == "E10_BCL")              { v = cnt_info.e10.v_bcl[i];              }
      else if(c == "E10_BCU")              { v = cnt_info.e10.v_bcu[i];              }
      else if(c == "E25")                  { v = cnt_info.e25.v;                     }
      else if(c == "E25_BCL")              { v = cnt_info.e25.v_bcl[i];              }
      else if(c == "E25_BCU")              { v = cnt_info.e25.v_bcu[i];              }
      else if(c == "E50")                  { v = cnt_info.e50.v;                     }
      else if(c == "E50_BCL")              { v = cnt_info.e50.v_bcl[i];              }
      else if(c == "E50_BCU")              { v = cnt_info.e50.v_bcu[i];              }
      else if(c == "E75")                  { v = cnt_info.e75.v;                     }
      else if(c == "E75_BCL")              { v = cnt_info.e75.v_bcl[i];              }
      else if(c == "E75_BCU")              { v = cnt_info.e75.v_bcu[i];              }
      else if(c == "E90")                  { v = cnt_info.e90.v;                     }
      else if(c == "E90_BCL")              { v = cnt_info.e90.v_bcl[i];              }
      else if(c == "E90_BCU")              { v = cnt_info.e90.v_bcu[i];              }
      else if(c == "EIQR")                 { v = cnt_info.eiqr.v;                    }
      else if(c == "EIQR_BCL")             { v = cnt_info.eiqr.v_bcl[i];             }
      else if(c == "EIQR_BCU")             { v = cnt_info.eiqr.v_bcu[i];             }
      else if(c == "MAD")                  { v = cnt_info.mad.v;                     }
      else if(c == "MAD_BCL")              { v = cnt_info.mad.v_bcl[i];              }
      else if(c == "MAD_BCU")              { v = cnt_info.mad.v_bcu[i];              }
      else if(c == "ANOM_CORR")            { v = cnt_info.anom_corr.v;               }
      else if(c == "ANOM_CORR_NCL")        { v = cnt_info.anom_corr.v_ncl[i];        }
      else if(c == "ANOM_CORR_NCU")        { v = cnt_info.anom_corr.v_ncu[i];        }
      else if(c == "ANOM_CORR_BCL")        { v = cnt_info.anom_corr.v_bcl[i];        }
      else if(c == "ANOM_CORR_BCU")        { v = cnt_info.anom_corr.v_bcu[i];        }
      else if(c == "ME2")                  { v = cnt_info.me2.v;                     }
      else if(c == "ME2_BCL")              { v = cnt_info.me2.v_bcl[i];              }
      else if(c == "ME2_BCU")              { v = cnt_info.me2.v_bcu[i];              }
      else if(c == "MSESS")                { v = cnt_info.msess.v;                   }
      else if(c == "MSESS_BCL")            { v = cnt_info.msess.v_bcl[i];            }
      else if(c == "MSESS_BCU")            { v = cnt_info.msess.v_bcu[i];            }
      else if(c == "RMSFA")                { v = cnt_info.rmsfa.v;                   }
      else if(c == "RMSFA_BCL")            { v = cnt_info.rmsfa.v_bcl[i];            }
      else if(c == "RMSFA_BCU")            { v = cnt_info.rmsfa.v_bcu[i];            }
      else if(c == "RMSOA")                { v = cnt_info.rmsoa.v;                   }
      else if(c == "RMSOA_BCL")            { v = cnt_info.rmsoa.v_bcl[i];            }
      else if(c == "RMSOA_BCU")            { v = cnt_info.rmsoa.v_bcu[i];            }
      else if(c == "ANOM_CORR_UNCNTR")     { v = cnt_info.anom_corr_uncntr.v;        }
      else if(c == "ANOM_CORR_UNCNTR_BCL") { v = cnt_info.anom_corr_uncntr.v_bcl[i]; }
      else if(c == "ANOM_CORR_UNCNTR_BCU") { v = cnt_info.anom_corr_uncntr.v_bcu[i]; }
      else {
        mlog << Error << "\nstore_stat_cnt() -> "
             << "unsupported column name requested \"" << c
             << "\"\n\n";
        exit(1);
      }

      // Construct the NetCDF variable name
      var_name << cs_erase << "series_cnt_" << c;

      // Append threshold information, if supplied
      if(cnt_info.fthresh.get_type() != thresh_na ||
         cnt_info.othresh.get_type() != thresh_na) {
         var_name << "_fcst" << cnt_info.fthresh.get_abbr_str()
                  << "_" << setlogic_to_abbr(conf_info.cnt_logic)
                  << "_obs" << cnt_info.othresh.get_abbr_str();
      }

      // Append confidence interval alpha value
      if(n_ci > 1) var_name << "_a"  << cnt_info.alpha[i];

      // Add map for this variable name
      if(stat_data.count(var_name) == 0) {

         // Build key
         lty_stat << "CNT_" << c;

         // Add new map entry
         add_nc_var(var_name, c, stat_long_name[lty_stat],
                    cnt_info.fthresh.get_str(),
                    cnt_info.othresh.get_str(),
                    (n_ci > 1 ? cnt_info.alpha[i] : bad_data_double));
      }

      // Store the statistic value
      put_nc_val(n, var_name, (float) v);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_sl1l2(int n, const ConcatString &col,
                      const SL1L2Info &s_info) {
   double v;
   ConcatString lty_stat, var_name;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Get the column value
        if(c == "TOTAL")  { v = (double) s_info.scount; }
   else if(c == "FBAR")   { v = s_info.fbar;            }
   else if(c == "OBAR")   { v = s_info.obar;            }
   else if(c == "FOBAR")  { v = s_info.fobar;           }
   else if(c == "FFBAR")  { v = s_info.ffbar;           }
   else if(c == "OOBAR")  { v = s_info.oobar;           }
   else if(c == "MAE")    { v = s_info.mae;             }
   else if(c == "FABAR")  { v = s_info.fabar;           }
   else if(c == "OABAR")  { v = s_info.oabar;           }
   else if(c == "FOABAR") { v = s_info.foabar;          }
   else if(c == "FFABAR") { v = s_info.ffabar;          }
   else if(c == "OOABAR") { v = s_info.ooabar;          }
   else {
     mlog << Error << "\nstore_stat_sl1l2() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Construct the NetCDF variable name
   var_name << cs_erase << "series_sl1l2_" << c;

   // Append threshold information, if supplied
   if(s_info.fthresh.get_type() != thresh_na ||
      s_info.othresh.get_type() != thresh_na) {
      var_name << "_fcst" << s_info.fthresh.get_abbr_str()
               << "_" << setlogic_to_abbr(conf_info.cnt_logic)
               << "_obs" << s_info.othresh.get_abbr_str();
   }

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      lty_stat << "SL1L2_" << c;

      // Add new map entry
      add_nc_var(var_name, c, stat_long_name[lty_stat],
                 s_info.fthresh.get_str(),
                 s_info.othresh.get_str(),
                 bad_data_double);
   }

   // Store the statistic value
   put_nc_val(n, var_name, (float) v);

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_pct(int n, const ConcatString &col,
                    const PCTInfo &pct_info) {
   int i = 0;
   double v;
   ConcatString lty_stat, var_name;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);
   ConcatString d = c;

   // Get index value for variable column numbers
   if(check_reg_exp("_[0-9]", c.c_str())) {

      // Parse the index value from the column name
      i = atoi(strrchr(c.c_str(), '_') + 1) - 1;

      // Range check
      if(i < 0 || i >= pct_info.pct.nrows()) {
         mlog << Error << "\nstore_stat_pct() -> "
              << "range check error for column name requested \"" << c
              << "\"\n\n";
         exit(1);
      }
   }  // end if

   // Get the column value
        if(c == "TOTAL")                             { v = (double) pct_info.pct.n();                      }
   else if(c == "N_THRESH")                          { v = (double) pct_info.pct.nrows() + 1;              }
   else if(check_reg_exp("THRESH_[0-9]", c.c_str())) { v = pct_info.pct.threshold(i);                      }
   else if(check_reg_exp("OY_[0-9]", c.c_str()))     { v = (double) pct_info.pct.event_count_by_row(i);
                                                       d = "OY_I";                                         }
   else if(check_reg_exp("ON_[0-9]", c.c_str()))     { v = (double) pct_info.pct.nonevent_count_by_row(i);
                                                       d = "ON_I";                                         }
   else {
     mlog << Error << "\nstore_stat_pct() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Construct the NetCDF variable name
   var_name << cs_erase << "series_pct_" << c
            << "_obs" << pct_info.othresh.get_abbr_str();

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      lty_stat << "PCT_" << d;

      // Add new map entry
      add_nc_var(var_name, c, stat_long_name[lty_stat],
                 pct_info.fthresh.get_str(","),
                 pct_info.othresh.get_str(),
                 bad_data_double);
   }

   // Store the statistic value
   put_nc_val(n, var_name, (float) v);

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_pstd(int n, const ConcatString &col,
                     const PCTInfo &pct_info) {
   int i;
   double v;
   ConcatString lty_stat, var_name;
   int n_ci = 1;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Check for columns with normal or bootstrap confidence limits
   if(strstr(c.c_str(), "_NC") || strstr(c.c_str(), "_BC")) n_ci = pct_info.n_alpha;

   // Loop over the alpha values, if necessary
   for(i=0; i<n_ci; i++) {

      // Get the column value
           if(c == "TOTAL")       { v = (double) pct_info.pct.n();         }
      else if(c == "N_THRESH")    { v = (double) pct_info.pct.nrows() + 1; }
      else if(c == "BASER")       { v = pct_info.baser.v;                  }
      else if(c == "BASER_NCL")   { v = pct_info.baser.v_ncl[i];           }
      else if(c == "BASER_NCU")   { v = pct_info.baser.v_ncu[i];           }
      else if(c == "RELIABILITY") { v = pct_info.pct.reliability();        }
      else if(c == "RESOLUTION")  { v = pct_info.pct.resolution();         }
      else if(c == "UNCERTAINTY") { v = pct_info.pct.uncertainty();        }
      else if(c == "ROC_AUC")     { v = pct_info.pct.roc_auc();            }
      else if(c == "BRIER")       { v = pct_info.brier.v;                  }
      else if(c == "BRIER_NCL")   { v = pct_info.brier.v_ncl[i];           }
      else if(c == "BRIER_NCU")   { v = pct_info.brier.v_ncu[i];           }
      else if(c == "BSS")         { v = pct_info.bss;                      }
      else {
        mlog << Error << "\nstore_stat_pstd() -> "
             << "unsupported column name requested \"" << c
             << "\"\n\n";
        exit(1);
      }

      // Construct the NetCDF variable name
      var_name << cs_erase << "series_pstd_" << c;

      // Append confidence interval alpha value
      if(n_ci > 1) var_name << "_a"  << pct_info.alpha[i];

      // Add map for this variable name
      if(stat_data.count(var_name) == 0) {

         // Build key
         lty_stat << "PSTD_" << c;

         // Add new map entry
         add_nc_var(var_name, c, stat_long_name[lty_stat],
                    pct_info.fthresh.get_str(","),
                    pct_info.othresh.get_str(),
                    (n_ci > 1 ? pct_info.alpha[i] : bad_data_double));
      }

      // Store the statistic value
      put_nc_val(n, var_name, (float) v);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_pjc(int n, const ConcatString &col,
                    const PCTInfo &pct_info) {
   int i = 0;
   int tot;
   double v;
   ConcatString lty_stat, var_name;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);
   ConcatString d = c;

   // Get index value for variable column numbers
   if(check_reg_exp("_[0-9]", c.c_str())) {

      // Parse the index value from the column name
      i = atoi(strrchr(c.c_str(), '_') + 1) - 1;

      // Range check
      if(i < 0 || i >= pct_info.pct.nrows()) {
         mlog << Error << "\nstore_stat_pjc() -> "
              << "range check error for column name requested \"" << c
              << "\"\n\n";
         exit(1);
      }
   }  // end if

   // Store the total count
   tot = pct_info.pct.n();

   // Get the column value
        if(c == "TOTAL")                                  { v = (double) tot;                                       }
   else if(c == "N_THRESH")                               { v = (double) pct_info.pct.nrows() + 1;                  }
   else if(check_reg_exp("THRESH_[0-9]", c.c_str()))      { v = pct_info.pct.threshold(i);
                                                            d = "THRESH_I";                                         }
   else if(check_reg_exp("OY_TP_[0-9]", c.c_str()))       { v = pct_info.pct.event_count_by_row(i)/(double) tot;
                                                            d = "OY_TP_I";                                          }
   else if(check_reg_exp("ON_TP_[0-9]", c.c_str()))       { v = pct_info.pct.nonevent_count_by_row(i)/(double) tot;
                                                            d = "ON_TP_I";                                          }
   else if(check_reg_exp("CALIBRATION_[0-9]", c.c_str())) { v = pct_info.pct.row_calibration(i);
                                                            d = "CALIBRATION_I";                                    }
   else if(check_reg_exp("REFINEMENT_[0-9]", c.c_str()))  { v = pct_info.pct.row_refinement(i);
                                                            d = "REFINEMENT_I";                                     }
   else if(check_reg_exp("LIKELIHOOD_[0-9]", c.c_str()))  { v = pct_info.pct.row_event_likelihood(i);
                                                            d = "LIKELIHOOD_I";                                     }
   else if(check_reg_exp("BASER_[0-9]", c.c_str()))       { v = pct_info.pct.row_obar(i);
                                                            d = "BASER_I";                                          }
   else {
     mlog << Error << "\nstore_stat_pjc() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Construct the NetCDF variable name
   var_name << cs_erase << "series_pjc_" << c
            << "_obs" << pct_info.othresh.get_abbr_str();

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      lty_stat << "PJC_" << d;

      // Add new map entry
      add_nc_var(var_name, c, stat_long_name[lty_stat],
                 pct_info.fthresh.get_str(","),
                 pct_info.othresh.get_str(),
                 bad_data_double);
   }

   // Store the statistic value
   put_nc_val(n, var_name, (float) v);

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_prc(int n, const ConcatString &col,
                    const PCTInfo &pct_info) {
   int i = 0;
   double v;
   ConcatString lty_stat, var_name;
   TTContingencyTable ct;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);
   ConcatString d = c;

   // Get index value for variable column numbers
   if(check_reg_exp("_[0-9]", c.c_str())) {

      // Parse the index value from the column name
      i = atoi(strrchr(c.c_str(), '_') + 1) - 1;

      // Range check
      if(i < 0 || i >= pct_info.pct.nrows()) {
         mlog << Error << "\nstore_stat_prc() -> "
              << "range check error for column name requested \"" << c
              << "\"\n\n";
         exit(1);
      }

      // Get the 2x2 contingency table for this row
      ct = pct_info.pct.ctc_by_row(i);

   }  // end if

   // Get the column value
        if(c == "TOTAL")                             { v = (double) pct_info.pct.n();         }
   else if(c == "N_THRESH")                          { v = (double) pct_info.pct.nrows() + 1; }
   else if(check_reg_exp("THRESH_[0-9]", c.c_str())) { v = pct_info.pct.threshold(i);
                                                       d = "THRESH_I";                        }
   else if(check_reg_exp("PODY_[0-9]", c.c_str()))   { v = ct.pod_yes();
                                                       d = "PODY_I";                          }
   else if(check_reg_exp("POFD_[0-9]", c.c_str()))   { v = ct.pofd();
                                                       d = "POFD_I";                          }
   else {
     mlog << Error << "\nstore_stat_prc() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      lty_stat << "PRC_" << d;

      // Add new map entry
      add_nc_var(var_name, c, stat_long_name[lty_stat],
                 pct_info.fthresh.get_str(","),
                 pct_info.othresh.get_str(),
                 bad_data_double);
   }

   // Store the statistic value
   put_nc_val(n, var_name, (float) v);

   return;
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
   add_att(nc_out, "fcst_var",   (string)fcst_info->name_attr());
   add_att(nc_out, "fcst_lev",   (string)fcst_info->level_attr());
   add_att(nc_out, "fcst_units", (string)fcst_info->units_attr());
   add_att(nc_out, "obs_var",    (string)obs_info->name_attr());
   add_att(nc_out, "obs_lev",    (string)obs_info->level_attr());
   add_att(nc_out, "obs_units",  (string)obs_info->units_attr());

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
        << "\t-fcst  file_1 ... file_n | fcst_file_list\n"
        << "\t-obs   file_1 ... file_n | obs_file_list\n"
        << "\t[-both file_1 ... file_n | both_file_list]\n"
        << "\t[-paired]\n"
        << "\t-out file\n"
        << "\t-config file\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"-fcst file_1 ... file_n\" are the gridded "
        << "forecast files to be used (required).\n"

        << "\t\t\"-fcst fcst_file_list\" is an ASCII file containing "
        << "a list of gridded forecast files to be used (required).\n"

        << "\t\t\"-obs  file_1 ... file_n\" are the gridded "
        << "observation files to be used (required).\n"

        << "\t\t\"-obs  obs_file_list\" is an ASCII file containing "
        << "a list of gridded observation files to be used (required).\n"

        << "\t\t\"-both\" sets the \"-fcst\" and \"-obs\" options to "
        << "the same set of files (optional).\n"

        << "\t\t\"-paired\" to indicate that the input -fcst and -obs "
        << "file lists are already paired (optional).\n"

        << "\t\t\"-out file\" is the NetCDF output file containing "
        << "computed statistics (required).\n"

        << "\t\t\"-config file\" is a SeriesAnalysisConfig file "
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

   if(mlog.verbosity_level() >= 3) {
      mlog << Warning << "\nRunning Series-Analysis at verbosity >= 3 "
           << "produces excessive log output and can slow the runtime "
           << "considerably.\n\n";
   }
}

////////////////////////////////////////////////////////////////////////

void set_compress(const StringArray & a) {
   compress_level = atoi(a[0].c_str());
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
      if(sa.n() < 2) continue;

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
