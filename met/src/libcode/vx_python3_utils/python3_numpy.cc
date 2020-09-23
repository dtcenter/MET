

////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_log.h"
#include "empty_string.h"

#include "python3_numpy.h"
#include "python3_util.h"


////////////////////////////////////////////////////////////////////////


static const char junk_var_name [] = "__junk__";


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Python3_Numpy
   //


////////////////////////////////////////////////////////////////////////


Python3_Numpy::Python3_Numpy()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Python3_Numpy::~Python3_Numpy()

{

if ( Object )  PyBuffer_Release(&View);

clear();

}


////////////////////////////////////////////////////////////////////////


void Python3_Numpy::clear()

{

Object = 0;

Name.clear();

if ( Dim )  { delete [] Dim;  Dim = 0; }

N_Dims = 0;

Data_Obj = 0;

N_Data = 0;

Buffer = 0;

Item_Size = 0;

Script = 0;

Dtype.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Python3_Numpy::init_from_scratch()

{

Dim = 0;

Buffer = 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Python3_Numpy::dump(ostream & out) const

{

int j;

out << "name      = \"" << Name.contents() << "\"\n";
out << "n dims    = " << n_dims() << '\n';

out << "dims      = [";

for (j=0; j<N_Dims; ++j)  out << ' ' << Dim[j];

out << " ]\n";

out << "item size = " << item_size() << '\n';

out << "n data    = " << n_data() << '\n';

out << "dtype     = \"" << dtype() << "\"\n";





   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


int Python3_Numpy::dim(int k) const

{

if ( (k < 0) || (k >= N_Dims) )  {

   mlog << Error << "\nPython3_Numpy::dim(int) -> "
        << "range check error!\n\n";

   exit ( 1 );

}


return ( Dim[k] );

}


////////////////////////////////////////////////////////////////////////


void Python3_Numpy::set_name(const char * _name)

{

if ( empty(_name) )  {

   mlog << Error << "\nPython3_Numpy::set_name(const char *) -> "
        << "empty string!\n\n";

   exit ( 1 );

}

Name = _name;

return;

}


////////////////////////////////////////////////////////////////////////


void Python3_Numpy::set_script(Python3_Script & s)

{

Script = &s;

return;

}


////////////////////////////////////////////////////////////////////////


void Python3_Numpy::set(Python3_Script & script, const char * _name)

{

PyObject * obj = script.lookup(_name);

if ( ! obj )  {

   mlog << Error << "\nPython3_Numpy::set(Python3_Script &, const char *) -> "
        << "variable named \"" << _name << "\" not found in script \""
        << script.filename() << "\"\n\n";

   exit ( 1 );

}

set(obj);

set_name(_name);

return;

}


////////////////////////////////////////////////////////////////////////


void Python3_Numpy::set(PyObject * obj)

{

clear();

if ( ! obj )  {

   mlog << Error << "\nPython3_Numpy::set(PyObject *) -> "
        << "null object!\n\n";

   exit ( 1 );

}

Object = obj;

Script = 0;


   //
   //  get the dimensions
   //

PyObject * shape_tuple = PyObject_GetAttrString (Object, "shape");

N_Dims = PyTuple_Size (shape_tuple);

Dim = new int [N_Dims];

int j;
PyObject * item = 0;

for (j=0; j<N_Dims; ++j)  {

   item = PyTuple_GetItem (shape_tuple, j);

   Dim[j] = (int) PyLong_AsLong (item);

}

N_Data = 1;

for (j=0; j<N_Dims; ++j)  N_Data *= Dim[j];


Data_Obj = PyObject_GetAttrString (Object, "data");

if ( !PyObject_CheckBuffer(Data_Obj) )  {

   mlog << Error << "\nPython3_Numpy::set(PyObject *)-> "
        << "buffer interface not supported\n\n";

   exit ( 1 );

}

   //
   //  PyBUF_SIMPLE | PyBUF_C_CONTIGUOUS
   //
   //  should give us contiguous data in C order
   //


if ( PyObject_GetBuffer(Data_Obj, &View, PyBUF_SIMPLE | PyBUF_C_CONTIGUOUS) < 0 )  {

   mlog << Error << "\nPython3_Numpy::set(PyObject *)-> "
        << "failed to get buffer\n\n";

   exit ( 1 );

}

Buffer = View.buf;

Item_Size = (long) (View.itemsize);

   //
   //  get the data type
   //

PyObject * dtype_obj = get_attribute(Object, "dtype");

if ( ! dtype_obj )  {

   mlog << Error << "\nPython3_Numpy::set(PyObject *) -> "
        << "can't get numpy dtype attribute!\n\n";

   exit ( 1 );

}

PyObject * dtype_str_obj = get_attribute(dtype_obj, "str");

if ( ! dtype_str_obj )  {

   mlog << Error << "\nPython3_Numpy::set(PyObject *) -> "
        << "can't get numpy dtype attribute string!\n\n";

   exit ( 1 );

}

Dtype = PyUnicode_AsUTF8(dtype_str_obj);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////

