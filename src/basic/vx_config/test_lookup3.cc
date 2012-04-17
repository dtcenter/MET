

////////////////////////////////////////////////////////////////////////


static const bool debug = false;

static const char input_filename [] = "tt";


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


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);


   //
   //  redirect stderr to stdout
   //

if ( dup2(1, 2) < 0 )  {

   cerr << "\n\n  " << program_name << ": dup2 failed\n\n";

   exit ( 1 );

}

config.set_debug(debug);

bool status = false;

status = config.read(input_filename);

if ( ! status )  {

   cerr << "\n\n  " << program_name << ": failed to parse config file \"" << input_filename << "\"\n\n";

   exit ( 1 );

}

// config.dump(cout);

   //
   //  test
   //

const DictionaryEntry * e = config.lookup("fcst.field");
Dictionary * f = e->array_value();

const DictionaryEntry * ee = f->operator[](1);
Dictionary * ff = ee->dict_value();


const DictionaryEntry * eee = ff->lookup("level");

eee->dump(cout);


cout << "\n\n";

   //
   //  done
   //

return ( 0 );

}

////////////////////////////////////////////////////////////////////////

