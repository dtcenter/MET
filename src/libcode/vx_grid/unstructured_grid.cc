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
#include "unstructured_grid.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class UnstructuredGrid
   //


////////////////////////////////////////////////////////////////////////


UnstructuredGrid::UnstructuredGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


UnstructuredGrid::~UnstructuredGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


UnstructuredGrid::UnstructuredGrid(const UnstructuredData & data)

{

clear();

// JHG work here

}


////////////////////////////////////////////////////////////////////////


void UnstructuredGrid::clear()

{

Lats.clear();
Lons.clear();
Levels.clear();
Times.clear();

Dim1 = (NumArray *) 0;
Dim2 = (NumArray *) 0;

Nx = 0;
Ny = 0;

Name.clear();

memset(&Data, 0, sizeof(Data));

return;

}


////////////////////////////////////////////////////////////////////////


void UnstructuredGrid::latlon_to_xy(double lat, double lon, double & x, double & y) const

{

if ( !Dim1 || !Dim2 )  {
   mlog << Error << "\nUnstructuredGrid::latlon_to_xy() -> "
        << "dimensions not defined!\n\n";
   exit ( 1 );
}

int i, x_int, y_int;
x_int = y_int = bad_data_int;

// Search dimensions separately for matches
if ( Is2Dim ) {

   // Search the first dimension
   for(i=0; i<Dim1->n(); i++) {
      if(is_eq(lat, Dim1->buf()[i])) {
         x_int = i;
         break;
      }
   }

   // Search the second dimension
   for(i=0; i<Dim2->n(); i++) {
      if(is_eq(lat, Dim2->buf()[i])) {
         y_int = i;
         break;
      }
   }
}
else {

   // Search both dimensions
   for(i=0; i<Dim1->n(); i++) {
      if(is_eq(lat, Dim1->buf()[i]) && is_eq(lon, Dim2->buf()[i])) {
         x_int = y_int = i;
         break;
      }
   }
}

if ( is_bad_data(x_int) || is_bad_data(y_int) )  {
   mlog << Error << "\nUnstructuredGrid::latlon_to_xy() -> "
        << "no match found for (" << lat << ", " << lon << ").\n\n";
   exit ( 1 );
}

x = (double) x_int;
y = (double) y_int;

return;

}


////////////////////////////////////////////////////////////////////////


void UnstructuredGrid::xy_to_latlon(double x, double y, double & lat, double & lon) const

{

if ( !Dim1 || !Dim2 )  {
   mlog << Error << "\nUnstructuredGrid::xy_to_latlon() -> "
        << "dimensions not defined!\n\n";
   exit ( 1 );
}

int x_int = nint(x);
int y_int = nint(y);

if ( x_int < 0 || x_int >= Nx ||
     y_int < 0 || y_int >= Ny )  {
   mlog << Error << "\nUnstructuredGrid::xy_to_latlon() -> "
        << "(" << x_int << ", " << y_int << ") outside of the grid dimension ("
        << Nx << ", " << Ny <<")!\n\n";
   exit ( 1 );
}

lat = Dim1->buf()[x_int];
lon = Dim2->buf()[y_int];

return;

}


////////////////////////////////////////////////////////////////////////


double UnstructuredGrid::calc_area(int x, int y) const

{

   // Grid cell area is not defined for unstructured grids

return ( bad_data_double );

}


////////////////////////////////////////////////////////////////////////


int UnstructuredGrid::nx() const

{

return ( Nx );

}


////////////////////////////////////////////////////////////////////////


int UnstructuredGrid::ny() const

{

return ( Ny );

}


////////////////////////////////////////////////////////////////////////


ConcatString UnstructuredGrid::name() const

{

return ( Name );

}


////////////////////////////////////////////////////////////////////////


void UnstructuredGrid::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "Name   = ";

if ( Name.length() > 0 )  out << '\"' << Name << '\"';
else                      out << "(nul)\n";

out << '\n';

out << prefix << "Projection = Unstructured\n";

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


ConcatString UnstructuredGrid::serialize(const char *sep) const

{

ConcatString a;

a << "Projection: Unstructured" << sep;

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


GridInfo UnstructuredGrid::info() const

{

GridInfo i;

i.set(Data);

return ( i );

}


////////////////////////////////////////////////////////////////////////


double UnstructuredGrid::rot_grid_to_earth(int x, int y) const

{

//
// The rotation angle from grid relative to earth relative
// does not apply to unstructured grids
//

return ( 0.0 );

}


////////////////////////////////////////////////////////////////////////


bool UnstructuredGrid::wrap_lon() const

{

return ( false );

}


////////////////////////////////////////////////////////////////////////


void UnstructuredGrid::shift_right(int N)

{

mlog << Error << "\nUnstructuredGrid::shift_right(int) -> "
     << "shifting is not allowed for non-global grids\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


GridRep * UnstructuredGrid::copy() const

{

UnstructuredGrid * p = new UnstructuredGrid (Data);

p->Name = Name;

return ( p );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


Grid::Grid(const UnstructuredData & data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const UnstructuredData & data)

{

clear();

rep = new UnstructuredGrid (data);

if ( !rep )  {

   mlog << Error << "\nGrid::set(const UnstructuredData &) -> "
        << "memory allocation error\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////
