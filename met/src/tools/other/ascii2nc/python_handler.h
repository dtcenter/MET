// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __PYTHON_HANDLER_H__
#define  __PYTHON_HANDLER_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

extern "C" {

#include "Python.h"

}


#include "concat_string.h"
#include "string_array.h"

#include "file_handler.h"


////////////////////////////////////////////////////////////////////////


class PythonHandler : public FileHandler

{

   public:

      PythonHandler(const string &program_name);
      PythonHandler(const char * program_name, const char * ascii_filename);
      virtual ~PythonHandler();

      bool isFileType(LineDataFile &ascii_file) const;
  
      static string getFormatString() { return "python"; }


      bool use_pickle;

      ConcatString user_path_to_python;   //  if we're using pickle

      ConcatString user_script_filename;

      StringArray user_script_args;

   protected:  


         // Read the observations from the given file and add them to the
         // _observations vector.

      virtual bool _readObservations(LineDataFile &ascii_file);   //  this shouldn't be called

      virtual bool readAsciiFiles(const vector< ConcatString > &ascii_filename_list);

      bool do_pickle   ();
      bool do_straight ();   //  straight-up python, no pickle

      void load_python_obs(PyObject *);   //  python object is list of lists


      bool read_obs_from_script (const char * script_name, const char * variable_name);

      bool read_obs_from_pickle (const char * pickle_name, const char * variable_name);
  
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __PYTHON_HANDLER_H__  */


////////////////////////////////////////////////////////////////////////


