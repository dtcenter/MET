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

#include "vx_geodesy.h"
#include "vx_math.h"


////////////////////////////////////////////////////////////////////////


static void test_poles(const Spheroid &);


////////////////////////////////////////////////////////////////////////


int main()

{

static const Spheroid g = Meeus;
double lat, phi_prime, rho, H;
double rcp, rsp;

   //
   //  example taken from "Astronomical Algorithms", by
   //
   //    Jean Meeus, 1st edition
   //
   //    Example 10.a, page 78
   //

H = 1.706;

lat = 33.0 + (21.0/60.0) + (22.0/3600.0);

g.geographic_to_geocentric(lat, H, rho, phi_prime);

rcp = (rho*cosd(phi_prime))/(g.a_km());
rsp = (rho*sind(phi_prime))/(g.a_km());

cout << "\n\n"
     << "rsp = " << rsp << "\n"
     << "\n"
     << "rcp = " << rcp << "\n"
     << "\n\n";


   //
   //  convert back
   //

lat = H = 0.0;

g.geocentric_to_geographic(rho, phi_prime, lat, H);

cout << "\n\n"
     << "lat = " << lat << "\n"
     << "\n"
     << "H   = " << H   << "\n"
     << "\n\n";



test_poles(Meeus);
test_poles(Gsphere);






return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void test_poles(const Spheroid & g)

{

double lat, H, phi_prime, rho;

   //
   //  north pole
   //

lat = 90.0;
H   = 1.0;

g.geographic_to_geocentric(lat, H, rho, phi_prime);

cout << "\n\n"
     << "North Pole test for geoid " << g.name() << "\n"
     << "\n"
     << "rho = " << rho << "\n"
     << "\n"
     << "phi_prime = " << phi_prime << "\n"
     << "\n";

lat = H = 0.0;

g.geocentric_to_geographic(rho, phi_prime, lat, H);

cout << "lat = " << lat << "\n"
     << "\n"
     << "H   = " << H   << "\n"
     << "\n\n";


   //
   //  South pole
   //

lat = -90.0;
H   = 1.0;

g.geographic_to_geocentric(lat, H, rho, phi_prime);

cout << "\n\n"
     << "South Pole test for geoid " << g.name() << "\n"
     << "\n"
     << "rho = " << rho << "\n"
     << "\n"
     << "phi_prime = " << phi_prime << "\n"
     << "\n";

lat = H = 0.0;

g.geocentric_to_geographic(rho, phi_prime, lat, H);

cout << "lat = " << lat << "\n"
     << "\n"
     << "H   = " << H   << "\n"
     << "\n\n";

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


