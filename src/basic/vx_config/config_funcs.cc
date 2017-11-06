
////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "config_funcs.h"
#include "calculator.h"
#include "configobjecttype_to_string.h"


////////////////////////////////////////////////////////////////////////


extern Calculator hp;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class UserFunc_1Arg
   //


////////////////////////////////////////////////////////////////////////


UserFunc_1Arg::UserFunc_1Arg()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


UserFunc_1Arg::~UserFunc_1Arg()

{


}


////////////////////////////////////////////////////////////////////////


UserFunc_1Arg::UserFunc_1Arg(const UserFunc_1Arg & f)

{

init_from_scratch();

assign(f);

}


////////////////////////////////////////////////////////////////////////


UserFunc_1Arg & UserFunc_1Arg::operator=(const UserFunc_1Arg & f)

{

if ( this == &f )  return ( * this );

assign(f);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void UserFunc_1Arg::init_from_scratch()

{

entry = 0;

v = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void UserFunc_1Arg::assign(const UserFunc_1Arg & f)

{

// clear();

entry = f.entry;

v = f.v;


return;

}


////////////////////////////////////////////////////////////////////////


void UserFunc_1Arg::set(const DictionaryEntry * e)

{

if ( e->type() != UserFunctionType )  {

   cerr << "\n\n  UserFunc_1Arg::set(const DictionaryEntry *) -> bad object type: "
        << configobjecttype_to_string(e->type()) << "\n\n";

   exit ( 1 );

}

if ( e->n_args() != 1 )  {

   cerr << "\n\n  UserFunc_1Arg::set(const DictionaryEntry *) -> bad number of arguments: "
        << (e->n_args()) << "\n\n";

   exit ( 1 );

}

entry = e;

v = e->icv();



return;

}


////////////////////////////////////////////////////////////////////////


double UserFunc_1Arg::operator()(double x) const

{

Number n;

n.d = x;
n.is_int = false;

hp.run( *v, &n);

n = hp.pop();

return ( as_double(n) );

}


////////////////////////////////////////////////////////////////////////





