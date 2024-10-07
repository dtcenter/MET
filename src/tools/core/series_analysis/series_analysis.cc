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

static void open_aggr_file();
static DataPlane read_aggr_data_plane(const ConcatString &,
                                      const char *suggestion=nullptr);

static void process_scores();

static void do_categorical   (int, const PairDataPoint *);
static void do_multicategory (int, const PairDataPoint *);
static void do_continuous    (int, const PairDataPoint *);
static void do_partialsums   (int, const PairDataPoint *);
static void do_probabilistic (int, const PairDataPoint *);
static void do_climo_brier   (int, double, int, PCTInfo &);

static int  read_aggr_total  (int);
static void read_aggr_ctc    (int, const CTSInfo &,   CTSInfo &);
static void read_aggr_mctc   (int, const MCTSInfo &,  MCTSInfo &);
static void read_aggr_sl1l2  (int, const SL1L2Info &, SL1L2Info &);
static void read_aggr_sal1l2 (int, const SL1L2Info &, SL1L2Info &);
static void read_aggr_pct    (int, const PCTInfo &,   PCTInfo &);

static void store_stat_categorical(int,
               STATLineType, const ConcatString &,
               const CTSInfo &);
static void store_stat_multicategory(int,
               STATLineType, const ConcatString &,
               const MCTSInfo &);
static void store_stat_partialsums(int,
               STATLineType, const ConcatString &,
               const SL1L2Info &);
static void store_stat_continuous(int,
               STATLineType, const ConcatString &,
               const CNTInfo &);
static void store_stat_probabilistic(int,
               STATLineType, const ConcatString &,
               const PCTInfo &);

static void store_stat_all_ctc   (int, const CTSInfo &);
static void store_stat_all_mctc  (int, const MCTSInfo &);
static void store_stat_all_sl1l2 (int, const SL1L2Info &);
static void store_stat_all_sal1l2(int, const SL1L2Info &);
static void store_stat_all_pct   (int, const PCTInfo &);

static ConcatString build_nc_var_name_categorical(
                       STATLineType, const ConcatString &,
                       const CTSInfo &, double);
static ConcatString build_nc_var_name_multicategory(
                       STATLineType, const ConcatString &,
                       double);
static ConcatString build_nc_var_name_partialsums(
                       STATLineType, const ConcatString &,
                       const SL1L2Info &);
static ConcatString build_nc_var_name_continuous(
                       STATLineType, const ConcatString &,
                       const CNTInfo &, double);
static ConcatString build_nc_var_name_probabilistic(
                       STATLineType, const ConcatString &,
                       const PCTInfo &, double);

static void setup_nc_file(const VarInfo *, const VarInfo *);
static void add_stat_data(const ConcatString &, const ConcatString &,
                          const ConcatString &, const ConcatString &,
                          const ConcatString &, double);
