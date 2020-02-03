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
#include <stdlib.h>

#include "vx_log.h"
#include "vx_math.h"

#include "vx_python3_utils.h"
#include "python_handler.h"


////////////////////////////////////////////////////////////////////////


static const char generic_python_wrapper [] = "generic_python";
static const char generic_pickle_wrapper [] = "generic_pickle";

static const char write_pickle_wrapper   [] = "MET_BASE/wrappers/point_write_pickle.py";

static const char list_name              [] = "point_data";

static const char pickle_output_filename [] = "out.pickle";


////////////////////////////////////////////////////////////////////////


static const char sq = '\'';   //  single quote


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PythonHandler
   //


////////////////////////////////////////////////////////////////////////


PythonHandler::PythonHandler(const string &program_name) : FileHandler(program_name)

{

use_pickle = false;

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

use_pickle = false;

const char * c = getenv(user_python_path_env);

if ( c )  {

   use_pickle = true;

   user_path_to_python = c;

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


bool PythonHandler::readAsciiFiles(const vector< ConcatString > &ascii_filename_list)

{

bool status = false;

if ( use_pickle )  status = do_pickle   ();
else               status = do_straight ();

return ( status );

}


////////////////////////////////////////////////////////////////////////


bool PythonHandler::do_straight()

{

ConcatString command;
ConcatString short_user_name;
ConcatString path;


// path << cs_erase
//      << mbb << '/'
//      << wrappers_dir << '/'
//      << generic_python_wrapper;

// path << ".py";


path = generic_python_wrapper;

short_user_name = user_script_filename;

short_user_name.chomp(".py");

   //
   //  start up the python interpreter
   //

Python3_Script script(path.text());

// cout << "\n\n  do_straight() -> python_wrapper = \"" << script.filename() << "\"\n\n" << flush;

   //
   //  set up a "new" sys.argv list
   //     with the command-line arquments for
   //     the user's script
   //

if ( user_script_args.n() > 0 )  {

   script.reset_argv(short_user_name.text(), user_script_args);

}

   //
   //  import the user's script as a module
   //

PyObject * m = PyImport_Import(PyUnicode_FromString(short_user_name.text()));

if ( PyErr_Occurred() )  {

   PyErr_Print();

   mlog << Warning << "\nPythonHandler::do_straight() -> "
        << "an error occurred importing module "
        << '\"' << short_user_name.text() << "\"\n\n";

   return ( false );

}

   //
   //  get the dictionary (ie, namespace)
   //    for the module
   //

Python3_Dict md (m);

   //
   //  get the variable containing the
   //    list of obs
   //

PyObject * obj = md.lookup_list(list_name);

// cout << "obj = \"" << obj << "\"\n" << flush;

   //
   //  load the obs
   //

load_python_obs(obj);


return ( true );

}


////////////////////////////////////////////////////////////////////////


   //
   //  wrapper usage:  /path/to/python wrapper.py pickle_output_filename user_script_name [ user_script args ... ]
   //

bool PythonHandler::do_pickle()

{

int j;
const int N = user_script_args.n();
ConcatString command;
int status;


command << cs_erase
        << user_path_to_python    << ' '
        << replace_path(write_pickle_wrapper) << ' '
        << pickle_output_filename << ' '
        << user_script_filename;

for (j=0; j<N; ++j)  {

   command << ' ' << user_script_args[j];

};


// cout << "\n\n  PythonHandler::do_pickle() -> command = \"" << command << "\"\n\n" << flush;

// exit ( 1 );

status = system(command.text());

if ( status )  {

   mlog << Error
        << "\n\n  PythonHandler::do_pickle() -> command \"" << command.text() << "\" failed ... status = " << status << "\n\n";

   exit ( 1 );


}

ConcatString generic;

// generic << cs_erase << mbb << '/' << wrappers_dir << '/' << generic_pickle_wrapper << ".py";

generic = generic_pickle_wrapper;

Python3_Script script(generic.text());

script.read_pickle(list_name, pickle_output_filename);

PyObject * obj = script.lookup(list_name);

// cout << "\n\n  PythonHandler::do_pickle() -> obj = " << obj << "\n\n" << flush;

if ( ! PyList_Check(obj) )  {

   mlog << Error
        << "\n\n  PythonHandler::do_pickle() -> pickle object is not a list!\n\n";

   exit ( 1 );

}

load_python_obs(obj);

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


