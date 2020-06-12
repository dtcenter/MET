// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>


#include "num_array.h"

#include "is_bad_data.h"
#include "ptile.h"
#include "nint.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class NumArray
   //


////////////////////////////////////////////////////////////////////////


NumArray::NumArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


NumArray::~NumArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


NumArray::NumArray(const NumArray & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


NumArray & NumArray::operator=(const NumArray & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void NumArray::init_from_scratch()

{

e = (double *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::clear()

{

if ( e )  { delete [] e;  e = (double *) 0; }

Nelements = Nalloc = 0;

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::erase()

{

Nelements = 0;

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::assign(const NumArray & a)

{

clear();

if ( a.Nelements == 0 )  return;

extend(a.Nelements);

int j;

for (j=0; j<(a.Nelements); ++j)  {

   e[j] = a.e[j];

}

Nelements = a.Nelements;

Sorted = a.Sorted;


return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::extend(int len, bool exact)

{

if ( Nalloc >= len )  return;

if ( ! exact )  {

   int k;

   k = len/num_array_alloc_inc;

   if ( len%num_array_alloc_inc )  ++k;

   len = k*num_array_alloc_inc;

}

double * u = (double *) 0;

u = new double [len];

if ( !u )  {

   mlog << Error << "\nvoid NumArray::extend(int, bool) -> "
        << "memory allocation error\n\n";

   exit ( 1 );

}

int j;

memset(u, 0, len*sizeof(double));

if ( e )  {

   for (j=0; j<Nelements; ++j)  {

      u[j] = e[j];

   }

   delete [] e;  e = (double *) 0;

}

e = u; u = (double *) 0;

Nalloc = len;


return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "Nelements = " << Nelements << "\n";
out << prefix << "Nalloc    = " << Nalloc    << "\n";
out << prefix << "Sorted    = " << (Sorted ? "true" : "false") << "\n";

int j;

for (j=0; j<Nelements; ++j)  {

   out << prefix << "Element # " << j << " = " << e[j] << "\n";

}

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


double NumArray::operator[](int i) const

{

if ( (i < 0) || (i >= Nelements) )  {

   mlog << Error << "\nNumArray::operator[](int) const -> "
       << "range check error ... Nelements = " << Nelements
       << ", i = " << i << "\n\n";

   exit ( 1 );

}

return ( e[i] );

}


////////////////////////////////////////////////////////////////////////


int NumArray::has(int k, bool forward) const

{

return(has((double) k, forward));

}


////////////////////////////////////////////////////////////////////////


int NumArray::has(double d, bool forward) const

{

int j;
int found = 0;

if (forward) {
   for (j=0; j<Nelements; ++j) {
      if ( is_eq(e[j], d) ) {
          found = 1;
          break;
      }
   }
}
else {
   for (j=Nelements-1; j>=0; --j) {
      if ( is_eq(e[j], d) ) {
          found = 1;
          break;
      }
   }
}

return ( found );

}


////////////////////////////////////////////////////////////////////////


void NumArray::add(int k)

{

add((double) k);

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::add(double d)

{

extend(Nelements + 1, false);

e[Nelements++] = d;

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::add(const NumArray & a)

{

extend(Nelements + a.Nelements);

int j;

for (j=0; j<(a.Nelements); ++j)  {

   e[Nelements++] = a.e[j];

}

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::add_const(double v, int n)

{

extend(Nelements + n);

int j;

for (j=0; j<n; ++j)  {

   e[Nelements++] = v;

}

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::add_seq(int beg, int end)

{

extend(Nelements + (end - beg + 1));

int j;

for (j=beg; j<=end; ++j)  {

   e[Nelements++] = j;

}

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::add_css(const char *text)

{

StringArray sa;

sa.parse_css(text);

extend(Nelements + sa.n_elements());

int j;

for (j=0; j<sa.n_elements(); j++)  {

  add(atof(sa[j].c_str()));

}

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::add_css_sec(const char *text)

{

StringArray sa;

sa.parse_css(text);

extend(Nelements + sa.n_elements());

int j;

for (j=0; j<sa.n_elements(); j++)  {

  add(timestring_to_sec(sa[j].c_str()));

}

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::set(int i, int k)

{

set(i, (double) k);

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::set(int i, double d)

{

if ( (i < 0) || (i >= Nelements) )  {

   mlog << Error << "\nNumArray::set(int, double) -> "
        << "range check error\n\n";

   exit ( 1 );

}

e[i] = d;

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::sort_array()

{

sort(e, Nelements);

Sorted = true;

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::reorder(const NumArray &i_na) {
   NumArray tmp_na;
   int i, j;

   // Check that the index array is of the correct length
   if(i_na.n_elements() != Nelements) {
      mlog << Error << "\nNumArray::reorder(const NumArray &) -> "
           << "the index and sorting arrays must have the same length\n\n";
      exit(1);
   }

   // Store temporary copy of data array
   tmp_na = *this;
   clear();

   for(i=0; i<i_na.n_elements(); i++) {
      j = nint(i_na[i]) - 1;

      if(j<0 || j>=i_na.n_elements()) {
         mlog << Error << "\nNumArray::reorder(const NumArray &) -> "
              << "index out of bounds: " << j << "\n\n";
         exit(1);
      }

      // Add the elements in the indexed order
      add(tmp_na[j]);
   }

   Sorted = false;

   return;
}


////////////////////////////////////////////////////////////////////////
//
// Compute the rank of the values in the array and return the number
// of valid data values that were ranked.
//
////////////////////////////////////////////////////////////////////////


int NumArray::rank_array(int &ties)

{

int n_vld, i;

double *data      = (double *) 0;
int    *data_loc  = (int *) 0;
double *data_rank = (double *) 0;

//
// Arrays to store the raw data values to be ranked, their locations,
// and their computed ranks.  The ranks are stored as doubles since
// they can be set to 0.5 in the case of ties.
//
data      = new double [Nelements];
data_loc  = new int    [Nelements];
data_rank = new double [Nelements];

if ( !data || !data_loc || !data_rank )  {

   mlog << Error << "\nint NumArray::rank_array() -> "
        << "memory allocation error\n\n";

   exit ( 1 );

}

//
// Search the data array for valid data and keep track of its location
//
for(i=0, n_vld=0; i<Nelements; i++) {

   if(!is_bad_data(e[i])) {
      data[n_vld]     = e[i];
      data_loc[n_vld] = i;
      n_vld++;
   }
}

//
// Compute the rank of the data and store the ranks in the data_rank array
// Keep track of the number of ties in the ranks.
//
ties = do_rank(data, data_rank, n_vld);

//
// Store the data_rank values
//
for(i=0; i<n_vld; i++) e[data_loc[i]] = data_rank[i];

//
// Deallocate memory
//
if(data)      { delete [] data;      data      = (double *) 0; }
if(data_loc)  { delete [] data_loc;  data_loc  = (int *) 0;    }
if(data_rank) { delete [] data_rank; data_rank = (double *) 0; }

Sorted = false;

return ( n_vld );

}


////////////////////////////////////////////////////////////////////////


double NumArray::percentile_array(double t)

{

double v;

//
// Ensure that the array is sorted before computing the percentile.
//
if ( !Sorted ) sort_array();

v = percentile(e, Nelements, t);

return ( v );

}


////////////////////////////////////////////////////////////////////////


double NumArray::compute_percentile(double v, bool inclusive) const

{

int i, n, nvld;
double ptile;

for ( i=0,n=0,nvld=0; i<Nelements; i++ )  {

   if ( is_bad_data(e[i]) )  continue;

   nvld++;

   if ( (  inclusive && e[i] <= v ) ||
        ( !inclusive && e[i] <  v ) )  n++;

}

if ( nvld == 0 )  ptile = bad_data_double;
else              ptile = (double) n / nvld;

return ( ptile );

}


////////////////////////////////////////////////////////////////////////


double NumArray::iqr()

{

double v, v1, v2;

v1 = percentile_array(0.75);
v2 = percentile_array(0.25);
v  = (is_bad_data(v1) || is_bad_data(v2) ? bad_data_double : v1 - v2);

return(v);

}


////////////////////////////////////////////////////////////////////////


void NumArray::compute_mean_variance(double &mn, double &var) const

{

int j, count;
double s, s_sq;

if(Nelements == 0) {

   mn = var = bad_data_double;

   return;
}

s = s_sq = 0.0;
count = 0;

for(j=0; j<Nelements; j++) {
   if(is_bad_data(e[j])) continue;
   s    += e[j];
   s_sq += e[j]*e[j];
   count++;
}

if(count == 0) mn = bad_data_double;
else           mn = s/count;

if(count > 1) {

   // Check for slightly negative precision error
   var = (s_sq - s*s/(double) count)/((double) (count - 1));
   if(is_eq(var, 0.0)) var = 0;

}
else {
   var = bad_data_double;
}

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::compute_mean_stdev(double &mn, double &stdev) const

{

double var;

compute_mean_variance(mn, var);

stdev = square_root(var);

return;

}


////////////////////////////////////////////////////////////////////////


double NumArray::sum() const

{

int j, count;
double s;

for(j=0, count=0, s=0.0; j<Nelements; j++) {
   if(is_bad_data(e[j])) continue;
   s += e[j];
   count++;
}

if(count == 0) s = bad_data_double;

return(s);

}


////////////////////////////////////////////////////////////////////////


double NumArray::mode() const

{

int j, k, max_n, max_j;
NumArray uniq_v, uniq_n;
double v;

for(j=0; j<Nelements; j++) {

   // If value isn't already in the list, add it
   if(!uniq_v.has(e[j])) {
      uniq_v.add(e[j]);
      uniq_n.add(1);
   }
   // Otherwise, increment the existing count
   else {

      for(k=0; k<uniq_v.n_elements(); k++) {
         if(is_eq(uniq_v[k], e[j])) {
            uniq_n.set(k, uniq_n[k] + 1);
            break;
         }
      }
   }
}

// Search uniq_v and uniq_n for the most common value
// Return the minimum of the most common values
for(j=0, max_n=0, max_j=-1; j<uniq_n.n_elements(); j++) {

   if((uniq_n[j] >  max_n) ||
      (is_eq(uniq_n[j], max_n) && max_j >= 0 && uniq_v[j] < uniq_v[max_j])) {
      max_n = nint(uniq_n[j]);
      max_j = j;
   }
}

if(max_j >= 0) v = uniq_v[max_j];
else           v = bad_data_double;

return(v);

}


////////////////////////////////////////////////////////////////////////


double NumArray::min() const

{

if(Nelements == 0) return(bad_data_double);

int j;

double min_v = e[0];

for(j=0; j<Nelements; j++) {
   if(is_bad_data(e[j])) continue;
   if(e[j] < min_v) min_v = e[j];
}

return(min_v);

}


////////////////////////////////////////////////////////////////////////


double NumArray::max() const

{

if(Nelements == 0) return(bad_data_double);

int j;

double max_v = e[0];

for(j=0; j<Nelements; j++) {
   if(is_bad_data(e[j])) continue;
   if(e[j] > max_v) max_v = e[j];
}

return(max_v);

}


////////////////////////////////////////////////////////////////////////


double NumArray::range() const

{

double v, v1, v2;

v1 = max();
v2 = min();
v  = (is_bad_data(v1) || is_bad_data(v2) ? bad_data_double : v1 - v2);

return(v);

}


////////////////////////////////////////////////////////////////////////


int NumArray::n_valid() const

{

int j, n_vld;

for(j=0, n_vld=0; j<Nelements; j++) {
   if(!is_bad_data(e[j])) n_vld++;
}

return(n_vld);

}


////////////////////////////////////////////////////////////////////////


ConcatString NumArray::serialize() const

{

ConcatString s;

if(Nelements == 0) return(s);

int j;

s << e[0];
for(j=1; j<Nelements; j++) s << " " << e[j];

return(s);

}


////////////////////////////////////////////////////////////////////////


NumArray NumArray::subset(int beg, int end) const

{

NumArray subset_na;

// Check bounds
if ( beg < 0 || beg >= Nelements ||
     end < 0 || end >= Nelements ||
     end < beg )  {
   mlog << Error << "\nNumArray::subset(int, int) -> "
        << "range check error\n\n";
   exit ( 1 );
}

// Store subset
for(int i=beg; i<=end; i++) subset_na.add(e[i]);

return ( subset_na );

}



////////////////////////////////////////////////////////////////////////


NumArray NumArray::subset(const NumArray &keep) const

{

NumArray subset_na;

// Check bounds
if ( keep.n_elements() != Nelements )  {
   mlog << Error << "\nNumArray::subset(const NumArray &) -> "
        << "the number of elements do not match\n\n";
   exit ( 1 );
}

// Store subset
for(int i=0; i<=Nelements; i++)  {
   if(keep[i])  subset_na.add(e[i]);
}

return ( subset_na );

}


////////////////////////////////////////////////////////////////////////


double NumArray::mean() const

{

int j, count;
double s, mn;

for(j=0, count=0, s=0.0; j<Nelements; j++) {
   if(is_bad_data(e[j])) continue;
   s += e[j];
   count++;
}

if(count == 0) mn = bad_data_double;
else           mn = s/count;

return(mn);

}


////////////////////////////////////////////////////////////////////////


double NumArray::mean_sqrt() const

{

NumArray wgt;

// for simple mean, call weighted mean with constant weight
wgt.add_const(1.0, Nelements);

return(wmean_sqrt(wgt));

}


////////////////////////////////////////////////////////////////////////


double NumArray::mean_fisher() const

{

NumArray wgt;

// for simple mean, call weighted mean with constant weight
wgt.add_const(1.0, Nelements);

return(wmean_fisher(wgt));

}


////////////////////////////////////////////////////////////////////////


double NumArray::wmean(const NumArray &wgt) const

{

if ( wgt.n_elements() != Nelements )  {
   mlog << Error << "\nNumArray::wmean(const NumArray &) -> "
        << "the number of elements do not match\n\n";
   exit ( 1 );
}

int j, count;
double w, s, wmn;

for(j=0, count=0, w=0.0, s=0.0; j<Nelements; j++) {
   if(is_bad_data(e[j]) || is_bad_data(wgt[j])) continue;
   s += wgt[j]*e[j];
   w += wgt[j];
   count++;
}

if(count == 0) wmn = bad_data_double;
else           wmn = s/w;

return(wmn);

}


////////////////////////////////////////////////////////////////////////


double NumArray::wmean_sqrt(const NumArray &wgt) const

{

int j;
NumArray squares;
double v;

// square the current values
squares.extend(Nelements);
for(j=0; j<Nelements; j++) {
   v = (is_bad_data(e[j]) ? bad_data_double : e[j]*e[j]);
   squares.add(v);
}

v = squares.wmean(wgt);

if(!is_bad_data(v)) v = sqrt(v);

return(v);

}


////////////////////////////////////////////////////////////////////////


double NumArray::wmean_fisher(const NumArray &wgt) const

{

int j;
NumArray xform;
double v;

// apply fisher transform
xform.extend(Nelements);
for(j=0; j<Nelements; j++) {
   v = (is_bad_data(e[j]) ? bad_data_double : atanh(e[j]));
   xform.add(v);
}

v = xform.wmean(wgt);

if(!is_bad_data(v)) v = tanh(v);

return(v);

}

////////////////////////////////////////////////////////////////////////

   //
   //  Utility Functions
   //

////////////////////////////////////////////////////////////////////////


ConcatString write_css(const NumArray &na)

{

ConcatString css;

for ( int i=0; i<na.n_elements(); ++i ) {
   css << (i == 0 ? "" : ",") << na[i];
}

return(css);

}


////////////////////////////////////////////////////////////////////////


ConcatString write_css_hhmmss(const NumArray &na)

{

ConcatString css;

for ( int i=0; i<na.n_elements(); ++i ) {
   css << (i == 0 ? "" : ",") << sec_to_hhmmss(na[i]);
}

return(css);

}


////////////////////////////////////////////////////////////////////////
