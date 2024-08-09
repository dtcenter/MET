// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
//   011    05/28/21  Halley Gotway  Add MCTS HSS_EC output.
//   012    01/20/22  Halley Gotway  MET #2003 Add PSTD BRIERCL output.
//   013    05/25/22  Halley Gotway  MET #2147 Add CTS HSS_EC output.
//   014    07/06/22  Howard Soh     METplus-Internal #19 Rename main to met_main.
//   015    10/03/22  Presotpnik     MET #2227 Remove namespace netCDF from header files.
//   016    01/29/24  Halley Gotway  MET #2801 Configure time difference warnings.
//   017    07/05/24  Halley Gotway  MET #2924 Support forecast climatology.
//   018    07/26/24  Halley Gotway  MET #1371 Aggregate previous output.
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
#include "series_analysis.h"

#include "vx_statistics.h"
#include "vx_nc_util.h"
#include "vx_regrid.h"
#include "vx_log.h"
#include "enum_as_int.hpp"

using namespace std;
using namespace netCDF;

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
static DataPlane get_aggr_data(const ConcatString &);

static void process_scores();

static void do_categorical   (int, const PairDataPoint *);
static void do_multicategory (int, const PairDataPoint *);
static void do_continuous    (int, const PairDataPoint *);
static void do_partialsums   (int, const PairDataPoint *);
static void do_probabilistic (int, const PairDataPoint *);

// TODO: MET #1371
// - Add a PCT aggregation logic test
// - Switch to set_stat() and get_stat() functions
// - Can briercl be aggregated as a weighted average and used for bss?
// - How should valid data thresholds be applied when reading -aggr data?
// - Currently no way to aggregate anom_corr since CNTInfo::set(sl1l2)
//   doesn't support it.

static void read_aggr_ctc    (int, const CTSInfo &,   TTContingencyTable &);
static void read_aggr_mctc   (int, const MCTSInfo &,  ContingencyTable &);
static void read_aggr_sl1l2  (int, const SL1L2Info &, SL1L2Info &);
static void read_aggr_sal1l2 (int, const SL1L2Info &, SL1L2Info &);
static void read_aggr_pct    (int, const PCTInfo &,   Nx2ContingencyTable &);

static void store_stat_fho   (int, const ConcatString &, const CTSInfo &);
static void store_stat_ctc   (int, const ConcatString &, const CTSInfo &);
static void store_stat_cts   (int, const ConcatString &, const CTSInfo &);
static void store_stat_mctc  (int, const ConcatString &, const MCTSInfo &);
static void store_stat_mcts  (int, const ConcatString &, const MCTSInfo &);
static void store_stat_cnt   (int, const ConcatString &, const CNTInfo &);
static void store_stat_sl1l2 (int, const ConcatString &, const SL1L2Info &);
static void store_stat_sal1l2(int, const ConcatString &, const SL1L2Info &);
static void store_stat_pct   (int, const ConcatString &, const PCTInfo &);
static void store_stat_pstd  (int, const ConcatString &, const PCTInfo &);
static void store_stat_pjc   (int, const ConcatString &, const PCTInfo &);
static void store_stat_prc   (int, const ConcatString &, const PCTInfo &);

static void store_stat_all_ctc   (int, const CTSInfo &);
static void store_stat_all_mctc  (int, const MCTSInfo &);
static void store_stat_all_sl1l2 (int, const SL1L2Info &);
static void store_stat_all_sal1l2(int, const SL1L2Info &);
static void store_stat_all_pct   (int, const PCTInfo &);

static ConcatString build_nc_var_name_ctc(const ConcatString &, const CTSInfo &);
static ConcatString build_nc_var_name_sl1l2(const ConcatString &, const SL1L2Info &);
static ConcatString build_nc_var_name_sal1l2(const ConcatString &, const SL1L2Info &);
static ConcatString build_nc_var_name_cnt(const ConcatString &, const CNTInfo &);

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
static void set_aggr(const StringArray &);
static void set_paired(const StringArray &);
static void set_out_file(const StringArray &);
static void set_config_file(const StringArray &);
static void set_compress(const StringArray &);

static void parse_long_names();

////////////////////////////////////////////////////////////////////////

int met_main(int argc, char *argv[]) {

   // Process the command line arguments
   process_command_line(argc, argv);

   // Compute the scores and write them out
   process_scores();

   // Close the text files and deallocate memory
   clean_up();

   return 0;
}

////////////////////////////////////////////////////////////////////////

const string get_tool_name() {
   return "series_analysis";
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
   cline.add(set_fcst_files,  "-fcst",    -1);
   cline.add(set_obs_files,   "-obs",     -1);
   cline.add(set_both_files,  "-both",    -1);
   cline.add(set_aggr,        "-aggr",     1);
   cline.add(set_paired,      "-paired",   0);
   cline.add(set_config_file, "-config",   1);
   cline.add(set_out_file,    "-out",      1);
   cline.add(set_compress,    "-compress", 1);

   // Parse the command line
   cline.parse();

   // Check for error. There should be zero arguments left.
   if(cline.n() != 0) usage();

   // Recommend logging verbosity level of 3 or less
   if(mlog.verbosity_level() >= 3) {
      mlog << Debug(3) << "Running Series-Analysis at verbosity >= 3 "
           << "produces excessive log output and can slow the runtime "
           << "considerably.\n";
   }

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
   if(aggr_file == out_file) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the \"-out\" and \"-aggr\" options cannot be "
           << "set to the same file (\"" << aggr_file << "\")!\n\n";
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
      series_type = SeriesType::Fcst_Conf;
      n_series = conf_info.get_n_fcst();
      mlog << Debug(1)
           << "Series defined by the \"fcst.field\" configuration entry "
           << "of length " << n_series << ".\n";
   }
   else if(conf_info.get_n_obs() > 1) {
      series_type = SeriesType::Obs_Conf;
      n_series = conf_info.get_n_obs();
      mlog << Debug(1)
           << "Series defined by the \"obs.field\" configuration entry "
           << "of length " << n_series << ".\n";
   }
   else if(fcst_files.n() > 1) {
      series_type = SeriesType::Fcst_Files;
      n_series = fcst_files.n();
      mlog << Debug(1)
           << "Series defined by the forecast file list of length "
           << n_series << ".\n";
   }
   else if(obs_files.n() > 1) {
      series_type = SeriesType::Obs_Files;
      n_series = obs_files.n();
      mlog << Debug(1)
           << "Series defined by the observation file list of length "
           << n_series << ".\n";
   }
   else {
      series_type = SeriesType::Fcst_Conf;
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

   // Process masking regions
   conf_info.process_masks(grid);

   // Set the block size, if needed
   if(is_bad_data(conf_info.block_size)) {
      conf_info.block_size = grid.nxy();
   }

   // Compute the number of reads required
   n_reads = nint(ceil((double) grid.nxy() / conf_info.block_size));

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
   Met2dDataFile *mtddf = (Met2dDataFile *) nullptr;

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

   return mtddf;
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

      case SeriesType::Fcst_Conf:
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

      case SeriesType::Obs_Conf:
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

      case SeriesType::Fcst_Files:
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

      case SeriesType::Obs_Files:
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
              << enum_class_as_int(series_type) << "\n\n";
         exit(1);
   }

   // Setup the verification grid
   if(!grid.is_set()) process_grid(fcst_grid, obs_grid);

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

      mlog << Debug(3)
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

      mlog << Debug(3)
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

      ConcatString cs;
      cs << cs_erase
         << "Forecast and observation valid times do not match ("
         << unix_to_yyyymmdd_hhmmss(fcst_dp.valid()) << " != "
         << unix_to_yyyymmdd_hhmmss(obs_dp.valid()) << ") for "
         << fcst_info->magic_str() << " versus "
         << obs_info->magic_str() << ".";

      if(conf_info.conf.time_offset_warning(
            (int) (fcst_dp.valid() - obs_dp.valid()))) {
         mlog << Warning << "\nget_series_data() -> "
              << cs << "\n\n";
      }
      else {
         mlog << Debug(3) << cs << "\n";
      }
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
   if(dp.is_empty()) {
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
   Met2dDataFile *mtddf = (Met2dDataFile *) nullptr;
   bool found = false;

   // Check that the file exists
   if(!file_is_ok(cur_file, type)) {
      mlog << Warning << "\nread_single_entry() -> "
           << "File does not exist: " << cur_file << "\n\n";
      return false;
   }

   // Open the data file
   mtddf = mtddf_factory.new_met_2d_data_file(cur_file.c_str(), type);

   // Attempt to read the gridded data from the current file
   found = mtddf->data_plane(*info, dp);

   // Store the current grid
   if(found) cur_grid = mtddf->grid();

   // Close the data file
   delete mtddf; mtddf = (Met2dDataFile *) nullptr;

   return found;
}

