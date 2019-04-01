

////////////////////////////////////////////////////////////////////////


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
#include <string.h>
#include <cmath>

#include "builtin.h"


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

   { "sin",    1, builtin_sin,     0, 0, sin, 0 },
   { "cos",    1, builtin_cos,     0, 0, cos, 0 },
   { "tan",    1, builtin_tan,     0, 0, tan, 0 },

   { "sind",   1, builtin_sind,    0, 0, sin_deg, 0 },
   { "cosd",   1, builtin_cosd,    0, 0, cos_deg, 0 },
   { "tand",   1, builtin_tand,    0, 0, tan_deg, 0 },

   { "asin",   1, builtin_asin,    0, 0, asin, 0 },
   { "acos",   1, builtin_acos,    0, 0, acos, 0 },
   { "atan",   1, builtin_atan,    0, 0, atan, 0 },

   { "asind",  1, builtin_sind,    0, 0, arc_sin_deg, 0 },
   { "acosd",  1, builtin_cosd,    0, 0, arc_cos_deg, 0 },
   { "atand",  1, builtin_tand,    0, 0, arc_tan_deg, 0 },

   { "atan2",  2, builtin_atan2,   0, 0, 0, atan2 },
   { "atan2d", 2, builtin_atan2d,  0, 0, 0, atan2_deg },

   { "arg",    2, builtin_arg,     0, 0, 0, my_arg     },
   { "argd",   2, builtin_argd,    0, 0, 0, my_arg_deg },

   { "log",    1, builtin_log,     0, 0, log, 0 },
   { "exp",    1, builtin_exp,     0, 0, exp, 0 },

   { "log10",  1, builtin_log10,   0, 0, log10,    0 },
   { "exp10",  1, builtin_exp10,   0, 0, my_exp10, 0 },

   { "sqrt",   1, builtin_sqrt,    0, 0, sqrt, 0 },

   { "abs",    1, builtin_abs,     abs, 0, fabs, 0 },

   { "min",    2, builtin_min,     0, my_imin, 0, my_dmin },
   { "max",    2, builtin_max,     0, my_imax, 0, my_dmax },

   { "mod",    2, builtin_mod,     0, my_imod, 0, my_dmod },

   { "floor",  1, builtin_floor,   0, 0, floor, 0 },
   { "ceil",   1, builtin_ceil,    0, 0, ceil,  0 },

   { "step",   1, builtin_step,    my_istep, 0, my_dstep,  0 },

   // Functions defined in ConfigConstants
   // { "F_to_C", 1, builtin_F_to_C,  0, 0, my_F_to_C,  0 },
   // { "C_to_F", 1, builtin_C_to_F,  0, 0, my_C_to_F,  0 },

   { "nint",   1, builtin_nint,    0, 0, 0,  0 },
   { "sign",   1, builtin_sign,    0, 0, 0,  0 },


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

return ( sin(degrees*rad_per_deg) );

}


////////////////////////////////////////////////////////////////////////


double cos_deg(double degrees)

{

return ( cos(degrees*rad_per_deg) );

}


////////////////////////////////////////////////////////////////////////


double tan_deg(double degrees)

{

return ( tan(degrees*rad_per_deg) );

}


////////////////////////////////////////////////////////////////////////


double arc_sin_deg(double t)

{

return ( deg_per_rad*asin(t) );

}


////////////////////////////////////////////////////////////////////////


double arc_cos_deg(double t)

{

return ( deg_per_rad*acos(t) );

}


////////////////////////////////////////////////////////////////////////


double arc_tan_deg(double t)

{

return ( deg_per_rad*atan(t) );

}


////////////////////////////////////////////////////////////////////////


double atan2_deg(double y, double x)

{

return ( deg_per_rad*atan2(y, x) );

}


////////////////////////////////////////////////////////////////////////


double my_arg(double x, double y)

{

return ( atan2(y, x) );

}


////////////////////////////////////////////////////////////////////////


double my_arg_deg(double x, double y)

{

return ( deg_per_rad*atan2(y, x) );

}


////////////////////////////////////////////////////////////////////////


double my_exp10(double t)

{

return ( pow(10.0, t) );

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

return ( c );

}


////////////////////////////////////////////////////////////////////////


int my_nint(double x)

{

double y;
int a;

y = floor(x + 0.5);

a = (int) y;

if ( fabs(a - y) > 0.3 )  ++a;

return ( a );

}


////////////////////////////////////////////////////////////////////////


int my_isign(int k)

{

if ( k > 0 )  return ( 1 );

if ( k < 0 )  return ( -1 );


return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int my_dsign(double t)

{

if ( t > 0.0 )  return ( 1 );


if ( t < 0.0 )  return ( -1 );


return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int my_istep(int i)

{

if ( i >= 0 )  return ( 1 );

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


double my_dstep(double x)

{

if ( x >= 0.0 )  return ( 1.0 );

return ( 0.0 );

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

int j;

index = -1;

for (j=0; j<n_binfos; ++j)  {

   if ( binfo[j].name == text )  { index = j;  return ( true ); }

}


return ( false );

}


////////////////////////////////////////////////////////////////////////








