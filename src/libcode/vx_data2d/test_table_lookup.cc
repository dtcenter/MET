

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "table_lookup.h"
#include "vx_util.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc != 2 )  {

   cerr << "\n\n  usage:  " << program_name << " flat_file\n\n";

   exit ( 1 );

}

ConcatString input_filename = argv[1];

TableFlatFile f;
bool status = false;

status = f.read(input_filename);

if ( !status )  {

   cerr << "\n\n  " << program_name << ":  trouble reading flat file \"" << input_filename << "\"\n\n";

   exit ( 1 );

}

f.dump(cout);


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


