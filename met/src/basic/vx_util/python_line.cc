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
#include <string.h>

#include "vx_log.h"
#include "vx_math.h"
#include "stat_column_defs.h"
#include "util_constants.h"
#include "vx_python3_utils.h"
#include "python3_util.h"
#include "global_python.h"
#include "temp_file.h"

#include "python_line.h"


////////////////////////////////////////////////////////////////////////


static const char generic_python_wrapper [] = "generic_python";
static const char generic_pickle_wrapper [] = "generic_pickle";

static const char write_pickle_wrapper   [] = "MET_BASE/wrappers/write_pickle_mpr.py";

static const char list_name              [] = "mpr_data";

static const char pickle_base_name       [] = "tmp_mpr_pickle";

static const char line_type              [] = "MPR";


////////////////////////////////////////////////////////////////////////


static void get_cs     (PyObject * in, ConcatString & out);

static void get_int    (PyObject * in, int & out);

static void get_double (PyObject * in, double & out);

static bool is_na      (PyObject *);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PyLineDataFile
   //


////////////////////////////////////////////////////////////////////////


PyLineDataFile::PyLineDataFile()

{

script = 0;

close();

}


////////////////////////////////////////////////////////////////////////


PyLineDataFile::~PyLineDataFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


void PyLineDataFile::close()

{

first_call = false;

index = -1;

N = -1;

main_list = 0;   //  don't delete

UserScriptPath.clear();

UserScriptArgs.clear();

UserPathToPython.clear();

if ( script ) { delete script;  script = 0; }


return;

}


////////////////////////////////////////////////////////////////////////


bool PyLineDataFile::open(const char * user_script_path, const StringArray & user_script_args)

