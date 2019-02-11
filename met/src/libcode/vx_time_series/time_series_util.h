// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __TIME_SERIES_UTIL_H__
#define  __TIME_SERIES_UTIL_H__

////////////////////////////////////////////////////////////////////////

#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

//
// Enumeration for time-series analysis types
//

enum TimeSeriesType {
   TimeSeriesType_None,  // Default
   TimeSeriesType_DyDt,  // Threshold change over time window
   TimeSeriesType_Swing  // Apply swinging door algorithm
};

static const char timeseriestype_dydt_str[]  = "DYDT";
static const char timeseriestype_swing_str[] = "SWING";

////////////////////////////////////////////////////////////////////////

extern const char *   timeseriestype_to_string(const TimeSeriesType);
extern TimeSeriesType string_to_timeseriestype(const char *);

////////////////////////////////////////////////////////////////////////

extern bool compute_dydt_ramps(const char *name, const NumArray &vals,
                               const TimeArray &times,
                               const int step, const bool exact,
                               const SingleThresh &thresh,
                               NumArray &, NumArray &);

extern bool compute_swing_ramps(const char *name, const NumArray &vals,
                                const TimeArray &times,
                                const double width,
                                const SingleThresh &thresh,
                                NumArray &, NumArray &);

////////////////////////////////////////////////////////////////////////

#endif   // __TIME_SERIES_UTIL_H__

////////////////////////////////////////////////////////////////////////
