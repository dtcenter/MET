// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __SUMMARYCALCMEDIAN_H__
#define  __SUMMARYCALCMEDIAN_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "summary_calc_percentile.h"

////////////////////////////////////////////////////////////////////////


class SummaryCalcMedian : public SummaryCalcPercentile
{

public:

  SummaryCalcMedian();
  virtual ~SummaryCalcMedian();

protected:

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __SUMMARYCALCMEDIAN_H__  */


////////////////////////////////////////////////////////////////////////


