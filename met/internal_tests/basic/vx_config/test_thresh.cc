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
#include "threshold.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;


////////////////////////////////////////////////////////////////////////


extern ThreshNode * result;

extern bool test_mode;


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc == 1 )  {

   cerr << "\n\n  usage: " << program_name << " thresh_string value\n\n";

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
ConcatString local_thresh_string = (string)argv[1];
const double value = atof(argv[2]);
SingleThresh st;

test_mode = true;

status = config.read_string(local_thresh_string.c_str());

if ( ! status )  {

   cerr << "\n\n  " << program_name << ": failed to parse string \"" << local_thresh_string << "\"\n\n";

   exit ( 1 );

}

test_mode = false;

if ( ! result )  {

   cerr << "\n\n  " << program_name << ": no result set!\n\n";

   exit ( 1 );

}

st.set(result);

// status = result->check(value);

status = st.check(value);

cout << "\n\n" 
     << "Evaluates as:  " << (status ? "true" : "false") << "\n\n\n";

cout << "thresh_string = \"" << result->s.contents() << "\"\n\n";
cout << "  abbr_string = \"" << result->abbr_s.contents() << "\"\n\n";
cout << "        type  = " << result->type() << "\n\n";

   //
   //  done
   //

if ( result )  { delete result;  result = 0; }

return ( 0 );

}

////////////////////////////////////////////////////////////////////////




