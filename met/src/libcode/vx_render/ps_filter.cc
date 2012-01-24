// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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
#include <string.h>

#include "vx_log.h"
#include "ps_filter.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PSFilter
   //


////////////////////////////////////////////////////////////////////////


PSFilter::PSFilter()

{

next = (PSFilter *) 0;

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


