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

#include "vx_log.h"
#include "vx_math.h"

#include "vx_python3_utils.h"
#include "python_handler.h"


////////////////////////////////////////////////////////////////////////


// static const char python_wrapper [] = "a2nc_python.py";
// static const char pickle_wrapper [] = "a2nc_pickle.py";
static const char python_wrapper [] = "a2nc_python";
static const char pickle_wrapper [] = "a2nc_pickle";


static const char list_name [] = "point_data";


////////////////////////////////////////////////////////////////////////


static const char sq = '\'';   //  single quote


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PythonHandler
   //


////////////////////////////////////////////////////////////////////////


PythonHandler::PythonHandler(const string &program_name) : FileHandler(program_name)

{



}


////////////////////////////////////////////////////////////////////////


PythonHandler::PythonHandler(const char * program_name, const char * ascii_filename) : FileHandler(program_name)

{

int j;
ConcatString s = ascii_filename;
StringArray a = s.split(" ");

user_script_filename = a[0];

for (j=1; j<(a.n()); ++j)  {   //  j starts at one here, not zero

   user_script_args.add(a[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


PythonHandler::~PythonHandler()

{


}


////////////////////////////////////////////////////////////////////////


bool PythonHandler::isFileType(LineDataFile &ascii_file) const

{

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool PythonHandler::_readObservations(LineDataFile &ascii_file)

{

mlog << Error
     << "\n\n  bool PythonHandler::_readObservations(LineDataFile &) -> should never be called!\n\n";

exit ( 1 );


return ( false );

}


////////////////////////////////////////////////////////////////////////


void PythonHandler::load_python_obs(PyObject * obj)

{

if ( ! PyList_Check(obj) )  {

   mlog << Error
        << "\n\n  PythonHandler::load_python_obs(PyObject *) -> given object not a list!\n\n";

   exit ( 1 );

}

int j;
PyObject * a = 0;
Python3_List list(obj);
Observation obs;

for (j=0; j<(list.size()); ++j)  {

   a = list[j];

   if ( ! PyList_Check(obj) )  {

      mlog << Error
           << "\n\n  PythonHandler::load_python_obs(PyObject *) -> non-list object found in main list!\n\n";

      exit ( 1 );

   }

   obs.set(a);

   _addObservations(obs);

}   //  for j



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool PythonHandler::readAsciiFiles(const vector< ConcatString > &)

{

int j;
ConcatString command;
ConcatString short_user_name;


short_user_name = user_script_filename;

short_user_name.chomp(".py");

   //
   //  start up the python interpreter
   //

Python3_Script script(python_wrapper);

   //
   //  set up a "new" sys.argv list
   //     with the command-line arquments for
   //     the user's script
   //

if ( user_script_args.n() > 0 )  {

   command << cs_erase
           << "sys.argv = [ ";

   command << sq << short_user_name << sq << ", ";

   for (j=0; j<(user_script_args.n()); ++j)  {

      command << sq << user_script_args[j] << sq;

      if ( j < (user_script_args.n() - 1) )  command << ',';

      command << ' ';

   }

   command << ']';

   // cout << "command = \"" << command << "\"\n" << flush;

   script.run(command);

}

   //
   //  import the user's script as a module
   //

PyObject * m = PyImport_Import(PyUnicode_FromString(short_user_name.text()));

   //
   //  get the dictionary (ie, namespace)
   //    for the module
   //

Python3_Dict md (m);

   //
   //  lookup the variable containing the
   //    list of obs
   //

PyObject * obj = md.lookup_list(list_name);

// cout << "obj = \"" << obj << "\"\n" << flush;

   //
   //  load the obs
   //

load_python_obs(obj);

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


