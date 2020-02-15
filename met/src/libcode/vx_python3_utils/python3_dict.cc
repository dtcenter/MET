// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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


static void dump_dict(std::ostream &, PyObject * obj, int depth);

static void dump_dict_value(std::ostream &, PyObject * value, int depth);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Python3_Dict
   //


////////////////////////////////////////////////////////////////////////


Python3_Dict::Python3_Dict()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Python3_Dict::Python3_Dict(PyObject * _obj)

{

init_from_scratch();

set(_obj);

}


////////////////////////////////////////////////////////////////////////


Python3_Dict::~Python3_Dict()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Python3_Dict::Python3_Dict(const Python3_Dict &)

{

mlog << Error << "\nPython3_Dict::Python3_Dict(const Python3_Dict &) -> "
     << "should never be called!\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


Python3_Dict & Python3_Dict::operator=(const Python3_Dict &)

{

mlog << Error << "\nPython3_Dict(const Python3_Dict &) -> "
     << "should never be called!\n\n";

exit ( 1 );

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Python3_Dict::init_from_scratch()

{

Object = 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Python3_Dict::clear()

{

Object = 0;  //  don't deallocate

Size = 0;

return;

}


////////////////////////////////////////////////////////////////////////


int Python3_Dict::lookup_int(const char * key) const

{

int k;
PyObject * a = 0;

a = PyDict_GetItemString(Object, key);

if ( ! a )  {

   mlog << Error << "\nPython3_Dict::lookup_int(const char *) -> "
        << "value for key \"" << key << "\" not found\n\n";

   exit ( 1 );

}

if ( ! PyLong_Check(a) )  {

   mlog << Error << "\nPython3_Dict::lookup_int(const char *) -> "
        << "value for key \"" << key << "\" not an integer\n\n";

   exit ( 1 );

}

k = (int) PyLong_AS_LONG(a);

return ( k );

}


////////////////////////////////////////////////////////////////////////


double Python3_Dict::lookup_double(const char * key) const

{

double t;
PyObject * a = 0;

a = PyDict_GetItemString(Object, key);

if ( ! a )  {

   mlog << Error << "\nPython3_Dict::lookup_double(const char * key) -> "
        << "value for key \"" << key << "\" not found\n\n";

   exit ( 1 );

}

if ( ! PyFloat_Check(a) )  {

   mlog << Error << "\nPython3_Dict::lookup_double(const char * key) -> "
        << "value for key \"" << key << "\" not a floating point number\n\n";

   exit ( 1 );

}

t = PyFloat_AS_DOUBLE(a);

return ( t );

}


////////////////////////////////////////////////////////////////////////


ConcatString Python3_Dict::lookup_string(const char * key) const

{

ConcatString s;
PyObject * a = 0;

a = PyDict_GetItemString(Object, key);

if ( ! a )  {

   mlog << Error << "\nPython3_Dict::lookup_string(const char * key) -> "
        << "value for key \"" << key << "\" not found\n\n";

   exit ( 1 );

}

if ( ! PyUnicode_Check(a) )  {

   mlog << Error << "\nPython3_Dict::lookup_string(const char * key) -> "
        << "value for key \"" << key << "\" not a character string\n\n";

   exit ( 1 );

}

s = PyUnicode_AsUTF8(a);

return ( s );

}


////////////////////////////////////////////////////////////////////////


PyObject * Python3_Dict::lookup_dict(const char * key) const

{

PyObject * a = 0;

a = PyDict_GetItemString(Object, key);

if ( ! a )  {

   mlog << Error << "\nPython3_Dict::lookup_dict(const char * key) -> "
        << "value for key \"" << key << "\" not found\n\n";

   exit ( 1 );

}

if ( ! PyDict_Check(a) )  {

   mlog << Error << "\nPython3_Dict::lookup_dict(const char * key) -> "
        << "value for key \"" << key << "\" not a python dictionary\n\n";

   exit ( 1 );

}


return ( a );

}


////////////////////////////////////////////////////////////////////////


PyObject * Python3_Dict::lookup_list(const char * key) const

{

PyObject * a = 0;


a = PyDict_GetItemString(Object, key);


if ( ! a )  {

   mlog << Error << "\nPython3_Dict::lookup_list(const char * key) -> "
        << "value for key \"" << key << "\" not found\n\n";


   exit ( 1 );

}

if ( ! PyList_Check(a) )  {

   mlog << Error << "\nPython3_Dict::lookup_dict(const char * key) -> "
        << "value for key \"" << key << "\" not a python list\n\n";

   exit ( 1 );

}


return ( a );

}


////////////////////////////////////////////////////////////////////////


void Python3_Dict::dump(std::ostream & out, int depth) const

{

::dump_dict(out, Object, depth);

return;

}


////////////////////////////////////////////////////////////////////////


void Python3_Dict::set(PyObject * _obj)

{

     if ( PyDict_Check   (_obj) )  set_from_dict   (_obj);
else if ( PyModule_Check (_obj) )  set_from_module (_obj);
else {

   mlog << Error << "\nPython3_Dict::set(PyObject *) -> "
        << "bad object type\n\n";

   exit ( 1 );

}



return;

}


////////////////////////////////////////////////////////////////////////


void Python3_Dict::set_from_dict(PyObject * _obj)

{

clear();


if ( ! PyDict_Check(_obj) )  {

   mlog << Error << "\nPython3_Dict::set_from_dict(PyObject *) -> "
        << "object is not a python dictionary!\n\n";

   exit ( 1 );

}


Size = PyDict_Size (_obj);

Object = _obj;

return;

}


////////////////////////////////////////////////////////////////////////


void Python3_Dict::set_from_module(PyObject * _obj)

{

clear();


if ( ! PyModule_Check(_obj) )  {

   mlog << Error << "\nPython3_Dict::set_from_module(PyObject *) -> "
        << "object is not a python module!\n\n";

   exit ( 1 );

}

set_from_dict(PyModule_GetDict(_obj));

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void dump_dict(std::ostream & out, PyObject * obj, int depth)

{

int j, n;
ConcatString tab;

for (j=0; j<depth; ++j)  tab << "|  ";

   //
   //   get the size of the array
   //

n = PyDict_Size (obj);

out << tab << "Dictionary size = " << n << "\n";

PyObject * key   = 0;
PyObject * value = 0;
int status;
long pos;

j = pos = 0;


while ( (status = PyDict_Next (obj, &pos, &key, &value)) != 0 )  {

   out << tab << "\n";

   out << tab << "Item # " << j << "\n";   //  want "j" here, not "pos"

   if ( ! PyUnicode_Check(key) )  {

      mlog << Error << "\ndump_dict() -> "
           << "key is not a string!\n\n";

      exit ( 1 );

   }

   out << tab << "Key   = \"" << PyUnicode_AsUTF8(key) << "\"\n";

   out << tab << "Value: ";

   dump_dict_value(out, value, depth + 1);

   ++j;

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void dump_dict_value(std::ostream & out, PyObject * value, int depth)

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

   out << '\"' << PyUnicode_AsUTF8(value) << '\"'
       << tab << "(type: string)\n";

   return;

}

   ////////////////////////////


if ( PyLong_Check(value) )  {

   out << PyLong_AsLong(value)
       << tab << "(type: integer)\n";

   return;

}


   ////////////////////////////


if ( PyFloat_Check(value) )  {

   out << PyFloat_AsDouble(value)
       << tab << "(type: floating point)\n";

   return;

}


   ////////////////////////////


if ( PyDict_Check(value) )  {

   out << "(dict) ... \n";

   dump_dict(out, value, depth);

   return;

}


   ////////////////////////////

      //
      //  nope
      //

mlog << Error << "\nPython3_Dict::dump_dict_value() -> "
     << "can't determine type for dict value!\n\n";

exit ( 1 );


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////

