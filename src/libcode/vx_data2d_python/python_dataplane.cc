// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
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


static bool met_python_dataplane(const char *user_script_name,
                                 const StringArray &user_script_args,
                                 const bool use_xarray, DataPlane &met_dp_out,
                                 Grid &met_grid_out, VarInfoPython &vinfo);


static bool user_python_dataplane(const char *user_script_name,
                                  const StringArray &user_script_args,
                                  DataPlane &met_dp_out, Grid &met_grid_out,
                                  VarInfoPython &vinfo);


////////////////////////////////////////////////////////////////////////


void print_python_dict(PyObject *py_dict_obj, const char *method_name) {
if (py_dict_obj) {
   PyObject *key;
   PyObject *value;
   Py_ssize_t pos = 0;
   while (PyDict_Next(py_dict_obj, &pos, &key, &value)) {
       mlog << Debug(1) << method_name
            << "  python: pos=" << pos << " key=" <<  PyUnicode_AsUTF8(key) << "\n";
   }
}
else {
    mlog << Debug(1) << method_name
         << "  invalid python dict object\n";
}

return;

}


////////////////////////////////////////////////////////////////////////


bool python_dataplane(const char * user_script_name,
                      const StringArray & user_script_args,
                      const bool use_xarray, DataPlane & met_dp_out,
                      Grid & met_grid_out, VarInfoPython &vinfo)
{

bool status = false;
user_ppath = getenv(user_python_path_env);
if(nullptr != user_ppath) {
   status = user_python_dataplane(user_script_name, user_script_args,
                                  met_dp_out, met_grid_out, vinfo);
} else {
   status = met_python_dataplane(user_script_name, user_script_args,
                                 use_xarray, met_dp_out,
                                 met_grid_out, vinfo);
}

return status;

}

////////////////////////////////////////////////////////////////////////


static bool met_python_dataplane(const char * user_script_name,
                                 const StringArray &user_script_args,
                                 const bool use_xarray, DataPlane & met_dp_out,
                                 Grid & met_grid_out, VarInfoPython &vinfo)
{

const char *method_name = "met_python_dataplane() -> ";

mlog << Debug(3) << "Running MET compile time Python instance ("
     << MET_PYTHON_BIN_EXE << ") to run user's Python script ("
     << user_script_name << ").\n";

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

   return false;

}

   //
   //  set the arguments
   //

StringArray sa;
sa.add(validate_dataplane); // Kludge to use PyConfig_SetArgv
sa.add(validate_dataplane);
sa.add(user_script_args);

   //
   //  set the global python arguments
   //

if ( user_script_args.n() > 0 && ! GP.set_args(sa, method_name) )  return false;

   //
   //  import the python script as a module
   //

PyObject *module_obj = PyImport_ImportModule (validate_dataplane);

   //
   //  if needed, reload the module
   //

if ( do_reload )  {
   module_obj = PyImport_ReloadModule (module_obj);
}

if ( PyErr_Occurred() )  {

   PyErr_Print();

   mlog << Warning << "\n" << method_name
        << "an error occurred importing module \""
        << user_script_name << "\"\n\n";

   return false;

}

if ( ! module_obj )  {

   mlog << Warning << "\n" << method_name
        << "error running python script \""
        << user_script_name << "\"\n\n";

   return false;

}

   //
   //   get the namespace for the module (as a dictionary)
   //

PyObject *module_dict_obj = PyModule_GetDict (module_obj);

   //
   //  get handles to the objects of interest from the module_dict
   //

PyObject *key_obj = nullptr;

if ( use_xarray )  {

   //  look up the data array variable name from the dictionary
   key_obj = PyUnicode_FromString (xarray_dataarray_name);
   PyObject *data_array_obj = PyDict_GetItem (module_dict_obj, key_obj);

   if ( ! data_array_obj )  {
      mlog << Warning << "\n" << method_name
           << "trouble reading data from \""
           << user_script_name << "\"\n\n";
      return false;
   }

   dataplane_from_xarray(data_array_obj, met_dp_out, met_grid_out, vinfo);

} else {    //  numpy array & dict

      //  look up the data array variable name from the dictionary

   key_obj = PyUnicode_FromString (numpy_array_name);
   PyObject *numpy_array_obj = PyDict_GetItem (module_dict_obj, key_obj);

   key_obj = PyUnicode_FromString (numpy_dict_name);
   PyObject *attrs_dict_obj = PyDict_GetItem (module_dict_obj, key_obj);

   if ( !numpy_array_obj || !attrs_dict_obj )  {
      mlog << Warning << "\n" << method_name
           << "trouble reading data from \""
           << user_script_name << "\"\n";
      if ( !numpy_array_obj ) mlog << Warning << "\n" << method_name
                                   << numpy_array_name << " is missing\n";
      if ( !attrs_dict_obj ) mlog << Warning << "\n" << method_name
                                  << numpy_dict_name << " is missing\n";
      mlog << Warning << "\n";

      print_python_dict(module_dict_obj, method_name);

      return false;
   }

   Python3_Numpy np;

   np.set(numpy_array_obj);

   dataplane_from_numpy_array(np, attrs_dict_obj, met_dp_out, met_grid_out, vinfo);

}

   //
   //  done
   //

