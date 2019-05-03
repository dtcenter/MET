

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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


static double     st_func (double lat, bool is_north_hemisphere);
static double st_der_func (double lat, bool is_north_hemisphere);

static double st_inv_func (double r, bool is_north_hemisphere);

static void reduce(double & angle);


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

Nx = data.nx;
Ny = data.ny;

switch ( data.hemisphere )  {

   case 'N':  IsNorthHemisphere = true;   break;
   case 'S':  IsNorthHemisphere = false;  break;

   default:
      mlog << Error << "\nStereographicGrid::StereographicGrid(const StereographicData &) -> bad hemisphere ...\""
           << (data.hemisphere) << "\"\n\n";
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


void reduce(double & angle)

{

angle -= 360.0*floor( (angle/360.0) + 0.5 );

return;

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

   mlog << Error << "\nGrid::set(const StereographicData &) -> memory allocation error\n\n";

   exit ( 1 );

}

rep->refCount = 1;

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

rep->refCount = 1;

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

rep->refCount = 1;

return;

}
*/

////////////////////////////////////////////////////////////////////////






