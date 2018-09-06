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
//   $Id: RectangularTemplate.cc,v 1.4 2016/03/03 18:19:27 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * RectangularTemplate.cc: class implementing a Rectangular template
 *                      to be applied on gridded data.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2007
 *
 * Dan Megenhardt
 *
 *********************************************************************/

#include <vector>

#include <math.h>
#include <cstdio>

#include <RectangularTemplate.h>
#include <GridOffset.h>
#include <GridTemplate.h>
using namespace std;

/**********************************************************************
 * Constructor
 */

RectangularTemplate::RectangularTemplate(int height, int width) :
  GridTemplate(),
  _height(height),
  _width(width)
{

  /*
    CONVENTION:
    If width is even, we push the center to the lower left corner
    and have the "extra" in the right & above.
  */

  bool evenWidth = ((width % 2) == 0);
  bool evenHeight = ((height % 2) == 0);

  // equivalent of w/2 for even & (w-1)/2 for odd
  int halfwidth = width / 2;
  int halfheight = height / 2;

  int xmin = halfwidth;
  int ymin = halfheight;

  if (evenWidth){
    xmin--;
  }
  if (evenHeight){
    ymin--;
  }

  // Create the offsets list
  for (int y = -ymin; y <= halfheight; y++)
  {
    for (int x = -xmin; x <= halfheight; x++)
    {
      _addOffset(x, y);
    } /* endfor x = 0 */
  } /* endfor y = 0*/

  _setEdgeOffsets();
}


/**********************************************************************
 * Destructor
 */

RectangularTemplate::~RectangularTemplate(void)
{
  // Do nothing
}


/**********************************************************************
 * printOffsetList() - Print the offset list to the given stream.  This
 *                     is used for debugging.
 */

void RectangularTemplate::printOffsetList(FILE *stream)
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Rectangular template:");
  fprintf(stream, "    height = %d\n", _height);
  fprintf(stream, "    width = %d\n", _width);

  fprintf(stream, " grid points:\n");

  GridTemplate::printOffsetList(stream);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
