// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   mode_analysis.cc
//
//   Description:
//      Based on user specified options, this tool performs a variety
//      of analysis jobs on the ASCII object output of the MODE tool.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    01/01/08  Bullock         New
//   001    10/03/08  Halley Gotway   Add support for:
//                                    AREA_THRESH column,
//                                    apect_ratio min/max options,
//                                    fcst/obs init_time min/max options,
//                                    fcst/obs init_hour options
//   002    11/07/11  Holmes          Added use of command line class to
//                                    parse the command line arguments.
//   003    11/10/11  Holmes          Added code to enable reading of
//                                    multiple config files.
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util.h"
#include "vx_analysis_util.h"

#include "mode_analysis_Conf.h"
#include "config_to_att.h"


////////////////////////////////////////////////////////////////////////


static const char * default_config_filename = "MET_BASE/data/config/MODEAnalysisConfig_default";

static ConcatString config_filename;

static BasicModeAnalysisJob * job = (BasicModeAnalysisJob *) 0;

static ModeAttributes config_atts;

static mode_analysis_Conf config;

static const char * const program_name = "mode_analysis";

static StringArray mode_files;

static StringArray lookin_dirs;

static ofstream * dumpfile = (ofstream *) 0;

static ofstream * outfile  = (ofstream *) 0;

static int debug = 0;


////////////////////////////////////////////////////////////////////////


static void parse_command_line(int, char **);

static void usage();

static void set_lookin_path(const StringArray &);
static void set_summary_jobtype(const StringArray &);
static void set_bycase_jobtype(const StringArray &);
static void set_column_name(const StringArray &);
static void set_dump_row(const StringArray &);
static void set_out_filename(const StringArray &);
static void set_config_filename(const StringArray &);
static void set_debug_option(const StringArray &);

static void set_summary ();
static void set_bycase  ();


static void add_field    (const char *);

static void set_config   (const char * path);
static void set_dumpfile (const char * path);
static void set_outfile  (const char * path);

static void set_debug();

static void set_lookin(const char * path);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

char default_conf_file[PATH_MAX];

// Set handler to be called for memory allocation error
set_new_handler(oom);

if ( argc == 1 )  { usage(); }


parse_command_line(argc, argv);

cout << "\n\n";

if ( debug )  {

   job->dump(cout);

   cout << "\n\n";

}

if ( config_filename.length() > 0 )  {

      //
      // first read the default config file and then read the user's
      //
   replace_string(met_base_str, MET_BASE, default_config_filename, default_conf_file);
   if (debug)
      cout << "Reading Default Config File: " << default_conf_file << "\n" << flush;
   config.read(default_conf_file);
   if (debug)
      cout << "Reading User Config File: " << config_filename << "\n" << flush;
   config.read(config_filename);

   config_to_att(config, config_atts);

   job->atts.augment(config_atts);

   if ( debug )  {

      config_atts.dump(cout);

      job->dump(cout);

   }

}

if ( dumpfile )  job->dumpfile = dumpfile;
if ( outfile  )  job->outfile  = outfile;

job->do_job(mode_files);





   //
   //  done
   //

if ( dumpfile )  { delete dumpfile;  dumpfile = (ofstream *) 0; }
if ( outfile  )  { delete  outfile;   outfile = (ofstream *) 0; }

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void parse_command_line(int argc, char **argv)

{

CommandLine cline;
StringArray cmd_line;
int j;

   //
   // parse the command line into tokens
   //
cline.set(argc, argv);

   //
   // Allow for unrecognized command line switches.
   // This must be called after set above since set calls
   // clear which would reset this to false.
   // This allows us to be able to handle single jobs on
   // the command line.
   //
cline.allow_unrecognized_switches();

   //
   // set the usage function
   //
cline.set_usage(usage);

   //
   // add the options function calls
   //
cline.add(set_lookin_path, "-lookin", 1);
cline.add(set_summary_jobtype, "-summary", 0);
cline.add(set_bycase_jobtype, "-bycase", 0);
cline.add(set_column_name, "-column", 1);
cline.add(set_dump_row, "-dump_row", 1);
cline.add(set_out_filename, "-out", 1);
cline.add(set_config_filename, "-config", 1);
cline.add(set_debug_option, "-debug", 0);

   //
   // parse the command line
   //
cline.parse();


if ( !job )  {

   cerr << "\n\n  parse_command_line() -> no job type set!\n\n";

   exit ( 1 );

}

   //
   // fill a StringArray with the rest of the command line arguments
   // to parse as of old below. All that should remain are the
   // MODE FILE LIST and the MODE LINE OPTIONS if there are any of
   // either.
   //

cmd_line.clear();

for (j=0; j<(cline.n()); ++j)  {

   cmd_line.add(cline[j]);

}

   //
   //  get mode attribute options
   //

job->atts.parse_command_line(cmd_line);

   //
   //  that should be all the options
   //

if ( cmd_line.has_option(j) )  {

   cerr << "\n\n  " << program_name << " -> unrecognized switch: \""
        << cmd_line[j] << "\"\n\n";

   exit  ( 1 );

}

   //
   //  the rest must be mode output filenames
   //

if ( cmd_line.n_elements() != 0 )  {

   mode_files.add(cmd_line);

}

for (j=0; j<(lookin_dirs.n_elements()); ++j)  {

   StringArray a;

   a = get_mode_filenames_from_dir(lookin_dirs[j]);

   mode_files.add(a);

}


   //
   //  check that at least one mode file is specified
   //

if ( mode_files.n_elements() <= 0 )  {

   cerr << "\n\n  parse_command_line() -> at least one MODE file must be specified! "
        << "Use the -lookin option.\n\n";

   exit ( 1 );

}


return;

}


