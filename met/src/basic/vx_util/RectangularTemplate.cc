// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2021
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   RectangularTemplate.cc
//
//   Description:
//      Class implementing a Rectangular template to be
//      applied on gridded data.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    01-01-07  Megenhardt      Initial version.
//
///////////////////////////////////////////////////////////////////////////////

#include <vector>

#include <math.h>
#include <cstdio>

#include <RectangularTemplate.h>
#include <GridOffset.h>
#include <GridTemplate.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

RectangularTemplate::RectangularTemplate(int height, int width, bool is_global) :
   GridTemplate(), _height(height), _width(width), _isGlobal(is_global) {

   //
   // CONVENTION:
   // If width is even, we push the center to the lower left corner
   // and have the "extra" in the right & above.
   //

   bool evenWidth = ((width % 2) == 0);
   bool evenHeight = ((height % 2) == 0);

   // equivalent of w/2 for even & (w-1)/2 for odd
   int halfwidth = width / 2;
   int halfheight = height / 2;

   int xmin = halfwidth;
   int ymin = halfheight;

   if(evenWidth)  xmin--;
   if(evenHeight) ymin--;

   // Create the offsets list
   for(int y = -ymin; y <= halfheight; y++) {
      for(int x = -xmin; x <= halfheight; x++) {
         _addOffset(x, y);
      } // end for x
   } // end for y

   _setEdgeOffsets();

}

///////////////////////////////////////////////////////////////////////////////

RectangularTemplate::~RectangularTemplate(void) {
   // Do nothing
}

///////////////////////////////////////////////////////////////////////////////

void RectangularTemplate::printOffsetList(FILE *stream) {
   fprintf(stream, "\n\n");
   fprintf(stream, "Rectangular template:");
   fprintf(stream, "    height = %d\n", _height);
   fprintf(stream, "    width = %d\n", _width);
   fprintf(stream, "    is_global = %d\n", _isGlobal);

   fprintf(stream, " grid points:\n");

   GridTemplate::printOffsetList(stream);
}

///////////////////////////////////////////////////////////////////////////////
