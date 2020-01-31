

////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

#include "vx_log.h"
#include "empty_string.h"

#include "python3_util.h"
#include "python3_script.h"

#include "global_python.h"


////////////////////////////////////////////////////////////////////////


static const char sq = '\'';   //  single quote


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Python3_Script
   //


////////////////////////////////////////////////////////////////////////


Python3_Script::Python3_Script()

{

mlog << Error << "\nPython3_Script::Python3_Script() -> "
     << "should never be called!\n\n";

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

GP.initialize();

   //
   //   import the python script as a module
   //

ConcatString path = _script_filename;


path.chomp(".py");

Module = PyImport_ImportModule (path.text());

if ( ! Module )  {

   mlog << Error << "\nPython3_Script::Python3_Script(const char *) -> "
        << "unable to open script \"" << path << "\"\n\n";

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

   mlog << Error << "\nPython3_Script::run(const char *) -> "
        << "empty command!\n\n";

   exit ( 1 );

}

if ( PyRun_String(command, Py_file_input, Dict, Dict) < 0 )  {

   mlog << Error << "\nPython3_Script::run(const char *) -> "
        << "command \"" << command << "\" failed!\n\n";

   exit ( 1 );

}

fflush(stdout);
fflush(stderr);

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  example:
   //
   //      data = pickle.load( open( "save.p", "rb" ) )
   //


void Python3_Script::read_pickle(const char * variable, const char * pickle_filename) const

{

ConcatString command;

command << variable 
        << " = pickle.load(open(\""
        << pickle_filename
        << "\", \"rb\"))";

PyErr_Clear();

run(command.text());

if ( PyErr_Occurred() )  {

   mlog << Error << "\nPython3_Script::read_pickle() -> "
        << "command \"" << command << "\" failed!\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void Python3_Script::reset_argv(const char * script_name, const StringArray & args)

{

int j;
ConcatString command;
ConcatString module_name = script_name;
const int N = args.n();


module_name.chomp(".py");

command = "sys.argv = [ ";

command << sq << module_name << sq << ", ";

for (j=0; j<N; ++j)  {

   command << sq << args[j] << sq;

   if ( j < (N - 1) )  command << ',';

   command << ' ';

}

command << ']';

run(command.text());

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////

