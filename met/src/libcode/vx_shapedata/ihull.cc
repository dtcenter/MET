

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "ihull.h"


////////////////////////////////////////////////////////////////////////


static const int right_turn = -1;


////////////////////////////////////////////////////////////////////////


static int lex_compare(const void *, const void *);


////////////////////////////////////////////////////////////////////////


void ihull(const IntPoint * in, const int n_in, IntPoint * hull, int & n_hull)

{

   //
   //  Note: "hull" has to be at least of size n_in + 1
   //

   //
   //  sanity check input values
   //

   //
   //  sort the values in lexocographic order
   //

int j, k;
int n_old;
IntPoint * p = new IntPoint [n_in];

for (j=0; j<n_in; ++j)  {

   p[j].x = in[j].x;
   p[j].y = in[j].y;

   p[j].used = false;

   p[j].orig_index = j;

}

qsort(p, n_in, sizeof(*p), lex_compare);


   //
   //  upper hull
   //

n_hull = 0;

j = 0;

// hull[n_hull++] = p[j++];
// hull[n_hull++] = p[j++];

while ( j < n_in )  {

   hull[n_hull++] = p[j++];

   while ( n_hull >= 3 )  {

      k = calc_turn(hull[n_hull - 3], hull[n_hull - 2], hull[n_hull - 1]);

      if ( k == right_turn )  break;

      hull[n_hull - 2] = hull[n_hull - 1];  --n_hull;

   }   //  while

}   //  while

for (k=0; k<n_hull; ++k)  p[hull[k].orig_index].used = true;

// for (k=0; k<n_hull; ++k)  cout << hull[k].x << ' ' << hull[k].y << "\n";
// cout << "\n\n" << flush;

n_old = n_hull;

   //
   //  lower hull
   //

j = n_in - 2;

while ( j >= 0 )  {

   // if ( p[j].used )  { --j;  continue; }   //  don't look at points that are part of the upper hull

   hull[n_hull++] = p[j--];   //  this is why the hull array has to be bigger than the input array

   while ( n_hull >= 3 )  {

      k = calc_turn(hull[n_hull - 3], hull[n_hull - 2], hull[n_hull - 1]);

      // cout << "k = " << k << '\n';

      if ( k == right_turn )  break;

      hull[n_hull - 2] = hull[n_hull - 1];  --n_hull;

   }   //  while

}

   //
   //  did we add any points with the lower hull?
   //

if ( n_hull > n_old )  --n_hull;



   //
   //  done
   //

if ( p )  { delete [] p;  p = 0; }

return;

}


////////////////////////////////////////////////////////////////////////


int lex_compare(const void * _a, const void * _b)

{

const IntPoint & a = *((const IntPoint *) _a);
const IntPoint & b = *((const IntPoint *) _b);

if ( a.x < b.x )  return ( -1 );
if ( a.x > b.x )  return (  1 );

   //
   //  now we know that a.x = b.x
   //

if ( a.y < b.y )  return ( -1 );
if ( a.y > b.y )  return (  1 );


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


