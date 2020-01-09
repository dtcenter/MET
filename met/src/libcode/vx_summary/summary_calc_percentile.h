// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __SUMMARYCALCPERCENTILE_H__
#define  __SUMMARYCALCPERCENTILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "summary_calc.h"

////////////////////////////////////////////////////////////////////////


class SummaryCalcPercentile : public SummaryCalc
{

public:

  SummaryCalcPercentile(const string &type_string);
  virtual ~SummaryCalcPercentile();

  virtual string getType() const
  {
    return _type;
  }
  
  virtual double calcSummary(const NumArray &num_array) const
  {
    // Make a copy of the num_array so we don't change the original

    NumArray num_array_copy = num_array;
    
    return num_array_copy.percentile_array(_percentile);
  }

protected:

  double _percentile;
  string _type;
  
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __SUMMARYCALCPERCENTILE_H__  */


////////////////////////////////////////////////////////////////////////


