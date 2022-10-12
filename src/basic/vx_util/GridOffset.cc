// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   GridOffset.cc
//
//   Description:
//      Class implementing grid index points as described as
//      offsets from an origin.
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

using namespace std;

///////////////////////////////////////////////////////////////////////////////

GridOffset::GridOffset(int cur_x_offset, int cur_y_offset)
{
  this->x_offset = cur_x_offset;
  this->y_offset = cur_y_offset;
}

///////////////////////////////////////////////////////////////////////////////

GridOffset::GridOffset(const GridOffset& rhs)
{
  x_offset = rhs.x_offset;
  y_offset = rhs.y_offset;
}

///////////////////////////////////////////////////////////////////////////////

GridOffset::GridOffset(const GridOffset* rhs)
{
  x_offset = rhs->x_offset;
  y_offset = rhs->y_offset;
}

///////////////////////////////////////////////////////////////////////////////

GridOffset::~GridOffset(void)
{
}

///////////////////////////////////////////////////////////////////////////////
