

////////////////////////////////////////////////////////////////////////


static const int debug  = 1;


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "machine.h"


////////////////////////////////////////////////////////////////////////


   //
   //  declarations with external linkage
   //


extern int econfigdebug;


   //
   //  definitions with external linkage
   //


   //
   //  static definitions
   //


static Machine machine;

static const char * input_filename = (const char *) 0;


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv[])

{

int j;

j = strlen(argv[0]) - 1;

while ( (j >= 0) && (argv[0][j] != '/') )  --j;

++j;

const char * program_name = argv[0] + j;

if ( argc != 2 )  {

   cerr << "\n\n  usage:  " << program_name << " filename\n\n";

   exit ( 1 );

}


input_filename = argv[1];


   //
   //  redirect stderr to stdout
   //

if ( dup2(1, 2) < 0 )  {

   cerr << "\n\n  " << program_name << ": dup2 failed\n\n";

   exit ( 1 );

}

   //
   //  turn on debugging
   //

econfigdebug = debug;

   //
   //  read, write ...
   //

cout << "\n\n";

machine.read(input_filename);

cout << "\n\n";

machine.st_dump(cout);

cout << "\n\n";

   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////





