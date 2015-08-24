

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

Area = 0;

Xbar = Ybar = 0;

AxisAngle = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


void SingleAtt2D::assign(const SingleAtt2D & a)

{

clear();

ObjectNumber = a.ObjectNumber;

Area = a.Area;

Xbar = a.Xbar;
Ybar = a.Ybar;

AxisAngle = a.AxisAngle;



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

out << prefix << "Area       = " << Area << "\n";
out << prefix << "Centroid   = " << '(' << Xbar << ", " << Ybar << ")\n";
out << prefix << "AxisAngle  = " << AxisAngle << "\n";

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


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


SingleAtt2D calc_2d_single_atts(const Object & obj, const MtdFloatFile & raw, const char * model, int obj_number)

{

// int j;
// int n, Vol;
SingleAtt2D a;
// double bbox_volume;
ConcatString raw_filename;
// float * values = (float *) 0;
// const int   * i = 0;
// const float * r = 0;
Mtd_2D_Moments moments;
MtdIntFile f;
// const int n3 = (obj.nx())*(obj.ny())*(obj.nt());


a.ObjectNumber = obj_number;

f = obj.select(obj_number + 1);

moments = f.calc_2d_moments();

if ( moments.N == 0 )  {

   cerr << "\n\n  calc_2d_single_atts() -> empty object!\n\n";

   exit ( 1 );

}

a.Xbar = (moments.Sx)/(moments.N);
a.Ybar = (moments.Sy)/(moments.N);

a.Area = obj.total_volume();

a.set_axis(moments.calc_2D_axis_plane_angle());


   //
   //  percentiles
   //
/*
Vol = a.Volume;

values = new float [Vol];

if ( !values )  {

   cerr << "\n\n  calc_2d_single_atts() -> memory allocation error\n\n";

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





