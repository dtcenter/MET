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
#include <cmath>

#include "vx_util.h"

#include "trig.h"

#include "affine.h"
#include "viewgravity_to_string.h"


////////////////////////////////////////////////////////////////////////


inline double min(double a, double b) { return ( (a < b) ? a : b ); }


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AffineInterface
   //


////////////////////////////////////////////////////////////////////////


AffineInterface::AffineInterface()

{

}


////////////////////////////////////////////////////////////////////////


AffineInterface::~AffineInterface()

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


Affine::Affine(double _m11, double _m12, double _m21, double _m22, double _b1, double _b2)

{

init_from_scratch();

set_mb(_m11, _m12, _m21, _m22, _b1, _b2);

}


////////////////////////////////////////////////////////////////////////


Affine::Affine(double _m11, double _m12, double _m21, double _m22)

{

init_from_scratch();

set_m(_m11, _m12, _m21, _m22);

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

   //
   //  done
   //

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

   //
   //  done
   //

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


void Affine::set_m(double _m11, double _m12, double _m21, double _m22)

{

set_mb(_m11, _m12, _m21, _m22, 0.0, 0.0);

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


bool Affine::is_diagonal() const

{

const double bot = fabs(M11) + fabs(M22);

if ( bot == 0.0 )  return ( false );

const double top = fabs(M12) + fabs(M21);
const double tol = 1.0e-7;

return ( top/bot < tol );

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


Affine Affine::inverse() const

{

Affine a = *this;

a.invert();

return ( a );

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


void Affine::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "\n";

out << prefix << "m11 = " << M11 << "\n";
out << prefix << "m12 = " << M12 << "\n";
out << prefix << "m21 = " << M21 << "\n";
out << prefix << "m22 = " << M22 << "\n";

out << prefix << "\n";

out << prefix << "tx  = " << TX  << "\n";
out << prefix << "ty  = " << TY  << "\n";

out << prefix << "\n";

out << prefix << "det = " << Det << "\n";

out << prefix << "\n";

if ( is_conformal() )  {

   double scale, angle;

   scale = sqrt(Det);   //  the is_conformal() function checks that Det >= 0

   angle = atan2d(M12, M11);   //  M11 = scale*cos(angle), M12 = scale*sin(angle)

   out << prefix << "Is Conformal\n";
   out << prefix << "   Calculated Angle = " << angle << "\n";
   out << prefix << "   Calculated Scale = " << scale << "\n";

} else {

   out << prefix << "Not Conformal\n";

}

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void Affine::set_three_points_v1(double x1_from, double y1_from, 
                                 double x2_from, double y2_from, 
                                 double x3_from, double y3_from,

                                 double x1_to, double y1_to, 
                                 double x2_to, double y2_to, 
                                 double x3_to, double y3_to)

{

clear();

double D;
double QI_11, QI_12, QI_13;
double QI_21, QI_22, QI_23;
double QI_31, QI_32, QI_33;


D =   x1_from*(y2_from - y3_from)
    - x2_from*(y1_from - y3_from)
    + x3_from*(y1_from - y2_from);


if ( D == 0.0 )  {

   cerr << "\n\n  Affine::set_three_points_v1() -> collinear (u, v) points!\n\n";

   exit ( 1 );

}

D = 1.0/D;


QI_11 = D*( y2_from - y3_from );
QI_21 = D*( y3_from - y1_from );
QI_31 = D*( y1_from - y2_from );

QI_12 = D*( x3_from - x2_from );
QI_22 = D*( x1_from - x3_from );
QI_32 = D*( x2_from - x1_from );

QI_13 = D*( y3_from*x2_from - y2_from*x3_from );
QI_23 = D*( y1_from*x3_from - y3_from*x1_from );
QI_33 = D*( y2_from*x1_from - y1_from*x2_from );


M11 = x1_to*QI_11 + x2_to*QI_21 + x3_to*QI_31;
M12 = x1_to*QI_12 + x2_to*QI_22 + x3_to*QI_32;
TX  = x1_to*QI_13 + x2_to*QI_23 + x3_to*QI_33;

M21 = y1_to*QI_11 + y2_to*QI_21 + y3_to*QI_31;
M22 = y1_to*QI_12 + y2_to*QI_22 + y3_to*QI_32;
TY  = y1_to*QI_13 + y2_to*QI_23 + y3_to*QI_33;


calc_det();

if ( fabs(Det) < 1.0e-7 )  {

   cerr << "\n\n  Affine::set_three_points_v1() -> (x, y) points nearly collinear!\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Affine::set_three_points_v2(

        double x1_from, double y1_from, 
        double x1_to,   double y1_to, 

        double x2_from, double y2_from, 
        double x2_to,   double y2_to, 

        double x3_from, double y3_from, 
        double x3_to,   double y3_to)

{

set_three_points_v1(

   x1_from, y1_from, 
   x2_from, y2_from, 
   x3_from, y3_from,

   x1_to, y1_to, 
   x2_to, y2_to, 
   x3_to, y3_to);


return;

}


////////////////////////////////////////////////////////////////////////


AffineInterface * Affine::copy() const

{

Affine * a = new Affine ( *this );

return ( a );

}


////////////////////////////////////////////////////////////////////////


void Affine::set_pin(double x_from, double y_from, double x_to, double y_to)

{

double x0, y0;

set_translation(0.0, 0.0);

forward(x_from, y_from, x0, y0);

set_translation(x_to - x0, y_to - y0);

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
   //  Code for class DiagonalAffine
   //


////////////////////////////////////////////////////////////////////////


DiagonalAffine::DiagonalAffine()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


DiagonalAffine::~DiagonalAffine()

{

clear();

}


////////////////////////////////////////////////////////////////////////


DiagonalAffine::DiagonalAffine(const DiagonalAffine & g)

{

init_from_scratch();

assign(g);

}


////////////////////////////////////////////////////////////////////////


DiagonalAffine & DiagonalAffine::operator=(const DiagonalAffine & g)

{

if ( this == &g )  return ( * this );

assign(g);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void DiagonalAffine::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void DiagonalAffine::clear()

{

MX = MY = 1.0;

TX = TY = 0.0;

Det = 1.0;


return;

}


////////////////////////////////////////////////////////////////////////


void DiagonalAffine::assign(const DiagonalAffine & g)

{

clear();

MX = g.MX;
MY = g.MY;

TX  = g.TX;
TY  = g.TY;

Det = g.Det;


return;

}


////////////////////////////////////////////////////////////////////////


void DiagonalAffine::set_mb(double _mx, double _my, double _bx, double _by)

{

clear();

MX = _mx;
MY = _my;

TX  = _bx;
TY  = _by;

calc_det();

if ( fabs(Det) < 1.0e-7 )  {

   cerr << "\n\n  DiagonalAffine::set_mb() -> determinant nearly zero!\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void DiagonalAffine::set(const Box & From, const Box & To)

{

set_two_points_v2(From.left(),  From.bottom(), To.left(),  To.bottom(), 
                  From.right(), From.top(),    To.right(), To.top());


return;

}


////////////////////////////////////////////////////////////////////////


bool DiagonalAffine::is_conformal() const

{

double a, c;
const double tol = 1.0e-5;


   //
   //  calculate the entries of M^T M
   //

a = MX*MX;

c = MY*MY;


   //
   //  diagonal elements (nearly) equal?
   //

if ( fabs (a - c) > tol )  return ( false );

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


void DiagonalAffine::invert()

{


TX = -TX/MX;
TY = -TY/MY;

MX = 1.0/MX;
MY = 1.0/MY;

Det = 1.0/Det;


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void DiagonalAffine::forward(double u, double v, double & x, double & y) const

{

x = MX*u + TX;

y = MY*v + TY;


return;

}


////////////////////////////////////////////////////////////////////////


double DiagonalAffine::x_forward(double u) const

{

return ( MX*u + TX );

}


////////////////////////////////////////////////////////////////////////


double DiagonalAffine::y_forward(double v) const

{

return ( MY*v + TY );

}


////////////////////////////////////////////////////////////////////////


void DiagonalAffine::der_forward(double du, double dv, double & dx, double & dy) const

{


dx = MX*du;

dy = MY*dv;


return;

}


////////////////////////////////////////////////////////////////////////


double DiagonalAffine::x_der_forward(double du) const

{

return ( MX*du );

}


////////////////////////////////////////////////////////////////////////


double DiagonalAffine::y_der_forward(double dv) const

{

return ( MY*dv );

}


////////////////////////////////////////////////////////////////////////


void DiagonalAffine::reverse(double x, double y, double & u, double & v) const

{

u = (x - TX)/MX;
v = (y - TY)/MY;

return;

}


////////////////////////////////////////////////////////////////////////


double DiagonalAffine::x_reverse(double x) const

{

return ( (x - TX)/MX );

}


////////////////////////////////////////////////////////////////////////


double DiagonalAffine::y_reverse(double y) const

{

return ( (y - TY)/MY );

}


////////////////////////////////////////////////////////////////////////


void DiagonalAffine::der_reverse(double dx, double dy, double & du, double & dv) const

{

du = dx/MX;
dv = dy/MY;

return;

}


////////////////////////////////////////////////////////////////////////


double DiagonalAffine::x_der_reverse(double dx) const

{

return ( dx/MX );

}


////////////////////////////////////////////////////////////////////////


double DiagonalAffine::y_der_reverse(double dy) const

{

return ( dy/MY );

}


////////////////////////////////////////////////////////////////////////


void DiagonalAffine::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "\n";

out << prefix << "mx  = " << MX << "\n";
out << prefix << "my  = " << MY << "\n";

out << prefix << "\n";

out << prefix << "tx  = " << TX  << "\n";
out << prefix << "ty  = " << TY  << "\n";

out << prefix << "\n";

out << prefix << "det = " << Det << "\n";

out << prefix << "\n";

if ( is_conformal() )  {

   double scale, angle;

   scale = sqrt(Det);   //  the is_conformal() function checks that Det >= 0

   angle = atan2d(0.0, MX);   //  M11 = scale*cos(angle), M12 = scale*sin(angle)

   out << prefix << "Is Conformal\n";
   out << prefix << "   Calculated Angle = " << angle << "\n";
   out << prefix << "   Calculated Scale = " << scale << "\n";

} else {

   out << prefix << "Not Conformal\n";

}

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void DiagonalAffine::set_two_points(double x1_from, double y1_from, 
                                    double x2_from, double y2_from, 

                                    double x1_to, double y1_to, 
                                    double x2_to, double y2_to)

{

clear();


MX = (x2_to - x1_to)/(x2_from - x1_from);

TX = x1_to - MX*x1_from;


MY = (y2_to - y1_to)/(y2_from - y1_from);

TY = y1_to - MY*y1_from;


calc_det();

if ( fabs(Det) < 1.0e-7 )  {

   cerr << "\n\n  DiagonalAffine::set_two_points() -> (x, y) points nearly collinear!\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void DiagonalAffine::set_two_points_v2(double x1_from, double y1_from, double x1_to, double y1_to, 
                                       double x2_from, double y2_from, double x2_to, double y2_to)

{

set_two_points(

   x1_from, y1_from, 
   x2_from, y2_from, 

   x1_to, y1_to, 
   x2_to, y2_to);


return;

}


////////////////////////////////////////////////////////////////////////


AffineInterface * DiagonalAffine::copy() const

{

DiagonalAffine * a = new DiagonalAffine ( *this );

return ( a );

}


////////////////////////////////////////////////////////////////////////


void DiagonalAffine::set_pin(double x_from, double y_from, double x_to, double y_to)

{

double x0, y0;

set_translation(0.0, 0.0);

forward(x_from, y_from, x0, y0);

set_translation(x_to - x0, y_to - y0);

return;

}


////////////////////////////////////////////////////////////////////////


void DiagonalAffine::move(double du, double dv)

{

TX += du;

TY += dv;

return;

}


////////////////////////////////////////////////////////////////////////


Box DiagonalAffine::operator()(const Box & a) const

{

Box b;
double x_left, y_bot, x_right, y_top;

forward(a.left(), a.bottom(), x_left, y_bot);

forward(a.right(), a.top(), x_right, y_top);

b.set_lrbt(x_left, x_right, y_bot, y_top);


return ( b );

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


void ConformalAffine::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "\n";

out << prefix << "Angle    = " << Angle    << "\n";
out << prefix << "Scale    = " << Scale    << "\n";
out << prefix << "Tx       = " << TX       << "\n";
out << prefix << "Ty       = " << TY       << "\n";

   //
   //  done
   //

out.flush();

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

Angle -= 360.0*floor(Angle/360.0);

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


AffineInterface * ConformalAffine::copy() const

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


void ConformalAffine::set(const Box & image, const Box & view)

{

set(image, view, view_center_gravity);

return;

}


////////////////////////////////////////////////////////////////////////


void ConformalAffine::set(const Box & image, const Box & view, const ViewGravity g)

{

clear();

double u, v;
double mag;
double ximage, yimage, xview, yview;


mag = calc_mag(image.width(), image.height(), view.width(), view.height());

viewgravity_to_uv(g, u, v);


Scale = mag;
Angle = 0.0;

CosAngle = 1.0;
SinAngle = 0.0;


ximage = image.left()   + u*(image.width());
yimage = image.bottom() + v*(image.height());

xview  =  view.left()   + u*(view.width());
yview  =  view.bottom() + v*(view.height());


TX = xview - mag*ximage;
TY = yview - mag*yimage;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ConformalAffine::set_pin(double x_from, double y_from, double x_to, double y_to)

{

double x0, y0;

set_translation(0.0, 0.0);

forward(x_from, y_from, x0, y0);

set_translation(x_to - x0, y_to - y0);

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


bool ConformalAffine::is_diagonal() const

{

const double tol = 1.0e-7;

return ( fabs(SinAngle) < tol );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Box
   //


////////////////////////////////////////////////////////////////////////


Box::Box()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Box::Box(double L, double R, double B, double T)

{

init_from_scratch();

set_lrbt(L, R, B, T);

}


////////////////////////////////////////////////////////////////////////


Box::~Box()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Box::Box(const Box & b)

{

init_from_scratch();

assign(b);

}


////////////////////////////////////////////////////////////////////////


Box & Box::operator=(const Box & b)

{

if ( this == &b )  return ( * this );

assign(b);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Box::init_from_scratch()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void Box::clear()

{

Left = Right = Bottom = Top = 0.0;

IsEmpty = true;

return;

}


////////////////////////////////////////////////////////////////////////


void Box::assign(const Box & b)

{

clear();

Left   = b.Left;
Right  = b.Right;

Bottom = b.Bottom;
Top    = b.Top;

IsEmpty = b.IsEmpty;

return;

}


////////////////////////////////////////////////////////////////////////


void Box::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Left    = " << Left  << "\n";
out << prefix << "Right   = " << Right << "\n";
out << prefix << "\n";

out << prefix << "Bottom  = " << Bottom << "\n";
out << prefix << "Top     = " << Top    << "\n";
out << prefix << "\n";

out << prefix << "Width   = " << width()  << "\n";
out << prefix << "Height  = " << height() << "\n";
out << prefix << "\n";

out << prefix << "IsEmpty = " << ( IsEmpty ? "true" : "false" ) << "\n";
out << prefix << "\n";

if ( height() > 0.0 )  {

   out << prefix << "Aspect  = " << aspect()  << "\n";

}

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void Box::set_lrbt(double L, double R, double B, double T)

{

Left   = L;
Right  = R;

Bottom = B;
Top    = T;

if ( width() < 0.0 )  {

   cerr << "\n\n  Box::set_lrbt() -> negative width!\n\n";

   exit ( 1 );

}

if ( height() < 0.0 )  {

   cerr << "\n\n  Box::set_lrbt() -> negative height!\n\n";

   exit ( 1 );

}

calc_is_empty();

return;

}


////////////////////////////////////////////////////////////////////////


void Box::set_llwh(double x_ll, double y_ll, double W, double H)

{

set_lrbt(x_ll, x_ll + W, y_ll, y_ll + H);

calc_is_empty();

return;

}


////////////////////////////////////////////////////////////////////////


void Box::translate(double dx, double dy)

{

Left   += dx;
Right  += dx;

Top    += dy;
Bottom += dy;


return;

}


////////////////////////////////////////////////////////////////////////


void Box::x_translate(double dx)

{

Left   += dx;
Right  += dx;

return;

}


////////////////////////////////////////////////////////////////////////


void Box::y_translate(double dy)

{

Top    += dy;
Bottom += dy;

return;

}


////////////////////////////////////////////////////////////////////////


void Box::pin(double x, double y, double u, double v)

{

double t;

t = x - u*(Right - Left) - Left;

Left   += t;
Right  += t;

t = y - v*(Top - Bottom) - Bottom;

Top    += t;
Bottom += t;


return;

}


////////////////////////////////////////////////////////////////////////


void Box::scale_from_ll(double s)

{

if ( s <= 0.0 )  {

   cerr << "\n\n  Box::scale_from_ll(double) -> scale factor must be strictly positive\n\n";

   exit ( 1 );

}

   //
   //  rescale, with lower-left corner of the box fixed
   //

Right = Left + s*(Right - Left);

Top = Bottom + s*(Top - Bottom);


return;

}


////////////////////////////////////////////////////////////////////////


void Box::pad(double p)

{

pad(p, p);

return;

}


////////////////////////////////////////////////////////////////////////


void Box::pad(double px, double py)

{

Left   -= px;
Right  += px;

Top    += py;
Bottom -= py;

if ( (width() <= 0.0) || (height() <= 0.0) )  {

   cerr << "\n\n  Box::pad(double, double) -> width and/or height is negative!\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void Box::scale_from_center(double s)

{

if ( s <= 0.0 )  {

   cerr << "\n\n  Box::scale_from_center(double) -> scale factor must be strictly positive\n\n";

   exit ( 1 );

}

   //
   //  rescale, with center of the box fixed
   //

double t;

t = 0.5*(Right + Left)*(1.0 - s);

Left  = s*Left  + t;
Right = s*Right + t;

t = 0.5*(Top + Bottom)*(1.0 - s);

Top    = s*Top    + t;
Bottom = s*Bottom + t;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Box::scale_from_origin(double s)

{

if ( s <= 0.0 )  {

   cerr << "\n\n  Box::scale_from_origin(double) -> scale factor must be strictly positive\n\n";

   exit ( 1 );

}


Left   *= s;
Right  *= s;

Top    *= s;
Bottom *= s;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Box::scale_from_origin(double sx, double sy)

{

if ( (sx <= 0.0) || (sy <= 0.0) )  {

   cerr << "\n\n  Box::scale_from_origin(double, double) -> scale factor(s) must be strictly positive\n\n";

   exit ( 1 );

}


Left   *= sx;
Right  *= sx;

Top    *= sy;
Bottom *= sy;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Box::enclose(const Box & b)

{

Left   = min(Left,  b.Left);
Right  = max(Right, b.Right);

Bottom = min(Bottom,  b.Bottom);
Top    = max(Top,     b.Top);


return;

}


////////////////////////////////////////////////////////////////////////


void Box::place(const Box & viewport, const ViewGravity g)

{

double mag;
double u, v;
double x_view, y_view;

   //
   //  calculate magnification
   //

mag = calc_mag(*this, viewport);

scale_from_center(mag);

   //
   //  get pin point
   //

viewgravity_to_uv(g, u, v);

viewport.uv_to_xy(u, v, x_view, y_view);

pin(x_view, y_view, u, v);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


double Box::aspect() const

{

double a;

a = calc_aspect(width(), height());

return ( a );

}


////////////////////////////////////////////////////////////////////////


void Box::round()

{

Left   = floor (Left);
Right  = ceil  (Right);

Bottom = floor (Bottom);
Top    = floor (Top);


return;

}


////////////////////////////////////////////////////////////////////////


bool Box::is_inside(double x_test, double y_test) const

{

if ( IsEmpty )  return ( false );

if ( x_test < Left   )  return ( false );
if ( x_test > Right  )  return ( false );

if ( y_test < Bottom )  return ( false );
if ( y_test > Top    )  return ( false );

return ( true );

}


////////////////////////////////////////////////////////////////////////


void Box::center(double & x_cen, double & y_cen) const

{

x_cen = 0.5*(Left + Right);
y_cen = 0.5*(Bottom + Top);


return;

}


////////////////////////////////////////////////////////////////////////


DiagonalAffine Box::xy_to_uv() const

{

DiagonalAffine a;

a.set_two_points_v2(Left,  Bottom, 0.0, 0.0, 
                    Right, Top,    1.0, 1.0);


return ( a );

}


////////////////////////////////////////////////////////////////////////


DiagonalAffine Box::uv_to_xy() const

{

DiagonalAffine a;

a.set_two_points_v2(0.0, 0.0, Left,  Bottom, 
                    1.0, 1.0, Right, Top);

return ( a );

}


////////////////////////////////////////////////////////////////////////


void Box::xy_to_uv(double x, double y, double & u, double & v) const

{

// u = (x - Left)/(Right - Left);
// 
// v = (y - Bottom)/(Top - Bottom);

u = x_to_u(x);
v = y_to_v(y);


return;

}


////////////////////////////////////////////////////////////////////////


void Box::uv_to_xy(double u, double v, double & x, double & y) const

{

// x = Left + u*(Right - Left);
// 
// y = Bottom + v*(Top - Bottom);

x = u_to_x(u);
y = v_to_y(v);

return;

}


////////////////////////////////////////////////////////////////////////


double Box::area() const

{

if ( IsEmpty )  return ( 0.0 );

double a;

a = (width())*(height());

return ( a );

}


////////////////////////////////////////////////////////////////////////


void Box::calc_is_empty()

{

IsEmpty = false;

if ( (width() <= 0.0) || (height() <= 0.0) )  IsEmpty = true;

return;

}


////////////////////////////////////////////////////////////////////////


void Box::set_center(const Box & b)

{

if ( b.is_empty() )  {

   cerr << "\n\n  Box::set_center(const Box &) -> given box is empty!\n\n";

   exit ( 1 );

}

pin(b.x_center(), b.y_center(), 0.5, 0.5);

return;

}


////////////////////////////////////////////////////////////////////////


double Box::diagonal() const

{

if ( is_empty() )  return ( 0.0 );

const double dx = Right - Left;
const double dy = Top - Bottom;


return ( sqrt ( dx*dx + dy*dy ) );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


double calc_aspect(double width, double height)

{

if ( (width < 0.0) || (height < 0.0) )  {

   cerr << "\n\n  calc_aspect() const -> negative box dimensions!\n\n";

   exit ( 1 );

}

if ( height == 0.0 )  {

   cerr << "\n\n  calc_aspect() const -> zero height!\n\n";

   exit ( 1 );

}

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


double calc_mag(const Box & image, const Box & view)

{

double mag;

mag = calc_mag(image.width(), image.height(), view.width(), view.height());

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


DiagonalAffine operator*(const DiagonalAffine & a, const DiagonalAffine & b)

{

DiagonalAffine c;

c.MX = (a.MX)*(b.MX);

c.MY = (a.MY)*(b.MY);

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


Box intersect(const Box & a, const Box & b)

{

Box r;
double L, R, B, T;


L = max(a.Left,  b.Left);
R = min(a.Right, b.Right);

B = max(a.Bottom, b.Bottom);
T = min(a.Top,    b.Top);

r.set_lrbt(L, R, B, T);


return ( r );

}


////////////////////////////////////////////////////////////////////////


Box surround(const Box & a, const Box & b)

{

Box r;

if ( a.is_empty() && b.is_empty() )  return ( r );

if ( a.is_empty() )  return ( b );
if ( b.is_empty() )  return ( a );

double L, R, B, T;


L = min(a.Left,   b.Left);
R = max(a.Right,  b.Right);

B = min(a.Bottom, b.Bottom);
T = max(a.Top,    b.Top);

r.set_lrbt(L, R, B, T);


return ( r );

}


////////////////////////////////////////////////////////////////////////


Box surround(const Box * a, int n_boxes)

{

if ( n_boxes <= 0 )  {

   cerr << "\n\n  surround(const Box * a, int n_boxes) -> bad number of boxes ... " << n_boxes << "\n\n";

   exit ( 1 );

}

int j;
Box b;
double x_left  = 0.0;
double x_right = 0.0;
double y_bot   = 0.0;
double y_top   = 0.0;
const Box * p = a;

if ( p->is_nonempty() )  {

   x_left  = p->left();
   x_right = p->right();

   y_bot   = p->bottom();
   y_top   = p->top();

}

for (j=1; j<n_boxes; ++j)  {   //  j starts at one, here

   ++p;

   if ( p->is_nonempty() )  {

      x_left  = min(x_left,  p->left());
      x_right = max(x_right, p->right());

      y_bot   = min(y_bot,   p->bottom());
      y_top   = max(y_top,   p->top());

   }

}

b.set_lrbt(x_left, x_right, y_bot, y_top);

   //
   //  done
   //

return ( b );

}


////////////////////////////////////////////////////////////////////////


Box surround(const double * x_points, const double * y_points, const int n)

{

if ( n < 2 )  {

   cerr << "\n\n  surround(const double * x, const double * y, const int n) -> need at least 2 points!\n\n";

   exit ( 1 );

}

int j;
double xx, yy;
double L, R, B, T;
Box b;


L = R = x_points[0];
B = T = y_points[0];

for (j=1; j<n; ++j)  {   //  j starts at 1, here

   xx = x_points[j];
   yy = y_points[j];

   if ( xx < L )  L = xx;
   if ( xx > R )  R = xx;

   if ( yy < B )  B = yy;
   if ( yy > T )  T = yy;

}   //  for j

if ( (R - L) <= 0.0 )  {

   cerr << "\n\n  surround(const double * x, const double * y, const int n) -> bad box width!\n\n";

   exit ( 1 );

}

if ( (T - B) <= 0.0 )  {

   cerr << "\n\n  surround(const double * x, const double * y, const int n) -> bad box height!\n\n";

   exit ( 1 );

}

   //
   //  done
   //

b.set_lrbt(L, R, B, T);

return ( b );

}


////////////////////////////////////////////////////////////////////////


void viewgravity_to_uv(const ViewGravity g, double & u, double & v)

{

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
      cerr << "\n\n  viewgravity_to_uv() -> bad gravity ... "
           << viewgravity_to_string(g) << "\n\n";
      exit ( 1 );
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////