////////////////////////////////////////////////////////////////////////

DataPlane get_aggr_data(const ConcatString &var_name) {
   DataPlane aggr_dp;
   bool found = false;

   // Open the aggregate file, if needed
   if(!aggr_nc.MetNc) {

      mlog << Debug(1)
           << "Reading aggregate data file: " << aggr_file << "\n";

      aggr_nc.open(aggr_file.c_str());

      // Update timing info based on aggregate file global attributes
      ConcatString cs;

      if(get_att_value_string(aggr_nc.MetNc->Nc, "fcst_init_beg", cs)) {
         set_range(timestring_to_unix(cs.c_str()), fcst_init_beg, fcst_init_end);
      }
      if(get_att_value_string(aggr_nc.MetNc->Nc, "fcst_init_end", cs)) {
         set_range(timestring_to_unix(cs.c_str()), fcst_init_beg, fcst_init_end);
      }
      if(get_att_value_string(aggr_nc.MetNc->Nc, "fcst_valid_beg", cs)) {
         set_range(timestring_to_unix(cs.c_str()), fcst_valid_beg, fcst_valid_end);
      }
      if(get_att_value_string(aggr_nc.MetNc->Nc, "fcst_valid_end", cs)) {
         set_range(timestring_to_unix(cs.c_str()), fcst_valid_beg, fcst_valid_end);
      }
      if(get_att_value_string(aggr_nc.MetNc->Nc, "fcst_lead_beg", cs)) {
         set_range(timestring_to_sec(cs.c_str()), fcst_lead_beg, fcst_lead_end);
      }
      if(get_att_value_string(aggr_nc.MetNc->Nc, "fcst_lead_end", cs)) {
         set_range(timestring_to_sec(cs.c_str()), fcst_lead_beg, fcst_lead_end);
      }
      if(get_att_value_string(aggr_nc.MetNc->Nc, "obs_init_beg", cs)) {
         set_range(timestring_to_unix(cs.c_str()), obs_init_beg, obs_init_end);
      }
      if(get_att_value_string(aggr_nc.MetNc->Nc, "obs_init_end", cs)) {
         set_range(timestring_to_unix(cs.c_str()), obs_init_beg, obs_init_end);
      }
      if(get_att_value_string(aggr_nc.MetNc->Nc, "obs_valid_beg", cs)) {
         set_range(timestring_to_unix(cs.c_str()), obs_valid_beg, obs_valid_end);
      }
      if(get_att_value_string(aggr_nc.MetNc->Nc, "obs_valid_end", cs)) {
         set_range(timestring_to_unix(cs.c_str()), obs_valid_beg, obs_valid_end);
      }
      if(get_att_value_string(aggr_nc.MetNc->Nc, "obs_lead_beg", cs)) {
         set_range(timestring_to_sec(cs.c_str()), obs_lead_beg, obs_lead_end);
      }
      if(get_att_value_string(aggr_nc.MetNc->Nc, "obs_lead_end", cs)) {
         set_range(timestring_to_sec(cs.c_str()), obs_lead_beg, obs_lead_end);
      }
   }

   // Setup the data request
   VarInfoNcMet aggr_info;
   aggr_info.set_magic(var_name, "(*,*)");

   // Attempt to read the gridded data from the current file
   if(!aggr_nc.data_plane(aggr_info, aggr_dp)) {
      mlog << Error << "\nget_aggr_data() -> "
           << "Required variable \"" << aggr_info.magic_str() << "\""
           << " not found in aggregate file!\n\n";
      exit(1);
   }

   // Check that the grid has not changed
   if(aggr_nc.grid().nx() != grid.nx() ||
      aggr_nc.grid().ny() != grid.ny()) {
      mlog << Error << "\nget_aggr_data() -> "
           << "the input grid dimensions (" << grid.nx() << ", " << grid.ny()
           << ") and aggregate grid dimensions (" << aggr_nc.grid().nx()
           << ", " << aggr_nc.grid().ny() << ") do not match!\n\n";
      exit(1);
   }

   return aggr_dp;
}

////////////////////////////////////////////////////////////////////////

