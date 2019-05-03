
////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

clear();

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

return;

}


////////////////////////////////////////////////////////////////////////


void UserFunc_1Arg::assign(const UserFunc_1Arg & f)

{

clear();

Name  = f.Name;
NArgs = f.NArgs;
V     = f.V;

return;

}


////////////////////////////////////////////////////////////////////////


void UserFunc_1Arg::clear()

{

Name.clear();

NArgs = bad_data_int;

V.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void UserFunc_1Arg::set(const DictionaryEntry * e)

{

if ( ! e )  return;

if ( e->type() != UserFunctionType )  {

   cerr << "\n\n  UserFunc_1Arg::set(const DictionaryEntry *) -> "
        << "bad object type: "
        << configobjecttype_to_string(e->type()) << "\n\n";

   exit ( 1 );

}

if ( e->n_args() != 1 )  {

   cerr << "\n\n  UserFunc_1Arg::set(const DictionaryEntry *) -> "
        << "bad number of arguments: "
        << (e->n_args()) << "\n\n";

   exit ( 1 );

}

Name = e->name();

NArgs = e->n_args();

V = *(e->icv());

return;

}


////////////////////////////////////////////////////////////////////////


double UserFunc_1Arg::operator()(double x) const

{

Number n;

n.d = x;
n.is_int = false;

hp.run( V, &n);

n = hp.pop();

return ( as_double(n) );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class UserFunc_MultiArg
   //


////////////////////////////////////////////////////////////////////////


UserFunc_MultiArg::UserFunc_MultiArg()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


UserFunc_MultiArg::~UserFunc_MultiArg()

{

clear();

}


////////////////////////////////////////////////////////////////////////


UserFunc_MultiArg::UserFunc_MultiArg(const UserFunc_MultiArg & f)

{

init_from_scratch();

assign(f);

}


////////////////////////////////////////////////////////////////////////


UserFunc_MultiArg & UserFunc_MultiArg::operator=(const UserFunc_MultiArg & f)

{

if ( this == &f )  return ( * this );

assign(f);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void UserFunc_MultiArg::init_from_scratch()

{

return;

}


////////////////////////////////////////////////////////////////////////


void UserFunc_MultiArg::assign(const UserFunc_MultiArg & f)

{

clear();

Name  = f.Name;
NArgs = f.NArgs;
V     = f.V;

return;

}



////////////////////////////////////////////////////////////////////////


void UserFunc_MultiArg::clear()

{

Name.clear();

NArgs = bad_data_int;

V.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void UserFunc_MultiArg::set(const DictionaryEntry * e)

{

if ( e->type() != UserFunctionType )  {

   cerr << "\n\n  UserFunc_MultiArg::set(const DictionaryEntry *) -> "
        << "bad object type: "
        << configobjecttype_to_string(e->type()) << "\n\n";

   exit ( 1 );

}

if ( e->n_args() != 1 )  {

   cerr << "\n\n  UserFunc_MultiArg::set(const DictionaryEntry *) -> "
        << "bad number of arguments: "
        << (e->n_args()) << "\n\n";

   exit ( 1 );

}

Name = e->name();

NArgs = e->n_args();

V = *(e->icv());

return;

}


////////////////////////////////////////////////////////////////////////


Number UserFunc_MultiArg::operator()(const Number * n) const

{

hp.run( V, n);

return ( hp.pop() );

}


////////////////////////////////////////////////////////////////////////



