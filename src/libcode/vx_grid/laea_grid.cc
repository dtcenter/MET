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
#include "vx_geodesy.h"

#include "laea_grid.h"
#include "latlon_xyz.h"


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

/*
LaeaGrid::LaeaGrid(const LaeaData & data)

{

clear();

memset(&Grib2Data, 0, sizeof(Grib2Data));
Data = data;

Nx = data.nx;
Ny = data.ny;
 
Name = data.name;
SpheroidName = data.geoid;

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
*/

////////////////////////////////////////////////////////////////////////


LaeaGrid::LaeaGrid(const LaeaGrib2Data & grib2_data)

{

clear();

memset(&Data, 0, sizeof(Data));
Data = grib2_data;

lat_LL = grib2_data.lat_first;
lon_LL = grib2_data.lon_first;

lat_pole = grib2_data.standard_lat;
lon_pole = grib2_data.central_lon;

Name = grib2_data.name;
SpheroidName = grib2_data.spheroid_name;
Data.spheroid_name = SpheroidName.c_str();

Nx = grib2_data.nx;
Ny = grib2_data.ny;

geoid.set_ab(grib2_data.equatorial_radius_km, grib2_data.polar_radius_km);

geoid.set_name(grib2_data.spheroid_name);

// aff.set_mb(1.0, 0.0, 0.0, 1.0, 1.0, 1.0);
// aff.set_mb(2.0, 0.0, 0.0, 2.0, 1.0, 1.0);

double s = 1.0;

aff.set_mb(s, 0.0, 0.0, s, 1.0, 1.0);

// aff.set_mb(grib2_data.dx_km, 0.0, 0.0, grib2_data.dy_km, 1.0, 1.0);

double xx, yy;

// latlon_to_xy(Data.lat_pole, Data.lon_pole, xx, yy);
// aff.set_pin(xx, yy, 0.5*(Nx - 1.0), 0.5*(Ny - 1.0));

latlon_to_xy(grib2_data.lat_first, grib2_data.lon_first, xx, yy);
aff.set_pin(xx, yy, 0.0, 0.0);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////

/*
void LaeaGrid::calc_aff()

{

double u_LL, v_LL;
double u_LR, v_LR;
double u_UL, v_UL;

double x_LL, y_LL;
double x_LR, y_LR;
double x_UL, y_UL;


snyder_latlon_to_xy(Data.lat_LL, Data.lon_LL, u_LL, v_LL);
snyder_latlon_to_xy(Data.lat_LR, Data.lon_LR, u_LR, v_LR);
snyder_latlon_to_xy(Data.lat_UL, Data.lon_UL, u_UL, v_UL);


x_LL = 0.0;
y_LL = 0.0;

x_LR = Data.nx - 1.0;
y_LR = 0.0;

x_UL = 0.0;
y_UL = Data.ny  - 1.0;


aff.set_three_points(

   u_LL, v_LL, x_LL, y_LL, 

   u_LR, v_LR, x_LR, y_LR, 

   u_UL, v_UL, x_UL, y_UL

);


return;

}
*/

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

memset(&Data,      0, sizeof(Data));
// memset(&Grib2Data, 0, sizeof(Grib2Data));

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


// lon = lambda0 + atand(num/denom);   //  maybe want atan2 here?   //  Eq 24-26, page 188
lon = lambda0 + atan2d(num, denom);   //  maybe want atan2 here?

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


ConcatString LaeaGrid::spheroid_name() const

{

return ( SpheroidName );

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

return ( a );

}


////////////////////////////////////////////////////////////////////////


void LaeaGrid::deserialize(const StringArray &)

{

mlog << Error << "\nLaeaGrid::deserialize(const StringArray &) -> "
     << "not yet implemented\n\n";

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


GridRep * LaeaGrid::copy() const

{

LaeaGrid * p = 0;

p = new LaeaGrid (Data);

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


   //
   //  Snyder, page 187
   //


void LaeaGrid::snyder_latlon_to_xy(double lat, double lon, double & x_snyder, double & y_snyder) const

{

double A, B, D, Qp, Rq;
double beta1, beta;
double m1, lambda, lambda0, lat1, delta;

A = geoid.a_km();
// A = 1.0;


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
                                                   
return ( answer );                                 
                                                   
}


////////////////////////////////////////////////////////////////////////


bool LaeaGrid::wrap_lon() const

{

return ( false );

}


////////////////////////////////////////////////////////////////////////


void LaeaGrid::shift_right(int N)

{

if ( N == 0 )  return;

mlog << Error << "\nLaeaGrid::shift_right(int) -> "
     << "shifting is not allowed for non-global grids\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Grid functions
   //


////////////////////////////////////////////////////////////////////////

/*
Grid::Grid(const LaeaData & data)

{

init_from_scratch();

set(data);

}
*/

////////////////////////////////////////////////////////////////////////

/*
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
*/

////////////////////////////////////////////////////////////////////////


Grid::Grid(const LaeaGrib2Data & grib2_data)

{

init_from_scratch();

set(grib2_data);

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const LaeaGrib2Data & grib2_data)

{

clear();

rep = new LaeaGrid (grib2_data);

if ( !rep )  {

   mlog << Error << "\nGrid::set(const LaeaGrib2Data &) -> "
	<< "memory allocation error\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////

