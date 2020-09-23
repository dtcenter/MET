// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <algorithm>
#include <iostream>
#include <map>

#include "vx_math.h"

#include "summary_calc_max.h"
#include "summary_calc_mean.h"
#include "summary_calc_median.h"
#include "summary_calc_min.h"
#include "summary_calc_percentile.h"
#include "summary_calc_range.h"
#include "summary_calc_stdev.h"
#include "summary_key.h"
#include "summary_obs.h"

////////////////////////////////////////////////////////////////////////

SummaryObs::SummaryObs():
  dataSummarized(false)
{
}

////////////////////////////////////////////////////////////////////////

SummaryObs::~SummaryObs()
{
  // Do nothing
}

////////////////////////////////////////////////////////////////////////

long SummaryObs::countHeaders()
{
   return countHeaders(&observations);
}

////////////////////////////////////////////////////////////////////////

long SummaryObs::countHeaders(vector< Observation > *obs_vector_ptr)
{
   long nhdr = 0;
   const string method_name = "SummaryObs::countHeaders(vector *)";
   Observation prev_obs = Observation(
         "dummy_type", "dummy_sid", -1, -90, -180, -100,
         na_str, 1, 0, 0, 0, "dummay");

   for (vector< Observation >::iterator obs = obs_vector_ptr->begin();
        obs != obs_vector_ptr->end(); ++obs)
   {
      if (!obs->hasSameHeader(prev_obs))
      {
        nhdr++;
        prev_obs = *obs;
      }
      obs->setHeaderIndex(nhdr-1);
   } /* endfor - obs */

   mlog << Debug(5) << "    " << method_name << "  size: "
        << (int)obs_vector_ptr->size() << " header count: " << nhdr << "\n";

   return nhdr;
}

////////////////////////////////////////////////////////////////////////

long SummaryObs::countHeaders(vector< Observation > &obs_vector)
{
   long nhdr = 0;
   const string method_name = "SummaryObs::countHeaders(vector &)";
   Observation prev_obs = Observation(
         "dummy_type", "dummy_sid", -1, -90, -180, -100,
         na_str, 1, 0, 0, 0, "dummay");

   for (vector< Observation >::iterator obs = obs_vector.begin();
        obs != obs_vector.end(); ++obs)
   {
      if (!obs->hasSameHeader(prev_obs))
      {
        nhdr++;
        prev_obs = *obs;
      }
      obs->setHeaderIndex(nhdr-1);
   } /* endfor - obs */

   mlog << Debug(5) << "    " << method_name << "  size: "
        << (int)obs_vector.size() << " header count: " << nhdr << "\n";

   return nhdr;
}

////////////////////////////////////////////////////////////////////////

long SummaryObs::countSummaryHeaders()
{
   long nhdr = 0;
   Observation prev_obs = Observation(
         "dummy_type", "dummy_sid", -1, -90, -180, -100,
         na_str, 1, 0, 0, 0, "dummay");
   const string method_name = "SummaryObs::countSummaryHeaders()";

   int obs_count = 0;
   for (vector< Observation >::iterator obs = summaries.begin();
        obs != summaries.end(); ++obs)
   {
      obs_count++;
      if (!obs->hasSameHeader(prev_obs))
      {
        nhdr++;
        prev_obs = *obs;
        mlog << Debug(9) << "    " << method_name << "  hdrIndex: "
             << nhdr << " at obs " << obs_count << "\n";
      }
      obs->setHeaderIndex(nhdr-1);
   } /* endfor - obs */

   return nhdr;
}

////////////////////////////////////////////////////////////////////////

