// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

using namespace std;

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

////////////////////////////////////////////////////////////////////////
//
// Code for stats for 2x2 contingency tables
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::baser() const {

   return(oy_tp());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::baser_ci(double alpha,
                                    double &cl, double &cu) const {
   double v;

   v = baser();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fmean() const {

   return(fy_tp());
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fmean_ci(double alpha,
                                    double &cl, double &cu) const {
   double v;

   v = fmean();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::accuracy() const {
   double num, den, v;

   num = (double) fy_oy() + fn_on();
   den = (double) n();

   if(is_eq(den, 0.0)) v = bad_data_double;
   else                v = num/den;

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::accuracy_ci(double alpha,
                                       double &cl, double &cu) const {
   double v;

   v = accuracy();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return(v);
}

////////////////////////////////////////////////////////////////////////
//
// Taken from eq. 7.9, page 241 in Wilks
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::fbias() const {
   double num, den, v;

   num = (double) fy();
   den = (double) oy();

   if(is_eq(den, 0.0)) v = bad_data_double;
   else                v = num/den;

   return(v);
}

////////////////////////////////////////////////////////////////////////
//
// Taken from eq. 7.7, page 240 in Wilks
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::pod_yes() const {
   double num, den, v;

   num = (double) fy_oy();
   den = (double) oy();

   if(is_eq(den, 0.0)) v = bad_data_double;
   else                v = num/den;

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::pod_yes_ci(double alpha,
                                      double &cl, double &cu) const {
   double v;

   v = pod_yes();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::pod_no() const {
   double num, den, v;

   num = (double) fn_on();
   den = (double) on();

   if(is_eq(den, 0.0)) v = bad_data_double;
   else                v = num/den;

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::pod_no_ci(double alpha,
                                     double &cl, double &cu) const {
   double v;

   v = pod_no();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::pofd() const {
   double d, v;

   d = pod_no();

   if(is_bad_data(d)) v = bad_data_double;
   else               v = 1.0 - d;

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::pofd_ci(double alpha,
                                   double &cl, double &cu) const {
   double v;

   v = pofd();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return(v);
}

////////////////////////////////////////////////////////////////////////
//
// Taken from eq. 7.8, page 241 in Wilks
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::far() const {
   double num, den, v;

   num = (double) fy_on();
   den = (double) fy();

   if(is_eq(den, 0.0)) v = bad_data_double;
   else                v = num/den;

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::far_ci(double alpha,
                                  double &cl, double &cu) const {
   double v;

   v = far();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return(v);
}

////////////////////////////////////////////////////////////////////////
//
// Taken from eq. 7.6, page 240 in Wilks
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::csi() const {
   double num, den, v;

   num = (double) fy_oy();
   den = (double) (n() - fn_on());

   if(is_eq(den, 0.0)) v = bad_data_double;
   else                v = num/den;

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::csi_ci(double alpha,
                                  double &cl, double &cu) const {
   double v;

   v = csi();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return(v);
}

////////////////////////////////////////////////////////////////////////
//
// Equations given by Barb Brown
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::gss() const {
   long long a, b, c;
   double q, num, den, v;

   a = (long long) fy_oy();
   b = (long long) fy_on();
   c = (long long) fn_oy();

   if(n() == 0) return(bad_data_double);

   q   = (double) (a + b)*(a + c)/(n());
   num = (double) (a - q);
   den = (double) (a + b + c - q);

   if(is_eq(den, 0.0)) v = bad_data_double;
   else                v = num/den;

   return(v);
}

////////////////////////////////////////////////////////////////////////
//
// BAGSS Reference:
//    Bias Adjusted Precipitation Threat Scores
//    F. Mesinger, Adv. Geosci., 16, 137-142, 2008
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::bagss() const {
   double lf, lw, ha, num, den, v;

   if(n() == 0 || oy() == 0 || fn_oy() == 0 || fy_on() == 0) {
      return(bad_data_double);
   }

   lf = log((double) oy() / fn_oy());
   lw = sf_lambert_W0((double) oy() / fy_on() * lf);

   if(is_bad_data(lw) || is_bad_data(lf) || is_eq(lf, 0.0)) {
      return(bad_data_double);
   }
   else {
      ha = (double) oy() - (fy_on() / lf) * lw;
   }

   num = ha - (oy() * oy() / n());
   den = 2.0*oy() - ha - (oy() * oy() / n());

   if(is_eq(den, 0.0)) v = bad_data_double;
   else                v = num/den;

   return(v);
}

////////////////////////////////////////////////////////////////////////
//
// Taken from eq. 7.12, page 249 in Wilks
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::hk() const {
   long long a, b, c, d;
   long long num, den;
   double v;

   a = (long long) fy_oy();
   b = (long long) fy_on();
   c = (long long) fn_oy();
   d = (long long) fn_on();

   num = (a*d - b*c);
   den = (a + c)*(b + d);

   if(is_eq(den, 0.0)) v = bad_data_double;
   else                v = (double) num/den;

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::hk_ci(double alpha,
                                 double &cl, double &cu) const {
   double v;

   v = hk();

   compute_hk_ci(v, alpha, 1.0, fy_oy(), fy_on(), fn_oy(), fn_on(),
                 cl, cu);

   return(v);
}

////////////////////////////////////////////////////////////////////////
//
// Taken from eq. 7.10, page 249 in Wilks
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::hss() const {
   long long a, b, c, d;
   long long num, den;
   double v;

   a = (long long) fy_oy();
   b = (long long) fy_on();
   c = (long long) fn_oy();
   d = (long long) fn_on();

   num = 2*(a*d - b*c);
   den = (a + c)*(c + d) + (a + b)*(b + d);

   if(is_eq(den, 0.0)) v = bad_data_double;
   else                v = (double) num/den;

   return(v);
}


////////////////////////////////////////////////////////////////////////
//
// Taken from eq. 7.10, page 249 in Wilks
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::odds() const {
   double num, den, v, py, pn;

   py = pod_yes();
   pn = pofd();

   if(is_eq(py, 1.0) || is_bad_data(py) ||
      is_eq(pn, 1.0) || is_bad_data(pn)) return(bad_data_double);

   num = py/(1 - py);
   den = pn/(1 - pn);

   if(is_eq(den, 0.0)) v = bad_data_double;
   else                v = num/den;

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::odds_ci(double alpha,
                                   double &cl, double &cu) const {
   double v;

   v = odds();

   compute_woolf_ci(v, alpha, fy_oy(), fy_on(), fn_oy(), fn_on(),
                    cl, cu);

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::lodds() const {
   double v;

   if(fy_oy() == 0 || fy_on() == 0 || fn_oy() == 0 || fn_on() == 0) {
      v = bad_data_double;
   }
   else {
      v = log((double) fy_oy()) + log((double) fn_on()) -
          log((double) fy_on()) - log((double) fn_oy());
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::slor2() const {
   double v, df;

   if(fy_oy() == 0 || fy_on() == 0 || fn_oy() == 0 || fn_on() == 0) {
      v = bad_data_double;
   }
   else {
      df = 1.0/(1.0/fy_oy() + 1.0/fy_on() + 1.0/fn_oy() + 1.0/fn_on());
      v  = 1.0/df;
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::lodds_ci(double alpha,
                                    double &cl, double &cu) const {
   double v, s, se;

   v = lodds();
   s = slor2();

   if(is_bad_data(v) || is_bad_data(s)) {
      cl = cu = bad_data_double;
      return(v);
   }

   //
   // Compute the standard error
   //
   se = sqrt(s);

   compute_normal_ci(v, alpha, se, cl, cu);

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::orss() const {
   double v, num, den;

   num = fy_oy() * fn_on() - fy_on() * fn_oy();
   den = fy_oy() * fn_on() + fy_on() * fn_oy();

   if(den == 0) v = bad_data_double;
   else         v = (double) num / den;

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::orss_ci(double alpha,
                                   double &cl, double &cu) const {
   double v, r, s, se;

   v = orss();
   r = odds();
   s = slor2();

   if(is_bad_data(v) || is_bad_data(r) || is_bad_data(s)) {
      cl = cu = bad_data_double;
      return(v);
   }

   //
   // Compute the standard error
   //
   se = sqrt(s * 4.0 * pow(r, 2.0) / pow(r + 1, 4.0));

   compute_normal_ci(v, alpha, se, cl, cu);

   return(v);
}

////////////////////////////////////////////////////////////////////////
//
// Reference for EDS, SEDS, EDI, SEDI and standard errors:
//    Ferro, C. A. T., 2011: Extremal Dependence Indices: Improved
//       Verification Measures for Deterministic Forecasts of Rare
//       Binary Events, Wea. Forecasting, 26, 699-713.
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::eds() const {
   double v, num, den;

   if(fy_oy() == 0 || fy_oy() + fn_oy() == 0 || n() == 0) {
      v = bad_data_double;
   }
   else {
      num = log((double) (fy_oy() + fn_oy()) / n());
      den = log((double) fy_oy() / n());

      if(is_eq(den, 0.0)) v = bad_data_double;
      else                v = 2.0 * num / den - 1.0;
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::eds_ci(double alpha,
                                  double &cl, double &cu) const {
   double v, b, h, se;

   v = eds();
   b = baser();
   h = pod_yes();

   if(is_bad_data(v) || is_bad_data(b) || is_bad_data(h) ||
      is_eq(b, 0.0)  || is_eq(h, 0.0)) {
      cl = cu = bad_data_double;
      return(v);
   }

   //
   // Compute the standard error
   //
   se = 2.0 *
        fabs(log(b)) / (h * pow(log(b) + log(h), 2.0)) *
        sqrt(h * (1 - h) / (b * n()));

   compute_normal_ci(v, alpha, se, cl, cu);

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::seds() const {
   double v, num, den;

   if(fy_oy()           == 0           || n()     == 0 ||
      fy_oy() + fn_oy() == 0 || fy_oy() + fy_on() == 0) {
      v = bad_data_double;
   }
   else {
      num = log((double) (fy_oy() + fy_on()) / n()) +
            log((double) (fy_oy() + fn_oy()) / n());
      den = log((double) fy_oy() / n());

      if(is_eq(den, 0.0)) v = bad_data_double;
      else                v = num / den - 1.0;
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::seds_ci(double alpha,
                                   double &cl, double &cu) const {
   double v, b, h, se;

   v = seds();
   b = baser();
   h = pod_yes();

   if(is_bad_data(v) || is_bad_data(b) || is_bad_data(h) ||
      is_eq(b, 0.0)  || is_eq(h, 0.0)) {
      cl = cu = bad_data_double;
      return(v);
   }

   //
   // Compute the standard error
   //
   se = sqrt(h * (1.0 - h) / (n() * b)) *
        (-1.0 * log(fbias() * pow(b, 2.0)) /
        (h * pow(log(h * b), 2.0)));

   compute_normal_ci(v, alpha, se, cl, cu);

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::edi() const {
   double v, h, f, num, den;

   f = (double) fy_on() / (fy_on() + fn_on());
   h = pod_yes();

   if(is_bad_data(f) || is_bad_data(h) || is_eq(f, 0.0) || is_eq(h, 0.0)) {
      v = bad_data_double;
   }
   else {
      num = log(f) - log(h);
      den = log(f) + log(h);
      if(is_eq(den, 0.0)) v = bad_data_double;
      else                v = num/den;
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::edi_ci(double alpha,
                                  double &cl, double &cu) const {
   double v, h, f, b, se;

   v = edi();
   f = (double) fy_on() / (fy_on() + fn_on());
   h = pod_yes();
   b = baser();

   if(is_bad_data(f) || is_bad_data(h) || is_bad_data(b) ||
      is_eq(f, 0.0)  || is_eq(h, 0.0)  || is_eq(h, 1.0)) {
      cl = cu = bad_data_double;
      return(v);
   }

   //
   // Compute the standard error
   //
   se = 2.0 * fabs(log(f) + h / (1.0 - h) * log(h)) /
        (h * pow(log(f) + log(h), 2.0)) *
        sqrt(h * (1.0 - h) / (b * n()));

   compute_normal_ci(v, alpha, se, cl, cu);

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::sedi() const {
   double v, h, f, num, den;

   f = (double) fy_on() / (fy_on() + fn_on());
   h = pod_yes();

   if(is_bad_data(f) || is_bad_data(h) ||
      is_eq(f, 0.0)  || is_eq(h, 0.0)  ||
      is_eq(f, 1.0)  || is_eq(h, 1.0)) {
      v = bad_data_double;
   }
   else {
      num = (log(f) - log(h) - log(1 - f) + log(1 - h));
      den = (log(f) + log(h) + log(1 - f) + log(1 - h));
      if(is_eq(den, 0.0)) v = bad_data_double;
      else                v = num/den;
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////

double TTContingencyTable::sedi_ci(double alpha,
                                   double &cl, double &cu) const {
   double v, h, f, b, se, mf, mh;

   v = edi();
   f = (double) fy_on() / (fy_on() + fn_on());
   h = pod_yes();
   b = baser();

   if(is_bad_data(f) || is_bad_data(h) || is_bad_data(b) ||
      is_eq(f, 0.0)  || is_eq(h, 0.0)  ||
      is_eq(f, 1.0)  || is_eq(h, 1.0)) {
      cl = cu = bad_data_double;
      return(v);
   }

   //
   // Compute the standard error
   //
   mf = 1.0 - f;
   mh = 1.0 - h;

   se = 2.0 *
        fabs( (mh * mf + h * f) / (mh * mf) *
             log(f * mh) + 2.0 * h / mh * log(h * mf) ) /
        (h * pow(log(f * mh) + log(h * mf), 2.0)) *
        sqrt(h * mh / (b * n()));

   compute_normal_ci(v, alpha, se, cl, cu);

   return(v);
}

////////////////////////////////////////////////////////////////////////
//
//  Cost/Loss Relative Value
//  Reference: Eq. 8.75b, page 380 in Wilks, 3rd Ed. 2011
//
////////////////////////////////////////////////////////////////////////

double TTContingencyTable::cost_loss(double r) const {
   double num, den, h, m, f, b, v;

   // Total proportion of hits, misses, false alarms, and observations 
   h = (double) fy_oy() / n();
   m = (double) fn_oy() / n();
   f = (double) fy_on() / n();
   b = (double)    oy() / n();

   if(r < b) {
      num = (r * (h + f - 1)) + m;
      den =  r * (b - 1);
   }
   else {
      num = (r * (h + f)) + m - b;
      den =  b  * (r - 1);
   }

   if(is_eq(den, 0.0)) v = bad_data_double;
   else                v = num/den;

   return(v);
}

////////////////////////////////////////////////////////////////////////


   //
   //  Code for stats for arbitrary size contingency tables
   //


////////////////////////////////////////////////////////////////////////


double ContingencyTable::gaccuracy() const {
   double num, den, v;
   int i;

   if ( Nrows != Ncols )  {

      mlog << Error << "\nContingencyTable::gaccuracy() -> table not square!\n\n";

      exit ( 1 );

   }

   for(i=0, num=0.0; i<Nrows; i++) num += (double) entry(i, i);

   den = (double) total();

   if(is_eq(den, 0.0)) v = bad_data_double;
   else                v = num/den;

   return(v);
}


////////////////////////////////////////////////////////////////////////


double ContingencyTable::gheidke()const  //  Reference: Eq. 7.11, page 249 in Wilks, 1st Ed.

{

if ( Nrows != Ncols )  {

   mlog << Error << "\nContingencyTable::gheidke() -> table not square!\n\n";

   exit ( 1 );

}

const int N = total();

if ( N == 0 )  {

   mlog << Error << "\nContingencyTable::gheidke() -> table empty!\n\n";

   exit ( 1 );

}

const double DN = (double) N;
int j, k, m, n;
double num, denom, sum, ans;

   //
   //  first term in numerator
   //

sum = 0.0;

for (j=0; j<Nrows; ++j)  {

   n = rc_to_n(j, j);

   sum += ((*E)[n])/DN;

}

num = sum;

   //
   //  second term in numerator
   //
   //    (also second term in denominator)
   //

sum = 0.0;

for (j=0; j<Nrows; ++j)  {

   k = row_total(j);

   m = col_total(j);

   sum += (k/DN)*(m/DN);

}

num -= sum;

denom = 1.0 - sum;

   //
   //  result
   //

if (is_eq(denom, 0.0)) ans = bad_data_double;
else                   ans = num/denom;

   //
   //  done
   //

return ( ans );

}


////////////////////////////////////////////////////////////////////////


double ContingencyTable::gkuiper()const  //  Reference: Eq. 7.13, page 250 in Wilks, 1st Ed.

{

if ( Nrows != Ncols )  {

   mlog << Error << "\nContingencyTable::gkuiper() -> table not square!\n\n";

   exit ( 1 );

}

const int N = total();

if ( N == 0 )  {

   mlog << Error << "\nContingencyTable::gkuiper() -> table empty!\n\n";

   exit ( 1 );

}

const double DN = (double) N;
int j, k, m, n;
double num, denom, sum, t, ans;

   //
   //  first term in numerator
   //

sum = 0.0;

for (j=0; j<Nrows; ++j)  {

   n = rc_to_n(j, j);

   sum += ((*E)[n])/DN;

}

num = sum;

   //
   //  second term in numerator
   //

sum = 0.0;

for (j=0; j<Nrows; ++j)  {

   k = row_total(j);

   m = col_total(j);

   sum += (k/DN)*(m/DN);

}

num -= sum;

   //
   //  second term in denominator
   //

sum = 0.0;

for (j=0; j<Nrows; ++j)  {

   m = col_total(j);

   t = m/DN;

   sum += t*t;

}

denom = 1.0 - sum;

   //
   //  result
   //

if (is_eq(denom, 0.0)) ans = bad_data_double;
else                   ans = num/denom;

   //
   //  done
   //

return ( ans );

}


////////////////////////////////////////////////////////////////////////


double ContingencyTable::gerrity() const  //  Reference: Pages 84-91 in
                                          //  "Forecast Verification" by Jolliffe and Stephenson

{

if ( Nrows != Ncols )  {

   mlog << Error << "\nContingencyTable::gerrity() -> table not square!\n\n";

   exit ( 1 );

}

const int N = total();

if ( N == 0 )  {

   mlog << Error << "\nContingencyTable::gerrity() -> table empty!\n\n";

   exit ( 1 );

}

int j, k, m, n;
const double DN = (double) N;
double t, sum;
double * p = (double *) 0;
double * s  = (double *) 0;

   //
   //  can't compute gerrity when the first column contains all zeros
   //

if ( col_total(0) == 0 ) return ( bad_data_double );

p = new double [Nrows];

s = new double [Nrows*Nrows];

   //
   //  the p array
   //

for (j=0; j<Nrows; ++j)  {

   k = col_total(j);

   p[j] = k/DN;

}

   //
   //  scoring matrix
   //

calc_gerrity_scoring_matrix(Nrows, p, s);

   //
   //  calculate score
   //

sum = 0.0;

for (j=0; j<Nrows; ++j)  {

   for (k=0; k<Nrows; ++k)  {

      n = rc_to_n(j, k);

      m = (*E)[n];

      t = m/DN;

      sum += (s[n])*t;

   }

}

   //
   //  replace nan with bad data
   //

if (isnan(sum))  sum = bad_data_double;

   //
   //  done
   //

if ( p  )  { delete [] p;   p  = (double *) 0; }
if ( s  )  { delete [] s;   s  = (double *) 0; }

return ( sum );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void calc_gerrity_scoring_matrix(int N, const double * p, double * s)

{

int j, k, n;
double b, t, sum;
double * a          = (double *) 0;
double * recip_sum  = (double *) 0;
double * direct_sum = (double *) 0;


a          = new double [N];
recip_sum  = new double [N];
direct_sum = new double [N];

b = 1.0/(N - 1.0);


   //
   //  the a array
   //

sum = 0.0;

for (j=0; j<N; ++j)  {

   sum += p[j];

   a[j] = (1.0 - sum)/sum;

}

   //
   //  the recip_sum array
   //
   //    recip_sum[k] = sum(j=0 to k - 2) of 1/a[j]
   //
   //    note that this forces recip_sum[0] to be zero
   //

recip_sum[0] = 0.0;

sum = 0.0;

for (j=1; j<N; ++j)  {

   sum += 1.0/(a[j - 1]);

   recip_sum[j] = sum;

}

   //
   //  the direct_sum array
   //
   //    direct_sum[k] = sum(j=k to (N - 2)) of a[j]
   //
   //    note that this forces direct_sum[N - 1] to be zero
   //

direct_sum[N - 1] = 0.0;

sum = 0.0;

for (j=(N - 2); j>=0; --j)  {

   sum += a[j];

   direct_sum[j] = sum;

}

   //
   //  entries of the scoring matrix
   //

for (j=0; j<N; ++j)  {

   for (k=j; k<N; ++k)  {

      t = b*( recip_sum[j] - (k - j) + direct_sum[k] );

      n = j*N + k;   //  we don't need to worry about which way we load this matrix up, 'cuz it's symmetric

      s[n] = t;

      n = k*N + j;

      s[n] = t;

   }

}

   //
   //  done
   //

if ( a )           { delete [] a;           a          = (double *) 0; }
if ( recip_sum )   { delete [] recip_sum;   recip_sum  = (double *) 0; }
if ( direct_sum )  { delete [] direct_sum;  direct_sum = (double *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////
