

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "met_int_array.h"

#include "vx_util.h"


////////////////////////////////////////////////////////////////////////


static int is_bigger(const void *, const void *);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MetIntArray
   //


////////////////////////////////////////////////////////////////////////


MetIntArray::MetIntArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


MetIntArray::~MetIntArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


MetIntArray::MetIntArray(const MetIntArray & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


MetIntArray & MetIntArray::operator=(const MetIntArray & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void MetIntArray::init_from_scratch()

{

e = (int *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void MetIntArray::clear()

{

if ( e )  { delete [] e;  e = (int *) 0; }

Nelements = Nalloc = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void MetIntArray::assign(const MetIntArray & a)

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


void MetIntArray::extend(int n)

{

if ( Nalloc >= n )  return;

int k;

k = n/met_intarray_alloc_inc;

if ( n%met_intarray_alloc_inc )  ++k;

n = k*met_intarray_alloc_inc;

int * u = (int *) 0;

u = new int [n];

if ( !u )  {

   cerr << "\n\n  void MetIntArray::extend(int) -> memory allocation error\n\n";

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


void MetIntArray::dump(ostream & out, int depth) const

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


int MetIntArray::operator[](int n) const

{

if ( (n < 0) || (n >= Nelements) )  {

   cerr << "\n\n  MetIntArray::operator[](int) const -> range check error\n\n";

   exit ( 1 );

}

return ( e[n] );

}


////////////////////////////////////////////////////////////////////////


void MetIntArray::set_size(int n)

{

clear();

extend(n);

Nelements = n;

int j;

for (j=0; j<Nelements; ++j)  e[j] = 0;


return;

}


////////////////////////////////////////////////////////////////////////


int MetIntArray::has(int k) const

{

int j;

for (j=0; j<Nelements; ++j)  {

   if ( e[j] == k )  return ( 1 );

}

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int MetIntArray::has(int k, int & index) const

{

int j;

index = -1;

for (j=0; j<Nelements; ++j)  {

   if ( e[j] == k )  { index = j;  return ( 1 ); }

}

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void MetIntArray::add(int k)

{

extend(Nelements + 1);

e[Nelements++] = k;

return;

}


////////////////////////////////////////////////////////////////////////


void MetIntArray::add(const MetIntArray & a)

{

extend(Nelements + a.Nelements);

int j;

for (j=0; j<(a.Nelements); ++j)  {

   e[Nelements++] = a.e[j];

}


return;

}


////////////////////////////////////////////////////////////////////////


void MetIntArray::add_no_repeat(int k)

{

if ( has(k) )  return;

add(k);

return;

}


////////////////////////////////////////////////////////////////////////


void MetIntArray::add_no_repeat(const MetIntArray & a)

{

int j;

for (j=0; j<(a.n_elements()); ++j)  {

   add_no_repeat(a[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


void MetIntArray::sort_increasing()

{

if ( Nelements <= 1 )  return;

qsort(e, Nelements, sizeof(*e), is_bigger);

return;

}


////////////////////////////////////////////////////////////////////////


void MetIntArray::put(int index, int value)

{

if ( (index < 0) || (index >= Nelements) )  {

   cerr << "\n\n  MetIntArray::put() -> range check error\n\n";

   exit ( 1 );

}

e[index] = value;

return;

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




