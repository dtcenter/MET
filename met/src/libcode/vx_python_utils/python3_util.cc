

////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "concat_string.h"
#include "vx_math.h"

#include "python3_util.h"


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & out, PyObject * obj)

{

char * buf = 0;
size_t len = 0;
FILE * f   = 0;


if ( (f = open_memstream(&buf, &len)) == NULL )  {

   cerr << "\n\n  operator<<(ostream &, PyObject *) -> unable to open memory stream\n\n";

   exit ( 1 );

}


if ( PyObject_Print(obj, f, Py_PRINT_RAW) < 0 )  {

   cerr << "\n\n  operator<<(ostream &, PyObject *) -> PyObject_Print error\n\n";

   exit ( 1 );

}

fflush(f);   //  important

if ( buf )  out << buf;

   //
   //  cleanup
   //

fclose(f);   f = 0;

if ( buf )  { free(buf);   buf = 0; }

   //
   //  done
   //

return ( out );

}


////////////////////////////////////////////////////////////////////////


PyObject * get_attribute(PyObject * obj, const char * attribute_name)

{

if ( PyObject_HasAttrString(obj, attribute_name) == 0 )  return ( (PyObject *) 0 );

PyObject * att = PyObject_GetAttrString(obj, attribute_name);

return ( att );

}


////////////////////////////////////////////////////////////////////////


int pyobject_as_int (PyObject * obj)

{

int k;


if ( PyLong_Check(obj) )  {   //  long?

   k = (int) PyLong_AsLong(obj);

} else if ( PyFloat_Check(obj) )  {   //  double?

   k = nint(PyFloat_AsDouble(obj));

} else if ( PyUnicode_Check(obj) )  {   //  string?

   k = atoi(PyUnicode_AsUTF8(obj));

} else {

   cerr << "\n\n  pyobject_as_int (PyObject *) -> bad object type\n\n";

   exit ( 1 );

}


return ( k );

}


////////////////////////////////////////////////////////////////////////


double pyobject_as_double (PyObject * obj)

{

double x;


if ( PyLong_Check(obj) )  {   //  long?

   x = (double) PyLong_AsLong(obj);

} else if ( PyFloat_Check(obj) )  {   //  double?

   x = PyFloat_AsDouble(obj);

} else if ( PyUnicode_Check(obj) )  {   //  string?

   x = atof(PyUnicode_AsUTF8(obj));

} else {

   cerr << "\n\n  pyobject_as_double (PyObject *) -> bad object type\n\n";

   exit ( 1 );

}


return ( x );

}


////////////////////////////////////////////////////////////////////////


std::string pyobject_as_string (PyObject * obj)

{

std::string s;

if ( PyUnicode_Check(obj) )  {   //  string?

   s = PyUnicode_AsUTF8(obj);

} else {

   cerr << "\n\n  pyobject_as_string (PyObject *) -> bad object type\n\n";

   exit ( 1 );

}

return ( s );

}


////////////////////////////////////////////////////////////////////////


ConcatString pyobject_as_concat_string (PyObject * obj)

{

ConcatString s;

if ( PyUnicode_Check(obj) )  {   //  string?

   s = PyUnicode_AsUTF8(obj);

} else {

   cerr << "\n\n  pyobject_as_concat_string (PyObject *) -> bad object type\n\n";

   exit ( 1 );

}

return ( s );

}


////////////////////////////////////////////////////////////////////////




