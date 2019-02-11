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
#include <string.h>

#include "bit_filter.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class BitFilter
   //


////////////////////////////////////////////////////////////////////////


BitFilter::BitFilter()

{

shift = 0;

u = 0;

bits_per_component = 8;

}


////////////////////////////////////////////////////////////////////////


BitFilter::~BitFilter() { }


////////////////////////////////////////////////////////////////////////


BitFilter::BitFilter(int bits)

{

shift = 0;

u = 0;

bits_per_component = bits;

}


////////////////////////////////////////////////////////////////////////


void BitFilter::eat(unsigned char c)

{


u <<= bits_per_component;

u |= c;

shift += bits_per_component;


if ( shift >= 8 )  {

   // u = reflect(u);

   next->eat(u);

   u = 0;

   shift = 0;

}

return;

}


////////////////////////////////////////////////////////////////////////


void BitFilter::eod()

{

while ( shift > 0 )  eat(0);

next->eod();

return;

}


////////////////////////////////////////////////////////////////////////


