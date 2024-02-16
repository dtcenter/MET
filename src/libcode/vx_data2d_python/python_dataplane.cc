// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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


extern GlobalPython GP;   //  this needs external linkage


////////////////////////////////////////////////////////////////////////


static const char * user_ppath            = nullptr;

static const char write_tmp_py         [] = "MET_BASE/python/pyembed/write_tmp_dataplane.py";

static const char read_tmp_py          [] = "MET_BASE/python/pyembed/read_tmp_dataplane.py";

static const char tmp_nc_var_name      [] = "met_info";

static const char validate_dataplane   [] = "met.dataplane";   // NO ".py" suffix

////////////////////////////////////////////////////////////////////////


static bool straight_python_dataplane(const char * script_name,
                                      int script_argc, char ** script_argv,
                                      const bool use_xarray, DataPlane & met_dp_out,
                                      Grid & met_grid_out, VarInfoPython &vinfo);


static bool tmp_dataplane(const char * script_name,
                          int script_argc, char ** script_argv,
                          const bool use_xarray, DataPlane & met_dp_out,
                          Grid & met_grid_out, VarInfoPython &vinfo);


////////////////////////////////////////////////////////////////////////

void release_memory(int script_argc, char ** script_argv) {
   if ( script_argv )  {
      for (int i=0; i<script_argc; i++ )  {
         if ( script_argv[i] )  {
            delete [] script_argv[i];
            script_argv[i] = (char *) nullptr;
         }
      }
      delete [] script_argv;
      script_argv = (char **) nullptr;
   }
}

////////////////////////////////////////////////////////////////////////


bool python_dataplane(const char * user_script_name,
                      int user_script_argc, char ** user_script_argv,
                      const bool use_xarray, DataPlane & met_dp_out,
                      Grid & met_grid_out, VarInfoPython &vinfo)

