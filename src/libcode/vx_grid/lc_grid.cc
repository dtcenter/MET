

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


using namespace std;


#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_math.h"
#include "vx_util.h"
#include "vx_log.h"
#include "lc_grid.h"


////////////////////////////////////////////////////////////////////////


static double     lc_func(double lat, double Cone);
static double lc_der_func(double lat, double Cone);

static double lc_inv_func(double   r, double Cone);

static void reduce(double &);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class LambertGrid
   //


////////////////////////////////////////////////////////////////////////


LambertGrid::LambertGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


LambertGrid::~LambertGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void LambertGrid::clear()

{

Lat_LL = 0.0;
Lon_LL = 0.0;

Lon_orient = 0.0;

Alpha = 0.0;

Cone = 0.0;

Bx = 0.0;
By = 0.0;

Nx = 0;
Ny = 0;

Name.clear();

memset(&Data, 0, sizeof(Data));

return;

}


////////////////////////////////////////////////////////////////////////


LambertGrid::LambertGrid(const LambertData & data)

{

clear();



Lat_LL = data.lat_pin;   //  temporarily
Lon_LL = data.lon_pin;   //  temporarily

reduce(Lon_LL);

Lon_orient = data.lon_orient;

reduce(Lon_orient);

Bx = 0.0;
By = 0.0;

Nx = data.nx;
Ny = data.ny;

Name = data.name;

   //
   //  calculate Cone constant
   //

if ( fabs(data.scale_lat_1 - data.scale_lat_2) < 1.0e-5 )  Cone = sind(data.scale_lat_1);
else {

   double t, b;

   t = cosd(data.scale_lat_1)/cosd(data.scale_lat_2);

   b = tand(45.0 - 0.5*(data.scale_lat_1))/tand(45.0 - 0.5*(data.scale_lat_2));

   Cone = log(t)/log(b);

}

   //
   //  calculate Alpha
   //

Alpha = (-1.0/lc_der_func(data.scale_lat_1, Cone))*((data.r_km)/(data.d_km));

   //
   //  Calculate Bx, By
   //

double r0, theta0;

r0 = lc_func(data.lat_pin, Cone);

theta0 = Cone*(Lon_orient - Lon_LL);

Bx = data.x_pin - Alpha*r0*sind(theta0);
By = data.y_pin + Alpha*r0*cosd(theta0);

xy_to_latlon(0.0, 0.0, Lat_LL, Lon_LL);

reduce(Lon_LL);

Data = data;

   //
   //  Done
   //

}


////////////////////////////////////////////////////////////////////////


double LambertGrid::f(double lat) const

{

return ( lc_func(lat, Cone) );

}


////////////////////////////////////////////////////////////////////////


double LambertGrid::df(double lat) const

{

return ( lc_der_func(lat, Cone) );

}


////////////////////////////////////////////////////////////////////////


void LambertGrid::latlon_to_xy(double lat, double lon, double & x, double & y) const

{

double r, theta;


reduce(lon);

r = lc_func(lat, Cone);

theta = Cone*(Lon_orient - lon);

x = Bx + Alpha*r*sind(theta);

y = By - Alpha*r*cosd(theta);

return;

}


////////////////////////////////////////////////////////////////////////


void LambertGrid::xy_to_latlon(double x, double y, double & lat, double & lon) const

{

double r, theta;

x = (x - Bx)/Alpha;
y = (y - By)/Alpha;

r = sqrt( x*x + y*y );

lat = lc_inv_func(r, Cone);

if ( fabs(r) < 1.0e-5 )  theta = 0.0;
else                     theta = atan2d(x, -y);   //  NOT atan2d(y, x);

lon = Lon_orient - theta/Cone;

reduce(lon);

return;

}


////////////////////////////////////////////////////////////////////////


int LambertGrid::nx() const

{

return ( Nx );

}


////////////////////////////////////////////////////////////////////////


int LambertGrid::ny() const

{

return ( Ny );

}


////////////////////////////////////////////////////////////////////////


ConcatString LambertGrid::name() const

{

return ( Name );

}


////////////////////////////////////////////////////////////////////////


void LambertGrid::uv_to_xy(double u, double v, double & x, double & y) const

{

x = Alpha*v + Bx;

y = -Alpha*u + By;

return;

}


////////////////////////////////////////////////////////////////////////


void LambertGrid::xy_to_uv(double x, double y, double & u, double & v) const