{

close();

first_call = true;

UserScriptPath = user_script_path;

UserScriptArgs = user_script_args;

const char * c = getenv (user_python_path_env);

GP.initialize();

if ( c )  {

   UserPathToPython = c;

   do_pickle();

} else {

   do_straight ();

}

   //
   //   check that the python object we got is actually a list
   //

if ( ! PyList_Check(main_list) )  {

   mlog << Error << "\nPyLineDataFile::open() -> "
        << "pickle object is not a list!\n\n";

   exit ( 1 );

}

   //
   //   how many things in the list?
   //

N = PyList_Size(main_list);

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


ConcatString PyLineDataFile::make_header_line() const

{

ConcatString line;

line << "VERSION"        << ' ';
line << "MODEL"          << ' ';
line << "DESC"           << ' ';
line << "FCST_LEAD"      << ' ';
line << "FCST_VALID_BEG" << ' ';
line << "FCST_VALID_END" << ' ';
line << "OBS_LEAD"       << ' ';
line << "OBS_VALID_BEG"  << ' ';
line << "OBS_VALID_END"  << ' ';
line << "FCST_VAR"       << ' ';
line << "FCST_UNITS"     << ' ';
line << "FCST_LEV"       << ' ';
line << "OBS_VAR"        << ' ';
line << "OBS_UNITS"      << ' ';
line << "OBS_LEV"        << ' ';
line << "OBTYPE"         << ' ';
line << "VX_MASK"        << ' ';
line << "INTERP_MTHD"    << ' ';
line << "INTERP_PNTS"    << ' ';
line << "FCST_THRESH"    << ' ';
line << "OBS_THRESH"     << ' ';
line << "COV_THRESH"     << ' ';
line << "ALPHA"          << ' ';
line << "LINE_TYPE";

   //
   //  done
   //

return ( line );

}


////////////////////////////////////////////////////////////////////////


ConcatString PyLineDataFile::make_data_line()

{

int j;
ConcatString line;
ConcatString a;
PyObject * sublist = 0;


   //
   //  grab the next item in the main list
   //

sublist = PyList_GetItem(main_list, index);

if ( ! sublist )  {

   mlog << Error
        << "\n\n  PyLineDataFile::make_data_line() -> nul sublist pointer!\n\n";

   exit ( 1 );

}

   //
   //  check that this item is itself a list
   //

if ( ! PyList_Check(sublist) )  {

   mlog << Error
        << "\n\n  PyLineDataFile::make_data_line() -> python object is not a list!\n\n";

   exit ( 1 );

}

const int N = PyList_Size(sublist);


   //
   //  version
   //

line << met_version;

   //
   //  the rest
   //

for (j=0; j<N; ++j)  {

   get_cs(PyList_GetItem(sublist, j), a);

   line << ' ' << a;

}

   //
   //  done
   //

return ( line );

}


////////////////////////////////////////////////////////////////////////


void PyLineDataFile::do_straight()

{

ConcatString command, path, user_base;

path = generic_python_wrapper;

mlog << Debug(3) 
     << "PyLineDataFile::do_straight() -> "
     << "Running user's python script ("
     << UserScriptPath << ").\n";

user_base = UserScriptPath.basename();

user_base.chomp(".py");

   //
   //  start up the python interpreter
   //

script = new Python3_Script (path.text());

   //
   //  set up a "new" sys.argv list
   //     with the command-line arquments for
   //     the user's script
   //

if ( UserScriptArgs.n() > 0 )  {

   script->reset_argv(UserScriptPath.text(), UserScriptArgs);

}

   //
   //  import the user's script as a module
   //

PyObject * m = PyImport_Import(PyUnicode_FromString(user_base.text()));

if ( PyErr_Occurred() )  {

   PyErr_Print();

   mlog << Error
        << "\nPyLineDataFile::do_straight() -> "
        << "an error occurred importing module "
        << '\"' << user_base.text() << "\"\n\n";

   exit ( 1 );

   return;

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

main_list = md.lookup_list(list_name);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void PyLineDataFile::do_pickle()

{

int j;
const int N = UserScriptArgs.n();
ConcatString command;
ConcatString path;
ConcatString pickle_path;
const char * tmp_dir = 0;
int status;

mlog << Debug(3) << "Calling " << UserPathToPython
     << " to run user's python script (" << UserScriptPath
     << ").\n";

tmp_dir = getenv ("MET_TMP_DIR");

if ( ! tmp_dir )  tmp_dir = default_tmp_dir;

path << cs_erase
     << tmp_dir << '/'
     << pickle_base_name;

pickle_path = make_temp_file_name(path.text(), 0);

command << cs_erase
        << UserPathToPython                   << ' '    //  user's path to python
        << replace_path(write_pickle_wrapper) << ' '    //  write_pickle.py
        << pickle_path                        << ' '    //  pickle output filename
        << UserScriptPath;                              //  user's script name

for (j=0; j<N; ++j)  {

   command << ' ' << UserScriptArgs[j];

};

status = system(command.text());

if ( status )  {

   mlog << Error << "\nPyLineDataFile::do_pickle() -> "
        << "command \"" << command.text() << "\" failed ... status = "
        << status << "\n\n";

   exit ( 1 );

}

ConcatString wrapper;

wrapper = generic_pickle_wrapper;

script = new Python3_Script (wrapper.text());

script->read_pickle(list_name, pickle_path.text());

main_list = script->lookup(list_name);

if ( ! main_list )  {

   mlog << Error
        << "\n\n  PyLineDataFile::do_pickle() -> nul main list pointer!\n\n";

   exit ( 1 );

}

   //
   //  cleanup
   //

remove_temp_file(pickle_path);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool PyLineDataFile::next_line(ConcatString & s_out)

{

   //
   //  first call?
   //
   //  if so, return header line
   //

if ( first_call )  {

   s_out = make_header_line();

   first_call = false;

   return ( true );

}

   //
   //  nope, return data line
   //

++index;

if ( index >= N )  return ( false );

s_out = make_data_line();


return ( true );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void get_cs(PyObject * in, ConcatString & out)

{

   //
   //  ok if the string is "NA", no separate check needed
   //

out = pyobject_as_concat_string (in);

return;

}


////////////////////////////////////////////////////////////////////////


void get_int    (PyObject * in, int & out)

{

if ( is_na(in) )  {

   out = bad_data_int;

   return;

}

out = pyobject_as_int(in);

return;

}


////////////////////////////////////////////////////////////////////////


void get_double (PyObject * in, double & out)

{

if ( is_na(in) )  {

   out = bad_data_double;

   return;

}

out = pyobject_as_double(in);


return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  check that it's a string and that the string contents are "NA"
   //


bool is_na (PyObject * obj)

{

if ( ! PyUnicode_Check(obj) )  return ( false );

   //
   //  now we know it's a string, the value had better be "NA"
   //

ConcatString s = pyobject_as_concat_string(obj);

if ( strcmp(s.text(), na_string.c_str()) == 0 )  return ( true );

   //
   //  done
   //

return ( false );   //  control flow should never get here

}


////////////////////////////////////////////////////////////////////////



