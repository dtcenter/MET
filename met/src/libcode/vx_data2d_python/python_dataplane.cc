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


static const char * user_ppath = 0;

static const char write_pickle [] = "MET_BASE/wrappers/write_pickle.py";

static const char generic_read_pickle [] = "MET_BASE/wrappers/generic_pickle";   //  NO ".py" suffix

static const char pickle_out_name [] = "out.pickle";

static const char pickle_var_name [] = "met_info";


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


bool python_dataplane(const char * script_name,
                      int script_argc, char ** script_argv,
                      const bool use_xarray, DataPlane & met_dp_out,
                      Grid & met_grid_out, VarInfoPython &vinfo)

{

bool status = false;


mlog << Debug(4) << "\n\n  in python_dataplane()\n\n";

if ( (user_ppath = getenv(user_python_path_env)) != 0 )  {   //  do_pickle = true;

status = pickle_dataplane(script_name,
                          script_argc, script_argv,
                          use_xarray, met_dp_out,
                          met_grid_out, vinfo);


} else {

status = straight_python_dataplane(script_name,
                                   script_argc, script_argv,
                                   use_xarray, met_dp_out,
                                   met_grid_out, vinfo);


}




return ( status );

}

////////////////////////////////////////////////////////////////////////


bool straight_python_dataplane(const char * script_name,
                              int script_argc, char ** script_argv,
                              const bool use_xarray, DataPlane & met_dp_out,
                              Grid & met_grid_out, VarInfoPython &vinfo)

{

PyObject * module_obj      = 0;
PyObject * module_dict_obj = 0;
PyObject * key_obj         = 0;
PyObject * numpy_array_obj = 0;
PyObject * attrs_dict_obj  = 0;
// bool do_pickle = false;
// char * user_ppath = 0;
// 
// 
// if ( (user_ppath = getenv(user_python_path_env)) != 0 )  do_pickle = true;


Wchar_Argv wa;

wa.set(script_argc, script_argv);


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

if ( script_argc > 0 )  {

   // PySys_SetArgv (script_argc, script_argv);

   PySys_SetArgv (wa.wargc(), wa.wargv());

}

   //
   //  import the python script as a module
   //

module_obj = PyImport_ImportModule (script_name);

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
        << script_name << "\"\n\n";

   return ( false );

}

if ( ! module_obj )  {

   mlog << Warning << "\npython_dataplane() -> "
        << "error running python script \""
        << script_name << "\"\n\n";

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

      //   look up the data array variable name from the dictionary

   key_obj = PyUnicode_FromString (xarray_dataarray_name);

   data_array_obj = PyDict_GetItem (module_dict_obj, key_obj);

   if ( ! data_array_obj )  {

      mlog << Warning << "\npython_dataplane() -> "
           << "trouble reading data from \""
           << script_name << "\"\n\n";

      return ( false );
   }

   dataplane_from_xarray(data_array_obj, met_dp_out, met_grid_out, vinfo);

} else {    //  numpy array & dict

      //   look up the data array variable name from the dictionary

   key_obj = PyUnicode_FromString (numpy_array_name);

   numpy_array_obj = PyDict_GetItem (module_dict_obj, key_obj);

   key_obj = PyUnicode_FromString (numpy_dict_name);

   attrs_dict_obj = PyDict_GetItem (module_dict_obj, key_obj);

   if ( !numpy_array_obj || !attrs_dict_obj )  {

      mlog << Warning << "\npython_dataplane() -> "
           << "trouble reading data from \""
           << script_name << "\"\n\n";

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

   //  wrapper usage:  /path/to/python write_pickle.py pickle_output_filename <user_python_script>.py <args>

command << cs_erase
        << user_ppath                    << ' '    //  user's path to python
        << replace_path(write_pickle)    << ' '    //  write_pickle.py
        << pickle_out_name               << ' '    //  pickle output filename
        << user_script_name;                       //  user's script name

for (j=1; j<user_script_argc; ++j)  {   //  j starts at one, here

   command << ' ' << user_script_argv[j];

}

status = system(command.text());

if ( status )  {

   mlog << Error
        << "\n\n  pickle_dataplane() -> command \""
        << command.text() << "\" failed!\n\n";

   exit ( 1 );

}

   //
   //  now we've got the pickle file, so just read it
   //

/*

path << mb << '/' << generic_read_pickle;

// Python3_Script script(path.text());
Python3_Script script("generic_pickle");

PyErr_Clear();

// script.run("import pickle");
// script.run("import numpy");

// if ( PyErr_Occurred() )  {
// 
//     cout << "   import numpy failed!\n\n" << flush;
// 
//     exit ( 1 );
// 
// }

PyErr_Clear();

script.read_pickle  (pickle_var_name, pickle_out_name);

if ( PyErr_Occurred() )  {

   PyErr_Print();

   mlog << Error
        << "   read_pickle failed!\n\n" << flush;

   exit ( 1 );

}

PyErr_Clear();

PyObject * met_info = script.lookup(pickle_var_name);

if ( PyErr_Occurred() )  {

   PyErr_Print();

   cout << "   lookup failed!\n\n" << flush;

   exit ( 1 );

}
*/




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

/*
if ( script_argc > 0 )  {

   // PySys_SetArgv (script_argc, script_argv);

   PySys_SetArgv (wa.wargc(), wa.wargv());

}
*/


// run_python_string("import sys");
// run_python_string("import numpy");
// run_python_string("import pickle");
// if ( use_xarray )  run_python_string("import xarray");


   //
   //  import the python wrapper script as a module
   //


// path << mb << '/' << generic_read_pickle;

path = get_short_name(generic_read_pickle);

// cout << "\n\n  pickle_dataplane() -> path = \"" << path.text() << "\"\n\n" << flush;

PyObject * module_obj = PyImport_ImportModule (path.text());

   //
   //  if needed, reload the module
   //

if ( do_reload )  {

   module_obj = PyImport_ReloadModule (module_obj);

}


// command << cs_erase
//         << "met_info = pickle.load(open(\""
//         << pickle_out_name
//         << "\", \"rb\"))";
// 
// cout << "\n\n  command = \"" << command << "\"\n\n" << flush;
// 
// run_python_string(command.text());

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

// command << cs_erase << "x = 5";
// command << cs_erase << "met_info = pickle.load(open(\"out.pickle\", \"rb\"))";
// 
// run_python_string(command.text());

   //
   //   get the namespace for the module (as a dictionary)
   //

PyObject * module_dict_obj = PyModule_GetDict (module_obj);

// cout << "module_dict_obj = " << module_dict_obj << "\n" << flush;

PyObject * key_obj = PyUnicode_FromString ("met_info");

PyObject * data_obj = PyDict_GetItem (module_dict_obj, key_obj);

// cout << "data_obj = " << data_obj << "\n" << flush;

if ( ! PyDict_Check(data_obj) )  {

   cout << "\n\n  bad data object\n\n" << flush;

   exit ( 1 );

}




if ( use_xarray )  {

   exit ( 1 );

} else {

   key_obj = PyUnicode_FromString (numpy_dict_name);

   PyObject * attrs_dict_obj = PyDict_GetItem (data_obj, key_obj);

   cout << "attrs = " << attrs_dict_obj << "\n" << flush;

   key_obj = PyUnicode_FromString (numpy_array_name);

   PyObject * numpy_array_obj = PyDict_GetItem (data_obj, key_obj);

   cout << "met_data = " << numpy_array_obj << "\n" << flush;

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





