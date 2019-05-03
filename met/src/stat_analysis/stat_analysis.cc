// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   stat_analysis.cc
//
//   Description:
//      Based on user specified options, this tool performs a variety
//      of analysis jobs on the STAT output of the Grid-Stat,
//      Point-Stat, and Wavelet-Stat tools.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    09/05/07  Halley Gotway   New
//   001    07/01/08  Halley Gotway   Add the rank_corr_flag to the
//                    config file to disable computing rank
//                    correlations.
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cmath>
#include <cstdio>
#include <dirent.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "stat_analysis.h"
#include "parse_stat_line.h"
#include "aggr_stat_line.h"
#include "stat_analysis_job.h"

////////////////////////////////////////////////////////////////////////

static void parse_command_line(int &argc, char **argv);
static void sanity_check();
static void usage();
static void set_default_job();
static void process_search_dirs();
static void process_stat_file(const char *, int &, int &);
static void process_job(const char *, int);
static void clean_up();

//
// Routines for command-line switches
//
static void set_config(const char *);
static void set_search_dir(const char *);
static void set_out_file(const char *);
static void set_verbosity(int);
static int  is_switch(const char *);

////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv []) {
   int i, n;
   Result r;

   //
   // Set handler to be called for memory allocation error
   //
   set_new_handler(oom);

   //
   // Parse the command line
   //
   parse_command_line(argc, argv);

   //
   // If a config file was specified, set up the default job using the
   // config file.
   //
   if(config_file != (char *) 0) {

      //
      // Read the config file
      //
      conf.read(config_file);

      //
      // Sanity check the command line and config file options
      //
      sanity_check();

      //
      // Setup the default job using the options specified in the config
      // file
      //
      set_default_job();
   }

   //
   // Enclose within a try block to catch any run time errors, and
   // delete the temp file before exiting.
   //
   try {

      //
      // Process the STAT files found in the search directories.
      //
      process_search_dirs();

      //
      // If a config file was specified, process the jobs in the config
      // file.
      //
      if(config_file != (char *) 0) {

         n = conf.n_jobs_elements();

         for(i=0; i<n; i++) {
            r = conf.jobs(i);
            process_job(r.sval(), i+1);
         }
      }
      //
      // Otherwise, process the job specified on the command line.
      //
      else {
         process_job(command_line_job, 1);
      }

   }
   catch(int j) { // Catch an error

      cerr << "\n\nERROR: main() -> "
           << "encountered an error value of " << j
           << ".  Calling clean_up() and usage() before exiting.\n\n"
           << flush;

      clean_up();
      usage();
      exit(j);
   }

   //
   // Deallocate memory and clean up
   //
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void parse_command_line(int &argc, char **argv) {
   int i;

   //
   // Check for the minimum number of required arguments
   //
   if(argc < 5) {
      usage();
      exit(1);
   }

   //
   // Parse the command line arguments
   //
   config_file = (char *) 0;

   for(i=1; i<argc; i++) {

      if(strcmp(argv[i], "-config") == 0){
         set_config(argv[i+1]);
         i++;
      }
      else if(strcmp(argv[i], "-lookin") == 0) {
         while(i+1<argc && !is_switch(argv[i+1])) {
            set_search_dir(argv[i+1]);
            i++;
         }
      }
      else if(strcmp(argv[i], "-out") == 0) {
         set_out_file(argv[i+1]);
         i++;
      }
      else if(strcmp(argv[i], "-v") == 0) {
         set_verbosity(atoi(argv[i+1]));
         i++;
      }
   } // end for

   //
   // If no config file was specified, parse out the STAT Analysis job
   // command from the command line.
   //
   if(config_file == (char *) 0) {

      for(i=1; i<argc; i++) {

         //
         // Skip the command line options not specific to the
         // STAT Analysis job
         //
         if(strcmp(argv[i], "-out") == 0 ||
            strcmp(argv[i], "-v") == 0) {
            i++;
            continue;
         }
         else if(strcmp(argv[i], "-lookin") == 0) {

            while(i+1<argc && !is_switch(argv[i+1])) i++;

            continue;
         }
         //
         // Store the remaining options as the STAT Command Line job
         //
         else {
            if(strlen(command_line_job) == 0)
               sprintf(command_line_job, "%s", argv[i]);
            else
               sprintf(command_line_job, "%s %s",
                       command_line_job, argv[i]);
         }
      } // end for
   } // end if

   return;
}

