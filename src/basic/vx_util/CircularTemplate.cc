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

// RCS info: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:19:27 $
//   $Id: CircularTemplate.cc,v 1.6 2016/03/03 18:19:27 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * CircularTemplate.cc: class implementing a circular template to be
 *                      applied on gridded data.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <vector>

#include <math.h>
#include <cstdio>

#include "logger.h"

#include <CircularTemplate.h>
#include <GridOffset.h>
#include <GridTemplate.h>

using namespace std;


/**********************************************************************
 * Constructor
 */

CircularTemplate::CircularTemplate(const int width) :
	GridTemplate(),
	_width(width)
{

	//width of 2 is not supported
	if (width == 2){
		mlog << Error << "\nCircularTemplate::CircularTemplate() -> "
                     << "unsupported width of " << width
		     << " for circles.\n\n";
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

	// If the radius is small, then just make the
  // template cover the current grid square.
	// radius < 1 no longer supported.
	/*
  if (radius < 1.0)
  {
    _addOffset(0, 0);
    return;
  }
	*/
	
  // need to increase the area we look at if the width is even, because
  // some valid offset points will actually be farther from the reference point
  // than the radius, because the reference point is offset from the true
  // center of the circle.
  int maxOffset = static_cast<int>(floor(radius));
  if (evenWidth){
	  maxOffset++;
  }
  
  int minOffset = static_cast<int>(floor(-1 * radius));
	  
  for (int y = minOffset; y <= maxOffset; y++)
  {
    for (int x = minOffset; x <= maxOffset; x++)
    {
	    double double_x = (double)x;
	    double double_y = (double)y;

	    if (evenWidth){
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

	    if (distance <= radius)
		    {
			    _addOffset(x, y);
		    }
      
    } /* endfor - x */
    
  } /* endfor - y */

  _setEdgeOffsets();
 
}


/**********************************************************************
 * Destructor
 */

CircularTemplate::~CircularTemplate(void)
{
  // Do nothing
}
  

/**********************************************************************
 * printOffsetList() - Print the offset list to the given stream.  This
 *                     is used for debugging.
 */

void CircularTemplate::printOffsetList(FILE *stream)
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Circular template with width %d grid spaces:\n",
	  _width);
  
  GridTemplate::printOffsetList(stream);
}



/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
