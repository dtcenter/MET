// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
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

#include "trig.h"

#include "affine.h"
#include "viewgravity_to_string.h"


////////////////////////////////////////////////////////////////////////


inline double min(double a, double b) { return ( (a < b) ? a : b ); }


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ViewBox
   //


////////////////////////////////////////////////////////////////////////


ViewBox::ViewBox()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ViewBox::~ViewBox()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ViewBox::ViewBox(const ViewBox & v)

{

init_from_scratch();

assign(v);

}


////////////////////////////////////////////////////////////////////////


ViewBox & ViewBox::operator=(const ViewBox & v)

{

if ( this == &v )  return ( * this );

assign(v);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ViewBox::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void ViewBox::clear()

{

x_ll = y_ll = 0.0;

width = height = 100.0;

return;

}


////////////////////////////////////////////////////////////////////////


void ViewBox::assign(const ViewBox & v)

{

clear();

x_ll = v.x_ll;
y_ll = v.y_ll;

width = v.width;

height = v.height;


return;

}


////////////////////////////////////////////////////////////////////////


double ViewBox::aspect() const

{

double a;

a = calc_aspect(width, height);

return ( a );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class GeneralAffine
   //


////////////////////////////////////////////////////////////////////////


GeneralAffine::GeneralAffine()

{

}


////////////////////////////////////////////////////////////////////////


GeneralAffine::~GeneralAffine()

{

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Affine
   //


////////////////////////////////////////////////////////////////////////


Affine::Affine()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Affine::~Affine()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Affine::Affine(const Affine & g)

{

init_from_scratch();

assign(g);

}


////////////////////////////////////////////////////////////////////////


Affine & Affine::operator=(const Affine & g)

{

if ( this == &g )  return ( * this );

assign(g);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Affine::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Affine::clear()

{

M11 = M22 = 1.0;

M21 = M12 = 0.0;

TX = TY = 0.0;

Det = 1.0;


return;

}


////////////////////////////////////////////////////////////////////////


void Affine::assign(const Affine & g)

{

clear();

M11 = g.M11;
M12 = g.M12;
M21 = g.M21;
M22 = g.M22;

TX  = g.TX;
TY  = g.TY;

Det = g.Det;


return;

}


////////////////////////////////////////////////////////////////////////


void Affine::set_mb(double _m11, double _m12, double _m21, double _m22, double _b1, double _b2)

{

clear();

M11 = _m11;
M12 = _m12;
M21 = _m21;
M22 = _m22;

TX  = _b1;
TY  = _b2;

calc_det();

if ( fabs(Det) < 1.0e-7 )  {

   cerr << "\n\n  Affine::set_mb() -> determinant nearly zero!\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


bool Affine::is_conformal() const

{

double a, b, c;
const double tol = 1.0e-5;


   //
   //  calculate the entries of M^T M
   //

a = M21*M21 + M11*M11;

b = M21*M22 + M11*M12;

c = M22*M22 + M12*M12;


   //
   //  diagonal elements (nearly) equal?
   //

if ( fabs (a - c) > tol )  return ( false );

   //
   //  off-diagonal elements (nearly) zero?
   //

if ( fabs(b) > tol )  return ( false );

   //
   //  determinant positive?
   //

if ( Det < 0.0 )  return ( false );

   //
   //  ok
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


void Affine::invert()

{

const Affine g = * this;

Det = 1.0/(g.det());

M11 = g.m22();

M12 = -(g.m12());

M21 = -(g.m21());

M22 = g.m11();

M11 *= Det;
M12 *= Det;
M21 *= Det;
M22 *= Det;

TX = -( M11*(g.tx()) + M12*(g.ty()) );
TY = -( M21*(g.tx()) + M22*(g.ty()) );


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Affine::forward(double u, double v, double & x, double & y) const

{

x = M11*u + M12*v + TX;

y = M21*u + M22*v + TY;


return;

}


////////////////////////////////////////////////////////////////////////


void Affine::der_forward(double du, double dv, double & dx, double & dy) const

{


dx = M11*du + M12*dv;

dy = M21*du + M22*dv;


return;

}


////////////////////////////////////////////////////////////////////////


void Affine::reverse(double x, double y, double & u, double & v) const

{

x -= TX;
y -= TY;

u = M22*x - M12*y;

v = -M21*x + M11*y;

u /= Det;
v /= Det;

return;

}


////////////////////////////////////////////////////////////////////////


void Affine::der_reverse(double dx, double dy, double & du, double & dv) const

{


du = M22*dx - M12*dy;

dv = -M21*dx + M11*dy;

du /= Det;
dv /= Det;


return;

}


////////////////////////////////////////////////////////////////////////


void Affine::set_three_points(double u1, double v1, double u2, double v2, double u3, double v3,
                              double x1, double y1, double x2, double y2, double x3, double y3)

{

clear();

double D;
double QI11, QI12, QI13;
double QI21, QI22, QI23;
double QI31, QI32, QI33;


D =   u1*(v2 - v3)
    - u2*(v1 - v3)
    + u3*(v1 - v2);


if ( D == 0.0 )  {

   cerr << "\n\n  Affine::set_three_points() -> collinear (u, v) points!\n\n";

   exit ( 1 );

}

D = 1.0/D;


QI11 = D*( v2 - v3 );
QI21 = D*( v3 - v1 );
QI31 = D*( v1 - v2 );

QI12 = D*( u3 - u2 );
QI22 = D*( u1 - u3 );
QI32 = D*( u2 - u1 );

QI13 = D*( v3*u2 - v2*u3 );
QI23 = D*( v1*u3 - v3*u1 );
QI33 = D*( v2*u1 - v1*u2 );


M11 = x1*QI11 + x2*QI21 + x3*QI31;
M12 = x1*QI12 + x2*QI22 + x3*QI32;
TX  = x1*QI13 + x2*QI23 + x3*QI33;

M21 = y1*QI11 + y2*QI21 + y3*QI31;
M22 = y1*QI12 + y2*QI22 + y3*QI32;
TY  = y1*QI13 + y2*QI23 + y3*QI33;


calc_det();

if ( fabs(Det) < 1.0e-7 )  {

   cerr << "\n\n  Affine::set_three_points() -> (x, y) points nearly collinear!\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


GeneralAffine * Affine::copy() const

{

Affine * a = new Affine ( *this );

return ( a );

}


////////////////////////////////////////////////////////////////////////


void Affine::set_pin(double u, double v, double x, double y)

{

double u0, v0;

forward(u, v, u0, v0);

set_translation(TX + x - u0, TY + y - v0);

return;

}


////////////////////////////////////////////////////////////////////////


void Affine::move(double du, double dv)

{

TX += du;

TY += dv;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ConformalAffine
   //


////////////////////////////////////////////////////////////////////////


ConformalAffine::ConformalAffine()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ConformalAffine::~ConformalAffine()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ConformalAffine::ConformalAffine(const ConformalAffine & c)

{

init_from_scratch();

assign(c);

}


////////////////////////////////////////////////////////////////////////


ConformalAffine & ConformalAffine::operator=(const ConformalAffine & c)

{

if ( this == &c )  return ( * this );

assign(c);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ConformalAffine::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void ConformalAffine::clear()

{


Scale = 1.0;

Angle = 0.0;

CosAngle = 1.0;
SinAngle = 0.0;

TX = 0.0;
TY = 0.0;


return;

}


////////////////////////////////////////////////////////////////////////


void ConformalAffine::assign(const ConformalAffine & c)

{

clear();

Angle = c.Angle;

CosAngle = c.CosAngle;
SinAngle = c.SinAngle;

Scale = c.Scale;

TX = c.TX;
TY = c.TY;


return;

}


////////////////////////////////////////////////////////////////////////


void ConformalAffine::set_scale(double s)

{

if ( s <= 0.0 )  {

   cerr << "\n\n  ConformalAffine::set_scale(double) -> scale must be strictly positive!\n\n";

   exit ( 1 );

}

Scale = s;

return;

}


////////////////////////////////////////////////////////////////////////


void ConformalAffine::set_angle(double a)

{

Angle = a;

CosAngle = cosd(Angle);
SinAngle = sind(Angle);

return;

}


////////////////////////////////////////////////////////////////////////


void ConformalAffine::invert()

{

double tx_old, ty_old;


tx_old = TX;
ty_old = TY;


TX = -(CosAngle*tx_old - SinAngle*ty_old)/Scale;
TY = -(SinAngle*tx_old + CosAngle*ty_old)/Scale;


Angle = -Angle;

SinAngle = -SinAngle;   //  CosAngle is unchanged

Scale = 1.0/Scale;


return;

}


////////////////////////////////////////////////////////////////////////


ConformalAffine ConformalAffine::inverse() const

{

ConformalAffine ca = *this;

ca.invert();

return ( ca );

}


////////////////////////////////////////////////////////////////////////


GeneralAffine * ConformalAffine::copy() const

{

ConformalAffine * ca = new ConformalAffine ( *this );

return ( ca );

}


////////////////////////////////////////////////////////////////////////


void ConformalAffine::forward(double u, double v, double & x, double & y) const

{

x = Scale*(  u*CosAngle + v*SinAngle ) + TX;

y = Scale*( -u*SinAngle + v*CosAngle ) + TY;


return;

}


////////////////////////////////////////////////////////////////////////


void ConformalAffine::reverse(double x, double y, double & u, double & v) const

{

double xx, yy;

xx = x - TX;
yy = y - TY;


u = ( xx*CosAngle - yy*SinAngle )/Scale;

v = ( xx*SinAngle + yy*CosAngle )/Scale;


return;

}


////////////////////////////////////////////////////////////////////////


void ConformalAffine::der_forward(double du, double dv, double & dx, double & dy) const

{

dx = Scale*(  du*CosAngle + dv*SinAngle );

dy = Scale*( -du*SinAngle + dv*CosAngle );


return;

}


////////////////////////////////////////////////////////////////////////


void ConformalAffine::der_reverse(double dx, double dy, double & du, double & dv) const

{


du = ( dx*CosAngle - dy*SinAngle )/Scale;

dv = ( dx*SinAngle + dy*CosAngle )/Scale;


return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  "forward" maps image coords to view coords
   //


void ConformalAffine::set(const ViewBox & image, const ViewBox & view)

{

set(image, view, view_center_gravity);

return;

}


////////////////////////////////////////////////////////////////////////


void ConformalAffine::set(const ViewBox & image, const ViewBox & view, const ViewGravity g)

{

clear();

double u, v;
double mag;
double ximage, yimage, xview, yview;
char junk[128];


mag = calc_mag(image.width, image.height, view.width, view.height);


switch ( g )  {

   case view_center_gravity:      u = 0.5;   v = 0.5;   break;

   case view_north_gravity:       u = 0.5;   v = 1.0;   break;
   case view_south_gravity:       u = 0.5;   v = 0.0;   break;
   case view_east_gravity:        u = 1.0;   v = 0.5;   break;
   case view_west_gravity:        u = 0.0;   v = 0.5;   break;

   case view_northwest_gravity:   u = 0.0;   v = 1.0;   break;
   case view_northeast_gravity:   u = 1.0;   v = 1.0;   break;
   case view_southwest_gravity:   u = 0.0;   v = 0.0;   break;
   case view_southeast_gravity:   u = 1.0;   v = 0.0;   break;


   default:
      viewgravity_to_string(g, junk);
      cerr << "\n\n  ConformalAffine::set() -> bad gravity ... " << junk << "\n\n";
      exit ( 1 );
      break;

}   //  switch


Scale = mag;
Angle = 0.0;

CosAngle = 1.0;
SinAngle = 0.0;


ximage = image.x_ll + u*(image.width);
yimage = image.y_ll + v*(image.height);

xview  =  view.x_ll + u*(view.width);
yview  =  view.y_ll + v*(view.height);


TX = xview - mag*ximage;
TY = yview - mag*yimage;



return;

}


////////////////////////////////////////////////////////////////////////


void ConformalAffine::set_pin(double u, double v, double x, double y)

{

double u0, v0;

forward(u, v, u0, v0);

set_translation(TX + x - u0, TY + y - v0);

return;

}


////////////////////////////////////////////////////////////////////////


Affine ConformalAffine::affine_equivalent() const

{

Affine a;


a.set_mb(m11(), m12(), m21(), m22(), tx(), ty());


return ( a );

}


////////////////////////////////////////////////////////////////////////


void ConformalAffine::move(double du, double dv)

{

TX += du;

TY += dv;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


double calc_aspect(double width, double height)

{

return ( width/height );

}


////////////////////////////////////////////////////////////////////////


double calc_mag(double image_width, double image_height,
                double  view_width, double  view_height)

{

double image_aspect, view_aspect;
double rho, mag;

image_aspect = calc_aspect(image_width, image_height);
 view_aspect = calc_aspect( view_width,  view_height);

rho = image_aspect/view_aspect;

mag = min(rho, 1.0);

mag *= (view_width)/(image_width);

return ( mag );

}


////////////////////////////////////////////////////////////////////////


Affine operator*(const Affine & a, const Affine & b)

{

Affine c;

c.M11 = (a.M11)*(b.M11) + (a.M12)*(b.M21);
c.M12 = (a.M11)*(b.M12) + (a.M12)*(b.M22);

c.M21 = (a.M21)*(b.M11) + (a.M22)*(b.M21);
c.M22 = (a.M21)*(b.M12) + (a.M22)*(b.M22);

a.forward(b.TX, b.TY, c.TX, c.TY);

c.calc_det();

return ( c );

}


////////////////////////////////////////////////////////////////////////


ConformalAffine operator*(const ConformalAffine & a, const ConformalAffine & b)

{

ConformalAffine c;

   //
   //  the (forward) action of c is 
   //         the action of b followed by 
   //         the action of a
   //

c.set_scale( (a.Scale)*(b.Scale) );

c.set_angle( a.Angle + b.Angle );

a.forward(b.TX, b.TY, c.TX, c.TY);


return ( c );

}


////////////////////////////////////////////////////////////////////////








