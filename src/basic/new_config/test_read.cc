

////////////////////////////////////////////////////////////////////////


static const int debug = 1;


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

////////////////////////////////////////////////////////////////////////


extern int configparse();

extern FILE * configin;

extern int configdebug;

extern const char * bison_input_filename;


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

if ( argc != 2 )  {

   cerr << "\n\n  usage: " << argv[0] << " infile\n\n";

   exit ( 1 );

}

bison_input_filename = argv[1];

   //
   //  redirect stderr to stdout
   //

if ( dup2(1, 2) < 0 )  {

   cerr << "\n\n  " << argv[0] << ": dup2 failed\n\n";

   exit ( 1 );

}




configdebug = debug;

if ( (configin = fopen(bison_input_filename, "r")) == NULL )  {

   cerr << "\n\n  " << argv[0] << " -> unable to open input file \"" << bison_input_filename << "\"\n\n";

   exit ( 1 );

}

int parse_status;

parse_status = configparse();

if ( parse_status != 0 )  {

   cerr << "\n\n  " << argv[0] << " -> parse status is nonzero! ... = " << parse_status << "\n\n";

   exit ( 1 );

}

   //
   //  if we get to this point, parse must've been successfull
   //

cout << "\n\n  Parse was successfull!\n\n";

   //
   //  done
   //

fclose(configin);   configin = (FILE *) 0;

return ( 0 );

}

////////////////////////////////////////////////////////////////////////


