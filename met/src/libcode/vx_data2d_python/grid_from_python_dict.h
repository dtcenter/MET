// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_GRID_FROM_PYTHON_DICT_H__
#define  __MET_GRID_FROM_PYTHON_DICT_H__


////////////////////////////////////////////////////////////////////////


#include "vx_grid.h"


extern "C" {

#include "Python.h"

}


////////////////////////////////////////////////////////////////////////


extern void grid_from_python_dict(PyObject * dict, Grid & grid_out);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_GRID_FROM_PYTHON_DICT_H__  */


////////////////////////////////////////////////////////////////////////


