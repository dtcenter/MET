// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
#include "gsl/gsl_errno.h"

////////////////////////////////////////////////////////////////////////

double stats_lag1_autocorrelation(const NumArray &na) {
   double corr;
   double *src = (double *) 0;
   int n, i;

   n   = na.n_elements();
   src = new double [n];

   for(i=0; i<n; i++) src[i] = na[i];

   corr = gsl_stats_lag1_autocorrelation(src, 1, n);

   if(src) { delete [] src; src = (double *) 0; }

   return(corr);
}

////////////////////////////////////////////////////////////////////////

double sf_lambert_W0(const double x) {
   gsl_sf_result lw;
   double w;

   if(gsl_sf_lambert_W0_e(x, &lw) != GSL_SUCCESS) {
      mlog << Warning << "\nsf_lambert_W0() -> "
           << "call to gsl_sf_lambert_W0_e() for x = " << x 
           << " returned bad status!\n\n";
      w = bad_data_double;
   }
   else {
      w = lw.val;
   }

   return(w);
}

////////////////////////////////////////////////////////////////////////
