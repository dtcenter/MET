// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_PYTHON_DICT_H__
#define  __MET_PYTHON_DICT_H__


////////////////////////////////////////////////////////////////////////


extern "C" {

#include "Python.h"

}


////////////////////////////////////////////////////////////////////////


extern void dump_dict(PyObject * obj, int depth);

extern void dump_dict_value(PyObject * value, int depth);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_PYTHON_DICT_H__  */


////////////////////////////////////////////////////////////////////////



