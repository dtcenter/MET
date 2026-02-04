// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
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
#include <vector>
#include <algorithm>
#include <cmath>

#include "ihull.h"

using namespace std;


////////////////////////////////////////////////////////////////////////


static const int right_turn = -1;


////////////////////////////////////////////////////////////////////////


static bool lex_compare(const IntPoint &, const IntPoint &);


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
vector<IntPoint> p(n_in);

for (j=0; j<n_in; ++j)  {

   p[j].x = in[j].x;
   p[j].y = in[j].y;

   p[j].used = false;

   p[j].orig_index = j;

}

sort(p.begin(), p.end(), lex_compare);

   //
   //  upper hull
   //

n_hull = 0;

j = 0;

while ( j < n_in )  {

   hull[n_hull++] = p[j++];

   while ( n_hull >= 3 )  {

      k = calc_turn(hull[n_hull - 3], hull[n_hull - 2], hull[n_hull - 1]);

      if ( k == right_turn )  break;

      hull[n_hull - 2] = hull[n_hull - 1];  --n_hull;

   }   //  while

}   //  while

for (k=0; k<n_hull; ++k)  p[hull[k].orig_index].used = true;

n_old = n_hull;

   //
   //  lower hull
   //

j = n_in - 2;

while ( j >= 0 )  {

   hull[n_hull++] = p[j--];   //  this is why the hull array has to be bigger than the input array

   while ( n_hull >= 3 )  {

      k = calc_turn(hull[n_hull - 3], hull[n_hull - 2], hull[n_hull - 1]);

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

return;

}


////////////////////////////////////////////////////////////////////////


static bool lex_compare(const IntPoint &a, const IntPoint &b)

{

     if ( a.x < b.x )  return true;
else if ( a.x > b.x )  return false;

   //
   //  now we know that a.x = b.x
   //

     if ( a.y < b.y )  return true;
else if ( a.y > b.y )  return false;

   //
   //  done
   //

return false;

}


////////////////////////////////////////////////////////////////////////

