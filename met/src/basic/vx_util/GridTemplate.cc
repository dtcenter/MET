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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:19:27 $
//   $Id: GridTemplate.cc,v 1.14 2016/03/03 18:19:27 dixon Exp $
//   $Revision: 1.14 $
//   $State: Exp $

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * GridTemplate.cc: class implementing a template to be applied to
 *                  gridded data.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <vector>
#include <map>

#include <math.h>
#include <cstdio>

#include "vx_log.h"

#include "GridTemplate.h"
#include "GridOffset.h"

#include "RectangularTemplate.h"
#include "CircularTemplate.h"

using namespace std;

/**********************************************************************
 * Constructors
 */

GridTemplate::GridTemplate(void)
{
  // Do nothing
}

GridTemplate::GridTemplate(const GridTemplate& rhs)
{
   vector< GridOffset* >::const_iterator offset_iter;

   for (offset_iter = rhs._offsetList.begin();
        offset_iter != rhs._offsetList.end(); ++offset_iter)
      _offsetList.push_back(new GridOffset(*offset_iter));

   _pointInGridBase   = rhs._pointInGridBase;
   _pointInGridNumX   = rhs._pointInGridNumX;
   _pointInGridNumY   = rhs._pointInGridNumY;
   _pointInGridReturn = rhs._pointInGridReturn;
}

/**********************************************************************
 * Destructor
 */

GridTemplate::~GridTemplate(void)
{
  // Reclaim the space for the offset list

  vector< GridOffset* >::iterator list_iter;
  for (list_iter = _offsetList.begin(); list_iter != _offsetList.end();
       ++list_iter)
    delete *list_iter;

  _offsetList.erase(_offsetList.begin(), _offsetList.end());

}

/**********************************************************************
 * getFirstInGrid() - Get the first template grid point within the given
 *                    grid.
 *
 * Returns a pointer to a static object which must NOT be deleted by the
 * calling routine.
 */

GridPoint *GridTemplate::getFirstInGrid(
                            const int &base_x, const int &base_y,
                            const int &nx, const int &ny) const
{
  // Set the grid information

  setGrid(base_x, base_y, nx, ny);

  // Send back the first point

  return getNextInGrid();
}

/**********************************************************************
 * getNextInGrid() - Get the next template grid point within the grid.
 *                   Returns NULL when there are no more points in the
 *                   grid.
 *
 * Returns a pointer to a static object which must NOT be deleted by the
 * calling routine.
 */

GridPoint *GridTemplate::getNextInGrid(void) const
{
  while (_pointInGridIterator != _offsetList.end())
  {
    GridOffset *offset = *_pointInGridIterator;

    _pointInGridIterator++;

    _pointInGridReturn.x = _pointInGridBase.x + offset->x_offset;
    _pointInGridReturn.y = _pointInGridBase.y + offset->y_offset;

    if (_pointInGridReturn.x >= 0 &&
        _pointInGridReturn.x < _pointInGridNumX &&
        _pointInGridReturn.y >= 0 &&
        _pointInGridReturn.y < _pointInGridNumY)
    {
      return &_pointInGridReturn;
    }

  }

  return (GridPoint *)NULL;
}

/**********************************************************************
 * getFirst() - Get the first template grid point without checking
 *              the grid bounds.
 *
 * Returns a pointer to a static object which must NOT be deleted by the
 * calling routine.
 */

GridPoint *GridTemplate::getFirst(const int &base_x, const int &base_y,
                                  const int &nx, const int &ny) const
{
  // Set the grid information
  setGrid(base_x, base_y, nx, ny);

  // Send back the first point
  return getNext();
}

/**********************************************************************
 * getNext() - Get the next template grid point without checking the
 *             grid bounds. Returns NULL when there are no more points.
 *
 * Returns a pointer to a static object which must NOT be deleted by the
 * calling routine.
 */

GridPoint *GridTemplate::getNext(void) const
{
  while (_pointInGridIterator != _offsetList.end())
  {
    GridOffset *offset = *_pointInGridIterator;

    _pointInGridIterator++;

    _pointInGridReturn.x = _pointInGridBase.x + offset->x_offset;
    _pointInGridReturn.y = _pointInGridBase.y + offset->y_offset;

    return &_pointInGridReturn;

  }

  return (GridPoint *)NULL;
}

/**********************************************************************
 * getFirstInLftEdge() - Get the first template grid point in the left
 *                       column.
 *
 * Returns a pointer to a static object which must NOT be deleted by the
 * calling routine.
 */

