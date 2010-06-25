
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_math/vx_math.h"
#include "vx_util/vx_util.h"
#include "vx_data_grids/latlon_grid.h"


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

lat_ll_deg = lon_ll_deg = 0.0;

delta_lat_deg = delta_lon_deg = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


LatLonGrid::LatLonGrid(const LatLonData & data)

{

clear();

lat_ll_deg = data.lat_ll_deg;
lon_ll_deg = data.lon_ll_deg;

delta_lat_deg = data.delta_lat_deg;
delta_lon_deg = data.delta_lon_deg;

Nx = data.Nlon;

Ny = data.Nlat;

Name = data.name;

}


////////////////////////////////////////////////////////////////////////


void LatLonGrid::latlon_to_xy(double lat_deg, double lon_deg, double &x, double &y) const

{

y = (lat_deg - lat_ll_deg)/delta_lat_deg;

x = (lon_ll_deg - lon_deg)/delta_lon_deg;

return;

}


////////////////////////////////////////////////////////////////////////


void LatLonGrid::xy_to_latlon(double x, double y, double &lat_deg, double &lon_deg) const

{

lat_deg = lat_ll_deg + delta_lat_deg*y;

lon_deg = lon_ll_deg - delta_lon_deg*x;

return;

}


////////////////////////////////////////////////////////////////////////


double LatLonGrid::calc_area(int x, int y) const

{

double area = 0.0;
double lat_center_deg, lon_center_deg;
double delta_lon_rad = delta_lon_deg/deg_per_rad;
double lat_top_rad, lat_bottom_rad;


xy_to_latlon((double) x, (double) y, lat_center_deg, lon_center_deg);


lat_top_rad    = (lat_center_deg + 0.5*delta_lat_deg)/deg_per_rad;
lat_bottom_rad = (lat_center_deg - 0.5*delta_lat_deg)/deg_per_rad;

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

out << prefix << "lat_ll       = " << lat_ll_deg << "\n";
out << prefix << "lon_ll       = " << lon_ll_deg << "\n";

out << prefix << "delta_lat_ll = " << delta_lat_deg << "\n";
out << prefix << "delta_lon_ll = " << delta_lon_deg << "\n";

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

sprintf(junk, " lat_ll_deg: %.3f", lat_ll_deg);   a << junk;
sprintf(junk, " lon_ll_deg: %.3f", lon_ll_deg);   a << junk;

sprintf(junk, " delta_lat_deg: %.3f", delta_lat_deg);   a << junk;
sprintf(junk, " delta_lon_deg: %.3f", delta_lon_deg);   a << junk;

   //
   //  done
   //

return ( a );

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

   cerr << "\n\n  Grid::set(const LatLonData &) -> memory allocation error\n\n";

   exit ( 1 );

}

rep->refCount = 1;

return;

}


////////////////////////////////////////////////////////////////////////


