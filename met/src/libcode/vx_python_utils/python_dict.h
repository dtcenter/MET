// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_PYTHON_DICT_H__
#define  __MET_PYTHON_DICT_H__


////////////////////////////////////////////////////////////////////////


#include "concat_string.h"


////////////////////////////////////////////////////////////////////////


extern "C" {

#include "Python.h"

}


////////////////////////////////////////////////////////////////////////


   //
   //  convenience functions for looking things up in a python dictionary
   //


extern  int           dict_lookup_int    (PyObject * dict, const char * key);
extern  double        dict_lookup_double (PyObject * dict, const char * key);
extern  ConcatString  dict_lookup_string (PyObject * dict, const char * key);
extern  PyObject *    dict_lookup_dict   (PyObject * dict, const char * key);


////////////////////////////////////////////////////////////////////////

   //
   //  dumping dictionary contents to the screen
   //

extern void dump_dict(PyObject * obj, int depth);

extern void dump_dict_value(PyObject * value, int depth);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_PYTHON_DICT_H__  */


////////////////////////////////////////////////////////////////////////



