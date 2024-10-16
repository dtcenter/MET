// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>


#include "hist.h"
#include "nint.h"
#include "vx_log.h"


using namespace std;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Histogram
   //


////////////////////////////////////////////////////////////////////////


Histogram::Histogram()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Histogram::~Histogram()

{

Count.clear();

}


////////////////////////////////////////////////////////////////////////


Histogram::Histogram(const Histogram & h)

{

init_from_scratch();

assign(h);

}


////////////////////////////////////////////////////////////////////////


Histogram & Histogram::operator=(const Histogram & h)

{

if ( this == &h )  return *this;

assign(h);

return *this;

}


////////////////////////////////////////////////////////////////////////


void Histogram::init_from_scratch()

{

Count.clear();

Nbins = 0;

Bottom = 0.0;

Delta = 0.0;

MinValue = MaxValue = 0.0;

is_empty = 1;

TooBigCount = TooSmallCount = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void Histogram::clear()

{

//for (int j=0; j<Nbins; ++j)  Count[j] = 0;

is_empty = 1;

TooBigCount = TooSmallCount = 0;

MaxValue = MinValue = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


void Histogram::set_nbd(int n, double b, double d)

{

Nbins  = n;

Bottom = b;
Delta  = d;


Count.resize(Nbins, 0);

clear();


return;

}


////////////////////////////////////////////////////////////////////////


void Histogram::assign(const Histogram & h)

{

set_nbd(h.Nbins, h.Bottom, h.Delta);

is_empty = h.is_empty;

int j;


for (j=0; j<Nbins; ++j)  {

   Count[j] = h.Count[j];

}

MaxValue = h.MaxValue;
MinValue = h.MinValue;

TooBigCount = h.TooBigCount;
TooSmallCount = h.TooSmallCount;


return;

}


////////////////////////////////////////////////////////////////////////


int Histogram::bin_count(int k) const

{

if ( (k < 0) || (k >= Nbins) )  {

   mlog << Error << "\nHistogram::bin_count(int) const -> range check error!\n\n";

   exit ( 1 );

}

return Count[k];

}


////////////////////////////////////////////////////////////////////////


void Histogram::add(double value)

{

if ( Nbins == 0 )  {

   mlog << Error << "\nHistogram::add(int) -> histogram has no bins!\n\n";

   exit ( 1 );

}

if ( is_empty )  {

   MinValue = MaxValue = value;

   is_empty = 0;

} else {

   if ( value < MinValue )  MinValue = value;
   if ( value > MaxValue )  MaxValue = value;

}

if ( value < Bottom )  { ++TooSmallCount;  return; }

double Top = Bottom + Nbins*Delta;

if ( value >= Top )  { ++TooBigCount;  return; }

int k;


k = nint(floor((value - Bottom)/Delta));

++(Count[k]);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int Histogram::total_bin_count() const

{

if ( Nbins == 0 )  return 0;

int j, t;

t = 0;

for (j=0; j<Nbins; ++j)  {

   t += Count[j];

}


return t;

}


////////////////////////////////////////////////////////////////////////


int Histogram::max_bin_count() const

{

if ( Nbins == 0 )  return 0;

int j, m;

m = Count[0];

for (j=1; j<Nbins; ++j)  {   //  j starts at one here

   if ( Count[j] > m )  m = Count[j];

}


return m;

}


////////////////////////////////////////////////////////////////////////


int Histogram::min_bin_count() const

{

if ( Nbins == 0 )  return 0;

int j, m;

m = Count[0];

for (j=1; j<Nbins; ++j)  {   //  j starts at one here

   if ( Count[j] < m )  m = Count[j];

}


return m;

}


////////////////////////////////////////////////////////////////////////