bool SummaryObs::summarizeObs(const TimeSummaryInfo &summary_info)
{
   int summaryCount = 0;
   int summaryKeyCount = 0;
   // Save the summary information
   const char *var_name = 0;

   //_dataSummarized = true;
   TimeSummaryInfo inputSummaryInfo = summary_info;

   // Initialize the list of summary observations
   StringArray summary_vnames;
   StringArray printted_var_names;
   //vector< Observation > summary_obs;

   // Sort the observations.  This will put them in chronological order, with
   // secondary sorts of things like the station id
   sort(observations.begin(), observations.end());

   // Check for zero observations
   if (observations.size() == 0) return true;

   // Extract the desired time intervals from the summary information.
   // The vector will be in chronological order by start time, but could
   // overlap in time.
   vector< TimeSummaryInterval > time_intervals =
         getTimeIntervals(observations[0].getValidTime(),
                          observations[observations.size()-1].getValidTime(),
                          summary_info);

   // Get the summary calculators from the summary information.
   vector< SummaryCalc* > calculators = getSummaryCalculators(summary_info);

   // Get a pointer into the observations
   vector< Observation >::const_iterator curr_obs = observations.begin();

   // Loop through the time periods, processing the appropriate observations
   vector< TimeSummaryInterval >::const_iterator time_interval;
   for (time_interval = time_intervals.begin();
        time_interval != time_intervals.end(); ++time_interval)
   {
      mlog << Debug(3) << "Computing "
           << unix_to_yyyymmdd_hhmmss(time_interval->getBaseTime())
           << " time summary from "
           << unix_to_yyyymmdd_hhmmss(time_interval->getStartTime())
           << " to "
           << unix_to_yyyymmdd_hhmmss(time_interval->getEndTime())
           << ".\n";

      // Initialize the map used to sort observations in this time period
      // into their correct summary groups
      map< SummaryKey, NumArray* > summary_values;

      // Loop backwards through the observations to find the first observation
      // in the interval.  We need to do this because the user can define
      // overlapping intervals.
      while (curr_obs != observations.begin() &&
             curr_obs->getValidTime() > time_interval->getStartTime())
         --curr_obs;

      // At this point, we are either at the beginning of the observations list
      // or we are at the observation right before our current interval.  Process
      // observations until we get to the end of the interval.
      while (curr_obs != observations.end() &&
             curr_obs->getValidTime() < time_interval->getEndTime())
      {
        // We need to double-check that this observation is indeed within the
        // current time interval.  This takes care of the cases where there is
        // space between the time intervals and when we are first starting out.
        // It also allows us to go back one observation too far when looking for
        // the first observation in this time interval.

        if (!isInObsList(inputSummaryInfo, *curr_obs)) {
           if (!printted_var_names.has(curr_obs->getVarName().c_str())) {
              mlog << Debug(10)
                   << "SummaryObs::summarizeObs()  Filtered variable ["
                   << curr_obs->getVarName() << "] (id: " << curr_obs->getVarCode() << ")\n";
              printted_var_names.add(curr_obs->getVarName().c_str());
           }
        }
        else if (time_interval->isInInterval(curr_obs->getValidTime()))
        {
        // The summary key defines which observations should be grouped
        // together.  Any differences in key values indicates a different
        // summary.

           SummaryKey summary_key(curr_obs->getHeaderType(),
                                  curr_obs->getStationId(),
                                  curr_obs->getLatitude(),
                                  curr_obs->getLongitude(),
                                  curr_obs->getElevation(),
                                  curr_obs->getVarCode(),
                                  curr_obs->getHeight(),
                                  curr_obs->getPressureLevel(),
                                  curr_obs->getVarName());

           // Collect variable names
           var_name = curr_obs->getVarName().c_str();
           if (0 < strlen(var_name) && !summary_vnames.has(var_name)) {
              summary_vnames.add(var_name);
           }
           // If this is a new key, create a new NumArray
           if (summary_values.find(summary_key) == summary_values.end()) {
              summary_values[summary_key] = new NumArray;
              summaryKeyCount++;
           }

           // Add the observation to the correct summary
           summary_values[summary_key]->add(curr_obs->getValue());
         }

         // Move to the next obs
         ++curr_obs;
      }

      // Calculate the summaries and add them to the summary observations list

      map< SummaryKey, NumArray* >::const_iterator curr_values;
      for (curr_values = summary_values.begin();
           curr_values != summary_values.end(); ++curr_values)
      {
        // Loop through the calculators, saving a summary for each one
        vector< SummaryCalc* >::const_iterator calc_iter;
        for (calc_iter = calculators.begin();
             calc_iter != calculators.end(); ++calc_iter)
        {
           SummaryCalc *calc = *calc_iter;

           // Compute the expected number of observations and check valid data ratio
           if (summary_info.vld_freq > 0 && summary_info.vld_thresh > 0) {
              int n_expect = max(1, nint(summary_info.width / summary_info.vld_freq));
              int n_valid  = (*curr_values->second).n_valid();

              if (((double) n_valid / n_expect) < summary_info.vld_thresh) {
                 mlog << Debug(4)
                      << "Skipping time summary since the ratio of valid data "
                      << n_valid << "/" << n_expect << " < " << summary_info.vld_thresh
                      << " for " << curr_values->first.getHeaderType() << ", "
                      << calc->getType() << ", "
                      << summary_info.width << " seconds, "
                      << curr_values->first.getStationId() << ", "
                      << unix_to_yyyymmdd_hhmmss(time_interval->getBaseTime()) << ", "
                      << curr_values->first.getLatitude() << ", "
                      << curr_values->first.getLongitude() << ", "
                      << curr_values->first.getElevation() << ", "
                      << curr_values->first.getVarName() << ", "
                      << curr_values->first.getVarCode() << "\n";
                 continue;
              }
           }

           summaries.push_back(
                 Observation(
                       getSummaryHeaderType(curr_values->first.getHeaderType(),
                                            calc->getType(),
                                            summary_info.width),
                       curr_values->first.getStationId(),
                       time_interval->getBaseTime(),
                       curr_values->first.getLatitude(),
                       curr_values->first.getLongitude(),
                       curr_values->first.getElevation(),
                       "",
                       curr_values->first.getVarCode(),
                       curr_values->first.getPressureLevel(),
                       curr_values->first.getHeight(),
                       calc->calcSummary(*curr_values->second),
                       curr_values->first.getVarName()));
           summaryCount++;
        } /* endfor - calc */

      } /* endfor - curr_values */

      // Reclaim space for the summary arrays

      for (curr_values = summary_values.begin();
           curr_values != summary_values.end(); ++curr_values)
         delete curr_values->second;

   } /* endfor - time_interval */

   // Replace the observations vector with the summary observations

   //observations = summary_obs;
   for (int idx=0; idx<summary_vnames.n_elements(); idx++) {
      if (!obs_names.has(summary_vnames[idx])) obs_names.add(summary_vnames[idx]);
   }

   // Reclaim memory

   for (size_t i = 0; i < calculators.size(); ++i)
      delete calculators[i];

   mlog << Debug(3)
        << "SummaryObs::summarizeObs() summary key count: " << summaryKeyCount
        << "\tsummaryCount: " << summaryCount << "\n";

   return true;
}

