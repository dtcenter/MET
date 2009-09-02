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
#include <vx_data_grids/pc_grid.h>


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PlateCarreeGrid
   //


////////////////////////////////////////////////////////////////////////


PlateCarreeGrid::PlateCarreeGrid()

{

Name = (char *) 0;

clear();

}


////////////////////////////////////////////////////////////////////////


PlateCarreeGrid::~PlateCarreeGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void PlateCarreeGrid::clear()

{

pc_data.name          = (char *) 0;
pc_data.lat_ll_deg    = 0.0;
pc_data.lon_ll_deg    = 0.0;
pc_data.delta_lat_deg = 0.0;
pc_data.delta_lon_deg = 0.0;
pc_data.Nlat          = 0;
pc_data.Nlon          = 0;

if ( Name )  { delete [] Name;  Name = (char *) 0; }

Nx = Ny = 0;

lat_ll_deg = lon_ll_deg = 0.0;

delta_lat_deg = delta_lon_deg = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


PlateCarreeGrid::PlateCarreeGrid(const PlateCarreeData &data)

{

Name = (char *) 0;

clear();

pc_data = data;

lat_ll_deg = data.lat_ll_deg;
lon_ll_deg = data.lon_ll_deg;

delta_lat_deg = data.delta_lat_deg;
delta_lon_deg = data.delta_lon_deg;

Nx = data.Nlon;

Ny = data.Nlat;

Name = new char [1 + strlen(data.name)];

if ( !Name )  {

   cerr << "\n\n  PlateCarreeGrid::PlateCarreeGrid(const PlateCarreeData &) -> memory allocation error\n\n";

   exit ( 1 );

}

strcpy(Name, data.name);

}


////////////////////////////////////////////////////////////////////////


void PlateCarreeGrid::latlon_to_xy(double lat_deg, double lon_deg, double &x, double &y) const

{
double n;

y = (lat_deg - lat_ll_deg)/delta_lat_deg;

n = lon_ll_deg - lon_deg;
x = n - 360.0*floor(n/360.0);
x /= delta_lon_deg;

return;

}


////////////////////////////////////////////////////////////////////////


void PlateCarreeGrid::xy_to_latlon(double x, double y, double &lat_deg, double &lon_deg) const

{
double n;

// Valid latitude between -90 and 90
lat_deg = lat_ll_deg + delta_lat_deg*y;

// Valid longitude between -180 and 180
n = lon_ll_deg - delta_lon_deg*x + 180.0;
lon_deg = n - 360.0*floor(n/360.0);
lon_deg -= 180.0;

return;

}


////////////////////////////////////////////////////////////////////////


double PlateCarreeGrid::calc_area(int x, int y) const

{

const double R = EarthRadiusKM();

double area = 0.0;
double lat_center_deg, lon_center_deg;
double delta_lon_rad = delta_lon_deg * rad_per_deg;
//double delta_lon_rad = deg_to_rad(delta_lon_deg);
double lat_top_rad, lat_bottom_rad;


xy_to_latlon((double) x, (double) y, lat_center_deg, lon_center_deg);


lat_top_rad    = (lat_center_deg + 0.5*delta_lat_deg) * rad_per_deg;
lat_bottom_rad = (lat_center_deg - 0.5*delta_lat_deg) * rad_per_deg;
//lat_top_rad    = deg_to_rad(lat_center_deg + 0.5*delta_lat_deg);
//lat_bottom_rad = deg_to_rad(lat_center_deg - 0.5*delta_lat_deg);

area = ( sin(lat_top_rad) - sin(lat_bottom_rad) )*delta_lon_rad;

area = fabs(area)*R*R;


return ( area );

}


////////////////////////////////////////////////////////////////////////


double PlateCarreeGrid::calc_area_ll(int x, int y) const

{

const double R = EarthRadiusKM();

double area = 0.0;
double lat_deg, lon_deg;
double delta_lon_rad = delta_lon_deg * rad_per_deg;
double lat_top_rad, lat_bottom_rad;


xy_to_latlon((double) x, (double) y, lat_deg, lon_deg);


lat_top_rad    = (lat_deg + 1.0*delta_lat_deg) * rad_per_deg;
lat_bottom_rad = (lat_deg - 0.0*delta_lat_deg) * rad_per_deg;

area = ( sin(lat_top_rad) - sin(lat_bottom_rad) )*delta_lon_rad;

area = fabs(area)*R*R;


return ( area );

}


////////////////////////////////////////////////////////////////////////


int PlateCarreeGrid::nx() const

{

return ( Nx );

}


////////////////////////////////////////////////////////////////////////


int PlateCarreeGrid::ny() const

{

return ( Ny );

}


////////////////////////////////////////////////////////////////////////


double PlateCarreeGrid::EarthRadiusKM() const

{

return ( earth_radius_km );

}


////////////////////////////////////////////////////////////////////////


const char * PlateCarreeGrid::name() const

{

return ( (const char *) Name );

}


////////////////////////////////////////////////////////////////////////


ProjType PlateCarreeGrid::proj_type() const

{

return ( PlateCarreeProj );

}


////////////////////////////////////////////////////////////////////////


double PlateCarreeGrid::rot_grid_to_earth(int x, int y) const

{

//
// The rotation angle from grid relative to earth relative is zero
// for the PlateCarreeGrids that are defined north and east.  This may
// need to be changed when support is added for GRIB2.
//

return ( 0.0 );

}


////////////////////////////////////////////////////////////////////////


void PlateCarreeGrid::grid_data(GridData &gdata) const

{

gdata.pc_data = pc_data;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


Grid::Grid(const PlateCarreeData &data)

{

rep = (GridRep *) 0;

rep = new PlateCarreeGrid ( data );

if ( !rep )  {

   cerr << "\n\n  Grid::Grid(const PlateCarreeData &) -> memory allocation error\n\n";

   exit ( 1 );

}

rep->refCount = 1;

}


////////////////////////////////////////////////////////////////////////


