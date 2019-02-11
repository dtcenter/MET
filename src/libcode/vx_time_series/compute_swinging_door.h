// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __COMPUTE_SWINGING_DOOR_H__
#define  __COMPUTE_SWINGING_DOOR_H__

////////////////////////////////////////////////////////////////////////

#include <utility>
#include <vector>
#include <ctime>

#include "vx_cal.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

class SDObservation
{
  

public:

  SDObservation() :
    _validTime(0), _value(0.0)
  {}

  SDObservation(const time_t valid_time,
                const double value) :
    _validTime(valid_time), _value(value)
  {}
  
  SDObservation(const string &valid_time_string,
                const double value) :
    _validTime(_getTime(valid_time_string)), _value(value)
  {}
  
  
  virtual ~SDObservation() {}
    
  
  ////////////////////
  // Access methods //
  ////////////////////

  time_t getValidTime() const
  {
    return _validTime;
  }
  
  string getValidTimeString() const
  {
    return _getTimeString(_validTime);
  }
  
  double getValue() const
  {
    return _value;
  }
  
  void setValidTime(const time_t valid_time)
  {
    _validTime = valid_time;
  }
  
  void setValue(const double value)
  {
    _value = value;
  }
  
  ///////////////
  // Operators //
  ///////////////

  bool operator< (const SDObservation &other) const
  {
    // Sort on valid time

    return _validTime < other._validTime;
  }
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  time_t _validTime;
  double _value;
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  static time_t _getTime(const string &time_string)
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
  
  static string _getTimeString(const time_t &unix_time)
  {
    struct tm *time_struct = gmtime(&unix_time);
    
    char time_string[80];
    
    sprintf(time_string, "%04d%02d%02d_%02d%02d%02d",
            time_struct->tm_year + 1900, time_struct->tm_mon + 1,
            time_struct->tm_mday,
            time_struct->tm_hour, time_struct->tm_min, time_struct->tm_sec);
  
    return string(time_string);
  }
};


////////////////////////////////////////////////////////////////////////

// Compute the swinging door slopes.
//
// Inputs:
//      valid_times - the array of valid times for the data values.
//      data_values - the actual data values.  There must be a data
//                    value for every given valid time.
//      error - the error value to use in the algorithm.
// Outputs:
//      slopes - the array of slopes.  There will be a slope value
//               for every given valid time.
// Return:
//      Returns true if successful, false otherwise.

extern bool compute_swinging_door_slopes(const TimeArray &valid_times,
                                         const NumArray &data_values,
                                         const double error,
                                         NumArray &slopes);

////////////////////////////////////////////////////////////////////////

// Compute the swinging door ramps.
//
// Inputs:
//      observations - the array of observations to use.
//      error - the error value to use in the algorithm.
// Outputs:
//      ramps - the array of calculated compressed observations.
// Return:
//      Returns true if successful, false otherwise.

extern bool compute_swinging_door_ramps(const vector< SDObservation > &observations,
                                        const double error,
                                        vector< pair< SDObservation, SDObservation > > &ramps);

////////////////////////////////////////////////////////////////////////

#endif   // __COMPUTE_SWINGING_DOOR_H__

////////////////////////////////////////////////////////////////////////
