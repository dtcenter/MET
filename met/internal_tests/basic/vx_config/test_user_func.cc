// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


static const bool debug = false;


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_util.h"

#include "config_file.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Default values for command-line switches
   //

static bool do_dump = false;


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;


////////////////////////////////////////////////////////////////////////


static void set_do_dump(const StringArray &);

static void usage();


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_do_dump, "-dump", 0);

cline.parse();

if ( cline.n() == 0 )  usage();


MetConfig config;

   //
   //  redirect stderr to stdout
   //

if ( dup2(1, 2) < 0 )  {

   cerr << "\n\n  " << program_name << ": dup2 failed\n\n";

   exit ( 1 );

}

   //
   //  read the config file(s)
   //

config.set_debug(debug);

int j;
double x, y;
bool status = false;
ConcatString function_name;

for (j=0; j<(cline.n()); ++j)  {   //  j starts at one, here, not zero

   if ( cline[j].compare(":") == 0 )  break;

   status = config.read(cline[j].c_str());

   if ( ! status )  {

      cerr << "\n\n  " << program_name << ": failed to parse config file \"" << cline[j] << "\"\n\n";

      exit ( 1 );

   }

}

++j;   //  skip the ":"

function_name = cline[j++];

// int n_args = argc - j;

x = atof(cline[j].c_str());

   //
   //
   //

const DictionaryEntry * e = config.lookup(function_name.c_str());

if ( !e )  {

   cout << "\n\n  lookup failed for function \"" << function_name << "\"!\n\n";

   exit ( 1 );

}

if ( do_dump )  {

   cout << "\n\nDictionaryEntry dump:\n";

   e->dump(cout);

}

UserFunc_1Arg f;

f.set(e);

// if ( f.n_args() != n_args )  {
//
//    cout << "\n\n  " << program_name << ": expected "
//         << (f.n_args()) << " argument(s) for function \""
//         << function_name << "\" ... got " << n_args
//         << "\n\n";
//
//    exit ( 1 );
//
// }

y = f(x);


cout << "\n\n"
     << "function name = \"" << function_name << "\"\n\n";

// cout << "# args = " << n_args << "\n\n";


cout << "argument value = " << x << "\n\n";

cout << "result = " << y << "\n\n";

   //
   //  done
   //

return ( 0 );

}

////////////////////////////////////////////////////////////////////////


void set_do_dump(const StringArray &)

{

do_dump = true;

return;

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name << " config_file [ config_file2 ... ] : function_name arg1 [arg2 ...]\n\n";

exit ( 1 );


return;

}


////////////////////////////////////////////////////////////////////////


