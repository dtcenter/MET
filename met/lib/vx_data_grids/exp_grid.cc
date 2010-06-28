

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
#include "exp_grid.h"


////////////////////////////////////////////////////////////////////////


static void normalize(double &ax, double &ay, double &az);

static double dot(double ax, double ay, double az, double bx, double by, double bz);

static void cross(double ax, double ay, double az, double bx, double by, double bz, double &cx, double &cy, double &cz);

static void pl_to_vect(double lat_deg, double lon_deg, double &vx, double &vy, double &vz);

static void vect_to_pl(double vx, double vy, double vz, double &lat_deg, double &lon_deg);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ExpGrid
   //


////////////////////////////////////////////////////////////////////////


ExpGrid::ExpGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ExpGrid::~ExpGrid() 

{ 

clear();

}


////////////////////////////////////////////////////////////////////////


ExpGrid::ExpGrid(const ExpData &data)

{

clear();

lat_origin_deg = data.lat_origin_deg;
lon_origin_deg = data.lon_origin_deg;

pl_to_vect(data.lat_origin_deg, data.lon_origin_deg, e3x, e3y, e3z);

pl_to_vect(data.lat_2_deg, data.lon_2_deg, e2x, e2y, e2z);

cross(e2x, e2y, e2z, e3x, e3y, e3z, e1x, e1y, e1z);

normalize(e1x, e1y, e1z);

cross(e3x, e3y, e3z, e1x, e1y, e1z, e2x, e2y, e2z);

x_scale = data.x_scale;
y_scale = data.y_scale;

x_offset = data.x_offset;
y_offset = data.y_offset;

Nx = data.nx;
Ny = data.ny;

Name = data.name;

Data = data;

}


////////////////////////////////////////////////////////////////////////


void ExpGrid::clear()

{

e1x = 1.0;  e1y = 0.0;  e1z = 0.0;
e2x = 0.0;  e2y = 1.0;  e2z = 0.0;
e3x = 0.0;  e3y = 0.0;  e3z = 1.0;

x_scale = y_scale = 1.0;

x_offset = y_offset = 0.0;

Name.clear();

memset(&Data, 0, sizeof(Data));

}


////////////////////////////////////////////////////////////////////////


void ExpGrid::xy_to_latlon(double x, double y, double &lat, double &lon) const

{

double u, v, r;
double lat_new_deg, lon_new_deg;


xy_to_uv(x, y, u, v);

r = sqrt( u*u + v*v );


if ( r < 1.0e-5 )  {

   lat = lat_origin_deg;

   lon = lon_origin_deg;

   return;

}

lat_new_deg = 90.0 - deg_per_rad*r;

if ( (fabs(u) + fabs(v)) < 1.0e-5 )  lon_new_deg = 0.0;
else
   lon_new_deg = deg_per_rad*atan2(u, v);   //  NOT atan2(v, u);

new_to_old(lat_new_deg, lon_new_deg, lat, lon);

return;

}


////////////////////////////////////////////////////////////////////////


void ExpGrid::latlon_to_xy(double lat, double lon, double &x, double &y) const

{

double u, v, r;
double lat_new_deg, lon_new_deg;


old_to_new(lat, lon, lat_new_deg, lon_new_deg);


r = (90.0 - lat_new_deg)/deg_per_rad;

u = r*sin(lon_new_deg/deg_per_rad);

v = r*cos(lon_new_deg/deg_per_rad);

uv_to_xy(u, v, x, y);


return;

}


////////////////////////////////////////////////////////////////////////


int ExpGrid::nx() const

{

return ( Nx );

}


////////////////////////////////////////////////////////////////////////


int ExpGrid::ny() const

{

return ( Ny );

}


////////////////////////////////////////////////////////////////////////


ConcatString ExpGrid::name() const

{

return ( Name );

}


////////////////////////////////////////////////////////////////////////


