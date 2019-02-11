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

#include "hex_filter.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class HexEncodeFilter
   //


////////////////////////////////////////////////////////////////////////


HexEncodeFilter::HexEncodeFilter()

{

}


////////////////////////////////////////////////////////////////////////


HexEncodeFilter::~HexEncodeFilter()

{

}


////////////////////////////////////////////////////////////////////////


void HexEncodeFilter::eat(unsigned char c)

{

next->eat(hex_string[c >> 4]);
next->eat(hex_string[c & 15]);

return;

}


////////////////////////////////////////////////////////////////////////


void HexEncodeFilter::eod()

{

next->eat('>');

next->eod();

return;

}


////////////////////////////////////////////////////////////////////////


