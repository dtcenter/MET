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


bool is_inclusive(ThreshType t)

{

return ( t == thresh_le || t == thresh_ge || t == thresh_eq );

}


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


bool Or_Node::need_perc() const

{

if ( !left_child || !right_child )  {

   mlog << Error << "\nOr_Node::need_perc() -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

return ( left_child->need_perc() || right_child->need_perc() );

}


////////////////////////////////////////////////////////////////////////


void Or_Node::set_perc(const NumArray *fptr, const NumArray *optr, const NumArray *cptr)

{

set_perc(fptr, optr, cptr, 0, 0);

return;

}


////////////////////////////////////////////////////////////////////////


void Or_Node::set_perc(const NumArray *fptr, const NumArray *optr, const NumArray *cptr,
                       const SingleThresh *fthr, const SingleThresh *othr)

{

if ( !left_child || !right_child )  {

   mlog << Error << "\nOr_Node::set_perc() -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

 left_child->set_perc(fptr, optr, cptr, fthr, othr);
right_child->set_perc(fptr, optr, cptr, fthr, othr);

return;

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


bool And_Node::need_perc() const

{

if ( !left_child || !right_child )  {

   mlog << Error << "\nAnd_Node::need_perc() -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

return ( left_child->need_perc() || right_child->need_perc() );

}


////////////////////////////////////////////////////////////////////////


void And_Node::set_perc(const NumArray *fptr, const NumArray *optr, const NumArray *cptr)

{

set_perc(fptr, optr, cptr, 0, 0);

return;

}


////////////////////////////////////////////////////////////////////////


void And_Node::set_perc(const NumArray *fptr, const NumArray *optr, const NumArray *cptr,
                        const SingleThresh *fthr, const SingleThresh *othr)

{

if ( !left_child || !right_child )  {

   mlog << Error << "\nAnd_Node::set_perc() -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

 left_child->set_perc(fptr, optr, cptr, fthr, othr);
right_child->set_perc(fptr, optr, cptr, fthr, othr);

return;

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


bool Not_Node::need_perc() const

{

if ( !child )  {

   mlog << Error << "\nNot_Node::need_perc() -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

return ( child->need_perc() );

}


////////////////////////////////////////////////////////////////////////


void Not_Node::set_perc(const NumArray *fptr, const NumArray *optr, const NumArray *cptr)

{

set_perc(fptr, optr, cptr, 0, 0);

return;

}


////////////////////////////////////////////////////////////////////////


void Not_Node::set_perc(const NumArray *fptr, const NumArray *optr, const NumArray *cptr,
                        const SingleThresh *fthr, const SingleThresh *othr)

{

if ( !child )  {

   mlog << Error << "\nNot_Node::set_perc() -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

child->set_perc(fptr, optr, cptr, fthr, othr);

return;

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

op = no_thresh_type;

T = bad_data_double;

Ptype = no_perc_thresh_type;

PT = bad_data_double;

}


////////////////////////////////////////////////////////////////////////


Simple_Node::~Simple_Node()

{

}


////////////////////////////////////////////////////////////////////////


bool Simple_Node::check(double x) const

{

if ( op == thresh_na )  return ( true );

   //
   //  check that percentile thresholds have been resolved
   //

if ( Ptype != no_perc_thresh_type && is_bad_data(T) ) {

   mlog << Error << "\nSimple_Node::check(double) const -> "
        << "percentile threshold used before it was set!\n\n";

   exit ( 1 );

}

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

n->PT = PT;

n->Ptype = Ptype;

n->threshnode_assign(this);

return ( n );

}


////////////////////////////////////////////////////////////////////////


void Simple_Node::set_na()

{

op = thresh_na;

T = bad_data_double;

Ptype = no_perc_thresh_type;

PT = bad_data_double;

s = na_str;

abbr_s = na_str;

return;

}

////////////////////////////////////////////////////////////////////////


void Simple_Node::set_perc(const NumArray *fptr, const NumArray *optr, const NumArray *cptr)

{

set_perc(fptr, optr, cptr, 0, 0);

return;

}


////////////////////////////////////////////////////////////////////////


void Simple_Node::set_perc(const NumArray *fptr, const NumArray *optr, const NumArray *cptr,
                           const SingleThresh *fthr, const SingleThresh *othr)

{

int i, count;
double ptile, diff;
NumArray data;
const NumArray * ptr = 0;

   //
   //  handle sample percentile types
   //

     if ( Ptype == perc_thresh_sample_fcst  )  ptr = fptr;
else if ( Ptype == perc_thresh_sample_obs   )  ptr = optr;
else if ( Ptype == perc_thresh_sample_climo )  ptr = cptr;

   //
   //  handle bias-correction type
   //

else if ( Ptype == perc_thresh_freq_bias )  {

   if ( !fptr || !optr || !fthr || !othr )  {

      mlog << Error << "\nSimple_Node::set_perc() -> "
           << "not enough information provided to define the "
           << perc_thresh_info[Ptype].long_name
           << " threshold.\n\n";

      exit ( 1 );

   }

      //
      //  bias-correct the observation
      //

   if ( othr->get_ptype() == perc_thresh_freq_bias &&
        fthr->get_ptype() == no_perc_thresh_type   &&
        fthr->get_type()  != thresh_complex )  {

      ptr = optr;
      op  = fthr->get_type();
      PT  = fptr->compute_percentile(fthr->get_value(),
                                     is_inclusive(fthr->get_type()));

      mlog << Debug(3)
           << "The forecast threshold \"" << fthr->get_str()
           << "\" includes " << PT * 100.0 << "% of the data.\n";

   }

      //
      //  bias-correct the forecast
      //

   else if ( fthr->get_ptype() == perc_thresh_freq_bias &&
             othr->get_ptype() == no_perc_thresh_type   &&
             othr->get_type()  != thresh_complex )  {

      ptr = fptr;
      op  = othr->get_type();
      PT  = optr->compute_percentile(othr->get_value(),
                                     is_inclusive(othr->get_type()));

      mlog << Debug(3)
           << "The observation threshold \"" << othr->get_str()
           << "\" includes " << PT * 100.0 << "% of the data.\n";

   }

   else {

      mlog << Error << "\nSimple_Node::set_perc() -> "
           << "unsupported options for computing the "
           << perc_thresh_info[Ptype].long_name
           << " threshold.\n\n";

      exit ( 1 );

   }

   //
   //  multiple percentile by 100
   //

   if ( is_bad_data(PT) )  {

      mlog << Error << "\nSimple_Node::set_perc() -> "
           << "unable to compute the percentile for the "
           << perc_thresh_info[Ptype].long_name
           << " threshold.\n\n";

      exit ( 1 );
   }

   PT *= 100.0;

}
   //
   //  nothing to do
   //

else  {

   return;

}

if ( !ptr )  {

   mlog << Error << "\nSimple_Node::set_perc() -> "
        << perc_thresh_info[Ptype].long_name
        << " threshold requested but no data provided.\n\n";

   exit ( 1 );

}

   //
   //  remove bad data, if necessary
   //

if ( ptr->has(bad_data_double) ) {

   data.extend(ptr->n());

   for ( i=0; i<ptr->n(); i++ )  {

      if ( !is_bad_data(ptr->vals()[i]) )  data.add(ptr->vals()[i]);

   }
}
else {

   data = *ptr;

}

   //
   //  compute the percentile threshold
   //

if ( data.n() == 0 )  {

   mlog << Error << "\nSimple_Node::set_perc() -> "
        << "can't compute " << perc_thresh_info[Ptype].long_name
        << " threshold because no valid data was provided.\n\n";

      exit ( 1 );

}
else  {

   //
   //  strip previous definition from strings
   //

   s.strip_paren();
   abbr_s.strip_paren();

   //
   //  compute the percentile and update the strings
   //

   T = data.percentile_array((double) PT / 100.0);

   if ( !is_bad_data(T) )  {

      ConcatString cs;

      cs << T;

      fix_float(cs);

           s << "(" << cs << ")";
      abbr_s << "(" << cs << ")";

   }

   //
   //  compute the actual percentile and check tolerance
   //

   if ( op == thresh_le || op == thresh_ge || op == thresh_eq )  {

      for ( i=count=0; i<data.n(); i++ )  if ( data[i] <= T ) count++;

   }
   else  {

      for ( i=count=0; i<data.n(); i++ )  if ( data[i] <  T ) count++;

   }

   ptile = (double) count / data.n();
   diff  = abs(PT / 100.0 - ptile);

   if ( !is_eq(PT / 100.0, ptile, perc_thresh_default_tol) )  {

      mlog << Warning << "\nSimple_Node::set_perc() -> "
           << "the requested percentile (" << PT
           << "%) for threshold \"" << s
           << "\" differs from the actual percentile ("
           << ptile * 100.0 << ") by " << diff * 100.0 << "%.\n"
           << "This is common for small samples or data that contains "
           << "ties.\n\n";

   }
   else  {

      mlog << Debug(3)
           << "The requested percentile (" << PT
           << "%) for threshold threshold \"" << s
           << "\" includes " << ptile * 100.0 << "% of the data.\n";

   }

}

return;

}


////////////////////////////////////////////////////////////////////////


bool Simple_Node::need_perc() const

{

return ( Ptype == perc_thresh_sample_fcst  ||
         Ptype == perc_thresh_sample_obs   ||
         Ptype == perc_thresh_sample_climo ||
         Ptype == perc_thresh_freq_bias );

}


////////////////////////////////////////////////////////////////////////


void Simple_Node::multiply_by(const double x)

{

if ( !is_bad_data(T) )  T *= x;

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

   if ( ((Simple_Node *) node)->need_perc() ) {
      return ( node->type() == st.node->type() &&
               node->ptype() == st.node->ptype() &&
               is_eq(node->pvalue(), st.node->pvalue()) );
   }
   else {
      return ( node->type() == st.node->type() &&
               is_eq(node->value(), st.node->value()) );
   }
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


void SingleThresh::set(double pt, ThreshType ind, int perc_index, double t)

{

clear();

if ( (perc_index < 0) || (perc_index >= n_perc_thresh_infos) )  {

   mlog << Error
        << "\nSingleThresh::set(double pt, ThreshType ind, int perc_index, double t) -> "
        << "bad perc_index ... " << perc_index << "\n\n";

   exit ( 1 );

}

Simple_Node * a = new Simple_Node;

ConcatString cs;
cs << perc_thresh_info[perc_index].short_name << pt;
if( !is_bad_data(t) ) cs << "(" << t << ")";

a->T      = t;
a->op     = ind;
a->Ptype  = perc_thresh_info[perc_index].type;
a->PT     = pt;
a->s      << thresh_type_str[ind] << cs;
a->abbr_s << thresh_abbr_str[ind] << cs;

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

   //
   //  strip off trailing SetLogic symbols
   //

ConcatString cs = string(str);
cs.chomp(setlogic_symbol_union);
cs.chomp(setlogic_symbol_intersection);
cs.chomp(setlogic_symbol_symdiff);

ConcatString cs2;
cs2 << "threshold = " << cs << ";";

status = config.read_string(cs2.c_str());

if ( ! status )  {

   mlog << Error << "\nSingleThresh::set(const char *) -> "
        << "failed to parse string \"" << cs << "\"\n\n";

   exit ( 1 );

}

assign(config.lookup_thresh("threshold"));

return;

}


////////////////////////////////////////////////////////////////////////


bool SingleThresh::need_perc() const

{

if ( node )  {

   return ( node->need_perc() );

}

return ( false );

}


////////////////////////////////////////////////////////////////////////


void SingleThresh::set_perc(const NumArray *fptr, const NumArray *optr, const NumArray *cptr)

{

set_perc(fptr, optr, cptr, 0, 0);

return;

}


////////////////////////////////////////////////////////////////////////


void SingleThresh::set_perc(const NumArray *fptr, const NumArray *optr, const NumArray *cptr,
                            const SingleThresh *fthr, const SingleThresh *othr)
{

if ( node )  {

   node->set_perc(fptr, optr, cptr, fthr, othr);

}

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

out << prefix << "type   = " << get_type()   << "\n";
out << prefix << "value  = " << get_value()  << "\n";
out << prefix << "ptype  = " << get_ptype()  << "\n";
out << prefix << "pvalue = " << get_pvalue() << "\n";


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
