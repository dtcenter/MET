

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


static const double int_check_tol = 1.0e-5;


////////////////////////////////////////////////////////////////////////


inline bool is_integer(double x)   { return ( fabs(x - nint(x)) < int_check_tol ); }


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class GaussianGrid
   //


////////////////////////////////////////////////////////////////////////


GaussianGrid::GaussianGrid()

{

North_Latitudes = 0;

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

Delta_Lon = 0.0;

memset(&Data, 0, sizeof(Data));

if ( North_Latitudes )  { delete [] North_Latitudes;  North_Latitudes = 0; }

N_north_lats = 0;

return;

}


////////////////////////////////////////////////////////////////////////


GaussianGrid::GaussianGrid(const GaussianData & data)

{

North_Latitudes = 0;

clear();

Lon_Zero = data.Lon_Zero;

Delta_Lon = 360.0/(Nx - 1.0);

Name = data.name;

Data = data;

Nx = data.Nx;

Ny = data.Ny;

   //
   //  check that Ny is even
   //

if ( Ny%2 )  {

   cerr << "\n\n  GaussianGrid::GaussianGrid(const GaussianData &) -> Ny must be even!\n\n";

   exit ( 1 );

}

   //
   //  get the latitudes
   //


Legendre L;
int j, k;
double r, w;


N_north_lats = Ny/2;

North_Latitudes = new double [N_north_lats];

L.set_max_degree(N_north_lats);


for (j=0; j<N_north_lats; ++j)  {

   k = j + Ny/2;

   k = Ny - 1 - k;

   L.lether_root_weight(k, r, w);
   // L.d_and_r_root_weight(k, r, w);

   North_Latitudes[j] = r;

}

   //
   //  done
   //

}


////////////////////////////////////////////////////////////////////////


double GaussianGrid::y_to_lat(int y) const

{

int index;
double lat;

if ( y < N_north_lats )  {

   index = N_north_lats - 1 - y;

   lat = North_Latitudes[index];

   lat = -lat;

} else {

   index = y - N_north_lats;

   lat = North_Latitudes[index];

}



return ( lat );

}


////////////////////////////////////////////////////////////////////////


void GaussianGrid::latlon_to_xy(double lat, double lon, double & x, double & y) const

{

mlog << Error << "\n\n  GaussianGrid::latlon_to_xy() -> not yet implemented\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void GaussianGrid::xy_to_latlon(double x, double y, double & lat, double & lon) const

{

const int ix = nint(x);
const int iy = nint(y);

if ( (fabs(x - ix) >= int_check_tol) || (fabs(y - iy) >= int_check_tol) )  {

   cerr << "\n\n  GaussianGrid::xy_to_latlon() const -> x and y must be integers\n";

   exit ( 1 );

}

if ( (iy < 0) || (iy >= Ny) )  {

   cerr << "\n\n  GaussianGrid::xy_to_latlon() const -> range check error on y\n\n";

   exit ( 1 );

}

lat = y_to_lat(iy);

lon = Lon_Zero - ix*Delta_Lon;

return;

}


////////////////////////////////////////////////////////////////////////


double GaussianGrid::calc_area(int x, int y) const

{

double lat_top, lat_bot;
double area;

if ( y == (Ny - 1) )  lat_top = 90.0;
else                  lat_top = y_to_lat(y + 1);

lat_bot = y_to_lat(y);

area = rad_per_deg*fabs(Delta_Lon);   //  Delta_Lon in radians

area *= sind(lat_top) - sind(lat_bot);

area *= earth_radius_km*earth_radius_km;

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


