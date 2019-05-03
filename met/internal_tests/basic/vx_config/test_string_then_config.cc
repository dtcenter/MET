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
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_util.h"

#include "config_file.h"
#include "threshold.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc != 3 )  {

   cerr << "\n\n  usage: " << program_name << " config_string config_filename\n\n";

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

bool status = false;
ConcatString config_string   = (string)argv[1];
ConcatString config_filename = (string)argv[2];

   //
   //  read the string
   //


status = config.read_string(config_string.c_str());

if ( ! status )  {

   cerr << "\n\n  " << program_name << ": failed to parse string \"" << config_string << "\"\n\n";

   exit ( 1 );

}


   //
   //  read the file
   //

status = config.read(config_filename.c_str());

if ( ! status )  {

   cerr << "\n\n  " << program_name << ": failed to parse file \"" << config_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  dump
   //

config.dump(cout);

   //
   //  done
   //

return ( 0 );

}

////////////////////////////////////////////////////////////////////////




