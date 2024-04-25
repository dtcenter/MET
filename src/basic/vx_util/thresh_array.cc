// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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

#include "thresh_array.h"
#include "vx_math.h"
#include "vx_log.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

static const int thresharray_alloc_inc = 10;

////////////////////////////////////////////////////////////////////////
//
//  Code for class ThreshArray
//
////////////////////////////////////////////////////////////////////////

ThreshArray::ThreshArray() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ThreshArray::~ThreshArray() {

   clear();
}

////////////////////////////////////////////////////////////////////////

ThreshArray::ThreshArray(const ThreshArray & a) {

   init_from_scratch();

   assign(a);
}

////////////////////////////////////////////////////////////////////////

ThreshArray & ThreshArray::operator=(const ThreshArray & a) {

   if(this == &a) return *this;

   assign(a);

   return *this;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::init_from_scratch() {

   t = (SingleThresh *) nullptr;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::clear() {

   if(t) { delete [] t;  t = (SingleThresh *) nullptr; }

   Nelements = Nalloc = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::assign(const ThreshArray & a) {
   int j;

   clear();

   extend(a.Nelements);

   for(j=0; j<(a.Nelements); j++) add(a.t[j]);

   Nelements = a.Nelements;

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::dump(ostream & out, int depth) const {
   int j;

   Indent prefix(depth);
   Indent prefix2(depth + 1);

   out << prefix << "Nelements = " << Nelements << "\n";
   out << prefix << "Nalloc    = " << Nalloc    << "\n";

   for(j=0; j<Nelements; j++) {
      out << prefix2 << "Element # " << j << " = \"" << t[j].get_str() << "\"\n";
   }

   out.flush();

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::extend(int n, bool exact) {
   int j, k;

   if(n <= Nalloc) return;

   if(!exact) {
      k = n/thresharray_alloc_inc;

      if(n%thresharray_alloc_inc) k++;

      n = k*thresharray_alloc_inc;
   }

   SingleThresh *u = (SingleThresh *) nullptr;

   u = new SingleThresh [n];

   if(t) {
      for(j=0; j<Nelements; j++) u[j] = t[j];

      delete [] t; t = (SingleThresh *) nullptr;
   }

   t = u; u = (SingleThresh *) nullptr;

   Nalloc = n;

   return;
}

////////////////////////////////////////////////////////////////////////

bool ThreshArray::operator==(const ThreshArray &ta) const {

   // Check for the same length
   if(Nelements != ta.n()) return false;

   // Check for equality of individual elements
   for(int i=0; i<Nelements; i++) {
      if(!(t[i] == ta[i])) return false;
   }

   return true;
}

////////////////////////////////////////////////////////////////////////

SingleThresh ThreshArray::operator[](int n) const {

   if((n < 0) || (n >= Nelements)) {
      mlog << Error << "\nThreshArray::operator[](int) const -> "
           << "range check error!\n\n";
      exit(1);

   }

   return t[n];
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::add(const SingleThresh &st) {

   extend(Nelements + 1, false);

   t[Nelements] = st;

   Nelements++;

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::add(const double val, const ThreshType type) {
   SingleThresh st;

   st.set(val, type);

   extend(Nelements + 1, false);

   t[Nelements] = st;

   Nelements++;

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::add(const char *thresh_str) {
   SingleThresh st;

   st.set(thresh_str);

   extend(Nelements + 1, false);

   t[Nelements] = st;

   Nelements++;

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::add(const ThreshArray & a) {
   int j;

   if(a.n() == 0) return;

   extend(Nelements + a.n());

   for(j=0; j<(a.n()); j++) add(a[j]);

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::add_css(const char *text) {
   int j;
   StringArray sa;

   sa.parse_css(text);

   extend(Nelements + sa.n());

   for(j=0; j<sa.n(); j++) add(sa[j].c_str());

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::set(const SingleThresh &st) {

   clear();

   add(st);

   return;
}
////////////////////////////////////////////////////////////////////////

void ThreshArray::parse_thresh_str(const char *thresh_str) {
   char *c = (char *) nullptr;
   char *temp_ptr = (char *) nullptr;
   const char delim [] = " ";
   const char *method_name = "ThreshArray::parse_thresh_str()";

   char *line = m_strcpy2(thresh_str, method_name);
   if (line) {

      while((c = strtok_r(line, delim, &temp_ptr)) != nullptr ) {
      
         add(c);
      
      }

   }

   if(line) { delete [] line; line = (char *) nullptr; }

   return;
}

////////////////////////////////////////////////////////////////////////

int ThreshArray::has(const SingleThresh &st) const {
   int index, status;

   status = has(st, index);

   return status;
}

////////////////////////////////////////////////////////////////////////

int ThreshArray::has(const SingleThresh &st, int & index) const {
   int j;

   index = -1;

   if(Nelements == 0) return 0;

   for(j=0; j<Nelements; j++) {

      if(t[j] == st) { index = j; return 1; }
   }

   //
   // Not found
   //

   return 0;
}

////////////////////////////////////////////////////////////////////////

ConcatString ThreshArray::get_str(const char *sep, int precision) const {
   int i;
   ConcatString cur_str;
   ConcatString tmp_str;

   if(Nelements == 0) tmp_str = na_str;

   for(i=0; i<Nelements; i++) {
      cur_str = t[i].get_str(precision);

      if(i==0) tmp_str << cur_str;
      else     tmp_str << sep << cur_str;
   }

   return tmp_str;
}

////////////////////////////////////////////////////////////////////////

ConcatString ThreshArray::get_abbr_str(const char *sep, int precision) const {
   int i;
   ConcatString cur_str;
   ConcatString tmp_str;

   if(Nelements == 0) tmp_str = na_str;

   for(i=0; i<Nelements; i++) {
      cur_str = t[i].get_abbr_str(precision);

      if(i==0) tmp_str << cur_str;
      else     tmp_str << sep << cur_str;
   }

   return tmp_str;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::check_bin_thresh() const {
   int i;

   //
   // Check that the threshold values are monotonically increasing
   // and the threshold types are inequalities that remain the same
   //
   for(i=0; i<Nelements-1; i++) {

      if(t[i].get_value() >  t[i+1].get_value() ||
         t[i].get_type()  != t[i+1].get_type()  ||
         t[i].get_type()  == thresh_eq          ||
         t[i].get_type()  == thresh_ne) {

         mlog << Error << "\nThreshArray::check_bin_thresh() const -> "
              << "thresholds must be monotonically increasing and be of "
              << "the same inequality type (lt, le, gt, or ge): "
              << get_str() << "\n\n";
         exit(1);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

int ThreshArray::check_bins(double v) const {
   return check_bins(v, bad_data_double, bad_data_double);
}

////////////////////////////////////////////////////////////////////////

int ThreshArray::check_bins(double v, double mn, double sd) const {
   int i, bin;

   // Check for bad data or no thresholds
   if(is_bad_data(v) || Nelements == 0) return bad_data_int;

   // For < and <=, check thresholds left to right.
   if(t[0].get_type() == thresh_lt || t[0].get_type() == thresh_le) {

      for(i=0, bin=-1; i<Nelements; i++) {
         if(t[i].check(v, mn, sd)) {
            bin = i;
            break;
         }
      }
      if(bin == -1) bin = Nelements;
   }
   // For > and >=, check thresholds right to left.
   else {

      for(i=Nelements-1, bin=-1; i>=0; i--) {
         if(t[i].check(v, mn, sd)) {
            bin = i+1;
            break;
         }
      }
      if(bin == -1) bin = 0;
   }

   // The bin value returned is 1-based, not 0-based.

   return bin;
}

////////////////////////////////////////////////////////////////////////

bool ThreshArray::check_dbl(double v) const {
   return check_dbl(v, bad_data_double, bad_data_double);
}

////////////////////////////////////////////////////////////////////////

bool ThreshArray::check_dbl(double v, double mn, double sd) const {
   int i;

   //
   // Check if the value satisifes all the thresholds in the array
   //
   for(i=0; i<Nelements; i++) if(!t[i].check(v, mn, sd)) return false;

   return true;
}

////////////////////////////////////////////////////////////////////////

bool ThreshArray::need_perc() {
   bool status = false;

   for(int i=0; i<Nelements; i++) {
      if(t[i].need_perc()) {
         status = true;
         break;
      }
   }

   return status;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::set_perc(const NumArray *fptr, const NumArray *optr,
                           const NumArray *cptr) {

   if(Nelements == 0) return;

   for(int i=0; i<Nelements; i++) {
      t[i].set_perc(fptr, optr, cptr);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::set_perc(const NumArray *fptr, const NumArray *optr,
                           const NumArray *cptr, const ThreshArray *farr,
                           const ThreshArray *oarr) {

   if(Nelements == 0) return;

   if(!farr || !oarr) {
      mlog << Error << "\nThreshArray::set_perc() -> "
           << "no thresholds provided!\n\n";
      exit(1);
   }

   if(farr->n() != Nelements ||
      oarr->n() != Nelements) {
      mlog << Error << "\nThreshArray::set_perc() -> "
           << "not enough thresholds provided!\n\n";
      exit(1);
   }

   for(int i=0; i<Nelements; i++) {
      t[i].set_perc(fptr, optr, cptr,
                    &(farr->thresh()[i]), &(oarr->thresh()[i]));
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::multiply_by(const double x) {

   if(Nelements == 0) return;

   for(int i=0; i<Nelements; i++) t[i].multiply_by(x);

   return;

}

////////////////////////////////////////////////////////////////////////

void ThreshArray::get_simple_nodes(vector <Simple_Node> &v) {

   if(Nelements == 0) return;

   for(int i=0; i<Nelements; i++) t[i].get_simple_nodes(v);

   return;

}

////////////////////////////////////////////////////////////////////////

bool ThreshArray::equal_bin_width(double &width) const {

   // Check number of elements
   if(Nelements < 2) {
      width = bad_data_double;
      return false;
   }

   // Initialize return values
   width = t[1].get_value() - t[0].get_value();
   bool is_equal = true;

   // Check for consistent widths, ignoring the last bin
   for(int i=0; i<(Nelements-2); i++) {
      double cur_width = t[i+1].get_value() - t[i].get_value();
      if(!is_eq(width, cur_width, loose_tol)) {
         width = bad_data_double;
         is_equal = false;
         break;
      }
   } // end for i

   return is_equal;
}

////////////////////////////////////////////////////////////////////////
//
// External utility for parsing probability thresholds.
//
////////////////////////////////////////////////////////////////////////

ThreshArray string_to_prob_thresh(const char *s) {
   ThreshArray ta;

   // Parse the input string as a comma-separated list
   ta.add_css(s);

   // Handle special case of a single threshold of type equality
   if(ta.n() == 1 && ta[0].get_type() == thresh_eq) {

      // Store the threshold value
      double v = ta[0].get_value();

      // Threshold value must be between 0 and 1
      // or be an integer greater than 1
      if(v <= 0 || (v >=1  && !is_eq(nint(v), v))) {
         mlog << Error << "\nstring_to_prob_thresh() -> "
              << "the threshold string (" << s
              << ") must specify a probability bin width between 0 and 1 "
              << "or an integer number of ensemble members.\n\n";
         exit(1);
      }

      // Define probability bins from [0, 1] with equal width
      if(v > 0 && v < 1) {
         const char *ptr = strchr(s, '.');
         double prec = (ptr ? m_strlen(++ptr) : 0);
         ta = define_prob_bins(0.0, 1.0, v, prec);
      }
      // Define ensemble probability bins
      else {
         double inc = 1.0/nint(v);
         ta = define_prob_bins(-inc/2.0, 1.0+inc/2.0, inc, bad_data_int);
      }
   }

   // Check probability thresholds
   check_prob_thresh(ta);

   return ta;
}

////////////////////////////////////////////////////////////////////////

ThreshArray define_prob_bins(double beg, double end, double inc, int prec) {
   ThreshArray ta;
   double v;

   // Check inputs
   if(beg > 0 || end < 1 || inc <= 0 ||
      (!is_bad_data(prec) && prec < 0)) {
      mlog << Error << "\nget_prob_bins() -> "
           << "the probability thresholds must begin ("
           << beg << ") <= 0 and end ("
           << end << ") >= 1 with an increment ("
           << inc << ") between 0 and 1 and precision ("
           << prec << ") >= 0.\n\n";
      exit(1);
   }

   // Set the specified precision
   ConcatString cs;
   if(!is_bad_data(prec)) cs.set_precision(prec);

   // Construct a list of probability thresholds
   v = beg;
   while(v <= end) {
      cs << cs_erase << ">=" << v;
      ta.add(cs.c_str());
      v += inc;
   }

   return ta;
}

////////////////////////////////////////////////////////////////////////
//
// Convert array of probability thresholds to a string.
//
////////////////////////////////////////////////////////////////////////

ConcatString prob_thresh_to_string(const ThreshArray &ta) {
   ConcatString cs;
   ThreshArray prob_ta;
   double w;

   // Check for probability thresholds
   if(check_prob_thresh(ta, false)) {

      // Check for equal bin widths
      if(ta.equal_bin_width(w)) {
         if(is_eq(ta[0].get_value(), 0.0) &&
            is_eq(ta[(ta.n() - 1)].get_value(), 1.0)) {
            cs << "==" << w;
         }
         else {
            cs << "==" << nint(1/w);
         }
         prob_ta = string_to_prob_thresh(cs.c_str());
         if(!(ta == prob_ta)) cs.clear();
      }
   }

   // Return comma-separated list of thresholds
   if(cs.length() == 0) cs = ta.get_str();

   return cs;
}

////////////////////////////////////////////////////////////////////////

bool check_prob_thresh(const ThreshArray &ta, bool error_out) {

   const char * method_name = "check_prob_thresh() -> ";

   int n = ta.n();

   // Check for at least 3 thresholds that include the range [0, 1]
   if(n < 3 || ta[0].get_value() > 0 || ta[n-1].get_value() < 1) {
      if(error_out) {
         mlog << Error << "\n" << method_name
              << "when verifying a probability field, you must "
              << "select at least 3 thresholds which include the range [0, 1] "
              << "(current setting: " << ta.get_str() << ").\n"
              << "Consider using the \"==n\" shorthand notation to specify "
              << "probability bins of equal width for n < 1, or the integer "
              << "number of ensemble members for n > 1.\n\n";
         exit(1);
      }
      else {
         return false;
      }
   }

   // Check each probability bin
   for(int i=0; i<n; i++) {

      // Check that all threshold types are greater than or equal to
      if(ta[i].get_type() != thresh_ge) {
         if(error_out) {
            mlog << Error << "\n" << method_name
                 << "when verifying a probability field, all "
                 << "thresholds must be greater than or equal to, "
                 << "using \"ge\" or \">=\" (current setting: "
                 << ta.get_str() << ").\n"
                 << "Consider using the \"==n\" shorthand notation to specify "
                 << "probability bins of equal width, for n < 1, or the integer "
                 << "number of ensemble members, for n > 1.\n\n";
            exit(1);
         }
         else {
            return false;
         }
      }

      // Break out of the last loop
      if(i+1 == n) break;

      // Check that all probability bins overlap with [0, 1]
      if((ta[i].get_value() < 0 && ta[i+1].get_value() < 0) ||
         (ta[i].get_value() > 1 && ta[i+1].get_value() > 1)) {
         if(error_out) {
            mlog << Error << "\n" << method_name
                 << "when verifying a probability field, each probability bin "
                 << "must overlap the range [0, 1] (current setting: "
                 << ta.get_str() << ").\n"
                 << "Consider using the \"==n\" shorthand notation to specify "
                 << "probability bins of equal width, for n < 1, or the integer "
                 << "number of ensemble members, for n > 1.\n\n";
            exit(1);
         }
         else {
            return false;
         }
      }
   } // end for i

   return true;
}

////////////////////////////////////////////////////////////////////////
//
// Expand any percentile thresholds of type equality out to threshold
// bins which span 0 to 100.
//
////////////////////////////////////////////////////////////////////////

ThreshArray process_perc_thresh_bins(const ThreshArray &ta_in) {
   ThreshArray ta_bins, ta_out;
   ConcatString cs;
   int i, j;

   // Loop over and process each input threshold
   for(i=0; i<ta_in.n(); i++) {

      // Pass through non-equality thresholds thresholds to the output
      if(ta_in[i].get_type()   != thresh_eq                || 
         (ta_in[i].get_ptype() != perc_thresh_sample_fcst  &&
          ta_in[i].get_ptype() != perc_thresh_sample_obs   &&
          ta_in[i].get_ptype() != perc_thresh_sample_climo &&
          ta_in[i].get_ptype() != perc_thresh_climo_dist)) {
         ta_out.add(ta_in[i]);
      }
      // Expand single threshold to bins which span 0 to 100.
      else {

         // Store the threshold value
         int pvalue = nint(ta_in[i].get_pvalue());
         const char *ptype_str = perc_thresh_info[ta_in[i].get_ptype()].short_name;

         // Threshold value must be between 0 and 100
         if(pvalue <= 0 || pvalue >=100) {
            mlog << Error << "\nprocess_perc_thresh_bins() -> "
                 << "for percentile threshold (" << ta_in[i].get_str()
                 << ") the percentile threshold value (" << pvalue
                 << ") must be between 0 and 100.\n\n";
            exit(1);
         }

         // Construct list of percentile thresholds
         ta_bins.clear();
         for(j=0; (j+1)*pvalue<100; j++) {
            cs << cs_erase
               << ">="  << ptype_str << j*pvalue
               << "&&<" << ptype_str << (j+1)*pvalue;
            ta_bins.add(cs.c_str());
         }
         cs << cs_erase
            << ">="   << ptype_str << j*pvalue
            << "&&<=" << ptype_str << "100";
         ta_bins.add(cs.c_str());

         mlog << Debug(3) << "Expanded continuous percentile threshold (" 
              << ta_in[i].get_str() << ") to " << ta_bins.n()
              << " percentile bins (" << ta_bins.get_str() << ").\n";

         ta_out.add(ta_bins);
      }
   } // end for i

   return ta_out;
}

////////////////////////////////////////////////////////////////////////

ThreshArray process_rps_cdp_thresh(const ThreshArray &ta) {
   bool status = true;
   double step = bad_data_double;
   SingleThresh st;
   ThreshArray ta_out;

   // Check for evenly-spaced CDP thresholds
   for(int i=0; i<ta.n(); i++) {

      // Check for the CDP threshold type
      if(ta[i].get_ptype() != perc_thresh_climo_dist) {
         status = false;
         break;
      }

      // Check for even spacing
      if(i > 0) {
         if(is_bad_data(step)) {
            step = ta[i].get_pvalue() - ta[i-1].get_pvalue();
         }
         else if(!is_eq(step, ta[i].get_pvalue() - ta[i-1].get_pvalue())) {
            status = false;
            break;
         }
      }
   }

   if(status) {
      st.set(step, thresh_eq, perc_thresh_climo_dist);
      ta_out.add(st);
   }
   else {
      ta_out = ta;
   }

   return ta_out;
}

////////////////////////////////////////////////////////////////////////

ThreshArray derive_cdp_thresh(const ThreshArray &ta) {
   SingleThresh st;
   ThreshArray ta_out;

   for(int i=0; i<ta.n(); i++) {

      // Skip 0.0 and 1.0
      if(is_eq(ta[i].get_value(), 0.0) ||
         is_eq(ta[i].get_value(), 1.0)) continue;

      // Add CDP thresholds
      st.set(ta[i].get_value()*100.0, ta[i].get_type(), perc_thresh_climo_dist);

      ta_out.add(st);
   }

   return ta_out;
}

////////////////////////////////////////////////////////////////////////

ConcatString write_css(const ThreshArray &ta) {
   ConcatString css;

   for(int i=0; i<ta.n(); i++) {
      css << (i == 0 ? "" : ",") << ta[i].get_str();
   }

   return css;
}

////////////////////////////////////////////////////////////////////////
