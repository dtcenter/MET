// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __SUMMARYCALCRANGE_H__
#define  __SUMMARYCALCRANGE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "summary_calc.h"

////////////////////////////////////////////////////////////////////////


class SummaryCalcRange : public SummaryCalc
{

public:

  SummaryCalcRange();
  virtual ~SummaryCalcRange();

  virtual string getType() const
  {
    return "RANGE";
  }
  
  virtual double calcSummary(const NumArray &num_array) const
  {
    return num_array.max() - num_array.min();
  }
  
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __SUMMARYCALCRANGE_H__  */


////////////////////////////////////////////////////////////////////////


