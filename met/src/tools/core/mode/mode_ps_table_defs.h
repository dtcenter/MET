

////////////////////////////////////////////////////////////////////////


// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_PS_TABLE_DEFS__
#define  __MODE_PS_TABLE_DEFS__


////////////////////////////////////////////////////////////////////////


#include "vx_color.h"


////////////////////////////////////////////////////////////////////////


static const Color black (  0,   0,   0);
static const Color white (255, 255, 255);
static const Color green (  0, 255,   0);
static const Color blue  (  0,   0, 255);


static Color light_green = blend_colors(green, white, 0.95);

static Color blue1       = blend_colors(blue,  white, 0.95);
static Color blue2       = blend_colors(blue,  white, 0.85);

static Color light_gray  = blend_colors(white, black, 0.10);
static Color dark_gray   = blend_colors(white, black, 0.50);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_PS_TABLE_DEFS__  */


////////////////////////////////////////////////////////////////////////


