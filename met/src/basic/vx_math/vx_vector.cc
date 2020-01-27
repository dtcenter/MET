

////////////////////////////////////////////////////////////////////////


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
#include <cstdio>
#include <cmath>

#include "indent.h"
#include "trig.h"
#include "vx_vector.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Vector
   //


////////////////////////////////////////////////////////////////////////


Vector::Vector()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Vector::~Vector()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Vector::Vector(double XX, double YY)

{

init_from_scratch();

set_xy(XX, YY);

}


////////////////////////////////////////////////////////////////////////


Vector::Vector(double XX, double YY, double ZZ)

{

init_from_scratch();

set_xyz(XX, YY, ZZ);

}


////////////////////////////////////////////////////////////////////////


Vector::Vector(double XX, double YY, double ZZ, char)

{

init_from_scratch();

double t = XX*XX + YY*YY + ZZ*ZZ;

if ( t == 0.0 )  {

      //  cerr might not have been constructed yet

   fprintf(stderr, "Vector::Vector(double XX, double YY, double ZZ, char) -> can't make unit vector from zero vector!\n\n");

   exit ( 1 );

}

set_xyz(XX, YY, ZZ);

normalize();

}


////////////////////////////////////////////////////////////////////////


Vector::Vector(const Vector & v)

{

init_from_scratch();

assign(v);

}


////////////////////////////////////////////////////////////////////////


Vector & Vector::operator=(const Vector &v)

