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
#include <fstream>
#include <unistd.h>
#include <cstdlib>
#include <string.h>

#include "vx_ps_filter.h"


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

   cerr << "\n\n  PSFilter::eat(unsigned char) -> null next pointer\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void PSFilter::eod()

{

if ( next )  next->eod();
else {

   cerr << "\n\n  PSFilter::eod() -> null next pointer\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


