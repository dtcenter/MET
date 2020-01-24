

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

mlog << Error
     << "\n\n  Python3_Script::Python3_Script() -> should never be called!\n\n";

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

// wcout << "getpath = \"" << Py_GetPath() << "\"\n\n" << flush;

// ConcatString a;
// a << ".:/usr/local/anaconda3-20190923/lib/python37.zip:/usr/local/anaconda3-20190923/lib/python3.7:/usr/local/anaconda3-20190923/lib/python3.7/lib-dynload:" << getenv("MET_BUILD_BASE") << '/' << "data/wrappers";
// cout << "\n\n  a = \"" << a << "\"\n\n" << flush;

// exit ( 1 );
// Py_SetPath(Py_DecodeLocale(a.text(), 0));


// Py_Initialize();
// 
// setup_python_path();

GP.initialize();

   //
   //   import the python script as a module
   //

ConcatString path = _script_filename;


path.chomp(".py");

Module = PyImport_ImportModule (path.text());

// PyErr_Print();


if ( ! Module )  {

   mlog << Error
        << "\n\n  Python3_Script::Python3_Script(const char *) -> unable to open script \"" << path << "\"\n\n";

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

   mlog << Error
        << "\n\n   Python3_Script::run(const char *) -> empty command!\n\n";

   exit ( 1 );

}

if ( PyRun_String(command, Py_file_input, Dict, Dict) < 0 )  {

   mlog << Error
        << "\n\n   Python3_Script::run(const char *) -> command \""
        << command << "\" failed!\n\n";

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
   //       pickle.dump( data, open( "save.p", "wb" ) )
   //


void Python3_Script::write_pickle(const char * variable, const char * pickle_filename) const

{

ConcatString command;

command << "pickle.dump( "
        << variable
        << ", open( \""
        << pickle_filename
        << "\", \"wb\" ) )";


// cout << "\n\n  write_pickle() -> command = \"" << command << "\"\n\n";

// run(command);


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



// cout << "\n\n  read_pickle() -> command = \"" << command << "\"\n\n";

PyErr_Clear();

run(command.text());

if ( PyErr_Occurred() )  {

   mlog << Error
        << "\n\n  Python3_Script::read_pickle() -> command \""
        << command << "\" failed!\n\n";

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

// cout << "command = \"" << command << "\"\n" << flush;



run(command.text());

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////