////////////////////////////////////////////////////////////////////////

void sanity_check() {
   unixtime ut_beg, ut_end;

   //
   // Check for at least one search file or directory
   //
   if(search_dirs.n_elements() == 0) {
      cerr << "\n\nERROR: sanity_check() -> "
           << "no STAT search files or directories specified!\n\n"
           << flush;

      exit(1);
   }

   //
   // Check for fcst_valid_beg > fcst_valid_end
   //
   ut_beg = timestring_to_unix(conf.fcst_valid_beg().sval());
   ut_end = timestring_to_unix(conf.fcst_valid_end().sval());

   if((ut_beg > 0) && (ut_end > 0) && (ut_beg > ut_end)) {
      cerr << "\n\nERROR: sanity_check() -> "
           << "fcst_valid_beg is after fcst_valid_end: "
           << ut_beg << " > " << ut_end << "!\n\n";

      exit(1);
   }

   //
   // Check for obs_valid_beg > obs_valid_end
   //
   ut_beg = timestring_to_unix(conf.obs_valid_beg().sval());
   ut_end = timestring_to_unix(conf.obs_valid_end().sval());

   if((ut_beg > 0) && (ut_end > 0) && (ut_beg > ut_end)) {
      cerr << "\n\nERROR: sanity_check() -> "
           << "obs_valid_beg is after obs_valid_end: "
           << ut_beg << " > " << ut_end << "!\n\n";

      exit(1);
   }

   //
   // Check for fcst_init_beg > fcst_init_end
   //
   ut_beg = timestring_to_unix(conf.fcst_init_beg().sval());
   ut_end = timestring_to_unix(conf.fcst_init_end().sval());

   if((ut_beg > 0) && (ut_end > 0) && (ut_beg > ut_end)) {
      cerr << "\n\nERROR: sanity_check() -> "
           << "fcst_init_beg is after fcst_init_end: "
           << ut_beg << " > " << ut_end << "!\n\n";

      exit(1);
   }

   //
   // Check for obs_init_beg > obs_init_end
   //
   ut_beg = timestring_to_unix(conf.obs_init_beg().sval());
   ut_end = timestring_to_unix(conf.obs_init_end().sval());

   if((ut_beg > 0) && (ut_end > 0) && (ut_beg > ut_end)) {
      cerr << "\n\nERROR: sanity_check() -> "
           << "obs_init_beg is after obs_init_end: "
           << ut_beg << " > " << ut_end << "!\n\n";

      exit(1);
   }

   //
   // Conf: rank_corr_flag
   //
   if(conf.rank_corr_flag().ival() != 0 &&
      conf.rank_corr_flag().ival() != 1) {
      cerr << "\n\nERROR: sanity_check() -> "
           << "The rank_corr_flag (" << conf.rank_corr_flag().ival()
           << ") must be set to 0 or 1.\n\n"
           << flush;
      exit(1);
   }

   //
   // Conf: tmp_dir
   //
   if(opendir(conf.tmp_dir().sval()) == NULL ) {
      cerr << "\n\nERROR: sanity_check() -> "
           << "Cannot access the tmp_dir temporary directory: "
           << conf.tmp_dir().sval() << "\n\n" << flush;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void set_config(const char *path) {

   config_file = path;

   return;
}

////////////////////////////////////////////////////////////////////////

void set_search_dir(const char *path) {

   search_dirs.add(path);

   return;
}

////////////////////////////////////////////////////////////////////////

void set_out_file(const char *path) {

   out_file = path;

   //
   // Create an output file and set the sa_out ofstream to it.
   //
   sa_out = new ofstream;
   sa_out->open(out_file);

   if(!sa_out) {
      cerr << "\n\nERROR: set_out_file()-> "
           << "can't open the output file \"" << out_file
           << "\" for writing!\n\n" << flush;

      throw(1);
   }

   if(verbosity > 0) {
      cout << "Creating STAT-Analysis output file \""
           << out_file << "\"\n" << flush;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(int i) {

   verbosity = i;

   return;
}

////////////////////////////////////////////////////////////////////////

int is_switch(const char *c) {
   int r;

   if(strlen(c) > 0 && c[0] == '-') r = 1;
   else                             r = 0;

   return(r);
}

////////////////////////////////////////////////////////////////////////

void set_default_job() {
   int i, n;
   Result r;

   //
   // Clear out the default job
   //
   default_job.clear();

   //
   // Get info from config file and store in "default_job"
   //

   //
   // model
   //
   n = conf.n_model_elements();

   for(i=0; i<n; i++) {
      r = conf.model(i);
      default_job.model.add(r.sval());
   }

   //
   // fcst_lead
   //
   n = conf.n_fcst_lead_elements();

   for(i=0; i<n; i++) {
      r = conf.fcst_lead(i);
      default_job.fcst_lead.add(timestring_to_sec(r.sval()));
   }

   //
   // obs_lead
   //
   n = conf.n_obs_lead_elements();

   for(i=0; i<n; i++) {
      r = conf.obs_lead(i);
      default_job.obs_lead.add(timestring_to_sec(r.sval()));
   }

   //
   // fcst_valid_beg
   //
   r = conf.fcst_valid_beg();

   if(strlen(r.sval()) > 0)
      default_job.fcst_valid_beg = timestring_to_unix(r.sval());

   //
   // fcst_valid_end
   //
   r = conf.fcst_valid_end();

   if(strlen(r.sval()) > 0)
      default_job.fcst_valid_end = timestring_to_unix(r.sval());

   //
   // obs_valid_beg
   //
   r = conf.obs_valid_beg();

   if(strlen(r.sval()) > 0)
      default_job.obs_valid_beg = timestring_to_unix(r.sval());

   //
   // obs_valid_end
   //
   r = conf.obs_valid_end();

   if(strlen(r.sval()) > 0)
      default_job.obs_valid_end = timestring_to_unix(r.sval());

   //
   // fcst_init_beg
   //
   r = conf.fcst_init_beg();

   if(strlen(r.sval()) > 0)
      default_job.fcst_init_beg = timestring_to_unix(r.sval());

   //
   // fcst_init_end
   //
   r = conf.fcst_init_end();

   if(strlen(r.sval()) > 0)
      default_job.fcst_init_end = timestring_to_unix(r.sval());

   //
   // obs_init_beg
   //
   r = conf.obs_init_beg();

   if(strlen(r.sval()) > 0)
      default_job.obs_init_beg = timestring_to_unix(r.sval());

   //
   // obs_init_end
   //
   r = conf.obs_init_end();

   if(strlen(r.sval()) > 0)
      default_job.obs_init_end = timestring_to_unix(r.sval());

   //
   // fcst_init_hour
   //
   n = conf.n_fcst_init_hour_elements();

   for(i=0; i<n; i++) {
      r = conf.fcst_init_hour(i);
      default_job.fcst_init_hour.add(timestring_to_sec(r.sval()));
   }

   //
   // obs_init_hour
   //
   n = conf.n_obs_init_hour_elements();

   for(i=0; i<n; i++) {
      r = conf.obs_init_hour(i);
      default_job.obs_init_hour.add(timestring_to_sec(r.sval()));
   }

   //
   // fcst_var
   //
   n = conf.n_fcst_var_elements();

   for(i=0; i<n; i++) {
      r = conf.fcst_var(i);
      default_job.fcst_var.add(r.sval());
   }

   //
   // obs_var
   //
   n = conf.n_obs_var_elements();

   for(i=0; i<n; i++) {
      r = conf.obs_var(i);
      default_job.obs_var.add(r.sval());
   }

   //
   // fcst_lev
   //
   n = conf.n_fcst_lev_elements();

   for(i=0; i<n; i++) {
      r = conf.fcst_lev(i);
      default_job.fcst_lev.add(r.sval());
   }

   //
   // obs_lev
   //
   n = conf.n_obs_lev_elements();

   for(i=0; i<n; i++) {
      r = conf.obs_lev(i);
      default_job.obs_lev.add(r.sval());
   }

   //
   // obtype
   //
   n = conf.n_obtype_elements();

   for(i=0; i<n; i++) {
      r = conf.obtype(i);
      default_job.obtype.add(r.sval());
   }

   //
   // vx_mask
   //
   n = conf.n_vx_mask_elements();

   for(i=0; i<n; i++) {
      r = conf.vx_mask(i);
      default_job.vx_mask.add(r.sval());
   }

   //
   // interp_mthd
   //
   n = conf.n_interp_mthd_elements();

   for(i=0; i<n; i++) {
      r = conf.interp_mthd(i);
      default_job.interp_mthd.add(r.sval());
   }

   //
   // interp_pnts
   //
   n = conf.n_interp_pnts_elements();

   for(i=0; i<n; i++) {
      r = conf.interp_pnts(i);
      default_job.interp_pnts.add(r.ival());
   }

   //
   // fcst_thresh
   //
   n = conf.n_fcst_thresh_elements();

   for(i=0; i<n; i++) {
      r = conf.fcst_thresh(i);
      default_job.fcst_thresh.add(r.sval());
   }

   //
   // obs_thresh
   //
   n = conf.n_obs_thresh_elements();

   for(i=0; i<n; i++) {
      r = conf.obs_thresh(i);
      default_job.obs_thresh.add(r.sval());
   }

   //
   // cov_thresh
   //
   n = conf.n_cov_thresh_elements();

   for(i=0; i<n; i++) {
      r = conf.cov_thresh(i);
      default_job.cov_thresh.add(r.sval());
   }

   //
   // alpha
   //
   n = conf.n_alpha_elements();

   for(i=0; i<n; i++) {
      r = conf.alpha(i);
      default_job.alpha.add(r.ival());
   }

   //
   // line_type
   //
   n = conf.n_line_type_elements();

   for(i=0; i<n; i++) {
      r = conf.line_type(i);
      default_job.line_type.add(r.sval());
   }

   //
   // No settings in the default job for column_min_name,
   // column_min_value, column_max_name, and column_max_value since
   // those are strictly job command options.
   //

   //
   // out_out_alpha
   //
   default_job.out_alpha = conf.out_alpha().dval();

   //
   // boot_interval
   //
   default_job.boot_interval = conf.boot_interval().ival();

   //
   // boot_rep_prop
   //
   default_job.boot_rep_prop = conf.boot_rep_prop().dval();

   //
   // n_boot_rep
   //
   default_job.n_boot_rep = conf.n_boot_rep().ival();

   //
   // boot_rng
   //
   default_job.set_boot_rng(conf.boot_rng().sval());

   //
   // boot_seed
   //
   default_job.set_boot_seed(conf.boot_seed().sval());

   //
   // rank_corr_flag
   //
   default_job.rank_corr_flag = conf.rank_corr_flag().ival();

   //
   // tmp_dir
   //
   default_job.set_tmp_dir(conf.tmp_dir().sval());

   return;
}

////////////////////////////////////////////////////////////////////////

void process_search_dirs() {
   int n, i, j, max_len, n_read, n_keep;
   pid_t p;

   //
   // Initialize
   //
   n_read = n_keep = 0;

   //
   // Get the list of stat files in the search directories
   //
   files = get_stat_filenames(search_dirs);

   n = files.n_elements();

   if(n == 0) {
      cerr << "\n\nERROR: process_search_dirs() -> "
           << "no STAT files found in the directories specified!\n\n"
           << flush;

      throw(1);
   }

   //
   // Build the temp file name
   //
   p = getpid();
   tmp_file << "tmp_" << (int) p << ".stat";

   //
   // Open up the temp file for storing the intermediate STAT line data
   //
   if(config_file != (char *) 0) tmp_dir = conf.tmp_dir().sval();
   else                          tmp_dir = default_tmp_dir;

   tmp_path << tmp_dir << "/" << tmp_file;
   tmp_out.open(tmp_path);
   if(!tmp_out) {
      cerr << "\n\nERROR: process_search_dirs() -> "
           << "can't open the temporary file \"" << tmp_path
           << "\" for writing!\n\n" << flush;

      throw(1);
   }

   //
   // Go through each input file
   //
   max_len = 0;

   for(i=0; i<n; i++)  {
      j = strlen(files[i]);
      if(j > max_len) max_len = j;
   }
   max_len += 3;

   for(i=0; i<n; i++) {
      if(verbosity > 2) {

         cout << "Processing STAT file \"" << files[i] << "\" ";

         for(j=strlen(files[i]); j<max_len; j++) cout.put('.');

         cout << " " << i+1 << " of " << n << "\n";

         if((j%5) == 4) cout.put('\n');

         cout.flush();
      }

      process_stat_file(files[i], n_read, n_keep);
   }

   if(verbosity > 1) {
      cout << "STAT Lines read     = " << n_read << "\n";
      cout << "STAT Lines retained = " << n_keep << "\n";
   }

   tmp_out.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void process_stat_file(const char *filename, int &n_read, int &n_keep) {
   LineDataFile f;
   STATLine line;

   if(!(f.open(filename))) {
      cerr << "\n\nERROR: process_stat_file() -> "
           << "unable to open input stat file \""
           << filename << "\"\n\n" << flush;

      throw(1);
   }

   while(f >> line) {

      //
      // Continue if the line is not a valid STAT line type.
      //
      if(line.type() == no_stat_line_type)
         continue;

      n_read++;

      if(default_job.is_keeper(line)) {

         n_keep++;

         tmp_out << line;
      }
   } // end while

   f.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void process_job(const char * jobstring, int n_job) {
   STATAnalysisJob job;
   char full_jobstring[1024];

   //
   // Initialize to the default job
   //
   job = default_job;

   //
   // Parse the job command line options
   //
   job.parse_job_command(jobstring);

   //
   // Get the full jobstring
   //
   job.get_jobstring(full_jobstring);

   //
   // Do the job
   //
   do_job(full_jobstring, job, n_job, tmp_path, sa_out, verbosity);

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {

   //
   // Delete the temp file
   //
   if(remove(tmp_path) != 0) {
      cerr << "\n\nERROR: clean_up() -> "
           << "can't remove temporary file \"" << tmp_path
           << "\"\n\n" << flush;

      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\t-lookin path\n"
        << "\t[-out filename]\n"
        << "\t[-v level]\n"
        << "\t[-config config_file] | [JOB COMMAND LINE]\n\n"

        << "\twhere\t\"-lookin path\" specifies a STAT file or "
        << "top-level directory containing STAT files.  It allows the "
        << "use of wildcards (at least one required).\n"

        << "\t\t\"-out filename\" specifies a file name to which "
        << "output should written rather than the screen (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << verbosity << ") (optional).\n"

        << "\t\t\"-config config_file\" specifies a STATAnalysis "
        << "config file containing STATAnalysis jobs to be run.\n"

        << "\t\t\"JOB COMMAND LINE\" specifies all the arguments "
        << "necessary to perform a single STATAnalysis job.\n\n"

        << "\tNOTE: Refer to a STATAnalysis config file for details "
        << "on how to specify the JOB COMMAND LINE.\n\n"

        << flush;

   return;
}

////////////////////////////////////////////////////////////////////////
