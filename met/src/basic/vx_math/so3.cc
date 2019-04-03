

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
#include <cstdio>
#include <cmath>

#include "so3.h"

#include "ascii_table.h"
#include "fix_float.h"
#include "trig.h"


////////////////////////////////////////////////////////////////////////


static const double axis_test_tol = 1.0e-7;


////////////////////////////////////////////////////////////////////////


static int my_signum(double);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SO3
   //


////////////////////////////////////////////////////////////////////////


SO3::SO3()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


SO3::~SO3()

{

clear();

}


////////////////////////////////////////////////////////////////////////


SO3::SO3(const SO3 & s)

{

init_from_scratch();

assign(s);

}


////////////////////////////////////////////////////////////////////////


SO3 & SO3::operator=(const SO3 & s)

{

if ( this == &s )  return ( * this );

assign(s);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void SO3::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SO3::clear()

{

M11 = 1.0;
M12 = 0.0;
M13 = 0.0;

M21 = 0.0;
M22 = 1.0;
M23 = 0.0;

M31 = 0.0;
M32 = 0.0;
M33 = 1.0;


return;

}


////////////////////////////////////////////////////////////////////////


void SO3::assign(const SO3 & s)

{

clear();

M11 = s.M11;
M12 = s.M12;
M13 = s.M13;

M21 = s.M21;
M22 = s.M22;
M23 = s.M23;

M31 = s.M31;
M32 = s.M32;
M33 = s.M33;


return;

}


////////////////////////////////////////////////////////////////////////


void SO3::dump(ostream & out, int depth) const

{

Indent prefix(depth);
double ax, ay, az, angle;
ConcatString junk;


   //
   // 1st row
   //

junk.format("%13.10f  %13.10f  %13.10f", M11, M12, M13);

out << prefix << junk << "\n";

   //
   // 2nd row
   //

junk.format("%13.10f  %13.10f  %13.10f", M21, M22, M23);

out << prefix << junk << "\n";

   //
   // 3rd row
   //

junk.format("%13.10f  %13.10f  %13.10f", M31, M32, M33);

out << prefix << junk << "\n";

   //
   //  blank lline
   //

out << prefix << "\n";

   //
   //  axis & angle
   //

get_axis_angle(ax, ay, az, angle);

junk.format("%13.10f", angle);

fix_float(junk);

out << prefix << "Angle = " << junk << "\n";

junk.format("%13.10f  %13.10f  %13.10f", ax, ay, az);

out << prefix << "Axis  = " << junk << "\n";

   //
   //  identity ?
   //

out << prefix << "Is Identity = ";

if ( is_identity() )  out << "True\n";
else                  out << "False\n";

   //
   //  z-axis ?
   //

out << prefix << "Is Z-Axis   = ";

if ( is_z_axis() )  out << "True\n";
else                out << "False\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void SO3::set_identity()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SO3::set_axis_angle(double ax, double ay, double az, double angle)

{

double t, nx, ny, nz;
const double C = cosd(angle);
const double S = sind(angle);
const double D = 1.0 - C;

clear();

   //
   //  normalize the given vector
   //

t = ax*ax + ay*ay + az*az;

if ( t == 0.0 )  {

   cerr << "\n\n  SO3::set_axis_angle() -> zero vector for axis!\n\n";

   exit ( 1 );

}

t = 1.0/sqrt(t);

nx = t*ax;
ny = t*ay;
nz = t*az;

   //////////

M11 =  nx*nx + (1.0 - nx*nx)*C;

M12 = -nz*S + nx*ny*D;

M13 =  ny*S + nx*nz*D;

   //////////

M21 =  nz*S + nx*ny*D;

M22 =  ny*ny + (1.0 - ny*ny)*C;

M23 = -nx*S + ny*nz*D;

   //////////

M31 = -ny*S + nx*nz*D;

M32 =  nx*S + ny*nz*D;

M33 =  nz*nz + (1.0 - nz*nz)*C;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void SO3::pre_axis_angle(double ax, double ay, double az, double angle)

{

SO3 a, result;

a.set_axis_angle(ax, ay, az, angle);

result = a*(*this);

assign(result);

return;

}


////////////////////////////////////////////////////////////////////////


void SO3::post_axis_angle(double ax, double ay, double az, double angle)

{

SO3 a, result;

a.set_axis_angle(ax, ay, az, angle);

result = (*this)*a;

assign(result);

return;

}


////////////////////////////////////////////////////////////////////////


void SO3::invert()

{

   //
   //  the inverse is just the transpose,
   //    so swap the off-diagonal elements
   //

double t;

t   = M12;
M12 = M21;
M21 = t;

t   = M13;
M13 = M31;
M31 = t;

t   = M23;
M23 = M32;
M32 = t;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool SO3::forward(double u, double v, double w, double & x, double & y, double & z) const

{

x = M11*u + M12*v + M13*w;

y = M21*u + M22*v + M23*w;

z = M31*u + M32*v + M33*w;


return ( true );

}


////////////////////////////////////////////////////////////////////////


bool SO3::reverse(double x, double y, double z, double & u, double & v, double & w) const

{

u = M11*x + M21*y + M31*z;

v = M12*x + M22*y + M32*z;

w = M13*x + M23*y + M33*z;


return ( true );

}


////////////////////////////////////////////////////////////////////////


bool SO3::ccw(double u, double v, double w, double & x, double & y, double & z) const

{

bool status = forward(u, v, w, x, y, z);


return ( status );

}


////////////////////////////////////////////////////////////////////////


bool SO3::cw(double x, double y, double z, double & u, double & v, double & w) const

{

bool status = reverse(x, y, z, u, v, w);


return ( status );

}


////////////////////////////////////////////////////////////////////////


bool SO3::is_identity() const

{

double t;
const double tol = 1.0e-9;


   //
   //  check off-diagonal elmenets
   //

t =   fabs(M12) + fabs(M21)
    + fabs(M13) + fabs(M31)
    + fabs(M23) + fabs(M32);

if ( t > tol )  return ( false );

   //
   //  check diagonal elements
   //

t = fabs(M11) + fabs(M22) + fabs(M33) - 3.0;

if ( t > tol )  return ( false );

   //
   //  yup
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


void SO3::get_axis_angle(double & ax, double & ay, double & az, double & angle) const

{

double v1, v2, v3;
double t;
double S, C;
const double tol = 1.0e-9;


v3 =  0.5*(M21 - M12);
v2 =  0.5*(M13 - M31);
v1 =  0.5*(M32 - M23);

S = sqrt( v1*v1 + v2*v2 + v3*v3 );   //  sine of rotation angle

t = M11 + M22 + M33;   //  trace

C = 0.5*(t - 1.0);   //  cosine of rotation angle

angle = atan2d(S, C);

   //
   //  first assume sin(angle) != 0
   //

if ( S >= tol )  {

   ax = v1/S;
   ay = v2/S;
   az = v3/S;

   return;

}

   //
   //  angle must be 0 or 180
   //

if ( is_identity() )  {

   angle = 0.0;

   ax = 0.0;
   ay = 0.0;
   az = 1.0;

   return;

}

   //
   //  angle must be 180
   //

angle = 180.0;

double Q11, Q12, Q13;
double Q22, Q23;
double Q33;


Q11 = fabs( 0.5*(M11 + 1.0) );
Q22 = fabs( 0.5*(M22 + 1.0) );
Q33 = fabs( 0.5*(M33 + 1.0) );

Q12 = 0.5*M12;
Q13 = 0.5*M13;
Q23 = 0.5*M23;

      //////////////////////

if ( Q11 > tol )  {

   ax = sqrt(Q11);

   ay = my_signum(Q12)*sqrt(Q22);

   az = my_signum(Q13)*sqrt(Q33);

   return;

}

      //////////////////////

if ( Q22 > tol )  {

   ax = 0.0;

   ay = sqrt(Q22);

   az = my_signum(Q23)*sqrt(Q33);

   return;

}

      //////////////////////

ax = 0.0;

ay = 0.0;

az = 1.0;

   //
   //   done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool SO3::is_z_axis() const

{

if ( is_identity() )  return ( true );

double ax, ay, az;
double angle;
double t;


get_axis_angle(ax, ay, az, angle);

t = sqrt( ax*ax + ay*ay + az*az );

t = 1.0/t;

ax *= t;
ay *= t;
az *= t;

// az -= 1.0;

// t = sqrt( ax*ax + ay*ay + az*az );

t = sqrt( ax*ax + ay*ay );


return ( t < axis_test_tol );

}


////////////////////////////////////////////////////////////////////////


bool SO3::is_x_axis() const

{

if ( is_identity() )  return ( true );

double ax, ay, az;
double angle;
double t;


get_axis_angle(ax, ay, az, angle);

t = sqrt( ax*ax + ay*ay + az*az );

t = 1.0/t;

ax *= t;
ay *= t;
az *= t;

// az -= 1.0;

// t = sqrt( ax*ax + ay*ay + az*az );

t = sqrt( ay*ay + az*az );


return ( t < axis_test_tol );

}


////////////////////////////////////////////////////////////////////////


bool SO3::is_y_axis() const

{

if ( is_identity() )  return ( true );

double ax, ay, az;
double angle;
double t;


get_axis_angle(ax, ay, az, angle);

t = sqrt( ax*ax + ay*ay + az*az );

t = 1.0/t;

ax *= t;
ay *= t;
az *= t;

// az -= 1.0;

// t = sqrt( ax*ax + ay*ay + az*az );

t = sqrt( ax*ax + az*az );


return ( t < axis_test_tol );

}


////////////////////////////////////////////////////////////////////////


double SO3::operator()(int row, int col) const

{

if ( (row < 0) || (row >= 3) || (col < 0) || (col >= 3) )  {

   cerr << "\n\n  SO3::operator()(int, int) const -> range check error (1)\n\n";

   exit ( 1 );

}

int k;
double x;


k = 3*row + col;

switch ( k )  {

   case 0:   x = M11;  break;
   case 1:   x = M12;  break;
   case 2:   x = M13;  break;

   case 3:   x = M21;  break;
   case 4:   x = M22;  break;
   case 5:   x = M23;  break;

   case 6:   x = M31;  break;
   case 7:   x = M32;  break;
   case 8:   x = M33;  break;

   default:
      cerr << "\n\n  SO3::operator()(int, int) const -> range check error (2)\n\n";
      exit ( 1 );
      break;

}


return ( x );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


SO3 operator*(const SO3 & a, const SO3 & b)

{

SO3 c;

c.M11 = (a.M11)*(b.M11) + (a.M12)*(b.M21) + (a.M13)*(b.M31);
c.M12 = (a.M11)*(b.M12) + (a.M12)*(b.M22) + (a.M13)*(b.M32);
c.M13 = (a.M11)*(b.M13) + (a.M12)*(b.M23) + (a.M13)*(b.M33);


c.M21 = (a.M21)*(b.M11) + (a.M22)*(b.M21) + (a.M23)*(b.M31);
c.M22 = (a.M21)*(b.M12) + (a.M22)*(b.M22) + (a.M23)*(b.M32);
c.M23 = (a.M21)*(b.M13) + (a.M22)*(b.M23) + (a.M23)*(b.M33);


c.M31 = (a.M31)*(b.M11) + (a.M32)*(b.M21) + (a.M33)*(b.M31);
c.M32 = (a.M31)*(b.M12) + (a.M32)*(b.M22) + (a.M33)*(b.M32);
c.M33 = (a.M31)*(b.M13) + (a.M32)*(b.M23) + (a.M33)*(b.M33);


return ( c );

}


////////////////////////////////////////////////////////////////////////


void operator*=(SO3 & a, const SO3 & b)

{

SO3 c;

c = a*b;

a = c;


return;

}


////////////////////////////////////////////////////////////////////////


int my_signum(double t)

{

if ( t > 0.0 )  return ( 1 );

if ( t < 0.0 )  return ( -1 );

return ( 0 );

}


////////////////////////////////////////////////////////////////////////



