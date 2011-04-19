
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////
//
//   Filename:   pcp_combine.cc
//
//   Description:
//      Based on user specified time periods, this tool combines
//      one or more grib precipitation files into a single
//      precipitation file and will dump out the output in NetCDF
//      format.
//
//      The user must specify the following on the command line:
//      init_time_time, in_accum_time, valid_time, out_accum_time,
//      and out_file name
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    01-24-07  Halley Gotway  New
//   001    12-07-07  Halley Gotway  Change time format from
//                    YYYY-MM-DD_HH:MM:SS to YYYYMMDD_HHMMSS
//   002    01-31-08  Halley Gotway  Add support for the -add and
//                    -subtract options
//   003    09/23/08  Halley Gotway  Change argument sequence for the
//                    GRIB record access routines.
//   004    02/20/09  Halley Gotway  Append _HH to the variable name
//                    for non-zero accumulation times.
//   005    12/23/09  Halley Gotway  Call the library read_pds routine.
//   006    05/21/10  Halley Gotway  Enhance to search multiple
//                    -pcp_dir directory arguments.
//   007    06/25/10  Halley Gotway  Allow times to be specified in
//                    HH[MMSS] and YYYYMMDD[_HH[MMSS]] format.
//   008    06/30/10  Halley Gotway  Enhance grid equality checks.
//   009    07/27/10  Halley Gotway  Enhance to allow addition of any
//                    number of input files/accumulation intervals.
//                    Add lat/lon variables to NetCDF.
//   010    04/19/11  Halley Gotway  Bugfix for -add option.
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

#include "vx_grib_classes.h"
#include "vx_gdata.h"
#include "vx_met_util.h"
#include "grid.h"
#include "vx_statistics.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

// Constants
static const char *program_name    = "pcp_combine";
static const char *zero_time_str   = "00000000_000000";
static const char *default_pcp_dir = ".";
static const char *default_reg_exp = ".*";

// Run Command enumeration
enum RunCommand { sum = 0, add = 1, sub = 2 };

// Variables for top-level command line arguments
static RunCommand run_command = sum;
static int grib_code = apcp_grib_code;
static int grib_ptv  = 2;
static GCInfo gc_info;
static int verbosity = 1;

// Variables common to all commands
static ConcatString out_file;
static float       *pcp_data = (float *) 0;

// Variables for the sum command
static unixtime     init_time;
static int          in_accum;
static unixtime     valid_time;
static int          out_accum;
static StringArray  pcp_dir;
static ConcatString pcp_reg_exp(default_reg_exp);

// Variables for the add and subtract commands
static ConcatString *in_file;
static int          *accum;
static int           n_files;

///////////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);

static void process_sum_args(int, char **, int);
static void process_add_sub_args(int, char **, int);

static void do_sum_command(int, char **);
static void do_add_command(int, char **);
static void do_sub_command(int, char **);

static void sum_grib_files(Grid &, GribRecord &);
static int  search_pcp_dir(const char *, const unixtime, char *&);
static void check_file_time(char *, unixtime, int &);
static void get_field(const char *, int, WrfData &, unixtime &, unixtime &,
                      Grid &, GribRecord &);

static void write_netcdf(unixtime, unixtime, int, Grid &, GribRecord &);
static void clean_up();
static void usage(int, char **);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   //
   // Set handler to be called for memory allocation error
   //
   set_new_handler(oom);

   //
   // Process the command line arguments
   //
   process_command_line(argc, argv);

   //
   // Perform the requested job command
   //
   if     (run_command == sum) do_sum_command(argc, argv);
   else if(run_command == add) do_add_command(argc, argv);
   else                        do_sub_command(argc, argv);

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   int i, i_args;

   //
   // Check for the minimum number of arguments
   //
   if(argc < 4) {
      usage(argc, argv);
      exit(1);
   }

   //
   // Default to running the sum command
   //
   run_command = sum;
   i_args      = 1;

   //
   // Process the top-level command line arguments
   //
   for(i=0; i<argc; i++) {

      if(strcmp(argv[i], "-sum") == 0) {
         run_command = sum;
         i_args      = i+1;
      }
      else if(strcmp(argv[i], "-add") == 0) {
         run_command = add;
         i_args      = i+1;
      }
      else if(strcmp(argv[i], "-subtract") == 0) {
         run_command = sub;
         i_args      = i+1;
      }
      else if(strcmp(argv[i], "-gc") == 0) {
         grib_code = atoi(argv[i+1]);
         i++;
      }
      else if(strcmp(argv[i], "-ptv") == 0) {
         grib_ptv = atoi(argv[i+1]);
         i++;
      }
      else if(strcmp(argv[i], "-v") == 0) {
         verbosity = atoi(argv[i+1]);
         i++;
      }

      //
      // Optional sum arguments
      //
      else if(strcmp(argv[i], "-pcpdir") == 0) {
         pcp_dir.add(argv[i+1]);
         i++;
      }
      else if(strcmp(argv[i], "-pcprx") == 0) {
         pcp_reg_exp = argv[i+1];
         i++;
      }

      //
      // Unrecognized flags
      //
      else if(argv[i][0] == '-') {
         cerr << "\n\nERROR: process_command_line() -> "
              << "Unrecognized command line argument: " << argv[i]
              << "\n\n" << flush;
         exit(1);
      }
   }

   //
   // Process the specific command arguments
   //
   if(run_command == sum) process_sum_args(argc, argv, i_args);
   else                   process_add_sub_args(argc, argv, i_args);

   //
   // If pcp_dir is not set, set it to the current directory.
   //
   if(pcp_dir.n_elements() == 0) pcp_dir.add(default_pcp_dir);

   //
   // Deallocate memory and clean up
   //
   clean_up();

   return;
}

