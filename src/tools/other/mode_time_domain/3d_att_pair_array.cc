

////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util.h"

#include "3d_att_pair_array.h"


////////////////////////////////////////////////////////////////////////


   //
   //   Code for class PairAtt3DArray
   //


////////////////////////////////////////////////////////////////////////


PairAtt3DArray::PairAtt3DArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


PairAtt3DArray::~PairAtt3DArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


PairAtt3DArray::PairAtt3DArray(const PairAtt3DArray & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


PairAtt3DArray & PairAtt3DArray::operator=(const PairAtt3DArray & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void PairAtt3DArray::init_from_scratch()

{

e = (PairAtt3D *) 0;

AllocInc = 100;   //  default value

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3DArray::clear()

{

if ( e )  { delete [] e;  e = (PairAtt3D *) 0; }



Nelements = 0;

Nalloc = 0;

// AllocInc = 100;   //  don't reset AllocInc


return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3DArray::assign(const PairAtt3DArray & a)

{

clear();

if ( a.n_elements() == 0 )  return;

add(a);

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3DArray::extend(int N)

{

if ( N <= Nalloc )  return;

N = AllocInc*( (N + AllocInc - 1)/AllocInc );

int j;
PairAtt3D * u = new PairAtt3D [N];

if ( !u )  {

   mlog << Error << "PairAtt3DArray::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

for(j=0; j<Nelements; ++j)  {

   u[j] = e[j];

}

if ( e )  { delete [] e;  e = (PairAtt3D *) 0; }

e = u;

u = (PairAtt3D *) 0;

Nalloc = N;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3DArray::dump(ostream & out, int depth) const

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


void PairAtt3DArray::set_alloc_inc(int N)

{

if ( N < 0 )  {

   mlog << Error << "PairAtt3DArray::set_alloc_int(int) -> bad value ... " << N << "\n\n";

   exit ( 1 );

}

if ( N == 0 )  AllocInc = 100;   //  default value
else           AllocInc = N;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3DArray::add(const PairAtt3D & a)

{

extend(Nelements + 1);

e[Nelements++] = a;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3DArray::add(const PairAtt3DArray & a)

{

int j;

extend(Nelements + a.n_elements());

for (j=0; j<(a.n_elements()); ++j)  {

   add(a[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


PairAtt3D & PairAtt3DArray::operator[](int N) const

{

if ( (N < 0) || (N >= Nelements) )  {

   mlog << Error << "\n\n  PairAtt3DArray::operator[](int) -> range check error ... " << N << "\n\n";

   exit ( 1 );
}

return ( e[N] );

}


////////////////////////////////////////////////////////////////////////


int PairAtt3DArray::fcst_obj_number(int k) const

{

if ( (k < 0) || (k >= Nelements) )  {

   mlog << Error << "\n\n  PairAtt3DArray::fcst_obj_number(int) -> range check error\n\n";

   exit ( 1 );

}

return ( e[k].fcst_obj_number() );

}


////////////////////////////////////////////////////////////////////////


int PairAtt3DArray::obs_obj_number(int k) const

{

if ( (k < 0) || (k >= Nelements) )  {

   mlog << Error << "\n\n  PairAtt3DArray::obs_obj_number(int) -> range check error\n\n";

   exit ( 1 );

}

return ( e[k].obs_obj_number() );

}


////////////////////////////////////////////////////////////////////////


double PairAtt3DArray::total_interest(int k) const

{

if ( (k < 0) || (k >= Nelements) )  {

   mlog << Error << "\n\n  PairAtt3DArray::total_interest(int) -> range check error\n\n";

   exit ( 1 );

}

return ( e[k].total_interest() );

}


////////////////////////////////////////////////////////////////////////


