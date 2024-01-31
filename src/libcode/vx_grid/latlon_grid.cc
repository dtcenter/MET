// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_math.h"
#include "vx_util.h"
#include "vx_log.h"
#include "latlon_grid.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class LatLonGrid
   //


////////////////////////////////////////////////////////////////////////


LatLonGrid::LatLonGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


LatLonGrid::~LatLonGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void LatLonGrid::clear()

{

Name.clear();

Nx = Ny = 0;

lat_ll = lon_ll = 0.0;

delta_lat = delta_lon = 0.0;

wrapLon = false;

memset(&Data, 0, sizeof(Data));

return;

}


////////////////////////////////////////////////////////////////////////


LatLonGrid::LatLonGrid(const LatLonData & data)

{

set_from_data(data);

}


////////////////////////////////////////////////////////////////////////


void LatLonGrid::set_from_data(const LatLonData & data)

{


clear();

lat_ll = data.lat_ll;
lon_ll = data.lon_ll;

delta_lat = data.delta_lat;
delta_lon = data.delta_lon;

Nx = data.Nlon;

Ny = data.Nlat;

Name = data.name;

Data = data;

   //
   //  wrap longitudes when:
   //    - all 360 degrees are included
   //    - 360 is divisible by delta_lon
   //

bool   lon_rng_flag = fabs((Nx + 1)*delta_lon) >= 360.0;
double lon_div      = 360.0 / delta_lon;
bool   lon_div_flag = is_eq(lon_div, floor(lon_div));

wrapLon = (lon_rng_flag && lon_div_flag);

   //
   //  inform users about unexpected delta longitudes
   //

if ( lon_rng_flag && !lon_div_flag )  {

   mlog << Debug(4) << "Cannot wrap longitudes since 360 is not "
        << "evenly divisible by delta_lon = "
        << delta_lon << ".\n";

}

return;

}


////////////////////////////////////////////////////////////////////////


void LatLonGrid::latlon_to_xy(double lat, double lon, double &x, double &y) const

{

double n;

y = (lat - lat_ll)/delta_lat;

n = lon_ll - lon;
x = n - 360.0*floor(n/360.0);
x /= delta_lon;


return;

}


////////////////////////////////////////////////////////////////////////


void LatLonGrid::xy_to_latlon(double x, double y, double &lat, double &lon) const

{

lat = lat_ll + delta_lat*y;

lon = lon_ll - delta_lon*x;

return;

}


////////////////////////////////////////////////////////////////////////


double LatLonGrid::calc_area(int x, int y) const

{

double area, lat_bottom, lon_left;
double delta_lon_rad = delta_lon/deg_per_rad;
double lat_top_rad, lat_bottom_rad;


xy_to_latlon((double) x, (double) y, lat_bottom, lon_left);


lat_top_rad     = (lat_bottom + delta_lat)/deg_per_rad;
lat_bottom_rad  = lat_bottom/deg_per_rad;

area = ( sin(lat_top_rad) - sin(lat_bottom_rad) )*delta_lon_rad;

area = fabs(area)*earth_radius_km*earth_radius_km;


return ( area );

}


////////////////////////////////////////////////////////////////////////


int LatLonGrid::nx() const

{

return ( Nx );

}


////////////////////////////////////////////////////////////////////////


int LatLonGrid::ny() const

{

return ( Ny );

}


////////////////////////////////////////////////////////////////////////


ConcatString LatLonGrid::name() const

{

return ( Name );

}


////////////////////////////////////////////////////////////////////////


void LatLonGrid::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Name         = ";

if ( Name.length() > 0 )  out << '\"' << Name << '\"';
else                      out << "(nul)\n";

out << '\n';

out << prefix << "lat_ll       = " << lat_ll << "\n";
out << prefix << "lon_ll       = " << lon_ll << "\n";

out << prefix << "delta_lat_ll = " << delta_lat << "\n";
out << prefix << "delta_lon_ll = " << delta_lon << "\n";

out << prefix << "wrapLon      = " << bool_to_string(wrapLon) << "\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString LatLonGrid::serialize(const char *sep) const

{

ConcatString a;
char junk[256];


a << "Projection: Lat/Lon" << sep;

a << "Nx: " << Nx << sep;
a << "Ny: " << Ny << sep;

snprintf(junk, sizeof(junk), "lat_ll: %.3f", lat_ll);   a << junk << sep;
snprintf(junk, sizeof(junk), "lon_ll: %.3f", lon_ll);   a << junk << sep;

snprintf(junk, sizeof(junk), "delta_lat: %.3f", delta_lat);   a << junk << sep;
snprintf(junk, sizeof(junk), "delta_lon: %.3f", delta_lon);   a << junk << sep;

snprintf(junk, sizeof(junk), "wrapLon: %s", bool_to_string(wrapLon));   a << junk;

   //
   //  done
   //

return ( a );

}


////////////////////////////////////////////////////////////////////////


GridInfo LatLonGrid::info() const

{

GridInfo i;

i.set( Data );

return ( i );

}


////////////////////////////////////////////////////////////////////////


double LatLonGrid::rot_grid_to_earth(int x, int y) const

{

//
// The rotation angle from grid relative to earth relative is zero
// for the PlateCarreeGrids that are defined north and east.
//

return ( 0.0 );

}


////////////////////////////////////////////////////////////////////////


void LatLonGrid::shift_right(int N)

{

if ( N == 0 )  return;

if ( ! wrapLon ) {

   mlog << Error
        << "\n\n  LatLonGrid::shift_right(int) -> "
        << "shifting is not allowed for non-global grids\n\n";

   exit ( 1 );

}

mlog << Debug(3)
     << "Shifting global LatLon grid to the right " << N
     << " grid boxes.\n";

N %= Nx;

if ( N < 0 )  N += Nx;

if ( N == 0 )  return;

double new_lat_ll, new_lon_ll;

xy_to_latlon((double) -1.0*N, 0.0, new_lat_ll, new_lon_ll);

lon_ll = new_lon_ll;

Data.lon_ll = new_lon_ll;

return;

}


////////////////////////////////////////////////////////////////////////


GridRep * LatLonGrid::copy() const

{

LatLonGrid * p = new LatLonGrid (Data);

p->Name = Name;

return ( p );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


Grid::Grid(const LatLonData & data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const LatLonData & data)

{

clear();

rep = new LatLonGrid ( data );

if ( !rep )  {

   mlog << Error << "\nGrid::set(const LatLonData &) -> memory allocation error\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////