////////////////////////////////////////////////////////////////////////

void process_sum_args(int argc, char **argv, int i_args) {

   //
   // Check the number of arguments provided
   //
   if(i_args + 5 > argc) {
      cerr << "\n\nERROR: process_sum_args() -> "
           << "Not enough arguments provided.\n\n" << flush;
      usage(argc, argv);
      exit(1);
   }

   //
   // Parse the sum arguments
   //

   //
   // Init time
   //
   if(strcmp(argv[i_args], zero_time_str) == 0) {
      init_time = (unixtime) 0;
   }
   else {
      init_time = timestring_to_unix(argv[i_args]);
   }

   //
   // Input accumulation
   //
   in_accum = timestring_to_sec(argv[i_args+1]);

   //
   // Valid time
   //
   if(strcmp(argv[i_args+2], zero_time_str) == 0) {
      valid_time = (unixtime) 0;
   }
   else {
      valid_time = timestring_to_unix(argv[i_args+2]);
   }

   //
   // Output accumulation
   //
   out_accum = timestring_to_sec(argv[i_args+3]);

   //
   // Out file
   //
   out_file = argv[i_args+4];

   //
   // Check that accumulation intervals are greater than zero
   //
   if(in_accum <= 0 || out_accum <= 0) {
      cerr << "\n\nERROR: process_sum_args() -> "
           << "The input accumulation interval (" << argv[i_args+1] 
           << ") and output accumulation interval (" << argv[i_args+3]
           << ") must be greater than zero.\n\n" << flush;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_add_sub_args(int argc, char **argv, int i_args) {
   int i;

   //
   // Check for a minimum of two addition arguments
   //
   if( (run_command == add && i_args + 2 > argc) ||
       (run_command == sub && i_args + 5 > argc) ) {
      cerr << "\n\nERROR: process_add_sub_args() -> "
           << "Not enough arguments provided.\n\n" << flush;
      usage(argc, argv);
      exit(1);
   }

   //
   // Figure out the number of files provided
   //
   for(i=i_args, n_files=0; i<argc; i++) {

      // Only check accumulation interval for enough args
      if(i+1 < argc) {
         if(!is_hh(argv[i+1]) && !is_hhmmss(argv[i+1])) break;
         else {
            n_files++; // Increment file count
            i++;       // Advance past next file
         }
      }
   } // end for i

   //
   // Allocate memory for file names and accumulations
   //
   in_file = new ConcatString [n_files];
   accum   = new int [n_files];

   //
   // Store the input files and accumulations
   //
   for(i=0; i<n_files; i++) {
      in_file[i] = argv[i_args + i*2];
      accum[i]   = timestring_to_sec(argv[i_args + i*2 + 1]);
   }

   //
   // Store the output file
   //
   out_file = argv[i_args + n_files*2];

   return;
}

////////////////////////////////////////////////////////////////////////

void do_sum_command(int argc, char **argv) {
   Grid grid;
   GribRecord rec;
   int lead_time;
   char init_time_str[max_str_len], valid_time_str[max_str_len];
   char in_accum_str[max_str_len], out_accum_str[max_str_len];
   char lead_time_str[max_str_len];

   //
   // Compute the lead time
   //
   lead_time = valid_time - init_time;

   //
   // Build time strings
   //
   if(init_time != 0) unix_to_yyyymmdd_hhmmss(init_time, init_time_str);
   else               strcpy(init_time_str, zero_time_str);
   sec_to_hhmmss(in_accum, in_accum_str);
   unix_to_yyyymmdd_hhmmss(valid_time, valid_time_str);
   sec_to_hhmmss(out_accum, out_accum_str);
   sec_to_hhmmss(lead_time, lead_time_str);

   if(verbosity > 0) {

      cout << "Performing sum command: "
           << "Init/In_Accum/Valid/Out_Accum Times = "
           << init_time_str << "/" << in_accum_str << "/"
           << valid_time_str << "/" << out_accum_str << "\n" << flush;
   }

   //
   // Check that the output accumulation time is not greater than
   // the lead time, except when init_time = 0 for observations.
   //
   if(out_accum > lead_time && init_time != (unixtime) 0) {
      cerr << "\n\nERROR: do_sum_command() -> "
           << "The output accumulation time (" << out_accum_str
           << ") cannot be greater than the lead time ("
           << lead_time_str << ").\n\n"
           << flush;
      exit(1);
   }

   //
   // Check that the output accumulation time is divisible by the input
   // accumulation time.
   //
   if(out_accum%in_accum != 0) {
      cerr << "\n\nERROR: do_sum_command() -> "
           << "The output accumulation time (" << out_accum_str
           << ") must be divisible by the input accumulation "
           << "time (" << in_accum_str << ").\n\n"
           << flush;
      exit(1);
   }

   //
   // Check that the lead time is divisible by the the input
   // accumulation time except when init_time = 0 for observations.
   //
   if(lead_time%in_accum != 0 && init_time != (unixtime) 0) {
      cerr << "\n\nERROR: do_sum_command() -> "
           << "The lead time (" << lead_time_str
           << ") must be divisible by the input accumulation time ("
           << in_accum_str << ").\n\n" << flush;
      exit(1);
   }

   //
   // Find and sum up the matching precipitation files
   //
   sum_grib_files(grid, rec);

   //
   // Write the combined precipitation field out in NetCDF format
   //
   if(verbosity > 0) {
      cout << "Writing output file: " << out_file << "\n" << flush;
   }
   write_netcdf(init_time, valid_time, out_accum, grid, rec);

   return;
}

////////////////////////////////////////////////////////////////////////

void sum_grib_files(Grid &grid, GribRecord &rec) {
   int i, j, n, x, y;
   char valid_str[max_str_len];
   char in_accum_str[max_str_len], out_accum_str[max_str_len];
   double v;
   Grid gr;
   WrfData wd;

   //
   // Build time strings
   //
   sec_to_hhmmss(in_accum, in_accum_str);
   sec_to_hhmmss(out_accum, out_accum_str);

   //
   // Grib file info
   //
   unixtime *pcp_times;
   char    **pcp_files;
   int      *pcp_recs;

   //
   // Setup the GCInfo object
   //
   gc_info.code = grib_code;
   gc_info.lvl_type = AccumLevel;
   gc_info.lvl_1 = in_accum;
   gc_info.lvl_2 = in_accum;

   //
   // Compute the number of forecast precipitation files to be found,
   // and allocate memory to store their names and times
   //
   n_files   = out_accum/in_accum;
   pcp_times = new unixtime [n_files];
   pcp_files = new char * [n_files];
   pcp_recs  = new int [n_files];

   if(verbosity > 0) {
      cout << "Searching for " << n_files << " files "
           << "with accumulation times of " << in_accum_str
           << " to sum to a total accumulation time of "
           << out_accum_str << ".\n" << flush;
   }

   //
   // Compute the valid times for the precipitation files
   // to be found.
   //
   for(i=0; i<n_files; i++) {
      pcp_times[i] = valid_time - i*in_accum;
      pcp_files[i] = new char [PATH_MAX];
   }

   //
   // Search for each file time.
   //
   for(i=0; i<n_files; i++) {

      //
      // Search in each directory for the current file time.
      //
      for(j=0; j<pcp_dir.n_elements(); j++) {

         pcp_recs[i] = search_pcp_dir(pcp_dir[j], pcp_times[i], pcp_files[i]);

         if(pcp_recs[i] != -1) {

            if(verbosity > 1) {

               unix_to_yyyymmdd_hhmmss(pcp_times[i], valid_str);

               cout << "[" << i+1 << "] File " << pcp_files[i]
                    << " matches valid time of " << valid_str
                    << "\n" << flush;
            }
            break;
         } // end if

      } // end for j

      //
      // Check for no matching file found
      //
      if(pcp_recs[i] == -1) {

         unix_to_yyyymmdd_hhmmss(pcp_times[i], valid_str);

         cerr << "\n\nERROR: sum_grib_files() -> "
              << "Cannot find a file with a valid time of "
              << valid_str << " and accumulation time of "
              << in_accum_str << " matching the regular "
              << "expression \"" << pcp_reg_exp << "\"\n\n"
              << flush;
         exit(1);
      }

   } // end for i

   //
   // Open each of the files found and parse the data.
   //
   for(i=0; i<n_files; i++) {

      if(verbosity > 0) {
         cout << "[" << i+1 << "] Reading input file: " << pcp_files[i]
              << "\n" << flush;
      }

      read_grib_record(pcp_files[i], rec, pcp_recs[i],
                       gc_info, wd, gr, verbosity);

      //
      // For the first file processed store the grid, allocate memory
      // to store the precipitation sums, and initialize the sums
      //
      if(i == 0) {
         grid = gr;
         pcp_data = new float [grid.nx()*grid.ny()];

         //
         // Initialize the precipitation sums
         //
         for(x=0; x<grid.nx(); x++) {
            for(y=0; y<grid.ny(); y++) {
               pcp_data[wd.two_to_one(x, y)] = wd.get_xy_double(x, y);
            }
         }
      } // end if i == 0
      else {

         //
         // Check to make sure the grid stays the same
         //
         if(!(grid == gr)) {
            cerr << "\n\nERROR: sum_grib_files() -> "
                 << "The grid must remain the same for all "
                 << "data files.\n\n" << flush;
            exit(1);
         }

         //
         // Increment the precipitation sums keeping track of the bad data values
         //
         for(x=0; x<grid.nx(); x++) {
            for(y=0; y<grid.ny(); y++) {

               n = wd.two_to_one(x, y);
               v = wd.get_xy_double(x, y);

               if(is_bad_data(pcp_data[n]) ||
                  is_bad_data(v)) {
                  pcp_data[n] = bad_data_float;
               }
               else {
                  pcp_data[n] += v;
               }
            }
         }
      } // end else

   } // end for i

   //
   // Deallocate any memory that was allocated above
   //
   for(i=0; i<n_files; i++) {
      if(pcp_files[i]) { delete [] pcp_files[i]; pcp_files[i] = (char *) 0; }
   }
   if(pcp_files) { delete [] pcp_files; pcp_files = (char **) 0; }
   if(pcp_times) { delete [] pcp_times; pcp_times = (unixtime *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

int search_pcp_dir(const char *cur_dir, const unixtime cur_ut, char *&cur_file) {
   int i_rec;
   struct dirent *dirp;
   DIR *dp;

   //
   // Find the files matching the specified regular expression with
   // the correct valid and accumulation times.
   //
   if((dp = opendir(cur_dir)) == NULL ) {
      cerr << "\n\nERROR: search_pcp_dir() -> "
           << "Cannot open precipitation directory "
           << cur_dir << "\n\n" << flush;
      exit(1);
   }

   //
   // Initialize the record index to not found.
   //
   i_rec = -1;

   //
   // Process each file contained in the directory.
   //
   while((dirp = readdir(dp)) != NULL) {

      //
      // Ignore any hidden files.
      //
      if(dirp->d_name[0] == '.') continue;

      //
      // Check the file name for a matching regular expression.
      //
      if(check_reg_exp(pcp_reg_exp, dirp->d_name) == true) {

         //
         // Check the current file for matching initialization,
         // valid, lead, and accumulation times.
         //
         sprintf(cur_file, "%s/%s", cur_dir, dirp->d_name);
         check_file_time(cur_file, cur_ut, i_rec);

         if(i_rec != -1) break;
      } // end if
   } // end while

   if(closedir(dp) < 0) {
      cerr << "\n\nERROR: search_pcp_dir() -> "
           << "Cannot close precipitation directory "
           << cur_dir << "\n\n" << flush;
      exit(1);
   }

   return(i_rec);
}

////////////////////////////////////////////////////////////////////////

void check_file_time(char *file, unixtime pcp_valid, int &i_gc) {
   GribFile grib_file;
   GribRecord rec;
   int i, bms_flag, file_accum;
   unixtime file_init, file_valid;

   i_gc = -1;

   //
   // Open the precipitation grib file
   //
   if( !(grib_file.open(file)) ) {
      cout << "***WARNING*** check_file_time() -> "
           << "can't open precipitation grib file: "
           << file << "\n" << flush;
      return;
   }

   //
   // Find the grib record containing the accumulated precipitation.
   // A Grib file may contain multiple records of accumulated
   // precipitation.  Search grid records for one with the expected
   // accumulation interval.
   //
   for(i=0; i< grib_file.n_records(); i++) {

      if(grib_file.gribcode(i) == grib_code) {

         //
         // Read the record containing accumulated precip
         //
         grib_file.seek_record(i);
         grib_file >> rec;

         //
         // Read the Product Description Section
         //
         read_pds(rec, bms_flag, file_init, file_valid, file_accum);

         //
         // The file matches if the issuance times, valid times, and
         // accumulation times match
         //
         if((file_init == init_time || init_time == (unixtime) 0) &&
            (file_valid == pcp_valid) &&
            (file_accum == in_accum)) {
            i_gc = i;
            break;
         }
      } // end if
   } // end for

   grib_file.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void do_add_command(int argc, char **argv) {
   WrfData wd;
   GribRecord rec;
   Grid grid1, grid2;
   unixtime init_time1, init_time2;
   unixtime valid_time1, valid_time2;
   unixtime nc_init_time, nc_valid_time;
   int i, x, y, n, nc_accum;
   double v;
   char init_time1_str[max_str_len], init_time2_str[max_str_len];
   char valid_time1_str[max_str_len], valid_time2_str[max_str_len];
   char accum1_str[max_str_len], accum2_str[max_str_len];

   if(verbosity > 0) {
      cout << "Performing addition command for " << n_files
           << " files.\n" << flush;
   }

   //
   // Loop through each of the input files
   //
   for(i=0; i<n_files; i++) {

      //
      // Initialize for the first file
      //
      if(i == 0) {

         //
         // Read current field
         //
         if(verbosity > 0) {
            cout << "Reading input file: " << in_file[0] << "\n" << flush;
         }
         get_field(in_file[i].text(), accum[i], wd, init_time1, valid_time1, grid1, rec);

         //
         // Build time strings
         //
         unix_to_yyyymmdd_hhmmss(init_time1, init_time1_str);
         unix_to_yyyymmdd_hhmmss(valid_time1, valid_time1_str);
         sec_to_hhmmss(accum[i], accum1_str);

         // Initialize output times
         nc_init_time  = init_time1;
         nc_valid_time = valid_time1;
         nc_accum      = accum[i];

         //
         // Allocate space to store the sums and initialize
         //
         pcp_data = new float [grid1.nx()*grid1.ny()];
         for(n=0; n<grid1.nx()*grid1.ny(); n++) pcp_data[n] = 0.0;
      }
      //
      // Perform checks for multiple files
      //
      else {

         //
         // Read current field
         //
         if(verbosity > 0) {
            cout << "Reading input file: " << in_file[i] << "\n" << flush;
         }
         get_field(in_file[i].text(), accum[i], wd, init_time2, valid_time2, grid2, rec);

         //
         // Build time strings
         //
         unix_to_yyyymmdd_hhmmss(init_time2, init_time2_str);
         unix_to_yyyymmdd_hhmmss(valid_time2, valid_time2_str);
         sec_to_hhmmss(accum[i], accum2_str);

         //
         // Check for the same grid dimensions
         //
         if(!(grid1 == grid2)) {
            cerr << "\n\nERROR: do_add_command() -> "
                 << "the two input fields must be on the same grid\n\n"
                 << flush;
            exit(1);
         }

         // Output init time
         if(nc_init_time != init_time2) nc_init_time = (unixtime) 0;

         // Output valid time
         if(nc_valid_time < valid_time2) nc_valid_time = valid_time2;

         // Output accumulation time
         nc_accum += accum[i];

      }

      //
      // Increment sums for each grid point
      //
      for(x=0; x<grid1.nx(); x++) {
         for(y=0; y<grid1.ny(); y++) {

            n = wd.two_to_one(x, y);
            v = wd.get_xy_double(x, y);

            // Check for bad data
            if(is_bad_data(v)) pcp_data[n] = bad_data_double;
            // Otherwise, increment sums
            else               pcp_data[n] += v;

         } // end for y
      } // end for x
   } // end for i

   //
   // Write the combined precipitation field out in NetCDF format
   //
   if(verbosity > 0) {
      cout << "Writing output file: " << out_file << "\n" << flush;
   }
   write_netcdf(nc_init_time, nc_valid_time, nc_accum, grid1, rec);

   return;
}

////////////////////////////////////////////////////////////////////////

void do_sub_command(int argc, char **argv) {
   WrfData wd1, wd2;
   GribRecord rec;
   Grid grid1, grid2;
   unixtime init_time1, init_time2;
   unixtime valid_time1, valid_time2;
   unixtime nc_init_time, nc_valid_time;
   int x, y, n, nc_accum;
   double v1, v2, v;
   char init_time1_str[max_str_len], init_time2_str[max_str_len];
   char valid_time1_str[max_str_len], valid_time2_str[max_str_len];
   char accum1_str[max_str_len], accum2_str[max_str_len];

   //
   // Check for exactly two input files
   //
   if(n_files != 2) {
      cerr << "\n\nERROR: do_sub_command() -> "
           << "you must specify exactly two input files for subtraction\n\n"
           << flush;
      exit(1);
   }

   //
   // Read the two specified Grib files
   //
   if(verbosity > 0) {
      cout << "Reading input file: " << in_file[0] << "\n" << flush;
   }
   get_field(in_file[0].text(), accum[0], wd1, init_time1, valid_time1, grid1, rec);

   if(verbosity > 0) {
      cout << "Reading input file: " << in_file[1] << "\n" << flush;
   }
   get_field(in_file[1].text(), accum[1], wd2, init_time2, valid_time2, grid2, rec);

   //
   // Build time strings
   //
   unix_to_yyyymmdd_hhmmss(init_time1, init_time1_str);
   unix_to_yyyymmdd_hhmmss(init_time2, init_time2_str);
   unix_to_yyyymmdd_hhmmss(valid_time1, valid_time1_str);
   unix_to_yyyymmdd_hhmmss(valid_time2, valid_time2_str);
   sec_to_hhmmss(accum[0], accum1_str);
   sec_to_hhmmss(accum[1], accum2_str);

   //
   // Check for the same grid dimensions
   //
   if(!(grid1 == grid2)) {
      cerr << "\n\nERROR: do_sub_command() -> "
           << "the two input fields must be on the same grid\n\n"
           << flush;
      exit(1);
   }

   //
   // Compute output accumulation, initialization, and valid times
   // for the subtract command.
   //
   if(verbosity > 0) {
      cout << "Performing subtraction command.\n" << flush;
   }

   //
   // Output valid time
   //
   nc_valid_time = valid_time1;

   //
   // Output initialization time
   // Error if init_time1 != init_time2.
   //
   if(init_time1 != init_time2) {
      cerr << "\n\nERROR: do_sub_command() -> "
           << "init_time1 (" << init_time1_str
           <<  ") must be equal to init_time2 (" << init_time2_str
           << ") for subtraction.\n" << flush;
      exit(1);
   }
   nc_init_time = init_time1;

   //
   // Output accumulation time
   // Error if accum1 < accum2.
   //
   if(accum[0] < accum[1]) {
      cerr << "\n\nERROR: do_sub_command() -> "
           << "accum1 (" << accum1_str
           <<  ") must be greater than accum2 ("
           << accum2_str << ") for subtraction.\n" << flush;
      exit(1);
   }
   nc_accum = accum[0] - accum[1];

   //
   // Allocate space to store the differences
   //
   pcp_data = new float [wd1.get_nx()*wd1.get_ny()];

   //
   // Perform the specified command for each grid point
   //
   for(x=0; x<wd1.get_nx(); x++) {
      for(y=0; y<wd1.get_ny(); y++) {

         n  = wd1.two_to_one(x, y);
         v1 = wd1.get_xy_double(x, y);
         v2 = wd2.get_xy_double(x, y);

         // Check for bad data
         if(is_bad_data(v1) ||
            is_bad_data(v2))         v = bad_data_double;

         // Perform subtraction
         else                        v = v1 - v2;

         // Store the new value
         pcp_data[n] = v;

      } // end for y
   } // end for x

   //
   // Write the combined precipitation field out in NetCDF format
   //
   if(verbosity > 0) {
      cout << "Writing output file: " << out_file << "\n" << flush;
   }
   write_netcdf(nc_init_time, nc_valid_time, nc_accum, grid1, rec);

   return;
}

////////////////////////////////////////////////////////////////////////

void get_field(const char *get_file, int get_accum, WrfData &wd,
               unixtime &init_ut, unixtime &valid_ut,
               Grid &grid, GribRecord &rec) {
   GribFile grib_file;
   int bms_flag, rec_accum;
   char accum_str[max_str_len];

   //
   // Setup the GCInfo object
   //
   gc_info.code     = grib_code;
   gc_info.lvl_type = AccumLevel;
   gc_info.lvl_1    = get_accum;
   gc_info.lvl_2    = get_accum;

   //
   // Open the grib file specified
   //
   if( !(grib_file.open(get_file)) ) {
      cerr << "\n\nERROR: get_field() -> "
           << "can't open grib file: " << get_file
           << "\n\n" << flush;
      exit(1);
   }

   //
   // Find the requested field and accumulation interval
   //
   if(!get_grib_record(grib_file, rec, gc_info, wd,
                       grid, verbosity)) {

      sec_to_hhmmss(get_accum, accum_str);

      cerr << "\n\nERROR: get_field() -> "
           << "can't find grib code " << grib_code
           << " with accumulation of " << accum_str
           << " in GRIB file: " << get_file
           << "\n\n" << flush;
      exit(1);
   }

   //
   // Read the Product Description Section
   //
   read_pds(rec, bms_flag, init_ut, valid_ut, rec_accum);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_netcdf(unixtime nc_init, unixtime nc_valid, int nc_accum,
                  Grid &grid, GribRecord &rec) {
   char var_str[max_str_len];
   char tmp_str[max_str_len], tmp2_str[max_str_len];
   char command_str[max_str_len];
   char accum1_str[max_str_len], accum2_str[max_str_len];

   NcFile *f_out   = (NcFile *) 0;
   NcDim  *lat_dim = (NcDim *)  0;
   NcDim  *lon_dim = (NcDim *)  0;
   NcVar  *pcp_var = (NcVar *)  0;

   // Create a new NetCDF file and open it.
   f_out = new NcFile(out_file, NcFile::Replace);

   if(!f_out->is_valid()) {
      cerr << "\n\nERROR: write_netcdf() -> "
           << "trouble opening output file " << out_file
           << "\n\n" << flush;
      f_out->close();
      delete f_out;
      f_out = (NcFile *) 0;

      exit(1);
   }

   // Add global attributes
   write_netcdf_global(f_out, out_file.text(), program_name);

   if(run_command == sum) {
      sec_to_hhmmss(in_accum, accum1_str);
      sprintf(command_str,
              "Sum: %i files with accumulations of %s.",
              n_files, accum1_str);
   }
   else if(run_command == add) {
      sec_to_hhmmss(in_accum, accum1_str);
      sprintf(command_str,
              "Addition: %i files.", n_files);
   }
   else { // run_command == subtract
      sec_to_hhmmss(accum[0], accum1_str);
      sec_to_hhmmss(accum[1], accum2_str);
      sprintf(command_str,
              "Subtraction: %s with accumulation of %s minus %s with accumulation of %s.",
              in_file[0].text(), accum1_str,
              in_file[1].text(), accum2_str);
   }
   f_out->add_att("RunCommand", command_str);

   //
   // Add the projection information
   //
   write_netcdf_proj(f_out, grid);

   // Define Dimensions
   lat_dim = f_out->add_dim("lat", (long) grid.ny());
   lon_dim = f_out->add_dim("lon", (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(f_out, lat_dim, lon_dim, grid);

   // Define a name for the variable
   // If the accumulation time is non-zero, append it to the variable name
   get_grib_code_abbr(grib_code, grib_ptv, tmp_str);

   // For no accumulation interval, append nothing
   if(nc_accum <= 0) {
      strcpy(var_str, tmp_str);
   }
   // For an hourly accumulation interval, append _HH
   else if(nc_accum % sec_per_hour == 0) {
      sprintf(var_str, "%s_%.2i", tmp_str, nc_accum/sec_per_hour);
   }
   // For any other accumulation interval, append _HHMMSS
   else {
      sec_to_hhmmss(nc_accum, tmp2_str);
      sprintf(var_str, "%s_%s", tmp_str, tmp2_str);
   }

   // Define Variable
   pcp_var = f_out->add_var(var_str, ncFloat, lat_dim, lon_dim);

   // Add variable attributes
   get_grib_code_abbr(grib_code, grib_ptv, var_str);
   pcp_var->add_att("name",  var_str);
   get_grib_code_name(grib_code, grib_ptv, var_str);
   pcp_var->add_att("long_name", var_str);

   // Ouput level string
   if(nc_accum%sec_per_hour == 0) {
      sprintf(var_str, "A%i", nc_accum/sec_per_hour);
   }
   else {
      sec_to_hhmmss(nc_accum, tmp_str);
      sprintf(var_str, "A%s", tmp_str);
   }
   pcp_var->add_att("level", var_str);

   get_grib_code_unit(grib_code, grib_ptv, var_str);
   pcp_var->add_att("units", var_str);
   pcp_var->add_att("grib_code", grib_code);
   pcp_var->add_att("_FillValue", bad_data_float);

   //
   // Add initialization, valid, and accumulation time info as attributes to
   // the pcp_var
   //
   if(nc_init == (unixtime) 0) nc_init = nc_valid;

   //
   // Write out the times
   //
   write_netcdf_var_times(pcp_var, nc_init, nc_valid, nc_accum);

   //
   // Write the precip data
   //
   if(!pcp_var->put(&pcp_data[0], grid.ny(), grid.nx())) {

      cerr << "\n\nERROR: write_netcdf() -> "
           << "error with pcp_var->put\n\n" << flush;
      exit(1);
   }

   f_out->close();
   delete f_out;
   f_out = (NcFile *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {

   //
   // Deallocate memory and clean up
   //
   if(pcp_data) {
      delete [] pcp_data;
      pcp_data = (float *) 0;
   }

   return;
};

////////////////////////////////////////////////////////////////////////

void usage(int argc, char *argv[]) {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\t[[-sum] sum_args] | [-add add_args] | [-subtract subtract_args]\n"
        << "\t[-gc code]\n"
        << "\t[-ptv number]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-sum sum_args\" indicates that accumulations "
        << "from multiple files should be summed up using the "
        << "arguments provided.\n"

        << "\t\t\"-add add_args\" indicates that accumulations from "
        << "one or more files should be added together using the "
        << "arguments provided.\n"

        << "\t\t\"-subtract subtract_args\" indicates that "
        << "accumulations from two files should be subtracted using "
        << "the arguments provided.\n"

        << "\t\t\"-gc code\" overrides the default GRIB code ("
        << grib_code << ") to be used (optional).\n"

        << "\t\t\"-ptv number\" overrides the default GRIB parameter "
        << "table version number (" << grib_ptv
        << ") to be used (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << verbosity << ") (optional).\n\n"

        << "\t\tNote: Specifying \"-sum\" is not required since it is "
        << "the default behavior.\n\n"

        << "\tSUM_ARGS:\n"
        << "\t\tinit_time\n"
        << "\t\tin_accum\n"
        << "\t\tvalid_time\n"
        << "\t\tout_accum\n"
        << "\t\tout_file\n"
        << "\t\t[-pcpdir path]\n"
        << "\t\t[-pcprx reg_exp]\n\n"

        << "\t\twhere\t\"init_time\" indicates the initialization "
        << "time of the input GRIB files in YYYYMMDD[_HH[MMSS]] format "
        << "(required).\n"

        << "\t\t\t\"in_accum\" indicates the accumulation interval "
        << "of the input GRIB files in HH[MMSS] format (required).\n"

        << "\t\t\t\"valid_time\" indicates the desired valid "
        << "time in YYYYMMDD[_HH[MMSS]] format (required).\n"

        << "\t\t\t\"out_accum\" indicates the desired accumulation "
        << "interval for the output NetCDF file in HH[MMSS] format (required).\n"

        << "\t\t\t\"out_file\" indicates the name of the output NetCDF file to "
        << "be written consisting of the sum of the accumulation intervals "
        << "(required).\n"

        << "\t\t\t\"-pcpdir path\" overrides the default precipitation directory ("
        << default_pcp_dir << ") (optional).\n"

        << "\t\t\t\"-pcprx reg_exp\" overrides the default regular expression for "
        << "precipitation file naming convention (" << default_reg_exp
        << ") (optional).\n\n"

        << "\t\t\tNote: Set init_time to 00000000_000000 when summing "
        << "observation files.\n\n"

        << "\tADD_ARGS:\n"
        << "\t\tin_file1\n"
        << "\t\taccum1\n"
        << "\t\t...\n"
        << "\t\tin_filen\n"
        << "\t\taccumn\n"
        << "\t\tout_file\n\n"

        << "\t\twhere\t\"in_file1\" indicates the name of the first input GRIB "
        << "file to be used (required).\n"

        << "\t\t\t\"accum1\" indicates the accumulation interval "
        << "to be used from in_file1 in HH[MMSS] format (required).\n"

        << "\t\t\t\"in_filen\" indicates additional input GRIB files to be "
        << "added together (optional).\n"

        << "\t\t\t\"accumn\" indicates the accumulation interval "
        << "to be used in HH[MMSS] format (optional).\n"

        << "\t\t\t\"out_file\" indicates the name of the output NetCDF file to "
        << "be written (required).\n\n"

        << "\tSUBTRACT_ARGS:\n"
        << "\t\tin_file1\n"
        << "\t\taccum1\n"
        << "\t\tin_file2\n"
        << "\t\taccum2\n"
        << "\t\tout_file\n\n"

        << "\t\twhere\t\"in_file1\" indicates the name of the first input GRIB "
        << "file to be used (required).\n"

        << "\t\t\t\"accum1\" indicates the accumulation interval "
        << "to be used from in_file1 in HH[MMSS] format (required).\n"

        << "\t\t\t\"in_file2\" indicates the name of the second input GRIB "
        << "file to be subtracted from in_file1 (required).\n"

        << "\t\t\t\"accum2\" indicates the accumulation interval "
        << "to be used from in_file2 in HH[MMSS] format (required).\n"

        << "\t\t\t\"out_file\" indicates the name of the output NetCDF file to "
        << "be written (required).\n"

        << "\n" << flush;

   return;
}

////////////////////////////////////////////////////////////////////////
