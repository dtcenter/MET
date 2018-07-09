// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#include "data_plane.h"

#include "grid_from_python_dict.h"
#include "dataplane_from_xarray.h"
#include "dataplane_from_numpy_array.h"
#include "python_dataplane.h"


////////////////////////////////////////////////////////////////////////


void python_dataplane(const char * script_name, const bool use_xarray, DataPlane & met_dp_out, Grid & met_grid_out)

{

PyObject * module      = 0;
PyObject * module_dict = 0;
PyObject * key         = 0;
PyObject * numpy_array = 0;
PyObject * attrs_dict  = 0;


   //
   //   start up the python interpreter
   //

Py_Initialize();

   //
   //   import the python script as a module
   //

   module = PyImport_ImportModule        (script_name);
// module = PyImport_ImportModuleNoBlock (script_name);

if ( PyErr_Occurred() )  {

   mlog << Error
        << "\n\n   an error occurred\n\n";

   PyErr_PrintEx(1);

   exit ( 1 );

}

if ( ! module )  {

   mlog << Error
        << "python_dataplane() -> error running python script \""
        << script_name << "\"\n\n";

   Py_Finalize();

   exit ( 1 );

}

   //
   //   get the namespace for the module (as a dictionary)
   //

module_dict = PyModule_GetDict (module);

   //
   //  get handles to the objects of interest from the module_dict
   //

if ( use_xarray )  {

   PyObject * data_array = 0;

      //   look up the data array variable name from the dictionary

   key = PyString_FromString (xarray_dataarray_name);

   data_array = PyDict_GetItem (module_dict, key);

   dataplane_from_xarray(data_array, met_dp_out, met_grid_out);

} else {    //  numpy array & dict

      //   look up the data array variable name from the dictionary

   key = PyString_FromString (numpy_array_name);
   
   numpy_array = PyDict_GetItem (module_dict, key);

   key = PyString_FromString (numpy_dict_name);
   
   attrs_dict = PyDict_GetItem (module_dict, key);

   dataplane_from_numpy_array(numpy_array, attrs_dict, met_dp_out, met_grid_out);

}



   //
   //  done
   //

Py_Finalize();

return;

}


////////////////////////////////////////////////////////////////////////



