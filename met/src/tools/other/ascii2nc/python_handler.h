// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

#include "file_handler.h"


////////////////////////////////////////////////////////////////////////


class PythonHandler : public FileHandler

{

   public:

      PythonHandler(const string &program_name);
      virtual ~PythonHandler();

      bool isFileType(LineDataFile &ascii_file) const;
  
      static string getFormatString() { return "python_point"; }

   protected:  

         // Read the observations from the given file and add them to the
         // _observations vector.

      virtual bool _readObservations(LineDataFile &ascii_file);   //  this shouldn't be called

      void load_python_obs(PyObject *);   //  python object is list of lists


      void read_obs_from_script (const char * script_name, const char * variable_name);

      void read_obs_from_pickle (const char * pickle_name, const char * variable_name);
  
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __PYTHON_HANDLER_H__  */


////////////////////////////////////////////////////////////////////////


