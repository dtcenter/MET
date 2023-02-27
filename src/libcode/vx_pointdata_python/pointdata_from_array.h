// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_POINTDATA_FROM_ARRAY_H__
#define  __MET_POINTDATA_FROM_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include "met_point_data.h"

#include "python3_dict.h"
#include "python3_numpy.h"


extern "C" {

#include "Python.h"

}


////////////////////////////////////////////////////////////////////////

static const char  data_attr_name [] = "values";

////////////////////////////////////////////////////////////////////////


extern bool pointdata_from_np_array(Python3_Numpy & np, int * data_out);
extern bool pointdata_from_np_array(Python3_Numpy & np, float * data_out);
extern bool pointdata_from_np_array(Python3_Numpy & np, IntArray *data_out);
extern bool pointdata_from_np_array(Python3_Numpy & np, NumArray *data_out);
extern bool pointdata_from_str_array(PyObject *data_array, StringArray *data_out);

extern bool pointdata_from_python_list(PyObject *data_array, int *data_out);
extern bool pointdata_from_python_list(PyObject *data_array, float *data_out);
extern bool pointdata_from_python_list(PyObject *data_array, IntArray *data_out);
extern bool pointdata_from_python_list(PyObject *data_array, NumArray *data_out);

////////////////////////////////////////////////////////////////////////

#include "pointdata_from_array.hpp"

////////////////////////////////////////////////////////////////////////

#endif   /*  __MET_POINTDATA_FROM_NUMPY_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////

