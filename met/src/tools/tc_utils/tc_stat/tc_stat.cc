// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   tc_stat.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    04/03/12  Halley Gotway   New
//   001    07/14/14  Halley Gotway   Generalize rapid intensification.
//   002    09/28/16  Halley Gotway   Add DESC output column.
//   003    07/27/18  Halley Gotway   Support masks defined by
//                    the gen_vx_mask tool.

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

#include "tc_stat.h"
#include "tc_stat_job.h"

#include "vx_tc_util.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_log.h"

#include "met_file.h"

////////////////////////////////////////////////////////////////////////

static void   process_command_line(int, char **);
static void   process_search_dirs ();
static void   process_jobs        ();
static void   usage               ();
static void   set_lookin          (const StringArray &);
static void   set_out             (const StringArray &);
static void   set_logfile         (const StringArray &);
static void   set_verbosity       (const StringArray &);
static void   set_config          (const StringArray &);
static void   open_out_file       ();
static void   close_out_file      ();

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the input data
   process_search_dirs();

   // Process the jobs in the config file
   process_jobs();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;
   ConcatString default_config_file;
   int i;

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Allow for unrecognized command line switches
   cline.allow_unrecognized_switches();

   // Set the usage function
   cline.set_usage(usage);

   // Add function calls for the arguments
   cline.add(set_lookin,    "-lookin", -1);
   cline.add(set_out,       "-out",     1);
   cline.add(set_logfile,   "-log",     1);
   cline.add(set_verbosity, "-v",       1);
   cline.add(set_config,    "-config",  1);

   // Parse the command line
   cline.parse();

   // Parse remaining options into the job command
   command_line_job = "";
   for(i=0; i<cline.n(); i++) command_line_job << cline[i] << " ";

   // Check for the minimum number of arguments
   if(tcst_source.n_elements() == 0) {
      mlog << Error
           << "\nprocess_command_line(int argc, char **argv) -> "
           << "You must specify at least one source using the "
           << "\"-lookin\" option.\n\n";
      usage();
   }

   // List the input track files
   for(i=0; i<tcst_source.n_elements(); i++)
      mlog << Debug(1)
           << "[Source " << i+1 << "] Lookin: " << tcst_source[i] << "\n";


   // Check if a config file has been specified
   if(config_file.nonempty()) {

      // Default config file
      default_config_file = replace_path(default_config_filename);

      // List the config files
      mlog << Debug(1)
           << "Config File Default: " << default_config_file << "\n"
           << "Config File User: " << config_file << "\n";

      // Read the config files
      conf_info.read_config(default_config_file.c_str(), config_file.c_str());
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_search_dirs() {

   // Retrieve the file lists
   tcst_files = get_filenames(tcst_source, NULL, tc_stat_file_ext);

   // Check for matching files
   if(tcst_files.n_elements() == 0) {
      mlog << Error
           << "\nprocess_search_dirs() -> "
           << "no TCST files found in the directories specified!\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_jobs() {
   TCStatJobFactory factory;
   TCStatJob *cur_job = (TCStatJob *) 0;
   ConcatString jobstring;
   int i, n_jobs;
   TCLineCounts n;

   // Open the output file
   open_out_file();

   // If no config file, do a single job
   if(config_file.empty()) n_jobs = 1;
   else                    n_jobs = conf_info.Jobs.n_elements();

   // Loop through the jobs
   for(i=0; i<n_jobs; i++) {

      // If no config file, process the command line job
      if(config_file.empty()) {
         jobstring = command_line_job;
      }

      // Otherwise, get the current config file job
      else {

         // Initialize jobstring to the default filtering job
         jobstring = conf_info.Filter.serialize();

         // Add the current job options
         jobstring << conf_info.Jobs[i];
      }

      // Allocate a new job
      cur_job = factory.new_tc_stat_job(jobstring.c_str());

      // Set the job output file stream
      cur_job->JobOut = tc_stat_out;

      // Set the output precision
      cur_job->set_precision(conf_info.Conf.output_precision());

      // Serialize the current job
      mlog << Debug(2)
           << "\nProcessing Job " << i+1 << ": "
           << cur_job->serialize() << "\n";

      // Initialize counts
      memset(&n, 0, sizeof(TCLineCounts));

      // Do the job
      cur_job->do_job(tcst_files, n);

      mlog << Debug(2)
           << "Job " << i+1 << " used " << n.NKeep << " out of "
           << n.NRead << " lines read.\n";

      mlog << Debug(3)
           << "Total lines read                 = " << n.NRead             << "\n"
           << "Total lines kept                 = " << n.NKeep             << "\n"
           << "Rejected for track watch/warn    = " << n.RejTrackWatchWarn << "\n"
           << "Rejected for init threshold      = " << n.RejInitThresh     << "\n"
           << "Rejected for init string         = " << n.RejInitStr        << "\n"
           << "Rejected for out init mask       = " << n.RejOutInitMask    << "\n"
           << "Rejected for water only          = " << n.RejWaterOnly      << "\n"
           << "Rejected for rapid inten         = " << n.RejRIRW           << "\n"
           << "Rejected for landfall            = " << n.RejLandfall       << "\n"
           << "Rejected for amodel              = " << n.RejAModel         << "\n"
           << "Rejected for bmodel              = " << n.RejBModel         << "\n"
           << "Rejected for desc                = " << n.RejDesc           << "\n"
           << "Rejected for storm id            = " << n.RejStormId        << "\n"
           << "Rejected for basin               = " << n.RejBasin          << "\n"
           << "Rejected for cyclone             = " << n.RejCyclone        << "\n"
           << "Rejected for storm name          = " << n.RejStormName      << "\n"
           << "Rejected for init time           = " << n.RejInit           << "\n"
           << "Rejected for init hour           = " << n.RejInitHour       << "\n"
           << "Rejected for lead time           = " << n.RejLead           << "\n"
           << "Rejected for required lead times = " << n.RejLeadReq       << "\n"
           << "Rejected for valid time          = " << n.RejValid          << "\n"
           << "Rejected for valid hour          = " << n.RejValidHour      << "\n"
           << "Rejected for init mask           = " << n.RejInitMask       << "\n"
           << "Rejected for valid mask          = " << n.RejValidMask      << "\n"
           << "Rejected for line type           = " << n.RejLineType       << "\n"
           << "Rejected for numeric threshold   = " << n.RejColumnThresh   << "\n"
           << "Rejected for string matching     = " << n.RejColumnStr      << "\n"
           << "Rejected for match points        = " << n.RejMatchPoints    << "\n"
           << "Rejected for event equal         = " << n.RejEventEqual     << "\n"
           << "Rejected for out init mask       = " << n.RejOutInitMask    << "\n"
           << "Rejected for out valid mask      = " << n.RejOutValidMask   << "\n";

      // Deallocate current job
      if(cur_job) { delete cur_job; cur_job = (TCStatJob *) 0; }

   } // end for i

   // Close the output file
   close_out_file();

   return;
}

////////////////////////////////////////////////////////////////////////

void open_out_file() {

   if(out_file.empty()) return;

   // Create an output file and point to it
   tc_stat_out = new ofstream;
   tc_stat_out->open(out_file.c_str());

   if(!(*tc_stat_out)) {
      mlog << Error << "\nopen_out_file()-> "
           << "can't open the output file \"" << out_file
           << "\" for writing!\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void close_out_file() {

   // Close the output file
   if(tc_stat_out != (ofstream *) 0) {

      // List the file being closed
      mlog << Debug(1)
           << "\nCreating output file: " << out_file << "\n";

      // Close the output file
      tc_stat_out->close();
      delete tc_stat_out;
      tc_stat_out = (ofstream *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\t-lookin source\n"
        << "\t[-out file]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-config file] | [JOB COMMAND LINE]\n\n"

        << "\twhere\t\"-lookin source\" is used one or more times to "
        << "specify a file or top-level directory containing TC-MET "
        << "files \"" << tc_stat_file_ext << "\" data to process "
        << "(required).\n"

        << "\t\t\"-out file\" to redirect the job output to a "
        << "file (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t\"-config file\" specifies the TCStatConfig file "
        << "containing the desired configuration settings.\n"

        << "\t\t\"JOB COMMAND LINE\" specifies all the arguments "
        << "necessary to perform a single job.\n\n"

        << "\tNOTE: Refer to a TCStatConfig file for details "
        << "on how to specify the JOB COMMAND LINE.\n\n"

        << flush;

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_lookin(const StringArray & a) {
   for(int i=0; i<a.n_elements(); i++) tcst_source.add(a[i]);
}

////////////////////////////////////////////////////////////////////////

void set_out(const StringArray & a) {
   out_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a) {
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a) {
   mlog.set_verbosity_level(atoi(a[0].c_str()));
}

////////////////////////////////////////////////////////////////////////

void set_config(const StringArray & a) {
   config_file = a[0];
}

////////////////////////////////////////////////////////////////////////
