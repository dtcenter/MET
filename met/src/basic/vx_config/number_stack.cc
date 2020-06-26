

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
#include <string.h>
#include <cmath>

#include "vx_util.h"

#include "number_stack.h"


////////////////////////////////////////////////////////////////////////


static int default_ns_alloc_inc = 100;   //  default value



////////////////////////////////////////////////////////////////////////


   //
   //   Code for class NumberStack
   //


////////////////////////////////////////////////////////////////////////


NumberStack::NumberStack()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


NumberStack::~NumberStack()

{

clear(false);

}


////////////////////////////////////////////////////////////////////////


NumberStack::NumberStack(const NumberStack & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


NumberStack & NumberStack::operator=(const NumberStack & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void NumberStack::init_from_scratch()

{

e = 0;

AllocInc = default_ns_alloc_inc;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::clear(bool initialize)

{

if ( e )  { delete [] e;  e = 0; }


Nelements = 0;

Nalloc = 0;

if (initialize) {
   AllocInc = default_ns_alloc_inc;

   extend(default_ns_alloc_inc);
}

return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::assign(const NumberStack & s)

{

clear();

if ( s.depth() == 0 )  return;

extend(s.depth());

int j;

for (j=0; j<(s.depth()); ++j)  {

   e[j] = s.e[j];

}

Nelements = s.Nelements;

AllocInc = s.AllocInc;

return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::extend(int n)

{

if ( n <= Nalloc )  return;

if ( AllocInc <= 0 )  AllocInc = default_ns_alloc_inc;

n = AllocInc*( (n + AllocInc - 1)/AllocInc );

int j;
Number * u = new Number [n];

if ( !u )  {

   cerr << "NumberStack::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

for(j=0; j<Nelements; ++j)  {

   u[j] = e[j];

}

if ( e )  { delete [] e;  e = 0; }

e = u;

u = 0;

Nalloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::dump(ostream & out, int _depth_) const

{

Indent prefix(_depth_);

out << prefix << "Nelements = " << Nelements << "\n";
out << prefix << "Nalloc    = " << Nalloc    << "\n";
out << prefix << "AllocInc  = " << AllocInc  << "\n";

int j;

for(j=0; j<Nelements; ++j)  {

   out << prefix << "Element # " << j << "\n";

   // e[j].dump(out, _depth_ + 1);

}

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::set_alloc_inc(int n)

{

if ( n < 0 )  {

   cerr << "NumberStack::set_alloc_int(int) -> bad value ... " << n << "\n\n";

   exit ( 1 );

}

if ( n == 0 )  AllocInc = default_ns_alloc_inc;   //  default value
else           AllocInc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::push(const Number & a)

{

extend(Nelements + 1);

e[Nelements++] = a;

return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::push_int(const int k)

{

extend(Nelements + 1);

set_int(e[Nelements++], k);

return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::push_double (const double x)

{

extend(Nelements + 1);

set_double(e[Nelements++], x);

return;

}


////////////////////////////////////////////////////////////////////////


Number NumberStack::pop()

{

if ( Nelements <= 0 )  {

   cerr << "NumberStack::pop() -> stack empty!\n\n";

   exit ( 1 );

}

return ( e[--Nelements] );

}


////////////////////////////////////////////////////////////////////////


void NumberStack::pop2(Number & a, Number & b)

{

if ( Nelements < 2 )  {

   cerr << "NumberStack::pop2() -> stack empty!\n\n";

   exit ( 1 );

}

int k = Nelements - 1;

b = e[k--];
a = e[k--];

Nelements -= 2;

return;

}


////////////////////////////////////////////////////////////////////////


Number NumberStack::peek() const

{

if ( Nelements <= 0 )  {

   cerr << "NumberStack::pop() -> stack empty!\n\n";

   exit ( 1 );

}

return ( e[Nelements - 1] );

}


////////////////////////////////////////////////////////////////////////


