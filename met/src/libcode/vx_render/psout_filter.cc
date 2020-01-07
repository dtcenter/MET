// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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


static const bool default_ignore_columns = false;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PSOutputFilter
   //


////////////////////////////////////////////////////////////////////////


PSOutputFilter::PSOutputFilter()

{

file = (ofstream *) 0;

column = 0;

ignore_columns = default_ignore_columns;

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

ignore_columns = default_ignore_columns;

}


////////////////////////////////////////////////////////////////////////


void PSOutputFilter::eat(unsigned char c)

{

file->put(c);

++column;

if ( !ignore_columns && (column >= 70) && (c != '~') && (c != '>') )  { file->put('\n');  column = 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void PSOutputFilter::eod()

{

file = (ofstream *) 0;   //  don't delete it

column = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void PSOutputFilter::attach(ofstream * ofs)

{

detach();

file = ofs;

return;

}


////////////////////////////////////////////////////////////////////////


void PSOutputFilter::detach()

{

if ( file )  eod();

file = 0;

column = 0;

return;

}


////////////////////////////////////////////////////////////////////////