{

bool status = false;

if ( (user_ppath = getenv(user_python_path_env)) != 0 )  {   //  do_tmp_nc = true;

   status = tmp_dataplane(user_script_name,
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
const char *method_name = "straight_python_dataplane() -> ";

mlog << Debug(3) << "Running user's python script ("
     << user_script_name << ").\n";

cs        = user_script_name;
user_dir  = cs.dirname();
user_base = cs.basename();

Wchar_Argv wa;

int script_argc = user_script_argc + 1;
char ** script_argv = new char * [ script_argc ];

char a_var_name[512+1];
script_argv[0] = m_strcpy2(validate_dataplane, method_name, validate_dataplane);
for (int i=0; i<user_script_argc; i++ )  {
   snprintf(a_var_name, 512, "python_argv[%d]", i);
   script_argv[i+1] = m_strcpy2(user_script_argv[i], method_name, a_var_name);
}


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

   mlog << Warning << "\n" << method_name
        << "an error occurred initializing python\n\n";
   release_memory(script_argc, script_argv);

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

//module_obj = PyImport_ImportModule (user_base.c_str());
module_obj = PyImport_ImportModule (validate_dataplane);

   //
   //  if needed, reload the module
   //

if ( do_reload )  {

   module_obj = PyImport_ReloadModule (module_obj);

}

release_memory(script_argc, script_argv);

if ( PyErr_Occurred() )  {

   PyErr_Print();

   mlog << Warning << "\n" << method_name
        << "an error occurred importing module \""
        << user_script_name << "\"\n\n";

   return ( false );

}

if ( ! module_obj )  {

   mlog << Warning << "\n" << method_name
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

      mlog << Warning << "\n" << method_name
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

      mlog << Warning << "\n" << method_name
           << "trouble reading data from \""
           << user_script_name << "\"\n\n";
      if ( !numpy_array_obj ) mlog << Warning << "\n" << method_name
                                   << numpy_array_name << " is missing\n";
      if ( !attrs_dict_obj ) mlog << Warning << "\n" << method_name
                                  << numpy_dict_name << " is missing\n";
      mlog << Warning << "\n";

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


bool tmp_dataplane(const char * user_script_name,
                   int user_script_argc, char ** user_script_argv,
                   const bool use_xarray, DataPlane & met_dp_out,
                   Grid & met_grid_out, VarInfoPython &vinfo)

{

int j;
int status;
ConcatString command;
ConcatString path;
ConcatString tmp_nc_path;
const char * tmp_dir = 0;
Wchar_Argv wa;

mlog << Debug(3) << "Calling " << user_ppath
     << " to run user's python script (" << user_script_name
     << ").\n";

tmp_dir = getenv ("MET_TMP_DIR");

if ( ! tmp_dir )  tmp_dir = default_tmp_dir;

path << cs_erase
     << tmp_dir << '/'
     << tmp_py_base_name;

tmp_nc_path = make_temp_file_name(path.text(), 0);

command << cs_erase
        << user_ppath                    << ' '    //  user's path to python
        << replace_path(write_tmp_py)    << ' '    //  write_tmp_nc.py
        << tmp_nc_path                   << ' '    //  tmp_nc output filename
        << user_script_name;                       //  user's script name

for (j=1; j<user_script_argc; ++j)  {   //  j starts at one, here

   command << ' ' << user_script_argv[j];

}

mlog << Debug(4) << "Writing temporary Python dataplane file:\n\t"
     << command << "\n";

status = system(command.text());

if ( status )  {

   mlog << Error << "\ntmp_nc_dataplane() -> "
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

   mlog << Warning << "\ntmp_nc_dataplane() -> "
        << "an error occurred initializing python\n\n";

   return ( false );

}

   //
   //  set the arguments
   //

StringArray a;

a.add(validate_dataplane);

a.add(replace_path(read_tmp_py));

a.add(tmp_nc_path);

wa.set(a);

PySys_SetArgv (wa.wargc(), wa.wargv());

mlog << Debug(4) << "Reading temporary Python dataplane file: "
     << tmp_nc_path << "\n";

   //
   //  import the python wrapper script as a module
   //

//path = get_short_name(read_tmp_py);
path = get_short_name(validate_dataplane);

PyObject * module_obj = PyImport_ImportModule (path.text());

   //
   //  if needed, reload the module
   //

if ( do_reload )  {

   module_obj = PyImport_ReloadModule (module_obj);

}

if ( PyErr_Occurred() )  {

   PyErr_Print();

   mlog << Warning << "\ntmp_nc_dataplane() -> "
        << "an error occurred importing module "
        << '\"' << path << "\"\n\n";

   return ( false );

}

if ( ! module_obj )  {

   mlog << Warning << "\ntmp_nc_dataplane() -> "
        << "error running python script\n\n";

   return ( false );

}

   //
   //  read the tmp_nc file
   //

   //
   //   get the namespace for the module (as a dictionary)
   //

PyObject * module_dict_obj = PyModule_GetDict (module_obj);

PyObject * key_obj = PyUnicode_FromString (tmp_nc_var_name);

PyObject * data_obj = PyDict_GetItem (module_dict_obj, key_obj);

if ( ! data_obj || ! PyDict_Check(data_obj) )  {

   mlog << Error << "\ntmp_nc_dataplane() -> "
        << (!data_obj ? "no" : "bad") << " dict object from " << tmp_nc_var_name << "\n\n";

   exit ( 1 );

}

key_obj = PyUnicode_FromString (numpy_dict_name);

PyObject * attrs_dict_obj = PyDict_GetItem (data_obj, key_obj);

key_obj = PyUnicode_FromString (numpy_array_name);

PyObject * numpy_array_obj = PyDict_GetItem (data_obj, key_obj);

Python3_Numpy np;

np.set(numpy_array_obj);

dataplane_from_numpy_array(np, attrs_dict_obj, met_dp_out, met_grid_out, vinfo);

   //
   //  cleanup
   //

remove_temp_file(tmp_nc_path);

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////

