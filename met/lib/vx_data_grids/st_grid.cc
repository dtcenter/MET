// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <cmath>

#include "vx_math/vx_math.h"
#include <vx_data_grids/st_grid.h>


////////////////////////////////////////////////////////////////////////


static double     st_func(double lat_rad);
static double st_der_func(double lat_rad);

static double st_inv_func(double r);

static void reduce_rad(double &angle_rad);

static double stereographic_segment_area(double u0, double v0, double u1, double v1);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class StereographicGrid
   //


////////////////////////////////////////////////////////////////////////


StereographicGrid::StereographicGrid()

{

Name = (char *) 0;

clear();

}


////////////////////////////////////////////////////////////////////////


StereographicGrid::~StereographicGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


StereographicGrid::StereographicGrid(const StereographicData &data)

{

Name = (char *) 0;

clear();

st_data = data;

Phi1_radians = (data.p1_deg)/deg_per_rad;

Phi0_radians = (data.p0_deg)/deg_per_rad;
Lon0_radians = (data.l0_deg)/deg_per_rad;

reduce_rad(Lon0_radians);

Lon_cen_radians = (data.lcen_deg)/deg_per_rad;

reduce_rad(Lon_cen_radians);

Delta_km = data.d_km;

Radius_km = data.r_km;

Bx = 0.0;
By = 0.0;

Nx = data.nx;
Ny = data.ny;

Name = new char [1 + strlen(data.name)];

if ( !Name )  {

   cerr << "\n\n  StereographicGrid::StereographicGrid(const char *, etc) -> memory allocation error\n\n";

   exit ( 1 );

}

strcpy(Name, data.name);

   //
   //  calculate alpha
   //

alpha = (-1.0/st_der_func(Phi1_radians))*(Radius_km/Delta_km);

   //
   //  Calculate Bx, By
   //

double r0, theta0;

r0 = st_func(Phi0_radians);

theta0 = Lon_cen_radians - Lon0_radians;

Bx = -alpha*r0*sin(theta0);
By =  alpha*r0*cos(theta0);

   //
   //  Done
   //

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::clear()

{

st_data.name     = (char *) 0;
st_data.p1_deg   = 0.0;
st_data.p0_deg   = 0.0;
st_data.l0_deg   = 0.0;
st_data.lcen_deg = 0.0;
st_data.d_km     = 0.0;
st_data.r_km     = 0.0;
st_data.nx       = 0;
st_data.ny       = 0;

Phi1_radians = 0.0;

Phi0_radians = 0.0;
Lon0_radians = 0.0;

Lon_cen_radians = 0.0;

Delta_km = 0.0;

Radius_km = 0.0;

Bx = 0.0;
By = 0.0;

alpha = 0.0;

Nx = 0;
Ny = 0;

if ( Name )  { delete [] Name;  Name = (char *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::f(double lat_deg) const

{

return ( st_func(lat_deg/deg_per_rad) );

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::df(double lat_deg) const

{

return ( st_der_func(lat_deg/deg_per_rad) );

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::latlon_to_xy(double lat_deg, double lon_deg, double &x, double &y) const

{

double lat_rad, lon_rad;
double r, theta;

lat_rad = lat_deg/deg_per_rad;
lon_rad = lon_deg/deg_per_rad;

reduce_rad(lon_rad);

r = st_func(lat_rad);

theta = Lon_cen_radians - lon_rad;

x = Bx + alpha*r*sin(theta);

y = By - alpha*r*cos(theta);

return;

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::xy_to_latlon(double x, double y, double &lat_deg, double &lon_deg) const

{

double lat_rad, lon_rad;
double r, theta;

x = (x - Bx)/alpha;
y = (y - By)/alpha;

r = sqrt( x*x + y*y );

lat_rad = st_inv_func(r);

lat_deg = deg_per_rad*lat_rad;

if ( fabs(r) < 1.0e-5 )  theta = 0.0;
else                     theta = atan2(x, -y);   //  NOT atan2(y, x);

lon_rad = Lon_cen_radians - theta;

reduce_rad(lon_rad);

lon_deg = deg_per_rad*lon_rad;

return;

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::calc_area(int x, int y) const

{

double u[4], v[4];
double sum;
const double R = EarthRadiusKM();


xy_to_uv(x - 0.5, y - 0.5, u[0], v[0]);  //  lower left
xy_to_uv(x + 0.5, y - 0.5, u[1], v[1]);  //  lower right
xy_to_uv(x + 0.5, y + 0.5, u[2], v[2]);  //  upper right
xy_to_uv(x - 0.5, y + 0.5, u[3], v[3]);  //  upper left


sum = uv_closedpolyline_area(u, v, 4);

sum *= R*R;

return ( sum );

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::calc_area_ll(int x, int y) const

{

double u[4], v[4];
double sum;
const double R = EarthRadiusKM();


xy_to_uv(x      , y      , u[0], v[0]);  //  lower left
xy_to_uv(x + 1.0, y      , u[1], v[1]);  //  lower right
xy_to_uv(x + 1.0, y + 1.0, u[2], v[2]);  //  upper right
xy_to_uv(x      , y + 1.0, u[3], v[3]);  //  upper left


sum = uv_closedpolyline_area(u, v, 4);

sum *= R*R;

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


double StereographicGrid::EarthRadiusKM() const

{

return ( Radius_km );

}


////////////////////////////////////////////////////////////////////////


const char * StereographicGrid::name() const

{

return ( Name );

}


////////////////////////////////////////////////////////////////////////


ProjType StereographicGrid::proj_type() const

{

return ( StereographicProj );

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::rot_grid_to_earth(int x, int y) const

{

double lat_deg, lon_deg, angle;
double diff, hemi;

// Convert to lat/lon
xy_to_latlon((double) x, (double) y, lat_deg, lon_deg);

// Difference between lon and the center longitude
diff = Lon_cen_radians*deg_per_rad - lon_deg;

// Figure out if the grid is in the northern or southern hemisphere
// by checking whether the first latitude (p1_deg -> Phi1_radians)
// is greater than zero
// NH -> hemi = 1, SH -> hemi = -1
if(Phi1_radians < 0.0) hemi = -1.0;
else                   hemi = 1.0;

// Compute the rotation angle
angle = diff*hemi;

return(angle);

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

const double R = EarthRadiusKM();

int j;
double sum;
double *u = (double *) 0;
double *v = (double *) 0;

u = new double [n];
v = new double [n];

if ( !u || !v )  {

   cerr << "\n\n  StereographicGrid::xy_closedpolyline_area() -> memory allocation error\n\n";

   exit ( 1 );

}

for (j=0; j<n; ++j)  {

   xy_to_uv(x[j], y[j], u[j], v[j]);

}

sum = uv_closedpolyline_area(u, v, n);

sum *= R*R;

delete [] u;  u = (double *) 0;
delete [] v;  v = (double *) 0;

return ( sum );

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::uv_to_xy(double u, double v, double &x, double &y) const

{

x = alpha*v + Bx;

y = -alpha*u + By;

return;

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::xy_to_uv(double x, double y, double &u, double &v) const

{

u = (x - Bx)/alpha;

v = (y - By)/(-alpha);

return;

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::grid_data(GridData &gdata) const

{

gdata.st_data = st_data;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


double st_func(double lat_rad)

{

double r;

r = tan(piover4 - 0.5*lat_rad);

return ( r );

}


////////////////////////////////////////////////////////////////////////


double st_inv_func(double r)

{

double lat_rad;

lat_rad = piover2 - 2.0*atan(r);

return ( lat_rad );

}


////////////////////////////////////////////////////////////////////////


double st_der_func(double lat_rad)

{

double a;

a = -1.0/(1.0 + sin(lat_rad));

return ( a );

}


////////////////////////////////////////////////////////////////////////


void reduce_rad(double &angle_rad)

{

angle_rad = angle_rad - twopi*floor( (angle_rad/twopi) + 0.5 );

return;

}


////////////////////////////////////////////////////////////////////////


double stereographic_segment_area(double u0, double v0, double u1, double v1)

{

cerr << "\n\n  warning -> stereographic_segment_area() -> hasn't been tested yet!\n\n";

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


Grid::Grid(const StereographicData &data)

{

rep = (GridRep *) 0;

rep = new StereographicGrid (data);

if ( !rep )  {

   cerr << "\n\n  Grid::Grid(const StereographicData &) -> memory allocation error\n\n";

   exit ( 1 );

}

rep->refCount = 1;

}


////////////////////////////////////////////////////////////////////////






