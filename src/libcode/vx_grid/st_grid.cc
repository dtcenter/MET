

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
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
#include "st_grid.h"


////////////////////////////////////////////////////////////////////////


// static double     st_func (double lat, bool is_north_hemisphere);
// static double st_der_func (double lat, bool is_north_hemisphere);
// 
// static double st_inv_func (double r, bool is_north_hemisphere);

static void reduce(double & angle);

static double stereographic_segment_area(double u0, double v0, double u1, double v1);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class StereographicGrid
   //


////////////////////////////////////////////////////////////////////////


StereographicGrid::StereographicGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


StereographicGrid::~StereographicGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


StereographicGrid::StereographicGrid(const StereographicData & data)

{

clear();

Lon_orient = data.lon_orient;

if ( data.hemisphere == 'S' )  Lon_orient += 90.0;

Nx = data.nx;
Ny = data.ny;

switch ( data.hemisphere )  {

   case 'N':  IsNorthHemisphere = true;   break;
   case 'S':  IsNorthHemisphere = false;  break;

   default:
      mlog << Error << "\nStereographicGrid::StereographicGrid(const StereographicData &) -> "
           << "bad hemisphere ...\"" << (data.hemisphere) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch

Name = data.name;

   //
   //  calculate Alpha
   //

Alpha = (1.0 + sind(fabs(data.scale_lat)))*((data.r_km)/(data.d_km));

   //
   //  Calculate Bx, By
   //

double r0, theta0;

r0 = st_func(data.lat_pin, IsNorthHemisphere);

theta0 = Lon_orient - data.lon_pin;

Bx = data.x_pin - Alpha*r0*sind(theta0);
By = data.y_pin + Alpha*r0*cosd(theta0);

Data = data;

   //
   //  Done
   //

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::clear()

{

Lon_orient = 0.0;

Bx = 0.0;
By = 0.0;

Alpha = 0.0;

Nx = 0;
Ny = 0;

Name.clear();

IsNorthHemisphere = true;

memset(&Data, 0, sizeof(Data));

return;

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::f(double lat) const

{

return ( st_func(lat, IsNorthHemisphere) );

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::df(double lat) const

{

return ( st_der_func(lat, IsNorthHemisphere) );

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::latlon_to_xy(double lat, double lon, double & x, double & y) const

{

double r, theta;


reduce(lon);

r = st_func(lat, IsNorthHemisphere);

theta = Lon_orient - lon;

if ( !IsNorthHemisphere )  theta = -theta;

x = Bx + Alpha*r*sind(theta);

y = By - Alpha*r*cosd(theta);

return;

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::xy_to_latlon(double x, double y, double & lat, double & lon) const

{

double r, theta;


x = (x - Bx)/Alpha;
y = (y - By)/Alpha;

r = sqrt( x*x + y*y );

lat = st_inv_func(r, IsNorthHemisphere);

if ( fabs(r) < 1.0e-5 )  theta = 0.0;
else                     theta = atan2d(x, -y);   //  NOT atan2d(y, x);

if ( !IsNorthHemisphere )  theta = -theta;

lon = Lon_orient - theta;

reduce(lon);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::calc_area(int x, int y) const

{

double u[4], v[4];
double sum;


// xy_to_uv(x - 0.5, y - 0.5, u[0], v[0]);  //  lower left
// xy_to_uv(x + 0.5, y - 0.5, u[1], v[1]);  //  lower right
// xy_to_uv(x + 0.5, y + 0.5, u[2], v[2]);  //  upper right
// xy_to_uv(x - 0.5, y + 0.5, u[3], v[3]);  //  upper left


xy_to_uv(x      , y      , u[0], v[0]);  //  lower left
xy_to_uv(x + 1.0, y      , u[1], v[1]);  //  lower right
xy_to_uv(x + 1.0, y + 1.0, u[2], v[2]);  //  upper right
xy_to_uv(x      , y + 1.0, u[3], v[3]);  //  upper left


sum = uv_closedpolyline_area(u, v, 4);

sum *= earth_radius_km*earth_radius_km;

return ( sum );

}


////////////////////////////////////////////////////////////////////////


int StereographicGrid::nx() const

{

return ( Nx );

}


////////////////////////////////////////////////////////////////////////


int StereographicGrid::ny() const

{

return ( Ny );

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::uv_closedpolyline_area(const double *u, const double *v, int n) const

{

int j, k;
double sum;


sum = 0.0;

for (j=0; j<n; ++j)  {

   k = (j + 1)%n;

   sum += stereographic_segment_area(u[j], v[j], u[k], v[k]);

}   //  for j

sum = fabs(sum);

return ( sum );

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::xy_closedpolyline_area(const double *x, const double *y, int n) const

{

int j;
double sum;
double *u = (double *) 0;
double *v = (double *) 0;

u = new double [n];
v = new double [n];

if ( !u || !v )  {

   mlog << Error << "\nStereographicGrid::xy_closedpolyline_area() -> "
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


ConcatString StereographicGrid::name() const

{

return ( Name );

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::uv_to_xy(double u, double v, double &x, double &y) const

{

x = Alpha*v + Bx;

y = -Alpha*u + By;

return;

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::xy_to_uv(double x, double y, double &u, double &v) const

{

u = (x - Bx)/Alpha;

v = (y - By)/(-Alpha);

return;

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::dump(ostream & out, int depth) const

{

Indent prefix(depth);
char junk[256];



out << prefix << "Name       = ";

if ( Name.length() > 0 )  out << '\"' << Name << '\"';
else                      out << "(nul)\n";

out << '\n';   //  no prefix

out << prefix << "Projection = Stereographic\n";

out << prefix << "Hemisphere = " << (IsNorthHemisphere ? "North" : "South") << "\n";

sprintf(junk, "%.5f", Lon_orient);
fix_float(junk);
out << prefix << "Lon_orient       = " << junk << "\n";

sprintf(junk, "%.5f", Alpha);
fix_float(junk);
out << prefix << "Alpha      = " << junk << "\n";

sprintf(junk, "%.5f", Bx);
fix_float(junk);
out << prefix << "Bx         = " << junk << "\n";

sprintf(junk, "%.5f", By);
fix_float(junk);
out << prefix << "By         = " << junk << "\n";

comma_string(Nx, junk);
out << prefix << "Nx         = " << junk << "\n";

comma_string(Ny, junk);
out << prefix << "Ny         = " << junk << "\n";


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString StereographicGrid::serialize() const

{

ConcatString a;
char junk[256];

a << "Projection: Stereographic";

a << " Nx: " << Nx;
a << " Ny: " << Ny;

a << " IsNorthHemisphere: " << ( IsNorthHemisphere ? "true" : "false");

sprintf(junk, " Lon_orient: %.3f", Lon_orient);   a << junk;

sprintf(junk, " Bx: %.3f", Bx);   a << junk;
sprintf(junk, " By: %.3f", By);   a << junk;

sprintf(junk, " Alpha: %.4f", Alpha);   a << junk;

   //
   //  done
   //

return ( a );

}


////////////////////////////////////////////////////////////////////////


GridInfo StereographicGrid::info() const

{

GridInfo i;

i.set(Data);

return ( i );

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::rot_grid_to_earth(int x, int y) const

{

double lat, lon, angle;


xy_to_latlon((double) x, (double) y, lat, lon);

angle = Lon_orient - lon;

if ( !IsNorthHemisphere )  angle = -angle;

return ( angle );

}


////////////////////////////////////////////////////////////////////////


bool StereographicGrid::is_global() const

{

return ( false );

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::shift_right(int N)

{

if ( N == 0 )  return;

mlog << Error << "\nStereographicGrid::shift_right(int) -> "
     << "shifting is not allowed for non-global grids\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


GridRep * StereographicGrid::copy() const

{

StereographicGrid * p = new StereographicGrid (Data);

return ( p );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


double st_func(double lat, bool is_north_hemisphere)

{

double r;

if ( is_north_hemisphere )  r = tand(45.0 - 0.5*lat);
else                        r = tand(45.0 + 0.5*lat);

return ( r );

}


////////////////////////////////////////////////////////////////////////


double st_inv_func(double r, bool is_north_hemisphere)

{

double lat;

lat = 90.0 - 2.0*atand(r);

if ( !is_north_hemisphere )  lat = -lat;

return ( lat );

}


////////////////////////////////////////////////////////////////////////


double st_der_func(double lat, bool is_north_hemisphere)

{

double a;

if ( is_north_hemisphere )  a = -1.0/(1.0 + sind(lat));
else                        a = -1.0/(1.0 - sind(lat));

return ( a );

}


////////////////////////////////////////////////////////////////////////


double stereographic_alpha(double scale_lat, double r_km, double d_km)

{

double alpha;

alpha = (1.0 + sind(fabs(scale_lat)))*((r_km)/(d_km));


return ( alpha );

}


////////////////////////////////////////////////////////////////////////


void reduce(double & angle)

{

angle -= 360.0*floor( (angle/360.0) + 0.5 );

return;

}


////////////////////////////////////////////////////////////////////////


double stereographic_segment_area(double u0, double v0, double u1, double v1)

{

double b, answer, t1, t2;

b =   u0*u0 + v0*v0 + u1*u1 + v1*v1
    + u0*u0*v1*v1 + u1*u1*v0*v0
    - 2.0*( u0*u1 + v0*v1 + u0*v0*u1*v1 );

b = 1.0/sqrt(b);

t1 =  b*( u1*u1 + v1*v1 - u0*u1 - v0*v1 );
t2 = -b*( u0*u0 + v0*v0 - u0*u1 - v0*v1 );

answer = atan(t1) - atan(t2);

answer *= 2.0*b*( u0*v1 - u1*v0 );

return ( answer );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Grid functions
   //


////////////////////////////////////////////////////////////////////////


Grid::Grid(const StereographicData & data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////

/*
Grid::Grid(const StereoType2Data & data)

{

init_from_scratch();

set(data);

}
*/

////////////////////////////////////////////////////////////////////////

/*
Grid::Grid(const StereoType3Data & data)

{

init_from_scratch();

set(data);

}
*/

////////////////////////////////////////////////////////////////////////


void Grid::set(const StereographicData & data)

{

clear();

rep = new StereographicGrid (data);

if ( !rep )  {

   mlog << Error << "\nGrid::set(const StereographicData &) -> "
        << "memory allocation error\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////

/*
void Grid::set(const StereoType2Data & data)

{

clear();

rep = new StereographicGrid (data);

if ( !rep )  {

   mlog << Error << "\nGrid::set(const StereoType2Data &) -> memory allocation error\n\n";

   exit ( 1 );

}

return;

}
*/

////////////////////////////////////////////////////////////////////////

/*
void Grid::set(const StereoType3Data & data)

{

clear();

rep = new StereographicGrid (data);

if ( !rep )  {

   mlog << Error << "\nGrid::set(const StereoType3Data &) -> memory allocation error\n\n";

   exit ( 1 );

}

return;

}
*/

////////////////////////////////////////////////////////////////////////


Grid create_aligned_st(double lat1, double lon1, double lat2, double lon2,
                       double d_km, double r_km, int nx, int ny)

{

Grid g_new;
// double alpha;
double r1, r2;
double Qx, Qy;
StereographicData data;
bool is_north = false;


data.name = "zoom";

if ( lat1 >= 0.0 )  { data.hemisphere = 'N';   is_north = true;  }
else                { data.hemisphere = 'S';   is_north = false; }

data.scale_lat = lat1;

data.lat_pin = lat1;
data.lon_pin = lon1;

data.x_pin = 0.5*nx;
data.y_pin = 0.5*ny;

data.r_km = r_km;

data.d_km = d_km;

data.nx = nx;
data.ny = ny;

   //
   //  calculate orientation longitude
   //

// alpha = stereographic_alpha(data.scale_lat, data.r_km, data.d_km);

r1 = st_func(lat1, is_north);
r2 = st_func(lat2, is_north);

Qx = r1*sind(lon1) - r2*sind(lon2);
Qy = r1*cosd(lon1) - r2*cosd(lon2);

data.lon_orient = atan2d(Qx, Qy);

// cout << "\n\n  Lon orient = " << (data.lon_orient) << "\n\n";

   //
   //  done
   //

g_new.set(data);

return ( g_new );

}


////////////////////////////////////////////////////////////////////////






