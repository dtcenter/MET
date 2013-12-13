

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

#include "mode_line_array.h"


////////////////////////////////////////////////////////////////////////


   //
   //   Code for class ModeLineArray
   //


////////////////////////////////////////////////////////////////////////


ModeLineArray::ModeLineArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ModeLineArray::~ModeLineArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ModeLineArray::ModeLineArray(const ModeLineArray & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


ModeLineArray & ModeLineArray::operator=(const ModeLineArray & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ModeLineArray::init_from_scratch()

{

e = (ModeLine **) 0;

AllocInc = 50;   //  default value

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void ModeLineArray::clear()

{

if ( e )  {

   int j;

   for (j=0; j<Nalloc; ++j)  {

      if ( e[j] )  { delete e[j];  e[j] = (ModeLine *) 0; }

   }

   delete [] e;  e = (ModeLine **) 0;

}   //  if e


Nelements = 0;

Nalloc = 0;

// AllocInc = 50;   //  don't reset AllocInc


return;

}


////////////////////////////////////////////////////////////////////////


void ModeLineArray::assign(const ModeLineArray & a)

{

clear();

if ( a.n_elements() == 0 )  return;

add(a);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeLineArray::extend(int n)

{

if ( n <= Nalloc )  return;

n = AllocInc*( (n + AllocInc - 1)/AllocInc );

int j;
ModeLine ** u = new ModeLine * [n];

if ( !u )  {

   cerr << "ModeLineArray::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

memset(u, 0, n*sizeof(ModeLine *));

for(j=0; j<Nelements; ++j)  {

   u[j] = e[j];

}

if ( e )  { delete [] e;  e = (ModeLine **) 0; }

e = u;

u = (ModeLine **) 0;

Nalloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeLineArray::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Nelements = " << Nelements << "\n";
out << prefix << "Nalloc    = " << Nalloc    << "\n";
out << prefix << "AllocInc  = " << AllocInc  << "\n";

int j;

for(j=0; j<Nelements; ++j)  {

   out << prefix << "Element # " << j << " ... \n";

   e[j]->dump(out, depth + 1);

}

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void ModeLineArray::set_alloc_inc(int n)

{

if ( n < 0 )  {

   cerr << "ModeLineArray::set_alloc_int(int) -> bad value ... " << n << "\n\n";

   exit ( 1 );

}

if ( n == 0 )  AllocInc = 50;   //  default value
else           AllocInc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeLineArray::add(const ModeLine & a)

{

extend(Nelements + 1);

e[Nelements] = new ModeLine;

if ( !(e[Nelements]) )  {

   cerr << "ModeLineArray::add(const ModeLine &) -> memory allocation error\n\n";

   exit ( 1 );

}

*(e[Nelements++]) = a;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeLineArray::add(const ModeLineArray & a)

{

int j;

extend(Nelements + a.n_elements());

for (j=0; j<(a.n_elements()); ++j)  {

   add(a[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


ModeLine ModeLineArray::operator[](int n) const

{

if ( (n < 0) || (n >= Nelements) )  {

   cerr << "\n\n  ModeLineArray::operator[](int) -> range check error ... " << n << "\n\n";

   exit ( 1 );
}

return ( *(e[n]) );

}


////////////////////////////////////////////////////////////////////////


