// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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
#include <cstdio>
#include <cmath>

#include "vx_math.h"
#include "vx_util.h"
#include "vx_log.h"

#include "laea_grid.h"
#include "latlon_xyz.h"


////////////////////////////////////////////////////////////////////////


static void laea_reduce(double & angle);

static double laea_segment_area(double u0, double v0, double u1, double v1);


////////////////////////////////////////////////////////////////////////


static double laea_r(double lat_deg);

static double laea_r_inv(double r);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class LaeaGrid
   //


////////////////////////////////////////////////////////////////////////


LaeaGrid::LaeaGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


LaeaGrid::~LaeaGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


LaeaGrid::LaeaGrid(const LaeaData & data)

{

clear();

Nx = data.nx;
Ny = data.ny;
 
Name = data.name;

Data = data;

if ( strcmp(data.geoid, "WGS_84") == 0 )  {

   geoid.set_ab(6378.137, 6356.752);

   geoid.set_name("WGS_84");

} else {

   mlog << Error << "\nLaeaGrid::LaeaGrid(const LaeaData &) -> "
        << "unrecognized geoid ... \"" << data.geoid << "\"\n\n";

   exit ( 1 );

}

calc_aff();

}


////////////////////////////////////////////////////////////////////////


void LaeaGrid::calc_aff()

{

double u_ll, v_ll;
double u_lr, v_lr;
double u_ul, v_ul;

double x_ll, y_ll;
double x_lr, y_lr;
double x_ul, y_ul;


snyder_latlon_to_xy(Data.lat_ll, Data.lon_ll, u_ll, v_ll);
snyder_latlon_to_xy(Data.lat_lr, Data.lon_lr, u_lr, v_lr);
snyder_latlon_to_xy(Data.lat_ul, Data.lon_ul, u_ul, v_ul);


x_ll = 0.0;
y_ll = 0.0;

x_lr = Data.nx - 1.0;
y_lr = 0.0;

x_ul = 0.0;
y_ul = Data.ny  - 1.0;

aff.set_three_points(

   u_ll, v_ll, u_lr, v_lr, u_ul, v_ul,

   x_ll, y_ll, x_lr, y_lr, x_ul, y_ul

);


return;

}


////////////////////////////////////////////////////////////////////////


void LaeaGrid::clear()

{

aff.clear();

Nx = 0;
Ny = 0;

Name.clear();

geoid.clear();

memset(&Data, 0, sizeof(Data));

return;

}


////////////////////////////////////////////////////////////////////////


void LaeaGrid::latlon_to_xy(double lat, double lon, double & x, double & y) const

{

double u, v;


snyder_latlon_to_xy(lat, lon, u, v);


uv_to_xy(u, v, x, y);


return;

}


////////////////////////////////////////////////////////////////////////


void LaeaGrid::xy_to_latlon(double x, double y, double & lat, double & lon) const

{

double u, v, uu, vv;
double D, Rq, rho, beta, beta1, lambda0, lat1, ce, m1;
double num, denom, cor;
double s2, s4, s6, t2, t4, t6;
const double E  = geoid.e();
const double A  = geoid.a_km();
const double Qp = geoid.qp_direct();
const double E2 = E*E;
const double E4 = E2*E2;
const double E6 = E2*E4;


xy_to_uv(x, y, u, v);



lat1    = (Data.lat_pole);

lambda0 = -(Data.lon_pole);


beta1 = geoid.beta(lat1);

m1 = geoid.m_func(lat1);

Rq = A*sqrt(0.5*Qp);           //  Eq 3-13, page 187

D = (A*m1)/(Rq*cosd(beta1));   //  Eq 24-20, page 187

uu = u/D;

vv = D*v;

rho = sqrt( uu*uu + vv*vv );   //  Eq 24-28, page 189

ce = 2.0*asind(rho/(2.0*Rq));  //  Eq 24-29, page 189


beta = asind( cosd(ce)*sind(beta1) + ((D*v)/rho)*sind(ce)*cosd(beta1) );   //  Eq 24-30, page 189

s2 = sind(2.0*beta);
s4 = sind(4.0*beta);
s6 = sind(6.0*beta);

num = u*sind(ce);

denom = D*rho*cosd(beta1)*cosd(ce) - D*D*v*sind(beta1)*sind(ce);

lon = lambda0 + atan2d(num, denom);   //  Eq 24-26, page 188

lon = -lon;


t2 = E2/3.0 + (31.0*E4)/180.0 + (517.0*E6)/5040.0;

t4 = (23.0*E4)/360.0 + (251.0*E6)/3780.0;

t6 = (761.0*E6)/45360.0;


cor = t2*s2 + t4*s4 + t6*s6;   //  Eq 3-18, page 189

lat = beta + cor*deg_per_rad;



return;

}


