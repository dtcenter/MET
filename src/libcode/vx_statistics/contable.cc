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
#include <cstdio>
#include <cmath>
#include <vector>

#include "contable.h"

#include "vx_util.h"
#include "vx_log.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

static int table_rc_to_n(int r_table, int c_table, int w, int h);

////////////////////////////////////////////////////////////////////////
//
// Code for class ContingencyTable
//
////////////////////////////////////////////////////////////////////////

ContingencyTable::ContingencyTable() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ContingencyTable::~ContingencyTable() {
   clear();
}

////////////////////////////////////////////////////////////////////////

ContingencyTable::ContingencyTable(const ContingencyTable & t) {
   init_from_scratch();
   assign(t);
}

////////////////////////////////////////////////////////////////////////

ContingencyTable & ContingencyTable::operator=(const ContingencyTable & t) {
   if(this == &t) return *this;
   assign(t);
   return *this;
}

////////////////////////////////////////////////////////////////////////

ContingencyTable & ContingencyTable::operator+=(const ContingencyTable & t) {

   // Check consistent dimensions
   if(Nrows != t.Nrows || Ncols != t.Ncols) {
      mlog << Error << "\nContingencyTable::operator+=() -> "
           << "table dimensions do not match: (" << Nrows << ", " << Ncols
           << ") != (" << t.Nrows << ", " << t.Ncols << ")\n\n";
      exit(1);
   }

   // Check consistent expected correct
   if(!is_eq(ECvalue, t.ECvalue)) {
      mlog << Error << "\nContingencyTable::operator+=() -> "
           << "the expected correct values do not match: "
           << ECvalue << " != " << t.ECvalue << "\n\n";
      exit(1);
   }

   // Increment table entries
   for(int i=0; i<E.size(); ++i) E[i] += t.E[i];

   return *this;
}

////////////////////////////////////////////////////////////////////////

void ContingencyTable::init_from_scratch() {
}

////////////////////////////////////////////////////////////////////////

void ContingencyTable::clear() {

    E.clear();
    Nrows = Ncols = 0;
    ECvalue = bad_data_double;
    Name.clear();

    return;
}

////////////////////////////////////////////////////////////////////////

void ContingencyTable::assign(const ContingencyTable & t) {

   clear();

   E = t.E;
   Nrows = t.Nrows;
   Ncols = t.Ncols;
   ECvalue = t.ECvalue;
   Name = t.Name;

   return;
}

////////////////////////////////////////////////////////////////////////

void ContingencyTable::zero_out() {

   fill(E.begin(), E.end(), 0.0);

   return;
}

////////////////////////////////////////////////////////////////////////

void ContingencyTable::dump(ostream & out, int depth) const {
   Indent prefix(depth);
   ConcatString msg;

   out << prefix << "Name    = ";

   if(Name.nonempty()) out << R"(")" << Name << R"(")" << "\n";
   else                out << "(nul)\n";

   out << prefix << "Nrows   = " << Nrows << "\n";
   out << prefix << "Ncols   = " << Ncols << "\n";
   out << prefix << "ECvalue = " << ECvalue << "\n";
   out << prefix << "\n";

   if(!E.empty()) {

      for(int r=0; r<Nrows; ++r) {
         msg.format("Sum for row %2d is %f", r, row_total(r));
         out << prefix << msg << "\n";
         if((r%5) == 4 && r != (Nrows - 1)) out.put('\n');
      }

      out << prefix << "\n";

      for(int c=0; c<Ncols; ++c) {
         msg.format("Sum for col %2d is %f", c, col_total(c));
         out << prefix << msg << "\n";
         if((c%5) == 4 && c != (Ncols - 1)) out.put('\n');
      }

      out << prefix << "\n";

      out << prefix << "Table Total = " << total() << "\n";

      out << prefix << "\n";
   }

   out.flush();

   return;
}

////////////////////////////////////////////////////////////////////////

void ContingencyTable::set_size(int N) {

   ContingencyTable::set_size(N, N);

   return;
}

////////////////////////////////////////////////////////////////////////

void ContingencyTable::set_size(int NR, int NC) {

   clear();

   if(NR < 2 || NC < 2) {
      mlog << Error << "\nContingencyTable::set_size() -> "
           << "# rows (" << NR << ") and # cols (" << NC
           << ") must be at least 2!\n\n";
      exit(1);
   }

   Nrows = NR;
   Ncols = NC;

   E.resize(NR*NC, 0.0);

   // Set default expected correct value for square tables
   if(Nrows == Ncols) ECvalue = 1.0 / Nrows;

   return;
}