{

u = (x - Bx)/Alpha;

v = (y - By)/(-Alpha);

return;

}


////////////////////////////////////////////////////////////////////////


void LambertGrid::dump(ostream & out, int depth) const

{

Indent prefix(depth);



out << prefix << "Name       = ";

if ( Name.length() > 0 )  out << '\"' << Name << '\"';
else                      out << "(nul)\n";

out << '\n';

out << prefix << "Projection = Lambert Conformal\n";

out << prefix << "\n";

out << prefix << "Lat_LL     = " << Lat_LL << "\n";
out << prefix << "Lon_LL     = " << Lon_LL << "\n";

out << prefix << "\n";

out << prefix << "Alpha      = " << Alpha << "\n";
out << prefix << "Cone       = " << Cone  << "\n";

out << prefix << "\n";

out << prefix << "Bx         = " << Bx << "\n";
out << prefix << "By         = " << By << "\n";

out << prefix << "\n";

out << prefix << "Nx         = " << Nx << "\n";
out << prefix << "Ny         = " << Ny << "\n";


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString LambertGrid::serialize() const

{

ConcatString a;
char junk[256];

a << "Projection: Lambert Conformal";

a << " Nx: " << Nx;
a << " Ny: " << Ny;

sprintf(junk, " Lat_LL: %.3f", Lat_LL);   a << junk;
sprintf(junk, " Lon_LL: %.3f", Lon_LL);   a << junk;

sprintf(junk, " Lon_orient: %.3f", Lon_orient);   a << junk;

sprintf(junk, " Alpha: %.3f", Alpha);   a << junk;

sprintf(junk, " Cone: %.3f", Cone);   a << junk;

sprintf(junk, " Bx: %.4f", Bx);   a << junk;
sprintf(junk, " By: %.4f", By);   a << junk;

   //
   //  done
   //

return ( a );

}


////////////////////////////////////////////////////////////////////////


GridInfo LambertGrid::info() const

{

GridInfo i;

i.set(Data);

return ( i );

}


////////////////////////////////////////////////////////////////////////


double LambertGrid::rot_grid_to_earth(int x, int y) const

{

double lat, lon, angle;
double diff, hemi;


xy_to_latlon((double) x, (double) y, lat, lon);

diff = Lon_orient - lon;

// Figure out if the grid is in the northern or southern hemisphere
// by checking whether the first latitude (p1_deg -> Phi1_radians)
// is greater than zero
// NH -> hemi = 1, SH -> hemi = -1
// if(Phi1_radians < 0.0) hemi = -1.0;
// else                   hemi = 1.0;

   //
   //  assume northern hemisphere
   //

hemi = 1.0;

angle = diff*Cone*hemi;

return ( angle );

}


////////////////////////////////////////////////////////////////////////


bool LambertGrid::is_global() const

{

return ( false );

}


////////////////////////////////////////////////////////////////////////


void LambertGrid::shift_right(int N)

{

if ( N == 0 )  return;

mlog << Error
     << "\n\n  LambertGrid::shift_right(int) -> "
     << "shifting is not allowed for non-global grids\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


GridRep * LambertGrid::copy() const

{

LambertGrid * p = new LambertGrid (Data);

return ( p );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


double lc_func(double lat, double Cone)

{

double r;

r = tand(45.0 - 0.5*lat);

r = pow(r, Cone);

return ( r );

}


////////////////////////////////////////////////////////////////////////


double lc_inv_func(double r, double Cone)

{

double lat;

lat = 90.0 - 2.0*atand(pow(r, 1.0/Cone));

return ( lat );

}


////////////////////////////////////////////////////////////////////////


double lc_der_func(double lat, double Cone)

{

double a;

a = -(Cone/cosd(lat))*lc_func(lat, Cone);

return ( a );

}


////////////////////////////////////////////////////////////////////////


void reduce(double & angle)

{

angle -= 360.0*floor( (angle/360.0) + 0.5 );

return;

}


////////////////////////////////////////////////////////////////////////


Grid::Grid(const LambertData & data)

{

init_from_scratch();

set(data);


}


////////////////////////////////////////////////////////////////////////


void Grid::set(const LambertData & data)

{

clear();

rep = new LambertGrid (data);

if ( !rep )  {

   mlog << Error << "\nGrid::set(const LambertData &) -> memory allocation error\n\n";

   exit ( 1 );

}

}


////////////////////////////////////////////////////////////////////////



