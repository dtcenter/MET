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
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_log.h"

#include "nearest_interp.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Nearest_Interp
   //


////////////////////////////////////////////////////////////////////////


Nearest_Interp::Nearest_Interp()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Nearest_Interp::~Nearest_Interp()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Nearest_Interp::Nearest_Interp(const Nearest_Interp & n)

{

init_from_scratch();

assign(n);

}


////////////////////////////////////////////////////////////////////////


Nearest_Interp & Nearest_Interp::operator=(const Nearest_Interp & n)

{

if ( this == &n )  return ( * this );

assign(n);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Nearest_Interp::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Nearest_Interp::clear()

{

Interpolator::clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Nearest_Interp::assign(const Nearest_Interp & n)

{

clear();

Interpolator::assign(n);

return;

}


////////////////////////////////////////////////////////////////////////


Interpolator * Nearest_Interp::copy() const

{

Interpolator * i = (Interpolator *) 0;

i = new Nearest_Interp (*this);

return ( i );

}


////////////////////////////////////////////////////////////////////////


void Nearest_Interp::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Nearest_Interp ...\n";

Interpolator::dump(out, depth);

return;

}


////////////////////////////////////////////////////////////////////////


InterpolationValue Nearest_Interp::operator()(double x, double y) const

{

if ( !Data )  {

   mlog << Error << "\nNearest_Interp::operator()(double x, double y) const -> no data!\n\n";

   exit ( 1 );

}

double u, v;
int ix, iy, n;
InterpolationValue I;

u = x + Wm1o2;
v = y + Wm1o2;

ix = nint(floor(u + 0.5));
iy = nint(floor(v + 0.5));

n = two_to_one(ix, iy);

I = Data[n];

   //
   //  done
   //


return ( I );

}


////////////////////////////////////////////////////////////////////////



