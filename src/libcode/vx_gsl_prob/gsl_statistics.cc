// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>

#include "gsl_statistics.h"
#include "gsl/gsl_errno.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

double stats_lag1_autocorrelation(const NumArray &na) {
   double corr;
   int n, i;

   n = na.n_elements();
   vector<double> src(n);

   for(i=0; i<n; i++) src[i] = na[i];

   corr = gsl_stats_lag1_autocorrelation(src.data(), 1, n);

   return corr;
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

   return w;
}

////////////////////////////////////////////////////////////////////////
