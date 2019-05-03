// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include "gsl_statistics.h"

////////////////////////////////////////////////////////////////////////

double stats_lag1_autocorrelation(const NumArray &na) {
   double corr;
   double *src;
   int n, i;

   n   = na.n_elements();
   src = new double [n];

   for(i=0; i<n; i++) src[i] = na[i];

   corr = gsl_stats_lag1_autocorrelation(src, 1, n);

   if(src) { delete [] src; src = (double *) 0; }

   return(corr);
}

////////////////////////////////////////////////////////////////////////
