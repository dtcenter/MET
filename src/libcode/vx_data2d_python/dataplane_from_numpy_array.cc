// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#include "vx_python_utils.h"

#include "dataplane_from_numpy_array.h"
#include "grid_from_python_dict.h"


////////////////////////////////////////////////////////////////////////

   //
   //  2D numpy arrays seem to store things in row-major order
   //

inline void nympy_array_one_to_two(const int n, const int Ncols, int & row, int & col)

{

row = n/Ncols;

col = n%Ncols;

return;

}


////////////////////////////////////////////////////////////////////////


void dataplane_from_numpy_array(PyObject * numpy_array, PyObject * attrs_dict, DataPlane & dp_out, Grid & grid_out)

{

int j;
int dim, nrows, ncols, nx, ny, Nxy;
int r, c, x, y;
double value;
int sizes [max_tuple_data_dims];
void * p  = 0;
double * pp = 0;

   //
   //  get size of data array
   //

PyObject * shape_tuple = PyObject_GetAttrString (numpy_array, "shape");

dim = PyTuple_Size (shape_tuple);

if ( dim > max_tuple_data_dims )  {

   mlog << Error
        << "dataplane_from_numpy_array() -> too many dimensions in data ... "
        << dim << "\n\n";

   exit ( 1 );

}

get_tuple_int_values(shape_tuple, dim, sizes);

if ( dim != 2 )  {

   mlog << Error
        << "dataplane_from_numpy_array() -> can only handle 2-dimensional data, but given data is  "
        << dim << "-dimensional\n\n";

   exit ( 1 );

}

nrows = sizes[0];
ncols = sizes[1];

nx = ncols;
ny = nrows;

Nxy = nx*ny;

dp_out.set_size(nx, ny);

   //
   //  get a pointer to the actual data
   //

PyObject * the_data = PyObject_GetAttrString (numpy_array, "data");

PyTypeObject * type = (PyTypeObject *) PyObject_Type (the_data);


if ( type->tp_as_buffer->bf_getreadbuffer (the_data, 0, &p) < 0 )  {

   mlog << Error
        << "dataplane_from_numpy_array() -> getreadbufferproc errored out\n\n";

   // Py_Finalize();

   exit ( 1 );

}

pp = (double *) p;

for (j=0; j<Nxy; ++j)  {

   nympy_array_one_to_two(j, ncols, r, c);

   x = c;

   y = ny - 1 - r;

   // value = 0.0;
   value = pp[j];

   dp_out.set(value, x, y);

}   //  for j

   //
   //  get timestamp info from the attributes dictionary
   //
   //    init and valid times are strings like YYYYMMDD_HHMMSS
   //

ConcatString s;
unixtime t;


s = dict_lookup_string(attrs_dict, "init");

t = yyyymmdd_hhmmss_to_unix(s);

dp_out.set_init(t);

     ////////////////////

s = dict_lookup_string(attrs_dict, "valid");

t = yyyymmdd_hhmmss_to_unix(s);

dp_out.set_valid(t);

     ////////////////////

j = dict_lookup_int(attrs_dict, "lead");

j = 3600*(j/10000) + 60*((j/100)%100) + j%100;

dp_out.set_lead(j);

     ////////////////////

s = dict_lookup_string(attrs_dict, "accum");

t = hhmmss_to_sec(s);

dp_out.set_accum(t);

     ////////////////////

PyObject * py_grid = dict_lookup_dict(attrs_dict, "grid");

grid_from_python_dict(py_grid, grid_out);


     ////////////////////

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////



