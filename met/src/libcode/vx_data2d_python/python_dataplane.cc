// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#include "data_plane.h"

#include "grid_from_python_dict.h"
#include "dataplane_from_xarray.h"
#include "dataplane_from_numpy_array.h"
#include "python_dataplane.h"

#include "global_python.h"
#include "wchar_argv.h"


////////////////////////////////////////////////////////////////////////


GlobalPython GP;   //  this needs external linkage


////////////////////////////////////////////////////////////////////////


static const char * user_ppath            = 0;

static const char write_pickle         [] = "MET_BASE/wrappers/write_pickle_dataplane.py";

static const char read_pickle          [] = "read_pickle_dataplane";   //  NO ".py" suffix

static const char pickle_base_name     [] = "tmp_met_pickle";

static const char pickle_var_name      [] = "met_info";

static const char pickle_file_var_name [] = "pickle_filename";


////////////////////////////////////////////////////////////////////////


static bool straight_python_dataplane(const char * script_name,
                                      int script_argc, char ** script_argv,
                                      const bool use_xarray, DataPlane & met_dp_out,
                                      Grid & met_grid_out, VarInfoPython &vinfo);


static bool pickle_dataplane(const char * script_name,
                             int script_argc, char ** script_argv,
                             const bool use_xarray, DataPlane & met_dp_out,
                             Grid & met_grid_out, VarInfoPython &vinfo);


////////////////////////////////////////////////////////////////////////


bool python_dataplane(const char * user_script_name,
                      int user_script_argc, char ** user_script_argv,
                      const bool use_xarray, DataPlane & met_dp_out,
                      Grid & met_grid_out, VarInfoPython &vinfo)

{

bool status = false;

if ( (user_ppath = getenv(user_python_path_env)) != 0 )  {   //  do_pickle = true;

   status = pickle_dataplane(user_script_name,
                             user_script_argc, user_script_argv,
                             use_xarray, met_dp_out,
                             met_grid_out, vinfo);

} else {

   status = straight_python_dataplane(user_script_name,
                                      user_script_argc, user_script_argv,
                                      use_xarray, met_dp_out,
                                      met_grid_out, vinfo);

}




return ( status );

}

////////////////////////////////////////////////////////////////////////


bool straight_python_dataplane(const char * user_script_name,
                              int user_script_argc, char ** user_script_argv,
                              const bool use_xarray, DataPlane & met_dp_out,
                              Grid & met_grid_out, VarInfoPython &vinfo)

