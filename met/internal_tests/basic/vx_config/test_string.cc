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


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;


////////////////////////////////////////////////////////////////////////


static void read_file_into_string(const char * filename, ConcatString & out);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc == 1 )  {

   cerr << "\n\n  usage: " << program_name << " config_file\n\n";

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
ConcatString s;


read_file_into_string(argv[1], s);

status = config.read_string(s.c_str());

if ( ! status )  {

   cerr << "\n\n  " << program_name << ": failed to parse config file \"" << argv[0] << "\" as string\n\n";

   exit ( 1 );

}

s = "b = 10;";

status = config.read_string(s.c_str());

if ( ! status )  {

   cerr << "\n\n  " << program_name << ": failed to parse string \"" << s << "\"\n\n";

   exit ( 1 );

}

   //
   //  dump the contents
   //

cout << "\n\n";

config.dump(cout);

cout << "\n\n";

config.dump_config_format(cout);

cout << "\n\n";

   //
   //  done
   //

return ( 0 );

}

////////////////////////////////////////////////////////////////////////


void read_file_into_string(const char * filename, ConcatString & out)

{

ifstream in;
char c;

met_open(in, filename);

if ( !in )  {

   cerr << "\n\n  " << program_name << ": read_file_into_string() -> unable to open input file \""
        << filename << "\"\n\n";

   exit ( 1 );

}

while ( in.get(c) )  out << c;

   //
   //  done
   //

in.close();

return;

}


////////////////////////////////////////////////////////////////////////


