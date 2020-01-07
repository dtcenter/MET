

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
#include <cmath>

#include "legendre.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static const int lower_degree_limit = 3;

static const double pi = 4.0*atan(1.0);

static const double J1 = 2.40482555769577;   //  First positive zero of J_0(x)

   //
   //  this needs external linkage
   //

int iter_count = 0;


////////////////////////////////////////////////////////////////////////


static double dr_first_guess(const int k, const int n);

static double lether_first_guess(const int k, const int n);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Legendre
   //


////////////////////////////////////////////////////////////////////////


Legendre::Legendre()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Legendre::~Legendre()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Legendre::Legendre(const Legendre & L)

{

init_from_scratch();

assign(L);

return;

}


////////////////////////////////////////////////////////////////////////


Legendre & Legendre::operator=(const Legendre & L)

{

if ( this == &L )  return ( * this );

assign(L);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Legendre::init_from_scratch()

{

P = 0;

PP = 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Legendre::clear()

{

if ( P )  { delete [] P;  P = 0; }

if ( PP )  { delete [] PP;  PP = 0; }

X = 0.0;

MaxDegree = -1;


return;

}


////////////////////////////////////////////////////////////////////////


void Legendre::assign(const Legendre & L)

{

clear();

if ( L.MaxDegree < 0 )  return;

set_max_degree(L.MaxDegree);

X = L.X;

int j;

for (j=0; j<=MaxDegree; ++j)  {

   P[j] = L.P[j];

   PP[j] = L.PP[j];

}


return;

}


////////////////////////////////////////////////////////////////////////


void Legendre::set_max_degree(int N)

{

if ( N < lower_degree_limit )  {

   mlog << Error << "\nLegendre::set_max_degree(int) -> "
        << "max degree can't be less than "
        << lower_degree_limit << "\n\n";

   exit ( 1 );

}

clear();

MaxDegree = N;

P     = new double [N + 1];

PP = new double [N + 1];

calc(0.0);

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //   P_0 (x) = 1;
   //   P_1 (x) = x;
   //
   //      (adapted from 12.17a, page 541 in Arfken)
   //
   //   P_n (x) = 2xP_(n - 1)(x) - P_(n - 2)(x) - [ xP_(n - 1)(x) - P_(n - 2) ]/n;
   //
   //      (adapted from 12.24, page 541 in Arfken)
   //
   //   P_n' (x) = n P_(n - 1) (x) + x p_(n - 1)'(x)
   //


void Legendre::calc(double xx)

{

int j;
double nm2, nm1, d_nm1;
double v, dv;


X = xx;

nm2 = P[0] = 1.0;
nm1 = P[1] = xx;

PP[0] = 0.0;

d_nm1 = PP[1] = 1.0;

for (j=2; j<=MaxDegree; ++j)  {

   v = 2.0*xx*nm1 - nm2 - ( xx*nm1 -  nm2 )/j;

   dv = j*nm1 + xx*d_nm1;

   P[j] = v;

   PP[j] = dv;

      //////////////

   nm2 = nm1;
   nm1 = v;

   d_nm1 = dv;

}   //  for j


return;

}


////////////////////////////////////////////////////////////////////////


double Legendre::d_and_r_root(int k)

{

double r, w;

d_and_r_root_weight(k, r, w);

return ( r );

}


////////////////////////////////////////////////////////////////////////


void Legendre::d_and_r_root_weight(int k, double & r, double & w)

{

double cor, r_new;
double num, den;
const double tol = 1.0e-14;
const int n = MaxDegree;


iter_count = 0;

r = dr_first_guess(k, n);

do {

   ++iter_count;

   calc(r);

   r_new = r - (P[n])/(PP[n]);

   cor = r_new - r;

   r = r_new;

}  while ( fabs(cor) >= tol );

calc(r);

r = r - (P[n])/(PP[n]);


   //
   //  weight
   //

calc(r);

num = 2.0*(1.0 - r*r);

den = n*(P[n - 1]);

den *= den;

w = num/den;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Legendre::lether_root_weight(int k, double & x, double & w)

{

double x_new, cor;
double V, E5;
double B, C;
double num, den;
double Pn;

const double  tol  =  1.0e-14;
const int     n    =  MaxDegree;
const int     nn1  =  n*(n + 1);
const double  B_2  =  (3.0 + nn1)/3.0;
const double  B_0  =  (1.0 + nn1)/3.0;
const double  C_3  =  (6.0 + 5.0*nn1)/6.0;
const double  C_0  =  (4.0 + 5.0*nn1)/6.0;


iter_count = 0;


x = lether_first_guess(k, n);


do  {

   ++iter_count;

   calc(x);

   Pn = P[n];


   V = P[n - 1] - x*Pn;

   V = Pn/(n*V);


   B = B_2*x*x - B_0;


   C = C_3*x*x*x - C_0;


   E5 = x + V*(B + C*V);

   E5 = 1.0 + V*E5;

   E5 = x - (1.0 - x)*(1.0 + x)*V*E5;


   x_new = E5;

   cor = x_new - x;

   x = x_new;


}  while ( fabs(cor) >= tol );


   //
   //  weight
   //

calc(x);

num = 2.0*(1.0 - x*x);

den = n*(P[n - 1]);

den *= den;

w = num/den;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


   //
   //  Reference: 
   // 
   //    Methods of Numerical Integration, 2nd Ed.
   //       by Philip J. Davis
   //      and Philip Rabinowitz
   //

double dr_first_guess(const int k, const int n)

{

double x, t;
const int K = k + 1;


t = (4.0*K - 1.0)/(4.0*n + 2.0);

x = (1.0 - 1.0/(8.0*n*n) + 1.0/(8.0*n*n*n))*cos(t*pi);


return ( x );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Reference:
   //
   //    On the Construction of Gauss-Legendre Quadrature Rules
   //
   //         by Frank G. Lether
   //
   //      J. Comput. Appl. Math. 4 (1978) 47--51
   //


double lether_first_guess(const int k, const int n)

{

double x, s;
double v_kn, alpha_n, a_n;
const int K = k + 1;
const double dn = (double) n;


v_kn = ((4.0*K - 1.0)/(4.0*dn + 2.0))*pi;

alpha_n = dn*dn + dn + 1.0/3.0;

a_n = 1.0 - 1.0/(8.0*dn*dn) + 1.0/(8.0*dn*dn*dn);


if ( K == 1 )  {

   x = J1/sqrt(alpha_n);

   x *= 1.0 - (J1*J1 - 2.0)/(360.0*alpha_n*alpha_n);

   x = cos(x);

} else if ( K <= (n/3) )  {

   s = sin(v_kn);

   x = 39.0 - 28.0/(s*s);

   x = a_n - x/(384.0*dn*dn*dn*dn);

   x = x*cos(v_kn);

} else {

   x = a_n*cos(v_kn);

}

   //
   //  done
   //

return ( x );

}


////////////////////////////////////////////////////////////////////////



