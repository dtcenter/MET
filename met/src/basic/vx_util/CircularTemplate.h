// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2021
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   CircularTemplate.h
//
//   Description:
//      Class implementing a Circular template to be
//      applied on gridded data.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    01-01-99  Rehak           Initial version.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __CIRCULAR_TEMPLATE_H__
#define __CIRCULAR_TEMPLATE_H__

///////////////////////////////////////////////////////////////////////////////

#include <vector>

#include <cstdio>

#include <GridOffset.h>
#include <GridPoint.h>
#include <GridTemplate.h>

///////////////////////////////////////////////////////////////////////////////

class CircularTemplate : public GridTemplate
{
 public:

  // Constructor

  CircularTemplate(int width = 0); 
  
  // Destructor

  virtual ~CircularTemplate(void);
  
  // Print the offset list to the given stream.  This is used for debugging.

  void printOffsetList(FILE *stream);
  
  // Access methods

  int getWidth(void) const
  {
	  return _width;
  }
  
  
  virtual const char* getClassName(void) const{
	  return _className();
  }

 private:

  int _width;
  
  // Return the class name for error messages.
  static const char* _className(void)
  {
    return("CircleTemplate");
  }
  
};

///////////////////////////////////////////////////////////////////////////////

#endif   //  __CIRCULAR_TEMPLATE_H__

///////////////////////////////////////////////////////////////////////////////
