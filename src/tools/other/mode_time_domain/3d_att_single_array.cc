

////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util.h"

#include "3d_att_single_array.h"


////////////////////////////////////////////////////////////////////////


   //
   //   Code for class SingleAtt3DArray
   //


////////////////////////////////////////////////////////////////////////


SingleAtt3DArray::SingleAtt3DArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


SingleAtt3DArray::~SingleAtt3DArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


SingleAtt3DArray::SingleAtt3DArray(const SingleAtt3DArray & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


SingleAtt3DArray & SingleAtt3DArray::operator=(const SingleAtt3DArray & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3DArray::init_from_scratch()

{

e = (SingleAtt3D *) 0;

AllocInc = 100;   //  default value

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3DArray::clear()

{

if ( e )  { delete [] e;  e = (SingleAtt3D *) 0; }



Nelements = 0;

Nalloc = 0;

// AllocInc = 100;   //  don't reset AllocInc


return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3DArray::assign(const SingleAtt3DArray & a)

{

clear();

if ( a.n_elements() == 0 )  return;

add(a);

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3DArray::extend(int N)

{

if ( N <= Nalloc )  return;

N = AllocInc*( (N + AllocInc - 1)/AllocInc );

int j;
SingleAtt3D * u = new SingleAtt3D [N];

if ( !u )  {

   cerr << "SingleAtt3DArray::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

for(j=0; j<Nelements; ++j)  {

   u[j] = e[j];

}

if ( e )  { delete [] e;  e = (SingleAtt3D *) 0; }

e = u;

u = (SingleAtt3D *) 0;

Nalloc = N;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3DArray::dump(ostream & out, int depth) const

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


void SingleAtt3DArray::set_alloc_inc(int N)

{

if ( N < 0 )  {

   cerr << "SingleAtt3DArray::set_alloc_int(int) -> bad value ... " << N << "\n\n";

   exit ( 1 );

}

if ( N == 0 )  AllocInc = 100;   //  default value
else           AllocInc = N;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3DArray::add(const SingleAtt3D & a)

{

extend(Nelements + 1);

e[Nelements++] = a;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3DArray::add(const SingleAtt3DArray & a)

{

int j;

extend(Nelements + a.n_elements());

for (j=0; j<(a.n_elements()); ++j)  {

   add(a[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


SingleAtt3D & SingleAtt3DArray::operator[](int N) const

{

if ( (N < 0) || (N >= Nelements) )  {

   cerr << "\n\n  SingleAtt3DArray::operator[](int) -> range check error ... " << N << "\n\n";

   exit ( 1 );
}

return ( e[N] );

}


////////////////////////////////////////////////////////////////////////


