// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "threshold.h"
#include "util_constants.h"

#include "vx_config.h"
#include "vx_math.h"
#include "vx_log.h"
#include "is_bad_data.h"


////////////////////////////////////////////////////////////////////////


extern ThreshNode * result;

extern bool test_mode;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ThreshNode
   //


////////////////////////////////////////////////////////////////////////


ThreshNode::ThreshNode() { }


////////////////////////////////////////////////////////////////////////


ThreshNode::~ThreshNode() { }


////////////////////////////////////////////////////////////////////////


void ThreshNode::threshnode_assign(const ThreshNode * n)

{

s = n->s;

abbr_s = n->abbr_s;

}


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

n->threshnode_assign(this);

return ( n );

}


////////////////////////////////////////////////////////////////////////


void Or_Node::multiply_by(const double x)

{

if ( !left_child || !right_child )  {

   mlog << Error << "\nOr_Node::multiply_by(const double) -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

 left_child->multiply_by(x);
right_child->multiply_by(x);

return;

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

n->threshnode_assign(this);

return ( n );

}


////////////////////////////////////////////////////////////////////////


void And_Node::multiply_by(const double x)

{

if ( !left_child || !right_child )  {

   mlog << Error << "\nAnd_Node::multiply_by(const double) -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

 left_child->multiply_by(x);
right_child->multiply_by(x);

return;

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

n->threshnode_assign(this);

return ( n );

}


////////////////////////////////////////////////////////////////////////


void Not_Node::multiply_by(const double x)

{

if ( ! child )  {

   mlog << Error << "\nNot_Node::multiply_by(const double) -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

child->multiply_by(x);

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Simple_Node
   //


////////////////////////////////////////////////////////////////////////


Simple_Node::Simple_Node()

{

T = bad_data_double;

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
const bool is_na = is_bad_data(x);

switch ( op )  {

   case thresh_le:   tf = !is_na && ( eq || (x <= T));  break;
   case thresh_lt:   tf = !is_na && (!eq && (x <  T));  break;

   case thresh_ge:   tf = !is_na && ( eq || (x >= T));  break;
   case thresh_gt:   tf = !is_na && (!eq && (x >  T));  break;

   case thresh_eq:   tf =  eq;  break;
   case thresh_ne:   tf = !eq;  break;

   default:
      mlog << Error << "\nSimple_Node::check(double) const -> "
           << "bad op ... " << op << "\n\n";
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

n->threshnode_assign(this);

return ( n );

}


////////////////////////////////////////////////////////////////////////


void Simple_Node::set_na()

{

op = thresh_na;

T = bad_data_double;

s = na_str;

abbr_s = na_str;

return;

}


////////////////////////////////////////////////////////////////////////


void Simple_Node::multiply_by(const double x)

{

T *= x;

return;

}


////////////////////////////////////////////////////////////////////////
//
// Code for class SingleThresh
//
////////////////////////////////////////////////////////////////////////


SingleThresh::SingleThresh()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


SingleThresh::~SingleThresh()

{

clear();

}


////////////////////////////////////////////////////////////////////////


SingleThresh::SingleThresh(const SingleThresh & c)

{

init_from_scratch();

assign(c);

}


////////////////////////////////////////////////////////////////////////


SingleThresh::SingleThresh(const char * str)

{

init_from_scratch();

set(str);

}


////////////////////////////////////////////////////////////////////////


SingleThresh & SingleThresh::operator=(const SingleThresh & c)

{

if ( this == &c ) return ( * this );

assign(c);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


bool SingleThresh::operator==(const SingleThresh &st) const

{

   //  return true when both null and false when only one is null

if ( !node && !(st.node) )  return ( true );

if ( !node || !(st.node) )  return ( false );

   //  for complex thresholds, check the string representation

if (    node->type() == thresh_complex ||
     st.node->type() == thresh_complex )  {

   return ( get_str() == st.get_str() );

}

   //  for simple thresholds, check the type and value

else  {

   return ( node->type() == st.node->type() &&
            is_eq(node->value(), st.node->value()) );
}

}


////////////////////////////////////////////////////////////////////////


void SingleThresh::init_from_scratch()

{

node = 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SingleThresh::clear()

{

if ( node )  { delete node;  node = 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void SingleThresh::assign(const SingleThresh & c)

{

clear();

if ( !(c.node) )  return;

node = c.node->copy();

return;

}


////////////////////////////////////////////////////////////////////////


void SingleThresh::set(double t, ThreshType ind)

{

clear();

Simple_Node * a = new Simple_Node;

a->T      = t;
a->op     = ind;
a->s      << thresh_type_str[ind] << t;
a->abbr_s << thresh_abbr_str[ind] << t;

node = a;

a = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleThresh::set(const ThreshNode * n)

{

clear();

node = n->copy();

return;

}


////////////////////////////////////////////////////////////////////////


void SingleThresh::set_na()

{

clear();

Simple_Node * a = new Simple_Node;

a->set_na();

node = a;

a = 0;

return;

}


////////////////////////////////////////////////////////////////////////
//
// Parse out the threshold information from the form:
// lt, le, eq, ne, gt, ge
// ... OR ...
// <, <=, =, !=, >, >=
//
////////////////////////////////////////////////////////////////////////


void SingleThresh::set(const char *str)

{

   //
   // Check for bad data
   //

if(strcmp(str, na_str) == 0 || strcmp(str, bad_data_str) == 0) {

   set_na();

   return;

}

   //
   //  for real
   //

MetConfig config;
bool status = false;

test_mode = true;

   //
   //  strip off trailing SetLogic symbols
   //

ConcatString cs = str;
cs.chomp(setlogic_symbol_union);
cs.chomp(setlogic_symbol_intersection);
cs.chomp(setlogic_symbol_symdiff);

status = config.read_string(cs);

if ( ! status )  {

   mlog << Error << "\nSingleThresh::set(const char *) -> "
        << "failed to parse string \"" << cs << "\"\n\n";

   exit ( 1 );

}

test_mode = false;

if ( ! result )  {

   mlog << Error << "\nSingleThresh::set(const char *) -> "
        << "no result from parsing string \"" << cs << "\"\n\n";

   exit ( 1 );

}

set(result);

   //
   //  done
   //

delete result;  result = 0;

return;

}


////////////////////////////////////////////////////////////////////////
//
// Construct a string to represent the threshold type and value
//
////////////////////////////////////////////////////////////////////////


ConcatString SingleThresh::get_str(int precision) const

{

ConcatString t;

if ( node )  t = node->s;
else         t = na_str;

return(t);

}


////////////////////////////////////////////////////////////////////////
//
// Construct a string to represent the threshold type and value
//
////////////////////////////////////////////////////////////////////////


ConcatString SingleThresh::get_abbr_str(int precision) const

{

ConcatString t;

if ( node )  t = node->abbr_s;
else         t = na_str;

return(t);

}


////////////////////////////////////////////////////////////////////////


void SingleThresh::dump(ostream & out, int depth) const

{

Indent prefix(depth);
ConcatString s = get_str();

out << prefix << "type  = " << get_type()  << "   (" << s.contents() << ")\n";
out << prefix << "value = " << get_value() << '\n';


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void SingleThresh::multiply_by(const double x)

{

if ( ! node )  {

   mlog << Error << "\nSingleThresh::multiply_by(const double) -> "
        << "empty threshold!\n\n";

   exit ( 1 );

}

node->multiply_by(x);

return;

}


////////////////////////////////////////////////////////////////////////
//
// Begin miscellaneous functions
//
////////////////////////////////////////////////////////////////////////


bool check_threshold(double v, double t, int t_ind)

{

SingleThresh st;

st.set(t, (ThreshType) t_ind);

return(st.check(v));

}


////////////////////////////////////////////////////////////////////////
//
// End miscellaneous functions
//
////////////////////////////////////////////////////////////////////////
