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
#include <cstdlib>
#include <cmath>

#include "gsl_bvn.h"

////////////////////////////////////////////////////////////////////////

static const double cf = 45.0/atan(1.0);

////////////////////////////////////////////////////////////////////////
//
//  Code for bvn stuff
//
////////////////////////////////////////////////////////////////////////

BVN_ellipse_data bvn_get_ellipse(const BVN_params & params, double q)

{

double d11, d22, d12;
double theta;
double radius;
BVN_ellipse_data ellipse;

   //
   //  center
   //

ellipse.x_center = params.xbar;
ellipse.y_center = params.ybar;

   //
   //  angle
   //

theta = calc_angle_deg(params.Suu, params.Svv, params.Suv);

ellipse.angle_degrees = theta;

   //
   //  a, b
   //

radius = sqrt( -2.0*log(1.0 - q) );

d11 = params.Suu;
d22 = params.Svv;
d12 = params.Suv;

rotate_moments(d11, d22, d12, theta);

ellipse.a = radius*sqrt(fabs(d11));

ellipse.b = radius*sqrt(fabs(d22));

   //
   //  done
   //

return ( ellipse );

}

////////////////////////////////////////////////////////////////////////

double bvn_get_quantile(const BVN_params & params, double xx, double yy)

{

double q;
double r2;
double z_1, z_2, y_1, y_2;

y_1 = xx;
y_2 = yy;

bvn_y_to_z(params, y_1, y_2, z_1, z_2);

r2 = z_1*z_1 + z_2*z_2;

q = 1.0 - exp(-0.5*r2);

return ( q );

}

////////////////////////////////////////////////////////////////////////
//
//  Code for misc functions
//
////////////////////////////////////////////////////////////////////////

void rotate_moments(double & Sxx, double & Syy, double & Sxy, double angle_degrees)

{

double Txx, Tyy, Txy;
double c, s, a;


a = angle_degrees/cf;

c = cos(a);
s = sin(a);


Txx = c*c*Sxx + s*s*Syy + 2.0*c*s*Sxy;

Tyy = s*s*Sxx + c*c*Syy - 2.0*c*s*Sxy;

Txy = c*s*(Syy - Sxx) + (c*c - s*s)*Sxy;

Sxx = Txx;
Syy = Tyy;
Sxy = Txy;

return;

}

////////////////////////////////////////////////////////////////////////

void bvn_y_to_z(const BVN_params & params, double y_1, double y_2, double & z_1, double & z_2)

{

double theta;
double a11, a22, a12;


y_1 -= params.xbar;
y_2 -= params.ybar;

   //
   //  calculate A = S^(-1/2)
   //

a11 = params.Suu;
a22 = params.Svv;
a12 = params.Suv;

theta = calc_angle_deg(a11, a22, a12);

rotate_moments(a11, a22, a12, theta);

a11 = 1.0/sqrt(a11);
a22 = 1.0/sqrt(a22);

rotate_moments(a11, a22, a12, -theta);

   //
   //  calculate z
   //

z_1 = a11*y_1 + a12*y_2;

z_2 = a12*y_1 + a22*y_2;

   //
   //  done
   //

return;

}

////////////////////////////////////////////////////////////////////////

double calc_angle_deg(const double & Suu, const double & Svv, const double & Suv)

{

double angle_degrees;
double rho1, rho2;

rho1 = 0.5*(Suu - Svv);

rho2 = Suv;

angle_degrees = cf*0.5*atan2(rho2, rho1);

return ( angle_degrees );

}

////////////////////////////////////////////////////////////////////////
