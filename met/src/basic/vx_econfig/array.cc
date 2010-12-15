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
#include <cmath>

#include "array.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ArrayInfo
   //


////////////////////////////////////////////////////////////////////////


ArrayInfo::ArrayInfo()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ArrayInfo::~ArrayInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ArrayInfo::ArrayInfo(const ArrayInfo & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


ArrayInfo & ArrayInfo::operator=(const ArrayInfo & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ArrayInfo::init_from_scratch()

{

int j;

Dim = 0;

Nalloc = 0;

for (j=0; j<max_array_dim; ++j)  sizes[j] = 0;

v = (IcodeVector **) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void ArrayInfo::clear()

{

int j;



delete_v();  //  gotta be first



Dim = 0;

Nalloc = 0;

for (j=0; j<max_array_dim; ++j)  sizes[j] = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void ArrayInfo::assign(const ArrayInfo & a)

{

clear();

int j;


Dim = a.Dim;

Nalloc = a.Nalloc;

for (j=0; j<Dim; ++j)  sizes[j] = a.sizes[j];

v = new IcodeVector * [Nalloc];

if ( !v )  {

   cerr << "\n\n  void ArrayInfo::assign(const ArrayInfo &) -> memory allocation error\n\n";

   exit ( 1 );

}


for (j=0; j<Nalloc; ++j)  {

   v[j] = (IcodeVector *) 0;

}


for (j=0; j<Nalloc; ++j)  {

   put(j, *(a.v[j]));

}




return;

}


////////////////////////////////////////////////////////////////////////


void ArrayInfo::delete_v()

{

if ( !v )  return;

int j;


for (j=0; j<Nalloc; ++j)  {

   if ( v[j] )  { delete v[j];  v[j] = (IcodeVector *) 0; }

}


delete [] v;  v = (IcodeVector **) 0;

Nalloc = 0;

return;

}


////////////////////////////////////////////////////////////////////////


int ArrayInfo::size(int k) const

{

if ( (k < 0) || (k >= Dim) )  {

   cerr << "\n\n  int ArrayInfo::size(int) const -> range check error\n\n";

   exit ( 1 );

}

return ( sizes[k] );

}


////////////////////////////////////////////////////////////////////////


int ArrayInfo::indices_to_n(const int * indices) const

{

int j, k, n;
int scale, offset;


for (j=0; j<Dim; ++j)  {

   k = indices[j];

   if ( (k < 0) || (k >= sizes[j]) )  {

      cerr << "\n\n  ArrayInfo::indices_to_n() const -> range check error in array indice(s)\n\n";

      exit ( 1 );

   }

}


n = indices[0];


for (j=1; j<Dim; ++j)  {   //  j starts at ONE here, not zero

   scale  = sizes[j];

   offset = indices[j];

   n = n*scale + offset;

}



return ( n );

}


////////////////////////////////////////////////////////////////////////


void ArrayInfo::n_to_indices(int n, int * indices) const

{

int j;


for (j=(Dim - 1); j>=0; --j)  {

   indices[j] = n%(sizes[j]);

   n /= sizes[j];

}



return;

}


////////////////////////////////////////////////////////////////////////


void ArrayInfo::set(const int * s, int d)

{

if ( d > max_array_dim )  {

   cerr << "\n\n  ArrayInfo::set_sizes(const int *, int) -> bad dimension\n\n";

   exit ( 1 );

}

clear();

int j;

Dim = d;

for (j=0; j<Dim; ++j)  sizes[j] = s[j];

Nalloc = 1;

for (j=0; j<Dim; ++j)  Nalloc *= sizes[j];


v = new IcodeVector * [Nalloc];

if ( !v )  {

   cerr << "\n\n  void ArrayInfo::set(const int *, int) -> memory allocation error\n\n";

   exit ( 1 );

}


for (j=0; j<Nalloc; ++j)  v[j] = (IcodeVector *) 0;


return;

}


////////////////////////////////////////////////////////////////////////


void ArrayInfo::put(int n, const IcodeVector & icv)

{

if ( (n < 0) || (n >= Nalloc) )  {

   cerr << "\n\n  void ArrayInfo::put(int, const IcodeVector &) -> range check error\n\n";

   exit ( 1 );

}


if ( !(v[n]) )  v[n] = new IcodeVector;

if ( !(v[n]) )  {

   cerr << "\n\n  void ArrayInfo::put(int, const IcodeVector &) -> range check error\n\n";

   exit ( 1 );

}


*(v[n]) = icv;


return;

}


////////////////////////////////////////////////////////////////////////


void ArrayInfo::put(const int * indices, const IcodeVector & icv)

{

int n = indices_to_n(indices);


put(n, icv);


return;

}


////////////////////////////////////////////////////////////////////////


const IcodeVector * ArrayInfo::get(int n) const

{

if ( (n < 0) || (n >= Nalloc) )  {

   cerr << "\n\n  void ArrayInfo::get(int) -> range check error\n\n";

   exit ( 1 );

}


return ( v[n] );

}


////////////////////////////////////////////////////////////////////////


const IcodeVector * ArrayInfo::get(const int * indices) const

{

int n = indices_to_n(indices);

return ( get(n) );

}


////////////////////////////////////////////////////////////////////////


void ArrayInfo::dump(ostream & out, int indent_depth) const

{

int j, k;
Indent prefix;
int indices[max_array_dim];


prefix.depth = indent_depth;

out << prefix << "Dim = " << Dim << "\n";

out << prefix << "Nalloc = " << Nalloc << "\n";

for (j=0; j<Dim; ++j)  {

   out << prefix << "Size[" << j << "] = " << sizes[j] << "\n";

}

for (j=0; j<Nalloc; ++j)  {

   n_to_indices(j, indices);

   out << prefix << "Array Entry ";

   for (k=0; k<Dim; ++k)  {

      out << '[' << (indices[k]) << ']';

      // if ( (Dim > 1) && (k < (Dim - 1)) )  out << ", ";

   }

   out << " ... \n";

   v[j]->dump(out, indent_depth + 1);

}




return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////







