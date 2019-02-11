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
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_log.h"
#include "vx_util.h"

#include "min_interp.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Min_Interp
   //


////////////////////////////////////////////////////////////////////////


Min_Interp::Min_Interp()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Min_Interp::~Min_Interp()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Min_Interp::Min_Interp(const Min_Interp & m)

{

init_from_scratch();

assign(m);

}


////////////////////////////////////////////////////////////////////////


Min_Interp & Min_Interp::operator=(const Min_Interp & m)

{

if ( this == &m )  return ( * this );

assign(m);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Min_Interp::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Min_Interp::clear()

{

Interpolator::clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Min_Interp::assign(const Min_Interp & m)

{

clear();

Interpolator::assign(m);

return;

}


////////////////////////////////////////////////////////////////////////


Interpolator * Min_Interp::copy() const

{

Interpolator * i = (Interpolator *) 0;

i = new Min_Interp (*this);

return ( i );

}


////////////////////////////////////////////////////////////////////////


void Min_Interp::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Min_Interp ...\n";

Interpolator::dump(out, depth);

return;

}


////////////////////////////////////////////////////////////////////////


InterpolationValue Min_Interp::operator()(double x, double y) const

{

if ( !Data )  {

   mlog << Error << "\nMin_Interp::operator()(double x, double y) const -> no data!\n\n";

   exit ( 1 );

}

int j, n, good_count;
double z, Min;
InterpolationValue I;


n = (Width*Width);

good_count = 0;

Min = 1.0e30;

for (j=0; j<n; ++j)  {

   if ( !(Data[j].ok) )  continue;

   ++good_count;

   z = Data[j].value;

   if ( good_count == 1 )  Min = z;
   else {

      if ( z < Min )  Min = z;      

   }

}   // for j


if ( good_count >= NgoodNeeded )  {

   I.ok = true;

   I.value = Min;

} else {

   I.ok = false;

   I.value = 0.0;

}

   //
   //  done
   //


return ( I );

}


////////////////////////////////////////////////////////////////////////



