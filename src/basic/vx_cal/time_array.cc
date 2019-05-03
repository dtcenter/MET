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


#include "vx_cal.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class TimeArray
   //


////////////////////////////////////////////////////////////////////////


TimeArray::TimeArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


TimeArray::~TimeArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


TimeArray::TimeArray(const TimeArray & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


TimeArray & TimeArray::operator=(const TimeArray & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void TimeArray::init_from_scratch()

{

e = (unixtime*) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::clear()

{

if ( e )  { delete [] e;  e = (unixtime *) 0; }

Nelements = Nalloc = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::assign(const TimeArray & a)

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


void TimeArray::extend(int n)

{

if ( Nalloc >= n )  return;

int k;

k = n/time_array_alloc_inc;

if ( n%time_array_alloc_inc )  ++k;

n = k*time_array_alloc_inc;

unixtime * u = (unixtime *) 0;

u = new unixtime [n];

if ( !u )  {

   cerr << "\n\n  void TimeArray::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

int j;

memset(u, 0, n*sizeof(unixtime));

if ( e )  {

   for (j=0; j<Nelements; ++j)  {

      u[j] = e[j];

   }

   delete [] e;  e = (unixtime *) 0;

}

e = u; u = (unixtime *) 0;

Nalloc = n;


return;

}


////////////////////////////////////////////////////////////////////////


unixtime TimeArray::operator[](int n) const

{

if ( (n < 0) || (n >= Nelements) )  {

   cerr << "\n\n  TimeArray::operator[](int) const -> "
       << "range check error\n\n";

   exit ( 1 );

}

return ( e[n] );

}


////////////////////////////////////////////////////////////////////////


int TimeArray::has(unixtime u) const

{

int j;

for (j=0; j<Nelements; ++j)  {

   if ( e[j] == u )  return ( 1 );

}

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void TimeArray::add(unixtime u)

{

extend(Nelements + 1);

e[Nelements++] = u;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::add(const TimeArray & a)

{

extend(Nelements + a.Nelements);

int j;

for (j=0; j<(a.Nelements); ++j)  {

   e[Nelements++] = a.e[j];

}


return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::set(int n, unixtime u)

{

if ( (n < 0) || (n >= Nelements) )  {

   cerr << "\n\n  TimeArray::set(int, unixtime) -> range check error\n\n";

   exit ( 1 );

}

e[n] = u;

return;

}


////////////////////////////////////////////////////////////////////////


unixtime TimeArray::min() const

{

int j;
unixtime u;

for(j=0, u=e[0]; j<Nelements; j++) {
   if(e[j] < u) u = e[j];
}

return(u);

}


////////////////////////////////////////////////////////////////////////


unixtime TimeArray::max() const

{

int j;
unixtime u;

for(j=0, u=e[0]; j<Nelements; j++) {
   if(e[j] > u) u = e[j];
}

return(u);

}


////////////////////////////////////////////////////////////////////////
