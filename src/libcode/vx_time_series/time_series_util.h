// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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

void compute_ramps(const NumArray &vals, const TimeArray &times,
                   const int step, const bool exact,
                   const SingleThresh &thresh, NumArray &, NumArray &);

////////////////////////////////////////////////////////////////////////

#endif   // __TIME_SERIES_UTIL_H__

////////////////////////////////////////////////////////////////////////
