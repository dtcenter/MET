

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

#include "3d_att.h"


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

Volume = 0;

Xbar = Ybar = Tbar = 0;

Xmin = Xmax = 0;
Ymin = Ymax = 0;
Tmin = Tmax = 0;

Complexity = 0.0;

Xvelocity = Yvelocity = 0.0;

SpatialAxisAngle = 0.0;

Ptile_10 = 0.0;
Ptile_25 = 0.0;
Ptile_50 = 0.0;
Ptile_75 = 0.0;
Ptile_90 = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt3D::assign(const SingleAtt3D & a)

{

clear();

ObjectNumber = a.ObjectNumber;

Volume = a.Volume;

Xbar = a.Xbar;
Ybar = a.Ybar;
Tbar = a.Tbar;

Xmin = a.Xmin;
Xmax = a.Xmax;

Ymin = a.Ymin;
Ymax = a.Ymax;

Tmin = a.Tmin;
Tmax = a.Tmax;

Complexity = a.Complexity;

Xvelocity = a.Xvelocity;
Yvelocity = a.Yvelocity;

SpatialAxisAngle = a.SpatialAxisAngle;

Ptile_10 = a.Ptile_10;
Ptile_25 = a.Ptile_25;
Ptile_50 = a.Ptile_50;
Ptile_75 = a.Ptile_75;
Ptile_90 = a.Ptile_90;


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

out << prefix << "Volume           = " << Volume << "\n";
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


void SingleAtt3D::set_centroid(double _xbar, double _ybar, double _tbar)

{

Xbar = _xbar;
Ybar = _ybar;
Tbar = _tbar;

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

UnionVol = 0;

TimeCentroidDelta = 0.0;

SpaceCentroidDist = 0.0;

DirectionDifference = 0.0;

SpeedDelta = 0.0;

FcstObjectNumber = -1;
ObsObjectNumber  = -1;

VolumeRatio = 0.0;
AxisDiff    = 0.0;

StartTimeDelta = 0;
EndTimeDelta   = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::assign(const PairAtt3D & a)

{

clear();



IntersectionVol = a.IntersectionVol;

UnionVol = a.UnionVol;

TimeCentroidDelta = a.TimeCentroidDelta;

SpaceCentroidDist = a.SpaceCentroidDist;

DirectionDifference = a.DirectionDifference;

SpeedDelta = a.SpeedDelta;

FcstObjectNumber = a.FcstObjectNumber;
ObsObjectNumber = a.ObsObjectNumber;

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


void PairAtt3D::set_intersection_volume(int k)

{

IntersectionVol = k;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAtt3D::set_union_volume(int k)

{

UnionVol = k;

return;

}


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

char junk[512];
int c = 1;

   //  model

   c++;

   //  object number

sprintf(junk, "O_%d_F_%d", ObsObjectNumber, FcstObjectNumber);

table.set_entry(row, c++, junk);

   //  space centroid distance

sprintf(junk, "%.1f", SpaceCentroidDist);

table.set_entry(row, c++, junk);

   //  time centroid delta

sprintf(junk, "%.1f", TimeCentroidDelta);

table.set_entry(row, c++, junk);

   //  speed delta

sprintf(junk, "%.1f", SpeedDelta);

table.set_entry(row, c++, junk);

   //  direction diff

sprintf(junk, "%.1f", DirectionDifference);

table.set_entry(row, c++, junk);

   //  volume ratio

sprintf(junk, "%.2f", VolumeRatio);

table.set_entry(row, c++, junk);

   //  axis diff

sprintf(junk, "%.2f", AxisDiff);

table.set_entry(row, c++, junk);

   //  start time delta

sprintf(junk, "%d", StartTimeDelta);

table.set_entry(row, c++, junk);

   //  end time delta

sprintf(junk, "%d", EndTimeDelta);

table.set_entry(row, c++, junk);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


SingleAtt3D calc_single_atts(const Object & obj, const MtdFloatFile & raw, const char * model, int obj_number)

{

int j;
int n, Vol;
SingleAtt3D a;
double bbox_volume;
ConcatString raw_filename;
float * values = (float *) 0;
const int   * i = 0;
const float * r = 0;
MtdMoments moments;
MtdIntFile f;
const int n3 = (obj.nx())*(obj.ny())*(obj.nt());


a.ObjectNumber = obj_number;

f = obj.select(obj_number + 1);

moments = f.calc_moments();

if ( moments.N == 0 )  {

   cerr << "\n\n  calc_single_atts() -> empty object!\n\n";

   exit ( 1 );

}

a.Xbar = (moments.Sx)/(moments.N);
a.Ybar = (moments.Sy)/(moments.N);
a.Tbar = (moments.St)/(moments.N);

a.Volume = obj.total_volume();

f.calc_bbox(a.Xmin, a.Xmax, a.Ymin, a.Ymax, a.Tmin, a.Tmax);

bbox_volume =  (a.Xmax - a.Xmin - 1.0)
              *(a.Ymax - a.Ymin - 1.0)
              *(a.Tmax - a.Tmin - 1.0);

a.Complexity = ((double) (a.Volume))/bbox_volume;

   //
   //  velocity, orientation
   //

if ( a.n_times() <= 1 )  {

   a.Xvelocity = a.Yvelocity = 0.0;

   a.set_spatial_axis(0.0);

} else {

   moments.calc_velocity(a.Xvelocity, a.Yvelocity);

   a.set_spatial_axis(moments.calc_3D_axis_plane_angle());

}   //  else


   //
   //  percentiles
   //

Vol = a.Volume;

values = new float [Vol];

if ( !values )  {

   cerr << "\n\n  calc_single_atts() -> memory allocation error\n\n";

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


   //
   //   done
   //

if ( values )  { delete [] values;  values = 0; }

// a.dump(cout);

return ( a );

}


////////////////////////////////////////////////////////////////////////


PairAtt3D calc_pair_atts(const Object & fcst_obj,
                         const Object & obs_obj,
                         const SingleAtt3D & fcst_att,
                         const SingleAtt3D & obs_att)

{

int x, y, t;
int IV, UV;
PairAtt3D p;
double dx, dy;
double x1dot, x2dot, y1dot, y2dot;
double b;
double num, den;
bool obs_on  = false;
bool fcst_on = false;


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
p.set_union_volume        (UV);

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

b = sqrt( x1dot*x1dot + y1dot*y1dot );

x1dot /= b;
y1dot /= b;

b = sqrt( x2dot*x2dot + y2dot*y2dot );

x2dot /= b;
y2dot /= b;

p.set_direction_diff( acosd( x1dot*x2dot + y1dot*y2dot ) );

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

p.set_start_time_delta(t);

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
   //  We don't have to use "is_eq" to check whether each weight is
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







