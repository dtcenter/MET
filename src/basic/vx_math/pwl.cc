// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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

clear();

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


PiecewiseLinear::PiecewiseLinear(const char *NAME, int Npoints, const double *XX, const double *YY)

{

init_from_scratch();

Name = NAME;

for (int j=0; j<Npoints; ++j)  {

   X.push_back(XX[j]);
   Y.push_back(YY[j]);

}

}


////////////////////////////////////////////////////////////////////////


void PiecewiseLinear::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void PiecewiseLinear::assign(const PiecewiseLinear & p)

{

clear();

Name = p.Name;
X = p.X;
Y = p.Y;

return;

}


////////////////////////////////////////////////////////////////////////


void PiecewiseLinear::clear()

{

Name.clear();

X.clear();
Y.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void PiecewiseLinear::dump(ostream & out, int indent_depth) const

{

int j;
Indent prefix;
int N = X.size();


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


void PiecewiseLinear::set_name(const ConcatString NAME)

{

Name = NAME;

return;

}


////////////////////////////////////////////////////////////////////////


double PiecewiseLinear::x(int k) const

{

if ( k < 0 || k >= X.size() )  {

   mlog << Error << "\nPiecewiseLinear::x(int) -> range check error\n\n";

   exit ( 1 );

}

return ( X[k] );

}


////////////////////////////////////////////////////////////////////////


double PiecewiseLinear::y(int k) const

{

if ( k < 0 || k >= Y.size() )  {

   mlog << Error << "\nPiecewiseLinear::y(int) -> range check error\n\n";

   exit ( 1 );

}

return ( Y[k] );

}


////////////////////////////////////////////////////////////////////////


void PiecewiseLinear::add_point(double xx, double yy)

{

X.push_back(xx);
Y.push_back(yy);

return;

}


////////////////////////////////////////////////////////////////////////


double PiecewiseLinear::operator()(double t) const

{

int N = X.size();

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