////////////////////////////////////////////////////////////////////////


double LaeaGrid::calc_area(int x, int y) const

{

double u[4], v[4];
double sum;


xy_to_uv(x - 0.5, y - 0.5, u[0], v[0]);  //  lower left
xy_to_uv(x + 0.5, y - 0.5, u[1], v[1]);  //  lower right
xy_to_uv(x + 0.5, y + 0.5, u[2], v[2]);  //  upper right
xy_to_uv(x - 0.5, y + 0.5, u[3], v[3]);  //  upper left


sum = uv_closedpolyline_area(u, v, 4);

sum *= earth_radius_km*earth_radius_km;

return ( sum );

}


////////////////////////////////////////////////////////////////////////


int LaeaGrid::nx() const

{

return ( Nx );

}


////////////////////////////////////////////////////////////////////////


int LaeaGrid::ny() const

{

return ( Ny );

}


////////////////////////////////////////////////////////////////////////


ConcatString LaeaGrid::name() const

{

return ( Name );

}


////////////////////////////////////////////////////////////////////////


const char * LaeaGrid::projection_name() const

{

return ( "Polar Laea" );

}


////////////////////////////////////////////////////////////////////////


double LaeaGrid::uv_closedpolyline_area(const double *u, const double *v, int n) const

{

int j, k;
double sum;


sum = 0.0;

for (j=0; j<n; ++j)  {

   k = (j + 1)%n;

   sum += laea_segment_area(u[j], v[j], u[k], v[k]);

}   //  for j

sum = fabs(sum);

return ( sum );

}


////////////////////////////////////////////////////////////////////////


double LaeaGrid::xy_closedpolyline_area(const double *x, const double *y, int n) const

{

int j;
double sum;
double *u = (double *) 0;
double *v = (double *) 0;

u = new double [n];
v = new double [n];

if ( !u || !v )  {

   mlog << Error << "\nLaeaGrid::xy_closedpolyline_area() -> "
        << "memory allocation error\n\n";

   exit ( 1 );

}

for (j=0; j<n; ++j)  {

   xy_to_uv(x[j], y[j], u[j], v[j]);

}

sum = uv_closedpolyline_area(u, v, n);

sum *= earth_radius_km*earth_radius_km;

delete [] u;  u = (double *) 0;
delete [] v;  v = (double *) 0;

return ( sum );

}


////////////////////////////////////////////////////////////////////////


void LaeaGrid::uv_to_xy(double u, double v, double &x, double &y) const

{

aff.forward(u, v, x, y);

return;

}


////////////////////////////////////////////////////////////////////////


void LaeaGrid::xy_to_uv(double x, double y, double &u, double &v) const

{

aff.reverse(x, y, u, v);

return;

}


////////////////////////////////////////////////////////////////////////


void LaeaGrid::dump(ostream & out, int depth) const

