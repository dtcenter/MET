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
#include <errno.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "time_series_util.h"
#include "compute_swinging_door.h"

////////////////////////////////////////////////////////////////////////

TimeSeriesType string_to_timeseriestype(const char *s) {
   TimeSeriesType t;

        if(strcasecmp(s, timeseriestype_dydt_str)  == 0) t = TimeSeriesType_DyDt;
   else if(strcasecmp(s, timeseriestype_swing_str) == 0) t = TimeSeriesType_Swing;
   else                                                  t = TimeSeriesType_None;

   return(t);
}

////////////////////////////////////////////////////////////////////////

const char * timeseriestype_to_string(const TimeSeriesType t) {
   const char *s = (const char *) 0;

   switch(t) {
      case(TimeSeriesType_DyDt):  s = timeseriestype_dydt_str;  break;
      case(TimeSeriesType_Swing): s = timeseriestype_swing_str; break;
      default:                    s = na_str;                   break;
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////
//
// Apply ramp definition criteria to a time series of values and return
// an array of the same length using 0 and 1 to indicate the occurence
// of a ramp event.
//
// Assume the data values are sorted by time.
//
////////////////////////////////////////////////////////////////////////

bool compute_dydt_ramps(const char *name,
                        const NumArray &vals, const TimeArray &times,
                        const int step, const bool exact,
                        const SingleThresh &thresh,
                        NumArray &ramps, NumArray &prv) {
   int i, j;
   unixtime delta_ut;

   // Initialize
   ramps.clear();
   prv.clear();

   // Check threshold type for non-exact differences
   if(!exact &&
      thresh.get_type() != thresh_lt && thresh.get_type() != thresh_le &&
      thresh.get_type() != thresh_gt && thresh.get_type() != thresh_ge) {
      mlog << Error << "\ncompute_dydt_ramps() -> "
           << "for non-exact differences the ramp threshold ("
           << thresh.get_str() << ") must be of type <, <=, >, or >=.\n\n";
      return(false);
   }

   // Loop over the times
   for(i=0; i<times.n_elements(); i++) {

      // Initialize current ramp and previous values
      ramps.add(bad_data_double);
      prv.add(bad_data_double);

      // Search previous times
      for(j=0; j<i; j++) {

         // Compute time offset
         delta_ut = times[i] - times[j];

         // Check for an exact time difference.
         if(exact && delta_ut == step) prv.set(i, vals[j]);

         // Check all points in the time window
         if(!exact && delta_ut <= step) {

            // Initialize the previous value.
            if(is_bad_data(prv[i])) prv.set(i, vals[j]);

            // Update the previous value.
            if(!is_bad_data(prv[i]) && !is_bad_data(vals[j])) {

               // For greater-than type thresholds, find the minimum value.
               if((thresh.get_type() == thresh_gt ||
                   thresh.get_type() == thresh_ge) && vals[j] < prv[i]) {
                  prv.set(i, vals[j]);
               }

               // For less-than type thresholds, find the maximum value.
               if((thresh.get_type() == thresh_lt ||
                   thresh.get_type() == thresh_le) && vals[j] > prv[i]) {
                  prv.set(i, vals[j]);
               }
            }
         }
      } // end for j

      // Threshold difference to define ramp events
      if(!is_bad_data(vals[i]) && !is_bad_data(prv[i])) {
         if(thresh.check(vals[i] - prv[i])) ramps.set(i, 1);
         else                               ramps.set(i, 0);
      }

      // Print debug message when ramp event is found
      if(ramps[i] == 1) {
         mlog << Debug(4)
              << "Found " << name << " ramp event at "
              << unix_to_yyyymmdd_hhmmss(times[i])
              << (exact ? " exact " : " maximum " ) << sec_to_hhmmss(step)
              << " change " << prv[i] << " to " << vals[i] << ", "
              << vals[i] - prv[i] << thresh.get_str() << ".\n";
      }
   } // end for i

   return(true);
}

////////////////////////////////////////////////////////////////////////
//
// Apply the swinging door algorithm to a time series of values and
// return an array of the same length using 0 and 1 to indicate the
// occurence of a ramp event.
//
////////////////////////////////////////////////////////////////////////

bool compute_swing_ramps(const char *name,
                         const NumArray &vals, const TimeArray &times,
                         const double width, const SingleThresh &thresh,
                         NumArray &ramps, NumArray &slopes) {
   int i;
   bool ramp;

   // Initialize
   ramps.clear();
   slopes.clear();

   // Require at least 2 points
   if(times.n_elements() < 2) {
      for(i=0; i<times.n_elements(); i++) {
         ramps.add(bad_data_double);
         slopes.add(bad_data_double);
      }
      return(true);
   }

   mlog << Debug(4)
        << "Applying the swinging door algorithm.\n";

   if(!compute_swinging_door_slopes(times, vals, width, slopes)) {
      return(false);
   }
   
   // Apply the slope threshold to define ramps
   for(i=0; i<slopes.n_elements(); i++) {
      if(is_bad_data(slopes[i])) ramps.add(bad_data_double);
      else {
         ramp = thresh.check(slopes[i]);
         ramps.add((int) ramp);
      }

      // Print debug message when ramp event is found
      if(ramp) {
         mlog << Debug(4)
              << "Found " << name << " swinging door ramp event at "
              << unix_to_yyyymmdd_hhmmss(times[i])
              << " for slope of " << slopes[i]
              << thresh.get_str() << ".\n";
      }
      
   }
   
   return(true);
}

////////////////////////////////////////////////////////////////////////
