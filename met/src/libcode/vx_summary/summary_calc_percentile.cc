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

#include "summary_calc_percentile.h"

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SummaryCalcPercentile
   //


////////////////////////////////////////////////////////////////////////


SummaryCalcPercentile::SummaryCalcPercentile(const string &type_string) :
  SummaryCalc()
{
  // Verify the type string.  The string must be of the format "p##".

  if (type_string.size() != 3 ||
      type_string[0] != 'p' ||
      !isdigit(type_string[1]) ||
      !isdigit(type_string[2]))
  {
    mlog << Error << "\nSummaryCalcPercentile::SummaryCalcPercentile() -> "
	 << "invalid percentile type \"" << type_string
	 << "\" specified in configuration file.\n\n";
    exit(1);
  }
  
  // Pull the desired percentile from the string

  _percentile = atof(type_string.substr(1,2).c_str()) / 100.0;
  
  // Construct the type string

  ConcatString type_buffer;
  type_buffer.format("P%02d", (int)((_percentile * 100.0) + 0.25));
  _type = type_buffer;
}

////////////////////////////////////////////////////////////////////////

SummaryCalcPercentile::~SummaryCalcPercentile()
{
}

////////////////////////////////////////////////////////////////////////
// Protected/Private Methods
////////////////////////////////////////////////////////////////////////
