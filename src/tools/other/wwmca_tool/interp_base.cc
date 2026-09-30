// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
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

#include "vx_log.h"
#include "interp_base.h"
#include <vector>

using namespace std;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class InterpolationValue
   //


////////////////////////////////////////////////////////////////////////


InterpolationValue::InterpolationValue()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


InterpolationValue::~InterpolationValue()

{

clear();

}


////////////////////////////////////////////////////////////////////////


InterpolationValue::InterpolationValue(const InterpolationValue & i)

{

init_from_scratch();

assign(i);

}


////////////////////////////////////////////////////////////////////////


InterpolationValue & InterpolationValue::operator=(const InterpolationValue & i)

{

if ( this == &i )  return *this;

assign(i);

return *this;

}


////////////////////////////////////////////////////////////////////////


void InterpolationValue::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void InterpolationValue::clear()

{

ok = false;

value = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


void InterpolationValue::assign(const InterpolationValue & i)

{

clear();

ok = i.ok;

value = i.value;

return;

}


////////////////////////////////////////////////////////////////////////


void InterpolationValue::set_good(double _value)

{

ok = true;

value = _value;

return;

}


////////////////////////////////////////////////////////////////////////


void InterpolationValue::set_bad()

{

ok = false;

value = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Interpolator
   //


////////////////////////////////////////////////////////////////////////


