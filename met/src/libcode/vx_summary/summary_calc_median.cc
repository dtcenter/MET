// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <ctype.h>
#include <iostream>

#include "summary_calc_median.h"

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SummaryCalcMedian
   //


////////////////////////////////////////////////////////////////////////


SummaryCalcMedian::SummaryCalcMedian() :
  SummaryCalcPercentile("p50")
{
  _type = "MEDIAN";
}

////////////////////////////////////////////////////////////////////////

SummaryCalcMedian::~SummaryCalcMedian()
{
}

////////////////////////////////////////////////////////////////////////
// Protected/Private Methods
////////////////////////////////////////////////////////////////////////
