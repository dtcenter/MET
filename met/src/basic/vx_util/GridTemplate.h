// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Boulder, Colorado, USA
// ** BSD licence applies - redistribution and use in source and binary
// ** forms, with or without modification, are permitted provided that
// ** the following conditions are met:
// ** 1) If the software is modified to produce derivative works,
// ** such modified software should be clearly marked, so as not
// ** to confuse it with the version available from UCAR.
// ** 2) Redistributions of source code must retain the above copyright
// ** notice, this list of conditions and the following disclaimer.
// ** 3) Redistributions in binary form must reproduce the above copyright
// ** notice, this list of conditions and the following disclaimer in the
// ** documentation and/or other materials provided with the distribution.
// ** 4) Neither the name of UCAR nor the names of its contributors,
// ** if any, may be used to endorse or promote products derived from
// ** this software without specific prior written permission.
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 19:21:31 $
 *   $Id: GridTemplate.hh,v 1.9 2016/03/03 19:21:31 dixon Exp $
 *   $Revision: 1.9 $
 *   $State: Exp $
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * GridTemplate.hh: class implementing a template to be applied to
 *                  gridded data.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef GridTemplate_HH
#define GridTemplate_HH

#include <vector>
#include <cstdio>
#include <memory>
#include "GridOffset.h"
#include "GridPoint.h"

using namespace std;

class GridTemplate
{
 public:

  // Constructors

  GridTemplate(void);
  GridTemplate(const GridTemplate& rhs);

  // Destructor

  virtual ~GridTemplate(void);

  // Methods for iterating through the template within the grid centered
  // on the given point.  To use these methods, first call getFirstInGrid()
  // to get the first point.  Then call getNextInGrid() to get all remaining
  // points until a (GridPoint *)NULL is returned.  Any time getFirstInGrid()
  // is called, the iteration will be cleared and will start over again.
  //
  // base_x and base_y give the coordinates of the point around which the
  // template is to be applied.
  //
  // These routines return a point to a static object which must NOT be
  // deleted by the calling routine.

  GridPoint *getFirstInGrid(const int &base_x, const int &base_y,
                            const int &nx, const int &ny) const;
  GridPoint *getNextInGrid(void) const;

  GridPoint *getFirst(const int &base_x, const int &base_y,
                      const int &nx, const int &ny) const;
  GridPoint *getNext(void) const;

  GridPoint *getFirstInLftEdge(void) const;
  GridPoint *getNextInLftEdge(void)  const;

  GridPoint *getFirstInRgtEdge(void) const;
  GridPoint *getNextInRgtEdge(void)  const;

  GridPoint *getFirstInTopEdge(void) const;
  GridPoint *getNextInTopEdge(void)  const;

  GridPoint *getFirstInBotEdge(void) const;
  GridPoint *getNextInBotEdge(void)  const;

  void       setGrid(const int &base_x, const int &base_y,
                     const int &nx, const int &ny) const;

  void       incBaseX(const int &x_inc) const;
  void       incBaseY(const int &y_inc) const;

  // Printing methods

  void printOffsetList(FILE *stream);

  // Access methods

  inline void addOffset(const GridOffset &offset)
  {
    _offsetList.push_back(new GridOffset(offset.x_offset,
                                         offset.y_offset));
  }

  inline void addOffset(const int x_offset, const int y_offset)
  {
    _offsetList.push_back(new GridOffset(x_offset, y_offset));
  }

  int size(void) const
  {
    return _offsetList.size();
  }

  int getNumPts(void) const
  {
    return _offsetList.size();
  }

  virtual const char* getClassName(void) const = 0;

  virtual int getWidth() const = 0;

 protected:

  // The offsets that make up the circle

  vector< GridOffset* > _offsetList;

  // The offsets that define the first and last rows and columns

  vector< GridOffset* > _offsetLftEdge;   // not allocated
  vector< GridOffset* > _offsetRgtEdge;  // not allocated
  vector< GridOffset* > _offsetTopEdge;    // not allocated
  vector< GridOffset* > _offsetBotEdge; // not allocated

  // Iterator for finding points within a grid

  mutable vector< GridOffset* >::const_iterator _pointInGridIterator;
  mutable vector< GridOffset* >::const_iterator _pointInLftEdgeIterator;
  mutable vector< GridOffset* >::const_iterator _pointInRgtEdgeIterator;
  mutable vector< GridOffset* >::const_iterator _pointInTopEdgeIterator;
  mutable vector< GridOffset* >::const_iterator _pointInBotEdgeIterator;

  mutable GridPoint _pointInGridBase;
  mutable int _pointInGridNumX;
  mutable int _pointInGridNumY;
  mutable GridPoint _pointInGridReturn;

  // Add the given offset to the offset list

  void _addOffset(int x_offset, int y_offset);

  // Determine the offsets for the First/Last Row/Column

  void _setEdgeOffsets();

};

// The factory knows about child classes and can create them for you

class GridTemplateFactory {

 public:

   // constructor
   GridTemplateFactory();

   // destructor
   ~GridTemplateFactory();

   // do not assign specific values to these enumes.
   // other code requires them to start at zero and increase by 1
   // make sure GridTemplate_NUM_TEMPLATES is always last.
   // TODO: rename this Shape
   enum GridTemplates {
      GridTemplate_None,
      GridTemplate_Square,
      GridTemplate_Circle,
      //GridTemplate_Rectangle,
      GridTemplate_NUM_TEMPLATES
   };

   // String corresponding to the enumerated values above
    vector<string>      enum_to_string;

/////////////////////////////////////////////////////////////////
// exit on failure
   GridTemplates string2Enum(string target);

//////////////////////////////////////////////////////////////
// exit on failure
   string enum2String(GridTemplates gt);

/////////////////////////////////////////////////////////////////
// Caller assumes ownership of the returned pointer.
   GridTemplate* buildGT(string gt, int width);

/////////////////////////////////////////////////////////////////
// Caller assumes ownership of the returned pointer.
   GridTemplate* buildGT(GridTemplates gt, int width);

   //FUTURE DEV NOTE:
   // If we ever decide to support rectangles or elipses, we are going to
   // need to modify these methods to take more than just a single width & shape
   // to define the grid template.  I suggest building a structure/union that holds
   // all the defining characteristics of a grid template (shape, height, width, radius, etc.)
   // and passing that around everywhere that we currently pass around a shape & width.


};

#endif
