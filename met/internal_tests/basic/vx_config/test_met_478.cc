// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
#include "config_constants.h"
#include "threshold.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);


MetConfig config;

   //
   //  redirect stderr to stdout
   //

if ( dup2(1, 2) < 0 )  {

   cerr << "\n\n  " << program_name << ": dup2 failed\n\n";

   exit ( 1 );

}

config.set_debug(debug);

   //
   //
   //

config.read_string("name=\"APCP\"; level=\"(*,*)\";");
config.read(replace_path(config_const_filename).c_str()); 

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




