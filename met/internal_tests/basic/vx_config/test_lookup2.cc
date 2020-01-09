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
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_util.h"

#include "config_file.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static MetConfig config;


////////////////////////////////////////////////////////////////////////


static void test_lookup(const char * name);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc == 1 )  {

   cerr << "\n\n  usage: " << program_name << " config_file_list\n\n";

   exit ( 1 );

}


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
   //  test
   //

test_lookup("top");
test_lookup("top.mid");
test_lookup("top.mid.there");
test_lookup("top.mid.hello");

test_lookup("top.mid.bot");
test_lookup("top.mid.bot.dolly");
test_lookup("top.mid.bot.there");
test_lookup("top.mid.bot.hello");







cout << "\n\n";

   //
   //  done
   //

return ( 0 );

}

////////////////////////////////////////////////////////////////////////


void test_lookup(const char * name)

{

const DictionaryEntry * e = (const DictionaryEntry *) 0;

e = config.lookup(name);

if ( !e )  {

   cout << "\n\n  " << program_name << ": lookup test for \"" << name << "\" failed!\n\n";

   exit ( 1 );

}

   //
   //  success
   //

cout << "\n\n  " << program_name << ": lookup test for \"" << name << "\" succeeded.\n\n";

e->dump(cout);

cout << "\n\n";

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


