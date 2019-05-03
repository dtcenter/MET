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
#include <vx_data_grids/lc_grid.h>


////////////////////////////////////////////////////////////////////////


static double     lc_func(double lat_rad, double cone_constant);
static double lc_der_func(double lat_rad, double cone_constant);

static double lc_inv_func(double       r, double cone_constant);

static void reduce_rad(double &angle_rad);

static double lambert_segment_area(double u0, double v0, double u1, double v1, double c);

static double lambert_beta(double u0, double delta_u, double v0, double delta_v, double c, double t);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class LambertGrid
   //


////////////////////////////////////////////////////////////////////////


LambertGrid::LambertGrid()

{

Name = (char *) 0;

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

lc_data.name     = (char *) 0;
lc_data.p1_deg   = 0.0;
lc_data.p2_deg   = 0.0;
lc_data.p0_deg   = 0.0;
lc_data.l0_deg   = 0.0;
lc_data.lcen_deg = 0.0;
lc_data.d_km     = 0.0;
lc_data.r_km     = 0.0;
lc_data.nx       = 0;
lc_data.ny       = 0;

Phi1_radians = 0.0;
Phi2_radians = 0.0;

Phi0_radians = 0.0;
Lon0_radians = 0.0;

Lon_cen_radians = 0.0;

Delta_km = 0.0;

Radius_km = 0.0;

Bx = 0.0;
By = 0.0;

alpha = 0.0;

cone = 0.0;

Nx = 0;
Ny = 0;

if ( Name )  { delete [] Name;  Name = (char *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


LambertGrid::LambertGrid(const LambertData &data)

{

Name = (char *) 0;

clear();

lc_data = data;

Phi1_radians = (data.p1_deg)/deg_per_rad;
Phi2_radians = (data.p2_deg)/deg_per_rad;

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

   cerr << "\n\n  LambertGrid::LambertGrid(const LambertData &) -> memory allocation error\n\n";

   exit ( 1 );

}

strcpy(Name, data.name);

   //
   //  calculate cone constant
   //

if ( fabs(Phi1_radians - Phi2_radians) < 1.0e-5 )  cone = sin(Phi1_radians);
else {

   double t, b;

   t = cos(Phi1_radians)/cos(Phi2_radians);

   b = tan(piover4 - 0.5*Phi1_radians)/tan(piover4 - 0.5*Phi2_radians);

   cone = log(t)/log(b);

}

   //
   //  calculate alpha
   //

alpha = (-1.0/lc_der_func(Phi1_radians, cone))*(Radius_km/Delta_km);

   //
   //  Calculate Bx, By
   //

double r0, theta0;

r0 = lc_func(Phi0_radians, cone);

theta0 = cone*(Lon_cen_radians - Lon0_radians);

Bx = -alpha*r0*sin(theta0);
By =  alpha*r0*cos(theta0);

   //
   //  Done
   //

}


////////////////////////////////////////////////////////////////////////


double LambertGrid::f(double lat_deg) const

{

return ( lc_func(lat_deg/deg_per_rad, cone) );

}


////////////////////////////////////////////////////////////////////////


double LambertGrid::df(double lat_deg) const

{

return ( lc_der_func(lat_deg/deg_per_rad, cone) );

}


////////////////////////////////////////////////////////////////////////


void LambertGrid::latlon_to_xy(double lat_deg, double lon_deg, double &x, double &y) const

{

double lat_rad, lon_rad;
double r, theta;

lat_rad = lat_deg/deg_per_rad;
lon_rad = lon_deg/deg_per_rad;

reduce_rad(lon_rad);

r = lc_func(lat_rad, cone);

theta = cone*(Lon_cen_radians - lon_rad);

x = Bx + alpha*r*sin(theta);

y = By - alpha*r*cos(theta);

return;

}


////////////////////////////////////////////////////////////////////////


void LambertGrid::xy_to_latlon(double x, double y, double &lat_deg, double &lon_deg) const

{

double lat_rad, lon_rad;
double r, theta;

x = (x - Bx)/alpha;
y = (y - By)/alpha;

r = sqrt( x*x + y*y );

lat_rad = lc_inv_func(r, cone);

lat_deg = deg_per_rad*lat_rad;

if ( fabs(r) < 1.0e-5 )  theta = 0.0;
else                     theta = atan2(x, -y);   //  NOT atan2(y, x);

lon_rad = Lon_cen_radians - theta/cone;

reduce_rad(lon_rad);

lon_deg = deg_per_rad*lon_rad;

return;

}


////////////////////////////////////////////////////////////////////////


double LambertGrid::calc_area(int x, int y) const

{

const double R = EarthRadiusKM();

double u[4], v[4];
double sum;


xy_to_uv(x - 0.5, y - 0.5, u[0], v[0]);  //  lower left
xy_to_uv(x + 0.5, y - 0.5, u[1], v[1]);  //  lower right
xy_to_uv(x + 0.5, y + 0.5, u[2], v[2]);  //  upper right
xy_to_uv(x - 0.5, y + 0.5, u[3], v[3]);  //  upper left


sum = uv_closedpolyline_area(u, v, 4);

sum *= R*R;

return ( sum );

}


////////////////////////////////////////////////////////////////////////


double LambertGrid::calc_area_ll(int x, int y) const

{

const double R = EarthRadiusKM();

double u[4], v[4];
double sum;


xy_to_uv(x      , y      , u[0], v[0]);  //  lower left
xy_to_uv(x + 1.0, y      , u[1], v[1]);  //  lower right
xy_to_uv(x + 1.0, y + 1.0, u[2], v[2]);  //  upper right
xy_to_uv(x      , y + 1.0, u[3], v[3]);  //  upper left


sum = uv_closedpolyline_area(u, v, 4);

sum *= R*R;

return ( sum );

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


double LambertGrid::EarthRadiusKM() const

{

return ( Radius_km );

}


////////////////////////////////////////////////////////////////////////


const char * LambertGrid::name() const

{

return ( Name );

}


////////////////////////////////////////////////////////////////////////


ProjType LambertGrid::proj_type() const

{

return ( LambertProj );

}


////////////////////////////////////////////////////////////////////////


double LambertGrid::rot_grid_to_earth(int x, int y) const

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
angle = diff*cone*hemi;

return(angle);

}

////////////////////////////////////////////////////////////////////////


double LambertGrid::uv_closedpolyline_area(const double *u, const double *v, int n) const

{

int j, k;
double sum;


sum = 0.0;

for (j=0; j<n; ++j)  {

   k = (j + 1)%n;

   sum += lambert_segment_area(u[j], v[j], u[k], v[k], cone);

}

sum = fabs(sum);

return ( sum );

}


////////////////////////////////////////////////////////////////////////


double LambertGrid::xy_closedpolyline_area(const double *x, const double *y, int n) const

{

const double R = EarthRadiusKM();

int j;
double sum;
double *u = (double *) 0;
double *v = (double *) 0;

u = new double [n];
v = new double [n];

if ( !u || !v )  {

   cerr << "\n\n  LambertGrid::xy_closedpolyline_area() -> memory allocation error\n\n";

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


void LambertGrid::uv_to_xy(double u, double v, double &x, double &y) const

{

x = alpha*v + Bx;

y = -alpha*u + By;

return;

}


////////////////////////////////////////////////////////////////////////


void LambertGrid::xy_to_uv(double x, double y, double &u, double &v) const

{

u = (x - Bx)/alpha;

v = (y - By)/(-alpha);

return;

}


////////////////////////////////////////////////////////////////////////


void LambertGrid::grid_data(GridData &gdata) const

{

gdata.lc_data = lc_data;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


double lc_func(double lat_rad, double cone_constant)

{

double r;

r = tan(piover4 - 0.5*lat_rad);

r = pow(r, cone_constant);

return ( r );

}


////////////////////////////////////////////////////////////////////////


double lc_inv_func(double r, double cone_constant)

{

double lat_rad;

lat_rad = piover2 - 2.0*atan(pow(r, 1.0/cone_constant));

return ( lat_rad );

}


////////////////////////////////////////////////////////////////////////


double lc_der_func(double lat_rad, double cone_constant)

{

double a;

a = -(cone_constant/cos(lat_rad))*lc_func(lat_rad, cone_constant);

return ( a );

}


////////////////////////////////////////////////////////////////////////


void reduce_rad(double &angle_rad)

{

angle_rad = angle_rad - twopi*floor( (angle_rad/twopi) + 0.5 );

return;

}


////////////////////////////////////////////////////////////////////////


double lambert_segment_area(double u0, double v0, double u1, double v1, double c)

{

int i, j, k, n;
double rom, denom, h, delta_u, delta_v;
double trap, t[15], left, right, test, sum;
const double a = 0.0, b = 1.0;
const double tol = 1.0e-6;

delta_u = u1 - u0;
delta_v = v1 - v0;

i = 0;
n = 2;

h = (b - a)/n;

sum = lambert_beta(u0, delta_u, v0, delta_v, c, a) + lambert_beta(u0, delta_u, v0, delta_v, c, b);

t[0] = trap = (h/2.0)*sum + h*lambert_beta(u0, delta_u, v0, delta_v, c, a + h);

do {

   ++i;

   n *= 2;

   h = (b - a)/n;

   sum = 0.0;

   for (j=1; j<n; j+=2)   sum += lambert_beta(u0, delta_u, v0, delta_v, c, a + j*h);

   trap = 0.5*trap + h*sum;

   left = trap;

   for (k=1; k<=i; ++k)  {

      denom = pow(4.0, (double) k) - 1.0;

      right = left + (left - t[k-1])/denom;

      test = 2.0*(left - t[k-1]);

      t[k-1] = left;

      left = right;

   }

   t[i] = left;

}  while ( (fabs(test) >= tol) && (i <= 14) );

if ( i >= 14 )  {

   cerr << "\n\n  lambert_segment_area() -> array bounds error\n\n";

   exit ( 1 );

}

rom = t[i];

rom *= (2.0/c)*(u0*v1 - u1*v0);

return ( rom );

}


////////////////////////////////////////////////////////////////////////


double lambert_beta(double u0, double delta_u, double v0, double delta_v, double c, double t)

{

double answer;
double u, v, r2, e_top, e_bot;

u = u0 + t*delta_u;
v = v0 + t*delta_v;

r2 = u*u + v*v;

e_bot = 1.0/c;

e_top = e_bot - 1.0;

answer = pow(r2, e_top)/(1.0 + pow(r2, e_bot));

return ( answer );

}


////////////////////////////////////////////////////////////////////////


Grid::Grid(const LambertData &data)

{

rep = (GridRep *) 0;

rep = new LambertGrid (data);

if ( !rep )  {

   cerr << "\n\n  Grid::Grid(const LambertData &) -> memory allocation error\n\n";

   exit ( 1 );

}

rep->refCount = 1;

}



////////////////////////////////////////////////////////////////////////






