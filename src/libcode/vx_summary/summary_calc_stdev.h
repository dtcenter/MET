// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __SUMMARYCALCSTDEV_H__
#define  __SUMMARYCALCSTDEV_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "summary_calc.h"

////////////////////////////////////////////////////////////////////////


class SummaryCalcStdev : public SummaryCalc
{

public:

  SummaryCalcStdev();
  virtual ~SummaryCalcStdev();

  virtual string getType() const
  {
    return "SDEV";
  }
  
  virtual double calcSummary(const NumArray &num_array) const
  {
    double mean, stdev;
    num_array.compute_mean_stdev(mean, stdev);
    
    return stdev;
  }
  
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __SUMMARYCALCSTDEV_H__  */


////////////////////////////////////////////////////////////////////////