GridPoint *GridTemplate::getFirstInLftEdge(void) const
{
  // Reset the iterator and send back the first point

  _pointInLftEdgeIterator = _offsetLftEdge.begin();

  return getNextInLftEdge();
}

/**********************************************************************
 * getNextInLftEdge() - Get the next template grid point in the first
 *                      column. Returns NULL when there are no more
 *                      points in the first column.
 *
 * Returns a pointer to a static object which must NOT be deleted by the
 * calling routine.
 */

GridPoint *GridTemplate::getNextInLftEdge(void) const
{
  while (_pointInLftEdgeIterator != _offsetLftEdge.end())
  {
    GridOffset *offset = *_pointInLftEdgeIterator;

    _pointInLftEdgeIterator++;

    _pointInGridReturn.x = _pointInGridBase.x + offset->x_offset;
    _pointInGridReturn.y = _pointInGridBase.y + offset->y_offset;

    if (_pointInGridReturn.x >= 0 &&
        _pointInGridReturn.x < _pointInGridNumX &&
        _pointInGridReturn.y >= 0 &&
        _pointInGridReturn.y < _pointInGridNumY)
    {
      return &_pointInGridReturn;
    }

  }

  return (GridPoint *)NULL;
}

/**********************************************************************
 * getFirstInTopEdge() - Get the first template grid point in the
 *                       top row.
 *
 * Returns a pointer to a static object which must NOT be deleted by the
 * calling routine.
 */

GridPoint *GridTemplate::getFirstInTopEdge(void) const
{
  // Reset the iterator and send back the first point

  _pointInTopEdgeIterator = _offsetTopEdge.begin();

  return getNextInTopEdge();
}

/**********************************************************************
 * getNextInTopEdge() - Get the next template grid point in the top
 *                      row. Returns NULL when there are no more
 *                      points in the top row.
 *
 * Returns a pointer to a static object which must NOT be deleted by the
 * calling routine.
 */

GridPoint *GridTemplate::getNextInTopEdge(void) const
{
  while (_pointInTopEdgeIterator != _offsetTopEdge.end())
  {
    GridOffset *offset = *_pointInTopEdgeIterator;

    _pointInTopEdgeIterator++;

    _pointInGridReturn.x = _pointInGridBase.x + offset->x_offset;
    _pointInGridReturn.y = _pointInGridBase.y + offset->y_offset;

    if (_pointInGridReturn.x >= 0 &&
        _pointInGridReturn.x < _pointInGridNumX &&
        _pointInGridReturn.y >= 0 &&
        _pointInGridReturn.y < _pointInGridNumY)
    {
      return &_pointInGridReturn;
    }

  }

  return (GridPoint *)NULL;
}

/**********************************************************************
 * getFirstInRgtEdge() - Get the first template grid point in the
 *                       right column.
 *
 * Returns a pointer to a static object which must NOT be deleted by the
 * calling routine.
 */

GridPoint *GridTemplate::getFirstInRgtEdge(void) const
{
  // Reset the iterator and send back the first point

  _pointInRgtEdgeIterator = _offsetRgtEdge.begin();

  return getNextInRgtEdge();
}

/**********************************************************************
 * getNextInRgtEdge() - Get the next template grid point in the right
 *                      column. Returns NULL when there are no more
 *                      points in the right column.
 *
 * Returns a pointer to a static object which must NOT be deleted by the
 * calling routine.
 */

GridPoint *GridTemplate::getNextInRgtEdge(void) const
{
  while (_pointInRgtEdgeIterator != _offsetRgtEdge.end())
  {
    GridOffset *offset = *_pointInRgtEdgeIterator;

    _pointInRgtEdgeIterator++;

    _pointInGridReturn.x = _pointInGridBase.x + offset->x_offset;
    _pointInGridReturn.y = _pointInGridBase.y + offset->y_offset;

    if (_pointInGridReturn.x >= 0 &&
        _pointInGridReturn.x < _pointInGridNumX &&
        _pointInGridReturn.y >= 0 &&
        _pointInGridReturn.y < _pointInGridNumY)
    {
      return &_pointInGridReturn;
    }

  }

  return (GridPoint *)NULL;
}

/**********************************************************************
 * getFirstInBotEdge() - Get the first template grid point in the
 *                       bottom row.
 *
 * Returns a pointer to a static object which must NOT be deleted by the
 * calling routine.
 */

GridPoint *GridTemplate::getFirstInBotEdge(void) const
{
  // Reset the iterator and send back the first point

  _pointInBotEdgeIterator = _offsetBotEdge.begin();

  return getNextInBotEdge();
}

