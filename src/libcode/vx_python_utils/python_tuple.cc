// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_log.h"

#include "python_tuple.h"


////////////////////////////////////////////////////////////////////////


void get_tuple_int_values(PyObject * tuple, int & dim, int * values)

{

dim = PyTuple_Size (tuple);


if ( dim > max_tuple_data_dims )  {

   mlog << Error
        << "\n\n"
        << "increase parameter \"max_tuple_data_dims\" to at least "
        << dim << "\n\n";

   exit ( 1 );

}


int j;
PyObject * item = 0;

for (j=0; j<dim; ++j)  {

   item = PyTuple_GetItem (tuple, j);

   values[j] = (int) PyInt_AsLong (item);

}


return;

}


////////////////////////////////////////////////////////////////////////



