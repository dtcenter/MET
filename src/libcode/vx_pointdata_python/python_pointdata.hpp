// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __PYTHON_POINTDATA_HPP__
#define  __PYTHON_POINTDATA_HPP__

#include "vx_python3_utils.h"

////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

template <typename T>
static void set_array_from_python(PyObject *python_data, const char *python_key, T *out, bool required=true) {
   const char *method_name = "set_array_from_python(T *) -> ";
   PyObject *numpy_array_obj = PyDict_GetItemString (python_data, python_key);
   if (numpy_array_obj) {
      bool status = false;
      ConcatString py_type_name = Py_TYPE(numpy_array_obj)->tp_name;
      if ("numpy.ndarray" == py_type_name || "MaskedArray" == py_type_name ){
         Python3_Numpy np;
         np.set(numpy_array_obj);
         pointdata_from_np_array(np, out);
         status = true;
      }
      else if (PyList_Check(numpy_array_obj)) {
         status = pointdata_from_python_list(numpy_array_obj, out);
         if(!status) {
            if (PyFloat_Check(PyList_GetItem(numpy_array_obj, 0))) {
               mlog << Error << "\n" << method_name
                    << "Only int type is supported at python list for " << python_key << "."
                    << " Check the data type from python and consider using numpy for python embedding\n\n";
            }
            else {
               mlog << Error << "\n" << method_name
                    << "Only int/float type is supported at python list for " << python_key << "."
                    << " Check the data type from python and consider using numpy for python embedding\n\n";
            }
         }
      }
      else {
         mlog << Error << "\n" << method_name
              << "Not getting the point data by the key (" << python_key << ") from python object\n"
              << "          The python type \"" << py_type_name << "\" is not supported\n\n";
      }
      if (status) {
         mlog << Debug(7) << method_name
              << "get the point data for " << python_key << " from python object\n";
      }
      else exit (1);
   }
   else {
      if (required) {
         mlog << Error << "\n" << method_name
              << "error getting the point data by the key (" << python_key << ") from python object\n\n";
         exit (1);
      }
      else mlog << Debug(3) << method_name
                << "not exists the point data (" << python_key << ") from python object\n";
   }
}


////////////////////////////////////////////////////////////////////////


#endif   /*  __PYTHON_POINTDATA_HPP___  */


////////////////////////////////////////////////////////////////////////

