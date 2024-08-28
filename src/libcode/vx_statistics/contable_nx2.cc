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
#include "gsl/gsl_cdf.h"

#include "compute_ci.h"
#include "contable.h"

#include "vx_log.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

static const int use_center = 1;

////////////////////////////////////////////////////////////////////////
//
// Code for class Nx2ContingencyTable
//
////////////////////////////////////////////////////////////////////////

Nx2ContingencyTable::Nx2ContingencyTable() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

Nx2ContingencyTable::~Nx2ContingencyTable() {
   clear();
}

////////////////////////////////////////////////////////////////////////

Nx2ContingencyTable::Nx2ContingencyTable(const Nx2ContingencyTable & t) {
   init_from_scratch();
   assign(t);
}

////////////////////////////////////////////////////////////////////////

void Nx2ContingencyTable::init_from_scratch() {
   ContingencyTable::init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

void Nx2ContingencyTable::clear() {

   ContingencyTable::clear();
   Thresholds.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

Nx2ContingencyTable & Nx2ContingencyTable::operator=(const Nx2ContingencyTable &t) {

   if(this == &t) return *this;
   assign(t);

   return *this;
}

////////////////////////////////////////////////////////////////////////

void Nx2ContingencyTable::assign(const Nx2ContingencyTable & t) {

   clear();

   ContingencyTable::assign(t);
   Thresholds = t.Thresholds;

   return;
}

////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::n() const {
   return total();
}

////////////////////////////////////////////////////////////////////////

void Nx2ContingencyTable::set_size(int N) {
   ContingencyTable::set_size(N, 2);
   return;
}

////////////////////////////////////////////////////////////////////////

void Nx2ContingencyTable::set_size(int NR, int NC) {

   if(NC != 2) {
      mlog << Error << "\nNx2ContingencyTable::set_size(int, int) -> "
           << "must have 2 columns, not " << NC << "!\n\n";
      exit(1);
   }

   set_size(NR);

   return;
}

////////////////////////////////////////////////////////////////////////

int Nx2ContingencyTable::value_to_row(double t) const {

   if(Thresholds.empty()) {
      mlog << Error << "\nNx2ContingencyTable::value_to_row(double) const -> "
           << "thresholds array not set!\n\n";
      exit(1);
   }

   //  Thresholds array is of size Nrows + 1, so
   //  the last element has index Nrows, not Nrows - 1
   if(t < Thresholds[0]     && !is_eq(t, Thresholds[0]))      return -1;
   if(t > Thresholds[Nrows] && !is_eq(t, Thresholds[Nrows]) ) return -1;

   for(int j=0; j<Nrows; ++j) {
      if ((t > Thresholds[j]     ||  is_eq(t, Thresholds[j])    ) &&
          (t < Thresholds[j + 1] && !is_eq(t, Thresholds[j + 1]))) return j;
   }

   if(is_eq(t, Thresholds[Nrows])) return (Nrows - 1);

   return -1;
}

////////////////////////////////////////////////////////////////////////

void Nx2ContingencyTable::set_thresholds(const vector<double> &Values) {

   if(Values.size() != Nrows + 1) {
      mlog << Error << "\nNx2ContingencyTable::set_thresholds(const double *) -> "
           << "expected " << Nrows + 1 << " thresholds but only received "
           << Values.size() << "!\n\n";
      exit(1);
   }

   Thresholds = Values;

   return;
}

////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::threshold(int k) const {

   // there are Nrows + 1 thresholds
   if(k < 0 || k > Thresholds.size()) {
      mlog << Error << "\nNx2ContingencyTable::threshold(int) const -> "
           << "range check error!\n\n";
      exit(1);
   }

   return Thresholds[k];
}

////////////////////////////////////////////////////////////////////////

void Nx2ContingencyTable::inc_event(double t, double weight) {
   int r = value_to_row(t);

   if(r < 0) {
      mlog << Error << "\nNx2ContingencyTable::inc_event(double) -> "
           << "bad value ... " << t << "\n\n";
      exit(1);
   }

   inc_entry(r, nx2_event_column, weight);

   return;
}

////////////////////////////////////////////////////////////////////////

void Nx2ContingencyTable::inc_nonevent(double t, double weight) {
   int r = value_to_row(t);

   if(r < 0) {
      mlog << Error << "\nNx2ContingencyTable::inc_nonevent(double) -> "
           << "bad value ... " << t << "\n\n";
      exit(1);
   }

   inc_entry(r, nx2_nonevent_column, weight);

   return;
}

