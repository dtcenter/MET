

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "builtin.h"


using namespace std;


////////////////////////////////////////////////////////////////////////


static const double deg_per_rad = 45.0/atan(1.0);

static const double rad_per_deg = 1.0/deg_per_rad;


////////////////////////////////////////////////////////////////////////


static double  sin_deg      (double);
static double  cos_deg      (double);
static double  tan_deg      (double);

static double  arc_sin_deg  (double);
static double  arc_cos_deg  (double);
static double  arc_tan_deg  (double);

static double  atan2_deg    (double, double);

static double  my_arg       (double, double);
static double  my_arg_deg   (double, double);

static double  my_exp10     (double);

static int     my_imin      (int, int);
static int     my_imax      (int, int);

static double  my_dmin      (double, double);
static double  my_dmax      (double, double);

static int     my_imod      (int, int);
static double  my_dmod      (double, double);

static int     my_istep     (int);
static double  my_dstep     (double);

static double  my_F_to_C    (double);
static double  my_C_to_F    (double);


////////////////////////////////////////////////////////////////////////


const BuiltinInfo binfo[] = {

   { "sin",    1, builtin_sin,     nullptr, nullptr, sin, nullptr },
   { "cos",    1, builtin_cos,     nullptr, nullptr, cos, nullptr },
   { "tan",    1, builtin_tan,     nullptr, nullptr, tan, nullptr },

   { "sind",   1, builtin_sind,    nullptr, nullptr, sin_deg, nullptr },
   { "cosd",   1, builtin_cosd,    nullptr, nullptr, cos_deg, nullptr },
   { "tand",   1, builtin_tand,    nullptr, nullptr, tan_deg, nullptr },

   { "asin",   1, builtin_asin,    nullptr, nullptr, asin, nullptr },
   { "acos",   1, builtin_acos,    nullptr, nullptr, acos, nullptr },
   { "atan",   1, builtin_atan,    nullptr, nullptr, atan, nullptr },

   { "asind",  1, builtin_sind,    nullptr, nullptr, arc_sin_deg, nullptr },
   { "acosd",  1, builtin_cosd,    nullptr, nullptr, arc_cos_deg, nullptr },
   { "atand",  1, builtin_tand,    nullptr, nullptr, arc_tan_deg, nullptr },

   { "atan2",  2, builtin_atan2,   nullptr, nullptr, nullptr, atan2 },
   { "atan2d", 2, builtin_atan2d,  nullptr, nullptr, nullptr, atan2_deg },

   { "arg",    2, builtin_arg,     nullptr, nullptr, nullptr, my_arg     },
   { "argd",   2, builtin_argd,    nullptr, nullptr, nullptr, my_arg_deg },

   { "log",    1, builtin_log,     nullptr, nullptr, log, nullptr },
   { "exp",    1, builtin_exp,     nullptr, nullptr, exp, nullptr },

   { "log10",  1, builtin_log10,   nullptr, nullptr, log10,    nullptr },
   { "exp10",  1, builtin_exp10,   nullptr, nullptr, my_exp10, nullptr },

   { "sqrt",   1, builtin_sqrt,    nullptr, nullptr, sqrt, nullptr },

   { "abs",    1, builtin_abs,     abs, nullptr, fabs, nullptr },

   { "min",    2, builtin_min,     nullptr, my_imin, nullptr, my_dmin },
   { "max",    2, builtin_max,     nullptr, my_imax, nullptr, my_dmax },

   { "mod",    2, builtin_mod,     nullptr, my_imod, nullptr, my_dmod },

   { "floor",  1, builtin_floor,   nullptr, nullptr, floor, nullptr },
   { "ceil",   1, builtin_ceil,    nullptr, nullptr, ceil,  nullptr },

   { "step",   1, builtin_step,    my_istep, nullptr, my_dstep,  nullptr },

   // Functions defined in ConfigConstants
   // { "F_to_C", 1, builtin_F_to_C,  nullptr, nullptr, my_F_to_C,  nullptr },
   // { "C_to_F", 1, builtin_C_to_F,  nullptr, nullptr, my_C_to_F,  nullptr },

   { "nint",   1, builtin_nint,    nullptr, nullptr, nullptr,  nullptr },
   { "sign",   1, builtin_sign,    nullptr, nullptr, nullptr,  nullptr },


      //
      //  note that nint and sign are special
      //

};


