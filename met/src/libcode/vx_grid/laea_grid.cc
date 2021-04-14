

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

   mlog << Error
        << "\n\n  LaeaGrid::LaeaGrid(const LaeaData &) -> "
        << "unrecognized geoid ... \"" << data.geoid << "\"\n\n";

   exit ( 1 );

}

calc_aff();

}


////////////////////////////////////////////////////////////////////////


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


// aff.set_three_points_v2(
// 
//    u_LL, v_LL, x_LL, y_LL, 
// 
//    u_LR, v_LR, x_LR, y_LR, 
// 
//    u_UL, v_UL, x_UL, y_UL
// 
// );


aff.set_three_points(

   u_LL, v_LL, u_LR, v_LR, u_UL, v_UL, 

   x_LL, y_LL, x_LR, y_LR, x_UL, y_UL

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

/*
void LaeaGrid::latlon_old_to_rot(double lat_old, double lon_old, double & lat_rot, double & lon_rot) const

{

double u, v, w, uu, vv, ww;


grid_latlon_to_xyz(lat_old, lon_old, u, v, w);

grid_xyz_to_latlon(uu, vv, ww, lat_rot, lon_rot);


return;

}
*/

////////////////////////////////////////////////////////////////////////

/*
void LaeaGrid::latlon_rot_to_old(double lat_rot, double lon_rot, double & lat_old, double & lon_old) const

{

double u, v, w, uu, vv, ww;

grid_latlon_to_xyz(lat_rot, lon_rot, uu, vv, ww);

grid_xyz_to_latlon(u, v, w, lat_old, lon_old);


return;

}
*/

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


mlog << Error
     << "\n\n  LaeaGrid::xy_to_latlon() const -> not yet implemented\n\n";

exit ( 1 );


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

   mlog << Error
        << "\n\n  LaeaGrid::xy_closedpolyline_area() -> memory allocation error\n\n";

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

/*
void LaeaGrid::latlon_to_rt(double lat_deg, double lon_deg, double & r, double & theta) const

{

theta = 90.0 - lon_deg;

r = laea_r(lat_deg);

return;

}
*/


////////////////////////////////////////////////////////////////////////

/*
void LaeaGrid::rt_to_latlon(double r, double theta, double & lat_deg, double & lon_deg) const

{

lon_deg = 90.0 - theta;

lat_deg = laea_r_inv(r);

return;

}
*/

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

/*
void LaeaGrid::rt_to_uv(double r, double theta, double & u, double & v) const

{

u = r*sind(theta);

v = r*cosd(theta);

return;

}
*/

////////////////////////////////////////////////////////////////////////

/*
void LaeaGrid::uv_to_rt(double u, double v, double & r, double & theta) const

{

r = sqrt( u*u + v*v );

theta = atan2d(u, v);   //  NOT atan2d(v, u)

return;

}
*/

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


ConcatString LaeaGrid::serialize(int version) const

{

ConcatString a;
// char junk[256];


// a << grid_proj_ser_name << ' ' << stereographic_proj_ser_name;
// 
// a << ' ' << grid_version_ser_name << ' ' << version;
// 
// a << ' ' << grid_nx_ser_name << ' ' << Nx;
// a << ' ' << grid_ny_ser_name << ' ' << Ny;
// 
// a << ' ' << north_hemi_ser_name << ' ' << ( IsNorthHemisphere ? "true" : "false");
// 
// a << ' ' << lon_orient_ser_name << ' ';
// snprintf(junk, sizeof(junk), "%.3f", Lon_orient);
// fix_float(junk);
// a << junk;
// 
// a << ' ' << bx_ser_name << ' ';
// snprintf(junk, sizeof(junk), "%.3f", Bx);
// fix_float(junk);
// a << junk;
// 
// a << ' ' << by_ser_name << ' ';
// snprintf(junk, sizeof(junk), "%.3f", By);
// fix_float(junk);
// a << junk;
// 
// a << ' ' << alpha_ser_name << ' ';
// snprintf(junk, sizeof(junk), "%.5f", Alpha);
// fix_float(junk);
// a << junk;

   //
   //  done
   //

return ( a );

}


////////////////////////////////////////////////////////////////////////


void LaeaGrid::deserialize(const StringArray &)

{

mlog << Error
     << "\n\n  LaeaGrid::deserialize(const StringArray &) -> not yet implemented\n\n";

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
// A = 1.0;


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

mlog << Error
     << "\n\n  LaeaGrid::shift_right(int) -> not yet implemented!\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString LaeaGrid::serialize() const

{

mlog << Error
     << "\n\n  LaeaGrid::serialize() -> not yet implemented!\n\n";

exit ( 1 );

return ( ConcatString () );

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

// cerr << "  warning -> laea_segment_area() -> hasn't been tested yet!\n";
 
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

   mlog << Error
        << "\n\n  Grid::set(const LaeaData &) -> memory allocation error\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////




