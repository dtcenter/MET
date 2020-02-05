// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#include "vx_python3_utils.h"

#include "dataplane_from_numpy_array.h"
#include "dataplane_from_xarray.h"
#include "grid_from_python_dict.h"


////////////////////////////////////////////////////////////////////////


static const char  data_attr_name [] = "values";
static const char attrs_attr_name [] = "attrs";


////////////////////////////////////////////////////////////////////////

   //
   //  we just grab the numpy array and the attributes dictionary
   //
   //    from the xarray DataArray object, and then hand them
   //
   //    off to dataplane_from_numpy_array
   //

bool dataplane_from_xarray(PyObject * data_array, DataPlane & dp_out, Grid & grid_out, VarInfoPython &vinfo)

{

DataPlane dp;
PyObject * numpy_array = 0;
PyObject * attrs_dict  = 0;


numpy_array = PyObject_GetAttrString(data_array, data_attr_name);

attrs_dict  = PyObject_GetAttrString(data_array, attrs_attr_name);


   /////////////////////

Python3_Numpy np;

np.set(numpy_array);

bool status = dataplane_from_numpy_array(np, attrs_dict, dp_out, grid_out, vinfo);

   //
   //  done
   //

return ( status );

}


////////////////////////////////////////////////////////////////////////

