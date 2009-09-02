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

#include "vx_analysis_util/box.h"
#include "vx_math/ptile.h"
#include "vx_util/vx_util.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Box
   //


////////////////////////////////////////////////////////////////////////


Box::Box()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Box::~Box()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Box::Box(const Box & b)

{

init_from_scratch();

assign(b);

}


////////////////////////////////////////////////////////////////////////


Box & Box::operator=(const Box & b)

{

if ( this == &b )  return ( * this );

assign(b);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Box::init_from_scratch()

{

Outlier = (double *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Box::clear()

{

if ( Outlier )  { delete [] Outlier;  Outlier = (double *) 0; }

N = 0;

N_outliers = 0;

P05 = P25 = P50 = P75 = P95 = 0.0;


return;

}


////////////////////////////////////////////////////////////////////////


void Box::assign(const Box & b)

{

clear();

P05 = b.P05;
P25 = b.P25;
P50 = b.P50;
P75 = b.P75;
P95 = b.P95;

N = b.N;

N_outliers = b.N_outliers;

if ( N_outliers > 0 )  {

   Outlier = new double [N_outliers];

   if ( !Outlier )  {

      cerr << "\n\n  Box::assign(const Box &) -> memory allocation error\n\n";

      exit ( 1 );

   }

   memcpy(Outlier, b.Outlier, N_outliers*sizeof(double));

}


return;

}


////////////////////////////////////////////////////////////////////////


void Box::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "N   = " <<   N << "\n";
out << prefix << "P05 = " << P05 << "\n";
out << prefix << "P25 = " << P25 << "\n";
out << prefix << "P50 = " << P50 << "\n";
out << prefix << "P75 = " << P75 << "\n";
out << prefix << "P95 = " << P95 << "\n";

out << prefix << "N_outliers = " << N_outliers << "\n";

int j;

for (j=0; j<N_outliers; ++j)  {

   out << prefix << "Outlier # " << j << " = " << (Outlier[j]) << "\n";

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void Box::set_unsorted(const double * un, int _n)

{

double * s = (double *) 0;

s = new double [_n];

if ( !s )  {

   cerr << "\n\n  Box::set_unsorted() -> memory allocation error\n\n";

   exit ( 1 );

}

memcpy(s, un, _n*sizeof(double));

sort(s, _n);


set_sorted(s, _n);



   //
   //  done
   //

if ( s )  { delete [] s;  s = (double *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void Box::set_sorted(const double * s, int _n)

{

int j, k;
double x;


N = _n;

P05 = percentile(s, N, 0.05);
P25 = percentile(s, N, 0.25);
P50 = percentile(s, N, 0.50);
P75 = percentile(s, N, 0.75);
P95 = percentile(s, N, 0.95);

N_outliers = 0;


for (j=0; j<N; ++j)  {

   if ( is_outlier(s[j]) )  ++N_outliers;

}

if ( N_outliers > 0 )  {

   Outlier = new double [N_outliers];

   if ( !Outlier )  {

      cerr << "\n\n  Box::set_sorted() -> memory allocation error\n\n";

      exit ( 1 );

   }

   k = 0;

   for (j=0; j<N; ++j)  {

      x = s[j];

      if ( is_outlier(x) )  Outlier[k++] = x;

   }

}

   //
   //  done
   //


return;

}


////////////////////////////////////////////////////////////////////////


double Box::outlier(int k) const

{

if ( (k < 0) || (k >= N_outliers) )  {

   cerr << "\n\n  Box::outlier(int) const -> range check error\n\n";

   exit ( 1 );

}

return ( Outlier[k] );

}


////////////////////////////////////////////////////////////////////////


