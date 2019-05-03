// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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
					 const int width_secs) :
  _baseTime(base_time),
  _width(width_secs)
{
  _startTime = _baseTime - (_width / 2);
  _endTime = _startTime + _width - 1;
}

////////////////////////////////////////////////////////////////////////

TimeSummaryInterval::~TimeSummaryInterval()
{
  // Do nothing
}
