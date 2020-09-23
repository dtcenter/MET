// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __SUMMARYCALCMEAN_H__
#define  __SUMMARYCALCMEAN_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "summary_calc.h"

////////////////////////////////////////////////////////////////////////


class SummaryCalcMean : public SummaryCalc
{

public:

  SummaryCalcMean();
  virtual ~SummaryCalcMean();

  virtual string getType() const
  {
    return "MEAN";
  }
  
  virtual double calcSummary(const NumArray &num_array) const
  {
    return num_array.mean();
  }
  
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __SUMMARYCALCMEAN_H__  */


////////////////////////////////////////////////////////////////////////


