// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   moments.cc
//
//   Description:
//      Contains the definition of the Moments class which is
//      used to track the 1st, 2nd, and 3rd order moments for
//      geometric shapes.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    11-03-06  Halley Gotway   New
//
///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <math.h>
#include <stdlib.h>

#include "moments.h"
#include "vx_log.h"
#include "vx_math.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
//  Begin code for class Moments
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Moments::Moments() {
   clear();
}

///////////////////////////////////////////////////////////////////////////////

Moments::~Moments() {
}

///////////////////////////////////////////////////////////////////////////////

Moments::Moments(const Moments &m) {
   assign(m);
}

///////////////////////////////////////////////////////////////////////////////

Moments & Moments::operator=(const Moments &m) {
   if ( this == &m ) {
      return ( *this );
   }
   assign(m);

   return ( *this );
}

///////////////////////////////////////////////////////////////////////////////

void Moments::clear() {
   s_area = 0.0;
   f_area = 0.0;
   sx = sy = sxy = 0.0;
   sxx = syy = 0.0;
   sxxx = sxxy = sxyy = syyy = 0.0;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Moments::assign(const Moments &m) {
   clear();
   s_area = m.s_area;
   f_area = m.f_area;

   sx = m.sx;
   sy = m.sy;

   sxx = m.sxx;
   sxy = m.sxy;
   syy = m.syy;

   sxxx = m.sxxx;
   sxxy = m.sxxy;
   sxyy = m.sxyy;
   syyy = m.syyy;

   return;
}


///////////////////////////////////////////////////////////////////////////////
//
// Apply affine coordinate transformation to a shape's moments
//
///////////////////////////////////////////////////////////////////////////////

void Moments::transform(double m11, double m12, double m21,
         double m22, double b1, double b2) {

   double su, sv, suu, svv, suv, suuu, suuv, suvv, svvv, d;

   d = m11*m22 - m21*m12;

   su = sx;
   sv = sy;

   suu = sxx;
   suv = sxy;
   svv = syy;

   suuu = sxxx;
   suuv = sxxy;
   suvv = sxyy;
   svvv = syyy;

   sx   = d*(m11*su + m12*sv + b1*f_area);

   sy   = d*(m21*su + m22*sv + b2*f_area);

   sxx  = d*(m11*m11*suu + m12*m12*svv + b1*b1*f_area
        + 2.0*m11*m12*suv + 2.0*m11*b1*su + 2.0*m12*b1*sv);

   syy  = d*(m21*m21*suu + m22*m22*svv + b2*b2*f_area
        + 2.0*m21*m22*suv + 2.0*m21*b2*su + 2.0*m22*b2*sv);

   sxy  = d*(m11*m21*suu + m12*m22*svv + (m11*m22 + m12*m21)*suv
          + (m11*b2 + m21*b1)*su + (m22*b1 + m12*b2)*sv
           + b1*b2*f_area);

   sxxx = d*(m11*m11*m11*suuu + m12*m12*m12*svvv
        + b1*b1*b1*f_area + 3.0*m11*m11*m12*suuv + 3.0*m11*m12*m12*suvv
        + 3.0*m11*m11*b1*suu + 3.0*m12*m12*b1*svv
        + 6.0*m11*m12*b1*suv + 3.0*m11*b1*b1*su + 3.0*m12*b1*b1*sv);

   sxxy = d*(m11*m11*m21*suuu + m12*m12*m22*svvv + b1*b1*b2*f_area
        + (m11*m22 + 2.0*m12*m21)*m11*suuv
        + (2.0*m11*m22 + m12*m21)*m12*suvv
        + (m11*b2 + 2.0*m21*b1)*m11*suu
        + (m12*b2 + 2.0*m22*b1)*m12*svv
        + (m11*m12*b2 + m11*m22*b1 + m12*m21*b1)*2.0*suv
        + (2.0*m11*b2 + m21*b1)*b1*su + (2.0*m12*b2 + m22*b1)*b1*sv);

   sxyy = d*(m11*m21*m21*suuu + m12*m22*m22*svvv + b1*b2*b2*f_area
        + (2.0*m11*m22 + m12*m21*m21)*m21*suuv
        + (m11*m22 + 2.0*m12*m21)*m22*suvv
        + (2.0*m11*b2 + m21*b1)*m21*suu
        + (2.0*m12*b2 + m22*b1)*m22*svv
        + (m11*m22*b2 + m12*m21*b2 + m21*m22*b1)*2.0*suv
        + (m11*b2 + 2.0*m21*b1)*b2*su + (m12*b2 + 2.0*m22*b1)*b2*sv);

   syyy = d*(m21*m21*m21*suuu + m22*m22*m22*svvv + b2*b2*b2*f_area
        + 3.0*m21*m21*m22*suuv + 3.0*m21*m22*m22*suvv
        + 3.0*m21*m21*b2*suu + 3.0*m22*m22*b2*svv
        + 6.0*m21*m22*b2*suv + 3.0*m21*b2*b2*su + 3.0*m22*b2*b2*sv);

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// Translate moments 'a' units in x direction and 'b' units in y direction
//
///////////////////////////////////////////////////////////////////////////////

void Moments::translate(double a, double b) {
   transform(1.0, 0.0, 0.0, 1.0, -a, -b);

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// Rotate moments 'degrees' degrees in the counter clockwise direction
//
///////////////////////////////////////////////////////////////////////////////

void Moments::rotate_ccw(double degrees) {
   double c, s;
   double radians = degrees/deg_per_rad;

   c = cos(radians);
   s = sin(radians);

   transform(c, s, -s, c, 0.0, 0.0);

   return;
}


///////////////////////////////////////////////////////////////////////////////
//
// Scale moments in the x and y directions by a factor of s
//
///////////////////////////////////////////////////////////////////////////////

void Moments::scale(double s) {
   scale(s, s);

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// Scale moments in the x and y directions by factors of s1 and s2 resp
//
///////////////////////////////////////////////////////////////////////////////

void Moments::scale(double s1, double s2) {
   transform(1.0/s1, 0.0, 0.0, 1.0/s2, 0.0, 0.0);

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// Compute coordinates (xbar, ybar) of a shape's centroid
//
///////////////////////////////////////////////////////////////////////////////

void Moments::centroid(double &xbar, double &ybar) const {

   if(is_eq(f_area, 0.0)) {
      mlog << Error << "\nMoments::centroid -> "
           << "divide by zero: f_area = " << f_area << "\n\n";

      exit(1);
   }

   xbar = sx/f_area;
   ybar = sy/f_area;

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// Translate moments to a shape's centroid
//
///////////////////////////////////////////////////////////////////////////////

void Moments::center() {
   double xbar, ybar;

   centroid(xbar, ybar);
   translate(xbar, ybar);

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// Compute axis angle (in degrees) of a shape
//
///////////////////////////////////////////////////////////////////////////////

double Moments::angle_degrees() const {
   double deg;
   double xbar, ybar;
   Moments m = *this;

   // Translate to centroid - axis angle is translation invariant
   centroid(xbar, ybar);
   m.translate(xbar, ybar);

   // Compute axis angle using 2nd order moments
   deg = 0.5*deg_per_rad*atan2(2.0*(m.sxy), m.sxx - m.syy);

   return(deg);
}

///////////////////////////////////////////////////////////////////////////////
//
// Compute radius and center of curvature of a shape
//
///////////////////////////////////////////////////////////////////////////////

double Moments::curvature(double &xcurv, double &ycurv) const {
   double radius;
   double xbar, ybar, xcenter, ycenter, a;
   Moments m = *this;

   // Translate to centroid - radius of curvature is translation
   // invariant
   centroid(xbar, ybar);
   m.translate(xbar, ybar);

   // Compute coordinates of center of curvature w.r.t. centroid
   // using 3rd order moments
   xcenter = ( .5*( syy*( sxxx + sxyy ) - sxy*( syyy + sxxy ) ) )
        / ( sxx*syy - sxy*sxy );
   ycenter = ( .5*( sxy*( sxxx + sxyy ) - sxx*( syyy + sxxy ) ) )
        / ( sxy*sxy - sxx*syy );

   a = xcenter*xcenter + ycenter*ycenter + ( ( sxx + syy ) / f_area );
   radius = sqrt( a );

   // Translate center of curvature back away from centroid
   xcurv = xcenter + xbar;
   ycurv = ycenter + ybar;

   return(radius);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
//  End code for class Moments
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
