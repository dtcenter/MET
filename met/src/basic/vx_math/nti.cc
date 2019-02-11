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
#include <cmath>

#include "nti.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


int gcdi(int a, int b)

{

a = abs(a);
b = abs(b);

if ( a == 0 )  return ( b );

if ( b == 0 )  return ( a );

int c;


do {

   c = a%b;

   a = b;

   b = c;

} while ( b );



return ( a );

}


////////////////////////////////////////////////////////////////////////


int lcmi(int a, int b)

{

a = abs(a);
b = abs(b);

if ( (a == 0) || (b == 0) )  return ( 0 );

int c, d;


d = gcdi(a, b);

c = (a/d)*b;


return ( c );

}


////////////////////////////////////////////////////////////////////////


void bezouti(int a, int b, int & x, int & y)

{

if ( (a == 0) || (b == 0) )  {

   mlog << Error << "\nbezouti() -> neither a nor b can be zero!\n\n";

   exit ( 1 );

}

int q, r;
int sa, sb;
int ca1, ca2, cb1, cb2;
int cr1, cr2;
int L, L_over_a, L_over_b;
int n;


sa = sb = 1;

if ( a < 0 )  { sa = -1;  a = -a; }
if ( b < 0 )  { sb = -1;  b = -b; }

L = lcmi(a, b);

L_over_a = L/a;
L_over_b = L/b;


ca1 = 1;
ca2 = 0;

cb1 = 0;
cb2 = 1;

do {

   q = a/b;

   r = a - q*b;

   cr1 = ca1 - q*cb1;
   cr2 = ca2 - q*cb2;

   ca1 = cb1;
   ca2 = cb2;

   cb1 = cr1;
   cb2 = cr2;

   a = b;

   b = r;

} while ( b );


x = ca1;
y = ca2;

   //
   //  reduce x and y as much as possible
   //

n = (y*L_over_b - x*L_over_a)/( L_over_a*L_over_a + L_over_b*L_over_b );

x += n*L_over_a;
y -= n*L_over_b;

   //
   //  fudge signs, if needed
   //

if ( sa < 0 )  x = -x;
if ( sb < 0 )  y = -y;

   //
   //  done
   //

return;

}

////////////////////////////////////////////////////////////////////////


