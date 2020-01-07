// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __SUMMARYCALC_H__
#define  __SUMMARYCALC_H__


////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <vx_util.h>

////////////////////////////////////////////////////////////////////////


class SummaryCalc
{

public:

  SummaryCalc();
  virtual ~SummaryCalc();

  virtual string getType() const = 0;
  virtual double calcSummary(const NumArray &num_array) const = 0;
  
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __SUMMARYCALC_H__  */


////////////////////////////////////////////////////////////////////////