////////////////////////////////////////////////////////////////////////

void ContingencyTable::set_ec_value(double v) {

   // Do not override the default value with bad data
   if(!is_bad_data(v)) ECvalue = v;

   return;
}

////////////////////////////////////////////////////////////////////////

void ContingencyTable::set_name(const char * text) {

   Name = text;

   return;
}

////////////////////////////////////////////////////////////////////////

int ContingencyTable::rc_to_n(int r, int c) const {

   if(r < 0 || r >= Nrows || c < 0 || c >= Ncols) {
      mlog << Error << "\nContingencyTable::rc_to_n() -> "
           << "range check error requesting (" << r << ", "
           << c << ") from table with dimension (" << Nrows
           << ", " << Ncols << ")!\n\n";
      exit(1);
   }

   return r*Ncols + c;
}

////////////////////////////////////////////////////////////////////////

void ContingencyTable::set_entry(int row, int col, double value) {

   E[(rc_to_n(row, col))] = value;

   return;
}

////////////////////////////////////////////////////////////////////////

void ContingencyTable::inc_entry(int row, int col, double weight) {

   E[(rc_to_n(row, col))] += weight;

   return;
}

////////////////////////////////////////////////////////////////////////

double ContingencyTable::total() const {
   double sum = 0.0;

   for(auto &x : E) sum += x;

   return sum;
}

////////////////////////////////////////////////////////////////////////

double ContingencyTable::row_total(int row) const {
   double sum = 0.0;

   for(int col=0; col<Ncols; ++col) {
      sum += E[(rc_to_n(row, col))];
   }

   return sum;
}

////////////////////////////////////////////////////////////////////////

double ContingencyTable::col_total(int col) const {
   double sum = 0.0;

   for(int row=0; row<Nrows; ++row) {
      sum += E[(rc_to_n(row, col))];
   }

   return sum;
}

////////////////////////////////////////////////////////////////////////

double ContingencyTable::entry(int row, int col) const {
   return E[(rc_to_n(row, col))];
}

////////////////////////////////////////////////////////////////////////

double ContingencyTable::max() const {
   double max;

   if(E.empty()) max = 0.0;
   else          max = *max_element(E.begin(), E.end());

   return max;
}

////////////////////////////////////////////////////////////////////////

double ContingencyTable::min() const {
   double min;

   if(E.empty()) min = 0.0;
   else          min = *min_element(E.begin(), E.end());

   return min;
}

////////////////////////////////////////////////////////////////////////

