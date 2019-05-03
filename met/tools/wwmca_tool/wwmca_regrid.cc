

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_util/vx_util.h"

#include "wwmca_config.h"
#include "wwmca_ref.h"


////////////////////////////////////////////////////////////////////////


   //
   //  default valued for command-line switches
   //


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static ConcatString nh_filename;
static ConcatString sh_filename;

static ConcatString output_filename;


////////////////////////////////////////////////////////////////////////


static CommandLine cline;

static WwmcaRegridder regridder;

static WwmcaConfig config;

static ConcatString config_filename;


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_nh_filename (const StringArray &);
static void set_sh_filename (const StringArray &);
static void set_outfile     (const StringArray &);
static void set_config      (const StringArray &);

static void sanity_check();


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc == 1 )  usage();

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_nh_filename, "-nh",     1);
cline.add(set_sh_filename, "-sh",     1);
cline.add(set_outfile,     "-out",    1);
cline.add(set_config,      "-config", 1);

cline.parse();

if ( cline.n() != 0 )  usage();

sanity_check();

   //
   // read config file
   //

config.read(config_filename);

   //
   //  load up regridder
   //

if ( nh_filename.length() > 0 )  regridder.set_nh_file(nh_filename);
if ( sh_filename.length() > 0 )  regridder.set_sh_file(sh_filename);

regridder.set_config(config, config_filename);

   //
   //  done
   //

// regridder.dump(cout);

regridder.do_output(output_filename);









   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name
     << " -out filename"
     << " -config filename"
     << " [ -nh filename ]"
     << " [ -sh filename ]"
     << "\n\n";


exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void set_nh_filename(const StringArray & a)

{

nh_filename = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void set_sh_filename(const StringArray & a)

{

sh_filename = a[0];

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


void sanity_check()

{

bool trouble = false;

   //
   //  check that a config file was given
   //

if ( config_filename.length() == 0 )  {

   trouble = true;

   cerr << "\n\n  " << program_name << ": no config file set!\n\n";

   // exit ( 1 );   

}

   //
   //  check that an output file was given
   //

if ( output_filename.length() == 0 )  {

   trouble = true;

   cerr << "\n\n  " << program_name << ": no output file set!\n\n";

   // exit ( 1 );   

}

   //
   //  check that at least one input file was given
   //

if ( (nh_filename.length() == 0) && (sh_filename.length() == 0) )  {

   trouble = true;

   cerr << "\n\n  " << program_name << ": no input file(s) set!\n\n";

   // exit ( 1 );   

}


   //
   //  done
   //

if ( trouble )  usage();

return;

}


////////////////////////////////////////////////////////////////////////


