// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __TIMESUMMARYINTERVAL_H__
#define  __TIMESUMMARYINTERVAL_H__


////////////////////////////////////////////////////////////////////////

#include <string>


////////////////////////////////////////////////////////////////////////


class TimeSummaryInterval
{

public:

  TimeSummaryInterval(const time_t base_time,
                      const int width_beg_sec, const int width_end_sec);

  virtual ~TimeSummaryInterval();


  ////////////////////
  // Access methods //
  ////////////////////

  time_t getBaseTime() const
  {
    return _baseTime;
  }

  time_t getStartTime() const
  {
    return _startTime;
  }

  time_t getEndTime() const
  {
    return _endTime;
  }

  bool isInInterval(const time_t test_time) const
  {
    if (test_time >= _startTime && test_time <= _endTime)
      return true;

    return false;
  }

  ///////////////
  // Operators //
  ///////////////

  bool operator< (const TimeSummaryInterval &other) const
  {
    // Sort on the base time

    return _baseTime < other._baseTime;
  }

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  time_t _baseTime;
  int _width_beg;
  int _width_end;

  time_t _startTime;
  time_t _endTime;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __TIMESUMMARYINTERVAL_H__  */


////////////////////////////////////////////////////////////////////////


