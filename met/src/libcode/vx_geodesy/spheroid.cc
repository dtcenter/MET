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
#include <string.h>
#include <cmath>

#include "spheroid.h"
#include "vx_util.h"
#include "vx_math.h"



////////////////////////////////////////////////////////////////////////


   //
   //  external lingage for these
   //


   //
   //  This is the one Jean Meeus uses in all his books
   //

const Spheroid Meeus  ("Meeus",  default_semimajor, default_semiminor);

   //
   //  spherical earth
   //

const Spheroid Gsphere("Sphere", default_semimajor, default_semimajor);

   //
   //  the following was obtained from 
   //
   //     www.gfy.ku.dk/~iag/handbook/geodeti.htm
   //

const Spheroid IUGG_1980("IUGG_1980", 6378.137, 6356.7523141);

   //
   //  the following were obtained from Table 1 (page 68) of 
   //
   //      "Map Projections: Theory and Applications"
   //            by Frederick Pearson II
   //            CRC Press, 1990
   //            ISBN 0-8493-6888-X
   //

const Spheroid Everest     ("Everest",     6377.304, 6356.103);
const Spheroid Bessel      ("Bessel",      6377.397, 6356.082);
const Spheroid Airy        ("Airy",        6377.563, 6356.300);
const Spheroid Clarke_1858 ("Clarke_1858", 6378.294, 6356.621);
const Spheroid Clarke_1866 ("Clarke_1866", 6378.206, 6356.585);
const Spheroid Clarke_1880 ("Clarke_1880", 6378.249, 6356.517);
const Spheroid Hayford     ("Hayford",     6378.388, 6356.912);
const Spheroid Krasovski   ("Krasovski",   6378.245, 6356.863);
const Spheroid Hough       ("Hough",       6378.270, 6356.794);
const Spheroid Fischer_60  ("Fischer_60",  6378.166, 6356.784);
const Spheroid Kaula       ("Kaula",       6378.165, 6356.345);
const Spheroid IUGG_67     ("IUGG_67",     6378.160, 6356.775);
const Spheroid Fischer_68  ("Fischer_68",  6378.150, 6356.330);
const Spheroid WGS_72      ("WGS_72",      6378.135, 6356.751);
const Spheroid IUGG_75     ("IUGG_75",     6378.140, 6356.755);
const Spheroid WGS_84      ("WGS_84",      6378.137, 6356.752);


////////////////////////////////////////////////////////////////////////


static double ab_to_e(double a, double b);
static double ab_to_f(double a, double b);

static double phi_to_u(double a, double b, double phi);
static double u_to_phi(double a, double b, double u);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Spheroid
   //


////////////////////////////////////////////////////////////////////////


Spheroid::Spheroid()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Spheroid::Spheroid(const char * _name_, double _a_, double _b_)

{

init_from_scratch();

set_ab(_a_, _b_);

set_name(_name_);

}



////////////////////////////////////////////////////////////////////////


Spheroid::~Spheroid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Spheroid::Spheroid(const Spheroid & g)

{

init_from_scratch();

assign(g);

}


////////////////////////////////////////////////////////////////////////


Spheroid & Spheroid::operator=(const Spheroid & g)

{

if ( this == &g )  return ( * this );

assign(g);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Spheroid::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Spheroid::clear()

{

A_km = B_km = default_semimajor;

F = E = 0.0;

Name.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Spheroid::assign(const Spheroid & g)

{

clear();

A_km = g.A_km;

B_km = g.B_km;

E = g.E;

F = g.F;

set_name(g.Name.c_str());

return;

}


////////////////////////////////////////////////////////////////////////


void Spheroid::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Name = ";

if ( Name.nonempty() )  out << '\"' << Name << '\"' << "\n";
else                    out << "(nul)\b";

out << prefix << "A_km = " << A_km << "\n";
out << prefix << "B_km = " << B_km << "\n";
out << prefix << "F    = " << F    << "\n";
out << prefix << "E    = " << E    << "\n";


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void Spheroid::set_ab(double semimajor_km, double semiminor_km)

{

clear();

A_km = semimajor_km;

B_km = semiminor_km;

E = ab_to_e(A_km, B_km);

F = ab_to_f(A_km, B_km);



return;

}


////////////////////////////////////////////////////////////////////////


void Spheroid::set_af(double semimajor_km, double Flattening)

{

A_km = semimajor_km;

F = Flattening;

B_km = A_km*( 1.0 - F );

E = ab_to_e(A_km, B_km);

return;

}


////////////////////////////////////////////////////////////////////////


void Spheroid::set_name(const char * text)

{

Name = text;

return;

}


////////////////////////////////////////////////////////////////////////


void Spheroid::geographic_to_geocentric(double lat, double h_km, double & rho_km, double & phi_prime) const

{

double u;
double rcp, rsp;




u = phi_to_u(A_km, B_km, lat);


rsp = B_km*sind(u) + h_km*sind(lat);

rcp = A_km*cosd(u) + h_km*cosd(lat);


phi_prime = atan2d(rsp, rcp);

rho_km = sqrt ( rcp*rcp + rsp*rsp );


return;

}


////////////////////////////////////////////////////////////////////////


void Spheroid::geocentric_to_geographic(double rho_km, double phi_prime, double & lat, double & h_km) const

{

double alpha, beta;
double u, u_rad, func, dfunc;
double cu, su, cor;
double x0, y0, dx, dy;
const double tol = 1.0e-7;


alpha = rho_km*cosd(phi_prime);
beta  = rho_km*sind(phi_prime);

u_rad = rad_per_deg*phi_prime;

do {

   cu = cos(u_rad);
   su = sin(u_rad);

   func  = A_km*alpha*su - B_km*beta*cu + (B_km*B_km - A_km*A_km)*cu*su;

   dfunc = A_km*alpha*cu + B_km*beta*su + (B_km*B_km - A_km*A_km)*(cu*cu - su*su);

   cor = func/dfunc;

   u_rad -= cor;


}  while ( fabs(cor) >= tol );


u = deg_per_rad*u_rad;

lat = u_to_phi(A_km, B_km, u);


x0 = A_km*cosd(u);
y0 = B_km*sind(u);


dx = alpha - x0;
dy = beta  - y0;

h_km = sqrt( dx*dx + dy*dy );

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


double ab_to_e(double a, double b)

{

double e, t;


t = b/a;

e = sqrt( 1.0 - t*t );


return ( e );

}


////////////////////////////////////////////////////////////////////////


double ab_to_f(double a, double b)

{

double t, f;


t = b/a;

f = 1.0 - t;


return ( f );

}


////////////////////////////////////////////////////////////////////////


double phi_to_u(double a, double b, double phi)

{

double u;


u = atan2d(b*sind(phi), a*cosd(phi));


return ( u );

}


////////////////////////////////////////////////////////////////////////


double u_to_phi(double a, double b, double u)

{

double phi;


phi = atan2d(a*sind(u), b*cosd(u));


return ( phi );

}


////////////////////////////////////////////////////////////////////////