////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::event_count_by_thresh(double t) const {
   int r = value_to_row(t);

   if(r < 0) {
      mlog << Error << "\nNx2ContingencyTable::event_count_by_thresh(double) -> "
           << "bad value ... " << t << "\n\n";
      exit(1);
   }

   return entry(r, nx2_event_column);
}

////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::nonevent_count_by_thresh(double t) const {
   int r = value_to_row(t);

   if(r < 0) {
      mlog << Error << "\nNx2ContingencyTable::nonevent_count_by_thresh(double) -> "
           << "bad value ... " << t << "\n\n";
      exit(1);
   }

   return entry(r, nx2_nonevent_column);
}

////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::row_obar(int row) const {
   return compute_proportion(event_count_by_row(row), row_total(row));
}

////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::obar() const {
   return compute_proportion(event_col_total(), n());
}

////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::row_proby(int row) const {

   if(row < 0 || row >= Nrows) {
      mlog << Error << "\nNx2ContingencyTable::row_proby(int) const -> "
           << "range check error\n\n";
      exit(1);
   }

   double v;

   if(use_center) v = 0.5*(Thresholds[row] + Thresholds[row + 1]);
   else           v = Thresholds[row];

   return v;
}

////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::baser() const {
   return compute_proportion(event_col_total(), n());
}

////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::baser_ci(double alpha,
                                     double &cl, double &cu) const {
   double v = baser();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return v;
}

////////////////////////////////////////////////////////////////////////
//
// Reference: Equation 7.40, page 286 in Wilks, 2nd Ed.
//
////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::reliability() const {
   double sum = 0.0;
   for(int row=0; row<Nrows; ++row) {

      double Ni    = row_total(row);
      double yi    = row_proby(row);
      double obari = row_obar(row);

      // When obari is not defined, do not include in the sum
      if(is_bad_data(obari)) continue;

      double t = yi - obari;

      sum += Ni*t*t;
   }

   return compute_proportion(sum, n());
}

////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::resolution() const {
   double Obar = obar();
   double sum = 0.0;

   for(int row=0; row<Nrows; ++row) {

      double Ni = row_total(row);
      double obari = row_obar(row);

      // When obari is not defined, do not include it in the sum
      if(is_bad_data(obari)) continue;

      double t = obari - Obar;

      sum += Ni*t*t;
   }

   return compute_proportion(sum, n());
}

////////////////////////////////////////////////////////////////////////


double Nx2ContingencyTable::uncertainty() const {
   double a = obar();
   double v;

   if(is_bad_data(a)) v = bad_data_double;
   else               v = a*(1.0 - a);

   return v;
}

////////////////////////////////////////////////////////////////////////
//
// Reference: Equation 8.43, page 340 in Wilks, 3rd Ed.
//
////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::bss_smpl() const {
   double res = resolution();
   double rel = reliability();
   double unc = uncertainty();
   double bss;

   if(is_bad_data(res) || is_bad_data(rel) ||
      is_bad_data(unc) || is_eq(unc, 0.0)) {
      bss = bad_data_double;
   }
   else {
      bss = (res - rel)/unc;
   }

   return bss;
}

////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::brier_score() const {

   if(E.empty()) return bad_data_double;

   double sum = 0.0;
   double count;
   double yi;
   double t;

   // Terms for event
   for(int j=0; j<Nrows; ++j) {
      count = event_count_by_row(j);
      yi = row_proby(j);
      t = yi - 1.0;
      sum += count*t*t;
   }

   // Terms for nonevent
   for(int j=0; j<Nrows; ++j) {
      count = nonevent_count_by_row(j);
      yi = row_proby(j);
      t = yi;
      sum += count*t*t;
   }

   return compute_proportion(sum, n());
}

////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::row_calibration(int row) const {

   if(row < 0 || row >= Nrows) {
      mlog << Error << "\nNx2ContingencyTable::row_calibration(int) const -> "
           << "range check error\n\n";
      exit(1);
   }

   double num = event_count_by_row(row);
   double den = num + nonevent_count_by_row(row);

   return compute_proportion(num, den);
}

