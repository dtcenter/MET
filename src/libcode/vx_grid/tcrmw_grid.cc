// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
#include <cmath>

#include "tcrmw_grid.h"

#include "trig.h"


////////////////////////////////////////////////////////////////////////



static const double tol = 1.0e-4;

static const double km_per_deg = (pi/180.0)*earth_radius_km;

static const double deg_per_km = 1.0/km_per_deg;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class TcrmwGrid
   //


////////////////////////////////////////////////////////////////////////


TcrmwGrid::TcrmwGrid()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


TcrmwGrid::TcrmwGrid(const TcrmwData & d)

{

init_from_scratch();

set_from_data(d);

}


////////////////////////////////////////////////////////////////////////


TcrmwGrid::~TcrmwGrid()

{

   //
   //  should be inline
   //

}


////////////////////////////////////////////////////////////////////////


TcrmwGrid::TcrmwGrid(const TcrmwGrid & tg)

{

init_from_scratch();

assign(tg);

}


////////////////////////////////////////////////////////////////////////


TcrmwGrid & TcrmwGrid::operator=(const TcrmwGrid & tg)

{

if ( this == &tg )   return *this;

assign(tg);

return *this;

}


////////////////////////////////////////////////////////////////////////


void TcrmwGrid::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void TcrmwGrid::clear()

{

Ir.set_xyz(1.0, 0.0, 0.0);
Jr.set_xyz(0.0, 1.0, 0.0);
Kr.set_xyz(0.0, 0.0, 1.0);

Range_n = 0;
Azimuth_n = 0;

Range_max_km = 0.0;

Lat_Center_Deg = Lon_Center_Deg = 0.0;



return;

}


////////////////////////////////////////////////////////////////////////


void TcrmwGrid::assign(const TcrmwGrid & tg)

{

clear();

Ir = tg.Ir;
Jr = tg.Jr;
Kr = tg.Kr;

Range_n  = tg.Range_n;
Azimuth_n = tg.Azimuth_n;

Range_max_km = tg.Range_max_km;

Lat_Center_Deg = tg.Lat_Center_Deg;
Lon_Center_Deg = tg.Lon_Center_Deg;


return;

}


////////////////////////////////////////////////////////////////////////


void TcrmwGrid::set_from_data(const TcrmwData & d)

{

clear();


TData = d;

Range_n = d.range_n;

Azimuth_n = d.azimuth_n;

Range_max_km = d.range_max_km;

Lat_Center_Deg = d.lat_center;

Lon_Center_Deg = d.lon_center;



calc_ijk();

const double range_max_deg = Range_max_km*deg_per_km;

RotatedLatLonData RLLD;

RLLD.name       = d.name;

RLLD.rot_lat_ll = 90.0 - range_max_deg;
RLLD.rot_lon_ll =  0.0;

RLLD.delta_rot_lat = range_max_deg/(Range_n - 1);

// MET #2833 divide by n rather than n-1 for the azimuth increment
RLLD.delta_rot_lon = 360.0/Azimuth_n;

RLLD.Nlat = Range_n;
RLLD.Nlon = Azimuth_n;

RLLD.true_lat_south_pole = -Lat_Center_Deg;
RLLD.true_lon_south_pole = Lon_Center_Deg + 180.0;

RLLD.aux_rotation = 180.0 - Lon_Center_Deg;



RotatedLatLonGrid::set_from_rdata(RLLD);


er.set_tcrmw(Lat_Center_Deg, Lon_Center_Deg);


return;

}


////////////////////////////////////////////////////////////////////////


void TcrmwGrid::calc_ijk()

{

const Vector K (0.0, 0.0, 1.0);

Kr.set_latlon(Lat_Center_Deg, Lon_Center_Deg);

Ir = cross(K, Kr);

double len = Ir.abs();

if ( len < tol )  {

   mlog << Error << "\nTcrmwGrid::calc_ijk() -> "
        << "rotated poles too close to original poles!\n\n";

   exit ( 1 );

}

Ir.normalize();

Jr = cross(Kr, Ir);


return;

}


////////////////////////////////////////////////////////////////////////


void TcrmwGrid::range_azi_to_latlon(const double range_km, const double azi_deg, double & lat, double & lon) const

{

const double range_deg = deg_per_km*range_km;
const double lat_rot   = 90.0 - range_deg;
const double lon_rot   = azi_deg;
double x, y;

y = (lat_rot - RData.rot_lat_ll)/(RData.delta_rot_lat);

x = lon_rot/(RData.delta_rot_lon);


RotatedLatLonGrid::xy_to_latlon(x, y, lat, lon);


return;

}


////////////////////////////////////////////////////////////////////////


void TcrmwGrid::latlon_to_range_azi(const double lat, const double lon, double & range_km, double & azi_deg) const

{

double range_deg;
double x, y;
const double range_max_deg = deg_per_km*Range_max_km;

RotatedLatLonGrid::latlon_to_xy(lat, lon, x, y);

azi_deg = x*(RData.delta_rot_lon);

range_deg = range_max_deg - y*(RData.delta_rot_lat);

range_km = range_deg*km_per_deg;

return;

}


////////////////////////////////////////////////////////////////////////


void TcrmwGrid::wind_ne_to_rt (const double azi_deg,
                               const double u_wind, const double v_wind,
                               double & radial_wind, double & tangential_wind) const

{

double rcos = cosd(azi_deg);
double rsin = sind(azi_deg);

if (is_bad_data(u_wind) || is_bad_data(v_wind)) {
   radial_wind     = bad_data_double;
   tangential_wind = bad_data_double;   
}
else {
   radial_wind     =      rcos*u_wind + rsin*v_wind;
   tangential_wind = -1.0*rsin*u_wind + rcos*v_wind;
}

return;

}


////////////////////////////////////////////////////////////////////////


void TcrmwGrid::wind_ne_to_rt (const double lat, const double lon, 
                               const double u_wind, const double v_wind,
                               double & radial_wind, double & tangential_wind) const

{

double range_km, azi_deg;

latlon_to_range_azi(lat, lon, range_km, azi_deg);

wind_ne_to_rt(azi_deg, u_wind, v_wind, radial_wind, tangential_wind);

return;

}


////////////////////////////////////////////////////////////////////////


void TcrmwGrid::latlon_to_xy(double true_lat, double true_lon, double & x, double & y) const

{

RotatedLatLonGrid::latlon_to_xy(true_lat, true_lon, x, y);

x -= Nx*floor(x/Nx);

y -= Ny*floor(y/Ny);

return;

}


////////////////////////////////////////////////////////////////////////


void TcrmwGrid::xy_to_latlon(double x, double y, double & true_lat, double & true_lon) const

{

x -= Nx*floor(x/Nx);

y -= Ny*floor(y/Ny);

RotatedLatLonGrid::xy_to_latlon(x, y, true_lat, true_lon);

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


Grid::Grid(const TcrmwData & data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const TcrmwData & data)

{

clear();

rep = new TcrmwGrid ( data );

if ( !rep )  {

   mlog << Error << "\nGrid::set(const TcrmwData &) -> "
        << "memory allocation error\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////

