// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __SUMMARYOBS_H__
#define  __SUMMARYOBS_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "config_constants.h"
#include "observation.h"
#include "summary_calc.h"
#include "time_summary_interval.h"
#include "util_constants.h"

////////////////////////////////////////////////////////////////////////

class SummaryObs
{

public:

   SummaryObs();
   virtual ~SummaryObs();

   bool summarizeObs(const TimeSummaryInfo &summary_info);

   bool addObservationObj(const Observation &obs);
   bool addObservation(const std::string &header_type, const std::string &station_id,
                       const time_t valid_time,
                       const double latitude, const double longitude,
                       const double elevation,
                       const std::string &quality_flag,
                       const int var_code, const double pressure_level_hpa,
                       const double height_m, const double value,
                       const std::string &var_name = "");
   std::vector< Observation > getObservations();
   std::vector< Observation > getSummaries();
   long countHeaders();
   long countHeaders(std::vector< Observation > *obs_vector);
   long countHeaders(std::vector< Observation > &obs_vector);
   long countSummaryHeaders();
   time_t getValidTime(const std::string &time_string) const;
   TimeSummaryInfo getSummaryInfo();
   void setSummaryInfo(const TimeSummaryInfo &summary_info);
   StringArray getObsNames();


protected:

  static const float FILL_VALUE;


  ///////////////////////
  // Protected members //
  ///////////////////////

  bool dataSummarized;
  TimeSummaryInfo summaryInfo;

  // List of observations read
  std::vector< Observation > observations;
  std::vector< Observation > summaries;
  StringArray obs_names;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Count the number of headers needed for the netCDF file.  All of the
  // observations must be loaded into the observations vector before calling
  // this method.

  bool addObservation(const Observation &obs);

  // Use the configuration information to generate the list of summary
  // calculators needed.

  std::vector< SummaryCalc* > getSummaryCalculators(const TimeSummaryInfo &info) const;

  // Use the configuration file time summary information to figure out the
  // time intervals for our summaries

  std::vector< TimeSummaryInterval > getTimeIntervals(const time_t first_data_time,
                    const time_t last_data_time,
                    const TimeSummaryInfo &info) const;

  // Check to see if the observation is in the list.
  bool isInObsList(const TimeSummaryInfo &summary_info,
                   const Observation &obs) const;

  // Check to see if the given time is within the defined time interval

  bool isInTimeInterval(const time_t curr_time,
          const int begin_secs, const int end_secs) const;

  // Generate the summary header type string for the netCDF file

  std::string getSummaryHeaderType(const std::string &header_type,
                const std::string &summary_type,
                const int summary_width_secs) const;

public:

  // Convert the unix time to number of seconds since the beginning of
  // the day

  static int unixtimeToSecs(const time_t unix_time)
  {
    struct tm *time_struct = gmtime(&unix_time);

    return (time_struct->tm_hour * 3600) +
      (time_struct->tm_min * 60) + time_struct->tm_sec;
  }

  // Convert the number of seconds from the beginning of the day to a string

  static std::string secsToTimeString(const int secs)
  {
    // Get the different fields from the number of seconds

    int remaining_secs = secs;
    int hour = remaining_secs / 3600;
    remaining_secs -= hour * 3600;
    int minute = remaining_secs / 60;
    remaining_secs -= minute * 60;
    int second = remaining_secs;

    // Create the string

    char string_buffer[tmp_buf_size];

    snprintf(string_buffer, sizeof(string_buffer), "%02d%02d%02d", hour, minute, second);

    return std::string(string_buffer);
  }

  // Get the first possible interval time after 0:00Z

  static time_t getFirstIntervalOfDay(const time_t test_time,
      const int begin_secs, const int end_secs, const int step)
  {
    struct tm *time_struct = gmtime(&test_time);

    int start_of_day_secs = 0;

    if (begin_secs < end_secs)
    {
      // This is the "normal" case.  We can just take the begin time and
      // start there.

      start_of_day_secs = begin_secs;

    }
    else
    {
      // If we get here, we have one of two cases.  If the start time and end
      // time are equal, we assume that the user wants to process the entire
      // day but base the time intervals off of the specified time.  If the
      // start time is greater than the end time, then the time period spans
      // midnight.  Either way, we want to take the specified start time and
      // the step and step back to the first time on the specified day.

      start_of_day_secs = begin_secs % step;
    }

    time_struct->tm_hour = start_of_day_secs / 3600;
    start_of_day_secs -= time_struct->tm_hour * 3600;

    time_struct->tm_min = start_of_day_secs / 60;
    start_of_day_secs -= time_struct->tm_min * 60;

    time_struct->tm_sec = start_of_day_secs;

    return timegm(time_struct);
  }

  // Get the interval time of the interval that contains the given data
  // time.

  static time_t getIntervalTime(const time_t test_time,
      const int begin_secs, const int end_secs, const int step,
      const int width_beg, const int width_end)
  {
    // Get the first interval time for the day

    time_t test_interval = getFirstIntervalOfDay(test_time,
                    begin_secs, end_secs,
                    step);

    // Loop through the day until we find the first interval that includes the
    // given time or is later than the given time (because the user could define
    // the intervals so that there are unprocessed times between them).

    while (test_time > (test_interval + width_end))
    {
      test_interval += step;
    }

    return test_interval;
  }

  static time_t getEndOfDay(const time_t unix_time)
  {
    struct tm *time_struct = gmtime(&unix_time);

    time_struct->tm_hour = 23;
    time_struct->tm_min = 59;
    time_struct->tm_sec = 59;

    return timegm(time_struct);
  }

  static std::string _timeToString(const time_t unix_time)
  {
    struct tm *time_struct = gmtime(&unix_time);

    char time_string[tmp_buf_size];

    snprintf(time_string, sizeof(time_string), 
             "%04d%02d%02d_%02d%02d%02d",
             time_struct->tm_year + 1900, time_struct->tm_mon + 1,
             time_struct->tm_mday,
             time_struct->tm_hour, time_struct->tm_min, time_struct->tm_sec);

    return time_string;
  }

  static time_t _stringToTime(const std::string &time_string)
  {
    struct tm time_struct;
    memset(&time_struct, 0, sizeof(time_struct));

    time_struct.tm_year = atoi(time_string.substr(0, 4).c_str()) - 1900;
    time_struct.tm_mon  = atoi(time_string.substr(4, 2).c_str()) - 1;
    time_struct.tm_mday = atoi(time_string.substr(6, 2).c_str());
    time_struct.tm_hour = atoi(time_string.substr(9, 2).c_str());
    time_struct.tm_min  = atoi(time_string.substr(11, 2).c_str());
    time_struct.tm_sec  = atoi(time_string.substr(13, 2).c_str());

    return timegm(&time_struct);
  }

};

inline std::vector< Observation > SummaryObs::getObservations() { return observations; }
inline std::vector< Observation > SummaryObs::getSummaries()    { return summaries;    }
inline StringArray           SummaryObs::getObsNames()     { return obs_names;    }
inline void                  SummaryObs::setSummaryInfo(const TimeSummaryInfo &summary_info) { summaryInfo = summary_info;};
inline TimeSummaryInfo       SummaryObs::getSummaryInfo()  { return summaryInfo;};

////////////////////////////////////////////////////////////////////////

#endif   /*  __SUMMARYOBS_H__  */

////////////////////////////////////////////////////////////////////////
