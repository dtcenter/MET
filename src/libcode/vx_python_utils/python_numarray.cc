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

#include "python_numarray.h"
#include "python_tuple.h"


////////////////////////////////////////////////////////////////////////


void dump_numarray(PyObject * na)

{

int j, N, dim;
long value;
void * p  = 0;
long * pp = 0;
int values[max_tuple_data_dims];


   //
   //   get the size of the array
   //

PyObject * shape_tuple = PyObject_GetAttrString (na, "shape");

get_tuple_int_values(shape_tuple, dim, values);

cout << "data is " << dim << "-dimensional\n";

cout << "data shape is ";

for (j=0; j<dim; ++j)  {

   cout << values[j];

   if ( j < (dim - 1) )  cout << " x ";

}

cout << '\n';

N = 1;

for (j=0; j<dim; ++j)  N *= (values[j]);

   //
   //  get a pointer to the actual data
   //

PyObject * the_data = PyObject_GetAttrString (na, "data");

PyTypeObject * type = (PyTypeObject *) PyObject_Type (the_data);


if ( type->tp_as_buffer->bf_getreadbuffer (the_data, 0, &p) < 0 )  {

   cout << "\n\n  getreadbufferproc errored out\n\n";

   Py_Finalize();

   exit ( 1 );

}

   //
   //  loop through the elements of the list and print them out
   //
   //    ... seems to be row-major order
   //

pp = (long *) p;

for (j=0; j<N; ++j)  {

   value = pp[j];

   cout << "j = " << j << " ... value = " << value << "\n";

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


