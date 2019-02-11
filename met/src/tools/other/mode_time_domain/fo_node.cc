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
#include <cmath>

#include "vx_util.h"

#include "fo_node.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class FO_Node
   //


////////////////////////////////////////////////////////////////////////


FO_Node::FO_Node()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


FO_Node::~FO_Node()

{

clear();

}


////////////////////////////////////////////////////////////////////////


FO_Node::FO_Node(const FO_Node & n)

{

init_from_scratch();

assign(n);

}


////////////////////////////////////////////////////////////////////////


FO_Node & FO_Node::operator=(const FO_Node & n)

{

if ( this == &n )  return ( * this );

assign(n);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void FO_Node::init_from_scratch()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void FO_Node::clear()

{

Number    = 0;

IsFcst    = false;

IsVisited = false;

HasEdge   = false;

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Node::assign(const FO_Node & n)

{

clear();


Number    = n.Number;

IsFcst    = n.IsFcst;

IsVisited = n.IsVisited;

HasEdge   = n.HasEdge;


return;

}


////////////////////////////////////////////////////////////////////////


void FO_Node::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Number    = " << Number                    << '\n';
out << prefix << "IsFcst    = " << bool_to_string(IsFcst)    << '\n';
out << prefix << "IsVisited = " << bool_to_string(IsVisited) << '\n';
out << prefix << "HasEdge   = " << bool_to_string(IsVisited) << '\n';


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Node::set_number(int k)

{

Number = k;

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Node::set_visited(bool tf)

{

IsVisited = tf;

return;

}


////////////////////////////////////////////////////////////////////////


void FO_Node::set_has_edge(bool tf)

{

HasEdge = tf;

return;

}


////////////////////////////////////////////////////////////////////////



