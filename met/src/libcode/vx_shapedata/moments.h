// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   moments.h
//
//   Description:
//      Contains the declaration of the Moments class which is
//      used to track the 1st, 2nd, and 3rd order moments for
//      geometric shapes.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    11-03-06  Halley Gotway   New
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MOMENTS_H__
#define __MOMENTS_H__

///////////////////////////////////////////////////////////////////////////////

#include <math.h>

///////////////////////////////////////////////////////////////////////////////

class Moments {

   private:

      // Copy the contents of the argument into the object
      void assign(const Moments &);

   public:

      // Areas based on s_is_on and f_is_on logic
      double s_area;
      double f_area;

      // First order moments
      double sx;
      double sy;

      // Second order moments (sxy = syx)
      double sxx;
      double sxy;
      double syy;

      // Third order moments (sxxy = sxyx = syxx, sxyy = syxy = syyx)
      double sxxx;
      double sxxy;
      double sxyy;
      double syyy;

      // Constructors
      Moments();
      Moments(const Moments &);

      // Desctructor
      ~Moments();

      Moments & operator=(const Moments &);

      // Compute effects on moments for the following operations
      void translate(double, double);
      void rotate_ccw(double degrees);
      void scale(double);
      void scale(double, double);

      // Compute effects on moments for translating to the centroid
      void center();

      // Apply affine coordinate transformation to moments
      void transform(double m11, double m12, double m21,
                     double m22, double b1, double b2);

      // Compute the xbar, ybar coordinates for an object's centroid
      void centroid(double &, double &) const;

      // Compute the axis angle
      double angle_degrees() const;

      // Compute the center and radius of curvature for an object
      double curvature(double &, double &) const;

      // Reset class variables to default values
      void clear();
};

///////////////////////////////////////////////////////////////////////////////

#endif   // __MOMENTS_H__

///////////////////////////////////////////////////////////////////////////////