/**********************************************************************
 * getNextInBotEdge() - Get the next template grid point in the bottom
 *                      row. Returns NULL when there are no more points
 *                      in the bottom row.
 *
 * Returns a pointer to a static object which must NOT be deleted by the
 * calling routine.
 */

GridPoint *GridTemplate::getNextInBotEdge(void) const
{
  while (_pointInBotEdgeIterator != _offsetBotEdge.end())
  {
    GridOffset *offset = *_pointInBotEdgeIterator;

    _pointInBotEdgeIterator++;

    _pointInGridReturn.x = _pointInGridBase.x + offset->x_offset;
    _pointInGridReturn.y = _pointInGridBase.y + offset->y_offset;

    if (_pointInGridReturn.x >= 0 &&
        _pointInGridReturn.x < _pointInGridNumX &&
        _pointInGridReturn.y >= 0 &&
        _pointInGridReturn.y < _pointInGridNumY)
    {
      return &_pointInGridReturn;
    }

  }

  return (GridPoint *)NULL;
}

/**********************************************************************
 * setGrid() - Initialize the grid dimensions and base location.
 *
 */

void GridTemplate::setGrid(const int &base_x, const int &base_y,
                           const int &nx, const int &ny) const
{
  // Set up the iterators and save the grid information

  _pointInGridIterator    = _offsetList.begin();

  _pointInLftEdgeIterator = _offsetLftEdge.begin();
  _pointInRgtEdgeIterator = _offsetRgtEdge.begin();
  _pointInTopEdgeIterator = _offsetTopEdge.begin();
  _pointInBotEdgeIterator = _offsetBotEdge.begin();

  _pointInGridBase.x = base_x;
  _pointInGridBase.y = base_y;

  _pointInGridNumX = nx;
  _pointInGridNumY = ny;

  return;
}

/**********************************************************************
 * incBaseX() - Increment the base_x location and reset the offset
 *              iterators.
 */

void GridTemplate::incBaseX(const int &x_inc) const
{

  _pointInGridBase.x += x_inc;

  if (_pointInGridBase.x < 0 ||
      _pointInGridBase.x >= _pointInGridNumX ) {
     mlog << Error << "\nGridTemplate::incBaseX() -> "
          << "x (" << _pointInGridBase.x << ") out of range (0, "
          << _pointInGridNumX << ").\n\n";
     exit(1);
  }

  return;
}

/**********************************************************************
 * incBaseY() - Increment the base_y location and reset the offset
 *              iterators.
 */

void GridTemplate::incBaseY(const int &y_inc) const
{

  _pointInGridBase.y += y_inc;

  if (_pointInGridBase.y < 0 ||
      _pointInGridBase.y >= _pointInGridNumY ) {
    mlog << Error << "\nGridTemplate::incBaseY() -> "
         << "y (" << _pointInGridBase.y << ") out of range (0, "
         << _pointInGridNumY << ").\n\n";
    exit(1);
  }

  return;
}

/**********************************************************************
 * printOffsetList() - Print the offset list to the given stream.  This
 *                     is used for debugging.
 */

void GridTemplate::printOffsetList(FILE *stream)
{
  vector< GridOffset* >::iterator ol_iterator;

  for (ol_iterator  = _offsetList.begin();
       ol_iterator != _offsetList.end();
       ol_iterator++)
  {
    GridOffset *offset = *ol_iterator;

    double x = (double)offset->x_offset;
    double y = (double)offset->y_offset;

    double distance = sqrt((x * x) + (y * y));

    fprintf(stream, " %4d %4d   %f\n",
            offset->x_offset, offset->y_offset, distance);

  }

}

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _addOffset() - Add the given offset to the offset list.
 */

void GridTemplate::_addOffset(int x_offset, int y_offset)
{
  GridOffset *offset = new GridOffset(x_offset, y_offset);

  _offsetList.push_back(offset);

  return;
}

/**********************************************************************
 * _setEdgeOffsets() - Process the current offset list and find the
 *                     edges.
 */