////////////////////////////////////////////////////////////////////////

vector< SummaryCalc* > SummaryObs::getSummaryCalculators(const TimeSummaryInfo &info) const
{
   // Initialize the list of calculators
   vector< SummaryCalc * > calculators;

   // Loop through the summary types, creating the calculators
   for (int i = 0; i < info.type.n_elements(); ++i) {
      // Convert the current type to a string for easier processing

      string type = info.type[i];

      // Create the calculator specified

      if (type == "mean") {
        calculators.push_back(new SummaryCalcMean);
      }
      else if (type == "stdev") {
        calculators.push_back(new SummaryCalcStdev);
      }
      else if (type == "min") {
        calculators.push_back(new SummaryCalcMin);
      }
      else if (type == "max") {
        calculators.push_back(new SummaryCalcMax);
      }
      else if (type == "range") {
        calculators.push_back(new SummaryCalcRange);
      }
      else if (type == "median") {
        calculators.push_back(new SummaryCalcMedian);
      }
      else if (type[0] == 'p') {
        calculators.push_back(new SummaryCalcPercentile(type));
      }
   }

   return calculators;
}

////////////////////////////////////////////////////////////////////////

string SummaryObs::getSummaryHeaderType(const string &header_type,
      const string &summary_type,
      const int summary_width_secs) const
{
   // Extract the time values from the width

   char header_type_string[1024];

   snprintf(header_type_string, sizeof(header_type_string), "%s_%s_%s",
           header_type.c_str(), summary_type.c_str(),
           secsToTimeString(summary_width_secs).c_str());

   return string(header_type_string);
}