double ExpGrid::calc_area(int x, int y) const

{

cerr << "\n\n  ExpGrid::calc_area() -> not yet implemented\n\n";

exit ( 1 );

return ( 0.0 );

}


////////////////////////////////////////////////////////////////////////


void ExpGrid::uv_to_xy(double u, double v, double &x, double &y) const

{

x = x_offset + x_scale*(earth_radius_km*u);

y = y_offset + y_scale*(earth_radius_km*v);

return;

}


////////////////////////////////////////////////////////////////////////


void ExpGrid::xy_to_uv(double x, double y, double &u, double &v) const

{

u = (x - x_offset)/(earth_radius_km*x_scale);

v = (y - y_offset)/(earth_radius_km*y_scale);

return;

}


////////////////////////////////////////////////////////////////////////


void ExpGrid::old_to_new(double lat_old_deg, double lon_old_deg, double &lat_new_deg, double &lon_new_deg) const

{

double px, py, pz;
double p_dot_e1, p_dot_e2, p_dot_e3;

pl_to_vect(lat_old_deg, lon_old_deg, px, py, pz);

p_dot_e1 = dot(px, py, pz, e1x, e1y, e1z);
p_dot_e2 = dot(px, py, pz, e2x, e2y, e2z);
p_dot_e3 = dot(px, py, pz, e3x, e3y, e3z);

vect_to_pl(p_dot_e1, p_dot_e2, p_dot_e3, lat_new_deg, lon_new_deg);

return;

}


////////////////////////////////////////////////////////////////////////


void ExpGrid::new_to_old(double lat_new_deg, double lon_new_deg, double &lat_old_deg, double &lon_old_deg) const

{

double px, py, pz;
double p_dot_e1, p_dot_e2, p_dot_e3;

pl_to_vect(lat_new_deg, lon_new_deg, p_dot_e1, p_dot_e2, p_dot_e3);

px = dot(p_dot_e1, p_dot_e2, p_dot_e3, e1x, e2x, e3x);
py = dot(p_dot_e1, p_dot_e2, p_dot_e3, e1y, e2y, e3y);
pz = dot(p_dot_e1, p_dot_e2, p_dot_e3, e1z, e2z, e3z);

vect_to_pl(px, py, pz, lat_old_deg, lon_old_deg);

return;

}


////////////////////////////////////////////////////////////////////////


void ExpGrid::dump(ostream & out, int depth) const

