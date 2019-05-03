// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <cmath>
#include <ctype.h>

#include "vx_log.h"
#include "vx_util.h"

#include "wwmca_ref.h"


////////////////////////////////////////////////////////////////////////


static const char * default_config_filename = "MET_BASE/config/WWMCARegridConfig_default";


////////////////////////////////////////////////////////////////////////


   //
   //  Default values for command-line switches
   //


static ConcatString cp_nh_filename;
static ConcatString cp_sh_filename;

static ConcatString pt_nh_filename;
static ConcatString pt_sh_filename;

static ConcatString output_filename;

static int compress_level = -1;

////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;

static WwmcaRegridder regridder;

static MetConfig config;

static ConcatString config_filename;


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_nh_filename (const StringArray &);
static void set_sh_filename (const StringArray &);
static void set_outfile     (const StringArray &);
static void set_config      (const StringArray &);
static void set_logfile     (const StringArray &);
static void set_verbosity   (const StringArray &);
static void set_compress    (const StringArray &);

static void sanity_check();


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

ConcatString default_config_file;


program_name = get_short_name(argv[0]);

if ( argc == 1 )  usage();

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_nh_filename, "-nh",      -1);
cline.add(set_sh_filename, "-sh",      -1);

cline.add(set_outfile,     "-out",      1);
cline.add(set_config,      "-config",   1);
cline.add(set_logfile,     "-log",      1);
cline.add(set_verbosity,   "-v",        1);
cline.add(set_compress,    "-compress", 1);

cline.parse();

if ( cline.n() != 0 )  usage();

sanity_check();

   //
   //  create the default config file name
   //

default_config_file = replace_path(default_config_filename);

   //
   //  list the config files
   //

mlog << Debug(1)
     << "Default Config File: " << default_config_file << "\n"
     << "User Config File: "    << config_filename << "\n";

   //
   //  read config file constants, the default config file,
   //  and then the user config file.
   //

config.read(replace_path(config_const_filename).c_str());
config.read(default_config_file.c_str());
config.read(config_filename.c_str());

   //
   //  dump the contents of the config file
   //

if ( mlog.verbosity_level() >= 5 ) config.dump(cout);

    //
    //  parse swap endian
    //

bool swap = config.lookup_bool(conf_key_swap_endian);

   //
   //  load up regridder
   //

if ( cp_nh_filename.length() > 0 )  regridder.set_cp_nh_file(cp_nh_filename.c_str());
if ( cp_sh_filename.length() > 0 )  regridder.set_cp_sh_file(cp_sh_filename.c_str());
if ( pt_nh_filename.length() > 0 )  regridder.set_pt_nh_file(pt_nh_filename.c_str(), swap);
if ( pt_sh_filename.length() > 0 )  regridder.set_pt_sh_file(pt_sh_filename.c_str(), swap);

regridder.set_config(config, config_filename.c_str());

   //
   //  done
   //

if ( mlog.verbosity_level() >= 5 )  regridder.dump(cout);

regridder.do_output(output_filename.c_str());


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cout << "\nUsage: " << program_name << "\n"
     << "\t-out filename\n"
     << "\t-config filename\n"
     << "\t-nh filename [pt_filename]\n"
     << "\t-sh filename [pt_filename]\n"
     << "\t[-log file]\n"
     << "\t[-v level]\n"
     << "\t[-compress level]\n\n"
     << "\twhere\t\"-out filename\" is the name of the output "
     << "file to create (required).\n"
     << "\t\t\"-config filename\" is a WWMCARegridConfig file "
     << "containing the desired configuration settings (required).\n"
     << "\t\t\"-nh filename [pt_filename]\" are the names of the "
     << "binary data file and the optional pixel time (pt) file for the "
     << "northern hemisphere to regrid.\n"
     << "\t\t\"-sh filename [pt_filename]\" are the names of the "
     << "binary data file and the optional pixel time (pt) file for the "
     << "southern hemisphere to regrid.\n"
     << "\t\t\"-log file\" outputs log messages to the specified "
     << "file (optional).\n"
     << "\t\t\"-v level\" overrides the default level of logging ("
     << mlog.verbosity_level() << ") (optional).\n"
     << "\t\t\"-compress level\" overrides the compression level of NetCDF variable (optional).\n\n";


exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void set_nh_filename(const StringArray & a)

{

if ( a.n_elements() != 1 && a.n_elements() != 2 )  {
   mlog << Error << "\nset_nh_filename() -> "
        << "unexpected number of arguments (" << a.n_elements()
        << ").\n\n";
   exit ( 1 );
}

cp_nh_filename = a[0];
if ( a.n_elements() == 2)  pt_nh_filename = a[1];

return;

}


////////////////////////////////////////////////////////////////////////


void set_sh_filename(const StringArray & a)

{

if ( a.n_elements() != 1 && a.n_elements() != 2 )  {
   mlog << Error << "\nset_sh_filename() -> "
        << "unexpected number of arguments (" << a.n_elements()
        << ").\n\n";
   exit ( 1 );
}

cp_sh_filename = a[0];
if ( a.n_elements() == 2)  pt_sh_filename = a[1];

return;

}


////////////////////////////////////////////////////////////////////////


void set_outfile(const StringArray & a)

{

output_filename = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void set_config(const StringArray & a)

{

config_filename = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void set_logfile(const StringArray & a)

{

ConcatString filename;

filename = a[0];

mlog.open_log_file(filename);

return;

}


/////////////////////////////////////////////////////////////////////////


void set_verbosity(const StringArray & a)

{

mlog.set_verbosity_level(atoi(a[0].c_str()));

return;

}


////////////////////////////////////////////////////////////////////////

int get_compress() {
   //return ((compress_level < 0)? 0 : compress_level);
   return compress_level;
}

////////////////////////////////////////////////////////////////////////

void set_compress(const StringArray & a) {
   compress_level = atoi(a[0].c_str());
}


////////////////////////////////////////////////////////////////////////


void sanity_check()

{

bool trouble = false;

   //
   //  check that a config file was given
   //

if ( config_filename.length() == 0 )  {

   trouble = true;

   mlog << Error << "\n" << program_name << ": no config file set!\n\n";

   // exit ( 1 );

}

   //
   //  check that an output file was given
   //

if ( output_filename.length() == 0 )  {

   trouble = true;

   mlog << Error << "\n" << program_name << ": no output file set!\n\n";

   // exit ( 1 );

}

   //
   //  check that at least one input file was given
   //

if ( (cp_nh_filename.length() == 0) && (cp_sh_filename.length() == 0) )  {

   trouble = true;

   mlog << Error << "\n" << program_name << ": no input file(s) set!\n\n";

   // exit ( 1 );

}


   //
   //  done
   //

if ( trouble )  usage();

return;

}


////////////////////////////////////////////////////////////////////////


