// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


static const int use_center = 1;


////////////////////////////////////////////////////////////////////////


using namespace std;


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


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Nx2ContingencyTable
   //


////////////////////////////////////////////////////////////////////////


Nx2ContingencyTable::Nx2ContingencyTable()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Nx2ContingencyTable::~Nx2ContingencyTable()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Nx2ContingencyTable::Nx2ContingencyTable(const Nx2ContingencyTable & t)

{

init_from_scratch();

assign(t);

}



////////////////////////////////////////////////////////////////////////


void Nx2ContingencyTable::init_from_scratch()

{

ContingencyTable::init_from_scratch();

Thresholds = (double *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Nx2ContingencyTable::clear()

{

ContingencyTable::clear();

if ( Thresholds )  { delete [] Thresholds;  Thresholds = (double *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


Nx2ContingencyTable & Nx2ContingencyTable::operator=(const Nx2ContingencyTable & t)

{

if ( this == &t )  return ( * this );

assign(t);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Nx2ContingencyTable::assign(const Nx2ContingencyTable & t)

{

clear();

ContingencyTable::assign(t);

if(t.Thresholds) set_thresholds(t.Thresholds);

return;

}


////////////////////////////////////////////////////////////////////////


int Nx2ContingencyTable::n() const

{

int k;

k = total();

return ( k );

}


////////////////////////////////////////////////////////////////////////


void Nx2ContingencyTable::set_size(int N)

{

ContingencyTable::set_size(N, 2);

return;

}


////////////////////////////////////////////////////////////////////////


void Nx2ContingencyTable::set_size(int NR, int NC)

{

if ( NC != 2 )  {

   mlog << Error << "\nNx2ContingencyTable::set_size(int, int) -> must have 2 columns!\n\n";

   exit ( 1 );

}

set_size(NR);

return;

}


////////////////////////////////////////////////////////////////////////


int Nx2ContingencyTable::value_to_row(double t) const

{

if ( !Thresholds )  {

   mlog << Error << "\nNx2ContingencyTable::value_to_row(double) const -> thresholds array not set!\n\n";

   exit ( 1 );

}

if ( t < Thresholds[0]     && !is_eq(t, Thresholds[0]) )      return ( -1 );

if ( t > Thresholds[Nrows] && !is_eq(t, Thresholds[Nrows]) )  return ( -1 );
                                               //  Thresholds array is of size Nrows + 1, so
                                               //  the last element has index Nrows, not Nrows - 1

int j;

for (j=0; j<Nrows; ++j)  {

   if ( ( t > Thresholds[j    ] ||  is_eq(t, Thresholds[j    ]) ) &&
        ( t < Thresholds[j + 1] && !is_eq(t, Thresholds[j + 1]) ) )  return ( j );

}

if ( is_eq(t, Thresholds[Nrows]) ) return ( Nrows - 1 );

return ( -1 );

}


////////////////////////////////////////////////////////////////////////


void Nx2ContingencyTable::set_thresholds(const double * Values)

{

if ( E->empty() )  {

   mlog << Error << "\nNx2ContingencyTable::set_thresholds(const double *) -> table empty!\n\n";

   exit ( 1 );

}

if ( Thresholds )  { delete [] Thresholds;  Thresholds = (double *) 0; }

Thresholds = new double [Nrows + 1];

memcpy(Thresholds, Values, (Nrows + 1)*sizeof(double));

return;

}


////////////////////////////////////////////////////////////////////////


double Nx2ContingencyTable::threshold(int k) const

{

if ( !Thresholds )  {

   mlog << Error << "\nNx2ContingencyTable::threshold(int) const -> no thresholds set!\n\n";

   exit ( 1 );

}

if ( (k < 0) || (k > Nrows) )  {   //  there are Nrows + 1 thresholds

   mlog << Error << "\nNx2ContingencyTable::threshold(int) const -> range check error\n\n";

   exit ( 1 );

}

return ( Thresholds[k] );

}


////////////////////////////////////////////////////////////////////////


void Nx2ContingencyTable::inc_event(double t)

{

int r;

r = value_to_row(t);

if ( r < 0 )  {

   mlog << Error << "\nNx2ContingencyTable::inc_event(double) -> bad value ... " << t << "\n\n";

   exit ( 1 );

}

inc_entry(r, nx2_event_column);

return;

}


////////////////////////////////////////////////////////////////////////


void Nx2ContingencyTable::inc_nonevent(double t)

{

int r;

r = value_to_row(t);

if ( r < 0 )  {

   mlog << Error << "\nNx2ContingencyTable::inc_nonevent(double) -> bad value ... " << t << "\n\n";

   exit ( 1 );

}

inc_entry(r, nx2_nonevent_column);

return;

}


////////////////////////////////////////////////////////////////////////


int Nx2ContingencyTable::event_count_by_thresh(double t) const

{

int r;

r = value_to_row(t);

if ( r < 0 )  {

   mlog << Error << "\nNx2ContingencyTable::event_count_by_thresh(double) -> bad value ... " << t << "\n\n";

   exit ( 1 );

}

int k;

k = entry(r, nx2_event_column);

return ( k );

}


////////////////////////////////////////////////////////////////////////


int Nx2ContingencyTable::nonevent_count_by_thresh(double t) const

{

int r;

r = value_to_row(t);

if ( r < 0 )  {

   mlog << Error << "\nNx2ContingencyTable::nonevent_count_by_thresh(double) -> bad value ... " << t << "\n\n";

   exit ( 1 );

}

int k;

k = entry(r, nx2_nonevent_column);

return ( k );

}


////////////////////////////////////////////////////////////////////////


double Nx2ContingencyTable::row_obar(int row) const

{

const int obs_count = event_count_by_row(row);
const int Ni = row_total(row);
double x;

if(Ni == 0) x = bad_data_double;
else        x = ((double) obs_count)/((double) Ni);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double Nx2ContingencyTable::obar() const

{

const int obs_count = event_col_total();
const int N = n();
double x;

if (N == 0) x = bad_data_double;
else        x = ((double) obs_count)/((double) N);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double Nx2ContingencyTable::row_proby(int row) const

{

if ( (row < 0) || (row >= Nrows) )  {

   mlog << Error << "\nNx2ContingencyTable::row_proby(int) const -> range check error\n\n";

   exit ( 1 );

}

double x;

if ( use_center ) x = 0.5*(Thresholds[row] + Thresholds[row + 1]);
else              x = Thresholds[row];

return ( x );

}


////////////////////////////////////////////////////////////////////////


double Nx2ContingencyTable::baser() const {

   return ( (double) event_col_total()/n() );
}


////////////////////////////////////////////////////////////////////////


double Nx2ContingencyTable::baser_ci(double alpha,
                                    double &cl, double &cu) const {
   double v;

   v = baser();

   compute_proportion_ci(v, n(), alpha, 1.0, cl, cu);

   return ( v );
}


////////////////////////////////////////////////////////////////////////


   //
   //  Reference: Equation 7.40, page 286 in Wilks, 2nd Ed.
   //

double Nx2ContingencyTable::reliability() const

{

int row;
double sum;
const int N = n();
int Ni;
double yi, obari, t;


sum = 0.0;

for (row=0; row<Nrows; ++row)  {

   Ni = row_total(row);

   yi = row_proby(row);

   obari = row_obar(row);

   // When obari is not defined, don't include it in the sum
   if(is_bad_data(obari)) continue;

   t = yi - obari;

   sum += Ni*t*t;

}

if (N == 0) sum  = bad_data_double;
else        sum /= N;

return ( sum );

}


////////////////////////////////////////////////////////////////////////


double Nx2ContingencyTable::resolution() const

{

int row, Ni;
const int N = n();
double sum, Obar, obari, t;


Obar = obar();

sum = 0.0;

for (row=0; row<Nrows; ++row)  {

   Ni = row_total(row);

   obari = row_obar(row);

   // When obari is not defined, don't include it in the sum
   if(is_bad_data(obari)) continue;

   t = obari - Obar;

   sum += Ni*t*t;

}

if (N == 0) sum  = bad_data_double;
else        sum /= N;

return ( sum );

}


////////////////////////////////////////////////////////////////////////


double Nx2ContingencyTable::uncertainty() const

{

double a, b;

a = obar();

if (is_bad_data(a)) b = bad_data_double;
else                b = a*(1.0 - a);

return ( b );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Reference: Equation 8.43, page 340 in Wilks, 3rd Ed.
   //

double Nx2ContingencyTable::bss_smpl() const

{

double res, rel, unc, bss;

res = resolution();
rel = reliability();
unc = uncertainty();

if (is_bad_data(res) || is_bad_data(rel) ||
    is_bad_data(unc) || is_eq(unc, 0.0))  {
   bss = bad_data_double;
}
else {
   bss = ( res - rel ) / unc;
}

return ( bss );

}


////////////////////////////////////////////////////////////////////////


double Nx2ContingencyTable::brier_score() const

{

const int N = n();

if ( N == 0 )  return ( bad_data_double );

int j, count;
double t, yi, sum;

sum = 0.0;

   //
   //  terms for event
   //

for (j=0; j<Nrows; ++j)  {

   count = event_count_by_row(j);

   yi = row_proby(j);

   t = yi - 1.0;

   sum += count*t*t;

}   //  for j

   //
   //  terms for nonevent
   //

for (j=0; j<Nrows; ++j)  {

   count = nonevent_count_by_row(j);

   yi = row_proby(j);

   t = yi;

   sum += count*t*t;

}   //  for j

   //
   //  that's all the terms
   //

if (N == 0) sum  = bad_data_double;
else        sum /= N;

return ( sum );

}


////////////////////////////////////////////////////////////////////////


double Nx2ContingencyTable::row_calibration(int row) const

{

if ( (row < 0) || (row >= Nrows) )  {

   mlog << Error << "\nNx2ContingencyTable::row_calibration(int) const -> range check error\n\n";

   exit ( 1 );

}

double num, denom;
double x;

num   = (double) event_count_by_row(row);

denom = num + nonevent_count_by_row(row);

if(is_eq(denom, 0.0)) x = bad_data_double;
else                  x = num/denom;

return ( x );

}


////////////////////////////////////////////////////////////////////////


double Nx2ContingencyTable::row_refinement(int row) const

{

if ( (row < 0) || (row >= Nrows) )  {

   mlog << Error << "\nNx2ContingencyTable::row_refinement(int) const -> range check error\n\n";

   exit ( 1 );

}

int py_o1, py_o2;
const int N = n();
double x;

py_o1 =    event_count_by_row(row);
py_o2 = nonevent_count_by_row(row);

x = (double) (py_o1 + py_o2);

if (N == 0) x  = bad_data_double;
else        x /= N;

return ( x );

}


////////////////////////////////////////////////////////////////////////


double Nx2ContingencyTable::row_event_likelihood(int row) const

{

if ( (row < 0) || (row >= Nrows) )  {

   mlog << Error << "\nNx2ContingencyTable::row_event_likelihood(int) const -> range check error\n\n";

   exit ( 1 );

}

double x, num, denom;

denom = (double) event_col_total();

num   = (double) event_count_by_row(row);

if(is_eq(denom, 0.0)) x = bad_data_double;
else                  x = num/denom;

return ( x );

}


////////////////////////////////////////////////////////////////////////


double Nx2ContingencyTable::row_nonevent_likelihood(int row) const

{

if ( (row < 0) || (row >= Nrows) )  {

   mlog << Error << "\nNx2ContingencyTable::row_nonevent_likelihood(int) const -> range check error\n\n";

   exit ( 1 );

}

double x, num, denom;

denom = (double) nonevent_col_total();

num   = (double) nonevent_count_by_row(row);

if(is_eq(denom, 0.0)) x = bad_data_double;
else                  x = num/denom;

return ( x );

}


////////////////////////////////////////////////////////////////////////


TTContingencyTable Nx2ContingencyTable::ctc_by_row(int row) const

{

TTContingencyTable tt;

if ( (row < 0) || (row >= Nrows) )  {

   mlog << Error << "\nNx2ContingencyTable::ctc_by_row(int) const -> range check error\n\n";

   exit ( 1 );

}

int j;
int sy, sn;

   ///////////////////

sy = sn = 0;

for (j=(row + 1); j<Nrows; ++j)  {

   sy +=    event_count_by_row(j);
   sn += nonevent_count_by_row(j);

}

tt.set_fy_oy(sy);
tt.set_fy_on(sn);

   ///////////////////

sy = sn = 0;

for (j=0; j<=row; ++j)  {

   sy +=    event_count_by_row(j);
   sn += nonevent_count_by_row(j);

}

tt.set_fn_oy(sy);
tt.set_fn_on(sn);

   ///////////////////

return ( tt );

}


////////////////////////////////////////////////////////////////////////


double Nx2ContingencyTable::roc_auc() const

{

int j;
TTContingencyTable ct;
double area, x_prev, y_prev, x, y;

x_prev = y_prev = 1.0;

for(j=0, area=bad_data_double; j<Nrows; ++j)  {

   // 2x2 Contingency Table for this row
   ct = ctc_by_row(j);

   // Retrieve the ROC point for this row
   x = ct.pofd();
   y = ct.pod_yes();

   if(is_bad_data(x) || is_bad_data(y)) continue;

   // Initialize area to 0 for the first valid point
   if(is_bad_data(area)) area = 0.0;

   // Compute area under the curve using the trapezoid rule
   area += (x_prev - x)*(y_prev + y)*0.5;

   // Save current ROC point as the previous point
   x_prev = x;
   y_prev = y;

}

return ( area );

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


double Nx2ContingencyTable::brier_ci_halfwidth(double alpha) const

{

int j;
double ee, ne;
double ob, degf, bs, t;
double term, var;
double af4, sf3, sf2, af1;
double halfwidth;
const double N = (double) n();
const double Ninv = 1.0/N;

// N must be > 1 so that degf > 0 in the call to gsl_cdf_tdist_Pinv()

if(is_bad_data(bs = brier_score()) || N <= 1)  return ( bad_data_double );

degf = N - 1.0;

t = gsl_cdf_tdist_Pinv(1.0 - 0.5*alpha, degf);

ob = obar();

af1 = sf2 = sf3 = af4 = 0.0;

for (j=0; j<Nrows; ++j)  {

   ee = Ninv*(   event_count_by_row(j));
   ne = Ninv*(nonevent_count_by_row(j));

   term = ee + ne;
   // term = row_proby(j);

   term *= term;   //  squared
   term *= term;   //  fourth power

   af4 += term;

   term = ee*ee;

   af1 += ee;

   sf2 += term;

   sf3 += term*ee;

}   //  for j

af4 *= Ninv;
af1 *= Ninv;

term = 1.0 - 4.0*sf3 + 6.0*sf2 + 4.0*af1;

var = ( af4 + ob*term - bs*bs )*Ninv;

halfwidth = t*sqrt(var);

return ( halfwidth );

}


////////////////////////////////////////////////////////////////////////