{

Indent prefix(depth);



out << prefix << "Name       = ";

if ( Name.length() > 0 )  out << '\"' << Name << '\"';
else                      out << "(nul)\n";

out << '\n';   //  no prefix

out << prefix << "Projection = Laea\n";

out << prefix << "Nx         = " << comma_string(Nx) << "\n";

out << prefix << "Ny         = " << comma_string(Ny) << "\n";

// aff.dump(out, depth + 1);


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////

ConcatString LaeaGrid::serialize(const char *sep) const

{

ConcatString a;
char junk[256];

a << "Projection: Lambert Azimuthal Equal Area" << sep;

a << "Nx: " << Nx << sep;
a << "Ny: " << Ny << sep;

a << "geoid: " << Data.geoid << sep;

snprintf(junk, sizeof(junk), "lat_ll: %.3f", Data.lat_ll);   a << junk << sep;
snprintf(junk, sizeof(junk), "lon_ll: %.3f", Data.lon_ll);   a << junk << sep;

snprintf(junk, sizeof(junk), "lat_ul: %.3f", Data.lat_ul);   a << junk << sep;
snprintf(junk, sizeof(junk), "lon_ul: %.3f", Data.lon_ul);   a << junk << sep;

snprintf(junk, sizeof(junk), "lat_lr: %.3f", Data.lat_lr);   a << junk << sep;
snprintf(junk, sizeof(junk), "lon_lr: %.3f", Data.lon_lr);   a << junk << sep;

snprintf(junk, sizeof(junk), "lat_pole: %.3f", Data.lat_pole);   a << junk << sep;
snprintf(junk, sizeof(junk), "lon_pole: %.3f", Data.lon_pole);   a << junk << sep;

   //
   //  done
   //

return ( a );

}


////////////////////////////////////////////////////////////////////////


void LaeaGrid::deserialize(const StringArray &)

{

mlog << Error << "\nLaeaGrid::deserialize(const StringArray &) -> "
     << " not yet implemented\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


GridInfo LaeaGrid::info() const

{

GridInfo i;

i.set(Data);

return ( i );

}


////////////////////////////////////////////////////////////////////////


double LaeaGrid::rot_grid_to_earth(int x, int y) const

{

   //
   //  grid to earth transformation is not just a simple rotation
   //

return ( 0.0 );

}


////////////////////////////////////////////////////////////////////////


bool LaeaGrid::wrap_lon() const

{

return ( false );

}


////////////////////////////////////////////////////////////////////////


GridRep * LaeaGrid::copy() const

{

LaeaGrid * p = new LaeaGrid (Data);

return ( p );

}


////////////////////////////////////////////////////////////////////////


double LaeaGrid::snyder_m_func(double lat) const

{

double z;
const double  C = cosd(lat);
const double  S = sind(lat);
const double  E = geoid.e();
const double es = E*S;


z = sqrt(1.0 - es*es);

z = C/z;



return ( z );

}


////////////////////////////////////////////////////////////////////////


void LaeaGrid::snyder_latlon_to_xy(double lat, double lon, double & x_snyder, double & y_snyder) const

{

double A, B, D, Qp, Rq;
double beta1, beta;
double m1, lambda, lambda0, lat1, delta;

A = geoid.a_km();

lambda  = -lon;

lambda0 = -(Data.lon_pole);
lat1    =  (Data.lat_pole);


delta   =  lambda - lambda0;


beta1 = geoid.beta(lat1);

beta  = geoid.beta(lat);


Qp = geoid.qp_direct();

Rq = A*sqrt(0.5*Qp);


m1 = snyder_m_func(lat1);


B = 1.0 + sind(beta1)*sind(beta) + cosd(beta1)*cosd(beta)*cosd(delta);

B = sqrt(2.0/B);

B = Rq*B;


D = Rq*cosd(beta1);

D = (A*m1)/D;


x_snyder = cosd(beta)*sind(delta);

x_snyder *= B*D;



y_snyder = cosd(beta1)*sind(beta) - sind(beta1)*cosd(beta)*cosd(delta);

y_snyder *= B/D;



return;

}


////////////////////////////////////////////////////////////////////////


double LaeaGrid::scale_km() const

{

return ( 0.0 );

}


////////////////////////////////////////////////////////////////////////


bool LaeaGrid::is_global() const

{

return ( false );

}


////////////////////////////////////////////////////////////////////////


void LaeaGrid::shift_right(int)

{

mlog << Error << "\nLaeaGrid::shift_right(int) -> "
     << "not yet implemented!\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void laea_reduce(double & angle)

{

angle -= 360.0*floor( (angle/360.0) + 0.5 );

return;

}


////////////////////////////////////////////////////////////////////////


double laea_segment_area(double u0, double v0, double u1, double v1)
 
{
 
double answer;
 
answer = 0.5*( u0*v1 - v0*u1 );
                                                   
return ( answer );                                 
                                                   
}

////////////////////////////////////////////////////////////////////////


double laea_r(double lat_deg)

{

double t, r;

t = 2.0*(1.0 - sind(lat_deg));

r = sqrt(t);

return ( r );

}


////////////////////////////////////////////////////////////////////////


double laea_r_inv(double r)

{

double t, lat_deg;

t = 1.0 - 0.5*r*r;

lat_deg = asind(t);

return ( lat_deg );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Grid functions
   //


////////////////////////////////////////////////////////////////////////


Grid::Grid(const LaeaData & data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const LaeaData & data)

{

clear();

rep = new LaeaGrid (data);

if ( !rep )  {

   mlog << Error << "\nGrid::set(const LaeaData &) -> "
        << "memory allocation error\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////
