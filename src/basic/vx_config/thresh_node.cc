

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_log.h"
#include "is_bad_data.h"

#include "thresh_node.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ThreshNode
   //


////////////////////////////////////////////////////////////////////////


ThreshNode::ThreshNode() { }


////////////////////////////////////////////////////////////////////////


ThreshNode::~ThreshNode() { }


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Or_Node
   //


////////////////////////////////////////////////////////////////////////


Or_Node::Or_Node()

{

left_child = right_child = 0;

}


////////////////////////////////////////////////////////////////////////


Or_Node::~Or_Node()

{

if (  left_child )  { delete  left_child;   left_child = 0; }
if ( right_child )  { delete right_child;  right_child = 0; }

}


////////////////////////////////////////////////////////////////////////


bool Or_Node::check(double x) const

{

const bool tf_left = left_child->check(x);

if ( tf_left )  return ( true );

const bool tf_right = right_child->check(x);

return ( tf_left || tf_right );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * Or_Node::copy() const

{

Or_Node * n = new Or_Node;

if (  left_child )  n->left_child  = left_child->copy();
if ( right_child )  n->right_child = right_child->copy();

return ( n );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class And_Node
   //


////////////////////////////////////////////////////////////////////////


And_Node::And_Node()

{

left_child = right_child = 0;

}


////////////////////////////////////////////////////////////////////////


And_Node::~And_Node()

{

if (  left_child )  { delete  left_child;   left_child = 0; }
if ( right_child )  { delete right_child;  right_child = 0; }

}


////////////////////////////////////////////////////////////////////////


bool And_Node::check(double x) const

{

const bool tf_left = left_child->check(x);

if ( ! tf_left )  return ( false );

const bool tf_right = right_child->check(x);

return ( tf_left && tf_right );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * And_Node::copy() const

{

And_Node * n = new And_Node;

if (  left_child )  n->left_child  = left_child->copy();
if ( right_child )  n->right_child = right_child->copy();

return ( n );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Not_Node
   //


////////////////////////////////////////////////////////////////////////


Not_Node::Not_Node()

{

child = 0;

}


////////////////////////////////////////////////////////////////////////


Not_Node::~Not_Node()

{

if ( child )  { delete child;  child = 0; }

}


////////////////////////////////////////////////////////////////////////


bool Not_Node::check(double x) const

{

const bool tf = child->check(x);

return ( ! tf );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * Not_Node::copy() const

{

Not_Node * n = new Not_Node;

if ( child )  n->child  = child->copy();

return ( n );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Simple_Node
   //


////////////////////////////////////////////////////////////////////////


Simple_Node::Simple_Node()

{

T = 0.0;

// op = no_thresh_type;

}


////////////////////////////////////////////////////////////////////////


Simple_Node::~Simple_Node()

{

}


////////////////////////////////////////////////////////////////////////


bool Simple_Node::check(double x) const

{

if ( op == thresh_na )  return ( true );

bool tf = false;
const bool eq = is_eq(x, T);

switch ( op )  {

   case thresh_le:   tf = ( eq || (x <= T));  break;
   case thresh_lt:   tf = (!eq && (x <  T));  break;

   case thresh_ge:   tf = ( eq || (x >= T));  break;
   case thresh_gt:   tf = (!eq && (x >  T));  break;

   case thresh_eq:   tf =  eq;  break;
   case thresh_ne:   tf = !eq;  break;

   default:
      mlog << Error << "\n\n  Simple_Node::check(double) const -> bad op ... " << op << "\n\n";
      exit ( 1 );
      break;

}   //  switch



return ( tf );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * Simple_Node::copy() const

{

Simple_Node * n = new Simple_Node;

n->T = T;

n->op = op;

return ( n );

}


////////////////////////////////////////////////////////////////////////


ThreshType Simple_Node::type() const

{

return ( op );

}


////////////////////////////////////////////////////////////////////////








