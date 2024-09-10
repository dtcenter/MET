// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "compute_ci.h"
#include "contable.h"

#include "vx_util.h"
#include "vx_math.h"
#include "vx_gsl_prob.h"
#include "vx_log.h"

using namespace std;

////////////////////////////////////////////////////////////////////////
//
// Code for stats for 2x2 contingency tables
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::baser() const {
   return oy_tp();
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::baser_ci(double alpha,
                                    double &cl, double &cu) const {
   double v;

   v = baser();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fmean() const {
   return fy_tp();
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fmean_ci(double alpha,
                                    double &cl, double &cu) const {
   double v;

   v = fmean();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::accuracy() const {
   return compute_proportion(fy_oy() + fn_on(), n());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::accuracy_ci(double alpha,
                                       double &cl, double &cu) const {
   double v;

   v = accuracy();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return v;
}

////////////////////////////////////////////////////////////////////////
//
// Taken from eq. 7.9, page 241 in Wilks
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fbias() const {
   return compute_proportion(fy(), oy());
}

////////////////////////////////////////////////////////////////////////
//
// Taken from eq. 7.7, page 240 in Wilks
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::pod_yes() const {
   return compute_proportion(fy_oy(), oy());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::pod_yes_ci(double alpha,
                                      double &cl, double &cu) const {
   double v;

   v = pod_yes();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::pod_no() const {
   return compute_proportion(fn_on(), on());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::pod_no_ci(double alpha,
                                     double &cl, double &cu) const {
   double v;

   v = pod_no();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::pofd() const {
   double v;

   double d = pod_no();

   if(is_bad_data(d)) v = bad_data_double;
   else               v = 1.0 - d;

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::pofd_ci(double alpha,
                                   double &cl, double &cu) const {
   double v;

   v = pofd();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return v;
}

////////////////////////////////////////////////////////////////////////
//
// Taken from eq. 7.8, page 241 in Wilks
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::far() const {
   return compute_proportion(fy_on(), fy());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::far_ci(double alpha,
                                  double &cl, double &cu) const {
   double v;

   v = far();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return v;
}

////////////////////////////////////////////////////////////////////////
//
// Taken from eq. 7.6, page 240 in Wilks
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::csi() const {
   return compute_proportion(fy_oy(), n() - fn_on());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::csi_ci(double alpha,
                                  double &cl, double &cu) const {
   double v;

   v = csi();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return v;
}

////////////////////////////////////////////////////////////////////////
//
// Equations given by Barb Brown
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::gss() const {
   double a = fy_oy();
   double b = fy_on();
   double c = fn_oy();

   if(is_eq(n(), 0.0)) return bad_data_double;

   double q   = (a + b)*(a + c)/n();
   double num = (a - q);
   double den = (a + b + c - q);

   return compute_proportion(num, den);
}

////////////////////////////////////////////////////////////////////////
//
// Reference:
//    Bias Adjusted Precipitation Threat Scores
//    F. Mesinger, Adv. Geosci., 16, 137-142, 2008
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::bagss() const {

   if(is_eq(n(),     0.0) ||
      is_eq(oy(),    0.0) ||
      is_eq(fn_oy(), 0.0) ||
      is_eq(fy_on(), 0.0)) {
      return bad_data_double;
   }

   double lf = log(oy() / fn_oy());
   double lw = sf_lambert_W0(oy() / fy_on() * lf);
   double v;

   if(is_bad_data(lw) ||
      is_bad_data(lf) ||
      is_eq(lf, 0.0)) {
      v = bad_data_double;
   }
   else {
      double ha  = oy() - (fy_on() / lf) * lw;
      double num = ha - (oy() * oy() / n());
      double den = 2.0*oy() - ha - (oy() * oy() / n());

      v = compute_proportion(num, den);
   }

   return v;
}

////////////////////////////////////////////////////////////////////////
//
// Reference eq. 7.12, page 249 in Wilks
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::hk() const {
   double a = fy_oy();
   double b = fy_on();
   double c = fn_oy();
   double d = fn_on();

   double num = (a*d - b*c);
   double den = (a + c)*(b + d);

   return compute_proportion(num, den);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::hk_ci(double alpha,
                                 double &cl, double &cu) const {

   double v = hk();

   compute_hk_ci(v, alpha, 1.0,
                 fy_oy(), fy_on(),
                 fn_oy(), fn_on(),
                 cl, cu);

   return v;
}

////////////////////////////////////////////////////////////////////////
//
// Reference eq. 7.10, page 249 in Wilks
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::hss() const {
   double a = fy_oy();
   double b = fy_on();
   double c = fn_oy();
   double d = fn_on();

   double num = 2*(a*d - b*c);
   double den = (a + c)*(c + d) + (a + b)*(b + d);

   return compute_proportion(num, den);
}


////////////////////////////////////////////////////////////////////////
//
// Reference eq. 7.10, page 249 in Wilks
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::odds() const {
   double v;

   double py = pod_yes();
   double pn = pofd();

   if(is_eq(py, 1.0)  ||
      is_eq(pn, 1.0)  ||
      is_bad_data(py) ||
      is_bad_data(pn)) {
      v = bad_data_double;
   }

   else {
      double num = py/(1 - py);
      double den = pn/(1 - pn);
      v = compute_proportion(num, den);
   }

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::odds_ci(double alpha,
                                   double &cl, double &cu) const {

   double v = odds();

   compute_woolf_ci(v, alpha,
                    fy_oy(), fy_on(),
                    fn_oy(), fn_on(),
                    cl, cu);

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::lodds() const {
   double v;

   if(is_eq(fy_oy(), 0.0) ||
      is_eq(fy_on(), 0.0) ||
      is_eq(fn_oy(), 0.0) ||
      is_eq(fn_on(), 0.0)) {
      v = bad_data_double;
   }
   else {
      v = log(fy_oy()) + log(fn_on()) -
          log(fy_on()) - log(fn_oy());
   }

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::slor2() const {
   double v;

   if(is_eq(fy_oy(), 0.0) ||
      is_eq(fy_on(), 0.0) ||
      is_eq(fn_oy(), 0.0) ||
      is_eq(fn_on(), 0.0)) {
      v = bad_data_double;
   }
   else {
      double df = 1.0/(1.0/fy_oy() + 1.0/fy_on() + 1.0/fn_oy() + 1.0/fn_on());
      v = 1.0/df;
   }

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::lodds_ci(double alpha,
                                    double &cl, double &cu) const {
   double v = lodds();
   double s = slor2();

   if(is_bad_data(v) || is_bad_data(s)) {
      cl = cu = bad_data_double;
      return v;
   }

   //
   // Compute the standard error
   //
   double se = sqrt(s);

   compute_normal_ci(v, alpha, se, cl, cu);

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::orss() const {
   double r = odds();
   return compute_proportion(r - 1, r + 1);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::orss_ci(double alpha,
                                   double &cl, double &cu) const {
   double v = orss();
   double r = odds();
   double s = slor2();

   if(is_bad_data(v) || is_bad_data(r) || is_bad_data(s)) {
      cl = cu = bad_data_double;
   }
   else {

      // Compute the standard error
      double se = sqrt(s * 4.0 * pow(r, 2.0) / pow(r + 1, 4.0));

      compute_normal_ci(v, alpha, se, cl, cu);
   }

   return v;
}

////////////////////////////////////////////////////////////////////////
//
// Reference for EDS, SEDS, EDI, SEDI and standard errors:
//    Ferro, C. A. T., 2011: Extremal Dependence Indices: Improved
//    Verification Measures for Deterministic Forecasts of Rare
//    Binary Events, Wea. Forecasting, 26, 699-713.
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::eds() const {
   double v;

   if(is_eq(fy_oy(),           0.0) ||
      is_eq(fy_oy() + fn_oy(), 0.0) ||
      is_eq(n(),               0.0)) {
      v = bad_data_double;
   }
   else {
      double num = log((fy_oy() + fn_oy()) / n());
      double den = log( fy_oy() / n());

      if(is_eq(den, 0.0)) v = bad_data_double;
      else                v = 2.0 * num / den - 1.0;
   }

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::eds_ci(double alpha,
                                  double &cl, double &cu) const {
   double v = eds();
   double b = baser();
   double h = pod_yes();

   if(is_bad_data(v) ||
      is_bad_data(b) ||
      is_bad_data(h) ||
      is_eq(b, 0.0)  ||
      is_eq(h, 0.0)) {
      cl = cu = bad_data_double;
   }
   else {

      // Compute the standard error
      double se = 2.0 *
                  fabs(log(b)) / (h * pow(log(b) + log(h), 2.0)) *
                  sqrt(h * (1 - h) / (b * n()));

      compute_normal_ci(v, alpha, se, cl, cu);
   }

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::seds() const {
   double v;

   if(is_eq(fy_oy(),           0.0) ||
      is_eq(n(),               0.0) ||
      is_eq(fy_oy() + fn_oy(), 0.0) ||
      is_eq(fy_oy() + fy_on(), 0.0)) {
      v = bad_data_double;
   }
   else {
      double num = log((fy_oy() + fy_on()) / n()) +
                   log((fy_oy() + fn_oy()) / n());
      double den = log( fy_oy() / n());

      if(is_eq(den, 0.0)) v = bad_data_double;
      else                v = num / den - 1.0;
   }

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::seds_ci(double alpha,
                                   double &cl, double &cu) const {
   double v = seds();
   double b = baser();
   double h = pod_yes();

   if(is_bad_data(v) ||
      is_bad_data(b) ||
      is_bad_data(h) ||
      is_eq(b, 0.0)  ||
      is_eq(h, 0.0)) {
      cl = cu = bad_data_double;
   }
   else {

      // Compute the standard error
      double se = sqrt(h * (1.0 - h) / (n() * b)) *
                  (-1.0 * log(fbias() * pow(b, 2.0)) /
                  (h * pow(log(h * b), 2.0)));

      compute_normal_ci(v, alpha, se, cl, cu);
   }

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::edi() const {
   double v;

   double f = fy_on() / (fy_on() + fn_on());
   double h = pod_yes();

   if(is_bad_data(f) ||
      is_bad_data(h) ||
      is_eq(f, 0.0)  ||
      is_eq(h, 0.0)) {
      v = bad_data_double;
   }
   else {
      double num = log(f) - log(h);
      double den = log(f) + log(h);

      v = compute_proportion(num, den);
   }

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::edi_ci(double alpha,
                                  double &cl, double &cu) const {
   double v = edi();
   double f = fy_on() / (fy_on() + fn_on());
   double h = pod_yes();
   double b = baser();

   if(is_bad_data(f) ||
      is_bad_data(h) ||
      is_bad_data(b) ||
      is_eq(f, 0.0)  ||
      is_eq(h, 0.0)  ||
      is_eq(h, 1.0)) {
      cl = cu = bad_data_double;
   }
   else {
      // Compute the standard error
      double se = 2.0 * fabs(log(f) + h / (1.0 - h) * log(h)) /
                  (h * pow(log(f) + log(h), 2.0)) *
                  sqrt(h * (1.0 - h) / (b * n()));

      compute_normal_ci(v, alpha, se, cl, cu);
   }

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::sedi() const {
   double v;

   double f = fy_on() / (fy_on() + fn_on());
   double h = pod_yes();

   if(is_bad_data(f) ||
      is_bad_data(h) ||
      is_eq(f, 0.0)  ||
      is_eq(h, 0.0)  ||
      is_eq(f, 1.0)  ||
      is_eq(h, 1.0)) {
      v = bad_data_double;
   }
   else {
      double num = (log(f) - log(h) - log(1 - f) + log(1 - h));
      double den = (log(f) + log(h) + log(1 - f) + log(1 - h));

      v = compute_proportion(num, den);
   }

   return v;
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::sedi_ci(double alpha,
                                   double &cl, double &cu) const {
   double v = sedi();
   double f = fy_on() / (fy_on() + fn_on());
   double h = pod_yes();
   double b = baser();

   if(is_bad_data(f) ||
      is_bad_data(h) ||
      is_bad_data(b) ||
      is_eq(f, 0.0)  ||
      is_eq(h, 0.0)  ||
      is_eq(f, 1.0)  ||
      is_eq(h, 1.0)) {
      cl = cu = bad_data_double;
   }
   else {

      // Compute the standard error
      double mf = 1.0 - f;
      double mh = 1.0 - h;

      double se = 2.0 *
                  fabs((mh * mf + h * f) / (mh * mf) *
                        log(f * mh) + 2.0 * h / mh * log(h * mf)) /
                  (h * pow(log(f * mh) + log(h * mf), 2.0)) *
                  sqrt(h * mh / (b * n()));

      compute_normal_ci(v, alpha, se, cl, cu);
   }

   return v;
}

////////////////////////////////////////////////////////////////////////
//
//  Cost/Loss Relative Value
//  Reference: Eq. 8.75b, page 380 in Wilks, 3rd Ed. 2011
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::cost_loss(double r) const {
   double num;
   double den;

   if(is_eq(n(), 0.0)) return bad_data_double;

   // Total proportion of hits, misses, false alarms, and observations 
   double h = fy_oy() / n();
   double m = fn_oy() / n();
   double f = fy_on() / n();
   double b =    oy() / n();

   if(r < b) {
      num = (r * (h + f - 1)) + m;
      den =  r * (b - 1);
   }
   else {
      num = (r * (h + f)) + m - b;
      den =  b  * (r - 1);
   }

   return compute_proportion(num, den);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for stats for arbitrary size contingency tables
//
////////////////////////////////////////////////////////////////////////

double ContingencyTable::gaccuracy() const {

   if(Nrows != Ncols) {
      mlog << Error << "\nContingencyTable::gaccuracy() -> "
           << "table not square!\n\n";
      exit(1);
   }

   // MET #2542: return bad data for empty tables rather than erroring out
   if(E.empty()) return bad_data_double;

   double num = 0.0;
   for(int i=0; i<Nrows; i++) num += entry(i, i);

   double den = total();

   return compute_proportion(num, den);
}

////////////////////////////////////////////////////////////////////////
//
// Reference: Eq. 7.11, page 249 in Wilks, 1st Ed.
//
////////////////////////////////////////////////////////////////////////

double ContingencyTable::gheidke() const {

   if(Nrows != Ncols) {
      mlog << Error << "\nContingencyTable::gheidke() -> "
           << "table not square!\n\n";
      exit(1);
   }

   // MET #2542: return bad data for empty tables rather than erroring out
   if(E.empty()) return bad_data_double;

   // First term in numerator
   const double DN = total();
   double sum = 0.0;

   for(int j=0; j<Nrows; ++j) {
      sum += E[(rc_to_n(j,j))]/DN;
   }

   double num = sum;

   // Second term in numerator
   //    (also second term in denominator)

   sum = 0.0;
   for(int j=0; j<Nrows; ++j) {
      sum += (row_total(j)/DN)*(col_total(j)/DN);
   }

   num -= sum;

   double den = 1.0 - sum;

   return compute_proportion(num, den);
}

////////////////////////////////////////////////////////////////////////
//
//  Reference:
//  Ou et al., 2016: Sensitivity of Calibrated Week-2 Probabilistic
//     Forecast Skill to Reforecast Sampling of the NCEP Global
//     Ensemble Forecast System.
//     Weather and Forecasting, 31, 1093-1107.
//
////////////////////////////////////////////////////////////////////////

double ContingencyTable::gheidke_ec(double ec_value) const {

   if(Nrows != Ncols) {
      mlog << Error << "\nContingencyTable::gheidke_ec() -> "
           << "table not square!\n\n";
      exit(1);
   }

   if(ec_value < 0.0 || ec_value >= 1.0) {
      mlog << Error << "\nContingencyTable::gheidke_ec() -> "
           << "ec_value (" << ec_value << ") must be >=0 and <1.0!\n\n";
      exit(1);
   }

   // MET #2542: return bad data for empty tables rather than erroring out
   if(E.empty()) return bad_data_double;

   // Sum entries on the diagonal
   double sum = 0.0;
   for(int j=0; j<Nrows; ++j) {
      sum += E[(rc_to_n(j,j))];
   }

   // Expected correct by chance
   double DN = total();
   const double ec = DN * ec_value;

   double num = sum - ec;
   double den = DN - ec;

   return compute_proportion(num, den);
}

////////////////////////////////////////////////////////////////////////
//
// Reference: Eq. 7.13, page 250 in Wilks, 1st Ed.
//
////////////////////////////////////////////////////////////////////////

double ContingencyTable::gkuiper() const {

   if(Nrows != Ncols) {
      mlog << Error << "\nContingencyTable::gkuiper() -> "
           << "table not square!\n\n";
      exit(1);
   }

   // MET #2542: return bad data for empty tables rather than erroring out
   if(E.empty()) return bad_data_double;

   const double DN = total();

   // First term in numerator
   double sum = 0.0;
   for(int j=0; j<Nrows; ++j) {
      sum += E[(rc_to_n(j,j))]/DN;
   }

   double num = sum;

   // Second term in numerator
   sum = 0.0;
   for(int j=0; j<Nrows; ++j) {
      sum += (row_total(j)/DN)*(col_total(j)/DN);
   }

   num -= sum;

   // Second term in denominator
   sum = 0.0;
   for(int j=0; j<Nrows; ++j) {
      sum += pow(col_total(j)/DN, 2.0);
   }

   double den = 1.0 - sum;

   return(compute_proportion(num, den));
}

////////////////////////////////////////////////////////////////////////
//
// Reference: Pages 84-91 in
//    "Forecast Verification" by Jolliffe and Stephenson
//
////////////////////////////////////////////////////////////////////////

double ContingencyTable::gerrity() const {

   if(Nrows != Ncols) {
      mlog << Error << "\nContingencyTable::gerrity() -> "
           << "table not square!\n\n";
      exit(1);
   }

   // MET #2542: return bad data for empty tables rather than erroring out
   if(E.empty()) return bad_data_double;

   double DN = total();

   // Can't compute gerrity when the first column contains all zeros
   if(is_eq(col_total(0), 0.0)) return bad_data_double;

   // the p array
   vector<double> p(Nrows);
   for(int j=0; j<Nrows; ++j) {
      p[j] = col_total(j)/DN;
   }

   // Scoring matrix
   vector<double> s(Nrows*Nrows);
   calc_gerrity_scoring_matrix(Nrows, p, s);

   // Calculate score
   double sum = 0.0;
   for(int j=0; j<Nrows; ++j) {
      for(int k=0; k<Nrows; ++k) {
         int n = rc_to_n(j, k);
         sum += s[n] * E[n]/DN;
      }
   }

   // Replace nan with bad data
   if(is_bad_data(sum)) sum = bad_data_double;

   return sum;
}

////////////////////////////////////////////////////////////////////////
//
// Code for misc functions
//
////////////////////////////////////////////////////////////////////////

void calc_gerrity_scoring_matrix(int N, const vector<double> &p,
                                              vector<double> &s) {
   double b = 1.0/(N - 1.0);

   // the a array
   vector <double> a(N);
   double sum = 0.0;
   for(int j=0; j<N; ++j) {
      sum += p[j];
      a[j] = (1.0 - sum)/sum;
   }

   // the recip_sum array
   //    recip_sum[k] = sum(j=0 to k - 2) of 1/a[j]
   //    note that this forces recip_sum[0] to be zero
   vector <double> recip_sum(N);
   recip_sum[0] = 0.0;
   sum = 0.0;
   for(int j=1; j<N; ++j) {
      sum += 1.0/(a[j - 1]);
      recip_sum[j] = sum;
   }

   // the direct_sum array
   //    direct_sum[k] = sum(j=k to (N - 2)) of a[j]
   //    note that this forces direct_sum[N - 1] to be zero
   vector <double> direct_sum(N);
   direct_sum[N - 1] = 0.0;
   sum = 0.0;
   for(int j=(N - 2); j>=0; --j) {
      sum += a[j];
      direct_sum[j] = sum;
   }

   // Entries of the scoring matrix
   for(int j=0; j<N; ++j) {
      for(int k=j; k<N; ++k) {

         double t = b*( recip_sum[j] - (k - j) + direct_sum[k] );

         // Don't worry about which way we load
         // this matrix since it's symmetric
         int n = j*N + k;
         s[n] = t;
         n = k*N + j;
         s[n] = t;
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////