static void write_stat_data();

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
           << R"("-fcst" or "-both" option.)" << "\n\n";
      usage();
   }
   if(obs_files.n() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the observation file list must be set using the "
           << R"("-obs" or "-both" option.)" << "\n\n";
      usage();
   }
   if(config_file.length() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the configuration file must be set using the "
           << R"("-config" option.)" << "\n\n";
      usage();
   }
   if(out_file.length() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the output NetCDF file must be set using the "
           << R"("-out" option.)" << "\n\n";
      usage();
   }
   if(aggr_file == out_file) {
      mlog << Error << "\nprocess_command_line() -> "
           << R"(the "-out" and "-aggr" options cannot be )"
           << R"(set to the same file (")" << aggr_file
           << R"(")!)" << "\n\n";
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
        << R"(Length of configuration "fcst.field" = )"
        << conf_info.get_n_fcst() << "\n"
        << R"(Length of configuration "obs.field"  = )"
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
      n_series_pair = conf_info.get_n_fcst();
      mlog << Debug(1)
           << R"(Series defined by the "fcst.field" configuration entry )"
           << "of length " << n_series_pair << ".\n";
   }
   else if(conf_info.get_n_obs() > 1) {
      series_type = SeriesType::Obs_Conf;
      n_series_pair = conf_info.get_n_obs();
      mlog << Debug(1)
           << R"(Series defined by the "obs.field" configuration entry )"
           << "of length " << n_series_pair << ".\n";
   }
   else if(fcst_files.n() > 1) {
      series_type = SeriesType::Fcst_Files;
      n_series_pair = fcst_files.n();
      mlog << Debug(1)
           << "Series defined by the forecast file list of length "
           << n_series_pair << ".\n";
   }
   else if(obs_files.n() > 1) {
      series_type = SeriesType::Obs_Files;
      n_series_pair = obs_files.n();
      mlog << Debug(1)
           << "Series defined by the observation file list of length "
           << n_series_pair << ".\n";
   }
   else {
      series_type = SeriesType::Fcst_Conf;
      n_series_pair = 1;
      mlog << Debug(1)
           << R"(The "fcst.field" and "obs.field" configuration entries )"
           << R"(and the "-fcst" and "-obs" command line options )"
           << "all have length one.\n";
   }

   // If paired, check for consistent settings.
   if(paired) {

      // The number of forecast and observation files must match.
      if(fcst_files.n() != obs_files.n()) {
         mlog << Error << "\nprocess_command_line() -> "
              << R"(when using the "-paired" command line option, the )"
              << "number of forecast (" << fcst_files.n()
              << ") and observation (" << obs_files.n()
              << ") files must match.\n\n";
         usage();
      }

      // The number of files must match the series length.
      if(fcst_files.n() != n_series_pair) {
         mlog << Error << "\nprocess_command_line() -> "
              << R"(when using the "-paired" command line option, the )"
              << "file list length (" << fcst_files.n()
              << ") and series length (" << n_series_pair
              << ") must match.\n\n";
         usage();
      }

      // Set the series file names to the input file lists
      for(i=0; i<n_series_pair; i++) {
         found_fcst_files.add(fcst_files[i]);
         found_obs_files.add(obs_files[i]);
      }
   }
   // If not paired, initialize the series file names.
   else {
      for(i=0; i<n_series_pair; i++) {
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
           << R"(Consider increasing "block_size" in the configuration )"
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
      mlog << Error << "\nTrouble reading data file: "
           << file_list[i] << "\n\n";
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
        << n_series_pair << ": " << fcst_info->magic_str()
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
              << R"("regrid" section.)" << "\n\n";
         exit(1);
      }

      mlog << Debug(2)
           << "Regridding forecast " << fcst_info->magic_str()
           << " to the verification grid using "
           << fcst_info->regrid().get_str() << ".\n";
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
              << R"("regrid" section.)" << "\n\n";
         exit(1);
      }

      mlog << Debug(2)
           << "Regridding observation " << obs_info->magic_str()
           << " to the verification grid using "
           << obs_info->regrid().get_str() << ".\n";
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
         mlog << Debug(3)
              << cs << "\n";
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
   bool found = false;

   // Initialize
   dp.clear();

   // If not already found, search for a matching file
   if(found_files[i_series].length() == 0) {

      // Loop through the file list
      for(int i=0; i<search_files.n(); i++) {

         // Start the search with the value of i_series
         int j = (i_series + i) % search_files.n();

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
         for(int i=0; i<search_files.n(); i++)
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

void open_aggr_file() {

   mlog << Debug(1)
        << "Reading aggregate data file: " << aggr_file << "\n";

   if(!aggr_nc.open(aggr_file.c_str())) {
      mlog << Error << "\nopen_aggr_file() -> "
           << "unable to open the aggregate NetCDF file: "
           << aggr_file << "\n\n";
      exit(1);
   }

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

   // Store the aggregate series length
   n_series_aggr = get_int_var(aggr_nc.MetNc->Nc, n_series_var_name, 0);

   mlog << Debug(3)
        << "Aggregation series has length " << n_series_aggr << ".\n";

   return;
}

////////////////////////////////////////////////////////////////////////

DataPlane read_aggr_data_plane(const ConcatString &var_name,
                               const char *suggestion) {
   DataPlane aggr_dp;

   // Setup the data request
   VarInfoNcMet aggr_info;
   aggr_info.set_magic(var_name, "(*,*)");

   mlog << Debug(2)
        << R"(Reading aggregation ")"
        << aggr_info.magic_str()
        << R"(" field.)" << "\n";

   // Attempt to read the gridded data from the current file
   if(!aggr_nc.data_plane(aggr_info, aggr_dp)) {
      mlog << Error << "\nread_aggr_data_plane() -> "
           << R"(Required variable ")" << aggr_info.magic_str()
           << R"(" not found in the aggregate file!)" << "\n\n";
      if(suggestion) {
         mlog << Error
              << R"(Recommend recreating ")" << aggr_file
              << R"(" to request that )" << suggestion
              << " column(s) be written.\n\n";
      }
      exit(1);
   }

   // Check that the grid has not changed
   if(aggr_nc.grid().nx() != grid.nx() ||
      aggr_nc.grid().ny() != grid.ny()) {
      mlog << Error << "\nread_aggr_data_plane() -> "
           << "the input grid dimensions (" << grid.nx() << ", " << grid.ny()
           << ") and aggregate grid dimensions (" << aggr_nc.grid().nx()
           << ", " << aggr_nc.grid().ny() << ") do not match!\n\n";
      exit(1);
   }

   return aggr_dp;
}

////////////////////////////////////////////////////////////////////////

