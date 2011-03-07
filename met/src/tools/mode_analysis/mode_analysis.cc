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
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util/vx_util.h"

#include "vx_analysis_util/mode_columns.h"
#include "vx_analysis_util/mode_job.h"
#include "vx_analysis_util/mode_atts.h"
#include "vx_analysis_util/analysis_utils.h"

#include "mode_analysis_Conf.h"
#include "config_to_att.h"


////////////////////////////////////////////////////////////////////////


static ConcatString config_filename;

static BasicModeAnalysisJob * job = (BasicModeAnalysisJob *) 0;

static ModeAttributes config_atts;

static mode_analysis_Conf config;

static const char * const program_name = "mode_analysis";

static StringArray cline;

static StringArray mode_files;

static StringArray lookin_dirs;

static ofstream * dumpfile = (ofstream *) 0;

static ofstream * outfile  = (ofstream *) 0;

static int debug = 0;


////////////////////////////////////////////////////////////////////////


static void parse_command_line();

static void usage();

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

// Set handler to be called for memory allocation error
set_new_handler(oom);

if ( argc == 1 )  { usage();  exit(1); }

int j;

   //
   //  setup
   //

for (j=1; j<argc; ++j)  {

   cline.add(argv[j]);

}


parse_command_line();

cout << "\n\n";

if ( debug )  {

   job->dump(cout);

   cout << "\n\n";

}

if ( config_filename.length() > 0 )  {

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


void parse_command_line()

{

int j;
const char * c = (const char *) 0;

   //
   //  get analysis options
   //

j = 0;

while ( j < (cline.n_elements()) )  {

   c = cline[j];

   if ( c[0] != '-' )  { ++j;  continue; }

        if ( strcmp(c, "-lookin"    ) == 0 )  { set_lookin   (cline[j + 1]);  cline.shift_down(j, 2); }
   else if ( strcmp(c, "-config"    ) == 0 )  { set_config   (cline[j + 1]);  cline.shift_down(j, 2); }
   else if ( strcmp(c, "-out"       ) == 0 )  { set_outfile  (cline[j + 1]);  cline.shift_down(j, 2); }
   else if ( strcmp(c, "-debug"     ) == 0 )  { set_debug    ();              cline.shift_down(j, 1); }
   else if ( strcmp(c, "-help"      ) == 0 )  { usage();                      exit ( 1 );             }

   else if ( strcmp(c, "-summary"   ) == 0 )  { set_summary  ();              cline.shift_down(j, 1); }
   else if ( strcmp(c, "-bycase"    ) == 0 )  { set_bycase   ();              cline.shift_down(j, 1); }
   else if ( strcmp(c, "-column"    ) == 0 )  { add_field    (cline[j + 1]);  cline.shift_down(j, 2); }
   else if ( strcmp(c, "-dump_row")   == 0 )  { set_dumpfile (cline[j + 1]);  cline.shift_down(j, 2); }




      //
      //  no more options of interest here
      //

   else ++j;

}   //  while

if ( !job )  {

   cerr << "\n\n  parse_command_line() -> no job type set!\n\n";

   exit ( 1 );

}


   //
   //  get mode attribute options
   //

job->atts.parse_command_line(cline);

   //
   //  that should be all the options
   //

if ( cline.has_option(j) )  {

   cerr << "\n\n  " << program_name << " -> unrecognized switch: \""
        << cline[j] << "\"\n\n";

   exit  ( 1 );

}

   //
   //  the rest must be mode output filenames
   //

if ( cline.n_elements() != 0 )  {

   mode_files.add(cline);

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

return;

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




