// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>

#include "vx_log.h"
#include "vx_math.h"

#include "vx_python3_utils.h"
#include "python_handler.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PythonHandler
   //


////////////////////////////////////////////////////////////////////////


PythonHandler::PythonHandler(const string &program_name) : FileHandler(program_name)

{



}


////////////////////////////////////////////////////////////////////////


PythonHandler::~PythonHandler()

{


}


////////////////////////////////////////////////////////////////////////


bool PythonHandler::isFileType(LineDataFile &ascii_file) const

{

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool PythonHandler::_readObservations(LineDataFile &ascii_file)

{

mlog << Error
     << "\n\n  bool PythonHandler::_readObservations(LineDataFile &) -> should never be called!\n\n";

exit ( 1 );


return ( false );

}


////////////////////////////////////////////////////////////////////////


void PythonHandler::load_python_obs(PyObject * obj)

{

if ( ! PyList_Check(obj) )  {

   mlog << Error
        << "\n\n  PythonHandler::load_python_obs(PyObject *) -> given object not a list!\n\n";

   exit ( 1 );

}

int j;
PyObject * a = 0;
Python3_List list(obj);
Observation obs;

for (j=0; j<(list.size()); ++j)  {

   a = list[j];

   if ( ! PyList_Check(obj) )  {

      mlog << Error
           << "\n\n  PythonHandler::load_python_obs(PyObject *) -> non-list object found in main list!\n\n";

      exit ( 1 );

   }

   obs.set(a);

   _addObservations(obs);

}   //  for j



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void PythonHandler::read_obs_from_script (const char * script_name, const char * variable_name)

{

Python3_Script script(script_name);

PyObject * obj = script.lookup(variable_name);


load_python_obs(obj);


return;

}


////////////////////////////////////////////////////////////////////////


