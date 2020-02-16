// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __PYTHON_LINE_H__
#define  __PYTHON_LINE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

extern "C" {
#include "Python.h"
}

#include "vx_python3_utils.h"
#include "data_line.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Makes it look to the MET code as if the python data
   //
   //    is just a plain ascii text file, returning a 
   //
   //    single data line at a time.
   //


////////////////////////////////////////////////////////////////////////


class PyLineDataFile : public LineDataFile {

   protected:

      ConcatString UserScriptPath;

      StringArray UserScriptArgs;

      ConcatString UserPathToPython;

      void do_straight ();   //  straight-up python, no pickle
      void do_pickle   ();   //  pickle

      ConcatString make_header_line () const;
      ConcatString make_data_line   ();

   public:

      PyLineDataFile();
     ~PyLineDataFile();


      bool open(const char * user_script_path, const StringArray & user_script_args);

      void close();


      Python3_Script * script;


      bool first_call;

      int index;   //  list index of last line returned by next_line()

      int N;       //  # of elements in list

      PyObject * main_list;   //  not allocated

      // int operator>>(DataLine &);   //  virtual from base class

      bool is_header() const;   //  virtual from base class

      bool next_line(ConcatString & s_out);

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __PYTHON_LINE_H__  */


////////////////////////////////////////////////////////////////////////


