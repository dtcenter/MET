

////////////////////////////////////////////////////////////////////////


   //
   //  Warning:  This file is machine generated
   //
   //            Do not edit by hand
   //
   //
   //  Created by stackgen on August 20, 2019   3:12 pm   MDT
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util.h"

#include "token_stack.h"


////////////////////////////////////////////////////////////////////////


   //
   //   Code for class TokenStack
   //


////////////////////////////////////////////////////////////////////////


TokenStack::TokenStack()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


TokenStack::~TokenStack()

{

clear();

}


////////////////////////////////////////////////////////////////////////


TokenStack::TokenStack(const TokenStack & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


TokenStack & TokenStack::operator=(const TokenStack & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void TokenStack::init_from_scratch()

{

e = (Token *) nullptr;

AllocInc = 50;   //  default value

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void TokenStack::clear()

{

if ( e )  { delete [] e;  e = (Token *) nullptr; }



Nelements = 0;

Nalloc = 0;

// AllocInc = 50;   //  don't reset AllocInc


return;

}


////////////////////////////////////////////////////////////////////////


void TokenStack::assign(const TokenStack & _a)

{

clear();

if ( _a.depth() == 0 )  return;

extend(_a.depth());

int j;

for (j=0; j<(_a.depth()); ++j)  {

   e[j] = _a.e[j];

}

Nelements = _a.Nelements;

return;

}


////////////////////////////////////////////////////////////////////////


void TokenStack::extend(int n)

{

if ( n <= Nalloc )  return;

n = AllocInc*( (n + AllocInc - 1)/AllocInc );

int j;
Token * u = new Token [n];

if ( !u )  {

   cout << "TokenStack::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

for(j=0; j<Nelements; ++j)  {

   u[j] = e[j];

}

if ( e )  { delete [] e;  e = (Token *) nullptr; }

e = u;

u = (Token *) nullptr;

Nalloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void TokenStack::dump(ostream & out, int _depth_) const

{

Indent prefix(_depth_);

out << prefix << "Nelements = " << Nelements << "\n";
// out << prefix << "Nalloc    = " << Nalloc    << "\n";
// out << prefix << "AllocInc  = " << AllocInc  << "\n";

int j;

for(j=0; j<Nelements; ++j)  {

   // out << prefix << "Element # " << j << " ... \n";
   // e[j].dump(out, _depth_ + 1);
   out << prefix << "Element # " << j << " ... ";
   e[j].dump(out);


}

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void TokenStack::set_alloc_inc(int n)

{

if ( n < 0 )  {

   cout << "TokenStack::set_alloc_int(int) -> bad value ... " << n << "\n\n";

   exit ( 1 );

}

if ( n == 0 )  AllocInc = 50;   //  default value
else           AllocInc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void TokenStack::push(const Token & a)

{

extend(Nelements + 1);

e[Nelements++] = a;

return;

}


////////////////////////////////////////////////////////////////////////


Token TokenStack::pop()

{

if ( Nelements <= 0 )  {

   cout << "TokenStack::pop() -> stack empty!\n\n";

   exit ( 1 );

}

Token _t = e[Nelements - 1];

--Nelements;

return ( _t );

}


////////////////////////////////////////////////////////////////////////


Token TokenStack::peek() const

{

if ( Nelements <= 0 )  {

   cout << "TokenStack::pop() -> stack empty!\n\n";

   exit ( 1 );

}

Token _t = e[Nelements - 1];

return ( _t );

}


////////////////////////////////////////////////////////////////////////


int TokenStack::top_prec() const   //  the "in" prec

{

if ( Nelements <= 0 )  {

   cout << "TokenStack::top_prec() -> stack empty!\n\n";

   exit ( 1 );

}

int k = e[Nelements - 1].in_prec;

return ( k );

}


////////////////////////////////////////////////////////////////////////

/*
char TokenStack::top_value() const

{

if ( Nelements <= 0 )  {

   cout << "TokenStack::top_value() -> stack empty!\n\n";

   exit ( 1 );

}

char c = e[Nelements - 1].value;

return ( c );

}
*/


////////////////////////////////////////////////////////////////////////


bool TokenStack::top_is_mark() const

{

if ( Nelements <= 0 )  {

   cout << "TokenStack::top_is_mark() -> stack empty!\n\n";

   exit ( 1 );

}

bool tf = e[Nelements - 1].is_mark();

return ( tf );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & out, const TokenStack & ts)

{

ts.dump(out);

return ( out );

}


////////////////////////////////////////////////////////////////////////




