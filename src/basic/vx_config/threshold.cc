// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "threshold.h"
#include "util_constants.h"

#include "vx_config.h"
#include "vx_gsl_prob.h"
#include "vx_math.h"
#include "vx_log.h"
#include "is_bad_data.h"


using namespace std;


////////////////////////////////////////////////////////////////////////


extern ThreshNode * result;

extern bool test_mode;

extern const std::string scp_perc_thresh_type_str("SCP");

extern const std::string cdp_perc_thresh_type_str("CDP");

static bool print_climo_perc_thresh_log_message = true; 


////////////////////////////////////////////////////////////////////////


bool is_inclusive(ThreshType t)

{

return ( t == thresh_le || t == thresh_ge || t == thresh_eq );

}


////////////////////////////////////////////////////////////////////////


bool is_climo_dist_type(PercThreshType t)

{

return ( t == perc_thresh_fcst_climo_dist ||
         t == perc_thresh_obs_climo_dist );

}


////////////////////////////////////////////////////////////////////////


bool parse_perc_thresh(const char *str, PC_info *info)

{

bool match = false;

if ( perc_thresh_info_map.empty() ) return false;

ConcatString search_cs(str);

for (auto const& x : perc_thresh_info_map) {

   if ( search_cs.startswith(x.second.short_name.c_str()) &&
        is_number(str + x.second.short_name.size()) ) {

      if ( info ) {

         info->ptype = x.first;

         info->value = atof(str + x.second.short_name.size());

      }

      match = true;

      break;

   }

}

   //
   // MET #2924 For backward compatibility support SCP and CDP types
   //    

if ( !match &&
    (search_cs.startswith(scp_perc_thresh_type_str.c_str()) ||
     search_cs.startswith(cdp_perc_thresh_type_str.c_str())) ) {

   if ( print_climo_perc_thresh_log_message ) {

      mlog << Debug(2) << R"(Please replace the deprecated "SCP" and "CDP" )" 
           << R"(threshold types with "SOCP" and "OCDP", respectively, in the ")"
           << str << R"(" threshold string.\n)";

      print_climo_perc_thresh_log_message = false;

   }

   ConcatString cs;

   if ( search_cs.startswith(scp_perc_thresh_type_str.c_str()) ) {
      cs << perc_thresh_info_map.at(perc_thresh_sample_obs_climo).short_name;
      cs << str + scp_perc_thresh_type_str.size();
   }
   else {
      cs << perc_thresh_info_map.at(perc_thresh_obs_climo_dist).short_name;
      cs << str + cdp_perc_thresh_type_str.size();
   }

   return parse_perc_thresh(cs.c_str(), info);

}

return match;

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

left_child = right_child = nullptr;

}


////////////////////////////////////////////////////////////////////////


Or_Node::~Or_Node()

{

if (  left_child )  { delete  left_child;   left_child = nullptr; }
if ( right_child )  { delete right_child;  right_child = nullptr; }

}


////////////////////////////////////////////////////////////////////////


bool Or_Node::check(double x, const ClimoPntInfo *cpi) const

{

const bool tf_left = left_child->check(x, cpi);

if ( tf_left )  return true;

const bool tf_right = right_child->check(x, cpi);

return tf_right;

}


////////////////////////////////////////////////////////////////////////


ThreshNode * Or_Node::copy() const

{

Or_Node * n = new Or_Node;

if (  left_child )  n->left_child  = left_child->copy();
if ( right_child )  n->right_child = right_child->copy();

n->threshnode_assign(this);

return n;

}


////////////////////////////////////////////////////////////////////////


double Or_Node::obs_climo_prob() const

