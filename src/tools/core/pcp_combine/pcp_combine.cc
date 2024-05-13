// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   pcp_combine.cc
//
//   Description:
//      Based on the command line options, this tool combines one or
//      more gridded data files into a single gridded data file and
//      writes the output in NetCDF format.
//
//      The tool may be run in four different modes:
//         sum, add, subtract, derive
//
//      The sum command requires the user to specify an initialization
//      time, input accumulation interval, valid time, and output
//      accumulation interval.
//
//      The add, subtract, and derive commands require the user to
//      specify a list of input files.  Each input file may be followed
//      by an accumulation interval or a config file string, describing
//      the data to be processed.  Alternatively, use the -field command
//      line option to set all config file strings to the same thing.
//
//      The subtract command requires exactly two input files while the
//      add and derive commands support one or more input files.
//
//      In all cases, the last argument is the output NetCDF file name.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    01-24-07  Halley Gotway  New
//   001    12-07-07  Halley Gotway  Change time format from
//                    YYYY-MM-DD_HH:MM:SS to YYYYMMDD_HHMMSS.
//   002    01-31-08  Halley Gotway  Add support for the -add and
//                    -subtract options.
//   003    09/23/08  Halley Gotway  Change argument sequence for the
//                    GRIB record access routines.
//   004    02/20/09  Halley Gotway  Append _HH to the variable name
//                    for non-zero accumulation times.
//   005    12/23/09  Halley Gotway  Call the library read_pds routine.
//   006    05/21/10  Halley Gotway  Enhance to search multiple
//                    -pcpdir directory arguments.
//   007    06/25/10  Halley Gotway  Allow times to be specified in
//                    HH[MMSS] and YYYYMMDD[_HH[MMSS]] format.
//   008    06/30/10  Halley Gotway  Enhance grid equality checks.
//   009    07/27/10  Halley Gotway  Enhance to allow addition of any
//                    number of input files/accumulation intervals.
//                    Add lat/lon variables to NetCDF.
//   010    04/19/11  Halley Gotway  Bugfix for -add option.
//   011    10/20/11  Holmes         Added use of command line class to
//                    parse the command line arguments.
//   012    11/14/11  Halley Gotway  Bugfix for -add option when
//                    when handling missing data values.
//   013    12/21/11  Bullock        Ported to new repository.
//   014    03/07/12  Halley Gotway  Bugfix in get_field() function and
//                    remove unnecessary time strings.
//   015    04/12/12  Oldenburg      Support for all gridded data types.
//   016    01/23/13  Halley Gotway  Update usage statement and code
//                    cleanup.
//   017    10/17/13  Halley Gotway  Bugfix for closing file handles
//                    during pcpdir search.
//   018    04/16/14  Halley Gotway  Bugfix for the -varname option.
//   019    05/20/16  Prestopnik J   Removed -version (now in
//                    command_line.cc)
//   020    12/02/16  Halley Gotway  Change init and accumulation
//                    subtraction errors to warnings.
//   021    03/01/19  Halley Gotway  Add -derive command line option.
//   022    03/08/19  Halley Gotway  Support multiple -field options.
//   023    08/29/19  Halley Gotway  Support multiple arguments for the
//                    the -pcpdir option.
//   024    07/06/22  Howard Soh     METplus-Internal #19 Rename main to met_main.
//   025    09/29/22  Prestopnik     MET #2227 Remove namespace netCDF from header files.
//   026    01/29/24  Halley Gotway  MET #2801 Configure time difference warnings.
//   027    05/09/24  Halley Gotway  MET #2883 Allow missing input files.
//
////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <dirent.h>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <netcdf>

#include "main.h"
#include "vx_log.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_nc_util.h"
#include "vx_grid.h"
#include "vx_statistics.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"

using namespace std;
using namespace netCDF;

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

static const char zero_time_str   [] = "00000000_000000";
static const char default_pcp_dir [] = ".";
static const char default_reg_exp [] = ".*";
static const char derive_options  [] =
   "sum, min, max, range, mean, stdev, vld_count";

// Run Command enumeration
enum class RunCommand { sum = 0, add = 1, sub = 2, der = 3 };

// Variables for top-level command line arguments
static RunCommand run_command = RunCommand::sum;

// Variables common to all commands
static int          n_files;
static StringArray  req_field_list;
static bool         field_option_used = false;
static ConcatString field_string;
static ConcatString out_filename;
static StringArray  req_out_var_name;
static int          i_out_var = 0;
static int          n_out_var;
static MetConfig    config;
static VarInfo *    var_info = (VarInfo *) nullptr;
static double       input_thresh = 1.0;
static double       vld_thresh = 1.0;
static int          compress_level = -1;

// Variables for the sum command
static unixtime     init_time;
static int          in_accum;
static unixtime     valid_time;
static int          out_accum;
static StringArray  pcp_dir;
static ConcatString pcp_reg_exp = (string)default_reg_exp;

// Variables for the derive command
static StringArray  file_list;
static StringArray  field_list;
static StringArray  derive_list;

// Output NetCDF file
NcFile *nc_out = (NcFile *) nullptr;
NcDim   lat_dim;
NcDim   lon_dim;

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);

static void process_sum_args(const CommandLine &);
static void process_add_sub_derive_args(const CommandLine &);

static void do_sum_command();
static void do_sub_command();
static void do_derive_command();

static void sum_data_files(Grid &, DataPlane &);
static int  search_pcp_dir(const char *, const unixtime,
                           ConcatString &);

static bool get_field(const char * filename, const char * cur_field,
                      const unixtime get_init_ut,
                      const unixtime get_valid_ut,
                      Grid & grid, DataPlane & plane,
                      bool error_out);

static void open_nc(const Grid &);
static void write_nc_data(unixtime, unixtime, int, const DataPlane &,
                          const char *derive_str,
                          const char *long_name_prefix);
static void close_nc();

static ConcatString parse_config_str(const char *);
static bool is_timestring(const char *);

static void usage();
static void set_sum(const StringArray &);
static void set_add(const StringArray &);
static void set_subtract(const StringArray &);
static void set_derive(const StringArray &);
static void set_pcpdir(const StringArray &);
static void set_pcprx(const StringArray &);
static void set_field(const StringArray & a);
static void set_name(const StringArray & a);
static void set_input_thresh(const StringArray & a);
static void set_vld_thresh(const StringArray & a);
static void set_compress(const StringArray &);

