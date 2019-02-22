// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


static const bool debug = true;


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_util.h"

#include "config_file.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc == 1 )  {

   cerr << "\n\n  usage: " << program_name << " config_file_list\n\n";

   exit ( 1 );

}

MetConfig config;

   //
   //  redirect stderr to stdout
   //

if ( dup2(1, 2) < 0 )  {

   cerr << "\n\n  " << program_name << ": dup2 failed\n\n";

   exit ( 1 );

}

config.set_debug(debug);

int j;
bool status = false;

for (j=1; j<argc; ++j)  {   //  j starts at one, here

   status = config.read(argv[j]);

   if ( ! status )  {

      cerr << "\n\n  " << program_name << ": failed to parse config file \"" << argv[j] << "\"\n\n";

      exit ( 1 );

   }

}

   //
   //  dump the contents
   //

cout << "\n\n";

config.dump(cout);

cout << "\n\n";


/*
config.dump_config_format(cout);

cout << "\n\n";
*/
   //
   //  done
   //

return ( 0 );

}

////////////////////////////////////////////////////////////////////////


