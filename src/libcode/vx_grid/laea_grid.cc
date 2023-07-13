// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "vx_math.h"
#include "vx_util.h"
#include "vx_geodesy.h"

#include "laea_grid.h"
#include "latlon_xyz.h"


using namespace std;


////////////////////////////////////////////////////////////////////////


static double laea_segment_area(double u0, double v0, double u1, double v1);


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

memset(&Data, 0, sizeof(Data));
Data = data;

lat_LL = data.lat_first;
lon_LL = data.lon_first;

lat_pole = data.standard_lat;
lon_pole = data.central_lon;

Name = data.name;
SpheroidName = data.spheroid_name;
Data.spheroid_name = SpheroidName.c_str();

Nx = data.nx;
Ny = data.ny;

geoid.set_ab(data.equatorial_radius_km, data.polar_radius_km);

geoid.set_name(data.spheroid_name);

double s = 1.0;

aff.set_mb(s, 0.0, 0.0, s, 1.0, 1.0);

double xx, yy;

latlon_to_xy(data.lat_first, data.lon_first, xx, yy);
aff.set_pin(xx, yy, 0.0, 0.0);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


LaeaGrid::LaeaGrid(const LaeaNetcdfData & nc)

{

double u, v, lat, lon;
const double tol = 1.0e-5;

clear();


Data.name                 = nc.name;

Data.radius_km            = 0.0;

Data.is_sphere            = false;

Data.equatorial_radius_km = nc.semi_major_axis_km;
Data.polar_radius_km      = nc.semi_minor_axis_km;

Data.dx_km                = nc.dx_km;
Data.dy_km                = nc.dy_km;

Data.standard_lat         = nc.proj_origin_lat;
Data.central_lon          = nc.proj_origin_lon;

Data.nx                   = nc.nx;
Data.ny                   = nc.ny;

Nx = Data.nx;
Ny = Data.ny;

Name = Data.name;

if ( fabs((nc.semi_major_axis_km - nc.semi_minor_axis_km)/(nc.semi_major_axis_km)) < tol )  {

   Data.radius_km = nc.semi_major_axis_km;

   Data.is_sphere = true;

}

geoid.set_ab(Data.equatorial_radius_km, Data.polar_radius_km);

geoid.set_name("WGS_84");

aff.set_mb(1.0/(Data.dx_km), 0.0, 0.0, 1.0/(Data.dy_km), 0.0, 0.0);

latlon_to_xy(nc.proj_origin_lat, nc.proj_origin_lon, u, v);

aff.set_translation(nc.x_pin - u, nc.y_pin - v);

latlon_to_xy(nc.proj_origin_lat, nc.proj_origin_lon, u, v);

      ////////////////////////

xy_to_latlon(0.0, 0.0, lat, lon);

Data.lat_first = lat;
Data.lon_first = lon;

xy_to_latlon(Nx - 1.0, 0.0, lat, lon);

lat_LR = lat;
lon_LR = lon;

xy_to_latlon(0.0, Ny - 1.0, lat, lon);

lat_UL = lat;
lon_UL = lon;

   //
   //  done
   //

}


////////////////////////////////////////////////////////////////////////


void LaeaGrid::clear()

{

aff.clear();

Nx = 0;
Ny = 0;

Name.clear();
SpheroidName.clear();

geoid.clear();

lat_LL = 0.0;
lon_LL = 0.0;

lat_UL = 0.0;
lon_UL = 0.0;

lat_LR = 0.0;
lon_LR = 0.0;

lat_pole = 0.0;

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

lat = lon = 0.0;

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

lat1    = lat_pole;

lambda0 = -lon_pole;

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

lon = lambda0 + atan2d(num, denom);   // Eq 24-26, page 188

lon = -lon;

t2 = E2/3.0 + (31.0*E4)/180.0 + (517.0*E6)/5040.0;

t4 = (23.0*E4)/360.0 + (251.0*E6)/3780.0;

t6 = (761.0*E6)/45360.0;

cor = t2*s2 + t4*s4 + t6*s6;   //  Eq 3-18, page 189

lat = beta + cor*deg_per_rad;

   //
   //  done
   //

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

return sum;

}


////////////////////////////////////////////////////////////////////////


int LaeaGrid::nx() const

{

return Nx;

}


////////////////////////////////////////////////////////////////////////


