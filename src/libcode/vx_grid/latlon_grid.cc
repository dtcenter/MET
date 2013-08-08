

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2013
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

memset(&Data, 0, sizeof(Data));

return;

}


////////////////////////////////////////////////////////////////////////


LatLonGrid::LatLonGrid(const LatLonData & data)

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

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString LatLonGrid::serialize() const

{

ConcatString a;
char junk[256];


a << "Projection: Lat/Lon";

a << " Nx: " << Nx;
a << " Ny: " << Ny;

sprintf(junk, " lat_ll: %.3f", lat_ll);   a << junk;
sprintf(junk, " lon_ll: %.3f", lon_ll);   a << junk;

sprintf(junk, " delta_lat: %.3f", delta_lat);   a << junk;
sprintf(junk, " delta_lon: %.3f", delta_lon);   a << junk;

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
// for the PlateCarreeGrids that are defined north and east.  This may
// need to be changed when support is added for GRIB2.
//

return ( 0.0 );

}


////////////////////////////////////////////////////////////////////////


bool LatLonGrid::is_global() const

{

const bool full_range_lat = (fabs(Ny*delta_lat) >= 180.0);
const bool full_range_lon = (fabs(Nx*delta_lon) >= 360.0);

return ( full_range_lat && full_range_lon );

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

rep->refCount = 1;

return;

}


////////////////////////////////////////////////////////////////////////


