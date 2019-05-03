// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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

static const char format_int        [] = "%d";


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

TimeIndex = -1;

IsFcst = true;

ValidTime = (unixtime) 0;

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

TimeIndex = a.TimeIndex;

CentroidLat = a.CentroidLat;
CentroidLon = a.CentroidLon;

IsFcst = a.IsFcst;

ValidTime = a.ValidTime;

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
out << prefix << "ValidTime     = " << ValidTime     << "\n";

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


void SingleAtt2D::set_valid_time(const unixtime t)

{

ValidTime = t;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


SingleAtt2D calc_2d_single_atts(const MtdIntFile & mask_2d, const int obj_number)   //  zero-based

{

SingleAtt2D a;
Mtd_2D_Moments moments;


a.ObjectNumber = obj_number;

moments = mask_2d.calc_2d_moments();

if ( moments.N == 0 )  {

   mlog << Error << "\n\n  calc_2d_single_atts() -> empty object!\n\n";

   exit ( 1 );

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
/*
Vol = a.Volume;

values = new float [Vol];

if ( !values )  {

   mlog << Error << "\n\n  calc_2d_single_atts() -> memory allocation error\n\n";

   exit ( 1 );

}

n = 0;

i = obj.data();
r = raw.data();

for (j=0; j<n3; ++j)  {

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
*/

   //
   //   done
   //

// if ( values )  { delete [] values;  values = 0; }

// a.dump(cout);

return ( a );

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2D::write_txt(AsciiTable & table, const int row) const

{

int c = n_header_3d_cols;
const char * format = 0;
char junk[512];
ConcatString s;

   //
   //  object number
   //

if ( IsFcst )  s << 'F';
else           s << 'O';

s << '_' << ObjectNumber;

table.set_entry(row, c++, s.text());

   // 
   //   cluster number
   // 

s.erase();

s << 'C';

if ( is_fcst() )  s << 'F';
else              s << 'O';

if ( ClusterNumber >= 1 )  {

   sprintf(junk, format_int, ClusterNumber);

   s << '_' << junk;

}

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
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////





