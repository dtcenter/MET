// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   shape.h
//
//   Description:
//      Contains the declaration of the Shape class which is
//      the base class for the Wrfata class.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    11-03-06  Halley Gotway   New
//
////////////////////////////////////////////////////////////////////////

#ifndef __WRFDATA_SHAPE_H__
#define __WRFDATA_SHAPE_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

#include "moments.h"

////////////////////////////////////////////////////////////////////////

class Shape {

   public:

      virtual ~Shape();

      virtual Moments moments() const = 0;
      virtual void calc_moments() = 0;
      virtual void centroid(double &, double &) const = 0;
      virtual double angle_degrees() const = 0;
      virtual double area() const = 0;
      virtual double length() const = 0;
      virtual double width() const = 0;
      virtual void clear() = 0;
};

////////////////////////////////////////////////////////////////////////

class FreelyMoveableShape : public Shape {

   public:

      virtual ~FreelyMoveableShape();

      virtual void translate(double, double) = 0;
      virtual void rotate_ccw(double) = 0;
      virtual void scale(double) = 0;
      virtual void scale(double, double) = 0;

      virtual int is_inside(double, double) const = 0;

};

////////////////////////////////////////////////////////////////////////

#endif // __WRFDATA_SHAPE_H__

////////////////////////////////////////////////////////////////////////
