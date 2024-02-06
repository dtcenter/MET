// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
//   001    22-09-28  Prestopnik      MET #2227 Remove namespace std from header files
//
////////////////////////////////////////////////////////////////////////

#ifndef __SHAPE_H__
#define __SHAPE_H__

////////////////////////////////////////////////////////////////////////

#include "moments.h"

////////////////////////////////////////////////////////////////////////

class Shape {

   public:

      virtual ~Shape();

      virtual Moments moments()                    const = 0;
      virtual void    calc_moments()               const = 0;
      virtual void    centroid(double &, double &) const = 0;
      virtual double  angle_degrees()              const = 0;
      virtual double  area()                       const = 0;
      virtual double  length()                     const = 0;
      virtual double  width()                      const = 0;
      virtual void    clear()                      const = 0;
};

////////////////////////////////////////////////////////////////////////

#endif // __SHAPE_H__

////////////////////////////////////////////////////////////////////////