{

if ( this == &v )  return ( * this );

assign(v);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Vector::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::clear()

{

X = Y = Z = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::dump(ostream & out, int depth) const

{

Indent prefix(depth);
char junk[128];

out << prefix << "[ ";

snprintf(junk, sizeof(junk), "%.8f", X);

out << junk << ", ";

snprintf(junk, sizeof(junk), "%.8f", Y);

out << junk << ", ";

snprintf(junk, sizeof(junk), "%.8f", Z);

out << junk << " ]\n";

   //
   //  done
   //

out << flush;

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::assign(const Vector & v)

{

X = v.X;
Y = v.Y;
Z = v.Z;


return;

}


////////////////////////////////////////////////////////////////////////


double Vector::abs() const

{

return ( sqrt( X*X + Y*Y + Z*Z ) );

}


////////////////////////////////////////////////////////////////////////


double Vector::abs_squared() const

{

return ( X*X + Y*Y + Z*Z );

}


////////////////////////////////////////////////////////////////////////


void Vector::normalize()

{

double t = abs();

if ( t == 0.0 )  {

   cerr << "\n\n  Vector::normalize() -> can't normalize zero vector!\n\n";

   exit ( 1 );

}

t = 1.0/t;

X *= t;
Y *= t;
Z *= t;

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::set_xy(double XX, double YY)

{

set_xyz(XX, YY, 0.0);

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::set_xyz(double XX, double YY, double ZZ)

{

X = XX;

Y = YY;

Z = ZZ;

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::set_altaz(double alt, double azi)

{

set_aad(alt, azi, 1.0);

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::set_aad(double alt, double azi, double dist)

{

X = dist*cosd(alt)*sind(azi);
Y = dist*cosd(alt)*cosd(azi);
Z = dist*sind(alt);

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::set_latlon(double lat, double lon)

{

set_aad(lat, lon, 1.0);

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::get_latlon(double & lat, double & lon) const

{

get_altaz(lat, lon);

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::get_altaz(double & alt, double & azi) const

{

double r;

r = abs();

if ( r == 0.0 )  {

   cerr << "\n\n  Vector::get_altaz() -> zero vector!\n\n";

   exit ( 1 );

}

alt = asind(Z/r);

if ( fabs(X/r) + fabs(Y/r) < 1.0e-6 )  azi = 0.0;
else                                   azi = atan2d(X, Y);   //  not (Y, X)!

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::rotate(const Vector & axis, double angle)

{

double c, s, d, t;
double m11, m12, m13;
double m21, m22, m23;
double m31, m32, m33;
double n1, n2, n3;
double xx, yy, zz;



angle = -angle;   //  so rotation is ccw



t = axis.abs();

if ( t == 0.0 )  {

   cerr << "\n\n  Vector::rotate() -> axis is the zero vector!\n\n";

   exit ( 1 );

}

t = 1.0/t;

n1 = t*(axis.X);
n2 = t*(axis.Y);
n3 = t*(axis.Z);

c = cosd(angle);

s = sind(angle);

d = 1.0 - c;


m11 = n1*n1 + (1.0 - n1*n1)*c;
m22 = n2*n2 + (1.0 - n2*n2)*c;
m33 = n3*n3 + (1.0 - n3*n3)*c;

m12 =  n3*s + n1*n2*d;
m21 = -n3*s + n1*n2*d;

m13 = -n2*s + n1*n3*d;
m31 =  n2*s + n1*n3*d;

m23 =  n1*s + n2*n3*d;
m32 = -n1*s + n2*n3*d;


xx = m11*X + m12*Y + m13*Z;
yy = m21*X + m22*Y + m23*Z;
zz = m31*X + m32*Y + m33*Z;

X = xx;
Y = yy;
Z = zz;

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::operator+=(const Vector & a)

{

X += a.X;
Y += a.Y;
Z += a.Z;

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::operator-=(const Vector & a)

{

X -= a.X;
Y -= a.Y;
Z -= a.Z;

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::operator*=(double t)

{

X *= t;
Y *= t;
Z *= t;

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::operator*=(int i)

{

const double t = (double) i;

X *= t;
Y *= t;
Z *= t;

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::operator/=(double t)

{

if ( t == 0.0 )  {

   cerr << "\n\n  Vector::operator/=(double) -> can't divide by zero!\n\n";

   exit ( 1 );

}

X /= t;
Y /= t;
Z /= t;

return;

}


////////////////////////////////////////////////////////////////////////


void Vector::operator/=(int i)

{

if ( i == 0 )  {

   cerr << "\n\n  Vector::operator/=(int) -> can't divide by zero!\n\n";

   exit ( 1 );

}

const double t = (double) i;

X /= t;
Y /= t;
Z /= t;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


double abs(const Vector & a)

{

double t;

t = a.abs();

return ( t );

}


////////////////////////////////////////////////////////////////////////


double abs_squared(const Vector & a)

{

double t;

t = a.abs_squared();

return ( t );

}


////////////////////////////////////////////////////////////////////////


double included_angle(const Vector & a, const Vector & b)

{

double deg;
double num, denom;

num = dot(a, b);

denom = (a.abs())*(b.abs());

deg = acosd(num/denom);

return ( deg );

}


////////////////////////////////////////////////////////////////////////


double dot(const Vector & a, const Vector & b)

{

double d;

d = (a.x())*(b.x()) + (a.y())*(b.y()) + (a.z())*(b.z());

return ( d );

}


////////////////////////////////////////////////////////////////////////


Vector cross(const Vector & a, const Vector & b)

{

Vector c;
double xx, yy, zz;

xx = (a.y())*(b.z()) - (a.z())*(b.y());  
yy = (a.z())*(b.x()) - (a.x())*(b.z());  
zz = (a.x())*(b.y()) - (a.y())*(b.x());

c.set_xyz(xx, yy, zz);

return ( c );

}


////////////////////////////////////////////////////////////////////////


double tsp(const Vector & a, const Vector & b, const Vector & c)

{

double det;

det = dot(cross(a, b), c);

return ( det );

}


////////////////////////////////////////////////////////////////////////


Vector altaz_to_north(double alt, double azi)

{

Vector v;
double x, y, z;


x = -sind(alt)*sind(azi);
y = -sind(alt)*cosd(azi);
z =  cosd(alt);

v.set_xyz(x, y, z);

return ( v );

}


////////////////////////////////////////////////////////////////////////


Vector altaz_to_east(double alt, double azi)

{

Vector v;
double x, y, z;


x = -cosd(azi);
y =  sind(azi);
z =  0.0;

v.set_xyz(x, y, z);

return ( v );

}


////////////////////////////////////////////////////////////////////////


Vector latlon_to_east(double lat, double lon)

{

Vector v;
double x, y, z;

x = -cosd(lon);
y =  sind(lon);
z =  0.0;


v.set_xyz(x, y, z);

return ( v );

}


////////////////////////////////////////////////////////////////////////


Vector latlon_to_north(double lat, double lon)

{

Vector v;
double x, y, z;
const double sin_lat = sind(lat);


x = -sin_lat*sind(lon);
y = -sin_lat*cosd(lon);
z = cosd(lat);


v.set_xyz(x, y, z);

return ( v );

}


////////////////////////////////////////////////////////////////////////


Vector proj_onto(const Vector & a, const Vector & b)  //  a projected onto b

{

Vector c;
Vector bn = b;

bn.normalize();

c = bn*dot(bn, a);

return ( c );

}


////////////////////////////////////////////////////////////////////////


Vector proj_onto_perp(const Vector & a, const Vector & b)  //  a projected onto b-perp

{

Vector c;

c = a - proj_onto(a, b);

return ( c );

}


////////////////////////////////////////////////////////////////////////


Vector operator-(const Vector & a)   //  unary minus 

{

Vector c;
double x, y, z;

x = -(a.x());
y = -(a.y());
z = -(a.z());

c.set_xyz(x, y, z);

return ( c );

}


////////////////////////////////////////////////////////////////////////


Vector operator*(double t, const Vector & a)

{

Vector b;
double x, y, z;

x = t*(a.x());
y = t*(a.y());
z = t*(a.z());

b.set_xyz(x, y, z);

return ( b );

}


////////////////////////////////////////////////////////////////////////


Vector operator*(int i, const Vector & a)

{

const double t = (double) i;
Vector b;
double x, y, z;

x = t*(a.x());
y = t*(a.y());
z = t*(a.z());

b.set_xyz(x, y, z);

return ( b );

}


////////////////////////////////////////////////////////////////////////


Vector operator/(const Vector & a, double t)

{

if ( t == 0.0 )  {

   cerr << "\n\n  Vector operator/(const Vector &a, double t) -> division by zero!\n\n";

   exit ( 1 );

}

Vector b;
double x, y, z;


x = (a.x())/t;
y = (a.y())/t;
z = (a.z())/t;

b.set_xyz(x, y, z);

return ( b );

}


////////////////////////////////////////////////////////////////////////


Vector operator/(const Vector & a, int i)

{

if ( i == 0 )  {

   cerr << "\n\n  Vector operator/(const Vector &a, double t) -> division by zero!\n\n";

   exit ( 1 );

}

const double t = (double) i;
Vector b;
double x, y, z;


x = (a.x())/t;
y = (a.y())/t;
z = (a.z())/t;

b.set_xyz(x, y, z);

return ( b );

}


////////////////////////////////////////////////////////////////////////


Vector operator*(const Vector & a, double t)

{

Vector b;
double x, y, z;

x = t*(a.x());
y = t*(a.y());
z = t*(a.z());

b.set_xyz(x, y, z);

return ( b );

}


////////////////////////////////////////////////////////////////////////


Vector operator*(const Vector & a, int i)

{

const double t = (double) i;
Vector b;
double x, y, z;

x = t*(a.x());
y = t*(a.y());
z = t*(a.z());

b.set_xyz(x, y, z);

return ( b );

}

////////////////////////////////////////////////////////////////////////


Vector operator+(const Vector & a, const Vector & b)

{

Vector c;
double x, y, z;

x = (a.x()) + (b.x());
y = (a.y()) + (b.y());
z = (a.z()) + (b.z());

c.set_xyz(x, y, z);

return ( c );

}


////////////////////////////////////////////////////////////////////////


Vector operator-(const Vector & a, const Vector & b)

{

Vector c;
double x, y, z;

x = (a.x()) - (b.x());
y = (a.y()) - (b.y());
z = (a.z()) - (b.z());

c.set_xyz(x, y, z);

return ( c );

}


////////////////////////////////////////////////////////////////////////


Vector latlon_to_vector(double lat, double lon)

{

Vector v;

v.set_latlon(lat, lon);

return ( v );

}


////////////////////////////////////////////////////////////////////////


void latlon_to_vector(double lat, double lon, double & x, double & y, double & z)

{

Vector v;

v.set_latlon(lat, lon);

x = v.x();
y = v.y();
z = v.z();

return;

}


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & out, const Vector & v)

{

char junk[128];

out << "[ ";

snprintf(junk, sizeof(junk), "%11.8f", v.x());

// fix_float(junk);

out << junk << ", ";

snprintf(junk, sizeof(junk), "%11.8f", v.y());

// fix_float(junk);

out << junk << ", ";

snprintf(junk, sizeof(junk), "%11.8f", v.z());

// fix_float(junk);

out << junk << " ]";


return ( out );

}


////////////////////////////////////////////////////////////////////////


Vector normalize(const Vector & a)

{

Vector b = a;

b.normalize();

return ( b );

}


////////////////////////////////////////////////////////////////////////









