// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __PYTHON_DATAPLANE__
#define  __PYTHON_DATAPLANE__


////////////////////////////////////////////////////////////////////////


static const char xarray_dataarray_name [] = "met_data";

static const char numpy_array_name      [] = "met_data";
static const char numpy_dict_name       [] = "attrs";


////////////////////////////////////////////////////////////////////////


#include "data_plane.h"
#include "var_info_python.h"


extern "C" {

#include "Python.h"

}


////////////////////////////////////////////////////////////////////////


extern bool python_dataplane(const char * script_name,
                             int script_argc, char **script_argv,
                             const bool use_xarray,
                             DataPlane & dp_out, Grid & g_out,
                             VarInfoPython &vinfo);


////////////////////////////////////////////////////////////////////////


#endif   /*  __PYTHON_DATAPLANE__  */


////////////////////////////////////////////////////////////////////////


