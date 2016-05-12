// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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
//      Based on the command line options, this tool combines one or
//      more gridded data files into a single gridded data file and
//      writes the output in NetCDF format.
//
//      The tool may be run three different modes: sum, add, subtract
//
//      The sum command requires the user to specify an initialization
//      time, input accumulation interval, valid time, and output
//      accumulation interval.
//
//      The add and subtract commands require the user to specify a
//      list of input files, each followed by an accumulation interval.
//      The subtract command requires exactly two input files while the
//      add command supports one or more input files.
//
//      In all cases, the last argument is the output NetCDF file name.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    01-24-07  Halley Gotway  New
//
//   001    12-07-07  Halley Gotway  Change time format from
//                                   YYYY-MM-DD_HH:MM:SS to YYYYMMDD_HHMMSS
//
//   002    01-31-08  Halley Gotway  Add support for the -add and
//                                   -subtract options
//
//   003    09/23/08  Halley Gotway  Change argument sequence for the
//                                   GRIB record access routines.
//
//   004    02/20/09  Halley Gotway  Append _HH to the variable name
//                                   for non-zero accumulation times.
//
//   005    12/23/09  Halley Gotway  Call the library read_pds routine.
//
//   006    05/21/10  Halley Gotway  Enhance to search multiple
//                                   -pcp_dir directory arguments.
//
//   007    06/25/10  Halley Gotway  Allow times to be specified in
//                                   HH[MMSS] and YYYYMMDD[_HH[MMSS]] format.
//
//   008    06/30/10  Halley Gotway  Enhance grid equality checks.
//
//   009    07/27/10  Halley Gotway  Enhance to allow addition of any
//                                   number of input files/accumulation intervals.
//                                   Add lat/lon variables to NetCDF.
//
//   010    04/19/11  Halley Gotway  Bugfix for -add option.
//
//   011    10/20/11  Holmes         Added use of command line class to
//                                   parse the command line arguments.
//
//   012    11/14/11  Halley Gotway  Bugfix for -add option when
//                                   when handling missing data values.
//
//   013    12/21/11  Bullock        Ported to new repository.
//
//   014    03/07/12  Halley Gotway  Bugfix in get_field() function and
//                                   remove unnecessary time strings.
//
//   015    04/12/12  Oldenburg      Support for all gridded data types.
//   016    01/23/13  Halley Gotway  Update usage statement and code
//                                   cleanup.
//   017    10/17/13  Halley Gotway  Bugfix for closing file handles during
//                                   pcpdir search.
//   018    04/16/14  Halley Gotway  Bugfix for the -varname option.
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
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "netcdf.hh"

#include "vx_log.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_nc_util.h"
#include "vx_grid.h"
#include "vx_statistics.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

static const char zero_time_str   [] = "00000000_000000";
static const char default_pcp_dir [] = ".";
static const char default_reg_exp [] = ".*";

// Run Command enumeration
enum RunCommand { sum = 0, add = 1, sub = 2 };

// Variables for top-level command line arguments
static RunCommand run_command = sum;
static int verbosity = 1;

// Variables common to all commands
static ConcatString out_filename;
static MetConfig config;

// Variables for the sum command
static unixtime     init_time;
static int          in_accum;
static unixtime     valid_time;
static int          out_accum;
static StringArray  pcp_dir;
static ConcatString pcp_reg_exp = default_reg_exp;
static ConcatString user_dict = "";
static ConcatString field_name = "";
static bool         name_flag = false;
static VarInfo*     var_info = (VarInfo *) 0;

// Variables for the add and subtract commands
static ConcatString *in_file = (ConcatString *) 0;
static int          *accum = (int *) 0;
static StringArray   accum_mag;
static int           n_files;

///////////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);

static void process_sum_args(const CommandLine &);
static void process_add_sub_args(const CommandLine &);

static void do_sum_command();
static void do_add_command();
static void do_sub_command();

static void sum_data_files(Grid &, DataPlane &);
static int  search_pcp_dir(const char *, const unixtime, ConcatString &);