{

Indent prefix(depth);
char junk[256];


out << prefix << "Name       = ";

if ( Name.length() > 0 )  {

   out << '\"' << Name << '\"';

} else {

   out << "(nul)";

}

out << "\n";

out << prefix << "Projection = Exponential\n";

sprintf(junk, "%8.5f  %8.5f  %8.5f", e1x, e1y, e1z);

out << prefix << "E1         = [ " << junk << " ]\n";

sprintf(junk, "%8.5f  %8.5f  %8.5f", e2x, e2y, e2z);

out << prefix << "E2         = [ " << junk << " ]\n";

sprintf(junk, "%8.5f  %8.5f  %8.5f", e3x, e3y, e3z);

out << prefix << "E3         = [ " << junk << " ]\n";

out << prefix << "Lat Origin = " << lat_origin_deg << "\n";
out << prefix << "Lon Origin = " << lon_origin_deg << "\n";

out << prefix << "x_scale    = " << x_scale << "\n";
out << prefix << "y_scale    = " << y_scale << "\n";

out << prefix << "x_offset   = " << x_offset << "\n";
out << prefix << "y_offset   = " << y_offset << "\n";

out << prefix << "Nx         = " << Nx << "\n";
out << prefix << "Ny         = " << Ny << "\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString ExpGrid::serialize() const

{

ConcatString a;
char junk[256];

a << "Projection: Exponential";

a << " Nx: " << Nx;
a << " Ny: " << Ny;

sprintf(junk, " e1x: %.6f", e1x);   a << junk;
sprintf(junk, " e1y: %.6f", e1y);   a << junk;
sprintf(junk, " e1z: %.6f", e1z);   a << junk;

sprintf(junk, " e2x: %.6f", e2x);   a << junk;
sprintf(junk, " e2y: %.6f", e2y);   a << junk;
sprintf(junk, " e2z: %.6f", e2z);   a << junk;

sprintf(junk, " e3x: %.6f", e3x);   a << junk;
sprintf(junk, " e3y: %.6f", e3y);   a << junk;
sprintf(junk, " e3z: %.6f", e3z);   a << junk;

sprintf(junk, " lat_origin_deg: %.2f", lat_origin_deg);   a << junk;
sprintf(junk, " lon_origin_deg: %.2f", lon_origin_deg);   a << junk;

sprintf(junk, " x_scale: %.5f", x_scale);   a << junk;
sprintf(junk, " y_scale: %.5f", y_scale);   a << junk;

sprintf(junk, " x_offset: %.3f", x_offset);   a << junk;
sprintf(junk, " y_offset: %.3f", y_offset);   a << junk;

   //
   //  done
   //

return ( a );

}


////////////////////////////////////////////////////////////////////////


GridInfo ExpGrid::info() const

{

GridInfo i;

i.set(Data);

return ( i );

}


////////////////////////////////////////////////////////////////////////


double ExpGrid::rot_grid_to_earth(int x, int y) const

{

cerr << "\n\n  ExpGrid::rot_grid_to_earth() -> not yet implemented\n\n";

exit ( 1 );

return ( 0.0 );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void normalize(double &ax, double &ay, double &az)

{

double t;

t = sqrt( dot(ax, ay, az, ax, ay, az) );

if ( t < 1.0e-5 )  {

   cerr << "\n\n  normalize() -> vector nearly zero!\n\n";

   exit ( 1 );

}

t = 1.0/t;

ax *= t;
ay *= t;
az *= t;

return;

}


////////////////////////////////////////////////////////////////////////


double dot(double ax, double ay, double az, double bx, double by, double bz)

{

return ( ax*bx + ay*by + az*bz );

}


////////////////////////////////////////////////////////////////////////


void cross(double ax, double ay, double az, double bx, double by, double bz, double &cx, double &cy, double &cz)

{

cx = ay*bz - az*by;

cy = az*bx - ax*bz;

cz = ax*by - ay*bx;

return;

}


////////////////////////////////////////////////////////////////////////


void pl_to_vect(double lat_deg, double lon_deg, double &vx, double &vy, double &vz)

{

double lat_rad, lon_rad;

lat_rad = lat_deg/deg_per_rad;
lon_rad = lon_deg/deg_per_rad;

vx = cos(lat_rad)*sin(lon_rad);
vy = cos(lat_rad)*cos(lon_rad);
vz = sin(lat_rad);

return;

}


////////////////////////////////////////////////////////////////////////


void vect_to_pl(double vx, double vy, double vz, double &lat_deg, double &lon_deg)

{

double t;

t = 1.0/sqrt(vx*vx + vy*vy + vz*vz);

vx *= t;
vy *= t;
vz *= t;

lat_deg = deg_per_rad*asin(vz);

if ( (fabs(vx) + fabs(vy)) < 1.0e-5 )  lon_deg = 0;
else
   lon_deg = deg_per_rad*atan2(vx, vy);   //   NOT atan2(vy, vx) !!

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Grid code
   //


////////////////////////////////////////////////////////////////////////


Grid::Grid(const ExpData & data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const ExpData & data)

{

clear();

rep = new ExpGrid (data);

if ( !rep )  {

   cerr << "\n\n  Grid::set(const ExpData &) -> memory allocation error\n\n";

   exit ( 1 );

}

rep->refCount = 1;

return;

}


////////////////////////////////////////////////////////////////////////


