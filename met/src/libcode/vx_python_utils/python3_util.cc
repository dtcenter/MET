

////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "concat_string.h"

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







