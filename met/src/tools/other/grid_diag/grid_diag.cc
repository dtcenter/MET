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

static void process_command_line(int, char **);

static void process_files(void);

static Met2dDataFile *get_mtddf(const StringArray &, const GrdFileType);

static void get_series_data(int, VarInfo *,
                            DataPlane &);
static void get_series_entry(int, VarInfo *, const StringArray &,
                             const GrdFileType, StringArray &,
                             DataPlane &);
static bool read_single_entry(VarInfo *, const ConcatString &,
                              const GrdFileType, DataPlane &, Grid &);

static void set_range(const unixtime &, unixtime &, unixtime &);
static void set_range(const int &, int &, int &);

static void clean_up();

static void usage();
static void set_data_files(const StringArray &);
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
   process_command_line(argc, argv);

   // Close files and deallocate memory
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

   // Store the input data file types
   dtype = data_mtddf->file_type();

   mlog << Debug(2) << "Process configuration.\n";

   // Process the configuration
   conf_info.process_config(dtype);

   // Determine the verification grid
   grid = parse_vx_grid(conf_info.data_info[0]->regrid(),
                        &(data_mtddf->grid()), &(data_mtddf->grid()));

   // Process masking regions
   conf_info.process_masks(grid);
}

////////////////////////////////////////////////////////////////////////

void process_files(void) {
   int i;

   // List the lengths of the series options
   mlog << Debug(1)
        << "Length of configuration \"data.field\" = "
        << conf_info.get_n_data() << "\n"
        << "Length of forecast file list         = "
        << data_files.n_elements() << "\n";

   // Determine the length of the series to be analyzed.  Series is
   // defined by the first parameter of length greater than one:
   // - Configuration data.field
   // - Forecast file list
   if(conf_info.get_n_data() > 1) {
      series_type = SeriesType_Data_Conf;
      n_series = conf_info.get_n_data();
      mlog << Debug(1)
           << "Series defined by the \"data.field\" configuration entry "
           << "of length " << n_series << ".\n";
   }
   else if(data_files.n_elements() > 1) {
      series_type = SeriesType_Data_Files;
      n_series = data_files.n_elements();
      mlog << Debug(1)
           << "Series defined by the forecast file list of length "
           << n_series << ".\n";
   }
   else {
      series_type = SeriesType_Data_Conf;
      n_series = 1;
      mlog << Debug(1)
           << "The \"data.field\" configuration entry "
           << "and the \"-data\" command line option "
           << "all have length one.\n";
   }

   // Initialize the series file names
   for(i=0; i<n_series; i++) {
      found_data_files.add("");
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
   if(i == data_files.n_elements()) {
      mlog << Error << "\nTrouble reading data files.\n\n";
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
                     VarInfo *data_info,
                     DataPlane &data_dp) {

   mlog << Debug(2)
        << "Processing series entry " << i_series + 1 << " of "
        << n_series << ": " << data_info->magic_str() << "\n";

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
      mlog << Debug(1) << "Output file: " << out_file << "\n";

      delete nc_out;
      nc_out = (NcFile *) 0;
   }

   // Deallocate memory for data files
   if(data_mtddf) { delete data_mtddf; data_mtddf = (Met2dDataFile *) 0; }

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

void set_data_files(const StringArray & a) {
   data_files = a;
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
