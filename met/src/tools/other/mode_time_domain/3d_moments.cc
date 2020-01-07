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

#include "3d_moments.h"
#include "trig.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Mtd_3D_Moments
   //


////////////////////////////////////////////////////////////////////////


Mtd_3D_Moments::Mtd_3D_Moments()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Mtd_3D_Moments::~Mtd_3D_Moments()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Mtd_3D_Moments::Mtd_3D_Moments(const Mtd_3D_Moments & m)

{

init_from_scratch();

assign(m);

}


////////////////////////////////////////////////////////////////////////


Mtd_3D_Moments & Mtd_3D_Moments::operator=(const Mtd_3D_Moments & m)

{

if ( this == &m )  return ( * this );

assign(m);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Mtd_3D_Moments::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_3D_Moments::clear()

{

N = 0.0;

Sx = Sy = St = 0.0;

Sxx = Sxy = Sxt = Syy = Syt = Stt = 0.0;

IsCentralized = false;

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_3D_Moments::assign(const Mtd_3D_Moments & m)

{

clear();

N = m.N;

Sx = m.Sx;
Sy = m.Sy;
St = m.St;

Sxx = m.Sxx;
Sxy = m.Sxy;
Sxt = m.Sxt;
Syy = m.Syy;
Syt = m.Syt;
Stt = m.Stt;

IsCentralized = m.IsCentralized;


return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_3D_Moments::add(double x, double y, double t)

{

++N;

Sx += x;
Sy += y;
St += t;

Sxx += x*x;
Sxy += x*y;
Sxt += x*t;

Syy += y*y;
Syt += y*t;

Stt += t*t;

IsCentralized = false;

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_3D_Moments::centralize()

{

if ( N == 0 )  {

   mlog << Error << "\n\n  Mtd_3D_Moments::centralize() -> no data!\n\n";

   exit ( 1 );

}

if ( IsCentralized )  return;

const double X_bar = Sx/N;
const double Y_bar = Sy/N;
const double T_bar = St/N;

const double sxx_old = Sxx;
const double sxy_old = Sxy;
const double sxt_old = Sxt;

const double syy_old = Syy;
const double syt_old = Syt;

const double stt_old = Stt;


Sxx = sxx_old - N*X_bar*X_bar;

Syy = syy_old - N*Y_bar*Y_bar;

Stt = stt_old - N*T_bar*T_bar;


Sxy = sxy_old - N*X_bar*Y_bar;

Sxt = sxt_old - N*X_bar*T_bar;

Syt = syt_old - N*Y_bar*T_bar;

   //
   //  done
   //

IsCentralized = true;

return;

}


////////////////////////////////////////////////////////////////////////


void Mtd_3D_Moments::calc_3d_velocity(double & vx, double & vy) const

{

if ( ! IsCentralized )  {

   mlog << Error << "\n\n  Mtd_3D_Moments::calc_3d_velocity(double &, double &) const -> moments must be centralized first!\n\n";

   exit ( 1 );

}

vx = Sxt/Stt;

vy = Syt/Stt;

return;

}


////////////////////////////////////////////////////////////////////////


double Mtd_3D_Moments::calc_3d_axis_plane_angle() const

{

if ( ! IsCentralized )  {

   mlog << Error << "\n\n  Mtd_3D_Moments::calc_3d_axis_plane_angle() const -> moments must be centralized first!\n\n";

   exit ( 1 );

}

double angle, top, bot;

top = 2.0*(Sxy*Stt - Sxt*Syt);

bot = (Sxx - Syy)*Stt + Syt*Syt - Sxt*Sxt;

angle = 0.5*atan2d(top, bot);

return ( angle );

}


////////////////////////////////////////////////////////////////////////





