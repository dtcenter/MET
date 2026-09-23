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

#include "2d_att_array.h"

using namespace std;


////////////////////////////////////////////////////////////////////////


   //
   //   Code for class SingleAtt2DArray
   //


////////////////////////////////////////////////////////////////////////


SingleAtt2DArray::SingleAtt2DArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


SingleAtt2DArray::~SingleAtt2DArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


SingleAtt2DArray::SingleAtt2DArray(const SingleAtt2DArray & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


SingleAtt2DArray & SingleAtt2DArray::operator=(const SingleAtt2DArray & a)

{

if ( this == &a )  return *this;

assign(a);

return *this;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2DArray::init_from_scratch()

{

AllocInc = 50;   //  default value

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2DArray::clear()

{

e.clear();


return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2DArray::assign(const SingleAtt2DArray & a)

{

clear();

if ( a.n_elements() == 0 )  return;

add(a);

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2DArray::dump(ostream & out, int depth) const

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


void SingleAtt2DArray::set_alloc_inc(int N)

{

if ( N < 0 )  {

   mlog << Error << "\nSingleAtt2DArray::set_alloc_int(int) -> "
        << "bad value ... " << N << "\n\n";

   exit ( 1 );

}

if ( N == 0 )  AllocInc = 50;   //  default value
else           AllocInc = N;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2DArray::add(const SingleAtt2D & a)

{

e.push_back(a);

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2DArray::add(const SingleAtt2DArray & a)

{

int j;

e.reserve(e.size() + a.n_elements());

for (j=0; j<(a.n_elements()); ++j)  {

   add(a[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


SingleAtt2D & SingleAtt2DArray::operator[](int N) const

{

if ( (N < 0) || (N >= (int) e.size()) )  {

   mlog << Error << "\nSingleAtt2DArray::operator[](int) -> "
        << "range check error ... " << N << "\n\n";

   exit ( 1 );
}

return const_cast<SingleAtt2D &>(e[N]);

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2DArray::patch_cluster_numbers(const MM_Engine & engine)

{

int j, s_id, c_id;

for (j=0; j<(int) e.size(); ++j)  {

   s_id = e[j].object_number();   //  1-based

   if ( e[j].is_fcst() )   c_id = engine.map_fcst_id_to_composite (s_id - 1);   // 0-based
   else                    c_id = engine.map_obs_id_to_composite  (s_id - 1);   // 0-based

   e[j].set_cluster_number(c_id + 1);   //  1-based

}   //  for j


return;

}


////////////////////////////////////////////////////////////////////////


unixtime SingleAtt2DArray::valid_time(int index) const

{

if ( (index < 0) || (index >= (int) e.size()) )  {

   mlog << Error << "\nSingleAtt2DArray::valid_time(int) const -> "
        << "range check error\n\n";

   exit ( 1 );

}

return e[index].valid_time();

}


////////////////////////////////////////////////////////////////////////


int SingleAtt2DArray::lead_time(int index) const

{

if ( (index < 0) || (index >= (int) e.size()) )  {

   mlog << Error << "\nSingleAtt2DArray::lead_time(int) const -> "
        << "range check error\n\n";

   exit ( 1 );

}

return e[index].lead_time();

}


////////////////////////////////////////////////////////////////////////


int SingleAtt2DArray::time_index(int index) const

{

if ( (index < 0) || (index >= (int) e.size()) )  {

   mlog << Error << "\nSingleAtt2DArray::time_index(int) const -> "
        << "range check error\n\n";

   exit ( 1 );

}

return e[index].time_index();

}


////////////////////////////////////////////////////////////////////////


