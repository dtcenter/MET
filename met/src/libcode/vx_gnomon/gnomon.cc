// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research(UCAR)
// ** National Center for Atmospheric Research(NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


using namespace std;


////////////////////////////////////////////////////////////////////////


static const double lat0 =  40.0;

static const double lon0 = 105.0;


static const double mag_factor = 100.0;

static const double cf = 57.2957795130823208768;


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_log.h"
#include "trig.h"
#include "gnomon.h"


////////////////////////////////////////////////////////////////////////


static double e1x, e1y, e1z, e2x, e2y, e2z, e3x, e3y, e3z;

static int initialized = 0;


////////////////////////////////////////////////////////////////////////


static void initialize_constants();


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class GnomonicProjection
   //


////////////////////////////////////////////////////////////////////////


GnomonicProjection::GnomonicProjection()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


GnomonicProjection::~GnomonicProjection()

{

clear();

}


////////////////////////////////////////////////////////////////////////


GnomonicProjection::GnomonicProjection(const GnomonicProjection & g)

{

init_from_scratch();

assign(g);

}


////////////////////////////////////////////////////////////////////////


GnomonicProjection & GnomonicProjection::operator=(const GnomonicProjection & g)

{

if ( this == &g )  return ( * this );

assign(g);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void GnomonicProjection::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void GnomonicProjection::clear()

{

set_center(0.0, 0.0);

return;

}


////////////////////////////////////////////////////////////////////////


void GnomonicProjection::assign(const GnomonicProjection & g)

{

clear();

Ex = g.Ex;
Ey = g.Ey;
Ez = g.Ez;

Nx = g.Nx;
Ny = g.Ny;
Nz = g.Nz;

Ux = g.Ux;
Uy = g.Uy;
Uz = g.Uz;

return;

}


////////////////////////////////////////////////////////////////////////


void GnomonicProjection::set_center(double lat, double lon)

{

if ( fabs(lat) > 89.0 )  {

   mlog << Error << "\nGnomonicProjection::set_center() -> given center point is too close to the poles\n\n";

   exit ( 1 );

}

Ux =  cosd(lat)*sind(lon);
Uy =  cosd(lat)*cosd(lon);
Uz =  sind(lat);

Nx = -sind(lat)*sind(lon);
Ny = -sind(lat)*cosd(lon);
Nz =  cosd(lat);

Ex = -cosd(lon);
Ey =  sind(lon);
Ez =  0.0;


return;

}


////////////////////////////////////////////////////////////////////////


int GnomonicProjection::latlon_to_uv(double lat, double lon, double & u, double & v) const

{

double px, py, pz;
double p_dot_b1, p_dot_b2, p_dot_b3;

px =  cosd(lat)*sind(lon);
py =  cosd(lat)*cosd(lon);
pz =  sind(lat);

p_dot_b3 = px*Ux + py*Uy + pz*Uz;

if ( p_dot_b3 <= 0.0 )  return ( 0 );

p_dot_b1 = px*Ex + py*Ey + pz*Ez;
p_dot_b2 = px*Nx + py*Ny + pz*Nz;

u = p_dot_b1/p_dot_b3;

v = p_dot_b2/p_dot_b3;


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


void gnomon_latlon_to_xy(double lat, double lon, double &x, double &y)

{

if ( !initialized )  initialize_constants();

lat /= cf;
lon /= cf;

double qx, qy, qz;
double q_dot_e1, q_dot_e2, q_dot_e3;

qx = cos(lat)*sin(lon);
qy = cos(lat)*cos(lon);
qz = sin(lat);

q_dot_e1 = qx*e1x + qy*e1y + qz*e1z;
q_dot_e2 = qx*e2x + qy*e2y + qz*e2z;
q_dot_e3 = qx*e3x + qy*e3y + qz*e3z;

x = mag_factor*(q_dot_e1/q_dot_e3);
y = mag_factor*(q_dot_e2/q_dot_e3);

return;

}


////////////////////////////////////////////////////////////////////////


void gnomon_xy_to_latlon(double x, double y, double &lat, double &lon)

{

if ( !initialized )  initialize_constants();

x /= mag_factor;
y /= mag_factor;

double qx, qy, qz;
double norm;

qx = x*e1x + y*e2x + e3x;
qy = x*e1y + y*e2y + e3y;
qz = x*e1z + y*e2z + e3z;

norm = 1.0/sqrt( qx*qx + qy*qy + qz*qz );

qx *= norm;
qy *= norm;
qz *= norm;

lat = cf*asin(qz);

if ( (fabs(qx) + fabs(qy)) < 1.0e-5 )
   lon = 0.0;
else
   lon = cf*atan2(qx, qy);

return;

}


////////////////////////////////////////////////////////////////////////


void initialize_constants()

{

double lat, lon;
double sp, cp, sl, cl;

lat = lat0/cf;
lon = lon0/cf;

sp = sin(lat);
cp = cos(lat);

sl = sin(lon);
cl = cos(lon);

e1x = -cl;
e1y =  sl;
e1z = 0.0;

e2x = -sp*sl;
e2y = -sp*cl;
e2z = cp;

e3x = cp*sl;
e3y = cp*cl;
e3z = sp;

initialized = 1;

return;

}


////////////////////////////////////////////////////////////////////////
