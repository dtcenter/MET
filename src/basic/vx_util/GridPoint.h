// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   GridPoint.h
//
//   Description:
//      Class implementing grid index points.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    01-01-99  Rehak           Initial version.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __GRID_POINT_H__
#define __GRID_POINT_H__

///////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "GridOffset.h"

///////////////////////////////////////////////////////////////////////////////

class GridPoint
{
 public:

  // Constructors

  GridPoint(int x = 0, int y = 0);
  explicit GridPoint(GridPoint *point);
  GridPoint(GridPoint *point, GridOffset *offset);

  // Destructor

  ~GridPoint(void);


  ////////////////////
  // Access methods //
  ////////////////////


  inline int getIndex(const int nx, const int ny) const
  {
    return (y * nx) + x;
  }

  inline void setPoint(const int cur_x, const int cur_y)
  {
    this->x = cur_x;
    this->y = cur_y;
  }

  inline void setPoint(const GridPoint *point)
  {
    x = point->x;
    y = point->y;
  }

  inline void setPoint(const GridPoint &point)
  {
    x = point.x;
    y = point.y;
  }

  inline void setPoint(const GridPoint *point, const GridOffset *offset)
  {
    x = point->x + offset->x_offset;
    y = point->y + offset->y_offset;
  }

  inline void setPoint(const GridPoint &point, const GridOffset &offset)
  {
    x = point.x + offset.x_offset;
    y = point.y + offset.y_offset;
  }


  /////////////////////
  // Utility methods //
  /////////////////////

  // Rotate the point about the origin by the given angle.  The angle
  // value must be given in degrees.

  void rotate(const double angle);


  ///////////////
  // Operators //
  ///////////////

  bool operator==(const GridPoint &other) const
  {
    return (this->x == other.x &&
	    this->y == other.y);
  }


  bool operator!=(const GridPoint &other) const
  {
    return (this->x != other.x ||
	    this->y != other.y);
  }


  ////////////////////
  // Public members //
  ////////////////////

  // The actual offset values

  int x;
  int y;


 private:

  /////////////////////
  // Private methods //
  /////////////////////

  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("GridPoint");
  }

};

///////////////////////////////////////////////////////////////////////////////

#endif   //  __GRID_POINT_H__

///////////////////////////////////////////////////////////////////////////////
