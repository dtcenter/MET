

////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_log.h"
#include "vx_math.h"

#include "python3_util.h"
#include "global_python.h"

using namespace std;


////////////////////////////////////////////////////////////////////////

GlobalPython GP;   // this needs external linkage

////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & out, PyObject * obj)

{

   //
   // PyObject_Print() with Py_PRINT_RAW writes str(obj),
   // so write that string directly
   //

if ( ! obj )  { out << "<nil>";   return out; }

PyObject * str_obj = PyObject_Str(obj);

if ( ! str_obj )  {

   mlog << Error << "\noperator<<(ostream &, PyObject *) -> "
        << "PyObject_Str error\n\n";

   exit ( 1 );

}

const char * buf = PyUnicode_AsUTF8(str_obj);

if ( ! buf )  {

   mlog << Error << "\noperator<<(ostream &, PyObject *) -> "
        << "PyUnicode_AsUTF8 error\n\n";

   exit ( 1 );

}

out << buf;

   //
   // cleanup
   //

Py_DECREF(str_obj);   str_obj = 0;

   //
   // done
   //

return out;

}


////////////////////////////////////////////////////////////////////////


PyObject * get_attribute(PyObject * obj, const char * attribute_name)

{

if ( PyObject_HasAttrString(obj, attribute_name) == 0 )  return (PyObject *) 0;

PyObject * att = PyObject_GetAttrString(obj, attribute_name);

return att;

}


////////////////////////////////////////////////////////////////////////


int pyobject_as_int (PyObject * obj)

{

int k = bad_data_int;


if ( PyLong_Check(obj) )  {   // long?

   k = (int) PyLong_AsLong(obj);

} else if ( PyFloat_Check(obj) )  {   // double?

   k = nint(PyFloat_AsDouble(obj));

} else if ( PyUnicode_Check(obj) )  {   // string?

   k = atoi(PyUnicode_AsUTF8(obj));

} else {

   mlog << Error << "\npyobject_as_int (PyObject *) -> "
        << "bad object type\n\n";

   exit ( 1 );

}

return k;

}


////////////////////////////////////////////////////////////////////////


bool pyobject_as_bool (PyObject * obj)

{

return ( 1 == PyObject_IsTrue(obj) );

}


////////////////////////////////////////////////////////////////////////


double pyobject_as_double (PyObject * obj)

{

double x = bad_data_double;


if ( PyLong_Check(obj) )  {   // long?

   x = (double) PyLong_AsLong(obj);

} else if ( PyFloat_Check(obj) )  {   // double?

   x = PyFloat_AsDouble(obj);

} else if ( PyUnicode_Check(obj) )  {   // string?

   x = atof(PyUnicode_AsUTF8(obj));

} else {

   mlog << Error << "\npyobject_as_double (PyObject *) -> "
        << "bad object type\n\n";

   exit ( 1 );

}

return x;

}


////////////////////////////////////////////////////////////////////////


std::string pyobject_as_string (PyObject * obj)

{

std::string s;

if ( PyUnicode_Check(obj) )  {   // string?

   s = PyUnicode_AsUTF8(obj);

} else {

   mlog << Error << "\npyobject_as_string (PyObject *) -> "
        << "bad object type\n\n";

   exit ( 1 );

}

return s;

}


////////////////////////////////////////////////////////////////////////


ConcatString pyobject_as_concat_string (PyObject * obj)

{

ConcatString s;

if ( PyUnicode_Check(obj) )  {   // string?

   s = PyUnicode_AsUTF8(obj);

} else {

   mlog << Error << "\npyobject_as_concat_string(PyObject *) -> "
        << "bad object type\n\n";

   exit ( 1 );

}

return s;

}


////////////////////////////////////////////////////////////////////////


StringArray pyobject_as_string_array (PyObject * obj)

{

StringArray a;
ConcatString s;
PyObject *item = 0;

int size = PyList_Size (obj);
for (int idx=0; idx<size; idx++) {
   item = PyList_GetItem(obj, idx);
   s = pyobject_as_concat_string (item);
   a.add(s);
}

return a;

}


////////////////////////////////////////////////////////////////////////


void run_python_string(const char * s)

{

if ( PyRun_SimpleString(s) < 0 )  {

   mlog << Error << "\nrun_python_string() -> "
        << "command \"" << s << "\" failed!\n\n";

   fflush(stdout);
   fflush(stderr);

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////