static void get_field(const char * filename, const int get_accum,
                      const unixtime get_init_ut, const unixtime get_valid_ut,
                      Grid & grid, DataPlane & plane);

static void get_field(const char * filename, const char * fld_accum_mag,
                      const unixtime get_init_ut, const unixtime get_valid_ut,
                      Grid & grid, DataPlane & plane);

static void write_netcdf(unixtime, unixtime, int, const Grid &, const DataPlane &);

static bool is_timestring(const char *);

static void usage();
static void set_sum(const StringArray &);
static void set_add(const StringArray &);
static void set_subtract(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);
static void set_pcpdir(const StringArray &);
static void set_pcprx(const StringArray &);
static void set_user_dict(const StringArray & a);
static void set_name(const StringArray & a);

static void show_version(const StringArray &);


////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])

{

program_name = get_short_name(argv[0]);

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
   if     (run_command == sum) do_sum_command();
   else if(run_command == add) do_add_command();
   else                        do_sub_command();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv)

{

   CommandLine cline;

   //
   // check for zero arguments
   //
   if (argc == 1)
      usage();

   //
   // Default to running the sum command
   //
   run_command = sum;

   //
   // parse the command line into tokens
   //
   cline.set(argc, argv);

   //
   // set the usage function
   //
   cline.set_usage(usage);

   //
   // add the options function calls
   //
   cline.add(set_sum,       "-sum",      0);
   cline.add(set_add,       "-add",      0);
   cline.add(set_subtract,  "-subtract", 0);
   cline.add(set_pcpdir,    "-pcpdir",   1);
   cline.add(set_pcprx,     "-pcprx",    1);
   cline.add(set_user_dict, "-config",   1);
   cline.add(set_user_dict, "-field",    1);
   cline.add(set_name,      "-name",     1);
   cline.add(set_name,      "-varname",  1);
   cline.add(set_logfile,   "-log",      1);
   cline.add(set_verbosity, "-v",        1);

   cline.add(show_version,  "-version",  0);

   //
   // parse the command line
   //
   cline.parse();

   //
   // set the verbosity level
   //
   mlog.set_verbosity_level(verbosity);

   //
   // Check for error. Depending on the type of command, there should
   // be a different number of arguments left. For the sum command
   // there should be five arguments left: the init_time, the in_accum,
   // the valid_time, the out_accum, and the out_file. For the add
   // command there should be at least three arguments left: in_file1,
   // accum1, [in_file2, accum2, ..., in_filen, accumn], and the
   // out_file. For the subtract command there should be five
   // arguments left: in_file1, accum1, in_file2, accum2, and
   // out_file.
   //
   if (run_command == sum)
   {
      if (cline.n() != 5)
         usage();

   }
   else if (run_command == add)
   {
      if (cline.n() < 3)
         usage();

   }
   else
   {
      if (cline.n() != 5)
         usage();

   }

   //
   // Process the specific command arguments
   //
   if(run_command == sum) process_sum_args(cline);
   else                   process_add_sub_args(cline);

   //
   // If pcp_dir is not set, set it to the current directory.
   //
   if(pcp_dir.n_elements() == 0) pcp_dir.add(default_pcp_dir);

   //
   // Initialize the MetConfig object
   //
   config.read(replace_path(config_const_filename));

   //
   //  done
   //

   return;
}

////////////////////////////////////////////////////////////////////////

void process_sum_args(const CommandLine & cline)

