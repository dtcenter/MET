// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __SUMMARYCALCSUM_H__
#define  __SUMMARYCALCSUM_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "summary_calc.h"

////////////////////////////////////////////////////////////////////////


class SummaryCalcSum : public SummaryCalc
{

public:

  SummaryCalcSum();
  virtual ~SummaryCalcSum();

  virtual std::string getType() const
  {
    return "SUM";
  }
  
  virtual double calcSummary(const NumArray &num_array) const
  {
    return num_array.sum();
  }
  
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __SUMMARYCALCSUM_H__  */


////////////////////////////////////////////////////////////////////////


