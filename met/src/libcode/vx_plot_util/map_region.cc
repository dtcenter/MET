

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_log.h"
#include "map_region.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MapRegion
   //


////////////////////////////////////////////////////////////////////////


MapRegion::MapRegion()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


MapRegion::~MapRegion()

{

clear();

}


////////////////////////////////////////////////////////////////////////


MapRegion::MapRegion(const MapRegion & r)

{

init_from_scratch();

assign(r);

}


////////////////////////////////////////////////////////////////////////


MapRegion & MapRegion::operator=(const MapRegion & r)

{

if ( this == &r )  return ( * this );

assign(r);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void MapRegion::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void MapRegion::clear()

{

number = 0;
n_points = 0;

lat_min = lat_max = lon_min = lon_max = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


void MapRegion::assign(const MapRegion & r)

{

clear();

number = r.number;

n_points = r.n_points;

lat_min = r.lat_min;
lat_max = r.lat_max;

lon_min = r.lon_min;
lon_max = r.lon_max;

memcpy(lat, r.lat, max_region_points*sizeof(double));
memcpy(lon, r.lon, max_region_points*sizeof(double));

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


bool operator>>(istream & in, MapRegion & r)

{

int j;

r.clear();

   // read in the region number, the number of points for this region number,
   // and the a, b, and c values
   in >> r.number >> r.n_points;

   // check for end of file
   if (!in) return (false);

   // read in the minimum latitude, the maximum latitude, the minimum longitude,
   // and the maximum longitude for this region number
   in >> r.lat_min >> r.lat_max >> r.lon_min >> r.lon_max;

   // check that the number of points to read in is not greater than the size
   // of the arrays to hold the lat/lon point values
   if(r.n_points > max_region_points) {
      mlog << Error << "\noperator>>(ifstream &, MapRegion &) ->"
           << "map region has too many points\n\n";
      exit (1);
   }

   // read in the lat/lon point values
   for(j=0; j<r.n_points; j++) {
      in >> r.lat[j] >> r.lon[j];
   }

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////





