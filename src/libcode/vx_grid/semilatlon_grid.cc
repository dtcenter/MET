// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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
#include "semilatlon_grid.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SemiLatLonGrid
   //


////////////////////////////////////////////////////////////////////////


SemiLatLonGrid::SemiLatLonGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


SemiLatLonGrid::~SemiLatLonGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


SemiLatLonGrid::SemiLatLonGrid(const SemiLatLonData & data)

{

clear();

if ( data.name )  Name = data.name;

  // Process the lat/lon dimensions

Lats = data.lats;
Lons = data.lons;

if ( Lons.n() > 0 )  xDim = &Lons;
if ( Lats.n() > 0 )  yDim = &Lats;

  // Process the other dimensions

add_dimension(data.levels, Levels);
add_dimension(data.times,  Times);

if ( !xDim || !yDim )  {

   mlog << Error << "\nSemiLatLonGrid::SemiLatLonGrid(const SemiLatLonData & data) -> "
        << "exactly two dimensions should have non-zero length: lats ("
        << Lats.n() << "), lons (" << Lons.n() << "), levels (" << Levels.n()
        << "), times (" << Times.n() << ")\n\n";
   exit ( 1 );

}

Nx = xDim->n();
Ny = yDim->n();

   // 1-dimensional array of lat/lon locations

if ( Lats.n() > 0 && Lons.n() > 0 )  {

   Is2Dim = false;

   if ( Lats.n() != Lons.n() )  {

      mlog << Error << "\nSemiLatLonGrid::SemiLatLonGrid(const SemiLatLonData & data) -> "
           << "for one dimensional arrays, the number lats and lons must match ("
           << Lats.n() << " != " << Lons.n() << ")!\n\n";
      exit ( 1 );

   }

}

   // 2-dimensional data

else {

   Is2Dim = true;

}

   // Store the data

Data = data;

return;

}


////////////////////////////////////////////////////////////////////////


void SemiLatLonGrid::add_dimension(const NumArray &data, NumArray &dim)

{

   // store the dimension data

dim = data;

   // set dimensions is non-zero

if ( dim.n() == 0 )  return;

     if ( !xDim )  xDim = &dim;
else if ( !yDim )  yDim = &dim;
else {

   mlog << Error << "\nvoid SemiLatLonGrid::add_dimension() -> "
        << "more than two non-zero dimensions!\n\n";
   exit ( 1 );

}

return;

}

////////////////////////////////////////////////////////////////////////


void SemiLatLonGrid::clear()

{

Name.clear();

Lats.clear();
Lons.clear();
Levels.clear();
Times.clear();

xDim = (NumArray *) 0;
yDim = (NumArray *) 0;

Nx = 0;
Ny = 0;

memset(&Data, 0, sizeof(Data));

return;

}


////////////////////////////////////////////////////////////////////////


void SemiLatLonGrid::latlon_to_xy(double lat, double lon, double & x, double & y) const

{

if ( !xDim || !yDim )  {
   mlog << Error << "\nSemiLatLonGrid::latlon_to_xy() -> "
        << "dimensions not defined!\n\n";
   exit ( 1 );
}

int i, x_int, y_int;
x_int = y_int = bad_data_int;

// Search dimensions separately for matches
if ( Is2Dim ) {

   // Search the first dimension
   for(i=0; i<xDim->n(); i++) {
      if(is_eq(lat, xDim->buf()[i])) {
         x_int = i;
         break;
      }
   }

   // Search the second dimension
   for(i=0; i<yDim->n(); i++) {
      if(is_eq(lat, yDim->buf()[i])) {
         y_int = i;
         break;
      }
   }
}
else {

   // Search both dimensions
   for(i=0; i<xDim->n(); i++) {
      if(is_eq(lat, xDim->buf()[i]) && is_eq(lon, yDim->buf()[i])) {
         x_int = y_int = i;
         break;
      }
   }
}

if ( is_bad_data(x_int) || is_bad_data(y_int) )  {
   mlog << Error << "\nSemiLatLonGrid::latlon_to_xy() -> "
        << "no match found for (" << lat << ", " << lon << ").\n\n";
   exit ( 1 );
}

x = (double) x_int;
y = (double) y_int;

return;

}


