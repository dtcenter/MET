// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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


static int line_count(const char * filename);

static int is_inside(const double * x, const double * y, const int n, const double x_test, const double y_test);


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

Name     = (char *) 0;
FileName = (char *) 0;

Lat = (double *) 0;
Lon = (double *) 0;

U = (double *) 0;
V = (double *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void MaskPoly::clear()

{

if ( Name )      { delete [] Name;      Name     = (char *) 0; }
if ( FileName )  { delete [] FileName;  FileName = (char *) 0; }

if ( Lat )  { delete [] Lat;  Lat = (double *) 0; }

if ( Lon )  { delete [] Lon;  Lon = (double *) 0; }

if ( U )  { delete [] U;  U = (double *) 0; }
if ( V )  { delete [] V;  V = (double *) 0; }

Npoints = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void MaskPoly::assign(const MaskPoly & m)

{

clear();

if ( m.Npoints == 0 )  return;

Npoints = m.Npoints;

Lat = new double [Npoints];
Lon = new double [Npoints];

U = new double [Npoints];
V = new double [Npoints];

memcpy(Lat, m.Lat, Npoints*sizeof(double));

memcpy(Lon, m.Lon, Npoints*sizeof(double));

memcpy(U, m.U, Npoints*sizeof(double));

memcpy(V, m.V, Npoints*sizeof(double));

if ( m.Name )  {

   int n = strlen(m.Name);

   Name = new char [1 + n];

   memset(Name, 0, 1 + n);

   strcpy(Name, m.Name);

}

if ( m.FileName )  {

   int n = strlen(m.FileName);

   FileName = new char [1 + n];

   memset(FileName, 0, 1 + n);

   strcpy(FileName, m.FileName);

}

return;

}


////////////////////////////////////////////////////////////////////////


void MaskPoly::dump(ostream & out, int depth) const

{

if ( !Name )  return;

int j;
char NS, EW;
char junk[256];

Indent prefix(depth);
Indent p2(depth + 1);


out << prefix << "Name     = \"" << Name     << "\"\n";
out << prefix << "FileName = \"" << FileName << "\"\n";
out << prefix << "Npoints  = "   << Npoints  << "\n";

for (j=0; j<Npoints; ++j)  {

   if ( Lat[j] >= 0.0 )  NS = 'N';
   else                  NS = 'S';

   if ( Lon[j] >= 0.0 )  EW = 'W';
   else                  EW = 'E';

   sprintf(junk, "Point # %2d -> %7.2f %c   %8.2f %c", j, fabs(Lat[j]), NS, fabs(Lon[j]), EW);

   out << p2 << junk << "\n";

}

   //
   //   done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void MaskPoly::load(const char * filename)

{

int j, k, n;
char line[512];
double a, b;
ifstream in;


clear();

Npoints = line_count(filename);

--Npoints;   //  first line in file is polyline name

if ( Npoints < 0 )  {

   mlog << Error << "\nMaskPoly::load() -> "
        << "can't determine line count in file \""
        << filename << "\"\n\n";

   exit ( 1 );

}

Lat = new double [Npoints];
Lon = new double [Npoints];

U   = new double [Npoints];
V   = new double [Npoints];

memset(Lat, 0, Npoints*sizeof(double));
memset(Lon, 0, Npoints*sizeof(double));

memset(U, 0, Npoints*sizeof(double));
memset(V, 0, Npoints*sizeof(double));

in.open(filename);

if ( !in )  {

   mlog << Error << "\nMaskPoly::load() -> "
        << "can't open mask poly file \""
        << filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  store file name
   //

FileName = new char [ strlen(filename) ];

strcpy(FileName, filename);

   //
   //  get name
   //

in.getline(line, sizeof(line));

n = strlen(line);

Name = new char [1 + n];

memset(Name, 0, 1 + n);

strcpy(Name, line);

   //
   //  get points
   //

for (j=0; j<Npoints; ++j)  {

   in.getline(line, sizeof(line));

   if ( !in )  {

      mlog << Error << "\nMaskPoly::load() -> "
           << "read error in mask poly file \""
           << filename << "\"\n\n";

      exit ( 1 );

   }

   k = sscanf(line, "%lf%lf", &a, &b);

   if ( k != 2 )  {

      mlog << Error << "\nMaskPoly::load() -> "
           << "read error in mask poly file \""
           << filename << "\"\n\n";

      exit ( 1 );

   }

   Lat[j] = a;

   //
   //  check that the point isn't too close to the poles
   //

   if ( 90.0 - abs(Lat[j]) < 1.0 ) {

      mlog << Error << "\nMaskPoly::load() -> "
           << "encountered latitude value too close to a pole, (Lat, Lon) = ("
           << a << ", " << b << ") in mask poly file \"" << filename << "\"\n\n";

      exit ( 1 );

   }

   b = -b;   //  toggle from degrees_west to degrees_east

   b -= 360.0*floor((b + 180.0)/360.0);

   Lon[j] = b;

   U[j]   = Lon[j];  // U equals Lon for a Lat/Lon projection
   V[j]   = Lat[j];  // V equals Lat for a Lat/Lon projection

}

in.close();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool MaskPoly::latlon_is_inside(double lat, double lon) const

{

int status;

   //
   //  toggle from degrees_west to degrees_east
   //

lon = -lon;

status = latlon_is_inside_dege(lat, lon);

return ( status != 0 );

}


////////////////////////////////////////////////////////////////////////


bool MaskPoly::latlon_is_inside_dege(double lat, double lon) const

{

int status;

status = is_inside(U, V, Npoints, lon, lat);

return ( status != 0 );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


int line_count(const char * filename)

{

int count;
char c;
ifstream in;


in.open(filename);

if ( !in )  return ( -1 );

count = 0;

while ( in.get(c) )  {

   if ( c == '\n' )  ++count;

}

in.close();


return ( count );

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


int is_inside(const double * x, const double * y, const int n, const double x_test, const double y_test)

{

int j, k;
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






