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
// Assume the time-series is sorted.
//
////////////////////////////////////////////////////////////////////////

NumArray compute_ramps(const NumArray &vals, const TimeArray &times,
                       const int step, const bool exact,
                       const SingleThresh &thresh) {
   int i, j;
   unixtime delta_ut;
   double cur, prv;
   NumArray ramps;

   // Check threshold type for non-exact differences
   if(!exact &&
      thresh.get_type() != thresh_lt && thresh.get_type() != thresh_le &&
      thresh.get_type() != thresh_gt && thresh.get_type() != thresh_ge) {
      mlog << Error << "\ncompute_ramps() -> "
           << "for non-exact differences the ramp threshold ("
           << thresh.get_str() << ") must be of type <, <=, >, or >=.\n\n";
      exit(1);
   }

   // Loop over the times
   for(i=0; i<times.n_elements(); i++) {

      // Initialize current ramp value
      ramps.add(bad_data_double);

      // Store the current wind speed maximum
      cur = vals[i];
      prv = bad_data_double;

      // Search previous times
      for(j=0; j<i; j++) {

         // Compute time offset
         delta_ut = times[i] - times[j];

         // Check for an exact time difference.
         if(exact && delta_ut == step) prv = vals[j];

         // Check all points in the time window
         if(!exact && delta_ut <= step) {

            // Initialize the previous value.
            if(is_bad_data(prv)) prv = vals[j];

            // Update the previous value.
            if(!is_bad_data(prv) && !is_bad_data(vals[j])) {

               // For greater-than type thresholds, find the minimum value.
               if((thresh.get_type() == thresh_gt ||
                   thresh.get_type() == thresh_ge) && vals[j] < prv) {
                  prv = vals[j];
               }

               // For less-than type thresholds, find the maximum value.
               if((thresh.get_type() == thresh_lt ||
                   thresh.get_type() == thresh_le) && vals[j] > prv) {
                  prv = vals[j];
               }
            }
         }
      } // end for j

      // Threshold difference to define ramp events
      if(!is_bad_data(cur) && !is_bad_data(prv)) {
         if(thresh.check(cur - prv)) ramps.set(i, 1);
         else                        ramps.set(i, 0);
      }

      // Print debug message when ramp event is found
      if(ramps[i] == 1) {
         mlog << Debug(4)
              << "Found ramp event at " << unix_to_yyyymmdd_hhmmss(times[i])
              << (exact ? " exact " : " maximum " ) << sec_to_hhmmss(step)
              << " change " << prv << " to " << cur << ", "
              << cur - prv << thresh.get_str() << "\n";
      }
   } // end for i

   return(ramps);
}

////////////////////////////////////////////////////////////////////////
