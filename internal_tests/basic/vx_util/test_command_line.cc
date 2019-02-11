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
#include <cmath>

#include "vx_util.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static void set_xyz     (const StringArray &);
static void set_verbose (const StringArray &);

static void usage();


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

CommandLine c;

c.set(argc, argv);

c.add(set_xyz,     "-xyz",     3);
c.add(set_verbose, "-verbose", 0);

c.set_usage(usage);

// c.set_allow_numbers(true);

// mlog << Debug(1) << "\n\n";



// mlog << Debug(1) << "After setup ... \n\n";
// 
// c.dump(cout);
// 
// mlog << Debug(1) << "\n\n";



// mlog << Debug(1) << "Parsing ... \n\n";

c.parse();

if ( c.n() == 0 )  usage();

// mlog << Debug(1) << "\n\n";



// mlog << Debug(1) << "Command-line Arguments left after parsing... \n\n";
// 
// c.dump(cout);
// 
// mlog << Debug(1) << "\n\n";


return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void set_xyz(const StringArray & a)

{

mlog << Debug(1) << "\n\n  void set_xyz(const StringArray &) called with these arguments:\n\n";

a.dump(cout);

mlog << Debug(1) << "\n\n";

return;

}


////////////////////////////////////////////////////////////////////////


void set_verbose(const StringArray & a)

{

mlog << Debug(1) << "\n\n  void set_verbose(const StringArray &) called with these arguments:\n\n";

a.dump(cout);

mlog << Debug(1) << "\n\n";

return;

}


////////////////////////////////////////////////////////////////////////


void usage()

{

mlog << Error << "\nusage:  " << program_name << " [ -xyz x y z ] [ -verbose ] file_list\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


