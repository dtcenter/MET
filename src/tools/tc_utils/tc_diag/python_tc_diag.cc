// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
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

static const char *user_ppath                = nullptr;
static const char write_tmp_diag          [] = "MET_BASE/python/pyembed/write_tmp_diag.py";
static const char read_tmp_diag           [] = "pyembed.read_tmp_diag";   //  NO ".py" suffix
static const char python_tc_diag_dir      [] = "MET_BASE/python/tc_diag";
static const char tmp_diag_base_name      [] = "tmp_diag_data";
static const char diag_data_dict_name     [] = "diag_data";
static const char storm_data_dict_name    [] = "storm_data";
static const char sounding_data_dict_name [] = "sounding_data";
static const char custom_data_dict_name   [] = "custom_data";
static const char units_dict_name         [] = "units";
static const char long_name_dict_name     [] = "long_name";
static const char comments_item_name      [] = "comments";

////////////////////////////////////////////////////////////////////////

static bool met_python_tc_diag(
               const ConcatString &script_name,
               TmpFileInfo &tmp_info);

static bool user_python_tc_diag(
               const ConcatString &script_name,
               TmpFileInfo &tmp_info);

static bool parse_python_diag_data(
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

bool python_tc_diag(const ConcatString &script_name, TmpFileInfo &tmp_info) {
   bool status = false;

   // Check for MET_PYTHON_EXE
   user_ppath = getenv(user_python_path_env);
   if(user_ppath != nullptr) {
      status = user_python_tc_diag(script_name, tmp_info);
   }
   // Use compiled python instance
   else {
      status = met_python_tc_diag(script_name, tmp_info);
   }

   return status;
}

////////////////////////////////////////////////////////////////////////

static bool met_python_tc_diag(const ConcatString &diag_script,
                               TmpFileInfo &tmp_info) {
   const char *method_name = "met_python_tc_diag() -> ";

   mlog << Debug(3) << "Running MET compile time Python instance ("
        << MET_PYTHON_BIN_EXE << ") to run Python diagnostics script ("
        << diag_script << " " << tmp_info.tmp_file << ").\n";

   // Reload the module if GP has already been initialized
   bool do_reload = GP.is_initialized;

   GP.initialize();

   // Start up the python interpreter
   if(PyErr_Occurred()) {
      PyErr_Print();
      mlog << Warning << "\n" << method_name
           << "an error occurred initializing python\n\n";
      return false;
   }

   // Set the arguments
   StringArray sa = diag_script.split(" ");
   ConcatString script_name(sa[0]);
   sa.insert(0, script_name.c_str()); // Kludge with PyConfig_SetArgv
   sa.add(tmp_info.tmp_file);

   // Add the tc_diag python directory to the path
   ConcatString command;
   command << "import sys; sys.path.append(\""
           << replace_path(python_tc_diag_dir)
           << "\")";
   run_python_string(command.text());

   // Set the global python arguments
   if(sa.n() > 0 && !GP.set_args(sa, method_name)) return false;

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
      mlog << Warning << "\n" << method_name
           << "an error occurred importing module \""
           << diag_script << "\"\n\n";
      return false;
   }

   if(!module_obj) {
      mlog << Warning << "\n" << method_name
           << "error running Python script \""
           << diag_script << "\"\n\n";
      return false;
   }

   // Parse the diagnostics from python
   return parse_python_diag_data(module_obj, tmp_info);
}

////////////////////////////////////////////////////////////////////////