void process_scores() {
   int i, x, y;
   int i_point = 0;
   VarInfo *fcst_info = (VarInfo *) nullptr;
   VarInfo *obs_info  = (VarInfo *) nullptr;
   DataPlane fcst_dp, obs_dp;
   const char *method_name = "process_scores() ";

   // Climatology mean and standard deviation
   DataPlane fcmn_dp, fcsd_dp;
   DataPlane ocmn_dp, ocsd_dp;

   // Number of points skipped due to valid data threshold
   int n_skip_zero = 0;
   int n_skip_pos  = 0;

   // Create a vector of PairDataPoint objects
   vector<PairDataPoint> pd_block;
   pd_block.resize(conf_info.block_size);
   for(auto &x : pd_block) x.extend(n_series);

   // Loop over the data reads
   for(int i_read=0; i_read<n_reads; i_read++) {

      // Re-initialize the PairDataPoint objects
      for(auto &x : pd_block) {
         x.erase();
         x.set_climo_cdf_info_ptr(&conf_info.cdf_info);
      }

      // Loop over the series variable
      for(int i_series=0; i_series<n_series; i_series++) {

         // Get the index for the forecast and climo VarInfo objects
         int i_fcst = (conf_info.get_n_fcst() > 1 ? i_series : 0);

         // Store the current VarInfo objects
         fcst_info = conf_info.fcst_info[i_fcst];
         obs_info  = (conf_info.get_n_obs() > 1 ?
                      conf_info.obs_info[i_series] :
                      conf_info.obs_info[0]);

         // Retrieve the data planes for the current series entry
         get_series_data(i_series, fcst_info, obs_info, fcst_dp, obs_dp);

         // Define starting point for this data pass
         if(i_series == 0) {

            // Starting grid point
            i_point = i_read*conf_info.block_size;

            mlog << Debug(2)
                 << "Processing data pass number " << i_read + 1 << " of "
                 << n_reads << " for grid points " << i_point + 1 << " to "
                 << min(i_point + conf_info.block_size, grid.nxy()) << ".\n";
         }

         // Read climatology data for the current series entry
         fcmn_dp = read_climo_data_plane(
                      conf_info.conf.lookup_array(conf_key_fcst_climo_mean_field, false),
                      i_fcst, fcst_dp.valid(), grid);
         fcsd_dp = read_climo_data_plane(
                      conf_info.conf.lookup_array(conf_key_fcst_climo_stdev_field, false),
                      i_fcst, fcst_dp.valid(), grid);
         ocmn_dp = read_climo_data_plane(
                      conf_info.conf.lookup_array(conf_key_obs_climo_mean_field, false),
                      i_fcst, fcst_dp.valid(), grid);
         ocsd_dp = read_climo_data_plane(
                      conf_info.conf.lookup_array(conf_key_obs_climo_stdev_field, false),
                      i_fcst, fcst_dp.valid(), grid);

         bool fcmn_flag = !fcmn_dp.is_empty();
         bool fcsd_flag = !fcsd_dp.is_empty();
         bool ocmn_flag = !ocmn_dp.is_empty();
         bool ocsd_flag = !ocsd_dp.is_empty();

         mlog << Debug(3)
              << "For " << fcst_info->magic_str() << ", found "
              << (fcmn_flag ? 0 : 1) << " forecast climatology mean and "
              << (fcsd_flag ? 0 : 1) << " standard deviation field(s), and "
              << (ocmn_flag ? 0 : 1) << " observation climatology mean and "
              << (ocsd_flag ? 0 : 1) << " standard deviation field(s).\n";

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
         for(i=0; i<conf_info.block_size && (i_point+i)<grid.nxy(); i++) {

            // Convert n to x, y
            DefaultTO.one_to_two(grid.nx(), grid.ny(), i_point+i, x, y);

            // Skip points outside the mask and bad data
            if(!conf_info.mask_area(x, y)                ||
               is_bad_data(fcst_dp(x, y))                ||
               is_bad_data(obs_dp(x,y))                  ||
               (fcmn_flag && is_bad_data(fcmn_dp(x, y))) ||
               (fcsd_flag && is_bad_data(fcsd_dp(x, y))) ||
               (ocmn_flag && is_bad_data(ocmn_dp(x, y))) ||
               (ocsd_flag && is_bad_data(ocsd_dp(x, y)))) continue;

            // Store climo data
            ClimoPntInfo cpi((fcmn_flag ? fcmn_dp(x, y) : bad_data_double),
                             (fcsd_flag ? fcsd_dp(x, y) : bad_data_double),
                             (ocmn_flag ? ocmn_dp(x, y) : bad_data_double),
                             (ocsd_flag ? ocsd_dp(x, y) : bad_data_double));

            pd_block[i].add_grid_pair(fcst_dp(x, y), obs_dp(x, y),
                                      cpi, default_grid_weight);

         } // end for i
      } // end for i_series

      // Compute statistics for each grid point in the block
      for(i=0; i<conf_info.block_size && (i_point+i)<grid.nxy(); i++) {

         // Determine x,y location
         DefaultTO.one_to_two(grid.nx(), grid.ny(), i_point+i, x, y);

         // Check for the required number of matched pairs
         if(pd_block[i].f_na.n()/(double) n_series < conf_info.vld_data_thresh) {
            mlog << Debug(4)
                 << "[" << i+1 << " of " << conf_info.block_size
                 << "] Skipping point (" << x << ", " << y << ") with "
                 << pd_block[i].f_na.n() << " matched pairs.\n";

            // Keep track of the number of points skipped
            if(pd_block[i].f_na.n() == 0) n_skip_zero++;
            else                          n_skip_pos++;

            continue;
         }
         else {
            mlog << Debug(4)
                 << "[" << i+1 << " of " << conf_info.block_size
                 << "] Processing point (" << x << ", " << y << ") with "
                 << pd_block[i].n_obs << " matched pairs.\n";
         }

         // Compute contingency table counts and statistics
         if(!conf_info.fcst_info[0]->is_prob() &&
            (conf_info.output_stats[STATLineType::fho].n() +
             conf_info.output_stats[STATLineType::ctc].n() +
             conf_info.output_stats[STATLineType::cts].n()) > 0) {
            do_categorical(i_point+i, &pd_block[i]);
         }

         // Compute multi-category contingency table counts and statistics
         if(!conf_info.fcst_info[0]->is_prob() &&
            (conf_info.output_stats[STATLineType::mctc].n() +
             conf_info.output_stats[STATLineType::mcts].n()) > 0) {
            do_multicategory(i_point+i, &pd_block[i]);
         }

         // Compute continuous statistics
         if(!conf_info.fcst_info[0]->is_prob() &&
            conf_info.output_stats[STATLineType::cnt].n() > 0) {
            do_continuous(i_point+i, &pd_block[i]);
         }

         // Compute partial sums
         if(!conf_info.fcst_info[0]->is_prob() &&
            (conf_info.output_stats[STATLineType::sl1l2].n() +
             conf_info.output_stats[STATLineType::sal1l2].n()) > 0) {
            do_partialsums(i_point+i, &pd_block[i]);
         }

         // Compute probabilistics counts and statistics
         if(conf_info.fcst_info[0]->is_prob() &&
            (conf_info.output_stats[STATLineType::pct].n() +
             conf_info.output_stats[STATLineType::pstd].n() +
             conf_info.output_stats[STATLineType::pjc].n() +
             conf_info.output_stats[STATLineType::prc].n()) > 0) {
            do_probabilistic(i_point+i, &pd_block[i]);
         }
      } // end for i

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

   // Print summary counts
   mlog << Debug(2)
        << "Finished processing statistics for "
        << grid.nxy() - n_skip_zero - n_skip_pos << " of "
        << grid.nxy() << " grid points.\n"
        << "Skipped " << n_skip_zero << " of " << grid.nxy()
        << " points with no valid data.\n"
        << "Skipped " << n_skip_pos << " of " << grid.nxy()
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

void do_categorical(int n, const PairDataPoint *pd_ptr) {

   mlog << Debug(4) << "Computing Categorical Statistics.\n";

   // Allocate objects to store categorical statistics
   int n_cts = conf_info.fcat_ta.n();
   CTSInfo *cts_info = new CTSInfo [n_cts];

   // Setup CTSInfo objects
   for(int i=0; i<n_cts; i++) {
      cts_info[i].cts.set_ec_value(conf_info.hss_ec_value);
      cts_info[i].fthresh = conf_info.fcat_ta[i];
      cts_info[i].othresh = conf_info.ocat_ta[i];
      cts_info[i].allocate_n_alpha(conf_info.ci_alpha.n());

      for(int j=0; j<conf_info.ci_alpha.n(); j++) {
         cts_info[i].alpha[j] = conf_info.ci_alpha[j];
      }
   }

   // Aggregate input pair data with existing CTC counts
   if(aggr_file.nonempty()) {

      // Index NumArray to use all points
      NumArray i_na;
      i_na.add_seq(0, pd_ptr->n_obs-1);

      // Loop over the thresholds
      for(int i=0; i<n_cts; i++) {

         // Compute the current CTSInfo
         compute_ctsinfo(*pd_ptr, i_na, false, false, cts_info[i]);

         // Read the CTC data to be aggregated
         TTContingencyTable aggr_ctc;
         read_aggr_ctc(n, cts_info[i], aggr_ctc);

         // Aggregate CTC counts
         cts_info[i].cts += aggr_ctc;

         // Compute statistics and confidence intervals
         cts_info[i].compute_stats();
         cts_info[i].compute_ci();

      } // end for i
   }
   // Compute the counts, stats, normal confidence intervals, and
   // bootstrap confidence intervals
   else if(conf_info.boot_interval == BootIntervalType::BCA) {
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
   for(int i=0; i<n_cts; i++) {

      // Add statistic value for each possible FHO column
      for(int j=0; j<conf_info.output_stats[STATLineType::fho].n(); j++) {
         store_stat_fho(n, conf_info.output_stats[STATLineType::fho][j],
                        cts_info[i]);
      }

      // Add statistic value for each possible CTC column
      for(int j=0; j<conf_info.output_stats[STATLineType::ctc].n(); j++) {
         store_stat_ctc(n, conf_info.output_stats[STATLineType::ctc][j],
                        cts_info[i]);
      }

      // Add statistic value for each possible CTS column
      for(int j=0; j<conf_info.output_stats[STATLineType::cts].n(); j++) {
         store_stat_cts(n, conf_info.output_stats[STATLineType::cts][j],
                        cts_info[i]);
      }
   } // end for i

   // Deallocate memory
   if(cts_info) { delete [] cts_info; cts_info = (CTSInfo *) nullptr; }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_multicategory(int n, const PairDataPoint *pd_ptr) {

   mlog << Debug(4) << "Computing Multi-Category Statistics.\n";

   // Object to store multi-category statistics
   MCTSInfo mcts_info;

   // Setup the MCTSInfo object
   mcts_info.cts.set_size(conf_info.fcat_ta.n() + 1);
   mcts_info.cts.set_ec_value(conf_info.hss_ec_value);
   mcts_info.set_fthresh(conf_info.fcat_ta);
   mcts_info.set_othresh(conf_info.ocat_ta);

   mcts_info.allocate_n_alpha(conf_info.ci_alpha.n());
   for(int i=0; i<conf_info.ci_alpha.n(); i++) {
      mcts_info.alpha[i] = conf_info.ci_alpha[i];
   }

   // Aggregate input pair data with existing MCTC counts
   if(aggr_file.nonempty()) {

      // Index NumArray to use all points
      NumArray i_na;
      i_na.add_seq(0, pd_ptr->n_obs-1);

      // Compute the current MCTSInfo
      compute_mctsinfo(*pd_ptr, i_na, false, false, mcts_info);

      // Read the MCTC data to be aggregated
      ContingencyTable aggr_ctc;
      read_aggr_mctc(n, mcts_info, aggr_ctc);

      // Aggregate MCTC counts
      mcts_info.cts += aggr_ctc;

      // Compute statistics and confidence intervals
      mcts_info.compute_stats();
      mcts_info.compute_ci();

   }
   // Compute the counts, stats, normal confidence intervals, and
   // bootstrap confidence intervals
   else if(conf_info.boot_interval == BootIntervalType::BCA) {
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
   for(int i=0; i<conf_info.output_stats[STATLineType::mctc].n(); i++) {
      store_stat_mctc(n, conf_info.output_stats[STATLineType::mctc][i],
                      mcts_info);
   }

   // Add statistic value for each possible MCTS column
   for(int i=0; i<conf_info.output_stats[STATLineType::mcts].n(); i++) {
      store_stat_mcts(n, conf_info.output_stats[STATLineType::mcts][i],
                      mcts_info);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_continuous(int n, const PairDataPoint *pd_ptr) {
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

      // Aggregate input pair data with existing partial sums
      if(aggr_file.nonempty()) {

         // Compute partial sums from the pair data
         SL1L2Info s_info;
         s_info.fthresh = cnt_info.fthresh;
         s_info.othresh = cnt_info.othresh;
         s_info.logic   = cnt_info.logic;
         s_info.set(*pd_ptr);

         // Aggregate partial sums
         SL1L2Info aggr_psum;
         read_aggr_sl1l2(n, s_info, aggr_psum);
         s_info += aggr_psum;

         // Compute continuous statistics from partial sums
         compute_cntinfo(s_info, false, cnt_info);
      }
      // Compute continuous statistics from the pair data
      else {

         // Apply continuous filtering thresholds to subset pairs
         pd = pd_ptr->subset_pairs_cnt_thresh(cnt_info.fthresh, cnt_info.othresh,
                                              cnt_info.logic);

         // Check for no matched pairs to process
         if(pd.n_obs == 0) continue;

         // Compute the stats, normal confidence intervals, and
         // bootstrap confidence intervals
         int precip_flag = (conf_info.fcst_info[0]->is_precipitation() &&
                            conf_info.obs_info[0]->is_precipitation());

         if(conf_info.boot_interval == BootIntervalType::BCA) {
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
      }

      // Add statistic value for each possible CNT column
      for(j=0; j<conf_info.output_stats[STATLineType::cnt].n(); j++) {
         store_stat_cnt(n, conf_info.output_stats[STATLineType::cnt][j],
                        cnt_info);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_partialsums(int n, const PairDataPoint *pd_ptr) {

   mlog << Debug(4) << "Computing Scalar Partial Sums.\n";

   // Process each filtering threshold
   for(int i=0; i<conf_info.fcnt_ta.n(); i++) {

      // Store thresholds
      SL1L2Info s_info;
      s_info.fthresh = conf_info.fcnt_ta[i];
      s_info.othresh = conf_info.ocnt_ta[i];
      s_info.logic   = conf_info.cnt_logic;

      // Compute partial sums
      s_info.set(*pd_ptr);

      // Aggregate input pair data with existing partial sums
      if(aggr_file.nonempty()) {

         // Read the partial sum data to be aggregated
         SL1L2Info aggr_psum;

         // Aggregate SL1L2 partial sums
         if(conf_info.output_stats[STATLineType::sl1l2].n() > 0) {
            read_aggr_sl1l2(n, s_info, aggr_psum);
            s_info += aggr_psum;
         }

         // Aggregate SAL1L2 partial sums
         if(conf_info.output_stats[STATLineType::sal1l2].n() > 0) {
            read_aggr_sal1l2(n, s_info, aggr_psum);
            s_info += aggr_psum;
         }
      }

      // Add statistic value for each possible SL1L2 column
      for(int j=0; j<conf_info.output_stats[STATLineType::sl1l2].n(); j++) {
         store_stat_sl1l2(n, conf_info.output_stats[STATLineType::sl1l2][j], s_info);
      }

      // Add statistic value for each possible SAL1L2 column
      for(int j=0; j<conf_info.output_stats[STATLineType::sal1l2].n(); j++) {
         store_stat_sal1l2(n, conf_info.output_stats[STATLineType::sal1l2][j], s_info);
      }

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void read_aggr_ctc(int n, const CTSInfo &cts_info,
                   TTContingencyTable &aggr_ctc) {

   // Initialize
   aggr_ctc.zero_out();

   // Loop over the CTC column names
   for(int i=0; i<n_ctc_columns; i++) {

      ConcatString c(to_upper(ctc_columns[i]));
      ConcatString var_name(build_nc_var_name_ctc(c, cts_info));

      // Read aggregate data, if needed
      if(aggr_data.count(var_name) == 0) {
         aggr_data[var_name] = get_aggr_data(var_name);
      }

      // Get the n-th value
      double v = aggr_data[var_name].buf()[n];

      // Populate the CTC table
           if(c == "FY_OY")    { aggr_ctc.set_fy_oy(nint(v)); }
      else if(c == "FY_ON")    { aggr_ctc.set_fy_on(nint(v)); }
      else if(c == "FN_OY")    { aggr_ctc.set_fn_oy(nint(v)); }
      else if(c == "FN_ON")    { aggr_ctc.set_fn_on(nint(v)); }
      else if(c == "EC_VALUE") { aggr_ctc.set_ec_value(v);    }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void read_aggr_sl1l2(int n, const SL1L2Info &s_info,
                     SL1L2Info &aggr_psum) {

   // Initialize
   aggr_psum.zero_out();

   // Loop over the SL1L2 column names
   for(int i=0; i<n_sl1l2_columns; i++) {

      ConcatString c(to_upper(sl1l2_columns[i]));
      ConcatString var_name(build_nc_var_name_sl1l2(c, s_info));

      // Read aggregate data, if needed
      if(aggr_data.count(var_name) == 0) {
         aggr_data[var_name] = get_aggr_data(var_name);
      }

      // Populate the partial sums
      aggr_psum.set_sl1l2_stat(sl1l2_columns[i],
                               aggr_data[var_name].buf()[n]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void read_aggr_sal1l2(int n, const SL1L2Info &s_info,
                      SL1L2Info &aggr_psum) {

   // Initialize
   aggr_psum.zero_out();

   // Loop over the SAL1L2 column names
   for(int i=0; i<n_sal1l2_columns; i++) {

      ConcatString c(to_upper(sal1l2_columns[i]));
      ConcatString var_name(build_nc_var_name_sal1l2(c, s_info));

      // Read aggregate data, if needed
      if(aggr_data.count(var_name) == 0) {
         aggr_data[var_name] = get_aggr_data(var_name);
      }

      // Populate the partial sums
      aggr_psum.set_sal1l2_stat(sal1l2_columns[i],
                                aggr_data[var_name].buf()[n]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void read_aggr_mctc(int n, const MCTSInfo &mcts_info,
                    ContingencyTable &aggr_ctc) {

   // Initialize
   aggr_ctc = mcts_info.cts;
   aggr_ctc.zero_out();

   // Get MCTC column names
   StringArray mctc_cols(get_mctc_columns(aggr_ctc.nrows()));

   // Loop over the MCTC column names
   for(int i=0; i<mctc_cols.n(); i++) {

      ConcatString c(to_upper(mctc_cols[i]));
      ConcatString var_name("series_mctc_");
      var_name << c;

      // Read aggregate data, if needed
      if(aggr_data.count(var_name) == 0) {
         aggr_data[var_name] = get_aggr_data(var_name);
      }

      // Get the n-th value
      double v = aggr_data[var_name].buf()[n];

      // Check the number of categories
      if(c == "N_CAT" && !is_bad_data(v) &&
         aggr_ctc.nrows() != nint(v)) {
         mlog << Error << "\nread_aggr_mctc() -> "
              << "the number of MCTC categories do not match ("
              << nint(v) << " != " << aggr_ctc.nrows() << ")!\n\n";
         exit(1);
      }
      // Check the expected correct
      else if(c == "EC_VALUE" && !is_bad_data(v) &&
              !is_eq(v, aggr_ctc.ec_value(), loose_tol)) {
         mlog << Error << "\nread_aggr_mctc() -> "
              << "the MCTC expected correct values do not match ("
              << v << " != " << aggr_ctc.ec_value() << ")!\n\n";
         exit(1);
      }
      // Populate the MCTC table
      else if(check_reg_exp("F[0-9]*_O[0-9]*", c.c_str())) {
         StringArray sa(c.split("_"));
         int i_row = atoi(sa[0].c_str()+1) - 1;
         int i_col = atoi(sa[1].c_str()+1) - 1;
         aggr_ctc.set_entry(i_row, i_col, nint(v));
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void read_aggr_pct(int n, const PCTInfo &pct_info,
                   Nx2ContingencyTable &aggr_pct) {

   // Initialize
   aggr_pct = pct_info.pct;
   aggr_pct.zero_out();

   // Get PCT column names
   StringArray pct_cols(get_pct_columns(aggr_pct.nrows()));

   // Loop over the PCT colum names
   for(int i=0; i<pct_cols.n(); i++) {

      ConcatString c(to_upper(pct_cols[i]));
      ConcatString var_name("series_pct_");
      var_name << c << "_obs" << pct_info.othresh.get_abbr_str();

      // Read aggregate data, if needed
      if(aggr_data.count(var_name) == 0) {
         aggr_data[var_name] = get_aggr_data(var_name);
      }

      // Get the n-th value
      double v = aggr_data[var_name].buf()[n];

      // Check the number of thresholds
      if(c == "N_THRESH" && !is_bad_data(v) &&
         aggr_pct.nrows() != nint(v)+1) {
         mlog << Error << "\nread_aggr_pct() -> "
              << "the number of PCT categories do not match ("
              << nint(v)+1 << " != " << aggr_pct.nrows() << ")!\n\n";
         exit(1);
      }
      // Set the event counts
      else if(check_reg_exp("OY_[0-9]", c.c_str())) {

         // Parse the index value from the column name
         int i_row = atoi(strrchr(c.c_str(), '_') + 1) - 1;
         aggr_pct.set_event(i_row, nint(v));
      }
      // Set the non-event counts
      else if(check_reg_exp("ON_[0-9]", c.c_str())) {

         // Parse the index value from the column name
         int i_row = atoi(strrchr(c.c_str(), '_') + 1) - 1;
         aggr_pct.set_nonevent(i_row, nint(v));
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_probabilistic(int n, const PairDataPoint *pd_ptr) {
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

      // Aggregate input pair data with existing PCT counts
      if(aggr_file.nonempty()) {

         // Compute the probabilistic counts
         compute_pctinfo(*pd_ptr, false, pct_info);

         // Read the PCT data to be aggregated
         Nx2ContingencyTable aggr_pct;
         read_aggr_pct(n, pct_info, aggr_pct);

         // Aggregate PCT counts
         pct_info.pct += aggr_pct;

         // Zero out the climatology PCT table which cannot be aggregated
         pct_info.climo_pct.zero_out();

         // Compute statistics and confidence intervals
         pct_info.compute_stats();
         pct_info.compute_ci();

      }
      // Compute the probabilistic counts and statistics
      else {
         compute_pctinfo(*pd_ptr, true, pct_info);
      }

      // Add statistic value for each possible PCT column
      for(j=0; j<conf_info.output_stats[STATLineType::pct].n(); j++) {
         store_stat_pct(n, conf_info.output_stats[STATLineType::pct][j],
                        pct_info);
      }

      // Add statistic value for each possible PSTD column
      for(j=0; j<conf_info.output_stats[STATLineType::pstd].n(); j++) {
         store_stat_pstd(n, conf_info.output_stats[STATLineType::pstd][j],
                         pct_info);
      }

      // Add statistic value for each possible PJC column
      for(j=0; j<conf_info.output_stats[STATLineType::pjc].n(); j++) {
         store_stat_pjc(n, conf_info.output_stats[STATLineType::pjc][j],
                        pct_info);
      }

      // Add statistic value for each possible PRC column
      for(j=0; j<conf_info.output_stats[STATLineType::prc].n(); j++) {
         store_stat_prc(n, conf_info.output_stats[STATLineType::prc][j],
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
   double v;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Handle ALL columns
   if(c == all_columns) return store_stat_all_ctc(n, cts_info);

   // Get the column value
        if(c == "TOTAL")    { v = cts_info.cts.n();        }
   else if(c == "FY_OY")    { v = cts_info.cts.fy_oy();    }
   else if(c == "FY_ON")    { v = cts_info.cts.fy_on();    }
   else if(c == "FN_OY")    { v = cts_info.cts.fn_oy();    }
   else if(c == "FN_ON")    { v = cts_info.cts.fn_on();    }
   else if(c == "EC_VALUE") { v = cts_info.cts.ec_value(); }
   else {
     mlog << Error << "\nstore_stat_ctc() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Construct the NetCDF variable name
   ConcatString var_name(build_nc_var_name_ctc(c, cts_info));

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      ConcatString lty_stat("CTC_");
      lty_stat << c;

      // Add new map entry
      add_nc_var(var_name, c, stat_long_name[lty_stat],
                 cts_info.fthresh.get_str(),
                 cts_info.othresh.get_str(),
                 bad_data_double);
   }

   // Store the statistic value
   put_nc_val(n, var_name, v);

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
           if(c == "TOTAL")      { v = (double) cts_info.cts.n(); }
      else if(c == "BASER")      { v = cts_info.baser.v;          }
      else if(c == "BASER_NCL")  { v = cts_info.baser.v_ncl[i];   }
      else if(c == "BASER_NCU")  { v = cts_info.baser.v_ncu[i];   }
      else if(c == "BASER_BCL")  { v = cts_info.baser.v_bcl[i];   }
      else if(c == "BASER_BCU")  { v = cts_info.baser.v_bcu[i];   }
      else if(c == "FMEAN")      { v = cts_info.fmean.v;          }
      else if(c == "FMEAN_NCL")  { v = cts_info.fmean.v_ncl[i];   }
      else if(c == "FMEAN_NCU")  { v = cts_info.fmean.v_ncu[i];   }
      else if(c == "FMEAN_BCL")  { v = cts_info.fmean.v_bcl[i];   }
      else if(c == "FMEAN_BCU")  { v = cts_info.fmean.v_bcu[i];   }
      else if(c == "ACC")        { v = cts_info.acc.v;            }
      else if(c == "ACC_NCL")    { v = cts_info.acc.v_ncl[i];     }
      else if(c == "ACC_NCU")    { v = cts_info.acc.v_ncu[i];     }
      else if(c == "ACC_BCL")    { v = cts_info.acc.v_bcl[i];     }
      else if(c == "ACC_BCU")    { v = cts_info.acc.v_bcu[i];     }
      else if(c == "FBIAS")      { v = cts_info.fbias.v;          }
      else if(c == "FBIAS_BCL")  { v = cts_info.fbias.v_bcl[i];   }
      else if(c == "FBIAS_BCU")  { v = cts_info.fbias.v_bcu[i];   }
      else if(c == "PODY")       { v = cts_info.pody.v;           }
      else if(c == "PODY_NCL")   { v = cts_info.pody.v_ncl[i];    }
      else if(c == "PODY_NCU")   { v = cts_info.pody.v_ncu[i];    }
      else if(c == "PODY_BCL")   { v = cts_info.pody.v_bcl[i];    }
      else if(c == "PODY_BCU")   { v = cts_info.pody.v_bcu[i];    }
      else if(c == "PODN")       { v = cts_info.podn.v;           }
      else if(c == "PODN_NCL")   { v = cts_info.podn.v_ncl[i];    }
      else if(c == "PODN_NCU")   { v = cts_info.podn.v_ncu[i];    }
      else if(c == "PODN_BCL")   { v = cts_info.podn.v_bcl[i];    }
      else if(c == "PODN_BCU")   { v = cts_info.podn.v_bcu[i];    }
      else if(c == "POFD")       { v = cts_info.pofd.v;           }
      else if(c == "POFD_NCL")   { v = cts_info.pofd.v_ncl[i];    }
      else if(c == "POFD_NCU")   { v = cts_info.pofd.v_ncu[i];    }
      else if(c == "POFD_BCL")   { v = cts_info.pofd.v_bcl[i];    }
      else if(c == "POFD_BCU")   { v = cts_info.pofd.v_bcu[i];    }
      else if(c == "FAR")        { v = cts_info.far.v;            }
      else if(c == "FAR_NCL")    { v = cts_info.far.v_ncl[i];     }
      else if(c == "FAR_NCU")    { v = cts_info.far.v_ncu[i];     }
      else if(c == "FAR_BCL")    { v = cts_info.far.v_bcl[i];     }
      else if(c == "FAR_BCU")    { v = cts_info.far.v_bcu[i];     }
      else if(c == "CSI")        { v = cts_info.csi.v;            }
      else if(c == "CSI_NCL")    { v = cts_info.csi.v_ncl[i];     }
      else if(c == "CSI_NCU")    { v = cts_info.csi.v_ncu[i];     }
      else if(c == "CSI_BCL")    { v = cts_info.csi.v_bcl[i];     }
      else if(c == "CSI_BCU")    { v = cts_info.csi.v_bcu[i];     }
      else if(c == "GSS")        { v = cts_info.gss.v;            }
      else if(c == "GSS_BCL")    { v = cts_info.gss.v_bcl[i];     }
      else if(c == "GSS_BCU")    { v = cts_info.gss.v_bcu[i];     }
      else if(c == "HK")         { v = cts_info.hk.v;             }
      else if(c == "HK_NCL")     { v = cts_info.hk.v_ncl[i];      }
      else if(c == "HK_NCU")     { v = cts_info.hk.v_ncu[i];      }
      else if(c == "HK_BCL")     { v = cts_info.hk.v_bcl[i];      }
      else if(c == "HK_BCU")     { v = cts_info.hk.v_bcu[i];      }
      else if(c == "HSS")        { v = cts_info.hss.v;            }
      else if(c == "HSS_BCL")    { v = cts_info.hss.v_bcl[i];     }
      else if(c == "HSS_BCU")    { v = cts_info.hss.v_bcu[i];     }
      else if(c == "ODDS")       { v = cts_info.odds.v;           }
      else if(c == "ODDS_NCL")   { v = cts_info.odds.v_ncl[i];    }
      else if(c == "ODDS_NCU")   { v = cts_info.odds.v_ncu[i];    }
      else if(c == "ODDS_BCL")   { v = cts_info.odds.v_bcl[i];    }
      else if(c == "ODDS_BCU")   { v = cts_info.odds.v_bcu[i];    }
      else if(c == "LODDS")      { v = cts_info.lodds.v;          }
      else if(c == "LODDS_NCL")  { v = cts_info.lodds.v_ncl[i];   }
      else if(c == "LODDS_NCU")  { v = cts_info.lodds.v_ncu[i];   }
      else if(c == "LODDS_BCL")  { v = cts_info.lodds.v_bcl[i];   }
      else if(c == "LODDS_BCU")  { v = cts_info.lodds.v_bcu[i];   }
      else if(c == "ORSS")       { v = cts_info.orss.v;           }
      else if(c == "ORSS_NCL")   { v = cts_info.orss.v_ncl[i];    }
      else if(c == "ORSS_NCU")   { v = cts_info.orss.v_ncu[i];    }
      else if(c == "ORSS_BCL")   { v = cts_info.orss.v_bcl[i];    }
      else if(c == "ORSS_BCU")   { v = cts_info.orss.v_bcu[i];    }
      else if(c == "EDS")        { v = cts_info.eds.v;            }
      else if(c == "EDS_NCL")    { v = cts_info.eds.v_ncl[i];     }
      else if(c == "EDS_NCU")    { v = cts_info.eds.v_ncu[i];     }
      else if(c == "EDS_BCL")    { v = cts_info.eds.v_bcl[i];     }
      else if(c == "EDS_BCU")    { v = cts_info.eds.v_bcu[i];     }
      else if(c == "SEDS")       { v = cts_info.seds.v;           }
      else if(c == "SEDS_NCL")   { v = cts_info.seds.v_ncl[i];    }
      else if(c == "SEDS_NCU")   { v = cts_info.seds.v_ncu[i];    }
      else if(c == "SEDS_BCL")   { v = cts_info.seds.v_bcl[i];    }
      else if(c == "SEDS_BCU")   { v = cts_info.seds.v_bcu[i];    }
      else if(c == "EDI")        { v = cts_info.edi.v;            }
      else if(c == "EDI_NCL")    { v = cts_info.edi.v_ncl[i];     }
      else if(c == "EDI_NCU")    { v = cts_info.edi.v_ncu[i];     }
      else if(c == "EDI_BCL")    { v = cts_info.edi.v_bcl[i];     }
      else if(c == "EDI_BCU")    { v = cts_info.edi.v_bcu[i];     }
      else if(c == "SEDI")       { v = cts_info.sedi.v;           }
      else if(c == "SEDI_NCL")   { v = cts_info.sedi.v_ncl[i];    }
      else if(c == "SEDI_NCU")   { v = cts_info.sedi.v_ncu[i];    }
      else if(c == "SEDI_BCL")   { v = cts_info.sedi.v_bcl[i];    }
      else if(c == "SEDI_BCU")   { v = cts_info.sedi.v_bcu[i];    }
      else if(c == "BAGSS")      { v = cts_info.bagss.v;          }
      else if(c == "BAGSS_BCL")  { v = cts_info.bagss.v_bcl[i];   }
      else if(c == "BAGSS_BCU")  { v = cts_info.bagss.v_bcu[i];   }
      else if(c == "HSS_EC")     { v = cts_info.hss_ec.v;         }
      else if(c == "HSS_EC_BCL") { v = cts_info.hss_ec.v_bcl[i];  }
      else if(c == "HSS_EC_BCU") { v = cts_info.hss_ec.v_bcu[i];  }
      else if(c == "EC_VALUE")   { v = cts_info.cts.ec_value();   }
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

   // Handle ALL columns
   if(c == all_columns) return store_stat_all_mctc(n, mcts_info);

   // Get the column value
        if(c == "TOTAL")    { v = (double) mcts_info.cts.total();    }
   else if(c == "N_CAT")    { v = (double) mcts_info.cts.nrows();    }
   else if(c == "EC_VALUE") { v =          mcts_info.cts.ec_value(); }
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
           if(c == "TOTAL")      { v = (double) mcts_info.cts.total(); }
      else if(c == "N_CAT")      { v = (double) mcts_info.cts.nrows(); }
      else if(c == "ACC")        { v = mcts_info.acc.v;                }
      else if(c == "ACC_NCL")    { v = mcts_info.acc.v_ncl[i];         }
      else if(c == "ACC_NCU")    { v = mcts_info.acc.v_ncu[i];         }
      else if(c == "ACC_BCL")    { v = mcts_info.acc.v_bcl[i];         }
      else if(c == "ACC_BCU")    { v = mcts_info.acc.v_bcu[i];         }
      else if(c == "HK")         { v = mcts_info.hk.v;                 }
      else if(c == "HK_BCL")     { v = mcts_info.hk.v_bcl[i];          }
      else if(c == "HK_BCU")     { v = mcts_info.hk.v_bcu[i];          }
      else if(c == "HSS")        { v = mcts_info.hss.v;                }
      else if(c == "HSS_BCL")    { v = mcts_info.hss.v_bcl[i];         }
      else if(c == "HSS_BCU")    { v = mcts_info.hss.v_bcu[i];         }
      else if(c == "GER")        { v = mcts_info.ger.v;                }
      else if(c == "GER_BCL")    { v = mcts_info.ger.v_bcl[i];         }
      else if(c == "GER_BCU")    { v = mcts_info.ger.v_bcu[i];         }
      else if(c == "HSS_EC")     { v = mcts_info.hss_ec.v;             }
      else if(c == "HSS_EC_BCL") { v = mcts_info.hss_ec.v_bcl[i];      }
      else if(c == "HSS_EC_BCU") { v = mcts_info.hss_ec.v_bcu[i];      }
      else if(c == "EC_VALUE")   { v = mcts_info.cts.ec_value();       }
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
      else if(c == "SI")                   { v = cnt_info.si.v;                      }
      else if(c == "SI_BCL")               { v = cnt_info.si.v_bcl[i];               }
      else if(c == "SI_BCU")               { v = cnt_info.si.v_bcu[i];               }
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
      else if(c == "SI")                   { v = cnt_info.si.v;                      }
      else if(c == "SI_BCL")               { v = cnt_info.si.v_bcl[i];               }
      else if(c == "SI_BCU")               { v = cnt_info.si.v_bcu[i];               }
      else {
        mlog << Error << "\nstore_stat_cnt() -> "
             << "unsupported column name requested \"" << c
             << "\"\n\n";
        exit(1);
      }

      // Construct the NetCDF variable name
      ConcatString var_name(build_nc_var_name_cnt(c, cnt_info));

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

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Handle ALL columns
   if(c == all_columns) return store_stat_all_sl1l2(n, s_info);

   // Get the column value
        if(c == "TOTAL") { v = (double) s_info.scount; }
   else if(c == "FBAR")  { v = s_info.fbar;            }
   else if(c == "OBAR")  { v = s_info.obar;            }
   else if(c == "FOBAR") { v = s_info.fobar;           }
   else if(c == "FFBAR") { v = s_info.ffbar;           }
   else if(c == "OOBAR") { v = s_info.oobar;           }
   else if(c == "MAE")   { v = s_info.smae;            }
   else {
     mlog << Error << "\nstore_stat_sl1l2() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Construct the NetCDF variable name
   ConcatString var_name(build_nc_var_name_sl1l2(c, s_info));

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      ConcatString lty_stat("SL1L2_");
      lty_stat << c;

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

void store_stat_sal1l2(int n, const ConcatString &col,
                       const SL1L2Info &s_info) {
   double v;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Handle ALL columns
   if(c == all_columns) return store_stat_all_sal1l2(n, s_info);

   // Get the column value
        if(c == "TOTAL")  { v = (double) s_info.sacount; }
   else if(c == "FABAR")  { v = s_info.fabar;            }
   else if(c == "OABAR")  { v = s_info.oabar;            }
   else if(c == "FOABAR") { v = s_info.foabar;           }
   else if(c == "FFABAR") { v = s_info.ffabar;           }
   else if(c == "OOABAR") { v = s_info.ooabar;           }
   else if(c == "MAE")    { v = s_info.samae;            }
   else {
     mlog << Error << "\nstore_stat_sal1l2() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Construct the NetCDF variable name
   ConcatString var_name(build_nc_var_name_sal1l2(c, s_info));

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      ConcatString lty_stat("SAL1L2_");
      lty_stat << c;

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

   // Handle ALL columns
   if(c == all_columns) return store_stat_all_pct(n, pct_info);

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
      else if(c == "BRIERCL")     { v = pct_info.briercl.v;                }
      else if(c == "BRIERCL_NCL") { v = pct_info.briercl.v_ncl[i];         }
      else if(c == "BRIERCL_NCU") { v = pct_info.briercl.v_ncu[i];         }
      else if(c == "BSS")         { v = pct_info.bss;                      }
      else if(c == "BSS_SMPL")    { v = pct_info.bss_smpl;                 }
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

void store_stat_all_ctc(int n, const CTSInfo &cts_info) {
   for(int i=0; i<n_ctc_columns; i++) {
      store_stat_ctc(n, ctc_columns[i], cts_info);
   }
}

////////////////////////////////////////////////////////////////////////

void store_stat_all_mctc(int n, const MCTSInfo &mcts_info) {
   StringArray mctc_cols(get_mctc_columns(mcts_info.cts.nrows()));
   for(int i=0; i<mctc_cols.n(); i++) {
      store_stat_mctc(n, mctc_cols[i], mcts_info);
   }
}

////////////////////////////////////////////////////////////////////////

void store_stat_all_sl1l2(int n, const SL1L2Info &s_info) {
   for(int i=0; i<n_sl1l2_columns; i++) {
      store_stat_sl1l2(n, sl1l2_columns[i], s_info);
   }
}

////////////////////////////////////////////////////////////////////////

void store_stat_all_sal1l2(int n, const SL1L2Info &s_info) {
   for(int i=0; i<n_sal1l2_columns; i++) {
      store_stat_sal1l2(n, sal1l2_columns[i], s_info);
   }
}

////////////////////////////////////////////////////////////////////////

void store_stat_all_pct(int n, const PCTInfo &pct_info) {
   StringArray pct_cols(get_pct_columns(pct_info.pct.nrows() + 1));
   for(int i=0; i<pct_cols.n(); i++) {
      store_stat_pct(n, pct_cols[i], pct_info);
   }
}

////////////////////////////////////////////////////////////////////////

ConcatString build_nc_var_name_ctc(const ConcatString &col,
                                   const CTSInfo &cts_info) {

   // Append the column name
   ConcatString var_name("series_ctc_");
   var_name << col;

   // Append threshold information
   if(cts_info.fthresh == cts_info.othresh) {
      var_name << "_" << cts_info.fthresh.get_abbr_str();
   }
   else {
      var_name << "_fcst" << cts_info.fthresh.get_abbr_str()
               << "_obs" << cts_info.othresh.get_abbr_str();
   }

   return var_name;
}

////////////////////////////////////////////////////////////////////////

ConcatString build_nc_var_name_sl1l2(const ConcatString &col,
                                     const SL1L2Info &s_info) {

   // Append the column name
   ConcatString var_name("series_sl1l2_");
   var_name << col;

   // Append threshold information, if supplied
   if(s_info.fthresh.get_type() != thresh_na ||
      s_info.othresh.get_type() != thresh_na) {
      var_name << "_fcst" << s_info.fthresh.get_abbr_str()
               << "_" << setlogic_to_abbr(s_info.logic)
               << "_obs" << s_info.othresh.get_abbr_str();
   }

   return var_name;
}

////////////////////////////////////////////////////////////////////////

ConcatString build_nc_var_name_sal1l2(const ConcatString &col,
                                      const SL1L2Info &s_info) {

   // Append the column name
   ConcatString var_name("series_sal1l2_");
   var_name << col;

   // Append threshold information, if supplied
   if(s_info.fthresh.get_type() != thresh_na ||
      s_info.othresh.get_type() != thresh_na) {
      var_name << "_fcst" << s_info.fthresh.get_abbr_str()
               << "_" << setlogic_to_abbr(s_info.logic)
               << "_obs" << s_info.othresh.get_abbr_str();
   }

   return var_name;
}

////////////////////////////////////////////////////////////////////////

ConcatString build_nc_var_name_cnt(const ConcatString &col,
                                   const CNTInfo &cnt_info) {

   // Append the column name
   ConcatString var_name("series_cnt_");
   var_name << col;

   // Append threshold information, if supplied
   if(cnt_info.fthresh.get_type() != thresh_na ||
      cnt_info.othresh.get_type() != thresh_na) {
      var_name << "_fcst" << cnt_info.fthresh.get_abbr_str()
               << "_" << setlogic_to_abbr(cnt_info.logic)
               << "_obs" << cnt_info.othresh.get_abbr_str();
   }

   return var_name;
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
   write_netcdf_proj(nc_out, grid, lat_dim, lon_dim);

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
      nc_out = (NcFile *) nullptr;
   }

   // Close the aggregate NetCDF file
   if(aggr_nc.MetNc) aggr_nc.close();

   // Deallocate memory for data files
   if(fcst_mtddf) { delete fcst_mtddf; fcst_mtddf = nullptr; }
   if(obs_mtddf)  { delete obs_mtddf;  obs_mtddf  = nullptr; }

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
        << "\t[-aggr file]\n"
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

        << "\t\t\"-aggr file\" specifies a series_analysis output "
        << "file with partial sums and/or contingency table counts to be "
        << "updated prior to deriving statistics (optional).\n"

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

void set_aggr(const StringArray & a) {
   aggr_file = a[0];
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
