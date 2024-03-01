// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


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


using namespace std;


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

if ( Lats.n() > 0 && Lons.n() > 0 )  {

   if ( Lats.n() != Lons.n() )  {
      mlog << Error << "\nSemiLatLonGrid::SemiLatLonGrid(const SemiLatLonData & data) -> "
           << "when both are specified, the number of lats (" << Lats.n()
           << ") and lons (" << Lons.n() << ") must match.\n\n";
      exit ( 1 );
   }

   IsLatLon = true;
   xDim = &Lons;
}
else if ( Lons.n() > 0 )  { xDim = &Lons; }
else if ( Lats.n() > 0 )  { yDim = &Lats; }
else {

   mlog << Error << "\nSemiLatLonGrid::SemiLatLonGrid(const SemiLatLonData & data) -> "
        << "the number of lats (" << Lats.n() << ") and lons (" << Lons.n()
        << ") cannot both be zero.\n\n";
   exit ( 1 );

}

  // Process the other dimensions

add_dimension(data.levels, Levels);
add_dimension(data.times,  Times);

if ( !xDim || !yDim )  {

   mlog << Error << "\nSemiLatLonGrid::SemiLatLonGrid(const SemiLatLonData & data) -> "
        << "the number of levels (" << Levels.n() << ") and times (" << Times.n()
        << ") cannot both be zero.\n\n";
   exit ( 1 );

}

Nx = xDim->n();
Ny = yDim->n();

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

xDim = (NumArray *) nullptr;
yDim = (NumArray *) nullptr;

IsLatLon = false;
Nx = 0;
Ny = 0;

Data.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SemiLatLonGrid::latlon_to_xy(double xdim_val, double ydim_val, double &x, double &y) const

{

if ( !xDim || !yDim )  {
   mlog << Error << "\nSemiLatLonGrid::latlon_to_xy() -> "
        << "dimensions not defined!\n\n";
   exit ( 1 );
}

if ( IsLatLon )  {
   mlog << Error << "\nSemiLatLonGrid::latlon_to_xy() -> "
        << "not supported when IsLatLon is true.\n\n";
   exit ( 1 );
}

int i, x_int, y_int;
x_int = y_int = bad_data_int;

// Search the X dimension
for(i=0; i<xDim->n(); i++) {
   if(is_eq(xdim_val, xDim->buf()[i])) {
      x_int = i;
      break;
   }
}

// Search the Y dimension
for(i=0; i<yDim->n(); i++) {
   if(is_eq(ydim_val, yDim->buf()[i])) {
      y_int = i;
      break;
   }
}

if ( is_bad_data(x_int) || is_bad_data(y_int) )  {
   mlog << Error << "\nSemiLatLonGrid::latlon_to_xy() -> "
        << "no match found for (" << xdim_val << ", " << ydim_val << ").\n\n";
   exit ( 1 );
}

x = (double) x_int;
y = (double) y_int;

return;

}


////////////////////////////////////////////////////////////////////////


void SemiLatLonGrid::xy_to_latlon(double x, double y, double & xdim_val, double & ydim_val) const

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

xdim_val = xDim->buf()[x_int];
ydim_val = yDim->buf()[y_int];

return;

}


////////////////////////////////////////////////////////////////////////


double SemiLatLonGrid::calc_area(int x, int y) const

{

   // Grid cell area is not defined for semilatlon grids

return bad_data_double;

}


////////////////////////////////////////////////////////////////////////


int SemiLatLonGrid::nx() const

{

return Nx;

}


////////////////////////////////////////////////////////////////////////


int SemiLatLonGrid::ny() const

{

return Ny;

}


////////////////////////////////////////////////////////////////////////


ConcatString SemiLatLonGrid::name() const

{

return Name;

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

out << prefix << "IsLatLon = " << bool_to_string(IsLatLon) << "\n";
out << prefix << "Nx       = " << Nx << "\n";
out << prefix << "Ny       = " << Ny << "\n";

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

a << "Lats: " << Lats.summarize() << sep;
a << "Lons: " << Lons.summarize() << sep;
a << "Levels: " << Levels.summarize() << sep;
a << "Times: " << Times.summarize() << sep;

   //
   //  done
   //

return a;

}


////////////////////////////////////////////////////////////////////////


GridInfo SemiLatLonGrid::info() const

{

GridInfo i;

i.set(Data);

return i;

}


////////////////////////////////////////////////////////////////////////


double SemiLatLonGrid::rot_grid_to_earth(int x, int y) const

{

//
// The rotation angle from grid relative to earth relative
// does not apply to semilatlon grids
//

return 0.0;

}


////////////////////////////////////////////////////////////////////////


bool SemiLatLonGrid::wrap_lon() const

{

return false;

}


////////////////////////////////////////////////////////////////////////


void SemiLatLonGrid::shift_right(int N)

{

mlog << Error << "\nSemiLatLonGrid::shift_right(int) -> "
     << "shifting is not allowed for non-global grids\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


GridRep * SemiLatLonGrid::copy() const

{

SemiLatLonGrid * p = new SemiLatLonGrid (Data);

p->Name = Name;

return p;

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
