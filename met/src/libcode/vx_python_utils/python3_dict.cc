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
#include "concat_string.h"

#include "python3_dict.h"


////////////////////////////////////////////////////////////////////////


int dict_lookup_int(PyObject * dict, const char * key)

{

int k;
PyObject * a = 0;

a = PyDict_GetItemString(dict, key);

if ( ! a )  {

   cerr << "\n\n  dict_lookup_int(PyObject * dict, const char * key) -> value for key \""
        << key << "\" not found\n\n";

   exit ( 1 );

}

if ( ! PyLong_Check(a) )  {

   cerr << "\n\n  dict_lookup_int(PyObject * dict, const char * key) -> value for key \""
        << key << "\" not an integer\n\n";

   exit ( 1 );

}

k = (int) PyLong_AS_LONG(a);

return ( k );

}


////////////////////////////////////////////////////////////////////////


double dict_lookup_double(PyObject * dict, const char * key)

{

double t;
PyObject * a = 0;

a = PyDict_GetItemString(dict, key);

if ( ! a )  {

   cerr << "\n\n  dict_lookup_double(PyObject * dict, const char * key) -> value for key \""
        << key << "\" not found\n\n";

   exit ( 1 );

}

if ( ! PyFloat_Check(a) )  {

   cerr << "\n\n  dict_lookup_double(PyObject * dict, const char * key) -> value for key \""
        << key << "\" not a floating point number\n\n";

   exit ( 1 );

}

t = PyFloat_AS_DOUBLE(a);

return ( t );

}


////////////////////////////////////////////////////////////////////////


ConcatString dict_lookup_string(PyObject * dict, const char * key)

{

ConcatString s;
PyObject * a = 0;

a = PyDict_GetItemString(dict, key);

if ( ! a )  {

   cerr << "\n\n  dict_lookup_string(PyObject * dict, const char * key) -> value for key \""
        << key << "\" not found\n\n";

   exit ( 1 );

}

if ( ! PyUnicode_Check(a) )  {

   cerr << "\n\n  dict_lookup_string(PyObject * dict, const char * key) -> value for key \""
        << key << "\" not a character string\n\n";

   exit ( 1 );

}

s = PyUnicode_AsUTF8(a);

return ( s );

}


////////////////////////////////////////////////////////////////////////


PyObject * dict_lookup_dict(PyObject * dict, const char * key)

{

PyObject * a = 0;

a = PyDict_GetItemString(dict, key);

if ( ! a )  {

   cerr << "\n\n  dict_lookup_dict(PyObject * dict, const char * key) -> value for key \""
        << key << "\" not found\n\n";

   exit ( 1 );

}

if ( ! PyDict_Check(a) )  {

   cerr << "\n\n  dict_lookup_dict(PyObject * dict, const char * key) -> value for key \""
        << key << "\" not a python dictionary\n\n";

   exit ( 1 );

}


return ( a );

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

   mlog << Error
        << "\n\n  dump_dict() -> not a dictionary!\n\n";

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

      mlog << Error
           << "\n\n  dump_dict() -> key is not a string!\n\n";

      exit ( 1 );

   }

   cout << tab << "Key   = \"" << PyUnicode_AsUTF8(key) << "\"\n";

   cout << tab << "Value: ";

   dump_dict_value(value, depth + 1);

   // cout << '\n';

   ++j;

}



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void dump_dict_value(PyObject * value, int depth)

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

   dump_dict(value, depth);

   return;

}


   ////////////////////////////

      //
      //  nope
      //

mlog << Error
     << "\n\n  dump_dict_value() -> can't determine type for dict value!\n\n";

exit ( 1 );


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////