void process_scores() {
   int x;
   int y;
   int i_point = 0;
   VarInfo *fcst_info = (VarInfo *) nullptr;
   VarInfo *obs_info  = (VarInfo *) nullptr;
   DataPlane fcst_dp;
   DataPlane obs_dp;
   vector<PairDataPoint> pd_block;
   const char *method_name = "process_scores() ";

   // Climatology mean and standard deviation
   DataPlane fcmn_dp, fcsd_dp;
   DataPlane ocmn_dp, ocsd_dp;

   // Open the aggregate file, if needed
   if(aggr_file.nonempty()) open_aggr_file();

   // Number of points skipped due to valid data threshold
   int n_skip_zero_vld = 0;
   int n_skip_some_vld = 0;

   // Loop over the data reads
   for(int i_read=0; i_read<n_reads; i_read++) {

      // Loop over the series variable
      for(int i_series=0; i_series<n_series_pair; i_series++) {

         // Get the index for the VarInfo objects
         int i_fcst = (conf_info.get_n_fcst() > 1 ? i_series : 0);
         int i_obs  = (conf_info.get_n_obs()  > 1 ? i_series : 0);

         // Store the current VarInfo objects
         fcst_info = (conf_info.get_n_fcst() > 1 ?
                      conf_info.fcst_info[i_series] :
                      conf_info.fcst_info[0]);
         obs_info  = (conf_info.get_n_obs() > 1 ?
                      conf_info.obs_info[i_series] :
                      conf_info.obs_info[0]);

         // Retrieve the data planes for the current series entry
         get_series_data(i_series, fcst_info, obs_info, fcst_dp, obs_dp);

         // Initialize PairDataPoint vector, if needed
         // block_size is defined in get_series_data()
         if(pd_block.empty()) {
            pd_block.resize(conf_info.block_size);
            for(auto &pd : pd_block) pd.extend(n_series_pair);
         }

         // Beginning of each data pass
         if(i_series == 0) {

            // Re-initialize the PairDataPoint objects
            for(auto &pd : pd_block) {
               pd.erase();
               pd.set_climo_cdf_info_ptr(&conf_info.cdf_info);
            }

            // Starting grid point
            i_point = i_read*conf_info.block_size;

            mlog << Debug(2)
                 << "Processing data pass number " << i_read + 1 << " of "
                 << n_reads << " for grid points " << i_point + 1 << " to "
                 << min(i_point + conf_info.block_size, grid.nxy()) << ".\n";
         }

         // Read forecast climatology data
         fcmn_dp = read_climo_data_plane(
                      conf_info.conf.lookup_dictionary(conf_key_fcst),
                      conf_key_climo_mean,
                      i_fcst, fcst_dp.valid(), grid,
                      "forecast climatology mean");
         fcsd_dp = read_climo_data_plane(
                      conf_info.conf.lookup_dictionary(conf_key_fcst),
                      conf_key_climo_stdev,
                      i_fcst, fcst_dp.valid(), grid,
                      "forecast climatology standard deviation");

         // Read observation climatology data
         ocmn_dp = read_climo_data_plane(
                      conf_info.conf.lookup_dictionary(conf_key_obs),
                      conf_key_climo_mean,
                      i_obs, fcst_dp.valid(), grid,
                      "observation climatology mean");
         ocsd_dp = read_climo_data_plane(
                      conf_info.conf.lookup_dictionary(conf_key_obs),
                      conf_key_climo_stdev,
                      i_obs, fcst_dp.valid(), grid,
                      "observation climatology standard deviation");

         bool fcmn_flag = !fcmn_dp.is_empty();
         bool fcsd_flag = !fcsd_dp.is_empty();
         bool ocmn_flag = !ocmn_dp.is_empty();
         bool ocsd_flag = !ocsd_dp.is_empty();

         mlog << Debug(3)
              << "For " << fcst_info->magic_str() << ", found "
              << (fcmn_flag ? 1 : 0) << " forecast climatology mean and "
              << (fcsd_flag ? 1 : 0) << " standard deviation field(s), and "
              << (ocmn_flag ? 1 : 0) << " observation climatology mean and "
              << (ocsd_flag ? 1 : 0) << " standard deviation field(s).\n";

         // Setup the output NetCDF file on the first pass
         if(!nc_out) setup_nc_file(fcst_info, obs_info);

         // Update timing info
         set_range(fcst_dp.init(),  fcst_init_beg,  fcst_init_end);
         set_range(fcst_dp.valid(), fcst_valid_beg, fcst_valid_end);
         set_range(fcst_dp.lead(),  fcst_lead_beg,  fcst_lead_end);
         set_range(obs_dp.init(),   obs_init_beg,   obs_init_end);
         set_range(obs_dp.valid(),  obs_valid_beg,  obs_valid_end);
         set_range(obs_dp.lead(),   obs_lead_beg,   obs_lead_end);

         // Store matched pairs for each grid point
         for(int i=0; i<conf_info.block_size && (i_point+i)<grid.nxy(); i++) {

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
      for(int i=0; i<conf_info.block_size && (i_point+i)<grid.nxy(); i++) {

         // Determine x,y location
         DefaultTO.one_to_two(grid.nx(), grid.ny(), i_point+i, x, y);

         // Compute the total number of valid points and series length
         int n_valid  = pd_block[i].f_na.n() +
                        (aggr_file.empty() ? 0 : read_aggr_total(i_point+i));
         int n_series = n_series_pair + n_series_aggr;

         // Check for the required number of matched pairs
         if(n_valid / (double) n_series < conf_info.vld_data_thresh) {
            mlog << Debug(4)
                 << "[" << i+1 << " of " << conf_info.block_size
                 << "] Skipping point (" << x << ", " << y << ") with "
                 << n_valid << " of " << n_series << " valid matched pairs.\n";

            // Keep track of the number of points skipped
            if(n_valid == 0) n_skip_zero_vld++;
            else             n_skip_some_vld++;

            continue;
         }
         else {
            mlog << Debug(4)
                 << "[" << i+1 << " of " << conf_info.block_size
                 << "] Processing point (" << x << ", " << y << ") with "
                 << n_valid << " of " << n_series << " valid matched pairs.\n";
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

   // Write the computed statistics
   write_stat_data();

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
        << grid.nxy() - n_skip_zero_vld - n_skip_some_vld << " of "
        << grid.nxy() << " grid points.\n"
        << "Skipped " << n_skip_zero_vld << " of " << grid.nxy()
        << " points with no valid data.\n"
        << "Skipped " << n_skip_some_vld << " of " << grid.nxy()
        << " points that did not meet the valid data threshold.\n";

   // Print config file suggestions about missing data
   if(n_skip_some_vld > 0 && conf_info.vld_data_thresh == 1.0) {
      mlog << Debug(2)
           << "Some points skipped due to missing data:\n"
           << R"(Consider decreasing "vld_thresh" in the config file )"
           << "to include more points.\n"
           << R"(Consider requesting "TOTAL" from "output_stats" )"
           << "in the config file to see the valid data counts.\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_categorical(int n, const PairDataPoint *pd_ptr) {

   mlog << Debug(4)
        << "Computing Categorical Statistics.\n";

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
         CTSInfo aggr_cts;
         read_aggr_ctc(n, cts_info[i], aggr_cts);

         // Aggregate CTC counts
         cts_info[i].cts += aggr_cts.cts;

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
         store_stat_categorical(n, STATLineType::fho,
            conf_info.output_stats[STATLineType::fho][j],
            cts_info[i]);
      }

      // Add statistic value for each possible CTC column
      for(int j=0; j<conf_info.output_stats[STATLineType::ctc].n(); j++) {
         store_stat_categorical(n, STATLineType::ctc,
            conf_info.output_stats[STATLineType::ctc][j],
            cts_info[i]);
      }

      // Add statistic value for each possible CTS column
      for(int j=0; j<conf_info.output_stats[STATLineType::cts].n(); j++) {
         store_stat_categorical(n, STATLineType::cts,
            conf_info.output_stats[STATLineType::cts][j],
            cts_info[i]);
      }
   } // end for i

   // Deallocate memory
   if(cts_info) { delete [] cts_info; cts_info = (CTSInfo *) nullptr; }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_multicategory(int n, const PairDataPoint *pd_ptr) {

   mlog << Debug(4)
        << "Computing Multi-Category Statistics.\n";

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
      MCTSInfo aggr_mcts;
      read_aggr_mctc(n, mcts_info, aggr_mcts);

      // Aggregate MCTC counts
      mcts_info.cts += aggr_mcts.cts;

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
      store_stat_multicategory(n, STATLineType::mctc,
         conf_info.output_stats[STATLineType::mctc][i],
         mcts_info);
   }

   // Add statistic value for each possible MCTS column
   for(int i=0; i<conf_info.output_stats[STATLineType::mcts].n(); i++) {
      store_stat_multicategory(n, STATLineType::mcts,
         conf_info.output_stats[STATLineType::mcts][i],
         mcts_info);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_continuous(int n, const PairDataPoint *pd_ptr) {
   CNTInfo cnt_info;
   PairDataPoint pd;

   mlog << Debug(4)
        << "Computing Continuous Statistics.\n";

   // Process each filtering threshold
   for(int i=0; i<conf_info.fcnt_ta.n(); i++) {

      // Initialize
      cnt_info.clear();

      // Store thresholds
      cnt_info.fthresh = conf_info.fcnt_ta[i];
      cnt_info.othresh = conf_info.ocnt_ta[i];
      cnt_info.logic   = conf_info.cnt_logic;

      // Setup the CNTInfo alpha values
      cnt_info.allocate_n_alpha(conf_info.ci_alpha.n());
      for(int j=0; j<conf_info.ci_alpha.n(); j++) {
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

         // Aggregate scalar partial sums
         SL1L2Info aggr_psum;
         read_aggr_sl1l2(n, s_info, aggr_psum);
         s_info += aggr_psum;

         // Aggregate scalar anomaly partial sums
         if(conf_info.output_stats[STATLineType::cnt].has("ANOM_CORR")) {
            read_aggr_sal1l2(n, s_info, aggr_psum);
            s_info += aggr_psum;
         }

         // Compute continuous statistics from partial sums
         compute_cntinfo(s_info, cnt_info);
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
      for(int j=0; j<conf_info.output_stats[STATLineType::cnt].n(); j++) {
         store_stat_continuous(n, STATLineType::cnt,
            conf_info.output_stats[STATLineType::cnt][j],
            cnt_info);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_partialsums(int n, const PairDataPoint *pd_ptr) {

   mlog << Debug(4)
        << "Computing Scalar Partial Sums.\n";

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
         store_stat_partialsums(n, STATLineType::sl1l2,
            conf_info.output_stats[STATLineType::sl1l2][j],
            s_info);
      }

      // Add statistic value for each possible SAL1L2 column
      for(int j=0; j<conf_info.output_stats[STATLineType::sal1l2].n(); j++) {
         store_stat_partialsums(n, STATLineType::sal1l2,
            conf_info.output_stats[STATLineType::sal1l2][j],
            s_info);
      }

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

int read_aggr_total(int n) {

   // Read TOTAL data, if needed
   if(aggr_data.count(total_name) == 0) {

      // Retrive all the aggregate file variable names
      StringArray aggr_var_names;
      get_var_names(aggr_nc.MetNc->Nc, &aggr_var_names);

      // Search for one containing TOTAL
      for(int i=0; i<aggr_var_names.n(); i++) {
         if(aggr_var_names[i].find(total_name) != string::npos) {
            aggr_data[total_name] = read_aggr_data_plane(aggr_var_names[i]);
            break;
         }
      }

      // Check for a match
      if(aggr_data.count(total_name) == 0) {
         mlog << Error << "\nread_aggr_total() -> "
              << R"(No variable containing ")" << total_name
              << R"(" "found in the aggregate file!)" << "\n\n";
         exit(1);
      }
   }

   // Replace bad data with a total count of 0
   int total = nint(aggr_data[total_name].buf()[n]);
   if(is_bad_data(total)) total = 0;

   // Return the TOTAL count for the current point
   return total;
}

////////////////////////////////////////////////////////////////////////

void read_aggr_ctc(int n, const CTSInfo &cts_info,
                   CTSInfo &aggr_cts) {

   // Initialize
   aggr_cts.cts.zero_out();

   // Loop over the CTC columns
   for(auto &col : ctc_columns) {

      ConcatString c(to_upper(col));
      ConcatString var_name(build_nc_var_name_categorical(
                               STATLineType::ctc, c,
                               cts_info, bad_data_double));

      // Read aggregate data, if needed
      if(aggr_data.count(var_name) == 0) {
         aggr_data[var_name] = read_aggr_data_plane(
                                  var_name, R"("ALL" CTC)");
      }

      // Populate the CTC table
      aggr_cts.set_stat_ctc(col, aggr_data[var_name].buf()[n]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void read_aggr_mctc(int n, const MCTSInfo &mcts_info,
                    MCTSInfo &aggr_mcts) {

   // Initialize
   aggr_mcts.cts = mcts_info.cts;
   aggr_mcts.cts.zero_out();

   // Get MCTC column names
   StringArray mctc_cols(get_mctc_columns(aggr_mcts.cts.nrows()));

   // Loop over the MCTC columns
   for(int i=0; i<mctc_cols.n(); i++) {

      // Construct the NetCDF variable name
      ConcatString c(to_upper(mctc_cols[i]));
      ConcatString var_name(build_nc_var_name_multicategory(
                               STATLineType::mctc,
                               c, bad_data_double));

      // Read aggregate data, if needed
      if(aggr_data.count(var_name) == 0) {
         aggr_data[var_name] = read_aggr_data_plane(
                                  var_name, R"("ALL" MCTC)");
      }

      // Get the n-th value
      double v = aggr_data[var_name].buf()[n];

      // Store the number of pairs
      if(c == "TOTAL" && !is_bad_data(v)) {
         aggr_mcts.cts.set_n_pairs(nint(v));
      }
      // Check the number of categories
      else if(c == "N_CAT" && !is_bad_data(v) &&
         aggr_mcts.cts.nrows() != nint(v)) {
         mlog << Error << "\nread_aggr_mctc() -> "
              << "the number of MCTC categories do not match ("
              << nint(v) << " != " << aggr_mcts.cts.nrows() << ")!\n\n";
         exit(1);
      }
      // Check the expected correct
      else if(c == "EC_VALUE" && !is_bad_data(v) &&
              !is_eq(v, aggr_mcts.cts.ec_value(), loose_tol)) {
         mlog << Error << "\nread_aggr_mctc() -> "
              << "the MCTC expected correct values do not match ("
              << v << " != " << aggr_mcts.cts.ec_value() << ")!\n\n";
         exit(1);
      }
      // Populate the MCTC table
      else if(check_reg_exp("F[0-9]*_O[0-9]*", c.c_str())) {
         StringArray sa(c.split("_"));
         int i_row = atoi(sa[0].c_str()+1) - 1;
         int i_col = atoi(sa[1].c_str()+1) - 1;
         aggr_mcts.cts.set_entry(i_row, i_col, nint(v));
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void read_aggr_sl1l2(int n, const SL1L2Info &s_info,
                     SL1L2Info &aggr_psum) {

   // Initialize
   aggr_psum.zero_out();

   // Loop over the SL1L2 columns
   for(auto &col : sl1l2_columns) {

      ConcatString c(to_upper(col));
      ConcatString var_name(build_nc_var_name_partialsums(
                               STATLineType::sl1l2, c,
                               s_info));

      // Read aggregate data, if needed
      if(aggr_data.count(var_name) == 0) {
         aggr_data[var_name] = read_aggr_data_plane(
                                  var_name, R"("ALL" SL1L2)");
      }

      // Populate the partial sums
      aggr_psum.set_stat_sl1l2(col, aggr_data[var_name].buf()[n]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void read_aggr_sal1l2(int n, const SL1L2Info &s_info,
                      SL1L2Info &aggr_psum) {

   // Initialize
   aggr_psum.zero_out();

   // Loop over the SAL1L2 columns
   for(auto &col : sal1l2_columns) {

      ConcatString c(to_upper(col));
      ConcatString var_name(build_nc_var_name_partialsums(
                               STATLineType::sal1l2, c,
                               s_info));

      // Read aggregate data, if needed
      if(aggr_data.count(var_name) == 0) {
         aggr_data[var_name] = read_aggr_data_plane(
                                  var_name, R"("ALL" SAL1L2)");
      }

      // Populate the partial sums
      aggr_psum.set_stat_sal1l2(col, aggr_data[var_name].buf()[n]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void read_aggr_pct(int n, const PCTInfo &pct_info,
                   PCTInfo &aggr_pct) {

   // Initialize
   aggr_pct.pct = pct_info.pct;
   aggr_pct.pct.zero_out();

   // Get PCT column names
   StringArray pct_cols(get_pct_columns(aggr_pct.pct.nrows()+1));

   // Loop over the PCT columns
   for(int i=0; i<pct_cols.n(); i++) {

      // Skip unused "THRESH_" columns
      if(pct_cols[i].find("THRESH_") != string::npos) continue;

      // Construct the NetCDF variable name
      ConcatString c(to_upper(pct_cols[i]));
      ConcatString var_name(build_nc_var_name_probabilistic(
                               STATLineType::pct, c,
                               pct_info, bad_data_double));

      // Read aggregate data, if needed
      if(aggr_data.count(var_name) == 0) {
         aggr_data[var_name] = read_aggr_data_plane(
                                  var_name, R"("ALL" PCT)");
      }

      // Get the n-th value
      double v = aggr_data[var_name].buf()[n];

      // Store the number of pairs
      if(c == "TOTAL" && !is_bad_data(v)) {
         aggr_pct.pct.set_n_pairs(nint(v));
      }
      // Check the number of thresholds
      else if(c == "N_THRESH" && !is_bad_data(v) &&
         (aggr_pct.pct.nrows()+1) != nint(v)) {
         mlog << Error << "\nread_aggr_pct() -> "
              << "the number of PCT thresholds do not match ("
              << nint(v) << " != " << aggr_pct.pct.nrows()+1
              << ")!\n\n";
         exit(1);
      }
      // Set the event counts
      else if(check_reg_exp("OY_[0-9]", c.c_str())) {

         // Parse the index value from the column name
         int i_row = atoi(strrchr(c.c_str(), '_') + 1) - 1;
         aggr_pct.pct.set_event(i_row, nint(v));
      }
      // Set the non-event counts
      else if(check_reg_exp("ON_[0-9]", c.c_str())) {

         // Parse the index value from the column name
         int i_row = atoi(strrchr(c.c_str(), '_') + 1) - 1;
         aggr_pct.pct.set_nonevent(i_row, nint(v));
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_probabilistic(int n, const PairDataPoint *pd_ptr) {

   mlog << Debug(4)
        << "Computing Probabilistic Statistics.\n";

   // Object to store probabilistic statistics
   PCTInfo pct_info;

   // Setup the PCTInfo object
   pct_info.fthresh = conf_info.fcat_ta;
   pct_info.allocate_n_alpha(conf_info.ci_alpha.n());

   for(int i=0; i<conf_info.ci_alpha.n(); i++) {
      pct_info.alpha[i] = conf_info.ci_alpha[i];
   }

   // Compute PCTInfo for each observation threshold
   for(int i=0; i<conf_info.ocat_ta.n(); i++) {

      // Set the current observation threshold
      pct_info.othresh = conf_info.ocat_ta[i];

      // Aggregate input pair data with existing PCT counts
      if(aggr_file.nonempty()) {

         // Compute the probabilistic counts
         compute_pctinfo(*pd_ptr, false, pct_info);

         // Read the PCT data to be aggregated
         PCTInfo aggr_pct;
         read_aggr_pct(n, pct_info, aggr_pct);

         // Aggregate PCT counts
         pct_info.pct += aggr_pct.pct;

         // The climatology PCT table cannot be aggregated since the counts
         // are not written to the output. Store the pair climo brier score
         // before zeroing the PCT table.
         double briercl_pair = pct_info.climo_pct.brier_score();
         pct_info.climo_pct.zero_out();

         // Compute statistics and confidence intervals
         pct_info.compute_stats();
         pct_info.compute_ci();

         // Custom logic for the climatology Brier Score
         if(conf_info.output_stats[STATLineType::pstd].has("BRIERCL") ||
            conf_info.output_stats[STATLineType::pstd].has("BSS")) {
            do_climo_brier(n, briercl_pair, n_series_pair, pct_info);
         }
      }
      // Compute the probabilistic counts and statistics
      else {
         compute_pctinfo(*pd_ptr, true, pct_info);
      }

      // Add statistic value for each possible PCT column
      for(int j=0; j<conf_info.output_stats[STATLineType::pct].n(); j++) {
         store_stat_probabilistic(n, STATLineType::pct,
            conf_info.output_stats[STATLineType::pct][j],
            pct_info);
      }

      // Add statistic value for each possible PSTD column
      for(int j=0; j<conf_info.output_stats[STATLineType::pstd].n(); j++) {
         store_stat_probabilistic(n, STATLineType::pstd,
            conf_info.output_stats[STATLineType::pstd][j],
            pct_info);
      }

      // Add statistic value for each possible PJC column
      for(int j=0; j<conf_info.output_stats[STATLineType::pjc].n(); j++) {
         store_stat_probabilistic(n, STATLineType::pjc,
            conf_info.output_stats[STATLineType::pjc][j],
            pct_info);
      }

      // Add statistic value for each possible PRC column
      for(int j=0; j<conf_info.output_stats[STATLineType::prc].n(); j++) {
         store_stat_probabilistic(n, STATLineType::prc,
            conf_info.output_stats[STATLineType::prc][j],
            pct_info);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_climo_brier(int n, double briercl_pair,
                    int total_pair, PCTInfo &pct_info) {

   // Aggregate the climatology brier score as a weighted
   // average and recompute the brier skill score

   if(is_bad_data(briercl_pair) || total_pair == 0) return;

   // Construct the NetCDF variable name
   ConcatString var_name(build_nc_var_name_probabilistic(
                            STATLineType::pstd, "BRIERCL",
                            pct_info, bad_data_double));

   // Read aggregate data, if needed
   if(aggr_data.count(var_name) == 0) {
      aggr_data[var_name] = read_aggr_data_plane(
                               var_name, R"(the "BRIERCL" PSTD)");
   }

   // Get the n-th BRIERCL value
   double briercl_aggr = aggr_data[var_name].buf()[n];
   int total_aggr = read_aggr_total(n);

   // Aggregate BRIERCL as a weighted average
   if(!is_bad_data(briercl_pair) &&
      !is_bad_data(briercl_aggr) &&
      (total_pair + total_aggr) > 0) {

      pct_info.briercl.v = (total_pair * briercl_pair +
                            total_aggr * briercl_aggr) /
                           (total_pair + total_aggr);

      // Compute the brier skill score
      if(!is_bad_data(pct_info.brier.v) &&
         !is_bad_data(pct_info.briercl.v)) {
         pct_info.bss = 1.0 - (pct_info.brier.v / pct_info.briercl.v);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_categorical(int n, STATLineType lt,
        const ConcatString &col,
        const CTSInfo &cts_info) {

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Handle ALL CTC columns
   if(lt == STATLineType::ctc && c == all_columns) {
      return store_stat_all_ctc(n, cts_info);
   }

   // Check for columns with normal or bootstrap confidence limits
   int n_alpha = 1;
   if(lt == STATLineType::cts && is_ci_stat_name(c)) {
      n_alpha = cts_info.n_alpha;
   }

   // Loop over the alpha values
   for(int i_alpha=0; i_alpha<n_alpha; i_alpha++) {

      // Store alpha value
      double alpha = (n_alpha > 1 ? cts_info.alpha[i_alpha] : bad_data_double);

      // Construct the NetCDF variable name
      ConcatString var_name(build_nc_var_name_categorical(
                               lt, c, cts_info, alpha));

      // Add map for this variable name
      if(stat_data.count(var_name) == 0) {

         // Build key
         ConcatString lty_stat(statlinetype_to_string(lt));
         lty_stat << "_" << c;

         // Add new map entry
         add_stat_data(var_name, c, stat_long_name[lty_stat],
                       cts_info.fthresh.get_str(),
                       cts_info.othresh.get_str(),
                       alpha);
      }

      // Store the statistic value
      stat_data[var_name].dp.buf()[n] = cts_info.get_stat(lt, c, i_alpha);

   } // end for i_alpha

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_multicategory(int n, STATLineType lt,
        const ConcatString &col,
        const MCTSInfo &mcts_info) {

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Handle ALL MCTC columns
   if(lt == STATLineType::mctc && c == all_columns) {
      return store_stat_all_mctc(n, mcts_info);
   }

   // Check for columns with normal or bootstrap confidence limits
   int n_alpha = 1;
   if(lt == STATLineType::mcts && is_ci_stat_name(c)) {
      n_alpha = mcts_info.n_alpha;
   }

   // Loop over the alpha values
   for(int i_alpha=0; i_alpha<n_alpha; i_alpha++) {

      // Store alpha value
      double alpha = (n_alpha > 1 ? mcts_info.alpha[i_alpha] : bad_data_double);

      // Construct the NetCDF variable name
      ConcatString var_name(build_nc_var_name_multicategory(
                               lt, c, alpha));

      // Store the data value
      ConcatString col_name;
      auto v = (float) mcts_info.get_stat(lt, c, col_name, i_alpha);

      // Add map for this variable name
      if(stat_data.count(var_name) == 0) {

         // Build key
         ConcatString lty_stat;
         lty_stat << statlinetype_to_string(lt) << "_" << col_name;

         // Add new map entry
         add_stat_data(var_name, c, stat_long_name[lty_stat],
                       mcts_info.fthresh.get_str(","),
                       mcts_info.othresh.get_str(","),
                       alpha);
      }

      // Store the statistic value
      stat_data[var_name].dp.buf()[n] = v;

   } // end for i_alpha

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_continuous(int n, STATLineType lt,
        const ConcatString &col,
        const CNTInfo &cnt_info) {

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Check for columns with normal or bootstrap confidence limits
   int n_alpha = 1;
   if(is_ci_stat_name(c)) n_alpha = cnt_info.n_alpha;

   // Loop over the alpha values
   for(int i_alpha=0; i_alpha<n_alpha; i_alpha++) {

      // Store alpha value
      double alpha = (n_alpha > 1 ? cnt_info.alpha[i_alpha] : bad_data_double);

      // Construct the NetCDF variable name
      ConcatString var_name(build_nc_var_name_continuous(
                               lt, c, cnt_info, alpha));

      // Add map for this variable name
      if(stat_data.count(var_name) == 0) {

         // Build key
         ConcatString lty_stat(statlinetype_to_string(lt));
         lty_stat << "_" << c;

         // Add new map entry
         add_stat_data(var_name, c, stat_long_name[lty_stat],
                       cnt_info.fthresh.get_str(),
                       cnt_info.othresh.get_str(),
                       alpha);
      }

      // Store the statistic value
      stat_data[var_name].dp.buf()[n] = cnt_info.get_stat(c, i_alpha);

   } // end for i_alpha

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_partialsums(int n, STATLineType lt,
        const ConcatString &col,
        const SL1L2Info &s_info) {

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Handle ALL columns
   if(c == all_columns) {
           if(lt == STATLineType::sl1l2)  return store_stat_all_sl1l2(n, s_info);
      else if(lt == STATLineType::sal1l2) return store_stat_all_sal1l2(n, s_info);
   }

   // Construct the NetCDF variable name
   ConcatString var_name(build_nc_var_name_partialsums(
                            lt, c, s_info));

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      ConcatString lty_stat(statlinetype_to_string(lt));
      lty_stat << "_" << c;

      // Add new map entry
      add_stat_data(var_name, c, stat_long_name[lty_stat],
                    s_info.fthresh.get_str(),
                    s_info.othresh.get_str(),
                    bad_data_double);
   }

   // Store the statistic value
   stat_data[var_name].dp.buf()[n] = s_info.get_stat(lt, c);

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_probabilistic(int n, STATLineType lt,
        const ConcatString &col,
        const PCTInfo &pct_info) {

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Handle ALL PCT columns
   if(lt == STATLineType::pct && c == all_columns) {
      return store_stat_all_pct(n, pct_info);
   }

   // Check for columns with normal or bootstrap confidence limits
   int n_alpha = 1;
   if(is_ci_stat_name(c)) n_alpha = pct_info.n_alpha;

   // Loop over the alpha values
   for(int i_alpha=0; i_alpha<n_alpha; i_alpha++) {

      // Store alpha value
      double alpha = (n_alpha > 1 ? pct_info.alpha[i_alpha] : bad_data_double);

      // Construct the NetCDF variable name
      ConcatString var_name(build_nc_var_name_probabilistic(
                               lt, c, pct_info, alpha));

      // Store the data value
      ConcatString col_name;
      auto v = (float) pct_info.get_stat(lt, c, col_name, i_alpha);

      // Add map for this variable name
      if(stat_data.count(var_name) == 0) {

         // Build key
         ConcatString lty_stat(statlinetype_to_string(lt));
         lty_stat << "_" << col_name;

         // Add new map entry
         add_stat_data(var_name, c, stat_long_name[lty_stat],
                       pct_info.fthresh.get_str(","),
                       pct_info.othresh.get_str(),
                       alpha);
      }

      // Store the statistic value
      stat_data[var_name].dp.buf()[n] = v;

   } // end for i_alpha

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_all_ctc(int n, const CTSInfo &cts_info) {
   for(auto &col : ctc_columns) {
      store_stat_categorical(n, STATLineType::ctc, col, cts_info);
   }
}

////////////////////////////////////////////////////////////////////////

void store_stat_all_mctc(int n, const MCTSInfo &mcts_info) {
   StringArray mctc_cols(get_mctc_columns(mcts_info.cts.nrows()));
   for(int i=0; i<mctc_cols.n(); i++) {
      store_stat_multicategory(n, STATLineType::mctc, mctc_cols[i], mcts_info);
   }
}

////////////////////////////////////////////////////////////////////////

void store_stat_all_sl1l2(int n, const SL1L2Info &s_info) {
   for(auto &col : sl1l2_columns) {
      store_stat_partialsums(n, STATLineType::sl1l2, col, s_info);
   }
}

////////////////////////////////////////////////////////////////////////

void store_stat_all_sal1l2(int n, const SL1L2Info &s_info) {
   for(auto &col : sal1l2_columns) {
      store_stat_partialsums(n, STATLineType::sal1l2, col, s_info);
   }
}

////////////////////////////////////////////////////////////////////////

void store_stat_all_pct(int n, const PCTInfo &pct_info) {
   StringArray pct_cols(get_pct_columns(pct_info.pct.nrows() + 1));
   for(int i=0; i<pct_cols.n(); i++) {
      // Skip unused "THRESH_" columns
      if(pct_cols[i].find("THRESH_") != string::npos) continue;
      store_stat_probabilistic(n, STATLineType::pct, pct_cols[i], pct_info);
   }
}

////////////////////////////////////////////////////////////////////////

ConcatString build_nc_var_name_categorical(
                STATLineType lt, const ConcatString &col,
                const CTSInfo &cts_info, double alpha) {

   // Append the column name
   ConcatString var_name("series_");
   var_name << to_lower(statlinetype_to_string(lt)) << "_" << col;

   // Append threshold information
   if(cts_info.fthresh == cts_info.othresh) {
      var_name << "_" << cts_info.fthresh.get_abbr_str();
   }
   else {
      var_name << "_fcst" << cts_info.fthresh.get_abbr_str()
               << "_obs" << cts_info.othresh.get_abbr_str();
   }

   // Append confidence interval alpha value
   if(!is_bad_data(alpha)) var_name << "_a"  << alpha;

   return var_name;
}

////////////////////////////////////////////////////////////////////////

ConcatString build_nc_var_name_multicategory(
                STATLineType lt, const ConcatString &col,
                double alpha) {

   // Append the column name
   ConcatString var_name("series_");
   var_name << to_lower(statlinetype_to_string(lt)) << "_" << col;

   // Append confidence interval alpha value
   if(!is_bad_data(alpha)) var_name << "_a"  << alpha;

   return var_name;
}

////////////////////////////////////////////////////////////////////////

ConcatString build_nc_var_name_partialsums(
                STATLineType lt, const ConcatString &col,
                const SL1L2Info &s_info) {

   // Append the column name
   ConcatString var_name("series_");
   var_name << to_lower(statlinetype_to_string(lt)) << "_"  << col;

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

ConcatString build_nc_var_name_continuous(
                STATLineType lt, const ConcatString &col,
                const CNTInfo &cnt_info, double alpha) {

   // Append the column name
   ConcatString var_name("series_");
   var_name << to_lower(statlinetype_to_string(lt)) << "_" << col;

   // Append threshold information, if supplied
   if(cnt_info.fthresh.get_type() != thresh_na ||
      cnt_info.othresh.get_type() != thresh_na) {
      var_name << "_fcst" << cnt_info.fthresh.get_abbr_str()
               << "_" << setlogic_to_abbr(cnt_info.logic)
               << "_obs" << cnt_info.othresh.get_abbr_str();
   }

   // Append confidence interval alpha value
   if(!is_bad_data(alpha)) var_name << "_a"  << alpha;

   return var_name;
}

////////////////////////////////////////////////////////////////////////

ConcatString build_nc_var_name_probabilistic(
                STATLineType lt, const ConcatString &col,
                const PCTInfo &pct_info, double alpha) {

   // Append the column name
   ConcatString var_name("series_");
   var_name << to_lower(statlinetype_to_string(lt)) << "_" << col;

   // Append the observation threshold
   var_name << "_obs" << pct_info.othresh.get_abbr_str();

   // Append confidence interval alpha value
   if(!is_bad_data(alpha)) var_name << "_a"  << alpha;

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
   NcVar var = add_var(nc_out, n_series_var_name, ncInt, deflate_level);
   add_att(&var, "long_name", "length of series");

   int n_series = n_series_pair + n_series_aggr;
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

void add_stat_data(const ConcatString &var_name,
                   const ConcatString &name,
                   const ConcatString &long_name,
                   const ConcatString &fcst_thresh,
                   const ConcatString &obs_thresh,
                   double alpha) {

   NcVarData data;
   data.dp.set_size(grid.nx(), grid.ny(), bad_data_double);
   data.name        = name;
   data.long_name   = long_name;
   data.fcst_thresh = fcst_thresh;
   data.obs_thresh  = obs_thresh;
   data.alpha       = alpha;

   // Store the new NcVarData object
   stat_data[var_name] = data;
   stat_data_keys.push_back(var_name);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_stat_data() {

   mlog << Debug(2)
        << "Writing " << stat_data_keys.size()
        << " output variables.\n";

   int deflate_level = compress_level;
   if(deflate_level < 0) deflate_level = conf_info.get_compression_level();

   // Allocate memory to store data values for each grid point
   float *data = new float [grid.nx()*grid.ny()];

   // Write output for each stat_data map entry
   for(auto &key : stat_data_keys) {

      NcVarData *ptr = &stat_data[key];

      // Add a new variable to the NetCDF file
      NcVar nc_var = add_var(nc_out, key, ncFloat, lat_dim, lon_dim, deflate_level);

      // Add variable attributes
      add_att(&nc_var, "_FillValue", bad_data_float);
      add_att(&nc_var, "name", ptr->name);
      add_att(&nc_var, "long_name", ptr->long_name);
      if(ptr->fcst_thresh.length() > 0) add_att(&nc_var, "fcst_thresh", ptr->fcst_thresh);
      if(ptr->obs_thresh.length() > 0) add_att(&nc_var, "obs_thresh", ptr->obs_thresh);
      if(!is_bad_data(ptr->alpha)) add_att(&nc_var, "alpha", ptr->alpha);

      // Store the data
      for(int x=0; x<grid.nx(); x++) {
         for(int y=0; y<grid.ny(); y++) {
            int n = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);
            data[n] = (float) ptr->dp(x, y);
         } // end for y
      } // end for x

      // Write out the data
      if(!put_nc_data_with_dims(&nc_var, &data[0], grid.ny(), grid.nx())) {
         mlog << Error << "\nwrite_stat_data() -> "
              << R"(error writing ")" << key
              << R"(" data to the output file.)" << "\n\n";
         exit(1);
      }
   }

   // Clean up
   if(data) { delete [] data;  data = (float *) nullptr; }

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

   // Close the output NetCDF file
   if(nc_out) {

      // List the NetCDF file after it is finished
      mlog << Debug(1)
           << "Output file: " << out_file << "\n";

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

        << "\twhere\t"
        << R"("-fcst file_1 ... file_n" are the gridded )"
        << "forecast files to be used (required).\n"

        << "\t\t"
        << R"("-fcst fcst_file_list" is an ASCII file containing )"
        << "a list of gridded forecast files to be used (required).\n"

        << "\t\t"
        << R"("-obs  file_1 ... file_n" are the gridded )"
        << "observation files to be used (required).\n"

        << "\t\t"
        << R"("-obs  obs_file_list" is an ASCII file containing )"
        << "a list of gridded observation files to be used (required).\n"

        << "\t\t"
        << R"("-both" sets the "-fcst" and "-obs" options to )"
        << "the same set of files (optional).\n"

        << "\t\t"
        << R"("-aggr file" specifies a series_analysis output )"
        << "file with partial sums and/or contingency table counts to be "
        << "updated prior to deriving statistics (optional).\n"

        << "\t\t"
        << R"("-paired" to indicate that the input -fcst and -obs )"
        << "file lists are already paired (optional).\n"

        << "\t\t"
        << R"("-out file" is the NetCDF output file containing )"
        << "computed statistics (required).\n"

        << "\t\t"
        << R"("-config file" is a SeriesAnalysisConfig file )"
        << "containing the desired configuration settings (required).\n"

        << "\t\t"
        << R"("-log file" outputs log messages to the specified )"
        << "file (optional).\n"

        << "\t\t"
        << R"("-v level" overrides the default level of logging ()"
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t"
        << R"("-compress level" overrides the compression level of NetCDF variable ()"
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
           << R"(can't open the ASCII file ") << file_name
           << R"(" for reading!)" << "\n\n";
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
