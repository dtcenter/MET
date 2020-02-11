// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MPR_PYTHON_HANDLER_H__
#define  __MPR_PYTHON_HANDLER_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

extern "C" {

#include "Python.h"

}


#include "data_line.h"
#include "parse_stat_line.h"


////////////////////////////////////////////////////////////////////////


class PyLineDataFile : public LineDataFile {

   protected:

   public:

      PyLineDataFile();
     ~PyLineDataFile();



};


////////////////////////////////////////////////////////////////////////


   //
   //  populates a single MPRData struct from a python list record
   //

extern void mpr_populate(PyObject * list_in, MPRData & mpr_out);   


////////////////////////////////////////////////////////////////////////


#endif   /*  __MPR_PYTHON_HANDLER_H__  */


////////////////////////////////////////////////////////////////////////