////////////////////////////////////////////////////////////////////////


void SemiLatLonGrid::xy_to_latlon(double x, double y, double & lat, double & lon) const

{

if ( !xDim || !yDim )  {
   mlog << Error << "\nSemiLatLonGrid::xy_to_latlon() -> "
        << "dimensions not defined!\n\n";
   exit ( 1 );
}

int x_int = nint(x);
int y_int = nint(y);

if ( x_int < 0 || x_int >= Nx ||
     y_int < 0 || y_int >= Ny )  {
   mlog << Error << "\nSemiLatLonGrid::xy_to_latlon() -> "
        << "(" << x_int << ", " << y_int << ") outside of the grid dimension ("
        << Nx << ", " << Ny <<")!\n\n";
   exit ( 1 );
}

lat = xDim->buf()[x_int];
lon = yDim->buf()[y_int];

return;

}


////////////////////////////////////////////////////////////////////////


double SemiLatLonGrid::calc_area(int x, int y) const

{

   // Grid cell area is not defined for semilatlon grids

return ( bad_data_double );

}


////////////////////////////////////////////////////////////////////////


int SemiLatLonGrid::nx() const

{

return ( Nx );

}


////////////////////////////////////////////////////////////////////////


int SemiLatLonGrid::ny() const

{

return ( Ny );

}


////////////////////////////////////////////////////////////////////////


ConcatString SemiLatLonGrid::name() const

{

return ( Name );

}


////////////////////////////////////////////////////////////////////////


void SemiLatLonGrid::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "Name   = ";

if ( Name.length() > 0 )  out << '\"' << Name << '\"';
else                      out << "(nul)\n";

out << '\n';

out << prefix << "Projection = SemiLatLon\n";

out << prefix << "Lats:\n";
Lats.dump(out, depth+1);

out << prefix << "Lons:\n";
Lons.dump(out, depth+1);

out << prefix << "Levels:\n";
Levels.dump(out, depth+1);

out << prefix << "Times:\n";
Times.dump(out, depth+1);

out << prefix << "Is2Dim = " << bool_to_string(Is2Dim) << "\n";
out << prefix << "Nx     = " << Nx << "\n";
out << prefix << "Ny     = " << Ny << "\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString SemiLatLonGrid::serialize(const char *sep) const

{

ConcatString a;

a << "Projection: SemiLatLon" << sep;

a << "Nx: " << Nx << sep;
a << "Ny: " << Ny << sep;

a << "Lats: " << Lats.serialize() << sep;
a << "Lons: " << Lons.serialize() << sep;
a << "Levels: " << Levels.serialize() << sep;
a << "Times: " << Times.serialize() << sep;

   //
   //  done
   //

return ( a );

}


////////////////////////////////////////////////////////////////////////


GridInfo SemiLatLonGrid::info() const

{

GridInfo i;

i.set(Data);

return ( i );

}


////////////////////////////////////////////////////////////////////////


double SemiLatLonGrid::rot_grid_to_earth(int x, int y) const

{

//
// The rotation angle from grid relative to earth relative
// does not apply to semilatlon grids
//

return ( 0.0 );

}


////////////////////////////////////////////////////////////////////////


bool SemiLatLonGrid::wrap_lon() const

{

return ( false );

}


////////////////////////////////////////////////////////////////////////


void SemiLatLonGrid::shift_right(int N)

{

mlog << Error << "\nSemiLatLonGrid::shift_right(int) -> "
     << "shifting is not allowed for non-global grids\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


GridRep * SemiLatLonGrid::copy() const

{

SemiLatLonGrid * p = new SemiLatLonGrid (Data);

p->Name = Name;

return ( p );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


Grid::Grid(const SemiLatLonData & data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const SemiLatLonData & data)

{

clear();

rep = new SemiLatLonGrid (data);

if ( !rep )  {

   mlog << Error << "\nGrid::set(const SemiLatLonData &) -> "
        << "memory allocation error\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////
