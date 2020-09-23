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

#include "ascii85_filter.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ASCII85EncodeFilter
   //


////////////////////////////////////////////////////////////////////////


ASCII85EncodeFilter::ASCII85EncodeFilter()

{

u = 0;

count = 0;

}


////////////////////////////////////////////////////////////////////////


ASCII85EncodeFilter::~ASCII85EncodeFilter()

{

u = 0;

count = 0;

}


////////////////////////////////////////////////////////////////////////


void ASCII85EncodeFilter::eat(unsigned char c)

{

int j;
unsigned char a[5];


u = 256*u + c;

++count;

if ( count == 4 )  {

   count = 0;

   if ( u == 0 )  {

      next->eat('z');

   } else {

      for (j=4; j>=0; --j)  {

         a[j] = (unsigned char) (u%85);

         u /= 85;

      }

      for (j=0; j<5; ++j)  next->eat(33 + a[j]);

   }   //  else

   u = 0;

}   //  if count == 4

return;

}


////////////////////////////////////////////////////////////////////////


void ASCII85EncodeFilter::eod()

{

int j;
unsigned char a[5];


if ( count > 0 )  {

   for (j=count; j<4; ++j)  u *= 256;

   for (j=4; j>=0; --j)  {

      a[j] = (unsigned char) (u%85);

      u /= 85;

   }

   for (j=0; j<=count; ++j)  next->eat(33 + a[j]);

}   //  if count > 0

next->eat('~');
next->eat('>');

next->eod();

return;

}


////////////////////////////////////////////////////////////////////////


