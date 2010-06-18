// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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

#include "vx_gsl_prob/gsl_statistics.h"

////////////////////////////////////////////////////////////////////////

double stats_lag1_autocorrelation(const NumArray &na) {
   double auto_corr;
   double *tmp;
   int i;

   tmp = new double [na.n_elements()];

   for(i=0; i<na.n_elements(); i++) tmp[i] = na[i];

   auto_corr = gsl_stats_lag1_autocorrelation(tmp, sizeof(double),
                                              na.n_elements());

   return(auto_corr);
}

////////////////////////////////////////////////////////////////////////
