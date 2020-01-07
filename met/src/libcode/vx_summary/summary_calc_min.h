// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __SUMMARYCALCMIN_H__
#define  __SUMMARYCALCMIN_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "summary_calc.h"

////////////////////////////////////////////////////////////////////////


class SummaryCalcMin : public SummaryCalc
{

public:

  SummaryCalcMin();
  virtual ~SummaryCalcMin();

  virtual string getType() const
  {
    return "MIN";
  }
  
  virtual double calcSummary(const NumArray &num_array) const
  {
    return num_array.min();
  }
  
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __SUMMARYCALCMIN_H__  */


////////////////////////////////////////////////////////////////////////


