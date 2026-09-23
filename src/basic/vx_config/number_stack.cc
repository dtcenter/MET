

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util.h"

#include "number_stack.h"


using namespace std;


////////////////////////////////////////////////////////////////////////


constexpr int default_ns_alloc_inc = 100;   //  default value



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

if ( this == &a )  return *this;

assign(a);

return *this;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::init_from_scratch()

{

AllocInc = default_ns_alloc_inc;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::clear(bool initialize)

{

e.clear();

if (initialize) {
   AllocInc = default_ns_alloc_inc;

   e.reserve(default_ns_alloc_inc);
}

return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::assign(const NumberStack & s)

{

clear();

if ( s.depth() == 0 )  return;

e = s.e;

AllocInc = s.AllocInc;

return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::dump(ostream & out, int _depth_) const

{

Indent prefix(_depth_);

out << prefix << "Nelements = " << e.size()     << "\n";
out << prefix << "Nalloc    = " << e.capacity() << "\n";
out << prefix << "AllocInc  = " << AllocInc     << "\n";

int j;

for(j=0; j<(int) e.size(); ++j)  {

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

e.push_back(a);

return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::push_int(const int k)

{

e.emplace_back();

set_int(e.back(), k);

return;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::push_double (const double x)

{

e.emplace_back();

set_double(e.back(), x);

return;

}


////////////////////////////////////////////////////////////////////////


Number NumberStack::pop()

{

if ( e.empty() )  {

   cerr << "NumberStack::pop() -> stack empty!\n\n";

   exit ( 1 );

}

Number a = e.back();

e.pop_back();

return a;

}


////////////////////////////////////////////////////////////////////////


void NumberStack::pop2(Number & a, Number & b)

{

if ( e.size() < 2 )  {

   cerr << "NumberStack::pop2() -> stack empty!\n\n";

   exit ( 1 );

}

size_t k = e.size() - 1;

b = e[k--];
a = e[k];

e.pop_back();
e.pop_back();

return;

}


////////////////////////////////////////////////////////////////////////


Number NumberStack::peek() const

{

if ( e.empty() )  {

   cerr << "NumberStack::pop() -> stack empty!\n\n";

   exit ( 1 );

}

return e.back();

}


////////////////////////////////////////////////////////////////////////


