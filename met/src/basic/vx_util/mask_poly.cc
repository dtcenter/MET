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
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "mask_poly.h"

#include "vx_math.h"
#include "vx_util.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static int is_inside(const NumArray &x, const NumArray &y,
                     const double x_test, const double y_test);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MaskPoly
   //


////////////////////////////////////////////////////////////////////////


MaskPoly::MaskPoly()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


MaskPoly::~MaskPoly()

{

clear();

}


////////////////////////////////////////////////////////////////////////


MaskPoly::MaskPoly(const MaskPoly & m)

{

init_from_scratch();

assign(m);

}


////////////////////////////////////////////////////////////////////////


MaskPoly & MaskPoly::operator=(const MaskPoly & m)

{

if ( this == &m )  return ( * this );

assign(m);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void MaskPoly::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void MaskPoly::clear()

{

Name.clear();
FileName.clear();

Lat.clear();
Lon.clear();

U.clear();
V.clear();

LonShift = 0.0;

Npoints = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void MaskPoly::assign(const MaskPoly & m)

{

clear();

Name     = m.Name;
FileName = m.FileName;

if ( m.Npoints == 0 )  return;

Npoints = m.Npoints;

Lat = m.Lat;
Lon = m.Lon;

U = m.U;
V = m.V;

LonShift = m.LonShift;

return;

}


////////////////////////////////////////////////////////////////////////


void MaskPoly::dump(ostream & out, int depth) const

{

if ( Name.empty() )  return;

int j;
char NS, EW;
char junk[256];

Indent prefix(depth);
Indent p2(depth + 1);


out << prefix << "Name     = \"" << Name     << "\"\n";
out << prefix << "FileName = \"" << FileName << "\"\n";
out << prefix << "Npoints  = "   << Npoints  << "\n";
out << prefix << "LonShift = "   << LonShift << "\n";

for (j=0; j<Npoints; ++j)  {

   if ( Lat[j] >= 0.0 )  NS = 'N';
   else                  NS = 'S';

   if ( Lon[j] >= 0.0 )  EW = 'W';
   else                  EW = 'E';

   snprintf(junk, sizeof(junk), "Point # %2d -> %7.2f %c   %8.2f %c", j, fabs(Lat[j]), NS, fabs(Lon[j]), EW);

   out << p2 << junk << "\n";

}

   //
   //   done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


double MaskPoly::lat(int i) const {

if ( i < 0 || i >= Npoints ) {

   mlog << Error << "\nMaskPoly::lat(int i) const -> "
        << "range check error \"" << i << "\"\n\n";

   exit ( 1 );

}

   //
   //   done
   //

return ( Lat[i] );

}


////////////////////////////////////////////////////////////////////////


double MaskPoly::lon(int i) const {

double adj_lon;

if ( i < 0 || i >= Npoints ) {

   mlog << Error << "\nMaskPoly::lon(int i) const -> "
        << "range check error \"" << i << "\"\n\n";

   exit ( 1 );

}

adj_lon  = Lon[i] - LonShift;
adj_lon -= 360.0*floor((adj_lon + 180.0)/360.0);

   //
   //   done
   //

return ( adj_lon );

}


////////////////////////////////////////////////////////////////////////


void MaskPoly::load(const char * filename)

{

int j, n_read;
char line[512];
double a, b;
ifstream in;


clear();

met_open(in, filename);

if ( !in )  {

   mlog << Error << "\nMaskPoly::load() -> "
        << "can't open mask poly file \""
        << filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  store file name
   //

FileName = filename;

   //
   //  initialize
   //

n_read = 0;
Name   = "";

while ( in.getline(line, sizeof(line)) ) {

   //
   //  increment line counter
   //

   n_read++;

   //
   //  skip blank lines
   //

   if ( check_reg_exp(ws_line_reg_exp, line) ) continue;

   //
   //  get name
   //

   if ( Name.length() == 0 ) {

      Name = line;

      //
      //  check for embedded whitespace
      //

      if ( check_reg_exp(ws_reg_exp, Name.c_str()) == true ) {
         mlog << Error << "\nMaskPoly::load() -> "
              << "masking polyline files consist of a string for the "
              << "name followed by pairs of latitude and longitude values. "
              << "The polyline name cannot contain embedded whitespace!\n\n";
         exit(1);
      }

      continue;

   }

   //
   //  get points
   //

   j = sscanf(line, "%lf%lf", &a, &b);

   if ( j != 2 )  {

      mlog << Error << "\nMaskPoly::load() -> "
           << "read error on line number " << n_read << " of mask poly file \""
           << filename << "\"\n\n";

      exit ( 1 );

   }

   Lat.add ( a );

   //
   //  check that the point isn't too close to the poles
   //

   if ( 90.0 - fabs(Lat[Npoints]) < 1.0 ) {

      mlog << Error << "\nMaskPoly::load() -> "
           << "encountered latitude value too close to a pole, (Lat, Lon) = ("
           << a << ", " << b << ") in mask poly file \"" << filename << "\"\n\n";

      exit ( 1 );

   }

   b = -b;   //  toggle from degrees_east to degrees_west
             //  input longitudes are degrees_east
             //  internal longitudes are degrees_west

   b -= 360.0*floor((b + 180.0)/360.0);

   Lon.add ( b );

   U.add ( Lon[Npoints] );  // U equals Lon for a Lat/Lon projection
   V.add ( Lat[Npoints] );  // V equals Lat for a Lat/Lon projection

   Npoints++;               // Increment NPoints
}

in.close();

   //
   // If the polyline crosses the international date line,
   // shift the longitudes by 180 degrees
   //

if ( fabs(Lon.max() - Lon.min()) > 180.0 ) {

   LonShift = 180.0;

   for ( j=0; j<Npoints; j++ ) {

      b  = Lon[j] + LonShift;
      b -= 360.0*floor((b + 180.0)/360.0);

      Lon.set(j, b);
        U.set(j, b);
   }
}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////
//
//  Input test longitude is degrees_west.
//
////////////////////////////////////////////////////////////////////////


bool MaskPoly::latlon_is_inside(double cur_lat, double cur_lon) const

{

int status;

double adj_lon;

adj_lon  = cur_lon + LonShift;
adj_lon -= 360.0*floor((adj_lon + 180.0)/360.0);

status = is_inside(U, V, adj_lon, cur_lat);

return ( status != 0 );

}


////////////////////////////////////////////////////////////////////////
//
//  Input test longitude is degrees_east.
//
////////////////////////////////////////////////////////////////////////

bool MaskPoly::latlon_is_inside_dege(double cur_lat, double cur_lon) const

{

int status;

   //
   //  toggle from degrees_east to degrees_west
   //

cur_lon = -cur_lon;

status = latlon_is_inside(cur_lat, cur_lon);

return ( status != 0 );

}


////////////////////////////////////////////////////////////////////////


   //
   //  This routine tests whether or not the point
   //  ( x_test, y_test ) is inside the region determined
   //  by the points ( x[i], y[i] ),  0 <= i < n.
   //
   //  If the test point is not inside the region, a zero value is
   //  returned, otherwise the value tells how many times the region
   //  wraps around the test point, and the sign tells the direction
   //  in which the winding occurs: + CCW, - CW.
   //
   //  This routine does not test to see if the point is exactly
   //  on the boundary of the region.
   //
   //  It assumes the vertices are connected by straight lines
   //
   //
   //  Author: Randy Bullock
   //


////////////////////////////////////////////////////////////////////////


int is_inside(const NumArray &x, const NumArray &y, const double x_test, const double y_test)

{

int j, k;
int n = x.n_elements();
double angle, angle0, a, b, c, d;


a = x[0] - x_test;
b = y[0] - y_test;

angle = angle0 = atan2(b, a)/pi;

for (j=0; j<n; ++j)  {

   k = (j + 1)%n;

   c = x[k] - x_test;
   d = y[k] - y_test;

   angle += atan2(a*d - b*c, a*c + b*d)/pi;

   a = c;
   b = d;

}

return ( nint( (angle - angle0)/2 ) );

}


////////////////////////////////////////////////////////////////////////






