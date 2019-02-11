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

#include "2d_moments.h"
#include "trig.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Mtd_2D_Moments
   //


////////////////////////////////////////////////////////////////////////


Mtd_2D_Moments::Mtd_2D_Moments()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Mtd_2D_Moments::~Mtd_2D_Moments()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Mtd_2D_Moments::Mtd_2D_Moments(const Mtd_2D_Moments & m)

{

init_from_scratch();

assign(m);

}


////////////////////////////////////////////////////////////////////////


Mtd_2D_Moments & Mtd_2D_Moments::operator=(const Mtd_2D_Moments & m)

{

if ( this == &m )  return ( * this );

assign(m);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Mtd_2D_Moments::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_2D_Moments::clear()

{

N = 0.0;

Sx = Sy = 0.0;

Sxx = Sxy = Syy = 0.0;

IsCentralized = false;

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_2D_Moments::assign(const Mtd_2D_Moments & m)

{

clear();

N = m.N;

Sx = m.Sx;
Sy = m.Sy;

Sxx = m.Sxx;
Sxy = m.Sxy;
Syy = m.Syy;

IsCentralized = m.IsCentralized;


return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_2D_Moments::add(double x, double y)

{

++N;

Sx += x;
Sy += y;

Sxx += x*x;
Sxy += x*y;
Syy += y*y;

IsCentralized = false;

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_2D_Moments::centralize()

{

if ( N == 0 )  {

   mlog << Error << "\n\n  Mtd_2D_Moments::centralize() -> no data!\n\n";

   exit ( 1 );

}

if ( IsCentralized )  return;

const double X_bar = Sx/N;
const double Y_bar = Sy/N;

const double sxx_old = Sxx;
const double sxy_old = Sxy;
const double syy_old = Syy;



Sxx = sxx_old - N*X_bar*X_bar;

Syy = syy_old - N*Y_bar*Y_bar;

Sxy = sxy_old - N*X_bar*Y_bar;

   //
   //  done
   //

IsCentralized = true;

return;

}


////////////////////////////////////////////////////////////////////////


double Mtd_2D_Moments::calc_2D_axis_plane_angle() const

{

if ( ! IsCentralized )  {

   mlog << Error << "\n\n  Mtd_2D_Moments::calc_2D_axis_plane_angle() const -> moments must be centralized first!\n\n";

   exit ( 1 );

}

double angle, rho_1, rho_2;

rho_1 = (Sxx - Syy)/2.0;

rho_2 = Sxy;

angle = 0.5*atan2d(rho_2, rho_1);

return ( angle );

}


////////////////////////////////////////////////////////////////////////





