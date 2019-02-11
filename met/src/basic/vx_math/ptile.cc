// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



///////////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <stdlib.h>
#include <cmath>


#include "ptile.h"
#include "is_bad_data.h"

#include "nint.h"


///////////////////////////////////////////////////////////////////////////////


static int compare_double (const void *, const void *);
static int compare_float  (const void *, const void *);

static int compare_rank(const void *, const void *);

static void reset_rank(double *, int, int, double);


///////////////////////////////////////////////////////////////////////////////


void sort(double *array, const int n)

{

if ( n <= 1 )  return;

qsort(array, n, sizeof(double), compare_double);

return;

}


///////////////////////////////////////////////////////////////////////////////


double percentile(const double *ordered_array, const int n, const double t)

{

int index;
double delta; 
double p = bad_data_double;

if ( n > 0 ) {
   index = nint(floor((n - 1)*t));
   delta = (n-1)*t - index;
   p = (1 - delta)*ordered_array[index] + delta*ordered_array[index+1];
}

return ( p );

}


///////////////////////////////////////////////////////////////////////////////


void sort_f(float * array, const int n)

{

if ( n <= 1 )  return;

qsort(array, n, sizeof(float), compare_float);

return;

}


///////////////////////////////////////////////////////////////////////////////


float percentile_f(const float * ordered_array, const int n, const double t)

{

int index;
float delta; 
float p = bad_data_float;

if ( n > 0 ) {

   index = nint(floor((n - 1)*t));

   delta = (n - 1)*t - index;

   p = (1 - delta)*(ordered_array[index]) + delta*(ordered_array[index + 1]);

}

return ( p );

}


///////////////////////////////////////////////////////////////////////////////


int compare_double(const void *p1, const void *p2)

{

const double *a = (const double *) p1;
const double *b = (const double *) p2;


if ( (*a) < (*b) )  return ( -1 );

if ( (*a) > (*b) )  return (  1 );


return ( 0 );

}


///////////////////////////////////////////////////////////////////////////////


int compare_float(const void * p1, const void * p2)

{

const float * a = (const float *) p1;
const float * b = (const float *) p2;


if ( (*a) < (*b) )  return ( -1 );

if ( (*a) > (*b) )  return (  1 );


return ( 0 );

}


///////////////////////////////////////////////////////////////////////////////
//
// Convert an array of doubles into a corresponding array of ranks for the
// data.  Return the number of rank ties that were encountered.  When ties in
// the raw data are encountered, replace the ranks for all of the tied data
// with the mean of the ranks.
//
///////////////////////////////////////////////////////////////////////////////


int do_rank(const double *array, double *rank, int n)

{

if ( n <= 1 )  return(0);

int i, j, ties_current, ties_total, tie_rank_start = 0, tie_rank_end;
double tie_rank_mean;
RankInfo *rank_info = (RankInfo *) 0;
double *ordered_array = (double *) 0;
double prev_v, v;

rank_info = new RankInfo [n];
ordered_array = new double [n];

// Each RankInfo structure contains a index value from 0 to n-1 and a pointer
// to the data to be ranked
for(i=0; i<n; i++) {
   rank_info[i].index = i;
   rank_info[i].data = array;
}

// Sort the ranks in the RankInfo structures by comparing the data
// values rather than the indices themselves
qsort(rank_info, n, sizeof(RankInfo), compare_rank);

// Compute and store the inverse permutation of the ranks computed
for(i=0; i<n; i++) rank[rank_info[i].index] = i+1;

// Use the rank just computed to create an ordered version of the data
for(i=0; i<n; i++) { ordered_array[nint(rank[i] - 1)] = array[i]; }

// Search through the ordered array looking for ties.  When ties are found
// replace the corresponding ranks with the mean of the ranks.
prev_v = -1.0e30;
ties_current = ties_total = 0;

for(i=0; i<n; i++) {

   v = ordered_array[i];

   // Check for a tie with the previous data value
   if(is_eq(v, prev_v)) {

      // Check if this is the beginning of a run of ties
      if(ties_current == 0) {
        // Save the previous rank as the beginning of the run (i, not i+1)
        tie_rank_start = i;
      }

      ties_current++;
      ties_total++;
   } // v != prev_v
   else {

      // Check if there was a run of ties that just ended
      if(ties_current != 0) {
         tie_rank_end = i;

         // Compute the mean of the tied ranks
         tie_rank_mean = (tie_rank_start + tie_rank_end)/2.0;

         // For each rank between tie_rank_start and tie_rank_end,
         // replace it with tie_rank_mean
         for(j=tie_rank_start; j<=tie_rank_end; j++) {
            reset_rank(rank, n, j, tie_rank_mean);
         }

         // Reset ties_current to zero
         ties_current = 0;
      }
   }
   prev_v = v;
}

// Check the end of the loop for any remaining ties
if(ties_current != 0) {
   tie_rank_end = n;

   // Compute the mean of the tied ranks
   tie_rank_mean = (tie_rank_start + tie_rank_end)/2.0;

   // For each rank between tie_rank_start and tie_rank_end,
   // replace it with tie_rank_mean
   for(j=tie_rank_start; j<=tie_rank_end; j++) {
      reset_rank(rank, n, j, tie_rank_mean);
   }

   // Reset ties_current to zero
   ties_current = 0;
}

if(rank_info)     { delete [] rank_info;     rank_info = (RankInfo *) 0; }
if(ordered_array) { delete [] ordered_array; ordered_array = (double *) 0; }

return(ties_total);

}


///////////////////////////////////////////////////////////////////////////////


int compare_rank(const void *p1, const void *p2)

{

const RankInfo a = *((RankInfo *) p1);
const RankInfo b = *((RankInfo *) p2);

if ( a.data[a.index] < b.data[b.index] )  return ( -1 );

if ( a.data[a.index] > b.data[b.index] )  return (  1 );

return ( 0 );

}


///////////////////////////////////////////////////////////////////////////////

static void reset_rank(double *rank, int n, int old_rank, double new_rank)

{
int i;

for(i=0; i<n; i++) {
   if(is_eq(rank[i], old_rank)) rank[i] = new_rank;
}

return;

}

///////////////////////////////////////////////////////////////////////////////
