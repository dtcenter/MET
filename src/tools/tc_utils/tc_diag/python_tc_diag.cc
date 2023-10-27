// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <map>

#include "python_tc_diag.h"

#include "vx_config.h"
#include "vx_python3_utils.h"
#include "vx_log.h"

#include "global_python.h"
#include "wchar_argv.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

extern GlobalPython GP;   //  this needs external linkage

////////////////////////////////////////////////////////////////////////

static const char * user_ppath         = nullptr;
static const char write_tmp_diag    [] = "MET_BASE/python/pyembed/write_tmp_tc_diag.py";
static const char read_tmp_diag     [] = "pyembed.read_tmp_tc_diag";   //  NO ".py" suffix
static const char python_tc_diag_dir[] = "MET_BASE/python/tc_diag";

static const char storm_data_dict_name    [] = "storm_data";
static const char sounding_data_dict_name [] = "sounding_data";
static const char custom_data_dict_name   [] = "custom_data";
static const char units_dict_name         [] = "units";
static const char long_name_dict_name     [] = "long_name";
static const char comments_item_name      [] = "comments";

////////////////////////////////////////////////////////////////////////

static bool straight_python_tc_diag(
               const ConcatString &script_name,
               TmpFileInfo &tmp_info);

static bool tmp_nc_tc_diag(
               const ConcatString &script_name,
               TmpFileInfo &tmp_info);

static bool parse_python_module(
               PyObject *diag_dict,
               TmpFileInfo &tmp_info);

static bool parse_python_string_value_map(
               PyObject *dict,
               const char *name,
               vector<string> &k,
               map<string,double> &m);

static bool parse_python_string_string_map(
               PyObject *dict,
               const char *name,
               map<string,string> &m);

static bool parse_python_string(
               PyObject *dict,
               const char *name,
               string &s);

////////////////////////////////////////////////////////////////////////

bool python_tc_diag(const ConcatString &script_name,
        TmpFileInfo &tmp_info) {
   bool status = false;

   // Check for MET_PYTHON_EXE
   if ((user_ppath = getenv(user_python_path_env)) != nullptr ) {
      status = tmp_nc_tc_diag(script_name, tmp_info);
   }
   // Use compiled python instance
   else {
      status = straight_python_tc_diag(script_name, tmp_info);
   }

   return status;
}

////////////////////////////////////////////////////////////////////////

bool straight_python_tc_diag(const ConcatString &diag_script,
        TmpFileInfo &tmp_info) {
   const char *method_name = "straight_python_tc_diag()";

   mlog << Debug(3) << "Running Python diagnostics script ("
        << diag_script << " " << tmp_info.tmp_file << ").\n";

   // Prepare arguments
   StringArray arg_sa = diag_script.split(" ");
   arg_sa.add(tmp_info.tmp_file);
   Wchar_Argv wa;
   wa.set(arg_sa);

   // Reload the module if GP has already been initialized
   bool do_reload = GP.is_initialized;

   GP.initialize();

   if(PyErr_Occurred()) {
      PyErr_Print();
      mlog << Warning << "\n" << method_name << " -> "
           << "an error occurred initializing python\n\n";
      return false;
   }

   // Set the arguments
   run_python_string("import os");
   run_python_string("import sys");

   // Add the tc_diag python directory to the path
   ConcatString command;
   ConcatString tc_diag_dir(replace_path(python_tc_diag_dir));

   command << cs_erase
           << "sys.path.append(\""
           << tc_diag_dir
           << "\")";
   run_python_string(command.text());

   // Add the directory of the script to the path, if needed
   ConcatString script_name = arg_sa[0];

   if(tc_diag_dir != script_name.dirname()) {

      command << cs_erase
              << "sys.path.append(\""
              << script_name.dirname()
              << "\");";
      run_python_string(command.text());
   }

   if(arg_sa.n() > 0) {
      PySys_SetArgv(wa.wargc(), wa.wargv());
   }

   // Import the python script as a module
   ConcatString script_base = script_name.basename();
   script_base.chomp(".py");

   PyObject *module_obj = PyImport_ImportModule(script_base.c_str());

   // Reload the module, if needed
   if(do_reload) {
      module_obj = PyImport_ReloadModule(module_obj);
   }

   if(PyErr_Occurred()) {
      PyErr_Print();
      mlog << Warning << "\n" << method_name << " -> "
           << "an error occurred importing module \""
           << diag_script << "\"\n\n";
      return false;
   }

   if(!module_obj) {
      mlog << Warning << "\n" << method_name << " -> "
           << "error running Python script \""
           << diag_script << "\"\n\n";
      return false;
   }

   // Parse the diagnostics from python
   return(parse_python_module(module_obj, tmp_info));
}

////////////////////////////////////////////////////////////////////////

