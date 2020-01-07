// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

ConcatString line;
StringArray a;

   // read the map region meta data
   if(!line.read_line(in)) return (false);

   // split up the meta data line
   a = line.split(" ");

   // check for the minumum number of elements
   if(a.n_elements() < min_region_header_elements) {
      mlog << Warning
           << "\nbool operator>>(istream & in, MapRegion & r) -> "
           << "found fewer than the expected number of elements ("
           << a.n_elements() << "<" << min_region_header_elements
           << ") in map region line:\n" << line << "\n\n";
      return(false);
   }

   // parse the region header line:
   //  - region number
   //  - the number of points for this region number
   //  - min/max latitude
   //  - min/max longitude
   //  - optional region description
   r.number   = atoi(a[0].c_str());
   r.n_points = atoi(a[1].c_str());
   r.lat_min  = atof(a[2].c_str());
   r.lat_max  = atof(a[3].c_str());
   r.lon_min  = atof(a[4].c_str());
   r.lon_max  = atof(a[5].c_str());

   // check that the number of points to read in is not greater than the size
   // of the arrays to hold the lat/lon point values
   if(r.n_points > max_region_points) {
      mlog << Error << "\noperator>>(ifstream &, MapRegion &) -> "
           << "map region has too many points\n\n";
      exit (1);
   }

   // parse the lat/lon data lines
   for(j=0; j<r.n_points; j++) {
      if(!line.read_line(in)) return (false);
      a = line.split(" ");
      r.lat[j] = atof(a[0].c_str());
      r.lon[j] = atof(a[1].c_str());
   }

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////





