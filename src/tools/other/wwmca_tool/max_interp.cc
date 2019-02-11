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

#include "max_interp.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Max_Interp
   //


////////////////////////////////////////////////////////////////////////


Max_Interp::Max_Interp()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Max_Interp::~Max_Interp()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Max_Interp::Max_Interp(const Max_Interp & m)

{

init_from_scratch();

assign(m);

}


////////////////////////////////////////////////////////////////////////


Max_Interp & Max_Interp::operator=(const Max_Interp & m)

{

if ( this == &m )  return ( * this );

assign(m);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Max_Interp::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Max_Interp::clear()

{

Interpolator::clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Max_Interp::assign(const Max_Interp & m)

{

clear();

Interpolator::assign(m);

return;

}


////////////////////////////////////////////////////////////////////////


Interpolator * Max_Interp::copy() const

{

Interpolator * i = (Interpolator *) 0;

i = new Max_Interp (*this);

return ( i );

}


////////////////////////////////////////////////////////////////////////


void Max_Interp::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Max_Interp ...\n";

Interpolator::dump(out, depth);

return;

}



////////////////////////////////////////////////////////////////////////


InterpolationValue Max_Interp::operator()(double x, double y) const

{

if ( !Data )  {

   mlog << Error << "\nMax_Interp::operator()(double x, double y) const -> no data!\n\n";

   exit ( 1 );

}

int j, n, good_count;
double z, Max;
InterpolationValue I;


n = (Width*Width);

good_count = 0;

Max = -1.0e30;

for (j=0; j<n; ++j)  {

   if ( !(Data[j].ok) )  continue;

   ++good_count;

   z = Data[j].value;

   if ( good_count == 1 )  Max = z;
   else {

      if ( z > Max )  Max = z;      

   }

}   // for j


if ( good_count >= NgoodNeeded )  {

   I.ok = true;

   I.value = Max;

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