bool tmp_nc_tc_diag(const ConcatString &diag_script,
        TmpFileInfo &tmp_info) {
   const char *method_name = "tmp_nc_tc_diag()";
   int i, status;
   ConcatString command;
   ConcatString path;
   ConcatString tmp_nc_path;
   const char * tmp_dir = nullptr;
   Wchar_Argv wa;

   // TODO: Implement read/write temp tc_diag python functionality
   mlog << Error << "\n" << method_name << " -> "
        << "not yet fully implemented ... exiting!\n\n";
   exit(1);

   /*
   mlog << Debug(3) << "Calling " << user_ppath
        << " to run Python diagnostics script ("
        << diag_script << " " << tmp_file_name << ").\n";

   // Create a temp file
   tmp_dir = getenv ("MET_TMP_DIR");
   if(!tmp_dir) tmp_dir = default_tmp_dir;

   path << cs_erase
     << tmp_dir << '/'
     << tmp_nc_base_name;

   tmp_nc_path = make_temp_file_name(path.text(), 0);

   // Construct the system command
   command << cs_erase
           << user_ppath                   << ' ' // user's path to python
           << replace_path(write_tmp_diag) << ' ' // write_tmp_diag.py
           << tmp_nc_path                  << ' ' // tmp_nc output filename
           << diag_script                  << ' ' // python script name
           << tmp_file_name;                      // input temp NetCDF file

   mlog << Debug(4) << "Writing temporary Python dataplane file:\n\t"
        << command << "\n";

   status = system(command.text());

   if(status) {
      mlog << Error << "\n" << method_name << " -> "
           << "command \"" << command.text() << "\" failed ... status = "
           << status << "\n\n";
      exit(1);
   }

   // Reload the module if GP has already been initialized
   bool do_reload = GP.is_initialized;

   GP.initialize();

   if(PyErr_Occurred()) {
      PyErr_Print();
      mlog << Warning << "\n" << method_name << " -> "
           << "an error occurred initializing python\n\n";
      return false;
   }

   // Prepare arguments to read input
   StringArray arg_sa;
   arg_sa.add(read_tmp_diag);
   arg_sa.add(tmp_nc_path);
   wa.set(arg_sa);

   PySys_SetArgv (wa.wargc(), wa.wargv());

   mlog << Debug(4) << "Reading temporary Python diagnostics file: "
        << tmp_nc_path << "\n";

   // Import the python wrapper script as a module
   path = read_tmp_diag;
   path = path.basename();
   path.chomp(".py");

   PyObject * module_obj = PyImport_ImportModule(path.c_str());

   // Reload the module, if needed
   if(do_reload) {
      module_obj = PyImport_ReloadModule (module_obj);
   }

   if(PyErr_Occurred()) {
      PyErr_Print();
      mlog << Warning << "\n" << method_name << " -> "
        << "an error occurred importing module "
        << '\"' << path << "\"\n\n";
      return false;
   }

   if(!module_obj) {
      mlog << Warning << "\n" << method_name << " -> "
        << "error running Python script\n\n";
      return false;
   }

   // Parse the diagnostics from python
   bool status = parse_python_module(module_obj, tmp_info);

   // Cleanup
   remove_temp_file(tmp_nc_path);

   return(status);
   */
}

////////////////////////////////////////////////////////////////////////

