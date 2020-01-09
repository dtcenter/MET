// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __SUMMARYCALCMAX_H__
#define  __SUMMARYCALCMAX_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "summary_calc.h"

////////////////////////////////////////////////////////////////////////


class SummaryCalcMax : public SummaryCalc
{

public:

  SummaryCalcMax();
  virtual ~SummaryCalcMax();

  virtual string getType() const
  {
    return "MAX";
  }
  
  virtual double calcSummary(const NumArray &num_array) const
  {
    return num_array.max();
  }
  
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __SUMMARYCALCMAX_H__  */


////////////////////////////////////////////////////////////////////////


