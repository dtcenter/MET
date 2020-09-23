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

#include "python3_list.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Python3_List
   //


////////////////////////////////////////////////////////////////////////


Python3_List::Python3_List()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Python3_List::Python3_List(PyObject * _obj)

{

init_from_scratch();

set(_obj);

}


////////////////////////////////////////////////////////////////////////


Python3_List::~Python3_List()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Python3_List::Python3_List(const Python3_List &)

{

mlog << Error << "\nPython3_List::Python3_List(const Python3_List &) -> "
     << "should never be called!\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


Python3_List & Python3_List::operator=(const Python3_List &)

{

mlog << Error << "\nPython3_List(const Python3_List &) -> "
     << "should never be called!\n\n";

exit ( 1 );

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Python3_List::init_from_scratch()

{

Object = 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Python3_List::clear()

{

Object = 0;  //  don't deallocate

Size = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void Python3_List::set(PyObject * _obj)

{

clear();


if ( ! PyList_Check(_obj) )  {

   mlog << Error << "\nPython3_List::set(PyObject *) -> "
        << "object is not a dictionary!\n\n";

   exit ( 1 );

}


Size = PyList_Size (_obj);

Object = _obj;

return;

}


////////////////////////////////////////////////////////////////////////


PyObject * Python3_List::operator[](int n) const

{

PyObject * a = 0;

if ( (n < 0) || (n >= Size) )  {

   mlog << Error << "\nPython3_List::operator[](int) const -> "
        << "range check error\n\n";

   exit ( 1 );

}


a = PyList_GetItem(Object, n);


return ( a );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////

