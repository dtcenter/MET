

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

Latitudes = 0;

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

if ( Latitudes )  { delete [] Latitudes;  Latitudes = 0; }

return;

}


////////////////////////////////////////////////////////////////////////


GaussianGrid::GaussianGrid(const GaussianData & data)

{

Latitudes = 0;

clear();

Lon_Zero = data.lon_zero;


Name = data.name;

Data = data;

Nx = data.nx;

Ny = data.ny;

Delta_Lon = -360.0/(Nx - 1.0);

   //
   //  check that Ny is even
   //

if ( Ny%2 )  {

   mlog << Error << "\nGaussianGrid::GaussianGrid(const GaussianData &) -> "
        << "Ny must be even!\n\n";

   exit ( 1 );

}

   //
   //  get the latitudes
   //


Legendre L;
int i, j, k;
double r, w;
double latitude;
const int ny_half = Ny/2;


Latitudes = new double [Ny];

L.set_max_degree(Ny);

for (j=0; j<Ny; ++j)  Latitudes[j] = 0.0;

for (j=0; j<ny_half; ++j)  {

   k = j + ny_half;

   k = Ny - 1 - k;

   L.lether_root_weight(k, r, w);
   // L.d_and_r_root_weight(k, r, w);

   latitude = asind(r);

   i = j + ny_half;

   Latitudes[i] = latitude;

   Latitudes[Ny - 1 - i] = -latitude;

}

   //
   //  done
   //

}


////////////////////////////////////////////////////////////////////////


void GaussianGrid::latlon_to_xy(double lat, double lon, double & x, double & y) const

{

int j;
const double lat_top = Latitudes[Ny - 1];


     if ( lat >  lat_top )  y = Ny - 1;
else if ( lat < -lat_top )  y = 0;
else {

   double t;

   for (j=0; j<(Ny - 1); ++j)  {

      if ( (lat >= Latitudes[j]) && (lat <= Latitudes[j + 1]) )  {

         t = (lat - Latitudes[j])/(Latitudes[j + 1] - Latitudes[j]);

         y = j + t;

         break;

      }

   }

}   //  else

lon -= Lon_Zero;

lon -= 360.0*floor((lon + 180.0)/360.0);


x = lon/Delta_Lon;

if ( x < 0.0 )  x += Nx;

// x = nint(x);




   //
   //  done
   //


return;

}


////////////////////////////////////////////////////////////////////////


void GaussianGrid::xy_to_latlon(double x, double y, double & lat, double & lon) const

{

const int ix = nint(x);
const int iy = nint(y);

if ( (fabs(x - ix) >= int_check_tol) || (fabs(y - iy) >= int_check_tol) )  {

   mlog << Error << "\nGaussianGrid::xy_to_latlon() const -> "
        << "x and y must be integers\n";

   exit ( 1 );

}

if ( (iy < 0) || (iy >= Ny) )  {

   mlog << Error << "\nGaussianGrid::xy_to_latlon() const -> "
        << "range check error on y\n\n";

   exit ( 1 );

}

lat = Latitudes[iy];

lon = Lon_Zero + ix*Delta_Lon;

return;

}


////////////////////////////////////////////////////////////////////////


double GaussianGrid::calc_area(int x, int y) const

{

double lat_top, lat_bot;
double area;

if ( y == (Ny - 1) )  lat_top = 90.0;
else                  lat_top = Latitudes[y + 1];

lat_bot = Latitudes[y];

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
    
ConcatString a;
char junk[256];

a << "Projection: Gaussian";

snprintf(junk, sizeof(junk), " Lon_Zero: %.4f", Lon_Zero);   a << junk;

a << " Nx: " << Nx;
a << " Ny: " << Ny;

   //
   //  done
   //

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

p->Name = Name;

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


