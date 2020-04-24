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

static const char write_pickle_wrapper   [] = "MET_BASE/wrappers/write_pickle_point.py";

static const char list_name              [] = "point_data";

static const char pickle_base_name       [] = "tmp_ascii2nc_pickle";


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

mlog << Error << "\nbool PythonHandler::_readObservations(LineDataFile &) -> "
     << "should never be called!\n\n";

exit ( 1 );


return ( false );

}


////////////////////////////////////////////////////////////////////////


void PythonHandler::load_python_obs(PyObject * obj)

{

if ( ! PyList_Check(obj) )  {

   mlog << Error << "\nPythonHandler::load_python_obs(PyObject *) -> "
        << "given object not a list!\n\n";

   exit ( 1 );

}

int j;
PyObject * a = 0;
Python3_List list(obj);
Observation obs;

   //
   //  initialize use_var_id to false
   //

use_var_id = false;

for (j=0; j<(list.size()); ++j)  {

   a = list[j];

   if ( ! PyList_Check(a) )  {

      mlog << Error << "\nPythonHandler::load_python_obs(PyObject *) -> "
           << "non-list object found in main list!\n\n";

      exit ( 1 );

   }

   obs.set(a);

   //
   //  set the observation variable code
   //

   if ( use_var_id || !is_number(obs.getVarName().c_str()) )  {

      use_var_id = true;

      int var_index;

      //  update the list of variable names

      if ( !obs_names.has(obs.getVarName(), var_index) )  {
         obs_names.add(obs.getVarName());
         obs_names.has(obs.getVarName(), var_index);
      }

      obs.setVarCode(var_index);

   }
   else  {

      obs.setVarCode(atoi(obs.getVarName().c_str()));

   }

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

ConcatString command, path, user_base;

path = generic_python_wrapper;

mlog << Debug(3)
     << "Running user's python script ("
     << user_script_filename << ").\n";

user_base = user_script_filename.basename();

user_base.chomp(".py");

   //
   //  start up the python interpreter
   //

Python3_Script script(path.text());

   //
   //  set up a "new" sys.argv list
   //     with the command-line arquments for
   //     the user's script
   //

if ( user_script_args.n() > 0 )  {

   script.reset_argv(user_script_filename.text(), user_script_args);

}

   //
   //  import the user's script as a module
   //

PyObject * m = PyImport_Import(PyUnicode_FromString(user_base.text()));

if ( PyErr_Occurred() )  {

   PyErr_Print();

   mlog << Warning << "\nPythonHandler::do_straight() -> "
        << "an error occurred importing module "
        << '\"' << user_base.text() << "\"\n\n";

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
ConcatString path;
ConcatString pickle_path;
const char * tmp_dir = 0;
int status;

mlog << Debug(3) << "Calling " << user_path_to_python
     << " to run user's python script (" << user_script_filename
     << ").\n";

tmp_dir = getenv ("MET_TMP_DIR");

if ( ! tmp_dir )  tmp_dir = default_tmp_dir;

path << cs_erase
     << tmp_dir << '/'
     << pickle_base_name;

pickle_path = make_temp_file_name(path.text(), 0);

command << cs_erase
        << user_path_to_python                << ' '    //  user's path to python
        << replace_path(write_pickle_wrapper) << ' '    //  write_pickle.py
        << pickle_path                        << ' '    //  pickle output filename
        << user_script_filename;                        //  user's script name

for (j=0; j<N; ++j)  {

   command << ' ' << user_script_args[j];

};

status = system(command.text());

if ( status )  {

   mlog << Error << "\nPythonHandler::do_pickle() -> "
        << "command \"" << command.text() << "\" failed ... status = "
        << status << "\n\n";

   exit ( 1 );

}

ConcatString wrapper;

wrapper = generic_pickle_wrapper;

Python3_Script script(wrapper.text());

script.read_pickle(list_name, pickle_path.text());

PyObject * obj = script.lookup(list_name);

if ( ! PyList_Check(obj) )  {

   mlog << Error << "\nPythonHandler::do_pickle() -> "
        << "pickle object is not a list!\n\n";

   exit ( 1 );

}

load_python_obs(obj);

   //
   //  cleanup
   //

remove_temp_file(pickle_path);

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////