void GridTemplate::_setEdgeOffsets()
{
  vector< GridOffset* >::iterator v_iterator;
  map< int, GridOffset* >::iterator m_iterator;
  map< int, GridOffset* > min_x_by_y;
  map< int, GridOffset* > max_x_by_y;
  map< int, GridOffset* > min_y_by_x;
  map< int, GridOffset* > max_y_by_x;
  int x, y;

  // Loop over the offsets.
  // For each row, find the min/max col.
  // For each col, find the min/max row.

  for (v_iterator  = _offsetList.begin();
       v_iterator != _offsetList.end();
       v_iterator++)
  {
    GridOffset *offset = *v_iterator;
    x = offset->x_offset;
    y = offset->y_offset;

    // Min x for each y
    if(min_x_by_y.count(y) == 0)         { min_x_by_y[y] = offset; }
    else if(x < min_x_by_y[y]->x_offset) { min_x_by_y[y] = offset; }

    // Max x for each y
    if(max_x_by_y.count(y) == 0)         { max_x_by_y[y] = offset; }
    else if(x > max_x_by_y[y]->x_offset) { max_x_by_y[y] = offset; }

    // Min y for each x
    if(min_y_by_x.count(x) == 0)         { min_y_by_x[x] = offset; }
    else if(y < min_y_by_x[x]->y_offset) { min_y_by_x[x] = offset; }

    // Max y for each x
    if(max_y_by_x.count(x) == 0)         { max_y_by_x[x] = offset; }
    else if(y > max_y_by_x[x]->y_offset) { max_y_by_x[x] = offset; }

  }

  // Store min_x_by_y map as _offsetLftEdge vector
  for (m_iterator  = min_x_by_y.begin();
       m_iterator != min_x_by_y.end();
       m_iterator++)
  {
     _offsetLftEdge.push_back(m_iterator->second);
  }

  // Store max_x_by_y map as _offsetRgtEdge vector
  for (m_iterator  = max_x_by_y.begin();
       m_iterator != max_x_by_y.end();
       m_iterator++)
  {
     _offsetRgtEdge.push_back(m_iterator->second);
  }

  // Store max_y_by_x map as _offsetTopEdge vector
  for (m_iterator  = max_y_by_x.begin();
       m_iterator != max_y_by_x.end();
       m_iterator++)
  {
     _offsetTopEdge.push_back(m_iterator->second);
  }

  // Store min_y_by_x map as _offsetBotEdge vector
  for (m_iterator  = min_y_by_x.begin();
       m_iterator != min_y_by_x.end();
       m_iterator++)
  {
     _offsetBotEdge.push_back(m_iterator->second);
  }

  return;
}

////////////////////////////////////////////////////////////////
//   GridTemplateFactory
////////////////////////////////////////////////////////////////
GridTemplateFactory::GridTemplateFactory() {
   enum_to_string.resize(GridTemplate_NUM_TEMPLATES);

   enum_to_string[GridTemplate_None] = "";
   enum_to_string[GridTemplate_Square] = "SQUARE";
   enum_to_string[GridTemplate_Circle] = "CIRCLE";
   //enum_to_string[GridTemplate_Rectangle] = "RECTANGLE";
}

GridTemplateFactory::~GridTemplateFactory() {
   enum_to_string.clear();
}

/////////////////////////////////////////////////////////////////
// log & exit on failure
GridTemplateFactory::GridTemplates GridTemplateFactory::string2Enum(string target) {

   for (unsigned int ix = 0; ix < GridTemplate_NUM_TEMPLATES; ix++){
      if (enum_to_string[ix] == target){
         return static_cast<GridTemplates>(ix);
      }
   }
   mlog << Error << "\nGridTemplateFactory::string2Enum() -> "
        << "Unexpected target value of " << target << ".\n\n";
   exit(1);
}


/////////////////////////////////////////////////////////////////
// log & exit on failure
string GridTemplateFactory::enum2String(GridTemplates target) {

   if( static_cast<int>(target) > enum_to_string.size() - 1){

      mlog << Error << "\nGridTemplateFactory::enum2String() -> "
           << "target out of range " << target << " > "
           << (static_cast<int>(enum_to_string.size()) - 1)
           << ".\n\n";
      exit(1);
   }
   return enum_to_string[static_cast<int>(target)];
}

/////////////////////////////////////////////////////////////////
// Caller assumes ownership of the returned pointer.
GridTemplate* GridTemplateFactory::buildGT(string gt, int width) {
   return buildGT(string2Enum(gt), width);
}

/////////////////////////////////////////////////////////////////
// Caller assumes ownership of the returned pointer.
GridTemplate* GridTemplateFactory::buildGT(GridTemplates gt, int width) {

   switch (gt) {
      case(GridTemplate_Square):
         return new RectangularTemplate(width,width);

      case(GridTemplate_Circle):
         return new CircularTemplate(width);

      default:
         mlog << Error << "\nbuildGT() -> "
              << "Unexpected gt value of " << gt << ".\n\n";
         exit(1);
      }
}
