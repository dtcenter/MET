

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cmath>

#include "vx_util.h"

#include "tokenizer.h"
#include "token_stack.h"
#include "make_program.h"
#include "bool_calc.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;


////////////////////////////////////////////////////////////////////////


static void usage();


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc <= 2 )  usage();



int j;
ifstream in;
Program program;
ConcatString input;


in.open(argv[1]);

if ( ! in )  {

   cerr << "\n\n  " << program_name << ": trouble opening input file \""
        << argv[1] << "\"\n\n";

   exit ( 1 );

}

input.read_line(in);

input.chomp('\n');

input.ws_strip();

in.close();


cout << "\n\n   input = \"" << input << "\"\n\n" << flush;

const int n_args = argc - 2;
vector<bool> a;
bool tf = false;
BoolCalc c;


c.set(input);


if ( n_args < (c.Max_local) )  {

   cerr << "\n\n  too few arguments!\n\n";

   exit ( 1 );

}

if ( n_args > (c.Max_local) )  {

   cerr << "\n\n  too many arguments!\n\n";

   exit ( 1 );

}


for (j=0; j<n_args; ++j)  {

   tf = false;

   if ( strcmp(argv[j + 2], "1") == 0 )  tf = true;   

   a.push_back(tf);

}


// c.dump_program(cout);

bool result = c.run(a);

cout << "\n\n   result is " << ( result ? 1 : 0 ) << "\n\n";


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name << " file arg1 arg2 ... \n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////



