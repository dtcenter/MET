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

#include "fo_node_array.h"


////////////////////////////////////////////////////////////////////////


   //
   //   Code for class FO_Node_Array
   //


////////////////////////////////////////////////////////////////////////


FO_Node_Array::FO_Node_Array()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


FO_Node_Array::~FO_Node_Array()

{

clear();

}


////////////////////////////////////////////////////////////////////////


FO_Node_Array::FO_Node_Array(const FO_Node_Array & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


FO_Node_Array & FO_Node_Array::operator=(const FO_Node_Array & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void FO_Node_Array::init_from_scratch()

{

e = (FO_Node *) 0;

AllocInc = 30;   //  default value

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Node_Array::clear()

{

if ( e )  { delete [] e;  e = (FO_Node *) 0; }



Nelements = 0;

Nalloc = 0;

// AllocInc = 30;   //  don't reset AllocInc


return;

}


////////////////////////////////////////////////////////////////////////


void FO_Node_Array::assign(const FO_Node_Array & a)

{

clear();

if ( a.n_elements() == 0 )  return;

add(a);

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Node_Array::extend(int N)

{

if ( N <= Nalloc )  return;

N = AllocInc*( (N + AllocInc - 1)/AllocInc );

int j;
FO_Node * u = new FO_Node [N];

if ( !u )  {

   mlog << Error << "FO_Node_Array::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

for(j=0; j<Nelements; ++j)  {

   u[j] = e[j];

}

if ( e )  { delete [] e;  e = (FO_Node *) 0; }

e = u;

u = (FO_Node *) 0;

Nalloc = N;

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Node_Array::dump(ostream & out, int depth) const

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


void FO_Node_Array::set_alloc_inc(int N)

{

if ( N < 0 )  {

   mlog << Error << "FO_Node_Array::set_alloc_int(int) -> bad value ... " << N << "\n\n";

   exit ( 1 );

}

if ( N == 0 )  AllocInc = 30;   //  default value
else           AllocInc = N;

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Node_Array::add(const FO_Node & a)

{

extend(Nelements + 1);

e[Nelements++] = a;

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Node_Array::add(const FO_Node_Array & a)

{

int j;

extend(Nelements + a.n_elements());

for (j=0; j<(a.n_elements()); ++j)  {

   add(a[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


FO_Node & FO_Node_Array::operator[](int N) const

{

if ( (N < 0) || (N >= Nelements) )  {

   mlog << Error << "\n\n  FO_Node_Array::operator[](int) -> range check error ... " << N << "\n\n";

   exit ( 1 );
}

return ( e[N] );

}


////////////////////////////////////////////////////////////////////////


