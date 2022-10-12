// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   GridPoint.cc
//
//   Description:
//      Class implementing grid index points.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    01-01-99  Rehak           Initial version.
//
///////////////////////////////////////////////////////////////////////////////

#include <vector>

#include <math.h>
#include <cstdio>

#include <GridOffset.h>
#include <GridPoint.h>
using namespace std;

///////////////////////////////////////////////////////////////////////////////

GridPoint::GridPoint(int cur_x, int cur_y)
{
  setPoint(cur_x, cur_y);
}

///////////////////////////////////////////////////////////////////////////////

GridPoint::GridPoint(GridPoint *point)
{
  setPoint(point);
}

///////////////////////////////////////////////////////////////////////////////

GridPoint::GridPoint(GridPoint *point, GridOffset *offset)
{
  setPoint(point, offset);
}

///////////////////////////////////////////////////////////////////////////////

GridPoint::~GridPoint(void)
{
}

///////////////////////////////////////////////////////////////////////////////
//
// Rotate the point about the origin by the given angle.
// The angle value must be given in degrees.
//
///////////////////////////////////////////////////////////////////////////////

void GridPoint::rotate(const double angle)
{
	//const double M_PI = 3.14159265358979323846;
  double angle_rad = angle * M_PI / 180.0;
  double cosa = cos(angle_rad);
  double sina = sin(angle_rad);

  double new_x = x * cosa + y * sina;
  double new_y = -x * sina + y * cosa;

  x = (int)(new_x + 0.5);
  y = (int)(new_y + 0.5);
}

///////////////////////////////////////////////////////////////////////////////
