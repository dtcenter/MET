

////////////////////////////////////////////////////////////////////////


using namespace std;


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "concat_string.h"

#include "python_dict.h"


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

   if ( ! PyString_Check(key) )  {

      cerr << "\n\n  dump_dict() -> key is not a string!\n\n";

      exit ( 1 );

   }

   cout << tab << "Key   = \"" << PyString_AsString(key) << "\"\n";

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


if ( PyString_Check(value) )  {

   cout << '\"' << PyString_AsString(value) << '\"'
        << tab << "(type: string)\n";

   return;

}

   ////////////////////////////


if ( PyInt_Check(value) )  {

   cout << PyInt_AsLong(value)
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

cerr << "\n\n  dump_dict_value() -> can't determine type for dict value!\n\n";

exit ( 1 );


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////