////////////////////////////////////////////////////////////////////////

vector< TimeSummaryInterval > SummaryObs::getTimeIntervals(
      const time_t first_data_time,
      const time_t last_data_time,
      const TimeSummaryInfo &info) const
{
   // Add the time intervals based on the relationship between the begin and
   // end times.

   vector< TimeSummaryInterval > time_intervals;
   time_t interval_time = getIntervalTime(first_data_time, info.beg, info.end,
                                          info.step, info.width_beg, info.width_end);
   while (interval_time < last_data_time) {
      // We need to process each day separately so that we can always start
      // at the indicated start time on each day.
      time_t day_end_time = getEndOfDay(interval_time);
      while (interval_time < day_end_time &&
             interval_time < last_data_time)
      {
         // See if the current time is within the defined time intervals
         if (isInTimeInterval(interval_time, info.beg, info.end)) {
           time_intervals.push_back(TimeSummaryInterval(interval_time, info.width_beg, info.width_end));
         }

         // Increment the current time
         interval_time += info.step;
      }
   }

   return time_intervals;
}

////////////////////////////////////////////////////////////////////////

time_t SummaryObs::getValidTime(const string &time_string) const
{
   struct tm time_struct;
   memset(&time_struct, 0, sizeof(time_struct));

   time_struct.tm_year = atoi(time_string.substr(0, 4).c_str()) - 1900;
   time_struct.tm_mon = atoi(time_string.substr(4, 2).c_str()) - 1;
   time_struct.tm_mday = atoi(time_string.substr(6, 2).c_str());
   time_struct.tm_hour = atoi(time_string.substr(9, 2).c_str());
   time_struct.tm_min = atoi(time_string.substr(11, 2).c_str());
   time_struct.tm_sec = atoi(time_string.substr(13, 2).c_str());

   return timegm(&time_struct);
}

////////////////////////////////////////////////////////////////////////

bool SummaryObs::isInObsList(const TimeSummaryInfo &summary_info,
                 const Observation &obs) const {
   return (0 == (summary_info.grib_code.n_elements()+summary_info.obs_var.n_elements())
       || summary_info.grib_code.has(obs.getVarCode())
       || summary_info.obs_var.has(obs.getVarName().c_str()));
}

////////////////////////////////////////////////////////////////////////

bool SummaryObs::isInTimeInterval(const time_t test_time,
      const int begin_secs, const int end_secs) const
{
   // If the begin and end times are the same, assume the user wants all times
   if (begin_secs == end_secs)
      return true;

   // Extract the seconds from the test time
   int test_secs = unixtimeToSecs(test_time);

   // Test for an interval that doesn't span midnight
   if (begin_secs < end_secs)
      return test_secs >= begin_secs && test_secs <= end_secs;

   // If we get here, the time interval spans midnight
   if (test_secs >= 0 && test_secs <= end_secs)
      return true;

   if (test_secs >= begin_secs && test_secs <= sec_per_day)
      return true;

   return false;
}

////////////////////////////////////////////////////////////////////////

bool SummaryObs::addObservationObj(const Observation &obs)
{
   bool result = false;

   // Do not filter by grib_code or obs_var here
   observations.push_back(obs);

   const ConcatString var_name = obs.getVarName();
   if (0 < var_name.length() && !obs_names.has(var_name)) {
      obs_names.add(var_name);
   }
   result = true;
   return result;
}

////////////////////////////////////////////////////////////////////////

bool SummaryObs::addObservation(
      const string &header_type, const string &station_id,
      const time_t valid_time,
      const double latitude, const double longitude,
      const double elevation,
      const string &quality_flag,
      const int var_code, const double pressure_level_hpa,
      const double height_m, const double value,
      const string &var_name)
{
   return addObservationObj(
             Observation(header_type, station_id, valid_time,
                         latitude, longitude, elevation,
                         quality_flag, var_code, pressure_level_hpa,
                         height_m, value, var_name));
}

////////////////////////////////////////////////////////////////////////