const int n_binfos = sizeof(binfo)/sizeof(binfo[0]);


////////////////////////////////////////////////////////////////////////


static const double five_ninths = 5.0/9.0;    //  used in conversion between farenheight and celcius


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


double sin_deg(double degrees)

{

return sin(degrees*rad_per_deg);

}


////////////////////////////////////////////////////////////////////////


double cos_deg(double degrees)

{

return cos(degrees*rad_per_deg);

}


////////////////////////////////////////////////////////////////////////


double tan_deg(double degrees)

{

return tan(degrees*rad_per_deg);

}


////////////////////////////////////////////////////////////////////////


double arc_sin_deg(double t)

{

return deg_per_rad*asin(t);

}


////////////////////////////////////////////////////////////////////////


double arc_cos_deg(double t)

{

return deg_per_rad*acos(t);

}


////////////////////////////////////////////////////////////////////////


double arc_tan_deg(double t)

{

return deg_per_rad*atan(t);

}


////////////////////////////////////////////////////////////////////////


double atan2_deg(double y, double x)

{

return deg_per_rad*atan2(y, x);

}


////////////////////////////////////////////////////////////////////////


double my_arg(double x, double y)

{

return atan2(y, x);

}


////////////////////////////////////////////////////////////////////////


double my_arg_deg(double x, double y)

{

return deg_per_rad*atan2(y, x);

}


////////////////////////////////////////////////////////////////////////


double my_exp10(double t)

{

return pow(10.0, t);

}


////////////////////////////////////////////////////////////////////////


int my_imin(int a, int b)

{

return ( (a < b) ? a : b );

}


////////////////////////////////////////////////////////////////////////


int my_imax(int a, int b)

{

return ( (a > b) ? a : b );

}


////////////////////////////////////////////////////////////////////////


double my_dmin(double a, double b)

{

return ( (a < b) ? a : b );

}


////////////////////////////////////////////////////////////////////////


double my_dmax(double a, double b)

{

return ( (a > b) ? a : b );

}


////////////////////////////////////////////////////////////////////////


int my_imod(int a, int b)

{

if ( b < 0 )  b = -b;

if ( a < 0 )  a = b - a;

if ( b == 0 )  return ( a );

return ( a%b );

}


////////////////////////////////////////////////////////////////////////


double my_dmod(double a, double b)

{

if ( a < 0 )  a = b - a;

if ( b < 0 )  b = -b;

if ( b == 0.0 )  return ( a );

double c;

c = a - b*floor(a/b);

return c;

}


////////////////////////////////////////////////////////////////////////


int my_nint(double x)

{

double y;
int a;

y = floor(x + 0.5);

a = (int) y;

if ( fabs(a - y) > 0.3 )  ++a;

return a;

}


////////////////////////////////////////////////////////////////////////


int my_isign(int k)

{

if ( k > 0 )  return 1;

if ( k < 0 )  return -1;


return 0;

}


////////////////////////////////////////////////////////////////////////


int my_dsign(double t)

{

if ( t > 0.0 )  return 1;


if ( t < 0.0 )  return -1;


return 0;

}


////////////////////////////////////////////////////////////////////////


int my_istep(int i)

{

if ( i >= 0 )  return 1;

return 0;

}


////////////////////////////////////////////////////////////////////////


double my_dstep(double x)

{

if ( x >= 0.0 )  return 1.0;

return 0.0;

}


////////////////////////////////////////////////////////////////////////


   //
   //  F = (9/5)*C + 32
   //


double my_F_to_C(double F)

{

return ( five_ninths*(F - 32.0) );

}


////////////////////////////////////////////////////////////////////////


double my_C_to_F(double C)

{

return ( 1.8*C + 32.0 );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


bool is_builtin(const ConcatString text, int & index)

{

index = -1;

for (int j=0; j<n_binfos; ++j)  {

   if ( binfo[j].name == text )  { index = j;  return true; }

}


return false;

}


////////////////////////////////////////////////////////////////////////