////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::row_refinement(int row) const {

   if(row < 0 || row >= Nrows) {
      mlog << Error << "\nNx2ContingencyTable::row_refinement(int) const -> "
           << "range check error\n\n";
      exit(1);
   }

   return compute_proportion(   event_count_by_row(row) +
                             nonevent_count_by_row(row), n());
}

////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::row_event_likelihood(int row) const {

   if(row < 0 || row >= Nrows) {
      mlog << Error << "\nNx2ContingencyTable::row_event_likelihood(int) const -> "
           << "range check error\n\n";
      exit(1);
   }

   return compute_proportion(event_count_by_row(row),
                             event_col_total());
}

////////////////////////////////////////////////////////////////////////


double Nx2ContingencyTable::row_nonevent_likelihood(int row) const {

   if(row < 0 || row >= Nrows) {
      mlog << Error << "\nNx2ContingencyTable::row_nonevent_likelihood(int) const -> "
           << "range check error\n\n";
      exit(1);
   }

   return compute_proportion(nonevent_count_by_row(row),
                             nonevent_col_total());
}

////////////////////////////////////////////////////////////////////////

TTContingencyTable Nx2ContingencyTable::ctc_by_row(int row) const {
   TTContingencyTable tt;

   if(row < 0 || row >= Nrows) {
      mlog << Error << "\nNx2ContingencyTable::ctc_by_row(int) const -> "
           << "range check error\n\n";
      exit(1);
   }

   double sy;
   double sn;

   for(int j=(row + 1), sy=sn=0.0; j<Nrows; ++j) {
      sy +=    event_count_by_row(j);
      sn += nonevent_count_by_row(j);
   }

   tt.set_fy_oy(sy);
   tt.set_fy_on(sn);

   for(int j=0, sy=sn=0.0; j<=row; ++j) {
      sy +=    event_count_by_row(j);
      sn += nonevent_count_by_row(j);
   }

   tt.set_fn_oy(sy);
   tt.set_fn_on(sn);

   return tt;
}

////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::roc_auc() const {
   TTContingencyTable ct;
   double x_prev = 1.0;
   double y_prev = 1.0;
   double area = 0.0;

   for(int j=0, area=bad_data_double; j<Nrows; ++j) {

      // 2x2 Contingency Table for this row
      ct = ctc_by_row(j);

      // Retrieve the ROC point for this row
      double x = ct.pofd();
      double y = ct.pod_yes();

      if(is_bad_data(x) || is_bad_data(y)) continue;

      // Compute area under the curve using the trapezoid rule
      area += (x_prev - x)*(y_prev + y)*0.5;

      // Save current ROC point as the previous point
      x_prev = x;
      y_prev = y;
   }

   return area;
}

////////////////////////////////////////////////////////////////////////
//
// Compute a confidence interval for the Brier Score.
// Reference:
// Bradley, A.A, S.S. Schwartz, and T. Hashino, 2008:
// Sampling Uncertainty and Confidence Intervals for the Brier Score
// and Brier Skill Score.  Weather and Forecasting, 23, 992-1006.
//
////////////////////////////////////////////////////////////////////////

double Nx2ContingencyTable::brier_ci_halfwidth(double alpha) const {
   double bs = brier_score();
   const double N = n();
   const double Ninv = 1.0/N;

   // N must be > 1 so that degf > 0 in the call to gsl_cdf_tdist_Pinv()
   if(is_bad_data(bs) || N <= 1.0)  return bad_data_double;

   double degf = N - 1.0;
   double t = gsl_cdf_tdist_Pinv(1.0 - 0.5*alpha, degf);
   double ob = obar();

   double af1 = 0.0;
   double sf2 = 0.0;
   double sf3 = 0.0;
   double af4 = 0.0;

   for(int j=0; j<Nrows; ++j) {

      double ee = Ninv*(   event_count_by_row(j));
      double ne = Ninv*(nonevent_count_by_row(j));
      double term = ee + ne;

      term *= term; //  squared
      term *= term; //  fourth power
      af4 += term;
      term = ee*ee;
      af1 += ee;
      sf2 += term;
      sf3 += term*ee;
   }

   af4 *= Ninv;
   af1 *= Ninv;

   double term = 1.0 - 4.0*sf3 + 6.0*sf2 + 4.0*af1;
   double var = (af4 + ob*term - bs*bs)*Ninv;
   double halfwidth = t*sqrt(var);

   return halfwidth;
}

////////////////////////////////////////////////////////////////////////