bool parse_python_module(PyObject *module_obj,
        TmpFileInfo &tmp_info) {
   const char *method_name = "parse_python_module()";
   bool status = true;

   // Get the namespace for the module (as a dictionary)
   PyObject *module_dict_obj = PyModule_GetDict(module_obj);

   if(!module_dict_obj || !PyDict_Check(module_dict_obj)) {
      mlog << Warning << "\n" << method_name << " -> "
           << "python module is not a dictionary.\n\n";
      return(false);
   }

   // Storm data
   if(status) status = parse_python_string_value_map(
                          module_dict_obj,
                          storm_data_dict_name,
                          tmp_info.diag_storm_keys,
                          tmp_info.diag_storm_map);

   // Sounding data
   if(status) status = parse_python_string_value_map(
                          module_dict_obj,
                          sounding_data_dict_name,
                          tmp_info.diag_sounding_keys,
                          tmp_info.diag_sounding_map);

   // Custom data
   if(status) status = parse_python_string_value_map(
                          module_dict_obj,
                          custom_data_dict_name,
                          tmp_info.diag_custom_keys,
                          tmp_info.diag_custom_map);

   // Units
   if(status) status = parse_python_string_string_map(
                          module_dict_obj,
                          units_dict_name,
                          tmp_info.diag_units_map);

   // Long names
   if(status) status = parse_python_string_string_map(
                          module_dict_obj,
                          long_name_dict_name,
                          tmp_info.diag_long_name_map);

   // Comments
   string str;
   if(status) status = parse_python_string(module_dict_obj,
                          comments_item_name, str);

   ConcatString cs(str);
   StringArray sa = cs.split("\n");

   // Skip COMMENTS title line
   for(int i=0; i<sa.n(); i++) {
      if(sa[i].find("COMMENTS") == string::npos) {
         tmp_info.comment_lines.add(sa[i]);
      }
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool parse_python_string_value_map(PyObject *dict,
        const char *name,
        vector<string> &k,
        map<string,double> &m) {

   const char *method_name = "parse_python_string_value_map()";

   PyObject *val_obj = nullptr;
   PyObject *key_obj = nullptr;
   int status;
   double val;
   long pos;

   PyObject *data_obj = PyDict_GetItem(dict,
		           PyUnicode_FromString(name));

   if(!data_obj || !PyDict_Check(data_obj)) {
      mlog << Warning << "\n" << method_name << " -> "
           << "trouble parsing the \"" << name
           << "\" python dictionary.\n\n";
      return(false);
   }

   // Initialize
   pos = 0;

   // Loop through the dictionary entries
   while((status = PyDict_Next(data_obj, &pos, &key_obj, &val_obj)) != 0) {

      // All keys must be strings
      if(!PyUnicode_Check(key_obj)) {
         mlog << Error << "\n" << method_name << " -> "
              << "key is not a string!\n\n";
         exit(1);
      }

      // Parse key as a string and value as a number
      string key_str = PyUnicode_AsUTF8(key_obj);
      if(PyLong_Check(val_obj)) {
         val = (double) PyLong_AsLong(val_obj);
      }
      else if(PyFloat_Check(val_obj)) {
         val = PyFloat_AsDouble(val_obj);
      }
      else {
         mlog << Error << "\n" << method_name << " -> "
              << "value for \"" << key_str
              << "\" is not a numeric python data type!\n\n";
         exit(1);
      }

      // Check for duplicates
      if(m.count(key_str) > 0) {
         mlog << Warning << "\n" << method_name << " -> "
              << "ignoring duplicate entries for \""
              << key_str << "\" = " << val << "!\n\n";
      }
      // Store key/value pair in the dictionary
      else {
         mlog << Debug(5) << "Adding to map \""
              << key_str << "\" = " << val << "\n";
         k.push_back(key_str);
         m[key_str] = val;
      }
   } // end while

   return(true);
}

////////////////////////////////////////////////////////////////////////

bool parse_python_string_string_map(PyObject *dict,
        const char *name, map<string,string> &m) {

   const char *method_name = "parse_python_string_string_map()";

   PyObject *key_obj = nullptr;
   PyObject *val_obj = nullptr;
   int status;
   long pos;

   PyObject *data_obj = PyDict_GetItem(dict,
                           PyUnicode_FromString(name));

   if(!data_obj || !PyDict_Check(data_obj)) {
      mlog << Warning << "\n" << method_name << " -> "
           << "trouble parsing the \"" << name
           << "\" python dictionary.\n\n";
      return(false);
   }

   // Initialize
   pos = 0;

   // Loop through the dictionary entries
   while((status = PyDict_Next(data_obj, &pos, &key_obj, &val_obj)) != 0) {

      // All keys and values must be strings
      if(!PyUnicode_Check(key_obj) || !PyUnicode_Check(val_obj)) {
         mlog << Error << "\n" << method_name << " -> "
              << "key or value is not a string!\n\n";
         exit(1);
      }

      // Parse key and value as strings
      string key_str = PyUnicode_AsUTF8(key_obj);
      string val_str = PyUnicode_AsUTF8(val_obj);

      // Check for duplicates
      if(m.count(key_str) > 0) {
         mlog << Warning << "\n" << method_name << " -> "
              << "ignoring duplicate entries for \""
              << key_str << "\" = \"" << val_str << "\"!\n\n";
      }
      // Store key/value pair in the dictionary
      else {
         mlog << Debug(5) << "Adding to map \""
              << key_str << "\" = \"" << val_str << "\"\n";
         m[key_str] = val_str;
      }
   } // end while

   return(true);
}

////////////////////////////////////////////////////////////////////////

bool parse_python_string(PyObject *dict,
        const char *name, string &s) {

   const char *method_name = "parse_python_string()";

   PyObject *data_obj = PyDict_GetItem(dict,
                           PyUnicode_FromString(name));

   if(!data_obj || !PyUnicode_Check(data_obj)) {
      mlog << Warning << "\n" << method_name << " -> "
           << "trouble parsing the \"" << name
           << "\" python string.\n\n";
      return(false);
   }

   // Store the string
   s = PyUnicode_AsUTF8(data_obj);

   return(true);
}

////////////////////////////////////////////////////////////////////////