static bool user_python_tc_diag(const ConcatString &diag_script,
                                TmpFileInfo &tmp_info) {
   const char *method_name = "user_python_tc_diag() -> ";

   mlog << Debug(3) << "Running user-specified Python instance (MET_PYTHON_EXE="
        << user_ppath << ") to run Python diagnostics script ("
        << diag_script << " " << tmp_info.tmp_file << ").\n";

   // Create a temp file
   const char *tmp_dir = getenv ("MET_TMP_DIR");
   if(!tmp_dir) tmp_dir = default_tmp_dir;

   ConcatString path;
   path << tmp_dir << '/'
        << tmp_diag_base_name;

   ConcatString tmp_file_name(make_temp_file_name(path.text(), nullptr));

   // Construct the system command
   ConcatString command;
   command << user_ppath                   << ' ' // user's path to python
           << replace_path(write_tmp_diag) << ' ' // write_tmp_diag.py
           << tmp_file_name                << ' ' // temporary output filename
           << diag_script                  << ' ' // python diagnostics script
           << tmp_info.tmp_file;                  // cylindrical coordinates tmp nc filename

   mlog << Debug(4) << "Writing temporary Python diagnostics file:\n\t"
        << command << "\n";

   int status = system(command.text());

   if(status) {
      mlog << Error << "\n" << method_name
           << "command \"" << command.text() << "\" failed ... status = "
           << status << "\n\n";
      exit(1);
   }

   // If the global python object has already been initialized,
   // we need to reload the module
   bool do_reload = GP.is_initialized;

   GP.initialize();

   // Start up the python interpreter
   if(PyErr_Occurred()) {
      PyErr_Print();
      mlog << Warning << "\n" << method_name
           << "an error occurred initializing python\n\n";
      return false;
   }

   // Set the arguments
   StringArray sa;
   sa.add(read_tmp_diag); // Kludge to use PyConfig_SetArgv
   sa.add(read_tmp_diag);
   sa.add(tmp_file_name);

   // Set the global python arguments
   if(!GP.set_args(sa, method_name)) return false;

   mlog << Debug(4) << "Reading temporary Python diagnostics data file: "
        << tmp_file_name << "\n";

   // Import the python wrapper script as a module
   path = get_short_name(read_tmp_diag);
   PyObject *module_obj = PyImport_ImportModule (path.text());

   // If needed, reload the module
   if(do_reload) {
      module_obj = PyImport_ReloadModule (module_obj);
   }

   if(PyErr_Occurred()) {
      PyErr_Print();
      mlog << Warning << "\n" << method_name
           << "an error occurred importing module "
           << "\"" << path << "\"\n\n";
      return false;
   }

   if(!module_obj) {
      mlog << Warning << "\n" << method_name
           << "error running python script\n\n";
      return false;
   }

   // Parse the diagnostics from python
   status = parse_python_diag_data(module_obj, tmp_info);

   // Cleanup
   remove_temp_file(tmp_file_name);

   return status;
}

////////////////////////////////////////////////////////////////////////

static bool parse_python_diag_data(PyObject *module_obj,
                                   TmpFileInfo &tmp_info) {
   const char *method_name = "parse_python_diag_data() -> ";
   bool status = true;

   // Get the namespace for the module (as a dictionary)
   PyObject *module_dict_obj = PyModule_GetDict(module_obj);

   if(!module_dict_obj || !PyDict_Check(module_dict_obj)) {
      mlog << Warning << "\n" << method_name
           << "python module is not a dictionary.\n\n";
      return false;
   }

   // Get the diag_data item
   PyObject *data_obj = PyDict_GetItem(module_dict_obj,
                           PyUnicode_FromString(diag_data_dict_name));

   if(!data_obj || !PyDict_Check(data_obj)) {
      mlog << Warning << "\n" << method_name
           << "trouble parsing the \"" << diag_data_dict_name
           << "\" python dictionary.\n\n";
      return false;
   }

   // Storm data
   if(status) status = parse_python_string_value_map(
                          data_obj,
                          storm_data_dict_name,
                          tmp_info.diag_storm_keys,
                          tmp_info.diag_storm_map);

   // Sounding data
   if(status) status = parse_python_string_value_map(
                          data_obj,
                          sounding_data_dict_name,
                          tmp_info.diag_sounding_keys,
                          tmp_info.diag_sounding_map);

   // Custom data
   if(status) status = parse_python_string_value_map(
                          data_obj,
                          custom_data_dict_name,
                          tmp_info.diag_custom_keys,
                          tmp_info.diag_custom_map);

   // Units
   if(status) status = parse_python_string_string_map(
                          data_obj,
                          units_dict_name,
                          tmp_info.diag_units_map);

   // Long names
   if(status) status = parse_python_string_string_map(
                          data_obj,
                          long_name_dict_name,
                          tmp_info.diag_long_name_map);

   // Comments
   string str;
   if(status) status = parse_python_string(
                          data_obj,
                          comments_item_name,
                          str);

   ConcatString cs(str);
   StringArray sa = cs.split("\n");

   // Skip COMMENTS title line
   for(int i=0; i<sa.n(); i++) {
      if(sa[i].find("COMMENTS") == string::npos) {
         tmp_info.comment_lines.add(sa[i]);
      }
   }

   return status;
}

