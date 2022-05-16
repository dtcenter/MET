

////////////////////////////////////////////////////////////////////////


using namespace std;


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <cstdio>

#include "concat_string.h"

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

extern "C" {

#include "Python.h"

}

#include "vx_util.h"

#include "python3_util.h"
#include "python3_script.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;


////////////////////////////////////////////////////////////////////////


static void usage();


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc != 3 )  usage();

ConcatString   script_name = argv[1];
ConcatString variable_name = argv[2];

script_name.chomp(".py");

Python3_Script script(script_name);

PyObject * var = 0;


var = script.lookup(variable_name);

if ( var )  {

   cout << "\n\n   " << program_name << ":  " << variable_name << " = " << var << "\n\n";

} else {

   cerr << "\n\n  " << program_name << ": variable \""
        << variable_name << "\" not found in script\n\n";

   exit ( 1 );

}

// script.run("b = data[1][2]");
// 
// cout << "b = " << script.lookup("b") << "\n\n";



   //
   //  done, finish up
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name << " script_name variable_name\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////






