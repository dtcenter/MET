// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

#include "vx_analysis_util/time_series.h"
#include "vx_util/vx_util.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class TimeSeries
   //


////////////////////////////////////////////////////////////////////////


TimeSeries::TimeSeries()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


TimeSeries::~TimeSeries()

{

clear();

}


////////////////////////////////////////////////////////////////////////


TimeSeries::TimeSeries(const TimeSeries & ts)

{

init_from_scratch();

assign(ts);

}


////////////////////////////////////////////////////////////////////////


TimeSeries & TimeSeries::operator=(const TimeSeries & ts)

{

if ( this == &ts )  return ( * this );

assign(ts);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void TimeSeries::init_from_scratch()

{

Value = (double *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void TimeSeries::clear()

{

if ( Value )  { delete [] Value;  Value = (double *) 0; }

Nelements = 0;

TimeStart = (unixtime) 0;

TimeDelta = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void TimeSeries::dump(ostream & out, int depth) const

{

Indent prefix(depth);
char junk[256];


out << prefix << "Nelements = " << Nelements << "\n";

if ( Nelements == 0 )  { out.flush();  return; }

int month, day, year, hour, minute, second;

unix_to_mdyhms(TimeStart, month, day, year, hour, minute, second);

sprintf(junk, "%s %d, %d   %02d:%02d:%02d", 
               short_month_name[month], day, year, 
               hour, minute, second);

out << prefix << "TimeStart = " << TimeStart << "  (" << junk << ")\n";

hour   = TimeDelta/3600;
minute = (TimeDelta%3600)/60;
second = TimeDelta%60;

sprintf(junk, "%02d:%02d:%02d", hour, minute, second);

out << prefix << "TimeDelta = " << TimeDelta << "  (" << junk << ")\n";

   //
   //  don't print out the values, there's probably a lot of them
   //


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


double TimeSeries::operator()(int index) const

{

if ( (index < 0) || (index >= Nelements) )  {

   cerr << "\n\n  TimeSeries::operator()(int) const -> range check error\n\n";

   exit ( 1 );

}

return ( Value[index] );

}


////////////////////////////////////////////////////////////////////////


void TimeSeries::put(double x, int index)

{

if ( (index < 0) || (index >= Nelements) )  {

   cerr << "\n\n  TimeSeries::put(double, int) const -> range check error\n\n";

   exit ( 1 );

}

Value[index] = x;

return;

}


////////////////////////////////////////////////////////////////////////






