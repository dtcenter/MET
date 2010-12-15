// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
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

#include "is_bad_data.h"
#include "ptile.h"

#include "double_array.h"
#include "indent.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class DoubleArray
   //


////////////////////////////////////////////////////////////////////////


DoubleArray::DoubleArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


DoubleArray::~DoubleArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


DoubleArray::DoubleArray(const DoubleArray & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


DoubleArray & DoubleArray::operator=(const DoubleArray & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void DoubleArray::init_from_scratch()

{

e = (double *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void DoubleArray::clear()

{

if ( e )  { delete [] e;  e = (double *) 0; }

Nelements = Nalloc = 0;

StatsDone = 0;

Min = Max = 0.0;

P10 = P25 = P50 = P75 = P90 = 0.0;

Mean = StdDev = Sum = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


void DoubleArray::assign(const DoubleArray & a)

{

clear();

if ( a.Nelements == 0 )  return;

extend(a.Nelements);

int j;

for (j=0; j<(a.Nelements); ++j)  {

   e[j] = a.e[j];

}

Nelements = a.Nelements;

StatsDone = a.StatsDone;

Min = a.Min;
Max = a.Max;

P10 = a.P10;
P25 = a.P25;
P50 = a.P50;
P75 = a.P75;
P90 = a.P90;

Mean = a.Mean;
StdDev = a.StdDev;

return;

}


////////////////////////////////////////////////////////////////////////


void DoubleArray::extend(int n)

{

if ( Nalloc >= n )  return;

int k;

k = n/doublearray_alloc_inc;

if ( n%doublearray_alloc_inc )  ++k;

n = k*doublearray_alloc_inc;

double * u = (double *) 0;

u = new double [n];

if ( !u )  {

   cerr << "\n\n  void DoubleArray::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

int j;

memset(u, 0, n*sizeof(double));

if ( e )  {

   for (j=0; j<Nelements; ++j)  {

      u[j] = e[j];

   }

   delete [] e;  e = (double *) 0;

}

e = u;  u = (double *) 0;

Nalloc = n;



return;

}


////////////////////////////////////////////////////////////////////////


void DoubleArray::dump(ostream & out, int depth) const

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


double DoubleArray::operator[](int n) const

{

if ( (n < 0) || (n >= Nelements) )  {

   cerr << "\n\n  DoubleArray::operator[](int) const -> range check error\n\n";

   exit ( 1 );

}

return ( e[n] );

}


////////////////////////////////////////////////////////////////////////


int DoubleArray::has(double t) const

{

int j;

for (j=0; j<Nelements; ++j)  {

   if ( is_eq(e[j], t) )  return ( 1 );

}

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void DoubleArray::add(double t)

{

if ( StatsDone )  {

   cerr << "\n\n  DoubleArray::add(double) ->  can't add to array after stats have been calculated\n\n";

   exit ( 1 );

}

extend(Nelements + 1);

e[Nelements++] = t;

return;

}


////////////////////////////////////////////////////////////////////////


void DoubleArray::add(const DoubleArray & a)

{

if ( StatsDone )  {

   cerr << "\n\n  DoubleArray::add(const DoubleArray &) ->  can't add to array after stats have been calculated\n\n";

   exit ( 1 );

}

extend(Nelements + a.Nelements);

int j;

for (j=0; j<(a.Nelements); ++j)  {

   e[Nelements++] = a.e[j];

}


return;

}


////////////////////////////////////////////////////////////////////////


void DoubleArray::do_stats()

{

if ( StatsDone )  return;

if ( Nelements == 0 )  return;

   //
   //  sort the elements in the array
   //

sort(e, Nelements);

Min = e[0];
Max = e[Nelements - 1];
Sum = 0.0;

P10 = percentile(e, Nelements, 0.10);
P25 = percentile(e, Nelements, 0.25);
P50 = percentile(e, Nelements, 0.50);
P75 = percentile(e, Nelements, 0.75);
P90 = percentile(e, Nelements, 0.90);

   //
   //  mean and standard deviation
   //

int j;
double x;
double sx, sxx;


sx = sxx = 0.0;

for (j=0; j<Nelements; ++j)  {

   x = e[j];

   sx  += x;
   sxx += x*x;

}

Sum  = sx;

sx  /= Nelements;
sxx /= Nelements;

Mean = sx;

StdDev = sxx - Mean*Mean;

if ( StdDev < 0.0 )  {   //  this could happen due to round-off error

   StdDev = 0.0;

} else {

   StdDev = sqrt(StdDev);

}

   //
   //  done
   //

StatsDone = 1;

return;

}


////////////////////////////////////////////////////////////////////////






