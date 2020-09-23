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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "vx_util.h"
#include "vx_math.h"
#include "nav.h"

#include "3d_att.h"
#include "3d_txt_header.h"
#include "3d_single_columns.h"


////////////////////////////////////////////////////////////////////////


static const char format_int        [] = "%d";
static const char format_int_0      [] = "%03d";

static const char format_2_decimals [] = "%.2f";
static const char format_3_decimals [] = "%.3f";


////////////////////////////////////////////////////////////////////////


static double calc_2d_dist(const double x1_grid, const double y1_grid, 
                           const double x2_grid, const double y2_grid, const Grid &);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SingleAtt3D
   //


////////////////////////////////////////////////////////////////////////


SingleAtt3D::SingleAtt3D()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


SingleAtt3D::~SingleAtt3D()

{

clear();

}


////////////////////////////////////////////////////////////////////////


SingleAtt3D::SingleAtt3D(const SingleAtt3D & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


SingleAtt3D & SingleAtt3D::operator=(const SingleAtt3D & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::clear()

{

ObjectNumber = 0;

ClusterNumber = 0;

Volume = 0;

Xbar = Ybar = Tbar = 0;

Xmin = Xmax = 0;
Ymin = Ymax = 0;
Tmin = Tmax = 0;

IsFcst   = true;
IsSimple = true;

Centroid_Lat = 0.0;
Centroid_Lon = 0.0;

Complexity = 0.0;

Xvelocity = Yvelocity = 0.0;

SpatialAxisAngle = 0.0;

CdistTravelled = 0.0;

Ptile_10 = 0.0;
Ptile_25 = 0.0;
Ptile_50 = 0.0;
Ptile_75 = 0.0;
Ptile_90 = 0.0;

Ptile_Value = 0;
Ptile_User  = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::assign(const SingleAtt3D & a)

{

clear();

ObjectNumber = a.ObjectNumber;

ClusterNumber = a.ClusterNumber;

Volume = a.Volume;

IsFcst = a.IsFcst;

IsSimple = a.IsSimple;

Xbar = a.Xbar;
Ybar = a.Ybar;
Tbar = a.Tbar;

Xmin = a.Xmin;
Xmax = a.Xmax;

Ymin = a.Ymin;
Ymax = a.Ymax;

Tmin = a.Tmin;
Tmax = a.Tmax;

Centroid_Lat = a.Centroid_Lat;
Centroid_Lon = a.Centroid_Lon;

Complexity = a.Complexity;

Xvelocity = a.Xvelocity;
Yvelocity = a.Yvelocity;

SpatialAxisAngle = a.SpatialAxisAngle;

CdistTravelled = a.CdistTravelled;

Ptile_10 = a.Ptile_10;
Ptile_25 = a.Ptile_25;
Ptile_50 = a.Ptile_50;
Ptile_75 = a.Ptile_75;
Ptile_90 = a.Ptile_90;

Ptile_Value = a.Ptile_Value;
Ptile_User  = a.Ptile_User;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::centroid (double & _xbar, double & _ybar, double & _tbar) const

{

_xbar = Xbar;
_ybar = Ybar;
_tbar = Tbar;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::bounding_box (int & _xmin, int & _xmax,
                                int & _ymin, int & _ymax,
                                int & _tmin, int & _tmax) const

{

_xmin = Xmin;
_xmax = Xmax;

_ymin = Ymin;
_ymax = Ymax;

_tmin = Tmin;
_tmax = Tmax;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::dump(ostream & out, int depth) const

{

Indent prefix;

out << prefix << "ObjectNumber     = " << ObjectNumber  << "\n";
out << prefix << "ClusterNumber    = " << ClusterNumber << "\n";
out << prefix << "Volume           = " << Volume        << "\n";
out << prefix << "IsFcst           = " << bool_to_string(IsFcst) << "\n";
out << prefix << "IsSimple         = " << bool_to_string(IsSimple) << "\n";
out << prefix << "Centroid         = " << '(' << Xbar << ", " << Ybar << ", " << Tbar << ")\n";
out << prefix << "Bbox             = " << '(' << Xmin << ", " << Ymax << ")   "
                                       << '(' << Ymin << ", " << Ymax << ")   "
                                       << '(' << Tmin << ", " << Tmax << ")\n";

out << prefix << "Complexity       = " << Complexity << "\n";

out << prefix << "Velocity         = " << '(' << Xvelocity << ", " << Yvelocity << ")\n";

out << prefix << "SpatialAxisAngle = " << SpatialAxisAngle << "\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::set_cdist_travelled(double dist)

{

CdistTravelled = dist;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::set_centroid(double _xbar, double _ybar, double _tbar)

{

Xbar = _xbar;
Ybar = _ybar;
Tbar = _tbar;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::set_centroid_latlon(double _lat, double _lon)

{

Centroid_Lat = _lat;
Centroid_Lon = _lon;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::set_bounding_box (int _xmin, int _xmax,
                                    int _ymin, int _ymax,
                                    int _tmin, int _tmax)

{

Xmin = _xmin;
Xmax = _xmax;

Ymin = _ymin;
Ymax = _ymax;

Tmin = _tmin;
Tmax = _tmax;


return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::set_velocity(double vx, double vy)

{

Xvelocity = vx;

Yvelocity = vy;


return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::set_spatial_axis(double angle)

{

   //
   //  reduce angle to range (-90, 90]
   //

angle += 180.0*floor((90.0 - angle)/180.0);

SpatialAxisAngle = angle;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::velocity(double & vx, double & vy) const

{

vx = Xvelocity;

vy = Yvelocity;


return;

}


////////////////////////////////////////////////////////////////////////


double SingleAtt3D::speed() const

{

double s;

s = sqrt( Xvelocity*Xvelocity + Yvelocity*Yvelocity );

return ( s );

}


////////////////////////////////////////////////////////////////////////


int SingleAtt3D::n_times() const

{

int n;

n = Tmax - Tmin + 1;

return ( n );

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::set_fcst(bool tf)

{

IsFcst = tf;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::set_obs(bool tf)

{

IsFcst = !tf;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::set_simple(bool tf)

{

IsSimple = tf;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::set_cluster(bool tf)

{

IsSimple = !tf;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::write_txt(AsciiTable & table, const int row) const

{

char junk[512];
int c = n_header_3d_cols;
int k;
const char * format = 0;
ConcatString s;


   //
   //  object number
   //

if ( is_cluster() )  s << 'C';

if ( is_fcst() )  s << 'F';
else              s << 'O';

snprintf(junk, sizeof(junk), format_int_0, ObjectNumber);

s << junk;

table.set_entry(row, c++, s.text());

   //
   //  cluster number
   //

s.erase();

s << 'C';

if ( is_fcst() )  s << 'F';
else              s << 'O';

k = max<int>(ClusterNumber, 0);

snprintf(junk, sizeof(junk), format_int_0, k);

s << junk;

table.set_entry(row, c++, s.text());

   //
   //  centroid x, y, t
   //

   format = format_2_decimals;

snprintf(junk, sizeof(junk), format, Xbar);

   table.set_entry(row, c++, junk);

snprintf(junk, sizeof(junk), format, Ybar);

   table.set_entry(row, c++, junk);

snprintf(junk, sizeof(junk), format, Tbar);

   table.set_entry(row, c++, junk);

   //
   //  space centroid lat, lon
   //

   format = format_2_decimals;

snprintf(junk, sizeof(junk), format, Centroid_Lat);

   table.set_entry(row, c++, junk);

snprintf(junk, sizeof(junk), format, -Centroid_Lon);

   table.set_entry(row, c++, junk);

   //
   //  velocity xdot, ydot
   //

   format = format_2_decimals;

snprintf(junk, sizeof(junk), format, Xvelocity);

   table.set_entry(row, c++, junk);

snprintf(junk, sizeof(junk), format, Yvelocity);

   table.set_entry(row, c++, junk);

   //
   //  spatial axis angle
   //

   format = format_2_decimals;

snprintf(junk, sizeof(junk), format, SpatialAxisAngle);

   table.set_entry(row, c++, junk);

   //
   //  volume
   //

snprintf(junk, sizeof(junk), format_int, Volume);

   table.set_entry(row, c++, junk);

   //
   //  start time, end time
   //

   format = format_int;

snprintf(junk, sizeof(junk), format, Tmin);

   table.set_entry(row, c++, junk);

snprintf(junk, sizeof(junk), format, Tmax);

   table.set_entry(row, c++, junk);

   //
   //  centroid distance travelled
   //

   format = format_3_decimals;

snprintf(junk, sizeof(junk), format, CdistTravelled);

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
   //  custom intensity value
   //

snprintf(junk, sizeof(junk), format, Ptile_User);

   table.set_entry(row, c++, junk);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PairAtt3D
   //


////////////////////////////////////////////////////////////////////////


PairAtt3D::PairAtt3D()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


PairAtt3D::~PairAtt3D()

{

clear();

}


////////////////////////////////////////////////////////////////////////


PairAtt3D::PairAtt3D(const PairAtt3D & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


PairAtt3D & PairAtt3D::operator=(const PairAtt3D & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::clear()

{

IntersectionVol = 0;

IsSimple = true;

// UnionVol = 0;

TimeCentroidDelta = 0.0;

SpaceCentroidDist = 0.0;

DirectionDifference = 0.0;

SpeedDelta = 0.0;

FcstObjectNumber = -1;
ObsObjectNumber  = -1;

FcstClusterNumber = -1;
ObsClusterNumber  = -1;

VolumeRatio = 0.0;
AxisDiff    = 0.0;

StartTimeDelta = 0;
EndTimeDelta   = 0;

TotalInterest = -1.0;

DurationDifference = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::assign(const PairAtt3D & a)

{

clear();

IsSimple = a.IsSimple;

FcstObjectNumber = a.FcstObjectNumber;
ObsObjectNumber  = a.ObsObjectNumber;

FcstClusterNumber = a.FcstClusterNumber;
ObsClusterNumber  = a.ObsClusterNumber;

IntersectionVol = a.IntersectionVol;

// UnionVol = a.UnionVol;


TimeCentroidDelta = a.TimeCentroidDelta;

SpaceCentroidDist = a.SpaceCentroidDist;

DirectionDifference = a.DirectionDifference;

SpeedDelta = a.SpeedDelta;

VolumeRatio = a.VolumeRatio;
AxisDiff    = a.AxisDiff;

StartTimeDelta = a.StartTimeDelta;
EndTimeDelta   = a.EndTimeDelta;

TotalInterest = a.TotalInterest;

DurationDifference = a.DurationDifference;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::set_duration_difference(int k)

{

DurationDifference = k;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::set_total_interest(double t)

{

TotalInterest = t;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::set_fcst_obj_number(int k)

{

FcstObjectNumber = k;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::set_obs_obj_number(int k)

{

ObsObjectNumber = k;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::set_fcst_cluster_number(int k)

{

FcstClusterNumber = k;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::set_obs_cluster_number(int k)

{

ObsClusterNumber = k;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::set_intersection_volume(int k)

{

IntersectionVol = k;

return;

}


////////////////////////////////////////////////////////////////////////

/*
void PairAtt3D::set_union_volume(int k)

{

UnionVol = k;

return;

}
*/

////////////////////////////////////////////////////////////////////////


void PairAtt3D::set_space_centroid_dist(double d)

{

SpaceCentroidDist = d;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::set_time_centroid_delta(double d)

{

TimeCentroidDelta = d;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::set_speed_delta(double d)

{

SpeedDelta = d;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::set_direction_diff(double d)

{

DirectionDifference = d;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::set_volume_ratio(double d)

{

VolumeRatio = d;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::set_axis_diff(double d)

{

AxisDiff = d;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::set_start_time_delta(int d)

{

StartTimeDelta = d;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::set_end_time_delta(int d)

{

EndTimeDelta = d;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::write_txt(AsciiTable & table, const int row) const

{

int k;
char junk[512];
int c = n_header_3d_cols;


   //  object number

if ( is_simple() )  snprintf(junk, sizeof(junk), "F%03d_O%03d",   FcstObjectNumber, ObsObjectNumber);
else                snprintf(junk, sizeof(junk), "CF%03d_CO%03d", FcstObjectNumber, ObsObjectNumber);

table.set_entry(row, c++, junk);

   //
   //  cluster number
   //

if ( FcstClusterNumber == ObsClusterNumber )  k = ObsClusterNumber;
else                                          k = 0;

snprintf(junk, sizeof(junk), "CF%03d_CO%03d", k, k);

table.set_entry(row, c++, junk);

   ////////////////////////////

   //  space centroid distance

snprintf(junk, sizeof(junk), format_2_decimals, SpaceCentroidDist);

table.set_entry(row, c++, junk);

   //  time centroid delta

snprintf(junk, sizeof(junk), format_2_decimals, TimeCentroidDelta);

table.set_entry(row, c++, junk);

   //  axis diff

snprintf(junk, sizeof(junk), format_3_decimals, AxisDiff);

table.set_entry(row, c++, junk);

   //  speed delta

snprintf(junk, sizeof(junk), format_2_decimals, SpeedDelta);

table.set_entry(row, c++, junk);

   //  direction diff

snprintf(junk, sizeof(junk), format_2_decimals, DirectionDifference);

table.set_entry(row, c++, junk);

   //  volume ratio

snprintf(junk, sizeof(junk), format_2_decimals, VolumeRatio);

table.set_entry(row, c++, junk);

   //  start time delta

k = nint(StartTimeDelta);

snprintf(junk, sizeof(junk), format_int, k);

table.set_entry(row, c++, junk);

   //  end time delta

k = nint(EndTimeDelta);

snprintf(junk, sizeof(junk), format_int, k);

table.set_entry(row, c++, junk);

   //  intersection volume

snprintf(junk, sizeof(junk), format_int, IntersectionVol);

table.set_entry(row, c++, junk);

   //  duration difference

snprintf(junk, sizeof(junk), format_int, DurationDifference);

table.set_entry(row, c++, junk);

   //  total interest

snprintf(junk, sizeof(junk), format_3_decimals, TotalInterest);

table.set_entry(row, c++, junk);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::dump(ostream & out, int depth) const

{

Indent indent(depth);

out << indent << "FcstObjectNumber    = " << FcstObjectNumber    << '\n';
out << indent << " ObsObjectNumber    = " <<  ObsObjectNumber    << '\n';
out << indent << "IntersectionVol     = " << IntersectionVol     << '\n';
// out << indent << "UnionVol            = " << UnionVol            << '\n';
out << indent << "TimeCentroidDelta   = " << TimeCentroidDelta   << '\n';
out << indent << "SpaceCentroidDist   = " << SpaceCentroidDist   << '\n';
out << indent << "DirectionDifference = " << DirectionDifference << '\n';
out << indent << "SpeedDelta          = " << SpeedDelta          << '\n';
out << indent << "VolumeRatio         = " << VolumeRatio         << '\n';
out << indent << "AxisDiff            = " << AxisDiff            << '\n';
out << indent << "StartTimeDelta      = " << StartTimeDelta      << '\n';
out << indent << "EndTimeDelta        = " << EndTimeDelta        << '\n';
out << indent << "TotalInterest       = " << TotalInterest       << '\n';


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


SingleAtt3D calc_3d_single_atts(const Object & mask, const MtdFloatFile & raw, const char * model, const int ptile_value)

{

int j, t;
int n, nt, Vol;
SingleAtt3D a;
double bbox_volume;
double lat, lon;
double xbar_2d, ybar_2d, x_old, y_old;
double dist;
ConcatString raw_filename;
float * values = (float *) 0;
const int   * i = 0;
const float * r = 0;
Mtd_3D_Moments moments;
const int n3 = (mask.nx())*(mask.ny())*(mask.nt());
const Grid * grid = mask.grid_p();


moments = mask.calc_3d_moments();

if ( moments.N == 0 )  {

   mlog << Error << "\n\n  calc_3d_single_atts() -> empty object!\n\n";

   exit ( 1 );

}

a.Xbar = (moments.Sx)/(moments.N);
a.Ybar = (moments.Sy)/(moments.N);
a.Tbar = (moments.St)/(moments.N);

moments.centralize();


raw.xy_to_latlon(a.Xbar, a.Ybar, lat, lon);

a.Centroid_Lat = lat;
a.Centroid_Lon = lon;

a.Volume = mask.volume(0);

mask.calc_3d_bbox(a.Xmin, a.Xmax, a.Ymin, a.Ymax, a.Tmin, a.Tmax);

bbox_volume =  (a.Xmax - a.Xmin + 1.0)
              *(a.Ymax - a.Ymin + 1.0)
              *(a.Tmax - a.Tmin + 1.0);

a.Complexity = ((double) (a.Volume))/bbox_volume;

   //
   //  velocity, orientation
   //

if ( a.n_times() <= 1 )  {

   a.Xvelocity = a.Yvelocity = 0.0;

   a.set_spatial_axis(0.0);

} else {

   moments.calc_3d_velocity(a.Xvelocity, a.Yvelocity);

   a.set_spatial_axis(moments.calc_3d_axis_plane_angle());

}   //  else


   //
   //  distance travelled by the 2D centroid
   //

dist = 0.0;

nt = a.Tmax - a.Tmin + 1;

for (j=0,x_old=0,y_old=0; j<nt; ++j)  {

   t = j + a.Tmin;

   mask.calc_2d_centroid_at_t(t, xbar_2d, ybar_2d);

   if ( j == 0 )  {

       x_old = xbar_2d;
       y_old = ybar_2d;

   } else {
 
      dist += calc_2d_dist(xbar_2d, ybar_2d, x_old, y_old, *grid);

   }   //  else

}

a.set_cdist_travelled(dist);

   //
   //  percentiles
   //

Vol = a.Volume;

values = new float [Vol];

if ( !values )  {

   mlog << Error << "\n\n  calc_3d_single_atts() -> memory allocation error\n\n";

   exit ( 1 );

}

n = 0;

i = mask.data();
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

a.Ptile_Value = ptile_value;

a.Ptile_User = percentile_f(values, n, (double) (a.Ptile_Value/100.0));


   //
   //   done
   //

if ( values )  { delete [] values;  values = 0; }

// a.dump(cout);

return ( a );

}


////////////////////////////////////////////////////////////////////////


PairAtt3D calc_3d_pair_atts(const Object      & fcst_obj,
                            const Object      &  obs_obj,
                            const SingleAtt3D & fcst_att,
                            const SingleAtt3D &  obs_att)

{

int x, y, t;
int IV, UV;
PairAtt3D p;
double dx, dy;
double x1dot, x2dot, y1dot, y2dot;
double b, b1, b2;
double num, den;
bool obs_on  = false;
bool fcst_on = false;
const double tol = 1.0e-3;


p.set_fcst_obj_number (fcst_att.object_number());
p.set_obs_obj_number  (obs_att.object_number());

   //
   //  intersection and union volumes
   //

IV = UV = 0;

for (x=0; x<(fcst_obj.nx()); ++x)  {

   for (y=0; y<(fcst_obj.ny()); ++y)  {

      for (t=0; t<(fcst_obj.nt()); ++t)  {

         obs_on =  obs_obj(x, y, t);
        fcst_on = fcst_obj(x, y, t);

        if ( fcst_on || obs_on ) ++UV;
        if ( fcst_on && obs_on ) ++IV;

      }

   }

}

p.set_intersection_volume (IV);
// p.set_union_volume        (UV);

   //
   //  centroid distances
   //

p.set_time_centroid_delta(obs_att.tbar() - fcst_att.tbar());

dx = fcst_att.xbar() - obs_att.xbar();
dy = fcst_att.ybar() - obs_att.ybar();

p.set_space_centroid_dist( sqrt( dx*dx + dy*dy ) );

fcst_att.velocity(x1dot, y1dot);
 obs_att.velocity(x2dot, y2dot);

   //
   //  speed and direction differences
   //

p.set_speed_delta(fcst_att.speed() - obs_att.speed());

b1 = sqrt( x1dot*x1dot + y1dot*y1dot );
b2 = sqrt( x2dot*x2dot + y2dot*y2dot );

if ( fabs(b1) < tol || fabs(b2) < tol )  p.set_direction_diff( 0.0 );
else {

   x1dot /= b1;
   y1dot /= b1;

   x2dot /= b2;
   y2dot /= b2;

   b = x1dot*x2dot + y1dot*y2dot;

   if ( b > 0.999999 )  p.set_direction_diff( 0.0 );
   else                 p.set_direction_diff( acosd(b) );

}

   //
   //  volume ratio
   //

num = (double) (fcst_att.volume());
den = (double) (obs_att.volume());

p.set_volume_ratio(num/den);

   //
   //  axis difference
   //

b = fabs( fcst_att.spatial_axis() - obs_att.spatial_axis() );

if ( b > 90.0 )  b -= 90.0;   //  we want the smaller of the two angles that the intersecting lines (planes) make

p.set_axis_diff(b);

   //
   //  start time delta
   //

t = fcst_att.tmin() - obs_att.tmin();

p.set_start_time_delta(t);

   //
   //  end time delta
   //

t = fcst_att.tmax() - obs_att.tmax();

p.set_end_time_delta(t);

   //
   //  duration difference
   //

t = fcst_att.n_times() - obs_att.n_times();

p.set_duration_difference(t);

   //
   //  done
   //

return ( p );

}


////////////////////////////////////////////////////////////////////////

/*
double calc_total_interest(const PairAtt3D & p, const MtdConfigInfo & conf)

{

double t = 0.0;
double num, den;   //  numerator and denominator in the expression for total interest
double I, w;
PiecewiseLinear * f = 0;

num = 0.0;
den = 0.0;

   //
   //  We don't need to use "is_eq" to check whether each weight is
   //     nonzero, because the MtdConfigInfo::read_config() function has
   //     already done that.  That same function has already tested that
   //     the weights are not all zero.
   //

   //
   //  space centroid dist
   //

w = conf.space_centroid_dist_wt;

if ( w != 0.0 )  {

   f = conf.space_centroid_dist_if;

   I = (*f)(p.space_centroid_dist());

   num += w*I;

   den += w;

}

   //
   //  time centroid delta
   //

w = conf.time_centroid_delta_wt;

if ( w != 0.0 )  {

   f = conf.time_centroid_delta_if;

   I = (*f)(p.time_centroid_delta());

   num += w*I;

   den += w;

}

   //
   //  speed delta
   //

w = conf.speed_delta_wt;

if ( w != 0.0 )  {

   f = conf.speed_delta_if;

   I = (*f)(p.speed_delta());

   num += w*I;

   den += w;

}

   //
   //  direction difference
   //

w = conf.direction_diff_wt;

if ( w != 0.0 )  {

   f = conf.direction_diff_if;

   I = (*f)(p.direction_difference());

   num += w*I;

   den += w;

}

   //
   //  volume ratio
   //

w = conf.volume_ratio_wt;

if ( w != 0.0 )  {

   f = conf.volume_ratio_if;

   I = (*f)(p.volume_ratio());

   num += w*I;

   den += w;

}

   //
   //  axis angle difference
   //

w = conf.axis_angle_diff_wt;

if ( w != 0.0 )  {

   f = conf.axis_angle_diff_if;

   I = (*f)(p.axis_angle_diff());

   num += w*I;

   den += w;

}

   //
   //  start time delta
   //

w = conf.start_time_delta_wt;

if ( w != 0.0 )  {

   f = conf.start_time_delta_if;

   I = (*f)(p.start_time_delta());

   num += w*I;

   den += w;

}

   //
   //  end time delta
   //

w = conf.end_time_delta_wt;

if ( w != 0.0 )  {

   f = conf.end_time_delta_if;

   I = (*f)(p.end_time_delta());

   num += w*I;

   den += w;

}

   //
   //  The denominator is just the sum of the weights,
   //    which, as stated above, we already know is nonzero.
   //

t = num/den;

   //
   //  done
   //

return ( t );

}
*/

////////////////////////////////////////////////////////////////////////


double calc_2d_dist(const double x1_grid, const double y1_grid, 
                    const double x2_grid, const double y2_grid, const Grid & grid)

{

double dist;

   //
   //  distance in grid units
   //

// const double dx = x1_grid - x2_grid;
// const double dy = y1_grid - y2_grid;
// 
// dist = sqrt ( dx*dx + dy*dy );

   //
   //  great circle distance
   //

double lat1, lon1, lat2, lon2;

grid.xy_to_latlon(x1_grid, y1_grid, lat1, lon1);
grid.xy_to_latlon(x2_grid, y2_grid, lat2, lon2);

dist = gc_dist(lat1, lon1, lat2, lon2);


   //
   //  done
   //

return ( dist );

}


////////////////////////////////////////////////////////////////////////







