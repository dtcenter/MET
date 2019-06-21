// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

#include "global_python.h"


////////////////////////////////////////////////////////////////////////


GlobalPython GP;   //  this needs external linkage


////////////////////////////////////////////////////////////////////////


static const char python_path_env [] = "PYTHONPATH";


////////////////////////////////////////////////////////////////////////


bool python_dataplane(const char * script_name,
                      int script_argc, char ** script_argv,
                      const bool use_xarray, DataPlane & met_dp_out,
                      Grid & met_grid_out, VarInfoPython &vinfo)

{

PyObject * module      = 0;
PyObject * module_dict = 0;
PyObject * key         = 0;
PyObject * numpy_array = 0;
PyObject * attrs_dict  = 0;

   //
   //  if the global python object has already been initialized,
   //  we need to reload the module
   //

bool do_reload = GP.is_initialized;

GP.initialize();

   //
   //   start up the python interpreter
   //

if ( PyErr_Occurred() )  {

   mlog << Warning << "\npython_dataplane() -> "
        << "an error occurred initializing python\n\n";

   return ( false );

}

   //
   //  read the PYTHONPATH environment variable
   //

ConcatString python_path;
get_env("PYTHONPATH", python_path);

   //
   //  set the arguments
   //

if ( script_argc > 0 )  {

   PySys_SetArgv (script_argc, script_argv);

}

   //
   //  import the python script as a module
   //

module = PyImport_ImportModule (script_name);

   //
   //  if needed, reload the module
   //

if ( do_reload )  {

   module = PyImport_ReloadModule (module);

}

if ( PyErr_Occurred() )  {

   mlog << Warning << "\npython_dataplane() -> "
        << "an error occurred importing module \"" << python_path
        << "/" << script_name << ".py\"\n\n";

   return ( false );

}

if ( ! module )  {

   mlog << Warning << "\npython_dataplane() -> "
        << "error running python script \"" << python_path
        << "/" << script_name << ".py\"\n\n";

   return ( false );

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

   if ( !data_array )  {

      mlog << Warning << "\npython_dataplane() -> "
           << "trouble reading data from \"" << python_path
           << "/" << script_name << ".py\"\n\n";

      return ( false );
   }

   dataplane_from_xarray(data_array, met_dp_out, met_grid_out, vinfo);

} else {    //  numpy array & dict

      //   look up the data array variable name from the dictionary

   key = PyString_FromString (numpy_array_name);

   numpy_array = PyDict_GetItem (module_dict, key);

   key = PyString_FromString (numpy_dict_name);

   attrs_dict = PyDict_GetItem (module_dict, key);

   if ( !numpy_array || !attrs_dict )  {

      mlog << Warning << "\npython_dataplane() -> "
           << "trouble reading data from \"" << python_path
           << "/" << script_name << ".py\"\n\n";

      return ( false );
   }

   dataplane_from_numpy_array(numpy_array, attrs_dict, met_dp_out, met_grid_out, vinfo);

}

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////



