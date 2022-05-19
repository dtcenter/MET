// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   RectangularTemplate.h
//
//   Description:
//      Class implementing a Rectangular template to be
//      applied on gridded data.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    01-01-07  Megenhardt      Initial version.
//   001    09-07-21  Halley Gotway   Add wrap_lon.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __RECTANGULAR_TEMPLATE_H__
#define __RECTANGULAR_TEMPLATE_H__

///////////////////////////////////////////////////////////////////////////////

#include <vector>

#include <cstdio>

#include <GridOffset.h>
#include <GridPoint.h>
#include <GridTemplate.h>

///////////////////////////////////////////////////////////////////////////////

class RectangularTemplate : public GridTemplate {

   public:

      RectangularTemplate(int height, int width, bool wrap_lon);
      virtual ~RectangularTemplate(void);

      void printOffsetList(FILE *stream);

      // Access methods

      int getHeight(void) const {
         return _height;
      }

      int getWidth(void) const {
         return _width;
      }

      const char* getClassName(void) const {
         return RectangularTemplate::_className();
      }

   private:

      int _height;
      int _width;

      // Return the class name for error messages.
      static const char* _className(void) {
         return("RectangularTemplate");
      }
};

///////////////////////////////////////////////////////////////////////////////

#endif   //  __RECTANGULAR_TEMPLATE_H__

///////////////////////////////////////////////////////////////////////////////
