// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
#include <cmath>

#include "by_case_info.h"
#include "analysis_utils.h"
#include "mode_atts.h"

#include "vx_log.h"

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ByCaseInfo
   //


////////////////////////////////////////////////////////////////////////


ByCaseInfo::ByCaseInfo()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ByCaseInfo::~ByCaseInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ByCaseInfo::ByCaseInfo(const ByCaseInfo & b)

{

init_from_scratch();

assign(b);

}


////////////////////////////////////////////////////////////////////////


ByCaseInfo & ByCaseInfo::operator=(const ByCaseInfo & b)

{

if ( this == &b )  return ( * this );

assign(b);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ByCaseInfo::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void ByCaseInfo::clear()

{

valid            = (unixtime) 0;

line_count       = 0;

area_matched     = 0.0;
area_unmatched   = 0.0;

n_fcst_matched   = 0;
n_fcst_unmatched = 0;

n_obs_matched    = 0;
n_obs_unmatched  = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void ByCaseInfo::assign(const ByCaseInfo & b)

{

clear();

valid              = b.valid;

line_count         = b.line_count;

area_matched       = b.area_matched;
area_unmatched     = b.area_unmatched;

n_fcst_matched     = b.n_fcst_matched;
n_fcst_unmatched   = b.n_fcst_unmatched;

n_obs_matched      = b.n_obs_matched;
n_obs_unmatched    = b.n_obs_unmatched;


return;

}


////////////////////////////////////////////////////////////////////////


void ByCaseInfo::dump(ostream & out, int depth) const

{

Indent prefix(depth);
ConcatString junk;


make_timestring(valid, junk);


out << prefix << "Valid Time       = " << junk            << "\n";

out << prefix << "Line Count       = " << line_count       << "\n";

out << prefix << "Area Matched     = " << area_matched     << "\n";
out << prefix << "Area Unmatched   = " << area_unmatched   << "\n";

out << prefix << "# Fcst Matched   = " << n_fcst_matched   << "\n";
out << prefix << "# Fcst Unmatched = " << n_fcst_unmatched << "\n";

out << prefix << "# Obs Matched    = " << n_obs_matched    << "\n";
out << prefix << "# Obs Unmatched  = " << n_obs_unmatched  << "\n";


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void ByCaseInfo::add(const ModeLine & L)

{

int fcst, matched;
double x;


fcst      = L.is_fcst();
matched   = L.is_matched();

   //
   //  area
   //

x = L.area();

if ( x >= 0.0 )  {   //  check for "flag" value -9999

   if ( matched )  area_matched   += x;
   else            area_unmatched += x;

}

   //
   //  object counts
   //

     if (  fcst &&  matched )  ++n_fcst_matched;
else if (  fcst && !matched )  ++n_fcst_unmatched;
else if ( !fcst &&  matched )  ++n_obs_matched;
else if ( !fcst && !matched )  ++n_obs_unmatched;


   //
   //  done
   //

++line_count;

return;

}


////////////////////////////////////////////////////////////////////////






