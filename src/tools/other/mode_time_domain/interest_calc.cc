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

#include "vx_math.h"

#include "interest_calc.h"

using namespace std;


////////////////////////////////////////////////////////////////////////


static const int ic_alloc_inc = 20;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class InterestCalculator
   //


////////////////////////////////////////////////////////////////////////


InterestCalculator::InterestCalculator()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


InterestCalculator::~InterestCalculator()

{

clear();

}


////////////////////////////////////////////////////////////////////////


InterestCalculator::InterestCalculator(const InterestCalculator & i)

{

init_from_scratch();

assign(i);

}


////////////////////////////////////////////////////////////////////////


InterestCalculator & InterestCalculator::operator=(const InterestCalculator & i)

{

if ( this == &i )  return *this;

assign(i);

return *this;

}


////////////////////////////////////////////////////////////////////////


void InterestCalculator::init_from_scratch()

{



clear();

return;

}


////////////////////////////////////////////////////////////////////////


void InterestCalculator::clear()

{

W.clear();

F.clear();

A.clear();

Nelements = 0;

Scale = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


void InterestCalculator::assign(const InterestCalculator & i)

{

clear();

if ( i.Nelements <= 0 )  return;

Nelements = i.Nelements;

W = i.W;

F = i.F;

A = i.A;

Scale = i.Scale;

return;

}


////////////////////////////////////////////////////////////////////////


void InterestCalculator::extend(int N)

{

N = (N + ic_alloc_inc - 1)/ic_alloc_inc;

N *= ic_alloc_inc;

W.reserve(N);

F.reserve(N);

A.reserve(N);

return;

}


////////////////////////////////////////////////////////////////////////


void InterestCalculator::add(double _weight, PWL _func, Argument _a)

{

if ( is_eq(_weight, 0.0) )  return;

if ( _weight < 0.0 )  {

   mlog << Error << "\n\n  InterestCalculator::add() -> weithgs must be >= 0\n\n";

   exit ( 1 );

}

if (Nelements < 0) Nelements = 0;   // SonarQube findings

extend(Nelements + 1);

W.push_back(_weight);

F.push_back(_func);

A.push_back(_a);

++Nelements;


   //
   //  done
   //


return;

}


////////////////////////////////////////////////////////////////////////


void InterestCalculator::check()

{

if ( Nelements == 0 )  {

   mlog << Error << "\n\n  InterestCalculator::scale() -> empty!\n\n";

   exit ( 1 );

}

int j;
double sum;   //  sum of all the weights

sum = 0.0;

for (j=0; j<Nelements; ++j)  {

   sum += W[j];

}

   //
   //  We know the weights are all nonnegative, so the only way
   //     the sum can be zero is if all the weights are zero, or, 
   //     equivalently, if Nelements is zero (which we've already
   //     checked for).
   //

Scale = 1.0/sum;

return;

}


////////////////////////////////////////////////////////////////////////


double InterestCalculator::operator()(const PairAtt3D & p)

{

int j;
double sum;
double w, x, I;
PWL f = 0;
Argument a = 0;

sum = 0.0;

for (j=0; j<Nelements; ++j)  {

   w = W[j];

   f = F[j];

   a = A[j];

   x = p.*a;

   I = (*f)(x);

   sum += w*I;

}   //  for j


sum *= Scale;

return sum;

}


////////////////////////////////////////////////////////////////////////


