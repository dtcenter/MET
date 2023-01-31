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


switch ( data.hemisphere )  {

   case 'N':  IsNorthHemisphere = true;   break;
   case 'S':  IsNorthHemisphere = false;  break;

   default:
      mlog << Error << "\nStereographicGrid::StereographicGrid(const StereographicData &) -> "
           << "bad hemisphere ...\"" << (data.hemisphere) << "\"\n\n";
      exit ( 1 );

}   //  switch

const double H = ( IsNorthHemisphere ? 1.0 : -1.0 );


Nx = data.nx;
Ny = data.ny;

Name = data.name;

Lon_orient = data.lon_orient;
Data = data;


   //
   //  calculate Alpha
   //

Alpha = (1.0 + H*sind(data.scale_lat))*((data.r_km)/(data.d_km));


   //
   //  Calculate Bx, By
   //

double r0, theta0;

r0 = st_func(data.lat_pin, is_north());

theta0 = H*(Lon_orient - data.lon_pin);

Bx = data.x_pin - Alpha*r0*H*sind(theta0);
By = data.y_pin + Alpha*r0*H*cosd(theta0);

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

return ( st_func(lat, is_north()) );

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::df(double lat) const

{

return ( st_der_func(lat, is_north()) );

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::latlon_to_xy(double lat, double lon, double & x, double & y) const

{

double r, theta;

const double H = ( IsNorthHemisphere ? 1.0 : -1.0 );

reduce(lon);

if(is_eq(Data.eccentricity, 0.0)) {

   r = st_func(lat, is_north());

   theta = H*(Lon_orient - lon);

   x = Bx + Alpha*r*H*sind(theta);

   y = By - Alpha*r*H*cosd(theta);
}
else {
   double delta_sign;
   st_latlon_to_xy_func(lat, -lon, x, y, Data.scale_factor, (Data.lon_orient*-1.0),
                        (Data.r_km*m_per_km), Data.false_east, Data.false_north,
                        Data.eccentricity, IsNorthHemisphere);

   delta_sign = ((Data.d_km > 0) ? 1.0 : -1.0 );
   x = delta_sign * ((x / (fabs(Data.d_km)*m_per_km)) - Data.x_pin);    // meters to index
   delta_sign = ((Data.dy_km > 0) ? 1.0 : -1.0 );
   y = delta_sign * ((y / (fabs(Data.dy_km)*m_per_km)) - Data.y_pin);   // meters to index

}

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::xy_to_latlon(double x, double y, double & lat, double & lon) const

{

double r, theta;

const double H = ( IsNorthHemisphere ? 1.0 : -1.0 );

if(is_eq(Data.eccentricity, 0.0)) {
   x = (x - Bx)/Alpha;
   y = (y - By)/Alpha;

   r = sqrt( x*x + y*y );

   lat = st_inv_func(r, is_north());

   if ( fabs(r) < 1.0e-5 )  theta = 0.0;
   else                     theta = atan2d(H*x, -H*y);   //  NOT atan2d(y, x);

   if ( is_south() )  theta = -theta;

   lon = Lon_orient - theta;
}
else {
   double x1 = (Data.x_pin + x) * (Data.d_km*m_per_km);     // index to meters
   double y1 = (Data.y_pin + y) * (Data.dy_km*m_per_km);    // index to meters
   st_xy_to_latlon_func(x1, y1, lat, lon, Data.scale_factor, (Data.r_km*m_per_km),
                        (-1.0*Data.lon_orient), Data.false_east, Data.false_north,
                        Data.eccentricity, IsNorthHemisphere);
}

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

ConcatString junk;



out << prefix << "Name       = ";

if ( Name.length() > 0 )  out << '\"' << Name << '\"';
else                      out << "(nul)\n";

out << '\n';   //  no prefix

out << prefix << "Projection = Stereographic\n";

out << prefix << "Hemisphere = " << (is_north() ? "North" : "South") << "\n";

junk.format("%.5f", Lon_orient);
fix_float(junk);
out << prefix << "Lon_orient       = " << junk << "\n";

junk.format("%.5f", Alpha);
fix_float(junk);
out << prefix << "Alpha      = " << junk << "\n";

junk.format("%.5f", Bx);
fix_float(junk);
out << prefix << "Bx         = " << junk << "\n";

junk.format("%.5f", By);
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


ConcatString StereographicGrid::serialize(const char *sep) const

{

ConcatString a;
char junk[256];

a << "Projection: Stereographic" << sep;

a << "Nx: " << Nx << sep;
a << "Ny: " << Ny << sep;

a << "IsNorthHemisphere: " << ( IsNorthHemisphere ? "true" : "false") << sep;

snprintf(junk, sizeof(junk), "Lon_orient: %.3f", Lon_orient);   a << junk << sep;

snprintf(junk, sizeof(junk), "Bx: %.3f", Bx);   a << junk << sep;
snprintf(junk, sizeof(junk), "By: %.3f", By);   a << junk << sep;

snprintf(junk, sizeof(junk), "Alpha: %.4f", Alpha);   a << junk;

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

if ( is_south() )  angle = -angle;

return ( angle );

}


////////////////////////////////////////////////////////////////////////


bool StereographicGrid::wrap_lon() const

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

p->Name = Name;

return ( p );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


double st_func(double lat, bool is_north_hemisphere, double eccentricity)

{

double r;

if ( is_north_hemisphere )  r = tand(45.0 - 0.5*lat);
else                        r = tand(45.0 + 0.5*lat);

if (!is_eq(eccentricity, 0.)) {
   r *= pow(((1 + eccentricity*sind(lat)) / (1 - eccentricity*sind(lat))),(eccentricity/2));
}
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
else                        a =  1.0/(1.0 - sind(lat));

return ( a );

}


////////////////////////////////////////////////////////////////////////
// lat/lon to meters


bool st_latlon_to_xy_func(double lat, double lon, double &x_m, double &y_m,
                          double scale_factor, double scale_lat, double semi_major_axis,
                          double false_east, double false_north,
                          double e, bool is_north_hemisphere)
{

   if (e >= 0.1) {
      mlog << Error << "\nst_latlon_to_xy_func() -> "
           << "eccentricity (" << e << ") should be less than 0.1 for earth\n\n";
      exit( 1 );
   }
   else {
      const double lat_rad = lat * rad_per_deg;
      const double lon_rad = lon * rad_per_deg;
      const double lat_sin = sin(lat_rad);
      const double lonO_rad = scale_lat * rad_per_deg;
      const double H = (is_north_hemisphere? 1.0 : -1.0 );
      double t = tan(M_PI/4 - H*lat_rad/2) * pow((1 + e*lat_sin)/(1 - e*lat_sin), e/2);
      double rho = (2 * semi_major_axis * scale_factor * t)
                    / sqrt(pow(1+e,1+e) * pow(1-e,1-e));
      // meters in polar stereographics, not index
      x_m = false_east + rho*sin(lon_rad - lonO_rad);
      y_m = false_north - H*rho*cos(lon_rad - lonO_rad);
   }

   return true;
}

////////////////////////////////////////////////////////////////////////
// meters to lat/lon


bool st_xy_to_latlon_func(double x_m, double y_m, double &lat, double &lon,
                          double scale_factor, double semi_major_axis,
                          double proj_vertical_lon, double false_east, double false_north,
                          double eccentricity, bool is_north_hemisphere)
{
bool result = true;
if (eccentricity >= 0.1) {
   mlog << Error << "\nst_xy_to_latlon_func() -> "
        << "eccentricity (" << eccentricity << ") should be less than 0.1 for earth\n\n";
   exit( 1 );
}
else {
   double chi;
   double x_diff = x_m - false_east;
   double y_diff = y_m - false_north;
   double lonO_rad = proj_vertical_lon * rad_per_deg;
   double r_rho = sqrt(x_diff*x_diff + y_diff*y_diff);
   double t   = r_rho * sqrt(pow((1+eccentricity),(1+eccentricity)) * pow((1-eccentricity),(1-eccentricity)))
                / (2*semi_major_axis*scale_factor);
   if (is_north_hemisphere) chi = M_PI/2 - 2 * atan(t);
   else chi = 2 * atan(t) - M_PI/2;

   lat = chi + (eccentricity*eccentricity/2 + 5*pow(eccentricity,4)/24
         + pow(eccentricity,6)/12 + 13*pow(eccentricity,8)/360*sin(2 * chi)
         + (7*pow(eccentricity,4)/48 + 29*pow(eccentricity,6)/240 + 811*pow(eccentricity,8)/11520)*sin(4*chi)
         + (7*pow(eccentricity,6)/120 + 81*pow(eccentricity,8)/1120)*sin(6*chi)
         + (4279*pow(eccentricity,8)/161280)*sin(8*chi));

   if (x_m == false_east) lon = lonO_rad;
   else if (is_north_hemisphere) lon = lonO_rad + atan2(x_diff,-y_diff);
   else lon = lonO_rad + atan2(x_diff,y_diff);

   lat /= rad_per_deg;
   lon /= rad_per_deg;
   reduce(lon);

}
return result;

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


double st_eccentricity_func(double semi_major_axis, double semi_minor_axis,
                            double inverse_flattening)
{

double c;
double eccentricity = bad_data_double;

if (is_eq(semi_minor_axis, bad_data_double)) {
   semi_minor_axis = semi_major_axis - semi_major_axis/inverse_flattening;
}
eccentricity = sqrt(semi_major_axis*semi_major_axis - semi_minor_axis*semi_minor_axis) / semi_major_axis;

return ( eccentricity );

}


////////////////////////////////////////////////////////////////////////


double st_sf_func(double standard_parallel, double eccentricity, bool is_north_hemisphere)
{

double scale_factor;
double tF, mF, temp1, temp2;
double sp_rad = standard_parallel * rad_per_deg;
double lat_sin = sin(sp_rad);
double lat_cos = cos(sp_rad);

temp1 = eccentricity * lat_sin;
temp2 = pow((1 + temp1)/(1 - temp1), eccentricity/2);
if (is_north_hemisphere) tF = tan(M_PI/4 - sp_rad/2) * temp2;
else tF = tan(M_PI/4 + sp_rad/2) / temp2;
mF = lat_cos / sqrt(1 - eccentricity*eccentricity * lat_sin*lat_sin);
scale_factor = mF * sqrt(pow((1+eccentricity),(1+eccentricity)) * pow((1-eccentricity),(1-eccentricity))) / (2 * tF);

return ( scale_factor );

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


Grid create_aligned_st(double lat_center,   double lon_center, 
                       double lat_previous, double lon_previous,
                       double d_km, double r_km, 
                       int nx, int ny)

{

Grid g_new;
// double alpha;
double r_center, r_previous;
double Qx, Qy;
double L;
StereographicData data;
bool is_north = false;


data.name = "st_zoom";

if ( lat_center >= 0.0 )  { data.hemisphere = 'N';   is_north = true;  }
else                      { data.hemisphere = 'S';   is_north = false; }


const double H = ( is_north ? 1.0 : -1.0 );


data.scale_lat = lat_center;

data.lat_pin = lat_center;
data.lon_pin = lon_center;

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

r_center   = st_func(lat_center,   is_north);
r_previous = st_func(lat_previous, is_north);

Qx = r_center*sind(lon_center) - r_previous*sind(lon_previous);
Qy = r_center*cosd(lon_center) - r_previous*cosd(lon_previous);

L = atan2d(H*Qx, H*Qy) - H*180.0;

   //
   //  reduce L to the range -180 to +180
   //

L += 180.0;

L -= 360.0*floor(L/360.0);

L -= 180.0;


data.lon_orient = L;

   //
   //  done
   //

g_new.set(data);

return ( g_new );

}


////////////////////////////////////////////////////////////////////////






