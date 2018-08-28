

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
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

#include "rot_latlon_grid.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class RotatedLatLonGrid
   //


////////////////////////////////////////////////////////////////////////


RotatedLatLonGrid::RotatedLatLonGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


RotatedLatLonGrid::~RotatedLatLonGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void RotatedLatLonGrid::clear()

{

LatLonGrid::clear();

memset(&RData, 0, sizeof(RData));

return;

}


////////////////////////////////////////////////////////////////////////


RotatedLatLonGrid::RotatedLatLonGrid(const RotatedLatLonData & rdata)

{

set_from_rdata(rdata);

}


////////////////////////////////////////////////////////////////////////


void RotatedLatLonGrid::set_from_rdata(const RotatedLatLonData & rdata)

{

LatLonData lld;
double aux_rotation;
double rot_lat_ll, rot_lon_ll;

clear();

er.set_np(rdata.true_lat_north_pole, rdata.true_lon_north_pole, 0.0);

er.latlon_true_to_rot(rdata.true_lat_ll, rdata.true_lon_ll, rot_lat_ll, rot_lon_ll);




lld.name      = rdata.name;

lld.lat_ll    = rot_lat_ll;
lld.lon_ll    = rot_lon_ll;

lld.delta_lat = rdata.delta_new_lat;
lld.delta_lon = rdata.delta_new_lon;

lld.Nlat      = rdata.Nlat;
lld.Nlon      = rdata.Nlon;

LatLonGrid::set_from_data(lld);

RData = rdata;

return;

}


////////////////////////////////////////////////////////////////////////


void RotatedLatLonGrid::latlon_to_xy(double lat_true, double lon_true, double &x, double &y) const

{

double lat_rot, lon_rot;

er.latlon_true_to_rot(lat_true, lon_true, lat_true, lon_true);

LatLonGrid::latlon_to_xy(lat_rot, lon_rot, x, y);


return;

}


////////////////////////////////////////////////////////////////////////


void RotatedLatLonGrid::xy_to_latlon(double x, double y, double & lat_true, double & lon_true) const

{

double lat_rot, lon_rot;

LatLonGrid::xy_to_latlon(x, y, lat_rot, lon_rot);

er.latlon_rot_to_true(lat_rot, lon_rot, lat_true, lon_true);

return;

}


////////////////////////////////////////////////////////////////////////


double RotatedLatLonGrid::calc_area(int x, int y) const

{

double area = LatLonGrid::calc_area(x, y);

return ( area );

}



////////////////////////////////////////////////////////////////////////


int RotatedLatLonGrid::nx() const

{

return ( Nx );

}


////////////////////////////////////////////////////////////////////////


int RotatedLatLonGrid::ny() const

{

return ( Ny );

}


////////////////////////////////////////////////////////////////////////


ConcatString RotatedLatLonGrid::name() const

{

return ( Name );

}


////////////////////////////////////////////////////////////////////////


void RotatedLatLonGrid::dump(ostream & out, int depth) const

{


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString RotatedLatLonGrid::serialize() const

{

ConcatString a;

   //
   //  done
   //

return ( a );

}


////////////////////////////////////////////////////////////////////////


GridInfo RotatedLatLonGrid::info() const

{

GridInfo i;

i.set( RData );

return ( i );

}


////////////////////////////////////////////////////////////////////////


double RotatedLatLonGrid::rot_grid_to_earth(int x, int y) const

{

//
// The rotation angle from grid relative to earth relative is zero
// for the PlateCarreeGrids that are defined north and east.  This may
// need to be changed when support is added for GRIB2.
//

return ( 0.0 );

}


////////////////////////////////////////////////////////////////////////


bool RotatedLatLonGrid::is_global() const

{

const double lon_range = fabs((Nx + 1)*delta_lon);
const double lat_range = fabs((Ny + 1)*delta_lat);

const bool full_range_lat = (lat_range >= 180.0);
const bool full_range_lon = (lon_range >= 360.0);

const bool answer = full_range_lat && full_range_lon;

return ( answer );

}


////////////////////////////////////////////////////////////////////////


void RotatedLatLonGrid::shift_right(int N)

{


mlog << Error
     << "\n\n  RotatedLatLonGrid::shift_right(int) -> not implemented\n\n";

exit ( 1 );


/*
if ( N == 0 )  return;

if ( ! is_global() ) {


   mlog << Error
        << "\n\n  RotatedLatLonGrid::shift_right(int) -> "
        << "shifting is not allowed for non-global grids\n\n";

   exit ( 1 );

}

mlog << Warning
     << "Shifting global LatLon grid to the right " << N
     << " grid boxes.\n";

N %= Nx;

if ( N < 0 )  N += Nx;

if ( N == 0 )  return;


double new_lat_ll, new_lon_ll;

// xy_to_latlon( N, 0.0, new_lat_ll, new_lon_ll);
   xy_to_latlon(-N, 0.0, new_lat_ll, new_lon_ll);

lon_ll = new_lon_ll;

RData.latlon.lon_ll = new_lon_ll;
*/

return;

}


////////////////////////////////////////////////////////////////////////


GridRep * RotatedLatLonGrid::copy() const

{

RotatedLatLonGrid * p = new RotatedLatLonGrid (RData);

return ( p );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


Grid::Grid(const RotatedLatLonData & rdata)

{

init_from_scratch();

set(rdata);

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const RotatedLatLonData & rdata)

{

clear();

rep = new RotatedLatLonGrid ( rdata );

if ( !rep )  {

   mlog << Error
        << "\nGrid::set(const RotatedLatLonData &) -> memory allocation error\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


