// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
#include <cstdio>
#include <cmath>

#include "vx_util.h"
#include "vx_math.h"

#include "2d_att.h"
#include "2d_moments.h"
#include "3d_txt_header.h"
#include "2d_columns.h"


////////////////////////////////////////////////////////////////////////


static const char format_2_decimals [] = "%.2f";

static const char format_int        [] = "%03d";


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SingleAtt2D
   //


////////////////////////////////////////////////////////////////////////


SingleAtt2D::SingleAtt2D()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


SingleAtt2D::~SingleAtt2D()

{

clear();

}


////////////////////////////////////////////////////////////////////////


SingleAtt2D::SingleAtt2D(const SingleAtt2D & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


SingleAtt2D & SingleAtt2D::operator=(const SingleAtt2D & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2D::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2D::clear()

{

ObjectNumber = 0;

ClusterNumber = 0;

Area = 0;

Xbar = Ybar = 0.0;

CentroidLat = CentroidLon = 0.0;

AxisAngle = 0.0;

Ptile_10 = 0.0;
Ptile_25 = 0.0;
Ptile_50 = 0.0;
Ptile_75 = 0.0;
Ptile_90 = 0.0;

TimeIndex = -1;

IsFcst = true;

Is_Cluster = false;

ValidTime = (unixtime) 0;

Lead_Time = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2D::assign(const SingleAtt2D & a)

{

clear();

ObjectNumber = a.ObjectNumber;

ClusterNumber = a.ClusterNumber;

Area = a.Area;

Xbar = a.Xbar;
Ybar = a.Ybar;

AxisAngle = a.AxisAngle;

Ptile_10 = a.Ptile_10;
Ptile_25 = a.Ptile_25;
Ptile_50 = a.Ptile_50;
Ptile_75 = a.Ptile_75;
Ptile_90 = a.Ptile_90;

TimeIndex = a.TimeIndex;

CentroidLat = a.CentroidLat;
CentroidLon = a.CentroidLon;

IsFcst = a.IsFcst;

Is_Cluster = a.Is_Cluster;

ValidTime = a.ValidTime;

Lead_Time = a.Lead_Time;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2D::centroid (double & _xbar, double & _ybar) const

{

_xbar = Xbar;
_ybar = Ybar;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2D::dump(ostream & out, int depth) const

{

Indent prefix;

out << prefix << "ObjectNumber  = " << ObjectNumber  << "\n";
out << prefix << "ClusterNumber = " << ClusterNumber << "\n";
out << prefix << "Area          = " << Area << "\n";
out << prefix << "Centroid      = " << '(' << Xbar << ", " << Ybar << ")\n";
out << prefix << "AxisAngle     = " << AxisAngle     << "\n";
out << prefix << "Ptile_10      = " << Ptile_10      << "\n";
out << prefix << "Ptile_25      = " << Ptile_25      << "\n";
out << prefix << "Ptile_50      = " << Ptile_50      << "\n";
out << prefix << "Ptile_75      = " << Ptile_75      << "\n";
out << prefix << "Ptile_90      = " << Ptile_90      << "\n";
out << prefix << "ValidTime     = " << ValidTime     << "\n";
out << prefix << "LeadTime      = " << Lead_Time     << "\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2D::set_centroid(double _xbar, double _ybar)

{

Xbar = _xbar;
Ybar = _ybar;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2D::set_axis(double angle)

{

   //
   //  reduce angle to range (-90, 90]
   //

angle += 180.0*floor((90.0 - angle)/180.0);

AxisAngle = angle;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2D::set_fcst(bool tf)

{

IsFcst = tf;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2D::set_obs(bool tf)

{

IsFcst = !tf;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2D::set_is_cluster(bool tf)

{

Is_Cluster = tf;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2D::set_is_simple(bool tf)

{

Is_Cluster = !tf;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2D::set_valid_time(const unixtime t)

{

ValidTime = t;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2D::set_lead_time(const int t)

{

Lead_Time = t;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


SingleAtt2D calc_2d_single_atts(const MtdIntFile & mask_2d, const DataPlane & raw_2d, const int obj_number)   //  zero-based

{

SingleAtt2D a;
Mtd_2D_Moments moments;
float * values = (float *) 0;
const int    * i = 0;
const double * r = 0;
const int nxy = (mask_2d.nx())*(mask_2d.ny());
int j, n;

a.ObjectNumber = obj_number;

moments = mask_2d.calc_2d_moments();

if ( moments.N == 0 )  {

   // mlog << Error << "\n\n  calc_2d_single_atts() -> empty object!\n\n";

   // exit ( 1 );

   return ( a );

}

a.Xbar = (moments.Sx)/(moments.N);
a.Ybar = (moments.Sy)/(moments.N);

mask_2d.grid().xy_to_latlon(a.Xbar, a.Ybar, a.CentroidLat, a.CentroidLon);

a.Area = mask_2d.volume(0);

moments.centralize();

a.AxisAngle = moments.calc_2D_axis_plane_angle();


   //
   //  percentiles
   //


values = new float [a.Area];

if ( !values )  {

   mlog << Error << "\n\n  calc_2d_single_atts() -> memory allocation error\n\n";

   exit ( 1 );

}

i = mask_2d.data();
r = raw_2d.data();
n = 0;

for (j=0; j<nxy; ++j)  {

   if ( *i )  {

      values[n++] = *r;

   }

   ++i; ++r;

}


sort_f(values, n);

a.Ptile_10 = percentile_f(values, n, 0.10);
a.Ptile_25 = percentile_f(values, n, 0.25);
a.Ptile_50 = percentile_f(values, n, 0.50);
a.Ptile_75 = percentile_f(values, n, 0.75);
a.Ptile_90 = percentile_f(values, n, 0.90);


   //
   //   done
   //

if ( values )  { delete [] values;  values = 0; }

return ( a );

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2D::write_txt(AsciiTable & table, const int row) const

{

int c = n_header_3d_cols;
int k;
const char * format = 0;
char junk[512];
ConcatString s;

   //
   //  object number
   //

s.erase();

if ( is_cluster() )  k = max<int>(ClusterNumber, 0);
else                 k = max<int>(ObjectNumber,  0);

if ( is_cluster() )  s << 'C';

if ( IsFcst )  s << 'F';
else           s << 'O';

snprintf(junk, sizeof(junk), format_int, k);

s << junk;

table.set_entry(row, c++, s.text());

   //
   //   cluster number
   //

s.erase();

k = max<int>(ClusterNumber, 0);

s << 'C';

if ( is_fcst() )  s << 'F';
else              s << 'O';

snprintf(junk, sizeof(junk), format_int, k);

s << junk;

table.set_entry(row, c++, s.text());

   //
   //  time index
   //

table.set_entry(row, c++, TimeIndex);

   //
   //  area
   //

table.set_entry(row, c++, Area);

   //
   //  centroid (x, y)
   //

format = format_2_decimals;

snprintf(junk, sizeof(junk), format, Xbar);

table.set_entry(row, c++, junk);

snprintf(junk, sizeof(junk), format, Ybar);

table.set_entry(row, c++, junk);

   //
   //  centroid lat/lon
   //

format = format_2_decimals;

snprintf(junk, sizeof(junk), format, CentroidLat);

table.set_entry(row, c++, junk);

snprintf(junk, sizeof(junk), format, -CentroidLon);   //  toggle sign

table.set_entry(row, c++, junk);

   //
   //  axis angle
   //

format = format_2_decimals;

snprintf(junk, sizeof(junk), format, AxisAngle);

table.set_entry(row, c++, junk);


   //
   //  intensities 10, 25, 50, 75, 90
   //

   format = format_2_decimals;

snprintf(junk, sizeof(junk), format, Ptile_10);

   table.set_entry(row, c++, junk);

snprintf(junk, sizeof(junk), format, Ptile_25);

   table.set_entry(row, c++, junk);

snprintf(junk, sizeof(junk), format, Ptile_50);

   table.set_entry(row, c++, junk);

snprintf(junk, sizeof(junk), format, Ptile_75);

   table.set_entry(row, c++, junk);

snprintf(junk, sizeof(junk), format, Ptile_90);

   table.set_entry(row, c++, junk);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////





