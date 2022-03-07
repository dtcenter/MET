// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   GridTemplate.h
//
//   Description:
//      Class implementing a template to be applied to
//      gridded data.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    01-01-99  Rehak           Initial version.
//   001    09-07-21  Halley Gotway   Add wrap_lon.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __GRID_TEMPLATE_H__
#define __GRID_TEMPLATE_H__

///////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>
#include <memory>
#include "GridOffset.h"
#include "GridPoint.h"

///////////////////////////////////////////////////////////////////////////////

using namespace std;

class GridTemplate {

   public:

      GridTemplate(void);
      GridTemplate(const GridTemplate& rhs);
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

      inline void addOffset(const GridOffset &offset) {
         _offsetList.push_back(new GridOffset(offset.x_offset,
                                              offset.y_offset));
      }

      inline void addOffset(const int x_offset, const int y_offset) {
         _offsetList.push_back(new GridOffset(x_offset, y_offset));
      }

      int size(void) const {
         return _offsetList.size();
      }

      int getNumPts(void) const {
         return _offsetList.size();
      }

      int getWrapLon(void) const {
         return _wrapLon;
      }

      virtual const char* getClassName(void) const = 0;

      virtual int getWidth() const = 0;

   protected:

      bool _wrapLon;

      // The offsets that make up the circle
      vector<GridOffset*> _offsetList;

      // The offsets that define the first and last rows and columns
      vector<GridOffset*> _offsetLftEdge; // not allocated
      vector<GridOffset*> _offsetRgtEdge; // not allocated
      vector<GridOffset*> _offsetTopEdge; // not allocated
      vector<GridOffset*> _offsetBotEdge; // not allocated

      // Iterator for finding points within a grid
      mutable vector<GridOffset*>::const_iterator _pointInGridIterator;
      mutable vector<GridOffset*>::const_iterator _pointInLftEdgeIterator;
      mutable vector<GridOffset*>::const_iterator _pointInRgtEdgeIterator;
      mutable vector<GridOffset*>::const_iterator _pointInTopEdgeIterator;
      mutable vector<GridOffset*>::const_iterator _pointInBotEdgeIterator;

      mutable GridPoint _pointInGridBase;
      mutable int _pointInGridNumX;
      mutable int _pointInGridNumY;
      mutable GridPoint _pointInGridReturn;

      // Add the given offset to the offset list
      void _addOffset(int x_offset, int y_offset);

      // Determine the offsets for the First/Last Row/Column
      void _setEdgeOffsets();
};

///////////////////////////////////////////////////////////////////////////////

// The factory knows about child classes and can create them for you

class GridTemplateFactory {

   public:


      GridTemplateFactory();
      ~GridTemplateFactory();

      // do not assign specific values to these enumes.
      // other code requires them to start at zero and increase by 1
      // make sure GridTemplate_NUM_TEMPLATES is always last.
      enum GridTemplates {
         GridTemplate_None,
         GridTemplate_Square,
         GridTemplate_Circle,
         GridTemplate_NUM_TEMPLATES
      };

      // String corresponding to the enumerated values above
      vector<string> enum_to_string;

      GridTemplates string2Enum(string target);
      string enum2String(GridTemplates gt);

      GridTemplate* buildGT(string gt, int width, bool wrap_lon);
      GridTemplate* buildGT(GridTemplates gt, int width, bool wrap_lon);

};

///////////////////////////////////////////////////////////////////////////////

#endif   //  __GRID_TEMPLATE_H__

///////////////////////////////////////////////////////////////////////////////
