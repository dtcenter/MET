// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2013
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

#include "psout_filter.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PSOutputFilter
   //


////////////////////////////////////////////////////////////////////////


PSOutputFilter::PSOutputFilter()

{

file = (ofstream *) 0;

column = 0;

}


////////////////////////////////////////////////////////////////////////


PSOutputFilter::~PSOutputFilter()

{

file = (ofstream *) 0;   //  don't delete it

}


////////////////////////////////////////////////////////////////////////


PSOutputFilter::PSOutputFilter(ofstream &s)

{

file = &s;

column = 0;

}


////////////////////////////////////////////////////////////////////////


void PSOutputFilter::eat(unsigned char c)

{

file->put(c);

++column;

if ( (column >= 70) && (c != '~') && (c != '>') )  { file->put('\n');  column = 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void PSOutputFilter::eod()

{

file = (ofstream *) 0;   //  don't delete it

return;

}


////////////////////////////////////////////////////////////////////////









