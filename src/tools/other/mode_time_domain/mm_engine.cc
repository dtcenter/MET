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
#include <cmath>

#include "mm_engine.h"

using namespace std;


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

if ( this == &e )  return *this;

assign(e);

return *this;

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

N_Composites = 0;

comp_to_eq.clear();

calc.clear();

part.clear();

graph.clear();


return;

}


////////////////////////////////////////////////////////////////////////


void MM_Engine::assign(const MM_Engine & e)

{

clear();

N_Composites = e.N_Composites;

comp_to_eq = e.comp_to_eq;

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

for (int j=0; j<(graph.n_total()); ++j)  {

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

int j;

   //
   //  fcst, obs
   //

for (j=0; j<(graph.n_fcst()); ++j)  {

   int f_i = graph.f_index(j);

   for (int k=0; k<(graph.n_obs()); ++k)  {

      int o_i = graph.o_index(k);

      if ( ! graph.has_fo_edge(j, k) )  continue;

      part.merge_values(f_i, o_i);

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


const EquivalenceClass * eq = nullptr;

N_Composites = 0;
IntArray index_list;

for (j=0; j<(part.n_elements()); ++j)  {

   eq = part(j);

   if ( eq->n_elements() <= 1 )  continue;

   index_list.add(j);

   ++N_Composites;

}   //  for j

comp_to_eq.clear();

for (j=0; j<index_list.n(); ++j)  {

   comp_to_eq.add(index_list[j]);

}

if ( mlog.verbosity_level() > 5 )  {

   ConcatString s;

   s << "Composites ...\n";

   for (j=0; j<comp_to_eq.n(); ++j)  {

      s << ' ' << comp_to_eq[j];

   }

   mlog << Debug(6) << s << "\n";

}

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


void MM_Engine::partition_dump(int verbosity) const

{

part.specialized_dump(verbosity, graph.n_fcst(), graph.n_obs());

return;

}


////////////////////////////////////////////////////////////////////////


int MM_Engine::composite_with_fcst (const int k) const

{

const EquivalenceClass * eq = nullptr;

int fcst_num = k;

for (int j=0; j<N_Composites; ++j)  {

   int m = comp_to_eq[j];

   eq = part(m);

   if ( eq->has(fcst_num) )  return j;

}

return -1;

}


////////////////////////////////////////////////////////////////////////


int MM_Engine::composite_with_obs (const int k) const

{

const EquivalenceClass * eq = nullptr;

int obs_num = k + graph.n_fcst();

for (int j=0; j<N_Composites; ++j)  {

   int m = comp_to_eq[j];

   eq = part(m);

   if ( eq->has(obs_num) )  return j;

}

return -1;

}


////////////////////////////////////////////////////////////////////////


IntArray MM_Engine::fcst_composite(const int _composite_number) const

{

IntArray a;
const EquivalenceClass * eq = part(comp_to_eq[_composite_number]);   //  this does range checking

for (int j=0; j<(eq->n_elements()); ++j)  {

   int k = eq->element(j);

   if ( k < graph.n_fcst() )  a.add(k);

}

return a;

}


////////////////////////////////////////////////////////////////////////


IntArray MM_Engine::obs_composite(const int _composite_number) const

{

IntArray a;
const EquivalenceClass * eq = part(comp_to_eq[_composite_number]);   //  this does range checking

for (int j=0; j<(eq->n_elements()); ++j)  {

   int k = eq->element(j);

   if ( k >= graph.n_fcst() )  a.add(k - graph.n_fcst());

}

return a;

}


////////////////////////////////////////////////////////////////////////


int MM_Engine::map_fcst_id_to_composite(const int id) const   //  zero-based

{

int k = id;

int j = part.which_class(k);

for (int m=0; m<N_Composites; ++m)  {

   if ( comp_to_eq[m] == j )  return m;

}


return -1;

}


////////////////////////////////////////////////////////////////////////


int MM_Engine::map_obs_id_to_composite(const int id) const   //  zero-based

{

int k = id + graph.n_fcst();

int j = part.which_class(k);

for (int m=0; m<N_Composites; ++m)  {

   if ( comp_to_eq[m] == j )  return m;

}


return -1;

}


////////////////////////////////////////////////////////////////////////


