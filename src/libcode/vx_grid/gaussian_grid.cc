

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
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
#include "gaussian_grid.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class GaussianGrid
   //


////////////////////////////////////////////////////////////////////////


GaussianGrid::GaussianGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


GaussianGrid::~GaussianGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void GaussianGrid::clear()

{

Name.clear();

Nx = Ny = 0;

Lon_Zero = 0.0;

memset(&Data, 0, sizeof(Data));

return;

}


////////////////////////////////////////////////////////////////////////


GaussianGrid::GaussianGrid(const GaussianData & data)

{

clear();

Nx = data.Nx;

Ny = data.Ny;

Lon_Zero = data.Lon_Zero;

Name = data.name;

Data = data;

}


////////////////////////////////////////////////////////////////////////


void GaussianGrid::latlon_to_xy(double lat, double lon, double &x, double &y) const

{

mlog << Error << "\n\n  GaussianGrid::latlon_to_xy() -> not yet implemented\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void GaussianGrid::xy_to_latlon(double x, double y, double &lat, double &lon) const

{

mlog << Error << "\n\n  GaussianGrid::xy_to_latlon() -> not yet implemented\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


double GaussianGrid::calc_area(int x, int y) const

{

mlog << Error << "\n\n  GaussianGrid::calc_area() -> not yet implemented\n\n";

exit ( 1 );

double area = 0.0;

return ( area );

}


////////////////////////////////////////////////////////////////////////


int GaussianGrid::nx() const

{

return ( Nx );

}


////////////////////////////////////////////////////////////////////////


int GaussianGrid::ny() const

{

return ( Ny );

}


////////////////////////////////////////////////////////////////////////


ConcatString GaussianGrid::name() const

{

return ( Name );

}


////////////////////////////////////////////////////////////////////////


void GaussianGrid::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Name         = ";

if ( Name.length() > 0 )  out << '\"' << Name << '\"';
else                      out << "(nul)\n";

out << '\n';

out << prefix << "Lon_Zero     = " << Lon_Zero << "\n";

out << prefix << "Nx           = " << Nx << "\n";
out << prefix << "Ny           = " << Ny << "\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString GaussianGrid::serialize() const

{

mlog << Error << "\n\n  GaussianGrid::serialize() -> not yet implemented\n\n";

exit ( 1 );

   //
   //  done
   //

ConcatString a;

return ( a );

}


////////////////////////////////////////////////////////////////////////


GridInfo GaussianGrid::info() const

{

GridInfo i;

i.set( Data );

return ( i );

}


////////////////////////////////////////////////////////////////////////


double GaussianGrid::rot_grid_to_earth(int x, int y) const

{

   //
   //  The rotation angle from grid relative to earth relative is zero
   //  for the Gaussian Grids that are defined north and east.  This may
   //  need to be changed when support is added for GRIB2.
   //

return ( 0.0 );

}


////////////////////////////////////////////////////////////////////////


bool GaussianGrid::is_global() const

{

return ( true );

}


////////////////////////////////////////////////////////////////////////


void GaussianGrid::shift_right(int N)

{

Lon_Zero -= N*(delta_lon());

return;

}


////////////////////////////////////////////////////////////////////////


GridRep * GaussianGrid::copy() const

{

GaussianGrid * p = new GaussianGrid (Data);

return ( p );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


Grid::Grid(const GaussianData & data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const GaussianData & data)

{

clear();

rep = new GaussianGrid ( data );

if ( !rep )  {

   mlog << Error << "\nGrid::set(const GaussianData &) -> memory allocation error\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


