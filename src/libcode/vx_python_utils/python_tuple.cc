


////////////////////////////////////////////////////////////////////////


using namespace std;


////////////////////////////////////////////////////////////////////////


#include <iostream>


#include "python_tuple.h"


////////////////////////////////////////////////////////////////////////


void get_tuple_int_values(PyObject * tuple, int & dim, int * values)

{

dim = PyTuple_Size (tuple);


if ( dim > max_tuple_data_dims )  {

   cerr << "\n\n"
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



