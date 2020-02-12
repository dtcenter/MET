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

   public:

      PyLineDataFile();
     ~PyLineDataFile();


      int index;   //  index into the list

      int N;   //  # of elements in list

      PyObject * main_list;   //  not allocated

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __PYTHON_LINE_H__  */


////////////////////////////////////////////////////////////////////////


