// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util.h"

#include "fo_node_array.h"

using namespace std;


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

if ( this == &a )  return *this;

assign(a);

return *this;

}


////////////////////////////////////////////////////////////////////////


void FO_Node_Array::init_from_scratch()

{

AllocInc = 30;   //  default value

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Node_Array::clear()

{

e.clear();

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


void FO_Node_Array::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Nelements = " << e.size() << "\n";
out << prefix << "Nalloc    = " << e.capacity() << "\n";
out << prefix << "AllocInc  = " << AllocInc  << "\n";

int j;

for(j=0; j<(int) e.size(); ++j)  {

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

   mlog << Error << "\nFO_Node_Array::set_alloc_int(int) -> "
        << "bad value ... " << N << "\n\n";

   exit ( 1 );

}

if ( N == 0 )  AllocInc = 30;   //  default value
else           AllocInc = N;

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Node_Array::add(const FO_Node & a)

{

e.push_back(a);

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Node_Array::add(const FO_Node_Array & a)

{

int j;

e.reserve(e.size() + a.n_elements());

for (j=0; j<(a.n_elements()); ++j)  {

   add(a[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


FO_Node & FO_Node_Array::operator[](int N) const

{

if ( (N < 0) || (N >= (int) e.size()) )  {

   mlog << Error << "\nFO_Node_Array::operator[](int) -> "
        << "range check error ... " << N << "\n\n";

   exit ( 1 );
}

return const_cast<FO_Node &>(e[N]);

}


////////////////////////////////////////////////////////////////////////


