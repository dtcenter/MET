// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "time_series_util.h"

////////////////////////////////////////////////////////////////////////
//
// Apply ramp definition criteria to a time series of values and return
// an array of the same length using 0 and 1 to indicate the occurance
// of a ramp event.
//
////////////////////////////////////////////////////////////////////////

bool compute_ramps(const NumArray &vals, const TimeArray &times,
                   const int step, const bool exact,
                   const SingleThresh &thresh, NumArray &ramps) {
   int i;

   ramps.clear();
   
   for(i=0; i<times.n_elements(); i++) ramps.add(0);

   return true;
}

////////////////////////////////////////////////////////////////////////
