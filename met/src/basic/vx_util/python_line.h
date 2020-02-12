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

#include "data_line.h"


////////////////////////////////////////////////////////////////////////


class PyLineDataFile : public LineDataFile {

   protected:

      ConcatString UserScriptPath;

      void do_straight ();   //  straight-up python, no pickle
      void do_pickle   ();   //  pickle

      ConcatString make_header_line () const;
      ConcatString make_data_line   ();

   public:

      PyLineDataFile();
     ~PyLineDataFile();


      bool open(const char * user_script_path);

      void close();



      bool first_call;

      int index;   //  index into the list

      int N;       //  # of elements in list

      PyObject * main_list;   //  not allocated

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __PYTHON_LINE_H__  */


////////////////////////////////////////////////////////////////////////


