// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


#include "time_summary_interval.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class TimeSummaryInterval
   //


////////////////////////////////////////////////////////////////////////

TimeSummaryInterval::TimeSummaryInterval(const time_t base_time,
                                         const int width_beg_sec,
                                         const int width_end_sec) :
  _baseTime(base_time),
  _width_beg(width_beg_sec),
  _width_end(width_end_sec)
{
  _startTime = _baseTime + _width_beg;
  _endTime   = _baseTime + _width_end;
}

////////////////////////////////////////////////////////////////////////

TimeSummaryInterval::~TimeSummaryInterval()
{
  // Do nothing
}
