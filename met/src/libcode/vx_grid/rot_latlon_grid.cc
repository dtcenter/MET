

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


clear();



   //
   //  set the parent projection
   //

lld.name      = rdata.name;

lld.lat_ll    = rdata.rot_lat_ll;
lld.lon_ll    = rdata.rot_lon_ll;

lld.delta_lat = rdata.delta_rot_lat;
lld.delta_lon = rdata.delta_rot_lon;

lld.Nlat      = rdata.Nlat;
lld.Nlon      = rdata.Nlon;

LatLonGrid::set_from_data(lld);

RData = rdata;

   //
   //  determine the Earth Rotation
   //

      //
      //    From http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/grib2_temp3-1.shtml
      //
      //
      //   2.  Three parameters define a general latitude/longitude coordinate system, formed by a 
      //       general rotation of the sphere. One choice for these parameters is:
      //       (a) The geographic latitude in degrees of the southern pole of the 
      //           coordinate system,06 for example.
      //       (b) The geographic longitude in degrees of the southern pole of 
      //           the coordinate system,#p for example.
      //       (c) The angle of rotation in degrees about the new polar axis 
      //           (measured clockwise when looking from the southern to the northern pole) 
      //           of the coordinate system, assuming the new axis to have been obtained by 
      //           first rotating the sphere through lon_p degrees about the geographic polar 
      //           axis and then rotating through (90 + lat_p) degrees so that the southern pole 
      //           moved along the (previously rotated) Greenwich meridian.
      //



// double rlat, rlon;
double angle;

    ////////////////////

angle = rdata.true_lon_south_pole;

// angle = -angle;

er.set_rotz(angle);   // cout << "rotz by " << angle << "\n";

// er.latlon_true_to_rot(40.0, 0.0, rlat, rlon);
// 
// cout << "rlat = " << rlat << " ... rlon = " << rlon << "\n";

    ////////////////////

angle = 90.0 + rdata.true_lat_south_pole;

angle = -angle;

er.pre_rotx(angle);   // cout << "pre rotx by " << angle << "\n";



// er.latlon_rot_to_true(-90.0, 100.0, rlat, rlon);
// 
// cout << "  south pole is at rlat = " << rlat << " ... rlon = " << rlon << "\n";


   //
   //  auilliary rotation, if any
   //

if ( rdata.aux_rotation != 0.0 )  {

   angle = rdata.aux_rotation;

   angle = -angle;

   er.post_rotz(angle);

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void RotatedLatLonGrid::latlon_to_xy(double lat_true, double lon_true, double &x, double &y) const

{

double lat_rot, lon_rot;

er.latlon_true_to_rot(lat_true, lon_true, lat_rot, lon_rot);

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

RData.dump(out, depth);



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


