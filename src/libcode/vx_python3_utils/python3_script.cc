// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


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

using namespace std;


////////////////////////////////////////////////////////////////////////


static const char sq = '\'';   //  single quote

static const char read_tmp_ascii_py [] = "MET_BASE/python/pyembed/read_tmp_ascii.py";


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

ModuleAscii = 0;

Dict = 0;

DictAscii = 0;

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

   PyErr_Print();
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


PyObject * Python3_Script::lookup_ascii(const char * name) const

{

PyObject * var = 0;

var = PyDict_GetItemString (DictAscii, name);

if ( ! var )  {

   mlog << Error << "\nPython3_Script::lookup_ascii(const char * name) -> "
        << "value for name \"" << name << "\" not found\n\n";


   exit ( 1 );

}

if ( ! PyList_Check(var) )  {

   mlog << Error << "\nPython3_Script::lookup_ascii(const char * name) -> "
        << "value for name \"" << name << "\" not a python list\n\n";

   exit ( 1 );

}

return ( var );

}


////////////////////////////////////////////////////////////////////////


PyObject * Python3_Script::run(const char * command) const

{

PyObject * pobj;

if ( empty(command) )  {

   mlog << Error << "\nPython3_Script::run(const char *) -> "
        << "empty command!\n\n";

   exit ( 1 );

}

pobj = PyRun_String(command, Py_file_input, Dict, Dict);

if ( ! pobj )  {

   mlog << Error << "\nPython3_Script::run(const char *) -> "
        << "command \"" << command << "\" failed!\n\n";

   exit ( 1 );

}

fflush(stdout);
fflush(stderr);

return pobj;

}


////////////////////////////////////////////////////////////////////////


void Python3_Script::import_read_tmp_ascii_py(void)

{

ConcatString py_module;

py_module << cs_erase
          << replace_path(read_tmp_ascii_py);

ConcatString command;

run_python_string("import sys");

command << cs_erase
        << "sys.path.append(\""
        << py_module.dirname().c_str()
        << "\")";

mlog << Debug(3) << command << "\n";

run_python_string(command.text());

mlog << Debug(2) << "Importing " << py_module << "\n";

ConcatString path = "read_tmp_ascii";

ModuleAscii = PyImport_ImportModule(path.text());

if ( ! ModuleAscii )  {

   PyErr_Print();
   mlog << Error << "\nPython3_Script::Python3_Script(const char *) -> "
        << "unable to import module \"" << path << "\"\n\n";

   Py_Finalize();

   exit ( 1 );

}

DictAscii = PyModule_GetDict(ModuleAscii);

   //
   //  done
   //

fflush(stdout);
fflush(stderr);

}


////////////////////////////////////////////////////////////////////////


PyObject* Python3_Script::read_tmp_ascii(const char * tmp_filename) const

{

mlog << Debug(2) << "Reading temporary ascii file: "
     << tmp_filename << "\n";

ConcatString command;

command << "read_tmp_ascii(\""
        << tmp_filename
        << "\")";

PyErr_Clear();

PyObject * pobj;

pobj = PyRun_String(command.text(), Py_file_input, DictAscii, DictAscii);

if ( PyErr_Occurred() )  {

   mlog << Error << "\nPython3_Script::read_tmp_ascii() -> "
        << "command \"" << command << "\" failed!\n\n";

   exit ( 1 );
}

return pobj;

}


////////////////////////////////////////////////////////////////////////


void Python3_Script::reset_argv(const char * script_name, const StringArray & args)

{

int j;
ConcatString cs, command;
const int N = args.n();

  //
  //  add the script directory to the system path
  //

cs = script_name;

run_python_string("import sys");

command << cs_erase
        << "sys.path.append(\""
        << cs.dirname().c_str()
        << "\")";

run_python_string(command.text());

  //
  //  set argv
  //

command = "sys.argv = [ ";

command << sq << script_name << sq << ", ";

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

