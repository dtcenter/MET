

////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

#include "empty_string.h"

#include "python3_script.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Python3_Script
   //


////////////////////////////////////////////////////////////////////////


Python3_Script::Python3_Script()

{

cerr << "\n\n  Python3_Script::Python3_Script() -> should never be called!\n\n";

exit ( 1 );


}


////////////////////////////////////////////////////////////////////////


Python3_Script::~Python3_Script()

{

clear();

Py_Finalize();

}


////////////////////////////////////////////////////////////////////////


void Python3_Script::clear()

{

Module = 0;

Dict = 0;

Script_Filename.clear();


return;

}


////////////////////////////////////////////////////////////////////////


Python3_Script::Python3_Script(const char * _script_filename)

{

clear();

fflush(stdout);
fflush(stderr);

   //
   //   start up the python interpreter
   //

Py_Initialize();

   //
   //   import the python script as a module
   //

ConcatString path = _script_filename;

path.chomp(".py");

Module = PyImport_ImportModule (path);

// PyErr_Print();

if ( ! Module )  {

   cerr << "\n\n  Python3_Script::Python3_Script(const char *) -> unable to run script \"" << path << "\"\n\n";

   Py_Finalize();

   exit ( 1 );

}

Script_Filename = path;

   //
   //   get the namespace for the module (as a dictionary)
   //

Dict = PyModule_GetDict (Module);





   //
   //  done
   //

fflush(stdout);
fflush(stderr);

}


////////////////////////////////////////////////////////////////////////


PyObject * Python3_Script::lookup(const char * name) const

{

PyObject * var = 0;

var = PyDict_GetItemString (Dict, name);

return ( var );

}


////////////////////////////////////////////////////////////////////////


void Python3_Script::run(const char * command) const

{

if ( empty(command) )  {

   cerr << "\n\n   Python3_Script::run(const char *) -> empty command!\n\n";

   exit ( 1 );

}

if ( PyRun_String(command, Py_file_input, Dict, Dict) < 0 )  {

   cerr << "\n\n   Python3_Script::run(const char *) -> command \""
        << command << "\" failed!\n\n";

   exit ( 1 );

}


fflush(stdout);
fflush(stderr);

return;

}


////////////////////////////////////////////////////////////////////////




