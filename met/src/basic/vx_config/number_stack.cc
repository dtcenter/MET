

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
#include <string.h>
#include <cmath>

#include "vx_util.h"

#include "number_stack.h"


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

clear();

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

e = (Number **) 0;

AllocInc = 50;   //  default value

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::clear()

{

if ( e )  {

   int j;

   for (j=0; j<Nalloc; ++j)  {

      if ( e[j] )  { delete e[j];  e[j] = (Number *) 0; }

   }

   delete [] e;  e = (Number **) 0;

}   //  if e


Nelements = 0;

Nalloc = 0;

// AllocInc = 50;   //  don't reset AllocInc


return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::assign(const NumberStack & _a)

{

clear();

if ( _a.depth() == 0 )  return;

extend(_a.depth());

int j;

for (j=0; j<(_a.depth()); ++j)  {

   *(e[j]) = *(_a.e[j]);

}

Nelements = _a.Nelements;

return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::extend(int n)

{

if ( n <= Nalloc )  return;

n = AllocInc*( (n + AllocInc - 1)/AllocInc );

int j;
Number ** u = new Number * [n];

if ( !u )  {

   cerr << "NumberStack::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

memset(u, 0, n*sizeof(Number *));

for(j=0; j<Nelements; ++j)  {

   u[j] = e[j];

}

if ( e )  { delete [] e;  e = (Number **) 0; }

e = u;

u = (Number **) 0;

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

   // e[j]->dump(out, _depth_ + 1);

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

if ( n == 0 )  AllocInc = 50;   //  default value
else           AllocInc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::push(const Number & a)

{

extend(Nelements + 1);

e[Nelements] = new Number;

if ( !(e[Nelements]) )  {

   cerr << "NumberStack::add(const Number &) -> memory allocation error\n\n";

   exit ( 1 );

}

*(e[Nelements++]) = a;

return;

}


////////////////////////////////////////////////////////////////////////


Number NumberStack::pop()

{

if ( Nelements <= 0 )  {

   cerr << "NumberStack::pop() -> stack empty!\n\n";

   exit ( 1 );

}

Number _t = *(e[Nelements - 1]);

--Nelements;

return ( _t );

}


////////////////////////////////////////////////////////////////////////


Number NumberStack::peek() const

{

if ( Nelements <= 0 )  {

   cerr << "NumberStack::pop() -> stack empty!\n\n";

   exit ( 1 );

}

Number _t = *(e[Nelements - 1]);

return ( _t );

}


////////////////////////////////////////////////////////////////////////