bool ContingencyTable::is_integer() const {
   bool status = true;

   // Check that each entry contains an integer
   for(auto &x : E) {
      if(!is_eq(trunc(x), x)) {
         status = false;
         break;
      }
   }

   return status;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class TTContingencyTable
//
////////////////////////////////////////////////////////////////////////

TTContingencyTable::TTContingencyTable() {
   ContingencyTable::set_size(2, 2);
}

////////////////////////////////////////////////////////////////////////

TTContingencyTable::~TTContingencyTable() {
}

////////////////////////////////////////////////////////////////////////

TTContingencyTable::TTContingencyTable(const TTContingencyTable & t) {
   assign(t);
}

////////////////////////////////////////////////////////////////////////

TTContingencyTable & TTContingencyTable::operator=(const TTContingencyTable & t) {

   if(this == &t) return *this;

   assign(t);

   return *this;
}

////////////////////////////////////////////////////////////////////////

void TTContingencyTable::set_fn_on(double k) {

   set_entry(FN_row, ON_col, k);

   return;
}

////////////////////////////////////////////////////////////////////////

void TTContingencyTable::set_fy_on(double k) {

   set_entry(FY_row, ON_col, k);

   return;
}

////////////////////////////////////////////////////////////////////////

void TTContingencyTable::set_fn_oy(double k) {

   set_entry(FN_row, OY_col, k);

   return;
}

////////////////////////////////////////////////////////////////////////

void TTContingencyTable::set_fy_oy(double k) {

   set_entry(FY_row, OY_col, k);

   return;
}

////////////////////////////////////////////////////////////////////////

void TTContingencyTable::inc_fn_on(double weight) {

   inc_entry(FN_row, ON_col, weight);

   return;
}

////////////////////////////////////////////////////////////////////////

void TTContingencyTable::inc_fy_on(double weight) {

   inc_entry(FY_row, ON_col, weight);

   return;
}

////////////////////////////////////////////////////////////////////////

void TTContingencyTable::inc_fn_oy(double weight) {

   inc_entry(FN_row, OY_col, weight);

   return;
}

////////////////////////////////////////////////////////////////////////

void TTContingencyTable::inc_fy_oy(double weight) {

   inc_entry(FY_row, OY_col, weight);

   return;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fy_oy() const {
   return entry(FY_row, OY_col);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fy_on() const {
   return entry(FY_row, ON_col);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fn_oy() const {
   return entry(FN_row, OY_col);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fn_on() const {
   return entry(FN_row, ON_col);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fy() const {
   return row_total(FY_row);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fn() const {
   return row_total(FN_row);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::oy() const {
   return col_total(OY_col);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::on() const {
   return col_total(ON_col);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::n() const {
   return total();
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::f_rate() const {
   return compute_proportion(fy(), n());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::h_rate() const {
   return compute_proportion(fy_oy(), n());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::o_rate() const {
   return compute_proportion(oy(), n());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fy_oy_tp() const {
   return compute_proportion(fy_oy(), n());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fy_on_tp() const {
   return compute_proportion(fy_on(), n());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fn_oy_tp() const {
   return compute_proportion(fn_oy(), n());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fn_on_tp() const {
   return compute_proportion(fn_on(), n());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fy_tp() const {
   return compute_proportion(fy(), n());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fn_tp() const {
   return compute_proportion(fn(), n());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::oy_tp() const {
   return compute_proportion(oy(), n());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::on_tp() const {
   return compute_proportion(on(), n());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fy_oy_fp() const {
   return compute_proportion(fy_oy(), fy());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fy_on_fp() const {
   return compute_proportion(fy_on(), fy());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fn_oy_fp() const {
   return compute_proportion(fn_oy(), fn());
}

////////////////////////////////////////////////////////////////////////


double TTContingencyTable::fn_on_fp() const {
   return compute_proportion(fn_on(), fn());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fy_oy_op() const {
   return compute_proportion(fy_oy(), oy());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fy_on_op() const {
   return compute_proportion(fy_on(), on());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fn_oy_op() const {
   return compute_proportion(fn_oy(), oy());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fn_on_op() const {
   return compute_proportion(fn_on(), on());
}

////////////////////////////////////////////////////////////////////////

void TTContingencyTable::set_size(int N) {
   mlog << Error << "\nTTContingencyTable::set_size(int) -> "
        << "2 x 2 tables cannot be resized!\n\n";
   exit(1);
}

////////////////////////////////////////////////////////////////////////

void TTContingencyTable::set_size(int NR, int NC) {
   mlog << Error << "\nTTContingencyTable::set_size(int, int) -> "
        << "2 x 2 tables cannot be resized!\n\n";
   exit(1);
}

////////////////////////////////////////////////////////////////////////
//
// Code for misc functions
//
////////////////////////////////////////////////////////////////////////

//
// Reference table 7.1a, page 242 in wilks
//

TTContingencyTable finley() {
   TTContingencyTable t;

   t.set_fy_oy(28);
   t.set_fn_oy(23);
   t.set_fy_on(72);
   t.set_fn_on(2680);

   t.set_name("Finley Tornado Forecasts (1884)");

   return t;
}

////////////////////////////////////////////////////////////////////////

//
// Reference table 7.1b, page 242 in wilks
//

TTContingencyTable finley_always_no() {
   TTContingencyTable t;

   t.set_fy_oy(0);
   t.set_fn_oy(51);
   t.set_fy_on(0);
   t.set_fn_on(2752);

   t.set_name("Finley Tornado Forecasts (Always No) (1884)");

   return t;
}

////////////////////////////////////////////////////////////////////////
// r_table < h
// c_table < w

int table_rc_to_n(int r_table, int c_table, int w, int h) {
   return r_table*w + c_table;
}

////////////////////////////////////////////////////////////////////////

double compute_proportion(double num, double den) {
   double prop;

   // Check for bad data and divide by zero
   if(is_bad_data(num)   ||
      is_bad_data(den) ||
      is_eq(den, 0.0)) {
      prop = bad_data_double;
   }
   else {
      prop = num/den;
   }

   return prop;
}

////////////////////////////////////////////////////////////////////////
