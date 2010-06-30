// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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


#include "vx_util/vx_util.h"
#include "vx_math/vx_math.h"


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


return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::extend(int n)

{

if ( Nalloc >= n )  return;

int k;

k = n/num_array_alloc_inc;

if ( n%num_array_alloc_inc )  ++k;

n = k*num_array_alloc_inc;

double * u = (double *) 0;

u = new double [n];

if ( !u )  {

   cerr << "\n\n  void NumArray::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

int j;

memset(u, 0, n*sizeof(double));

if ( e )  {

   for (j=0; j<Nelements; ++j)  {

      u[j] = e[j];

   }

   delete [] e;  e = (double *) 0;

}

e = u; u = (double *) 0;

Nalloc = n;


return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "Nelements = " << Nelements << "\n";
out << prefix << "Nalloc    = " << Nalloc    << "\n";

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


double NumArray::operator[](int n) const

{

if ( (n < 0) || (n >= Nelements) )  {

   cerr << "\n\n  NumArray::operator[](int) const -> "
       << "range check error\n\n";

   exit ( 1 );

}

return ( e[n] );

}


////////////////////////////////////////////////////////////////////////


int NumArray::has(int k) const

{

   return(has((double) k));

}


////////////////////////////////////////////////////////////////////////


int NumArray::is_bad_data(int n) const

{

int r;

if(abs(e[n] - bad_data_double) < 0.01) r = 1;
else                                   r = 0;

return ( r );

}


////////////////////////////////////////////////////////////////////////


int NumArray::has(double d) const

{

int j;

for (j=0; j<Nelements; ++j)  {

   if ( is_eq(e[j], d) )  return ( 1 );

}

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void NumArray::add(int k)

{

add((double) k);

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::add(double d)

{

extend(Nelements + 1);

e[Nelements++] = d;

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


return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::set(int n, int k)

{

set(n, (double) k);

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::set(int n, double d)

{

if ( (n < 0) || (n >= Nelements) )  {

   cerr << "\n\n  NumArray::set(int, double) -> range check error\n\n";

   exit ( 1 );

}

e[n] = d;

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::sort_array()

{

sort(e, Nelements);

return;

}


////////////////////////////////////////////////////////////////////////


void NumArray::reorder(const NumArray &i_na) {
   NumArray tmp_na;
   int i, j;

   // Check that the index array is of the correct length
   if(i_na.n_elements() != Nelements) {
      cerr << "\n\nNumArray::reorder(const NumArray &) -> "
           << "the index and sorting arrays must have the same length\n\n"
           << flush;
      exit(1);
   }

   // Store temporary copy of data array
   tmp_na = *this;
   clear();

   for(i=0; i<i_na.n_elements(); i++) {
      j = nint(i_na[i]) - 1;

      if(j<0 || j>=i_na.n_elements()) {
         cerr << "\n\nNumArray::reorder(const NumArray &) -> "
              << "index out of bounds: " << j << "\n\n"
              << flush;
         exit(1);
      }

      // Add the elements in the indexed order
      add(tmp_na[j]);
   }

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

int n, i;

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

   cerr << "\n\n  int NumArray::rank_array() -> memory allocation error\n\n";

   exit ( 1 );

}

//
// Search the data array for valid data and keep track of its location
//
n = 0;
for(i=0; i<Nelements; i++) {

   if(!is_bad_data(i)) {
      data[n]     = e[i];
      data_loc[n] = i;
      n++;
   }
}

//
// Compute the rank of the data and store the ranks in the data_rank array
// Keep track of the number of ties in the ranks.
//
ties = rank(data, data_rank, n);

//
// Store the data_rank values
//
for(i=0; i<n; i++) e[data_loc[i]] = data_rank[i];

//
// Deallocate memory
//
if(data)      { delete [] data;      data      = (double *) 0; }
if(data_loc)  { delete [] data_loc;  data_loc  = (int *) 0;    }
if(data_rank) { delete [] data_rank; data_rank = (double *) 0; }

return ( n );

}


////////////////////////////////////////////////////////////////////////


double NumArray::percentile_array(double t)

{

double v;

v = percentile(e, Nelements, t);

return ( v );

}


////////////////////////////////////////////////////////////////////////


void NumArray::compute_mean_stdev(double &mn, double &stdev) const

{

int j, count;
double s, s_sq;

if(Nelements == 0) {

   mn = stdev = bad_data_double;

   return;
}

s = s_sq = 0.0;
count = 0;

for(j=0; j<Nelements; j++) {
   if(is_bad_data(j)) continue;
   s    += e[j];
   s_sq += e[j]*e[j];
   count++;
}

mn  = s/count;

if(count > 1) {
   stdev = sqrt((s_sq - s*s/(double) count)/((double) (count - 1)));
}
else {
   stdev = bad_data_double;
}

return;

}


////////////////////////////////////////////////////////////////////////


double NumArray::mean() const

{

int j, count;
double s, mn;

for(j=0, count=0, s=0.0; j<Nelements; j++) {
   if(is_bad_data(j)) continue;
   s += e[j];
   count++;
}

if(count == 0) mn = bad_data_double;
else           mn = s/count;

return(mn);

}


////////////////////////////////////////////////////////////////////////


double NumArray::sum() const

{

int j, count;
double s;

for(j=0, count=0, s=0.0; j<Nelements; j++) {
   if(is_bad_data(j)) continue;
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
double v;
NumArray uniq_v, uniq_n;

for(j=0; j<Nelements; j++) {

   // If value isn't already in the list, add it
   if(!uniq_v.has(e[j])) {
      uniq_v.add(e[j]);
      uniq_n.add(1);
   }
   // Otherwise, increment the existing count
   else {

      for(k=0; k<uniq_v.n_elements(); k++) {
         if(uniq_v[k] == e[j]) {
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
      (uniq_n[j] == max_n && max_j >= 0 && uniq_v[j] < uniq_v[max_j])) {
      max_n = nint(uniq_n[j]);
      max_j = j;
   }
}

if(max_j >= 0) v = uniq_v[max_j];
else           v = bad_data_double;

return(v);

}

////////////////////////////////////////////////////////////////////////
