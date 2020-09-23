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
#include <string.h>
#include <cmath>

#include "vx_log.h"
#include "pwl.h"
#include "is_bad_data.h"


////////////////////////////////////////////////////////////////////////


static double linear_interpolate(double x, double x_0, double y_0, double x_1, double y_1);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PiecewiseLinear
   //


////////////////////////////////////////////////////////////////////////


PiecewiseLinear::PiecewiseLinear()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


PiecewiseLinear::~PiecewiseLinear()

{

n_alloc = N = 0;

if ( X )  { delete [] X;  X = (double *) 0; }
if ( Y )  { delete [] Y;  Y = (double *) 0; }

 if ( Name != "" )  { Name.clear(); }

}


////////////////////////////////////////////////////////////////////////


PiecewiseLinear::PiecewiseLinear(const PiecewiseLinear & pl)

{

init_from_scratch();

assign(pl);

}


////////////////////////////////////////////////////////////////////////


PiecewiseLinear & PiecewiseLinear::operator=(const PiecewiseLinear & pl)

{

if ( this == &pl )  return ( *this );

assign(pl);

return ( *this );

}


////////////////////////////////////////////////////////////////////////


void PiecewiseLinear::init_from_scratch()

{

n_alloc = N = 0;

Name.clear();

X = Y = (double *) 0;

extend(1);

return;
}


////////////////////////////////////////////////////////////////////////


void PiecewiseLinear::clear()

{


memset(X, 0, n_alloc*sizeof(double));
memset(Y, 0, n_alloc*sizeof(double));

N = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void PiecewiseLinear::assign(const PiecewiseLinear & p)

{

clear();

if ( p.N == 0 )  return;

N = p.N;

if ( p.Name != "" )  set_name(p.Name);

extend(p.N);

int j;

memset(X, 0, n_alloc*sizeof(double));
memset(Y, 0, n_alloc*sizeof(double));

for (j=0; j<N; ++j)  {

   X[j] = p.X[j];
   Y[j] = p.Y[j];

}


return;

}


////////////////////////////////////////////////////////////////////////


void PiecewiseLinear::extend(int n)

{

if ( n < n_alloc )  return;

int k;
double *u = (double *) 0;
double *v = (double *) 0;

   //
   //  round n up to the next multiple of pwl_alloc_inc
   //

k = n/pwl_alloc_inc;

if ( n%pwl_alloc_inc )  ++k;

n = k*pwl_alloc_inc;

   //
   //  allocate new arrays
   //

u = new double [n];
v = new double [n];

if ( !u || !v )  {

   mlog << Error << "\nPiecewiseLinear::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

memset(u, 0, n*sizeof(double));
memset(v, 0, n*sizeof(double));

   //
   //  copy any existing information over
   //

for (k=0; k<N; ++k)  {

   u[k] = X[k];
   v[k] = Y[k];

}

   //
   //  delete old arrays, if needed
   //

if ( n_alloc > 0 )  {

   delete [] X;   X = (double *) 0;
   delete [] Y;   Y = (double *) 0;

}

   //
   //  swap
   //

X = u;   u = (double *) 0;
Y = v;   v = (double *) 0;

   //
   //  done
   //

n_alloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


PiecewiseLinear::PiecewiseLinear(const char *NAME, int Npoints, const double *XX, const double *YY)

{

init_from_scratch();

  if ( NAME )  set_name((string)NAME);

extend(Npoints);

int j;

for (j=0; j<Npoints; ++j)  {

   X[j] = XX[j];
   Y[j] = YY[j];

}

N = Npoints;

}


////////////////////////////////////////////////////////////////////////


double PiecewiseLinear::x(int k) const

{

if ( (k < 0) || (k >= N) )  {

   mlog << Error << "\nPiecewiseLinear::x(int) -> range check error\n\n";

   exit ( 1 );

}

return ( X[k] );

}


////////////////////////////////////////////////////////////////////////


double PiecewiseLinear::y(int k) const

{

if ( (k < 0) || (k >= N) )  {

   mlog << Error << "\nPiecewiseLinear::y(int) -> range check error\n\n";

   exit ( 1 );

}

return ( Y[k] );

}


////////////////////////////////////////////////////////////////////////


double PiecewiseLinear::operator()(double t) const

{

if ( t < X[0] )      return ( Y[0] );

if ( t > X[N - 1] )  return ( Y[N - 1] );

int j;

for (j=0; j<(N - 1); ++j)  {

   if ( (t >= X[j]) && (t <= X[j + 1]) )  {

      return ( linear_interpolate(t, X[j], Y[j], X[j + 1], Y[j + 1]) );

   }

}


   //
   //  flow of control should not reach this point
   //

mlog << Error << "\nPiecewiseLinear::operator()(double) -> error in \""
     << Name << "\"! ... t = " << t << "\n\n";

exit ( 1 );

return ( -100 );   //  just to satisfy the compiler

}


////////////////////////////////////////////////////////////////////////


void PiecewiseLinear::set_name(const ConcatString T)

{

  if ( Name != "" )  { Name.clear(); }


  Name = T;


  return;

}


////////////////////////////////////////////////////////////////////////


void PiecewiseLinear::add_point(double xx, double yy)

{

extend(N + 1);

X[N] = xx;
Y[N] = yy;

++N;


return;

}


////////////////////////////////////////////////////////////////////////


void PiecewiseLinear::dump(ostream & out, int indent_depth) const

{

int j;
Indent prefix;


prefix.depth = indent_depth;


out << prefix << "PiecewiseLinear ... \n";

out << prefix << "   Name = " << Name << "\n";

out << prefix << "   # Points = " << N << "\n";


out.setf(ios::fixed);

for (j=0; j<N; ++j)  {

   out.width(3);  out << prefix << j << "    ";

   out.width(10);   out.precision(3);   out << (X[j]) << "   ";

   out.width(10);   out.precision(3);   out << (Y[j]);

   out << "\n";

}


out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


double linear_interpolate(double x, double x_0, double y_0, double x_1, double y_1)

{

double y, m;


m = (y_1 - y_0)/(x_1 - x_0);


y = y_0 + (x - x_0)*m;


return ( y );

}


////////////////////////////////////////////////////////////////////////


   //
   //  function to perform piecewise linear interpolation
   //


int pwl_interpolate(const double * y, const double * x, int n, double x_in, double & y_out)

{

int j;
double m;


y_out = 0.0;

if ( x_in < x[0] )  {   //  don't like this

   y_out = y[0];

   return ( 1 );

}

   //
   //  find bottom level for interpolation
   //

j = n - 1;

while ( (j >= 0) && (x[j] > x_in) )  --j;

   //
   //  range check
   //

if ( j < 0 )  { y_out = y[0];  return ( 0 ); }

if ( j >= (n - 1) )  { y_out = y[n - 1];   return ( 0 ); }

   //
   //  interpolate
   //

if ( is_eq(x[j + 1], x[j]) ) {

   y_out = y[j];

} else {

   m = (y[j + 1] - y[j])/(x[j + 1] - x[j]);

   y_out = y[j] + m*(x_in - x[j]);

}

   //
   //  done
   //

return ( 1 );

}


////////////////////////////////////////////////////////////////////////



