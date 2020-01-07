// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;

#include <stdlib.h>
#include <cmath>

#include "color.h"
#include "vx_log.h"
#include "vx_math.h"


////////////////////////////////////////////////////////////////////////


static double max3(double a, double b, double c);
static double min3(double a, double b, double c);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Color
   //


////////////////////////////////////////////////////////////////////////


Color::Color()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Color::~Color()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Color::Color(const Color &c)

{

init_from_scratch();

assign(c);

}


////////////////////////////////////////////////////////////////////////


Color & Color::operator=(const Color &c)

{

if ( this == &c )  return ( *this );

assign(c);

return ( *this );

}


////////////////////////////////////////////////////////////////////////


Color::Color(unsigned char u)

{

init_from_scratch();

set_gray(u);

}


////////////////////////////////////////////////////////////////////////


Color::Color(unsigned char r, unsigned char g, unsigned char b)

{

set_rgb(r, g, b);

}


////////////////////////////////////////////////////////////////////////


void Color::init_from_scratch()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void Color::clear()

{

R = G = B = (unsigned char) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void Color::assign(const Color & c)

{

clear();

R = c.R;
G = c.G;
B = c.B;


return;

}


////////////////////////////////////////////////////////////////////////


void Color::to_gray()

{

Color q = *this;

R = G = B = color_to_gray(q);

return;

}


////////////////////////////////////////////////////////////////////////


void Color::set_rgb(unsigned char r, unsigned char g, unsigned char b)

{

R = r;
G = g;
B = b;


return;

}


////////////////////////////////////////////////////////////////////////


void Color::set_hsv(double h, double s, double v)

{

unsigned char r, g, b;

hsv_to_rgb(h, s, v, r, g, b);

set_rgb(r, g, b);

return;

}


////////////////////////////////////////////////////////////////////////


void Color::set_gray(unsigned char u)

{

R = G = B = u;

return;

}


////////////////////////////////////////////////////////////////////////


void Color::tint(const Color & c0, double t)

{

Color c1;

c1 = blend_colors(*this, c0, t);

assign(c1);

return;

}


////////////////////////////////////////////////////////////////////////


void Color::dump(ostream & out, int indent_depth) const

{

int ir, ig, ib;
Indent prefix(indent_depth);


ir = (int) R;
ig = (int) G;
ib = (int) B;


out << prefix << "(" << ir << ", " << ig << ", " << ib << ")\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


int operator==(const Color &a, const Color &b)

{

if ( a.R != b.R )  return ( 0 );
if ( a.G != b.G )  return ( 0 );
if ( a.B != b.B )  return ( 0 );

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int operator!=(const Color &a, const Color &b)

{

if ( a.R != b.R )  return ( 1 );
if ( a.G != b.G )  return ( 1 );
if ( a.B != b.B )  return ( 1 );

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


unsigned char color_to_gray(const Color &c)

{

int j;
int r, g, b;
unsigned char a;

r = c.red();
g = c.green();
b = c.blue();

j = ( 5*r + 8*g + 3*b )/16;

if ( j <   0 )  j =   0;
if ( j > 255 )  j = 255;

a = (unsigned char) j;

return ( a );

}


////////////////////////////////////////////////////////////////////////


void rgb_to_hsv(unsigned char r, unsigned char g, unsigned char b, double & h, double & s, double & v)

{

double dr, dg, db;


dr = r/255.0;
dg = g/255.0;
db = b/255.0;


drgb_to_dhsv(dr, dg, db, h, s, v);


return;

}


////////////////////////////////////////////////////////////////////////


void hsv_to_rgb(double h, double s, double v, unsigned char & r, unsigned char & g, unsigned char & b)

{

double dr, dg, db;


dhsv_to_drgb(h, s, v, dr, dg, db);


r = (unsigned char) nint(255.0*dr);
g = (unsigned char) nint(255.0*dg);
b = (unsigned char) nint(255.0*db);


return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Reference:
   //
   //    "Computer Graphics: Principles and Practice, Second Edition in C"
   //           by Foley, van Dam, Feiner & Hughes
   //
   //           Fig. 13.33, Page 592
   //


void drgb_to_dhsv(double r, double g, double b, double & h, double & s, double & v)

{

double t = 0.0;
double delta;
double max_value, min_value;

   //
   //  all input & output values between 0 and 1
   //

max_value = max3(r, g, b);
min_value = min3(r, g, b);

v = max_value;

if ( is_eq(v, 0.0) )  { s = h = 0.0;  return; }

delta = max_value - min_value;

s = delta/max_value;

     if ( is_eq(r, max_value) )  t = (g - b)/delta;
else if ( is_eq(g, max_value) )  t = 2.0 + (b - r)/delta;
else if ( is_eq(b, max_value) )  t = 4.0 + (r - g)/delta;

t *= 60;

t -= 360.0*floor(t/360.0);

h = t/360.0;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Reference:
   //
   //    "Computer Graphics: Principles and Practice, Second Edition in C"
   //           by Foley, van Dam, Feiner & Hughes
   //
   //           Fig. 13.34, Page 593
   //


void dhsv_to_drgb(double h, double s, double v, double & r, double & g, double & b)

{

if ( is_eq(s, 0.0) )  { r = g = b = v;  return; };

int i;
double f, p, q, t;

   //
   //  all input & output values between 0 and 1 inclusive
   //

h *= 360.0;

h -= 360.0*floor(h/360.0);

h /= 60.0;

i = nint(floor(h));

f = h - i;

p = v*(1.0 - s);

q = v*(1.0 - s*f);

t = v*(1.0 - s*(1.0 - f));

switch ( i )  {

   case 0:  r = v;  g = t;  b = p;   break;
   case 1:  r = q;  g = v;  b = p;   break;
   case 2:  r = p;  g = v;  b = t;   break;
   case 3:  r = p;  g = q;  b = v;   break;
   case 4:  r = t;  g = p;  b = v;   break;
   case 5:  r = v;  g = p;  b = q;   break;

}

return;

}


////////////////////////////////////////////////////////////////////////


double max3(double a, double b, double c)

{

double n;

n = ( (a > b) ? a : b );

return ( (n > c) ? n : c );

}


////////////////////////////////////////////////////////////////////////


double min3(double a, double b, double c)

{

double n;

n = ( (a < b) ? a : b );

return ( (n < c) ? n : c );

}


////////////////////////////////////////////////////////////////////////


Color blend_colors(const Color & color0, const Color & color1, double t)

{

int R, G, B;
Color result;

if ( t < 0.0 )  t = 0.0;

if ( t > 1.0 )  t = 1.0;

double omt = 1.0 - t;


R = nint( t*(color1.red()  ) + omt*(color0.red()  ) );
G = nint( t*(color1.green()) + omt*(color0.green()) );
B = nint( t*(color1.blue() ) + omt*(color0.blue() ) );


result.set_rgb((unsigned char) R, (unsigned char) G, (unsigned char) B);


return ( result );

}


////////////////////////////////////////////////////////////////////////







