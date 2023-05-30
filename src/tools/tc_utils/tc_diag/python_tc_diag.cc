// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <map>

#include "vx_python3_utils.h"
#include "vx_log.h"

#include "global_python.h"
#include "wchar_argv.h"

////////////////////////////////////////////////////////////////////////

extern GlobalPython GP;   //  this needs external linkage

////////////////////////////////////////////////////////////////////////

static const char * user_ppath         = 0;
static const char write_tmp_diag    [] = "MET_BASE/python/pyembed/write_tmp_tc_diag.py";
static const char read_tmp_diag     [] = "pyembed.read_tmp_tc_diag";   //  NO ".py" suffix
static const char tc_diag_dict_name [] = "tc_diag";

////////////////////////////////////////////////////////////////////////

static bool straight_python_tc_diag(const ConcatString &script_name,
               const ConcatString &tmp_file_name,
               map<string,string> &diag_map);

static bool tmp_nc_tc_diag(const ConcatString &script_name,
               const ConcatString &tmp_file_name,
               map<string,string> &diag_map);

static void diag_map_from_python_dict(
               PyObject *diag_dict,
               map<string,string> &diag_map);

////////////////////////////////////////////////////////////////////////

bool python_tc_diag(const ConcatString &script_name,
        const ConcatString &tmp_file_name,
        map<string,string> &diag_map) {
   bool status = false;

   // Check for MET_PYTHON_EXE
   if ((user_ppath = getenv(user_python_path_env)) != 0 ) {
      status = tmp_nc_tc_diag(script_name,
                  tmp_file_name, diag_map);
   }
   // Use compiled python instance
   else {
      status = straight_python_tc_diag(script_name,
                  tmp_file_name, diag_map);
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool straight_python_tc_diag(const ConcatString &script_name,
        const ConcatString &tmp_file_name,
        map<string,string> &diag_map) {
   PyObject * module_obj      = 0;
   PyObject * module_dict_obj = 0;
   PyObject * key_obj         = 0;
   PyObject * diag_dict_obj   = 0;

   mlog << Debug(3) << "Running python diagnostics script ("
        << script_name << " " << tmp_file_name << ").\n";

   // Prepare arguments
   StringArray arg_sa = script_name.split(" ");
   arg_sa.add(tmp_file_name);
   Wchar_Argv wa;
   wa.set(arg_sa);

   // Reload the module if GP has already been initialized
   bool do_reload = GP.is_initialized;

   GP.initialize();

   if(PyErr_Occurred()) {
      PyErr_Print();
      mlog << Warning << "\nstraight_python_tc_diag() -> "
           << "an error occurred initializing python\n\n";
      return(false);
   }

   // Set the arguments
   run_python_string("import os");
   run_python_string("import sys");

   ConcatString command;
   command << cs_erase
           << "sys.path.append(\""
           << script_name.dirname()
           << "\")";

   run_python_string(command.text());

   if(arg_sa.n() > 0) {
      PySys_SetArgv(wa.wargc(), wa.wargv());
   }

   // Import the python script as a module
   ConcatString script_base = script_name.basename();
   script_base.chomp(".py");

   module_obj = PyImport_ImportModule(script_base.c_str());

   // Reload the module, if needed
   if(do_reload) {
      module_obj = PyImport_ReloadModule(module_obj);
   }

   if(PyErr_Occurred()) {
      PyErr_Print();
      mlog << Warning << "\nstraight_python_tc_diag() -> "
           << "an error occurred importing module \""
           << script_name << "\"\n\n";
      return(false);
   }

   if(!module_obj) {
      mlog << Warning << "\nstraight_python_tc_diag() -> "
           << "error running python script \""
           << script_name << "\"\n\n";
      return(false);
   }

   // Get the namespace for the module (as a dictionary)
   module_dict_obj = PyModule_GetDict(module_obj);

   // Get handle to the object of interest from the module_dict
   key_obj = PyUnicode_FromString(tc_diag_dict_name);

   diag_dict_obj = PyDict_GetItem (module_dict_obj, key_obj);

   if(!diag_dict_obj) {
      mlog << Warning << "\nstraight_python_tc_diag() -> "
           << "trouble reading data from \""
           << script_name << "\"\n\n";
      return(false);
   }

   // Populate the diagnostics map
   diag_map_from_python_dict(diag_dict_obj, diag_map);

   return(true);
}

////////////////////////////////////////////////////////////////////////

bool tmp_nc_tc_diag(const ConcatString &script_name,
        const ConcatString &tmp_file_name,
        map<string,string> &diag_map) {
/*
// JHG work here
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
     << tmp_nc_base_name;

tmp_nc_path = make_temp_file_name(path.text(), 0);

command << cs_erase
        << user_ppath                    << ' '    //  user's path to python
        << replace_path(write_tmp_nc)    << ' '    //  write_tmp_nc.py
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

a.add(read_tmp_nc);

a.add(tmp_nc_path);

wa.set(a);

PySys_SetArgv (wa.wargc(), wa.wargv());

mlog << Debug(4) << "Reading temporary Python dataplane file: "
     << tmp_nc_path << "\n";

   //
   //  import the python wrapper script as a module
   //

path = get_short_name(read_tmp_nc);

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
        << "bad dict object\n\n";

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
*/

   mlog << Warning << "\ntmp_nc_tc_diag() -> "
        << "not yet implemented!\n\n";

   return(false);
}

////////////////////////////////////////////////////////////////////////

void diag_map_from_python_dict(PyObject *diag_dict,
        map<string,string> &diag_map) {
   PyObject *key_obj = 0;
   PyObject *val_obj = 0;
   int status;
   long pos;

   // Initialize
   pos = 0;

   // Loop through the dictionary entries
   while((status = PyDict_Next(diag_dict, &pos, &key_obj, &val_obj)) != 0) {

      // All keys must be strings
      if(!PyUnicode_Check(key_obj)) {
         mlog << Error << "\ndiag_map_from_python_dict() -> "
              << "key is not a string!\n\n";
         exit(1);
      }

      // Parse key and value as strings
      string key_str = PyUnicode_AsUTF8(key_obj);
      ConcatString val_str;
           if(PyUnicode_Check(val_obj)) val_str << PyUnicode_AsUTF8(val_obj);
      else if(PyLong_Check(val_obj))    val_str << (int) PyLong_AsLong(val_obj);
      else if(PyFloat_Check(val_obj))   val_str << PyFloat_AsDouble(val_obj);
      else {
         mlog << Error << "\ndiag_map_from_python_dict() -> "
              << "unsupported python data type for the \""
              << key_str << "\" diagnostic!\n\n";
         exit(1);
      }

      // Check for duplicates
      if(diag_map.count(key_str) > 0) {
         mlog << Warning << "\ndiag_map_from_python_dict() -> "
              << "ignoring duplicate entry for TC diagnostic \""
              << key_str << "\" = \"" << val_str << "\"!\n\n";
      }
      // Store key/value pair in the dictionary
      else {
         mlog << Debug(5) << "Storing TC diagnostic \""
              << key_str << "\" = \"" << val_str << "\"\n";
         diag_map[key_str] = val_str;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////
