// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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

#include "vx_util/int_array.h"
#include "vx_util/indent.h"


////////////////////////////////////////////////////////////////////////


static int is_bigger(const void *, const void *);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class IntArray
   //


////////////////////////////////////////////////////////////////////////


IntArray::IntArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


IntArray::~IntArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


IntArray::IntArray(const IntArray & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


IntArray & IntArray::operator=(const IntArray & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void IntArray::init_from_scratch()

{

e = (int *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void IntArray::clear()

{

if ( e )  { delete [] e;  e = (int *) 0; }

Nelements = Nalloc = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void IntArray::assign(const IntArray & a)

{

clear();

if ( a.Nelements == 0 )  return;

extend(a.Nelements);

int j;

for (j=0; j<(a.Nelements); ++j)  {

   e[j] = a.e[j];

}

Nelements = a.Nelements;


return;

}


////////////////////////////////////////////////////////////////////////


void IntArray::extend(int n)

{

if ( Nalloc >= n )  return;

int k;

k = n/intarray_alloc_inc;

if ( n%intarray_alloc_inc )  ++k;

n = k*intarray_alloc_inc;

int * u = (int *) 0;

u = new int [n];

if ( !u )  {

   cerr << "\n\n  void IntArray::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

int j;

memset(u, 0, n*sizeof(int));

if ( e )  {

   for (j=0; j<Nelements; ++j)  {

      u[j] = e[j];

   }

   delete [] e;  e = (int *) 0;

}

e = u;   u = (int *) 0;

Nalloc = n;



return;

}


////////////////////////////////////////////////////////////////////////


void IntArray::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "Nelements = " << Nelements << "\n";
out << prefix << "Nalloc    = " << Nalloc    << "\n";

int j;

for (j=0; j<Nelements; ++j)  {

   out << prefix << "Element # " << j << " = " << e[j] << "\n";

}




   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


int IntArray::operator[](int n) const

{

if ( (n < 0) || (n >= Nelements) )  {

   cerr << "\n\n  IntArray::operator[](int) const -> range check error\n\n";

   exit ( 1 );

}

return ( e[n] );

}


////////////////////////////////////////////////////////////////////////


int IntArray::has(int k) const

{

int j;

for (j=0; j<Nelements; ++j)  {

   if ( e[j] == k )  return ( 1 );

}

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int IntArray::has(int k, int & index) const

{

int j;

index = -1;

for (j=0; j<Nelements; ++j)  {

   if ( e[j] == k )  { index = j;  return ( 1 ); }

}

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void IntArray::add(int k)

{

extend(Nelements + 1);

e[Nelements++] = k;

return;

}


////////////////////////////////////////////////////////////////////////


void IntArray::add(const IntArray & a)

{

extend(Nelements + a.Nelements);

int j;

for (j=0; j<(a.Nelements); ++j)  {

   e[Nelements++] = a.e[j];

}


return;

}


////////////////////////////////////////////////////////////////////////


void IntArray::sort_increasing()

{

if ( Nelements <= 1 )  return;

qsort(e, Nelements, sizeof(*e), is_bigger);

return;

}


////////////////////////////////////////////////////////////////////////


int IntArray::sum() const

{

int i, s;

for (i=0, s=0; i<Nelements; i++) s += e[i];

return(s);

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


int is_bigger(const void * p1, const void * p2)

{

const int & a = *((int *) p1);
const int & b = *((int *) p2);

if ( a < b )  return ( -1 );
if ( a > b )  return (  1 );

return ( 0 );

}


////////////////////////////////////////////////////////////////////////




