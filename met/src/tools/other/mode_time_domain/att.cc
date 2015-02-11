

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
#include "att.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SingleAttributes
   //


////////////////////////////////////////////////////////////////////////


SingleAttributes::SingleAttributes()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


SingleAttributes::~SingleAttributes()

{

clear();

}


////////////////////////////////////////////////////////////////////////


SingleAttributes::SingleAttributes(const SingleAttributes & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


SingleAttributes & SingleAttributes::operator=(const SingleAttributes & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void SingleAttributes::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAttributes::clear()

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


void SingleAttributes::assign(const SingleAttributes & a)

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


void SingleAttributes::centroid (double & _xbar, double & _ybar, double & _tbar) const

{

_xbar = Xbar;
_ybar = Ybar;
_tbar = Tbar;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAttributes::bounding_box (int & _xmin, int & _xmax,
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


void SingleAttributes::dump(ostream & out, int depth) const

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


void SingleAttributes::set_centroid(double _xbar, double _ybar, double _tbar)

{

Xbar = _xbar;
Ybar = _ybar;
Tbar = _tbar;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAttributes::set_bounding_box (int _xmin, int _xmax,
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


void SingleAttributes::set_velocity(double vx, double vy)

{

Xvelocity = vx;

Yvelocity = vy;


return;

}


////////////////////////////////////////////////////////////////////////


void SingleAttributes::set_spatial_axis(double angle)

{

   //
   //  reduce angle to range (-90, 90]
   //

angle += 180.0*floor((90.0 - angle)/180.0);

SpatialAxisAngle = angle;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAttributes::velocity(double & vx, double & vy) const

{

vx = Xvelocity;

vy = Yvelocity;


return;

}


////////////////////////////////////////////////////////////////////////


double SingleAttributes::speed() const

{

double s;

s = sqrt( Xvelocity*Xvelocity + Yvelocity*Yvelocity );

return ( s );

}


////////////////////////////////////////////////////////////////////////


int SingleAttributes::n_times() const

{

int n;

n = Tmax - Tmin + 1;

return ( n );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PairAttributes
   //


////////////////////////////////////////////////////////////////////////


PairAttributes::PairAttributes()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


PairAttributes::~PairAttributes()

{

clear();

}


////////////////////////////////////////////////////////////////////////


PairAttributes::PairAttributes(const PairAttributes & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


PairAttributes & PairAttributes::operator=(const PairAttributes & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void PairAttributes::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void PairAttributes::clear()

{

IntersectionVol = 0;

UnionVol = 0;

TimeCentroidDist = 0.0;

SpaceCentroidDist = 0.0;

DirectionDifference = 0.0;

SpeedDifference = 0.0;

FcstObjectNumber = -1;
ObsObjectNumber = -1;

return;

}


////////////////////////////////////////////////////////////////////////


void PairAttributes::assign(const PairAttributes & a)

{

clear();



IntersectionVol = a.IntersectionVol;

UnionVol = a.UnionVol;

TimeCentroidDist = a.TimeCentroidDist;

SpaceCentroidDist = a.SpaceCentroidDist;

DirectionDifference = a.DirectionDifference;

SpeedDifference = a.SpeedDifference;

FcstObjectNumber = a.FcstObjectNumber;
ObsObjectNumber = a.ObsObjectNumber;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


SingleAttributes calc_single_atts(const Object & obj, const Object & raw, const char * model, int obj_number)

{

int k, x, y, t;
int n, Vol;
SingleAttributes a;
double bbox_volume;
double xbar, ybar, tbar;
double dx, dy, dt;
double sxx, syy, stt, sxy, sxt, syt;
double rho_cos_alpha, rho_sin_alpha;
double alpha, theta;
ConcatString raw_filename;
double * values = (double *) 0;


a.ObjectNumber = obj_number;

obj.centroid(xbar, ybar, tbar);

a.Xbar = xbar;
a.Ybar = ybar;
a.Tbar = tbar;

a.Volume = obj.volume();

obj.bbox(a.Xmin, a.Xmax, a.Ymin, a.Ymax, a.Tmin, a.Tmax);

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

   sxx = syy = stt = sxy = sxt = syt = 0.0;

   k = 0;

   for (x=0; x<(obj.nx()); ++x)  {

      dx = x - xbar;

      for (y=0; y<(obj.ny()); ++y)  {

         dy = y - ybar;

         for (t=0; t<(obj.nt()); ++t)  {

            dt = t - tbar;

            if ( ! obj.s_is_on(x, y, t) )  continue;

            ++k;

            sxx += dx*dx;
            syy += dy*dy;
            stt += dt*dt;

            sxy += dx*dy;
            sxt += dx*dt;
            syt += dy*dt;

         }   //  for t

      }   //  for y

   }   //  for x

   a.Xvelocity = sxt/stt;

   a.Yvelocity = syt/stt;

   rho_cos_alpha = 2.0*(sxy*stt - sxt*syt);

   rho_sin_alpha = syy*stt - sxx*stt + sxt*sxt - syt*syt;

   alpha = atan2d(rho_sin_alpha, rho_cos_alpha);

   theta = 0.5*alpha - 45.0;

   // cout << "K      = " << k      << "\n";
   // cout << "a.Ymin = " << a.Ymin << "\n";
   // cout << "Theta  = " << theta  << "\n";
   // cout << "alpha  = " << alpha  << "\n";
   // cout << "rca    = " << rho_cos_alpha  << "\n";
   // cout << "rsa    = " << rho_sin_alpha  << "\n";

   a.set_spatial_axis(theta + 90.0);

}   //  else

// cout << "Sxx = " << sxx << "\n" << flush;
// cout << "Syy = " << syy << "\n" << flush;
// cout << "Stt = " << stt << "\n" << flush;

// cout << "\n";

// cout << "Sxy = " << sxy << "\n" << flush;
// cout << "Sxt = " << sxt << "\n" << flush;
// cout << "Syt = " << syt << "\n" << flush;

   //
   //  percentiles
   //

Vol = a.Volume;

values = new double [Vol];

if ( !values )  {

   cerr << "\n\n  calc_single_atts() -> memory allocation error\n\n";

   exit ( 1 );

}

n = 0;

for (x=0; x<(raw.nx()); ++x)  {

   for (y=0; y<(raw.ny()); ++y)  {

      for (t=0; t<(raw.nt()); ++t)  {

         if ( ! obj.s_is_on(x, y, t) )  continue;

         if ( n >= Vol )  {

            cerr << "\n\n  calc_single_atts() -> bad volume count!\n\n";

            exit ( 1 );

         }

         values[n++] = raw(x, y, t);

      }

   }

}

sort(values, n);

a.Ptile_10 = percentile(values, n, 0.10);
a.Ptile_25 = percentile(values, n, 0.25);
a.Ptile_50 = percentile(values, n, 0.50);
a.Ptile_75 = percentile(values, n, 0.75);
a.Ptile_90 = percentile(values, n, 0.90);

   //
   //   done
   //

if ( values )  { delete [] values;  values = (double *) 0; }

// a.dump(cout);

return ( a );

}


////////////////////////////////////////////////////////////////////////


PairAttributes calc_pair_atts(const Object & fcst_obj,
                              const Object & obs_obj,
                              const SingleAttributes & fcst_att,
                              const SingleAttributes & obs_att)

{

int x, y, t;
int IV, UV;
PairAttributes a;
double dx, dy;
double x1dot, x2dot, y1dot, y2dot;
double b;
bool obs_on  = false;
bool fcst_on = false;


a.FcstObjectNumber = fcst_att.object_number();
a.ObsObjectNumber  =  obs_att.object_number();

   //
   //  intersection and union volumes
   //

IV = UV = 0;

for (x=0; x<(fcst_obj.nx()); ++x)  {

   for (y=0; y<(fcst_obj.ny()); ++y)  {

      for (t=0; t<(fcst_obj.nt()); ++t)  {

         obs_on =  obs_obj.s_is_on(x, y, t);
        fcst_on = fcst_obj.s_is_on(x, y, t);

        if ( fcst_on || obs_on ) ++UV;
        if ( fcst_on && obs_on ) ++IV;

      }

   }

}

a.IntersectionVol = IV;
a.UnionVol        = UV;

   //
   //  centroid distances
   //

a.TimeCentroidDist = fcst_att.tbar() - obs_att.tbar();

dx = fcst_att.xbar() - obs_att.xbar();
dy = fcst_att.ybar() - obs_att.ybar();

a.SpaceCentroidDist = sqrt( dx*dx + dy*dy );

fcst_att.velocity(x1dot, y1dot);
 obs_att.velocity(x2dot, y2dot);

   //
   //  speed and direction differences
   //

a.SpeedDifference = fcst_att.speed() - obs_att.speed();;

b = sqrt( x1dot*x1dot + y1dot*y1dot );

x1dot /= b;
y1dot /= b;

b = sqrt( x2dot*x2dot + y2dot*y2dot );

x2dot /= b;
y2dot /= b;

a.DirectionDifference = acosd( x1dot*x2dot + y1dot*y2dot );


   //
   //  done
   //

return ( a );

}


////////////////////////////////////////////////////////////////////////







