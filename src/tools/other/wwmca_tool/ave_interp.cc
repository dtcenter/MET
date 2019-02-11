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

#include "ave_interp.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Ave_Interp
   //


////////////////////////////////////////////////////////////////////////


Ave_Interp::Ave_Interp()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Ave_Interp::~Ave_Interp()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Ave_Interp::Ave_Interp(const Ave_Interp & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


Ave_Interp & Ave_Interp::operator=(const Ave_Interp & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Ave_Interp::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Ave_Interp::clear()

{

Interpolator::clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Ave_Interp::assign(const Ave_Interp & a)

{

clear();

Interpolator::assign(a);

return;

}


////////////////////////////////////////////////////////////////////////


Interpolator * Ave_Interp::copy() const

{

Interpolator * i = (Interpolator *) 0;

i = new Ave_Interp (*this);

return ( i );

}


////////////////////////////////////////////////////////////////////////


void Ave_Interp::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Ave_Interp ...\n";

Interpolator::dump(out, depth);

return;

}


////////////////////////////////////////////////////////////////////////


InterpolationValue Ave_Interp::operator()(double x, double y) const

{

if ( !Data )  {

   mlog << Error << "\nAve_Interp::operator()(double x, double y) const -> no data!\n\n";

   exit ( 1 );

}

int j, n, good_count;
double sum;
InterpolationValue I;


n = (Width*Width);

good_count = 0;

sum = 0.0;

for (j=0; j<n; ++j)  {

   if ( !(Data[j].ok) )  continue;

   ++good_count;

   sum += Data[j].value;

}   // for j

if ( (good_count > 0) && (good_count >= NgoodNeeded) )  {

   I.ok = true;

   I.value = (sum/good_count);

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



