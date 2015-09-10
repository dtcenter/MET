

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "mm_engine.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MM_Engine
   //


////////////////////////////////////////////////////////////////////////


MM_Engine::MM_Engine()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


MM_Engine::~MM_Engine()

{

clear();

}


////////////////////////////////////////////////////////////////////////


MM_Engine::MM_Engine(const MM_Engine & e)

{

init_from_scratch();

assign(e);

}


////////////////////////////////////////////////////////////////////////


MM_Engine & MM_Engine::operator=(const MM_Engine & e)

{

if ( this == &e )  return ( * this );

assign(e);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void MM_Engine::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void MM_Engine::clear()

{

calc.clear();

part.clear();

graph.clear();


return;

}


////////////////////////////////////////////////////////////////////////


void MM_Engine::assign(const MM_Engine & e)

{

clear();

calc = e.calc;

part = e.part;

graph = e.graph;

return;

}


////////////////////////////////////////////////////////////////////////


void MM_Engine::set_size(const int _n_fcst, const int _n_obs)

{

graph.set_size(_n_fcst, _n_obs);

   //
   //  set up the initial partition
   //

int j;

for (j=0; j<(graph.n_total()); ++j)  {

   part.add_no_repeat(j);

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MM_Engine::do_match_merge()

{

int j, k;
int f_i, o_i;


   //
   //  fcst, obs
   //

for (j=0; j<(graph.n_fcst()); ++j)  {

   f_i = graph.f_index(j);

   for (k=0; k<(graph.n_obs()); ++k)  {

      o_i = graph.o_index(k);

      if ( ! graph.has_fo_edge(j, k) )  continue;

      // cout << "\n  Merging fcst " << j << ", obs " << k << '\n' << flush;

      part.merge_values(f_i, o_i);

      // specialzed_dump(graph.n_fcst(), graph.n_obs(), p);

   }   //  for k

}   //  for j


   //
   //  fcst, fcst
   //

   //
   //  obs, obs
   //

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MM_Engine::partition_dump(ostream & out) const

{

part.specialized_dump(out, graph.n_fcst(), graph.n_obs());

return;

}


////////////////////////////////////////////////////////////////////////


