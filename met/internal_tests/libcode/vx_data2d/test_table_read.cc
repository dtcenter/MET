// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

#include "table_lookup.h"
#include "vx_util.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc == 1 )  {

   cerr << "\n\n  usage:  " << program_name << " flat_file_list\n\n";

   exit ( 1 );

}

int j;
TableFlatFile f;
bool status = false;
const char * input_filename = (const char *) 0;

for (j=1; j<argc; ++j)  {   //  j starts at one, here

   input_filename = argv[j];

   status = f.read(input_filename);

   if ( !status )  {

      cerr << "\n\n  " << program_name << ":  trouble reading flat file \"" << input_filename << "\"\n\n";

      exit ( 1 );

   }

}   //  for j

f.dump(cout);


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


