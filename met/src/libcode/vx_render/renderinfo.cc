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

#include "vx_log.h"
#include "renderinfo.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class RenderInfo
   //


////////////////////////////////////////////////////////////////////////


RenderInfo::RenderInfo()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


RenderInfo::~RenderInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


RenderInfo::RenderInfo(const RenderInfo & r)

{

init_from_scratch();

assign(r);

}


////////////////////////////////////////////////////////////////////////


RenderInfo & RenderInfo::operator=(const RenderInfo & r)

{

if ( this == &r )  return ( * this );

assign(r);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void RenderInfo::init_from_scratch()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void RenderInfo::clear()

{

int j;

X_ll = Y_ll = 0.0;

X_mag = Y_mag = 0.0;

BW = false;

Nfilters = 0;

for (j=0; j<max_filters; ++j)  Filter[j] = NoFilter;

return;

}


////////////////////////////////////////////////////////////////////////


void RenderInfo::assign(const RenderInfo & r)

{

clear();

int j;

X_ll = r.X_ll;
Y_ll = r.Y_ll;

X_mag = r.X_mag;
Y_mag = r.Y_mag;

BW = r.BW;

Nfilters = r.Nfilters;

for (j=0; j<Nfilters; ++j)  Filter[j] = r.Filter[j];

return;

}


////////////////////////////////////////////////////////////////////////


void RenderInfo::add_filter(const int k)

{


if ( Nfilters > max_filters )  {

   mlog << Error << "\nRenderInfo::add_filter() -> "
        << "too many filters!\n\n";

   exit ( 1 );

}

Filter[Nfilters++] = k;

return;

}


////////////////////////////////////////////////////////////////////////


void RenderInfo::set_ll(double _x_ll, double _y_ll)

{

X_ll = _x_ll;
Y_ll = _y_ll;

return;

}


////////////////////////////////////////////////////////////////////////


void RenderInfo::set_mag(double _x_mag, double _y_mag)

{

if ( (_x_mag <= 0.0) || (_y_mag <= 0.0) )  {

   mlog << Error << "\nRenderInfo::set_mag(double, double) -> "
        << "bad magnification value\n\n";

   exit ( 1 );

}

X_mag = _x_mag;
Y_mag = _y_mag;

return;

}


////////////////////////////////////////////////////////////////////////


void RenderInfo::set_mag(double mag)

{

set_mag(mag, mag);

return;

}


////////////////////////////////////////////////////////////////////////


void RenderInfo::set_color(bool tf)

{

BW = ! tf;

return;

}


////////////////////////////////////////////////////////////////////////


int RenderInfo::filter(int n) const

{

if ( (n < 0) || (n >= Nfilters) )  {

   mlog << Error << "\nRenderInfo::filter(int) const -> "
        << "range check error!\n\n";

   exit ( 1 );

}

return ( Filter[n] );

}


////////////////////////////////////////////////////////////////////////