////////////////////////////////////////////////////////////////////////


void usage()

{


cout << "\n*** Model Evaluation Tools (MET" << met_version
     << ") ***\n\n"

     << "Usage: " << program_name << "\n"
     << "\t-lookin path\n"
     << "\t-summary | -bycase\n"
     << "\t[-column name]\n"
     << "\t[-dump_row filename]\n"
     << "\t[-out filename]\n"
     << "\t[-help]\n"
     << "\t[MODE FILE LIST]\n"
     << "\t[-config config_file] | [MODE LINE OPTIONS]\n\n"

     << "\twhere\t\"-lookin path\" specifies a MODE file or a top-level "
     << "directory containing MODE files to be used (at least one "
     << "required).\n"

     << "\t\t\"-summary\" or \"-by_case\" specifies the job type "
     << "to be run (exactly one required).\n"

     << "\t\t\"-column name\" specifies the column of the MODE line "
     << "for the analysis (at least one required for -summary job).\n"

     << "\t\t\"-dump_row filename\" specifies the file name to which "
     << "the MODE lines used for the analysis should be written "
     << "(optional).\n"

     << "\t\t\"-out filename\" specifies the file name to which "
     << "output should be written rather than the screen (optional).\n"

     << "\t\t\"-help\" prints this usage statement (optional).\n"

     << "\t\t\"MODE FILE LIST\" is a list of MODE file names to be "
     << "used (optional).\n"

     << "\t\t\"-config config_file\" specifies a MODEAnalysis config "
     << "file containing MODE line filtering options to be used "
     << "(optional).\n"

     << "\t\t\"MODE LINE OPTIONS\" specifies the MODE line filtering "
     << "options (see below) to be used explicitly on the command "
     << "line (optional).\n\n"

     << "MODE LINE OPTIONS:\n" << flush;

mode_atts_usage(cout);

exit (1);

}


////////////////////////////////////////////////////////////////////////


void set_summary()

{

if ( job )  {

   cerr << "\n\n  set_summary() -> job type already set!\n\n";

   exit ( 1 );

}

job = new SummaryJob;


return;

}


////////////////////////////////////////////////////////////////////////


void set_bycase()

{

if ( job )  {

   cerr << "\n\n  set_bycase() -> job type already set!\n\n";

   exit ( 1 );

}

job = new ByCaseJob;


return;

}


////////////////////////////////////////////////////////////////////////


void set_config(const char * path)

{

config_filename = path;

return;

}


////////////////////////////////////////////////////////////////////////


void add_field(const char * name)

{

if ( !job )  {

   cerr << "\n\n  add_field(const char *) -> no job type set!\n\n";

   exit ( 1 );

}

if ( all_digits(name) )  {

   int k = atoi(name);

   job->add_column_by_number(k - 1);

} else {

   job->add_column_by_name(name);

}

return;

}


////////////////////////////////////////////////////////////////////////


void set_dumpfile(const char * path)

{

if ( dumpfile )  {

   cerr << "\n\n  set_dumpfile(const char * path) -> output dump file already specified!\n\n";

   exit ( 1 );

}

dumpfile = new ofstream;

dumpfile->open(path);

if ( !(*dumpfile) )  {

   cerr << "\n\n  set_dumpfile(const char * path) -> can't open dump file \""
        << path << "\" for output\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void set_outfile(const char * path)

{

if ( outfile )  {

   cerr << "\n\n  void set_outfile(const char *) -> output file already set\n\n";

   exit ( 1 );

}

outfile = new ofstream;

outfile->open(path);

if ( !(*outfile) )  {

   cerr << "\n\n  set_outfile(const char * path) -> unable to open output file \""
        << path << "\"\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void set_lookin(const char * path)

{

struct stat s;

if ( stat(path, &s) < 0 )  {

   cerr << "\n\n  set_lookin() -> can't stat \"" << path << "\"\n\n";

   exit ( 1 );

}

     if ( S_ISREG(s.st_mode) )  mode_files.add(path);
else if ( S_ISDIR(s.st_mode) )  lookin_dirs.add(path);
else {

   cerr << "\n\n  set_lookin() -> bad type for \"" << path << "\"\n\n";

   exit ( 1 );

}


return;

}


////////////////////////////////////////////////////////////////////////


void set_debug()

{

debug = 1;

return;

}


////////////////////////////////////////////////////////////////////////


void set_lookin_path(const StringArray & a)

{

set_lookin(a[0]);

return;

}


////////////////////////////////////////////////////////////////////////


void set_summary_jobtype(const StringArray &)
{

set_summary();

return;

}


////////////////////////////////////////////////////////////////////////


void set_bycase_jobtype(const StringArray &)

{

set_bycase();

return;

}


////////////////////////////////////////////////////////////////////////


void set_column_name(const StringArray & a)

{

add_field(a[0]);

return;

}


////////////////////////////////////////////////////////////////////////


void set_dump_row(const StringArray & a)

{

set_dumpfile(a[0]);

return;

}


////////////////////////////////////////////////////////////////////////


void set_out_filename(const StringArray & a)

{

set_outfile(a[0]);

return;

}


////////////////////////////////////////////////////////////////////////


void set_config_filename(const StringArray & a)

{

set_config(a[0]);

return;

}


////////////////////////////////////////////////////////////////////////


void set_debug_option(const StringArray &)

{

set_debug();

return;

}


////////////////////////////////////////////////////////////////////////