{

PyObject * module_obj      = 0;
PyObject * module_dict_obj = 0;
PyObject * key_obj         = 0;
PyObject * numpy_array_obj = 0;
PyObject * attrs_dict_obj  = 0;
ConcatString cs, user_dir, user_base;

mlog << Debug(3) << "Running user's python script ("
     << user_script_name << ").\n";

cs        = user_script_name;
user_dir  = cs.dirname();
user_base = cs.basename();

Wchar_Argv wa;

wa.set(user_script_argc, user_script_argv);

   //
   //  if the global python object has already been initialized,
   //  we need to reload the module
   //

bool do_reload = GP.is_initialized;

GP.initialize();

   //
   //   start up the python interpreter
   //

if ( PyErr_Occurred() )  {

   PyErr_Print();

   mlog << Warning << "\npython_dataplane() -> "
        << "an error occurred initializing python\n\n";

   return ( false );

}

   //
   //  set the arguments
   //

run_python_string("import os");
run_python_string("import sys");

ConcatString command;

command << cs_erase
        << "sys.path.append(\""
        << user_dir
        << "\")";

run_python_string(command.text());

if ( user_script_argc > 0 )  {

   PySys_SetArgv (wa.wargc(), wa.wargv());

}

   //
   //  import the python script as a module
   //

module_obj = PyImport_ImportModule (user_base.c_str());

   //
   //  if needed, reload the module
   //

if ( do_reload )  {

   module_obj = PyImport_ReloadModule (module_obj);

}

if ( PyErr_Occurred() )  {

   PyErr_Print();

   mlog << Warning << "\npython_dataplane() -> "
        << "an error occurred importing module \""
        << user_script_name << "\"\n\n";

   return ( false );

}

if ( ! module_obj )  {

   mlog << Warning << "\npython_dataplane() -> "
        << "error running python script \""
        << user_script_name << "\"\n\n";

   return ( false );

}

   //
   //   get the namespace for the module (as a dictionary)
   //

module_dict_obj = PyModule_GetDict (module_obj);

   //
   //  get handles to the objects of interest from the module_dict
   //

if ( use_xarray )  {

   PyObject * data_array_obj = 0;

      //  look up the data array variable name from the dictionary

   key_obj = PyUnicode_FromString (xarray_dataarray_name);

   data_array_obj = PyDict_GetItem (module_dict_obj, key_obj);

   if ( ! data_array_obj )  {

      mlog << Warning << "\npython_dataplane() -> "
           << "trouble reading data from \""
           << user_script_name << "\"\n\n";

      return ( false );
   }

   dataplane_from_xarray(data_array_obj, met_dp_out, met_grid_out, vinfo);

} else {    //  numpy array & dict

      //  look up the data array variable name from the dictionary

   key_obj = PyUnicode_FromString (numpy_array_name);

   numpy_array_obj = PyDict_GetItem (module_dict_obj, key_obj);

   key_obj = PyUnicode_FromString (numpy_dict_name);

   attrs_dict_obj = PyDict_GetItem (module_dict_obj, key_obj);

   if ( !numpy_array_obj || !attrs_dict_obj )  {

      mlog << Warning << "\npython_dataplane() -> "
           << "trouble reading data from \""
           << user_script_name << "\"\n\n";

      return ( false );
   }

   Python3_Numpy np;

   np.set(numpy_array_obj);

   dataplane_from_numpy_array(np, attrs_dict_obj, met_dp_out, met_grid_out, vinfo);

}

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool pickle_dataplane(const char * user_script_name,
                      int user_script_argc, char ** user_script_argv,
                      const bool use_xarray, DataPlane & met_dp_out,
                      Grid & met_grid_out, VarInfoPython &vinfo)

{

int j;
int status;
ConcatString command;
ConcatString path;
ConcatString pickle_path;
const char * tmp_dir = 0;
Wchar_Argv wa;

mlog << Debug(3) << "Calling " << user_ppath
     << " to run user's python script (" << user_script_name
     << ").\n";

tmp_dir = getenv ("MET_TMP_DIR");

if ( ! tmp_dir )  tmp_dir = default_tmp_dir;

path << cs_erase
     << tmp_dir << '/'
     << pickle_base_name;

pickle_path = make_temp_file_name(path.text(), 0);

command << cs_erase
        << user_ppath                    << ' '    //  user's path to python
        << replace_path(write_pickle)    << ' '    //  write_pickle.py
        << pickle_path                   << ' '    //  pickle output filename
        << user_script_name;                       //  user's script name

for (j=1; j<user_script_argc; ++j)  {   //  j starts at one, here

   command << ' ' << user_script_argv[j];

}

status = system(command.text());

if ( status )  {

   mlog << Error << "\npickle_dataplane() -> "
        << "command \"" << command.text() << "\" failed ... status = "
        << status << "\n\n";

   exit ( 1 );

}

   //
   //  if the global python object has already been initialized,
   //  we need to reload the module
   //

bool do_reload = GP.is_initialized;

GP.initialize();

   //
   //   start up the python interpreter
   //

if ( PyErr_Occurred() )  {

   PyErr_Print();

   mlog << Warning << "\npython_dataplane() -> "
        << "an error occurred initializing python\n\n";

   return ( false );

}

mlog << Debug(3) << "Reading temporary pickle file: "
     << pickle_path << "\n";

   //
   //  set the arguments
   //

StringArray a;

a.add(read_pickle);

a.add(pickle_path);

wa.set(a);

PySys_SetArgv (wa.wargc(), wa.wargv());

   //
   //  import the python wrapper script as a module
   //

path = get_short_name(read_pickle);

PyObject * module_obj = PyImport_ImportModule (path.text());

   //
   //  if needed, reload the module
   //

if ( do_reload )  {

   module_obj = PyImport_ReloadModule (module_obj);

}

if ( PyErr_Occurred() )  {

   PyErr_Print();

   mlog << Warning << "\npython_dataplane() -> "
        << "an error occurred importing module "
        << '\"' << path << "\"\n\n";

   return ( false );

}

if ( ! module_obj )  {

   mlog << Warning << "\npython_dataplane() -> "
        << "error running python script\n\n";

   return ( false );

}

   //
   //  read the pickle file
   //

   //
   //   get the namespace for the module (as a dictionary)
   //

PyObject * module_dict_obj = PyModule_GetDict (module_obj);

PyObject * key_obj = PyUnicode_FromString (pickle_var_name);

PyObject * data_obj = PyDict_GetItem (module_dict_obj, key_obj);

if ( ! data_obj || ! PyDict_Check(data_obj) )  {

   mlog << Error << "\npickle_dataplane() -> "
        << "bad dict object\n\n";

   exit ( 1 );

}

if ( use_xarray )  {

   exit ( 1 );

} else {

   key_obj = PyUnicode_FromString (numpy_dict_name);

   PyObject * attrs_dict_obj = PyDict_GetItem (data_obj, key_obj);

   key_obj = PyUnicode_FromString (numpy_array_name);

   PyObject * numpy_array_obj = PyDict_GetItem (data_obj, key_obj);

   Python3_Numpy np;

   np.set(numpy_array_obj);

   dataplane_from_numpy_array(np, attrs_dict_obj, met_dp_out, met_grid_out, vinfo);

}

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