{

if ( !left_child || !right_child )  {

   mlog << Error << "\nOr_Node::obs_climo_prob() -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

double prob       = bad_data_double;
double prob_left  = left_child->obs_climo_prob();
double prob_right = right_child->obs_climo_prob();

if ( !is_bad_data(prob_left) && !is_bad_data(prob_right) )  {

   prob = min(prob_left + prob_right, 1.0);

}

return prob;

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


void Or_Node::set_perc(const NumArray *fptr, const NumArray *optr,
                       const NumArray *fcptr, const NumArray *ocptr,
                       const SingleThresh *fthr, const SingleThresh *othr)

{

if ( !left_child || !right_child )  {

   mlog << Error << "\nOr_Node::set_perc() -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

 left_child->set_perc(fptr, optr, fcptr, ocptr, fthr, othr);
right_child->set_perc(fptr, optr, fcptr, ocptr, fthr, othr);

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


void Or_Node::get_simple_nodes(vector<Simple_Node> &v) const

{

if ( !left_child || !right_child )  {

   mlog << Error << "\nOr_Node::get_simple_nodes() -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

 left_child->get_simple_nodes(v);
right_child->get_simple_nodes(v);

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class And_Node
   //


////////////////////////////////////////////////////////////////////////


And_Node::And_Node()

{

left_child = right_child = nullptr;

}


////////////////////////////////////////////////////////////////////////


And_Node::~And_Node()

{

if (  left_child )  { delete  left_child;   left_child = nullptr; }
if ( right_child )  { delete right_child;  right_child = nullptr; }

}


////////////////////////////////////////////////////////////////////////


bool And_Node::check(double x, const ClimoPntInfo *cpi) const

{

const bool tf_left = left_child->check(x, cpi);

if ( ! tf_left )  return false;

const bool tf_right = right_child->check(x, cpi);

return ( tf_left && tf_right );

}


////////////////////////////////////////////////////////////////////////


ThreshNode * And_Node::copy() const

{

And_Node * n = new And_Node;

if (  left_child )  n->left_child  = left_child->copy();
if ( right_child )  n->right_child = right_child->copy();

n->threshnode_assign(this);

return n;

}


////////////////////////////////////////////////////////////////////////


double And_Node::obs_climo_prob() const

{

if ( !left_child || !right_child )  {

   mlog << Error << "\nAnd_Node::obs_climo_prob() -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

double prob       = bad_data_double;
double prob_left  = left_child->obs_climo_prob();
double prob_right = right_child->obs_climo_prob();

   //
   // For opposing inequalities, compute the difference in percentiles
   //

if ( !is_bad_data(prob_left) && !is_bad_data(prob_right) ) {

   //
   // Support complex threshold types >a&&<b and <a&&>b
   //

   if ( (  left_child->type() == thresh_gt ||  left_child->type() == thresh_ge ) &&
        ( right_child->type() == thresh_lt || right_child->type() == thresh_le ) )  {

      prob = max( 0.0, prob_right - ( 1.0 - prob_left ) );

   }
   else if ( (  left_child->type() == thresh_lt ||  left_child->type() == thresh_le ) &&
             ( right_child->type() == thresh_gt || right_child->type() == thresh_ge ) )  {

      prob = max( 0.0, prob_left - ( 1.0 - prob_right ) );

   }
}

return prob;

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


void And_Node::set_perc(const NumArray *fptr, const NumArray *optr,
                        const NumArray *fcptr, const NumArray *ocptr,
                        const SingleThresh *fthr, const SingleThresh *othr)

{

if ( !left_child || !right_child )  {

   mlog << Error << "\nAnd_Node::set_perc() -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

 left_child->set_perc(fptr, optr, fcptr, ocptr, fthr, othr);
right_child->set_perc(fptr, optr, fcptr, ocptr, fthr, othr);

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


void And_Node::get_simple_nodes(vector<Simple_Node> &v) const

{

if ( !left_child || !right_child )  {

   mlog << Error << "\nAnd_Node::get_simple_nodes() -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

 left_child->get_simple_nodes(v);
right_child->get_simple_nodes(v);

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Not_Node
   //


////////////////////////////////////////////////////////////////////////


Not_Node::Not_Node()

{

child = nullptr;

}


////////////////////////////////////////////////////////////////////////


Not_Node::~Not_Node()

{

if ( child )  { delete child;  child = nullptr; }

}


////////////////////////////////////////////////////////////////////////


bool Not_Node::check(double x, const ClimoPntInfo *cpi) const

{

const bool tf = child->check(x, cpi);

return !tf;

}


////////////////////////////////////////////////////////////////////////


ThreshNode * Not_Node::copy() const

{

Not_Node * n = new Not_Node;

if ( child )  n->child  = child->copy();

n->threshnode_assign(this);

return n;

}


////////////////////////////////////////////////////////////////////////


double Not_Node::obs_climo_prob() const

{

double prob       = bad_data_double;
double prob_child = child->obs_climo_prob();

if ( !is_bad_data(prob_child) )  prob = 1.0 - prob_child;

return prob;

}


////////////////////////////////////////////////////////////////////////


bool Not_Node::need_perc() const

{

if ( !child )  {

   mlog << Error << "\nNot_Node::need_perc() -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

return child->need_perc();

}


////////////////////////////////////////////////////////////////////////


void Not_Node::set_perc(const NumArray *fptr, const NumArray *optr,
                        const NumArray *fcptr, const NumArray *ocptr,
                        const SingleThresh *fthr, const SingleThresh *othr)


{

if ( !child )  {

   mlog << Error << "\nNot_Node::set_perc() -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

child->set_perc(fptr, optr, fcptr, ocptr, fthr, othr);

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


void Not_Node::get_simple_nodes(vector<Simple_Node> &v) const

{

if ( !child )  {

   mlog << Error << "\nNot_Node::get_simple_nodes() -> "
        << "node not populated!\n\n";

   exit ( 1 );

}

child->get_simple_nodes(v);

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


bool Simple_Node::check(double x, const ClimoPntInfo *cpi) const

{

if ( op == thresh_na )  return true;

double tval;

   //
   //  check climo distribution percentile thresholds
   //

if ( is_climo_dist_type(Ptype) ) {

   //
   //  check the pointer
   //

   if(!cpi) {

      mlog << Error << "\nSimple_Node::check(double, const ClimoPntInfo *) const -> "
           << "climatological distribution percentile threshold type requested "
           << "with no ClimoPntInfo provided!\n\n";

      exit ( 1 );

   }

   double cmn = (Ptype == perc_thresh_fcst_climo_dist ? cpi->fcmn : cpi->ocmn);
   double csd = (Ptype == perc_thresh_fcst_climo_dist ? cpi->fcsd : cpi->ocsd);

   //
   //  check the climo data
   //

   if(is_bad_data(cmn) || is_bad_data(csd)) {

      mlog << Error << "\nSimple_Node::check(double, const ClimoPntInfo *) const -> "
           << "climatological distribution percentile threshold \"" << s
           << "\" requested with invalid mean (" << cmn
           << ") or standard deviation (" << csd << ").\n\n";

      exit ( 1 );

   }

   tval = normal_cdf_inv(PT/100.0, cmn, csd);

}
else {

   tval = T;

}

   //
   //  check that percentile thresholds have been resolved
   //

if ( Ptype != no_perc_thresh_type && is_bad_data(tval) ) {

   mlog << Error << "\nSimple_Node::check(double, const ClimoPntInfo *) const -> "
        << "percentile threshold \"" << s
        << "\" used before it was set.\n\n";

   exit ( 1 );

}

bool tf = false;
const bool eq = is_eq(x, tval);
const bool is_na = is_bad_data(x);

switch ( op )  {

   case thresh_le:   tf = !is_na && ( eq || (x <= tval));  break;
   case thresh_lt:   tf = !is_na && (!eq && (x <  tval));  break;

   case thresh_ge:   tf = !is_na && ( eq || (x >= tval));  break;
   case thresh_gt:   tf = !is_na && (!eq && (x >  tval));  break;

   case thresh_eq:   tf =  eq;  break;
   case thresh_ne:   tf = !eq;  break;

   default:
      mlog << Error << "\nSimple_Node::check(double, const ClimoPntInfo *) const -> "
           << "bad op ... " << op << "\n\n";
      exit ( 1 );

}   //  switch

return tf;

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

return n;

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


void Simple_Node::set_perc(const NumArray *fptr, const NumArray *optr,
                           const NumArray *fcptr, const NumArray *ocptr,
                           const SingleThresh *fthr, const SingleThresh *othr)

{

int i;
double ptile, diff;
NumArray data;
const NumArray * ptr = nullptr;
bool fbias_fcst = false;

   //
   //  handle sample percentile types
   //

     if ( Ptype == perc_thresh_sample_fcst       )  ptr = fptr;
else if ( Ptype == perc_thresh_sample_obs        )  ptr = optr;
else if ( Ptype == perc_thresh_sample_fcst_climo )  ptr = fcptr;
else if ( Ptype == perc_thresh_sample_obs_climo  )  ptr = ocptr;

   //
   //  handle bias-correction type
   //

else if ( Ptype == perc_thresh_freq_bias )  {

   if ( !fptr || !optr || !fthr || !othr )  {

      mlog << Error << "\nSimple_Node::set_perc() -> "
           << "not enough information provided to define the "
           << perc_thresh_info_map.at(Ptype).long_name
           << " threshold \"" << s << "\".\n\n";

      exit ( 1 );

   }

      //
      //  bias-correct the observation
      //

   if ( othr->get_ptype() == perc_thresh_freq_bias &&
        fthr->get_ptype() == no_perc_thresh_type   &&
        fthr->get_type()  != thresh_complex )  {

      fbias_fcst = false;

      ptr = optr;
      op  = fthr->get_type();
      PT  = fptr->compute_percentile(fthr->get_value(),
                                     is_inclusive(fthr->get_type()));

      mlog << Debug(3)
           << "The forecast threshold value \"" << fthr->get_str()
           << "\" represents the " << PT * 100.0 << "-th percentile.\n";

   }

      //
      //  bias-correct the forecast
      //

   else if ( fthr->get_ptype() == perc_thresh_freq_bias &&
             othr->get_ptype() == no_perc_thresh_type   &&
             othr->get_type()  != thresh_complex )  {

      fbias_fcst = true;

      ptr = fptr;
      op  = othr->get_type();
      PT  = optr->compute_percentile(othr->get_value(),
                                     is_inclusive(othr->get_type()));

      mlog << Debug(3)
           << "The observation threshold value \"" << othr->get_str()
           << "\" represents the " << PT * 100.0 << "-th percentile.\n";

   }

   else {

      mlog << Error << "\nSimple_Node::set_perc() -> "
           << "unsupported options for computing the "
           << perc_thresh_info_map.at(Ptype).long_name
           << " threshold \"" << s << "\".\n\n";

      exit ( 1 );

   }

   //
   //  multiple percentile by 100
   //

   if ( is_bad_data(PT) )  {

      mlog << Error << "\nSimple_Node::set_perc() -> "
           << "unable to compute the percentile for the "
           << perc_thresh_info_map.at(Ptype).long_name
           << " threshold \"" << s << "\".\n\n";

      exit ( 1 );
   }

   PT *= 100.0;

} // end else if PT == perc_thresh_freq_bias

   //
   //  nothing to do
   //

else  {

   return;

}

if ( !ptr )  {

   mlog << Error << "\nSimple_Node::set_perc() -> "
        << perc_thresh_info_map.at(Ptype).long_name
        << " threshold \"" << s
        << "\" requested but no data provided.\n\n";

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
        << "can't compute " << perc_thresh_info_map.at(Ptype).long_name
        << " threshold \"" << s
        << "\" because no valid data was provided.\n\n";

   exit ( 1 );

}
else  {

   //
   //  strip previous definition from strings
   //

   s.strip_paren();
   abbr_s.strip_paren();

   //
   //  parse the frequency bias value from the threshold string
   //

   if ( Ptype == perc_thresh_freq_bias )  {

      ConcatString fs = s;

      fs.replace("==FBIAS", " ", false);

      double fbias_val = atof(fs.c_str());

      //
      //  range check requested bias value
      //

      if ( fbias_val <= 0.0 )  {

         mlog << Error << "\nSimple_Node::set_perc() -> "
              << "the requested frequency bias value (" << fbias_val
              << ") must be > 0 in threshold \"" << s << "\".\n\n";

      }

      //
      //  adjust PT by the requested frequency bias amount
      //

      double PT_new = 0.;

      if ( fbias_fcst )  {
               if ( op == thresh_le || op == thresh_lt )  PT_new = PT * fbias_val;
          else if ( op == thresh_ge || op == thresh_gt )  PT_new = PT / fbias_val;     
      }
      else  {
               if ( op == thresh_le || op == thresh_lt )  PT_new = PT / fbias_val;
          else if ( op == thresh_ge || op == thresh_gt )  PT_new = PT * fbias_val;     
      }

      if ( PT_new > 100.0 )  {
         mlog << Warning << "\nSimple_Node::set_perc() -> "
              << "For " << (fbias_fcst ? "forecast" : "observation" )
              << " threshold \"" << s << "\" the required percentile of "
              << PT_new << " exceeds the maximum possible value. "
              << "Resetting to 100.\n\n";
         
         PT_new = 100.0;
      }

      mlog << Debug(3)
           << "For " << (fbias_fcst ? "forecast" : "observation" )
           << " threshold \"" << s << "\" with type \"" << thresh_type_str[op]
           << "\" update the requested percentile from " << PT << " to "
           << PT_new << ".\n";

      PT = PT_new;
   }

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

   ptile = data.compute_percentile(T, is_inclusive(op));
   diff  = abs(PT / 100.0 - ptile);

   if ( !is_eq(PT / 100.0, ptile, perc_thresh_default_tol) )  {

      mlog << Warning << "\nSimple_Node::set_perc() -> "
           << "the requested percentile (" << PT
           << ") for threshold \"" << s
           << "\" differs from the actual percentile ("
           << ptile * 100.0 << ") by " << diff * 100.0 << ".\n"
           << "This is common for small samples or data that contains "
           << "ties.\n\n";

   }
   else  {

      mlog << Debug(3)
           << "The requested percentile (" << PT
           << ") for threshold \"" << s << "\" includes "
           << ptile * 100.0 << "% of the data.\n";

   }

}

return;

}


////////////////////////////////////////////////////////////////////////


double Simple_Node::obs_climo_prob() const

{
   
double prob = bad_data_double;

if ( Ptype == perc_thresh_obs_climo_dist ) {

   // Observation climo probability varies based on the threshold type
   switch ( op )  {

      case thresh_lt:
      case thresh_le:

         prob = PT/100.0;
         break;

      case thresh_eq:

         prob = 0.0;
         break;

      case thresh_ne:

         prob = 1.0;
         break;

      case thresh_gt:
      case thresh_ge:

         prob = 1.0 - PT/100.0;
         break;

      default:

         mlog << Error << "\nSimple_Node::obs_climo_prob() -> "
              << "cannot convert observation climatological distribution "
              << "percentile threshold to a probability!\n\n";

         exit ( 1 );

   }  // switch
}

return prob;

}


////////////////////////////////////////////////////////////////////////


bool Simple_Node::need_perc() const

{

return ( Ptype == perc_thresh_sample_fcst       ||
         Ptype == perc_thresh_sample_obs        ||
         Ptype == perc_thresh_sample_fcst_climo ||
         Ptype == perc_thresh_sample_obs_climo  ||
         Ptype == perc_thresh_freq_bias );

}


////////////////////////////////////////////////////////////////////////


void Simple_Node::multiply_by(const double x)

{

if ( !is_bad_data(T) )  T *= x;

return;

}


////////////////////////////////////////////////////////////////////////


void Simple_Node::get_simple_nodes(vector<Simple_Node> &v) const

{

v.push_back(*this);

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

if ( this == &c ) return *this;

assign(c);

return *this;

}


////////////////////////////////////////////////////////////////////////


bool SingleThresh::operator==(const SingleThresh &st) const

{

   //  return true when both null and false when only one is null

if ( !node && !(st.node) )  return true;

if ( !node || !(st.node) )  return false;

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

node = nullptr;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SingleThresh::clear()

{

if ( node )  { delete node;  node = nullptr; }

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

a = nullptr;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleThresh::set(double pt, ThreshType ind, PercThreshType ptype, double t)

{

clear();

if ( ptype == no_perc_thresh_type )  {

   mlog << Error << "\nSingleThresh::set(double, ThreshType, PercThreshType, double) -> "
        << "bad percentile threshold type\n\n";

   exit ( 1 );

}

Simple_Node * a = new Simple_Node;

ConcatString cs;
cs << perc_thresh_info_map.at(ptype).short_name << pt;
if( !is_bad_data(t) ) cs << "(" << t << ")";

a->T      = t;
a->op     = ind;
a->Ptype  = ptype;
a->PT     = pt;
a->s      << thresh_type_str[ind] << cs;
a->abbr_s << thresh_abbr_str[ind] << cs;

node = a;

a = nullptr;

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

a = nullptr;

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

   return node->need_perc();

}

return false;

}


////////////////////////////////////////////////////////////////////////


void SingleThresh::set_perc(const NumArray *fptr, const NumArray *optr,
                            const NumArray *fcptr, const NumArray *ocptr,
                            const SingleThresh *fthr, const SingleThresh *othr)

{

if ( node )  {

   node->set_perc(fptr, optr, fcptr, ocptr, fthr, othr);

}

return;

}


////////////////////////////////////////////////////////////////////////


void SingleThresh::get_simple_nodes(vector<Simple_Node> &v) const {

if ( node )  {

    node->get_simple_nodes(v);

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

return t;

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

return t;

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


bool SingleThresh::check(double x, const ClimoPntInfo *cpi) const

{

return ( node ? node->check(x, cpi) : true );


}


////////////////////////////////////////////////////////////////////////
//
// End code for class SingleThresh
//
////////////////////////////////////////////////////////////////////////