return true;

}


////////////////////////////////////////////////////////////////////////


static bool user_python_dataplane(const char * user_script_name,
                                  const StringArray & user_script_args,
                                  DataPlane & met_dp_out, Grid & met_grid_out,
                                  VarInfoPython &vinfo)
{

const char *method_name = "user_python_dataplane() -> ";

mlog << Debug(3) << "Running user-specified Python instance (MET_PYTHON_EXE="
     << user_ppath << ") to run user's Python script ("
     << user_script_name << ").\n";

const char *tmp_dir = getenv ("MET_TMP_DIR");

if ( ! tmp_dir ) tmp_dir = default_tmp_dir;

ConcatString path;
path << tmp_dir << '/'
     << tmp_py_base_name;

ConcatString tmp_file_name(make_temp_file_name(path.text(), nullptr));

ConcatString command;
command << user_ppath                    << ' '    //  user's path to python
        << replace_path(write_tmp_py)    << ' '    //  write_tmp_nc.py
        << tmp_file_name                 << ' '    //  temporary output filename
        << user_script_name;                       //  user's script name

for (int j=1; j<user_script_args.n(); ++j)  {   //  j starts at one, here
   command << ' ' << user_script_args[j];
}

mlog << Debug(4) << "Writing temporary Python dataplane file:\n\t"
     << command << "\n";

int status = system(command.text());

if ( status )  {
   mlog << Error << "\n" << method_name
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

   mlog << Warning << "\n" << method_name
        << "an error occurred initializing python\n\n";

   return false;

}

   //
   //  set the arguments
   //

StringArray sa;
sa.add(validate_dataplane); // Kludge to use PyConfig_SetArgv
sa.add(validate_dataplane);
sa.add(replace_path(read_tmp_py));
sa.add(tmp_file_name);

   //
   //  set the global python arguments
   //

if ( user_script_args.n() > 0 && ! GP.set_args(sa, method_name) )  return false;

mlog << Debug(4) << "Reading temporary Python dataplane file: "
     << tmp_file_name << "\n";

   //
   //  import the python wrapper script as a module
   //

path = get_short_name(validate_dataplane);

PyObject *module_obj = PyImport_ImportModule(path.text());

   //
   //  if needed, reload the module
   //

if ( do_reload )  {
   module_obj = PyImport_ReloadModule(module_obj);
}

if ( PyErr_Occurred() )  {

   PyErr_Print();

   mlog << Warning << "\n" << method_name
        << "an error occurred importing module "
        << '\"' << path << "\"\n\n";

   return false;

}

if ( ! module_obj )  {
   mlog << Warning << "\n" << method_name
        << "error running python script\n\n";
   return false;
}

   //
   //  read the tmp_nc file
   //

   //
   //   get the namespace for the module (as a dictionary)
   //

PyObject *module_dict_obj = PyModule_GetDict(module_obj);
if (!module_dict_obj) {
   mlog << Warning << "\n" << method_name
        << " No dict object from python\n\n";
}

PyObject *key_obj = PyUnicode_FromString(tmp_nc_var_name);

PyObject *data_obj = PyDict_GetItem(module_dict_obj, key_obj);

if ( ! data_obj || ! PyDict_Check(data_obj) )  {
   mlog << Error << "\n" << method_name
        << (!data_obj ? "no" : "bad") << " dict object from " << tmp_nc_var_name << "\n\n";

   print_python_dict(module_dict_obj, method_name);
   exit ( 1 );

}

key_obj = PyUnicode_FromString(numpy_dict_name);

PyObject *attrs_dict_obj = PyDict_GetItem(data_obj, key_obj);

key_obj = PyUnicode_FromString(numpy_array_name);

PyObject *numpy_array_obj = PyDict_GetItem(data_obj, key_obj);

Python3_Numpy np;

np.set(numpy_array_obj);

dataplane_from_numpy_array(np, attrs_dict_obj, met_dp_out, met_grid_out, vinfo);

   //
   //  cleanup
   //

remove_temp_file(tmp_file_name);

   //
   //  done
   //

return true;

}


////////////////////////////////////////////////////////////////////////