////////////////////////////////////////////////////////////////////////

int met_main(int argc, char *argv[]) {
   int i, j;

   program_name = get_short_name(argv[0]);

   //
   // Process the command line arguments
   //
   process_command_line(argc, argv);

   //
   // Process each requested field
   //
   for(i=0; i<req_field_list.n(); i++) {

      //
      // Reinitialize for the current loop.
      //
      field_string = req_field_list[i];
      if(var_info) { delete var_info; var_info = (VarInfo *) nullptr; }

      //
      // Reset when reading multiple fields from the same input files.
      //
      if(field_option_used) {
         field_list.clear();
         for(j=0; j<file_list.n(); j++) field_list.add(field_string);
      }

      //
      // Perform the requested run or subtract command.
      // Derive handles add and derive.
      //
           if(run_command == RunCommand::sum) do_sum_command();
      else if(run_command == RunCommand::sub) do_sub_command();
      else                                    do_derive_command();
   }

   //
   // Clean up
   //
   close_nc();

   return 0;
}

////////////////////////////////////////////////////////////////////////

const string get_tool_name() {
   return "pcp_combine";
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;

   //
   // Check for zero arguments
   //
   if(argc == 1) usage();

   //
   // Default to running the sum command
   //
   run_command = RunCommand::sum;
   derive_list.add("sum");

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
   cline.add(set_sum,          "-sum",          0);
   cline.add(set_add,          "-add",          0);
   cline.add(set_subtract,     "-subtract",     0);
   cline.add(set_derive,       "-derive",       1);
   cline.add(set_pcpdir,       "-pcpdir",      -1);
   cline.add(set_pcprx,        "-pcprx",        1);
   cline.add(set_field,        "-field",        1);
   cline.add(set_name,         "-name",         1);
   cline.add(set_name,         "-varname",      1);
   cline.add(set_input_thresh, "-input_thresh", 1);
   cline.add(set_vld_thresh,   "-vld_thresh",   1);
   cline.add(set_compress,     "-compress",     1);

   //
   // Parse the command line.
   //
   cline.parse();

   // Initialize
   in_accum = 1;
   out_accum = 0;
   init_time = valid_time = (unixtime) 0;

   //
   // Process the specific command arguments.
   //
   if(run_command == RunCommand::sum) process_sum_args(cline);
   else                               process_add_sub_derive_args(cline);

   //
   // If -field not set, set to a list of length 1 with an empty string.
   //
   if(req_field_list.n() == 0) req_field_list.add("");

   //
   // Determine the number of output variables to write.
   //
   n_out_var = derive_list.n() * req_field_list.n();

   //
   // If pcp_dir is not set, set to the default.
   //
   if(pcp_dir.n_elements() == 0) pcp_dir.add(default_pcp_dir);

   //
   // Initialize the MetConfig object.
   //
   config.read(replace_path(config_const_filename).c_str());

   //
   // Check the -name option.
   //
   if(req_out_var_name.n() > 0 &&
      req_out_var_name.n() != n_out_var) {
      mlog << Error << "\nprocess_command_line() -> "
           << "expected " << n_out_var
           << " entries for the \"-name\" command line option but got "
           << req_out_var_name.n() << "!\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_sum_args(const CommandLine & cline) {

   //
   // Check for enough command line arguments:
   //   init_time, in_accum, valid_time, out_accum, out_file
   //
   if(cline.n() != 5) {
      mlog << Error << "\nprocess_sum_args() -> "
           << "expected exactly 5 arguments but got " << cline.n()
           << "!\n\n";
      exit(1);
   }

   //
   // Init time
   //
   if(cline[0].compare(zero_time_str) == 0) {
      init_time = (unixtime) 0;
   }
   else {
      init_time = timestring_to_unix(cline[0].c_str());
   }

   //
   // Input accumulation
   //
   in_accum = timestring_to_sec(cline[1].c_str());

   //
   // Valid time
   //
   if(cline[2].compare(zero_time_str) == 0) {
      valid_time = (unixtime) 0;
   }
   else {
      valid_time = timestring_to_unix(cline[2].c_str());
   }

   //
   // Output accumulation
   //
   out_accum = timestring_to_sec(cline[3].c_str());

   //
   // Out file
   //
   out_filename = cline[4];

   //
   // Check that accumulation intervals are greater than zero
   //
   if(in_accum <= 0 || out_accum <= 0) {
      mlog << Error << "\nprocess_sum_args() -> "
           << "the input accumulation interval (" << cline[1]
           << ") and output accumulation interval (" << cline[3]
           << ") must be greater than zero.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_add_sub_derive_args(const CommandLine & cline) {
   int i;

   //
   // Check for enough command line arguments
   //
   if(cline.n() < 2) {
      mlog << Error << "\nprocess_add_sub_derive_args() -> "
           << "expected at least 2 arguments but got " << cline.n()
           << "!\n\n";
      exit(1);
   }

   //
   // Store last entry as the output file
   //
   out_filename = cline[cline.n()-1];

   //
   // If the -field command line option was used, process remaining
   // arguments as a list of file names.
   //
   if(req_field_list.n() > 0) {

      mlog << Debug(2)
           << "Since the \"-field\" command line option was used, "
           << "parsing the command line arguments as a list of "
           << "files.\n";

      StringArray sa;
      for(i=0; i<(cline.n()-1); i++) sa.add(cline[i]);
      file_list = parse_file_list(sa);
   }
   //
   // If the -field command line option was not used, process remaining
   // arguments as pairs of filenames followed by configuration strings.
   //
   else {

      mlog << Debug(2)
           << "Since the \"-field\" command line option was not used, "
           << "parsing the command line arguments a list of files, "
           << "each followed by a configuration string.\n";

      for(i=0, n_files=0; i<(cline.n() - 1); i+=2) {
         file_list.add(cline[i]);

         //
         // Check if this is actually a file name
         //
         if(file_exists(cline[i+1].c_str())) {
            mlog << Error << "\nprocess_add_sub_derive_args() -> "
                 << "file name used when config string expected ("
                 << cline[i+1]
                 << "). Did you forget the \"-field\" option?\n\n";
            exit(1);
         }
         field_list.add(cline[i+1]);
      }
   }

   //
   // Store the number of input files.
   //
   n_files = file_list.n();

   return;
}

////////////////////////////////////////////////////////////////////////

void do_sum_command() {
   DataPlane plane;
   Grid grid;
   int lead_time;
   ConcatString init_time_str;

   //
   // Compute the lead time.
   //
   lead_time = valid_time - init_time;

   //
   // Build init time string.
   //
   if(init_time != 0) {
      init_time_str = unix_to_yyyymmdd_hhmmss(init_time);
   }
   else {
      init_time_str = zero_time_str;
   }

   mlog << Debug(2)
        << "Performing sum command: "
        << "Init/In_Accum/Valid/Out_Accum Times = "
        << init_time_str  << "/" << sec_to_hhmmss(in_accum)  << "/"
        << unix_to_yyyymmdd_hhmmss(valid_time) << "/"
        << sec_to_hhmmss(out_accum) << "\n";

   //
   // Check that the output accumulation time is not greater than
   // the lead time, except when init_time = 0 for observations.
   //
   if(out_accum > lead_time && init_time != (unixtime) 0) {
      mlog << Error << "\ndo_sum_command() -> "
           << "the output accumulation time ("
           << sec_to_hhmmss(out_accum)
           << ") can't be greater than the lead time ("
           << sec_to_hhmmss(lead_time) << ").\n\n";
      exit(1);
   }

   //
   // Check that the output accumulation time is divisible by the input
   // accumulation time.
   //
   if(out_accum%in_accum != 0) {
      mlog << Error << "\ndo_sum_command() -> "
           << "the output accumulation time ("
           << sec_to_hhmmss(out_accum)
           << ") must be divisible by the input accumulation "
           << "time (" << sec_to_hhmmss(in_accum) << ").\n\n";
      exit(1);
   }

   //
   // Check that the lead time is divisible by the input
   // accumulation time except when init_time = 0 for observations.
   //
   if(lead_time%in_accum != 0 && init_time != (unixtime) 0) {
      mlog << Error << "\ndo_sum_command() -> "
           << "the lead time (" << sec_to_hhmmss(lead_time)
           << ") must be divisible by the input accumulation time ("
           << sec_to_hhmmss(in_accum) << ").\n\n";
      exit(1);
   }

   //
   // Find and sum up the matching precipitation files.
   //
   sum_data_files(grid, plane);

   //
   // Write output.
   //
   if(!nc_out) open_nc(grid);
   write_nc_data(init_time, valid_time, out_accum, plane, "sum", "");

   return;
}

////////////////////////////////////////////////////////////////////////

void sum_data_files(Grid & grid, DataPlane & plane) {
   int i, j, x, y, n_vld;
   DataPlane part;
   double v_sum, v_part;
   Grid cur_grid;
   unixtime     * pcp_times = (unixtime *) nullptr;
   int          * pcp_recs  = (int *) nullptr;
   ConcatString * pcp_files = (ConcatString *) nullptr;

   //
   // Compute the number of forecast precipitation files to be found,
   // and allocate memory to store their names and times.
   //
   n_files   = out_accum/in_accum;
   pcp_times = new unixtime [n_files];
   pcp_recs  = new int [n_files];
   pcp_files = new ConcatString [n_files];

   mlog << Debug(2)
        << "Searching for " << n_files << " files "
        << "with accumulation times of " << sec_to_hhmmss(in_accum)
        << " to sum to a total accumulation time of "
        << sec_to_hhmmss(out_accum) << ".\n";

   //
   // Search for each file time.
   //
   for(i=0, n_vld=0; i<n_files; i++) {

      //
      // Define the target valid time and initialize.
      //
      pcp_times[i] = valid_time - i*in_accum;
      pcp_recs[i] = -1;
      pcp_files[i] = "";

      //
      // Search in each directory for the current file time.
      //
      for(j=0; j<pcp_dir.n_elements(); j++) {

         pcp_recs[i] = search_pcp_dir(pcp_dir[j].c_str(), pcp_times[i],
                                      pcp_files[i]);

         if(pcp_recs[i] != -1) {
            mlog << Debug(1)
                 << "[" << (i+1) << "] File " << pcp_files[i]
                 << " matches valid time of "
                 << unix_to_yyyymmdd_hhmmss(pcp_times[i]) << "\n";
            break;
         } // end if

      } // end for j

      //
      // Check for no matching file found.
      //
      if(pcp_recs[i] != -1) {
         n_vld++;
      }
      else {
         mlog << Warning << "\nsum_data_files() -> "
              << "can't find a file with a valid time of "
              << unix_to_yyyymmdd_hhmmss(pcp_times[i])
              << " and accumulation time of "
              << sec_to_hhmmss(in_accum) << " matching the regular "
              << "expression \"" << pcp_reg_exp << "\"\n\n";
      }

   } // end for i

   // Check for enough valid input files.
   if((double) n_vld/n_files < input_thresh) {
      mlog << Error << "\nsum_data_files() -> "
           << n_vld << " of " << n_files << " (" << (double) n_vld/n_files
           << ") valid inputs does not meet the required input threshold ("
           << input_thresh << ").\n\n";
      exit(1);
   }

   //
   // Open each of the files found and parse the data.
   //
   for(i=0; i<n_files; i++) {

      //
      // Skip missing inputs.
      //
      if(pcp_recs[i] == -1) continue;

      mlog << Debug(1)
           << "[" << (i+1) << "] Reading input file: " << pcp_files[i]
           << "\n";

      //
      // Build the configuration string.
      //
      ConcatString cur_field = field_string;
      if(cur_field.empty()) cur_field << sec_to_hhmmss(in_accum);

      //
      // Read data for the file.
      // Error out for any problems reading the requested data.
      //
      get_field(pcp_files[i].c_str(), cur_field.c_str(),
                init_time, pcp_times[i],
                cur_grid, part, true);

      //
      // Initialize the grid and data after the first successful read.
      //
      if(grid.nxy() == 0) {
         grid  = cur_grid;
         plane = part;
      }
      else {

         //
         // Check to make sure the grid stays the same.
         //
         if(grid != cur_grid) {
            mlog << Error << "\nsum_data_files() -> "
                 << "the input fields must be on the same grid.\n"
                 << grid.serialize() << "\n" << cur_grid.serialize()
                 << "\n\n";
            exit(1);
         }

         //
         // Increment the precipitation sums keeping track of the bad
         // data values.
         //
         for(x=0; x<grid.nx(); x++) {
            for(y=0; y<grid.ny(); y++) {

               v_sum = plane(x, y);

               if(is_bad_data(v_sum)) continue;

               v_part = part(x, y);

               if(is_bad_data(v_part) ) {
                  plane.set(bad_data_float, x, y);
                  continue;
               }

               plane.set(v_sum + v_part, x, y);
            } // for y
         } // for x
      } // end else
   } // end for i

   //
   // Deallocate any memory that was allocated above.
   //
   if(pcp_files) { delete [] pcp_files; pcp_files = (ConcatString *) nullptr; }
   if(pcp_times) { delete [] pcp_times; pcp_times = (unixtime *) nullptr; }
   if(pcp_recs ) { delete [] pcp_recs;  pcp_recs  = (int *) nullptr; }

   return;
}

////////////////////////////////////////////////////////////////////////

int search_pcp_dir(const char *cur_dir, const unixtime cur_ut,
                   ConcatString & cur_file) {
   int i_rec;
   struct dirent *dirp = (struct dirent *) nullptr;
   DIR *dp = (DIR *) nullptr;

   //
   // Find the files matching the specified regular expression with
   // the correct valid and accumulation times.
   //
   dp = met_opendir(cur_dir);
   if(!dp) {
      mlog << Error << "\nsearch_pcp_dir() -> "
           << "can't open search directory: " << cur_dir << "\n\n";
      exit(1);
   }

   //
   // Initialize the record index to not found.
   //
   i_rec = -1;

   //
   // Process each file contained in the directory.
   //
   while((dirp = readdir(dp)) != nullptr) {

      //
      // Ignore any hidden files.
      //
      if(dirp->d_name[0] == '.') continue;

      //
      // Check the file name for a matching regular expression.
      //
      if(check_reg_exp(pcp_reg_exp.c_str(), dirp->d_name) == true) {

         //
         // Check the current file for matching initialization,
         // valid, lead, and accumulation times.
         //
         cur_file << cs_erase << cur_dir << '/' << dirp->d_name;

         Met2dDataFileFactory factory;
         Met2dDataFile * mtddf;
         VarInfoFactory var_fac;
         VarInfo * cur_var;

         //
         // Create a data file object.
         //
         mtddf = factory.new_met_2d_data_file(cur_file.c_str());
         if(!mtddf) {
            mlog << Warning << "search_pcp_dir() -> "
                 << "can't open data file \"" << cur_file << "\"\n";
            continue;
         }

         //
         // Create a VarInfo object from the data file.
         //
         cur_var = var_fac.new_var_info(mtddf->file_type());
         if(!cur_var) {
            delete mtddf;  mtddf = nullptr;
            mlog << Warning << "search_pcp_dir() -> "
                 << "unable to determine filetype of \"" << cur_file
                 << "\"\n";
            continue;
         }

         //
         // Initialize the VarInfo object with a field dictionary and
         // the requested timing information.
         //
         ConcatString cs = field_string;
         if(cs.empty()) cs << sec_to_hhmmss(in_accum);
         config.read_string(parse_config_str(cs.c_str()).c_str());
         cur_var->set_dict(config);

         cur_var->set_valid(cur_ut);
         cur_var->set_init(init_time);
         cur_var->set_lead(init_time ?
                           cur_ut - init_time : bad_data_int);

         //
         // Look for a VarInfo record match in the data file.
         //
         i_rec = mtddf->index(*cur_var);

         //
         // Cleanup.
         //
         if(mtddf)   { delete mtddf;   mtddf   = (Met2dDataFile *) nullptr; }
         if(cur_var) { delete cur_var; cur_var = (VarInfo *)       nullptr; }

         // Check for a valid match
         if(i_rec != -1) {
            met_closedir(dp);
            break;
         }

      } // end if

   } // end while

   if(dp) met_closedir(dp);

   return i_rec;
}

////////////////////////////////////////////////////////////////////////

void do_sub_command() {
   DataPlane plus, minus, diff;
   Grid grid1, grid2;
   unixtime nc_init_time, nc_valid_time;
   int i, nxy, nc_accum;

   //
   // Check for exactly two input files.
   //
   if(n_files != 2) {
      mlog << Error << "\ndo_sub_command() -> "
           << "you must specify exactly two input files for "
           << "subtraction.\n\n";
      exit(1);
   }

   //
   // Read the two specified data files.
   // Error out for any problems reading the requested data.
   //
   mlog << Debug(1)
        << "Reading input file: " << file_list[0] << "\n";
   get_field(file_list[0].c_str(), field_list[0].c_str(),
             0, 0, grid1, plus, true);

   mlog << Debug(1)
        << "Reading input file: " << file_list[1] << "\n";
   get_field(file_list[1].c_str(), field_list[1].c_str(),
             0, 0, grid2, minus, true);

   //
   // Check for the same grid dimensions.
   //
   if(grid1 != grid2) {
      mlog << Error << "\ndo_sub_command() -> "
           << "the input fields must be on the same grid.\n"
           << grid1.serialize() << "\n" << grid2.serialize() << "\n\n";
      exit(1);
   }

   //
   // Compute output accumulation, initialization, and valid times
   // for the subtract command.
   //
   mlog << Debug(2) << "Performing subtraction command.\n";

   //
   // Output valid time
   //
   nc_valid_time = plus.valid();

   //
   // Check that the initialization times match.
   //
   if(plus.init() != minus.init()) {

      ConcatString cs;
      cs << cs_erase
         << "The initialization times do not match ("
         << unix_to_yyyymmdd_hhmmss(plus.init()) << " != "
         << unix_to_yyyymmdd_hhmmss(minus.init())
         << ") for subtraction. Using the first value.";

      if(config.time_offset_warning(
            (int) (plus.init() - minus.init()))) {
         mlog << Warning << "\ndo_sub_command() -> "
               << cs << "\n\n";
      }
      else {
         mlog << Debug(3) << cs << "\n";
      }
   }
   nc_init_time = plus.init();

   //
   // Output accumulation time
   // Warning if accum1 < accum2.
   //
   if(plus.accum() < minus.accum()) {
      mlog << Warning << "\ndo_sub_command() -> "
           << "the first accumulation interval is less than the "
           << "second (" << sec_to_hhmmss(plus.accum()) << " < "
           << sec_to_hhmmss(minus.accum()) << ") for subtraction.\n\n";
   }
   nc_accum = plus.accum() - minus.accum();

   //
   // Initialize.
   //
   diff = plus;

   //
   // Update value for each grid point.
   //
   for(i=0, nxy=grid1.nx()*grid1.ny(); i<nxy; i++) {
      if(is_bad_data( diff.data()[i]) ||
         is_bad_data(minus.data()[i])) {
         diff.buf()[i] = bad_data_double;
      }
      else {
         diff.buf()[i] -= minus.data()[i];
      }
   }

   //
   // Write output.
   //
   if(!nc_out) open_nc(grid1);
   write_nc_data(nc_init_time, nc_valid_time, nc_accum, diff,
                 "diff", "");

   return;
}

////////////////////////////////////////////////////////////////////////

void do_derive_command() {
   Grid grid, cur_grid;
   DataPlane cur_dp, der_dp;
   DataPlane min_dp, max_dp, sum_dp, sum_sq_dp, vld_dp;
   MaskPlane mask;
   unixtime nc_init_time, nc_valid_time;
   int nc_accum, nc_accum_sum;
   int i, j, n_skip, nxy, n_vld;
   ConcatString derive_list_css;
   double v;

   //
   // Initialize
   //
   nc_init_time = nc_valid_time = (unixtime) 0;
   nc_accum = nc_accum_sum = nxy = 0;

   //
   // List of all requested field derivations.
   //
   derive_list_css = write_css(derive_list);

   //
   // Check for at least one derivation option.
   //
   if(derive_list.n() == 0) {
      mlog << Error << "\ndo_derive_command() -> "
           << "at least one derivation option must be specified!\n\n";
      exit(1);
   }

   mlog << Debug(2)
        << "Performing derivation command (" << derive_list_css
        << ") for " << n_files << " files.\n";

   //
   // Loop through the input files.
   //
   for(i=0, n_vld=0; i<n_files; i++) {

      //
      // Read the current field.
      // Keep track of the file number of successful file reads.
      //
      if(get_field(file_list[i].c_str(), field_list[i].c_str(),
                   0, 0, cur_grid, cur_dp, false)) n_vld++;
      else                                         continue;

      //
      // Initialize
      //
      if(grid.nx() == 0 || grid.ny() == 0) {

         //
         // Initialize the grid.
         //
         grid = cur_grid;
         nxy  = grid.nx() * grid.ny();

         //
         // Initialize the timing information.
         //
         nc_init_time  = cur_dp.init();
         nc_valid_time = cur_dp.valid();
         nc_accum      = cur_dp.accum();
         nc_accum_sum  = cur_dp.accum();

         //
         // Initialize to bad data.
         //
         der_dp.set_size(grid.nx(), grid.ny());
         der_dp.set_constant(bad_data_double);
         min_dp  = der_dp;
         max_dp  = der_dp;

         //
         // Initialize to 0.
         //
         der_dp.set_constant(0.0);
         sum_dp    = der_dp;
         sum_sq_dp = der_dp;
         vld_dp    = der_dp;
      }
      //
      // Check for grid mismatch.
      //
      else if(grid != cur_grid) {
         mlog << Error << "\ndo_derive_command() -> "
              << "the input fields must be on the same grid.\n"
              << grid.serialize() << "\n" << cur_grid.serialize()
              << "\n\n";
         exit(1);
      }
      //
      // Update timing information.
      //
      else {

         // Output init time:
         //    If it changes, reset to 0.
         if(nc_init_time != cur_dp.init()) {
            nc_init_time = (unixtime) 0;
         }

         // Output valid time:
         //    If it changes, keep track of the maximum.
         if(nc_valid_time < cur_dp.valid()) {
            nc_valid_time = cur_dp.valid();
         }

         // Output accumulation time:
         //    If it changes, reset to 0.
         if(nc_accum != cur_dp.accum()) {
            nc_accum = 0;
         }
         nc_accum_sum  += cur_dp.accum();
      }

      //
      // Update sums and counts.
      //
      for(j=0; j<nxy; j++) {

         // Get current data value.
         v = cur_dp.data()[j];

         // Update valid counts.
         if(!is_bad_data(v)) vld_dp.buf()[j] += 1;
         else                continue;

         // Update min/max fields which may contain bad data.
         if(is_bad_data(min_dp.data()[j]) || v < min_dp.data()[j]) {
            min_dp.buf()[j] = v;
         }
         if(is_bad_data(max_dp.data()[j]) || v > max_dp.data()[j]) {
            max_dp.buf()[j] = v;
         }

         // Update the sums which do not have bad data.
         sum_dp.buf()[j]    += v;
         sum_sq_dp.buf()[j] += v*v;
      }
   } // end for i in n_files

   // Check for enough valid input files.
   if((double) n_vld/n_files < input_thresh) {
      mlog << Error << "\ndo_derive_command() -> "
           << n_vld << " of " << n_files << " (" << (double) n_vld/n_files
           << ") valid inputs does not meet the required input threshold ("
           << input_thresh << ").\n\n";
      exit(1);
   }

   //
   // Compute the valid data mask, relative the number of valid inputs.
   //
   mask.set_size(grid.nx(), grid.ny());
   for(j=0, n_skip=0; j<nxy; j++) {
      mask.buf()[j] = ((double) vld_dp.data()[j]/n_vld) >= vld_thresh;
      if(!mask.data()[j]) n_skip++;
   }

   mlog << Debug(2)
        << "Skipping " << n_skip << " of " << nxy << " grid points which "
        << "do not meet the valid data threshold (" << vld_thresh
        << ").\n";

   //
   // Apply the valid data mask.
   //
   apply_mask(min_dp,    mask);
   apply_mask(max_dp,    mask);
   apply_mask(sum_dp,    mask);
   apply_mask(sum_sq_dp, mask);

   //
   // Open the output file, if needed.
   //
   if(!nc_out) open_nc(grid);

   //
   // Loop through the derived fields.
   //
   for(i=0; i<derive_list.n(); i++) {

      //
      // Write the current derived field.
      //
      if(strcasecmp(derive_list[i].c_str(), "sum") == 0) {
         write_nc_data(nc_init_time, nc_valid_time, nc_accum_sum,
                       sum_dp, derive_list[i].c_str(), "Sum of ");
      }
      else if(strcasecmp(derive_list[i].c_str(), "min") == 0) {
         write_nc_data(nc_init_time, nc_valid_time, nc_accum,
                       min_dp, derive_list[i].c_str(), "Minimum Value of ");
      }
      else if(strcasecmp(derive_list[i].c_str(), "max") == 0) {
      write_nc_data(nc_init_time, nc_valid_time, nc_accum,
                    max_dp, derive_list[i].c_str(), "Maximum Value of ");
      }
      else if(strcasecmp(derive_list[i].c_str(), "range") == 0) {
         der_dp = max_dp;
         for(j=0; j<nxy; j++) {
            if(is_bad_data(max_dp.data()[j]) ||
               is_bad_data(min_dp.data()[j])) {
               der_dp.buf()[j] = bad_data_double;
            }
            else {
               der_dp.buf()[j] = max_dp.data()[j] - min_dp.data()[j];
            }
         }
         write_nc_data(nc_init_time, nc_valid_time, nc_accum,
                       der_dp, derive_list[i].c_str(), "Range of ");
      }
      else if(strcasecmp(derive_list[i].c_str(), "mean") == 0) {
         der_dp = sum_dp;
         for(j=0; j<nxy; j++) {
            if(is_bad_data(sum_dp.data()[j]) ||
               is_bad_data(vld_dp.data()[j]) ||
               is_eq(vld_dp.data()[j], 0.0)) {
               der_dp.buf()[j] = bad_data_double;
            }
            else {
               der_dp.buf()[j] = sum_dp.data()[j]/vld_dp.data()[j];
            }
         }
         write_nc_data(nc_init_time, nc_valid_time, nc_accum,
                       der_dp, derive_list[i].c_str(), "Mean Value of ");
      }
      else if(strcasecmp(derive_list[i].c_str(), "stdev") == 0) {
         der_dp = sum_dp;
         for(j=0; j<nxy; j++) {
            double s  = sum_dp.data()[j];
            double sq = sum_sq_dp.data()[j];
            double nd  = vld_dp.data()[j];
            if(is_bad_data(s) || is_bad_data(sq) ||
               is_bad_data(nd) || nd <= 1) {
               der_dp.buf()[j] = bad_data_double;
            }
            else {
               v = (sq - s*s/nd)/(nd-1);
               if(is_eq(v, 0.0)) v = 0.0;
               der_dp.buf()[j] = sqrt(v);
            }
         }
         write_nc_data(nc_init_time, nc_valid_time, nc_accum,
                       der_dp, derive_list[i].c_str(),
                       "Standard Deviation of ");
      }
      else if(strcasecmp(derive_list[i].c_str(), "vld_count") == 0) {
         write_nc_data(nc_init_time, nc_valid_time, nc_accum,
                       vld_dp, derive_list[i].c_str(), "Valid Data Count of ");
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

bool get_field(const char *filename, const char *cur_field,
               const unixtime get_init_ut, const unixtime get_valid_ut,
               Grid & grid, DataPlane & plane, bool error_out) {
   Met2dDataFileFactory factory;
   Met2dDataFile *mtddf = nullptr;
   GrdFileType ftype;
   VarInfoFactory var_fac;
   VarInfo *cur_var = nullptr;
   const char *method_name = "get_field() -> ";

   //
   // Track the read status.
   //
   bool status = true;

   //
   // Build the field config string.
   //
   ConcatString config_str = parse_config_str(cur_field);

   mlog << Debug(1)
        << "Reading data (" << config_str
        << ") from input file: " << filename << "\n";

   //
   // Parse the config string.
   //
   config.read_string(config_str.c_str());

   //
   // Get the gridded file type from config string, if present.
   //
   ftype = parse_conf_file_type(&config);

   //
   // Check that the file exists.
   //
   if(!file_exists(filename) &&
      !is_python_grdfiletype(ftype)) {
      log_missing_file(method_name, "input file", filename);
      status = false;
   }

   //
   // Open the data file.
   //
   if(status) {
      mtddf = factory.new_met_2d_data_file(filename, ftype);
      if(!mtddf) {
         mlog << Warning << "\n" << method_name
              << "can't open data file \"" << filename << "\"\n\n";
         status = false;
      }
   }

   //
   // Build a VarInfo object.
   //
   if(status) {
      cur_var = var_fac.new_var_info(mtddf->file_type());
      if(!cur_var) {
         mlog << Warning << "\n" << method_name
              << "unable to determine filetype of \"" << filename
              << "\"\n\n";
         status = false;
      }
   }

   //
   // Setup the VarInfo object and read the data.
   //
   if(status) {

      //
      // Initialize the VarInfo object with a config.
      //
      cur_var->set_dict(config);

      //
      // Set the VarInfo timing object.
      //
      if(get_valid_ut != 0) cur_var->set_valid(get_valid_ut);
      if(get_init_ut  != 0) cur_var->set_init(get_init_ut);

      //
      // Read the record of interest into a DataPlane object.
      //
      if(!mtddf->data_plane(*cur_var, plane)) {
         mlog << Warning << "\n" << method_name
              << "can't get data plane from file \"" << filename
              << "\"\n\n";
         status = false;
      }
   }

   //
   // Store grid and global VarInfo, if needed.
   //
   if(status) {

      grid = mtddf->grid();

      if(!var_info) {
         var_info = var_fac.new_var_info(mtddf->file_type());
         *var_info = *cur_var;
      }
   }

   //
   // Cleanup.
   //
   if(mtddf)   { delete mtddf;   mtddf   = (Met2dDataFile *) nullptr; }
   if(cur_var) { delete cur_var; cur_var = (VarInfo *)       nullptr; }

   //
   // Error out and exit, if requested.
   //
   if(!status && error_out) {
      mlog << Error << "\n" << method_name
           << "trouble reading data (" << config_str
           << ") from required input file: " << filename << "\n\n";
      exit(1);
   }

   return status;

}

////////////////////////////////////////////////////////////////////////

void open_nc(const Grid &grid) {
   ConcatString command_str;

   // List the output file.
   mlog << Debug(1)
        << "Creating output file: " << out_filename << "\n";

   // Create a new NetCDF file and open it.
   nc_out = open_ncfile(out_filename.c_str(), true);

   if(IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nopen_nc() -> "
           << "trouble opening output file " << out_filename
           << "\n\n";
      delete nc_out;
      nc_out = (NcFile *) nullptr;
      exit(1);
   }

   // Add global attributes.
   write_netcdf_global(nc_out, out_filename.c_str(), program_name.c_str());

   if(run_command == RunCommand::sum) {
      command_str << cs_erase
                  << "Sum: " << n_files
                  << " files with accumulations of "
                  << sec_to_hhmmss(in_accum) << '.';
   } else if(run_command == RunCommand::add) {
      command_str << cs_erase
                  << "Addition: " << n_files << " files.";
   }
   else if(run_command == RunCommand::sub) {
      command_str << cs_erase
                  << "Subtraction: "
                  << file_list[0] << " minus " << file_list[1];
   }
   else { // run_command = RunCommand::der
      command_str << cs_erase
                  << "Derive: " << write_css(derive_list) << " of "
                  << n_files << " files.";
   }

   add_att(nc_out, "RunCommand", command_str.c_str());

   // Add the projection information.
   write_netcdf_proj(nc_out, grid, lat_dim, lon_dim);

   // Add the lat/lon variables.
   write_netcdf_latlon(nc_out, &lat_dim, &lon_dim, grid);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nc_data(unixtime nc_init, unixtime nc_valid, int nc_accum,
                   const DataPlane &cur_dp, const char *derive_str,
                   const char *long_name_prefix) {
   ConcatString var_str, cs;
   StringArray sa;
   NcVar nc_var;

   if (!var_info) {
      mlog << Error << "\nwrite_nc_data() -> "
           << "var_info is null.\n\n";
      exit(1);
   }

   //
   // Write to the -name command line argument, if specified.
   //
   if(req_out_var_name.n() == n_out_var) {
      var_str = req_out_var_name[i_out_var];
   }
   //
   // Otherwise, build the output variable name from the VarInfo.
   //
   else {

      //
      // For zero accum, use the name and level, unless it includes
      // special characters.
      //
      if(nc_accum == 0) {
         var_str = var_info->name_attr();
         cs      = var_info->level_attr();
         if(!check_reg_exp("[\\(\\*\\,\\)]", cs.c_str())) var_str << "_" << cs;
      }
      //
      // For non-zero accum, use the name and accumulation interval.
      //
      else {

         //
         // Use the name prior to the first underscore.
         //
         cs      = var_info->name_attr();
         sa      = cs.split("_");
         var_str = sa[0];

         //
         // For an hourly accumulation interval, append _HH.
         //
         if(nc_accum%sec_per_hour == 0) {
            var_str.set_precision(2);
            var_str << '_' << HH(nc_accum/sec_per_hour);
         }
         //
         // For any other accumulation interval, append _HHMMSS.
         //
         else {
            var_str << "_" << sec_to_hhmmss(nc_accum);
         }
      }

      //
      // Append the derivation string.
      //
      if(run_command == RunCommand::der) var_str << "_" << derive_str;
   }

   mlog << Debug(2)
        << "Writing output variable \"" << var_str
        << "\" for the \"" << derive_str << "\" of \""
        << var_info->magic_str() << "\".\n";

   int deflate_level = compress_level;
   if(deflate_level < 0) deflate_level = config.nc_compression();

   // Define variable.
   nc_var = add_var(nc_out, var_str.c_str(), ncFloat,
                    lat_dim, lon_dim, deflate_level);

   // Add variable attributes.
   add_att(&nc_var, "name",  var_str.c_str());
   if(run_command == RunCommand::der) cs = long_name_prefix;
   else                               cs.clear();
   cs << var_info->long_name_attr();
   add_att(&nc_var, "long_name", cs.c_str());

   // Ouput level string.
   if(nc_accum != 0) {
      if(nc_accum%sec_per_hour == 0) {
         var_str << cs_erase << 'A' << (nc_accum/sec_per_hour);
      } else {
         var_str << cs_erase << 'A' << sec_to_hhmmss(nc_accum);
      }
   } else {
      var_str << cs_erase << var_info->level_attr();
   }

   add_att(&nc_var, "level", var_str.c_str());
   add_att(&nc_var, "units", var_info->units_attr().c_str());
   add_att(&nc_var, "_FillValue", bad_data_float);

   //
   // Add initialization, valid, and accumulation time info as
   // attributes to the nc_var.
   //
   if(nc_init == (unixtime) 0) nc_init = nc_valid;

   //
   // Write out the times.
   //
   write_netcdf_var_times(&nc_var, nc_init, nc_valid, nc_accum);

   //
   // Write the data.
   //
   if(!put_nc_data_with_dims(&nc_var, cur_dp.data(),
                             cur_dp.ny(), cur_dp.nx())) {
      mlog << Error << "\nwrite_nc_data() -> "
           << "error with nc_var->put()\n\n";
      exit(1);
   }

   //
   // Increment the counter.
   //
   i_out_var++;

   return;
}

////////////////////////////////////////////////////////////////////////

void close_nc() {

   //
   // List the output file.
   //
   mlog << Debug(1)
        << "Closing output file: " << out_filename << "\n";

   //
   // Clean up.
   //
   if(nc_out)    { delete nc_out;   nc_out   = (NcFile *)  nullptr; }
   if(var_info ) { delete var_info; var_info = (VarInfo *) nullptr; }

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString parse_config_str(const char *s) {
   ConcatString config_str;

   if(is_timestring(s)) {
      config_str.format("name=\"APCP\"; level=\"A%s\";", s);
   }
   else {
      config_str = s;
   }

   return config_str;
}

////////////////////////////////////////////////////////////////////////

bool is_timestring(const char * text) {

   if(is_hh(text))     return true;
   if(is_hhmmss(text)) return true;

   return false;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\t[-sum] sum_args |\n"
        << "\t-add input_files |\n"
        << "\t-subtract input_files |\n"
        << "\t-derive stat_list input_files\n"
        << "\tout_file\n"
        << "\t[-field string]\n"
        << "\t[-name list]\n"
        << "\t[-input_thresh n]\n"
        << "\t[-vld_thresh n]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"-sum sum_args\" indicates that data from "
        << "multiple files should be summed up using the arguments "
        << "provided.\n"

        << "\t\t\"-add input_files\" indicates that data from one or "
        << "more input files should be added together.\n"

        << "\t\t\"-subtract input_files\" indicates that data from "
        << "exactly two input files should be subtracted.\n"

        << "\t\t\"-derive stat_list input_files\" indicates that "
        << "the comma-separated list of statistics in \"stat_list\"\n"
        << "\t\t(" << derive_options << ") should be derived from one "
        << "or more input files.\n"

        << "\t\t\"out_file\" is the name of the output NetCDF file to "
        << "be written (required).\n"

        << "\t\t\"-field string\" may be used multiple times to define "
        << "the data to be processed (optional).\n"

        << "\t\t\"-name list\" is a comma-separated list of output "
        << "variable names to be written to the \"out_file\" "
        << "(optional).\n"

        << "\t\t\"-input_thresh\" overrides the default required ratio "
        << "of valid input files (" << input_thresh << ") (optional).\n"

        << "\t\t\"-vld_thresh\" overrides the default required ratio "
        << "of valid data at each grid point (" << vld_thresh
        << ") (optional).\n"

        << "\t\t\"-log file\" write log messages to the specified file "
        << "(optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t\"-compress level\" overrides the compression level of "
        << "NetCDF variable ("  << config.nc_compression()
        << ") (optional).\n\n"

        << "\tSUM_ARGS:\n"
        << "\t\tinit_time\n"
        << "\t\tin_accum\n"
        << "\t\tvalid_time\n"
        << "\t\tout_accum\n"
        << "\t\t[-pcpdir path]\n"
        << "\t\t[-pcprx reg_exp]\n\n"

        << "\t\twhere\t\"init_time\" is the initialization time of the "
        << "input data files in YYYYMMDD[_HH[MMSS]] format "
        << "(required).\n"

        << "\t\t\t\"in_accum\" is the accumulation interval of the "
        << "input data files in HH[MMSS] format (required).\n"

        << "\t\t\t\"valid_time\" is the desired output valid time in "
        << "YYYYMMDD[_HH[MMSS]] format (required).\n"

        << "\t\t\t\"out_accum\" is the desired output accumulation "
        << "interval in HH[MMSS] format (required).\n"

        << "\t\t\t\"-pcpdir path\" is used one or more times to "
        << "override the default search directory (" << default_pcp_dir
        << ") and allows wildcards (optional).\n"

        << "\t\t\t\"-pcprx reg_exp\" overrides the default regular "
        << "expression for input file naming convention ("
        << default_reg_exp << ") (optional).\n\n"

        << "\t\tNote:\tSpecifying \"-sum\" is not required since it is "
        << "the default behavior.\n"
        << "\t\t\tNote: Set \"init_time\" to 00000000_000000 when "
        << "summing observation files.\n\n"

        << "\tINPUT_FILES:\n"
        << "\t\tfile_1 config_str_1 ... file_n config_str_n | \n"
        << "\t\tfile_1 ... file_n |\n"
        << "\t\tinput_file_list\n\n"

        << "\t\twhere\t\"file_i\" is the name of the i-th input "
        << "gridded data file.\n"

        << "\t\t\t\"config_str_i\" is the field to be extracted from "
        << "the i-th gridded data file.\n"

        << "\t\t\t\"input_file_list\" is an ASCII file containing a list of "
        << "gridded data files.\n\n"

        << "\t\tNote:\tFor \"-subtract\", exactly 2 input files must "
        << "be specified.\n"
        << "\t\tNote:\tThe \"-field\" option is required unless a "
        << "\"config_str\" is specified for each input file.\n"
        << "\t\tNote:\tThe \"config_str\" and \"-field\" strings may "
        << "be set to a timestring in HH[MMSS] format\n"
        << "\t\t\tfor accumulated precipitation or a full "
        << "configuration string.\n"

        << "\n" << flush;

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_sum(const StringArray &) {
   run_command = RunCommand::sum;
   derive_list.clear();
   derive_list.add("sum");
}

////////////////////////////////////////////////////////////////////////

void set_add(const StringArray &) {
   run_command = RunCommand::add;
   derive_list.clear();
   derive_list.add("sum");
}

////////////////////////////////////////////////////////////////////////

void set_subtract(const StringArray &) {
   run_command = RunCommand::sub;
   derive_list.clear();
   derive_list.add("diff");
}

////////////////////////////////////////////////////////////////////////

void set_derive(const StringArray & a) {
   run_command = RunCommand::der;
   derive_list.clear();

   StringArray sa;
   sa.add_css(a[0]);

   //
   // Parse the derivation options
   //
   for(int i=0; i<sa.n(); i++) {
      if(!strcasestr(derive_options, sa[i].c_str())) {
         mlog << Error << "\nset_derive() -> "
           << "\"" << sa[i] << "\" is not a supported option for the "
           << "\"-derive\" command!\n\n";
         exit(1);
      }
      derive_list.add(sa[i]);
   }
}

////////////////////////////////////////////////////////////////////////

void set_pcpdir(const StringArray & a) {
   pcp_dir.add(a);
}

////////////////////////////////////////////////////////////////////////

void set_pcprx(const StringArray & a) {
   pcp_reg_exp = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_field(const StringArray & a) {
   req_field_list.add(a[0]);
   field_option_used = true;
}

////////////////////////////////////////////////////////////////////////

void set_name(const StringArray & a) {
   req_out_var_name.add_css(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_input_thresh(const StringArray & a) {
   input_thresh = atof(a[0].c_str());
   if(input_thresh > 1 || input_thresh < 0) {
      mlog << Error << "\nset_input_thresh() -> "
           << "the \"-input_thresh\" command line option (" << input_thresh
           << ") must be set between 0 and 1!\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

void set_vld_thresh(const StringArray & a) {
   vld_thresh = atof(a[0].c_str());
   if(vld_thresh > 1 || vld_thresh < 0) {
      mlog << Error << "\nset_vld_thresh() -> "
           << "the \"-vld_thresh\" command line option (" << vld_thresh
           << ") must be set between 0 and 1!\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

void set_compress(const StringArray & a) {
   compress_level = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////
