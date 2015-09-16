

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

// N_Fcst_Composites = 0;
// N_Obs_Composites  = 0;

calc.clear();

part.clear();

graph.clear();


return;

}


////////////////////////////////////////////////////////////////////////


void MM_Engine::assign(const MM_Engine & e)

{

clear();

// N_Fcst_Composites = e.N_Fcst_Composites;
// N_Obs_Composites  = e.N_Obs_Composites;

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
   //  get number of fcst and obs composites
   //

/*
int m;
bool has_fcst = false;
bool has_obs  = false;
const EquivalenceClass * eq = 0;


N_Fcst_Composites = N_Obs_Composites = 0;

for (j=0; j<(part.n_elements()); ++j)  {

   has_fcst = has_obs = false;

   eq = part(j);

   for (k=0; k<eq->n_elements(); ++k)  {

      m = eq->element(k);

      if ( m < graph.n_fcst() )  has_fcst = true;
      else                       has_obs  = true;

   }   //  for k

   if ( has_fcst )  ++N_Fcst_Composites;
   if ( has_obs  )  ++N_Obs_Composites;

}   //  for j
*/

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


int MM_Engine::composite_with_fcst (const int k) const

{

int j, fcst_num, index;

fcst_num = k;

for (j=0; j<(part.n_elements()); ++j)  {

   if ( part.has(fcst_num, index) )  return ( index );

}   //  for j


return ( -1 );

}


////////////////////////////////////////////////////////////////////////


int MM_Engine::composite_with_obs (const int k) const

{

int j, obs_num, index;

obs_num = k + graph.n_fcst();

for (j=0; j<(part.n_elements()); ++j)  {

   if ( part.has(obs_num, index) )  return ( index );

}   //  for j


return ( -1 );

}


////////////////////////////////////////////////////////////////////////


IntArray MM_Engine::fcst_composite(const int _composite_number) const

{

int j, k;
IntArray a;
const EquivalenceClass * eq = part(_composite_number);   //  this does range checking

for (j=0; j<(eq->n_elements()); ++j)  {

   k = eq->element(j);

   if ( k < graph.n_fcst() )  a.add(k);

}

return ( a );

}


////////////////////////////////////////////////////////////////////////


IntArray MM_Engine::obs_composite(const int _composite_number) const

{

int j, k;
IntArray a;
const EquivalenceClass * eq = part(_composite_number);   //  this does range checking

for (j=0; j<(eq->n_elements()); ++j)  {

   k = eq->element(j);

   if ( k >= graph.n_fcst() )  a.add(k - graph.n_fcst());

}

return ( a );

}


////////////////////////////////////////////////////////////////////////


int MM_Engine::map_fcst_id_to_composite(const int id) const   //  zero-based

{

int j, k;


k = id;

j = part.which_class(k);


return ( j );

}


////////////////////////////////////////////////////////////////////////


int MM_Engine::map_obs_id_to_composite(const int id) const   //  zero-based

{

int j, k;


k = id + graph.n_fcst();

j = part.which_class(k);


return ( j );

}


////////////////////////////////////////////////////////////////////////


