// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   GridOffset.h
//
//   Description:
//      Class implementing grid index points as described as
//      offsets from an origin.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    01-01-99  Rehak           Initial version.
//   001    09-06-22  Prestopnik      MET #2227 Remove namesapce std from header files
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __GRID_OFFSET_H__
#define __GRID_OFFSET_H__

///////////////////////////////////////////////////////////////////////////////

#include <cstdio>

///////////////////////////////////////////////////////////////////////////////

class GridOffset
{
 public:

  // Constructors

  GridOffset(int x_offset = 0, int y_offset = 0);
  GridOffset(const GridOffset& rhs);
  GridOffset(const GridOffset* rhs);
  
  // Destructor

  ~GridOffset(void);
  
  // The actual offset values

  int x_offset;
  int y_offset;
  
};

///////////////////////////////////////////////////////////////////////////////

#endif   //  __GRID_OFFSET_H__

///////////////////////////////////////////////////////////////////////////////
