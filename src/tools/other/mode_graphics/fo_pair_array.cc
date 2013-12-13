

////////////////////////////////////////////////////////////////////////


   //
   //  Warning:  This file is machine generated
   //
   //            Do not edit by hand
   //
   //
   //  Created by arraygen on December 12, 2013   11:56 am   MST
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util.h"

#include "fo_pair_array.h"


////////////////////////////////////////////////////////////////////////


   //
   //   Code for class FO_PairArray
   //


////////////////////////////////////////////////////////////////////////


FO_PairArray::FO_PairArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


FO_PairArray::~FO_PairArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


FO_PairArray::FO_PairArray(const FO_PairArray & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


FO_PairArray & FO_PairArray::operator=(const FO_PairArray & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void FO_PairArray::init_from_scratch()

{

e = (FO_Pair *) 0;

AllocInc = 20;   //  default value

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void FO_PairArray::clear()

{

if ( e )  { delete [] e;  e = (FO_Pair *) 0; }



Nelements = 0;

Nalloc = 0;

// AllocInc = 20;   //  don't reset AllocInc


return;

}


////////////////////////////////////////////////////////////////////////


void FO_PairArray::assign(const FO_PairArray & a)

{

clear();

if ( a.n_elements() == 0 )  return;

add(a);

return;

}


////////////////////////////////////////////////////////////////////////


void FO_PairArray::extend(int n)

{

if ( n <= Nalloc )  return;

n = AllocInc*( (n + AllocInc - 1)/AllocInc );

int j;
FO_Pair * u = new FO_Pair [n];

if ( !u )  {

   cerr << "FO_PairArray::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

for(j=0; j<Nelements; ++j)  {

   u[j] = e[j];

}

if ( e )  { delete [] e;  e = (FO_Pair *) 0; }

e = u;

u = (FO_Pair *) 0;

Nalloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void FO_PairArray::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Nelements = " << Nelements << "\n";
out << prefix << "Nalloc    = " << Nalloc    << "\n";
out << prefix << "AllocInc  = " << AllocInc  << "\n";

int j;

for(j=0; j<Nelements; ++j)  {

   out << prefix << "Element # " << j << " ... \n";

   e[j].dump(out, depth + 1);

}

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void FO_PairArray::set_alloc_inc(int n)

{

if ( n < 0 )  {

   cerr << "FO_PairArray::set_alloc_int(int) -> bad value ... " << n << "\n\n";

   exit ( 1 );

}

if ( n == 0 )  AllocInc = 20;   //  default value
else           AllocInc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void FO_PairArray::add(const FO_Pair & a)

{

extend(Nelements + 1);

e[Nelements++] = a;

return;

}


////////////////////////////////////////////////////////////////////////


void FO_PairArray::add(const FO_PairArray & a)

{

int j;

extend(Nelements + a.n_elements());

for (j=0; j<(a.n_elements()); ++j)  {

   add(a[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


FO_Pair FO_PairArray::operator[](int n) const

{

if ( (n < 0) || (n >= Nelements) )  {

   cerr << "\n\n  FO_PairArray::operator[](int) -> range check error ... " << n << "\n\n";

   exit ( 1 );
}

return ( e[n] );

}


////////////////////////////////////////////////////////////////////////


