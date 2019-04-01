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
#include <fstream>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <string.h>

#include "vx_log.h"
#include "empty_string.h"
#include "ps_filter.h"


////////////////////////////////////////////////////////////////////////


static const int default_decimal_places = 5;

static const int     max_decimal_places = 10;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PSFilter
   //


////////////////////////////////////////////////////////////////////////


PSFilter::PSFilter()

{

next = (PSFilter *) 0;

set_decimal_places(default_decimal_places);

}


////////////////////////////////////////////////////////////////////////


PSFilter::~PSFilter()

{

if ( next )  { delete next;  next = (PSFilter *) 0; }

}


////////////////////////////////////////////////////////////////////////


void PSFilter::eat(unsigned char c)

{

if ( next )  next->eat(c);
else {

   mlog << Error << "\nPSFilter::eat(unsigned char) -> null next pointer\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void PSFilter::eod()

{

if ( next )  next->eod();
else {

   mlog << Error << "\nPSFilter::eod() -> null next pointer\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void PSFilter::set_decimal_places(int k)

{

if ( (k < 0) || (k > max_decimal_places) )  {

   mlog << Error
        << "\n\n  PSFilter::set_decimal_places(int) -> bad value ... " << k << "\n\n";

   exit ( 1 );

}

DecimalPlaces = k;

snprintf(double_format, sizeof(double_format), "%%.%df", DecimalPlaces);   // example:  "%.5f"


return;

}


////////////////////////////////////////////////////////////////////////


PSFilter & PSFilter::operator<<(int k)

{

char junk[128];

snprintf(junk, sizeof(junk), "%d", k);

(*this) << junk;

return ( *this );

}


////////////////////////////////////////////////////////////////////////


PSFilter & PSFilter::operator<<(const char * s)

{

if ( empty(s) )  {

   mlog << Error
        << "\n\n  PSFilter::operator<<(const char *) -> empty string!\n\n";

   exit ( 1 );

}

while ( *s )  {

   eat((unsigned char) (*s));

   ++s;

}


return ( *this );

}


////////////////////////////////////////////////////////////////////////


PSFilter & PSFilter::operator<<(const ConcatString & s)

{

operator<<(s.text());

return ( *this );

}


////////////////////////////////////////////////////////////////////////


PSFilter & PSFilter::operator<<(const double x)

{

  ConcatString junk;

  junk.format(double_format, x);

  operator<<(junk);

  return ( * this );

}


////////////////////////////////////////////////////////////////////////