{

   //
   // Parse the sum arguments
   //

   //
   // Init time
   //
   if(strcmp(cline[0], zero_time_str) == 0) {
      init_time = (unixtime) 0;
   }
   else {
      init_time = timestring_to_unix(cline[0]);
   }

   //
   // Input accumulation
   //
   in_accum = timestring_to_sec(cline[1]);

   //
   // Valid time
   //
   if(strcmp(cline[2], zero_time_str) == 0) {
      valid_time = (unixtime) 0;
   }
   else {
      valid_time = timestring_to_unix(cline[2]);
   }

   //
   // Output accumulation
   //
   out_accum = timestring_to_sec(cline[3]);

   //
   // Out file
   //
   out_filename = cline[4];

   //
   // Check that accumulation intervals are greater than zero
   //
   if(in_accum <= 0 || out_accum <= 0) {
      mlog << Error << "\nprocess_sum_args() -> "
           << "The input accumulation interval (" << cline[1]
           << ") and output accumulation interval (" << cline[3]
           << ") must be greater than zero.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_add_sub_args(const CommandLine & cline)

{

   int i;

   //
   // Figure out the number of files provided
   //
   for(i=0, n_files=0; i<(cline.n() - 1); i+=2) { // Only check accumulation interval for enough args

      n_files++; // Increment file count

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
      in_file[i] = cline[i*2];
      accum_mag.add( cline[i*2 + 1] );
   }

   //
   // Store the output file
   //
   out_filename = cline[n_files*2];

   return;
}

////////////////////////////////////////////////////////////////////////

void do_sum_command()

{

   DataPlane plane;
   Grid grid;
   int lead_time;
   ConcatString init_time_str;

   //
   // Compute the lead time
   //
   lead_time = valid_time - init_time;

   //
   // Build init time string
   //
   if(init_time != 0) init_time_str = unix_to_yyyymmdd_hhmmss(init_time);
   else               init_time_str = zero_time_str;

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
           << "The output accumulation time (" << sec_to_hhmmss(out_accum)
           << ") cannot be greater than the lead time ("
           << sec_to_hhmmss(lead_time) << ").\n\n";
      exit(1);
   }

   //
   // Check that the output accumulation time is divisible by the input
   // accumulation time.
   //
   if(out_accum%in_accum != 0) {
      mlog << Error << "\ndo_sum_command() -> "
           << "The output accumulation time (" << sec_to_hhmmss(out_accum)
           << ") must be divisible by the input accumulation "
           << "time (" << sec_to_hhmmss(in_accum) << ").\n\n";
      exit(1);
   }

   //
   // Check that the lead time is divisible by the the input
   // accumulation time except when init_time = 0 for observations.
   //
   if(lead_time%in_accum != 0 && init_time != (unixtime) 0) {
      mlog << Error << "\ndo_sum_command() -> "
           << "The lead time (" << sec_to_hhmmss(lead_time)
           << ") must be divisible by the input accumulation time ("
           << sec_to_hhmmss(in_accum) << ").\n\n";
      exit(1);
   }

   //
   // Find and sum up the matching precipitation files
   //
   sum_data_files(grid, plane);

   //
   // Write the combined precipitation field out in NetCDF format
   //
   mlog << Debug(1)
        << "Writing output file: " << out_filename << "\n";

   write_netcdf(init_time, valid_time, out_accum, grid, plane);

   return;
}

////////////////////////////////////////////////////////////////////////

void sum_data_files(Grid & grid, DataPlane & plane)

{

   int i, j, x, y;
   DataPlane part;
   double v_sum, v_part;
   Grid gr;
   unixtime     * pcp_times = (unixtime *) 0;
   int          * pcp_recs  = (int *) 0;
   ConcatString * pcp_files = (ConcatString *) 0;

   //
   // Compute the number of forecast precipitation files to be found,
   // and allocate memory to store their names and times
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
   // Compute the valid times for the precipitation files
   // to be found.
   //
   for(i=0; i<n_files; i++) {
      pcp_times[i] = valid_time - i*in_accum;
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

         if( pcp_recs[i] != -1 )  {

            mlog << Debug(1)
                 << "[" << (i+1) << "] File " << pcp_files[i]
                 << " matches valid time of " << unix_to_yyyymmdd_hhmmss(pcp_times[i])
                 << "\n";

            break;

         } // if

      } // end for j

      //
      // Check for no matching file found
      //
      if(pcp_recs[i] == -1) {

         mlog << Error << "\nsum_data_files() -> "
              << "Cannot find a file with a valid time of "
              << unix_to_yyyymmdd_hhmmss(pcp_times[i])
              << " and accumulation time of "
              << sec_to_hhmmss(in_accum) << " matching the regular "
              << "expression \"" << pcp_reg_exp << "\"\n\n";
         exit(1);
      }

   } // end for i

    /////////////////////////////

   //
   // Open each of the files found and parse the data.
   //
   for(i=0; i<n_files; i++) {

      mlog << Debug(1)
           << "[" << (i+1) << "] Reading input file: " << pcp_files[i]
           << "\n";

      //
      // Read data for the file.
      //
      get_field(pcp_files[i], in_accum, init_time, pcp_times[i], gr, part);

      //
      // For the first file processed store the grid, allocate memory
      // to store the precipitation sums, and initialize the sums
      //
      if ( i == 0 )  {

         grid = gr;

         plane = part;

      } else {

         //
         // Check to make sure the grid stays the same
         //
         if(!(grid == gr)) {
            mlog << Error << "\nsum_data_files() -> "
                 << "the input fields must be on the same grid.\n"
                 << grid.serialize() << "\n" << gr.serialize() << "\n\n";
            exit(1);
         }

         //
         // Increment the precipitation sums keeping track of the bad data values
         //
         for(x=0; x<grid.nx(); x++) {

            for(y=0; y<grid.ny(); y++) {

               v_sum = plane(x, y);

               if ( is_bad_data(v_sum) )  continue;

               v_part = part(x, y);

               if ( is_bad_data(v_part) ) {

                  plane.set(bad_data_double, x, y);

                  continue;

               }

               plane.set(v_sum + v_part, x, y);

            }   //  for y

         }   //  for x

      } // end else

   } // end for i

   //
   // Deallocate any memory that was allocated above
   //
   if(pcp_files) { delete [] pcp_files; pcp_files = (ConcatString *) 0; }
   if(pcp_times) { delete [] pcp_times; pcp_times = (unixtime *) 0; }
   if(pcp_recs ) { delete [] pcp_recs;  pcp_recs  = (int *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

int search_pcp_dir(const char *cur_dir, const unixtime cur_ut, ConcatString & cur_file)

{

   int i_rec;
   struct dirent *dirp = (struct dirent *) 0;
   DIR *dp = (DIR *) 0;

   //
   // Find the files matching the specified regular expression with
   // the correct valid and accumulation times.
   //
   if((dp = opendir(cur_dir)) == NULL ) {
      mlog << Error << "\nsearch_pcp_dir() -> "
           << "Cannot open precipitation directory "
           << cur_dir << "\n\n";
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
         cur_file << cs_erase << cur_dir << '/' << dirp->d_name;

         Met2dDataFileFactory factory;
         Met2dDataFile * datafile = (Met2dDataFile *) 0;
         VarInfoFactory var_fac;
         VarInfo* var;

         //  create a data file object
         datafile = factory.new_met_2d_data_file(cur_file);
         if( !datafile ){
            mlog << Warning << "search_pcp_dir() - can't open data file \"" << cur_file << "\"\n";
            continue;
         }

         //  create a VarInfo object from the data file
         var = var_fac.new_var_info(datafile->file_type());
         if( !var ){
            mlog << Warning << "search_pcp_dir() -> unable to determine filetype of \"" << cur_file << "\"\n";
            continue;
         }

         //  initialize the VarInfo object with a field dictionary and
         //  the requested timing information
         ConcatString accum_dict = user_dict;
         if( user_dict.empty() ){
            accum_dict.format("name=\"APCP\";level=\"A%s\";", sec_to_hhmmss(in_accum).text());
         }
         config.read_string( accum_dict.text() );
         var->set_dict( config );

         var->set_valid(cur_ut);
         var->set_init(init_time);
         var->set_lead(init_time ? cur_ut - init_time : bad_data_int);

         //  look for a VarInfo record match in the data file
         i_rec = datafile->index(*var);

         //  delete allocated data file
         if( datafile ) {
            delete datafile;
            datafile = (Met2dDataFile *) 0;
         }

         //  check for a valid match
         if( -1 != i_rec ) break;

      } // end if

   } // end while

   if(closedir(dp) < 0) {
      mlog << Error << "\nsearch_pcp_dir() -> "
           << "Cannot close precipitation directory "
           << cur_dir << "\n\n";
      exit(1);
   }

   return(i_rec);
}

////////////////////////////////////////////////////////////////////////

void do_add_command()

{

   Grid grid1, grid2;
   DataPlane total, part;
   double total_value, part_value;
   unixtime nc_init_time, nc_valid_time;
   int i, x, y, nc_accum;

   mlog << Debug(2)
        << "Performing addition command for " << n_files
        << " files.\n";

   //
   // Read current field
   //
   mlog << Debug(1)
        << "Reading input file: " << in_file[0] << "\n";


   get_field(in_file[0], accum_mag[0], 0, 0, grid1, total);

   // Initialize output times
   nc_init_time  = total.init();
   nc_valid_time = total.valid();
   nc_accum = total.accum();

   //
   // Loop through the rest of the input files
   //
   for(i=1; i<n_files; i++) {   //  i starts at one here, not zero

      //
      // Read current field
      //
      mlog << Debug(1) << "Reading input file: " << in_file[i] << "\n";

      get_field(in_file[i].text(), accum_mag[i], 0, 0, grid2, part);

      //
      // Check for the same grid dimensions
      //
      if( grid1 != grid2 ) {
         mlog << Error << "\ndo_add_command() -> "
              << "the input fields must be on the same grid.\n"
              << grid1.serialize() << "\n" << grid2.serialize() << "\n\n";
         exit(1);
      }

      // Output init time
      if(nc_init_time != part.init()) nc_init_time = (unixtime) 0;

      // Output valid time
      if(nc_valid_time < part.valid()) nc_valid_time = part.valid();

      // Output accumulation time
      nc_accum += part.accum();

      //
      // Increment sums for each grid point
      //

      for(x=0; x<(part.nx()); x++) {

         for(y=0; y<(part.ny()); y++) {

            total_value = total(x, y);

            if ( is_bad_data(total_value) )  continue;

            part_value = part(x, y);

            if ( is_bad_data(part_value) )  continue;

            total.set(total_value + part_value, x, y);

         } // end for y

      } // end for x

   } // end for i

   //
   // Write the combined precipitation field out in NetCDF format
   //
   mlog << Debug(1)
        << "Writing output file: " << out_filename << "\n";

   write_netcdf(nc_init_time, nc_valid_time, nc_accum, grid1, total);

   return;

}

////////////////////////////////////////////////////////////////////////

void do_sub_command()

{

   DataPlane plus, minus, difference;
   Grid grid1, grid2;
   unixtime nc_init_time, nc_valid_time;
   int x, y, nc_accum;
   double v_plus, v_minus;

   //
   // Check for exactly two input files
   //
   if(n_files != 2) {
      mlog << Error << "\ndo_sub_command() -> "
           << "you must specify exactly two input files for subtraction.\n\n";
      exit(1);
   }

   //
   // Read the two specified data files
   //
   mlog << Debug(1)
        << "Reading input file: " << in_file[0] << "\n";

   get_field(in_file[0], accum_mag[0], 0, 0, grid1, plus);

   mlog << Debug(1)
        << "Reading input file: " << in_file[1] << "\n";

   get_field(in_file[1], accum_mag[1], 0, 0, grid2, minus);

   //
   // Check for the same grid dimensions
   //
   if( grid1 != grid2 ) {
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
   // Output initialization time
   // Error if init_time1 != init_time2.
   //
   if(plus.init() != minus.init()) {
      mlog << Error << "\ndo_sub_command() -> "
           << "init_time1 (" << unix_to_yyyymmdd_hhmmss(plus.init())
           <<  ") must be equal to init_time2 ("
           << unix_to_yyyymmdd_hhmmss(minus.init())
           << ") for subtraction.\n\n";
      exit(1);
   }
   nc_init_time = plus.init();

   //
   // Output accumulation time
   // Error if accum1 < accum2.
   //
   if(plus.accum() < minus.accum()) {
      mlog << Error << "\ndo_sub_command() -> "
           << "accum1 (" << sec_to_hhmmss(plus.accum())
           <<  ") must be greater than accum2 ("
           << sec_to_hhmmss(minus.accum()) << ") for subtraction.\n\n";
      exit(1);
   }
   nc_accum = plus.accum() - minus.accum();

   //
   // Allocate space to store the differences
   //
   difference.set_size(plus.nx(), plus.ny());

   //
   // Perform the specified command for each grid point
   //

   difference.set_constant(bad_data_float);

   for(x=0; x<(plus.nx()); x++) {

      for(y=0; y<(plus.ny()); y++) {

         v_plus = plus(x, y);

         if ( is_bad_data(v_plus) )  continue;

         v_minus = minus(x, y);

         if ( is_bad_data(v_minus) )  continue;

         difference.set(v_plus - v_minus, x, y);

      } // end for y

   } // end for x

   //
   // Write the combined precipitation field out in NetCDF format
   //
   mlog << Debug(1) << "Writing output file: " << out_filename << "\n";
   write_netcdf(nc_init_time, nc_valid_time, nc_accum, grid1, difference);

   return;

}

////////////////////////////////////////////////////////////////////////

void get_field(const char * filename, const int get_accum,
               const unixtime get_init_ut, const unixtime get_valid_ut,
               Grid & grid, DataPlane & plane)

{
   get_field(filename, sec_to_hhmmss(get_accum), get_init_ut, get_valid_ut, grid, plane);
}

////////////////////////////////////////////////////////////////////////

void get_field(const char * filename, const char * fld_accum_mag,
               const unixtime get_init_ut, const unixtime get_valid_ut,
               Grid & grid, DataPlane & plane)

{

   Met2dDataFileFactory factory;
   Met2dDataFile * datafile = (Met2dDataFile *) 0;
   GrdFileType ftype;
   VarInfoFactory var_fac;
   VarInfo* var;

   //  build the field config string
   ConcatString config_str = is_timestring(fld_accum_mag) ? user_dict.text() : fld_accum_mag;
   if( config_str.empty() ){
      config_str.format("name=\"APCP\";level=\"A%s\";", fld_accum_mag);
   }

   //  parse the config string
   config.read_string(config_str);

   //  get the gridded file type from config string, if present
   ftype = parse_conf_file_type(&config);

   //  open the data file and build a VarInfo object
   datafile = factory.new_met_2d_data_file(filename, ftype);
   if( !datafile ){
      mlog << Error << "\nget_field() -> can't open data file \"" << filename
           << "\"\n\n";
      exit ( 1 );
   }

   var = var_fac.new_var_info(datafile->file_type());
   if( !var ){
      mlog << Error << "\nget_field() -> unable to determine filetype of \""
           << filename << "\"\n\n";
      exit (1);
   }

   //  initialize the VarInfo object with a config
   var->set_dict(config);

   //  set the VarInfo timing object
   if(get_valid_ut != 0) var->set_valid(get_valid_ut);
   if(get_init_ut  != 0) var->set_init(get_init_ut);

   //  build an output field name using the magic string
   if( !var_info ){
      var_info = var;
      if( field_name.empty() ){
         field_name = var->magic_str();
         field_name = str_replace_all(field_name, "(", "");
         field_name = str_replace_all(field_name, ")", "");
         field_name = str_replace_all(field_name, "*", "");
         field_name = str_replace_all(field_name, ",", "");
         field_name = str_replace_all(field_name, "/", "_");
      }
   }

   //  read the record of interest into a DataPlane object
   if( ! datafile->data_plane(*var, plane) ){
      mlog << Error << "\nget_field() -> can't get data plane from file \"" << filename
           << "\"\n\n";
      exit ( 1 );
   }

   grid = datafile->grid();

   if ( datafile )  { delete datafile;  datafile = (Met2dDataFile *) 0; }

   return;

}

////////////////////////////////////////////////////////////////////////

void write_netcdf(unixtime nc_init, unixtime nc_valid, int nc_accum,
                  const Grid &grid, const DataPlane & plane)

{

   ConcatString var_str;
   ConcatString tmp_str, tmp2_str;
   ConcatString command_str;

   NcFile *f_out   = (NcFile *) 0;
   NcDim  *lat_dim = (NcDim *)  0;
   NcDim  *lon_dim = (NcDim *)  0;
   NcVar  *pcp_var = (NcVar *)  0;

   // Create a new NetCDF file and open it.
   f_out = new NcFile(out_filename, NcFile::Replace);

   if(!f_out->is_valid()) {
      mlog << Error << "\nwrite_netcdf() -> "
           << "trouble opening output file " << out_filename
           << "\n\n";
      f_out->close();
      delete f_out;  f_out = (NcFile *) 0;

      exit(1);
   }

   // Add global attributes
   write_netcdf_global(f_out, out_filename, program_name);

   if(run_command == sum) {

      command_str << cs_erase
                  << "Sum: " << n_files << " files with accumulations of "
                  << sec_to_hhmmss(in_accum) << '.';

   } else if(run_command == add) {

      command_str << cs_erase
                  << "Addition: " << n_files << " files.";

   }
   else { // run_command == subtract

      command_str << cs_erase
                  << "Subtraction: "
                  << in_file[0]
                  << " with accumulation of "
                  << sec_to_hhmmss(accum[0])
                  << " minus "
                  << in_file[1]
                  << " with accumulation of "
                  << sec_to_hhmmss(accum[1]) << '.';

   }

   f_out->add_att("RunCommand", (const char *) command_str);

   //
   // Add the projection information
   //
   write_netcdf_proj(f_out, grid);

   // Define Dimensions
   lat_dim = f_out->add_dim("lat", (long) grid.ny());
   lon_dim = f_out->add_dim("lon", (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(f_out, lat_dim, lon_dim, grid);

   // If the -varname command line option was used or the accumulation
   // interval is zero, just use the field_name
   if(name_flag || nc_accum <= 0) {
      var_str = field_name;
   }
   // Otherwise, append the acculuation interval to the variable name
   else {

      // Store up to the first underscore
      tmp_str       = field_name;
      StringArray l = tmp_str.split("_");
      tmp_str       = l[0];

      // For an hourly accumulation interval, append _HH
      if(nc_accum % sec_per_hour == 0) {
         var_str.set_precision(2);
         var_str << cs_erase << tmp_str << '_' << HH(nc_accum/sec_per_hour);
      }

      // For any other accumulation interval, append _HHMMSS
      else {
         tmp2_str = sec_to_hhmmss(nc_accum);
         var_str << cs_erase << tmp_str << '_' << tmp2_str;
      }
   }

   // Define Variable
   pcp_var = f_out->add_var((const char *) var_str, ncFloat, lat_dim, lon_dim);

   // Add variable attributes
   pcp_var->add_att("name",  (const char *) var_str);
   pcp_var->add_att("long_name", var_info->long_name());

   // Ouput level string
   if(nc_accum%sec_per_hour == 0) {
      var_str << cs_erase << 'A' << (nc_accum/sec_per_hour);
   } else {
      var_str << cs_erase << 'A' << sec_to_hhmmss(nc_accum);
   }

   pcp_var->add_att("level", (const char *) var_str);
   pcp_var->add_att("units",      var_info->units());
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
   if(!pcp_var->put(plane.data(), plane.ny(), plane.nx())) {

      mlog << Error << "\nwrite_netcdf() -> "
           << "error with pcp_var->put()\n\n";
      exit(1);
   }

   f_out->close();
   delete f_out;
   f_out = (NcFile *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////


bool is_timestring(const char * text)

{

if ( is_hh(text) ) return ( true );

if ( is_hhmmss(text) ) return ( true );

return ( false );

}


////////////////////////////////////////////////////////////////////////

void usage()

{

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\t[[-sum] sum_args] | [-add add_args] | [-subtract subtract_args]\n"
        << "\t[-field string]\n"
        << "\t[-name variable_name]\n"
        << "\t[-log file]\n"
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

        << "\t\t\"-field string\" defines the data to be extracted from "
        << "the input files (optional).\n"

        << "\t\t\"-name variable_name\" name of combined variable in "
        << "output NetCDF file (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << verbosity << ") (optional).\n\n"

        << "\t\tNote: Specifying \"-sum\" is not required since it is "
        << "the default behavior.\n"

        << "\t\tNote: For \"-add\" and \"-subtract\", the accumulation intervals "
        << "may be substituted with config file strings.\n\n"

        << "\tSUM_ARGS:\n"
        << "\t\tinit_time\n"
        << "\t\tin_accum\n"
        << "\t\tvalid_time\n"
        << "\t\tout_accum\n"
        << "\t\tout_file\n"
        << "\t\t[-pcpdir path]\n"
        << "\t\t[-pcprx reg_exp]\n\n"

        << "\t\twhere\t\"init_time\" indicates the initialization "
        << "time of the input data files in YYYYMMDD[_HH[MMSS]] format "
        << "(required).\n"

        << "\t\t\t\"in_accum\" indicates the accumulation interval "
        << "of the input data files in HH[MMSS] format (required).\n"

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
        << "\t\tin_file1 accum1 [in_file2 accum2 ... in_filen accumn]\n"
        << "\t\tout_file\n\n"

        << "\t\twhere\t\"in_file1\" indicates the name of the first input data "
        << "file to be used (required).\n"

        << "\t\t\t\"accum1\" indicates the accumulation interval to be used "
        << "from in_file1 in HH[MMSS] format (required).\n"

        << "\t\t\t\"in_filen\" indicates additional input data files to be "
        << "added together (optional).\n"

        << "\t\t\t\"accumn\" indicates the accumulation interval to be used "
        << "from in_filen in HH[MMSS] format (required).\n"

        << "\t\t\t\"out_file\" indicates the name of the output NetCDF file to "
        << "be written (required).\n\n"

        << "\tSUBTRACT_ARGS:\n"
        << "\t\tin_file1 accum1\n"
        << "\t\tin_file2 accum2\n"
        << "\t\tout_file\n\n"

        << "\t\twhere\t\"in_file1\" indicates the name of the first input data "
        << "file to be used (required).\n"

        << "\t\t\t\"accum1\" indicates the accumulation interval to be used "
        << "from in_file1 in HH[MMSS] format (required).\n"

        << "\t\t\t\"in_file2\" indicates the name of the second input data "
        << "file to be subtracted from in_file1 (required).\n"

        << "\t\t\t\"accum2\" indicates the accumulation interval to be used "
        << "from in_file2 in HH[MMSS] format (required).\n"

        << "\t\t\t\"out_file\" indicates the name of the output NetCDF file to "
        << "be written (required).\n"

        << "\n" << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_sum(const StringArray &)
{
   run_command = sum;
}

////////////////////////////////////////////////////////////////////////

void set_add(const StringArray &)
{
   run_command = add;
}

////////////////////////////////////////////////////////////////////////

void set_subtract(const StringArray &)
{
   run_command = sub;
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a)
{
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a)
{
   verbosity = atoi(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_pcpdir(const StringArray & a)
{
   pcp_dir.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_pcprx(const StringArray & a)
{
   pcp_reg_exp = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_user_dict(const StringArray & a)
{
   user_dict = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_name(const StringArray & a)
{
   field_name = a[0];
   name_flag  = true;
}

////////////////////////////////////////////////////////////////////////


void show_version(const StringArray &)

{

cout << "\n\n  " << met_version << "\n\n";

exit ( 0 );

return;

}


////////////////////////////////////////////////////////////////////////
