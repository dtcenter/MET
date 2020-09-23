

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
#include "latlon_xyz.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class RotatedLatLonGrid
   //


////////////////////////////////////////////////////////////////////////


RotatedLatLonGrid::RotatedLatLonGrid()

{

clear();

}


// ////////////////////////////////////////////////////////////////////////


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

double angle;

    ////////////////////

angle = rdata.true_lon_south_pole;

er.set_rotz(angle);

    ////////////////////

angle = 90.0 + rdata.true_lat_south_pole;

angle = -angle;

er.pre_rotx(angle);

   //
   //  auilliary rotation, if any
   //

if ( rdata.aux_rotation != 0.0 )  {   //  rotate about grid center

   double lat, lon;
   double x, y, z;

   xy_to_latlon((rdata.Nlon)/2, (rdata.Nlat)/2, lat, lon);

   grid_latlon_to_xyz(lat, lon, x, y, z);

   angle = rdata.aux_rotation;

   angle = -angle;

   er.post_axis_angle(x, y, z, angle);

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
char junk[256];


a << "Projection: Rotated Lat/Lon";

a << " Nx: " << Nx;
a << " Ny: " << Ny;

snprintf(junk, sizeof(junk), " rot_lat_ll: %.3f", RData.rot_lat_ll);   a << junk;
snprintf(junk, sizeof(junk), " rot_lon_ll: %.3f", RData.rot_lon_ll);   a << junk;

snprintf(junk, sizeof(junk), " delta_rot_lat: %.3f", RData.delta_rot_lat);   a << junk;
snprintf(junk, sizeof(junk), " delta_rot_lon: %.3f", RData.delta_rot_lon);   a << junk;

snprintf(junk, sizeof(junk), " true_lat_south_pole: %.3f", RData.true_lat_south_pole);   a << junk;
snprintf(junk, sizeof(junk), " true_lon_south_pole: %.3f", RData.true_lon_south_pole);   a << junk;

snprintf(junk, sizeof(junk), " aux_rotation: %.3f", RData.aux_rotation);   a << junk;

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

if ( N == 0 )  return;

mlog << Error << "\nRotatedLatLonGrid::shift_right(int) -> "
     << "shifting is not implemented\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


GridRep * RotatedLatLonGrid::copy() const

{

RotatedLatLonGrid * p = new RotatedLatLonGrid (RData);

p->Name = Name;

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


