// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
#include <cstdio>
#include <cmath>

#include "ascii_table.h"
#include "fo_graph.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class FO_Graph
   //


////////////////////////////////////////////////////////////////////////


FO_Graph::FO_Graph()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


FO_Graph::~FO_Graph()

{

clear();

}


////////////////////////////////////////////////////////////////////////


FO_Graph::FO_Graph(const FO_Graph & g)

{

init_from_scratch();

assign(g);

}


////////////////////////////////////////////////////////////////////////


FO_Graph & FO_Graph::operator=(const FO_Graph & g)

{

if ( this == &g )  return ( * this );

assign(g);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void FO_Graph::init_from_scratch()

{

TheGraph = 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Graph::clear()

{

if ( TheGraph )  { delete [] TheGraph;  TheGraph = 0; }

N_fcst = 0;
N_obs  = 0;

N_total = 0;

N_nodes = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Graph::assign(const FO_Graph & g)

{

clear();

if ( ! (g.TheGraph) )  return;

const int N = g.N_fcst + g.N_obs;

N_nodes = N*N;

TheGraph = new FO_Node [N_nodes];

memcpy(TheGraph, g.TheGraph, N*sizeof(FO_Node));

   //
   //  done
   //

N_fcst = g.N_fcst;
N_obs  = g.N_obs;

N_total = g.N_total;

return;

}


////////////////////////////////////////////////////////////////////////


int FO_Graph::f_index(int f_num) const

{

if ( (f_num < 0) || (f_num >= N_fcst) )  {

   mlog << Error << "\n\n  FO_Graph::f_index(int f_num) const -> range check error!\n\n";

   exit ( 1 );

}

return ( f_num );

}


////////////////////////////////////////////////////////////////////////


int FO_Graph::o_index(int o_num) const

{

if ( (o_num < 0) || (o_num >= N_obs) )  {

   mlog << Error << "\n\n  FO_Graph::o_index(int o_num) const -> range check error!\n\n";

   exit ( 1 );

}

return ( N_fcst + o_num );

}


////////////////////////////////////////////////////////////////////////


void FO_Graph::set_size(int n_f, int n_o)

{

bool trouble = false;   //  in river city

if ( (n_f < 0) || (n_o < 0) )  trouble = true;

if ( (n_f == 0) && (n_o == 0) )  trouble = true;

if ( trouble )  {

   mlog << Error << "\n\n  FO_Graph::set_size(int n_f, int n_o) -> bad n_f or n_o value(s)\n\n";

   exit ( 1 );

}

clear();

N_fcst = n_f;
N_obs  = n_o;

N_total  = N_fcst + N_obs;

TheGraph = new FO_Node [N_total*N_total];




   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool FO_Graph::has_fo_edge(int n_f, int n_o) const

{

const int i_f = f_index(n_f);
const int i_o = o_index(n_o);

const int n = two_to_one(i_f, i_o);

return ( TheGraph[n].has_edge() );

}


////////////////////////////////////////////////////////////////////////


bool FO_Graph::has_ff_edge(int n_f_1, int n_f_2) const

{

const int i_f_1 = f_index(n_f_1);
const int i_f_2 = f_index(n_f_2);

const int n = two_to_one(i_f_1, i_f_2);

return ( TheGraph[n].has_edge() );

}


////////////////////////////////////////////////////////////////////////


bool FO_Graph::has_oo_edge(int n_o_1, int n_o_2) const

{

const int i_o_1 = o_index(n_o_1);
const int i_o_2 = o_index(n_o_2);

const int n = two_to_one(i_o_1, i_o_2);

return ( TheGraph[n].has_edge() );

}


////////////////////////////////////////////////////////////////////////


void FO_Graph::set_fo_edge(int n_f, int n_o)

{

const int i_f = f_index(n_f);
const int i_o = o_index(n_o);

const int n1 = two_to_one(i_f, i_o);
const int n2 = two_to_one(i_o, i_f);

TheGraph[n1].set_has_edge();
TheGraph[n2].set_has_edge();


return;

}


////////////////////////////////////////////////////////////////////////


void FO_Graph::set_ff_edge(int n_f_1, int n_f_2)

{

const int i_f_1 = f_index(n_f_1);
const int i_f_2 = f_index(n_f_2);

const int n1 = two_to_one(i_f_1, i_f_2);
const int n2 = two_to_one(i_f_2, i_f_1);

TheGraph[n1].set_has_edge();
TheGraph[n2].set_has_edge();

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Graph::set_oo_edge(int n_o_1, int n_o_2)

{

const int i_o_1 = o_index(n_o_1);
const int i_o_2 = o_index(n_o_2);

const int n1 = two_to_one(i_o_1, i_o_2);
const int n2 = two_to_one(i_o_2, i_o_1);

TheGraph[n1].set_has_edge();
TheGraph[n2].set_has_edge();

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Graph::erase_edges()

{

if ( ! TheGraph )  {

   mlog << Error << "\n\n  FO_Graph::erase_edges() -> empty graph!\n\n";

   exit ( 1 );

}

int j;
const int N2 = N_total*N_total;

for (j=0; j<N2; ++j)  {

   TheGraph[j].set_has_edge(false);

}

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Graph::do_dump_table(AsciiTable & table) const

{

if ( ! TheGraph )  {

   mlog << Error << "\n\n  FO_Graph::dump_as_table() -> empty graph!\n\n";

   exit ( 1 );

}

int r, c, j, k;
char junk[256];
const int fcst_start = 1;
const int fcst_stop  = fcst_start + N_fcst - 1;
const int  obs_start = fcst_stop + 2;
const int  obs_stop  = obs_start + N_obs - 1;


table.set_size(N_total + 2, N_total + 2);

// for (r=0; r<(table.nrows()); ++r)  {
// 
//    for (c=0; c<(table.ncols()); ++c)  {
// 
//       table.set_entry(r, c, '.');
// 
//    }
// 
// }

c = fcst_stop + 1;

for (r=1; r<(table.nrows()); ++r)  {

   table.set_entry(r, c, '|');
   table.set_entry(c, r, '-');

}

r = c = fcst_stop + 1;

table.set_entry(c, r, '+');

r = 0;

k = 0;

for (c=fcst_start; c<=fcst_stop; ++c)  {

   snprintf(junk, sizeof(junk), "F%d", k++);

   table.set_entry(r, c, junk);
   table.set_entry(c, r, junk);

}

k = 0;

for (c=obs_start; c<=obs_stop; ++c)  {

   snprintf(junk, sizeof(junk), "O%d", k++);

   table.set_entry(r, c, junk);
   table.set_entry(c, r, junk);

}

   //
   //  fcst-fcst edges
   //

for (j=0; j<N_fcst; ++j)  {

   for (k=0; k<N_fcst; ++k)  {

      if ( has_ff_edge(j, k) )  {

         r = fcst_start + j;
         c = fcst_start + k;

         table.set_entry(r, c, 'x');

      }

   }

}

   //
   //  obs-obs edges
   //

for (j=0; j<N_obs; ++j)  {

   for (k=0; k<N_obs; ++k)  {

      if ( has_oo_edge(j, k) )  {

         r = obs_start + j;
         c = obs_start + k;

         table.set_entry(r, c, 'x');

      }

   }

}


   //
   //  fcst-obs edges
   //

for (j=0; j<N_fcst; ++j)  {

   for (k=0; k<N_obs; ++k)  {

      if ( has_fo_edge(j, k) )  {

         r = fcst_start + j;
         c =  obs_start + k;

         table.set_entry(r, c, 'x');
         table.set_entry(c, r, 'x');

      }

   }

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Graph::dump_as_table(ostream & out) const

{

AsciiTable table;

do_dump_table(table);

out << table;

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Graph::dump_as_table(const int verbosity) const

{

int j;
ConcatString s;
AsciiTable table;

do_dump_table(table);

for (j=0; j<(table.nrows()); ++j)  {

   s = table.padded_row(j);

   mlog << Debug(verbosity) << s << "\n";

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////




