// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __GLOBAL_PYTHON__
#define  __GLOBAL_PYTHON__


////////////////////////////////////////////////////////////////////////


extern "C" {

#include "Python.h"

}


////////////////////////////////////////////////////////////////////////


#include "wchar_argv.h"
#include "python3_util.h"
#include "concat_string.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


class GlobalPython {

   public:

      GlobalPython();
     ~GlobalPython();

      void initialize();
      bool set_args(const StringArray &, const char *);
      void finalize();

      PyConfig config;
      bool is_initialized;

};

inline GlobalPython::GlobalPython()   { is_initialized = false; };
inline GlobalPython::~GlobalPython()  { is_initialized = false; };


////////////////////////////////////////////////////////////////////////


inline void GlobalPython::initialize()
{ 

if ( ! is_initialized )  {

   mlog << Debug(3) << "Initializing MET compile time Python instance: " << MET_PYTHON_BIN_EXE << "\n";

   wchar_t *python_path = Py_DecodeLocale(MET_PYTHON_BIN_EXE, nullptr);
   PyConfig_InitPythonConfig(&config);
   PyStatus status = PyConfig_SetString(&config, &config.program_name, python_path);
   if (! PyStatus_Exception(status)) {
      status = Py_InitializeFromConfig(&config);
   }
   if (PyStatus_Exception(status)) {
      PyConfig_Clear(&config);
      Py_ExitStatusException(status);
   }

   is_initialized = true;

   //
   //  add MET-specific python directories to the path
   //

   ConcatString command;
   command << "import sys; sys.path.append(\""
           << replace_path(pyembed_dir)
           << "\");"
           << "sys.path.append(\""
           << replace_path(python_dir)
           << "\")";

   run_python_string(command.text());

}

return;

}


////////////////////////////////////////////////////////////////////////


inline bool GlobalPython::set_args(const StringArray &args,
                                   const char *caller)
{ 

Wchar_Argv wa;
wa.set(args);

   //
   //  append script location to the Python system path
   //

if(args.n() > 0) {
   ConcatString script_name(args[0]);
   ConcatString command;
   command << "import sys; sys.path.append(\""
           << script_name.dirname() << "\");";
   run_python_string(command.c_str());
}

   //
   //  add arguments to Python configuration
   //

PyStatus p_status = PyConfig_SetArgv(&(this->config), wa.wargc(), wa.wargv());
if (PyStatus_Exception(p_status)) {
   PyConfig_Clear(&(this->config));
   mlog << Warning << "\n" << caller 
        << "error setting python arguments\n\n";
   return false;
}

   //
   //  MET #3219 store the current Python system path
   //  since it is reset by the Python 3.10 version
   //  of Py_InitializeFromConfig()
   //

PyObject *sys_path_obj = PySys_GetObject("path");
StringArray sys_path_sa(pyobject_as_string_array(sys_path_obj));

   //
   //  re-initialize Python interpreter with arguments set 
   //

Py_InitializeFromConfig(&(this->config));

   //
   //  MET #3219 restore the Python system path from above
   //

ConcatString command;
command << "import os; import sys; ";
command << "sys.path.clear(); ";
for(int i=0; i<sys_path_sa.n(); i++) {
   command << "sys.path.append(\"" << sys_path_sa[i] << "\"); ";
}
run_python_string(command.c_str());

return true;

}


////////////////////////////////////////////////////////////////////////


inline void GlobalPython::finalize()
{ 

if ( is_initialized )  {

   Py_Finalize();

   is_initialized = false;

}

return;

}


////////////////////////////////////////////////////////////////////////


extern GlobalPython GP;


////////////////////////////////////////////////////////////////////////


#endif   /*  __GLOBAL_PYTHON__  */


////////////////////////////////////////////////////////////////////////

