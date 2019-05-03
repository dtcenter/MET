// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

using namespace std;


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cmath>

#include "is_bad_data.h"


////////////////////////////////////////////////////////////////////////


   //
   // function to perform piecewise interpolation
   //

int pwl_interpolate(const double * y, const double * x, int n, double x_in, double & y_out)

{

int j;
double m;


y_out = 0.0;

if ( x_in < x[0] )  {   //  don't like this

   y_out = y[0];

   return ( 1 );

}

   //
   //  find bottom level for interpolation
   //

j = n - 1;

while ( (j >= 0) && (x[j] > x_in) )  --j;

   //
   //  range check
   //

if ( j < 0 )  { y_out = y[0];  return ( 0 ); }

if ( j >= (n - 1) )  { y_out = y[n - 1];   return ( 0 ); }

   //
   //  interpolate
   //

if ( is_eq(x[j + 1], x[j]) ) {

   y_out = y[j];

} else {

   m = (y[j + 1] - y[j])/(x[j + 1] - x[j]);

   y_out = y[j] + m*(x_in - x[j]);

}

   //
   //  done
   //

return ( 1 );

}


////////////////////////////////////////////////////////////////////////