int LaeaGrid::ny() const

{

return Ny;

}


////////////////////////////////////////////////////////////////////////


ConcatString LaeaGrid::name() const

{

return Name;

}


////////////////////////////////////////////////////////////////////////


ConcatString LaeaGrid::spheroid_name() const

{

return SpheroidName;

}


////////////////////////////////////////////////////////////////////////


const char * LaeaGrid::projection_name() const

{

return "Laea";

}


////////////////////////////////////////////////////////////////////////


double LaeaGrid::uv_closedpolyline_area(const double *u, const double *v, int n) const

{

int k;
double sum;


sum = 0.0;

for (int j=0; j<n; ++j)  {

   k = (j + 1)%n;

   sum += laea_segment_area(u[j], v[j], u[k], v[k]);

}   //  for j

sum = fabs(sum);

return sum;

}


////////////////////////////////////////////////////////////////////////


double LaeaGrid::xy_closedpolyline_area(const double *x, const double *y, int n) const

{

double sum;
double *u = (double *) nullptr;
double *v = (double *) nullptr;

u = new double [n];
v = new double [n];

if ( !u || !v )  {

   mlog << Error << "\nLaeaGrid::xy_closedpolyline_area() -> "
	<< "memory allocation error\n\n";

   exit ( 1 );

}

for (int j=0; j<n; ++j)  {

   xy_to_uv(x[j], y[j], u[j], v[j]);

}

sum = uv_closedpolyline_area(u, v, n);

sum *= earth_radius_km*earth_radius_km;

delete [] u;  u = (double *) nullptr;
delete [] v;  v = (double *) nullptr;

return sum;

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

out << prefix << "Name         = ";
out << prefix << "SpheroidName = ";

if ( Name.length() > 0 )  out << '\"' << Name << '\"';
else                      out << "(nul)\n";

out << '\n';   //  no prefix

out << prefix << "Projection   = Laea\n";

out << prefix << "Nx           = " << comma_string(Nx) << "\n";

out << prefix << "Ny           = " << comma_string(Ny) << "\n";

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

a << "Projection: Labmbert Azimuthal Equal Area" << sep;

a << "Nx: " << Nx << sep;
a << "Ny: " << Ny << sep;

a << "SpheroidName: " << SpheroidName << sep;

   //
   //  done
   //

return a;

}


////////////////////////////////////////////////////////////////////////


void LaeaGrid::deserialize(const StringArray &)

{

mlog << Error << "\nLaeaGrid::deserialize(const StringArray &) -> "
     << "not yet implemented\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


GridInfo LaeaGrid::info() const

{

GridInfo i;

i.set(Data);

return i;

}


////////////////////////////////////////////////////////////////////////


double LaeaGrid::rot_grid_to_earth(int x, int y) const

{

   //
   //  grid to earth transformation is not just a simple rotation
   //

return 0.0;

}


////////////////////////////////////////////////////////////////////////


GridRep * LaeaGrid::copy() const

{

LaeaGrid * p = nullptr;

p = new LaeaGrid (Data);

return p;

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


return z;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Snyder, page 187
   //


void LaeaGrid::snyder_latlon_to_xy(double lat, double lon, double & x_snyder, double & y_snyder) const

{

double A, B, D, Qp, Rq;
double beta1, beta;
double m1, lambda, lambda0, lat1, delta;

A = geoid.a_km();

lambda  = -lon;

lambda0 = -lon_pole;
lat1    =  lat_pole;

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


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


double laea_segment_area(double u0, double v0, double u1, double v1)
 
{

double answer;

answer = 0.5*( u0*v1 - v0*u1 );

return answer;

}


////////////////////////////////////////////////////////////////////////


bool LaeaGrid::wrap_lon() const

{

return false;

}


////////////////////////////////////////////////////////////////////////


void LaeaGrid::shift_right(int N)

{

if ( N == 0 )  return;

mlog << Error << "\nLaeaGrid::shift_right(int) -> "
     << "shifting is not allowed for non-global grids\n\n";

exit ( 1 );

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


Grid::Grid(const LaeaNetcdfData & data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const LaeaNetcdfData & data)

{

clear();

rep = new LaeaGrid (data);

if ( !rep )  {

   mlog << Error << "\nGrid::set(const LaeaNetcdfData &) -> "
        << "memory allocation error\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////

