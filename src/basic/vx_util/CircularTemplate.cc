// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   CircularTemplate.cc
//
//   Description:
//      Class implementing a Circular template to be
//      applied on gridded data.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    01-01-99  Rehak           Initial version.
//   001    09-07-21  Halley Gotway   Add wrap_lon.
//
///////////////////////////////////////////////////////////////////////////////

#include <vector>

#include <math.h>
#include <cstdio>

#include "logger.h"

#include <CircularTemplate.h>
#include <GridOffset.h>
#include <GridTemplate.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

CircularTemplate::CircularTemplate(const int width, bool wrap_lon) :
   GridTemplate(), _width(width) {

   _wrapLon = wrap_lon;

	// width of 2 is not supported
	if (width == 2) {
      mlog << Error << "\nCircularTemplate::CircularTemplate() -> "
           << "unsupported width of " << width << " for circles.\n\n";
		exit(1);
	}

   bool evenWidth = ((width % 2) == 0);

   // if the width is even, that means we are dealing with a point interpolation
   // because grid interpolation has to be odd.
   // for an ODD WIDTH the reference point is the same as the center point and is the nearest grid point
   // for an EVEN WIDTH, we move the "reference" point, to the lower left grid point,
   // this means offsets are stored relative to the lower left corner of the true center.
   // but we find distances based on the the true center location when determining if an
   // offset is within the circle.

   double radius = (width-1)/2.0;
	
   // Create the offsets list.
	
   // Need to increase the area we look at if the width is even, because
   // some valid offset points will actually be farther from the reference point
   // than the radius, because the reference point is offset from the true
   // center of the circle.

   int maxOffset = static_cast<int>(floor(radius));
   if(evenWidth) maxOffset++;
  
   int minOffset = static_cast<int>(floor(-1 * radius));
	  
   for(int y = minOffset; y <= maxOffset; y++) {
      for(int x = minOffset; x <= maxOffset; x++) {
         double double_x = (double)x;
         double double_y = (double)y;

	      if(evenWidth) {
            // if width is even, the reference point is actually shifted 1/2 a grid spacing down and to the left,
            // from the true center of the circle.
		      //
		      // so when we calculate distance, we need to subtract .5 so that the distance reflects the distance from the center
		      // of the circle, instead of the distance from the reference.
		      //
		      // for example - a circle with width == 4.  The reference point is the lower left corner of the center square.
		      // the point directly below that is at (0,-1), but it's actually (-.5, -1.5) from the center of the circle.
		      //
		      // another example - same circle.  The point directly to the right of the reference point is (1,0), but it's
		      // actually (.5,-.5) from the center.
		   
            double_x -= 0.5;
            double_y -= 0.5;
         }
         double distance= sqrt((double_x * double_x) + (double_y * double_y));

	      if(distance <= radius) _addOffset(x, y);

      } // end for x
   } // end for y

   _setEdgeOffsets();

}

///////////////////////////////////////////////////////////////////////////////

CircularTemplate::~CircularTemplate(void) {
   // Do nothing
}

///////////////////////////////////////////////////////////////////////////////

void CircularTemplate::printOffsetList(FILE *stream) {
   fprintf(stream, "\n\n");
   fprintf(stream, "Circular template:");
   fprintf(stream, "    width = %d\n", _width);
   fprintf(stream, "    wrap_lon = %d\n", _wrapLon);
   fprintf(stream, " grid points:\n");

   GridTemplate::printOffsetList(stream);
}

///////////////////////////////////////////////////////////////////////////////
