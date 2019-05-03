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

#include "vx_renderinfo.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class RenderInfo
   //


////////////////////////////////////////////////////////////////////////


RenderInfo::RenderInfo()

{

x_ll = y_ll = 0.0;

magnification = 1.0;

bw = 0;

n_filters = 0;

int j;

for (j=0; j<max_filters; ++j)  filter[j] = NoFilter;

}


////////////////////////////////////////////////////////////////////////


RenderInfo::~RenderInfo() { }


////////////////////////////////////////////////////////////////////////


RenderInfo::RenderInfo(const RenderInfo &r)

{

x_ll = r.x_ll;
y_ll = r.y_ll;

magnification = r.magnification;

bw = r.bw;

n_filters = r.n_filters;

int j;

for (j=0; j<max_filters; ++j)  filter[j] = r.filter[j];

}


////////////////////////////////////////////////////////////////////////


RenderInfo & RenderInfo::operator=(const RenderInfo & r)

{

if ( this == &r )  return ( * this );

x_ll = r.x_ll;
y_ll = r.y_ll;

magnification = r.magnification;

bw = r.bw;

n_filters = r.n_filters;

int j;

for (j=0; j<max_filters; ++j)  filter[j] = r.filter[j];

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void RenderInfo::add_filter(const int k)

{


if ( n_filters > max_filters )  {

   cerr << "\n\n  RenderInfo::add_filter() -> too many filters!\n\n";

   exit ( 1 );

}

filter[n_filters++] = k;

return;

}


////////////////////////////////////////////////////////////////////////





