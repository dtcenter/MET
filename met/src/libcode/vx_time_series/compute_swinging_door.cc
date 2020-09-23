// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

#include "compute_swinging_door.h"

#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

bool compute_swinging_door_slopes(const TimeArray &valid_times,
                                  const NumArray &data_values,
                                  const double error,
                                  NumArray &slopes)
{
  // Make sure that the valid times and data arrays are consistent.

  if (valid_times.n_elements() != data_values.n_elements())
  {
    return false;
  }

  // Put the observations into an SDObservation array so we can
  // calculate the ramps.

  vector< SDObservation > obs;
  for (int i = 0; i < valid_times.n_elements(); ++i)
    obs.push_back(SDObservation((time_t)valid_times[i], data_values[i]));

  // Calculate the ramps

  vector< pair< SDObservation, SDObservation > > ramps;
  if (!compute_swinging_door_ramps(obs, error, ramps)) return false;

  // Convert the ramps to slope values

  slopes.erase();

  for (int i = 0; i < valid_times.n_elements(); ++i)
  {
    // Check for missing data

    if (is_bad_data(data_values[i]))
    {
      slopes.add(bad_data_double);
      continue;
    }

    // Loop through the ramps and compute slope for current time

    vector< pair< SDObservation, SDObservation > >::const_iterator ramp;
    for (ramp = ramps.begin(); ramp != ramps.end(); ++ramp)
    {

      // For the first point, use slope of the first ramp.
      // Otherwise, the end point of the ramp is assigned the previous slope.

      if ( (ramp           == ramps.begin()                &&
            valid_times[i] == ramp->first.getValidTime())  ||
           (valid_times[i] >  ramp->first.getValidTime()   &&
            valid_times[i] <= ramp->second.getValidTime()) )
      {
         int    run_secs = ramp->second.getValidTime() - ramp->first.getValidTime();
         double rise     = ramp->second.getValue() - ramp->first.getValue();
         double slope    = (run_secs == 0 ? bad_data_double : rise / (double)run_secs);
         slopes.add(slope);
         break;
      }
    } // end for ramp
  } // end for i

  // NOTE: Use bad_data_double for missing data values.  Check for this
  // in data_values and return it in slopes.

  return true;
}

////////////////////////////////////////////////////////////////////////

bool compute_swinging_door_ramps(const vector< SDObservation > &observations,
                                 const double error,
                                 vector< pair< SDObservation, SDObservation > > &ramps)
{
  // NOTE: Use bad_data_double for missing data values.  Check for this
  // in data_values and return it in slopes.  Can use is_bad_data(value)
  // which returns 1 for bad, 0 for good.

  // Check for empty list.

  if (observations.size() == 0) return true;

  // Save the information for the first point

  vector< SDObservation >::const_iterator curr_obs = observations.begin();

  time_t start_time = curr_obs->getValidTime();
  double start_value = curr_obs->getValue();
  double start_top_value = start_value + error;
  double start_bottom_value = start_value - error;

  double top_slope = -99999;
  double bottom_slope = 99999;

  SDObservation ramp_start = *curr_obs;

  // Loop through the rest of the observations

  time_t end_time  = start_time;
  double end_value = start_value;

  for (++curr_obs; curr_obs != observations.end(); ++curr_obs)
  {
    // Calculate the needed values for this point

    end_time  = curr_obs->getValidTime();
    end_value = curr_obs->getValue();

    double time_diff = (double)(end_time - start_time);

    double curr_top_slope = (end_value - start_top_value) / time_diff;
    double curr_bottom_slope = (end_value - start_bottom_value) / time_diff;

    if (top_slope < curr_top_slope)       top_slope    = curr_top_slope;
    if (bottom_slope > curr_bottom_slope) bottom_slope = curr_bottom_slope;

    // See if we are at the end of the current corridor

    if (top_slope > bottom_slope)
    {
      // Calculate the end point of this ramp

      double slope = (top_slope + bottom_slope) / 2.0;
      double end_ramp_value = start_value + slope * time_diff;

      // Save the end point of the previous corridor

      SDObservation ramp_end(end_time, end_ramp_value);
      ramps.push_back(pair< SDObservation, SDObservation >(ramp_start, ramp_end));

      // Save the values for the next corridor

      start_time = curr_obs->getValidTime();
      start_value = curr_obs->getValue();
      start_top_value = start_value + error;
      start_bottom_value = start_value - error;

      ramp_start = *curr_obs;
      ramp_start.setValue(start_value);

      top_slope = -99999;
      bottom_slope = 99999;
    }
  }

  // Save the last observation to finish off the last corridor.
  // NOTE: I should probably change this to get the middle of the final
  // doors instead.

  SDObservation ramp_end(end_time, end_value);
  ramps.push_back(pair< SDObservation, SDObservation >(ramp_start, ramp_end));

  return true;
}

////////////////////////////////////////////////////////////////////////
