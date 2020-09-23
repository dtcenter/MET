

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

#include "long_array.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


   //
   //   Code for class LongArray
   //


////////////////////////////////////////////////////////////////////////


LongArray::LongArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


LongArray::~LongArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


LongArray::LongArray(const LongArray & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


LongArray & LongArray::operator=(const LongArray & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void LongArray::init_from_scratch()

{

e = (long *) 0;

AllocInc = 10;   //  default value

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void LongArray::clear()

{

if ( e )  { delete [] e;  e = (long *) 0; }



Nelements = 0;

Nalloc = 0;

// AllocInc = 10;   //  don't reset AllocInc


return;

}


////////////////////////////////////////////////////////////////////////


void LongArray::assign(const LongArray & a)

{

clear();

if ( a.n_elements() == 0 )  return;

add(a);

return;

}


////////////////////////////////////////////////////////////////////////


void LongArray::extend(int n, bool exact)

{

if ( n <= Nalloc )  return;

if ( ! exact )  {

   n = AllocInc*( (n + AllocInc - 1)/AllocInc );

}

int j;
long * u = (long *) 0;

u = new long [n];

if ( !u )  {

   mlog << Error << "\nLongArray::extend(int, bool) -> "
        << "memory allocation error\n\n";

   exit ( 1 );

}

memset(u, 0, n*sizeof(long));

for(j=0; j<Nelements; ++j)  {

   u[j] = e[j];

}

if ( e )  { delete [] e;  e = (long *) 0; }

e = u;

u = (long *) 0;

Nalloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void LongArray::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Nelements = " << Nelements << "\n";
out << prefix << "Nalloc    = " << Nalloc    << "\n";
out << prefix << "AllocInc  = " << AllocInc  << "\n";

int j;

for(j=0; j<Nelements; ++j)  {

   out << prefix << "Element # " << j << " = " << e[j] << "\n";


}

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void LongArray::set_alloc_inc(int n)

{

if ( n < 0 )  {

   mlog << Error << "\nLongArray::set_alloc_int(int) -> "
        << "bad value ... " << n << "\n\n";

   exit ( 1 );

}

if ( n == 0 )  AllocInc = 10;   //  default value
else           AllocInc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void LongArray::add(const long & a)

{

extend(Nelements + 1, false);

e[Nelements++] = a;

return;

}


////////////////////////////////////////////////////////////////////////


void LongArray::add(const LongArray & a)

{

int j;

extend(Nelements + a.n_elements());

for (j=0; j<(a.n_elements()); ++j)  {

   add(a[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


int LongArray::has(const long l) const

{

int j;

for (j=0; j<Nelements; ++j)  {

   if ( e[j] == l )  return ( 1 );

}

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


long & LongArray::operator[](int n) const

{

if ( (n < 0) || (n >= Nelements) )  {

   mlog << Error << "\nLongArray::operator[](int) -> "
        << "range check error ... " << n << "\n\n";

   exit ( 1 );
}

return ( e[n] );

}


////////////////////////////////////////////////////////////////////////