////////////////////////////////////////////////////////////////////////

static bool parse_python_string_value_map(PyObject *dict,
                                          const char *name,
                                          vector<string> &k,
                                          map<string,double> &m) {

   const char *method_name = "parse_python_string_value_map() -> ";

   PyObject *val_obj = nullptr;
   PyObject *key_obj = nullptr;
   int status;
   double val;
   long pos;

   PyObject *data_obj = PyDict_GetItem(dict,
                           PyUnicode_FromString(name));

   if(!data_obj || !PyDict_Check(data_obj)) {
      mlog << Warning << "\n" << method_name
           << "trouble parsing the \"" << name
           << "\" python dictionary.\n\n";
      return false;
   }

   // Initialize
   pos = 0;

   // Loop through the dictionary entries
   while((status = PyDict_Next(data_obj, &pos, &key_obj, &val_obj)) != 0) {

      // All keys must be strings
      if(!PyUnicode_Check(key_obj)) {
         mlog << Error << "\n" << method_name
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
         mlog << Error << "\n" << method_name
              << "value for \"" << key_str
              << "\" is not a numeric python data type!\n\n";
         exit(1);
      }

      // Check for duplicates
      if(m.count(key_str) > 0) {
         mlog << Warning << "\n" << method_name
              << "ignoring duplicate entries for \""
              << key_str << "\" = " << val << "!\n\n";
      }
      // Store key/value pair in the dictionary
      else {
         mlog << Debug(5) << "Adding to map \""
              << key_str << "\" = " << val << "\n";
         k.emplace_back(key_str);
         m[key_str] = val;
      }
   } // end while

   return true;
}

////////////////////////////////////////////////////////////////////////

static bool parse_python_string_string_map(PyObject *dict,
                                           const char *name,
                                           map<string,string> &m) {

   const char *method_name = "parse_python_string_string_map() -> ";

   PyObject *key_obj = nullptr;
   PyObject *val_obj = nullptr;
   long pos;

   PyObject *data_obj = PyDict_GetItem(dict,
                           PyUnicode_FromString(name));

   if(!data_obj || !PyDict_Check(data_obj)) {
      mlog << Warning << "\n" << method_name
           << "trouble parsing the \"" << name
           << "\" python dictionary.\n\n";
      return false;
   }

   // Initialize
   pos = 0;

   // Loop through the dictionary entries
   while(0 != PyDict_Next(data_obj, &pos, &key_obj, &val_obj)) {

      // All keys and values must be strings
      if(!PyUnicode_Check(key_obj) || !PyUnicode_Check(val_obj)) {
         mlog << Error << "\n" << method_name
              << "key or value is not a string!\n\n";
         exit(1);
      }

      // Parse key and value as strings
      string key_str = PyUnicode_AsUTF8(key_obj);
      string val_str = PyUnicode_AsUTF8(val_obj);

      // Check for duplicates
      if(m.count(key_str) > 0) {
         mlog << Warning << "\n" << method_name
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

   return true;
}

////////////////////////////////////////////////////////////////////////

static bool parse_python_string(PyObject *dict,
                                const char *name, string &s) {

   const char *method_name = "parse_python_string() -> ";

   PyObject *data_obj = PyDict_GetItem(dict,
                           PyUnicode_FromString(name));

   if(!data_obj || !PyUnicode_Check(data_obj)) {
      mlog << Warning << "\n" << method_name
           << "trouble parsing the \"" << name
           << "\" python string.\n\n";
      return false;
   }

   // Store the string
   s = PyUnicode_AsUTF8(data_obj);

   return true;
}

////////////////////////////////////////////////////////////////////////
