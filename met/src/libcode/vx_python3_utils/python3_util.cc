

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


void dump_dict(PyObject * obj, int depth)

{

int j, n;
// PyDictObject * dict = (PyDictObject *) obj;
ConcatString tab;

for (j=0; j<depth; ++j)  tab << "|  ";

   //
   //  check that it's a dictionary
   //

if ( ! PyDict_Check (obj) )  {

   cerr << "\n\n  dump_dict() -> not a dictionary!\n\n";

   exit ( 1 );

}

   //
   //   get the size of the array
   //

n = PyDict_Size (obj);

cout << tab << "Dictionary size = " << n << "\n";

PyObject * key   = 0;
PyObject * value = 0;
int status;
long pos;

j = pos = 0;


while ( (status = PyDict_Next (obj, &pos, &key, &value)) != 0 )  {

   cout << tab << "\n";

   cout << tab << "Item # " << j << "\n";   //  want "j" here, not "pos"

   if ( ! PyUnicode_Check(key) )  {

      cerr << "\n\n  dump_dict() -> key is not a string!\n\n";

      exit ( 1 );

   }

   cout << tab << "Key   = \"" << PyUnicode_AsUTF8(key) << "\"\n";

   cout << tab << "Value: ";

   dump_dict_value(key, value, depth + 1);

   // cout << '\n';

   ++j;

}



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void dump_dict_value(PyObject * key, PyObject * value, int depth)

{

const char * const tab = "    ";

   //
   //  Just check all the cases.
   //
   //    Python types are not enums (ie, ints),
   //
   //    so we can't use a switch here.
   //


if ( PyUnicode_Check(value) )  {

   cout << '\"' << PyUnicode_AsUTF8(value) << '\"'
        << tab << "(type: string)\n";

   return;

}

   ////////////////////////////


if ( PyLong_Check(value) )  {

   cout << PyLong_AsLong(value)
        << tab << "(type: integer)\n";

   return;

}


   ////////////////////////////


if ( PyFloat_Check(value) )  {

   cout << PyFloat_AsDouble(value)
        << tab << "(type: floating point)\n";

   return;

}


   ////////////////////////////


if ( PyDict_Check(value) )  {

   cout << "(dict) ... \n";

   dump_dict(value, depth + 1);

   return;

}


   ////////////////////////////

      //
      //  nope
      //

// cerr << "\n\n  dump_dict_value() -> can't determine value type for key \""
//      << PyUnicode_AsUTF8(key)
//      << "\"\n\n";
// 
// exit ( 1 );


cout << "(unknown)\n";


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////






